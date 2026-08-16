# 🎵 LyricCast ESP32

A state-of-the-art, time-synchronized animated lyric visualizer for Spotify and Windows Media using an **ESP32** and a **0.96" Dual-Color OLED Display (SSD1306)**.

LyricCast hooks into Windows Media Session in real-time, fetches millisecond-accurate synchronized lyrics from LRC databases (LRCLib, NetEase, Musixmatch), and streams custom packets over Serial to the ESP32 firmware. The ESP32 handles dynamic typography, word bounding box math, smooth physics, multi-particle parallax animations, and animated pixel companions at ~30 FPS.

---

## ✨ Features

- **5 Dynamic Display Modes**:
  1. **Normal (Sentence by Sentence)**: Smooth vertical slide transitions between full lyric lines with automatic font scaling.
  2. **Sliding Rectangle Box (Karaoke)**: An inverse rectangular highlight box that slides word-by-word across sentences in real-time.
  3. **Giant Word-for-Word**: Massive dynamic typography displaying individual words in large fonts as they are sung.
  4. **Kinetic V1 (Cotodama Style)**: Words spawn with random typography and dynamic slide/pop animations.
  5. **Kinetic V2 (Cotodama Parallax ✦)**: Multi-word particle physics engine rendering up to 6 simultaneous floating words in a diagonal staircase flow with independent velocities.
- **🐱 Animated Pixel Companions (GIF Style)**:
  - 🐱 **Lo-Fi Pixel Cat**: Wearing headphones, tapping paws and wagging tail to the beat.
  - 🎧 **Vibe Dude (Dancer)**: Head-bobbing pixel character grooving in real time.
  - 💿 **Spinning Vinyl Record**: Rotating turntable with dancing sound ripples.
  - 👻 **Bouncing Pixel Ghost**: Floating cute spirit swaying to the rhythm.
  - *Dances on the left while lyrics flow on the right; takes center stage during song pauses!*
- **✍️ Handwritten & Aesthetic Script Typography**:
  - Flowing cursive italic fonts (`FreeSerifItalic` from 9pt up to 24pt) inspired by viral TikTok & Instagram aesthetic music setups.
  - Toggle between **Handwritten Script ✨**, **Modern Clean (FreeSans)**, and **Retro Monospace (FreeMono)**.
- **🎛️ Neumorphic Web Dashboard**: Control caption modes, mascots, font styles, and logo settings in real-time from any browser on your network (`http://localhost:8080/`).
- **🚀 One-Click Launcher (`Start_LyricCast.bat`)**: Double-click to start the background engine, automatically connect to the ESP32 COM port, and launch the web server.
- **🛑 Interactive Terminal Control**: Type `stop`, `exit`, or `quit` anytime in the terminal to shut down cleanly.

---

## 🧠 Engineering Secrets & Architectural "Trickery"

### 1. The 180° Hardware Inversion Trick
Most standard 0.96" Dual-Color OLED displays (SSD1306) are manufactured with a physical **Yellow band on top (Rows 0–15)** and a **Blue band on the bottom (Rows 16–63)**. 
- Having the song title on top and lyrics cramped below felt unbalanced.
- **The Solution:** By applying `display.setRotation(2)` in the firmware, we invert the display coordinate matrix 180 degrees.
- **The Result:** The physical Yellow band moves to the **bottom** of the screen (`Y=48..63`), acting as a retro cassette-style status bar with a smooth scrolling song marquee and a visual progress bar. This frees up the entire expansive 48-pixel Blue area (`Y=0..47`) on top for high-contrast animated lyrics and animated pixel mascots!

### 2. The Hybrid Multi-Language Rendering Engine
Standard embedded libraries like `Adafruit_GFX` only support 7-bit ASCII (English characters 0x20 to 0x7E). Loading full multi-megabyte CJK Unicode fonts would instantly exceed the ESP32's 320KB RAM.
- **The "Trickery":** We engineered a hybrid graphics bridge combining `Adafruit_GFX` with `U8g2_for_Adafruit_GFX`.
- **Real-Time UTF-8 Stream Inspection:** When a lyric line arrives over Serial, the firmware scans the byte array in real time for international multi-byte sequences ($\ge \text{0x80}$).
- **Dynamic Font Routing:** 
  - If English/Latin text is detected, it utilizes custom high-speed `FreeSans` or `FreeSerifItalic` fonts with pixel-accurate bounding box measurements.
  - If Japanese (Hiragana/Katakana/Kanji), Russian (Cyrillic), or international accented characters/emojis are detected, it dynamically hot-swaps the rendering engine to the U8g2 Unifont engine without dropping a frame.
- **Smart Quote Scrubbing:** The Python bridge sanitizes non-standard Unicode typographical quotes (`’`, `“`, `”`) into clean ASCII on the fly, preventing unnecessary fallback on English songs while preserving genuine international lyrics.

---

## 🛠️ Hardware Requirements & Wiring

- **Microcontroller**: ESP32 Dev Module (e.g. NodeMCU-32S / ESP32-WROOM-32)
- **Display**: 0.96" I2C Dual-Color OLED Display (128x64, SSD1306, Yellow/Blue)
- **Cable**: Micro-USB or USB-C data cable

### Pinout Connection
| ESP32 Pin | OLED Pin | Notes |
|:---------:|:--------:|:------|
| **3V3**   | **VCC**  | 3.3V Power |
| **GND**   | **GND**  | Ground |
| **GPIO 5** (or 21) | **SDA**  | I2C Data (Auto-detected) |
| **GPIO 4** (or 22) | **SCL**  | I2C Clock (Auto-detected) |

---

## 🚀 Quick Start Guide

### 1. Flash the ESP32 Firmware
1. Open the `esp32/` directory in VS Code with the **PlatformIO IDE** extension (or use `platformio run -t upload` via CLI).
2. Connect your ESP32 to your PC via USB.
3. Build and upload the firmware.

### 2. Run the Python Engine

#### Option A: One-Click Launcher (Recommended)
Double-click **`Start_LyricCast.bat`** in the repository root (or on your Desktop).

#### Option B: Manual CLI
```bash
cd python
pip install -r requirements.txt
python -u spotify_lyrics.py
```

### 3. Open the Web Dashboard
Open your browser and navigate to:
```
http://localhost:8080/
```
From the dashboard, choose your favorite caption mode, switch mascots, select handwritten script typography, and enjoy synchronized lyrics in real time!

---

## 🛑 Stopping the Server
In the terminal window, simply type:
```
stop
```
and press **Enter**. The serial connection and web server will safely shut down.

---

## 📦 Dependencies

- **ESP32 Firmware**:
  - `Adafruit SSD1306`
  - `Adafruit GFX Library`
  - `U8g2_for_Adafruit_GFX`
  - `Wire`
- **Python Host**:
  - `winsdk` (Windows Media Control integration)
  - `syncedlyrics` (LRC timestamp retrieval)
  - `pyserial` (Serial communication bridge)

---

## 📄 License
MIT License. Created by Mehul.
