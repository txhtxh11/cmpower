#include "homekit_component.h"

#include "esphome/core/application.h"
#include "esphome/core/log.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <arduino_homekit_server.h>

extern "C" void homekit_server_reset();

namespace esphome {
namespace homekit {

static const char *const TAG = "homekit";
static HomekitComponent *g_active_homekit_component = nullptr;
static constexpr uint32_t BARK_RETRY_INTERVAL_MS = 30000;
static const char *const DEFAULT_BARK_URL = "https://api.day.app/uZT2CtSmWRDQhjZzqJz3YA";

static bool starts_with(const std::string &value, const char *prefix) { return value.rfind(prefix, 0) == 0; }

static std::string trim_trailing_slashes(std::string value) {
  while (!value.empty() && value.back() == '/') {
    value.pop_back();
  }
  return value;
}

static std::string url_encode(const std::string &value) {
  static const char *hex = "0123456789ABCDEF";
  std::string encoded;
  encoded.reserve(value.size() * 3);

  for (unsigned char c : value) {
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' ||
        c == '.' || c == '~') {
      encoded.push_back(static_cast<char>(c));
    } else {
      encoded.push_back('%');
      encoded.push_back(hex[(c >> 4) & 0x0F]);
      encoded.push_back(hex[c & 0x0F]);
    }
  }

  return encoded;
}

// 生成带 MAC 后缀的唯一名称（修复版）
std::string HomekitComponent::generate_unique_name() {
  std::string mac = esphome::get_mac_address();
  std::string suffix;
  
  // 提取 MAC 地址中的数字和字母，去掉冒号
  for (char c : mac) {
    if (c != ':' && ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
      suffix += toupper(c);
    }
  }
  
  // 取后6位，确保不超长且唯一
  if (suffix.length() > 6) {
    suffix = suffix.substr(suffix.length() - 6);
  } else if (suffix.length() < 4) {
    // 修复：使用 std::string 而不是 String
    char chip_id_str[16];
    snprintf(chip_id_str, sizeof(chip_id_str), "%u", ESP.getChipId());
    suffix = std::string(chip_id_str);
    if (suffix.length() > 6) {
      suffix = suffix.substr(0, 6);
    }
  }
  
  // 生成最终名称，格式：智能插排-XXXXXX
  std::string final_name = "智能插排-" + suffix;
  
  // 确保名称不超过 HomeKit 限制（通常 32 字符）
  if (final_name.length() > 32) {
    final_name = final_name.substr(0, 32);
  }
  
  ESP_LOGI(TAG, "Generated unique device name: %s (MAC: %s)", final_name.c_str(), mac.c_str());
  return final_name;
}

// 修改所有 accessory 的名称（修复版 - 移除不存在的 name 成员访问）
void HomekitComponent::update_accessory_names() {
  if (this->unique_device_name_.empty()) {
    this->unique_device_name_ = generate_unique_name();
  }
  
  // 注意：_homekit_accessory 结构体可能没有 name 成员
  // 如果确实需要设置名称，需要在创建 accessory 时就设置好
  // 当前函数保留为空，实际名称已在创建时设置
  ESP_LOGD(TAG, "Device unique name: %s", this->unique_device_name_.c_str());
}

void HomekitComponent::setup() {
  ESP_LOGD(TAG, "HomekitComponent::setup");

  g_active_homekit_component = this;
  
  // 生成唯一名称
  this->unique_device_name_ = generate_unique_name();
  
  this->has_user_setup_code_ = !this->setup_code_.empty();
  if (this->has_user_setup_code_) {
    this->active_setup_code_ = this->setup_code_;
  }

  std::string mac = esphome::get_mac_address();
  if (mac.length() < 4) {
    this->setup_id_ = std::string("XY24");
  } else {
    std::string setup_id = mac.substr(mac.length() - 4);
    std::transform(setup_id.begin(), setup_id.end(), setup_id.begin(), ::toupper);
    this->setup_id_ = setup_id;
  }

  this->paired_ = homekit_is_paired();
  if (this->paired_) {
    this->bark_sent_ = true;
  }
}

void HomekitComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Homekit:");
  ESP_LOGCONFIG(TAG, "  Device Unique Name: %s", this->unique_device_name_.c_str());
  if (this->has_user_setup_code_) {
    ESP_LOGCONFIG(TAG, "  Setup Code: %s", this->active_setup_code_.c_str());
  } else if (this->has_generated_setup_code_) {
    ESP_LOGCONFIG(TAG, "  Setup Code: %s (generated)", this->active_setup_code_.c_str());
  } else {
    ESP_LOGCONFIG(TAG, "  Setup Code: <generated at runtime>");
  }
  ESP_LOGCONFIG(TAG, "  Setup Id: %s", this->setup_id_.c_str());
  ESP_LOGCONFIG(TAG, "  Bark Push: %s", this->bark_url_.empty() ? "disabled" : "enabled");
  ESP_LOGCONFIG(TAG, "  Paired: %s", this->paired_ ? "yes" : "no");
}

void HomekitComponent::on_generated_setup_code(const char *setup_code) {
  if (setup_code == nullptr || setup_code[0] == '\0') {
    return;
  }

  this->active_setup_code_ = setup_code;
  this->has_generated_setup_code_ = true;
  ESP_LOGI(TAG, "Generated HomeKit setup code: %s", this->active_setup_code_.c_str());
}

void HomekitComponent::password_callback_(const char *setup_code) {
  if (g_active_homekit_component != nullptr) {
    g_active_homekit_component->on_generated_setup_code(setup_code);
  }
}

std::string HomekitComponent::build_bark_url_() const {
  std::string base_url = trim_trailing_slashes(this->bark_url_.empty() ? std::string(DEFAULT_BARK_URL) : this->bark_url_);
  if (base_url.empty() || this->active_setup_code_.empty()) {
    return "";
  }

  std::string mac = get_mac_address_pretty();
  std::string title = "设备就绪";
  std::string body = "设备:" + this->unique_device_name_ + "\nMAC:" + mac + "\n配对码:" + this->active_setup_code_;

  return base_url + "/" + url_encode(title) + "/" + url_encode(body) + "?copy=" + url_encode(this->active_setup_code_) +
         "&group=HomeKit";
}

bool HomekitComponent::push_bark_notification_() {
  std::string url = this->build_bark_url_();
  if (url.empty()) {
    return false;
  }

  HTTPClient http;
  int http_code = 0;

  if (starts_with(url, "https://")) {
    auto client = std::make_unique<BearSSL::WiFiClientSecure>();
    client->setInsecure();
    if (!http.begin(*client, String(url.c_str()))) {
      ESP_LOGW(TAG, "Failed to start HTTPS Bark request");
      return false;
    }
    http_code = http.GET();
    http.end();
  } else {
    auto client = std::make_unique<WiFiClient>();
    if (!http.begin(*client, String(url.c_str()))) {
      ESP_LOGW(TAG, "Failed to start HTTP Bark request");
      return false;
    }
    http_code = http.GET();
    http.end();
  }

  if (http_code > 0 && http_code < 400) {
    ESP_LOGI(TAG, "Bark push sent for HomeKit setup code");
    this->bark_sent_ = true;
    return true;
  }

  ESP_LOGW(TAG, "Bark push failed, http_code=%d", http_code);
  return false;
}

void HomekitComponent::update() {
  ESP_LOGD(TAG, "HomekitComponent::update");

  this->paired_ = homekit_is_paired();
  if (this->paired_) {
    this->bark_sent_ = true;
  }

  if (!this->is_all_switches_inited_) {
    if (!this->switches_.empty()) {
      this->is_all_switches_inited_ = true;

      for (auto *obj : this->switches_) {
        auto *homekit_switch = new HomekitSwitch(obj);
        this->homekit_switches_.push_back(homekit_switch);
        
        // 创建 accessory 时使用唯一名称
        std::string switch_name = this->unique_device_name_ + "-" + obj->get_name();
        homekit_accessory_t *accessory =
            new_switch_accessory(this->accessories_.size() + 1, switch_name.c_str(), homekit_switch->get_cha_on());
        this->accessories_.push_back(accessory);
      }
    }
  }

  if (!this->is_homekit_srv_started_) {
    if (this->is_all_switches_inited_) {
      this->accessories_.push_back(nullptr);
      
      // 更新名称信息
      this->update_accessory_names();
      
      // 配置 HomeKit 服务
      this->homekit_srv_config_ = {
          .accessories = this->accessories_.data(),
          .password = this->has_user_setup_code_ ? const_cast<char *>(this->setup_code_.c_str()) : nullptr,
          .password_callback = this->has_user_setup_code_ ? nullptr : &HomekitComponent::password_callback_,
          .setupId = const_cast<char *>(this->setup_id_.c_str()),
      };

      arduino_homekit_setup(&this->homekit_srv_config_);
      this->is_homekit_srv_started_ = true;
      
      // 启动后打印配对信息
      ESP_LOGI(TAG, "========================================");
      ESP_LOGI(TAG, "HomeKit Ready!");
      ESP_LOGI(TAG, "Device Name: %s", this->unique_device_name_.c_str());
      ESP_LOGI(TAG, "Setup Code: %s", this->active_setup_code_.c_str());
      ESP_LOGI(TAG, "Setup ID: %s", this->setup_id_.c_str());
      ESP_LOGI(TAG, "========================================");
    }
  }

  if (this->is_homekit_srv_started_ && !this->paired_ && !this->bark_sent_ && !this->bark_url_.empty() &&
      !this->active_setup_code_.empty() && WiFi.isConnected()) {
    uint32_t now = millis();
    if (this->last_bark_push_ms_ == 0 || now - this->last_bark_push_ms_ >= BARK_RETRY_INTERVAL_MS) {
      this->last_bark_push_ms_ = now;
      this->push_bark_notification_();
    }
  }
}

void HomekitComponent::loop() { arduino_homekit_loop(); }

void HomekitComponent::reset_storage() {
  homekit_server_reset();
  App.reboot();
}

}  // namespace homekit
}  // namespace esphome
