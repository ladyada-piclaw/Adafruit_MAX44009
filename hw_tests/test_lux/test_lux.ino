/*!
 * @file test_lux.ino
 *
 * Hardware test: Read lux values and verify sensor produces real,
 * consistent measurements in different modes.
 */

#include <Adafruit_MAX44009.h>
#include <Wire.h>

Adafruit_MAX44009 max44009;

int passed = 0;
int failed = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== MAX44009 Lux Test ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    while (1)
      delay(10);
  }
  test(F("Sensor found"), true);

  // Wait for first measurement (800ms in default mode)
  delay(1000);

  // --- Basic read in default auto mode ---
  Serial.println(F("\n--- Default auto mode ---"));
  {
    float lux = max44009.readLux();
    Serial.print(F("Lux: "));
    Serial.println(lux);
    test(F("Not NAN"), !isnan(lux));
    test(F("Positive"), lux > 0);
    test(F("Not overrange"), !max44009.isOverrange());
  }

  // --- Consistency: 5 readings should be stable ---
  Serial.println(F("\n--- Consistency (5 reads, default mode) ---"));
  {
    float readings[5];
    for (int i = 0; i < 5; i++) {
      readings[i] = max44009.readLux();
      delay(900); // default mode updates every ~800ms
    }
    for (int i = 0; i < 5; i++) {
      Serial.print(readings[i]);
      Serial.print(F(" "));
    }
    Serial.println();

    float minVal = readings[0], maxVal = readings[0];
    for (int i = 1; i < 5; i++) {
      if (readings[i] < minVal)
        minVal = readings[i];
      if (readings[i] > maxVal)
        maxVal = readings[i];
    }
    bool consistent = (minVal > 0) ? (maxVal / minVal < 2.0) : (maxVal < 1.0);
    test(F("Readings within 2x of each other"), consistent);
  }

  // --- Continuous mode reads faster ---
  Serial.println(F("\n--- Continuous mode ---"));
  {
    max44009.setMode(MAX44009_MODE_CONTINUOUS);
    delay(200);
    float lux = max44009.readLux();
    Serial.print(F("Lux: "));
    Serial.println(lux);
    test(F("Continuous mode reads OK"), !isnan(lux) && lux > 0);
  }

  // --- Manual mode with different integration times ---
  Serial.println(F("\n--- Manual mode, varying integration ---"));
  {
    max44009.setMode(MAX44009_MODE_MANUAL_CONTINUOUS);

    max44009.setIntegrationTime(MAX44009_INTEGRATION_800MS);
    delay(900);
    float lux800 = max44009.readLux();
    Serial.print(F("800ms: "));
    Serial.println(lux800);

    max44009.setIntegrationTime(MAX44009_INTEGRATION_100MS);
    delay(200);
    float lux100 = max44009.readLux();
    Serial.print(F("100ms: "));
    Serial.println(lux100);

    max44009.setIntegrationTime(MAX44009_INTEGRATION_6_25MS);
    delay(50);
    float lux6 = max44009.readLux();
    Serial.print(F("6.25ms: "));
    Serial.println(lux6);

    // All should be valid (not NAN, not overrange)
    test(F("800ms valid"), !isnan(lux800));
    test(F("100ms valid"), !isnan(lux100));
    test(F("6.25ms valid"), !isnan(lux6));

    // At least one must be positive
    // In bright light, 800ms may saturate (reads lower)
    // In dim light, 6.25ms may read 0 (not enough sensitivity)
    bool anyPositive = (lux800 > 0) || (lux100 > 0) || (lux6 > 0);
    test(F("At least one > 0"), anyPositive);
  }

  // Restore default
  max44009.setMode(MAX44009_MODE_DEFAULT);

  Serial.println();
  Serial.println(F("=== Summary ==="));
  Serial.print(F("Passed: "));
  Serial.println(passed);
  Serial.print(F("Failed: "));
  Serial.println(failed);
  Serial.println(
      passed > 0 && failed == 0 ? F("ALL TESTS PASSED") : F("SOME TESTS FAILED"));
}

void loop() { delay(1000); }

void test(const __FlashStringHelper *name, bool condition) {
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
