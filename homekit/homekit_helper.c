#include "homekit_helper.h"

void my_identify(homekit_value_t _value) {}

homekit_characteristic_t* new_cha_name(const char* name) {
    return NEW_HOMEKIT_CHARACTERISTIC(NAME, (char*)name);
}

homekit_characteristic_t* new_cha_on() {
    return NEW_HOMEKIT_CHARACTERISTIC(ON, false);
}

homekit_accessory_t* new_switch_accessory(unsigned int id, const char* name, homekit_characteristic_t* cha_on) {
    return NEW_HOMEKIT_ACCESSORY(.id=id, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]) {
        NEW_HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            new_cha_name(name),
            NEW_HOMEKIT_CHARACTERISTIC(MANUFACTURER, "ESPHome"),
            NEW_HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "HK-SW"),
            NEW_HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266"),
            NEW_HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            NEW_HOMEKIT_CHARACTERISTIC(IDENTIFY, my_identify),
            NULL
        }),
        NEW_HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            cha_on,
            new_cha_name(name),
            NULL
        }),
        NULL
    });
}