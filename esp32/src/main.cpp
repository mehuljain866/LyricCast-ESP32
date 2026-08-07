#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_ADDR    0x3C
#define OLED_RESET   -1

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

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
  display.display();
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
        currentLyric = newCur;
        animating = true;
        animOffset = 0;
      }
    }
  }
}

void drawLyric(String text, int ySlide) {
  if (text.length() == 0) return;
  
  bool large = text.length() < 16;
  bool useStandard = text.length() > 60; 
  
  if (useStandard) {
    display.setFont(NULL);
    display.setTextSize(1);
    display.setTextWrap(true);
    // Center of Blue Zone (0-47) is ~24.
    // For standard font (y is top left), let's just start at 16
    display.setCursor(0, 16 - ySlide); 
    display.print(text);
    return;
  }
  
  if (large) display.setFont(&FreeSans12pt7b);
  else display.setFont(&FreeSans9pt7b);
  
  int lineHeight = large ? 18 : 14;
  
  String lines[5];
  int lineCount = 0;
  String currentLine = "";
  
  int start = 0;
  for(int i=0; i<=text.length(); i++) {
    if(i == text.length() || text.charAt(i) == ' ') {
      String word = text.substring(start, i);
      start = i + 1;
      
      String testLine = currentLine + (currentLine.length()>0 ? " " : "") + word;
      int16_t x1, y1; uint16_t w, h;
      display.getTextBounds(testLine, 0, 0, &x1, &y1, &w, &h);
      
      if (w > 128 && currentLine.length() > 0) {
        lines[lineCount++] = currentLine;
        currentLine = word;
      } else {
        currentLine = testLine;
      }
    }
  }
  if (currentLine.length() > 0) {
    lines[lineCount++] = currentLine;
  }
  
  // Center of Blue Zone (0-47) is y=24
  int totalHeight = lineCount * lineHeight;
  int startY = 24 - (totalHeight / 2) + lineHeight - 4; 
  
  for(int i=0; i<lineCount; i++) {
    display.setCursor(0, startY + (i * lineHeight) - ySlide);
    display.print(lines[i]);
  }
}

void loop() {
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    parseSerialData(line);
  }
  
  unsigned long now = millis();
  
  // Marquee
  if (now - lastMarqueeMs > 20) {
    lastMarqueeMs = now;
    String headerText = songTitle + " - " + songArtist;
    int textWidth = headerText.length() * 6;
    if (textWidth > 128) {
      marqueeX -= 1; 
      if (marqueeX < -(textWidth + 10)) {
        marqueeX = 128;
      }
    } else {
        marqueeX = (128 - textWidth) / 2;
    }
  }

  // Animation
  if (animating) {
    if (now - lastFrameMs > 15) { 
      lastFrameMs = now;
      animOffset += 3; 
      if (animOffset >= 24) {
        animating = false;
        animOffset = 0;
      }
    }
  }

  display.clearDisplay();
  
  // =====================================
  // BLUE ZONE (Physical Top, Code y=0-47)
  // =====================================
  // ONLY Lyrics now!
  
  if (animating) {
    drawLyric(oldLyric, animOffset);
    drawLyric(currentLyric, animOffset - 24);
  } else {
    drawLyric(currentLyric, 0);
  }
  
  // Mask overlaps from sliding lyrics into the yellow zone
  display.fillRect(0, 48, 128, 16, SSD1306_BLACK); 

  // =====================================
  // YELLOW ZONE (Physical Bottom, Code y=48-63)
  // =====================================
  // Contains Song Title Marquee AND Progress Bar!
  
  // 1. Song Title Marquee (y=48)
  display.setFont(NULL); 
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  String headerText = songTitle;
  if (songArtist.length() > 0 && songArtist != "Unknown") headerText += " - " + songArtist;
  
  display.setCursor((int)marqueeX, 48); // Set to y=48 in Yellow Zone!
  display.print(headerText);

  // 2. Progress Bar (y=58-63 zone)
  float currentProg = progressMs + (millis() - lastUpdateMs);
  if (currentProg > durationMs) currentProg = durationMs;
  if (durationMs < 1) durationMs = 1;
  int barWidth = (currentProg / durationMs) * 128;
  
  // Draw short, clean progress bar right below the title (y=58, height 4)
  display.drawRect(0, 58, 128, 4, SSD1306_WHITE);
  display.fillRect(0, 58, barWidth, 4, SSD1306_WHITE);

  display.display();
}
