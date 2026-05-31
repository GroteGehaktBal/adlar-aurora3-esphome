# Security And Safety

## Secrets

Never commit `secrets.yaml`, WiFi passwords, API encryption keys, OTA passwords or logs containing credentials. This repository includes `secrets.example.yaml` only.

## Operational Safety

This project can write settings to heating equipment. Treat write controls as operationally sensitive:

- Test read-only behavior first.
- Keep polling slow while validating RS485 stability.
- Avoid exposing installer/service parameters as ordinary Home Assistant controls.
- Confirm physical heating-system limits before changing setpoint ranges.

## Reporting

For credential leaks or software vulnerabilities, use GitHub's private vulnerability reporting if available. For register mistakes, unsafe defaults or hardware behavior, open an issue with logs and hardware details after removing secrets.
