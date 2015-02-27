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
  uint32_t verification_number;
} controller_status

//the perceived statuses of controllers 0 through 5. Note that status[0] is more 
//or less unused.
controller_status statuses[6];

void setup() {
  Serial.begin(57600); //initialize serial connection for debugging purposes.
  //initialize statuses array
  for (int i = 1; i <= 5; i++) {
    statuses[i].connected = false;
    statuses[i].verification_number = 0;
  }
}
