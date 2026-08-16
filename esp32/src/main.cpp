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
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

// Handwritten / Aesthetic Script Fonts (Insta & TikTok Cursive Style)
#include <Fonts/FreeSerifBoldItalic18pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
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
bool isTwoLineSongInfo = true; // Toggle between 2-Line static and 1-Line scrolling marquee

// ==========================================
// PROCEDURAL EASING FUNCTIONS
// ==========================================
float easeLinear(float t) { return t; }
float easeOutQuad(float t) { return t * (2.0f - t); }
float easeInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; }
float easeOutBack(float t) {
  float c1 = 1.70158f; float c3 = c1 + 1.0f;
  return 1.0f + c3 * pow(t - 1.0f, 3) + c1 * pow(t - 1.0f, 2);
}
float easeOutBounce(float t) {
  float n1 = 7.5625f, d1 = 2.75f;
  if (t < 1.0f / d1) return n1 * t * t;
  else if (t < 2.0f / d1) { t -= 1.5f / d1; return n1 * t * t + 0.75f; }
  else if (t < 2.5f / d1) { t -= 2.25f / d1; return n1 * t * t + 0.9375f; }
  else { t -= 2.625f / d1; return n1 * t * t + 0.984375f; }
}

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

const GFXfont* scriptFonts[] = {&FreeSerifBoldItalic18pt7b, &FreeSerifBoldItalic12pt7b, &FreeSerifBoldItalic9pt7b, &FreeSerifItalic9pt7b, NULL};
int scriptHeights[] = {28, 20, 15, 12, 8};

const GFXfont* monoFonts[] = {&FreeMonoBold18pt7b, &FreeMonoBold12pt7b, &FreeMonoBold9pt7b, &FreeMono9pt7b, NULL};
int monoHeights[] = {26, 18, 14, 10, 8};

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
// PROCEDURAL PROGRESSIVE VECTOR DOODLERS
// ==========================================

// 1. Blooming Hand-Sketched Rose / Flower
void drawProgressiveRose(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  
  // Center Spiral Bud
  int spiralSteps = (int)(min(1.0f, progress * 1.5f) * 12.0f);
  int lastX = cx, lastY = cy;
  for (int i = 0; i <= spiralSteps; i++) {
    float angle = i * 0.5f;
    float r = 0.8f * (float)i;
    int px = cx + (int)(cos(angle) * r);
    int py = cy + (int)(sin(angle) * r * 0.7f);
    if (i > 0) display.drawLine(lastX, lastY, px, py, SSD1306_WHITE);
    lastX = px; lastY = py;
  }
  
  // Petals Layer 1 (Left & Right curves)
  if (progress > 0.35f) {
    float p1 = min(1.0f, (progress - 0.35f) * 2.0f);
    int r = 6;
    for (float a = 0.5f; a < 2.8f * p1; a += 0.3f) {
      display.drawPixel(cx - (int)(cos(a) * r), cy - (int)(sin(a) * r * 0.8f), SSD1306_WHITE);
      display.drawPixel(cx + (int)(cos(a) * r), cy - (int)(sin(a) * r * 0.8f), SSD1306_WHITE);
    }
  }
  
  // Stem & Leaf
  if (progress > 0.6f) {
    float p2 = min(1.0f, (progress - 0.6f) * 2.5f);
    int stemLen = (int)(p2 * 9.0f);
    display.drawLine(cx, cy + 3, cx - 1, cy + 3 + stemLen, SSD1306_WHITE);
    // Leaf
    if (p2 > 0.5f) {
      display.drawLine(cx, cy + 6, cx + 4, cy + 4, SSD1306_WHITE);
      display.drawLine(cx + 4, cy + 4, cx + 2, cy + 7, SSD1306_WHITE);
    }
  }
}

// 2. Sketched Heart with Pulse
void drawProgressiveHeart(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  int steps = (int)(min(1.0f, progress * 1.3f) * 16.0f);
  int lastX = cx, lastY = cy;
  for (int i = 0; i <= steps; i++) {
    float t = (i / 16.0f) * 2.0f * PI;
    float x = 16.0f * pow(sin(t), 3);
    float y = -(13.0f * cos(t) - 5.0f * cos(2.0f*t) - 2.0f * cos(3.0f*t) - cos(4.0f*t));
    int px = cx + (int)((x / 16.0f) * 6.5f);
    int py = cy + (int)((y / 16.0f) * 6.5f);
    if (i > 0) display.drawLine(lastX, lastY, px, py, SSD1306_WHITE);
    lastX = px; lastY = py;
  }
  // Mini heart sparkle
  if (progress > 0.7f) {
    display.drawPixel(cx + 6, cy - 6, SSD1306_WHITE);
    display.drawPixel(cx + 7, cy - 7, SSD1306_WHITE);
  }
}

// 3. Procedural Flickering Fire / Flame
void drawProgressiveFlame(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  int h = (int)(progress * 12.0f);
  int flicker = (time / 80) % 3;
  // Outer flame
  display.drawLine(cx - 4, cy, cx - 1, cy - h + flicker, SSD1306_WHITE);
  display.drawLine(cx + 4, cy, cx + 1, cy - h + (2 - flicker), SSD1306_WHITE);
  display.drawLine(cx - 1, cy - h + flicker, cx, cy - h - 2 + flicker, SSD1306_WHITE);
  display.drawLine(cx + 1, cy - h + (2 - flicker), cx, cy - h - 2 + flicker, SSD1306_WHITE);
  // Inner core
  if (h > 6) {
    display.drawLine(cx - 2, cy, cx, cy - h + 4, SSD1306_WHITE);
    display.drawLine(cx + 2, cy, cx, cy - h + 4, SSD1306_WHITE);
  }
}

// 4. Falling Rain & Tears
void drawProgressiveRain(int x, int y, int w, int h, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  int numDrops = (int)(progress * 8.0f);
  int seed = (time / 100) % 5;
  for (int i = 0; i < numDrops; i++) {
    int rx = x + ((i * 19 + seed * 7) % w);
    int ry = y + ((i * 13 + (int)(time / 20)) % h);
    display.drawLine(rx, ry, rx - 1, ry + 3, SSD1306_WHITE);
    // Splash at bottom
    if (ry + 3 >= y + h - 2) {
      display.drawPixel(rx - 2, y + h - 1, SSD1306_WHITE);
      display.drawPixel(rx + 1, y + h - 1, SSD1306_WHITE);
    }
  }
}

// 5. Twinkling 4-Point Star
void drawProgressiveStar(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  int len = (int)(progress * 5.0f);
  display.drawLine(cx - len, cy, cx + len, cy, SSD1306_WHITE);
  display.drawLine(cx, cy - len, cx, cy + len, SSD1306_WHITE);
  if (progress > 0.5f) {
    display.drawPixel(cx - 1, cy - 1, SSD1306_WHITE);
    display.drawPixel(cx + 1, cy + 1, SSD1306_WHITE);
    display.drawPixel(cx - 1, cy + 1, SSD1306_WHITE);
    display.drawPixel(cx + 1, cy - 1, SSD1306_WHITE);
  }
}

// 6. Broken / Shatter Fractures
void drawProgressiveBroken(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  int len = (int)(progress * 14.0f);
  display.drawLine(cx - len/2, cy - len/2, cx - 2, cy - 1, SSD1306_WHITE);
  display.drawLine(cx - 2, cy - 1, cx + 3, cy + 2, SSD1306_WHITE);
  display.drawLine(cx + 3, cy + 2, cx + len/2, cy + len/2, SSD1306_WHITE);
  if (progress > 0.6f) {
    display.drawLine(cx - 1, cy - 1, cx - 4, cy + 4, SSD1306_WHITE);
    display.drawLine(cx + 2, cy + 1, cx + 5, cy - 4, SSD1306_WHITE);
  }
}

// 7. Angel / Bird Wings
void drawProgressiveWings(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  int w = (int)(progress * 12.0f);
  // Left wing
  display.drawLine(cx - 1, cy, cx - w/2, cy - 4, SSD1306_WHITE);
  display.drawLine(cx - w/2, cy - 4, cx - w, cy - 2, SSD1306_WHITE);
  display.drawLine(cx - w, cy - 2, cx - w/2, cy + 1, SSD1306_WHITE);
  display.drawLine(cx - w/2, cy + 1, cx - 1, cy, SSD1306_WHITE);
  // Right wing
  display.drawLine(cx + 1, cy, cx + w/2, cy - 4, SSD1306_WHITE);
  display.drawLine(cx + w/2, cy - 4, cx + w, cy - 2, SSD1306_WHITE);
  display.drawLine(cx + w, cy - 2, cx + w/2, cy + 1, SSD1306_WHITE);
  display.drawLine(cx + w/2, cy + 1, cx + 1, cy, SSD1306_WHITE);
}

// 8. Sunburst / Radiance
void drawProgressiveSun(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy, 3, SSD1306_WHITE);
  int rayLen = (int)(progress * 4.0f);
  display.drawLine(cx - 3 - rayLen, cy, cx - 4, cy, SSD1306_WHITE);
  display.drawLine(cx + 4, cy, cx + 3 + rayLen, cy, SSD1306_WHITE);
  display.drawLine(cx, cy - 3 - rayLen, cx, cy - 4, SSD1306_WHITE);
  display.drawLine(cx, cy + 4, cx, cy + 3 + rayLen, SSD1306_WHITE);
  if (progress > 0.6f) {
    display.drawPixel(cx - 3, cy - 3, SSD1306_WHITE);
    display.drawPixel(cx + 3, cy - 3, SSD1306_WHITE);
    display.drawPixel(cx - 3, cy + 3, SSD1306_WHITE);
    display.drawPixel(cx + 3, cy + 3, SSD1306_WHITE);
  }
}

// 9. Electric Lightning Bolt
void drawProgressiveLightning(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx, cy - 7, cx - 3, cy - 1, SSD1306_WHITE);
  display.drawLine(cx - 3, cy - 1, cx + 1, cy - 1, SSD1306_WHITE);
  if (progress > 0.4f) {
    display.drawLine(cx + 1, cy - 1, cx - 2, cy + 6, SSD1306_WHITE);
  }
}

// 10. Sketched Eye
void drawProgressiveEye(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  int w = (int)(progress * 8.0f);
  display.drawLine(cx - w, cy, cx, cy - 4, SSD1306_WHITE);
  display.drawLine(cx, cy - 4, cx + w, cy, SSD1306_WHITE);
  display.drawLine(cx - w, cy, cx, cy + 4, SSD1306_WHITE);
  display.drawLine(cx, cy + 4, cx + w, cy, SSD1306_WHITE);
  if (progress > 0.5f) {
    display.fillCircle(cx, cy, 1, SSD1306_WHITE);
  }
}

// 11. Sketched Arrow
void drawProgressiveArrow(int x1, int y1, int x2, int y2, float progress) {
  if (progress <= 0.0f) return;
  int curX = x1 + (int)((x2 - x1) * min(1.0f, progress * 1.3f));
  int curY = y1 + (int)((y2 - y1) * min(1.0f, progress * 1.3f));
  display.drawLine(x1, y1, curX, curY, SSD1306_WHITE);
  if (progress > 0.6f) {
    display.drawLine(curX, curY, curX - 3, curY - 2, SSD1306_WHITE);
    display.drawLine(curX, curY, curX - 3, curY + 2, SSD1306_WHITE);
  }
}

// 12. Sketched Underline
void drawProgressiveUnderline(int x1, int x2, int y, float progress) {
  if (progress <= 0.0f) return;
  int targetX = x1 + (int)((x2 - x1) * min(1.0f, progress * 1.3f));
  int curX = x1;
  int step = max(4, (x2 - x1) / 4);
  int curY = y;
  while (curX < targetX) {
    int nextX = min(targetX, curX + step);
    int nextY = y + ((curX / step) % 2 == 0 ? 1 : -1);
    display.drawLine(curX, curY, nextX, nextY, SSD1306_WHITE);
    curX = nextX;
    curY = nextY;
  }
}

// 13. Sketched Circle Loop around Keyword
void drawProgressiveCircle(int cx, int cy, int rx, int ry, float progress) {
  if (progress <= 0.0f) return;
  int steps = (int)(min(1.0f, progress * 1.2f) * 20.0f);
  int lastX = cx + rx, lastY = cy;
  for (int i = 0; i <= steps; i++) {
    float a = (i / 20.0f) * 2.0f * PI;
    int px = cx + (int)(cos(a) * rx);
    int py = cy + (int)(sin(a) * ry);
    if (i > 0) display.drawLine(lastX, lastY, px, py, SSD1306_WHITE);
    lastX = px; lastY = py;
  }
}

// Full-ASCII Text Rendering with Progressive Reveal
void drawProgressiveText(String text, int x, int y, int fontChoice, float progress) {
  if (text.length() == 0 || progress <= 0.0f) return;
  
  // Reveal text smoothly (guaranteeing all letters appear!)
  int visibleChars = max(1, (int)(text.length() * min(1.0f, progress * 2.0f)));
  String sub = text.substring(0, visibleChars);
  
  if (fontChoice == 0) {
    display.setFont(&FreeSerifBoldItalic12pt7b);
    display.setCursor(x, y);
    display.print(sub);
  } else if (fontChoice == 1) {
    display.setFont(&FreeSerifBoldItalic9pt7b);
    display.setCursor(x, y);
    display.print(sub);
  } else if (fontChoice == 2) {
    display.setFont(&FreeSerifItalic9pt7b);
    display.setCursor(x, y);
    display.print(sub);
  } else if (fontChoice == 3) {
    display.setFont(&FreeSans9pt7b);
    display.setCursor(x, y);
    display.print(sub);
  } else {
    display.setFont(NULL);
    display.setTextSize(1);
    display.setCursor(x, y);
    display.print(sub);
  }
}

// ==========================================
// SCENE GRAPH & PROCEDURAL STATE
// ==========================================
bool isSketchbookMode = true;
String sketchMetaphor = "NORMAL";
String sketchDoodle = "NONE";
String sketchComposition = "CENTER";
String sketchFocalWord = "";
String sketchPrefix = "";
String sketchSuffix = "";
int sketchTilt = 0;
bool sketchUnderline = false;
uint32_t sketchDurationMs = 3000;
unsigned long sketchSceneStartMs = 0;

// Dynamic Font Caching Structures
struct LyricLayout {
  bool isInternational;
  const GFXfont* font;         
  const uint8_t* u8g2_font;    
  int lineCount;
  String lines[6];
  int lineStartX[6];
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
  layout.u8g2_font = NULL;
  layout.isInternational = hasInternationalChars(text);
  
  int maxWidth = 126;
  layout.startX = 2;
  int availWidth = 126;
  
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
    for (int k=0; k<lineCount; k++) {
      layout.lines[k] = lines[k];
      int lw = u8g2_gfx.getUTF8Width(lines[k].c_str());
      int lx = layout.startX + (availWidth - lw) / 2;
      layout.lineStartX[k] = max(layout.startX, lx);
    }
    
    int totalHeight = lineCount * layout.lineHeight;
    int topY = max(0, (48 - totalHeight) / 2);
    layout.startY = topY + layout.lineHeight - 3;
    return layout;
  }

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
        
        int w = 0;
        if (testFont != NULL) {
          int16_t x1, y1; uint16_t tw, th;
          display.getTextBounds(word, 0, 0, &x1, &y1, &tw, &th);
          w = tw;
        } else {
          w = word.length() * 6;
        }
        
        if (w > maxWidth && !isSmallestFallback) { wordTooWide = true; break; } 
        
        String testLine = currentLine + (currentLine.length()>0 ? " " : "") + word;
        if (testFont != NULL) {
          int16_t x1, y1; uint16_t tw, th;
          display.getTextBounds(testLine, 0, 0, &x1, &y1, &tw, &th);
          w = tw;
        } else {
          w = testLine.length() * 6;
        }
        
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
      
      for (int k=0; k<lineCount; k++) {
        layout.lines[k] = lines[k];
        int lw = 0;
        if (testFont != NULL) {
          int16_t x1, y1; uint16_t tw, th;
          display.getTextBounds(lines[k], 0, 0, &x1, &y1, &tw, &th);
          lw = tw;
        } else {
          lw = lines[k].length() * 6;
        }
        int lx = layout.startX + (availWidth - lw) / 2;
        layout.lineStartX[k] = max(layout.startX, lx);
      }
      
      int topY = max(0, (48 - totalHeight) / 2);
      if (testFont != NULL) {
        layout.startY = topY + testLineHeight - 3;
      } else {
        layout.startY = topY;
      }
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
// KINETIC V1 & V2 PARTICLES
// ============================
int kineticTargetX = 0;
int kineticTargetY = 0;
int kineticCurrentX = 0;
int kineticCurrentY = 0;
int kineticAnimType = 0;

#define MAX_PARTICLES 6
struct Particle {
  String word;
  float x, y;
  float vy;
  float vx;
  int fontIndex;
  int animStyle;
  int age;
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
  spawnDiagX = 0;
  spawnDiagY = 44;
}

void spawnParticle(String word) {
  const GFXfont** fonts = getActiveFonts();
  int fIdx = random(0, 4);
  const GFXfont* font = fonts[fIdx];

  int16_t x1, y1; uint16_t ww, hh;
  display.setFont(font);
  if (font == NULL) display.setTextSize(1);
  display.getTextBounds(word, 0, 0, &x1, &y1, &ww, &hh);
  int wordWidth = (int)ww;

  int sx = spawnDiagX;
  int sy = spawnDiagY;

  if (sx + wordWidth > 124) sx = max(0, 124 - wordWidth);

  spawnDiagX += 16;
  spawnDiagY -= 5;
  if (spawnDiagX > 80) { spawnDiagX = 0; spawnDiagY = 44; }

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
    particles[slot].x = -wordWidth;
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
    display.setTextColor(SSD1306_WHITE);
    display.setCursor((int)p.x, drawY);
    display.print(p.word);
  }
}

// ==========================================
// PROCEDURAL SKETCHBOOK SCENE ENGINE
// ==========================================
void drawSketchbookScene() {
  unsigned long elapsed = millis() - sketchSceneStartMs;
  float rawProgress = (sketchDurationMs > 0) ? ((float)elapsed / sketchDurationMs) : 1.0f;
  if (rawProgress > 1.0f) rawProgress = 1.0f;
  
  float enterT = min(1.0f, rawProgress * 2.5f);
  float enterEased = easeOutBack(enterT);
  
  int minX = 2;
  int availWidth = 124;
  
  int focalY = 26;
  int focalXOffset = 0;
  
  // Dynamic Semantic Motion Physics
  if (sketchMetaphor == "FALLING") {
    focalY = (int)(4 + easeOutBounce(enterT) * 22.0f);
  } else if (sketchMetaphor == "FLYING") {
    focalY = (int)(42 - easeOutQuad(enterT) * 18.0f);
  } else if (sketchMetaphor == "RUNNING") {
    focalXOffset = (int)(-40 + enterEased * 40.0f);
  } else if (sketchMetaphor == "SPINNING") {
    focalXOffset = (int)(sin(rawProgress * 12.0f) * 8.0f);
  }
  
  // 1. Render Prefix (Full ASCII Serif / Sans)
  if (sketchPrefix.length() > 0 && sketchComposition != "ISOLATED") {
    display.setFont(&FreeSerifItalic9pt7b);
    int16_t x1, y1; uint16_t pw, ph;
    display.getTextBounds(sketchPrefix, 0, 0, &x1, &y1, &pw, &ph);
    
    int px = minX + (availWidth - (int)pw) / 2;
    if (sketchComposition == "DIAGONAL") px = minX + 2;
    
    drawProgressiveText(sketchPrefix, max(minX, px), 11, 2, rawProgress);
  }
  
  // 2. Render Focal Word with Progressive Cursive / Serif
  if (sketchFocalWord.length() > 0) {
    int fontId = 0; // FreeSerifBoldItalic12pt
    display.setFont(&FreeSerifBoldItalic12pt7b);
    int16_t x1, y1; uint16_t fw, fh;
    display.getTextBounds(sketchFocalWord, 0, 0, &x1, &y1, &fw, &fh);
    
    if (fw > availWidth) {
      fontId = 1; // FreeSerifBoldItalic9pt
      display.setFont(&FreeSerifBoldItalic9pt7b);
      display.getTextBounds(sketchFocalWord, 0, 0, &x1, &y1, &fw, &fh);
    }
    
    int fx = minX + (availWidth - (int)fw) / 2 + focalXOffset;
    if (fx < minX) fx = minX;
    
    drawProgressiveText(sketchFocalWord, fx, focalY, fontId, rawProgress);
    
    // Hand-Drawn Wobbly Underline
    if (sketchUnderline) {
      drawProgressiveUnderline(fx - 2, fx + fw + 2, focalY + 3, rawProgress);
    }
    
    // Hand-Drawn Circle
    if (sketchDoodle == "CIRCLE") {
      drawProgressiveCircle(fx + fw/2, focalY - fh/2, fw/2 + 4, fh/2 + 3, rawProgress);
    }
    
    // Procedural Semantic Vector Doodles!
    if (sketchDoodle == "ROSE") {
      drawProgressiveRose(fx + fw + 8, focalY - 6, rawProgress);
    } else if (sketchDoodle == "HEART") {
      drawProgressiveHeart(fx + fw + 8, focalY - 8, rawProgress);
    } else if (sketchDoodle == "STAR") {
      drawProgressiveStar(fx - 8, focalY - 10, rawProgress);
      drawProgressiveStar(fx + fw + 8, focalY + 2, rawProgress);
    } else if (sketchDoodle == "FIRE") {
      drawProgressiveFlame(fx + fw + 8, focalY, rawProgress, millis());
    } else if (sketchDoodle == "RAIN") {
      drawProgressiveRain(0, 0, 128, 46, rawProgress, millis());
    } else if (sketchDoodle == "BROKEN") {
      drawProgressiveBroken(fx + fw + 7, focalY - 6, rawProgress);
    } else if (sketchDoodle == "WINGS") {
      drawProgressiveWings(fx + fw + 8, focalY - 6, rawProgress);
    } else if (sketchDoodle == "SUN") {
      drawProgressiveSun(fx + fw + 8, focalY - 6, rawProgress);
    } else if (sketchDoodle == "LIGHTNING") {
      drawProgressiveLightning(fx + fw + 7, focalY - 6, rawProgress);
    } else if (sketchDoodle == "EYE") {
      drawProgressiveEye(fx + fw + 8, focalY - 6, rawProgress);
    } else if (sketchDoodle == "ARROW") {
      drawProgressiveArrow(minX + 2, focalY - 10, fx - 4, focalY - 4, rawProgress);
    }
  }
  
  // 3. Render Suffix (Full ASCII)
  if (sketchSuffix.length() > 0 && sketchComposition != "ISOLATED") {
    display.setFont(&FreeSerifItalic9pt7b);
    int16_t x1, y1; uint16_t sw, sh;
    display.getTextBounds(sketchSuffix, 0, 0, &x1, &y1, &sw, &sh);
    
    int sx = minX + (availWidth - (int)sw) / 2;
    if (sketchComposition == "DIAGONAL") sx = minX + availWidth - (int)sw - 2;
    
    drawProgressiveText(sketchSuffix, max(minX, sx), 42, 2, rawProgress);
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
  else if (data.startsWith("I|")) {
    // Song info display mode: I|TWOLINE or I|MARQUEE
    if (data.indexOf("TWOLINE") > 0) {
      isTwoLineSongInfo = true;
    } else {
      isTwoLineSongInfo = false;
      marqueeX = 128;
    }
  }
  else if (data.startsWith("K|")) {
    // Protocol: K|<metaphor>|<doodle>|<composition>|<focal>|<prefix>|<suffix>|<tilt>|<underline>|<durationMs>
    int p1 = data.indexOf('|', 2);
    int p2 = data.indexOf('|', p1 + 1);
    int p3 = data.indexOf('|', p2 + 1);
    int p4 = data.indexOf('|', p3 + 1);
    int p5 = data.indexOf('|', p4 + 1);
    int p6 = data.indexOf('|', p5 + 1);
    int p7 = data.indexOf('|', p6 + 1);
    int p8 = data.indexOf('|', p7 + 1);
    
    if (p1 > 0 && p2 > 0 && p3 > 0 && p4 > 0 && p5 > 0 && p6 > 0 && p7 > 0 && p8 > 0) {
      sketchMetaphor = data.substring(2, p1);
      sketchDoodle = data.substring(p1 + 1, p2);
      sketchComposition = data.substring(p2 + 1, p3);
      sketchFocalWord = data.substring(p3 + 1, p4);
      sketchPrefix = data.substring(p4 + 1, p5);
      sketchSuffix = data.substring(p5 + 1, p6);
      sketchTilt = data.substring(p6 + 1, p7).toInt();
      sketchUnderline = (data.substring(p7 + 1, p8).toInt() == 1);
      sketchDurationMs = data.substring(p8 + 1).toInt();
      sketchSceneStartMs = millis();
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
            int rFont = random(0, 4);
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
          
          kineticTargetX = random(0, 128 - w1);
          if (kineticTargetX < 0) kineticTargetX = 0;
          kineticTargetY = random(h1 + 5, 45);
          
          kineticAnimType = random(0, 3);
          if (kineticAnimType == 0) { kineticCurrentX = -128; kineticCurrentY = kineticTargetY; }
          else if (kineticAnimType == 1) { kineticCurrentX = 128; kineticCurrentY = kineticTargetY; }
          else { kineticCurrentX = kineticTargetX; kineticCurrentY = kineticTargetY; }
        }
        else if (isGiantMode || isSketchbookMode) {
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
    isSketchbookMode = (data.indexOf("SKETCHBOOK") > 0);
    isKineticV2Mode = (data.indexOf("KINETIC2") > 0);
    isKineticMode = (data.indexOf("KINETIC") > 0 && !isKineticV2Mode && !isSketchbookMode);
    isGiantMode = (data.indexOf("GIANT") > 0);
    isSlidingMode = (data.indexOf("SLIDING") > 0);
    
    if (isKineticV2Mode && !wasV2) {
      initParticles();
    }
  }
  else if (data.startsWith("F|")) {
    String fontCode = data.substring(2);
    fontCode.trim();
    if (fontCode == "HANDWRITTEN") currentFontStyle = FONT_HANDWRITTEN;
    else if (fontCode == "MONO") currentFontStyle = FONT_MONO;
    else currentFontStyle = FONT_SANS;
    
    currentLayout = calculateLayout(currentLyric);
  }
  else if (data.startsWith("W|")) {
    highlightWordIndex = data.substring(2).toInt();
  }
}

void setup() {
  Serial.begin(115200);
  
  Wire.begin(5, 4);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Wire.begin(21, 22);
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
      while(true);
    }
  }
  
  display.setRotation(2); // 180 deg flip for Yellow Status band on bottom
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  
  u8g2_gfx.begin(display);
  
  display.setFont(&FreeSans12pt7b);
  display.setCursor(14, 28);
  display.print("LyricCast");
  
  display.setFont(NULL);
  display.setTextSize(1);
  display.setCursor(20, 52);
  display.print("Ready for Spotify");
  display.display();
  delay(1200);
  
  initParticles();
}

void drawCachedLyric(const LyricLayout& layout, int ySlide) {
  int startX = layout.startX;
  
  if (isGiantMode) {
    if (layout.font == NULL) {
      display.setFont(NULL);
      display.setTextSize(2);
      display.setCursor(startX, 16);
      display.print(currentLyric);
    } else {
      display.setFont(layout.font);
      display.setCursor(startX, layout.startY);
      display.print(currentLyric);
    }
    return;
  }
  
  if (isKineticMode) {
    if (layout.font == NULL) {
       display.setFont(NULL);
       display.setTextSize(1);
       display.setCursor(kineticCurrentX, kineticCurrentY);
       display.print(layout.lines[0]);
    } else {
       display.setFont(layout.font);
       display.setCursor(kineticCurrentX, kineticCurrentY);
       display.print(layout.lines[0]);
    }
    return;
  }
  
  if (layout.isInternational) {
    u8g2_gfx.setFont(layout.u8g2_font);
    for(int i=0; i<layout.lineCount; i++) {
      u8g2_gfx.setCursor(layout.lineStartX[i], layout.startY + (i * layout.lineHeight) - ySlide);
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
      display.setCursor(layout.lineStartX[i], lineY);
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
            
            display.fillRect(layout.lineStartX[i] + highlightX - 2, rectY, highlightW + 4, rectH, SSD1306_INVERSE);
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
  
  if (!isTwoLineSongInfo) {
    if (now - lastMarqueeMs > 20) {
      lastMarqueeMs = now;
      String headerText = songTitle;
      if (songArtist.length() > 0 && songArtist != "Unknown") headerText += " - " + songArtist;
      
      int textWidth = 0;
      if (hasInternationalChars(headerText)) {
          u8g2_gfx.setFont(u8g2_font_unifont_t_japanese1);
          textWidth = u8g2_gfx.getUTF8Width(headerText.c_str());
      } else {
          textWidth = headerText.length() * 6;
      }
      
      // Auto-center when text fits completely (e.g. Disease - Justin Bieber)
      if (textWidth <= 128) {
        marqueeX = (128 - textWidth) / 2;
      } else {
        marqueeX -= 1; 
        if (marqueeX < -(textWidth + 10)) {
          marqueeX = 128;
        }
      }
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

  if (!isIdle) {
    if (isSketchbookMode) {
      drawSketchbookScene();
    } else if (isKineticV2Mode) {
      drawParticles();
    } else if (animating) {
      drawCachedLyric(oldLayout, animOffset);
      drawCachedLyric(currentLayout, animOffset - 64);
    } else {
      drawCachedLyric(currentLayout, 0);
    }
  } else {
    // Centered Idle Music Icon
    display.setFont(&FreeSerifBoldItalic12pt7b);
    display.setCursor(56, 28);
    display.print("~");
    drawProgressiveStar(64, 16, 0.9f);
  }
  
  // Mask overlaps onto Yellow Zone
  display.fillRect(0, 48, 128, 16, SSD1306_BLACK); 

  // =====================================
  // YELLOW ZONE (Physical Bottom, Code y=48-63)
  // =====================================
  
  float currentProg = progressMs + (millis() - lastUpdateMs);
  if (currentProg > durationMs) currentProg = durationMs;
  if (durationMs < 1) durationMs = 1;
  int barWidth = (currentProg / durationMs) * 128;

  if (isTwoLineSongInfo) {
    // 2-Line High-Density Static Layout with ZERO bottom padding!
    display.setFont(NULL);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);
    
    // Line 1: Song Title
    int tWidth = songTitle.length() * 6;
    int tX = max(0, (128 - tWidth) / 2);
    display.setCursor(tX, 48);
    display.print(songTitle);
    
    // Line 2: Artist Name
    String art = (songArtist.length() > 0 && songArtist != "Unknown") ? songArtist : "";
    int aWidth = art.length() * 6;
    int aX = max(0, (128 - aWidth) / 2);
    display.setCursor(aX, 55);
    display.print(art);
    
    // Progress Bar pinned right down to the South edge (Y=62, Height=2, 0px bottom padding!)
    display.drawFastHLine(0, 62, 128, SSD1306_WHITE);
    display.fillRect(0, 62, barWidth, 2, SSD1306_WHITE);
  } else {
    // 1-Line Scrolling Marquee Layout
    String headerText = songTitle;
    if (songArtist.length() > 0 && songArtist != "Unknown") headerText += " - " + songArtist;
    
    if (hasInternationalChars(headerText)) {
      if (isCyrillic(headerText)) {
        u8g2_gfx.setFont(u8g2_font_unifont_t_cyrillic);
      } else {
        u8g2_gfx.setFont(u8g2_font_unifont_t_japanese1);
      }
      u8g2_gfx.setCursor((int)marqueeX, 56);
      u8g2_gfx.print(headerText);
    } else {
      display.setFont(NULL); 
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setTextWrap(false);
      display.setCursor((int)marqueeX, 48); 
      display.print(headerText);
    }

    display.drawRect(0, 58, 128, 4, SSD1306_WHITE);
    display.fillRect(0, 58, barWidth, 4, SSD1306_WHITE);
  }

  display.display();
}
