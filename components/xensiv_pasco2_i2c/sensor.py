import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import sensor, i2c
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CARBON_DIOXIDE,
    STATE_CLASS_MEASUREMENT,
    UNIT_PARTS_PER_MILLION,
)
from esphome import pins

CODEOWNERS = ["@goram"]
DEPENDENCIES = ["i2c"]

CONF_INTERRUPT_PIN = "interrupt_pin"

xensiv_pasco2_i2c_ns = cg.esphome_ns.namespace("xensiv_pasco2_i2c")
XensivPasCO2I2C = xensiv_pasco2_i2c_ns.class_(
    "XensivPasCO2I2C", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        XensivPasCO2I2C,
        unit_of_measurement=UNIT_PARTS_PER_MILLION,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_CARBON_DIOXIDE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.Optional(CONF_INTERRUPT_PIN): pins.internal_gpio_input_pin_schema,
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x28))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    await i2c.register_i2c_device(var, config)
    
    if CONF_INTERRUPT_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
        cg.add(var.set_interrupt_pin(pin))