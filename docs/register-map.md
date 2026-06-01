# Register Map

This project uses zero-based Modbus register addresses, matching ESPHome and Home Assistant Modbus examples. If a vendor document uses `40001` style notation, map carefully before copying values.

## Communication Defaults

| Setting | Value |
| --- | --- |
| Modbus role | ESPHome client/master |
| Slave ID | `1` |
| Baud rate | `9600` |
| Data bits | `8` |
| Parity | `NONE` |
| Stop bits | `1` |
| Poll interval | `30s` |
| Command throttle | `250ms` |

## Main Read Registers

| Address | Entity | Type | Scale/unit | Notes |
| ---: | --- | --- | --- | --- |
| `0x0000` | Running status 1 raw | `U_WORD` | raw | Source for status text and several status bits |
| `0x0001` | Running status 2 raw | `U_WORD` | raw | Source for controller-on bit |
| `0x0002` | Fault state 1 raw | `U_WORD` | raw | Source for fault summary |
| `0x0003` | Fault state 2 raw | `U_WORD` | raw | Source for fault summary |
| `0x0004` | Fault state 3 raw | `U_WORD` | raw | Source for fault summary |
| `0x0027` | Compressor target frequency | `S_WORD` | Hz | Target frequency |
| `0x0040` | Compressor frequency | `S_WORD` | Hz | Running frequency |
| `0x0041` | Fan speed | `S_WORD` | rpm | Community sources differ on label/unit; verify locally |
| `0x0044` | AC input voltage | `S_WORD` | V | Measurement |
| `0x0045` | AC input current | `S_WORD` | x0.1 A | Measurement |
| `0x0046` | Compressor phase current | `S_WORD` | x0.1 A | Measurement |
| `0x0047` | Compressor IPM temperature | `S_WORD` | °C | Measurement |
| `0x004A` | Ambient temperature T1 | `S_WORD` | °C | Measurement |
| `0x004F` | Return water temperature T6 | `S_WORD` | °C | Water inlet/return |
| `0x0050` | Supply water temperature T7 | `S_WORD` | °C | Water outlet/supply |
| `0x0057` | Pump PWM value | `S_WORD` | raw | Unit intentionally left blank |
| `0x0058` | Water flow | `S_WORD` | L/min | Measurement |
| `0x005C` | Input power | `S_WORD` | x0.01 kW | Disabled by default |
| `0x005D` | Total heat pump electricity consumption | `U_WORD` | kWh | Disabled by default; verify wrap/reset behavior |

## Write Registers

| Address | Entity | Range/options | Default exposure | Notes |
| ---: | --- | --- | --- | --- |
| `0x0300` | Cooling water temperature setpoint | 7-25 °C | Disabled by default | Cooling setpoint |
| `0x0301` | Heating water temperature setpoint | 20-45 °C | Enabled | Heating setpoint, deliberately conservative |
| `0x0304` | Operating mode | Cooling, heating, domestic hot water, floor modes | Enabled | Verify mode support on your installation |
| `0x0305` | Heat pump power | Off/On | Enabled | Test carefully |
| `0x0306` | Indoor setpoint | 10-25 °C | Disabled by default | Only useful for room-control setups |
| `0x0307` | Power mode | Standard, high power, silent | Enabled | Maps to frequency conversion mode |
| `0x0313` | Cooling curve | Off, H1-H8, L1-L8 | Disabled by default | H/L mapping based on an Adlar community example |
| `0x0314` | Heating curve | Off, H1-H8, L1-L8 | Disabled by default | Some R290 maps only expose curve 1-8 |

## Scaling Notes

The current YAML treats temperatures as whole degrees, because the found Adlar/Home Assistant example does the same. Some related protocol notes describe temperature values as tenths of a degree. If all temperatures are exactly 10x too high, add `multiply: 0.1` filters to temperature sensors and adjust write scaling for setpoints.

## Hardware Feedback Wanted

If you test this on real hardware, please open a hardware test report with:

- Heat pump model and firmware if known.
- JÅN module version if known.
- Whether the JÅN port is a dedicated slave/server port or a shared RS485 bus.
- Which registers read correctly.
- Which write controls were tested.
- Any Modbus timeout, CRC, or collision symptoms.
