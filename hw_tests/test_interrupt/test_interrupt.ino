/*!
 * @file test_interrupt.ino
 *
 * Hardware test: Interrupt register AND physical INT pin on D2.
 * Wiring: MAX44009 INT pin -> Metro Mini D2 (with external pull-up)
 * Note: INT is open-drain, active LOW on trigger.
 * Note: Reading INT_STATUS clears the interrupt.
 */

#include <Adafruit_MAX44009.h>
#include <Wire.h>

#define INT_PIN 2

Adafruit_MAX44009 max44009;

int passed = 0;
int failed = 0;


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  pinMode(INT_PIN, INPUT_PULLUP);

  Serial.println(F("=== MAX44009 Interrupt Test (register + pin) ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    failed++;
    goto summary;
  }
  test(F("Sensor found"), true);

  // --- Register-level tests ---

  // Test interrupt enable/disable
  {
    max44009.enableInterrupt(true);
    bool enabled = max44009.isInterruptEnabled();
    Serial.print(F("Enable interrupt: "));
    Serial.println(enabled ? F("enabled") : F("disabled"));
    test(F("Interrupt enabled"), enabled == true);
  }

  {
    max44009.enableInterrupt(false);
    bool enabled = max44009.isInterruptEnabled();
    Serial.print(F("Disable interrupt: "));
    Serial.println(enabled ? F("enabled") : F("disabled"));
    test(F("Interrupt disabled"), enabled == false);
  }

  // Test status clear on read
  {
    max44009.enableInterrupt(true);
    max44009.getInterruptStatus(); // clear any pending
    bool status2 = max44009.getInterruptStatus();
    Serial.print(F("Status after clear: "));
    Serial.println(status2 ? F("1") : F("0"));
    test(F("Status cleared after read"), status2 == false);
    max44009.enableInterrupt(false);
  }

  // --- Physical INT pin tests ---
  Serial.println();
  Serial.println(F("--- INT Pin Tests (D2) ---"));

  // With interrupts disabled, pin should be HIGH (open-drain, pulled up)
  {
    max44009.enableInterrupt(false);
    delay(100);
    int pinVal = digitalRead(INT_PIN);
    Serial.print(F("INT pin (irq disabled): "));
    Serial.println(pinVal ? F("HIGH") : F("LOW"));
    test(F("Pin HIGH when irq disabled"), pinVal == HIGH);
  }

  // Set upper threshold very low (0 lux) so ambient light triggers interrupt
  // Set timer to 0 for immediate interrupt
  {
    max44009.setUpperThreshold(0.0);
    max44009.setThresholdTimer(0);
    max44009.getInterruptStatus(); // clear any pending

    // Enable interrupts
    max44009.enableInterrupt(true);

    // Wait for a measurement cycle (up to ~900ms in default mode)
    Serial.print(F("Waiting for interrupt trigger..."));
    delay(1000);
    Serial.println(F(" done"));

    // Check if pin went LOW (interrupt asserted)
    int pinVal = digitalRead(INT_PIN);
    Serial.print(F("INT pin (threshold exceeded): "));
    Serial.println(pinVal ? F("HIGH") : F("LOW"));
    test(F("Pin LOW when lux > upper threshold"), pinVal == LOW);

    // Check register agrees
    bool status = max44009.getInterruptStatus();
    Serial.print(F("Interrupt status register: "));
    Serial.println(status ? F("1 (set)") : F("0 (clear)"));
    test(F("Status register set"), status == true);

    // After reading status, pin should go HIGH again
    delay(10);
    pinVal = digitalRead(INT_PIN);
    Serial.print(F("INT pin after status read: "));
    Serial.println(pinVal ? F("HIGH") : F("LOW"));
    test(F("Pin HIGH after status clear"), pinVal == HIGH);
  }

  // Restore defaults
  max44009.enableInterrupt(false);
  max44009.setUpperThreshold(188000.0);
  max44009.setLowerThreshold(0.0);
  max44009.setThresholdTimer(255);

summary:
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
