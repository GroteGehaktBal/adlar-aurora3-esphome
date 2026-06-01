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
- Which firmware profile was tested, especially `active_sidecar`, `active_probe` or `passive_monitor`.
- Which write controls were tested, and under what operating mode.

## Pull Request Guidelines

- Keep safety defaults conservative.
- Do not expose installer/service parameters as normal Home Assistant controls without a clear reason.
- Keep `secrets.yaml`, firmware binaries and local build output out of git.
- Run `esphome config adlar_aurora3_xiao_esp32c6_active_sidecar.yaml` before opening a PR that changes the main sidecar behavior.
- Run `esphome compile adlar_aurora3_xiao_esp32c6_active_sidecar.yaml` when changing YAML behavior.
- Update `docs/register-map.md` if you add, remove or rescale registers.

## Naming

User-facing names should stay in English so the project remains useful internationally. Product names such as Adlar, Adlår, Aurora III and JÅN may be used when needed to identify compatible hardware.
