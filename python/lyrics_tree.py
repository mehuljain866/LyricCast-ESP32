"""
LyricCast Tree-Like Lyrics Engine
Implements a robust multi-tier fallback architecture:
1. Spotify Official Native Lyrics (via sp_dc cookie if configured)
2. LRCLIB Exact Match (filtered by exact track_name, artist_name, album_name, duration)
3. LRCLIB Duration-Ranked Search (chooses result with smallest duration delta)
4. Multi-provider syncedlyrics (NetEase, Megalobiz, Genius fallback)
"""

import os
import urllib.request
import urllib.parse
import json
import re
import syncedlyrics

USER_AGENT = "LyricCast-ESP32/1.0 (https://github.com/mehuljain866/LyricCast-ESP32)"

class SpotifyLyricsTree:
    def __init__(self):
        self.cached_spotify_token = None
        self.spotify_token_expires = 0
        self.detected_sp_dc = None

    def auto_detect_sp_dc(self):
        if self.detected_sp_dc:
            return self.detected_sp_dc
        try:
            import browser_cookie3
            loaders = [
                browser_cookie3.chrome,
                browser_cookie3.edge,
                browser_cookie3.brave,
                browser_cookie3.firefox,
                browser_cookie3.opera
            ]
            for loader in loaders:
                try:
                    cj = loader(domain_name="spotify.com")
                    for c in cj:
                        if c.name == "sp_dc" and c.value:
                            print(f"[Spotify Tree] Auto-detected sp_dc cookie from {loader.__name__}!")
                            self.detected_sp_dc = c.value
                            return self.detected_sp_dc
                except Exception:
                    continue
        except Exception:
            pass
        return None

    def get_spotify_web_token(self, sp_dc):
        import time
        if self.cached_spotify_token and time.time() < self.spotify_token_expires:
            return self.cached_spotify_token
            
        url = "https://open.spotify.com/get_access_token?reason=transport&productType=web_player"
        req = urllib.request.Request(url, headers={
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
            "Cookie": f"sp_dc={sp_dc.strip()}"
        })
        try:
            with urllib.request.urlopen(req, timeout=5) as resp:
                data = json.loads(resp.read().decode('utf-8'))
                token = data.get("accessToken")
                exp = data.get("accessTokenExpirationTimestampMs", 0) / 1000.0
                if token:
                    self.cached_spotify_token = token
                    self.spotify_token_expires = exp - 60
                    return token
        except Exception as e:
            print(f"[Spotify Tree] Spotify web token error: {e}")
        return None

    def search_spotify_track_id(self, token, title, artist):
        query = f"{title} {artist}"
        url = f"https://api.spotify.com/v1/search?q={urllib.parse.quote(query)}&type=track&limit=1"
        req = urllib.request.Request(url, headers={
            "Authorization": f"Bearer {token}",
            "User-Agent": USER_AGENT
        })
        try:
            with urllib.request.urlopen(req, timeout=5) as resp:
                data = json.loads(resp.read().decode('utf-8'))
                items = data.get("tracks", {}).get("items", [])
                if items:
                    return items[0].get("id")
        except Exception as e:
            print(f"[Spotify Tree] Track ID search error: {e}")
        return None

    def fetch_spotify_official(self, sp_dc, title, artist):
        if not sp_dc or len(sp_dc.strip()) < 10:
            return None
        token = self.get_spotify_web_token(sp_dc)
        if not token:
            return None
        track_id = self.search_spotify_track_id(token, title, artist)
        if not track_id:
            return None

        url = f"https://spclient.wg.spotify.com/color-lyrics/v2/track/{track_id}?format=json&market=from_token"
        req = urllib.request.Request(url, headers={
            "Authorization": f"Bearer {token}",
            "App-Platform": "WebPlayer",
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
        })
        try:
            with urllib.request.urlopen(req, timeout=5) as resp:
                data = json.loads(resp.read().decode('utf-8'))
                lines = data.get("lyrics", {}).get("lines", [])
                if lines:
                    lrc_lines = []
                    for line in lines:
                        ms = int(line.get("startTimeMs", 0))
                        seconds = ms / 1000.0
                        m = int(seconds // 60)
                        s = seconds % 60
                        words = line.get("words", "").strip()
                        lrc_lines.append(f"[{m:02d}:{s:05.2f}] {words}")
                    print(f"[Spotify Tree] [Tier 1: Official Spotify API] Lyrics found for '{title}'!")
                    return "\n".join(lrc_lines)
        except Exception as e:
            print(f"[Spotify Tree] Color-lyrics error: {e}")
        return None

    def fetch_lrclib_exact(self, title, artist, album=None, duration_s=None):
        params = {"track_name": title, "artist_name": artist}
        if album:
            params["album_name"] = album
        if duration_s and duration_s > 0:
            params["duration"] = int(round(duration_s))

        url = f"https://lrclib.net/api/get?{urllib.parse.urlencode(params)}"
        req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
        try:
            with urllib.request.urlopen(req, timeout=4) as resp:
                data = json.loads(resp.read().decode('utf-8'))
                lrc = data.get("syncedLyrics")
                if lrc:
                    print(f"[Spotify Tree] [Tier 2: LRCLIB Exact Master Match] Lyrics found ({data.get('duration')}s)!")
                    return lrc
        except Exception:
            pass
        return None

    def fetch_lrclib_duration_ranked(self, title, artist, duration_s=None):
        params = {"track_name": title, "artist_name": artist}
        url = f"https://lrclib.net/api/search?{urllib.parse.urlencode(params)}"
        req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
        try:
            with urllib.request.urlopen(req, timeout=5) as resp:
                results = json.loads(resp.read().decode('utf-8'))
                
            if not results:
                return None

            best_lrc = None
            best_delta = 9999.0
            
            for r in results:
                synced = r.get("syncedLyrics")
                if not synced:
                    continue
                r_dur = r.get("duration")
                if duration_s and r_dur:
                    delta = abs(r_dur - duration_s)
                    if delta < best_delta:
                        best_delta = delta
                        best_lrc = synced
                elif not best_lrc:
                    best_lrc = synced

            if best_lrc and (best_delta <= 5.0 or duration_s is None):
                print(f"[Spotify Tree] [Tier 3: LRCLIB Duration-Ranked] Lyrics matched (delta: {best_delta:.1f}s)!")
                return best_lrc
        except Exception:
            pass
        return None

    def fetch_syncedlyrics_fallback(self, title, artist):
        try:
            lrc = syncedlyrics.search(f"{title} {artist}")
            if lrc:
                print(f"[Spotify Tree] [Tier 4: Syncedlyrics Multi-Provider] Fallback lyrics found!")
                return lrc
        except Exception as e:
            print(f"[Spotify Tree] Syncedlyrics error: {e}")
        return None

    def get_lyrics(self, title, artist, album=None, duration_s=None, sp_dc=None):
        """
        Cascades through the tree:
        0. Local Override (lyrics/ folder)
        1. Official Spotify (auto-detected from browser or configured)
        2. LRCLIB Exact Master Match (album + duration)
        3. LRCLIB Duration-Ranked Search
        4. Multi-Provider Syncedlyrics
        """
        # Tier 0: Local custom .lrc file in lyrics/ folder
        local_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "lyrics"))
        if os.path.exists(local_dir):
            for fname in [f"{title}.lrc", f"{artist} - {title}.lrc", f"{title} - {artist}.lrc"]:
                fpath = os.path.join(local_dir, fname)
                if os.path.exists(fpath):
                    try:
                        with open(fpath, "r", encoding="utf-8") as f:
                            print(f"[Spotify Tree] [Tier 0: Local Override] Loaded {fname}!")
                            return f.read()
                    except Exception:
                        pass

        # Tier 1: Official Spotify (Auto-detected from browser or user input)
        active_sp_dc = sp_dc or self.auto_detect_sp_dc()
        if active_sp_dc:
            lrc = self.fetch_spotify_official(active_sp_dc, title, artist)
            if lrc:
                return lrc

        # Tier 2: LRCLIB Exact Duration Match
        lrc = self.fetch_lrclib_exact(title, artist, album, duration_s)
        if lrc:
            return lrc

        # Tier 3: LRCLIB Duration-Ranked Search
        lrc = self.fetch_lrclib_duration_ranked(title, artist, duration_s)
        if lrc:
            return lrc

        # Tier 4: Multi-provider fallback
        lrc = self.fetch_syncedlyrics_fallback(title, artist)
        if lrc:
            return lrc

        return None

lyrics_tree = SpotifyLyricsTree()
