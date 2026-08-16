import asyncio
import time
import serial
import serial.tools.list_ports
import syncedlyrics
import sys
import io
import os
import threading
import json
from http.server import SimpleHTTPRequestHandler, HTTPServer
from winsdk.windows.media.control import GlobalSystemMediaTransportControlsSessionManager as MediaManager

from director import LyricDirector
from sketchbook_engine import SketchbookEngine

SYNC_OFFSET_SECONDS = 0.20

director = LyricDirector()
sketchbook = SketchbookEngine()

CURRENT_SETTINGS = {
    'captionMode': 'sketchbook',
    'font': 'handwritten',
    'companion': 'cat',
    'logo': 'music',
    'uiScale': '100'
}

CURRENT_SCENE = {
    "type": "idle",
    "raw_text": "...",
    "prefix": "",
    "focal_word": "",
    "suffix": "",
    "metaphor": "idle",
    "doodle": "none",
    "tilt_angle": 0,
    "has_underline": False,
    "title": "Waiting for Spotify",
    "artist": ""
}

def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "CP210" in port.description or "CH340" in port.description or "UART" in port.description:
            return port.device
    return "COM5" 

async def get_media_info():
    sessions = await MediaManager.request_async()
    current_session = sessions.get_current_session()
    if current_session:
        info = await current_session.try_get_media_properties_async()
        timeline = current_session.get_timeline_properties()
        playback = current_session.get_playback_info()
        if info and timeline and playback:
            pos_ms = timeline.position.total_seconds() * 1000
            dur_ms = timeline.end_time.total_seconds() * 1000
            is_playing = (playback.playback_status == 4) 
            return info.title, info.artist, pos_ms, dur_ms, is_playing
    return None, None, 0, 1000, False

def parse_lrc(lrc_text):
    if not lrc_text: return []
    lines = lrc_text.strip().split('\n')
    lyrics = []
    for line in lines:
        if line.startswith('[') and ']' in line:
            timestamp_str = line[1:line.find(']')]
            text = line[line.find(']')+1:].strip()
            if text == "": text = "..." 
            try:
                if ':' in timestamp_str:
                    m, s = timestamp_str.split(':')
                    seconds = int(m) * 60 + float(s)
                    lyrics.append({'time': seconds, 'text': text})
            except:
                pass
    return sorted(lyrics, key=lambda x: x['time'])

class DashboardHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.path = '/index.html'
        elif self.path == '/api/current_scene':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(CURRENT_SCENE).encode('utf-8'))
            return
        return super().do_GET()

    def do_POST(self):
        global CURRENT_SETTINGS
        if self.path == '/api/settings':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            try:
                settings = json.loads(post_data.decode('utf-8'))
                CURRENT_SETTINGS.update(settings)
                print(f"\nSettings updated from Dashboard: {CURRENT_SETTINGS}")
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(b'{"status": "ok"}')
            except Exception as e:
                self.send_response(400)
                self.end_headers()
        else:
            self.send_response(404)
            self.end_headers()

def start_server():
    server = HTTPServer(('0.0.0.0', 8080), DashboardHandler)
    print("Dashboard Server started at http://0.0.0.0:8080")
    server.serve_forever()

def get_word_index(words, line_duration, time_into_line):
    if not words: return 0
    total_chars = sum(len(w) for w in words)
    if total_chars == 0: total_chars = 1
    
    estimated_time = (total_chars * 0.07) + (len(words) * 0.15)
    
    if line_duration > 10.0:
        anim_duration = min(8.0, max(estimated_time, 2.0))
    else:
        anim_duration = line_duration
    
    if time_into_line >= anim_duration:
        return len(words) - 1
        
    chars_so_far = 0
    for i, w in enumerate(words):
        chars_so_far += len(w)
        estimated_finish = (chars_so_far * 0.07) + ((i+1) * 0.15)
        ratio = estimated_finish / estimated_time
        finish_time = ratio * anim_duration
        if time_into_line < finish_time:
            return i
            
    return len(words) - 1

def listen_for_stop():
    while True:
        try:
            cmd = input().strip().lower()
            if cmd in ['stop', 'exit', 'quit', 'q']:
                print("\n[LyricCast] Stop command received. Shutting down cleanly...")
                os._exit(0)
        except (EOFError, KeyboardInterrupt):
            os._exit(0)
        except Exception:
            pass

async def main():
    global CURRENT_SCENE
    threading.Thread(target=start_server, daemon=True).start()
    threading.Thread(target=listen_for_stop, daemon=True).start()
    
    print("\n" + "=" * 52)
    print("           LYRICCAST BACKEND SERVER")
    print("  Web Dashboard: http://localhost:8080/")
    print("  Type 'stop' or 'quit' + Enter to exit cleanly.")
    print("=" * 52 + "\n")
    
    ser = None
    
    def connect_serial():
        nonlocal ser
        print("Finding ESP32...")
        port = find_esp32_port()
        try:
            ser = serial.Serial(port, 115200, timeout=1)
            print(f"Connected to ESP32 on {port}")
            return True
        except Exception as e:
            print(f"Could not connect to {port}. Retrying in 2 seconds... Error: {e}")
            return False

    current_song = ""
    lyrics = []
    last_sent_text = ""
    last_metadata_time = 0
    last_api_pos_ms = 0
    local_sync_time = time.time()
    last_sent_mode = ""
    last_sent_word_index = -1
    last_sent_font = ""
    last_sent_info_layout = ""

    while True:
        try:
            if ser is None:
                if not connect_serial():
                    await asyncio.sleep(2)
                    continue

            title, artist, api_pos_ms, dur_ms, is_playing = await get_media_info()
            
            if title and artist:
                song_key = f"{title} - {artist}"
                
                if api_pos_ms != last_api_pos_ms:
                    last_api_pos_ms = api_pos_ms
                    local_sync_time = time.time()
                
                if is_playing:
                    pos_ms = api_pos_ms + ((time.time() - local_sync_time) * 1000)
                else:
                    pos_ms = api_pos_ms
                    local_sync_time = time.time() 

                if song_key != current_song:
                    print(f"\nNew song detected: {song_key}")
                    current_song = song_key
                    director.reset_song(title)
                    lyrics = []
                    last_sent_text = ""
                    
                    ser.write(f"M|{title}|{artist}|{pos_ms:.0f}|{dur_ms:.0f}\n".encode('utf-8', 'replace'))
                    ser.write(f"L|Fetching lyrics...|\n".encode('utf-8', 'replace'))
                    
                    try:
                        lrc = syncedlyrics.search(f"{title} {artist}")
                        lyrics = parse_lrc(lrc)
                        if not lyrics:
                            ser.write(f"L|No synced lyrics found|\n".encode('utf-8', 'replace'))
                    except Exception as e:
                        ser.write(f"L|Error finding lyrics|\n".encode('utf-8', 'replace'))

                now = time.time()
                if now - last_metadata_time > 2.0:
                    last_metadata_time = now
                    ser.write(f"M|{title}|{artist}|{pos_ms:.0f}|{dur_ms:.0f}\n".encode('utf-8', 'replace'))

                # Send song info layout updates if changed (2-Line static vs Marquee)
                current_info_layout = CURRENT_SETTINGS.get('songInfoLayout', 'twoline')
                if current_info_layout != last_sent_info_layout:
                    if current_info_layout == 'twoline':
                        ser.write(f"I|TWOLINE\n".encode('utf-8', 'replace'))
                    else:
                        ser.write(f"I|MARQUEE\n".encode('utf-8', 'replace'))
                    last_sent_info_layout = current_info_layout

                # Send font updates if changed
                current_font = CURRENT_SETTINGS.get('font', 'handwritten')
                if current_font != last_sent_font:
                    ser.write(f"F|{current_font.upper()}\n".encode('utf-8', 'replace'))
                    last_sent_font = current_font
                    last_sent_text = "" 

                # Send mode updates if changed
                current_mode = CURRENT_SETTINGS.get('captionMode', 'sketchbook')
                if current_mode != last_sent_mode:
                    if current_mode == 'sketchbook':
                        ser.write(f"S|SKETCHBOOK\n".encode('utf-8', 'replace'))
                    elif current_mode == 'giant':
                        ser.write(f"S|GIANT\n".encode('utf-8', 'replace'))
                    elif current_mode == 'kinetic2':
                        ser.write(f"S|KINETIC2\n".encode('utf-8', 'replace'))
                    elif current_mode == 'kinetic':
                        ser.write(f"S|KINETIC\n".encode('utf-8', 'replace'))
                    elif current_mode == 'sliding':
                        ser.write(f"S|SLIDING\n".encode('utf-8', 'replace'))
                    else:
                        ser.write(f"S|NORMAL\n".encode('utf-8', 'replace'))
                    last_sent_mode = current_mode
                    last_sent_text = ""
                    last_sent_word_index = -1

                if lyrics:
                    current_lyric = "..."
                    current_lyric_time = 0
                    next_lyric_time = dur_ms / 1000.0
                    
                    position_s = (pos_ms / 1000.0) + SYNC_OFFSET_SECONDS
                    for i in range(len(lyrics)):
                        if position_s >= lyrics[i]['time']:
                            current_lyric = lyrics[i]['text']
                            current_lyric = current_lyric.replace("’", "'").replace("‘", "'").replace("”", '"').replace("“", '"')
                            current_lyric_time = lyrics[i]['time']
                            if i + 1 < len(lyrics):
                                next_lyric_time = lyrics[i+1]['time']
                        else:
                            break
                            
                    if current_lyric == "...":
                        logo_set = CURRENT_SETTINGS.get('logo', 'music')
                        if logo_set == 'music': current_lyric = "♫"
                        elif logo_set == 'heart': current_lyric = "♥"
                        elif logo_set == 'star': current_lyric = "★"
                        elif logo_set == 'smile': current_lyric = "☺"
                        
                    # =====================================
                    # SKETCHBOOK / SEMANTIC DIRECTOR MODE
                    # =====================================
                    if current_mode == 'sketchbook':
                        if current_lyric != last_sent_text:
                            last_sent_text = current_lyric
                            line_duration = max(1.5, next_lyric_time - current_lyric_time)
                            scene = director.analyze_line(current_lyric, duration=line_duration, song_position=position_s)
                            scene['title'] = title
                            scene['artist'] = artist
                            CURRENT_SCENE = scene
                            
                            # Format and send ESP32 Sketchbook Packet
                            esp_packet = sketchbook.format_esp32_packet(scene)
                            ser.write(esp_packet.encode('utf-8', 'replace'))
                            print(f"[{position_s:.2f}s] [🎬 {scene['metaphor']}|🎨 {scene['doodle']}] {current_lyric}")

                    elif current_mode == 'giant' or current_mode == 'kinetic' or current_mode == 'kinetic2':
                        words = current_lyric.split()
                        if not words: words = [current_lyric]
                        
                        line_duration = next_lyric_time - current_lyric_time
                        time_into_line = position_s - current_lyric_time
                        word_index = get_word_index(words, line_duration, time_into_line)
                        
                        word_to_send = words[word_index]
                        if word_to_send != last_sent_text:
                            last_sent_text = word_to_send
                            ser.write(f"L|{word_to_send}|\n".encode('utf-8', 'replace'))
                            
                    elif current_mode == 'sliding':
                        if current_lyric != last_sent_text:
                            last_sent_text = current_lyric
                            print(f"[{position_s:.2f}s] {current_lyric}")
                            ser.write(f"L|{current_lyric}|\n".encode('utf-8', 'replace'))
                            
                        words = current_lyric.split()
                        if words:
                            line_duration = next_lyric_time - current_lyric_time
                            time_into_line = position_s - current_lyric_time
                            word_index = get_word_index(words, line_duration, time_into_line)
                            
                            if word_index != last_sent_word_index:
                                last_sent_word_index = word_index
                                ser.write(f"W|{word_index}\n".encode('utf-8', 'replace'))
                    else:
                        # Normal Mode
                        if current_lyric != last_sent_text:
                            last_sent_text = current_lyric
                            print(f"[{position_s:.2f}s] {current_lyric}")
                            ser.write(f"L|{current_lyric}|\n".encode('utf-8', 'replace'))
            else:
                if current_song != "NONE":
                    current_song = "NONE"
                    CURRENT_SCENE = {
                        "type": "idle",
                        "raw_text": "Waiting for Spotify",
                        "prefix": "",
                        "focal_word": "Spotify",
                        "suffix": "",
                        "metaphor": "idle",
                        "doodle": "none",
                        "tilt_angle": 0,
                        "has_underline": False,
                        "title": "Waiting for Spotify",
                        "artist": ""
                    }
                    ser.write(f"M|Waiting for Spotify|...|0|1000\n".encode('utf-8', 'replace'))
                    ser.write(f"L|Play a song|\n".encode('utf-8', 'replace'))

        except serial.SerialException as e:
            print(f"Serial connection lost: {e}")
            ser = None
        except Exception as e:
            pass

        await asyncio.sleep(0.03)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nExiting...")
