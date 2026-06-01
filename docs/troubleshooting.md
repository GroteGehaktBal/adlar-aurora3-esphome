# Troubleshooting

Start with the ESP powered over USB and the heat pump/JÅN wiring connected with the system in a stable, non-critical state. Test read-only behavior before changing setpoints or modes.

## No Modbus Data

Check these first:

1. Confirm the current firmware is the Aurora III profile: `9600` baud, `8N2`, input-register reads and slave ID `1`.
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
