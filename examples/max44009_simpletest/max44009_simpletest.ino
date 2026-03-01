/*!
 * @file max44009_simpletest.ino
 *
 * Simple example for the MAX44009 ambient light sensor.
 * Reads lux value every second in auto-ranging mode.
 */

#include <Adafruit_MAX44009.h>
#include <Wire.h>

Adafruit_MAX44009 max44009;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("MAX44009 Simple Test"));

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009 sensor!"));
    while (1) {
      delay(10);
    }
  }

  Serial.println(F("MAX44009 found!"));
}

void loop() {
  float lux = max44009.readLux();

  if (max44009.isOverrange()) {
    Serial.println(F("Sensor overrange!"));
  } else if (isnan(lux)) {
    Serial.println(F("Error reading lux"));
  } else {
    Serial.print(F("Lux: "));
    Serial.println(lux);
  }

  delay(1000);
}
