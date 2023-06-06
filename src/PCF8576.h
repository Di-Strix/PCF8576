#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <map>
#include <type_traits>

namespace PCF8576Context {

/*
  First eight bits of 16-bit block are reserved for shift value,
  the next 8 bits are reserved for the bitmask

   | mask | shift |
   0000000000000000
*/
typedef uint16_t ModeInfo;

template <uint8_t shift, uint8_t size>
constexpr ModeInfo SET_MODE_INFO()
{
  static_assert(size <= 8, "Size of the block cannot exceed 8 bits");
  static_assert(shift <= 8, "Shift amount cannot exceed 8 bits");

  uint16_t result = UINT16_MAX;
  result <<= size;
  result = ~result;
  result <<= shift + 8;
  result |= shift;

  return result;
}

enum class PCFConfig : ModeInfo {
  PowerMode = SET_MODE_INFO<4, 1>(), // power dissipation (see PowerMode enum for available values)
  DisplayStatus = SET_MODE_INFO<3, 1>(), // display status (see DisplayStatus enum for available values)
  LCDBiasConfig = SET_MODE_INFO<2, 1>(), // LCD bias configuration (see LCDBiasConfig enum for available values)
  LCDDriveMode = SET_MODE_INFO<0, 2>(), // LCD drive mode selection (see LCDDriveMode enum for available values)
};

constexpr uint8_t GET_MODE_SHIFT(PCFConfig info) { return static_cast<uint8_t>(info); }
constexpr uint8_t GET_MODE_MASK(PCFConfig info) { return static_cast<uint8_t>(static_cast<ModeInfo>(info) >> 8); }

template <typename T>
constexpr uint8_t SET(PCFConfig info, T value) { return static_cast<uint8_t>(value) << GET_MODE_SHIFT(info); }

template <typename T>
constexpr uint8_t MERGE(uint8_t reg, PCFConfig info, T value) { return (reg & ~GET_MODE_MASK(info)) | SET(info, value); }

template <typename T>
constexpr T GET(uint8_t reg, PCFConfig info) { return static_cast<T>((reg & GET_MODE_MASK(info)) >> GET_MODE_SHIFT(info)); }

enum class PowerMode : uint8_t {
  Normal = 0b0, // normal-power mode
  PowerSaving = 0b1, // power-saving mode
};

enum class DisplayStatus : uint8_t {
  Disabled = 0b0, // disabled
  Enabled = 0b1, // enabled
};

// Is not applicable for the static LCD drive mode.
enum class LCDBiasConfig : uint8_t {
  Third = 0b0, // 1⁄3 bias
  Half = 0b1, // 1⁄2 bias
};

enum class LCDDriveMode : uint8_t {
  Static = 0b01, // static; BP0
  OneToTwo = 0b10, // 1:2 multiplex; BP0, BP1
  OneToThree = 0b11, // 1:3 multiplex; BP0, BP1, BP2
  OneToFour = 0b00, // 1:4 multiplex; BP0, BP1, BP2, BP3
};

class PCF8576 {
  private:
  TwoWire* _wire;
  uint8_t _scl, _sda, _address;

  uint8_t mode = SET(PCFConfig::PowerMode, PowerMode::Normal)
      | SET(PCFConfig::DisplayStatus, DisplayStatus::Enabled)
      | SET(PCFConfig::LCDDriveMode, LCDDriveMode::Static);

  public:
#if defined(ESP8266) || defined(ESP32)
  PCF8576(uint8_t address, int16_t scl = SCL, int16_t sda = SDA, TwoWire* wire = &Wire);
#else
  PCF8576(uint8_t address, int16_t scl, int16_t sda, TwoWire* wire);
#endif

  bool begin();
  bool isConnected();

  bool setAddress(uint8_t address);
  uint8_t getAddress();

  void setPowerSavingEnabled(bool enabled);
  bool getPowerSavingEnabled();

  void setDisplayEnabled(bool enabled);
  bool getDisplayEnabled();

  // Is not applicable for the static LCD drive mode.
  bool setBiasConfig(LCDBiasConfig config);
  // Is not applicable for the static LCD drive mode.
  LCDBiasConfig getBiasConfig(LCDBiasConfig config);

  void setLCDDriveMode(LCDDriveMode mode);
  LCDDriveMode getLCDDriveMode();

  void setPixelState(uint8_t pixelIndex, bool enabled);

  protected:
  bool sendConfig();
};

};

using PCF8576Context::LCDBiasConfig;
using PCF8576Context::LCDDriveMode;
using PCF8576Context::PCF8576;
