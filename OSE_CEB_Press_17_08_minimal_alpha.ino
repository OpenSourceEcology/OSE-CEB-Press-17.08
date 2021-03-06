/* Open Source Ecology CEB Press v19.01 with Arduino Uno or Mega as controller of choice.
 Use Mega to keep consistency with OSE D3D 3D Printer to minimize GVCS part count.
 Switches FET's HIGH/LOW to control two hydraulic solenoids,
 measures piston motion time relative to pressure sensor trigger,
 and repeats cycle while auto calibrating timing from previous cycles and startup positions.
 
 Extension time is measured. No timing compensation is necessary for compression chamber closure.
 
 Faults should be self-resolving based on pressing sequence. See sequence:
 
 https://docs.google.com/presentation/d/1YLo7e6RW4SUq7v5NFQ2WoX2ZFHsnoa3wI9m8Oc5-24g/edit#slide=id.g4d84192e63_0_0
 
 Uses HiLetgo Relay Shield 5V 4 Channel for Arduino.
 
 Relay shield - https://www.amazon.com/HiLetgo-Relay-Shield-Channel-Arduino/dp/B07F7Y55Z7/ref=sr_1_2?ie=UTF8&qid=1547696929&sr=8-2&keywords=hiletgo+relay+shield
 
 Relay 1 is controlled by digital pin 7
 Relay 2 is controlled by digital pin 6
 Relay 3 is controlled by digital pin 5
 Relay 4 is controlled by digital pin 4

 A high written to a pin turns the relay ON
 A low written to a pin turns the relay OFF
 
 Contributions by:
 Abe Anderson
 http://opensourceecology.org/wiki/AbeAnd_Log
 Marcin Jakubowski
 http://opensourceecology.org/wiki/Marcin_Log
  
 Unfamiliar with code structures? See https://www.arduino.cc/en/Reference/HomePage
 
 License:
 See GPLv3 license file in repo.
 */

//defines to make it easier for non-coders to make adjustments for troubleshooting and custom changes


#define SOLENOID_UP 4      //Extension. See pin mapping above.
#define SOLENOID_DOWN 5    //swap these pin numbers for wire inversion  

#define SOLENOID_LEFT 6    //Extension.
#define SOLENOID_RIGHT 7   //swap these pin numbers for wire inversion     

    

#define PRESSURE_SENSOR 41  //labeled A3 or F3 silkscreen on the PCB
#define PRESSURE_SENSOR_DEBOUNCE 20 //milliseconds to delay for pressure sensor debounce
#define DELAY 500  // 1/2 sec extra to compress brick via main Cyl (default 500ms)
#define K_A_DRAWER 0.84 // T_c = T_e * (k_A)  for 2.5in x 14in cylinder  (default 0.008)

// custom structures, function declarations or prototypes
bool lowPressure();    //function to read if pressure sensor is HIGH
unsigned long drawerExtTime = 0;

void setup() {

  //initialize pin I/O Inputs use internal resistor pullups where needed and outputs get set low to prevent glitches while booting
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

  //Initialize  to start position retract main cyl for 0.5 sec to release pressue contract drawer cylinder fully
  digitalWrite(SOLENOID_DOWN, HIGH);
  delay(DELAY);
  digitalWrite(SOLENOID_DOWN, LOW);
  while (lowPressure() == true) {
    digitalWrite(SOLENOID_RIGHT, HIGH);
  }
//  digitalWrite(SOLENOID_RIGHT, LOW);// check this - but - pressure may still be high; depends on solenoid - but for 
                                    // cylinder soilenoid, the pressure may stay high
//  digitalWrite(SOLENOID_LEFT, HIGH);//release pressure in lines 63-65
//  delay(DELAY/10);
//  digitalWrite(SOLENOID_LEFT, LOW);
}

void loop() {

  //Auto mode

  unsigned long previousMillis = 0;

  //Step 1 Calibration Extend drawer Cylinder Fully T_ext is measure
  previousMillis = millis();
  while (lowPressure() == true) {

    digitalWrite(SOLENOID_LEFT, HIGH);
  }
  digitalWrite(SOLENOID_LEFT, LOW);
  drawerExtTime = millis() - previousMillis;

  //Step 2 Main Cyl moves down to allow soil loading
  while (lowPressure() == true) {
    digitalWrite(SOLENOID_DOWN, HIGH);
  }
  digitalWrite(SOLENOID_DOWN, LOW);

  //Step 3 Contract drawer cylinder half way to close compression chamber
  previousMillis = millis();
  while (lowPressure() == true) {

    while ((millis() - previousMillis) <= (( drawerExtTime * K_A_DRAWER ) / 2 )) {
      digitalWrite(SOLENOID_RIGHT, HIGH);
    }
    digitalWrite(SOLENOID_RIGHT, LOW);
  }

  //Step 4 compression by main cyl with delay
  while (lowPressure() == true) {
    digitalWrite(SOLENOID_UP, HIGH);
  }
  delay(DELAY);
  digitalWrite(SOLENOID_UP, LOW);

  //Step 5 main Cyl release pressure
  digitalWrite(SOLENOID_DOWN, HIGH);
  delay(DELAY/5);
  digitalWrite(SOLENOID_DOWN, LOW);

  //Step 6 drawer Cyl contracts opening chamber
  while (lowPressure() == true) {
    digitalWrite(SOLENOID_RIGHT, HIGH);
  }
  digitalWrite(SOLENOID_RIGHT, LOW);

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

