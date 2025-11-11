#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace xensiv_pasco2_i2c {

class XensivPasCO2I2C : public PollingComponent, public i2c::I2CDevice, public sensor::Sensor {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  void read_co2_ppm();
  
  protected:
  float co2_ppm_{0.0f};
  uint16_t version_{2};
  bool set_contiuous_operation_mode_();
  bool single_shot_measure_co2_ppm_();
};

}  // namespace xensiv_pasco2_i2c
}  // namespace esphome