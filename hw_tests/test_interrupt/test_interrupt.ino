/*!
 * @file test_interrupt.ino
 *
 * Hardware test: Enable/disable interrupt and check status register.
 * Note: Reading INT_STATUS clears the interrupt.
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

  Serial.println(F("=== MAX44009 Interrupt Test ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    failed++;
    goto summary;
  }
  test(F("Sensor found"), true);

  // Test interrupt enable
  {
    max44009.enableInterrupt(true);
    bool enabled = max44009.isInterruptEnabled();
    Serial.print(F("Enable interrupt: "));
    Serial.println(enabled ? F("enabled") : F("disabled"));
    test(F("Interrupt enabled"), enabled == true);
  }

  // Test interrupt disable
  {
    max44009.enableInterrupt(false);
    bool enabled = max44009.isInterruptEnabled();
    Serial.print(F("Disable interrupt: "));
    Serial.println(enabled ? F("enabled") : F("disabled"));
    test(F("Interrupt disabled"), enabled == false);
  }

  // Test reading interrupt status (should be 0 with no threshold set)
  {
    max44009.enableInterrupt(true);
    // Clear any pending interrupt by reading
    bool status1 = max44009.getInterruptStatus();
    Serial.print(F("Initial status (may be set): "));
    Serial.println(status1 ? F("1") : F("0"));

    // Read again - should be cleared now
    bool status2 = max44009.getInterruptStatus();
    Serial.print(F("After clear (should be 0): "));
    Serial.println(status2 ? F("1") : F("0"));
    test(F("Status cleared after read"), status2 == false);
  }

  // Disable interrupt
  max44009.enableInterrupt(false);

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
