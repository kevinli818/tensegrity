#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "Arduino.h"
#include "assert.h"
#include "tensegrity_wireless.h"

#define CE_PIN 9
#define CSN_PIN 10
extern RF24 radio(CE_PIN, CSN_PIN);

uint64_t addresses[6] = {0xF0F0F0F000, 0xF0F0F0F011, 0xF0F0F0F022,
                         0xF0F0F0F033, 0xF0F0F0F044, 0xF0F0F0F055};
typedef struct
{
  bool has_responded; //true if controller has responded to latest EchoRequest
} controller;
//the perceived status of controllers 0 through 5. Note that status[0] is more or
//less unused as there's no need for the master to track connectivity with itself
controller controllers[6];
static uint8_t id;
//number generated by the system clock used to verify an EchoResponse
uint32_t verification_number;

//////////////////////////////// Radio Setup ////////////////////////////////

/* Initializes the controllers radio using my_id as the id permanently 
 * used by this controller
 */
void radio_init(uint8_t my_id) {
  id = my_id;
  //initialize controller array
  for (int i = 1; i <= 5; i++) {
    controllers[i].has_responded = false;
  }
  radio.begin();
  if (id == 0) { //radio belongs to the master controller
    //open broadcast pipe and individual reading pipes
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
    radio.openReadingPipe(2, addresses[1]);
    radio.openReadingPipe(3, addresses[1]);
    radio.openReadingPipe(4, addresses[1]);
    radio.openReadingPipe(5, addresses[1]);
  }
  else {
    radio.openReadingPipe(1, addresses[0]);
    radio.openWritingPipe(addresses[id]);
  }
  radio.startListening();
}

uint8_t radio_read_byte() {
  uint8_t x;
  radio.read(&x, sizeof(uint8_t));
  return x;
}

bool radio_has_data() {
  return radio.available();
}
//////////////////////////// Message marshalling ////////////////////////////

Message * marshall_echo(uint8_t controller_id, Echo *e) {
  Message *m = (Message*) malloc(BASE_MSG_LENGTH);
  m->message_id = ECHO;
  m->controller_id = controller_id;
  m->payload = malloc(ECHO_LENGTH);
  *(uint32_t*)m->payload = e->verification_number;
  return m;
}

//TODO(vdonato): implement marshall_motor_command and marshall_encoder_reading.
/* Marshall Motor_command mc to a message intended for controller c_id */
Message * marshall_motor_command(uint8_t c_id, Motor_command * mc) {
  return NULL;
}

/* Marshall Encoder_reading er to a message marked as sent from c_id */
Message * marshall_motor_command(uint8_t c_id, Encoder_reading * er) {
  return NULL;
}

/////////////////////////// Message unmarshalling ///////////////////////////

Echo * unmarshall_echo(void *payload) {
  uint32_t *p = (uint32_t*) payload;
  Echo *e = (Echo*) malloc(ECHO_LENGTH);
  e->verification_number = *p;
  return e;
}

//TODO(vdonato): implement both unmarshall functions below.
Motor_command * unmarshall_motor_command(void *payload) {
  return NULL;
}

Encoder_reading * unmarshall_encoder_reading(void *payload) {
  return NULL;
}

////////////////////////// Master Helper Functions //////////////////////////
void send_echoes() {
  Serial.println("Sending echo requests.");
  Echo *e = (Echo *) malloc(ECHO_LENGTH);
  verification_number = (uint32_t) millis();
  e->verification_number = verification_number;
  Message *m = marshall_echo(id, e);
  radio.stopListening();
  radio.write(&m->message_id, sizeof(uint8_t));
  radio.write(&m->controller_id, sizeof(uint8_t));
  radio.write(m->payload, ECHO_LENGTH);
  radio.startListening();
  free(e);
  free(m->payload);
  free(m);
}

/* verifies that each slave controller has responded
 * to the most recently sent echo request.
 */
void check_connections() {
  bool all_connected = true;
  for (int i = 1; i <= 5; i++) {
    if (!controllers[i].has_responded) {
      Serial.print("Controller ");
      Serial.print(i);
      Serial.println(" not connected.");
      all_connected = false;
    }
  }
  if (all_connected) {
    Serial.println("All controllers connected!");
  }
}

/* updates the status of the controller an EchoResponse was received from */
void handle_echo(uint8_t c_id) {
  void *payload = malloc(ECHO_LENGTH);
  while (!radio.available()) {;} //spin and wait for next part of message to come in.
  radio.read(payload, ECHO_LENGTH);
  Echo *e = unmarshall_echo(payload);
  if (e->verification_number == verification_number) {
    Serial.print("Received echo response from controller ");
    Serial.print(c_id);
    Serial.println(" .");
    controllers[c_id].has_responded = true;
  }
  free(payload);
  free(e);
}

/////////////////////////// Slave Helper Functions //////////////////////////
void send_echo_response() {
  void *payload = malloc(ECHO_LENGTH);
  while(!radio.available()) {;}
  radio.read(payload, ECHO_LENGTH);
  Echo *e = unmarshall_echo(payload);
  Message *m = marshall_echo(id, e);
  radio.stopListening();
  radio.write(&m->message_id, sizeof(uint8_t));
  radio.write(&m->controller_id, sizeof(uint8_t));
  radio.write(m->payload, ECHO_LENGTH);
  radio.startListening();
  free(e);
  free(m->payload);
  free(m);
}
