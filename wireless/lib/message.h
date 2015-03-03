#ifndef MESSAGE_H
#define MESSAGE_H

/*
 * The message_ids of each message that can be sent 
 */
#define ECHO 0

/* 
 * The lengths of the payloads of each message type. Note that this does not
 * include the two bytes reserved for the message_id and controller_id as these
 * bits are common to all protocol messages.
 */
#define BASE_MSG_LENGTH 2 //2 bytes total in the header.
#define ECHO_LENGTH 4     //echos are only made up of a single 32 bit int.

/*
 * Structs for messages passed over the radio and messages in their user level
 * representations. After a message is sent or received and processed, the 
 * sender/receiver is responsible for freeing the memory used. Same goes for
 * the user representation of a message.
 */
typedef struct
{
  unsigned char message_id;
  unsigned char controller_id;
  void *payload;
} Message;
typedef struct
{
  unsigned int verification_number;
} Echo;


///////////////////////// Message sending/receiving /////////////////////////

/* 
 * Function pointers allowing for different radio
 * libraries to be able to be used to send messages.
 */
typedef void radio_write_func (void *data, unsigned int size);
//typedef void radio_read_func(void *data, unsigned int size);
void send_message(radio_write_func*, Message*);
//void receive_message(radio_read_func*);

//////////////////////////// Message marshalling ////////////////////////////
Message *marshall_echo(unsigned char, int);

/////////////////////////// Message unmarshalling ///////////////////////////
Echo *unmarshall_echo(void*);
#endif /* MESSAGE_H */
