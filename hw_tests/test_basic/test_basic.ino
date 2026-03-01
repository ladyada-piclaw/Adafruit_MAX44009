/*!
 * @file test_basic.ino
 *
 * Hardware test: Basic I2C communication and config register default.
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

  Serial.println(F("=== MAX44009 Basic Test ==="));
  Serial.println();

  // Test 1: I2C communication / begin()
  bool beginResult = max44009.begin();
  test(F("I2C begin"), beginResult);

  if (!beginResult) {
    Serial.println(F("Cannot continue without I2C communication"));
    goto summary;
  }

  // Test 2: Read config register default (should be 0x03)
  // We need to read raw register to check default
  {
    Wire.beginTransmission(0x4A);
    Wire.write(0x02);  // CONFIG register
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)0x4A, (uint8_t)1);
    uint8_t config = Wire.read();
    Serial.print(F("Config register value: 0x"));
    Serial.println(config, HEX);
    test(F("Config default 0x03"), config == 0x03);
  }

  // Test 3: Verify mode is DEFAULT after begin
  {
    max44009_mode_t mode = max44009.getMode();
    test(F("Mode is DEFAULT"), mode == MAX44009_MODE_DEFAULT);
  }

  // Test 4: Verify integration time is 100ms (default TIM=011)
  {
    max44009_integration_time_t intTime = max44009.getIntegrationTime();
    test(F("Integration time is 100ms"), intTime == MAX44009_INTEGRATION_100MS);
  }

  // Test 5: Verify CDR is 0 (full current)
  {
    bool cdr = max44009.getCurrentDivisionRatio();
    test(F("CDR is false (full current)"), cdr == false);
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
