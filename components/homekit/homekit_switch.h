#pragma once

#include <string.h>

#include "esphome/components/switch/switch.h"
#include "homekit_helper.h"

namespace esphome {
namespace homekit {

class HomekitSwitch {
 protected:
  switch_::Switch *bind_switch_;
  homekit_characteristic_t *cha_on_;
  homekit_characteristic_t *cha_name_;

  void sync_from_switch_(bool state) {
    if (this->cha_on_ == nullptr) {
      return;
    }

    this->cha_on_->value = HOMEKIT_BOOL_CPP(state);
    homekit_characteristic_notify(this->cha_on_, this->cha_on_->value);
  }

  static void setter(homekit_characteristic_t *ch, homekit_value_t value) {
    auto *instance = reinterpret_cast<HomekitSwitch *>(ch->context);
    if (instance == nullptr || instance->bind_switch_ == nullptr) {
      return;
    }

    instance->bind_switch_->control(value.bool_value);
  }

  static homekit_value_t getter(const homekit_characteristic_t *ch) {
    return HOMEKIT_BOOL_CPP(reinterpret_cast<HomekitSwitch *>(ch->context)->bind_switch_->state);
  }

 public:
  explicit HomekitSwitch(switch_::Switch *s) : bind_switch_(s) {
    this->cha_name_ = new homekit_characteristic_t;
    memset(this->cha_name_, 0, sizeof(homekit_characteristic_t));
    this->cha_name_->type = HOMEKIT_CHARACTERISTIC_NAME;
    this->cha_name_->description = "Name";
    this->cha_name_->format = homekit_format_string;
    this->cha_name_->value = HOMEKIT_STRING_CPP(strdup(s->get_name().c_str()));

    this->cha_on_ = new_cha_on();
    this->cha_on_->value = HOMEKIT_BOOL_CPP(s->state);
    this->cha_on_->getter_ex = getter;
    this->cha_on_->setter_ex = setter;
    this->cha_on_->context = this;

    this->bind_switch_->add_on_state_callback([this](bool state) { this->sync_from_switch_(state); });
  }

  homekit_characteristic_t *get_cha_on() { return this->cha_on_; }
  homekit_characteristic_t *get_cha_name() { return this->cha_name_; }
};

}  // namespace homekit
}  // namespace esphome
