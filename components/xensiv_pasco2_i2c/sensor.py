import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import sensor, i2c
from esphome.const import (
    CONF_ID,
    CONF_CO2
    DEVICE_CLASS_CARBON_DIOXIDE,
    STATE_CLASS_MEASUREMENT,
    UNIT_PARTS_PER_MILLION,
    ICON_MOLECULE_CO2,
)
from esphome import pins

CODEOWNERS = ["@goram"]
DEPENDENCIES = ["i2c"]

CONF_INTERRUPT_PIN = "interrupt_pin"
CONF_SENSOR_RATE = "sensor_rate"
CONF_OPERATION_MODE = "operation_mode"

xensiv_pasco2_i2c_ns = cg.esphome_ns.namespace("xensiv_pasco2_i2c")
XensivPasCO2I2C = xensiv_pasco2_i2c_ns.class_(
    "XensivPasCO2I2C", cg.Component, i2c.I2CDevice
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(XensivPasCO2I2C),
        cv.Required(CONF_CO2): sensor.sensor_schema(
            unit_of_measurement=UNIT_PARTS_PER_MILLION,
            icon=ICON_MOLECULE_CO2,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_CARBON_DIOXIDE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_INTERRUPT_PIN): pins.internal_gpio_input_pin_schema,
        cv.Optional(CONF_SENSOR_RATE, default="10s"): cv.All(
            cv.positive_time_period_seconds,
            cv.Range(min=cv.TimePeriod(seconds=5), max=cv.TimePeriod(seconds=4095))
        ),
        cv.Optional(CONF_OPERATION_MODE, default="continuous"): cv.enum(
            {
                "single_shot": 0,
                "continuous": 1,
            }
        ),
    }
).extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(0x28))

SENSOR_MAP = {
    CONF_CO2: "set_co2_sensor",
}


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    
    for key, funcName in SENSOR_MAP.items():
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, funcName)(sens))
    
    if CONF_INTERRUPT_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
        cg.add(var.set_interrupt_pin(pin))

    if CONF_SENSOR_RATE in config:
        # Convert TimePeriod to total seconds
        cg.add(var.set_sensor_rate_value(config[CONF_SENSOR_RATE].total_seconds))

    if CONF_OPERATION_MODE in config:
        cg.add(var.set_operation_mode(config[CONF_OPERATION_MODE]))