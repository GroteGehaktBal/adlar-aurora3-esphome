# Troubleshooting

Start with the ESP powered over USB and the heat pump/JÅN wiring connected with the system in a stable, non-critical state. Test read-only behavior before changing setpoints or modes.

## No Modbus Data

Check these first:

1. Confirm the RS485 module is powered from XIAO `3V3`, not 5V.
2. For an XY-485-style board, confirm XIAO `D6/TX` goes to module `RXD` and XIAO `D7/RX` goes to module `TXD`.
3. Swap RS485 `A` and `B`.
4. Confirm `GND` is connected between the transceiver and JÅN port.
5. For an automatic-direction module, keep `flow_control_pin` out of the YAML and leave `D2` disconnected.
6. If your RS485 module needs manual direction, tie `DE` and `/RE` together, connect them to `D2`, and add `flow_control_pin: D2`.
7. Try slave ID `1` first, then verify with a Modbus scanner if needed.

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

## CRC Errors Or Intermittent Timeouts

Likely causes:

- `A/B` swapped or unstable wiring.
- Long cable without suitable termination/biasing.
- Another Modbus master is already active on the same RS485 bus.
- Polling too fast.

Keep `modbus_update_interval` at `30s` or slower during early testing. Do not lower `modbus_command_throttle` until the bus is stable.

## Temperatures Are 10x Wrong

Some related R290 protocol descriptions use tenths of a degree. This YAML uses whole degrees because the found Adlar/Home Assistant example does. If values are exactly 10x too high or low, adjust scaling consistently for both sensors and writable setpoints.

## Writes Do Not Stick

Check:

- Whether the heat pump is in a mode that allows that setting.
- Whether the JÅN module or original controller overwrites the value.
- Whether the register uses single-register write. This YAML uses `use_write_multiple: false` for writable controls.
- Whether your installation uses another master on the same RS485 bus.

## Suggested First Entities To Watch

- `Running status 1 raw`
- `Heat pump status text`
- `Return water temperature T6`
- `Supply water temperature T7`
- `Water flow`
- `Compressor frequency`

Only after these behave sensibly should you test `Heating water temperature setpoint`.
