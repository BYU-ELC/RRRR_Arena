#define MAX_NUM_PENALTIES 3
#define TOTAL_NUM_BUTTONS 6
#define INTERVAL 10000
#define DEBOUNCE 2000

#include <LiquidCrystal_I2C.h>

enum State {
  state_idle,
  state_running,
  state_activateButton,
  state_gameOver,
  state_gameFinished,
} curr_state, prev_state;

unsigned long startTime;           // time the game started
unsigned long timePassed;          // time passed in the game
unsigned long timePassedSeconds;   // for printing time in seconds
unsigned long prevTimePassedSeconds;      // this variable is only used for comparison with timePassed to see if we need to refresh the screen
unsigned long currentButtonTime;   // time for this specific button in milliseconds
unsigned long currentButtonTimeSeconds;
int           numPenalties;        // how many penalties were incurred as of right now?
int           buttonsRunningTotal; // keep track of how many "chances" there were to press a button
long          randomNumber;        // For choosing a light 1 - 4 to light up
int           pointsEarned;      // Number of buttons successfully pressed

// Code for Each Physical Button and Color Spectrum LED
int button1Pin = 2;
int button2Pin = 3;
int button3Pin = 4;
int button4Pin = 5;
int buttonStart = 6; // this button functions to both start the game from idle mode, and to return to idle mode from end mode

bool button1State; // will tell us if button high or low
long button1_timeHigh; // will be used for debouncing: how long since it changed state to being pressed?

bool button2State; // will tell us if button high or low
long button2_timeHigh; // will be used for debouncing: how long since it changed state to being pressed?

bool button3State; // will tell us if button high or low
long button3_timeHigh; // will be used for debouncing: how long since it changed state to being pressed?

bool button4State; // will tell us if button high or low
long button4_timeHigh; // will be used for debouncing: how long since it changed state to being pressed?

int led1Pin = 8;
int led2Pin = 9;
int led3Pin = 10;
int led4Pin = 11;

// Code for Initializing the Display
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 2); // can change to 0x27, 20, 4 for 4 row LCD

void setup() {

  // Initialize the PRNG
  // if analog input pin 0 is left unconnected, random analog noise will call to randomSeed() to generate different seed numbers each time the sketch runs
  randomSeed(analogRead(0));

  // Initialize State Machine State Variables
  prev_state = state_gameOver;
  curr_state = state_idle;


  // Initialize Buttons
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
  pinMode(button4Pin, INPUT);
  pinMode(buttonStart, INPUT);

  // Initialize LEDs
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  pinMode(led4Pin, OUTPUT);
  digitalWrite(led1Pin, LOW);
  digitalWrite(led2Pin, LOW);
  digitalWrite(led3Pin, LOW);
  digitalWrite(led4Pin, LOW);

  // Begin Serial for Debugging
  Serial.begin(9600);

  //Initiate LCD Display:
  lcd.init();
  lcd.backlight();

  //Set time elapsed to 0 to start the game...
  timePassed     = 0;
  prevTimePassedSeconds = 0;
}

void loop() {
  switch(curr_state) {

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------    

    case state_idle:
      // This If Statement is ONLY for making sure our serial display (debugging) isn't flooded with writes
      if (prev_state != curr_state) {
        Serial.println("state: idle");
        prev_state = curr_state;

        //Display update code belongs here to prevent constant refresh
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Press * to Start");
      }
      
      if(digitalRead(buttonStart) == HIGH) { // if button 1 is being pressed then start!
        curr_state = state_activateButton;
        
        // Transition code
        startTime               = millis();
        prevTimePassedSeconds   = 0;
        numPenalties            = 0;
        buttonsRunningTotal     = 0;
        pointsEarned            = 0;

        //New game started, therefore initialize all time since previous button pressed to "0" (see millis() and debounce behavior)
        button1_timeHigh = 0;
        button2_timeHigh = 0;
        button3_timeHigh = 0;
        button4_timeHigh = 0;
        
        lcd.clear();
        break;
      }
      break;
    
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------     
    
    case state_activateButton:
      // This If Statement is ONLY for making sure our serial display (debugging) isn't flooded with writes
      if (prev_state != curr_state) {
        Serial.println("state: activate button");
        prev_state = curr_state;
      }

      if (buttonsRunningTotal >= TOTAL_NUM_BUTTONS) {
        curr_state = state_gameFinished;
        break;
      }
    
      randomNumber = random(4) + 1;
//      lcd.setCursor(0,0);
//      lcd.print(randomNumber);     
//      lcd.setCursor(0,1);
//      lcd.print(numPenalties);
      switch(randomNumber) {
        case 1:
          Serial.println("\tTurning LED 1 On");
          digitalWrite(led1Pin, HIGH);
          break;
        case 2:
          Serial.println("\tTurning LED 2 On");
          digitalWrite(led2Pin, HIGH);
          break;
        case 3:
          Serial.println("\tTurning LED 3 On");
          digitalWrite(led3Pin, HIGH);
          break;
        case 4:
          Serial.println("\tTurning LED 4 On");
          digitalWrite(led4Pin, HIGH);
          break;
      }
      currentButtonTime = millis(); // this is a generic variable to track how long whatever button it was has been on
      
      curr_state = state_running;
      break;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
      
    case state_running:
      // This If Statement is ONLY for making sure our serial display (debugging) isn't flooded with writes
      if (prev_state != curr_state) {
        Serial.println("state: running");
        prev_state = curr_state;
      }
      /* Check for the following in the following order:
       *  Reset Pressed? --> to idle state
       *  Time Passed? (false) --> Button Pressed (true) --> Max penalties Reached
       *    Button Pressed? (false) --> dont change anything pass through the state machine again OR (true) --> Increment num of buttons pressed and go back to activate button
       *    Max Penalties Reached? (false) --> increment number of penalties and move into activateButton state OR (true) --> move into gameOver state
       */

       // Write the current total running time to the LCD
       timePassed = millis() - startTime;
       timePassedSeconds = (timePassed / 1000);
       if (timePassedSeconds != prevTimePassedSeconds) {
         lcd.setCursor(10,0);
         lcd.print("TT:");
         lcd.print(timePassedSeconds);
       }


      if((millis() - currentButtonTime) > INTERVAL) {
        numPenalties++;
        buttonsRunningTotal++;
        lcd.setCursor(0, 1);
        lcd.print("PEN:");
        lcd.print(numPenalties);
        Serial.println("\tTime Passed and button not pressed...");
        switch(randomNumber) {
          case 1:
            Serial.println("\tTurning LED Off");
            digitalWrite(led1Pin, LOW);
            break;
          case 2:
            Serial.println("\tTurning LED Off");
            digitalWrite(led2Pin, LOW);
            break;
          case 3:
            Serial.println("\tTurning LED Off");
            digitalWrite(led3Pin, LOW);
            break;
          case 4:
            Serial.println("\tTurning LED Off");
            digitalWrite(led4Pin, LOW);
            break;
        }
        if (numPenalties >= MAX_NUM_PENALTIES) {
          curr_state = state_gameOver; 
        } else {
          curr_state = state_activateButton;
        }
        break;
      }

      button1State = digitalRead(button1Pin);
      //Debouncing
      if (button1State == HIGH) {                        // if the button reads high
        if ((millis() - button1_timeHigh) <= DEBOUNCE) { // if less than DEBOUNCE time has passed since we last triggered "high"
          button1State = LOW;                            // ignore the press for this iteration around
        }
      }
      
      button2State = digitalRead(button2Pin);
      //Debouncing
      if (button2State == HIGH) {                        // if the button reads high
        if ((millis() - button2_timeHigh) <= DEBOUNCE) { // if less than DEBOUNCE time has passed since we last triggered "high"
          button2State = LOW;                            // ignore the press for this iteration around
        }
      }
      
      button3State = digitalRead(button3Pin);
      //Debouncing
      if (button3State == HIGH) {                        // if the button reads high
        if ((millis() - button3_timeHigh) <= DEBOUNCE) { // if less than DEBOUNCE time has passed since we last triggered "high"                   
          button3State = LOW;                            // ignore the press for this iteration around
        }
      }
      
      button4State = digitalRead(button4Pin);
      //Debouncing
      if (button4State == HIGH) {                        // if the button reads high
        if ((millis() - button4_timeHigh) <= DEBOUNCE) { // if less than DEBOUNCE time has passed since we last triggered "high"
          button4State = LOW;                            // ignore the press for this iteration around
        }
      }

      if (button1State == HIGH) {
        button1_timeHigh = millis();
        if(randomNumber == 1) {
          // They pressed the right button, so reward them
          Serial.println("\tButton 1 Pressed... Point Earned");
          buttonsRunningTotal++;
          pointsEarned++;
          // Write the number of buttons out ouf 6 to the LCD;
          lcd.setCursor(0, 0);
          lcd.print("PTS:");
          lcd.print(pointsEarned);
          
          Serial.println("\tTurning LED 1 Off");
          digitalWrite(led1Pin, LOW);
          curr_state = state_activateButton;
          break;
        } else {
          // They pressed the wrong button, so penalize them
          numPenalties++;
          Serial.println("\tButton 1 Pressed Incorrectly... Incrementing Penalties");
          lcd.setCursor(0, 1);
          lcd.print("PEN:");
          lcd.print(numPenalties);
          if (numPenalties >= MAX_NUM_PENALTIES) {
            curr_state = state_gameOver;
            break;
          }
        }
      }

      if (button2State == HIGH) {
        button2_timeHigh = millis();
        if(randomNumber == 2) {
          // They pressed the right button, so reward them
          Serial.println("\tButton 2 Pressed... Point Earned");
          buttonsRunningTotal++;
          pointsEarned++;
          // Write the number of buttons out ouf 6 to the LCD;
          lcd.setCursor(0, 0);
          lcd.print("PTS:");
          lcd.print(pointsEarned);
          
          Serial.println("\tTurning LED 2 Off");
          digitalWrite(led2Pin, LOW);        
          curr_state = state_activateButton;
          break;      
        } else {
          // They pressed the wrong button, so penalize them
          numPenalties++;
          Serial.println("\tButton 2 Pressed Incorrectly... Incrementing Penalties");
          lcd.setCursor(0, 1);
          lcd.print("PEN:");
          lcd.print(numPenalties);
          if (numPenalties >= MAX_NUM_PENALTIES) {
            curr_state = state_gameOver;
            break;
          }
        }
      }

      if (button3State == HIGH) {
        button3_timeHigh = millis();
        if(randomNumber == 3) {
          // They pressed the right button, so reward them
          Serial.println("\tButton 3 Pressed... Point Earned");
          buttonsRunningTotal++;
          pointsEarned++;
          // Write the number of buttons out ouf 6 to the LCD;
          lcd.setCursor(0, 0);
          lcd.print("PTS:");
          lcd.print(pointsEarned);
 
          Serial.println("\tTurning LED 3 Off");
          digitalWrite(led3Pin, LOW);
          curr_state = state_activateButton;
          break;          
        } else {
          // They pressed the wrong button, so penalize them
          numPenalties++;
          Serial.println("\tButton 3 Pressed Incorrectly... Incrementing Penalties");
          lcd.setCursor(0, 1);
          lcd.print("PEN:");
          lcd.print(numPenalties);
          if (numPenalties >= MAX_NUM_PENALTIES) {
            curr_state = state_gameOver;
            break;
          }
        }
      }

      if (button4State == HIGH) {
        button4_timeHigh = millis();
        if(randomNumber == 4) {
          // They pressed the right button, so reward them
          Serial.println("\tButton 4 Pressed... Point Earned");
          buttonsRunningTotal++;
          pointsEarned++;
          // Write the number of buttons out ouf 6 to the LCD;
          lcd.setCursor(0, 0);
          lcd.print("PTS:");
          lcd.print(pointsEarned);

          Serial.println("\tTurning LED 4 Off");
          digitalWrite(led4Pin, LOW);
          curr_state = state_activateButton;
          break;
        } else {
          // They pressed the wrong button, so penalize them
          numPenalties++;
          Serial.println("\tButton 4 Pressed Incorrectly... Incrementing Penalties");
          lcd.setCursor(0, 1);
          lcd.print("PEN:");
          lcd.print(numPenalties);
          if (numPenalties >= MAX_NUM_PENALTIES) {
            curr_state = state_gameOver;
            break;
          }
        }
      }
      
      break;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
      
    case state_gameOver:
      // This If Statement is ONLY for making sure our serial display (debugging) isn't flooded with writes
      if (prev_state != curr_state) {
        Serial.println("state: game over");
        prev_state = curr_state;

        //Display update code belongs here to prevent constant refresh
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("GAME OVER!");
        delay(500);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Score: ");
        lcd.print(pointsEarned);
        lcd.setCursor(10,0);
        lcd.print("Pen: ");
        lcd.print(numPenalties);
        lcd.setCursor(0,1);
        lcd.print("Time: ");
        lcd.print(timePassed); // total time to run the game   
      }
      
      if (digitalRead(buttonStart) == HIGH) {
        curr_state = state_idle;
        break;
      }

      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, LOW);
      digitalWrite(led3Pin, LOW);
      digitalWrite(led4Pin, LOW);
      
      break;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
      
    case state_gameFinished:
      // This If Statement is ONLY for making sure our serial display (debugging) isn't flooded with writes
      if (prev_state != curr_state) {
        Serial.println("state: game completed");
        prev_state = curr_state;

        //Display update code belongs here to prevent constant refresh
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("GAME COMPLETE!");
        delay(500);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Score: ");
        lcd.print(pointsEarned);
        lcd.setCursor(10,0);
        lcd.print("Pen: ");
        lcd.print(numPenalties);
        lcd.setCursor(0,1);
        lcd.print("Time: ");
        lcd.print(timePassed); // total time to run the game        
      }

      if (digitalRead(buttonStart) == HIGH) {
        curr_state = state_idle;
        break;
      }

      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, LOW);
      digitalWrite(led3Pin, LOW);
      digitalWrite(led4Pin, LOW);
      
      break;
  }

  // to prevent the display from refreshing so often, let's update a variable called previous number of penalties to see if we need to refresh the screen the next time through the loop
  prevTimePassedSeconds = timePassedSeconds;
}
