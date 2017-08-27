/* Open Source Ecology CEB Press v16.09 v8 Teensy Microcontroller code for Auto mode operation
  Switches FET's HIGH/LOW to control two 3 position hydraulic solenoids,
  measures piston motion time relative to pressure sensor trigger,
  and repeats cycle while auto calibrating timing from previous cycles and startup positions.

  Compensates for difference in time for Extension and Contraction of Rods.
  T_extend = T_contract  * (A_cyl - A_rod) / A_cyl)

  Faults require manual user intervention to reset to starting position if faults occur and power must be shujt off to controller by engaging OFF/MANUAL mode(s).

  Contributions by:
  Abe Anderson
  
  Unfamiliar with code structures? See https://www.arduino.cc/en/Reference/HomePage
  
  

  License:
  See GPLv3 license file in repo.
*/


//defines to make it easier for non coders to make adjustments for troubleshooting and custom changes

#define SOLENOID_RIGHT 5   //swap these pin numbers for wire inversion      (deafult pin 5)
#define SOLENOID_LEFT 4    //    (default pin 4)

#define SOLENOID_DOWN 15    //swap these pin numbers for wire inversion   (default pin 15)
#define SOLENOID_UP 14      //    (default pin 14)

#define PRESSURE_SENSOR 41  //labeled A3 or F3 silkscreen on the PCB

#define SWITCH_DEBOUNCE 3 //milliseconds to delay for switch debounce
#define PRESSURE_SENSOR_DEBOUNCE 20 //milliseconds to delay for pressure sensor debounce
#define COMPRESS_DELAY 500  // 1/2 sec extra to compress brick via main Cyl (default 500ms)
#define RELEASE_PRESSURE_DELAY 100    //releases pressure from the drawer bottom after compression (default 100ms)
#define K_A_MAIN 0.004  // T_e = T_c * (k_A)   for 1.25in x14in cylinder  (default 0.004)
#define K_A_DRAWER 0.008 // T_e = T_c * (k_A)  for 2.75in x10in cylinder  (default 0.008)
#define MAXDRIFT 100    //Sets maximum time difference in milliseconds from one cycle to the next for all steps to check for faults (default 100ms)

// custom structures, function declarations or prototypes
bool lowPressure();    //function to read if pressure sensor is HIGH
void faultCheck( unsigned long currentTime, unsigned long prevTime);
bool move( byte cylinderDirection, word delayTime);


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

  //Step 1 Initialize  to start position retract main cyl for 1 sec to release pressue contract drawer cyl fully
  digitalWrite(SOLENOID_DOWN, HIGH);
  delay(1000);
  digitalWrite(SOLENOID_DOWN, LOW);
  while ((lowPressure() == true) {
    digitalWrite(SOLENOID_RIGHT, HIGH);
    }
         digitalWrite(SOLENOID_RIGHT, LOW);
}
         
   //Create initial button polling routine for a several seconds to check for thickness setting option?
   //Indicator lights would be useful for button selection.
   //Poll for button press(es) debounce and count. Need to write, copy, and/or point to var for use in loop?

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
	word delayTime = 0;

	  
      //Step 2 Calibration Extend drawer Cyl Fully and measure T_ret at Presure sensor high

       
            //Retraction drawer Cyl RIGHT measure T_ret at Presure sensor high
	  
	  /*
	  Is it useful to write a function that takes parameters to specify cylinder, direction, HIGH/LOW, and millis duration values?
	  Would require making vars for many current defines.
	  Could pass move time and return new Time.
	  4 bytes for solenoid directions
	  bool for LOW/HIGH
	  
	  */
	  
	  
            while ((lowPressure() == true) {
              previousMillis = millis();
              digitalWrite(SOLENOID_RIGHT, HIGH);
            }
            digitalWrite(SOLENOID_RIGHT, LOW);
            drawerRetTime = millis() - previousMillis;

            if (drawerExtTimePre == 0) {
              drawerExtTimePre = drawerExtTime;
            }
            else {
              if (drawerExtTime != drawerExtTimePre) {
                faultCheck( unsigned long drawerExtTime, unsigned long drawerExtTimePre);
                }
              }
            }
          }
          drawerRetTimePre = drawerRetTime;
        }

      //Step 2 Ejection by extending main cyl UP until pressure sensor high measure T_ext

          while ((lowPressure() == true) {
            previousMillis = millis();
            digitalWrite(SOLENOID_UP, HIGH);
          }
          digitalWrite(SOLENOID_UP, LOW);
          mainEjcTime = millis() - previousMillis;

          if (mainEjcTimePre == 0) {
            mainEjcTimePre = mainEjcTime;
          }
          else {
            if (mainEjcTime != mainEjcTimePre) {
              faultCheck( unsigned long currentTime, unsigned long prevTime);
              }
            }
          }
          mainEjcTimePre = mainEjcTime;

      //Step 3 Brick Removal 2nd Cyl extended LEFT until Presure sensor high

          while ((lowPressure() == true) {
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
             faultCheck( unsigned long currentTime, unsigned long prevTime);
              }
            }
          }
          drawerExtTimePre = drawerExtTime;

      //Step 4 Soil Load main Cyl moves DOWN/retracts and soil enters chamber

          while (lowPressure() == true) {
            previousMillis = millis();
            while ((millis() - previousMillis) <= mainRetTime) {
              digitalWrite(SOLENOID_DOWN, HIGH);
            }
            digitalWrite(SOLENOID_DOWN, LOW);
            mainRetTime = millis() - previousMillis;
          }

          if (mainRetTimePre == 0) {
            mainRetTimePre = mainRetTime;
          }
          else {
            if (mainRetTime != mainRetTimePre) {
            faultCheck( unsigned long currentTime, unsigned long prevTime);
              }
            }
          }
          mainRetTimePre = mainRetTime;

      //Step 5 Chamber/Drawer Closure drawer retraction time to midpoint is calculated from initial full contraction from the midpoint (step 1 measurement)
          while (lowPressure() == true) 
            drawerMidTime = drawerExtTime / kADrawer ;
            previousMillis = millis();
            while ((millis() - previousMillis) <= drawerMidTime) {
              digitalWrite(SOLENOID_RIGHT, HIGH);
            }
            digitalWrite(SOLENOID_RIGHT, LOW);
          }
          if ( drawerMidTimePre == 0) {
            drawerMidTimePre =  drawerMidTime;
          }
          else {
            if ( drawerMidTime !=  drawerMidTimePre) {
            faultCheck( unsigned long currentTime, unsigned long prevTime);
              }
            }
          }
          drawerMidTimePre =  drawerMidTime;

      //Step 6 Brick Pressing Main Cyl moves to T_ext + 1/2 sec compression delay and pressure release
  
          while (lowPressure() == true) {
            previousMillis = millis();
            digitalWrite(SOLENOID_UP, HIGH);
          }
          previousMillis = millis() - previousMillis;
          mainCompTime = previousMillis;
          delay(COMPRESS_DELAY);
          digitalWrite(SOLENOID_UP, LOW);

          //release pressure from drawer
          digitalWrite(SOLENOID_DOWN, HIGH);
          delay(RELEASE_PRESSURE_DELAY);
          digitalWrite(SOLENOID_DOWN, LOW);

          if ( mainCompTimePre == 0) {
            mainCompTimePre =  mainCompTime;
          }
          else {
            if ( mainCompTime !=  mainCompTimePre) {
              faultCheck( unsigned long currentTime, unsigned long prevTime);
              }
            }
          }
          mainCompTimePre =  mainCompTime;
    }
  }
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

// Movement function passed value of cylinder direction and a delay time for variation required in some steps. Handles all timing internally.
bool move( byte cylinderDirection, word delayTime)
  
  unsigned long previousMillis = 0;

  static unsigned long drawerExtTime = 0;
  static unsigned long drawerExtTimePre = 0;   //previous time  

  static unsigned long drawerMidTime = 0;    //time for retraction from removal point to mid point calculated from step 1 then measured and compared at every cycle.
  static unsigned long drawerMidTimePre = 0;    //previous time
	  
  static unsigned long drawerRetTime = 0;   //measured
  static unsigned long drawerRetTimePre = 0;    //keep previous time of drawer Cyl Retraction Time to compare to check for  drift

  static unsigned long mainRetTime = 0;    //
  static unsigned long mainRetTimePre = 0;    //previous time
  static unsigned long mainCalTime = 0;     //Calculated time for post calibration return of main to user preset

  static unsigned long mainEjcTime = 0;   //time to eject brick
  static unsigned long mainEjcTimePre = 0;    //previous time

  static unsigned long mainCompTime = 0;   //measured
  static unsigned long mainCompTimePre = 0;    //keep running average of main Cyl Extension Time to compare to check for  drift

  static float kAMain = K_A_MAIN;
  static float kADrawer = K_A_DRAWER;

 
				 
          while (lowPressure() == true) {
              previousMillis = millis();
              digitalWrite(SOLENOID_DOWN, HIGH);
            }
            digitalWrite(SOLENOID_DOWN, LOW);
            mainRetTime = millis() - previousMillis;
            mainRetTimePre = mainRetTime;
            
          //return main cylinder to user set point
          while ((lowPressure() == true) {
            mainCalTime = mainRetTime * kAMain;
            previousMillis = millis();
            while ((millis() - previousMillis) < mainCalTime) {
              digitalWrite(SOLENOID_UP, HIGH);
            }
            digitalWrite(SOLENOID_UP, LOW);





*/
				 
