/*
  Passive Modbus RTU frame analyzer for the Adlar/JAN RS485 bus.

  Target: classic 5V Arduino Uno/Nano with a 5V automatic-direction
  RS485-to-TTL board.

  For the XY-485 board tested in this project, the working TTL orientation is:
    Arduino 5V  -> RS485 module VCC
    Arduino GND -> RS485 module GND/DNG
    Arduino D10 -> RS485 module RXD
    Arduino D11 -> RS485 module TXD

  That means the module labels appear to be from the microcontroller side:
    host RXD -> module RXD
    host TXD -> module TXD

  RS485 side:
    RS485 module A+  -> JAN A
    RS485 module B-  -> JAN B
    RS485 module GND -> JAN GND

  Serial Monitor:
    115200 baud

  This sketch is passive. It does not send Modbus requests. It only groups
  received bytes into frames, checks CRC, and prints likely request/response
  details.
*/

#include <SoftwareSerial.h>

constexpr uint8_t RS485_RX_PIN = 10;
constexpr uint8_t RS485_TX_PIN = 11;
constexpr uint32_t RS485_BAUD = 9600;
constexpr uint8_t FRAME_MAX = 96;
constexpr uint16_t FRAME_GAP_MS = 6;

SoftwareSerial rs485(RS485_RX_PIN, RS485_TX_PIN);

uint8_t frame[FRAME_MAX];
uint8_t frame_length = 0;
uint32_t last_byte_ms = 0;

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

bool has_valid_crc(const uint8_t *data, uint8_t length) {
  if (length < 4) {
    return false;
  }

  const uint16_t expected_crc = modbus_crc16(data, length - 2);
  const uint16_t actual_crc = static_cast<uint16_t>(data[length - 2]) |
                              (static_cast<uint16_t>(data[length - 1]) << 8);
  return expected_crc == actual_crc;
}

uint16_t word_at(const uint8_t *data, uint8_t index) {
  return (static_cast<uint16_t>(data[index]) << 8) | data[index + 1];
}

void describe_frame(const uint8_t *data, uint8_t length) {
  const uint8_t slave = data[0];
  const uint8_t function = data[1];

  Serial.print(F(" slave="));
  Serial.print(slave);
  Serial.print(F(" fc="));
  Serial.print(function);

  if ((function == 0x03 || function == 0x04) && length == 8) {
    const uint16_t address = word_at(data, 2);
    const uint16_t count = word_at(data, 4);
    Serial.print(F(" request addr="));
    Serial.print(address);
    Serial.print(F(" count="));
    Serial.print(count);
    return;
  }

  if ((function == 0x03 || function == 0x04) && length >= 5) {
    const uint8_t byte_count = data[2];
    if (length == static_cast<uint8_t>(byte_count + 5)) {
      Serial.print(F(" response bytes="));
      Serial.print(byte_count);

      if (byte_count >= 2) {
        Serial.print(F(" first_value="));
        Serial.print(word_at(data, 3));
      }
      return;
    }
  }

  if ((function & 0x80) != 0 && length == 5) {
    Serial.print(F(" exception code="));
    Serial.print(data[2]);
    return;
  }

  Serial.print(F(" unknown_shape"));
}

void flush_frame() {
  if (frame_length == 0) {
    return;
  }

  Serial.print(millis());
  Serial.print(F(" ms frame "));
  print_hex_buffer(frame, frame_length);

  if (has_valid_crc(frame, frame_length)) {
    Serial.print(F("  CRC OK"));
    describe_frame(frame, frame_length);
  } else {
    Serial.print(F("  CRC BAD"));
  }

  Serial.println();
  frame_length = 0;
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
  Serial.println(F("Adlar/JAN RS485 passive Modbus analyzer"));
  Serial.println(F("Serial monitor: 115200 baud"));
  Serial.println(F("RS485 receive: 9600 baud"));
  Serial.println(F("Expected working TTL wiring: D10 -> module RXD, D11 -> module TXD"));
  Serial.print(F("Arduino D10/RX idle state: "));
  Serial.println(digitalRead(RS485_RX_PIN) == HIGH ? F("HIGH (expected)") : F("LOW (bad)"));
}

void loop() {
  while (rs485.available() > 0) {
    const uint8_t value = static_cast<uint8_t>(rs485.read());

    if (frame_length > 0 && millis() - last_byte_ms > FRAME_GAP_MS) {
      flush_frame();
    }

    if (frame_length < FRAME_MAX) {
      frame[frame_length++] = value;
    } else {
      flush_frame();
    }

    last_byte_ms = millis();
  }

  if (frame_length > 0 && millis() - last_byte_ms > FRAME_GAP_MS) {
    flush_frame();
  }
}
