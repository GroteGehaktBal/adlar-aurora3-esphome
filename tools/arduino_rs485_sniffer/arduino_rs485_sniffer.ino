/*
  Passive RS485 byte sniffer for testing a 5V MAX485-style module.

  Target: classic 5V Arduino Uno/Nano.

  Wiring:
    Arduino 5V  -> RS485 module VCC
    Arduino GND -> RS485 module GND/DNG
    Arduino D10 -> RS485 module TXD
    Arduino D11 -> RS485 module RXD

  RS485 side:
    RS485 module A+  -> JÅN A
    RS485 module B-  -> JÅN B
    RS485 module GND -> JÅN GND

  Serial Monitor:
    115200 baud

  The RS485 bus profile used by public Aurora III examples is 9600 8N2.
  SoftwareSerial receives this well enough as 9600 8N1 because the second
  stop bit is just extra idle time between bytes.
*/

#include <SoftwareSerial.h>

constexpr uint8_t RS485_RX_PIN = 10;
constexpr uint8_t RS485_TX_PIN = 11;
constexpr uint32_t RS485_BAUD = 9600;

SoftwareSerial rs485(RS485_RX_PIN, RS485_TX_PIN);

uint32_t last_byte_ms = 0;
bool line_open = false;

void print_hex_byte(uint8_t value) {
  if (value < 0x10) {
    Serial.print('0');
  }
  Serial.print(value, HEX);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // Needed on boards with native USB; harmless on Uno/Nano.
  }

  pinMode(RS485_TX_PIN, OUTPUT);
  digitalWrite(RS485_TX_PIN, HIGH);  // Keep module RXD idle-high; do not transmit.

  rs485.begin(RS485_BAUD);

  Serial.println();
  Serial.println(F("Adlar Aurora III RS485 passive byte sniffer"));
  Serial.println(F("Arduino side: 115200 baud"));
  Serial.println(F("RS485 side: 9600 baud, passive receive"));
  Serial.println(F("Waiting for bytes..."));
}

void loop() {
  while (rs485.available() > 0) {
    uint8_t value = static_cast<uint8_t>(rs485.read());

    if (!line_open || millis() - last_byte_ms > 20) {
      if (line_open) {
        Serial.println();
      }
      Serial.print(millis());
      Serial.print(F(" ms RX: "));
      line_open = true;
    } else {
      Serial.print(':');
    }

    print_hex_byte(value);
    last_byte_ms = millis();
  }

  if (line_open && millis() - last_byte_ms > 20) {
    Serial.println();
    line_open = false;
  }
}
