#include "assert.h"
#include "tensegrity_wireless.h"
#include "Arduino.h"


//////////////////////////// Message marshalling ////////////////////////////
Message * marshall_echo(uint8_t controller_id, Echo *e) {
  Message *m = (Message*) malloc(BASE_MSG_LENGTH);
  m->message_id = ECHO;
  m->controller_id = controller_id;
  m->payload = malloc(ECHO_LENGTH);
  *(uint32_t*)m->payload = e->verification_number;
  return m;
}

/////////////////////////// Message unmarshalling ///////////////////////////
/*
 * Given a pointer to the payload, unmarshalls an Echo message and 
 * returns the corresponding struct.
 */
Echo * unmarshall_echo(void *data) {
  uint32_t *p = (uint32_t*) data;
  Echo *e = (Echo*) malloc(ECHO_LENGTH);
  e->verification_number = *p;
  return e;
}
