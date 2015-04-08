#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "Arduino.h"
#include "assert.h"
#include "tensegrity_wireless.h"

#define CE_PIN 9
#define CSN_PIN 10
RF24 radio(CE_PIN, CSN_PIN);

void send_message(void);
uint8_t calculate_checksum(Message *);
uint8_t get_checksum(Message *);

uint64_t addresses[6] = {0xF0F0F000, 0xF0F0F011, 0xF0F0F022,
                         0xF0F0F033, 0xF0F0F044, 0xF0F0F055};

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
  radio.setRetries(15, 15);
  radio.setPALevel(RF24_PA_HIGH);
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
    radio.openWritingPipe(addresses[my_id]);
  }
  radio.startListening();
}

bool radio_has_data() {
  return radio.available();
}
//////////////////////////// Message sending ////////////////////////////
void send_echo(uint8_t c_id, uint32_t verification_number) {
  Message *m = (Message *) tx_buffer;
  m->message_id = ECHO; m->controller_id = c_id;
  m->payload.verification_number = verification_number;
  Serial.println("Now sending echo.");
  send_message();
}

/* Sends motor command to the specified slave controller. The 
 * motor_command sent has values m1, m2, m3, m4. See the spec for 
 * more details.
 */
void send_motor_command(uint8_t c_id, int32_t m1, int32_t m2,
                        int32_t m3, int32_t m4) {
  Message *m = (Message *) tx_buffer;
  m->message_id = MOTOR_COMMAND;
  m->controller_id = c_id;
  m->payload.motor_command.m1 = m1;
  m->payload.motor_command.m2 = m2;
  m->payload.motor_command.m3 = m3;
  m->payload.motor_command.m4 = m4;
  Serial.println("Now sending motor command.");
  send_message();
}

/* Sends encoder_reading from the current controller to the master.
 * The encoder_reading sent has readings e1, e2, e3, e4. See the spec
 * for more details.
 */
void send_encoder_reading(int32_t e1, int32_t e2, 
                          int32_t e3, int32_t e4) {
  Message *m = (Message *) tx_buffer;
  m->message_id = ENCODER_READING;
  m->controller_id = id;
  m->payload.encoder_reading.e1 = e1;
  m->payload.encoder_reading.e2 = e2;
  m->payload.encoder_reading.e3 = e3;
  m->payload.encoder_reading.e4 = e4;
  Serial.println("Now sending encoder reading.");
  send_message();
}

void send_endcap_sensor_reading(bool e1, bool e2) {
  Message *m = (Message *) tx_buffer;
  m->message_id = ENDCAP_SENSOR_READING;
  m->controller_id = id;
  m->payload.endcap_sensor_reading.e1 = (uint8_t) (e1 == true);
  m->payload.endcap_sensor_reading.e2 = (uint8_t) (e2 == true);
  Serial.println("Now sending endcap sensor reading.");
  send_message();
}

///////////////////////////// Message receiving /////////////////////////////
Message * receive_message() {
  radio.read(rx_buffer, MESSAGE_LENGTH);
  Message *m = (Message *) rx_buffer;
  if (calculate_checksum(m) != (get_checksum(m))) {
    Serial.println("WARNING: Checksum differs from expected value.");
    m->message_id = ERROR;
  } 
  return m;
}

////////////////////////////// Message sending //////////////////////////////
/* Sends the current contents of tx_buffer over the radio as a message of 
 * type m_id. Note that this function should not be called outside of this 
 * file.
 */
void send_message() {
  tx_buffer[31] = calculate_checksum((Message *) tx_buffer);
  radio.stopListening();
  radio.write(tx_buffer, MESSAGE_LENGTH);
  radio.startListening();
}

/* Calculates the checksum of the given message. The checksum is calculated
 * by simply XORing the first 31 bytes of the message together.
 */
uint8_t calculate_checksum(Message *m) {
  uint8_t *p = (uint8_t *) m;
  uint8_t checksum = *p;
  for (int i = 1; i <= 30; i++) {
    checksum ^= *(p+i);
  }
  return checksum;
}

/* Retrieves the checksum of the given message. The checksum is always stored
 * as the 32nd byte of a message.
 */
uint8_t get_checksum(Message *m) {
  uint8_t *p = (uint8_t *) m;
  return *(p+31);
}
