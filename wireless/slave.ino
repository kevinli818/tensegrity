#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <tensegrity_wireless.h>

//ID is 0 for the master controller, 1 through 5 for slaves.
#define ID 1

//helper functions used in loop()

void setup() {
  Serial.begin(57600);
  radio_init(ID);
}

void loop() {
  if (radio_has_data()) {
    Message *m = receive_message();
    switch(m->message_id) {
      case ECHO:
        if (m->controller_id != 0) {
          Serial.println("Error. EchoRequests should only be coming in from master.");
        }
        send_echo(ID, m->payload.verification_number);
        break;
      case MOTOR_COMMAND: //TODO(vdonato): actuate motors based on MOTOR_COMMAND
        break;
    }
  }
}
