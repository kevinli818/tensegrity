#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "Arduino.h"
#include "assert.h"
#include "tensegrity_wireless.h"

#define CE_PIN 9
#define CSN_PIN 10
RF24 radio(CE_PIN, CSN_PIN);

void send_message(uint8_t);

uint64_t addresses[6] = {0xF0F0F0F000, 0xF0F0F0F011, 0xF0F0F0F022,
                         0xF0F0F0F033, 0xF0F0F0F044, 0xF0F0F0F055};

static uint8_t id;
//number generated by the system clock used to verify an EchoResponse

uint8_t tx_buffer[MESSAGE_LENGTH] = {0};
uint8_t rx_buffer[MESSAGE_LENGTH] = {0};

//////////////////////////////// Radio Setup ////////////////////////////////

/* Initializes the controllers radio using my_id as the id permanently 
 * used by this controller
 */
void radio_init(uint8_t my_id) {
  id = my_id;
  radio.begin();
  if (id == 0) { //radio belongs to the master controller
    //open broadcast pipe and individual reading pipes
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
    radio.openReadingPipe(2, addresses[2]);
    radio.openReadingPipe(3, addresses[3]);
    radio.openReadingPipe(4, addresses[4]);
    radio.openReadingPipe(5, addresses[5]);
  }
  else {
    radio.openReadingPipe(1, addresses[0]);
    radio.openWritingPipe(addresses[id]);
  }
  radio.startListening();
}

bool radio_has_data() {
  return radio.available();
}
//////////////////////////// Message sending ////////////////////////////
void send_echo(uint8_t c_id, uint32_t verification_number) {
  Message *m = (Message *) tx_buffer;
  m->message_id = ECHO;
  m->controller_id = c_id;
  m->payload.verification_number = verification_number;
  Serial.println("Now sending echo.");
  send_message(ECHO);
}

/* Sends motor command to the specified slave controller. The 
 * motor_command sent has values m1, m2, m3, m4. See the spec for 
 * more details.
 */
void send_motor_command(uint8_t c_id, uint32_t m1, uint32_t m2,
                        uint32_t m3, uint32_t m4) {
  Message *m = (Message *) tx_buffer;
  m->message_id = MOTOR_COMMAND;
  m->controller_id = c_id;
  m->payload.motor_command.m1 = m1;
  m->payload.motor_command.m2 = m2;
  m->payload.motor_command.m3 = m3;
  m->payload.motor_command.m4 = m4;
  Serial.println("Now sending motor command.");
  send_message(MOTOR_COMMAND);
}

/* Sends encoder_reading from the current controller to the master.
 * The encoder_reading sent has readings e1, e2, e3, e4. See the spec
 * for more details.
 */
void send_encoder_reading(uint32_t e1, uint32_t e2, 
                          uint32_t e3, uint32_t e4) {
  Message *m = (Message *) tx_buffer;
  m->message_id = ENCODER_READING;
  m->controller_id = id;
  m->payload.encoder_reading.e1 = e1;
  m->payload.encoder_reading.e2 = e2;
  m->payload.encoder_reading.e3 = e3;
  m->payload.encoder_reading.e4 = e4;
  Serial.println("Now sending encoder reading.");
  send_message(ENCODER_READING);
}

///////////////////////////// Message receiving /////////////////////////////
Message * receive_message() {
  radio.read(rx_buffer, MESSAGE_LENGTH);
  return (Message *) rx_buffer;
}

////////////////////////////// Message sending //////////////////////////////
/* Sends the current contents of tx_buffer over the radio as a message of 
 * type m_id. Note that this function should not be called outside of this 
 * file.
 */
void send_message(uint8_t m_id) {
  radio.stopListening();
  switch(m_id) {
    case ECHO:
      radio.write(tx_buffer, MESSAGE_LENGTH);
      break;
    case MOTOR_COMMAND:
      radio.write(tx_buffer, MESSAGE_LENGTH);
      break;
    case ENCODER_READING:
      radio.write(tx_buffer, MESSAGE_LENGTH);
      break;
  }
  radio.startListening();
}
