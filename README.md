# Adlar Aurora III ESPHome Modbus

![ESPHome](https://img.shields.io/badge/ESPHome-2026.5%2B-2f7d95)
![Home Assistant](https://img.shields.io/badge/Home%20Assistant-ready-41bdf5)
![Hardware](https://img.shields.io/badge/hardware-Seeed%20XIAO%20ESP32--C6-6f42c1)
![Status](https://img.shields.io/badge/status-passive%20bus%20verified%2C%20active%20control%20experimental-d29922)
![License](https://img.shields.io/badge/license-MIT-2da44e)

ESPHome configuration for monitoring and limited local control of an Adlår/Adlar Aurora III R290 heat pump through the JÅN module using Modbus RTU over RS485.

This repository is intended as a clean, reusable starting point for Home Assistant users who want a local, cloud-independent integration for an Adlar Aurora III or a related SolarEast/Sunrain R290 heat pump.

> Status: the ESPHome configurations have been validated and successfully compiled for the Seeed Studio XIAO ESP32C6. Live JÅN bus traffic has been decoded with the Arduino passive analyzer at 9600 baud, 8N2, slave `1`. The passive ESPHome monitor is the safest first Home Assistant profile; active polling/control remains experimental on an already-active JÅN bus.

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

The JÅN RS485 `A/B` terminals may measure around 5V because they are the differential bus side. That does not mean the ESP UART `TXD/RXD` side should use 5V. Prefer a 3.3V-safe RS485 transceiver for direct ESP wiring, or level-shift the transceiver `TXD/RO` output before feeding it into ESP `RX`.

If you need to test whether a MAX485-style board requires 5V, see [docs/5v-rs485-module-test.md](docs/5v-rs485-module-test.md). The repository also includes an Arduino test sketch for classic 5V Arduino boards; it can passively sniff bytes and actively send slow Modbus read probes.

## Quick Start

1. Clone this repository or import the YAML into ESPHome.
2. Copy `secrets.example.yaml` to your own ESPHome `secrets.yaml`.
3. Fill in your WiFi, API and OTA secrets.
4. Check the wiring in [docs/wiring.md](docs/wiring.md).
5. For a first safe install on an existing JÅN bus, compile and flash `adlar_aurora3_xiao_esp32c6_passive_monitor.yaml`.
6. Watch `Passive valid Modbus frames`, `Passive request frames`, `Passive response frames`, `Passive published values`, `Passive ambient temperature`, `Passive outlet water temperature`, `Passive AC voltage`, and `Passive zone 1 heating setpoint current`.
7. Only move to active polling or write controls after passive data is stable and plausible.

```bash
esphome config adlar_aurora3_xiao_esp32c6_passive_monitor.yaml
esphome compile adlar_aurora3_xiao_esp32c6_passive_monitor.yaml
```

The older active profile is still available as `adlar_aurora3_xiao_esp32c6.yaml`, but it transmits Modbus requests. Use it only after you understand the bus-ownership notes below.

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

## Passive ESPHome Monitor

After the Arduino passive analyzer sees valid request/response pairs, flash `adlar_aurora3_xiao_esp32c6_passive_monitor.yaml`. This profile also listens only: it does not create a Modbus controller and does not transmit requests. It parses the existing JÅN bus stream, pairs read requests with their responses, and publishes the observed values as Home Assistant entities.

This is currently the best profile for side-by-side use with the JÅN module because the JÅN/internal controller already owns the Modbus conversation.

If `Passive valid Modbus frames` increases but `Passive published values` stays at zero, check the diagnostic counters:

- `Passive request frames` and `Passive response frames` both increase: the parser sees both sides and should publish once a mapped register appears.
- `Passive response frames` increases but `Passive request frames` stays at zero: the ESP is only seeing one side of the RS485 conversation, so the response cannot be mapped to a register address yet.
- `Passive unmatched responses` increases: the ESP sees response frames without the matching request frame.

## Active Read Probe Firmware

If the passive monitor sees only response frames, flash `adlar_aurora3_xiao_esp32c6_active_probe.yaml` before trying the full active profile. This firmware sends only slow read-only Modbus requests and waits for a quiet bus before each request. It never writes setpoints or modes.

Watch `Active probe requests sent`, `Active probe replies`, `Active probe timeouts`, and `Active probe bus busy skips`. If replies increase and the temperature/voltage entities populate, the XIAO can actively read the heat pump and the full active profile can be tested carefully later.

## UART Loopback Test

If the sniffer logs no RX bytes, flash `adlar_aurora3_xiao_esp32c6_uart_loopback.yaml`. Disconnect the RS485 module, temporarily short XIAO `D6` to XIAO `D7`, and watch the logs. Matching `[uart_debug]` TX and RX lines prove the ESPHome UART pins are working before replacing or retesting the RS485 transceiver.

If XIAO `D6` and `D7` are hard to reach, you can leave the RS485 module connected to the XIAO and temporarily short the module TTL-side `TXD` and `RXD` pins instead. Keep the module disconnected from the JÅN `A/B/GND` terminals while doing this. This tests the ESP UART pins and wiring to the module, but it does not test the RS485 differential transceiver.

## RS485 Link Test

If you have a known-good USB-RS485 adapter or a second RS485 transceiver, flash `adlar_aurora3_xiao_esp32c6_rs485_link_test.yaml`. Keep the RS485 module connected to the XIAO, disconnect it from the JÅN module, and connect RS485 `A/B/GND` to the known-good adapter/transceiver. The firmware sends `ADLAR485` every 5 seconds at 9600 8N2 and logs both TX and RX bytes.

Do not short RS485 `A` to `B` as a loopback test. A real RS485 transceiver test needs a second receiver/transceiver on the bus.

## Wiring Summary

| XIAO ESP32C6 | GPIO | XY-485 / automatic-direction RS485 module |
| --- | ---: | --- |
| D6 / TX | GPIO16 | TXD |
| D7 / RX | GPIO17 | RXD |
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

The passive monitor does not act as a Modbus client/master. It listens to the existing JÅN/internal Modbus conversation and is therefore the safest profile for side-by-side monitoring.

The active ESPHome profile does act as an additional Modbus client/master. RS485 supports multiple electrical nodes on the same pair, but Modbus RTU has no real bus arbitration. Public Aurora III community setups work by tapping the same JÅN/internal bus and polling gently.

The active configuration therefore polls gently: every 60 seconds by default with a 750 ms command throttle and 500 ms send wait time. Do not reduce those intervals until the bus has been proven stable.

See [docs/troubleshooting.md](docs/troubleshooting.md) for symptoms such as CRC errors, no response, temperatures that are off by a factor of 10, or write values that do not stick.

## Repository Contents

| Path | Purpose |
| --- | --- |
| `adlar_aurora3_xiao_esp32c6.yaml` | Main ESPHome configuration |
| `adlar_aurora3_xiao_esp32c6_passive_monitor.yaml` | Passive ESPHome monitor that publishes observed JÅN bus values without transmitting |
| `adlar_aurora3_xiao_esp32c6_active_probe.yaml` | Slow read-only active probe for cases where passive monitoring sees only responses |
| `adlar_aurora3_xiao_esp32c6_bringup.yaml` | Minimal one-register-per-minute first-contact firmware |
| `adlar_aurora3_xiao_esp32c6_sniffer.yaml` | Passive RS485 receive-only sniffer firmware |
| `adlar_aurora3_xiao_esp32c6_uart_loopback.yaml` | Local XIAO D6/D7 UART loopback test firmware |
| `adlar_aurora3_xiao_esp32c6_rs485_link_test.yaml` | RS485 transceiver test firmware for a USB-RS485 adapter or second transceiver |
| `secrets.example.yaml` | Example secrets file |
| `docs/wiring.md` | Wiring and RS485 notes |
| `docs/5v-rs485-module-test.md` | Safe test procedure for 5V MAX485-style modules |
| `docs/register-map.md` | Register overview, scaling and writable addresses |
| `docs/troubleshooting.md` | Troubleshooting and first-test procedure |
| `tools/arduino_rs485_sniffer/` | Arduino sketch for passive and active 5V RS485 receive testing |
| `tools/arduino_rs485_slave_scan/` | Arduino sketch for a slow read-only Modbus slave/register scan |
| `tools/arduino_rs485_passive_analyzer/` | Arduino sketch for decoding existing Modbus frames without transmitting |
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
