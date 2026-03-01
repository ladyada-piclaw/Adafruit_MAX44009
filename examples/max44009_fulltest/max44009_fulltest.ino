/*!
 * @file max44009_fulltest.ino
 *
 * Comprehensive example exercising all MAX44009 features:
 * modes, integration time, CDR, thresholds, and timer.
 */

#include <Adafruit_MAX44009.h>
#include <Wire.h>

Adafruit_MAX44009 max44009;

void printMode(max44009_mode_t mode) {
  switch (mode) {
    case MAX44009_MODE_DEFAULT:
      Serial.print(F("DEFAULT"));
      break;
    case MAX44009_MODE_CONTINUOUS:
      Serial.print(F("CONTINUOUS"));
      break;
    case MAX44009_MODE_MANUAL:
      Serial.print(F("MANUAL"));
      break;
    case MAX44009_MODE_MANUAL_CONTINUOUS:
      Serial.print(F("MANUAL_CONTINUOUS"));
      break;
  }
}

void printIntegrationTime(max44009_integration_time_t time) {
  switch (time) {
    case MAX44009_INTEGRATION_800MS:
      Serial.print(F("800ms"));
      break;
    case MAX44009_INTEGRATION_400MS:
      Serial.print(F("400ms"));
      break;
    case MAX44009_INTEGRATION_200MS:
      Serial.print(F("200ms"));
      break;
    case MAX44009_INTEGRATION_100MS:
      Serial.print(F("100ms"));
      break;
    case MAX44009_INTEGRATION_50MS:
      Serial.print(F("50ms"));
      break;
    case MAX44009_INTEGRATION_25MS:
      Serial.print(F("25ms"));
      break;
    case MAX44009_INTEGRATION_12_5MS:
      Serial.print(F("12.5ms"));
      break;
    case MAX44009_INTEGRATION_6_25MS:
      Serial.print(F("6.25ms"));
      break;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("MAX44009 Full Test"));
  Serial.println(F("=================="));

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009 sensor!"));
    while (1) {
      delay(10);
    }
  }

  Serial.println(F("MAX44009 found!"));
  Serial.println();

  // Test modes
  Serial.println(F("=== Mode Test ==="));
  max44009_mode_t modes[] = {MAX44009_MODE_DEFAULT, MAX44009_MODE_CONTINUOUS,
                             MAX44009_MODE_MANUAL, MAX44009_MODE_MANUAL_CONTINUOUS};
  for (int i = 0; i < 4; i++) {
    max44009.setMode(modes[i]);
    max44009_mode_t readMode = max44009.getMode();
    Serial.print(F("Set: "));
    printMode(modes[i]);
    Serial.print(F(" -> Read: "));
    printMode(readMode);
    Serial.println(modes[i] == readMode ? F(" [OK]") : F(" [FAIL]"));
  }
  Serial.println();

  // Test integration times (requires manual mode)
  Serial.println(F("=== Integration Time Test ==="));
  max44009.setMode(MAX44009_MODE_MANUAL);  // Required for all integration times
  max44009_integration_time_t times[] = {
      MAX44009_INTEGRATION_800MS,  MAX44009_INTEGRATION_400MS,
      MAX44009_INTEGRATION_200MS,  MAX44009_INTEGRATION_100MS,
      MAX44009_INTEGRATION_50MS,   MAX44009_INTEGRATION_25MS,
      MAX44009_INTEGRATION_12_5MS, MAX44009_INTEGRATION_6_25MS};
  for (int i = 0; i < 8; i++) {
    max44009.setIntegrationTime(times[i]);
    max44009_integration_time_t readTime = max44009.getIntegrationTime();
    Serial.print(F("Set: "));
    printIntegrationTime(times[i]);
    Serial.print(F(" -> Read: "));
    printIntegrationTime(readTime);
    Serial.println(times[i] == readTime ? F(" [OK]") : F(" [FAIL]"));
  }
  Serial.println();

  // Test CDR
  Serial.println(F("=== CDR Test ==="));
  max44009.setCurrentDivisionRatio(false);
  Serial.print(F("CDR=false: "));
  Serial.println(max44009.getCurrentDivisionRatio() == false ? F("[OK]") : F("[FAIL]"));

  max44009.setCurrentDivisionRatio(true);
  Serial.print(F("CDR=true: "));
  Serial.println(max44009.getCurrentDivisionRatio() == true ? F("[OK]") : F("[FAIL]"));

  max44009.setCurrentDivisionRatio(false);  // Reset
  Serial.println();

  // Test thresholds
  Serial.println(F("=== Threshold Test ==="));
  float testLux[] = {10.0, 100.0, 1000.0, 10000.0};
  for (int i = 0; i < 4; i++) {
    max44009.setUpperThreshold(testLux[i]);
    float readLux = max44009.getUpperThreshold();
    Serial.print(F("Upper threshold set: "));
    Serial.print(testLux[i]);
    Serial.print(F(" -> read: "));
    Serial.println(readLux);
  }
  Serial.println();

  // Test timer
  Serial.println(F("=== Timer Test ==="));
  uint8_t testTimers[] = {0, 10, 100, 255};
  for (int i = 0; i < 4; i++) {
    max44009.setThresholdTimer(testTimers[i]);
    uint8_t readTimer = max44009.getThresholdTimer();
    Serial.print(F("Timer set: "));
    Serial.print(testTimers[i]);
    Serial.print(F(" ("));
    Serial.print(testTimers[i] * 100);
    Serial.print(F("ms) -> read: "));
    Serial.print(readTimer);
    Serial.println(testTimers[i] == readTimer ? F(" [OK]") : F(" [FAIL]"));
  }
  Serial.println();

  // Test interrupt enable
  Serial.println(F("=== Interrupt Enable Test ==="));
  max44009.enableInterrupt(true);
  Serial.print(F("Interrupt enabled: "));
  Serial.println(max44009.isInterruptEnabled() ? F("[OK]") : F("[FAIL]"));

  max44009.enableInterrupt(false);
  Serial.print(F("Interrupt disabled: "));
  Serial.println(!max44009.isInterruptEnabled() ? F("[OK]") : F("[FAIL]"));
  Serial.println();

  // Reset to default mode for lux readings
  max44009.setMode(MAX44009_MODE_DEFAULT);
  Serial.println(F("=== Continuous Lux Readings ==="));
}

void loop() {
  float lux = max44009.readLux();
  float luxFast = max44009.readLuxFast();

  Serial.print(F("Lux (full): "));
  if (isnan(lux)) {
    Serial.print(max44009.isOverrange() ? F("OVERRANGE") : F("ERROR"));
  } else {
    Serial.print(lux);
  }

  Serial.print(F("  Lux (fast): "));
  if (isnan(luxFast)) {
    Serial.print(max44009.isOverrange() ? F("OVERRANGE") : F("ERROR"));
  } else {
    Serial.print(luxFast);
  }

  Serial.println();
  delay(1000);
}
