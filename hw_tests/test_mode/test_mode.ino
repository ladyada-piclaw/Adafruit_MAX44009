/*!
 * @file test_mode.ino
 *
 * Hardware test: Set and verify each operating mode.
 */

#include <Adafruit_MAX44009.h>
#include <Wire.h>

Adafruit_MAX44009 max44009;

int passed = 0;
int failed = 0;

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

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("=== MAX44009 Mode Test ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    failed++;
    goto summary;
  }
  test(F("Sensor found"), true);

  // Test each mode
  {
    max44009.setMode(MAX44009_MODE_DEFAULT);
    max44009_mode_t readMode = max44009.getMode();
    Serial.print(F("MODE_DEFAULT (0x00): read 0x"));
    Serial.println(readMode, HEX);
    test(F("MODE_DEFAULT"), readMode == MAX44009_MODE_DEFAULT);
  }

  {
    max44009.setMode(MAX44009_MODE_CONTINUOUS);
    max44009_mode_t readMode = max44009.getMode();
    Serial.print(F("MODE_CONTINUOUS (0x80): read 0x"));
    Serial.println(readMode, HEX);
    test(F("MODE_CONTINUOUS"), readMode == MAX44009_MODE_CONTINUOUS);
  }

  {
    max44009.setMode(MAX44009_MODE_MANUAL);
    max44009_mode_t readMode = max44009.getMode();
    Serial.print(F("MODE_MANUAL (0x40): read 0x"));
    Serial.println(readMode, HEX);
    test(F("MODE_MANUAL"), readMode == MAX44009_MODE_MANUAL);
  }

  {
    max44009.setMode(MAX44009_MODE_MANUAL_CONTINUOUS);
    max44009_mode_t readMode = max44009.getMode();
    Serial.print(F("MODE_MANUAL_CONTINUOUS (0xC0): read 0x"));
    Serial.println(readMode, HEX);
    test(F("MODE_MANUAL_CONTINUOUS"), readMode == MAX44009_MODE_MANUAL_CONTINUOUS);
  }

  // Reset to default
  max44009.setMode(MAX44009_MODE_DEFAULT);

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
