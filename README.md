# Animated Spotify OLED Sync

A perfectly synchronized, animated lyric display for Spotify using an ESP32 and a 0.96" Dual-Color OLED display.

This project uses the Windows Media API to fetch whatever you are currently listening to, fetches perfectly time-synced lyrics from LRCLib and Musixmatch, and streams them over Serial to an ESP32. The ESP32 handles all the beautiful typography, text wrapping, marquee scrolling, and smooth sliding animations.

## Hardware Requirements
* **ESP32** (e.g., NodeMCU-32S, ESP32-WROOM)
* **0.96" I2C OLED Display** (128x64, SSD1306, Dual-Color Yellow/Blue)
* Micro-USB or USB-C cable for data and power

## Wiring
| ESP32 Pin | OLED Pin |
|-----------|----------|
| 3V3       | VCC      |
| GND       | GND      |
| D21       | SDA      |
| D22       | SCL      |

## Features
* **Zero Delay Sync:** A precise 1-second lookahead offset completely eliminates serial communication delays, offering perfectly synced lyrics.
* **Smart Text Wrapping:** A custom text-wrapping algorithm measures pixel width and perfectly vertically centers lyrics, dynamically shrinking font sizes for long rap verses.
* **Dual-Color Optimized UI:** 
  * The top Yellow band elegantly displays a fast-scrolling marquee of the Song Title & Artist.
  * A sleek progress bar runs along the bottom of the Yellow band.
  * The entire Blue area is dedicated to large, animated, sliding lyrics.

## Setup & Installation

### 1. Flash the ESP32
1. Install [PlatformIO](https://platformio.org/).
2. Open the `esp32` directory in VS Code / PlatformIO.
3. Build and upload the firmware to your ESP32.
   * *Note: If the upload fails to connect, hold down the `BOOT` button on your ESP32 during the "Connecting..." phase.*

### 2. Run the Python Bridge
The Python script acts as the master clock and fetches data from Spotify (via Windows Media) and the internet.

1. Install Python 3.10+.
2. Navigate to the `python` directory.
3. Install the required dependencies:
   ```bash
   pip install -r requirements.txt
   ```
4. Run the script:
   ```bash
   python spotify_lyrics.py
   ```
   *(Ensure your ESP32 is plugged in. The script will automatically detect the COM port).*

## How it Works
1. `winsdk` silently hooks into the Windows 10/11 Media Session API to detect the currently playing song on Spotify (or any other supported media player).
2. `syncedlyrics` reaches out to LRC databases (LRCLib, Musixmatch) to download the `.lrc` synced timestamp file for the track.
3. The script continuously loops, comparing the current track progress to the LRC timestamps, and sends proprietary `M|...` (Metadata) and `L|...` (Lyric) packets over Serial to the ESP32.
4. The ESP32 parses this data and renders the beautiful UI using the `Adafruit_GFX` library and custom `FreeSans` fonts.
