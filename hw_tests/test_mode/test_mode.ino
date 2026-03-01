/*!
 * @file test_mode.ino
 *
 * Hardware test: Verify mode affects actual measurement rate.
 * Strategy: In manual modes, toggle CDR and measure how long until the
 * raw lux registers change.
 * MANUAL (CONT=0) = ~800ms cycle, MANUAL_CONTINUOUS (CONT=1) = fast.
 */

#include <Adafruit_MAX44009.h>
#include <Wire.h>

Adafruit_MAX44009 max44009;

int passed = 0;
int failed = 0;


// Read raw lux registers as a 16-bit value for exact comparison

// Toggle CDR and measure how long until raw lux registers change

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== MAX44009 Mode Test (timing) ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    while (1)
      delay(10);
  }
  test(F("Sensor found"), true);

  // --- MANUAL mode (CONT=0): 800ms cycle ---
  Serial.println(F("\n--- Manual mode, CONT=0 (expect ~800ms cycle) ---"));
  max44009.setMode(MAX44009_MODE_MANUAL);
  max44009.setIntegrationTime(MAX44009_INTEGRATION_100MS);
  test(F("Mode set to MANUAL"), max44009.getMode() == MAX44009_MODE_MANUAL);

  // Let a full cycle complete so we start fresh
  delay(1000);
  uint16_t raw1 = readRawLux();
  Serial.print(F("Raw before CDR toggle: 0x"));
  Serial.println(raw1, HEX);

  unsigned long manualTime = measureUpdateRate(3000);
  uint16_t raw2 = readRawLux();
  Serial.print(F("Raw after change: 0x"));
  Serial.println(raw2, HEX);
  Serial.print(F("Update latency: "));
  Serial.print(manualTime);
  Serial.println(F(" ms"));
  test(F("Manual ~800ms cycle"), manualTime >= 400 && manualTime <= 1500);

  // --- MANUAL_CONTINUOUS (CONT=1) + 100ms: fast ---
  Serial.println(F("\n--- Manual continuous + 100ms (expect ~100ms) ---"));
  max44009.setMode(MAX44009_MODE_MANUAL_CONTINUOUS);
  max44009.setIntegrationTime(MAX44009_INTEGRATION_100MS);
  test(F("Mode set to MANUAL_CONTINUOUS"),
       max44009.getMode() == MAX44009_MODE_MANUAL_CONTINUOUS);
  delay(200);
  unsigned long contTime = measureUpdateRate(2000);
  Serial.print(F("Update latency: "));
  Serial.print(contTime);
  Serial.println(F(" ms"));
  test(F("Continuous 100ms: < 300ms"), contTime > 0 && contTime <= 300);

  // --- MANUAL_CONTINUOUS + 6.25ms: very fast ---
  Serial.println(F("\n--- Manual continuous + 6.25ms (expect < 50ms) ---"));
  max44009.setIntegrationTime(MAX44009_INTEGRATION_6_25MS);
  delay(50);
  unsigned long fastTime = measureUpdateRate(500);
  Serial.print(F("Update latency: "));
  Serial.print(fastTime);
  Serial.println(F(" ms"));
  test(F("Continuous 6.25ms: < 50ms"), fastTime > 0 && fastTime <= 50);

  // --- Verify ordering ---
  Serial.println(F("\n--- Timing order ---"));
  Serial.print(F("Manual800="));
  Serial.print(manualTime);
  Serial.print(F("  Cont100="));
  Serial.print(contTime);
  Serial.print(F("  Cont6.25="));
  Serial.println(fastTime);
  test(F("Manual > Continuous 100ms"), manualTime > contTime);
  test(F("Continuous 100ms > 6.25ms"), contTime > fastTime);

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

uint16_t readRawLux() {
  Adafruit_I2CDevice i2c(0x4A);
  Adafruit_BusIO_Register lux_reg(&i2c, 0x03, 2, MSBFIRST);
  uint16_t raw;
  lux_reg.read(&raw);
  return raw;
}

unsigned long measureUpdateRate(unsigned long timeoutMs) {
  uint16_t initial = readRawLux();

  bool currentCDR = max44009.getCurrentDivisionRatio();
  max44009.setCurrentDivisionRatio(!currentCDR);

  unsigned long start = millis();
  while ((millis() - start) < timeoutMs) {
    uint16_t raw = readRawLux();
    if (raw != initial) {
      unsigned long elapsed = millis() - start;
      max44009.setCurrentDivisionRatio(currentCDR);
      return elapsed;
    }
    delay(2);
  }
  max44009.setCurrentDivisionRatio(currentCDR);
  return 0;
}
