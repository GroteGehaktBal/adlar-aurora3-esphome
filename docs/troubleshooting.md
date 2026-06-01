# Troubleshooting

Start with the ESP powered over USB and the heat pump/JÅN wiring connected with the system in a stable, non-critical state. Test read-only behavior before changing setpoints or modes.

## No Modbus Data

Check these first:

1. Confirm the RS485 module is powered at 3.3V.
2. Confirm XIAO `D6` goes to module `DI/TX-in` and XIAO `D7` goes to module `RO/RX-out`.
3. Swap RS485 `A` and `B`.
4. Confirm `GND` is connected between the transceiver and JÅN port.
5. If your RS485 module has automatic direction control, remove `flow_control_pin: D2`.
6. If your RS485 module needs manual direction, tie `DE` and `/RE` together and connect them to `D2`.
7. Try slave ID `1` first, then verify with a Modbus scanner if needed.

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
