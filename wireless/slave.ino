#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <tensegrity_wireless.h>
#include <Encoder.h>

//ID is 0 for the master controller, 1 through 5 for slaves.
#define ID 4
#define ENCODER_READING_FREQUENCY 500

/* Motor init */
Encoder motor4(16, 17);
int sp4 = 20, spb4 = 21;
Encoder motor3(14, 15);
int sp3 = 23, spb3 = 22;
Encoder motor2(7, 8);
int sp2 = 5, spb2 = 6;
Encoder motor1(1, 2);
int sp1 = 3, spb1 = 4;

float K_1 = .5,K_2 = .5, K_3 = .5, K_4 = .5; 
int   Kp_1 = 2, Kp_2 = 2, Kp_3 = 2, Kp_4 = 2;    
int   Ki_1 = 1,Ki_2 = 1,Ki_3 = 1,Ki_4 = 1;    
int   Kd_1 = 2, Kd_2 = 2, Kd_3 = 2, Kd_4 = 2;  
int last_error_1 = 0, last_error_2 = 0, last_error_3 = 0, last_error_4 = 0;
int integrated_error_1 = 0, integrated_error_2 = 0, integrated_error_3 = 0, integrated_error_4 = 0;

int pTerm_1 = 0, iTerm_1 = 0, dTerm_1 = 0;
int pTerm_2 = 0, iTerm_2 = 0, dTerm_2 = 0;
int pTerm_3 = 0, iTerm_3 = 0, dTerm_3 = 0;
int pTerm_4 = 0, iTerm_4 = 0, dTerm_4 = 0;

int target_1, target_2, target_3, target_4;
int error_1 = 0, error_2 = 0, error_3 = 0, error_4 = 0;
float updatePid1 = 0,  updatePid2 = 0,  updatePid3 = 0,  updatePid4 = 0;

int GUARD_GAIN = 10;

volatile int encoder1Pos = 0;
volatile int encoder2Pos = 0;
volatile int encoder3Pos = 0;
volatile int encoder4Pos = 0;

elapsedMillis next_encoder_reading;
elapsedMillis print_debug_data;

void setup() {
  pinMode(sp1, OUTPUT);
  pinMode(spb1, OUTPUT);
  pinMode(sp2, OUTPUT);
  pinMode(spb2, OUTPUT);
  pinMode(sp3, OUTPUT);
  pinMode(spb3, OUTPUT);
  pinMode(sp4, OUTPUT);
  pinMode(spb4, OUTPUT);
  
  Serial.begin(57600);
  radio_init(ID);
}

void loop() {
  encoder1Pos = motor1.read();
  encoder2Pos = motor2.read();
  encoder3Pos = motor3.read();
  encoder4Pos = motor4.read();

  if (next_encoder_reading >= ENCODER_READING_FREQUENCY) {
    send_encoder_reading(encoder1Pos, encoder2Pos, encoder3Pos, encoder4Pos);
    next_encoder_reading = 0;
  }

  if (radio_has_data()) {
    Message *m = receive_message();
    Serial.println("Message received.");
    switch(m->message_id) {
      case ECHO:
        if (m->controller_id != 0) {
          Serial.println("Error. EchoRequests should only be coming in from master.");
        }
        delay(5*ID);
        send_echo(ID, m->payload.verification_number);
        break;
      case MOTOR_COMMAND:
        if (m->controller_id == ID) {
          Serial.println("Received motor command.");
          target_1 = (m->payload.motor_command.m1 != MOTOR_STAY) ? m->payload.motor_command.m1 : target_1;
          target_2 = (m->payload.motor_command.m2 != MOTOR_STAY) ? m->payload.motor_command.m2 : target_2;
          target_3 = (m->payload.motor_command.m3 != MOTOR_STAY) ? m->payload.motor_command.m3 : target_3;
          target_4 = (m->payload.motor_command.m4 != MOTOR_STAY) ? m->payload.motor_command.m4 : target_4;
        }
        break;
      case ENCODER_READING:
        Serial.println("Error: EncoderReadings should only be sent from slave->master.");
        break;
      case ENDCAP_SENSOR_READING:
        Serial.println("Error: EndcapSensorReadings should only be sent from slave->master.");
        break;
      case ERROR: //TODO(vdonato): add more nontrivial error handling
        Serial.println("Something went wrong.");
        break;
    }
  }

  error_1 = target_1 - encoder1Pos;
  error_2 = target_2 - encoder2Pos;
  error_3 = target_3 - encoder3Pos;
  error_4 = target_4 - encoder4Pos;
  
  pTerm_1 = Kp_1 * error_1;
  pTerm_2 = Kp_2 * error_2;
  pTerm_3 = Kp_3 * error_3;
  pTerm_4 = Kp_4 * error_4;
  
  integrated_error_1 += error_1; 
  integrated_error_2 += error_2;   
  integrated_error_3 += error_3; 
  integrated_error_4 += error_4; 
  
  iTerm_1 = Ki_1 * constrain(integrated_error_1, -GUARD_GAIN, GUARD_GAIN);
  iTerm_2 = Ki_2 * constrain(integrated_error_2, -GUARD_GAIN, GUARD_GAIN);
  iTerm_3 = Ki_3 * constrain(integrated_error_3, -GUARD_GAIN, GUARD_GAIN);
  iTerm_4 = Ki_4 * constrain(integrated_error_4, -GUARD_GAIN, GUARD_GAIN);
  
  dTerm_1 = Kd_1 * (error_1 - last_error_1);                            
  dTerm_2 = Kd_2 * (error_2 - last_error_2);
  dTerm_3 = Kd_3 * (error_3 - last_error_3);
  dTerm_4 = Kd_4 * (error_4 - last_error_4);
  
  last_error_1 = error_1;
  last_error_2 = error_2;
  last_error_3 = error_3;
  last_error_4 = error_4;
  
  updatePid1 = constrain(K_1*(pTerm_1 + iTerm_1 + dTerm_1), -255, 255);
  updatePid2 = constrain(K_2*(pTerm_2 + iTerm_2 + dTerm_2), -255, 255);
  updatePid3 = constrain(K_3*(pTerm_3 + iTerm_3 + dTerm_3), -255, 255);
  updatePid4 = constrain(K_4*(pTerm_4 + iTerm_4 + dTerm_4), -255, 255);
  
  if (updatePid4 < 0) {
    analogWrite(sp4, abs(updatePid4));
  } 
  else {
    analogWrite(spb4, updatePid4); 
  } 
  if (updatePid3 < 0) {
    analogWrite(sp3, abs(updatePid3));
  } 
  else {
    analogWrite(spb3, updatePid3); 
  } 
  if (updatePid2 < 0) {
    analogWrite(sp2, abs(updatePid2));
  } 
  else {
    analogWrite(spb2, updatePid2); 
  } 
  if (updatePid1 < 0) {
    analogWrite(sp1, abs(updatePid1));
  } 
  else {
    analogWrite(spb1, updatePid1); 
  } 
  
  if (print_debug_data >= 1000) {
    Serial.print("Encoder 1: \t");
    Serial.println (encoder1Pos, DEC);
    Serial.print("Encoder 2: \t");
    Serial.println (encoder2Pos, DEC);
    Serial.print("Encoder 3: \t");
    Serial.println (encoder3Pos, DEC);
    Serial.print("Encoder 4: \t");
    Serial.println (encoder4Pos, DEC);

    Serial.print("Target 1: \t");
    Serial.println (target_1, DEC);
    Serial.print("Target 2: \t");
    Serial.println (target_2, DEC);
    Serial.print("Target 3: \t");
    Serial.println (target_3, DEC);
    Serial.print("Target 4: \t");
    Serial.println (target_4, DEC);
    print_debug_data = 0;
  }
}
