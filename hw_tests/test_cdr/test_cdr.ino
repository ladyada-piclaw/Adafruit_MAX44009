/*!
 * @file test_cdr.ino
 *
 * Hardware test: Set and verify Current Division Ratio (CDR).
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

  Serial.println(F("=== MAX44009 CDR Test ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    failed++;
    goto summary;
  }
  test(F("Sensor found"), true);

  // Enable manual mode (CDR only writable in manual mode)
  max44009.setMode(MAX44009_MODE_MANUAL);
  test(F("Manual mode set"), max44009.getMode() == MAX44009_MODE_MANUAL);

  // Test CDR = false (full current, default)
  {
    max44009.setCurrentDivisionRatio(false);
    bool cdr = max44009.getCurrentDivisionRatio();
    Serial.print(F("CDR=false: read "));
    Serial.println(cdr ? F("true") : F("false"));
    test(F("CDR false"), cdr == false);
  }

  // Test CDR = true (1/8 current)
  {
    max44009.setCurrentDivisionRatio(true);
    bool cdr = max44009.getCurrentDivisionRatio();
    Serial.print(F("CDR=true: read "));
    Serial.println(cdr ? F("true") : F("false"));
    test(F("CDR true"), cdr == true);
  }

  // Reset CDR and mode
  max44009.setCurrentDivisionRatio(false);
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
