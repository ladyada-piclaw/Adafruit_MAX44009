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
  // Options: MAX44009_MODE_DEFAULT (auto, 800ms cycle, lowest power)
  //          MAX44009_MODE_CONTINUOUS (auto, fast updates)
  //          MAX44009_MODE_MANUAL (manual gain/time, 800ms cycle)
  //          MAX44009_MODE_MANUAL_CONTINUOUS (manual gain/time, fast updates)
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
  // Only configurable in MANUAL modes; skip if using auto-ranging
  if (max44009.getMode() == MAX44009_MODE_MANUAL ||
      max44009.getMode() == MAX44009_MODE_MANUAL_CONTINUOUS) {
    Serial.println(F("\n--- Integration Time ---"));

    max44009.setIntegrationTime(MAX44009_INTEGRATION_100MS);
    // Options: MAX44009_INTEGRATION_800MS  (best low-light sensitivity)
    //          MAX44009_INTEGRATION_400MS
    //          MAX44009_INTEGRATION_200MS
    //          MAX44009_INTEGRATION_100MS  (default, best high-brightness)
    //          MAX44009_INTEGRATION_50MS   (manual mode only)
    //          MAX44009_INTEGRATION_25MS   (manual mode only)
    //          MAX44009_INTEGRATION_12_5MS (manual mode only)
    //          MAX44009_INTEGRATION_6_25MS (manual mode only)
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

    max44009.setCurrentDivisionRatio(false);
    // false = full photodiode current to ADC (normal)
    // true  = 1/8 current to ADC (for very bright environments)
    Serial.print(F("CDR: "));
    Serial.println(max44009.getCurrentDivisionRatio() ? F("1/8 (divided)")
                                                      : F("Full current"));

  } else {
    Serial.println(F("\n(Integration time and CDR are auto-managed)"));
  }

  // === Interrupt Configuration ===
  Serial.println(F("\n--- Interrupt Configuration ---"));

  max44009.setUpperThreshold(10000.0);
  // Range: 0.045 to 188,000 lux (quantized to exponent/mantissa encoding)
  Serial.print(F("Upper Threshold: "));
  Serial.print(max44009.getUpperThreshold());
  Serial.println(F(" lux"));

  max44009.setLowerThreshold(1.0);
  // Range: 0.045 to 188,000 lux
  Serial.print(F("Lower Threshold: "));
  Serial.print(max44009.getLowerThreshold());
  Serial.println(F(" lux"));

  max44009.setThresholdTimer(10);
  // Timer value * 100ms = delay before interrupt fires
  // 0 = immediate, 255 = 25.5 seconds (default)
  Serial.print(F("Threshold Timer: "));
  Serial.print(max44009.getThresholdTimer());
  Serial.println(F(" (x100ms)"));

  max44009.enableInterrupt(false);
  // true = INT pin asserts low when lux outside threshold window
  // Note: INT pin is open-drain, needs external pull-up
  // Note: reading getInterruptStatus() clears the interrupt
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
