#include "esphome/core/log.h"
#include "xensiv_pasco2_i2c.h"

namespace esphome
{
    namespace xensiv_pasco2_i2c
    {
        static const char *const TAG = "xensiv_pasco2_i2c.component";

        XensivPasCO2I2C::XensivPasCO2I2C(uint32_t update_interval) : PollingComponent(update_interval), test_value_(0) {}

        void XensivPasCO2I2C::setup()
        {
            ESP_LOGCONFIG(TAG, "Setting up XensivPasCO2I2C component");
            // Initialization code here
        }

        void XensivPasCO2I2C::update()
        {
            ESP_LOGD(TAG, "Updating XensivPasCO2I2C component");
            // Update logic here
            test_value_ += 1;  // Example update
            
            // Publish the value to Home Assistant
            if (this->test_sensor_ != nullptr) {
                this->test_sensor_->publish_state(test_value_);
            }
        }
        void XensivPasCO2I2C::dump_config()
        {
            ESP_LOGCONFIG(TAG, "XensivPasCO2I2C Component:");
            ESP_LOGCONFIG(TAG, "  Test Value: %u", this->test_value_);
        }

    }
}