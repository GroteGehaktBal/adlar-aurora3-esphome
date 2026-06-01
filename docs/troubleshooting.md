# Troubleshooting

Start with the ESP powered over USB and the heat pump/JÅN wiring connected with the system in a stable, non-critical state. Test read-only behavior before changing setpoints or modes.

## No Modbus Data

Check these first:

1. Confirm the current firmware is really running. The bring-up firmware should show a `Firmware profile` entity with `bring-up 0.3.1 - 9600 8N2 - slave 1 and 251 probe`.
2. For the tested XY-485-style board, confirm XIAO `D6/TX` goes to module `TXD` and XIAO `D7/RX` goes to module `RXD`.
3. If that never produces replies, also try the other TTL orientation: XIAO `D6/TX` to module `RXD` and XIAO `D7/RX` to module `TXD`. Some boards label TX/RX from the module's point of view.
4. Confirm `GND` is connected between the transceiver and JÅN port. If the board has an `E` terminal, verify with a multimeter whether `E` is common with TTL-side `GND/DNG`; if not, add a proper signal-ground connection.
5. Swap RS485 `A` and `B` at one end only. Many RS485 boards and devices use opposite `A/B` naming.
6. For an automatic-direction module, keep `flow_control_pin` out of the YAML and leave `D2` disconnected.
7. If your RS485 module needs manual direction, tie `DE` and `/RE` together, connect them to `D2`, and add `flow_control_pin: D2`.
8. Try slave ID `1` first. If there is still no response after wiring is confirmed, try `251`, which appears in Aurora III protocol notes.

Useful LED check for XY-485-style boards:

- No TTL-side LED activity while ESPHome is online: check module power and XIAO `D6/D7` wiring.
- TX activity but no RX activity: check JÅN `A/B/GND`, then swap `A/B`.
- TX activity but no RX activity after the 0.2.0 firmware: check common ground and slave ID.
- TX and RX activity but all values stay `NA`: check baud rate, slave ID, parity, stop bits and register assumptions.

Do not use Home Assistant write controls until at least a few read-only sensors show plausible values.

## Passive ESPHome Monitor

If the Arduino passive analyzer prints valid request/response pairs, flash `adlar_aurora3_xiao_esp32c6_passive_monitor.yaml` next. This is the safest Home Assistant profile for an already-active JÅN bus because it listens only and publishes observed values.

Expected signs:

- `Firmware profile` shows `passive-monitor 0.7.1 - stream parser`.
- `Passive valid Modbus frames` increases.
- `Passive request frames` and `Passive response frames` both increase.
- `Passive published values` increases when known registers are observed.
- `Passive ambient temperature`, `Passive outlet water temperature`, `Passive AC voltage`, or `Passive zone 1 heating setpoint current` become populated when the JÅN controller polls those registers.

If `Passive valid Modbus frames` increases but `Passive published values` stays at zero:

- If only `Passive response frames` increases, the ESP is seeing valid responses but not the matching requests. A Modbus response does not include the register address, so the firmware cannot safely map it to a Home Assistant entity yet.
- If only `Passive request frames` increases, the ESP is seeing the JÅN requests but not the heat-pump responses.
- If both request and response counters increase but `Passive ignored valid frames` also increases, check `Passive last observed address` and the first `passive_modbus` log lines; the register may not be mapped yet.

If the passive monitor only sees responses, try `adlar_aurora3_xiao_esp32c6_active_probe.yaml` before flashing the full main firmware. It sends slow read-only requests after a quiet bus window and exposes:

- `Active probe requests sent`
- `Active probe replies`
- `Active probe timeouts`
- `Active probe bus busy skips`
- the mapped temperature, voltage, flow, frequency and raw status entities

If `Active probe bus busy skips` increases but `Active probe requests sent` stays at zero, the bus never stayed quiet long enough for the conservative probe. If requests increase but replies do not, the ESP transmit path or bus ownership is still the likely problem.

## Active Sidecar Profile

Once the active probe gets replies, flash `adlar_aurora3_xiao_esp32c6_active_sidecar.yaml`. This is the complete profile for a shared JÅN bus. It reads all currently mapped registers and exposes guarded controls, but it still behaves politely: one request at a time, only after a quiet bus window.

Expected signs:

- `Firmware profile` shows `active-sidecar 1.0.0 - slow reads + guarded writes`.
- `Active sidecar read requests` increases.
- `Active sidecar read replies` increases shortly after the requests.
- `Active sidecar timeouts` stays near zero.
- `Active sidecar bus busy skips` may increase; this is normal and means the firmware is waiting while the existing JÅN traffic is active.
- Temperature entities such as `Inlet water temperature`, `Outlet water temperature`, and `Ambient temperature` populate within a few minutes.

The sidecar reads more than 60 registers one by one. A full first pass can take several minutes, especially when the bus is busy. Do not judge the firmware after only a few seconds.

If replies increase but many entities stay `NA`, watch `Active sidecar last address` and `Active sidecar last frame`. Some optional sensors only populate if the heat pump has that physical sensor or feature installed.

If `Active sidecar read requests` increases but `Active sidecar read replies` does not, return to the active probe and confirm wiring. The sidecar uses the same Modbus timing approach as the probe, so a sudden failure usually points to a wiring, bus-load, or firmware-selection issue.

For write testing:

1. Leave `Enable write controls` off while verifying read-only sensors.
2. Confirm the `... current` entity for the setting is populated, for example `Zone 1 heating setpoint current`.
3. Turn on `Enable write controls`.
4. Make one small change, preferably a harmless setpoint step.
5. Confirm `Active sidecar write requests` and `Active sidecar write replies` both increase.
6. Turn `Enable write controls` off again.

If the write is acknowledged but the value changes back later, the JÅN module or weather-compensation logic may be overwriting that register. This is expected behavior on some installations, especially for register `2107`.

## Passive Bus Sniffing

If both slave `1` and slave `251` time out with the bring-up firmware, flash `adlar_aurora3_xiao_esp32c6_sniffer.yaml`.

This sniffer profile does not intentionally transmit on RS485. It keeps XIAO `D6/TX` at UART idle-high for automatic-direction boards, listens on XIAO `D7/RX` at 9600 8N2, and writes raw received bytes to the ESPHome logs as `[uart_debug]` lines.

Results:

- `[uart_debug]` RX lines appear: the RS485 receive path works and the JÅN bus is active. Active Modbus failures are then more likely caused by bus ownership, timing or the request profile.
- No `[uart_debug]` RX lines appear: check XIAO `D7/RX` to RS485 module `RXD` for the tested XY-485 orientation, module power, JÅN `GND` to TTL-side `GND/DNG`, and both A/B orientations.
- RX lines appear only in one A/B orientation: keep that orientation for further tests.

## UART Loopback Test

If the sniffer profile shows no `[uart_debug]` RX lines in either A/B orientation, test the XIAO UART pins by themselves.

1. Power down or unplug the XIAO.
2. Disconnect the RS485 module from the XIAO.
3. Temporarily connect XIAO `D6` directly to XIAO `D7`.
4. Flash `adlar_aurora3_xiao_esp32c6_uart_loopback.yaml`.
5. Watch ESPHome logs for matching `[uart_debug]` TX and RX byte lines every 5 seconds.

If XIAO `D6` and `D7` are hard to reach, leave the RS485 module connected to the XIAO and temporarily short the module TTL-side `TXD` and `RXD` pins instead. Keep RS485 `A/B/GND` disconnected from the JÅN module during this test. This is easier to do at the RS485 module connector, but it only proves the ESP UART pins and TTL wiring. It does not prove that the RS485 differential side can talk to the heat pump.

If loopback works, the XIAO pins and ESPHome UART configuration are good. Focus next on the RS485 transceiver module or the JÅN bus connection. If loopback does not work, check that the board really exposes XIAO `D6` as GPIO16 and `D7` as GPIO17, and inspect solder joints on those pins.

With the sniffer firmware powered and the RS485 module connected, useful idle voltage checks are:

- XIAO `D6` / module `TXD` to GND: should sit near `3.3V`.
- Module `RXD` / XIAO `D7` to GND: should usually sit near `3.3V` when the RS485 receiver is idle.
- JÅN `GND`, module `DNG/GND`, and XIAO `GND`: should be continuous.

## RS485 Transceiver Link Test

If you have a known-good USB-RS485 adapter or a second RS485 module, test through the RS485 differential side. This does not use the heat pump or JÅN module. If you do not have a second RS485 device, skip this test and use the TTL-side loopback above.

1. Power down or unplug the XIAO.
2. Keep the RS485 module connected to the XIAO TTL side:
   `D6/TX` to module `TXD`, `D7/RX` to module `RXD`, `3V3` to `VCC`, and `GND` to `GND/DNG` for the tested XY-485 orientation.
3. Disconnect RS485 `A/B/GND` from the JÅN module.
4. Connect module `A/B/GND` to a known-good USB-RS485 adapter or a second RS485 transceiver.
5. Flash `adlar_aurora3_xiao_esp32c6_rs485_link_test.yaml`.
6. Set the other side to `9600 8N2`.
7. The other side should receive `ADLAR485` every 5 seconds. Bytes sent from the other side should appear in ESPHome as `[uart_debug]` RX lines.

Do not short RS485 `A` to `B` for a one-board loopback test. On many half-duplex automatic-direction boards the receiver is disabled while transmitting, and shorting the differential pair can hide the real problem or overload the driver.

If this link test works, the XIAO and RS485 transceiver are basically functional. If it still cannot communicate with the JÅN module, focus on bus ownership, JÅN terminal wiring, A/B naming, termination/biasing, or whether the JÅN port is only active in a specific operating mode. If this link test fails, replace the RS485 transceiver with a known 3.3V-safe module such as an SP3485/MAX3485-based board or use a proven industrial RS485 gateway.

## RS485 Bus Voltage Versus ESP Logic Voltage

Measuring around 5V on JÅN `A/B` does not mean the ESP board or TTL UART side should be powered from 5V. RS485 has two separate sides:

- `A/B`: differential bus side. This can sit around a 5V idle or bias voltage and still be normal.
- `TXD/RXD`: local TTL side between the transceiver and ESP. This must stay within the ESP GPIO voltage range.

For ESP32-C6 boards, treat GPIO pins as 3.3V logic. A dev board may expose a `5V` pin for powering external modules, but its GPIO pins are still not 5V tolerant.

If a 5V MAX485-style module is used anyway, protect the ESP RX pin. A safe pattern is:

- ESP `TX` at 3.3V can usually drive module `RXD/DI` high enough for a 5V transceiver.
- Module `TXD/RO` back into ESP `RX` must be level shifted or divided down if it rises near 5V.

Do not connect a module powered from 5V directly to ESP `RX` until module `TXD` has been measured while disconnected from the ESP.

For a safe 5V test with a classic Arduino Uno/Nano/Mega or with an ESPHome sniffer plus voltage divider, see [5V RS485 module test](5v-rs485-module-test.md).

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
