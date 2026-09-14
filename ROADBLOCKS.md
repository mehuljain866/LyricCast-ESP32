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

---

## 6. Expressive++ Unicode Emoji & Display Roadblocks

### Roadblock 6.1: Repetitive Hand-Drawn Doodles vs. Full Unicode Catalog (3,600+ Emojis)
* **The Symptom:** Hand-drawn vector doodles (50 presets) became visually repetitive across long listening sessions. Expanding to full Unicode emojis directly on the ESP32 faced a massive storage barrier: storing 3,600+ bitmap glyphs in ESP32 Flash memory would consume megabytes of firmware space and make adding new emojis impossible without reflashing the microcontroller.
* **The Root Cause:** Embedded microcontrollers lack the font rendering engines (FreeType/HarfBuzz) and vector font file storage (`seguiemj.ttf` is > 1.5MB) required for native emoji rendering.
* **The Solution:**
  - Implemented **Dynamic Python-Side 1-Bit Monochrome Rasterization**: whenever an emoji is detected via the semantic dictionary (1,200+ keywords), Pillow dynamically rasterizes it at 16×16 pixels into a 32-byte 1-bit monochrome bitmap.
  - The 32 bytes are encoded into a compact 64-character hex string (`BMP:<hex>`) and streamed in the existing serial scene packet (`K|...`).
  - Transmission takes only **5.5 ms** over 115200 baud serial, and the ESP32 renders it instantly in **< 0.1 ms** via `display.drawBitmap()`, allowing 100% access to all Unicode emojis with 0 KB added Flash bloat.

---

### Roadblock 6.2: 1-Bit Monochrome OLED Gradients & Brightness Limitation
* **The Symptom:** The SSD1306 OLED display is binary monochrome (pixels are strictly 100% on or 100% off with no per-pixel hardware brightness control), making smooth visual gradients and atmospheric depth seemingly impossible.
* **The Root Cause:** Hardware limitation of standard 0.96" I2C OLED displays.
* **The Solution:**
  - Implemented **Spatial 4×4 Bayer Ordered Dithering**: by mapping pixel activation probability through a 4×4 threshold matrix, the human eye perceives soft spatial grayscale gradients.
  - Added an animated `PARTICLE_GRADIENT` wave mode that produces shifting, ethereal aurora mist across the display canvas.

---

### Roadblock 6.3: Dotted Line Visual Artifact on Physical OLED Displays
* **The Symptom:** An ambient 3-row dithering floor gradient (`y=45..47`) intended to create a soft floor glow appeared on the physical 0.96" OLED screen as an unnatural, distracting dotted line separating the blue and yellow zones.
* **The Root Cause:** At standard viewing distances, 128×64 pixels on a small 0.96" diagonal display are large enough that isolated single-pixel stipples in ordered dithering resolve as discrete dots rather than a seamless blended gradient.
* **The Solution:** Completely eliminated the static boundary dithering line (`drawExpressiveHorizonGradient()`) from firmware and simulator, keeping the canvas boundary crisp, clean, and distraction-free.

---

### Roadblock 6.4: Out-of-Context Emoji Triggering & Static Placement
* **The Symptom:** Emojis were triggered inappropriately (e.g., words like *"fast"* triggering a fast-food hamburger 🍔, *"play"* triggering a video game controller 🎮, *"class"* triggering a basketball 🏀), lyrics often lacked emojis when lines didn't contain concrete nouns, and rendered emojis were completely static and placed at a rigid coordinate that clipped on long words.
* **The Root Cause:** 
  1. Primitive dictionary used naive single-word substring matching with no lyrical context awareness.
  2. No support for multi-word idioms (e.g., *"shut up and dance"*, *"broken heart"*, *"on fire"*).
  3. No kinetic motion choreography; bitmaps were rendered as static stamps.
* **The Solution:**
  - Built a 5-layer **Context-Aware Semantic Emoji Engine (`emoji_engine.py`)**:
    - **Layer 1: Multi-Word Lyrical Idioms** (hundreds of song expressions matched with top priority).
    - **Layer 2: Hero Focal Word Semantic Direct Match** (reinforcing the director's focal word).
    - **Layer 3: Keyword Scanning with Disambiguation Blacklists** (preventing food/gaming/sports collisions in lyrical contexts).
    - **Layer 4: Sentiment & Valence Fallback** (Joy $\to$ ✨, Sadness $\to$ 🌧️, Romance $\to$ 💖, Hype $\to$ ⚡).
    - **Layer 5: Melodic Default** ($\to$ 🎵).
  - Built a **5-Way Kinetic Emoji Motion Choreographer**:
    - `BOUNCE` (0): Elastic pop entrance with energetic harmonic bobbing.
    - `FLOAT` (1): Dreamy sinusoidal floating drift for mellow/sad/space lyrics.
    - `PULSE` (2): Double-throb heartbeat for romantic and emotional lyrics.
    - `WIGGLE` (3): High-frequency rapid tremor for fire, rock, and hype lyrics.
    - `SPARKLE` (4): Orbital micro-sparkle stars revolving around the emoji.
  - Implemented **Adaptive On-Screen Positioning**: dynamically positions emojis beside the focal word, floating above wide focal words, or centered, guaranteeing emojis are **never dropped or clipped**.

---

### Roadblock 6.5: Lyric Text Truncation, 1-Bit Monochrome Legibility & Generic Emoji Bobbing
* **The Symptom:** 
  1. Wide lyric lines exceeded the 128px screen width and were getting cut off or forced into tiny, squashed fonts.
  2. Color-dependent or complex emojis (like 🛑 red stop sign, 🤷 shrug, 💑 couple kissing) turned into illegible, noisy blobs when thresholded to 16×16 1-bit monochrome.
  3. Repeated choruses repeatedly rendered the exact same emoji over and over.
  4. Emotional emojis (such as a beating heart ❤️) merely bobbed up and down generically rather than actually beating or throbbing.
  5. Only 1 emoji could be displayed per line, stuck in a rigid column.
* **The Root Cause:**
  1. 128×64 physical OLED screen dimensions are constrained; without a panning camera, long lines cannot fit without font degradation.
  2. Certain Unicode emojis depend strictly on color or intricate detail that fails in 1-bit monochrome 16×16 bitmaps.
  3. Lack of repetition tracking across chorus refrains.
  4. Firmware lacked dynamic pixel scaling for genuine anatomical `lub-dub` pulse animations.
* **The Solution:**
  1. **Virtual Camera Pan Engine**: Firmware calculates total scene content width $W_{\text{total}}$. When $W_{\text{total}} > 120\text{px}$, the virtual camera smoothly pans horizontally across the line from left to right as playback progresses ($0.15 \to 0.85$ progress) using `easeInOutQuad`. 100% of lyric words remain visible with zero font scaling or truncation.
  2. **1-Bit Curated Monochrome Whitelist**: Replaced all low-contrast/color-dependent emojis with crisp silhouettes (e.g. 🛑 $\to$ ✋, 🤷 $\to$ ❓, 💑 $\to$ ❤️). Handcrafted a pixel-perfect 16×16 solid beating heart bitmap.
  3. **Repetition Cycling**: Added song refrain repetition tracker that alternates complementary emoji pairs across repeated choruses (e.g. Chorus 1: 🪩+💃, Chorus 2: 💃+⚡, Chorus 3: 🪩+✨).
  4. **Dynamic Scaler & Authentic Kinetic Motions**: Implemented real-time dynamic scaling in firmware to render genuine double-beat `lub-dub` heartbeat throbs ($0.85 \leftrightarrow 1.25\times$), flame chaotic jitter, and rhythmic dance sways.
  5. **Multi-Emoji Streaming**: Extended serial protocol (`BMP:<hex1>,<hex2>`) to stream up to 2 emojis per line revealed dynamically across the panning camera.

---

### Roadblock 6.6: Concrete Lexicon Gaps ("Trees" Showing Music Note) & 1D Rail Confinement
* **The Symptom:**
  1. Lyrics mentioning concrete nouns like *"trees"* displayed a generic music note (`🎵`), and world/earth/mountain/city words failed to trigger context-accurate emojis.
  2. The virtual camera was confined to a rigid 1-dimensional horizontal line ($X$-axis rail), making short lines feel stationary and lacking cinematic, organic dynamism.
  3. Paired emojis were placed along a flat horizontal line, and eye emojis (`👀`, `👁️`) bobbed up and down rather than glancing and scanning side-to-side.
* **The Root Cause:**
  1. Semantic dictionary lacked nature/flora, geographical, and landscape mappings, while the default fallback palette contained `🎵` and `🎶`, causing non-musical lyrics without a direct match to display musical icons.
  2. Camera transform in firmware only modulated $camX$, keeping $camY = 0$.
  3. Motion choreography lacked a horizontal scan vector for gaze-based emojis.
* **The Solution:**
  1. **Concrete Lexicon Expansion & Musical Fallback Purge**:
     - Added comprehensive coverage for trees (`🌲`, `🌳`, `🌴`, `🌱`), Earth/world (`🌍`), mountains (`🏔️`), city/skyscrapers (`🏙️`), maps/borders (`🗺️`), flowers (`🌺`), leaves (`🍁`).
     - Completely removed `🎵` and `🎶` from `FALLBACK_PALETTE`, reserving musical note emojis exclusively for genuine musical terms (sing, song, guitar, piano, melody).
  2. **2D Cinematic Open Canvas Engine**:
     - Implemented multi-axis camera motion: non-linear crane arcs ($camY = \sin(\text{progress} \cdot \pi) \cdot 6.0f$), vertical ascents for flying metaphors, and descents for falling metaphors.
     - Dynamic diagonal emoji placement in 2D mode: Emoji 1 sits top-left ($Y = 8$), Emoji 2 sits bottom-right ($Y = 28$), framing lyrics across an open 2D canvas.
  3. **Watchful Eye Scan Motion (`MOTION_EYES_SCAN = 5`)**:
     - Added horizontal gaze-scanning jitter (`sin(now * 0.005f) * 3.5f`) mapped specifically to `👀` and `👁️`.
  4. **Dashboard Camera Dimension Toggle**:
     - Added a segmented radio toggle to the Web Dashboard allowing instant switching between `1D: X-Axis (Horizontal Sweep)` and `2D: XY-Axis (Cinematic Open Canvas)`, with full auto-saving and real-time syncing to ESP32 firmware via bit 12 (`0x1000`) of `fx_flags`.

