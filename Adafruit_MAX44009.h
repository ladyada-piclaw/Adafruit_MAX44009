/*!
 * @file Adafruit_MAX44009.h
 *
 * @brief Arduino library for the MAX44009 ambient light sensor.
 *
 * This is a library for the MAX44009 ambient light sensor with I2C interface.
 * The MAX44009 provides wide dynamic range (0.045 to 188,000 lux) with
 * ultra-low power consumption.
 *
 * Written by Adafruit Industries.
 *
 * MIT license, all text here must be included in any redistribution.
 */

#ifndef ADAFRUIT_MAX44009_H
#define ADAFRUIT_MAX44009_H

#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Arduino.h>
#include <Wire.h>

/** Default I2C address (A0 = GND) */
#define MAX44009_DEFAULT_ADDRESS 0x4A
/** Alternate I2C address (A0 = VCC) */
#define MAX44009_ALT_ADDRESS 0x4B

/** @name Register addresses
 *  @{
 */
#define MAX44009_REG_INT_STATUS 0x00   /**< Interrupt status register */
#define MAX44009_REG_INT_ENABLE 0x01   /**< Interrupt enable register */
#define MAX44009_REG_CONFIG 0x02       /**< Configuration register */
#define MAX44009_REG_LUX_HIGH 0x03     /**< Lux reading high byte */
#define MAX44009_REG_LUX_LOW 0x04      /**< Lux reading low byte */
#define MAX44009_REG_THRESH_UPPER 0x05 /**< Upper threshold register */
#define MAX44009_REG_THRESH_LOWER 0x06 /**< Lower threshold register */
#define MAX44009_REG_THRESH_TIMER 0x07 /**< Threshold timer register */
/** @} */

/** Lux calculation multiplier for full resolution */
#define MAX44009_LUX_MULTIPLIER 0.045

/**
 * @brief Integration time settings for the MAX44009
 *
 * In automatic mode (MANUAL=0), only the first four values (800ms-100ms)
 * are available. The shorter integration times (50ms-6.25ms) require
 * manual mode to be enabled.
 */
typedef enum {
  MAX44009_INTEGRATION_800MS = 0b000, /**< 800ms - best low-light sensitivity */
  MAX44009_INTEGRATION_400MS = 0b001, /**< 400ms */
  MAX44009_INTEGRATION_200MS = 0b010, /**< 200ms */
  MAX44009_INTEGRATION_100MS =
      0b011, /**< 100ms - default, best high-brightness */
  MAX44009_INTEGRATION_50MS = 0b100,   /**< 50ms - manual mode only */
  MAX44009_INTEGRATION_25MS = 0b101,   /**< 25ms - manual mode only */
  MAX44009_INTEGRATION_12_5MS = 0b110, /**< 12.5ms - manual mode only */
  MAX44009_INTEGRATION_6_25MS = 0b111, /**< 6.25ms - manual mode only */
} max44009_integration_time_t;

/**
 * @brief Operating mode settings for the MAX44009
 *
 * Controls the CONT (bit 7) and MANUAL (bit 6) bits of the CONFIG register.
 * - DEFAULT: Auto-ranging, measures every 800ms (lowest power)
 * - CONTINUOUS: Auto-ranging, measures as fast as integration time allows
 * - MANUAL: Manual gain/time control, measures every 800ms
 * - MANUAL_CONTINUOUS: Manual gain/time, fast updates
 */
typedef enum {
  MAX44009_MODE_DEFAULT =
      0x00, /**< CONT=0, MANUAL=0 - auto-ranging, 800ms cycle */
  MAX44009_MODE_CONTINUOUS = 0x80, /**< CONT=1, MANUAL=0 - auto-ranging, fast */
  MAX44009_MODE_MANUAL =
      0x40, /**< CONT=0, MANUAL=1 - manual config, 800ms cycle */
  MAX44009_MODE_MANUAL_CONTINUOUS =
      0xC0, /**< CONT=1, MANUAL=1 - manual, fast */
} max44009_mode_t;

/**
 * @brief Class for interfacing with the MAX44009 ambient light sensor
 */
class Adafruit_MAX44009 {
 public:
  Adafruit_MAX44009();
  ~Adafruit_MAX44009();

  bool begin(uint8_t addr = MAX44009_DEFAULT_ADDRESS, TwoWire* wire = &Wire);

  float readLux();
  bool isOverrange();

  void setIntegrationTime(max44009_integration_time_t time);
  max44009_integration_time_t getIntegrationTime();

  void setMode(max44009_mode_t mode);
  max44009_mode_t getMode();

  void setCurrentDivisionRatio(bool divide);
  bool getCurrentDivisionRatio();

  void enableInterrupt(bool enable);
  bool isInterruptEnabled();
  bool getInterruptStatus();

  void setUpperThreshold(float lux);
  float getUpperThreshold();

  void setLowerThreshold(float lux);
  float getLowerThreshold();

  void setThresholdTimer(uint8_t value);
  uint8_t getThresholdTimer();

 private:
  Adafruit_I2CDevice* _i2c_dev; /**< I2C device object */
  bool _overrange;              /**< Last read was overrange */

  uint8_t _luxToThreshold(float lux);
  float _thresholdToLux(uint8_t threshold);
};

#endif // ADAFRUIT_MAX44009_H
