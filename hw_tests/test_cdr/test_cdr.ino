/*!
 * @file test_cdr.ino
 *
 * Hardware test: Verify Current Division Ratio affects raw ADC encoding.
 * CDR=1 passes 1/8 of photodiode current to ADC. The chip compensates
 * by shifting exponent/mantissa, so the encoded lux stays the same but
 * the raw register values change. CDR=1 should show a higher exponent
 * and/or lower mantissa than CDR=0 for the same light level.
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
  delay(250);
  uint16_t rawFull = readRawLux();
  float luxFull = max44009.readLux();
  uint8_t expFull = (rawFull >> 12) & 0x0F;
  uint8_t mantFull = ((rawFull >> 4) & 0xF0) | (rawFull & 0x0F);
  Serial.print(F("Raw: 0x"));
  Serial.print(rawFull, HEX);
  Serial.print(F("  exp="));
  Serial.print(expFull);
  Serial.print(F(" mant="));
  Serial.print(mantFull);
  Serial.print(F("  Lux: "));
  Serial.println(luxFull);
  test(F("Valid reading"), !isnan(luxFull) && luxFull > 0);

  // --- Read with divided current (CDR=1) ---
  Serial.println(F("\n--- CDR=1 (1/8 current) ---"));
  max44009.setCurrentDivisionRatio(true);
  test(F("CDR set to true"), max44009.getCurrentDivisionRatio() == true);
  delay(250);
  uint16_t rawDiv = readRawLux();
  float luxDiv = max44009.readLux();
  uint8_t expDiv = (rawDiv >> 12) & 0x0F;
  uint8_t mantDiv = ((rawDiv >> 4) & 0xF0) | (rawDiv & 0x0F);
  Serial.print(F("Raw: 0x"));
  Serial.print(rawDiv, HEX);
  Serial.print(F("  exp="));
  Serial.print(expDiv);
  Serial.print(F(" mant="));
  Serial.print(mantDiv);
  Serial.print(F("  Lux: "));
  Serial.println(luxDiv);
  test(F("Valid reading"), !isnan(luxDiv) && luxDiv > 0);

  // --- Verify CDR changes the encoding ---
  Serial.println(F("\n--- Hardware verification ---"));

  // Raw registers must differ
  test(F("Raw registers differ"), rawFull != rawDiv);

  // CDR=1 reduces photodiode current, so the chip compensates:
  // higher exponent and/or lower mantissa
  test(F("CDR=1 exponent >= CDR=0"), expDiv >= expFull);
  test(F("CDR=1 mantissa <= CDR=0"), mantDiv <= mantFull);

  // Both should still read the same lux (chip compensates)
  float luxDiff = luxFull - luxDiv;
  if (luxDiff < 0)
    luxDiff = -luxDiff;
  Serial.print(F("Lux difference: "));
  Serial.println(luxDiff);
  test(F("Lux values match (chip compensates)"), luxDiff < 1.0);

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
  Adafruit_I2CDevice i2c(0x4A);
  Adafruit_BusIO_Register lux_reg(&i2c, 0x03, 2, MSBFIRST);
  uint16_t raw;
  lux_reg.read(&raw);
  return raw;
}
