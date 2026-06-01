# Testing A 5V RS485 Module Safely

Some low-cost automatic-direction RS485 boards are built around MAX485-style 5V parts, even when they appear to work from 3.3V on the TTL side. If the ESP UART loopback works but the JÅN bus still gives no data, it is reasonable to test whether the board needs 5V on `VCC`.

Do not connect a 5V-powered module `TXD` output directly to an ESP32-C6 `RX` pin. ESP32 GPIO pins are 3.3V logic and are not 5V tolerant.

## What The 5V On JÅN A/B Means

The JÅN `A/B` terminals are the RS485 differential bus side. Measuring around 5V there can be normal bus bias or idle voltage. It does not mean that ESP `TXD/RXD` logic should be 5V.

The two sides are different:

| Side | Signals | Safe voltage idea |
| --- | --- | --- |
| RS485 bus side | `A`, `B`, `GND` | Can be 5V-biased and still be valid RS485 |
| ESP TTL side | `VCC`, `RXD`, `TXD`, `GND` | Must be safe for ESP GPIO, normally 3.3V |

## Option A: Test With A 5V Arduino

Use this only with a classic 5V Arduino such as an Uno, Nano, or Mega. Do not use a 3.3V Arduino for this 5V test unless you add level shifting.

Wiring for an Uno/Nano using the sketch in `tools/arduino_rs485_sniffer`:

| Arduino Uno/Nano | RS485 module TTL side |
| --- | --- |
| `5V` | `VCC` |
| `GND` | `GND/DNG` |
| `D10` | `TXD` |
| `D11` | `RXD` |

If probes show `TX` requests but always `RX: no reply`, try the alternative TTL orientation before changing any JÅN wiring:

| Arduino Uno/Nano | RS485 module TTL side |
| --- | --- |
| `D10` | `RXD` |
| `D11` | `TXD` |

This is necessary because different RS485 boards use `TXD/RXD` labels from different points of view.

The correct TTL orientation should leave Arduino `D10/RX` idle HIGH when no bytes are being received. If the serial monitor shows all-zero bytes such as `00:00:00:00`, that is not a Modbus reply. It usually means Arduino `D10/RX` is held LOW, the TTL data wires are in the wrong orientation, or the receiver output is noisy.

RS485 side:

| RS485 module | JÅN Modbus port |
| --- | --- |
| `A+` | `A` |
| `B-` | `B` |
| `E` or bus `GND` | JÅN `GND` |

Open the Arduino serial monitor at `115200` baud.

The sketch now does two things:

1. It passively prints any bytes it hears on the bus.
2. It actively sends slow Modbus read probes to slave `1` and `251`.

If the module receives JÅN bus traffic or gets a Modbus reply at 5V, the sketch prints raw hex bytes. If it prints `RX: no reply`, the Arduino sent the request but did not receive a response.

To prove the Arduino TTL wiring before connecting to JÅN:

1. Disconnect RS485 `A/B/GND` from JÅN.
2. Temporarily short the module TTL-side `TXD` and `RXD` pins.
3. Upload the sketch.
4. The serial monitor should show `RX note: exact echo. TTL TXD/RXD loopback is working.`

Remove the `TXD`/`RXD` short before connecting to JÅN again.

For a Mega, use hardware `Serial1` instead if preferred:

| Arduino Mega | RS485 module TTL side |
| --- | --- |
| `5V` | `VCC` |
| `GND` | `GND/DNG` |
| `RX1` / pin `19` | `TXD` |
| `TX1` / pin `18` | `RXD` |

## Option B: Test 5V Power With ESPHome And A Divider

If you want to keep using the ESPHome sniffer while powering the RS485 module from 5V, protect ESP `RX`.

Minimum safe wiring:

| Connection | Note |
| --- | --- |
| Module `VCC` -> `5V` | From a dev board or Arduino 5V pin |
| Module `GND/DNG` -> ESP `GND` | Grounds must be common |
| ESP `D6/TX` -> module `RXD` | Usually safe to try direct; 3.3V may be high enough |
| Module `TXD` -> voltage divider -> ESP `D7/RX` | Do not connect directly until measured safe |

Example resistor divider:

```text
module TXD --- 10k ---+--- ESP D7/RX
                      |
                     20k
                      |
                     GND
```

This divides a 5V module output to about 3.3V at ESP `RX`.

If only a single series resistor is available, do not treat it as a proper level shifter. A single series resistor may limit current, but it does not make a 5V logic signal a 3.3V logic signal.

## Expected Results

- Arduino or ESPHome sees raw bytes only when the module is powered from 5V: the current RS485 board likely needs 5V for the bus side.
- No bytes even at 5V in both A/B orientations: the issue is likely not just transceiver supply voltage.
- ESPHome TX works but JÅN never replies: the bus may not accept a second Modbus master, the slave ID/register request may still be wrong, or the module direction/RS485 side is not working correctly.

## Optional Slow Slave Scan

If the main Arduino sketch sends probes but always prints `RX: no reply`, you can try the slower scanner in `tools/arduino_rs485_slave_scan`.

It tries:

- slave IDs `1` through `10`, plus `247` and `251`
- input registers `0`, `1`, `38`, and `40`
- holding registers `0`, `38`, and `2100`

It is still read-only, but it sends many requests. Use it only while watching the heat pump/JÅN behavior, and stop the scan if anything becomes unstable.

If the scanner completes a pass with `Valid replies in this pass: 0` in both A/B orientations, the remaining likely causes are physical bus wiring/topology, the RS485 board not driving/receiving correctly on the differential side, or the JÅN terminal not being the reachable Modbus slave bus expected by the public community examples.

For a permanent ESP32 installation, prefer a true 3.3V RS485 transceiver such as an SP3485/MAX3485-based module, an isolated 3.3V RS485 module, or a proven external RS485 gateway.
