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

#define THICKNESS_SELECT 40		//Input Button for thickness selection		(default pin 40?)

#define SOLENOID_RIGHT 5   //swap these pin numbers for wire inversion      (default pin 5)
#define SOLENOID_LEFT 4    //    (default pin 4)

#define SOLENOID_DOWN 15    //swap these pin numbers for wire inversion   (default pin 15)
#define SOLENOID_UP 14      //    (default pin 14)

#define PRESSURE_SENSOR 41  //labeled A3 or F3 silkscreen on the PCB

#define SWITCH_DEBOUNCE 3 //milliseconds to delay for switch debounce
#define PRESSURE_SENSOR_DEBOUNCE 20 //milliseconds to delay for pressure sensor debounce
#define COMPRESS_DELAY 1000  // 1 sec extra to compress brick via main Cyl (default 1000ms)
#define RELEASE_PRESSURE_DELAY 100    //releases pressure from the drawer bottom after compression (default 100ms)
#define K_A_MAIN 0.004  // T_e = T_c * (k_A)   for 1.25in x14in cylinder  (default 0.004)
#define K_A_DRAWER 0.008 // T_e = T_c * (k_A)  for 2.75in x10in cylinder  (default 0.008)
#define MAXDRIFT 100    //Sets maximum time difference in milliseconds from one cycle to the next for all steps to check for faults (default 100ms)

// custom structures, function declarations or prototypes
bool lowPressure();    //function to read if pressure sensor is HIGH
void faultCheck( unsigned long currentTime, unsigned long prevTime);
bool move( byte cylinderDirection, word delayMod, byte thicknessDelay);


void setup() {

  //initialize pin I/O Inputs use internal resistor pullups where needed and outputs get set low to prevent glitches while booting
  //pingMode(THICKNESS_SELECT, INPUT);
  //digitalWrite(THICKNESS_SELECT, INPUT_PULLUP);
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
  while ((lowPressure() == true) {
    digitalWrite(SOLENOID_RIGHT, HIGH);
    }
         digitalWrite(SOLENOID_RIGHT, LOW);
}

  
void loop() {
  /*
    Auto mode starting assumptions
    User has manually tested system for proper function
    and it is ready to extend the drawer measure time
    and
  */
	  
	byte drawerExtend = SOLENOID_LEFT;
	byte drawerContract = SOLENOID_RIGHT;
	byte mainExtend = SOLENOID_UP;
	byte mainContract = SOLENOID_DOWN;
	word delayMod = 0;		
	static byte thicknessDelay = 1;		//used for thickness button input selection
	
	//Create initial button polling routine for a several seconds to check for thickness setting option?
	//Indicator lights would be useful for button selection.
	//Poll for button press(es) debounce and count. Need to write, copy, and/or point to var for use in loop?
	  
	/*
	
	if(digitalRead( THICKNESS_SELECT ) == true {
	//debounce
	thicknessDelay = 3;
	//count presses correlate with 3, 2, & 4. use as divider for 3 in, 2 in, and 1 in block.
	
	}
	
	*/  
	  
    //Step 1 Calibration Extend drawer Cylinder Fully T_ext is measured
  	 	  
	move(drawerExtend);
	  
	//Step 2 Main Cyl moves down to allow soil loading measure T_con time
	
	move(mainContract, thicknessDelay);
	
	//Step 3 Contract drawer cylinder half way to close compression chamber
	delayMod = 2;
	move(drawerContract, delayMod);
	  
	//Step 4 compression by main cyl with 1 sec
	delayMod = COMPRESS_DELAY;
	move(mainExtend, delayMod);
	
	//Step 5 main Cyl release pressure
	delayMod = COMPRESS_DELAY;
	move(mainExtend, delayMod);
	
	//Step 6 drawer Cyl contracts opening chamber
	move(drawerContract);
	  
	//Step 7 main moves brick up to eject
	move(mainExtend);
	
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
                 
//Checks for excess drift in timing and enters infinte loop if to high stopping machine. Takes two millis time parameters as input.
void faultCheck( unsigned long currentTime, unsigned long prevTime) {
  unsigned long minimum = 0;    //do math
  unsigned long maximum = 0;    //and compare values
  byte drift = 0;               //for timing drift tracking Is a byte to small? Does it need more than 250ms of variation?
            
  minimum = min(currentTime, prevTime);
  maximum = max(currentTime, prevTime);
  drift = maximum - minimum;
  //serialprint out drift var for trouble shooting
  if (drift > MAXDRIFT) {
    while( true ) { //sleep in infinite loop
    }
  }
  return;
}

// Movement function passes value of cylinder direction, optional thickness delay and a delay time for variation required in some steps. Handles all timing internally.
bool move( byte cylinderDirection, word delayMod, byte thicknessDelay) {
  
  unsigned long currentTime = 0;
  unsigned long previousMillis = 0;

  static unsigned long drawerExtTime = 0;
  static unsigned long drawerExtTimePre = 0;   //previous time  

  static unsigned long drawerMidTime = 0;    //time for retraction from removal point to mid point calculated from step 1 then measured and compared at every cycle.
  static unsigned long drawerMidTimePre = 0;    //previous time
	  
//  static unsigned long drawerRetTime = 0;   //measured
//  static unsigned long drawerRetTimePre = 0;    //keep previous time of drawer Cyl Retraction Time to compare to check for  drift

//  static unsigned long mainRetTime = 0;    //
//  static unsigned long mainRetTimePre = 0;    //previous time
  static unsigned long mainCalTime = 0;     //Calculated time for post calibration return of main to user preset

  static unsigned long mainEjcTime = 0;   //time to eject brick
  static unsigned long mainEjcTimePre = 0;    //previous time

  static unsigned long mainCompTime = 0;   //measured
  static unsigned long mainCompTimePre = 0;    //keep running average of main Cyl Extension Time to compare to check for  drift

  static float kAMain = K_A_MAIN;
  static float kADrawer = K_A_DRAWER;

	  
          while (lowPressure() == true) {
              previousMillis = millis();
              digitalWrite(cylinderDirection, HIGH);
            }
            digitalWrite(cylinderDirection, LOW);
            currentTime = millis() - previousMillis;
//            stepTime() = currentTime;
            // maybe another separate function to manage times/steps and calcs?
          //return main cylinder to user set point
          /*
			while ((lowPressure() == true) {
            mainCalTime = mainRetTime * kAMain;
            previousMillis = millis();
            while ((millis() - previousMillis) < mainCalTime) {
              digitalWrite(SOLENOID_UP, HIGH);
            }
            digitalWrite(SOLENOID_UP, LOW);
			  
		  }
		  */
}
		 
