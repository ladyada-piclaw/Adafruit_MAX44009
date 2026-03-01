/*!
 * @file Adafruit_MAX44009.cpp
 *
 * @mainpage Adafruit MAX44009 Ambient Light Sensor Library
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's MAX44009 driver for the Arduino
 * platform. It is designed specifically to work with the MAX44009 ambient
 * light sensor.
 *
 * The MAX44009 is an ultra-low-power ambient light sensor with a wide
 * dynamic range from 0.045 lux to 188,000 lux.
 *
 * @section dependencies Dependencies
 *
 * This library depends on <a
 * href="https://github.com/adafruit/Adafruit_BusIO">Adafruit BusIO</a>.
 *
 * @section author Author
 *
 * Written by Adafruit Industries.
 *
 * @section license License
 *
 * MIT license, all text here must be included in any redistribution.
 */

#include "Adafruit_MAX44009.h"

/**
 * @brief Construct a new Adafruit_MAX44009 object
 */
Adafruit_MAX44009::Adafruit_MAX44009() {
  _i2c_dev = nullptr;
  _overrange = false;
}

/**
 * @brief Destroy the Adafruit_MAX44009 object
 */
Adafruit_MAX44009::~Adafruit_MAX44009() {
  if (_i2c_dev) {
    delete _i2c_dev;
  }
}

/**
 * @brief Initialize the MAX44009 sensor
 * @param addr I2C address (default 0x4A)
 * @param wire Pointer to TwoWire instance (default &Wire)
 * @return true if initialization was successful, false otherwise
 */
bool Adafruit_MAX44009::begin(uint8_t addr, TwoWire* wire) {
  if (_i2c_dev) {
    delete _i2c_dev;
  }
  _i2c_dev = new Adafruit_I2CDevice(addr, wire);

  if (!_i2c_dev->begin()) {
    return false;
  }

  // Verify communication by reading CONFIG register
  // Default value should be 0x03 (TIM=011, CDR=0, MANUAL=0, CONT=0)
  Adafruit_BusIO_Register config_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_CONFIG, 1);
  uint8_t config_val;
  if (!config_reg.read(&config_val)) {
    return false;
  }

  return true;
}

/**
 * @brief Read lux value with full 12-bit resolution
 *
 * Uses a sequential 2-byte read starting at LUX_HIGH to ensure consistent
 * data. The sensor disables internal register updates during I2C transactions.
 *
 * @return Lux value as float, or NAN if sensor is in overrange condition
 */
float Adafruit_MAX44009::readLux() {
  uint8_t buffer[2];
  uint8_t reg = MAX44009_REG_LUX_HIGH;

  // Sequential read: register address, then 2 bytes (repeated START, no STOP)
  if (!_i2c_dev->write_then_read(&reg, 1, buffer, 2)) {
    _overrange = false;
    return NAN;
  }

  return _luxFromRegisters(buffer[0], buffer[1]);
}

/**
 * @brief Read lux value using only the high byte (faster, lower resolution)
 *
 * Uses only 4-bit mantissa for faster reads when high precision is not needed.
 *
 * @return Lux value as float, or NAN if sensor is in overrange condition
 */
float Adafruit_MAX44009::readLuxFast() {
  Adafruit_BusIO_Register lux_high_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_LUX_HIGH, 1);

  uint8_t lux_high;
  if (!lux_high_reg.read(&lux_high)) {
    _overrange = false;
    return NAN;
  }

  uint8_t exponent = (lux_high >> 4) & 0x0F;

  // Check for overrange (exponent 0xF)
  if (exponent == 0x0F) {
    _overrange = true;
    return NAN;
  }

  _overrange = false;
  uint8_t mantissa = lux_high & 0x0F;

  // Quick formula: 2^exponent * mantissa * 0.72
  return (float)(1 << exponent) * (float)mantissa * 0.72;
}

/**
 * @brief Check if the last reading was an overrange condition
 * @return true if sensor was saturated, false otherwise
 */
bool Adafruit_MAX44009::isOverrange() {
  return _overrange;
}

/**
 * @brief Set the integration time
 *
 * Note: Integration times 50ms and faster require manual mode to be enabled.
 * In auto mode, only 800ms, 400ms, 200ms, and 100ms are available.
 *
 * @param time Integration time setting
 */
void Adafruit_MAX44009::setIntegrationTime(max44009_integration_time_t time) {
  Adafruit_BusIO_Register config_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_CONFIG, 1);
  Adafruit_BusIO_RegisterBits tim_bits =
      Adafruit_BusIO_RegisterBits(&config_reg, 3, 0);
  tim_bits.write(time);
}

/**
 * @brief Get the current integration time setting
 * @return Current integration time
 */
max44009_integration_time_t Adafruit_MAX44009::getIntegrationTime() {
  Adafruit_BusIO_Register config_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_CONFIG, 1);
  Adafruit_BusIO_RegisterBits tim_bits =
      Adafruit_BusIO_RegisterBits(&config_reg, 3, 0);
  return (max44009_integration_time_t)tim_bits.read();
}

/**
 * @brief Set the operating mode
 *
 * Controls the CONT (bit 7) and MANUAL (bit 6) bits of the CONFIG register.
 *
 * @param mode Operating mode
 */
void Adafruit_MAX44009::setMode(max44009_mode_t mode) {
  Adafruit_BusIO_Register config_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_CONFIG, 1);
  Adafruit_BusIO_RegisterBits mode_bits =
      Adafruit_BusIO_RegisterBits(&config_reg, 2, 6);
  // Mode enum has bits in positions 7:6, shift down to get 2-bit value
  mode_bits.write(mode >> 6);
}

/**
 * @brief Get the current operating mode
 * @return Current operating mode
 */
max44009_mode_t Adafruit_MAX44009::getMode() {
  Adafruit_BusIO_Register config_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_CONFIG, 1);
  Adafruit_BusIO_RegisterBits mode_bits =
      Adafruit_BusIO_RegisterBits(&config_reg, 2, 6);
  // Read 2-bit value and shift up to match enum
  return (max44009_mode_t)(mode_bits.read() << 6);
}

/**
 * @brief Set the current division ratio
 *
 * When enabled, only 1/8 of the photodiode current is used, allowing
 * measurements in very high brightness conditions.
 *
 * @param divide true to divide by 8, false for full current
 */
void Adafruit_MAX44009::setCurrentDivisionRatio(bool divide) {
  Adafruit_BusIO_Register config_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_CONFIG, 1);
  Adafruit_BusIO_RegisterBits cdr_bit =
      Adafruit_BusIO_RegisterBits(&config_reg, 1, 3);
  cdr_bit.write(divide ? 1 : 0);
}

/**
 * @brief Get the current division ratio setting
 * @return true if current is divided by 8, false for full current
 */
bool Adafruit_MAX44009::getCurrentDivisionRatio() {
  Adafruit_BusIO_Register config_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_CONFIG, 1);
  Adafruit_BusIO_RegisterBits cdr_bit =
      Adafruit_BusIO_RegisterBits(&config_reg, 1, 3);
  return cdr_bit.read() != 0;
}

/**
 * @brief Enable or disable the interrupt
 * @param enable true to enable interrupts, false to disable
 */
void Adafruit_MAX44009::enableInterrupt(bool enable) {
  Adafruit_BusIO_Register int_enable_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_INT_ENABLE, 1);
  Adafruit_BusIO_RegisterBits inte_bit =
      Adafruit_BusIO_RegisterBits(&int_enable_reg, 1, 0);
  inte_bit.write(enable ? 1 : 0);
}

/**
 * @brief Check if interrupts are enabled
 * @return true if interrupts are enabled, false otherwise
 */
bool Adafruit_MAX44009::isInterruptEnabled() {
  Adafruit_BusIO_Register int_enable_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_INT_ENABLE, 1);
  Adafruit_BusIO_RegisterBits inte_bit =
      Adafruit_BusIO_RegisterBits(&int_enable_reg, 1, 0);
  return inte_bit.read() != 0;
}

/**
 * @brief Get and clear the interrupt status
 *
 * Note: Reading this register clears the interrupt. If you need to check
 * the status without clearing, consider checking the INT pin state instead.
 *
 * @return true if an interrupt event occurred, false otherwise
 */
bool Adafruit_MAX44009::getInterruptStatus() {
  Adafruit_BusIO_Register int_status_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_INT_STATUS, 1);
  Adafruit_BusIO_RegisterBits ints_bit =
      Adafruit_BusIO_RegisterBits(&int_status_reg, 1, 0);
  return ints_bit.read() != 0;
}

/**
 * @brief Set the upper threshold for interrupt generation
 * @param lux Upper threshold lux value
 */
void Adafruit_MAX44009::setUpperThreshold(float lux) {
  Adafruit_BusIO_Register thresh_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_THRESH_UPPER, 1);
  thresh_reg.write(_luxToThreshold(lux));
}

/**
 * @brief Get the upper threshold value
 * @return Upper threshold in lux
 */
float Adafruit_MAX44009::getUpperThreshold() {
  Adafruit_BusIO_Register thresh_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_THRESH_UPPER, 1);
  uint8_t val;
  thresh_reg.read(&val);
  return _thresholdToLux(val);
}

/**
 * @brief Set the lower threshold for interrupt generation
 * @param lux Lower threshold lux value
 */
void Adafruit_MAX44009::setLowerThreshold(float lux) {
  Adafruit_BusIO_Register thresh_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_THRESH_LOWER, 1);
  thresh_reg.write(_luxToThreshold(lux));
}

/**
 * @brief Get the lower threshold value
 * @return Lower threshold in lux
 */
float Adafruit_MAX44009::getLowerThreshold() {
  Adafruit_BusIO_Register thresh_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_THRESH_LOWER, 1);
  uint8_t val;
  thresh_reg.read(&val);
  return _thresholdToLux(val);
}

/**
 * @brief Set the threshold timer value
 *
 * The interrupt will only trigger if the lux value remains outside the
 * threshold window for longer than (timer * 100ms).
 *
 * @param value Timer value (0-255, time = value * 100ms)
 */
void Adafruit_MAX44009::setThresholdTimer(uint8_t value) {
  Adafruit_BusIO_Register timer_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_THRESH_TIMER, 1);
  timer_reg.write(value);
}

/**
 * @brief Get the threshold timer value
 * @return Timer value (time = value * 100ms)
 */
uint8_t Adafruit_MAX44009::getThresholdTimer() {
  Adafruit_BusIO_Register timer_reg =
      Adafruit_BusIO_Register(_i2c_dev, MAX44009_REG_THRESH_TIMER, 1);
  uint8_t val;
  timer_reg.read(&val);
  return val;
}

/**
 * @brief Convert raw register values to lux
 * @param high High byte (LUX_HIGH register)
 * @param low Low byte (LUX_LOW register)
 * @return Lux value, or NAN if overrange
 */
float Adafruit_MAX44009::_luxFromRegisters(uint8_t high, uint8_t low) {
  uint8_t exponent = (high >> 4) & 0x0F;

  // Check for overrange (exponent 0xF)
  if (exponent == 0x0F) {
    _overrange = true;
    return NAN;
  }

  _overrange = false;

  // Full 8-bit mantissa: upper 4 bits from LUX_HIGH, lower 4 from LUX_LOW
  uint8_t mantissa = ((high & 0x0F) << 4) | (low & 0x0F);

  // Lux = 2^exponent * mantissa * 0.045
  return (float)(1 << exponent) * (float)mantissa * MAX44009_LUX_MULTIPLIER;
}

/**
 * @brief Convert lux value to threshold register format
 *
 * Threshold registers use only the upper 4 bits of mantissa.
 *
 * @param lux Lux value to convert
 * @return 8-bit threshold register value
 */
uint8_t Adafruit_MAX44009::_luxToThreshold(float lux) {
  if (lux <= 0) {
    return 0x00;
  }

  // Find the exponent
  uint8_t exponent = 0;
  float divisor = MAX44009_LUX_MULTIPLIER;

  while (exponent < 14) {
    float max_for_exp =
        (float)(1 << exponent) * 255.0 * MAX44009_LUX_MULTIPLIER;
    if (lux <= max_for_exp) {
      break;
    }
    exponent++;
  }

  // Calculate mantissa
  float mantissa_f = lux / ((float)(1 << exponent) * MAX44009_LUX_MULTIPLIER);
  uint8_t mantissa = (uint8_t)mantissa_f;

  // Threshold uses only upper 4 bits of mantissa
  uint8_t mantissa_upper = (mantissa >> 4) & 0x0F;

  return (exponent << 4) | mantissa_upper;
}

/**
 * @brief Convert threshold register value to lux
 *
 * Threshold registers use only the upper 4 bits of mantissa.
 * Lower 4 bits are implicitly 0xF (15).
 *
 * @param threshold 8-bit threshold register value
 * @return Lux value
 */
float Adafruit_MAX44009::_thresholdToLux(uint8_t threshold) {
  uint8_t exponent = (threshold >> 4) & 0x0F;
  uint8_t mantissa_upper = threshold & 0x0F;

  // Lower 4 bits are implicitly 0xF
  uint8_t full_mantissa = (mantissa_upper << 4) | 0x0F;

  return (float)(1 << exponent) * (float)full_mantissa *
         MAX44009_LUX_MULTIPLIER;
}
