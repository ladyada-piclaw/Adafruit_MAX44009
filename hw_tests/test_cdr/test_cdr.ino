/*!
 * @file test_cdr.ino
 *
 * Hardware test: Verify Current Division Ratio affects raw ADC output.
 * CDR=1 passes 1/8 of photodiode current to ADC, so raw registers change
 * even though the lux formula compensates.
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

  Serial.println(F("=== MAX44009 CDR Test (hardware) ==="));
  Serial.println();

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009!"));
    while (1)
      delay(10);
  }
  test(F("Sensor found"), true);

  // Use manual continuous mode to control CDR and get fast updates
  max44009.setMode(MAX44009_MODE_MANUAL_CONTINUOUS);
  max44009.setIntegrationTime(MAX44009_INTEGRATION_100MS);
  delay(200);

  // --- Read with full current (CDR=0) ---
  Serial.println(F("\n--- CDR=0 (full current) ---"));
  max44009.setCurrentDivisionRatio(false);
  test(F("CDR set to false"), max44009.getCurrentDivisionRatio() == false);
  delay(200);
  uint16_t rawFull = readRawLux();
  float luxFull = max44009.readLux();
  Serial.print(F("Raw: 0x"));
  Serial.print(rawFull, HEX);
  Serial.print(F("  Lux: "));
  Serial.println(luxFull);
  test(F("Got valid reading"), !isnan(luxFull) && luxFull > 0);

  // --- Read with divided current (CDR=1) ---
  Serial.println(F("\n--- CDR=1 (1/8 current) ---"));
  max44009.setCurrentDivisionRatio(true);
  test(F("CDR set to true"), max44009.getCurrentDivisionRatio() == true);
  delay(200);
  uint16_t rawDiv = readRawLux();
  float luxDiv = max44009.readLux();
  Serial.print(F("Raw: 0x"));
  Serial.print(rawDiv, HEX);
  Serial.print(F("  Lux: "));
  Serial.println(luxDiv);
  test(F("Got valid reading"), !isnan(luxDiv) && luxDiv > 0);

  // --- Key test: raw registers MUST differ ---
  // The lux formula compensates, so final lux may be similar,
  // but raw ADC values must change when CDR is toggled.
  Serial.println(F("\n--- Hardware verification ---"));
  Serial.print(F("Raw full: 0x"));
  Serial.print(rawFull, HEX);
  Serial.print(F("  Raw div: 0x"));
  Serial.println(rawDiv, HEX);
  test(F("Raw registers differ (CDR affects ADC)"), rawFull != rawDiv);

  // Both lux values should be reasonable (sensor works in both modes)
  test(F("Full current lux reasonable"), luxFull > 1.0 && luxFull < 100000.0);
  test(F("Divided lux reasonable"), luxDiv > 1.0 && luxDiv < 100000.0);

  // Restore default
  max44009.setCurrentDivisionRatio(false);
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
