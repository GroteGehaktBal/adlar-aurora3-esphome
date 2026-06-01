# Register Map

This project uses zero-based Modbus register addresses, matching ESPHome and Home Assistant Modbus examples. If a vendor document uses `30001` or `40001` style notation, map carefully before copying values.

## Communication Defaults

| Setting | Value |
| --- | --- |
| Modbus role | Passive monitor, read-only active probe, or active sidecar client |
| Slave ID | `1` by default, try `251` if needed |
| Baud rate | `9600` |
| Data bits | `8` |
| Parity | `NONE` |
| Stop bits | `2` |
| Input registers | Function code 4 |
| Holding register readback | Function code 3 |
| Holding register writes | Function code 6 in the active sidecar |
| Active sidecar pace | One register per request, every 2s when the bus is quiet |
| Quiet window | At least 350 ms after the last observed RX bytes |

## Main Input Registers

| Address | Entity | Type | Scale/unit | Notes |
| ---: | --- | --- | --- | --- |
| `1` | Thermostat type raw | `U_WORD` | raw | Configuration/diagnostic |
| `2` | Zone config raw | `U_WORD` | raw | Configuration/diagnostic |
| `7` | Refrigerant type raw | `U_WORD` | raw | Configuration/diagnostic |
| `9` | Effective mode raw | `U_WORD` | raw | Bitfield: cooling/heating/DHW effective |
| `12` | Zone 1 heating max temperature | `S_WORD` | x0.1 °C | Reported limit |
| `13` | Zone 1 heating min temperature | `S_WORD` | x0.1 °C | Reported limit |
| `14` | Zone 1 DHW max temperature | `S_WORD` | x0.1 °C | Reported limit |
| `15` | Zone 1 DHW min temperature | `S_WORD` | x0.1 °C | Reported limit |
| `30` | Room temperature max | `S_WORD` | x0.1 °C | Reported limit |
| `31` | Room temperature min | `S_WORD` | x0.1 °C | Reported limit |
| `35` | Zone 1 terminal heating raw | `U_WORD` | raw | Terminal/configuration diagnostic |
| `38` | System status bits | `U_WORD` | raw | Source for oil return, defrost, anti-freezing and status text |
| `39` | External heat request raw | `U_WORD` | raw | External demand diagnostic |
| `40` | Room temperature | `S_WORD` | x0.1 °C | Room sensor value, if populated |
| `41` | Total leaving water temperature | `S_WORD` | x0.1 °C | Overall leaving water sensor |
| `42` | Inlet water temperature | `S_WORD` | x0.1 °C | Water inlet/return |
| `43` | Outlet water temperature | `S_WORD` | x0.1 °C | Water outlet/supply |
| `44` | Buffer tank upper temperature | `S_WORD` | x0.1 °C | May be `0`/unpopulated if no buffer sensor |
| `45` | Buffer tank lower temperature | `S_WORD` | x0.1 °C | May be `0`/unpopulated if no buffer sensor |
| `46` | DHW tank temperature | `S_WORD` | x0.1 °C | May be `0`/unpopulated if no DHW sensor |
| `47` | Zone 2 mixing station temperature | `S_WORD` | x0.1 °C | Optional zone 2 sensor |
| `48` | Solar water heating temperature | `S_WORD` | x0.1 °C | Optional solar input |
| `49` | Outdoor unit coil temperature | `S_WORD` | x0.1 °C | Refrigerant/coil sensor |
| `50` | Ambient temperature | `S_WORD` | x0.1 °C | Outdoor ambient |
| `51` | EXV outlet temperature | `S_WORD` | x0.1 °C | Refrigerant circuit |
| `52` | Discharge temperature | `S_WORD` | x0.1 °C | Compressor discharge |
| `53` | Suction temperature | `S_WORD` | x0.1 °C | Compressor suction |
| `56` | Target outlet temperature | `S_WORD` | x0.1 °C | Target water outlet |
| `60` | Inlet water pressure | `U_WORD` | x0.1 bar | Hydraulic pressure |
| `61` | Outlet water pressure | `U_WORD` | x0.1 bar | Hydraulic pressure |
| `62` | Pump PWM output | `U_WORD` | % | Pump command |
| `63` | Pump PWM feedback | `U_WORD` | % | Pump feedback candidate |
| `64` | Water flow | `U_WORD` | x0.1 m³/h | Used for thermal power estimate |
| `66` | Climate curve cooling set | `S_WORD` | x0.1 °C | What the internal/JÅN curve calculates |
| `67` | Climate curve heating set | `S_WORD` | x0.1 °C | What the internal/JÅN curve calculates |
| `70` | EEV opening | `U_WORD` | steps | Electronic expansion valve |
| `72` | Fan speed | `U_WORD` | rpm | Outdoor fan |
| `74` | AC voltage | `U_WORD` | V | May be zero on some firmwares |
| `75` | AC current | `U_WORD` | x0.1 A | May be zero on some firmwares |
| `76` | DC bus voltage | `U_WORD` | V | Inverter DC bus |
| `77` | Compressor current | `U_WORD` | x0.1 A | Compressor/inverter current |
| `78` | Compressor target frequency | `U_WORD` | Hz | Target frequency |
| `79` | Compressor actual frequency | `U_WORD` | Hz | Running frequency |
| `80` | Main water pump status raw | `U_WORD` | raw | `1` means on |
| `81` | Four-way valve status raw | `U_WORD` | raw | Cooling/heating cycle valve candidate |
| `86` | High pressure | `U_WORD` | kPa | Refrigerant pressure |
| `87` | Low pressure | `U_WORD` | kPa | Refrigerant pressure |
| `90` | Error E01-E16 raw | `U_WORD` | raw | Fault bitfield |
| `91` | Error E17-E32 raw | `U_WORD` | raw | Fault bitfield |
| `96` | Error P01-P16 raw | `U_WORD` | raw | Protection bitfield |
| `97` | Error P17-P32 raw | `U_WORD` | raw | Protection bitfield |
| `101` | Target operation mode raw | `U_WORD` | raw | Operation mode diagnostic |
| `103` | Actual operation mode raw | `U_WORD` | raw | Operation mode diagnostic |

## Holding Registers

| Address | Entity | Range/options | Default exposure | Notes |
| ---: | --- | --- | --- | --- |
| `2100` | HVAC mode | Cooling `1`, heating `2`, auto `4` | Disabled by default | Write carefully |
| `2101` | Zone control | All off `0`, zone 1 `1`, zone 2 `2`, all on `3` | Disabled by default | Write carefully |
| `2102` | DHW mode | Off `0`, on `1` | Disabled by default | Write carefully |
| `2103` | Function A raw | raw bitfield | Disabled by default | Includes silent/electric heater/disinfection bits in community examples |
| `2105` | DHW setpoint | 30-60 °C, x10 write scaling | Disabled by default | Write carefully |
| `2107` | Zone 1 heating setpoint | 18-50 °C, x10 write scaling | Disabled by default | JÅN/weather compensation may overwrite it |
| `2114` | Room temperature setpoint | 10-30 °C, x10 write scaling | Disabled by default | Behavior depends on thermostat configuration |
| `6035` | Observed holding 6035 raw | raw | Diagnostic read-only in this project | Seen on one tested bus, meaning not confirmed |

## Active Sidecar Register Coverage

`adlar_aurora3_xiao_esp32c6_active_sidecar.yaml` currently reads every input and holding register listed above, one register per request. It also derives:

- heat pump state text from system status bits;
- mode text from holding register `2100`;
- zone-control text from holding register `2101`;
- E/P fault summaries from registers `90`, `91`, `96`, and `97`;
- water temperature delta, estimated thermal power, estimated electrical power, and estimated COP.

Derived power and COP entities are estimates. They should be treated as useful trend indicators until electrical power and flow scaling are confirmed on more installations.

## Scaling Notes

The Aurora III profile treats temperature registers as tenths of a degree. For example, raw `215` is exposed as `21.5 °C`. Writable temperature controls multiply the Home Assistant value by `10` before writing the register.

## Observed Live Bus Capture

These values were captured from one JÅN/Aurora III installation with the passive Arduino analyzer. They confirm a live 9600 8N2 Modbus RTU bus on slave `1`. The meanings are still provisional unless they match the register table above and stay plausible across more installations.

| Function | Address | Captured raw value | Interpreted value | Notes |
| ---: | ---: | ---: | --- | --- |
| `4` | `43` | `222` | `22.2 °C` | Matches outlet/supply water temperature scaling |
| `4` | `50` | `228` | `22.8 °C` | Matches ambient temperature scaling |
| `4` | `63` | `5` | `5 %` | Pump/PWM-style feedback candidate |
| `4` | `64` | `0` | `0.0 m³/h` | Water-flow register candidate |
| `4` | `66` | `180` | `18.0 °C` | Cooling curve/set value candidate |
| `4` | `67` | `180` | `18.0 °C` | Heating curve/set value candidate |
| `4` | `73` | `0` | raw `0` | Observed, meaning not confirmed |
| `4` | `74` | `226` | `226 V` | AC voltage candidate |
| `4` | `79` | `0` | `0 Hz` | Compressor actual frequency candidate |
| `4` | `83` | `0` | raw `0` | Observed, meaning not confirmed |
| `4` | `90` | `0` | raw `0` | Fault bitfield candidate |
| `4` | `92` | `0` | raw `0` | Observed, meaning not confirmed |
| `4` | `97` | `0` | raw `0` | Protection/fault bitfield candidate |
| `3` | `2100` | `2` | raw `2` | Current/target HVAC mode candidate |
| `3` | `2107` | `300` | `30.0 °C` | Zone 1 heating setpoint/current JÅN value candidate |
| `3` | `6035` | `1` | raw `1` | Observed holding register outside the public control set |

## Hardware Feedback Wanted

If you test this on real hardware, please open a hardware test report with:

- Heat pump model and firmware if known.
- JÅN module version if known.
- Whether slave ID `1` or `251` works.
- Which registers read correctly.
- Which write controls were tested.
- Any Modbus timeout, CRC, or collision symptoms.
