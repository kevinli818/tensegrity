#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

byte addresses[][6] = {"Master", "Slave1", "Slave2", "Slave3", "Slave4", "Slave5"};
//id is 0 for the master controller. 1 through 5 for the slaves.
int id = 1;
