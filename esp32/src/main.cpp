#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>

#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

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

// Dynamic Font Caching Structures
struct LyricLayout {
  bool isInternational;
  const GFXfont* font;         // For English
  const uint8_t* u8g2_font;    // For International
  int lineCount;
  String lines[6];
  int startY;
  int lineHeight;
};

LyricLayout currentLayout;
LyricLayout oldLayout;

const GFXfont* availableFonts[] = {&FreeSans24pt7b, &FreeSans18pt7b, &FreeSans12pt7b, &FreeSans9pt7b, NULL};
int fontHeights[] = {36, 26, 18, 14, 8};

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
  
  if (text.length() == 0) {
    layout.isInternational = false;
    layout.font = &FreeSans9pt7b;
    return layout;
  }

  if (layout.isInternational) {
    // Determine which universal font to use
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
    
    // Simple word wrapping for U8G2 using UTF-8 bounds
    for(int i = 0; i <= text.length(); i++) {
      if(i == text.length() || text.charAt(i) == ' ') {
        String word = text.substring(start, i);
        start = i + 1;
        if(word.length() == 0) continue;
        
        String testLine = currentLine + (currentLine.length()>0 ? " " : "") + word;
        int w = u8g2_gfx.getUTF8Width(testLine.c_str());
        
        if (w > 128 && currentLine.length() > 0) {
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

  // --- ENGLISH DYNAMIC SCALING (Same as before) ---
  for (int f = 0; f < 5; f++) {
    bool isSmallestFallback = (f == 4);
    const GFXfont* testFont = availableFonts[f];
    int testLineHeight = fontHeights[f];
    
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
        if (w > 128 && !isSmallestFallback) { wordTooWide = true; break; } 
        
        String testLine = currentLine + (currentLine.length()>0 ? " " : "") + word;
        display.getTextBounds(testLine, 0, 0, &x1, &y1, &w, &h);
        
        if (w > 128 && currentLine.length() > 0) {
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
      layout.lineHeight = testLineHeight;
      layout.lineCount = lineCount;
      for (int k=0; k<lineCount; k++) layout.lines[k] = lines[k];
      
      if (testFont == NULL) {
        // Default font draws downwards from top-left
        layout.startY = 24 - (totalHeight / 2); 
      } else {
        // FreeSans fonts draw upwards from baseline
        layout.startY = 24 - (totalHeight / 2) + testLineHeight - 4; 
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
  float vy;       // upward velocity (negative = move up)
  float vx;       // horizontal drift
  int fontIndex;  // 0=24pt,1=18pt,2=12pt,3=9pt,4=null
  int animStyle;  // 0=float, 1=drift-right, 2=fadepop, 3=bounce
  int age;        // frames alive
  int popAge;     // for fadepop: frames since spawn
  float bounceOffset; // for bounce
  bool alive;
};

Particle particles[MAX_PARTICLES];
int nextParticleSlot = 0;
int spawnDiagX = 0;  // tracks diagonal x offset for staircase pattern
int spawnDiagY = 44; // starts at bottom of blue zone

void initParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    particles[i].alive = false;
    particles[i].word = "";
  }
  spawnDiagX = 0;
  spawnDiagY = 44;
}

void spawnParticle(String word) {
  // Measure word width to prevent off-screen spawn
  int fIdx = random(1, 5); // 1=18pt, 2=12pt, 3=9pt, 4=null
  const GFXfont* font = availableFonts[fIdx];
  int fHeight = fontHeights[fIdx];

  int16_t x1, y1; uint16_t ww, hh;
  display.setFont(font);
  if (font == NULL) display.setTextSize(1);
  display.getTextBounds(word, 0, 0, &x1, &y1, &ww, &hh);
  int wordWidth = (int)ww;

  // Staircase diagonal spawn position
  int sx = spawnDiagX;
  int sy = spawnDiagY;

  // Clamp x so word doesn't overflow right
  if (sx + wordWidth > 124) sx = max(0, 124 - wordWidth);

  // Advance diagonal for next word
  spawnDiagX += 14;
  spawnDiagY -= 5;
  if (spawnDiagX > 80) { spawnDiagX = 0; spawnDiagY = 44; } // wrap

  int style = random(0, 4);

  // Find a free slot, killing oldest if full
  int slot = -1;
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].alive) { slot = i; break; }
  }
  if (slot < 0) {
    // Kill the oldest (next round-robin slot)
    slot = nextParticleSlot;
    nextParticleSlot = (nextParticleSlot + 1) % MAX_PARTICLES;
  }

  particles[slot].word = word;
  particles[slot].fontIndex = fIdx;
  particles[slot].animStyle = style;
  particles[slot].age = 0;
  particles[slot].popAge = 0;
  particles[slot].bounceOffset = 0;
  particles[slot].alive = true;

  if (style == 1) { // drift right: spawn from left edge
    particles[slot].x = -wordWidth;
    particles[slot].vx = 1.5;
  } else {
    particles[slot].x = sx;
    particles[slot].vx = (style == 3) ? 0.3 : 0.0;
  }
  particles[slot].y = sy;
  particles[slot].vy = -(0.4 + random(0, 4) * 0.1); // float upward at 0.4-0.7px/frame
}

void updateParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].alive) continue;
    Particle& p = particles[i];
    p.age++;
    p.y += p.vy;
    p.x += p.vx;
    if (p.animStyle == 3) { // bounce
      p.bounceOffset = sin(p.age * 0.25) * 3.0;
    }
    // Kill if floated off top, or too far right
    if (p.y < -fontHeights[p.fontIndex]) p.alive = false;
    if (p.x > 132) p.alive = false;
  }
}

void drawParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].alive) continue;
    Particle& p = particles[i];
    const GFXfont* font = availableFonts[p.fontIndex];
    int drawY = (int)(p.y + p.bounceOffset);

    if (drawY > 47 || drawY < -fontHeights[p.fontIndex]) continue; // clamp to blue zone

    display.setFont(font);
    if (font == NULL) display.setTextSize(1);

    if (p.animStyle == 2 && p.age < 4) {
      // fadepop: draw smaller first then snap to full size
      display.setFont(availableFonts[min(p.fontIndex + (3 - p.age), 4)]);
      if (availableFonts[min(p.fontIndex + (3 - p.age), 4)] == NULL) display.setTextSize(1);
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
          // In V2, each incoming word spawns a particle
          animating = false;
          spawnParticle(newCur);
        }
        else if (isKineticMode) {
          animating = false;
          
          if (!currentLayout.isInternational) {
            int rFont = random(1, 4); // 1=18pt, 2=12pt, 3=9pt
            currentLayout.font = availableFonts[rFont];
            currentLayout.lineHeight = fontHeights[rFont];
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
          
          kineticTargetY = random(h1 + 5, 45); // safely in top 45 pixels
          
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
    if (isKineticV2Mode && !wasV2) initParticles(); // reset particles on mode switch
  }
  else if (data.startsWith("W|")) {
    highlightWordIndex = data.substring(2).toInt();
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
  
  // Link U8G2 to Adafruit GFX
  u8g2_gfx.begin(display);
  u8g2_gfx.setFontMode(1);                 // Transparent background
  u8g2_gfx.setFontDirection(0);            // Left to right
  u8g2_gfx.setForegroundColor(SSD1306_WHITE);
  
  display.display();
  
  currentLayout = calculateLayout("...");
}

void drawCachedLyric(LyricLayout &layout, int ySlide) {
  if (layout.lineCount == 0) return;
  
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
      u8g2_gfx.setCursor(0, layout.startY + (i * layout.lineHeight) - ySlide);
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
      display.setCursor(0, lineY);
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
            // Find X offset by measuring text before this word
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
            
            // Draw Inverse Rectangle with fixed height to prevent jitter
            int rectY = (layout.isInternational) ? lineY - 14 : ((layout.font == NULL) ? lineY - 1 : lineY - layout.lineHeight + 4);
            int rectH = (layout.isInternational) ? 16 : ((layout.font == NULL) ? 9 : layout.lineHeight - 2);
            
            display.fillRect(highlightX - 2, rectY, highlightW + 4, rectH, SSD1306_INVERSE);
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
  
  if (now - lastMarqueeMs > 20) {
    lastMarqueeMs = now;
    String headerText = songTitle + " - " + songArtist;
    int textWidth = 0;
    
    // Check if title is international to use U8G2 width
    if (hasInternationalChars(headerText)) {
        u8g2_gfx.setFont(u8g2_font_unifont_t_japanese1);
        textWidth = u8g2_gfx.getUTF8Width(headerText.c_str());
    } else {
        textWidth = headerText.length() * 6; // Rough estimate for default font
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
  
  if (isKineticV2Mode) {
    drawParticles();
  } else if (animating) {
    drawCachedLyric(oldLayout, animOffset);
    drawCachedLyric(currentLayout, animOffset - 64);
  } else {
    drawCachedLyric(currentLayout, 0);
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
    u8g2_gfx.setCursor((int)marqueeX, 60); // Baseline Y for unifont
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
