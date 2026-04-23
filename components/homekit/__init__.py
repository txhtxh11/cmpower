import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import switch
from esphome.const import CONF_ID

DEPENDENCIES = ["network", "switch"]
AUTO_LOAD = ["switch"]
MULTI_CONF = False

homekit_ns = cg.esphome_ns.namespace("homekit")

HomekitComponent = homekit_ns.class_("HomekitComponent", cg.PollingComponent)
HomekitResetAction = homekit_ns.class_("HomekitResetAction", automation.Action)

CONF_SWITCHES = "switches"
CONF_SETUP_CODE = "setup_code"
CONF_BARK_URL = "bark_url"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HomekitComponent),
        cv.Optional(CONF_SETUP_CODE, default=""): cv.string,
        cv.Optional(CONF_BARK_URL, default=""): cv.string,
        cv.Optional(CONF_SWITCHES): cv.ensure_list(cv.use_id(switch.Switch)),
    }
).extend(cv.COMPONENT_SCHEMA)


def to_code(config):
    cg.add_library(
        "HomeKit-ESP8266",
        "8a8e1a065005e9252d728b24f96f6d0b29993f67",
        "https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266.git",
    )

    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_setup_code(config[CONF_SETUP_CODE]))
    cg.add(var.set_bark_url(config[CONF_BARK_URL]))

    if CONF_SWITCHES in config:
        for sw in config[CONF_SWITCHES]:
            s = yield cg.get_variable(sw)
            cg.add(var.add_switch(s))

    yield cg.register_component(var, config)


@automation.register_action(
    "homekit.reset_storage",
    HomekitResetAction,
    automation.maybe_simple_id(
        {
            cv.Required(CONF_ID): cv.use_id(HomekitComponent),
        }
    ),
)
async def homekit_reset_to_code(config, action_id, template_arg, args):
    var = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, var)
