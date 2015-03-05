#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <tensegrity_wireless.h>
#include "assert.h"

//ID is 0 for the master controller, 1 through 5 for slaves.
#define ID 1

//helper functions used in loop()
void send_echo_response(void);

void setup() {
  Serial.begin(57600);
  radio_init(ID);
}

void loop() {
  uint8_t m_id;
  uint8_t c_id;
  if (radio_has_data()) {
    m_id = radio_read_byte();
    while(!radio_has_data()) {;}   //spin until next part of message comes in
    c_id = radio_read_byte();
    assert(c_id == 0); //echo came from master controller
    switch(m_id) {
      case ECHO:
        send_echo_response();
        break;
    }
  }
}
