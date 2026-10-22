# cmpower — CMPOWER W1 智能排插 ESPHome 组件

> 中移铁通 CMPOWER W1 智能排插的 ESPHome 外部组件：`sy7t609` 计量芯片驱动 + `homekit` 桥接，编译后直接接入 Apple「家庭」App。

## 硬件

- MCU：ESP8266（ESP-12E，4M Flash）
- 计量：SY7T609（UART，9600 8N1）
- 两路继电器（总控 / 分路）+ 三色 LED + 一个物理按键

## 目录

```
components/
├── cmpower.yaml      # 完整设备配置，可直接编译
├── homekit/          # HomeKit 外部组件（HAP 实现）
├── sy7t609/          # SY7T609 计量组件（电压/电流/功率/电量/功率因数/频率/芯片温度）
└── firmware.bin      # 预编译固件
```

## 用法

把 `components/` 里的内容放到 ESPHome 配置目录（`cmpower.yaml` 与 `homekit/`、`sy7t609/` 同级），然后：

```bash
esphome run cmpower.yaml
```

`cmpower.yaml` 里已经写好 `external_components: { type: local, path: . }`，
不需要额外安装组件。

## 功能

- HomeKit 直连，配对码在 `HomeKit 配对码` 文本传感器里显示（也可在 YAML 里固定 `setup_code`）
- 总开关 / 分路开关，分路开启会自动带上总控
- 物理按键：单击切分路、双击切总控、长按 5 秒恢复出厂
- 电量累计断电记忆（`globals` + `restore_value`），可一键清零
- 电压 / 电流 / 功率 / 无功功率 / 功率因数 / 频率 / 芯片温度
- Web Server + OTA + 恢复出厂 / 重启按钮

## 许可证

本项目采用 MIT 许可证，详见 [LICENSE](LICENSE)。