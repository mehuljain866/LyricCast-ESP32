# 🎵 LyricCast ESP32

A state-of-the-art, time-synchronized animated lyric visualizer for Spotify and Windows Media using an **ESP32** and a **0.96" Dual-Color OLED Display (SSD1306)**.

LyricCast hooks into Windows Media Session in real-time, fetches millisecond-accurate synchronized lyrics from LRC databases (LRCLib, NetEase, Musixmatch), and streams custom packets over Serial to the ESP32 firmware. The ESP32 handles dynamic typography, word bounding box math, smooth physics, and multi-particle parallax animations at ~30 FPS.

---

## ✨ Features

- **5 Dynamic Display Modes**:
  1. **Normal (Sentence by Sentence)**: Smooth vertical slide transitions between full lyric lines with automatic font scaling.
  2. **Sliding Rectangle Box (Karaoke)**: An inverse rectangular highlight box that slides word-by-word across sentences in real-time.
  3. **Giant Word-for-Word**: Massive dynamic typography displaying individual words in large fonts as they are sung.
  4. **Kinetic V1 (Cotodama Style)**: Words spawn with random typography and dynamic slide/pop animations.
  5. **Kinetic V2 (Cotodama Parallax ✦)**: Multi-word particle physics engine rendering up to 6 simultaneous floating words in a diagonal staircase flow with independent velocities.
- **Neumorphic Web Dashboard**: Control caption modes, logo styles, and settings in real-time from any browser on your network (`http://localhost:8080/`).
- **One-Click Launcher (`Start_LyricCast.bat`)**: Double-click to start the background engine, automatically connect to the ESP32 COM port, and launch the web server.
- **Interactive Terminal Control**: Type `stop`, `exit`, or `quit` anytime in the terminal to shut down cleanly.
- **Dual-Color OLED Layout**:
  - **Yellow Zone (Physical Bottom)**: Smooth scrolling marquee of Song Title & Artist + visual track progress bar.
  - **Blue Zone (Physical Top)**: Dedicated area for animated lyric renderings.
- **International Language Support**: Automatically switches to UTF-8 font renderers for Japanese, Cyrillic, accented characters, and emojis.
- **Smart Offset & Anti-Flood Timing**: Character-proportional pacing algorithms and serial caching prevent packet drops, lag, and jitter.

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
From the dashboard, choose your favorite caption mode, switch logos, and enjoy synchronized lyrics in real time!

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
