/*!
 * @file max44009_interrupt.ino
 *
 * Example showing interrupt-based threshold detection with the MAX44009.
 * Sets upper and lower thresholds, enables interrupts, and polls status.
 */

#include <Adafruit_MAX44009.h>
#include <Wire.h>

Adafruit_MAX44009 max44009;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("MAX44009 Interrupt Example"));

  if (!max44009.begin()) {
    Serial.println(F("Could not find MAX44009 sensor!"));
    while (1) {
      delay(10);
    }
  }

  Serial.println(F("MAX44009 found!"));

  // Set thresholds - interrupt triggers when lux goes outside this window
  max44009.setLowerThreshold(50.0);   // Below 50 lux
  max44009.setUpperThreshold(500.0);  // Above 500 lux

  Serial.print(F("Lower threshold: "));
  Serial.print(max44009.getLowerThreshold());
  Serial.println(F(" lux"));

  Serial.print(F("Upper threshold: "));
  Serial.print(max44009.getUpperThreshold());
  Serial.println(F(" lux"));

  // Set threshold timer (value * 100ms)
  // 10 = 1 second debounce
  max44009.setThresholdTimer(10);
  Serial.print(F("Threshold timer: "));
  Serial.print(max44009.getThresholdTimer() * 100);
  Serial.println(F(" ms"));

  // Enable interrupts
  max44009.enableInterrupt(true);
  Serial.println(F("Interrupts enabled"));
  Serial.println();
}

void loop() {
  float lux = max44009.readLux();

  Serial.print(F("Lux: "));
  if (isnan(lux)) {
    Serial.print(F("ERROR"));
  } else {
    Serial.print(lux);
  }

  // Check interrupt status (note: reading clears the interrupt!)
  bool interrupt = max44009.getInterruptStatus();
  if (interrupt) {
    Serial.print(F(" - INTERRUPT!"));
  }

  Serial.println();
  delay(500);
}
