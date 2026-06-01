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

Observed working TTL orientation for the tested XY-485 board:

| Arduino Uno/Nano | RS485 module TTL side |
| --- | --- |
| `5V` | `VCC` |
| `GND` | `GND/DNG` |
| `D10` | `RXD` |
| `D11` | `TXD` |

Alternative orientation for boards labelled from the module's point of view:

| Arduino Uno/Nano | RS485 module TTL side |
| --- | --- |
| `D10` | `TXD` |
| `D11` | `RXD` |

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
| `RX1` / pin `19` | `RXD` |
| `TX1` / pin `18` | `TXD` |

## Option B: Test 5V Power With ESPHome And A Divider

If you want to keep using the ESPHome sniffer while powering the RS485 module from 5V, protect ESP `RX`.

Minimum safe wiring:

| Connection | Note |
| --- | --- |
| Module `VCC` -> `5V` | From a dev board or Arduino 5V pin |
| Module `GND/DNG` -> ESP `GND` | Grounds must be common |
| ESP `D6/TX` -> module `TXD` | Tested XY-485 orientation; try the alternate orientation only if needed |
| Module `RXD` -> voltage divider -> ESP `D7/RX` | Do not connect directly until measured safe |

Example resistor divider:

```text
module RXD --- 10k ---+--- ESP D7/RX
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

If the active scanner sees valid CRC frames that look like `01:04:00:48:00:01:...` or `01:03:17:EB:00:01:...`, those are probably existing Modbus requests on the bus, not replies to the Arduino probe. Switch to `tools/arduino_rs485_passive_analyzer` to decode the bus without transmitting.

Before interpreting a zero-reply scan, check the startup line:

- `Arduino D10/RX idle state: HIGH (expected)` means the TTL receive line is electrically plausible.
- `Arduino D10/RX idle state: LOW (bad)` means the receive line is held low; fix TTL wiring or module power before testing JÅN again.

If you short module `TXD` and `RXD` with JÅN disconnected, the scanner should print `exact echo`. That proves the Arduino can send and receive through the TTL wiring, but it is not a heat-pump reply.

For a permanent ESP32 installation, prefer a true 3.3V RS485 transceiver such as an SP3485/MAX3485-based module, an isolated 3.3V RS485 module, or a proven external RS485 gateway.
