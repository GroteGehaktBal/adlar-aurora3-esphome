/*
  RS485 test tool for an Adlar/JAN module and a 5V MAX485-style board.

  Target: classic 5V Arduino Uno/Nano.

  Wiring:
    Arduino 5V  -> RS485 module VCC
    Arduino GND -> RS485 module GND/DNG
    Arduino D10 -> RS485 module TXD
    Arduino D11 -> RS485 module RXD

  If every probe says "no reply", try swapping only the two TTL data wires:
    Arduino D10 -> RS485 module RXD
    Arduino D11 -> RS485 module TXD

  RS485 side:
    RS485 module A+  -> JAN A
    RS485 module B-  -> JAN B
    RS485 module GND -> JAN GND

  Serial Monitor:
    115200 baud

  What it does:
    1. Prints any passive bytes it sees on the RS485 bus.
    2. Every few seconds sends a minimal Modbus RTU read request to
       slave 1 and slave 251, using the public Aurora III 9600 8N2 profile.
    3. Prints either the reply bytes or "no reply".

  Quick TTL loopback check:
    - Disconnect RS485 A/B/GND from JAN.
    - Temporarily short the module TTL-side TXD and RXD pins.
    - The sketch should print the same bytes it sent as an RX echo.

  Note:
    SoftwareSerial is 8N1, so this sketch inserts an extra idle bit time
    after every transmitted byte to approximate 8N2 for the Modbus request.
    Receiving 8N2 as 8N1 is fine because the second stop bit is idle time.
*/

#include <SoftwareSerial.h>

constexpr uint8_t RS485_RX_PIN = 10;
constexpr uint8_t RS485_TX_PIN = 11;
constexpr uint32_t RS485_BAUD = 9600;
constexpr uint16_t REPLY_TIMEOUT_MS = 700;
constexpr uint16_t PASSIVE_WINDOW_MS = 2500;

SoftwareSerial rs485(RS485_RX_PIN, RS485_TX_PIN);

struct Probe {
  uint8_t slave;
  uint8_t function;
  uint16_t address;
  uint16_t count;
  const char *label;
};

Probe probes[] = {
  {1, 0x04, 0x0026, 1, "slave 1 input register 38"},
  {251, 0x04, 0x0026, 1, "slave 251 input register 38"},
  {1, 0x03, 0x0026, 1, "slave 1 holding register 38"},
  {251, 0x03, 0x0026, 1, "slave 251 holding register 38"},
};

uint8_t next_probe = 0;
uint32_t last_probe_ms = 0;
uint32_t last_passive_byte_ms = 0;
bool passive_line_open = false;
bool warned_all_zero_reply = false;
bool warned_rx_idle_low = false;

void print_hex_byte(uint8_t value) {
  if (value < 0x10) {
    Serial.print('0');
  }
  Serial.print(value, HEX);
}

void print_hex_buffer(const uint8_t *data, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    if (i > 0) {
      Serial.print(':');
    }
    print_hex_byte(data[i]);
  }
}

bool all_bytes_are_zero(const uint8_t *data, uint8_t length) {
  if (length == 0) {
    return false;
  }

  for (uint8_t i = 0; i < length; i++) {
    if (data[i] != 0x00) {
      return false;
    }
  }

  return true;
}

void print_rx_idle_state() {
  const bool rx_high = digitalRead(RS485_RX_PIN) == HIGH;

  Serial.print(F("Arduino D10/RX idle state: "));
  Serial.println(rx_high ? F("HIGH (expected)") : F("LOW (bad)"));

  if (!rx_high && !warned_rx_idle_low) {
    Serial.println(F("Hint: RX idle LOW usually means the TTL wiring is wrong, the receiver output is held low, or the line is noisy."));
    warned_rx_idle_low = true;
  }
}

uint16_t modbus_crc16(const uint8_t *data, uint8_t length) {
  uint16_t crc = 0xFFFF;

  for (uint8_t pos = 0; pos < length; pos++) {
    crc ^= static_cast<uint16_t>(data[pos]);

    for (uint8_t i = 0; i < 8; i++) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }

  return crc;
}

void build_modbus_read_request(const Probe &probe, uint8_t *frame) {
  frame[0] = probe.slave;
  frame[1] = probe.function;
  frame[2] = highByte(probe.address);
  frame[3] = lowByte(probe.address);
  frame[4] = highByte(probe.count);
  frame[5] = lowByte(probe.count);

  const uint16_t crc = modbus_crc16(frame, 6);
  frame[6] = lowByte(crc);
  frame[7] = highByte(crc);
}

void write_frame_with_extra_stop_bit(const uint8_t *frame, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    rs485.write(frame[i]);
    rs485.flush();
    delayMicroseconds(140);  // One extra 9600-baud stop-bit plus margin.
  }
}

void drain_rs485() {
  while (rs485.available() > 0) {
    rs485.read();
  }
}

uint8_t read_reply(uint8_t *buffer, uint8_t max_length, uint16_t timeout_ms) {
  uint8_t length = 0;
  const uint32_t start_ms = millis();
  uint32_t last_byte_ms = start_ms;

  while (millis() - start_ms < timeout_ms) {
    while (rs485.available() > 0) {
      if (length < max_length) {
        buffer[length++] = static_cast<uint8_t>(rs485.read());
      } else {
        rs485.read();
      }
      last_byte_ms = millis();
    }

    if (length > 0 && millis() - last_byte_ms > 30) {
      break;
    }
  }

  return length;
}

void close_passive_line_if_needed() {
  if (passive_line_open && millis() - last_passive_byte_ms > 30) {
    Serial.println();
    passive_line_open = false;
  }
}

void read_passive_bytes() {
  while (rs485.available() > 0) {
    const uint8_t value = static_cast<uint8_t>(rs485.read());

    if (!passive_line_open || millis() - last_passive_byte_ms > 30) {
      if (passive_line_open) {
        Serial.println();
      }
      Serial.print(millis());
      Serial.print(F(" ms passive RX: "));
      passive_line_open = true;
    } else {
      Serial.print(':');
    }

    print_hex_byte(value);
    last_passive_byte_ms = millis();
  }

  close_passive_line_if_needed();
}

void run_probe(const Probe &probe) {
  close_passive_line_if_needed();

  uint8_t request[8];
  uint8_t reply[64];
  build_modbus_read_request(probe, request);

  Serial.println();
  Serial.print(F("TX probe "));
  Serial.print(probe.label);
  Serial.print(F(": "));
  print_hex_buffer(request, sizeof(request));
  Serial.println();

  drain_rs485();
  write_frame_with_extra_stop_bit(request, sizeof(request));

  const uint8_t reply_length = read_reply(reply, sizeof(reply), REPLY_TIMEOUT_MS);

  if (reply_length == 0) {
    Serial.println(F("RX: no reply"));
  } else {
    Serial.print(F("RX: "));
    print_hex_buffer(reply, reply_length);
    Serial.println();

    if (all_bytes_are_zero(reply, reply_length) && !warned_all_zero_reply) {
      Serial.println(F("RX note: all-zero bytes are not a valid Modbus reply. RX is probably stuck LOW or the TTL TXD/RXD orientation is wrong."));
      warned_all_zero_reply = true;
    }

    if (reply_length == sizeof(request) && memcmp(request, reply, sizeof(request)) == 0) {
      Serial.println(F("RX note: exact echo. TTL TXD/RXD loopback is working."));
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // Needed on boards with native USB; harmless on Uno/Nano.
  }

  pinMode(RS485_TX_PIN, OUTPUT);
  digitalWrite(RS485_TX_PIN, HIGH);

  rs485.begin(RS485_BAUD);

  Serial.println();
  Serial.println(F("Adlar Aurora III RS485 Arduino test"));
  Serial.println(F("Arduino serial monitor: 115200 baud"));
  Serial.println(F("RS485 side: 9600 baud, approx 8N2 TX, 8N1 RX"));
  Serial.println(F("D10 <- module TXD, D11 -> module RXD"));
  Serial.println(F("First listening passively, then sending slow Modbus probes."));
  print_rx_idle_state();
}

void loop() {
  read_passive_bytes();

  if (millis() - last_probe_ms >= PASSIVE_WINDOW_MS) {
    last_probe_ms = millis();
    run_probe(probes[next_probe]);

    next_probe++;
    if (next_probe >= sizeof(probes) / sizeof(probes[0])) {
      next_probe = 0;
    }
  }
}
