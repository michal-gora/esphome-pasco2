#include "esphome/core/log.h"
#include "xensiv_pasco2_i2c.h"

namespace esphome
{
    namespace xensiv_pasco2_i2c
    {
        static const char *const TAG = "xensiv_pasco2_i2c.component";

        void XensivPasCO2I2C::setup()
        {
            ESP_LOGCONFIG(TAG, "Setting up XensivPasCO2I2C component");
            set_continuous_operation_mode_();
            this->co2_ppm_ = 42.0;
        }

        void XensivPasCO2I2C::update()
        {
            ESP_LOGD(TAG, "Updating XensivPasCO2I2C component");

            // Read CO2 value from I2C sensor
            //     TODO : Replace with actual I2C read operations
            //                Example placeholder : uint16_t co2_ppm = 0;
            this->read_co2_ppm();

            // For now, publish a dummy value if sensor is configured
            // if (this->co2_sensor_ != nullptr) {
            //     // Placeholder: publish a test value (400-500 ppm range)
            //     float dummy_co2 = 400.0f + (test_value_ % 100);
            //     this->co2_sensor_->publish_state(dummy_co2);
            //     ESP_LOGD(TAG, "Published dummy CO2 value: %.2f ppm", dummy_co2);
            // } else {
            //     ESP_LOGW(TAG, "CO2 sensor not configured");
            // }
        }

        bool XensivPasCO2I2C::set_continuous_operation_mode_()
        {
            // Write 0x26 to MEAS_CFG register (0x04) to enable continuous measurement mode
            uint8_t meas_cfg_value = 0x26;
            if (this->write_byte(0x04, meas_cfg_value))
            {
                ESP_LOGCONFIG(TAG, "Sensor set to continuous measurement mode (MEAS_CFG=0x26)");
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "Failed to set sensor to continuous measurement mode");
                return false;
            }
        }

        bool XensivPasCO2I2C::single_shot_measure_co2_ppm_()
        {
            // starts single-shot measurement
            uint8_t meas_cfg_value = 0x25;
            if (this->write_byte(0x04, meas_cfg_value))
            {
                ESP_LOGCONFIG(TAG, "Sensor set to continuous measurement mode (MEAS_CFG=0x26)");
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "Failed to set sensor to continuous measurement mode");
                return false;
            }
        }

        void XensivPasCO2I2C::read_co2_ppm()
        {
            // Try reading x bytes starting from register 0x0 for debugging
            const size_t debug_bytes_to_read = 17; // Set this variable to change how many bytes to read
            uint8_t debug_data[debug_bytes_to_read] = {0};

            uint8_t co2_ppm_val[2] = {0};
            uint8_t* meas_sts;

            // DRDY flag check
            if (this->read_bytes(0x07, meas_sts, 1))
            {
                // Get DRDY flag in MEAS_STS (address 0x07, bit 4)
                bool drdy = (*meas_sts & (1 << 4)) != 0;
                ESP_LOGD(TAG, "MEAS_STS (0x07): 0x%02X, DRDY: %s", *meas_sts, drdy ? "SET" : "NOT SET");

                if (drdy)
                {
                    if (this->read_bytes(0x05, co2_ppm_val, 2))
                    {
                        // Read CO2PPM_H (0x05) and CO2PPM_L (0x06)
                        uint8_t co2ppm_h = co2_ppm_val[0];
                        uint8_t co2ppm_l = co2_ppm_val[1];
                        ESP_LOGD(TAG, "CO2PPM_H (0x05): 0x%02X", co2ppm_h);
                        ESP_LOGD(TAG, "CO2PPM_L (0x06): 0x%02X", co2ppm_l);
                        if (drdy)
                        {
                            int16_t co2_raw = (static_cast<int16_t>(co2ppm_h) << 8) | co2ppm_l;
                            this->co2_ppm_ = static_cast<float>(co2_raw);
                            this->publish_state(this->co2_ppm_);
                            ESP_LOGD(TAG, "DRDY was set!, CO2 value ready: %.2f ppm", this->co2_ppm_);
                        }
                        else
                        {
                            ESP_LOGW(TAG, "DRDY not set, CO2 value not ready");
                        }
                    }
                }else{
                    ESP_LOGD(TAG, "DRDY not set, CO2 value not ready");
                }
            }else
            {
                ESP_LOGW(TAG, "Failed to read MEAS_STS register for DRDY check");
            }


            // Raw data for debugging
            // if (this->read_bytes(0x0, debug_data, debug_bytes_to_read))
            // {

            //     ESP_LOGD(TAG, "I2C raw data:");
            //     for (size_t i = 0; i < debug_bytes_to_read; ++i)
            //     {
            //         ESP_LOGD(TAG, "  Byte %zu: 0x%02X", i, debug_data[i]);
            //     }
            // }
            // else
            // {
            //     ESP_LOGW(TAG, "Failed to read I2C debug data from sensor");
            // }
        }

        void XensivPasCO2I2C::dump_config()
        {
            ESP_LOGCONFIG(TAG, "XensivPasCO2I2C Component:");
            ESP_LOGCONFIG(TAG, " Firmware Version: 0x%04X", this->version_);
            LOG_I2C_DEVICE(this);
            LOG_UPDATE_INTERVAL(this);
            ESP_LOGCONFIG(TAG, "  Last CO2 Value: %.2f ppm", this->co2_ppm_);
        }

    }
}