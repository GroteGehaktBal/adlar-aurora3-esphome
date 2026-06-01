# Troubleshooting

Start with the ESP powered over USB and the heat pump/JÅN wiring connected with the system in a stable, non-critical state. Test read-only behavior before changing setpoints or modes.

## No Modbus Data

Check these first:

1. Confirm the current firmware is really running. The bring-up firmware should show a `Firmware profile` entity with `bring-up 0.3.1 - 9600 8N2 - slave 1 and 251 probe`.
2. For an XY-485-style board, confirm XIAO `D6/TX` goes to module `RXD` and XIAO `D7/RX` goes to module `TXD`.
3. Confirm `GND` is connected between the transceiver and JÅN port. If the board has an `E` terminal, verify with a multimeter whether `E` is common with TTL-side `GND/DNG`; if not, add a proper signal-ground connection.
4. Swap RS485 `A` and `B` at one end only. Many RS485 boards and devices use opposite `A/B` naming.
5. For an automatic-direction module, keep `flow_control_pin` out of the YAML and leave `D2` disconnected.
6. If your RS485 module needs manual direction, tie `DE` and `/RE` together, connect them to `D2`, and add `flow_control_pin: D2`.
7. Try slave ID `1` first. If there is still no response after wiring is confirmed, try `251`, which appears in Aurora III protocol notes.

Useful LED check for XY-485-style boards:

- No TTL-side LED activity while ESPHome is online: check module power and XIAO `D6/D7` wiring.
- TX activity but no RX activity: check JÅN `A/B/GND`, then swap `A/B`.
- TX activity but no RX activity after the 0.2.0 firmware: check common ground and slave ID.
- TX and RX activity but all values stay `NA`: check baud rate, slave ID, parity, stop bits and register assumptions.

Do not use Home Assistant write controls until at least a few read-only sensors show plausible values.

## Passive Bus Sniffing

If both slave `1` and slave `251` time out with the bring-up firmware, flash `adlar_aurora3_xiao_esp32c6_sniffer.yaml`.

This sniffer profile does not intentionally transmit on RS485. It keeps XIAO `D6/TX` at UART idle-high for automatic-direction boards, listens on XIAO `D7/RX` at 9600 8N2, and writes raw received bytes to the ESPHome logs as `[uart_debug]` lines.

Results:

- `[uart_debug]` RX lines appear: the RS485 receive path works and the JÅN bus is active. Active Modbus failures are then more likely caused by bus ownership, timing or the request profile.
- No `[uart_debug]` RX lines appear: check XIAO `D7/RX` to RS485 module `TXD`, module power, JÅN `GND` to TTL-side `GND/DNG`, and both A/B orientations.
- RX lines appear only in one A/B orientation: keep that orientation for further tests.

## UART Loopback Test

If the sniffer profile shows no `[uart_debug]` RX lines in either A/B orientation, test the XIAO UART pins by themselves.

1. Power down or unplug the XIAO.
2. Disconnect the RS485 module from the XIAO.
3. Temporarily connect XIAO `D6` directly to XIAO `D7`.
4. Flash `adlar_aurora3_xiao_esp32c6_uart_loopback.yaml`.
5. Watch ESPHome logs for matching `[uart_debug]` TX and RX byte lines every 5 seconds.

If loopback works, the XIAO pins and ESPHome UART configuration are good. Focus next on the RS485 transceiver module or the JÅN bus connection. If loopback does not work, check that the board really exposes XIAO `D6` as GPIO16 and `D7` as GPIO17, and inspect solder joints on those pins.

With the sniffer firmware powered and the RS485 module connected, useful idle voltage checks are:

- XIAO `D6` / module `RXD` to GND: should sit near `3.3V`.
- Module `TXD` / XIAO `D7` to GND: should usually sit near `3.3V` when the RS485 receiver is idle.
- JÅN `GND`, module `DNG/GND`, and XIAO `GND`: should be continuous.

## Confirming The Flashed Firmware

The local ESPHome web UI is served by the ESP itself. Entity names therefore tell you which firmware is actually running.

If you flash `adlar_aurora3_xiao_esp32c6_bringup.yaml`, the web UI should show:

- `Firmware profile`
- `Bring-up slave 1 system status bits`
- `Bring-up slave 251 system status bits`
- `Bring-up status`

If the web UI still shows old names such as `Circulation pump active`, `Controller power`, `Heat pump defrost` or `Compressor target frequency`, the board is still running an older configuration. In ESPHome Builder, open the device's YAML with `EDIT`, replace the entire file with the bring-up YAML, save, use `Clean build files`, then install again.

The bring-up firmware intentionally sends only two Modbus requests per minute: one to slave `1` and one to slave `251`. If the `RXD` LED blinks constantly, an older or different firmware may still be running, or the failed requests are being retried by the Modbus stack.

## USB Port Disappears While Flashing

If the XIAO ESP32C6 already runs firmware that quickly enters deep sleep, the USB serial/JTAG device can appear for only a moment and then disappear again. Do not try to click the browser dialog faster. Put the board into ROM bootloader/download mode instead.

Recommended recovery sequence:

1. Disconnect the RS485 module and any external wiring while flashing the first time.
2. Unplug USB-C from the XIAO.
3. Press and hold the XIAO `BOOT` button. Do not hold `RESET`.
4. While still holding `BOOT`, plug the USB-C cable into the computer.
5. Wait a second, then release `BOOT`.
6. In ESPHome Web/Builder, click install/connect again and select the new `cu.usbmodem...` serial port.

Alternative sequence while the board is already plugged in:

1. Hold `BOOT`.
2. Tap `RESET` once.
3. Release `RESET`.
4. Release `BOOT`.
5. Try the ESPHome serial connection again.

On macOS, the correct port usually looks like `/dev/cu.usbmodemXXXX`. Ignore `cu.Bluetooth-Incoming-Port` and other unrelated ports. If the browser still cannot keep the port open, use Chrome/Edge or flash from the command line with `esptool` and the ESPHome factory binary.

References: [Seeed XIAO ESP32C6 BootLoader Mode](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#bootloader-mode) and [Espressif ESP32-C6 Boot Mode Selection](https://docs.espressif.com/projects/esptool/en/latest/esp32c6/advanced-topics/boot-mode-selection.html).

## Parallel JÅN Bus Notes

The existing JÅN wires should stay connected. This project taps the same RS485 bus in parallel, as public Aurora III community examples do. Because Modbus RTU does not arbitrate multiple masters, the firmware polls slowly and groups reads. If the heat pump or JÅN becomes unstable, disconnect the ESP/RS485 module and return to the original wiring.

Do not enable Home Assistant write entities until these read-only entities show plausible values:

- `System status bits`
- `Heat pump status text`
- `Inlet water temperature`
- `Outlet water temperature`
- `Water flow`
- `Compressor actual frequency`

## CRC Errors Or Intermittent Timeouts

Likely causes:

- `A/B` swapped or unstable wiring.
- Long cable without suitable termination/biasing.
- Another Modbus master is already active on the same RS485 bus.
- Polling too fast.
- Wrong stop-bit setting. Aurora III community configs use `8N2`, not `8N1`.

Keep `modbus_update_interval` at `60s` or slower during early testing. Do not lower `modbus_command_throttle` until the bus is stable.

## Temperatures Are 10x Wrong

The Aurora III profile uses tenths of a degree. If values are exactly 10x too high or low on your unit, adjust scaling consistently for both sensors and writable setpoints.

## Writes Do Not Stick

Check:

- Whether the heat pump is in a mode that allows that setting.
- Whether the JÅN module overwrites register `2107` through weather compensation.
- Whether the register uses single-register write. This YAML uses `use_write_multiple: false` for writable controls.
- Whether your installation uses another master on the same RS485 bus.

## Suggested First Entities To Watch

- `System status bits`
- `Heat pump status text`
- `Inlet water temperature`
- `Outlet water temperature`
- `Water flow`
- `Compressor actual frequency`

Only after these behave sensibly should you enable and test `Zone 1 heating setpoint`.
