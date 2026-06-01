# Changelog

## 0.6.6 - 2026-06-01

- Added Arduino RX idle-state diagnostics and all-zero reply warnings to distinguish real bus bytes from a stuck-low RX line.

## 0.6.5 - 2026-06-01

- Documented ambiguous `TXD/RXD` labeling on RS485-to-TTL boards and added the alternate TTL wiring test.

## 0.6.4 - 2026-06-01

- Added a slow Arduino read-only Modbus slave/register scanner for harder first-contact tests.

## 0.6.3 - 2026-06-01

- Changed the Arduino RS485 sketch from passive-only sniffing to a combined passive sniffer and slow active Modbus probe.
- Added a TTL echo check for the Arduino plus RS485 module wiring.

## 0.6.2 - 2026-06-01

- Added a safe 5V MAX485-style module test guide.
- Added a passive Arduino RS485 byte sniffer sketch for classic 5V Arduino boards.

## 0.6.1 - 2026-06-01

- Clarified the difference between RS485 `A/B` bus voltage and ESP-side `TXD/RXD` logic voltage.
- Added safe notes for testing or powering 5V MAX485-style modules with ESP32-C6 boards.

## 0.6.0 - 2026-06-01

- Added an RS485 link-test firmware that sends `ADLAR485` every 5 seconds through the transceiver at 9600 8N2.
- Documented why a one-board RS485 `A`-to-`B` short is not a valid loopback test.
- Added guidance for testing against a USB-RS485 adapter or second RS485 transceiver.

## 0.5.0 - 2026-06-01

- Added a local XIAO D6/D7 UART loopback firmware for proving the ESPHome UART pins independently from the RS485 transceiver and JÅN bus.
- Documented idle voltage checks for the RS485 TTL side.

## 0.4.1 - 2026-06-01

- Updated the passive sniffer to declare XIAO `D6/TX` so automatic-direction RS485 boards see a stable UART idle-high input while sniffing.

## 0.4.0 - 2026-06-01

- Added a passive RX-only sniffer firmware for checking whether the XIAO and RS485 transceiver can hear existing JÅN bus traffic.
- Documented how to interpret `[uart_debug]` RX logs before continuing active Modbus polling tests.

## 0.3.1 - 2026-06-01

- Updated bring-up firmware to probe both likely Aurora III Modbus slave IDs: `1` and `251`.
- Added separate diagnostic sensors for slave `1` and slave `251` first-contact testing.
- Disabled Modbus command retries in the bring-up firmware so failed probes stay easy to read in logs.

## 0.3.0 - 2026-06-01

- Added a minimal bring-up firmware that exposes a visible firmware marker and sends only one Modbus read per minute.
- Documented how to confirm that the expected firmware is actually running before debugging RS485 wiring.

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
