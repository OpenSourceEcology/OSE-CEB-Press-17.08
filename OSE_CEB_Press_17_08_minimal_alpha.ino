/* Open Source Ecology CEB Press v17.09 v8 Teensy Microcontroller code for Auto mode operation
 Switches FET's HIGH/LOW to control two 3 position hydraulic solenoids,
 measures piston motion time relative to pressure sensor trigger,
 and repeats cycle while auto calibrating timing from previous cycles and startup positions.
 
 Compensates for difference in time for Extension and Contraction of Rods.
 T_extend = T_contract  * (A_cyl - A_rod) / A_cyl)
 
 Faults require manual user intervention to reset to starting position if faults occur and power must be shujt off to controller by engaging OFF/MANUAL mode(s).
 
 Contributions by:
 Abe Anderson
 http://opensourceecology.org/wiki/AbeAnd_Log
 
 Unfamiliar with code structures? See https://www.arduino.cc/en/Reference/HomePage
 
 License:
 See GPLv3 license file in repo.
 */

//defines to make it easier for non coders to make adjustments for troubleshooting and custom changes

#define SOLENOID_RIGHT 5   //swap these pin numbers for wire inversion      (deafult pin 5)
#define SOLENOID_LEFT 4    //    (default pin 4)

#define SOLENOID_DOWN 15    //swap these pin numbers for wire inversion   (default pin 15)
#define SOLENOID_UP 14      //    (default pin 14)

#define MODE_SELECT 7    //This is for the 3 position SPDT switch for Manual/OFF/Auto
#define PRESSURE_SENSOR 41  //labeled A3 or F3 silkscreen on the PCB

#define SWITCH_DEBOUNCE 3 //milliseconds to delay for switch debounce
#define PRESSURE_SENSOR_DEBOUNCE 20 //milliseconds to delay for pressure sensor debounce
#define COMPRESS_DELAY 500  // 1/2 sec extra to compress brick via main Cyl (default 500ms)
#define RELEASE_PRESSURE_DELAY 100    //releases pressure from the drawer bottom after compression (default 100ms)
#define K_A_MAIN 0.004  // T_e = T_c * (k_A)   for 1.25in x14in cylinder  (default 0.004)
#define K_A_DRAWER 0.008 // T_e = T_c * (k_A)  for 2.75in x10in cylinder  (default 0.008)
#define MAXDRIFT 500    //Sets maximum time difference in milliseconds from one cycle to the next for all steps to check for faults (default 500ms)

// custom structures, function declarations or prototypes
bool lowPressure();    //function to read if pressure sensor is HIGH

void setup() {

  //initialize pin I/O Inputs use internal resistor pullups where needed and outputs get set low to prevent glitches while booting
  pinMode(MODE_SELECT, INPUT);
  digitalWrite(MODE_SELECT, INPUT_PULLUP);
  pinMode(SOLENOID_RIGHT, OUTPUT);
  digitalWrite(SOLENOID_RIGHT, LOW);
  pinMode(SOLENOID_LEFT, OUTPUT);
  digitalWrite(SOLENOID_LEFT, LOW);
  pinMode(SOLENOID_DOWN, OUTPUT);
  digitalWrite(SOLENOID_DOWN, LOW);
  pinMode(SOLENOID_UP, OUTPUT);
  digitalWrite(SOLENOID_UP, LOW);
  pinMode(PRESSURE_SENSOR, INPUT);
  digitalWrite(PRESSURE_SENSOR, INPUT_PULLUP);

  //Initialize  to start position retract main cyl for 1 sec to release pressue contract drawer cylinder fully
  digitalWrite(SOLENOID_DOWN, HIGH);
  delay(1000);
  digitalWrite(SOLENOID_DOWN, LOW);
  while (lowPressure() == true) {
    digitalWrite(SOLENOID_RIGHT, HIGH);
  }
  digitalWrite(SOLENOID_RIGHT, LOW);
}

void loop() {

  //Auto mode

  unsigned long previousMillis = 0;

  static unsigned long drawerExtTime = 0;
  static unsigned long drawerExtTimePre = 0;   //previous time

  static float kADrawer = K_A_DRAWER;
  unsigned long minimum = 0;    //do math
  unsigned long maximum = 0;    //and compare values
  word drift = 0;               //for timing drift tracking

  //Step 1 Calibration Extend drawer Cylinder Fully T_ext is measured
  while (lowPressure() == true) {
    previousMillis = millis();
    digitalWrite(SOLENOID_LEFT, HIGH);
  }
  digitalWrite(SOLENOID_LEFT, LOW);
  drawerExtTime = millis() - previousMillis;

  if (drawerExtTimePre == 0) {
    drawerExtTimePre = drawerExtTime;
  }
  else {
    if (drawerExtTime != drawerExtTimePre) {
      minimum = min(drawerExtTime, drawerExtTimePre);
      maximum = max(drawerExtTime, drawerExtTime);
      drift = maximum - minimum;
      if (drift > MAXDRIFT) {
        while( true ) { //sleep in infinite loop
        }
      }
    }
  }
  drawerExtTimePre = drawerExtTime;

  //Step 2 Main Cyl moves down to allow soil loading measure T_con time
    while (lowPressure() == true) {
      digitalWrite(SOLENOID_DOWN, HIGH);
    }
    digitalWrite(SOLENOID_DOWN, LOW);
  }

  //Step 3 Contract drawer cylinder half way to close compression chamber
  while (lowPressure() == true) {
    //drawerMidTime = drawerExtTime / kADrawer ;
    previousMillis = millis();
    while ((millis() - previousMillis) <= (drawerExtTime/2)) {
      digitalWrite(SOLENOID_RIGHT, HIGH);
    }
    digitalWrite(SOLENOID_RIGHT, LOW);
  }

  //Step 4 compression by main cyl with 1 sec
  while (lowPressure() == true) {
    digitalWrite(SOLENOID_UP, HIGH);
  }
  delay(COMPRESS_DELAY);
  digitalWrite(SOLENOID_UP, LOW);

  //Step 5 main Cyl release pressure
  digitalWrite(SOLENOID_DOWN, HIGH);
  delay(RELEASE_PRESSURE_DELAY);
  digitalWrite(SOLENOID_DOWN, LOW);

  //Step 6 drawer Cyl contracts opening chamber
  while (lowPressure() == true) {
    digitalWrite(SOLENOID_DOWN, HIGH);
  }
  digitalWrite(SOLENOID_DOWN, LOW);

  //Step 7 main moves brick up to eject
  while (lowPressure() == true) {
    digitalWrite(SOLENOID_UP, HIGH);
  }
  digitalWrite(SOLENOID_UP, LOW);

  //Loops back to step 1 to eject brick
}
//end of main loop

//custom functions

//reads pressure sensor state HIGH is false and LOW is true
bool lowPressure() {
  if (digitalRead(PRESSURE_SENSOR) == HIGH) {
    delay(PRESSURE_SENSOR_DEBOUNCE);
    if (digitalRead(PRESSURE_SENSOR) == HIGH) {
      return false;
    }
    else {
      return true;
    }
  }
  else {
    return true;
  }
}