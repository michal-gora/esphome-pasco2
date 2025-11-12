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

            // Perform full sensor reset (reset sticky bits, set to idle state)
            // According to the datasheet:
            // - Write 0xA3 to register 0x10 (SOFT_RESET)

            // Soft reset
            if (this->write_byte(0x10, 0xA3))
            {
                ESP_LOGCONFIG(TAG, "Sensor soft reset (SOFT_RESET=0xA3 to 0x10)");
            }
            else
            {
                ESP_LOGW(TAG, "Failed to perform sensor soft reset");
            }
            
            // Run sensor initialization after a delay to avoid blocking the main thread
            this->set_timeout(3000, [this]() { XensivPasCO2I2C::setup_sensor_(this); });
        }

        void XensivPasCO2I2C::setup_sensor_(XensivPasCO2I2C *arg)
        {
            arg->select_sensor_rate_();
            arg->set_continuous_operation_mode_with_interrupt_();

            // Set up interrupt pin if configured
            if (arg->interrupt_pin_ != nullptr)
            {
                arg->interrupt_pin_->setup();
                // Input only - sensor has push-pull output (high-active)
                arg->interrupt_pin_->pin_mode(gpio::FLAG_INPUT);
                arg->interrupt_pin_->attach_interrupt(
                    XensivPasCO2I2C::gpio_intr_,
                    arg,
                    gpio::INTERRUPT_RISING_EDGE // High-active interrupt
                );
                ESP_LOGCONFIG(TAG, "  Interrupt pin configured (high-active)");
            }
            
        }

        void XensivPasCO2I2C::update()
        {
            ESP_LOGD(TAG, "Updating XensivPasCO2I2C component");

            // // Check if interrupt triggered
            // if (this->data_ready_) {
            //     ESP_LOGD(TAG, "Data ready from interrupt");
            //     this->data_ready_ = false;
            // }

            // set_continuous_operation_mode_with_interrupt_();
            // this->read_co2_ppm();

            uint8_t rate_h = 0, rate_l = 0;
            if (this->read_bytes(0x02, &rate_h, 1) && this->read_bytes(0x03, &rate_l, 1))
            {
                int16_t rate = (static_cast<int16_t>(rate_h) << 8) | rate_l;
                ESP_LOGD(TAG, "Sensor rate (0x02/0x03): %d", rate);
            }
            else
            {
                ESP_LOGW(TAG, "Failed to read sensor rate registers (0x02/0x03)");
            }
            uint8_t meas_cfg_val = 0;
            if (this->read_bytes(0x04, &meas_cfg_val, 1))
            {
                ESP_LOGD(TAG, "MEAS_CFG (0x04): 0x%02X", meas_cfg_val);
            }
            else
            {
                ESP_LOGW(TAG, "Failed to read MEAS_CFG register (0x04)");
            }
        }

        void XensivPasCO2I2C::gpio_intr_(XensivPasCO2I2C *arg)
        {
            // ISR - keep this minimal, no logging in ISR!
            arg->read_co2_ppm();
            ESP_LOGW(TAG, "Interrupt triggered - data ready");
            // Clear MEAS_STS
            uint8_t int_sts_clr_mask = 0x2;
            arg->write_byte(0x07, int_sts_clr_mask);
        }

        bool XensivPasCO2I2C::set_continuous_operation_mode_with_interrupt_()
        {
            // Set interrupt by writing 0x15 to register 0x08
            uint8_t int_cfg_value = 0x15;
            bool int_success = this->write_byte(0x08, int_cfg_value);

            // Write 0x26 to MEAS_CFG register (0x04) to enable continuous measurement mode
            uint8_t meas_cfg_value = 0x26;
            bool success = this->write_byte(0x04, meas_cfg_value);

            if (success && int_success)
            {
                ESP_LOGCONFIG(TAG, "Sensor set to continuous measurement mode (MEAS_CFG=0x26) and interrupt configured (INT_CFG=0x15)");
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "Failed to set sensor to continuous measurement mode or configure interrupt");
                return false;
            }
        }

        bool XensivPasCO2I2C::select_sensor_rate_()
        {
            // Rate is stored in 12 bits across two registers (0x02 and 0x03)
            // Register 0x02: bits [11:8] (upper 4 bits)
            // Register 0x03: bits [7:0] (lower 8 bits)
            int16_t rate = this->sensor_rate_;
            uint8_t rate_h = (rate >> 8) & 0x0F; // Upper 4 bits (mask to 12-bit max)
            uint8_t rate_l = rate & 0xFF;        // Lower 8 bits

            ESP_LOGD(TAG, "Setting sensor rate to %d seconds (0x%02X%02X)", rate, rate_h, rate_l);

            if (this->write_byte(0x02, rate_h) && this->write_byte(0x03, rate_l))
            {
                ESP_LOGCONFIG(TAG, "Sensor rate set to %d seconds", rate);
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "Failed to set sensor rate");
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
            uint8_t *meas_sts;

            // DRDY flag check
            if (this->read_bytes(0x07, meas_sts, 1))
            {
                // Get DRDY flag in MEAS_STS (address 0x07, bit 4)
                bool drdy = (*meas_sts & (1 << 4)) != 0;
                ESP_LOGD(TAG, "MEAS_STS (0x07): 0x%02X, DRDY: %s", *meas_sts, drdy ? "SET" : "NOT SET");

                if (drdy || true) // TODO fix
                {
                    if (this->read_bytes(0x05, co2_ppm_val, 2))
                    {
                        // Read CO2PPM_H (0x05) and CO2PPM_L (0x06)
                        uint8_t co2ppm_h = co2_ppm_val[0];
                        uint8_t co2ppm_l = co2_ppm_val[1];
                        int16_t co2_raw = (static_cast<int16_t>(co2ppm_h) << 8) | co2ppm_l;
                        this->co2_ppm_ = static_cast<float>(co2_raw);
                        this->publish_state(this->co2_ppm_);
                    }
                }
                else
                {
                    ESP_LOGD(TAG, "DRDY not set, CO2 value not ready");
                }
            }
            else
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