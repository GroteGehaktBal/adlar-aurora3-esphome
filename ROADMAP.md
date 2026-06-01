# Roadmap

This list is intentionally conservative. Heat pump integrations should become more useful without making unsafe behavior easy.

## High-Value Improvements

- Collect real hardware test reports for different Adlar/JÅN module revisions.
- Confirm whether the JÅN RS485 port is a dedicated slave/server port or a shared bus in common installations.
- Add a verified compatibility table.
- Validate the active sidecar profile on more installations and transceiver modules.
- Add a slim read-only sidecar variant once the full active sidecar is stable across more homes.
- Document known-good RS485 transceiver modules.
- Add screenshots of the ESPHome entities and Home Assistant device page.

## Register Work

- Verify temperature scaling across multiple installations.
- Identify confirmed Modbus registers, if any, for selecting weather-compensation curves and editing custom curve points.
- Confirm how JÅN dynamic weather compensation combines room setpoint, room temperature and water-temperature target.
- Verify fan speed unit, because community sources differ.
- Verify pump PWM unit and range.
- Verify total energy counter wrap/reset behavior.
- Expand the fault bit mapping with confirmed descriptions.
- Confirm which optional sensors are present on common one-zone, two-zone, buffer-tank, and DHW setups.

## Home Assistant Experience

- Add suggested Lovelace dashboard snippets.
- Add a polished Home Assistant dashboard example for the active sidecar profile.
- Add automations/examples for read-only alerts.
- Add safety-first example automations for setpoint changes.
- Consider a Home Assistant `climate` abstraction only after mode and setpoint write behavior is confirmed on multiple installations.

## Project Quality

- Add CI that validates the ESPHome YAML on pull requests.
- Add a hardware-test matrix in the README.
- Add release tags for known-good configurations.
- Add a public discussion category for installation reports once the repository is public.
