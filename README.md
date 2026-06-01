# Adlar Aurora III ESPHome Modbus

![ESPHome](https://img.shields.io/badge/ESPHome-2026.5%2B-2f7d95)
![Home Assistant](https://img.shields.io/badge/Home%20Assistant-ready-41bdf5)
![Hardware](https://img.shields.io/badge/hardware-Seeed%20XIAO%20ESP32--C6-6f42c1)
![Status](https://img.shields.io/badge/status-compile--tested%2C%20hardware%20verification%20needed-d29922)
![License](https://img.shields.io/badge/license-MIT-2da44e)

ESPHome configuration for monitoring and limited local control of an Adlår/Adlar Aurora III R290 heat pump through the JÅN module using Modbus RTU over RS485.

This repository is intended as a clean, reusable starting point for Home Assistant users who want a local, cloud-independent integration for an Adlar Aurora III or a related SolarEast/Sunrain R290 heat pump.

> Status: the ESPHome configuration has been validated and successfully compiled for the Seeed Studio XIAO ESP32C6. The register set is based on public Adlar/SolarEast/R290 community configurations. Real hardware behavior must still be verified carefully per installation.

## Unofficial Project

This is an independent community project. It is not affiliated with, endorsed by, sponsored by, or supported by Adlår/Adlar or SolarEast. Product names are used only to identify compatible hardware.

## What This Project Does

- Reads status bits, fault bits, temperatures, water flow, compressor frequency, current and voltage.
- Exposes limited Home Assistant controls: heating setpoint, cooling setpoint, operating mode, on/off and power mode.
- Keeps riskier settings such as weather-compensation curve selection disabled by default.
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
6. Start with read-only monitoring. Watch `Return water temperature T6`, `Supply water temperature T7`, `Water flow` and `Running status 1 raw`.
7. Only enable write controls in Home Assistant after read data is stable and plausible.

```bash
esphome config adlar_aurora3_xiao_esp32c6.yaml
esphome compile adlar_aurora3_xiao_esp32c6.yaml
```

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

The default YAML is configured for automatic-direction RS485 modules. If you use a basic manual-direction MAX485 module, wire `DE` and `/RE` to `D2` and add `flow_control_pin: D2` to the `modbus:` block.

More detail and a diagram are available in [docs/wiring.md](docs/wiring.md).

## Important Safety Notes

ESPHome acts as the Modbus master/client in this configuration. Connect it directly only if the JÅN port is intended to behave as a Modbus slave/server interface, or start with read-only monitoring. If the JÅN module itself is already acting as a master on the same RS485 bus, there is no real bus arbitration and frames can collide.

The configuration therefore polls gently: every 30 seconds by default with a 250 ms command throttle. Do not reduce those intervals until the bus has been proven stable.

See [docs/troubleshooting.md](docs/troubleshooting.md) for symptoms such as CRC errors, no response, temperatures that are off by a factor of 10, or write values that do not stick.

## Repository Contents

| Path | Purpose |
| --- | --- |
| `adlar_aurora3_xiao_esp32c6.yaml` | Main ESPHome configuration |
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

- The Aurora III is assumed to use the same Modbus RTU register layer as the SolarEast/Sunrain R290 BLN/TC family.
- Modbus defaults: slave ID `1`, 9600 baud, 8 data bits, no parity, 1 stop bit.
- Temperatures are treated as whole degrees because the found Adlar/Home Assistant configuration does the same. If your values are exactly 10x too high, adjust temperature scaling.
- Weather-compensation curve selects are deliberately `disabled_by_default: true`. Some register maps expose only curve 1-8, while the found Adlar example also uses H/L curve values.
- The heating setpoint is limited to 20-45 °C. Raise this only if your heating system is designed for it.

## Sources

- [Adlår Aurora III Pro manual](https://www.support.adlar.com/wp-content/uploads/2025/08/250606-Aurora-III-Pro-R290-Gebruikershandleiding.pdf)
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
