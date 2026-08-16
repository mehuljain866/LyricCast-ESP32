#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>

// 1. Modern Clean Sans Fonts
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansOblique12pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>

// 2. Handwritten / Aesthetic Script Fonts (Insta & TikTok Cursive Style)
#include <Fonts/FreeSerifBoldItalic18pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifItalic12pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

// 3. Classic Editorial Serif Fonts
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>

// 4. Retro Monospace Fonts
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
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
bool isTwoLineSongInfo = true;

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
// FONT THEMES & FAMILIES
// ==========================================
enum FontStyle {
  FONT_MIX = 0,          // Dynamic Indie Mix (Randomized Pairings)
  FONT_HANDWRITTEN = 1,  // Cursive & Italic Script
  FONT_SANS = 2,         // Modern Clean Sans
  FONT_SERIF = 3,        // Classic Editorial Serif
  FONT_MONO = 4,         // Retro Monospace
  FONT_ARCADE = 5        // Retro Monospace / Sans
};
FontStyle currentFontStyle = FONT_MIX;

const GFXfont* sansFonts[] = {&FreeSansBold18pt7b, &FreeSansBold12pt7b, &FreeSansBold9pt7b, &FreeSans9pt7b, NULL};
int sansHeights[] = {28, 20, 15, 12, 8};

const GFXfont* scriptFonts[] = {&FreeSerifBoldItalic18pt7b, &FreeSerifBoldItalic12pt7b, &FreeSerifBoldItalic9pt7b, &FreeSerifItalic9pt7b, NULL};
int scriptHeights[] = {28, 20, 15, 12, 8};

const GFXfont* serifFonts[] = {&FreeSerifBold18pt7b, &FreeSerifBold12pt7b, &FreeSerifBold9pt7b, &FreeSerif9pt7b, NULL};
int serifHeights[] = {28, 20, 15, 12, 8};

const GFXfont* monoFonts[] = {&FreeMonoBold18pt7b, &FreeMonoBold12pt7b, &FreeMonoBold9pt7b, &FreeMono9pt7b, NULL};
int monoHeights[] = {26, 18, 14, 10, 8};

const GFXfont* arcadeFonts[] = {&FreeMonoBold12pt7b, &FreeMonoBold9pt7b, &FreeMono9pt7b, NULL, NULL};
int arcadeHeights[] = {18, 14, 10, 8, 8};

const GFXfont** getActiveFonts() {
  if (currentFontStyle == FONT_HANDWRITTEN) return scriptFonts;
  if (currentFontStyle == FONT_SERIF) return serifFonts;
  if (currentFontStyle == FONT_MONO) return monoFonts;
  if (currentFontStyle == FONT_ARCADE) return arcadeFonts;
  if (currentFontStyle == FONT_SANS) return sansFonts;
  return scriptFonts;
}

int* getActiveHeights() {
  if (currentFontStyle == FONT_HANDWRITTEN) return scriptHeights;
  if (currentFontStyle == FONT_SERIF) return serifHeights;
  if (currentFontStyle == FONT_MONO) return monoHeights;
  if (currentFontStyle == FONT_ARCADE) return arcadeHeights;
  if (currentFontStyle == FONT_SANS) return sansHeights;
  return scriptHeights;
}

const GFXfont* getFontByChoice(int choice) {
  switch (choice) {
    case 0: return &FreeSerifBoldItalic12pt7b; // Cursive Bold 12
    case 1: return &FreeSansBold12pt7b;       // Sans Bold 12
    case 2: return &FreeSerifBold12pt7b;      // Serif Bold 12
    case 3: return &FreeSerifItalic9pt7b;     // Cursive Italic 9
    case 4: return &FreeSans9pt7b;            // Sans Regular 9
    case 5: return &FreeMonoBold9pt7b;        // Mono Bold 9
    case 6: return &FreeSansOblique9pt7b;     // Sans Oblique 9
    case 7: return &FreeSansBold9pt7b;        // Sans Bold 9
    case 8: return &FreeSerifBold9pt7b;       // Serif Bold 9
    case 9: return &FreeMono9pt7b;            // Mono Regular 9
    case 10: return &FreeSans12pt7b;          // Sans Regular 12
    case 11: return &FreeSerifItalic12pt7b;   // Cursive Italic 12
    case 12: return &FreeSansBold18pt7b;      // Sans Giant 18
    case 13: return &FreeSerifBoldItalic18pt7b; // Cursive Giant 18
    case 14: return &FreeMonoBold12pt7b;      // Mono Bold 12
    case 15: return &FreeSansOblique12pt7b;   // Sans Oblique 12
    default: return NULL;
  }
}

// 100% Full Text Immediate Centering (Always perfectly centered horizontally!)
void drawProgressiveText(String text, int x, int y, int fontChoice, float progress) {
  if (text.length() == 0 || progress <= 0.0f) return;
  const GFXfont* f = getFontByChoice(fontChoice);
  display.setFont(f);
  int drawY = y;
  if (f == NULL || fontChoice == -1) {
    display.setTextSize(1);
    // Baseline conversion: default system font expects top-left cursor (height ~8px)
    drawY = max(0, y - 7);
  }
  display.setCursor(x, drawY);
  display.print(text);
}

void drawProgressiveCoffin(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  // Hexagonal coffin outline
  display.drawLine(cx - 3, cy - 6, cx + 3, cy - 6, SSD1306_WHITE); // Head
  display.drawLine(cx - 3, cy - 6, cx - 5, cy - 2, SSD1306_WHITE); // Left shoulder
  display.drawLine(cx + 3, cy - 6, cx + 5, cy - 2, SSD1306_WHITE); // Right shoulder
  display.drawLine(cx - 5, cy - 2, cx - 3, cy + 6, SSD1306_WHITE); // Left taper
  display.drawLine(cx + 5, cy - 2, cx + 3, cy + 6, SSD1306_WHITE); // Right taper
  display.drawLine(cx - 3, cy + 6, cx + 3, cy + 6, SSD1306_WHITE); // Foot
  // Engraved cross
  if (progress > 0.3f) {
    display.drawLine(cx, cy - 3, cx, cy + 2, SSD1306_WHITE);
    display.drawLine(cx - 2, cy - 1, cx + 2, cy - 1, SSD1306_WHITE);
  }
}

// ==========================================
// CUSTOMIZABLE AMBIENT PARTICLES (6 Styles)
// ==========================================
enum ParticleStyle {
  PARTICLE_SPARKLES = 0,
  PARTICLE_DUST = 1,
  PARTICLE_STARS = 2,
  PARTICLE_BUBBLES = 3,
  PARTICLE_RAIN = 4,
  PARTICLE_OFF = 5
};
ParticleStyle currentParticleStyle = PARTICLE_SPARKLES;

void drawLivingCanvas() {
  if (currentParticleStyle == PARTICLE_OFF) return;
  
  unsigned long t = millis();
  if (currentParticleStyle == PARTICLE_SPARKLES) {
    for (int i = 0; i < 3; i++) {
      int px = (int)((t / (30 + i * 15) + i * 43) % 126);
      int py = (int)(10 + sin(t * 0.003f + i * 2.0f) * 6.0f + i * 11);
      if (py >= 2 && py <= 44) {
        display.drawPixel(px, py, SSD1306_WHITE);
        display.drawPixel(px - 1, py, SSD1306_WHITE);
        display.drawPixel(px + 1, py, SSD1306_WHITE);
        display.drawPixel(px, py - 1, SSD1306_WHITE);
        display.drawPixel(px, py + 1, SSD1306_WHITE);
      }
    }
  } else if (currentParticleStyle == PARTICLE_DUST) {
    for (int i = 0; i < 4; i++) {
      int px = (int)((t / (25 + i * 20) + i * 35) % 126);
      int py = (int)(6 + sin(t * 0.002f + i * 1.5f) * 8.0f + i * 10);
      if (py >= 2 && py <= 44) display.drawPixel(px, py, SSD1306_WHITE);
    }
  } else if (currentParticleStyle == PARTICLE_STARS) {
    for (int i = 0; i < 2; i++) {
      int px = (int)((t / (40 + i * 25) + i * 60) % 124) + 2;
      int py = (int)(12 + sin(t * 0.004f + i * 3.0f) * 6.0f + i * 16);
      if (py >= 4 && py <= 42) {
        display.drawLine(px - 2, py, px + 2, py, SSD1306_WHITE);
        display.drawLine(px, py - 2, px, py + 2, SSD1306_WHITE);
      }
    }
  } else if (currentParticleStyle == PARTICLE_BUBBLES) {
    for (int i = 0; i < 3; i++) {
      int px = (int)((i * 45 + 15) % 120);
      int py = 43 - (int)((t / (18 + i * 5) + i * 14) % 43);
      if (py >= 2 && py <= 43) display.drawCircle(px, py, 1 + (i % 2), SSD1306_WHITE);
    }
  } else if (currentParticleStyle == PARTICLE_RAIN) {
    for (int i = 0; i < 4; i++) {
      int px = (int)((i * 33 + (t / 15)) % 126);
      int py = (int)((t / 10 + i * 12) % 43);
      display.drawLine(px, py, px - 1, py + 2, SSD1306_WHITE);
    }
  }
}

// ==========================================
// 50+ PROCEDURAL VECTOR DOODLE ARSENAL
// ==========================================

void drawProgressiveTelephone(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx - 5, cy - 2, cx + 5, cy - 2, SSD1306_WHITE);
  display.drawLine(cx - 4, cy - 1, cx + 4, cy - 1, SSD1306_WHITE);
  display.drawCircle(cx - 5, cy + 1, 2, SSD1306_WHITE);
  display.drawCircle(cx + 5, cy + 1, 2, SSD1306_WHITE);
  if (progress > 0.4f) {
    display.drawPixel(cx, cy + 2, SSD1306_WHITE);
    display.drawPixel(cx + 1, cy + 3, SSD1306_WHITE);
    display.drawPixel(cx, cy + 4, SSD1306_WHITE);
    display.drawPixel(cx - 1, cy + 5, SSD1306_WHITE);
  }
}

void drawProgressiveDownArrow(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx, cy - 5, cx, cy + 3, SSD1306_WHITE);
  display.drawLine(cx - 3, cy, cx, cy + 3, SSD1306_WHITE);
  display.drawLine(cx + 3, cy, cx, cy + 3, SSD1306_WHITE);
}

void drawProgressiveUpArrow(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx, cy + 4, cx, cy - 4, SSD1306_WHITE);
  display.drawLine(cx - 3, cy - 1, cx, cy - 4, SSD1306_WHITE);
  display.drawLine(cx + 3, cy - 1, cx, cy - 4, SSD1306_WHITE);
}

void drawProgressiveLeftArrow(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx + 4, cy, cx - 4, cy, SSD1306_WHITE);
  display.drawLine(cx - 1, cy - 3, cx - 4, cy, SSD1306_WHITE);
  display.drawLine(cx - 1, cy + 3, cx - 4, cy, SSD1306_WHITE);
}

void drawProgressiveHouse(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 5, cy, 10, 7, SSD1306_WHITE);
  display.drawLine(cx - 7, cy, cx, cy - 6, SSD1306_WHITE);
  display.drawLine(cx + 7, cy, cx, cy - 6, SSD1306_WHITE);
  display.drawLine(cx - 1, cy + 3, cx - 1, cy + 7, SSD1306_WHITE);
  display.drawLine(cx + 1, cy + 3, cx + 1, cy + 7, SSD1306_WHITE);
  display.drawLine(cx + 3, cy - 3, cx + 3, cy - 5, SSD1306_WHITE);
}

void drawProgressiveGun(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 6, cy - 3, 12, 3, SSD1306_WHITE);
  display.drawRect(cx - 6, cy, 4, 6, SSD1306_WHITE);
  display.drawPixel(cx - 1, cy + 1, SSD1306_WHITE);
}

void drawProgressiveEarring(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy - 2, 3, SSD1306_WHITE);
  display.drawLine(cx, cy + 1, cx, cy + 5, SSD1306_WHITE);
  display.drawPixel(cx, cy + 6, SSD1306_WHITE);
}

void drawProgressiveRose(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
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
  if (progress > 0.4f) {
    display.drawLine(cx, cy + 3, cx - 1, cy + 9, SSD1306_WHITE);
    display.drawLine(cx, cy + 5, cx + 3, cy + 4, SSD1306_WHITE);
  }
}

void drawProgressiveHeart(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  float pulse = 1.0f + sin(time * 0.008f) * 0.12f;
  int steps = (int)(min(1.0f, progress * 1.3f) * 16.0f);
  int lastX = cx, lastY = cy;
  for (int i = 0; i <= steps; i++) {
    float t = (i / 16.0f) * 2.0f * PI;
    float x = 16.0f * pow(sin(t), 3);
    float y = -(13.0f * cos(t) - 5.0f * cos(2.0f*t) - 2.0f * cos(3.0f*t) - cos(4.0f*t));
    int px = cx + (int)((x / 16.0f) * 5.5f * pulse);
    int py = cy + (int)((y / 16.0f) * 5.5f * pulse);
    if (i > 0) display.drawLine(lastX, lastY, px, py, SSD1306_WHITE);
    lastX = px; lastY = py;
  }
}

void drawProgressiveNotes(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  int floatY = (int)(sin(time * 0.006f) * 1.5f);
  display.fillCircle(cx - 3, cy + floatY, 2, SSD1306_WHITE);
  display.drawLine(cx - 1, cy + floatY, cx - 1, cy - 5 + floatY, SSD1306_WHITE);
  display.drawLine(cx - 1, cy - 5 + floatY, cx + 1, cy - 3 + floatY, SSD1306_WHITE);
  if (progress > 0.4f) {
    int ny = cy - 2 - floatY;
    display.fillCircle(cx + 4, ny + 3, 2, SSD1306_WHITE);
    display.fillCircle(cx + 8, ny + 1, 2, SSD1306_WHITE);
    display.drawLine(cx + 6, ny + 3, cx + 6, ny - 3, SSD1306_WHITE);
    display.drawLine(cx + 10, ny + 1, cx + 10, ny - 5, SSD1306_WHITE);
    display.drawLine(cx + 6, ny - 3, cx + 10, ny - 5, SSD1306_WHITE);
  }
}

void drawProgressiveFlame(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  int h = (int)(progress * 10.0f);
  int flicker = (time / 80) % 3;
  display.drawLine(cx - 3, cy, cx - 1, cy - h + flicker, SSD1306_WHITE);
  display.drawLine(cx + 3, cy, cx + 1, cy - h + (2 - flicker), SSD1306_WHITE);
  display.drawLine(cx - 1, cy - h + flicker, cx, cy - h - 2 + flicker, SSD1306_WHITE);
  display.drawLine(cx + 1, cy - h + (2 - flicker), cx, cy - h - 2 + flicker, SSD1306_WHITE);
}

void drawProgressiveRain(int x, int y, int w, int h, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  int numDrops = (int)(progress * 8.0f);
  int seed = (time / 100) % 5;
  for (int i = 0; i < numDrops; i++) {
    int rx = x + ((i * 19 + seed * 7) % w);
    int ry = y + ((i * 13 + (int)(time / 20)) % h);
    display.drawLine(rx, ry, rx - 1, ry + 3, SSD1306_WHITE);
  }
}

void drawProgressiveStar(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  int len = (int)(progress * 4.0f);
  display.drawLine(cx - len, cy, cx + len, cy, SSD1306_WHITE);
  display.drawLine(cx, cy - len, cx, cy + len, SSD1306_WHITE);
  if (progress > 0.5f) {
    display.drawPixel(cx - 1, cy - 1, SSD1306_WHITE);
    display.drawPixel(cx + 1, cy + 1, SSD1306_WHITE);
    display.drawPixel(cx - 1, cy + 1, SSD1306_WHITE);
    display.drawPixel(cx + 1, cy - 1, SSD1306_WHITE);
  }
}

void drawProgressiveBroken(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  int len = (int)(progress * 10.0f);
  display.drawLine(cx - len/2, cy - len/2, cx - 2, cy - 1, SSD1306_WHITE);
  display.drawLine(cx - 2, cy - 1, cx + 3, cy + 2, SSD1306_WHITE);
  display.drawLine(cx + 3, cy + 2, cx + len/2, cy + len/2, SSD1306_WHITE);
}

void drawProgressiveWings(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  int flap = (int)(sin(time * 0.012f) * 2.0f);
  int w = (int)(progress * 10.0f);
  display.drawLine(cx - 1, cy, cx - w/2, cy - 3 + flap, SSD1306_WHITE);
  display.drawLine(cx - w/2, cy - 3 + flap, cx - w, cy - 1, SSD1306_WHITE);
  display.drawLine(cx - w, cy - 1, cx - 1, cy, SSD1306_WHITE);
  display.drawLine(cx + 1, cy, cx + w/2, cy - 3 + flap, SSD1306_WHITE);
  display.drawLine(cx + w/2, cy - 3 + flap, cx + w, cy - 1, SSD1306_WHITE);
  display.drawLine(cx + w, cy - 1, cx + 1, cy, SSD1306_WHITE);
}

void drawProgressiveButterfly(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  int flap = (int)(sin(time * 0.016f) * 2.0f);
  display.drawPixel(cx, cy, SSD1306_WHITE);
  display.drawLine(cx - 1, cy, cx - 4 + flap, cy - 3, SSD1306_WHITE);
  display.drawLine(cx - 4 + flap, cy - 3, cx - 3, cy + 2, SSD1306_WHITE);
  display.drawLine(cx - 3, cy + 2, cx - 1, cy + 1, SSD1306_WHITE);
  display.drawLine(cx + 1, cy, cx + 4 - flap, cy - 3, SSD1306_WHITE);
  display.drawLine(cx + 4 - flap, cy - 3, cx + 3, cy + 2, SSD1306_WHITE);
  display.drawLine(cx + 3, cy + 2, cx + 1, cy + 1, SSD1306_WHITE);
}

void drawProgressiveSun(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy, 3, SSD1306_WHITE);
  int rayLen = (int)(progress * 3.0f);
  display.drawLine(cx - 3 - rayLen, cy, cx - 4, cy, SSD1306_WHITE);
  display.drawLine(cx + 4, cy, cx + 3 + rayLen, cy, SSD1306_WHITE);
  display.drawLine(cx, cy - 3 - rayLen, cx, cy - 4, SSD1306_WHITE);
  display.drawLine(cx, cy + 4, cx, cy + 3 + rayLen, SSD1306_WHITE);
}

void drawProgressiveLightning(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx, cy - 5, cx - 3, cy - 1, SSD1306_WHITE);
  display.drawLine(cx - 3, cy - 1, cx + 1, cy - 1, SSD1306_WHITE);
  if (progress > 0.4f) display.drawLine(cx + 1, cy - 1, cx - 2, cy + 5, SSD1306_WHITE);
}

void drawProgressiveEye(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  int w = (int)(progress * 6.0f);
  display.drawLine(cx - w, cy, cx, cy - 3, SSD1306_WHITE);
  display.drawLine(cx, cy - 3, cx + w, cy, SSD1306_WHITE);
  display.drawLine(cx - w, cy, cx, cy + 3, SSD1306_WHITE);
  display.drawLine(cx, cy + 3, cx + w, cy, SSD1306_WHITE);
  if (progress > 0.5f) display.fillCircle(cx, cy, 1, SSD1306_WHITE);
}

void drawProgressiveCrown(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx - 5, cy + 2, cx + 5, cy + 2, SSD1306_WHITE);
  display.drawLine(cx - 5, cy + 2, cx - 5, cy - 3, SSD1306_WHITE);
  display.drawLine(cx + 5, cy + 2, cx + 5, cy - 3, SSD1306_WHITE);
  display.drawLine(cx - 5, cy - 3, cx - 2, cy, SSD1306_WHITE);
  display.drawLine(cx - 2, cy, cx, cy - 4, SSD1306_WHITE);
  display.drawLine(cx, cy - 4, cx + 2, cy, SSD1306_WHITE);
  display.drawLine(cx + 2, cy, cx + 5, cy - 3, SSD1306_WHITE);
}

void drawProgressiveDiamond(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx - 4, cy - 2, cx + 4, cy - 2, SSD1306_WHITE);
  display.drawLine(cx - 4, cy - 2, cx - 2, cy - 4, SSD1306_WHITE);
  display.drawLine(cx + 4, cy - 2, cx + 2, cy - 4, SSD1306_WHITE);
  display.drawLine(cx - 2, cy - 4, cx + 2, cy - 4, SSD1306_WHITE);
  display.drawLine(cx - 4, cy - 2, cx, cy + 4, SSD1306_WHITE);
  display.drawLine(cx + 4, cy - 2, cx, cy + 4, SSD1306_WHITE);
}

void drawProgressiveClock(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy, 4, SSD1306_WHITE);
  int handAngle = (time / 300) % 12;
  float a = handAngle * (2.0f * PI / 12.0f);
  display.drawLine(cx, cy, cx + (int)(sin(a) * 3.0f), cy - (int)(cos(a) * 3.0f), SSD1306_WHITE);
}

void drawProgressiveCar(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 5, cy - 2, 10, 4, SSD1306_WHITE);
  display.drawLine(cx - 3, cy - 2, cx - 1, cy - 4, SSD1306_WHITE);
  display.drawLine(cx - 1, cy - 4, cx + 2, cy - 4, SSD1306_WHITE);
  display.drawLine(cx + 2, cy - 4, cx + 4, cy - 2, SSD1306_WHITE);
  display.drawCircle(cx - 3, cy + 3, 1, SSD1306_WHITE);
  display.drawCircle(cx + 3, cy + 3, 1, SSD1306_WHITE);
}

void drawProgressiveCoffee(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 4, cy - 1, 8, 5, SSD1306_WHITE);
  display.drawLine(cx + 4, cy, cx + 6, cy + 1, SSD1306_WHITE);
  display.drawLine(cx + 6, cy + 1, cx + 6, cy + 3, SSD1306_WHITE);
  display.drawLine(cx + 6, cy + 3, cx + 4, cy + 4, SSD1306_WHITE);
  int s1 = (int)(sin((time + 0) * 0.01f) * 1.2f);
  display.drawLine(cx - 1 + s1, cy - 3, cx - 1 + s1, cy - 5, SSD1306_WHITE);
}

void drawProgressiveGhost(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  int fy = (int)(sin(time * 0.008f) * 1.5f);
  int gy = cy + fy;
  display.drawCircle(cx, gy - 2, 3, SSD1306_WHITE);
  display.drawLine(cx - 3, gy - 2, cx - 3, gy + 3, SSD1306_WHITE);
  display.drawLine(cx + 3, gy - 2, cx + 3, gy + 3, SSD1306_WHITE);
  display.drawLine(cx - 3, gy + 3, cx - 1, gy + 1, SSD1306_WHITE);
  display.drawLine(cx - 1, gy + 1, cx + 1, gy + 3, SSD1306_WHITE);
  display.drawLine(cx + 1, gy + 3, cx + 3, gy + 1, SSD1306_WHITE);
}

void drawProgressiveMoon(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  int r = 4;
  int steps = (int)(progress * 10.0f);
  for (int i = 0; i <= steps; i++) {
    float a = -PI/2 + (i / 10.0f) * PI;
    display.drawPixel(cx + (int)(cos(a) * r), cy + (int)(sin(a) * r), SSD1306_WHITE);
    display.drawPixel(cx + (int)(cos(a) * (r - 1.8f)) + 1, cy + (int)(sin(a) * (r - 1.8f)), SSD1306_WHITE);
  }
}

void drawProgressiveKey(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx - 3, cy, 2, SSD1306_WHITE);
  display.drawLine(cx - 1, cy, cx + 4, cy, SSD1306_WHITE);
  display.drawLine(cx + 2, cy, cx + 2, cy + 2, SSD1306_WHITE);
}

void drawProgressiveSword(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx - 4, cy + 4, cx + 4, cy - 4, SSD1306_WHITE);
  display.drawLine(cx - 2, cy + 1, cx - 1, cy + 2, SSD1306_WHITE);
}

void drawProgressiveBulb(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy - 2, 3, SSD1306_WHITE);
  display.drawLine(cx - 1, cy + 2, cx + 1, cy + 2, SSD1306_WHITE);
}

void drawProgressiveTarget(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy, 4, SSD1306_WHITE);
  display.drawCircle(cx, cy, 2, SSD1306_WHITE);
}

void drawProgressiveAnchor(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy - 3, 1, SSD1306_WHITE);
  display.drawLine(cx - 2, cy + 4, cx - 2, cy + 4, SSD1306_WHITE);
  display.drawLine(cx - 4, cy + 1, cx + 4, cy + 1, SSD1306_WHITE);
  display.drawLine(cx - 4, cy + 1, cx - 4, cy + 3, SSD1306_WHITE);
  display.drawLine(cx + 4, cy + 1, cx + 4, cy + 3, SSD1306_WHITE);
}

void drawProgressiveDice(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 4, cy - 4, 8, 8, SSD1306_WHITE);
  display.drawPixel(cx - 2, cy - 2, SSD1306_WHITE);
  display.drawPixel(cx + 2, cy + 2, SSD1306_WHITE);
  display.drawPixel(cx, cy, SSD1306_WHITE);
}

void drawProgressivePlanet(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy, 3, SSD1306_WHITE);
  display.drawLine(cx - 6, cy + 2, cx + 6, cy - 2, SSD1306_WHITE);
}

void drawProgressiveLeaf(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx - 4, cy + 3, cx + 4, cy - 3, SSD1306_WHITE);
  display.drawLine(cx - 4, cy + 3, cx, cy - 4, SSD1306_WHITE);
  display.drawLine(cx, cy - 4, cx + 4, cy - 3, SSD1306_WHITE);
}

void drawProgressiveCandle(int cx, int cy, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 2, cy - 1, 4, 7, SSD1306_WHITE);
  display.drawLine(cx, cy - 1, cx, cy - 3, SSD1306_WHITE);
  display.drawPixel(cx + ((time/100)%2==0?0:1), cy - 4, SSD1306_WHITE);
}

void drawProgressivePill(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 5, cy - 2, 10, 4, SSD1306_WHITE);
  display.drawLine(cx, cy - 2, cx, cy + 2, SSD1306_WHITE);
}

void drawProgressiveGlasses(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 6, cy - 2, 4, 4, SSD1306_WHITE);
  display.drawRect(cx + 2, cy - 2, 4, 4, SSD1306_WHITE);
  display.drawLine(cx - 2, cy - 1, cx + 2, cy - 1, SSD1306_WHITE);
}

void drawProgressiveSkull(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy - 2, 3, SSD1306_WHITE);
  display.drawRect(cx - 2, cy + 1, 4, 3, SSD1306_WHITE);
  display.drawPixel(cx - 1, cy - 2, SSD1306_BLACK);
  display.drawPixel(cx + 1, cy - 2, SSD1306_BLACK);
}

void drawProgressiveBalloon(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy - 3, 3, SSD1306_WHITE);
  display.drawLine(cx, cy, cx - 1, cy + 4, SSD1306_WHITE);
}

void drawProgressiveBell(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx - 4, cy + 2, cx + 4, cy + 2, SSD1306_WHITE);
  display.drawLine(cx - 4, cy + 2, cx - 2, cy - 3, SSD1306_WHITE);
  display.drawLine(cx + 4, cy + 2, cx + 2, cy - 3, SSD1306_WHITE);
  display.drawLine(cx - 2, cy - 3, cx + 2, cy - 3, SSD1306_WHITE);
  display.drawPixel(cx, cy + 3, SSD1306_WHITE);
}

void drawProgressiveGift(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 4, cy - 2, 8, 7, SSD1306_WHITE);
  display.drawLine(cx, cy - 2, cx, cy + 5, SSD1306_WHITE);
  display.drawLine(cx - 4, cy + 1, cx + 4, cy + 1, SSD1306_WHITE);
  display.drawPixel(cx - 1, cy - 3, SSD1306_WHITE);
  display.drawPixel(cx + 1, cy - 3, SSD1306_WHITE);
}

void drawProgressiveShoe(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx - 5, cy + 2, cx + 5, cy + 2, SSD1306_WHITE);
  display.drawLine(cx - 5, cy + 2, cx - 5, cy - 2, SSD1306_WHITE);
  display.drawLine(cx - 5, cy - 2, cx - 2, cy - 2, SSD1306_WHITE);
  display.drawLine(cx - 2, cy - 2, cx + 2, cy, SSD1306_WHITE);
  display.drawLine(cx + 2, cy, cx + 5, cy + 2, SSD1306_WHITE);
}

void drawProgressiveWine(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx - 3, cy - 4, cx + 3, cy - 4, SSD1306_WHITE);
  display.drawLine(cx - 3, cy - 4, cx, cy, SSD1306_WHITE);
  display.drawLine(cx + 3, cy - 4, cx, cy, SSD1306_WHITE);
  display.drawLine(cx, cy, cx, cy + 4, SSD1306_WHITE);
  display.drawLine(cx - 2, cy + 4, cx + 2, cy + 4, SSD1306_WHITE);
}

void drawProgressiveHeadphones(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx, cy - 2, 4, SSD1306_WHITE);
  display.fillRect(cx - 5, cy - 2, 2, 4, SSD1306_WHITE);
  display.fillRect(cx + 4, cy - 2, 2, 4, SSD1306_WHITE);
  display.fillRect(cx - 4, cy + 1, 8, 3, SSD1306_BLACK);
}

void drawProgressiveBattery(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 5, cy - 3, 10, 6, SSD1306_WHITE);
  display.drawRect(cx + 5, cy - 1, 1, 2, SSD1306_WHITE);
  display.fillRect(cx - 3, cy - 1, 4, 2, SSD1306_WHITE);
}

void drawProgressiveBow(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawLine(cx - 4, cy - 4, cx - 4, cy + 4, SSD1306_WHITE);
  display.drawLine(cx - 4, cy - 4, cx + 2, cy, SSD1306_WHITE);
  display.drawLine(cx - 4, cy + 4, cx + 2, cy, SSD1306_WHITE);
  display.drawLine(cx - 5, cy, cx + 4, cy, SSD1306_WHITE);
}

void drawProgressiveCloud(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx - 2, cy - 1, 3, SSD1306_WHITE);
  display.drawCircle(cx + 3, cy, 2, SSD1306_WHITE);
  display.drawLine(cx - 5, cy + 2, cx + 5, cy + 2, SSD1306_WHITE);
}

void drawProgressiveCherry(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawCircle(cx - 2, cy + 2, 2, SSD1306_WHITE);
  display.drawCircle(cx + 3, cy + 2, 2, SSD1306_WHITE);
  display.drawLine(cx - 2, cy, cx, cy - 4, SSD1306_WHITE);
  display.drawLine(cx + 3, cy, cx, cy - 4, SSD1306_WHITE);
}

void drawProgressivePadlock(int cx, int cy, float progress) {
  if (progress <= 0.0f) return;
  display.drawRect(cx - 3, cy, 7, 5, SSD1306_WHITE);
  display.drawCircle(cx, cy - 1, 2, SSD1306_WHITE);
}

void drawProgressiveBubble(int cx, int cy, int rx, int ry, float progress) {
  if (progress <= 0.0f) return;
  int steps = (int)(min(1.0f, progress * 1.3f) * 16.0f);
  int lastX = cx + rx, lastY = cy;
  for (int i = 0; i <= steps; i++) {
    float a = (i / 16.0f) * 2.0f * PI;
    int px = cx + (int)(cos(a) * rx);
    int py = cy + (int)(sin(a) * ry);
    if (i > 0) display.drawLine(lastX, lastY, px, py, SSD1306_WHITE);
    lastX = px; lastY = py;
  }
}

void drawProgressiveBox(int x1, int y1, int x2, int y2, float progress) {
  if (progress <= 0.0f) return;
  int h = y2 - y1;
  int drawH = (int)(progress * (float)h);
  display.drawLine(x1, y1, x1 + 3, y1, SSD1306_WHITE);
  display.drawLine(x1, y1, x1, y1 + drawH, SSD1306_WHITE);
  display.drawLine(x1, y1 + drawH, x1 + 3, y1 + drawH, SSD1306_WHITE);
  display.drawLine(x2, y1, x2 - 3, y1, SSD1306_WHITE);
  display.drawLine(x2, y1, x2, y1 + drawH, SSD1306_WHITE);
  display.drawLine(x2, y1 + drawH, x2 - 3, y1 + drawH, SSD1306_WHITE);
}

void drawProgressiveWave(int x1, int x2, int y, float progress, unsigned long time) {
  if (progress <= 0.0f) return;
  int w = x2 - x1;
  int targetX = x1 + (int)(progress * (float)w);
  int lastX = x1, lastY = y;
  for (int x = x1; x <= targetX; x += 3) {
    int relX = x - x1;
    int dy = 0;
    if (relX > w/3 && relX < 2*w/3) {
      int phase = (relX - w/3);
      if (phase < 5) dy = -4;
      else if (phase < 10) dy = 4;
      else if (phase < 15) dy = -2;
    }
    int cy = y + dy;
    if (x > x1) display.drawLine(lastX, lastY, x, cy, SSD1306_WHITE);
    lastX = x; lastY = cy;
  }
}

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

void drawCornerFrames(float progress) {
  if (progress <= 0.0f) return;
  int len = (int)(progress * 6.0f);
  display.drawLine(2, 2, 2 + len, 2, SSD1306_WHITE);
  display.drawLine(2, 2, 2, 2 + len, SSD1306_WHITE);
  display.drawLine(125, 2, 125 - len, 2, SSD1306_WHITE);
  display.drawLine(125, 2, 125, 2 + len, SSD1306_WHITE);
  display.drawLine(2, 44, 2 + len, 44, SSD1306_WHITE);
  display.drawLine(2, 44, 2, 44 - len, SSD1306_WHITE);
  display.drawLine(125, 44, 125 - len, 44, SSD1306_WHITE);
  display.drawLine(125, 44, 125, 44 - len, SSD1306_WHITE);
}

void drawDoodle(String doodle, int cx, int cy, float progress, unsigned long now) {
  if (doodle == "PHONE") drawProgressiveTelephone(cx, cy, progress);
  else if (doodle == "DOWN") drawProgressiveDownArrow(cx, cy, progress);
  else if (doodle == "UP") drawProgressiveUpArrow(cx, cy, progress);
  else if (doodle == "LEFT") drawProgressiveLeftArrow(cx, cy, progress);
  else if (doodle == "HOME") drawProgressiveHouse(cx, cy, progress);
  else if (doodle == "GUN") drawProgressiveGun(cx, cy, progress);
  else if (doodle == "EARRING") drawProgressiveEarring(cx, cy, progress);
  else if (doodle == "ROSE") drawProgressiveRose(cx, cy, progress);
  else if (doodle == "HEART") drawProgressiveHeart(cx, cy, progress, now);
  else if (doodle == "NOTE") drawProgressiveNotes(cx, cy, progress, now);
  else if (doodle == "STAR") { drawProgressiveStar(cx - 5, cy - 3, progress); drawProgressiveStar(cx + 5, cy + 3, progress); }
  else if (doodle == "FIRE") drawProgressiveFlame(cx, cy, progress, now);
  else if (doodle == "RAIN") drawProgressiveRain(0, 0, 128, 44, progress, now);
  else if (doodle == "BROKEN") drawProgressiveBroken(cx, cy, progress);
  else if (doodle == "WINGS") drawProgressiveWings(cx, cy, progress, now);
  else if (doodle == "BUTTERFLY") drawProgressiveButterfly(cx, cy, progress, now);
  else if (doodle == "SUN") drawProgressiveSun(cx, cy, progress);
  else if (doodle == "MOON") drawProgressiveMoon(cx, cy, progress);
  else if (doodle == "LIGHTNING") drawProgressiveLightning(cx, cy, progress);
  else if (doodle == "EYE") drawProgressiveEye(cx, cy, progress);
  else if (doodle == "CROWN") drawProgressiveCrown(cx, cy, progress);
  else if (doodle == "DIAMOND") drawProgressiveDiamond(cx, cy, progress);
  else if (doodle == "CLOCK") drawProgressiveClock(cx, cy, progress, now);
  else if (doodle == "CAR") drawProgressiveCar(cx, cy, progress, now);
  else if (doodle == "COFFEE") drawProgressiveCoffee(cx, cy, progress, now);
  else if (doodle == "GHOST") drawProgressiveGhost(cx, cy, progress, now);
  else if (doodle == "KEY") drawProgressiveKey(cx, cy, progress);
  else if (doodle == "SWORD") drawProgressiveSword(cx, cy, progress);
  else if (doodle == "BULB") drawProgressiveBulb(cx, cy, progress);
  else if (doodle == "TARGET") drawProgressiveTarget(cx, cy, progress);
  else if (doodle == "ANCHOR") drawProgressiveAnchor(cx, cy, progress);
  else if (doodle == "DICE") drawProgressiveDice(cx, cy, progress);
  else if (doodle == "PLANET") drawProgressivePlanet(cx, cy, progress);
  else if (doodle == "LEAF") drawProgressiveLeaf(cx, cy, progress);
  else if (doodle == "CANDLE") drawProgressiveCandle(cx, cy, progress, now);
  else if (doodle == "PILL") drawProgressivePill(cx, cy, progress);
  else if (doodle == "GLASSES") drawProgressiveGlasses(cx, cy, progress);
  else if (doodle == "SKULL") drawProgressiveSkull(cx, cy, progress);
  else if (doodle == "BALLOON") drawProgressiveBalloon(cx, cy, progress);
  else if (doodle == "BELL") drawProgressiveBell(cx, cy, progress);
  else if (doodle == "GIFT") drawProgressiveGift(cx, cy, progress);
  else if (doodle == "SHOE") drawProgressiveShoe(cx, cy, progress);
  else if (doodle == "WINE") drawProgressiveWine(cx, cy, progress);
  else if (doodle == "HEADPHONES") drawProgressiveHeadphones(cx, cy, progress);
  else if (doodle == "BATTERY") drawProgressiveBattery(cx, cy, progress);
  else if (doodle == "BOW") drawProgressiveBow(cx, cy, progress);
  else if (doodle == "CLOUD") drawProgressiveCloud(cx, cy, progress);
  else if (doodle == "CHERRY") drawProgressiveCherry(cx, cy, progress);
  else if (doodle == "PADLOCK") drawProgressivePadlock(cx, cy, progress);
  else if (doodle == "COFFIN") drawProgressiveCoffin(cx, cy, progress);
  else if (doodle == "ARROW") drawProgressiveArrow(4, cy - 4, cx - 2, cy - 2, progress);
}

// ==========================================
// SCENE GRAPH & KINETIC TRANSITIONS
// ==========================================
bool isSketchbookMode = true;

struct SketchScene {
  String metaphor = "NORMAL";
  String doodle = "NONE";
  String composition = "CENTER";
  String focalWord = "";
  String prefix = "";
  String suffix = "";
  int tilt = 0;
  bool underline = false;
  uint32_t durationMs = 2500;
  int fontPreset = 0;
  int fxFlags = 0;
};

SketchScene currentSketch;
SketchScene oldSketch;

unsigned long sketchSceneStartMs = 0;
unsigned long sketchTransitionStartMs = 0;
int sketchTransitionType = 0; // 0=SlideUp, 1=SlideDown, 2=PopZoom, 3=SmoothDrop

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
// RENDER SINGLE SKETCH SCENE (Strict 128x48 Pixel Geometry with Offset)
// ==========================================
void drawSingleSketchScene(const SketchScene& s, int yOffset, float progress, unsigned long now) {
  if (s.focalWord.length() == 0 && s.prefix.length() == 0 && s.suffix.length() == 0) return;

  int minX = 2;
  int availWidth = 124;

  bool hasPrefix = (s.prefix.length() > 0);
  bool hasFocal = (s.focalWord.length() > 0);
  bool hasSuffix = (s.suffix.length() > 0);
  String comp = s.composition;
  if (comp == "MONOLITH" && (hasPrefix || hasSuffix)) comp = "CENTER";

  // Strict 128x48 Blue Zone Vertical Alignment (Ample spacing, zero overlap)
  int prefixY = 10 + yOffset;
  int focalY = 25 + yOffset;
  int suffixY = 41 + yOffset;

  if (hasPrefix && hasFocal && hasSuffix) {
    prefixY = 10 + yOffset;
    focalY = 25 + yOffset;
    suffixY = 41 + yOffset;
  } else if (hasPrefix && hasFocal && !hasSuffix) {
    prefixY = 12 + yOffset;
    focalY = 36 + yOffset;
  } else if (!hasPrefix && hasFocal && hasSuffix) {
    focalY = 15 + yOffset;
    suffixY = 39 + yOffset;
  } else if (!hasPrefix && hasFocal && !hasSuffix) {
    focalY = 28 + yOffset; // Perfectly centered vertically!
  }

  float breathe = sin(now * 0.006f) * 1.0f;
  if (s.metaphor == "FALLING") {
    focalY += (int)(-5 + easeOutBounce(min(1.0f, progress * 3.5f)) * 5.0f);
  } else if (s.metaphor == "FLYING") {
    focalY += (int)(5 - easeOutQuad(min(1.0f, progress * 3.5f)) * 5.0f);
  } else {
    focalY += (int)breathe;
  }

  if (s.fxFlags & 2) {
    drawCornerFrames(progress);
  }

  if (s.fxFlags & 4) {
    int barH = (int)(min(1.0f, progress * 2.0f) * 34.0f);
    display.fillRect(2, 4 + yOffset, 2, barH, SSD1306_WHITE);
    minX = 7;
    availWidth = 119;
  }

  // Font Preset Selection
  int prefixFont = 3, focalFont = 0, suffixFont = 3;
  if (currentFontStyle == FONT_SANS) {
    prefixFont = 4; focalFont = 1; suffixFont = 4;
  } else if (currentFontStyle == FONT_SERIF) {
    prefixFont = 3; focalFont = 2; suffixFont = 8;
  } else if (currentFontStyle == FONT_MONO) {
    prefixFont = 9; focalFont = 14; suffixFont = 5;
  } else if (currentFontStyle == FONT_ARCADE) {
    prefixFont = 5; focalFont = 14; suffixFont = 7;
  } else if (currentFontStyle == FONT_HANDWRITTEN) {
    prefixFont = 3; focalFont = 0; suffixFont = 11;
  } else if (currentFontStyle == FONT_MIX) {
    switch (s.fontPreset % 8) {
      case 0: prefixFont = 3; focalFont = 0; suffixFont = 3; break;
      case 1: prefixFont = 6; focalFont = 1; suffixFont = 4; break;
      case 2: prefixFont = 4; focalFont = 2; suffixFont = 3; break;
      case 3: prefixFont = 9; focalFont = 14; suffixFont = 5; break;
      case 4: prefixFont = 7; focalFont = 11; suffixFont = 6; break;
      case 5: prefixFont = 8; focalFont = 10; suffixFont = 8; break;
      case 6: prefixFont = 3; focalFont = 7; suffixFont = 5; break;
      case 7: prefixFont = 5; focalFont = 0; suffixFont = 4; break;
    }
  }

  // 1. MONOLITH COMPOSITION (Giant single word)
  if (comp == "MONOLITH") {
    display.setFont(&FreeSansBold18pt7b);
    int16_t x1, y1; uint16_t fw, fh;
    display.getTextBounds(s.focalWord, 0, 0, &x1, &y1, &fw, &fh);
    if (fw > availWidth) {
      focalFont = 1;
      display.setFont(&FreeSansBold12pt7b);
      display.getTextBounds(s.focalWord, 0, 0, &x1, &y1, &fw, &fh);
      if (fw > availWidth) {
        focalFont = 7;
        display.setFont(&FreeSansBold9pt7b);
        display.getTextBounds(s.focalWord, 0, 0, &x1, &y1, &fw, &fh);
      }
    }
    int fx = minX + (availWidth - (int)fw) / 2;
    if (fx + fw > 126) fx = max(minX, 126 - (int)fw);
    if (fx < minX) fx = minX;
    int fy = 29 + yOffset + (int)breathe;
    if (fy >= -10 && fy <= 55) {
      drawProgressiveText(s.focalWord, fx, fy, focalFont, progress);
      if (s.underline) drawProgressiveUnderline(fx - 2, fx + fw + 2, fy + 3, progress);
      if (s.doodle != "NONE" && fx + fw + 14 <= 126) {
        drawDoodle(s.doodle, min(120, fx + fw + 8), fy - 6, progress, now);
      }
    }
    return;
  }

  // 2. Draw Prefix
  if (hasPrefix && prefixY >= -10 && prefixY <= 55) {
    const GFXfont* pFont = getFontByChoice(prefixFont);
    display.setFont(pFont);
    int16_t x1, y1; uint16_t pw, ph;
    display.getTextBounds(s.prefix, 0, 0, &x1, &y1, &pw, &ph);
    if (pw > availWidth) {
      prefixFont = 4;
      pFont = getFontByChoice(prefixFont);
      display.setFont(pFont);
      display.getTextBounds(s.prefix, 0, 0, &x1, &y1, &pw, &ph);
      if (pw > availWidth) {
        prefixFont = -1;
        display.setFont(NULL);
        pw = s.prefix.length() * 6;
      }
    }
    int px = (comp == "STACKED") ? minX + 2 : minX + (availWidth - (int)pw) / 2;
    if (px + pw > 126) px = max(minX, 126 - (int)pw);
    if (px < minX) px = minX;
    drawProgressiveText(s.prefix, px, prefixY, prefixFont, progress);
  }

  // 3. Draw Focal Word
  if (hasFocal && focalY >= -10 && focalY <= 55) {
    const GFXfont* fFont = getFontByChoice(focalFont);
    display.setFont(fFont);
    int16_t x1, y1; uint16_t fw, fh;
    display.getTextBounds(s.focalWord, 0, 0, &x1, &y1, &fw, &fh);
    if (fw > availWidth) {
      focalFont = (focalFont == 1 || focalFont == 10) ? 7 : 3;
      fFont = getFontByChoice(focalFont);
      display.setFont(fFont);
      display.getTextBounds(s.focalWord, 0, 0, &x1, &y1, &fw, &fh);
      if (fw > availWidth) {
        focalFont = -1;
        display.setFont(NULL);
        fw = s.focalWord.length() * 6;
      }
    }
    int fx = (comp == "STACKED") ? minX + 2 : minX + (availWidth - (int)fw) / 2;
    if (fx + fw > 126) fx = max(minX, 126 - (int)fw);
    if (fx < minX) fx = minX;

    if (s.fxFlags & 1) {
      int16_t bx, by; uint16_t bw, bh;
      if (focalFont >= 0) display.getTextBounds(s.focalWord, fx, focalY, &bx, &by, &bw, &bh);
      else { bx = fx; by = focalY - 8; bw = fw; bh = 9; }
      int rx = max(0, bx - 3);
      int ry = max(0, by - 1);
      int rw = min(128 - rx, bw + 6);
      int rh = min(47 - ry, bh + 3);
      if (hasPrefix && ry < prefixY + 2) ry = prefixY + 2;
      if (hasSuffix && ry + rh > suffixY - 8) rh = max(6, (suffixY - 8) - ry);
      if (ry < 48 && ry + rh > 0) {
        display.fillRect(rx, ry, rw, rh, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        drawProgressiveText(s.focalWord, fx, focalY, focalFont, progress);
        display.setTextColor(SSD1306_WHITE);
      }
    } else {
      drawProgressiveText(s.focalWord, fx, focalY, focalFont, progress);
    }

    if (s.underline && !(s.fxFlags & 1)) {
      int ux1 = max(minX, fx - 2);
      int ux2 = min(126, fx + (int)fw + 2);
      drawProgressiveUnderline(ux1, ux2, min(45, focalY + 3), progress);
    }

    if (s.doodle == "CIRCLE") {
      drawProgressiveCircle(fx + fw/2, focalY - fh/2, min(24, (int)fw/2 + 4), min(12, (int)fh/2 + 3), progress);
    } else if (s.doodle == "BOX") {
      int bx1 = max(minX, fx - 4), bx2 = min(126, fx + (int)fw + 4);
      drawProgressiveBox(bx1, max(1, focalY - (int)fh - 1), bx2, min(45, focalY + 3), progress);
    } else if (s.doodle == "BUBBLE") {
      drawProgressiveBubble(fx + fw/2, focalY - fh/2, min(26, (int)fw/2 + 6), min(14, (int)fh/2 + 4), progress);
    } else if (s.doodle == "WAVE") {
      drawProgressiveWave(minX, minX + availWidth, 44 + yOffset, progress, now);
    } else if (s.doodle != "NONE" && s.doodle != "UNDERLINE") {
      int dx = fx + fw + 6;
      int dy = focalY - 5;
      if (dx + 12 > 126) dx = max(4, fx - 14);
      drawDoodle(s.doodle, dx, dy, progress, now);
    }
  }

  // 4. Draw Suffix
  if (hasSuffix && suffixY >= -10 && suffixY <= 55) {
    const GFXfont* sFont = getFontByChoice(suffixFont);
    display.setFont(sFont);
    int16_t x1, y1; uint16_t sw, sh;
    display.getTextBounds(s.suffix, 0, 0, &x1, &y1, &sw, &sh);
    if (sw > availWidth) {
      suffixFont = 4;
      sFont = getFontByChoice(suffixFont);
      display.setFont(sFont);
      display.getTextBounds(s.suffix, 0, 0, &x1, &y1, &sw, &sh);
      if (sw > availWidth) {
        suffixFont = -1;
        display.setFont(NULL);
        sw = s.suffix.length() * 6;
      }
    }
    int sx = (comp == "STACKED") ? minX + 2 : minX + (availWidth - (int)sw) / 2;
    if (sx + sw > 126) sx = max(minX, 126 - (int)sw);
    if (sx < minX) sx = minX;
    drawProgressiveText(s.suffix, sx, suffixY, suffixFont, progress);
  }
}

// ==========================================
// ANIMATED SKETCHBOOK SCENE ENGINE (Smooth Multi-Style Line Transitions)
// ==========================================
void drawSketchbookScene() {
  unsigned long now = millis();
  unsigned long elapsed = now - sketchSceneStartMs;
  float rawProgress = (currentSketch.durationMs > 0) ? ((float)elapsed / currentSketch.durationMs) : 1.0f;
  if (rawProgress > 1.0f) rawProgress = 1.0f;

  drawLivingCanvas();

  unsigned long transElapsed = now - sketchTransitionStartMs;
  const unsigned long TRANS_DURATION = 320; // 320ms smooth kinetic transition

  if (transElapsed < TRANS_DURATION && oldSketch.focalWord.length() > 0) {
    float transT = (float)transElapsed / (float)TRANS_DURATION;

    if (sketchTransitionType == 0) {
      // 1. Kinetic Slide Up Transition (Smooth page wipe up)
      float easeT = easeInOutQuad(transT);
      int yOut = (int)(-48.0f * easeT);
      int yIn = (int)(48.0f * (1.0f - easeT));
      drawSingleSketchScene(oldSketch, yOut, 1.0f, now);
      drawSingleSketchScene(currentSketch, yIn, rawProgress, now);
    } else if (sketchTransitionType == 1) {
      // 2. Kinetic Slide Down Transition (Smooth page wipe down)
      float easeT = easeInOutQuad(transT);
      int yOut = (int)(48.0f * easeT);
      int yIn = (int)(-48.0f * (1.0f - easeT));
      drawSingleSketchScene(oldSketch, yOut, 1.0f, now);
      drawSingleSketchScene(currentSketch, yIn, rawProgress, now);
    } else if (sketchTransitionType == 2) {
      // 3. Elastic Pop Up Transition
      float easeT = easeOutBack(transT);
      int yIn = (int)(20.0f * (1.0f - min(1.0f, easeT)));
      drawSingleSketchScene(currentSketch, yIn, rawProgress, now);
    } else {
      // 4. Smooth Fade Drop Transition
      float easeT = easeOutQuad(transT);
      int yIn = (int)(-18.0f * (1.0f - easeT));
      drawSingleSketchScene(currentSketch, yIn, rawProgress, now);
    }
  } else {
    // Resting active scene
    drawSingleSketchScene(currentSketch, 0, rawProgress, now);
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
        oldSketch.focalWord = "";
        oldSketch.prefix = "";
        oldSketch.suffix = "";
      }
      progressMs = data.substring(split2 + 1, split3).toFloat();
      durationMs = data.substring(split3 + 1).toFloat();
      lastUpdateMs = millis();
    }
  } 
  else if (data.startsWith("I|")) {
    if (data.indexOf("TWOLINE") > 0) {
      isTwoLineSongInfo = true;
    } else {
      isTwoLineSongInfo = false;
      marqueeX = 128;
    }
  }
  else if (data.startsWith("P|")) {
    String pCode = data.substring(2);
    pCode.trim();
    if (pCode == "DUST") currentParticleStyle = PARTICLE_DUST;
    else if (pCode == "STARS") currentParticleStyle = PARTICLE_STARS;
    else if (pCode == "BUBBLES") currentParticleStyle = PARTICLE_BUBBLES;
    else if (pCode == "RAIN") currentParticleStyle = PARTICLE_RAIN;
    else if (pCode == "OFF") currentParticleStyle = PARTICLE_OFF;
    else currentParticleStyle = PARTICLE_SPARKLES;
  }
  else if (data.startsWith("K|")) {
    int p1 = data.indexOf('|', 2);
    int p2 = data.indexOf('|', p1 + 1);
    int p3 = data.indexOf('|', p2 + 1);
    int p4 = data.indexOf('|', p3 + 1);
    int p5 = data.indexOf('|', p4 + 1);
    int p6 = data.indexOf('|', p5 + 1);
    int p7 = data.indexOf('|', p6 + 1);
    int p8 = data.indexOf('|', p7 + 1);
    int p9 = data.indexOf('|', p8 + 1);
    int p10 = (p9 > 0) ? data.indexOf('|', p9 + 1) : -1;
    
    if (p1 > 0 && p2 > 0 && p3 > 0 && p4 > 0 && p5 > 0 && p6 > 0 && p7 > 0 && p8 > 0) {
      SketchScene newScene;
      newScene.metaphor = data.substring(2, p1);
      newScene.doodle = data.substring(p1 + 1, p2);
      newScene.composition = data.substring(p2 + 1, p3);
      newScene.focalWord = data.substring(p3 + 1, p4);
      newScene.prefix = data.substring(p4 + 1, p5);
      newScene.suffix = data.substring(p5 + 1, p6);
      newScene.tilt = data.substring(p6 + 1, p7).toInt();
      newScene.underline = (data.substring(p7 + 1, p8).toInt() == 1);
      
      if (p9 > 0) {
        newScene.durationMs = data.substring(p8 + 1, p9).toInt();
        if (p10 > 0) {
          newScene.fontPreset = data.substring(p9 + 1, p10).toInt();
          newScene.fxFlags = data.substring(p10 + 1).toInt();
        } else {
          newScene.fontPreset = data.substring(p9 + 1).toInt();
          newScene.fxFlags = 0;
        }
      } else {
        newScene.durationMs = data.substring(p8 + 1).toInt();
        newScene.fontPreset = 0;
        newScene.fxFlags = 0;
      }
      
      if (newScene.focalWord != currentSketch.focalWord || newScene.prefix != currentSketch.prefix || newScene.suffix != currentSketch.suffix) {
        oldSketch = currentSketch;
        currentSketch = newScene;
        sketchSceneStartMs = millis();
        sketchTransitionStartMs = millis();
        sketchTransitionType = random(0, 4); // Randomized kinetic transitions!
      }
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
    if (fontCode == "MIX" || fontCode == "RANDOM") currentFontStyle = FONT_MIX;
    else if (fontCode == "HANDWRITTEN") currentFontStyle = FONT_HANDWRITTEN;
    else if (fontCode == "SERIF") currentFontStyle = FONT_SERIF;
    else if (fontCode == "MONO") currentFontStyle = FONT_MONO;
    else if (fontCode == "ARCADE") currentFontStyle = FONT_ARCADE;
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
    drawProgressiveNotes(64, 20, 0.9f, now);
    drawProgressiveStar(36, 16, 0.8f);
    drawProgressiveStar(92, 16, 0.8f);
  }
  
  // Mask overlaps onto Yellow Zone (row 48..63)
  display.fillRect(0, 48, 128, 16, SSD1306_BLACK); 

  // =====================================
  // YELLOW ZONE (Physical Bottom, Code y=48-63)
  // =====================================
  
  float currentProg = progressMs + (millis() - lastUpdateMs);
  if (currentProg > durationMs) currentProg = durationMs;
  if (durationMs < 1) durationMs = 1;
  int barWidth = (currentProg / durationMs) * 128;

  if (isTwoLineSongInfo) {
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
    
    // Progress Bar pinned right down to South edge (Y=62, Height=2, 0px bottom padding!)
    display.drawFastHLine(0, 62, 128, SSD1306_WHITE);
    display.fillRect(0, 62, barWidth, 2, SSD1306_WHITE);
  } else {
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
