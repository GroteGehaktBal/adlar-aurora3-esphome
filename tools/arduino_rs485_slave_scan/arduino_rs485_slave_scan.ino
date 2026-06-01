/*
  Slow Modbus RTU slave scanner for an Adlar/JAN RS485 bus.

  Target: classic 5V Arduino Uno/Nano with a 5V MAX485-style module.

  Wiring:
    Arduino 5V  -> RS485 module VCC
    Arduino GND -> RS485 module GND/DNG
    Arduino D10 -> RS485 module TXD
    Arduino D11 -> RS485 module RXD

  If a full scan gets zero replies, try swapping only the two TTL data wires:
    Arduino D10 -> RS485 module RXD
    Arduino D11 -> RS485 module TXD

  RS485 side:
    RS485 module A+  -> JAN A
    RS485 module B-  -> JAN B
    RS485 module GND -> JAN GND

  Serial Monitor:
    115200 baud

  This scanner is read-only. It tries a small set of plausible slave IDs
  and functions at 9600 baud. It approximates 8N2 by adding one idle bit
  time after each SoftwareSerial byte.
*/

#include <SoftwareSerial.h>

constexpr uint8_t RS485_RX_PIN = 10;
constexpr uint8_t RS485_TX_PIN = 11;
constexpr uint32_t RS485_BAUD = 9600;
constexpr uint16_t REPLY_TIMEOUT_MS = 800;
constexpr uint16_t BETWEEN_REQUESTS_MS = 1200;

SoftwareSerial rs485(RS485_RX_PIN, RS485_TX_PIN);

const uint8_t slave_ids[] = {
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 247, 251
};

struct RegisterProbe {
  uint8_t function;
  uint16_t address;
  const char *label;
};

const RegisterProbe register_probes[] = {
  {0x04, 0x0000, "input 0"},
  {0x04, 0x0001, "input 1"},
  {0x04, 0x0026, "input 38"},
  {0x04, 0x0028, "input 40"},
  {0x03, 0x0000, "holding 0"},
  {0x03, 0x0026, "holding 38"},
  {0x03, 0x0834, "holding 2100"},
};

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

void build_read_request(uint8_t slave, const RegisterProbe &probe, uint8_t *frame) {
  frame[0] = slave;
  frame[1] = probe.function;
  frame[2] = highByte(probe.address);
  frame[3] = lowByte(probe.address);
  frame[4] = 0x00;
  frame[5] = 0x01;

  const uint16_t crc = modbus_crc16(frame, 6);
  frame[6] = lowByte(crc);
  frame[7] = highByte(crc);
}

void write_frame_with_extra_stop_bit(const uint8_t *frame, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    rs485.write(frame[i]);
    rs485.flush();
    delayMicroseconds(140);
  }
}

void drain_rs485() {
  while (rs485.available() > 0) {
    rs485.read();
  }
}

uint8_t read_reply(uint8_t *buffer, uint8_t max_length) {
  uint8_t length = 0;
  const uint32_t start_ms = millis();
  uint32_t last_byte_ms = start_ms;

  while (millis() - start_ms < REPLY_TIMEOUT_MS) {
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

bool looks_like_modbus_reply(uint8_t slave, const uint8_t *reply, uint8_t length) {
  if (length < 5) {
    return false;
  }

  if (reply[0] != slave) {
    return false;
  }

  const uint16_t expected_crc = modbus_crc16(reply, length - 2);
  const uint16_t actual_crc = static_cast<uint16_t>(reply[length - 2]) |
                              (static_cast<uint16_t>(reply[length - 1]) << 8);

  return expected_crc == actual_crc;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  pinMode(RS485_TX_PIN, OUTPUT);
  digitalWrite(RS485_TX_PIN, HIGH);
  rs485.begin(RS485_BAUD);

  Serial.println();
  Serial.println(F("Adlar/JAN RS485 slow Modbus slave scan"));
  Serial.println(F("Serial monitor: 115200 baud"));
  Serial.println(F("RS485: 9600 baud, approx 8N2 TX, 8N1 RX"));
  Serial.println(F("Read-only scan. This will repeat until reset."));
  print_rx_idle_state();
}

void loop() {
  uint8_t request[8];
  uint8_t reply[64];
  uint16_t replies = 0;

  for (uint8_t s = 0; s < sizeof(slave_ids) / sizeof(slave_ids[0]); s++) {
    const uint8_t slave = slave_ids[s];

    for (uint8_t p = 0; p < sizeof(register_probes) / sizeof(register_probes[0]); p++) {
      const RegisterProbe &probe = register_probes[p];
      build_read_request(slave, probe, request);

      Serial.print(F("TX slave "));
      Serial.print(slave);
      Serial.print(F(" "));
      Serial.print(probe.label);
      Serial.print(F(" FC"));
      Serial.print(probe.function, HEX);
      Serial.print(F(": "));
      print_hex_buffer(request, sizeof(request));
      Serial.println();

      drain_rs485();
      write_frame_with_extra_stop_bit(request, sizeof(request));

      const uint8_t reply_length = read_reply(reply, sizeof(reply));
      if (reply_length > 0) {
        Serial.print(F("RX: "));
        print_hex_buffer(reply, reply_length);

        if (looks_like_modbus_reply(slave, reply, reply_length)) {
          Serial.print(F("  <-- valid Modbus CRC"));
          replies++;
        } else if (all_bytes_are_zero(reply, reply_length)) {
          Serial.print(F("  <-- all-zero bytes; RX likely stuck LOW or TTL orientation is wrong"));
          warned_all_zero_reply = true;
        } else {
          Serial.print(F("  <-- bytes seen, CRC/slave not valid"));
        }

        Serial.println();
      }

      delay(BETWEEN_REQUESTS_MS);
    }
  }

  Serial.println();
  Serial.print(F("Scan pass complete. Valid replies in this pass: "));
  Serial.println(replies);
  if (warned_all_zero_reply) {
    Serial.println(F("All-zero RX bytes are not valid Modbus traffic. Return to the TTL orientation where D10/RX idles HIGH."));
  }
  print_rx_idle_state();
  Serial.println(F("Repeating in 10 seconds..."));
  Serial.println();
  delay(10000);
}
