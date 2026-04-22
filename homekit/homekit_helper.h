#pragma once
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#ifdef __cplusplus
extern "C" {
#endif

void my_identify(homekit_value_t _value);
homekit_characteristic_t* new_cha_name(const char* name);
homekit_characteristic_t* new_cha_on();
homekit_accessory_t* new_switch_accessory(unsigned int id, const char* name, homekit_characteristic_t* cha_on);

#ifdef __cplusplus
}
#endif