
#include <Wire.h>
#include <Adafruit_MotorShield.h>
//#include <LiquidCrystal.h>
#include <Servo.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *TurnTable = AFMS.getStepper(200, 2);
Adafruit_DCMotor *LinearActuator = AFMS.getMotor(2);
Servo SlideSpinner;
//LiquidCrystal lcd (2, 3, 4, 5, 6, 7);

//user defined variables
const int UPSTOP = 860;
const int UPSTOPL = 700;
const int DOWNSTOP = 600;
const int SPEEDACTUATOR = 150;
const int SPEEDTABLE = 20;
float SLIDESPINMIN[ ] = {1, 0.5, 0.5, 1, 0.5, 0.5}; //time in minutes in each beaker
const int BILAYERS = 15;
float ACTUATORLENGTH = 5.5;
const int BEAKERS = 6;
const int STEPSIZE = 1;
const int STEPSEARCH = 8;//was 10
const int ROTATION = 531;
const int VARIANCE = 50;
const int laseon = 0;
int MAXBRIGHTNESS = 40;
int pos = 0;

//constants
const int STARTBUTTON = 8;
const int UPBUTTON = 9;
float TABLESPIN[] = {88, 88, 87, 88, 87, 85};
long TIME = 1000L * 60L;
float DIPDEPTH = 1000 * ACTUATORLENGTH;
float SLIDESPIN[BEAKERS];
unsigned long timea = 0;
unsigned long current = 0;

// laser Variables
double lasertime = 60;
int laserpower = 1;

void setup() {
  SlideSpinner.attach(10);
  pinMode (STARTBUTTON, INPUT);
  pinMode (UPBUTTON, INPUT);
  pinMode (3, OUTPUT);
  Serial.begin(9600);
  AFMS.begin();
  TurnTable->setSpeed(SPEEDTABLE);
  LinearActuator->setSpeed(SPEEDACTUATOR);
  for (int i = 0; i < BEAKERS; i++)
  {
    SLIDESPIN[i] = TIME * SLIDESPINMIN[i];
  }
  //lcd.begin(16, 2);


}
void loop() {

  if (digitalRead (STARTBUTTON) == HIGH)
  {
    Serial.println("tone coming");
    tone(5, 4000, 5000);
    SlideSpinner.attach(10);
    SlideSpinner.write(pos);
    delay(3);
    SlideSpinner.detach();
    //home to start place so don't crash into beakers!
    if (analogRead(A1) < UPSTOP)
    {
      while (analogRead(A1) < UPSTOP) {
        LinearActuator->run(BACKWARD);
        while (analogRead(A1) < UPSTOP) {
          //Serial.println(analogRead(A1));
        }
        LinearActuator->run(RELEASE);
        Serial.println(analogRead(A1));
      }

    }
    for (int findstart = 0; findstart <= ROTATION; findstart++)
    {
      TurnTable->step(STEPSIZE, FORWARD, MICROSTEP);
      TurnTable->release();
      Serial.println(analogRead(A0));
      if (analogRead(A0) > MAXBRIGHTNESS)
      {
        MAXBRIGHTNESS = analogRead(A0);
      }
    }
    int THRESHOLD = MAXBRIGHTNESS - VARIANCE;
    Serial.print("Threshold: ");
    Serial.println(THRESHOLD);
    for (int goforward = 0; goforward <= ROTATION; goforward++)
    {
      TurnTable->step(STEPSIZE, FORWARD, MICROSTEP);
      if (analogRead(A0) > THRESHOLD)
      {
        TurnTable->release();
        break;
      }
    }
    SlideSpinner.attach(10);
    //lcd.clear();

    for (int j = 0; j < BILAYERS; j++)
    {
      Serial.print("Bilayer: ");
      Serial.println(j + 1);
      for (int i = 0; i < BEAKERS; i++)
      {

        Serial.print("Beaker: ");
        Serial.println(i + 1);
        //linear actuator move down
        //Serial.println(analogRead(A1));
        while (analogRead(A1) > DOWNSTOP) {
          LinearActuator->run(FORWARD);
          while (analogRead(A1) > DOWNSTOP) {
            // Serial.println(analogRead(A1));
          }
          //Serial.println(analogRead(A1));
          LinearActuator->run(RELEASE);
        }

        timea = millis();
        current = millis();
        if (i + 1 == 1)
        {
          //Check to see if before laser and agitate if needed...
          if (j < laseon)
          {
            while (current - timea < SLIDESPIN[i])
            {
              current = millis();
              for (pos = 0; pos <= 180; pos += 1)
              {
                SlideSpinner.write(pos);
                delay(3);
              }
              for (pos = 180; pos >= 0; pos -= 1)
              {
                SlideSpinner.write(pos);
                delay(3);
              }
            }
          }
          else
          {
          LinearActuator->setSpeed(40);
            //The is the beginning of the laser patterning code....This will hopefully create a track
            double updownlasetime = millis();
            analogWrite(3, 255 * laserpower / 100);
            Serial.println("Laser On");

            while (millis() < (updownlasetime + 1000 * lasertime)) {
              //Move up
              if (analogRead(A1) < UPSTOPL) {
                LinearActuator->run(BACKWARD);
                while (analogRead(A1) < UPSTOPL) {
                  while (analogRead(A1) < UPSTOPL) {
                  }
                 // Serial.println("out of firstup");
                }
                Serial.println("out of secondup");
              }
              //Maybe add a delay here?
              //Move down
              if (analogRead(A1) > DOWNSTOP) {
                LinearActuator->run(FORWARD);
                while (analogRead(A1) > DOWNSTOP) {
                  while (analogRead(A1) > DOWNSTOP) {
                  }
                  //Serial.println("out of firstdown");
                }
                Serial.println("out of seconddown");
              }
            }
            LinearActuator->run(RELEASE);
            Serial.println("Released");
            analogWrite(3, 0);
            Serial.println("Laser Off");

            //This is the end of the laser patterning code.
            Serial.println((SLIDESPIN[i] + (updownlasetime - millis()))/1000/60);
            if ((SLIDESPIN[i] + (updownlasetime - millis())) > 0) {
              delay(SLIDESPIN[i] + (updownlasetime - millis()));
            }
            
          LinearActuator->setSpeed(SPEEDACTUATOR);
          
          }
        }
        else
        {
          //slide spinner
          while (current - timea < SLIDESPIN[i])
          {
            current = millis();
            for (pos = 0; pos <= 180; pos += 1)
            {
              SlideSpinner.write(pos);
              delay(3);
            }
            for (pos = 180; pos >= 0; pos -= 1)
            {
              SlideSpinner.write(pos);
              delay(3);
            }
          }
        }
        //linear actuator move up
        while (analogRead(A1) < UPSTOP) {
          LinearActuator->run(BACKWARD);
          while (analogRead(A1) < UPSTOP) {
            //Serial.println(analogRead(A1));
          }
          LinearActuator->run(RELEASE);
          //Serial.println(analogRead(A1));
        }

        //turn table
        TurnTable->step(TABLESPIN[i], FORWARD, MICROSTEP);
        TurnTable->release();

      }
      TurnTable->release();
      delay(100);
      SlideSpinner.detach();
      boolean foundspot = 0;
      for (int searchforward = 0; searchforward <= STEPSEARCH; searchforward++)
      {
        Serial.println(analogRead(A0));
        if (analogRead(A0) > THRESHOLD)
        {
          TurnTable->release();
          foundspot = 1;
          break;
        }
        TurnTable->step(STEPSIZE, FORWARD, MICROSTEP);
        TurnTable->release();
        // delay(20);
      }
      if (foundspot == 0)
      {
        for (int searchbackward = 0; searchbackward <= 2 + STEPSEARCH ; searchbackward++)
        {
          Serial.println(analogRead(A0));
          if (analogRead(A0) > THRESHOLD)
          {

            TurnTable->release();
            foundspot = 1;
            break;
          }
          TurnTable->step(STEPSIZE, BACKWARD, MICROSTEP);
          //  delay(20);
          TurnTable->release();
        }


      }
      Serial.println("done");
      Serial.println(analogRead(A0));
      if (foundspot == 0)
      {
        while (digitalRead (STARTBUTTON) == LOW)
        {
          Serial.println(analogRead(A0));
          delay(20);
        }
      }
      SlideSpinner.attach(10);
      //MAKE BUZZ SOUND make sure its the right PIN!!!
      Serial.println("tone coming");

      tone(5, 4000, 5000);
      delay(5000);
    }

  }

  //linear actuator button
  if (digitalRead (UPBUTTON) == HIGH)
  {
    Serial.println("Moving up");
    while (analogRead(A1) < UPSTOP) {
      LinearActuator->run(BACKWARD);
      while (analogRead(A1) < UPSTOP) {
        //Serial.println(analogRead(A1));
      }
      LinearActuator->run(RELEASE);
      //Serial.println(analogRead(A1));
    }

    Serial.println("Done Moving up");
  }
}
