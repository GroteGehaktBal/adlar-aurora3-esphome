# Roadmap

This list is intentionally conservative. Heat pump integrations should become more useful without making unsafe behavior easy.

## High-Value Improvements

- Collect real hardware test reports for different Adlar/JÅN module revisions.
- Confirm whether the JÅN RS485 port is a dedicated slave/server port or a shared bus in common installations.
- Add a verified compatibility table.
- Add an optional read-only YAML variant with all write controls removed.
- Add an optional advanced YAML variant for users who explicitly want disabled-by-default curve controls.
- Document known-good RS485 transceiver modules.
- Add screenshots of the ESPHome entities and Home Assistant device page.

## Register Work

- Verify temperature scaling across multiple installations.
- Verify fan speed unit, because community sources differ.
- Verify pump PWM unit and range.
- Verify total energy counter wrap/reset behavior.
- Expand the fault bit mapping with confirmed descriptions.
- Add optional sensors for extra registers only after real-world validation.

## Home Assistant Experience

- Add suggested Lovelace dashboard snippets.
- Add template sensors for COP estimation when electrical input power is confirmed.
- Add automations/examples for read-only alerts.
- Add safety-first example automations for setpoint changes.

## Project Quality

- Add CI that validates the ESPHome YAML on pull requests.
- Add a hardware-test matrix in the README.
- Add release tags for known-good configurations.
- Add a public discussion category for installation reports once the repository is public.
