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

            // Soft reset - write magic value 0xA3 to SENS_RST register to trigger reset
            if (this->write_byte(XENSIV_PAS_GAS_REG_SENS_RST, 0xA3))
            {
                ESP_LOGCONFIG(TAG, "Sensor soft reset");
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
            arg->setup_interrupt_();
            arg->set_operation_mode_();

            // Set up interrupt pin if configured
            if (arg->interrupt_pin_ != nullptr)
            {
                arg->interrupt_pin_->setup();
                // Input only - sensor has push-pull output (active low)
                arg->interrupt_pin_->pin_mode(gpio::FLAG_INPUT);
                arg->interrupt_pin_->attach_interrupt(
                    XensivPasCO2I2C::gpio_intr_,
                    arg,
                    gpio::INTERRUPT_FALLING_EDGE // Active low interrupt
                );
                ESP_LOGCONFIG(TAG, "  Interrupt pin configured (active low)");
            }
            
        }

        void XensivPasCO2I2C::gpio_intr_(XensivPasCO2I2C *arg)
        {
            // ISR - keep this minimal, no logging in ISR!
            arg->read_co2_ppm();
            ESP_LOGW(TAG, "Interrupt triggered - data ready");
            // Clear MEAS_STS INT_STS_CLR bit
            arg->write_byte(XENSIV_PAS_GAS_REG_MEAS_STS, XENSIV_PAS_GAS_REG_MEAS_STS_INT_STS_CLR_MSK);
        }

        bool XensivPasCO2I2C::setup_interrupt_()
        {
            // Set interrupt: INT_FUNC=1 (data ready), INT_TYP=0 (active low)
            uint8_t int_cfg_value = (1 << XENSIV_PAS_GAS_REG_INT_CFG_INT_FUNC_POS) | 
                                     (0 << XENSIV_PAS_GAS_REG_INT_CFG_INT_TYP_POS);
            if (this->write_byte(XENSIV_PAS_GAS_REG_INT_CFG, int_cfg_value))
            {
                ESP_LOGCONFIG(TAG, "Interrupt configured (active low, data ready)");
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "Failed to configure interrupt");
                return false;
            }
        }

        bool XensivPasCO2I2C::set_operation_mode_()
        {
            if (this->operation_mode_ == 1) // Single-shot mode
            {
                return true; // Single-shot mode setup done using single_shot_measure_co2_ppm()
            }
            // Set to continuous measurement mode: OP_MODE=2, BOC_CFG=1
            uint8_t meas_cfg_value = (2 << XENSIV_PAS_GAS_REG_MEAS_CFG_OP_MODE_POS) | 
                                      (1 << XENSIV_PAS_GAS_REG_MEAS_CFG_BOC_CFG_POS);
            bool success = this->write_byte(XENSIV_PAS_GAS_REG_MEAS_CFG, meas_cfg_value);

            if (success)
            {
                ESP_LOGCONFIG(TAG, "Sensor set to continuous measurement mode");
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "Failed to set sensor to continuous measurement mode");
                return false;
            }
        }

        bool XensivPasCO2I2C::select_sensor_rate_()
        {
            // Rate validation is done in sensor.py (5-4905 seconds)
            // Rate is stored as 12-bit value across MEAS_RATE_H and MEAS_RATE_L registers
            uint16_t rate = this->sensor_rate_;
            uint8_t rate_h = (rate >> 8) & 0xFF; // Upper byte
            uint8_t rate_l = rate & 0xFF;         // Lower byte

            ESP_LOGD(TAG, "Setting sensor rate to %d seconds", rate);

            if (!this->write_byte(XENSIV_PAS_GAS_REG_MEAS_RATE_H, rate_h)) {
                ESP_LOGE(TAG, "Failed to write MEAS_RATE_H");
                return false;
            }
            if (!this->write_byte(XENSIV_PAS_GAS_REG_MEAS_RATE_L, rate_l)) {
                ESP_LOGE(TAG, "Failed to write MEAS_RATE_L");
                return false;
            }

            ESP_LOGCONFIG(TAG, "Sensor rate set to %d seconds", rate);
            return true;
        }

        bool XensivPasCO2I2C::single_shot_measure_co2_ppm()
        {
            // Start single-shot measurement: OP_MODE=1, BOC_CFG=1
            uint8_t meas_cfg_value = (1 << XENSIV_PAS_GAS_REG_MEAS_CFG_OP_MODE_POS) | 
                                      (1 << XENSIV_PAS_GAS_REG_MEAS_CFG_BOC_CFG_POS);
            if (this->write_byte(XENSIV_PAS_GAS_REG_MEAS_CFG, meas_cfg_value))
            {
                ESP_LOGCONFIG(TAG, "Sensor set to single-shot measurement mode");
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "Failed to set sensor to single-shot measurement mode");
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

            // Only set to continuous mode if requested and not already in continuous mode
            if (this->operation_mode_ == 0) // 0 = continuous mode
            {
                uint8_t current_meas_cfg = 0;
                if (this->read_bytes(XENSIV_PAS_GAS_REG_MEAS_CFG, &current_meas_cfg, 1))
                {
                    // Build expected continuous mode value: OP_MODE=2, BOC_CFG=1
                    uint8_t continuous_mode_value = (2 << XENSIV_PAS_GAS_REG_MEAS_CFG_OP_MODE_POS) | 
                                                    (1 << XENSIV_PAS_GAS_REG_MEAS_CFG_BOC_CFG_POS);
                    if (current_meas_cfg != continuous_mode_value)
                    {
                        this->write_byte(XENSIV_PAS_GAS_REG_MEAS_CFG, continuous_mode_value);
                    }
                }
            }

            // DRDY flag check
            if (this->read_bytes(XENSIV_PAS_GAS_REG_MEAS_STS, meas_sts, 1))
            {
                // Get DRDY flag in MEAS_STS (bit 4)
                bool drdy = (*meas_sts & XENSIV_PAS_GAS_REG_MEAS_STS_DRDY_MSK) != 0;
                ESP_LOGD(TAG, "MEAS_STS: 0x%02X, DRDY: %s", *meas_sts, drdy ? "SET" : "NOT SET");

                if (drdy || true) // TODO fix
                {
                    if (this->read_bytes(XENSIV_PAS_GAS_REG_GASCONC_H, co2_ppm_val, 2))
                    {
                        // Read GASCONC_H and GASCONC_L
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
            // if (this->read_bytes(XENSIV_PAS_GAS_REG_PROD_ID, debug_data, debug_bytes_to_read))
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
            ESP_LOGCONFIG(TAG, "  Firmware Version: 0x%04X", this->version_);
            LOG_I2C_DEVICE(this);
            ESP_LOGCONFIG(TAG, "  Sensor Rate: %d seconds", this->sensor_rate_);
            if (this->interrupt_pin_ != nullptr) {
                LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
            }
            ESP_LOGCONFIG(TAG, "  Last CO2 Value: %.2f ppm", this->co2_ppm_);
        }

    }
}