/*
 * @file max44009_tft_demo.ino
 *
 * TFT demo for MAX44009 Ambient Light Sensor on Feather ESP32-S2 TFT
 * Displays lux as a large color-coded readout.
 * Uses GFXcanvas16 to eliminate flicker.
 *
 * Hardware:
 *  - Adafruit Feather ESP32-S2 TFT
 *  - MAX44009 Ambient Light Sensor breakout (I2C)
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code
 *
 * MIT license, all text above must be included in any redistribution.
 */

#include <Adafruit_GFX.h>
#include <Adafruit_MAX44009.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Wire.h>

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_MAX44009 max44009;

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135

#define COLOR_TITLE 0xFFFF
#define COLOR_BAR_BG 0x2104

// Canvas for the lux area (below title)
#define LUX_Y 28
#define LUX_W 240
#define LUX_H 107

#define LUX_MIN 0.045f
#define LUX_MAX 188000.0f

GFXcanvas16 luxCanvas(LUX_W, LUX_H);

uint16_t getLuxColor(float lux, const __FlashStringHelper** label) {
  if (lux < 1.0f) {
    *label = F("Dark");
    return 0x3BFF;
  } else if (lux < 50.0f) {
    *label = F("Dim");
    return 0x07FF;
  } else if (lux < 500.0f) {
    *label = F("Indoor");
    return 0x07E0;
  } else if (lux < 10000.0f) {
    *label = F("Overcast");
    return 0xFFE0;
  } else if (lux < 50000.0f) {
    *label = F("Daylight");
    return 0xFD20;
  }
  *label = F("Direct Sun");
  return 0xFFFF;
}

void formatLux(float lux, char* buffer, size_t len) {
  uint8_t decimals = 0;
  if (lux < 10.0f) {
    decimals = 2;
  } else if (lux < 1000.0f) {
    decimals = 1;
  }
  dtostrf(lux, 0, decimals, buffer);
  buffer[len - 1] = 0;
}

float clampLux(float lux) {
  if (lux < LUX_MIN)
    return LUX_MIN;
  if (lux > LUX_MAX)
    return LUX_MAX;
  return lux;
}

void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000))
    delay(10);

  Serial.println(F("MAX44009 Ambient Light Sensor TFT Demo"));

  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  tft.init(135, 240);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  const char title[] = "Adafruit MAX44009";
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(COLOR_TITLE);
  int16_t x1, y1;
  uint16_t tw, th;
  tft.getTextBounds(title, 0, 0, &x1, &y1, &tw, &th);
  tft.setCursor((SCREEN_WIDTH - tw) / 2, 22);
  tft.print(title);

  Wire.begin();

  if (!max44009.begin()) {
    Serial.println(F("Failed to find MAX44009 sensor!"));
    tft.setFont(&FreeSansBold24pt7b);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(20, 90);
    tft.print(F("No Sensor!"));
    while (1)
      delay(100);
  }

  Serial.println(F("MAX44009 found!"));

  max44009.setMode(MAX44009_MODE_CONTINUOUS);
}

void loop() {
  float lux = max44009.readLux();
  Serial.print(F("Lux: "));
  Serial.println(lux, 4);

  luxCanvas.fillScreen(ST77XX_BLACK);

  const __FlashStringHelper* label = F("\0");
  uint16_t color = getLuxColor(lux, &label);

  char luxBuf[12];
  formatLux(lux, luxBuf, sizeof(luxBuf));

  luxCanvas.setFont(&FreeSansBold24pt7b);
  luxCanvas.setTextSize(1);
  luxCanvas.setTextColor(color);

  int16_t x1, y1;
  uint16_t numW, numH;
  luxCanvas.getTextBounds(luxBuf, 0, 0, &x1, &y1, &numW, &numH);

  luxCanvas.setFont(&FreeSansBold18pt7b);
  int16_t ux1, uy1;
  uint16_t unitW, unitH;
  luxCanvas.getTextBounds(" lux", 0, 0, &ux1, &uy1, &unitW, &unitH);

  uint16_t gap = 4;
  uint16_t totalW = numW + gap + unitW;
  int16_t tx = (LUX_W - totalW) / 2;
  int16_t ty = (LUX_H / 2) + (numH / 2) - 24;

  luxCanvas.setFont(&FreeSansBold24pt7b);
  luxCanvas.setCursor(tx, ty);
  luxCanvas.print(luxBuf);

  luxCanvas.setFont(&FreeSansBold18pt7b);
  luxCanvas.setCursor(luxCanvas.getCursorX() + gap, ty);
  luxCanvas.print(F(" lux"));

  luxCanvas.setFont(&FreeSansBold18pt7b);
  luxCanvas.setTextColor(color);
  int16_t lx1, ly1;
  uint16_t labelW, labelH;
  luxCanvas.getTextBounds(label, 0, 0, &lx1, &ly1, &labelW, &labelH);
  int16_t labelX = (LUX_W - labelW) / 2;
  int16_t labelY = ty + labelH + 8;
  luxCanvas.setCursor(labelX, labelY);
  luxCanvas.print(label);

  float clamped = clampLux(lux);
  float logMin = log10f(LUX_MIN);
  float logMax = log10f(LUX_MAX);
  float logLux = log10f(clamped);
  float ratio = (logLux - logMin) / (logMax - logMin);
  if (ratio < 0.0f)
    ratio = 0.0f;
  if (ratio > 1.0f)
    ratio = 1.0f;

  const uint8_t barHeight = 8;
  const uint8_t barMargin = 4;
  int16_t barY = LUX_H - barHeight - barMargin;
  luxCanvas.fillRect(0, barY, LUX_W, barHeight, COLOR_BAR_BG);
  int16_t barFill = (int16_t)(ratio * (float)LUX_W + 0.5f);
  if (barFill > 0) {
    luxCanvas.fillRect(0, barY, barFill, barHeight, color);
  }

  tft.drawRGBBitmap(0, LUX_Y, luxCanvas.getBuffer(), LUX_W, LUX_H);

  delay(100);
}
