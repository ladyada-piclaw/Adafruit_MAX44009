/*!
 * @file test_integration.ino
 *
 * Hardware test: Verify integration time affects actual measurement timing.
 * Strategy: In manual continuous mode, toggle CDR and measure how long
 * until the raw lux registers change. Longer integration = longer update.
 *
 * We test three integration times (6.25ms, 100ms, 800ms) and verify
 * the ordering is correct: fast < medium < slow.
 */

#include <Adafruit_MAX44009.h>
#include <Wire.h>

Adafruit_MAX44009 max44009;

int passed = 0;
int failed = 0;



// Toggle CDR and measure how long until raw lux registers change.
// Takes median of 3 runs for stability.

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== MAX44009 Integration Time Test (timing) ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    while (1)
      delay(10);
  }
  test(F("Sensor found"), true);

  max44009.setMode(MAX44009_MODE_MANUAL_CONTINUOUS);
  test(F("Manual continuous mode"),
       max44009.getMode() == MAX44009_MODE_MANUAL_CONTINUOUS);

  // --- 6.25ms (3 runs, median) ---
  Serial.println(F("\n--- 6.25ms integration ---"));
  unsigned long t_fast =
      measureUpdateRate(MAX44009_INTEGRATION_6_25MS, 50, 500);
  Serial.print(F("Median update time: "));
  Serial.print(t_fast);
  Serial.println(F(" ms"));
  test(F("6.25ms: < 100ms"), t_fast > 0 && t_fast < 100);

  // --- 100ms (3 runs, median) ---
  Serial.println(F("\n--- 100ms integration ---"));
  unsigned long t_mid =
      measureUpdateRate(MAX44009_INTEGRATION_100MS, 200, 1000);
  Serial.print(F("Median update time: "));
  Serial.print(t_mid);
  Serial.println(F(" ms"));
  test(F("100ms: 30-400ms"), t_mid >= 30 && t_mid <= 400);

  // --- 800ms (3 runs, median) ---
  Serial.println(F("\n--- 800ms integration ---"));
  unsigned long t_slow =
      measureUpdateRate(MAX44009_INTEGRATION_800MS, 1000, 2000);
  Serial.print(F("Median update time: "));
  Serial.print(t_slow);
  Serial.println(F(" ms"));
  test(F("800ms: 400-1500ms"), t_slow >= 400 && t_slow <= 1500);

  // --- Ordering is the key test ---
  Serial.println(F("\n--- Timing order ---"));
  Serial.print(F("6.25ms="));
  Serial.print(t_fast);
  Serial.print(F("  100ms="));
  Serial.print(t_mid);
  Serial.print(F("  800ms="));
  Serial.println(t_slow);
  test(F("6.25ms < 100ms"), t_fast < t_mid);
  test(F("100ms < 800ms"), t_mid < t_slow);

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
  Wire.beginTransmission(0x4A);
  Wire.write(0x03);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)0x4A, (uint8_t)2);
  uint8_t h = Wire.read();
  uint8_t l = Wire.read();
  return ((uint16_t)h << 8) | l;
}

unsigned long measureUpdateRate(max44009_integration_time_t intTime,
                                unsigned long settleMs,
                                unsigned long timeoutMs) {
  unsigned long times[3];

  for (int run = 0; run < 3; run++) {
    max44009.setIntegrationTime(intTime);
    delay(settleMs);

    uint16_t initial = readRawLux();
    bool currentCDR = max44009.getCurrentDivisionRatio();
    max44009.setCurrentDivisionRatio(!currentCDR);

    unsigned long start = millis();
    while ((millis() - start) < timeoutMs) {
      uint16_t raw = readRawLux();
      if (raw != initial) {
        times[run] = millis() - start;
        break;
      }
      delay(1);
    }
    if ((millis() - start) >= timeoutMs)
      times[run] = 0;

    max44009.setCurrentDivisionRatio(currentCDR);
    delay(settleMs); // settle before next run
  }

  // Sort and return median
  for (int i = 0; i < 2; i++) {
    for (int j = i + 1; j < 3; j++) {
      if (times[j] < times[i]) {
        unsigned long tmp = times[i];
        times[i] = times[j];
        times[j] = tmp;
      }
    }
  }
  return times[1]; // median
}
