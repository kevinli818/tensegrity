#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <tensegrity_wireless.h>

#define ECHO_FREQUENCY 10000
#define ECHO_DEADLINE 1000
#define ID 0

//system clock time to send next keepalive message.
uint64_t next_echo = 0;
//system clock time to verify that echos have been responded to.
uint64_t echo_deadline = 0xFFFFFFFF;
bool echo_verified = true;

void setup() {
  Serial.begin(57600); //initialize serial connection for debugging purposes.
  radio_init(ID);
  next_echo = millis() + 500;
}

void loop() {
  //send echoes and set response deadline.
  if (millis() >= next_echo) {
    send_echoes();
    echo_deadline = millis() + ECHO_DEADLINE;
    echo_verified = false;
    next_echo = millis() + ECHO_FREQUENCY;
  }
  //checks to see if every controller has responded
  //to echo by the keepalive deadline.
  if (millis() >= echo_deadline && !echo_verified) {
    check_connections();
    echo_verified = true;
  }
  if (radio_has_data()) {
    uint8_t m_id;
    uint8_t c_id;
    m_id = radio_read_byte();
    while (!radio_has_data()) {;}
    c_id = radio_read_byte();
    switch (m_id) {
      case ECHO:
        handle_echo(c_id);
        break;
    }
  }
}
