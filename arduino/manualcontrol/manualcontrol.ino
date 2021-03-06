/*
  Arduino code for SDP Group 7 2014
  
  This sketch is for use with the ManualControl python class and allows
  for the control of the robot via a keyboard.
  
  This code implements an alternating bit protocol for use with the
  robot.py module. Please adhere to the existing examples when writing
  code as failing to acknowledge commands will result in very bad things.
  
  An example command implementation adhering to this protocol would be
  structured as follows:
  void command() {
    arg1 = comm.next() // Fixed number of arguments
    arg2 = comm.next()
    ack(comm.next()) // Acknowledge once args are read
    doStuff();
  }
*/

#include <SDPArduino.h>
#include <SerialCommand.h>
#include <Wire.h>

// Motor numbers
#define MOTOR_FR 5
#define MOTOR_B 1
#define MOTOR_FL 2
#define MOTOR_KICK 3
#define MOTOR_GRAB 4

// Drive constants
#define MOVE_PWR 100
#define TURN_PWR 50
#define CRAWL_PWR 100

// Kicker and grabber constants
#define SHOOT_POWER 100
#define SHOOT_SWING_TIME 220

#define PASS_POWER 75
#define PASS_SWING_TIME 190

#define KICKER_RESET_POWER 100
#define KICKER_RESET_TIME 190
 
#define GRABBER_POWER 100
#define GRABBER_TIME 800

// Command parser
SerialCommand comm;

// States
boolean grabber_open = false;
boolean kicker_ready = true;

void setup() {
  // Using library set up - it's fine
  SDPsetup();
  
  // Set up command action bindings
  comm.addCommand("FWD", forward);
  comm.addCommand("CRAWL_F", crawlForward);
  comm.addCommand("BACK", backward);
  comm.addCommand("CRAWL_B", crawlBackward);
  comm.addCommand("TURN_L", turnLeft);
  comm.addCommand("TURN_R", turnRight);
  comm.addCommand("O_GRAB", grabberOpen);
  comm.addCommand("C_GRAB", grabberClose);
  comm.addCommand("GRAB", grabberToggle);
  comm.addCommand("SHOOT", shoot);
  comm.addCommand("PASS", pass);
  comm.addCommand("STOP_D", stopDriveMotors);
  comm.addCommand("STOP_A", stopAllMotors);
  comm.addCommand("READY", isReady);
  comm.setDefaultHandler(invalidCommand);
}

void loop() {
  comm.readSerial();
}

void isReady() {
  /*
    Set grabber to default position and let the system know
    that the robot is ready to receive commands
   */
  ack(comm.next());
  grabberClose();
  stopAllMotors();
}

// Actions
void forward() {
  ack(comm.next());
  motorStop(MOTOR_B);
  motorBackward(MOTOR_FR, MOVE_PWR);
  motorForward(MOTOR_FL, MOVE_PWR);
}

void crawlForward() {
  ack(comm.next());
  motorStop(MOTOR_B);
  motorForward(MOTOR_FL, CRAWL_PWR);
  motorBackward(MOTOR_FR, CRAWL_PWR);
}

void backward() {
  ack(comm.next());
  motorStop(MOTOR_B);
  motorForward(MOTOR_FR, MOVE_PWR);
  motorBackward(MOTOR_FL, MOVE_PWR);
}

void crawlBackward() {
  ack(comm.next());
  motorStop(MOTOR_B);
  motorBackward(MOTOR_FL, CRAWL_PWR);
  motorForward(MOTOR_FR, CRAWL_PWR);
}

void turnLeft() {
  ack(comm.next());
  motorBackward(MOTOR_FL, TURN_PWR);
  motorBackward(MOTOR_FR, TURN_PWR);
  motorForward(MOTOR_B, TURN_PWR);
}

void turnRight() {
  ack(comm.next());
  motorForward(MOTOR_FL, TURN_PWR);
  motorForward(MOTOR_FR, TURN_PWR);
  motorBackward(MOTOR_B, TURN_PWR);
}

void grabberToggle() {
  if (grabber_open) {
    grabberClose();
  } else {
    grabberOpen();
  }
}

void grabberClose() {
  ack(comm.next());
  if (grabber_open && kicker_ready) {
    motorBackward(MOTOR_GRAB, GRABBER_POWER);
    grabber_open = false;
    delay(GRABBER_TIME);
    motorStop(MOTOR_GRAB);
  }
}

void grabberOpen() {
  ack(comm.next());
  if (!grabber_open) {
    motorForward(MOTOR_GRAB, GRABBER_POWER);
    grabber_open = true;
    delay(GRABBER_TIME);
    motorStop(MOTOR_GRAB);
  }
}

void pass() {
  ack(comm.next());
  if (kicker_ready && grabber_open) {
    kicker_ready = false;
    motorBackward(MOTOR_KICK, PASS_POWER);
    delay(PASS_SWING_TIME);
    motorStop(MOTOR_KICK);
    resetKicker();
  }
}

void shoot() {
  ack(comm.next());
  if (kicker_ready && grabber_open) {
    kicker_ready = false;
    motorBackward(MOTOR_KICK, SHOOT_POWER);
    delay(SHOOT_SWING_TIME);
    motorStop(MOTOR_KICK);
    resetKicker();
  }
}

void resetKicker() {
  if (!kicker_ready && grabber_open) {
    delay(100);
    motorForward(MOTOR_KICK, KICKER_RESET_POWER);
    delay(KICKER_RESET_TIME);
    motorStop(MOTOR_KICK);
    kicker_ready = true;
  }
  else {
    grabberOpen();
    resetKicker();
  }
}

void stopDriveMotors() {
  ack(comm.next());
  motorStop(MOTOR_FL);
  motorStop(MOTOR_B);
  motorStop(MOTOR_FR);
}

void stopAllMotors() {
  ack(comm.next());
  motorAllStop();
}

void ack(String ack_bit) {
  Serial.println(ack_bit);
  Serial.flush();  // force send
}

void invalidCommand(const char* command) {}
