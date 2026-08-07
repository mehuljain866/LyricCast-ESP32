import asyncio
import time
import serial
import serial.tools.list_ports
import syncedlyrics
from winsdk.windows.media.control import GlobalSystemMediaTransportControlsSessionManager as MediaManager

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
        if info and timeline:
            pos_ms = timeline.position.total_seconds() * 1000
            dur_ms = timeline.end_time.total_seconds() * 1000
            return info.title, info.artist, pos_ms, dur_ms
    return None, None, 0, 1000

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

async def main():
    print("Finding ESP32...")
    port = find_esp32_port()
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        print(f"Connected to ESP32 on {port}")
    except Exception as e:
        print(f"Could not connect to {port}. Error: {e}")
        return

    current_song = None
    lyrics = []
    last_sent_text = ""
    last_metadata_time = 0

    # OFFSET to fix the 1-line delay. 
    # Setting this to 1.0 means it looks ahead by 1 second, showing the lyric 1 second earlier.
    SYNC_OFFSET_SECONDS = 1.0 

    while True:
        try:
            title, artist, pos_ms, dur_ms = await get_media_info()
            
            if title and artist:
                song_key = f"{title} - {artist}"
                
                if song_key != current_song:
                    print(f"\nNew song detected: {song_key}")
                    current_song = song_key
                    lyrics = []
                    last_sent_text = ""
                    
                    ser.write(f"M|{title}|{artist}|{pos_ms:.0f}|{dur_ms:.0f}\n".encode())
                    ser.write(f"L|Fetching lyrics...|\n".encode())
                    
                    try:
                        lrc = syncedlyrics.search(f"{title} {artist}")
                        lyrics = parse_lrc(lrc)
                        if not lyrics:
                            ser.write(f"L|No synced lyrics found|\n".encode())
                            print("No synced lyrics found.")
                    except Exception as e:
                        print("Error fetching lyrics:", e)
                        ser.write(f"L|Error finding lyrics|\n".encode())

                now = time.time()
                if now - last_metadata_time > 2.0:
                    last_metadata_time = now
                    ser.write(f"M|{title}|{artist}|{pos_ms:.0f}|{dur_ms:.0f}\n".encode())

                if lyrics:
                    current_lyric = "..."
                    
                    position_s = (pos_ms / 1000.0) + SYNC_OFFSET_SECONDS
                    for i in range(len(lyrics)):
                        if position_s >= lyrics[i]['time']:
                            current_lyric = lyrics[i]['text']
                        else:
                            break
                    
                    if current_lyric != last_sent_text:
                        last_sent_text = current_lyric
                        print(f"[{position_s:.2f}s] {current_lyric}")
                        ser.write(f"L|{current_lyric}|\n".encode())
            else:
                if current_song != "NONE":
                    current_song = "NONE"
                    ser.write(f"M|Waiting for Spotify|...|0|1000\n".encode())
                    ser.write(f"L|Play a song|\n".encode())

        except Exception as e:
            print("Error:", e)
            
        await asyncio.sleep(0.1) 

if __name__ == "__main__":
    asyncio.run(main())
