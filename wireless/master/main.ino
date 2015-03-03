#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);

byte addresses[][6] = {"Master", "Slave1", "Slave2", "Slave3", "Slave4", "Slave5"};

//id is 0 for the master controller. 1 through 5 for the slaves.
int id = 0;

typedef struct
{
  bool connected;                
  bool has_responded; //true if controller has responded to latest EchoRequest
  //randomly generated number used to verify the correctness of an EchoResponse
  uint32_t verification_number;
} controller;

//the perceived statuses of controllers 0 through 5. Note that status[0] is more 
//or less unused as there's no use for the master to track connectivity with itself.
controller controllers[6];

//system clock time to verify that echos have been responded to.
uint64_t keepalive_deadline = 0;

void setup() {
  Serial.begin(57600); //initialize serial connection for debugging purposes.

  //initialize controller array
  for (int i = 1; i <= 5; i++) {
    controllers[i].connected = false;
    controllers[i].has_responded = false;
    controllers[i].verification_number = 0;
  }
}
