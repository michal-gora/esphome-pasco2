#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/gpio.h"

namespace esphome {
namespace xensiv_pasco2_i2c {

class XensivPasCO2I2C : public PollingComponent, public i2c::I2CDevice, public sensor::Sensor {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  void read_co2_ppm();
  
  void set_interrupt_pin(InternalGPIOPin *pin) { interrupt_pin_ = pin; }
  
 protected:
  static void gpio_intr(XensivPasCO2I2C *arg);
  
  float co2_ppm_{0.0f};
  uint16_t version_{2};
  bool set_continuous_operation_mode_();
  bool single_shot_measure_co2_ppm_();
  bool set_sensor_rate_(int16_t);
  
  InternalGPIOPin *interrupt_pin_{nullptr};
  volatile bool data_ready_{false};
};

}  // namespace xensiv_pasco2_i2c
}  // namespace esphome