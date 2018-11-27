/*
   ___ ____  ____       _    _____ __  __ ____  ____
  |_ _|  _ \/ ___|     / \  |  ___|  \/  |  _ \|  _ \
   | || |_) \___ \    / _ \ | |_  | |\/| | |_) | | | |
   | ||  _ < ___) |  / ___ \|  _| | |  | |  __/| |_| |
  |___|_| \_\____/  /_/   \_\_|   |_|  |_|_|   |____/

  AFMPD Vertical Calibration Rev1.0
  This code programs the teensy to do vertical only calibration of
  AFMPD in tank 8 of IRS Space Propulsion Lab

  Created 19th Sept 2018
  By Akash Mankar
*/

#include <Bounce2.h>          // Software Debouncing Library
#include <TMC2130Stepper.h>   // Stepper Motor Driver library
#include <Adafruit_GFX.h>     // OLED graphics library
#include <Adafruit_SSD1325.h> // OLED driver library


//SM3 Pin definations
#define EN_PIN    46
#define DIR_PIN   41
#define STEP_PIN  42
#define CS_PIN    43
#define MOSI_PIN  45
#define MISO_PIN 1
#define SCK_PIN  44

#define lsmot3 4
#define rsmot3 3

//Motor homing sanity check
bool sm3homed = false;

//Oled pin definations
#define OLED_CS 23
#define OLED_RESET 21
#define OLED_DC 22

//UX Pin definations
#define switch1p 52         // switch 5 NC pin  //NC - Normal Closed
#define switch1led 36      // switch 5 led pin
#define switch2p 53         // switch 6 NC pin 
#define switch2led 35      // switch 6 led pin

//Debouncing limit switches
Bounce calibupswitch = Bounce(rsmot3, 5);
Bounce calibdownswitch = Bounce(lsmot3, 5);
//Debouncing UX switches
Bounce switch1 = Bounce( switch1p, 5 );
Bounce switch2 = Bounce( switch2p, 5 );
TMC2130Stepper SM3 = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN, MOSI_PIN, MISO_PIN, SCK_PIN);
Adafruit_SSD1325 oled(OLED_DC, OLED_RESET, OLED_CS);

void setup() {
  pinMode(rsmot3, INPUT_PULLUP);
  pinMode(lsmot3, INPUT_PULLUP);

  pinMode(switch1p, INPUT);
  pinMode(switch2p, INPUT);
  //Initializing the stepper motor driver and configuring it via SPI
  SM3.begin();
  SM3.rms_current(900);
  SM3.stealthChop(1);
  SM3.microsteps(4);
  //Enabling the motor driver
  digitalWrite(EN_PIN, LOW);
  digitalWrite(DIR_PIN, HIGH);

  //OLED Initialization
  oled.begin();
  oled.clearDisplay();
  oled.setRotation(2);
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);
  oled.println("AF-MPD VERTICAL CALIBRATION");
  oled.display();
  delay(1000);
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(7, 24);

  oled.println("Waiting for Command");
  oled.display();
  //Homing on startup
  SM3home();
  oledtext("Motor at reference home", 1);
}

void loop() {
  //updating switch stauses
  calibupswitch.update();
  calibdownswitch.update();
  switch1.update();
  switch2.update();
  //control logic for UX interaction with switches
  if (switch1.fell()) {
    SM3home();
    oledtext("Motor at reference home", 1);
  }
  if (switch2.fell()) {
    if (sm3homed) {
      oledtext("Starting Calibration Procedure", 1);
      delay(100);
      SM3move(HIGH, 7000);
      delay(30000);
      SM3move(HIGH, 10000);
      delay(30000);
      SM3move(HIGH, 12000);
      delay(30000);
      SM3move(LOW, 12000);
      delay(30000);
      SM3move(LOW, 10000);
      delay(30000);
      SM3move(LOW, 5000);
      delay(30000);
      oledtext("Calibration Complete", 2);
    }
    else if (!sm3homed) {
      oledtext("Homing before calibration", 1);
      SM3home();
      oledtext("Starting Calibration Procedure", 1);
      delay(100);
      SM3move(HIGH, 7000);
      delay(6000);
      SM3move(HIGH, 10000);
      delay(6000);
      SM3move(HIGH, 12000);
      delay(6000);
      SM3move(LOW, 12000);
      delay(6000);
      SM3move(LOW, 10000);
      delay(6000);
      SM3move(LOW, 5000);
      delay(6000);
      oledtext("Calibration Complete", 2);
    }
  }
  oledtext("Waiting for command", 2);
  
}

// SM3 motor movement abstracted function
void SM3move(bool dir, int steps) {
  digitalWrite(DIR_PIN, dir); //setting direction to move
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(400);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(400);
  }
  sm3homed = false;
}

//SM3 in verical calibration homes to the downward
//limit switch
void SM3home() {
  if (!sm3homed) {
    oledtext("Homing Calibration Unit..", 1);
    digitalWrite(DIR_PIN, HIGH);
    do {
      calibdownswitch.update();
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(1500);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(1500);
    } while (homesense(calibdownswitch));

    //move preset number of steps in the opposite
    //direction to have a homing reference
    oledtext("Moving Calibration to go up a little.", 1);
    digitalWrite(DIR_PIN, LOW);

    for (int i = 0; i < 200; i++) {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(1500);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(1500);
    }
    sm3homed = true;
    oledtext("Homing Calibration Unit..Done.", 1);
    delay(500);
    oledtext("Waiting for Command..", 1);
  }
  else {
    oledtext("Calibration Unit already at reference home.", 1);
    delay(1000);
  }
}

//set oled text
char * oledtext(String txt, int size) {
  oled.clearDisplay();
  oled.setRotation(2);
  oled.setTextSize(size);
  oled.setTextColor(WHITE);
  oled.setCursor(5, 5);
  oled.println(txt);
  oled.display();
}

//check the limit switch press
bool homesense(Bounce db) {
  // Senses the limit switch press and returns false
  if (db.rose()) {
    return false;
  } else {
    return true;
  }
}

