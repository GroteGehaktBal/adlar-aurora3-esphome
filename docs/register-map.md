# Register Map

This project uses zero-based Modbus register addresses, matching ESPHome and Home Assistant Modbus examples. If a vendor document uses `30001` or `40001` style notation, map carefully before copying values.

## Communication Defaults

| Setting | Value |
| --- | --- |
| Modbus role | ESPHome client/master |
| Slave ID | `1` by default, try `251` if needed |
| Baud rate | `9600` |
| Data bits | `8` |
| Parity | `NONE` |
| Stop bits | `2` |
| Input registers | `register_type: read` / function code 4 |
| Holding registers | `register_type: holding` / function code 3 |
| Poll interval | `60s` |
| Command throttle | `750ms` |
| Send wait time | `500ms` |

## Main Input Registers

| Address | Entity | Type | Scale/unit | Notes |
| ---: | --- | --- | --- | --- |
| `9` | Effective mode raw | `U_WORD` | raw | Bitfield: cooling/heating/DHW effective |
| `38` | System status bits | `U_WORD` | raw | Source for oil return, defrost, anti-freezing and status text |
| `40` | Room temperature | `S_WORD` | x0.1 °C | Room sensor value, if populated |
| `42` | Inlet water temperature | `S_WORD` | x0.1 °C | Water inlet/return |
| `43` | Outlet water temperature | `S_WORD` | x0.1 °C | Water outlet/supply |
| `46` | DHW tank temperature | `S_WORD` | x0.1 °C | Disabled by default |
| `50` | Ambient temperature | `S_WORD` | x0.1 °C | Outdoor ambient |
| `52` | Discharge temperature | `S_WORD` | x0.1 °C | Compressor discharge |
| `53` | Suction temperature | `S_WORD` | x0.1 °C | Disabled by default |
| `56` | Target outlet temperature | `S_WORD` | x0.1 °C | Target water outlet |
| `61` | Outlet water pressure | `U_WORD` | x0.1 bar | Hydraulic pressure |
| `62` | Pump PWM output | `U_WORD` | % | Pump command |
| `64` | Water flow | `U_WORD` | x0.1 m³/h | Used for thermal power estimate |
| `67` | Climate curve heating set | `S_WORD` | x0.1 °C | What the internal/JÅN curve calculates |
| `70` | EEV opening | `U_WORD` | steps | Disabled by default |
| `72` | Fan speed | `U_WORD` | rpm | Outdoor fan |
| `74` | AC voltage | `U_WORD` | V | Disabled by default; may be zero on some firmwares |
| `75` | AC current | `U_WORD` | x0.1 A | Disabled by default; may be zero on some firmwares |
| `76` | DC bus voltage | `U_WORD` | V | Disabled by default |
| `77` | Compressor current | `U_WORD` | x0.1 A | Compressor/inverter current |
| `78` | Compressor target frequency | `U_WORD` | Hz | Target frequency |
| `79` | Compressor actual frequency | `U_WORD` | Hz | Running frequency |
| `80` | Main water pump status raw | `U_WORD` | raw | `1` means on |
| `81` | Four-way valve status raw | `U_WORD` | raw | Disabled by default |
| `86` | High pressure | `U_WORD` | kPa | Disabled by default |
| `87` | Low pressure | `U_WORD` | kPa | Disabled by default |
| `90` | Error E01-E16 raw | `U_WORD` | raw | Fault bitfield |
| `91` | Error E17-E32 raw | `U_WORD` | raw | Fault bitfield |
| `96` | Error P01-P16 raw | `U_WORD` | raw | Protection bitfield |
| `97` | Error P17-P32 raw | `U_WORD` | raw | Protection bitfield |
| `101` | Target operation mode raw | `U_WORD` | raw | Disabled by default |
| `103` | Actual operation mode raw | `U_WORD` | raw | Disabled by default |

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

## Scaling Notes

The Aurora III profile treats temperature registers as tenths of a degree. For example, raw `215` is exposed as `21.5 °C`. Writable temperature controls multiply the Home Assistant value by `10` before writing the register.

## Hardware Feedback Wanted

If you test this on real hardware, please open a hardware test report with:

- Heat pump model and firmware if known.
- JÅN module version if known.
- Whether slave ID `1` or `251` works.
- Which registers read correctly.
- Which write controls were tested.
- Any Modbus timeout, CRC, or collision symptoms.
