/*!
 * @file test_integration.ino
 *
 * Hardware test: Set and verify each integration time.
 * Note: Manual mode required for times faster than 100ms.
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

const char* timeNames[] = {"800ms", "400ms", "200ms", "100ms",
                           "50ms",  "25ms",  "12.5ms", "6.25ms"};

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("=== MAX44009 Integration Time Test ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    failed++;
    goto summary;
  }
  test(F("Sensor found"), true);

  // Enable manual mode (required for all integration times)
  max44009.setMode(MAX44009_MODE_MANUAL);
  test(F("Manual mode set"), max44009.getMode() == MAX44009_MODE_MANUAL);

  // Test each integration time
  {
    max44009_integration_time_t times[] = {
        MAX44009_INTEGRATION_800MS,  MAX44009_INTEGRATION_400MS,
        MAX44009_INTEGRATION_200MS,  MAX44009_INTEGRATION_100MS,
        MAX44009_INTEGRATION_50MS,   MAX44009_INTEGRATION_25MS,
        MAX44009_INTEGRATION_12_5MS, MAX44009_INTEGRATION_6_25MS};

    for (int i = 0; i < 8; i++) {
      max44009.setIntegrationTime(times[i]);
      max44009_integration_time_t readTime = max44009.getIntegrationTime();
      Serial.print(F("Integration "));
      Serial.print(timeNames[i]);
      Serial.print(F(": read "));
      Serial.println(readTime);
      test(F("  Match"), readTime == times[i]);
    }
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
