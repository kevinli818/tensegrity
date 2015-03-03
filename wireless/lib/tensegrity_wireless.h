#ifndef TENSEGRITY_WIRELESS_H
#define TENSEGRITY_WIRELESS_H
#include "Arduino.h"

/*
 * The message_ids of each message that can be sent 
 */
#define ECHO 0

#define BASE_MSG_LENGTH 4 //2 bytes total in the header 
                           //+2 bytes for the payload pointer
/* 
 * The lengths in bytes of the payloads of each message. Note that this does not
 * include the two bytes reserved for the message_id and controller_id as these
 * bits are common to all protocol messages.
 */
#define ECHO_LENGTH 4     //echos are only made up of a single 32 bit int.

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

//////////////////////////// Message marshalling ////////////////////////////
Message *marshall_echo(uint8_t, Echo *);

/////////////////////////// Message unmarshalling ///////////////////////////
Echo *unmarshall_echo(void *);

#endif /* TENSEGRITY_WIRELESS_H */
