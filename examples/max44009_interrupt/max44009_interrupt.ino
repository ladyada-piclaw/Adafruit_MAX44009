/*
 * Interrupt example for MAX44009 Ambient Light Sensor
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code for
 * Adafruit Industries. MIT license, check license.txt for more information
 *
 * Reads ambient light level, then sets thresholds slightly above and below.
 * Shade or shine the sensor to trigger an interrupt!
 *
 * Wiring: MAX44009 INT pin -> D2 (open-drain, needs external pull-up)
 */

#include <Adafruit_MAX44009.h>

#define INT_PIN 2 // Connect MAX44009 INT pin here

Adafruit_MAX44009 max44009;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  pinMode(INT_PIN, INPUT_PULLUP);

  Serial.println(F("MAX44009 Interrupt Example"));
  Serial.println(F("=========================="));

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009 sensor!"));
    while (1)
      delay(10);
  }

  Serial.println(F("MAX44009 found!"));

  // Read current ambient level
  float ambient = max44009.readLux();
  Serial.print(F("Ambient lux: "));
  Serial.println(ambient);

  // Set thresholds ~25% below and above ambient
  float lower = ambient * 0.75;
  float upper = ambient * 1.25;

  max44009.setLowerThreshold(lower);
  // Range: 0.045 to 188,000 lux
  Serial.print(F("Lower threshold: "));
  Serial.print(max44009.getLowerThreshold());
  Serial.println(F(" lux"));

  max44009.setUpperThreshold(upper);
  // Range: 0.045 to 188,000 lux
  Serial.print(F("Upper threshold: "));
  Serial.print(max44009.getUpperThreshold());
  Serial.println(F(" lux"));

  // Set threshold timer to 100ms (1 = 1 * 100ms)
  max44009.setThresholdTimer(1);
  // 0 = immediate, 255 = 25.5 seconds (default)
  Serial.print(F("Threshold timer: "));
  Serial.print(max44009.getThresholdTimer() * 100);
  Serial.println(F(" ms"));

  // Enable interrupts
  max44009.enableInterrupt(true);
  // INT pin is open-drain, asserts LOW when lux outside threshold window
  Serial.println(F("Interrupts enabled"));

  // Clear any pending interrupt
  max44009.getInterruptStatus();

  Serial.println(F("\nShade or shine the sensor to trigger an interrupt!"));
  Serial.println();
}

void loop() {
  float lux = max44009.readLux();

  // Check the physical INT pin first (LOW = interrupt asserted)
  bool pinLow = (digitalRead(INT_PIN) == LOW);

  // Then check register status (note: reading this clears the interrupt!)
  bool regStatus = max44009.getInterruptStatus();

  if (!isnan(lux)) {
    Serial.print(F("Lux: "));
    Serial.print(lux);

    if (pinLow || regStatus) {
      Serial.print(F(" ** INTERRUPT - pin:"));
      Serial.print(pinLow ? F("LOW") : F("HIGH"));
      Serial.print(F(" reg:"));
      Serial.print(regStatus ? F("SET") : F("CLEAR"));
    }

    Serial.println();
  }
  delay(500);
}
