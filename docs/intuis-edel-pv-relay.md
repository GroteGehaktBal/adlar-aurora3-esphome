# Intuis Edel PV Relay

This optional wiring adds a 3.3 V, high-level-trigger, one-channel relay to the
same Seeed Studio XIAO ESP32-C6 used by the active sidecar profile. It is
intended for an Intuis (formerly Auer) Edel EAU 200/270 v3, including reference
`352431`.

The relay controls a signal input only. It must never switch the water heater's
230 V supply.

## What One Relay Can Do

With the Intuis installer setting `MODE PV` enabled, dry-contact input 1
(`heures creuses`) becomes the `PV ECO` input. Closing it asks the water heater
to use the PV ECO temperature, which is adjustable between the normal hot-water
setpoint and 60 °C (60 °C factory setting).

`PV MAX`, adjustable up to 65 °C and able to use both the heat pump and electric
heater, is a separate dry-contact input 2. A one-channel SPDT relay cannot
independently control both inputs. Use a second relay channel if PV MAX control
is required; do not bridge inputs 1 and 2 together.

## XIAO To Relay Module

This configuration assumes the module really is rated for 3.3 V and its input
is active HIGH.

| XIAO ESP32-C6 | Relay module |
| --- | --- |
| `3V3` | `VCC` |
| `D1` / `GPIO1` | `IN` |
| `GND` | `GND` |

XIAO `D1` is `GPIO1` on the ESP32-C6 board. The existing RS485 connection keeps
using `D6/GPIO16` and `D7/GPIO17`, so there is no pin conflict.

For a defined off state during the brief interval before ESPHome initializes
the GPIO, add a 10 kOhm pull-down resistor between relay `IN` and `GND` unless
the module already has an input pull-down. Confirm that the combined RS485 and
relay-module current is within the XIAO supply capability and that the 3.3 V
rail remains stable while the relay is energized.

If a different module energizes when `IN` is LOW, do not use it with this YAML
unchanged. Add `inverted: true` to the switch pin configuration and verify the
power-up state on the bench before connecting the water heater.

## Relay Contacts To The Intuis

Work with both the water heater and the XIAO supply disconnected. Access to the
water heater's electrical enclosure and PCB should be handled by a qualified
installer.

1. Remove the factory red jumper from Intuis connector 1, labelled
   `heures creuses`.
2. Connect the two terminals of that connector to relay `COM` and relay `NO`.
   Polarity does not matter for a dry contact.
3. Leave relay `NC` unconnected and insulated.
4. Keep `VCC`, `IN`, and `GND` completely separate from the Intuis connector.
   Never put 230 V or another external voltage on dry-contact inputs 1 or 2.
5. Use a suitable enclosure, strain relief, ferrules, and the cable size and
   routing required by the Intuis installation manual.

```text
XIAO 3V3  ---------------- VCC   relay   COM ---------------- input 1 terminal A
XIAO D1 / GPIO1 ---------- IN             NO ---------------- input 1 terminal B
XIAO GND  ---------------- GND            NC ---------------- not connected
```

The ESPHome entity is `switch.intuis_pv_eco_boost` (the final Home Assistant
entity ID can vary). It uses `restore_mode: ALWAYS_OFF`, so a reboot opens
`COM-NO` and cancels the PV request. Turning the entity on energizes the relay
and closes `COM-NO`.

## Water-Heater Settings And Test

1. In the Intuis installer menu, enable `MODE PV`.
2. Set the normal hot-water target and `T° PV ECO` target. The latter cannot be
   higher than 60 °C.
3. Choose `PRIORITE` deliberately. With priority enabled, a PV signal can heat
   outside programmed time windows and while holiday or eco behavior would
   otherwise apply.
4. Before connecting the Intuis, power only the XIAO and relay. Confirm with a
   continuity meter that `COM-NO` is open at boot, closes when the Home
   Assistant switch is on, and opens when it is off.
5. After final connection, use the Intuis installer display (`AFFICHAGE` / input
   reading) to confirm `PV ECO` changes from `0` to `1` when the relay closes.

## Home Assistant Surplus Example

The following example assumes the P1 entity reports grid power in watts, with
negative values for export. Replace the entity IDs and thresholds to match the
actual meter. The two thresholds provide hysteresis, while the `for` periods
prevent short solar transients from toggling the request. The third automation
re-evaluates sustained export after Home Assistant starts or the ESPHome relay
returns from `unavailable`; this is necessary because a numeric-state trigger
only fires when its threshold is crossed.

```yaml
automation:
  - alias: "Intuis PV ECO - enable on sustained export"
    triggers:
      - trigger: numeric_state
        entity_id: sensor.p1_grid_power
        below: -700
        for: "00:10:00"
    actions:
      - action: switch.turn_on
        target:
          entity_id: switch.intuis_pv_eco_boost

  - alias: "Intuis PV ECO - disable when export ends"
    triggers:
      - trigger: numeric_state
        entity_id: sensor.p1_grid_power
        above: -150
        for: "00:05:00"
    actions:
      - action: switch.turn_off
        target:
          entity_id: switch.intuis_pv_eco_boost

  - alias: "Intuis PV ECO - recover after restart"
    mode: restart
    triggers:
      - trigger: homeassistant
        event: start
      - trigger: state
        entity_id: switch.intuis_pv_eco_boost
        from:
          - "unknown"
          - "unavailable"
        to: "off"
    actions:
      - wait_template: >-
          {{ has_value('sensor.p1_grid_power') and
             has_value('switch.intuis_pv_eco_boost') }}
        timeout: "00:05:00"
        continue_on_timeout: false
      - condition: numeric_state
        entity_id: sensor.p1_grid_power
        below: -700
      - wait_for_trigger:
          - trigger: template
            value_template: >-
              {{ not has_value('sensor.p1_grid_power') or
                 not has_value('switch.intuis_pv_eco_boost') or
                 states('sensor.p1_grid_power') | float >= -700 }}
        timeout: "00:10:00"
        continue_on_timeout: true
      - condition: template
        value_template: "{{ not wait.completed }}"
      - action: switch.turn_on
        target:
          entity_id: switch.intuis_pv_eco_boost
```

If the P1 sensor reports export as a positive value, reverse the comparisons.
The thresholds are examples, not equipment limits. Account for other household
loads and confirm that the Intuis internal controller remains responsible for
compressor timing, temperature limits, and all safety functions.

The relay deliberately remains `ALWAYS_OFF` in ESPHome. Restoring its previous
on-state in firmware would make a reboot automatically request heat without
first checking whether solar export still exists; the recovery automation above
performs that check instead.

## References

- [Intuis Edel EAU 200/270 v3 installer manual](https://medias.intuis.fr/storage/media/shares/Intuis/quable/mc_edel_eau_sol_notice_1898-376_352421-352431_intuis_Edel_200-270_EAU_v3_INST_24-06.pdf)
- [Seeed Studio XIAO ESP32-C6 pin map](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/)
- [ESPHome GPIO switch documentation](https://esphome.io/components/switch/gpio/)
