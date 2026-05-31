# Contributing

Contributions are welcome, especially real-world hardware test reports. This project touches heating equipment, so careful, boring details are valuable.

## Good Issues Include

- Heat pump model and firmware if known.
- JÅN module version or photo/label details if known.
- ESP board and RS485 transceiver type.
- Wiring details, including whether the transceiver has automatic direction.
- ESPHome version.
- Relevant logs with secrets removed.
- Which read registers look correct.
- Which write controls were tested, and under what operating mode.

## Pull Request Guidelines

- Keep safety defaults conservative.
- Do not expose installer/service parameters as normal Home Assistant controls without a clear reason.
- Keep `secrets.yaml`, firmware binaries and local build output out of git.
- Run `esphome config adlar_aurora3_xiao_esp32c6.yaml` before opening a PR.
- Run `esphome compile adlar_aurora3_xiao_esp32c6.yaml` when changing YAML behavior.
- Update `docs/register-map.md` if you add, remove or rescale registers.

## Naming

The current Home Assistant entity names are Dutch because this project started with a Dutch Adlar Aurora III installation. An English variant or package split is welcome if it stays easy to maintain.
