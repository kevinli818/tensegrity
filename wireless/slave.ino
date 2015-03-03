#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <tensegrity_wireless.h>

#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);

//id is 0 for the master controller. 1 through 5 for the slaves.
int id = 1;

//helper functions used in loop()
void send_echo_response(void);

void setup() {
  Serial.begin(57600);
  radio.begin();
  radio.openReadingPipe(1, 0xF0F0F0F000);
  //the last two numbers of the pipe should correspond to the controller id
  radio.openWritingPipe(0xF0F0F0F011);
  radio.startListening();
}

void loop() {
  uint8_t m_id;
  uint8_t c_id;
  if (radio.available()) {
    radio.read(&m_id, sizeof(char));
    while(!radio.available()) {;}   //spin until next part of message comes in
    radio.read(&c_id, sizeof(char));
    switch(m_id) {
      case ECHO:
        send_echo_response();
        break;
    }
  }
}

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
  free(m);
}
