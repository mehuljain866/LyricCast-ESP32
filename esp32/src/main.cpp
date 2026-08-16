#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>

// Modern Clean Fonts
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// Handwritten / Aesthetic Script Fonts (Insta & TikTok Cursive Style)
#include <Fonts/FreeSerifItalic24pt7b.h>
#include <Fonts/FreeSerifItalic18pt7b.h>
#include <Fonts/FreeSerifItalic12pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

// Retro Monospace Fonts
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_ADDR    0x3C
#define OLED_RESET   -1

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
U8G2_FOR_ADAFRUIT_GFX u8g2_gfx;

String songTitle = "Waiting for Spotify";
String songArtist = "";
float progressMs = 0;
float durationMs = 1000;
unsigned long lastUpdateMs = 0;

String currentLyric = "";
String oldLyric = "";

int animOffset = 0;
bool animating = false;
unsigned long lastFrameMs = 0;

float marqueeX = 128;
unsigned long lastMarqueeMs = 0;

// ==========================================
// FONT STYLES & FAMILIES
// ==========================================
enum FontStyle {
  FONT_HANDWRITTEN = 0,
  FONT_SANS = 1,
  FONT_MONO = 2
};
FontStyle currentFontStyle = FONT_HANDWRITTEN;

const GFXfont* sansFonts[] = {&FreeSans24pt7b, &FreeSans18pt7b, &FreeSans12pt7b, &FreeSans9pt7b, NULL};
int sansHeights[] = {36, 26, 18, 14, 8};

const GFXfont* scriptFonts[] = {&FreeSerifItalic24pt7b, &FreeSerifItalic18pt7b, &FreeSerifItalic12pt7b, &FreeSerifItalic9pt7b, NULL};
int scriptHeights[] = {36, 26, 18, 14, 8};

const GFXfont* monoFonts[] = {&FreeMonoBold18pt7b, &FreeMonoBold18pt7b, &FreeMonoBold12pt7b, &FreeMonoBold9pt7b, NULL};
int monoHeights[] = {30, 26, 18, 14, 8};

const GFXfont** getActiveFonts() {
  if (currentFontStyle == FONT_HANDWRITTEN) return scriptFonts;
  if (currentFontStyle == FONT_MONO) return monoFonts;
  return sansFonts;
}

int* getActiveHeights() {
  if (currentFontStyle == FONT_HANDWRITTEN) return scriptHeights;
  if (currentFontStyle == FONT_MONO) return monoHeights;
  return sansHeights;
}

// ==========================================
// ANIMATED PIXEL COMPANION SPRITES (24x24)
// ==========================================
enum CompanionType {
  COMPANION_NONE = 0,
  COMPANION_CAT = 1,
  COMPANION_VIBE = 2,
  COMPANION_VINYL = 3,
  COMPANION_GHOST = 4
};
CompanionType currentCompanion = COMPANION_CAT;
int companionFrame = 0;
unsigned long lastCompanionMs = 0;

// 1. Lo-Fi Headphone Cat (24x24, 4 Frames)
const unsigned char PROGMEM cat_frame0[] = {
  0x00,0x00,0x00, 0x00,0x00,0x00, 0x08,0x20,0x10, 0x1c,0x20,0x38,
  0x3e,0x70,0x7c, 0x3f,0xf8,0xfc, 0x7f,0xfc,0xfe, 0x7f,0xfe,0xfe,
  0x7c,0xee,0x3e, 0x7c,0xee,0x3e, 0x7f,0xfe,0xfe, 0x3f,0x1c,0xfc,
  0x3e,0x38,0x7c, 0x1f,0xf8,0xf8, 0x0f,0xf0,0xf0, 0x0f,0xf1,0xf0,
  0x07,0xe3,0xe0, 0x07,0xe7,0xe0, 0x03,0xc7,0xc0, 0x03,0xc7,0xc0,
  0x03,0xcf,0x80, 0x07,0xfe,0x00, 0x03,0xfc,0x00, 0x00,0x00,0x00
};
const unsigned char PROGMEM cat_frame1[] = {
  0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x08,0x20,0x10,
  0x1c,0x70,0x38, 0x3e,0xf8,0x7c, 0x3f,0xfc,0xfc, 0x7f,0xfe,0xfe,
  0x7c,0xee,0x3e, 0x7c,0xee,0x3e, 0x7f,0xfe,0xfe, 0x3f,0x1c,0xfc,
  0x3e,0x38,0x7c, 0x1f,0xf8,0xf8, 0x0f,0xf0,0xf0, 0x0f,0xf3,0xf0,
  0x07,0xe7,0xe0, 0x07,0xef,0xe0, 0x03,0xcf,0xc0, 0x03,0xcf,0xc0,
  0x07,0xde,0x80, 0x0f,0xfc,0x00, 0x07,0xf8,0x00, 0x00,0x00,0x00
};
const unsigned char PROGMEM cat_frame2[] = {
  0x00,0x00,0x00, 0x00,0x00,0x00, 0x04,0x40,0x20, 0x0e,0x40,0x70,
  0x1f,0xe0,0xf8, 0x3f,0xf1,0xfc, 0x7f,0xf9,0xfe, 0x7f,0xff,0xfe,
  0x7c,0xee,0x3e, 0x7c,0xee,0x3e, 0x7f,0xff,0xfe, 0x3f,0x39,0xfc,
  0x3e,0x73,0x7c, 0x1f,0xf7,0xf8, 0x0f,0xf3,0xf0, 0x0f,0xf1,0xf0,
  0x07,0xe0,0xe0, 0x07,0xe0,0xe0, 0x03,0xc1,0xc0, 0x03,0xc3,0xc0,
  0x03,0xc7,0x80, 0x07,0xfe,0x00, 0x03,0xfc,0x00, 0x00,0x00,0x00
};
const unsigned char PROGMEM cat_frame3[] = {
  0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x04,0x40,0x20,
  0x0e,0xe0,0x70, 0x1f,0xf1,0xf8, 0x3f,0xf9,0xfc, 0x7f,0xff,0xfe,
  0x7c,0xee,0x3e, 0x7c,0xee,0x3e, 0x7f,0xff,0xfe, 0x3f,0x39,0xfc,
  0x3e,0x73,0x7c, 0x1f,0xf7,0xf8, 0x0f,0xf3,0xf0, 0x0f,0xf1,0xf0,
  0x07,0xe0,0xe0, 0x07,0xe1,0xe0, 0x03,0xc3,0xc0, 0x03,0xc7,0xc0,
  0x07,0xee,0x80, 0x0f,0xfc,0x00, 0x07,0xf8,0x00, 0x00,0x00,0x00
};

// 2. Vibe Dancer / Head-Bobbing Dude (24x24, 4 Frames)
const unsigned char PROGMEM vibe_frame0[] = {
  0x00,0x38,0x00, 0x00,0x7c,0x00, 0x00,0xfe,0x00, 0x01,0xaa,0x80,
  0x01,0xaa,0x80, 0x01,0xba,0x80, 0x00,0xfe,0x00, 0x00,0x7c,0x00,
  0x00,0x38,0x00, 0x03,0xfe,0xc0, 0x07,0xff,0xe0, 0x0f,0xff,0xf0,
  0x0e,0xfe,0x70, 0x1c,0xfe,0x38, 0x18,0xfe,0x18, 0x00,0x7c,0x00,
  0x00,0x7c,0x00, 0x00,0xee,0x00, 0x00,0xee,0x00, 0x01,0xc7,0x00,
  0x01,0xc7,0x00, 0x01,0x83,0x00, 0x03,0x83,0x80, 0x03,0x01,0x80
};
const unsigned char PROGMEM vibe_frame1[] = {
  0x00,0x00,0x00, 0x00,0x38,0x00, 0x00,0x7c,0x00, 0x00,0xfe,0x00,
  0x01,0xaa,0x80, 0x01,0xba,0x80, 0x00,0xfe,0x00, 0x00,0x7c,0x00,
  0x07,0xfe,0x00, 0x0f,0xff,0xe0, 0x1f,0xff,0xf0, 0x1c,0xfe,0x70,
  0x38,0xfe,0x38, 0x30,0xfe,0x18, 0x00,0x7c,0x00, 0x00,0x7c,0x00,
  0x00,0xee,0x00, 0x01,0xc7,0x00, 0x03,0x83,0x80, 0x03,0x01,0x80,
  0x07,0x01,0xc0, 0x06,0x00,0xc0, 0x00,0x00,0x00, 0x00,0x00,0x00
};
const unsigned char PROGMEM vibe_frame2[] = {
  0x00,0x38,0x00, 0x00,0x7c,0x00, 0x00,0xfe,0x00, 0x01,0xaa,0x80,
  0x01,0xaa,0x80, 0x01,0xba,0x80, 0x00,0xfe,0x00, 0x00,0x7c,0x00,
  0x00,0x38,0x00, 0x07,0xfe,0x00, 0x0f,0xff,0x80, 0x1f,0xff,0xc0,
  0x1c,0xfe,0xe0, 0x18,0xfe,0x70, 0x00,0xfe,0x30, 0x00,0x7c,0x00,
  0x00,0x7c,0x00, 0x00,0xee,0x00, 0x00,0xee,0x00, 0x01,0xc7,0x00,
  0x01,0xc7,0x00, 0x03,0x83,0x80, 0x03,0x81,0x80, 0x01,0x80,0x00
};
const unsigned char PROGMEM vibe_frame3[] = {
  0x00,0x00,0x00, 0x00,0x38,0x00, 0x00,0x7c,0x00, 0x00,0xfe,0x00,
  0x01,0xaa,0x80, 0x01,0xba,0x80, 0x00,0xfe,0x00, 0x00,0x7c,0x00,
  0x00,0xfe,0xc0, 0x01,0xff,0xe0, 0x03,0xff,0xf0, 0x07,0xfe,0x38,
  0x0c,0xfe,0x1c, 0x08,0xfe,0x0c, 0x00,0x7c,0x00, 0x00,0x7c,0x00,
  0x00,0xee,0x00, 0x01,0xc7,0x00, 0x03,0x83,0x80, 0x03,0x01,0x80,
  0x07,0x01,0xc0, 0x06,0x00,0xc0, 0x00,0x00,0x00, 0x00,0x00,0x00
};

// 3. Spinning Vinyl Record (24x24, 4 Frames)
const unsigned char PROGMEM vinyl_frame0[] = {
  0x00,0x7e,0x00, 0x03,0xff,0xc0, 0x07,0x81,0xe0, 0x0e,0x7e,0x70,
  0x1c,0xff,0x38, 0x39,0x81,0x9c, 0x33,0x3c,0xcc, 0x73,0x7e,0xce,
  0x66,0xbd,0x66, 0x66,0xdb,0x66, 0x66,0xdb,0x67, 0x66,0xdb,0x67,
  0x66,0xbd,0x66, 0x66,0x7e,0x66, 0x73,0x3c,0xce, 0x33,0x18,0xcc,
  0x39,0x81,0x9c, 0x1c,0xff,0x38, 0x0e,0x7e,0x70, 0x07,0x81,0xe0,
  0x03,0xff,0xc0, 0x00,0x7e,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00
};
const unsigned char PROGMEM vinyl_frame1[] = {
  0x00,0x7e,0x00, 0x03,0xff,0xc0, 0x07,0xc3,0xe0, 0x0f,0x3c,0xf0,
  0x1e,0x7e,0x78, 0x3c,0xff,0x3c, 0x39,0xbd,0x9c, 0x73,0x7e,0xce,
  0x67,0xdb,0xe6, 0x66,0xdb,0x66, 0x66,0xdb,0x67, 0x66,0xdb,0x67,
  0x67,0xdb,0xe6, 0x66,0x7e,0x66, 0x73,0xbd,0xce, 0x39,0xff,0x9c,
  0x3c,0x7e,0x3c, 0x1e,0x3c,0x78, 0x0f,0xc3,0xf0, 0x07,0xff,0xe0,
  0x03,0xff,0xc0, 0x00,0x7e,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00
};
const unsigned char PROGMEM vinyl_frame2[] = {
  0x00,0x7e,0x00, 0x03,0xff,0xc0, 0x07,0xe7,0xe0, 0x0f,0xc3,0xf0,
  0x1f,0x3c,0xf8, 0x3e,0x7e,0x7c, 0x3c,0xff,0x3c, 0x79,0xbd,0x9e,
  0x73,0xdb,0xce, 0x66,0xdb,0x66, 0x66,0xdb,0x67, 0x66,0xdb,0x67,
  0x73,0xdb,0xce, 0x79,0xbd,0x9e, 0x3c,0xff,0x3c, 0x3e,0x7e,0x7c,
  0x1f,0x3c,0xf8, 0x0f,0xc3,0xf0, 0x07,0xe7,0xe0, 0x03,0xff,0xc0,
  0x00,0x7e,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00
};
const unsigned char PROGMEM vinyl_frame3[] = {
  0x00,0x7e,0x00, 0x03,0xff,0xc0, 0x07,0x81,0xe0, 0x0e,0x7e,0x70,
  0x1c,0xff,0x38, 0x39,0x81,0x9c, 0x33,0xbd,0xcc, 0x73,0x7e,0xce,
  0x66,0xdb,0x66, 0x66,0xdb,0x66, 0x66,0xdb,0x67, 0x66,0xdb,0x67,
  0x66,0xbd,0x66, 0x73,0x7e,0xce, 0x33,0xbd,0xcc, 0x39,0x81,0x9c,
  0x1c,0xff,0x38, 0x0e,0x7e,0x70, 0x07,0x81,0xe0, 0x03,0xff,0xc0,
  0x00,0x7e,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00
};

// 4. Bouncing Pixel Ghost / Blob (24x24, 4 Frames)
const unsigned char PROGMEM ghost_frame0[] = {
  0x00,0x7e,0x00, 0x01,0xff,0x80, 0x03,0xff,0xc0, 0x07,0xff,0xe0,
  0x0f,0xff,0xf0, 0x0f,0xbd,0xf0, 0x1f,0xbd,0xf8, 0x1f,0xff,0xf8,
  0x1f,0xff,0xf8, 0x1f,0xc3,0xf8, 0x1f,0xe7,0xf8, 0x1f,0xff,0xf8,
  0x0f,0xff,0xf0, 0x0f,0xff,0xf0, 0x07,0xff,0xe0, 0x07,0xff,0xe0,
  0x03,0xff,0xc0, 0x03,0xbd,0xc0, 0x03,0x18,0xc0, 0x00,0x00,0x00,
  0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00
};
const unsigned char PROGMEM ghost_frame1[] = {
  0x00,0x00,0x00, 0x00,0x7e,0x00, 0x01,0xff,0x80, 0x03,0xff,0xc0,
  0x07,0xff,0xe0, 0x0f,0xff,0xf0, 0x0f,0xbd,0xf0, 0x1f,0xbd,0xf8,
  0x1f,0xff,0xf8, 0x1f,0xc3,0xf8, 0x1f,0xe7,0xf8, 0x1f,0xff,0xf8,
  0x1f,0xff,0xf8, 0x0f,0xff,0xf0, 0x0f,0xff,0xf0, 0x07,0xff,0xe0,
  0x07,0xbd,0xe0, 0x07,0x18,0xe0, 0x00,0x00,0x00, 0x00,0x00,0x00,
  0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00
};
const unsigned char PROGMEM ghost_frame2[] = {
  0x00,0x7e,0x00, 0x01,0xff,0x80, 0x03,0xff,0xc0, 0x07,0xff,0xe0,
  0x0f,0xff,0xf0, 0x0f,0x99,0xf0, 0x1f,0x99,0xf8, 0x1f,0xff,0xf8,
  0x1f,0xff,0xf8, 0x1f,0xdb,0xf8, 0x1f,0xc3,0xf8, 0x1f,0xff,0xf8,
  0x0f,0xff,0xf0, 0x0f,0xff,0xf0, 0x07,0xff,0xe0, 0x07,0xff,0xe0,
  0x03,0xff,0xc0, 0x03,0x99,0xc0, 0x03,0x00,0xc0, 0x00,0x00,0x00,
  0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00
};
const unsigned char PROGMEM ghost_frame3[] = {
  0x00,0x00,0x00, 0x00,0x7e,0x00, 0x01,0xff,0x80, 0x03,0xff,0xc0,
  0x07,0xff,0xe0, 0x0f,0xff,0xf0, 0x0f,0xbd,0xf0, 0x1f,0xbd,0xf8,
  0x1f,0xff,0xf8, 0x1f,0xc3,0xf8, 0x1f,0xe7,0xf8, 0x1f,0xff,0xf8,
  0x1f,0xff,0xf8, 0x0f,0xff,0xf0, 0x07,0xff,0xe0, 0x07,0xff,0xe0,
  0x03,0xff,0xc0, 0x03,0xbd,0xc0, 0x03,0x18,0xc0, 0x00,0x00,0x00,
  0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00
};

void drawCompanion(int x, int y) {
  if (currentCompanion == COMPANION_NONE) return;
  
  const unsigned char* frame = NULL;
  if (currentCompanion == COMPANION_CAT) {
    if (companionFrame == 0) frame = cat_frame0;
    else if (companionFrame == 1) frame = cat_frame1;
    else if (companionFrame == 2) frame = cat_frame2;
    else frame = cat_frame3;
  } else if (currentCompanion == COMPANION_VIBE) {
    if (companionFrame == 0) frame = vibe_frame0;
    else if (companionFrame == 1) frame = vibe_frame1;
    else if (companionFrame == 2) frame = vibe_frame2;
    else frame = vibe_frame3;
  } else if (currentCompanion == COMPANION_VINYL) {
    if (companionFrame == 0) frame = vinyl_frame0;
    else if (companionFrame == 1) frame = vinyl_frame1;
    else if (companionFrame == 2) frame = vinyl_frame2;
    else frame = vinyl_frame3;
  } else if (currentCompanion == COMPANION_GHOST) {
    if (companionFrame == 0) frame = ghost_frame0;
    else if (companionFrame == 1) frame = ghost_frame1;
    else if (companionFrame == 2) frame = ghost_frame2;
    else frame = ghost_frame3;
  }
  
  if (frame != NULL) {
    display.drawBitmap(x, y, frame, 24, 24, SSD1306_WHITE);
  }
}

// Dynamic Font Caching Structures
struct LyricLayout {
  bool isInternational;
  const GFXfont* font;         // For English
  const uint8_t* u8g2_font;    // For International
  int lineCount;
  String lines[6];
  int startX;
  int startY;
  int lineHeight;
};

LyricLayout currentLayout;
LyricLayout oldLayout;

bool hasInternationalChars(String text) {
  for (int i = 0; i < text.length(); i++) {
    if ((uint8_t)text.charAt(i) >= 0x80) return true;
  }
  return false;
}

bool isCyrillic(String text) {
  for (int i = 0; i < text.length() - 1; i++) {
    uint8_t c = text.charAt(i);
    if (c == 0xD0 || c == 0xD1) return true;
  }
  return false;
}

LyricLayout calculateLayout(String text) {
  LyricLayout layout;
  layout.lineCount = 0;
  layout.font = NULL;
  layout.isInternational = hasInternationalChars(text);
  
  int maxWidth = (currentCompanion != COMPANION_NONE) ? 98 : 128;
  layout.startX = (currentCompanion != COMPANION_NONE) ? 28 : 0;
  
  if (text.length() == 0) {
    layout.isInternational = false;
    layout.font = getActiveFonts()[3];
    return layout;
  }

  if (layout.isInternational) {
    if (isCyrillic(text)) {
      layout.u8g2_font = u8g2_font_unifont_t_cyrillic;
    } else {
      layout.u8g2_font = u8g2_font_unifont_t_japanese1;
    }
    
    u8g2_gfx.setFont(layout.u8g2_font);
    layout.lineHeight = 16; 
    
    String lines[6];
    int lineCount = 0;
    String currentLine = "";
    int start = 0;
    
    for(int i = 0; i <= text.length(); i++) {
      if(i == text.length() || text.charAt(i) == ' ') {
        String word = text.substring(start, i);
        start = i + 1;
        if(word.length() == 0) continue;
        
        String testLine = currentLine + (currentLine.length()>0 ? " " : "") + word;
        int w = u8g2_gfx.getUTF8Width(testLine.c_str());
        
        if (w > maxWidth && currentLine.length() > 0) {
          if (lineCount < 6) lines[lineCount++] = currentLine;
          currentLine = word;
        } else {
          currentLine = testLine;
        }
      }
    }
    
    if (currentLine.length() > 0 && lineCount < 6) {
      lines[lineCount++] = currentLine;
    }
    
    layout.lineCount = lineCount;
    for (int k=0; k<lineCount; k++) layout.lines[k] = lines[k];
    
    int totalHeight = lineCount * layout.lineHeight;
    layout.startY = 24 - (totalHeight / 2) + layout.lineHeight - 4;
    return layout;
  }

  // --- ENGLISH DYNAMIC SCALING ---
  const GFXfont** fonts = getActiveFonts();
  int* heights = getActiveHeights();
  
  for (int f = 0; f < 5; f++) {
    bool isSmallestFallback = (f == 4);
    const GFXfont* testFont = fonts[f];
    int testLineHeight = heights[f];
    
    display.setFont(testFont);
    if (testFont == NULL) display.setTextSize(1);
    
    String lines[6];
    int lineCount = 0;
    String currentLine = "";
    int start = 0;
    bool wordTooWide = false;
    
    for(int i = 0; i <= text.length(); i++) {
      if(i == text.length() || text.charAt(i) == ' ') {
        String word = text.substring(start, i);
        start = i + 1;
        if(word.length() == 0) continue;
        
        int16_t x1, y1; uint16_t w, h;
        display.getTextBounds(word, 0, 0, &x1, &y1, &w, &h);
        if (w > maxWidth && !isSmallestFallback) { wordTooWide = true; break; } 
        
        String testLine = currentLine + (currentLine.length()>0 ? " " : "") + word;
        display.getTextBounds(testLine, 0, 0, &x1, &y1, &w, &h);
        
        if (w > maxWidth && currentLine.length() > 0) {
          if (lineCount < 6) lines[lineCount++] = currentLine;
          currentLine = word;
        } else {
          currentLine = testLine;
        }
      }
    }
    
    if (wordTooWide) continue;
    
    if (currentLine.length() > 0 && lineCount < 6) {
      lines[lineCount++] = currentLine;
    }
    
    int totalHeight = lineCount * testLineHeight;
    if (totalHeight <= 48 || isSmallestFallback) {
      layout.font = testFont;
      layout.lineCount = lineCount;
      layout.lineHeight = testLineHeight;
      for (int k=0; k<lineCount; k++) layout.lines[k] = lines[k];
      layout.startY = 24 - (totalHeight / 2) + testLineHeight - (testFont == NULL ? 0 : 4);
      return layout;
    }
  }
  return layout;
}

bool isGiantMode = false;
bool isSlidingMode = false;
bool isKineticMode = false;
bool isKineticV2Mode = false;
int highlightWordIndex = 0;

// ============================
// KINETIC V1 STATE
// ============================
int kineticTargetX = 0;
int kineticTargetY = 0;
int kineticCurrentX = 0;
int kineticCurrentY = 0;
int kineticAnimType = 0;

// ============================
// KOTODAMA V2 PARTICLE ENGINE
// ============================
#define MAX_PARTICLES 6

struct Particle {
  String word;
  float x, y;
  float vy;       // upward velocity
  float vx;       // horizontal drift
  int fontIndex;  // 0=24pt,1=18pt,2=12pt,3=9pt,4=null
  int animStyle;  // 0=float, 1=drift-right, 2=fadepop, 3=bounce
  int age;        // frames alive
  float bounceOffset;
  bool alive;
};

Particle particles[MAX_PARTICLES];
int nextParticleSlot = 0;
int spawnDiagX = 0;
int spawnDiagY = 44;

void initParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    particles[i].alive = false;
    particles[i].word = "";
  }
  spawnDiagX = (currentCompanion != COMPANION_NONE) ? 28 : 0;
  spawnDiagY = 44;
}

void spawnParticle(String word) {
  const GFXfont** fonts = getActiveFonts();
  int* heights = getActiveHeights();
  
  int fIdx = random(1, 5); // 1=18pt, 2=12pt, 3=9pt, 4=null
  const GFXfont* font = fonts[fIdx];

  int16_t x1, y1; uint16_t ww, hh;
  display.setFont(font);
  if (font == NULL) display.setTextSize(1);
  display.getTextBounds(word, 0, 0, &x1, &y1, &ww, &hh);
  int wordWidth = (int)ww;

  int minX = (currentCompanion != COMPANION_NONE) ? 28 : 0;
  int sx = spawnDiagX;
  int sy = spawnDiagY;

  if (sx + wordWidth > 124) sx = max(minX, 124 - wordWidth);

  spawnDiagX += 14;
  spawnDiagY -= 5;
  if (spawnDiagX > (minX + 60)) { spawnDiagX = minX; spawnDiagY = 44; }

  int style = random(0, 4);

  int slot = -1;
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].alive) { slot = i; break; }
  }
  if (slot < 0) {
    slot = nextParticleSlot;
    nextParticleSlot = (nextParticleSlot + 1) % MAX_PARTICLES;
  }

  particles[slot].word = word;
  particles[slot].fontIndex = fIdx;
  particles[slot].animStyle = style;
  particles[slot].age = 0;
  particles[slot].bounceOffset = 0;
  particles[slot].alive = true;

  if (style == 1) {
    particles[slot].x = minX - wordWidth;
    particles[slot].vx = 1.5;
  } else {
    particles[slot].x = sx;
    particles[slot].vx = (style == 3) ? 0.3 : 0.0;
  }
  particles[slot].y = sy;
  particles[slot].vy = -(0.4 + random(0, 4) * 0.1);
}

void updateParticles() {
  int* heights = getActiveHeights();
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].alive) continue;
    Particle& p = particles[i];
    p.age++;
    p.y += p.vy;
    p.x += p.vx;
    if (p.animStyle == 3) {
      p.bounceOffset = sin(p.age * 0.25) * 3.0;
    }
    if (p.y < -heights[p.fontIndex]) p.alive = false;
    if (p.x > 132) p.alive = false;
  }
}

void drawParticles() {
  const GFXfont** fonts = getActiveFonts();
  int* heights = getActiveHeights();
  
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].alive) continue;
    Particle& p = particles[i];
    const GFXfont* font = fonts[p.fontIndex];
    int drawY = (int)(p.y + p.bounceOffset);

    if (drawY > 47 || drawY < -heights[p.fontIndex]) continue;

    display.setFont(font);
    if (font == NULL) display.setTextSize(1);

    if (p.animStyle == 2 && p.age < 4) {
      int scaledIdx = min(p.fontIndex + (3 - p.age), 4);
      display.setFont(fonts[scaledIdx]);
      if (fonts[scaledIdx] == NULL) display.setTextSize(1);
    }

    display.setCursor((int)p.x, drawY);
    display.print(p.word);
  }
}

void parseSerialData(String data) {
  data.trim();
  if (data.startsWith("M|")) {
    int split1 = data.indexOf('|', 2);
    int split2 = data.indexOf('|', split1 + 1);
    int split3 = data.indexOf('|', split2 + 1);
    
    if (split1 > 0 && split2 > 0 && split3 > 0) {
      String newTitle = data.substring(2, split1);
      String newArtist = data.substring(split1 + 1, split2);
      if (newTitle != songTitle || newArtist != songArtist) {
        songTitle = newTitle;
        songArtist = newArtist;
        marqueeX = 128; 
      }
      progressMs = data.substring(split2 + 1, split3).toFloat();
      durationMs = data.substring(split3 + 1).toFloat();
      lastUpdateMs = millis();
    }
  } 
  else if (data.startsWith("L|")) {
    int split = data.indexOf('|', 2);
    if (split > 0) {
      String newCur = data.substring(2, split);
      if (newCur != currentLyric) {
        oldLyric = currentLyric;
        oldLayout = currentLayout;
        
        currentLyric = newCur;
        currentLayout = calculateLayout(currentLyric);
        
        if (isKineticV2Mode) {
          animating = false;
          spawnParticle(newCur);
        }
        else if (isKineticMode) {
          animating = false;
          const GFXfont** fonts = getActiveFonts();
          int* heights = getActiveHeights();
          
          if (!currentLayout.isInternational) {
            int rFont = random(1, 4); // 1=18pt, 2=12pt, 3=9pt
            currentLayout.font = fonts[rFont];
            currentLayout.lineHeight = heights[rFont];
          }
          
          int16_t x1, y1; uint16_t w1, h1;
          if (currentLayout.isInternational) {
            u8g2_gfx.setFont(currentLayout.u8g2_font);
            w1 = u8g2_gfx.getUTF8Width(currentLyric.c_str());
            h1 = 16;
          } else {
            display.setFont(currentLayout.font);
            display.getTextBounds(currentLyric, 0, 0, &x1, &y1, &w1, &h1);
          }
          
          int minX = (currentCompanion != COMPANION_NONE) ? 28 : 0;
          kineticTargetX = random(minX, 128 - w1);
          if (kineticTargetX < minX) kineticTargetX = minX;
          
          kineticTargetY = random(h1 + 5, 45);
          
          kineticAnimType = random(0, 3);
          if (kineticAnimType == 0) { kineticCurrentX = -128; kineticCurrentY = kineticTargetY; }
          else if (kineticAnimType == 1) { kineticCurrentX = 128; kineticCurrentY = kineticTargetY; }
          else { kineticCurrentX = kineticTargetX; kineticCurrentY = kineticTargetY; }
        }
        else if (isGiantMode) {
          animating = false;
        } else {
          animating = true;
          animOffset = 0;
        }
      }
    }
  }
  else if (data.startsWith("S|")) {
    bool wasV2 = isKineticV2Mode;
    isGiantMode = (data.indexOf("GIANT") > 0);
    isSlidingMode = (data.indexOf("SLIDING") > 0);
    isKineticMode = (data.indexOf("KINETIC2") < 0 && data.indexOf("KINETIC") > 0);
    isKineticV2Mode = (data.indexOf("KINETIC2") > 0);
    if (isKineticV2Mode && !wasV2) initParticles();
  }
  else if (data.startsWith("W|")) {
    highlightWordIndex = data.substring(2).toInt();
  }
  else if (data.startsWith("F|")) {
    String f = data.substring(2);
    f.toUpperCase();
    if (f.indexOf("HANDWRITTEN") >= 0 || f.indexOf("SCRIPT") >= 0 || f.indexOf("SERIF") >= 0) {
      currentFontStyle = FONT_HANDWRITTEN;
    } else if (f.indexOf("MONO") >= 0) {
      currentFontStyle = FONT_MONO;
    } else {
      currentFontStyle = FONT_SANS;
    }
    currentLayout = calculateLayout(currentLyric);
  }
  else if (data.startsWith("C|")) {
    String c = data.substring(2);
    c.toUpperCase();
    if (c.indexOf("CAT") >= 0) currentCompanion = COMPANION_CAT;
    else if (c.indexOf("VIBE") >= 0 || c.indexOf("DANCE") >= 0) currentCompanion = COMPANION_VIBE;
    else if (c.indexOf("VINYL") >= 0) currentCompanion = COMPANION_VINYL;
    else if (c.indexOf("GHOST") >= 0) currentCompanion = COMPANION_GHOST;
    else currentCompanion = COMPANION_NONE;
    currentLayout = calculateLayout(currentLyric);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 4);
  Wire.setClock(400000);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Wire.end(); Wire.begin(4, 15); Wire.setClock(400000);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
      Wire.end(); Wire.begin(21, 22); Wire.setClock(400000);
      display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    }
  }

  display.setRotation(2); 
  display.clearDisplay();
  display.setTextWrap(false); 
  
  u8g2_gfx.begin(display);
  u8g2_gfx.setFontMode(1);
  u8g2_gfx.setFontDirection(0);
  u8g2_gfx.setForegroundColor(SSD1306_WHITE);
  
  display.display();
  
  currentLayout = calculateLayout("...");
}

void drawCachedLyric(LyricLayout &layout, int ySlide) {
  if (layout.lineCount == 0) return;
  
  int startX = layout.startX;
  
  if (isKineticMode) {
    if (layout.isInternational) {
       u8g2_gfx.setFont(layout.u8g2_font);
       u8g2_gfx.setCursor(kineticCurrentX, kineticCurrentY);
       u8g2_gfx.print(layout.lines[0]);
    } else {
       if (layout.font == NULL) display.setTextSize(1);
       display.setFont(layout.font);
       display.setCursor(kineticCurrentX, kineticCurrentY);
       display.print(layout.lines[0]);
    }
    return;
  }
  
  if (layout.isInternational) {
    u8g2_gfx.setFont(layout.u8g2_font);
    for(int i=0; i<layout.lineCount; i++) {
      u8g2_gfx.setCursor(startX, layout.startY + (i * layout.lineHeight) - ySlide);
      u8g2_gfx.print(layout.lines[i]);
    }
  } else {
    if (layout.font == NULL) {
      display.setFont(NULL);
      display.setTextSize(1);
    } else {
      display.setFont(layout.font);
    }
    
    int wordCount = 0;
    for(int i=0; i<layout.lineCount; i++) {
      int lineY = layout.startY + (i * layout.lineHeight) - ySlide;
      display.setCursor(startX, lineY);
      display.print(layout.lines[i]);
      
      // Highlight Logic for Sliding Mode
      if (isSlidingMode && highlightWordIndex >= 0) {
        String words[20];
        int numWords = 0;
        int startIndex = 0;
        for (int k = 0; k <= layout.lines[i].length(); k++) {
          if (k == layout.lines[i].length() || layout.lines[i].charAt(k) == ' ') {
             words[numWords++] = layout.lines[i].substring(startIndex, k);
             startIndex = k + 1;
          }
        }
        
        for (int w = 0; w < numWords; w++) {
          if (wordCount == highlightWordIndex && currentLyric != "..." && currentLyric != "♫" && currentLyric != "♥" && currentLyric != "★" && currentLyric != "☺") {
            String textBefore = "";
            for(int prev=0; prev<w; prev++) {
              textBefore += words[prev] + " ";
            }
            String textWithWord = textBefore + words[w];
            
            int highlightX = 0;
            int highlightW = 0;
            
            if (layout.isInternational) {
                u8g2_gfx.setFont(layout.u8g2_font);
                highlightX = (w == 0) ? 0 : u8g2_gfx.getUTF8Width(textBefore.c_str());
                highlightW = u8g2_gfx.getUTF8Width(textWithWord.c_str()) - highlightX;
            } else {
                int16_t x1, y1; uint16_t w1, h1;
                display.getTextBounds(textBefore, 0, lineY, &x1, &y1, &w1, &h1);
                highlightX = (w == 0) ? 0 : w1;
                
                display.getTextBounds(textWithWord, 0, lineY, &x1, &y1, &w1, &h1);
                highlightW = w1 - highlightX;
            }
            
            int rectY = (layout.isInternational) ? lineY - 14 : ((layout.font == NULL) ? lineY - 1 : lineY - layout.lineHeight + 4);
            int rectH = (layout.isInternational) ? 16 : ((layout.font == NULL) ? 9 : layout.lineHeight - 2);
            
            display.fillRect(startX + highlightX - 2, rectY, highlightW + 4, rectH, SSD1306_INVERSE);
          }
          wordCount++;
        }
      }
    }
  }
}

void loop() {
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    parseSerialData(line);
  }
  
  unsigned long now = millis();
  
  // Companion Frame Advance (~160ms per frame)
  if (now - lastCompanionMs > 160) {
    lastCompanionMs = now;
    companionFrame = (companionFrame + 1) % 4;
  }
  
  if (now - lastMarqueeMs > 20) {
    lastMarqueeMs = now;
    String headerText = songTitle + " - " + songArtist;
    int textWidth = 0;
    
    if (hasInternationalChars(headerText)) {
        u8g2_gfx.setFont(u8g2_font_unifont_t_japanese1);
        textWidth = u8g2_gfx.getUTF8Width(headerText.c_str());
    } else {
        textWidth = headerText.length() * 6;
    }
    
    if (textWidth > 128) {
      marqueeX -= 1; 
      if (marqueeX < -(textWidth + 10)) {
        marqueeX = 128;
      }
    } else {
        marqueeX = (128 - textWidth) / 2;
    }
  }

  if (now - lastFrameMs > 10) { 
    lastFrameMs = now;
    if (animating) {
      animOffset += 4;
      if (animOffset > 48) {
        animating = false;
      }
    }
    
    if (isKineticMode) {
      if (kineticCurrentX < kineticTargetX) {
        kineticCurrentX += 20; 
        if (kineticCurrentX > kineticTargetX) kineticCurrentX = kineticTargetX;
      } else if (kineticCurrentX > kineticTargetX) {
        kineticCurrentX -= 20;
        if (kineticCurrentX < kineticTargetX) kineticCurrentX = kineticTargetX;
      }
    }
    if (isKineticV2Mode) {
      updateParticles();
    }
  }

  display.clearDisplay();
  
  // =====================================
  // BLUE ZONE (Physical Top, Code y=0-47)
  // =====================================
  
  bool isIdle = (currentLyric == "..." || currentLyric == "♫" || currentLyric == "♥" || currentLyric == "★" || currentLyric == "☺" || songTitle == "Waiting for Spotify");
  
  if (currentCompanion != COMPANION_NONE) {
    if (isIdle) {
      // Draw Mascot Centered when idle
      drawCompanion(52, 10);
    } else {
      // Draw Mascot on Left when lyrics are flowing
      drawCompanion(2, 10);
    }
  }

  if (!isIdle) {
    if (isKineticV2Mode) {
      drawParticles();
    } else if (animating) {
      drawCachedLyric(oldLayout, animOffset);
      drawCachedLyric(currentLayout, animOffset - 64);
    } else {
      drawCachedLyric(currentLayout, 0);
    }
  }
  
  // Mask overlaps
  display.fillRect(0, 48, 128, 16, SSD1306_BLACK); 

  // =====================================
  // YELLOW ZONE (Physical Bottom, Code y=48-63)
  // =====================================
  
  String headerText = songTitle;
  if (songArtist.length() > 0 && songArtist != "Unknown") headerText += " - " + songArtist;
  
  if (hasInternationalChars(headerText)) {
    if (isCyrillic(headerText)) {
      u8g2_gfx.setFont(u8g2_font_unifont_t_cyrillic);
    } else {
      u8g2_gfx.setFont(u8g2_font_unifont_t_japanese1);
    }
    u8g2_gfx.setCursor((int)marqueeX, 60);
    u8g2_gfx.print(headerText);
  } else {
    display.setFont(NULL); 
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);
    display.setCursor((int)marqueeX, 48); 
    display.print(headerText);
  }

  float currentProg = progressMs + (millis() - lastUpdateMs);
  if (currentProg > durationMs) currentProg = durationMs;
  if (durationMs < 1) durationMs = 1;
  int barWidth = (currentProg / durationMs) * 128;
  
  display.drawRect(0, 58, 128, 4, SSD1306_WHITE);
  display.fillRect(0, 58, barWidth, 4, SSD1306_WHITE);

  display.display();
}
