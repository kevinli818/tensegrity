#include "assert.h"
#include "lib/message.h"
#include <malloc.h>

//////////////////////////// Message marshalling ////////////////////////////
Message *marshall_echo(unsigned char controller_id, int verification_number) {
  Message *m = malloc(BASE_MSG_LENGTH + ECHO_LENGTH);
  m->message_id = ECHO;
  m->controller_id = controller_id;
  m->payload = ((char*) m + BASE_MSG_LENGTH);
  *(unsigned int*) m->payload = verification_number;
  return m;
}

/////////////////////////// Message unmarshalling ///////////////////////////
/*
 * Given a pointer to the payload, unmarshalls an Echo message and 
 * returns the corresponding struct.
 */
Echo *unmarshall_echo(void *data) {
  int *p = (int*) data;
  Echo *e = malloc(ECHO_LENGTH);
  e->verification_number = *p;
  return e;
}

///////////////////////// Message sending/receiving /////////////////////////
void send_message(radio_write_func *write_function, Message *m) {
  write_function(&m->message_id, sizeof(unsigned char));
  write_function(&m->controller_id, sizeof(unsigned char));
  switch(m->message_id) {
    case ECHO:
      write_function(m->payload, ECHO_LENGTH);
      break;
  }
}
/*
void receive_message(radio_read_func *read_function){
}
*/
