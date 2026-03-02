/*
 * Simple example for the MAX44009 Ambient Light Sensor
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code for
 * Adafruit Industries. MIT license, check license.txt for more information
 *
 * Reads lux every 100ms in auto-ranging mode. Serial Plotter friendly.
 */

#include <Adafruit_MAX44009.h>

Adafruit_MAX44009 max44009;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009 sensor!"));
    while (1)
      delay(10);
  }
}

void loop() {
  float lux = max44009.readLux();

  if (!isnan(lux)) {
    Serial.print(F("Lux:"));
    Serial.println(lux);
  }

  delay(100);
}
