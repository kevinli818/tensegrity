#ifndef TENSEGRITY_WIRELESS_H
#define TENSEGRITY_WIRELESS_H
#include "Arduino.h"

/*
 * The message_ids of each message that can be sent 
 */
#define ECHO 0
#define MOTOR_COMMAND 1
#define ENCODER_READING 2

/* 
 * The lengths in bytes of the payloads of each message. Note that this does not
 * include the four bytes reserved for the message_id, controller_id, and payload
 * pointer as these bits are common to all protocol messages.
 */
#define ECHO_LENGTH 4     //echos are only made up of a single 32 bit int.
#define MOTOR_COMMAND 16
#define ENCODER_READING_LENGTH 16
#define BASE_MSG_LENGTH 4 //2 bytes total in the header 
                          //+2 bytes for the payload pointer


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
  void *payload;
} Message;

typedef struct
{
  uint32_t verification_number;
} Echo;

typedef struct
{
  uint32_t motor1;
  uint32_t motor2;
  uint32_t motor3;
  uint32_t motor4;
} Motor_command;

typedef struct
{
  uint32_t encoder1;
  uint32_t encoder2;
  uint32_t encoder3;
  uint32_t encoder4;
} Encoder_reading;

/////////////////////////// Radio Setup/Wrappers ///////////////////////////
//Radio setup functions along with wrappers to make use of the radio appear 
//to be functional as opposed to object oriented.
//Note that the radio_read_byte and radio_has_data functions are only wrappers
//to make use of the radio appear completely functional to the user.

/* Initializes radio corresponding to the given controller ID*/
void radio_init(uint8_t);
uint8_t radio_read_byte(void);
bool radio_has_data(void);

//////////////////////////// Message marshalling ////////////////////////////
Message * marshall_echo(uint8_t, Echo *);
Message * marshall_motor_command(uint8_t, Motor_command *);
Message * marshall_encoder_reading(uint8_t, Encoder_reading *);

/////////////////////////// Message unmarshalling ///////////////////////////
Echo * unmarshall_echo(void *);
Motor_command * unmarshall_motor_command(void *);
Encoder_reading * unmarshall_encoder_reading(void *);

////////////////////////// Master Helper Functions //////////////////////////
void send_echoes(void);
void check_connections(void);
void handle_echo(uint8_t);

/////////////////////////// Slave Helper Functions //////////////////////////
void send_echo_response();

#endif /* TENSEGRITY_WIRELESS_H */
