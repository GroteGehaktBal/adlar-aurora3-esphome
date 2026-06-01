# Changelog

## 0.2.0 - 2026-06-01

- Switched the main ESPHome configuration to the Aurora III/Castra Aurora III protocol profile: 9600 8N2, input-register telemetry and holding-register controls.
- Reduced default bus load for parallel JÅN-bus use with 60s polling, 750ms command throttle and grouped register reads.
- Disabled write controls by default until read-only monitoring is stable.
- Updated wiring and troubleshooting notes for parallel connection to the existing JÅN Modbus terminals.
- Validated and compiled with ESPHome 2026.5.1.

## 0.1.0 - 2026-06-01

- Added ESPHome configuration for Adlar Aurora III / SolarEast R290 via JÅN RS485 Modbus RTU.
- Targeted Seeed Studio XIAO ESP32C6 with ESP-IDF.
- Added read sensors, diagnostics and conservative Home Assistant controls.
- Added public-ready documentation, issue templates, license and repository metadata.
- Converted user-facing documentation and Home Assistant entity names to English for international use.
- Validated and compiled with ESPHome 2026.5.1.
