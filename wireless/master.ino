#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <tensegrity_wireless.h>

#define ECHO_FREQUENCY 10000
#define ECHO_DEADLINE 500
#define ID 0

typedef struct
{
  bool has_responded; //true if controller has responded to the latest EchoRequest
} controller;
//the perceived status of controllers 0 through 5. Note that status[0] is unused as
//there's no need for the master to keep track of connectivity with itself.
controller controllers[6];
void check_connections(void);

elapsedMillis next_echo;
elapsedMillis echo_deadline;
//system clock time to verify that echos have been responded to.
bool echo_verified = true;
uint32_t verification_number = 0;

int motor_vals[5];
int k = 0;

void setup() {
  Serial.begin(57600);
  for (int i = 1; i <= 5; i++) {
    controllers[i].has_responded = false;
  }
  radio_init(ID);
}

void loop() {
  //send echoes and set response deadline.
  if (next_echo >= ECHO_FREQUENCY) {
    for (int i = 1; i <= 5; i++) {
      controllers[i].has_responded = false;
    }
    //send EchoRequest with c_id 0 (to all slaves)
    send_echo(ID, ++verification_number);
    next_echo = 0;
    echo_deadline = 0;
    echo_verified = false;
  }
  //checks to see if every controller has responded
  //to echo by the keepalive deadline.
  if (echo_deadline >= ECHO_DEADLINE && !echo_verified) {
    check_connections();
    echo_verified = true;
  }
  if (radio_has_data()) {
    Message *m = receive_message();
    switch (m->message_id) {
      case ECHO:
        if (m->payload.verification_number == verification_number) {
          controllers[m->controller_id].has_responded = true;
        }
        break;
      case MOTOR_COMMAND:
        Serial.println("ERROR: MotorCommands should only be sent from master->slave.");
        break;
      case ENCODER_READING:
        Serial.println("Received encoder readings");
        break;
      case ENDCAP_SENSOR_READING:
        Serial.println("Received endcap sensor readings");
        break;
      case ERROR: //TODO(vdonato): add more nontrivial error handling
        Serial.println("Something went wrong.");
        break;
    }
  }
  if (Serial.available()) {
    motor_vals[k] = (uint32_t) Serial.parseInt();
    Serial.println(motor_vals[k]);
    k++;
    if (k == 5) {
      send_motor_command(motor_vals[0], motor_vals[1], motor_vals[2], motor_vals[3], motor_vals[4]);
      k = 0;
    }
  }
}

void check_connections() {
  bool all_connected = true;
  for (int i = 1; i <= 5; i++) {
    if (!controllers[i].has_responded) {
      Serial.print("Controller ");
      Serial.print(i);
      Serial.println(" not connected.");
      all_connected = false;
    }
  }
  if (all_connected) {
    Serial.println("All controllers connected!");
  }
}
