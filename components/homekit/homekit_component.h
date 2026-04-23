#pragma once

#include <homekit/homekit.h>

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "homekit_helper.h"
#include "homekit_switch.h"

#include <array>
#include <string>
#include <vector>

namespace esphome {
namespace homekit {

class HomekitComponent : public PollingComponent {
 public:
  HomekitComponent() : PollingComponent(2000) {}

  void set_setup_code(const char *setup_code) { this->setup_code_ = setup_code; }
  void set_bark_url(const char *bark_url) { this->bark_url_ = bark_url; }
  void add_switch(switch_::Switch *s) { this->switches_.push_back(s); }
  const std::string &get_setup_code() const { return this->active_setup_code_; }
  bool is_paired() const { return this->paired_; }

  void reset_storage();
  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

 protected:
  void on_generated_setup_code(const char *setup_code);
  std::string build_bark_url_() const;
  bool push_bark_notification_();
  static void password_callback_(const char *setup_code);
  
  // 生成带 MAC 后缀的唯一名称
  std::string generate_unique_name();
  // 修改 accessory 名称
  void update_accessory_names();

  std::string setup_code_;
  std::string bark_url_;
  std::string setup_id_;
  std::string active_setup_code_;
  std::string unique_device_name_;  // 存储唯一设备名称
  bool has_user_setup_code_{false};
  bool has_generated_setup_code_{false};
  bool bark_sent_{false};
  bool paired_{false};
  uint32_t last_bark_push_ms_{0};
  bool is_all_switches_inited_{false};
  std::vector<switch_::Switch *> switches_;
  std::vector<HomekitSwitch *> homekit_switches_;

  bool is_homekit_srv_started_{false};
  homekit_server_config_t homekit_srv_config_{};
  std::vector<homekit_accessory_t *> accessories_;
};

template<typename... Ts> class HomekitResetAction : public Action<Ts...> {
 public:
  explicit HomekitResetAction(HomekitComponent *p) : p_(p) {}
  void play(Ts... x) override { p_->reset_storage(); }
  HomekitComponent *p_;
};

}  // namespace homekit
}  // namespace esphome
