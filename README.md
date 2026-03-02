# Adafruit MAX44009 Library [![Build Status](https://github.com/adafruit/Adafruit_MAX44009/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_MAX44009/actions)

Arduino library for the MAX44009 ambient light sensor.

## Description

The MAX44009 is an ultra-low-power ambient light sensor from Analog Devices (formerly Maxim). It features:

- **Wide lux range:** 0.045 lux to 188,000 lux
- **Ultra-low power:** 0.65µA operating current
- **Human-eye spectral response** with IR/UV blocking
- **Automatic gain and integration time adjustment**
- **Interrupt output** for threshold crossing
- **50/60Hz flicker rejection** for indoor lighting
- **I2C interface** at address 0x4A (default) or 0x4B

## Installation

This library is available through the Arduino Library Manager. Search for "Adafruit MAX44009".

### Dependencies

- [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO)

## Usage

```cpp
#include <Adafruit_MAX44009.h>

Adafruit_MAX44009 max44009;

void setup() {
  Serial.begin(115200);
  if (!max44009.begin()) {
    Serial.println("Could not find MAX44009!");
    while (1) delay(10);
  }
}

void loop() {
  float lux = max44009.readLux();
  Serial.print("Lux: ");
  Serial.println(lux);
  delay(1000);
}
```

## Operating Modes

| Mode | Description |
|------|-------------|
| `MAX44009_MODE_DEFAULT` | Auto-ranging, measures every 800ms (lowest power) |
| `MAX44009_MODE_CONTINUOUS` | Auto-ranging, fast updates |
| `MAX44009_MODE_MANUAL` | Manual gain/time, 800ms cycle |
| `MAX44009_MODE_MANUAL_CONTINUOUS` | Manual gain/time, fast updates |

## Examples

- **max44009_simpletest** — Basic auto-ranging lux reading
- **max44009_interrupt** — Threshold interrupt configuration
- **max44009_fulltest** — Exercises all features

## Documentation

[API documentation](https://adafruit.github.io/Adafruit_MAX44009/html/index.html) is available via Doxygen.

## License

BSD License — see [license.txt](license.txt) for details.
