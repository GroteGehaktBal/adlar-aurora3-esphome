# Adlar Aurora III ESPHome Modbus

![ESPHome](https://img.shields.io/badge/ESPHome-2026.5%2B-2f7d95)
![Home Assistant](https://img.shields.io/badge/Home%20Assistant-ready-41bdf5)
![Hardware](https://img.shields.io/badge/hardware-Seeed%20XIAO%20ESP32--C6-6f42c1)
![Status](https://img.shields.io/badge/status-compile--tested%2C%20hardware%20verification%20needed-d29922)
![License](https://img.shields.io/badge/license-MIT-2da44e)

ESPHome configuration for monitoring and limited local control of an Adlår/Adlar Aurora III R290 heat pump through the JÅN module using Modbus RTU over RS485.

This repository is intended as a clean, reusable starting point for Home Assistant users who want a local, cloud-independent integration for an Adlar Aurora III or a related SolarEast/Sunrain R290 heat pump.

> Status: the ESPHome configuration has been validated and successfully compiled for the Seeed Studio XIAO ESP32C6. The current profile follows the public Aurora III / Castra Aurora III community mapping: 9600 baud, 8N2, input registers for telemetry, holding registers for controls.

## Unofficial Project

This is an independent community project. It is not affiliated with, endorsed by, sponsored by, or supported by Adlår/Adlar or SolarEast. Product names are used only to identify compatible hardware.

## What This Project Does

- Reads status bits, fault bits, temperatures, water flow, compressor frequency, pressure, current and voltage.
- Exposes limited Home Assistant controls for heating setpoint, DHW setpoint, room setpoint, HVAC mode, DHW mode and zone control.
- Keeps all write controls disabled by default until read-only monitoring is proven stable.
- Uses the native ESPHome API, OTA updates and a simple local web interface.
- Deliberately avoids exposing installer/service parameters as ordinary Home Assistant controls.

## Hardware

Target hardware for this configuration:

- Seeed Studio XIAO ESP32C6
- Automatic-direction RS485-to-TTL transceiver such as an `XY-485` board
- Adlar/JÅN Modbus port with `A`, `B` and `GND`

Do not feed a 5V TTL signal directly into the XIAO ESP32C6. Its GPIO pins are not 5V tolerant.

## Quick Start

1. Clone this repository or import the YAML into ESPHome.
2. Copy `secrets.example.yaml` to your own ESPHome `secrets.yaml`.
3. Fill in your WiFi, API and OTA secrets.
4. Check the wiring in [docs/wiring.md](docs/wiring.md).
5. Compile and flash `adlar_aurora3_xiao_esp32c6.yaml`.
6. Start with read-only monitoring. Watch `Inlet water temperature`, `Outlet water temperature`, `Water flow`, `System status bits` and `Heat pump status text`.
7. Only enable write controls in Home Assistant after read data is stable and plausible.

```bash
esphome config adlar_aurora3_xiao_esp32c6.yaml
esphome compile adlar_aurora3_xiao_esp32c6.yaml
```

## Bring-Up Firmware

If the RS485 board TX/RX LEDs or the ESPHome web UI make it unclear which firmware is actually running, flash `adlar_aurora3_xiao_esp32c6_bringup.yaml` first. It exposes a clear `Firmware profile` text entity and probes the two most likely Aurora III slave IDs: `1` and `251`.

```bash
esphome config adlar_aurora3_xiao_esp32c6_bringup.yaml
esphome compile adlar_aurora3_xiao_esp32c6_bringup.yaml
```

In the local web UI, the bring-up firmware should show `Firmware profile` with `bring-up 0.3.1 - 9600 8N2 - slave 1 and 251 probe`. If old entity names such as `Circulation pump active` or `Controller power` still appear, the ESP was not flashed with this repository's current YAML.

## Passive Sniffer Firmware

If the bring-up firmware sends requests but gets no replies, flash `adlar_aurora3_xiao_esp32c6_sniffer.yaml`. It does not intentionally transmit anything on RS485. It keeps XIAO `D6/TX` at UART idle-high for automatic-direction boards, listens on `D7/RX` at 9600 8N2, and logs raw bytes as `[uart_debug]` lines.

Use this to confirm whether the XIAO and RS485 module can hear the existing JÅN/heat-pump bus traffic before debugging active Modbus polling.

## UART Loopback Test

If the sniffer logs no RX bytes, flash `adlar_aurora3_xiao_esp32c6_uart_loopback.yaml`. Disconnect the RS485 module, temporarily short XIAO `D6` to XIAO `D7`, and watch the logs. Matching `[uart_debug]` TX and RX lines prove the ESPHome UART pins are working before replacing or retesting the RS485 transceiver.

## Wiring Summary

| XIAO ESP32C6 | GPIO | XY-485 / automatic-direction RS485 module |
| --- | ---: | --- |
| D6 / TX | GPIO16 | RXD |
| D7 / RX | GPIO17 | TXD |
| 3V3 |  | VCC / UCC |
| GND |  | GND / DNG |

| XY-485 / automatic-direction RS485 module | JÅN Modbus port |
| --- | --- |
| A+ | A |
| B- | B |
| E / GND | GND |

The ESP/RS485 module is intended to be connected in parallel with the existing JÅN Modbus wiring. Do not remove the original heat-pump/JÅN wires. For a permanent install, use ferrules or a small junction terminal instead of clamping multiple loose conductors under one screw.

The default YAML is configured for automatic-direction RS485 modules. If you use a basic manual-direction MAX485 module, wire `DE` and `/RE` to `D2` and add `flow_control_pin: D2` to the `modbus:` block.

More detail and a diagram are available in [docs/wiring.md](docs/wiring.md).

## Important Safety Notes

ESPHome acts as an additional Modbus client/master in this configuration. RS485 supports multiple electrical nodes on the same pair, but Modbus RTU has no real bus arbitration. Public Aurora III community setups work by tapping the same JÅN/internal bus and polling gently.

The configuration therefore polls gently: every 60 seconds by default with a 750 ms command throttle and 500 ms send wait time. Do not reduce those intervals until the bus has been proven stable.

See [docs/troubleshooting.md](docs/troubleshooting.md) for symptoms such as CRC errors, no response, temperatures that are off by a factor of 10, or write values that do not stick.

## Repository Contents

| Path | Purpose |
| --- | --- |
| `adlar_aurora3_xiao_esp32c6.yaml` | Main ESPHome configuration |
| `adlar_aurora3_xiao_esp32c6_bringup.yaml` | Minimal one-register-per-minute first-contact firmware |
| `adlar_aurora3_xiao_esp32c6_sniffer.yaml` | Passive RS485 receive-only sniffer firmware |
| `adlar_aurora3_xiao_esp32c6_uart_loopback.yaml` | Local XIAO D6/D7 UART loopback test firmware |
| `secrets.example.yaml` | Example secrets file |
| `docs/wiring.md` | Wiring and RS485 notes |
| `docs/register-map.md` | Register overview, scaling and writable addresses |
| `docs/troubleshooting.md` | Troubleshooting and first-test procedure |
| `docs/publication-notes.md` | Notes for publishing and keeping the project clearly unofficial |
| `ROADMAP.md` | Possible future improvements |
| `DISCLAIMER.md` | Unofficial project and safety disclaimer |
| `CHANGELOG.md` | Change history |
| `CONTRIBUTING.md` | Issue, pull request and hardware-test guidelines |

## Assumptions And Limits

- Modbus defaults: slave ID `1`, 9600 baud, 8 data bits, no parity, 2 stop bits. Official protocol notes mention slave `251`; several community configs use slave `1` successfully.
- Live telemetry uses input registers (`register_type: read`, Modbus function code 4).
- Writable controls use holding registers (`2100`, `2101`, `2102`, `2105`, `2107`, `2114`) and are disabled by default.
- Temperatures are scaled as tenths of a degree (`123` = `12.3 °C`) for the Aurora III profile.
- The JÅN module may overwrite heating setpoint register `2107` when weather compensation is active.

## Sources

- [Adlår Aurora III Pro manual](https://www.support.adlar.com/wp-content/uploads/2025/08/250606-Aurora-III-Pro-R290-Gebruikershandleiding.pdf)
- [Aurora III / Castra Aurora III Home Assistant Modbus package](https://github.com/Ambrosiussen/Adlar-Castra-Aurora-III-HomeAssistant)
- [Adlar/Home Assistant Modbus example](https://github.com/rhjbruins/adlar_homeassistant)
- [SolarEast/R290 Home Assistant integration](https://github.com/CNC-Buddy/R290_heatpump)
- [Seeed XIAO ESP32C6 pin map](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/)
- [ESPHome Modbus](https://esphome.io/components/modbus/)
- [ESPHome Modbus Controller](https://esphome.io/components/modbus_controller/)

## Contributing

Real hardware test reports are very welcome, especially with exact heat pump model, JÅN module details, RS485 transceiver type, wiring notes and which registers behave correctly. Please use the issue templates.

## Publishing Notes

This repository is designed so it can later be made public: it uses original code and documentation, avoids vendor logos and copied manual text, and clearly marks itself as an unofficial community project. See [docs/publication-notes.md](docs/publication-notes.md).

## License

MIT. See [LICENSE](LICENSE).
