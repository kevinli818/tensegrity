#ifndef TENSEGRITY_WIRELESS_H
#define TENSEGRITY_WIRELESS_H
#include "Arduino.h"


/*
 * The message_ids of each message that can be sent 
 */
#define ECHO 0
#define MOTOR_COMMAND 1
#define ENCODER_READING 2
#define ENDCAP_SENSOR_READING 3
#define ERROR 7

/*
 * Special MotorCommand messages.
 */
#define MOTOR_STAY 0xFFF

#define MESSAGE_LENGTH 32

/*
 * Structs for messages passed over the radio and messages in their user level
 * representations. After a message is sent or received and processed, the 
 * sender/receiver is responsible for freeing the memory used. Same goes for
 * the user representation of a message.
 */
typedef struct
{
  uint8_t message_id;
  uint8_t controller_id;
  union payload {
    uint32_t verification_number;
    struct motor_command {
      int32_t m1;
      int32_t m2;
      int32_t m3;
      int32_t m4;
    } motor_command;
    struct encoder_reading {
      int32_t e1;
      int32_t e2;
      int32_t e3;
      int32_t e4;
    } encoder_reading;
    struct endcap_sensor_reading {
      uint8_t e1; //1 if on. off otherwise.
      uint8_t e2;
    } endcap_sensor_reading;
  } payload;
} Message;

/////////////////////////// Radio Setup/Wrappers ///////////////////////////
//Radio setup functions along with wrappers to make use of the radio appear 
//to be functional as opposed to object oriented.

/* Initializes radio corresponding to the given controller ID*/
void radio_init(uint8_t);
bool radio_has_data(void);

////////////////////////////// Message sending //////////////////////////////
void send_echo(uint8_t, uint32_t);
void send_motor_command(uint8_t, int32_t, int32_t, int32_t, int32_t);
void send_encoder_reading(int32_t, int32_t, int32_t, int32_t);
void send_endcap_sensor_reading(bool, bool);

///////////////////////////// Message receiving /////////////////////////////
Message * receive_message(void);

#endif /* TENSEGRITY_WIRELESS_H */
