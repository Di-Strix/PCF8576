#include "PCF8576.h"

PCF8576::PCF8576(uint8_t address, int16_t scl, int16_t sda, TwoWire* wire)
{
  this->_address = address;
  this->_scl = scl;
  this->_sda = sda;
  this->_wire = wire;
};

bool PCF8576::begin()
{
  this->_wire->begin(this->_sda, this->_scl);
  this->sendConfig();

  return this->isConnected();
}

bool PCF8576::isConnected()
{
  this->_wire->clearWriteError();
  this->_wire->beginTransmission(this->_address);

  return this->_wire->endTransmission() == 0;
}

bool PCF8576::setAddress(uint8_t address)
{
  this->_address = address;

  return this->isConnected();
}

uint8_t PCF8576::getAddress()
{
  return this->_address;
}

void PCF8576::setPowerSavingEnabled(bool enabled)
{
  this->mode = MERGE(this->mode, PCFConfig::PowerMode, enabled ? PowerMode::PowerSaving : PowerMode::Normal);
  this->sendConfig();
}

bool PCF8576::getPowerSavingEnabled()
{
  return GET<PowerMode>(this->mode, PCFConfig::PowerMode) == PowerMode::PowerSaving;
}

void PCF8576::setDisplayEnabled(bool enabled)
{
  this->mode = MERGE(this->mode, PCFConfig::DisplayStatus, enabled ? DisplayStatus::Enabled : DisplayStatus::Disabled);
  this->sendConfig();
}

bool PCF8576::getDisplayEnabled()
{
  return GET<DisplayStatus>(this->mode, PCFConfig::DisplayStatus) == DisplayStatus::Enabled;
}

bool PCF8576::setBiasConfig(LCDBiasConfig config)
{
  if (this->getLCDDriveMode() == LCDDriveMode::Static)
    return false;

  this->mode = MERGE(this->mode, PCFConfig::LCDBiasConfig, config);
  return this->sendConfig();
}

PCF8576Context::LCDBiasConfig PCF8576::getBiasConfig(LCDBiasConfig config)
{
  return GET<LCDBiasConfig>(this->mode, PCFConfig::LCDBiasConfig);
}

void PCF8576::setLCDDriveMode(LCDDriveMode mode)
{
  this->mode = MERGE(this->mode, PCFConfig::LCDDriveMode, mode);
  this->sendConfig();
}

PCF8576Context::LCDDriveMode PCF8576::getLCDDriveMode()
{
  return GET<LCDDriveMode>(this->mode, PCFConfig::LCDDriveMode);
}

void PCF8576::setPixelState(uint8_t pixelIndex, bool enabled)
{
  switch (this->getLCDDriveMode()) {
  case LCDDriveMode::Static:
    if (pixelIndex >= 40)
      return;

    this->_wire->beginTransmission(this->_address);
    this->_wire->write(pixelIndex);
    this->_wire->write(enabled);
    this->_wire->endTransmission();
    break;

  default:
    break;
  }
}

bool PCF8576::sendConfig()
{
  this->_wire->clearWriteError();

  this->_wire->beginTransmission(this->_address);
  this->_wire->write(COMMAND::LAST | COMMAND::MODE_SET | this->mode);
  this->_wire->endTransmission();

  return this->_wire->getWriteError() == 0;
}
