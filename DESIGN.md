# MAX44009 Ambient Light Sensor - Design Document

## Chip Overview

| Parameter | Value |
|-----------|-------|
| **Part Number** | MAX44009 (Analog Devices, formerly Maxim) |
| **Package** | 6-pin UTDFN-Opto (2mm × 2mm × 0.6mm) |
| **Interface** | I²C (up to 400kHz) |
| **Supply Voltage** | 1.7V to 3.6V |
| **Operating Current** | 0.65µA typical |
| **Temperature Range** | -40°C to +85°C |
| **Lux Range** | 0.045 lux to 188,000 lux |
| **ADC Resolution** | 22-bit effective dynamic range |

### I²C Addresses

| A0 Pin | 7-bit Address | 8-bit Write | 8-bit Read |
|--------|---------------|-------------|------------|
| GND    | **0x4A**      | 0x94        | 0x95       |
| VCC    | **0x4B**      | 0x96        | 0x97       |

### Key Features

- Ultra-low power: 0.65µA operating, industry lowest
- Wide dynamic range: 0.045 lux (dark) to 188,000 lux (direct sunlight)
- Human-eye spectral response with IR/UV blocking
- Automatic gain/integration time adjustment
- Interrupt output for threshold crossing
- 50/60Hz rejection for indoor lighting

---

## Register Map

| Addr | Name | R/W | Default | Description |
|------|------|-----|---------|-------------|
| 0x00 | INT_STATUS | R | 0x00 | Interrupt status |
| 0x01 | INT_ENABLE | R/W | 0x00 | Interrupt enable |
| 0x02 | CONFIG | R/W | 0x03 | Configuration |
| 0x03 | LUX_HIGH | R | 0x00 | Lux reading high byte |
| 0x04 | LUX_LOW | R | 0x00 | Lux reading low byte |
| 0x05 | THRESH_UPPER | R/W | 0xFF | Upper threshold |
| 0x06 | THRESH_LOWER | R/W | 0x00 | Lower threshold |
| 0x07 | THRESH_TIMER | R/W | 0xFF | Threshold timer |

---

## Bit Field Definitions

### Register 0x00 - INT_STATUS (Read-Only)

| Bit | Name | Width | Description |
|-----|------|-------|-------------|
| 7:1 | — | 7 | Reserved |
| 0 | INTS | 1 | Interrupt status (cleared on read) |

```cpp
Adafruit_BusIO_Register INT_STATUS = Adafruit_BusIO_Register(i2c_dev, 0x00, 1);
Adafruit_BusIO_RegisterBits INTS = Adafruit_BusIO_RegisterBits(&INT_STATUS, 1, 0);
```

**INTS Values:**
- 0 = No interrupt event occurred
- 1 = Light outside threshold window for longer than timer period

**Clearing:** Read INT_STATUS register OR write 0 to INTE bit.

---

### Register 0x01 - INT_ENABLE (R/W)

| Bit | Name | Width | Description |
|-----|------|-------|-------------|
| 7:1 | — | 7 | Reserved |
| 0 | INTE | 1 | Interrupt enable |

```cpp
Adafruit_BusIO_Register INT_ENABLE = Adafruit_BusIO_Register(i2c_dev, 0x01, 1);
Adafruit_BusIO_RegisterBits INTE = Adafruit_BusIO_RegisterBits(&INT_ENABLE, 1, 0);
```

**INTE Values:**
- 0 = Interrupts disabled (INT pin high-impedance, INTS not set)
- 1 = Interrupts enabled (INT pin pulled low on event, INTS set)

---

### Register 0x02 - CONFIG (R/W)

| Bit | Name | Width | Description |
|-----|------|-------|-------------|
| 7 | CONT | 1 | Continuous mode |
| 6 | MANUAL | 1 | Manual configuration mode |
| 5:4 | — | 2 | Reserved |
| 3 | CDR | 1 | Current division ratio |
| 2:0 | TIM | 3 | Integration time |

```cpp
Adafruit_BusIO_Register CONFIG = Adafruit_BusIO_Register(i2c_dev, 0x02, 1);
Adafruit_BusIO_RegisterBits CONT   = Adafruit_BusIO_RegisterBits(&CONFIG, 1, 7);
Adafruit_BusIO_RegisterBits MANUAL = Adafruit_BusIO_RegisterBits(&CONFIG, 1, 6);
Adafruit_BusIO_RegisterBits CDR    = Adafruit_BusIO_RegisterBits(&CONFIG, 1, 3);
Adafruit_BusIO_RegisterBits TIM    = Adafruit_BusIO_RegisterBits(&CONFIG, 3, 0);
```

**CONT (Continuous Mode):**
- 0 = Default mode (measures every 800ms, lowest power)
- 1 = Continuous mode (measures as fast as integration time allows)

**MANUAL (Manual Configuration):**
- 0 = Automatic mode (CDR and TIM are read-only, auto-adjusted)
- 1 = Manual mode (CDR and TIM can be programmed)

**CDR (Current Division Ratio):**
- 0 = Full photodiode current to ADC (normal)
- 1 = 1/8 of photodiode current to ADC (high-brightness mode)

---

### Integration Time Enum

```cpp
typedef enum {
  MAX44009_INTEGRATION_800MS  = 0b000,  // 800ms - best low-light sensitivity
  MAX44009_INTEGRATION_400MS  = 0b001,  // 400ms
  MAX44009_INTEGRATION_200MS  = 0b010,  // 200ms  
  MAX44009_INTEGRATION_100MS  = 0b011,  // 100ms - best high-brightness (default)
  MAX44009_INTEGRATION_50MS   = 0b100,  // 50ms  - manual mode only
  MAX44009_INTEGRATION_25MS   = 0b101,  // 25ms  - manual mode only
  MAX44009_INTEGRATION_12_5MS = 0b110,  // 12.5ms - manual mode only
  MAX44009_INTEGRATION_6_25MS = 0b111,  // 6.25ms - manual mode only
} max44009_integration_time_t;

typedef enum {
  MAX44009_MODE_DEFAULT           = 0x00,  // CONT=0, MANUAL=0 — auto-ranging, 800ms cycle
  MAX44009_MODE_CONTINUOUS        = 0x80,  // CONT=1, MANUAL=0 — auto-ranging, fast updates
  MAX44009_MODE_MANUAL            = 0x40,  // CONT=0, MANUAL=1 — manual gain/time, 800ms cycle
  MAX44009_MODE_MANUAL_CONTINUOUS = 0xC0,  // CONT=1, MANUAL=1 — manual gain/time, fast updates
} max44009_mode_t;
```

| TIM | Integration Time | Notes |
|-----|------------------|-------|
| 000 | 800ms | Best for low-light, available in auto mode |
| 001 | 400ms | Available in auto mode |
| 010 | 200ms | Available in auto mode |
| 011 | 100ms | Default, best for high-brightness |
| 100 | 50ms | Manual mode only |
| 101 | 25ms | Manual mode only |
| 110 | 12.5ms | Manual mode only |
| 111 | 6.25ms | Manual mode only |

---

### Register 0x03 - LUX_HIGH (Read-Only)

| Bit | Name | Width | Description |
|-----|------|-------|-------------|
| 7:4 | EXPONENT | 4 | Lux exponent E[3:0] |
| 3:0 | MANTISSA_H | 4 | Lux mantissa upper M[7:4] |

```cpp
Adafruit_BusIO_Register LUX_HIGH = Adafruit_BusIO_Register(i2c_dev, 0x03, 1);
Adafruit_BusIO_RegisterBits EXPONENT   = Adafruit_BusIO_RegisterBits(&LUX_HIGH, 4, 4);
Adafruit_BusIO_RegisterBits MANTISSA_H = Adafruit_BusIO_RegisterBits(&LUX_HIGH, 4, 0);
```

**Exponent Values:**
- 0x0 to 0xE (0-14): Valid exponent
- 0xF (15): **Overrange condition** (sensor saturated)

---

### Register 0x04 - LUX_LOW (Read-Only)

| Bit | Name | Width | Description |
|-----|------|-------|-------------|
| 7:4 | — | 4 | Reserved |
| 3:0 | MANTISSA_L | 4 | Lux mantissa lower M[3:0] |

```cpp
Adafruit_BusIO_Register LUX_LOW = Adafruit_BusIO_Register(i2c_dev, 0x04, 1);
Adafruit_BusIO_RegisterBits MANTISSA_L = Adafruit_BusIO_RegisterBits(&LUX_LOW, 4, 0);
```

---

### Register 0x05 - THRESH_UPPER (R/W)

| Bit | Name | Width | Description |
|-----|------|-------|-------------|
| 7:4 | UPPER_EXP | 4 | Upper threshold exponent |
| 3:0 | UPPER_MANT | 4 | Upper threshold mantissa (upper 4 bits) |

```cpp
Adafruit_BusIO_Register THRESH_UPPER = Adafruit_BusIO_Register(i2c_dev, 0x05, 1);
Adafruit_BusIO_RegisterBits UPPER_EXP  = Adafruit_BusIO_RegisterBits(&THRESH_UPPER, 4, 4);
Adafruit_BusIO_RegisterBits UPPER_MANT = Adafruit_BusIO_RegisterBits(&THRESH_UPPER, 4, 0);
```

---

### Register 0x06 - THRESH_LOWER (R/W)

| Bit | Name | Width | Description |
|-----|------|-------|-------------|
| 7:4 | LOWER_EXP | 4 | Lower threshold exponent |
| 3:0 | LOWER_MANT | 4 | Lower threshold mantissa (upper 4 bits) |

```cpp
Adafruit_BusIO_Register THRESH_LOWER = Adafruit_BusIO_Register(i2c_dev, 0x06, 1);
Adafruit_BusIO_RegisterBits LOWER_EXP  = Adafruit_BusIO_RegisterBits(&THRESH_LOWER, 4, 4);
Adafruit_BusIO_RegisterBits LOWER_MANT = Adafruit_BusIO_RegisterBits(&THRESH_LOWER, 4, 0);
```

---

### Register 0x07 - THRESH_TIMER (R/W)

| Bit | Name | Width | Description |
|-----|------|-------|-------------|
| 7:0 | TIMER | 8 | Threshold timer value |

```cpp
Adafruit_BusIO_Register THRESH_TIMER = Adafruit_BusIO_Register(i2c_dev, 0x07, 1);
```

**Timer Formula:**
```
Time delay = TIMER × 100ms
```

- 0x00 = Immediate interrupt (0ms delay)
- 0x01 = 100ms delay
- 0xFF = 25.5 seconds delay (default)

---

## Lux Calculation

### Full Resolution (12-bit, using both registers)

```cpp
float calculateLux(uint8_t luxHigh, uint8_t luxLow) {
  uint8_t exponent = (luxHigh >> 4) & 0x0F;
  
  // Check for overrange
  if (exponent == 0x0F) {
    return NAN;  // or return a saturated value indicator
  }
  
  uint8_t mantissa = ((luxHigh & 0x0F) << 4) | (luxLow & 0x0F);
  
  return (float)(1 << exponent) * (float)mantissa * 0.045f;
}
```

**Formula:**
```
Lux = 2^exponent × mantissa × 0.045
```

Where:
- `exponent` = bits [7:4] of LUX_HIGH (0-14)
- `mantissa` = bits [3:0] of LUX_HIGH concatenated with bits [3:0] of LUX_LOW (8-bit, 0-255)

### Quick Read (8-bit, high byte only)

For faster reads with reduced resolution:

```cpp
float calculateLuxQuick(uint8_t luxHigh) {
  uint8_t exponent = (luxHigh >> 4) & 0x0F;
  
  if (exponent == 0x0F) {
    return NAN;
  }
  
  uint8_t mantissa = luxHigh & 0x0F;
  
  return (float)(1 << exponent) * (float)mantissa * 0.72f;
}
```

**Quick Formula:**
```
Lux = 2^exponent × mantissa × 0.72
```

### Threshold Lux Calculation

```cpp
float calculateThresholdLux(uint8_t threshReg) {
  uint8_t exponent = (threshReg >> 4) & 0x0F;
  uint8_t mantissa = (threshReg & 0x0F);
  
  // Threshold mantissa is upper 4 bits only, lower 4 bits implicitly 0xF (15)
  uint8_t fullMantissa = (mantissa << 4) | 0x0F;
  
  return (float)(1 << exponent) * (float)fullMantissa * 0.045f;
}
```

---

## Multi-Byte Read Order

**Critical: Use Repeated START when reading LUX_HIGH and LUX_LOW**

The MAX44009 disables internal register updates during I²C read operations. Updates resume on STOP condition. To ensure data consistency:

```cpp
// CORRECT: Use repeated start (no STOP between reads)
i2c_dev->write_then_read(regHigh, 1, buffer, 2);  // Read 2 bytes starting at 0x03

// WRONG: Separate transactions allow update between reads
i2c_dev->write_then_read(&reg03, 1, &luxHigh, 1);  // STOP here
i2c_dev->write_then_read(&reg04, 1, &luxLow, 1);   // Value may have changed!
```

**Register auto-increment:** The MAX44009 supports sequential reads. Point to 0x03 and read 2 bytes to get both LUX_HIGH and LUX_LOW.

---

## Timing Requirements

### I²C Timing (400kHz Fast Mode)

| Parameter | Min | Max | Unit |
|-----------|-----|-----|------|
| Clock frequency | — | 400 | kHz |
| Bus free time (STOP to START) | 1.3 | — | µs |
| Hold time START | 0.6 | — | µs |
| SCL low period | 1.3 | — | µs |
| SCL high period | 0.6 | — | µs |
| Data setup time | 100 | — | ns |
| Data hold time | 0 | 0.9 | µs |
| Setup time STOP | 0.6 | — | µs |
| Spike suppression | 0 | 50 | ns |

### Conversion Timing

| Mode | Integration Time | Measurement Rate |
|------|-----------------|------------------|
| Default (CONT=0) | Any | Every 800ms |
| Continuous (CONT=1) | 6.25ms | Every 6.25ms |
| Continuous (CONT=1) | 100ms | Every 100ms |
| Continuous (CONT=1) | 800ms | Every 800ms |

**ADC Conversion Time:** ~100ms typical (varies by range)

---

## Interrupt Configuration

### Enabling Interrupts

1. Set upper threshold (0x05) and/or lower threshold (0x06)
2. Set threshold timer (0x07) for debounce time
3. Enable interrupts by writing 1 to INTE bit (0x01)

### Interrupt Behavior

- INT pin is open-drain, requires external pull-up
- INT asserts (LOW) when lux exceeds thresholds for timer duration
- Cleared by reading INT_STATUS (0x00) OR writing 0 to INTE

### Threshold Format (for Auto Mode)

For lux levels **below 11.5 lux**: `0000 MMMM` (exponent = 0)

For lux levels **above 11.5 lux**: `EEEE 1MMM` (M7 must be 1)

Maximum threshold code: `1110 1111` = ~188,000 lux

---

## Quirks and Gotchas

### 1. Default Mode Measures Every 800ms
Even with fast integration time (6.25ms), default mode only updates every 800ms. Set CONT=1 for faster updates.

### 2. Manual Mode Required for Fast Integration
Integration times 50ms, 25ms, 12.5ms, and 6.25ms are **only available in manual mode** (MANUAL=1).

### 3. Exponent 0xF = Overrange
An exponent value of 15 indicates the sensor is saturated. Return an error or max value.

### 4. CDR Affects Threshold Interpretation
In manual mode, the CDR and TIM settings affect which exponent ranges are valid for thresholds. See datasheet Table 9.

### 5. Config Register Default = 0x03
POR state sets TIM=011 (100ms), CDR=0, MANUAL=0, CONT=0. This is auto-ranging mode.

### 6. Reading Clears Interrupt
Reading INT_STATUS (0x00) clears the INTS bit. Don't poll this register unless you want to clear pending interrupts.

### 7. Repeated START Essential for Lux Reads
Register updates are blocked during I²C transactions. A STOP between LUX_HIGH and LUX_LOW reads allows an update, causing inconsistent data.

### 8. 50/60Hz Rejection
Built-in rejection for indoor fluorescent lighting works best with 100ms or longer integration times.

---

## Public API Function Map

### Core Functions

```cpp
class Adafruit_MAX44009 {
public:
  // Initialization
  bool begin(uint8_t addr = MAX44009_DEFAULT_ADDRESS, TwoWire *wire = &Wire);
  
  // Lux reading
  float readLux();           // Full resolution (12-bit mantissa)
  float readLuxFast();       // Quick read (4-bit mantissa)
  bool  isOverrange();       // Check if sensor saturated
  
  // Configuration
  void setIntegrationTime(max44009_integration_time_t time);
  max44009_integration_time_t getIntegrationTime();
  
  void setMode(max44009_mode_t mode);
  max44009_mode_t getMode();





  
  void setCurrentDivisionRatio(bool divide);  // true = divide by 8
  bool getCurrentDivisionRatio();
  
  // Interrupts
  void enableInterrupt(bool enable);
  bool isInterruptEnabled();
  bool getInterruptStatus();  // Clears interrupt on read
  
  // Thresholds
  void setUpperThreshold(float lux);
  float getUpperThreshold();
  
  void setLowerThreshold(float lux);
  float getLowerThreshold();
  
  void setThresholdTimer(uint8_t value);  // value × 100ms
  uint8_t getThresholdTimer();

private:
  Adafruit_I2CDevice *i2c_dev;
  
  float _luxFromRegisters(uint8_t high, uint8_t low);
  uint8_t _luxToThreshold(float lux);
};
```

### Constants

```cpp
#define MAX44009_DEFAULT_ADDRESS  0x4A  // A0 = GND
#define MAX44009_ALT_ADDRESS      0x4B  // A0 = VCC

// Register addresses
#define MAX44009_REG_INT_STATUS   0x00
#define MAX44009_REG_INT_ENABLE   0x01
#define MAX44009_REG_CONFIG       0x02
#define MAX44009_REG_LUX_HIGH     0x03
#define MAX44009_REG_LUX_LOW      0x04
#define MAX44009_REG_THRESH_UPPER 0x05
#define MAX44009_REG_THRESH_LOWER 0x06
#define MAX44009_REG_THRESH_TIMER 0x07

// Lux calculation constants
#define MAX44009_LUX_MULTIPLIER   0.045f  // Full resolution
#define MAX44009_LUX_MULTIPLIER_FAST 0.72f  // Quick read
```

---

## Hardware Test Plan

1. **Basic Communication:** Read CONFIG register, verify default 0x03
2. **Lux Reading:** Read in ambient light, verify reasonable value
3. **Manual Mode:** Set manual mode, change integration time, verify it changes
4. **Continuous Mode:** Enable, verify faster updates
5. **Thresholds:** Set low threshold above ambient, verify interrupt triggers
6. **Overrange:** Point at bright light source, check for exponent=0xF
