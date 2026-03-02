/*!
 * @file test_threshold.ino
 *
 * Hardware test: Set upper/lower thresholds and verify lux conversion roundtrip.
 */

#include <Adafruit_MAX44009.h>
#include <Wire.h>

Adafruit_MAX44009 max44009;

int passed = 0;
int failed = 0;


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("=== MAX44009 Threshold Test ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    failed++;
    goto summary;
  }
  test(F("Sensor found"), true);

  // Test upper threshold with various lux values
  Serial.println(F("\n--- Upper Threshold Tests ---"));
  {
    float testLux[] = {10.0, 50.0, 100.0, 500.0, 1000.0, 5000.0, 10000.0};
    for (int i = 0; i < 7; i++) {
      max44009.setUpperThreshold(testLux[i]);
      float readLux = max44009.getUpperThreshold();
      Serial.print(F("Set "));
      Serial.print(testLux[i]);
      Serial.print(F(" -> Read "));
      Serial.print(readLux);

      // Threshold has limited precision (4-bit mantissa)
      // Allow 50% tolerance for roundtrip
      float ratio = readLux / testLux[i];
      bool inRange = (ratio > 0.5 && ratio < 2.0);
      Serial.println(inRange ? F(" [OK]") : F(" [FAIL]"));
      if (inRange) passed++;
      else failed++;
    }
  }

  // Test lower threshold
  Serial.println(F("\n--- Lower Threshold Tests ---"));
  {
    float testLux[] = {5.0, 25.0, 100.0, 500.0};
    for (int i = 0; i < 4; i++) {
      max44009.setLowerThreshold(testLux[i]);
      float readLux = max44009.getLowerThreshold();
      Serial.print(F("Set "));
      Serial.print(testLux[i]);
      Serial.print(F(" -> Read "));
      Serial.print(readLux);

      float ratio = readLux / testLux[i];
      bool inRange = (ratio > 0.5 && ratio < 2.0);
      Serial.println(inRange ? F(" [OK]") : F(" [FAIL]"));
      if (inRange) passed++;
      else failed++;
    }
  }

  // Test threshold timer
  Serial.println(F("\n--- Threshold Timer Tests ---"));
  {
    uint8_t testTimers[] = {0, 1, 10, 100, 255};
    for (int i = 0; i < 5; i++) {
      max44009.setThresholdTimer(testTimers[i]);
      uint8_t readTimer = max44009.getThresholdTimer();
      Serial.print(F("Set "));
      Serial.print(testTimers[i]);
      Serial.print(F(" ("));
      Serial.print(testTimers[i] * 100);
      Serial.print(F("ms) -> Read "));
      Serial.print(readTimer);
      bool match = (readTimer == testTimers[i]);
      Serial.println(match ? F(" [OK]") : F(" [FAIL]"));
      if (match) passed++;
      else failed++;
    }
  }

summary:
  Serial.println();
  Serial.println(F("=== Summary ==="));
  Serial.print(F("Passed: "));
  Serial.println(passed);
  Serial.print(F("Failed: "));
  Serial.println(failed);
  Serial.println(passed > 0 && failed == 0 ? F("ALL TESTS PASSED") : F("SOME TESTS FAILED"));
}

void loop() {
  delay(1000);
}

void test(const __FlashStringHelper* name, bool condition) {
  Serial.print(name);
  Serial.print(F(": "));
  if (condition) {
    Serial.println(F("PASS"));
    passed++;
  } else {
    Serial.println(F("FAIL"));
    failed++;
  }
}
