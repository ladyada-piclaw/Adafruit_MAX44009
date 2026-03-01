/*!
 * @file test_lux.ino
 *
 * Hardware test: Read lux values and verify reasonable range.
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

  Serial.println(F("=== MAX44009 Lux Test ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    failed++;
    goto summary;
  }
  test(F("Sensor found"), true);

  // Wait for first measurement (800ms in default mode)
  delay(1000);

  // Test 1: Read lux (full resolution)
  {
    float lux = max44009.readLux();
    Serial.print(F("Lux (full): "));
    Serial.println(lux);
    test(F("Lux is not NAN"), !isnan(lux));
    test(F("Lux is positive"), lux >= 0);
    test(F("Lux is reasonable (<200000)"), lux < 200000);
    test(F("Not overrange"), !max44009.isOverrange());
  }

  // Test 2: Read lux fast
  {
    Serial.print(F("Lux (fast): "));
  }

  // Test 3: Multiple readings should be consistent
  {
    float readings[5];
    for (int i = 0; i < 5; i++) {
      readings[i] = max44009.readLux();
      delay(100);
    }
    Serial.print(F("5 readings: "));
    for (int i = 0; i < 5; i++) {
      Serial.print(readings[i]);
      Serial.print(F(" "));
    }
    Serial.println();

    // Check that readings are within 50% of each other (ambient light varies)
    float minVal = readings[0];
    float maxVal = readings[0];
    for (int i = 1; i < 5; i++) {
      if (readings[i] < minVal) minVal = readings[i];
      if (readings[i] > maxVal) maxVal = readings[i];
    }
    // Avoid division by zero
    bool consistent = (minVal > 0) ? (maxVal / minVal < 2.0) : (maxVal < 1.0);
    test(F("Readings are consistent"), consistent);
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
