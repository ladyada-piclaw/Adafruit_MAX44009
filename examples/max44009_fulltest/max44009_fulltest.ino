/*
 * Full test sketch for MAX44009 Ambient Light Sensor
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code for
 * Adafruit Industries. MIT license, check license.txt for more information
 *
 * Displays configuration and continuous lux readings.
 */

#include <Adafruit_MAX44009.h>

Adafruit_MAX44009 max44009;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("MAX44009 Full Test"));
  Serial.println(F("=================="));

  if (!max44009.begin()) {
    Serial.println(F("Couldn't find MAX44009 chip"));
    while (1)
      delay(10);
  }

  Serial.println(F("MAX44009 found!"));

  // === Mode Configuration ===
  Serial.println(F("\n--- Mode Configuration ---"));

  max44009.setMode(MAX44009_MODE_CONTINUOUS);
  Serial.print(F("Mode: "));
  switch (max44009.getMode()) {
  case MAX44009_MODE_DEFAULT:
    Serial.println(F("Default (auto, 800ms cycle)"));
    break;
  case MAX44009_MODE_CONTINUOUS:
    Serial.println(F("Continuous (auto, fast updates)"));
    break;
  case MAX44009_MODE_MANUAL:
    Serial.println(F("Manual (800ms cycle)"));
    break;
  case MAX44009_MODE_MANUAL_CONTINUOUS:
    Serial.println(F("Manual Continuous (fast updates)"));
    break;
  }

  // === Integration Time ===
  // Switch to manual mode to set integration time, then back to continuous
  Serial.println(F("\n--- Integration Time ---"));

  max44009.setMode(MAX44009_MODE_MANUAL_CONTINUOUS);
  max44009.setIntegrationTime(MAX44009_INTEGRATION_100MS);
  Serial.print(F("Integration Time: "));
  switch (max44009.getIntegrationTime()) {
  case MAX44009_INTEGRATION_800MS:
    Serial.println(F("800ms"));
    break;
  case MAX44009_INTEGRATION_400MS:
    Serial.println(F("400ms"));
    break;
  case MAX44009_INTEGRATION_200MS:
    Serial.println(F("200ms"));
    break;
  case MAX44009_INTEGRATION_100MS:
    Serial.println(F("100ms"));
    break;
  case MAX44009_INTEGRATION_50MS:
    Serial.println(F("50ms"));
    break;
  case MAX44009_INTEGRATION_25MS:
    Serial.println(F("25ms"));
    break;
  case MAX44009_INTEGRATION_12_5MS:
    Serial.println(F("12.5ms"));
    break;
  case MAX44009_INTEGRATION_6_25MS:
    Serial.println(F("6.25ms"));
    break;
  }

  // === Current Division Ratio ===
  Serial.println(F("\n--- Current Division Ratio ---"));

  max44009.setCurrentDivisionRatio(false); // false = full current
  Serial.print(F("CDR: "));
  Serial.println(max44009.getCurrentDivisionRatio() ? F("1/8 (divided)")
                                                    : F("Full current"));

  // === Switch back to auto continuous mode ===
  max44009.setMode(MAX44009_MODE_CONTINUOUS);

  // === Interrupt Configuration ===
  Serial.println(F("\n--- Interrupt Configuration ---"));

  max44009.setUpperThreshold(10000.0);
  Serial.print(F("Upper Threshold: "));
  Serial.print(max44009.getUpperThreshold());
  Serial.println(F(" lux"));

  max44009.setLowerThreshold(1.0);
  Serial.print(F("Lower Threshold: "));
  Serial.print(max44009.getLowerThreshold());
  Serial.println(F(" lux"));

  max44009.setThresholdTimer(10); // 10 * 100ms = 1 second
  Serial.print(F("Threshold Timer: "));
  Serial.print(max44009.getThresholdTimer());
  Serial.println(F(" (x100ms)"));

  max44009.enableInterrupt(false);
  Serial.print(F("Interrupt: "));
  Serial.println(max44009.isInterruptEnabled() ? F("Enabled") : F("Disabled"));

  // === Continuous Lux Readings ===
  Serial.println(F("\n--- Lux Readings ---"));

  delay(200); // Let sensor stabilize
}

void loop() {
  float lux = max44009.readLux();

  Serial.print(F("Lux:"));
  if (isnan(lux)) {
    Serial.println(F("OVERRANGE"));
  } else {
    Serial.println(lux);
  }

  delay(100);
}
