#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <tensegrity_wireless.h>

#define ECHO_FREQUENCY 10000
#define ECHO_DEADLINE 500
#define ID 0

/* defs added for autonomous circle path */
#define SLAVE_ID 3 //should probably change this number soon
#define COUNTS_PER_STEP 16200 //3600 counts per turn * 4.5 turns
#define MAX_ERROR 50
/* end of defs added for autonomous circle*/

typedef enum {
  NEUTRAL1,
  STEP1,
  NEUTRAL2,
  STEP2,
  NEUTRAL3,
  STEP3,
  NEUTRAL4,
  STEP4,
} robot_status_t;
robot_status_t robot_status = NEUTRAL1;
bool enabled = 0;

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

int32_t motor_vals[5];
int32_t encoder_readings[6][4]; //first index specifies controller number, second motor number
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
        Serial.println(m->payload.encoder_reading.e1);
        Serial.println(m->payload.encoder_reading.e2);
        Serial.println(m->payload.encoder_reading.e3);
        Serial.println(m->payload.encoder_reading.e4);
        encoder_readings[m->controller_id][0] = m->payload.encoder_reading.e1;
        encoder_readings[m->controller_id][1] = m->payload.encoder_reading.e2;
        encoder_readings[m->controller_id][2] = m->payload.encoder_reading.e3;
        encoder_readings[m->controller_id][3] = m->payload.encoder_reading.e4;
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
    char c = Serial.read();
    if (c == 'D') {
      enabled = 0;
      robot_status = NEUTRAL1;
    }
    else if (c == 'E') {
      enabled = 1;
    }

    //manual motor command input commented out for now.
    /*
    motor_vals[k] = (uint32_t) Serial.parseInt();
    Serial.println(motor_vals[k]);
    k++;
    if (k == 5) {
      send_motor_command(motor_vals[0], motor_vals[1], motor_vals[2], motor_vals[3], motor_vals[4]);
      k = 0;
    }
  */
  }

  switch (robot_status) {
    case NEUTRAL1:
      //only send motor_commands if needed since robot may be disabled
      if (!(abs(encoder_readings[SLAVE_ID][0]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][1]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][2]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][3]) <= MAX_ERROR)) {
        send_motor_command(SLAVE_ID, 0, 0, 0, 0);
      }
      //if all encoder readings are within the required tolerances
      if (abs(encoder_readings[SLAVE_ID][0]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][1]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][2]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][3]) <= MAX_ERROR) {
        if (enabled) {
          robot_status = STEP1; //move onto the next step only if enabled
        }
      }
      break;
    case STEP1:
      send_motor_command(SLAVE_ID, COUNTS_PER_STEP, 0, 0, 0);
      if (abs(encoder_readings[SLAVE_ID][0] - COUNTS_PER_STEP) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][1]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][2]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][3]) <= MAX_ERROR) {
        robot_status = NEUTRAL2;
      }
      break;
    case NEUTRAL2:
      send_motor_command(SLAVE_ID, 0, 0, 0, 0);
      if (abs(encoder_readings[SLAVE_ID][0]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][1]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][2]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][3]) <= MAX_ERROR) {
        robot_status = STEP2;
      }
      break;
    case STEP2:
      send_motor_command(SLAVE_ID, 0, COUNTS_PER_STEP, 0, 0);
      //if all encoder readings are within the required tolerances
      if (abs(encoder_readings[SLAVE_ID][0]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][1] - COUNTS_PER_STEP) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][2]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][3]) <= MAX_ERROR) {
        robot_status = NEUTRAL3;
      }
      break;
    case NEUTRAL3:
      send_motor_command(SLAVE_ID, 0, 0, 0, 0);
      //if all encoder readings are within the required tolerances
      if (abs(encoder_readings[SLAVE_ID][0]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][1]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][2]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][3]) <= MAX_ERROR) {
        robot_status = STEP3; //move onto the next step
      }
      break;
    case STEP3:
      send_motor_command(SLAVE_ID, 0, 0, COUNTS_PER_STEP, 0);
      //if all encoder readings are within the required tolerances
      if (abs(encoder_readings[SLAVE_ID][0]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][1]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][2] - COUNTS_PER_STEP) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][3]) <= MAX_ERROR) {
        robot_status = NEUTRAL4; //move onto the next step
      }
      break;
    case NEUTRAL4:
      send_motor_command(SLAVE_ID, 0, 0, 0, 0);
      //if all encoder readings are within the required tolerances
      if (abs(encoder_readings[SLAVE_ID][0]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][1]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][2]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][3]) <= MAX_ERROR) {
        robot_status = STEP4; //move onto the next step
      }
      break;
    case STEP4:
      send_motor_command(SLAVE_ID, 0, 0, 0, COUNTS_PER_STEP);
      //if all encoder readings are within the required tolerances
      if (abs(encoder_readings[SLAVE_ID][0]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][1]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][2]) <= MAX_ERROR &&
          abs(encoder_readings[SLAVE_ID][3]) <= MAX_ERROR - COUNTS_PER_STEP) {
        robot_status = NEUTRAL1; //move onto the next step
      }
      break;
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
