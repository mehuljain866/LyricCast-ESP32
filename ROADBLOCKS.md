# 🚧 LyricCast: Project Roadblocks & Engineering Log

> *"Basically me slamming until the wall breaks."*  
> This living document tracks the real-world engineering challenges, hardware quirks, and synchronization pitfalls encountered while building **LyricCast** (ESP32 + Spotify Semantic Storyboard Engine).

---

## 1. Timing & Synchronization Roadblocks

### Roadblock 1.1: Crowd-Sourced LRC Version Discrepancies (Album vs. Radio Cuts)
* **The Symptom:** In some tracks (e.g., *Chemtrails Over The Country Club* by Lana Del Rey), the lyrics appeared in the terminal and on screen **28 seconds too early**, completely desynced from what was playing.
* **The Root Cause:** Open community databases (LRCLIB, Musixmatch, NetEase) contain crowd-sourced `.lrc` files uploaded by different users using different audio cuts. When a song has an **Album Version** (e.g., 28-second piano intro) and a **Radio Edit** (intro cut down to 1 second), a naive query for `"Track Title - Artist"` often pulls the Radio Edit LRC file, baking a massive timing offset directly into the lyric timestamps.
* **The Solution / Roadmap:** 
  - Pass exact `duration` and `album_name` from Windows Media Session into the lyrics search query (`/api/get?track_name=...&artist_name=...&duration=...`) to filter out radio edits.
  - Implement a tiered **Tree-Like Fallback Structure**:
    1. **Primary**: Spotify-matched master cut (via exact track duration & album matching).
    2. **Secondary**: Official Musixmatch desktop metadata API.
    3. **Tertiary**: LRCLIB duration-filtered search (±2 second tolerance).
    4. **Fallback**: Fuzzy multi-provider search (`syncedlyrics`).

---

### Roadblock 1.2: Windows Media Session 2x Playback Speed Drift
* **The Symptom:** Lyrics started on time on Line 1, but by 30 seconds into the track, the display was running 10–20 seconds ahead of the music.
* **The Root Cause:** Windows Media Session (`winsdk.windows.media.control`) updates `timeline.position` periodically. In our earlier Python loop, we were adding local elapsed time `((time.time() - local_sync_time) * 1000)` on top of `timeline.position` without accounting for the fact that Windows was *also* advancing `timeline.position`. As a result, playback time was being **double-counted**, running at approximately **2x real speed**.
* **The Solution:** Restored the proven interpolation logic where `local_sync_time` resets strictly when `api_pos_ms` updates from Windows, preventing compounding time drift.

---

### Roadblock 1.3: The Artificial "UTC Delta" 2-Line Premature Jump
* **The Symptom:** At the very start of every song (at 0:00), the screen immediately skipped Line 1 and displayed Line 3.
* **The Root Cause:** In an attempt to improve millisecond precision, we added `delta = (now_utc - timeline.last_updated_time)`. However, `last_updated_time` in Windows Media Session does not update continuously; it records the creation time of the session object (often 5 to 7.5 seconds old). Adding `delta` was artificially adding **+6.0 seconds** to the song position, advancing the lyrics by **exactly 2 lines ahead**.
* **The Solution:** Completely eliminated the artificial session delta offset, re-anchoring directly to the true media timeline.

---

### Roadblock 1.4: Main Thread Freezing on Track Switch
* **The Symptom:** When a new song started, the ESP32 screen froze, serial communications paused, and the web dashboard stopped responding for 2 to 4 seconds.
* **The Root Cause:** `syncedlyrics.search()` was executing synchronously on the main asyncio event loop, blocking all serial writes and HTTP requests while downloading LRC data over the internet.
* **The Solution:** Re-queried the fresh real-time playback position from Windows immediately after the lyrics download completed, re-aligning the clock before emitting the first lyric packet.

---

## 2. Display & Hardware Layout Roadblocks

### Roadblock 2.1: The Dual-Color Split OLED (Yellow/Blue Partition Gap)
* **The Symptom:** If lyrics were rendered full-screen on the 0.96" SSD1306 OLED, letters running across the upper third had their top halves rendered in yellow and their bottom halves in blue, with a visible black dead gap cutting through the words.
* **The Root Cause:** The 0.96" OLED display used is physically manufactured with two distinct phosphor zones:
  - Top 16 pixel rows: **Yellow LEDs**
  - Bottom 48 pixel rows: **Blue LEDs**
  - Physical dead-band between row 15 and row 16.
* **The Solution:** Architected a strict two-section UI:
  - **Upper Zone (0–15px Yellow)**: Dedicated to static track info, artist name, progress bar, and companion cat.
  - **Lower Zone (16–63px Blue)**: Dedicated entirely to the dynamic semantic storyboard, kinetic typography, and procedural doodles.

---

### Roadblock 2.2: Vertical Baseline Overlaps & Spacing Clutter
* **The Symptom:** In two-line lyric compositions (prefix + focal word, or focal word + suffix), the descenders of the upper line (e.g., *g, y, p*) crashed into the ascenders of the lower line, creating illegible visual collisions.
* **The Root Cause:** Different font styles (FreeSans vs. FreeSerifItalic vs. FreeMono) have varying baseline-to-ascender ratios. Fixed line heights that worked for sans-serif caused cursive script lines to overlap.
* **The Solution:** 
  - Standardized font coordinate conversion to explicit baseline anchors.
  - Locked 2-line baseline separation to a tight, cohesive **17px** (`prefixY = 17`, `focalY = 34`).
  - Added bounding-box clearing around doodles and underlines.

---

### Roadblock 2.3: Screen Boundary Truncation on 128x48 Canvas
* **The Symptom:** Multi-word lyrics like *"You in those little high waisted shorts, oh"* got cut off on the right edge (e.g. displaying as *"dash dash rash"*).
* **The Root Cause:** The semantic director split words naively around the hero word without checking whether the remaining prefix or suffix exceeded the 128px screen width.
* **The Solution:** Added **balanced length penalties** in `director.py`. If either the candidate prefix or suffix exceeds 20 characters, a severe score penalty (`-(len - 20) * 3.0`) forces the splitter to choose a more balanced focal word near the center, ensuring zero clipping on both lines.

---

## 3. Typography & Aesthetics Roadblocks

### Roadblock 3.1: Mid-Song Font Jumping & Visual Incoherence
* **The Symptom:** In early builds, every line of a song changed fonts randomly, sometimes mixing 3 different typefaces in a single sentence (e.g., Cursive prefix + Sans-serif focal word + Monospace suffix).
* **The Root Cause:** Random preset selection was running per line instead of per song.
* **The Solution:**
  - Implemented **Per-Song Locked Typographic Themes**: when a song begins, 1 unified font family is chosen and locked for the entire duration of the track.
  - Enforced single-family line harmony: prefix, focal word, and suffix all use harmonious sizes of the **exact same font family**.
  - Added a dashboard toggle for **Genre-Adaptive Typography** (Indie -> Cursive Script, Pop -> Modern Sans, R&B -> Editorial Serif, Rock -> Bold Sans, Hip-Hop -> Monospace).

---

### Roadblock 3.2: Hero Word Hijacking by Pronouns & Stop Words
* **The Symptom:** In emotional sentences, boring helper words like *"I"*, *"it"*, or *"that"* were being blown up into giant focal words while the real emotional words were relegated to tiny subtext.
* **The Root Cause:** Stop word filtering was too shallow, and short-word tie-breakers favored earlier words in the sentence.
* **The Solution:** Expanded stop words to include all pronouns and auxiliary verbs (`I'm`, `you're`, `they'll`, `would`, `should`). Added semantic keyword bonuses for words matching procedural doodles (hearts, stars, rain, fire).

---

## 4. Communication & Operating System Roadblocks

### Roadblock 4.1: Windows COM Port Exclusivity (`PermissionError 13: Access is denied`)
* **The Symptom:** Launching the Python backend frequently resulted in:
  ```text
  Could not connect to COM5. Retrying in 2 seconds... Error: could not open port 'COM5': PermissionError(13, 'Access is denied.')
  ```
* **The Root Cause:** On Windows, serial COM ports are strictly exclusive. If a background process, PlatformIO upload task, or another Python terminal is already holding `COM5`, no other program can open it.
* **The Solution:** Added process-cleanup logic before launching serial bridges and built a clean `listen_for_stop()` thread in `spotify_lyrics.py` to allow graceful shutdown without leaving orphan processes.

---

### Roadblock 4.2: Intrusive Terminal Windows on Boot
* **The Symptom:** Setting `Start_LyricCast.bat` to run on Windows startup caused a persistent black terminal window to remain open on the desktop.
* **The Root Cause:** Windows `.bat` files always spawn a `cmd.exe` console window by default.
* **The Solution:** Created a VBScript wrapper (`LyricCast_AutoStart.vbs`) that invokes `Start_LyricCast.bat` with window style `0` (hidden), allowing the server to boot silently in the background. Added a direct toggle in the Web Dashboard under the **System** tab.

---

## 5. Standalone Wireless / Battery Roadblocks (Roadmap)

### Roadblock 5.1: Spotify Web API Authentication Without a PC
* **The Challenge:** To run without a USB cable, the ESP32 must fetch playback data directly from `https://api.spotify.com/v1/me/player/currently-playing`. Spotify uses OAuth2, which requires user authorization and token refreshing.
* **The Solution Strategy:** Implement a one-time captive portal on the ESP32 (or a web dashboard setup button) where the user pastes their Spotify App `client_id`, `client_secret`, and `refresh_token`. The ESP32 will then request fresh 1-hour bearer tokens directly over HTTPS.

---

### Roadblock 5.2: ESP32 Wi-Fi Power Draw vs. Battery Life
* **The Challenge:** The ESP32's Wi-Fi radio draws **90mA-140mA** during active listening, draining a small battery in a few hours.
* **The Solution Strategy:**
  - Use a **1000mAh-1200mAh LiPo battery** with a TP4056 USB-C charge controller (~8-10 hours continuous play).
  - Implement ESP32 **Light Sleep** when playback is paused, dropping current draw to **< 1mA** for multi-day standby.
