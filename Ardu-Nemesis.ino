#include <Servo.h>





Servo beltESC;

//declare servo objects that will be used throughout the rest of the project.
// constants won't change. They're used here to set pin numbers:

const int ledPin = 13;      // the number of the LED pin

//define pins for Digital switches
const int revSw = 12;
const int feedSw = 11;

//define pins for ESCs
const int beltPin = 6;
const int flywheelPin = 5;
int spinAverage = -10;
int feedAverage = -10;





const int feedESCMinSpeed = 1000;   //this varies by ESC.  Min speed must be low enough to arm.  Refer to speec controller's documentation for this
const int feedESCMaxSpeed = 2000;   //this varies by ESC.  Max speed here is defined seperately to allow for controlling fire rates.  Refer to speec controller's documentation for this


//variables to determine if we need to make changes to esc spinning states.  Eliminates unnecessary servo.writemicrosecond() calls
bool spinchange = false;
bool feedchange = false;
bool ragechange = false;

// Variables will change:
int ledState = LOW;         // the current state of the output pin
int spinButtonState;             // the current reading from the spin input pin
int feedButtonState;            // the current reading from the feed input pin

//track if the flywheels are currently spinning.  Used to prevent pusher from being run when flywheels are stationary
bool spinning = false;
//track if pusher is running
bool pushing = false;
//track if there has a request for the pusher to be retracted.
bool retract = false;



void setup() {
  //lcd.begin(16, 2); // initialize the lcd
  pinMode(ledPin, OUTPUT);
  pinMode(flywheelPin, OUTPUT);
  pinMode(revSw, INPUT_PULLUP);
  pinMode(feedSw, INPUT_PULLUP);


  // set initial LED state
  digitalWrite(ledPin, ledState);

  beltESC.attach(beltPin, feedESCMinSpeed, feedESCMaxSpeed);

  //write minimum spped to ESCs so they arm
  beltESC.write(feedESCMinSpeed);
  delay(1000);                  // waits for a second while the ESCs arm

}

void loop() {
  // read the state of the switch into a local variable:
  int spinReading = digitalRead(revSw);
  int feedReading = digitalRead(feedSw);


  //Rev Trigger Handling
  if (spinReading == HIGH) {
    if (spinAverage < 9) {
      spinAverage++;
      spinAverage++; //we are incrementing spinAverage twice here since it is more important to stop the action quickly than start it quickly for safety sake
      spinchange = true; //set the spin change flag so that we can evaluate if the flywheels need to be stopped

    }


  } else {
    if (spinAverage > -10) {
      spinAverage--;
      spinchange = true; //set the spin change flag so that we can evaluate if the flywheels need to be started
    }


  }


  //Feed Trigger Handling

  if (feedReading == HIGH) {
    if (feedAverage < 9) {
      feedAverage++;
      feedAverage++; //we are incrementing feedAverage twice here since it is more important to stop the action quickly than start it quickly for safety sake
      feedchange = true; //set the feed change flag so that we can evaluate if the pusher needs to be started
    }
  } else {
    if (feedAverage > -10) {
      feedAverage--;
      feedchange = true; //set the feed change flag so that we can evaluate if the pusher needs to be started
    }
  }




  if (spinchange == true) {
    if (spinAverage < 0) {
      digitalWrite(flywheelPin, HIGH);
      spinning = true;
      spinchange = false; //reset spin change flag since the change has been processed
      ledState = HIGH;;
    } else {
      digitalWrite(flywheelPin, LOW);
      spinning = false; //set spin state to false.
      spinchange = false; //reset spin change flag since the change has been processed
      ledState = LOW;

    }
  }



  if (spinning == true) { //check that flywheels are running before allowing pusher to engage.
    if (feedchange == true) { //only update pusher speed if the trigger state has changed
      if (feedAverage < 0) {
        //ledState = HIGH;
        beltESC.write(feedESCMaxSpeed);
        feedchange = false; //reset feed change flag since the change has been processed
        pushing = true;
       } else {
        //stop the pusher
        beltESC.write(feedESCMinSpeed);
        //ledState = LOW;
        feedchange = false; //reset feed change flag since the change has been processed
        pushing = false;
      }
    }
  }
  if (!spinning && pushing ) { //pusher is running and the flywheels are not.  THIS IS BAD! stop the pusher
    beltESC.write(feedESCMinSpeed);
    ledState = LOW;
    pushing = false;
  }



  // set the LED state:
  digitalWrite(ledPin, ledState);
  //write the neopixels


}
