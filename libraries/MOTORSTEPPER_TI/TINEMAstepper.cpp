/*
  TINEMAstepper.cpp - Library for stepper controlmonitoringwith TI.
  Created by Sandro Antenori, August 2021.
  Released into the public domain.
  
  Functions:
  Constructor to connect - only one of the stepping mode pins
  setpark is the only active influence, that starts, stops and calculate times 
*/

#include "Arduino.h"
#include "TINEMAstepper.h"





MyMOTO::MyMOTO(int iSteppingPIN, int idirPIN, int idisablePIN, int irunPIN) //Constructor to connect pins, berechnet Speed von 0-100%
{
  M_M012PIN = iSteppingPIN; //8
  pinMode(M_M012PIN, OUTPUT);
  //digitalWrite(M_M012PIN, LOW);  //Vollschrittmodus
  digitalWrite(M_M012PIN, HIGH);  //Halbschrittmodus
  
  M_DIRPIN = idirPIN; //6;
  pinMode(M_DIRPIN, OUTPUT);
  digitalWrite(M_DIRPIN, LOW);
  
  M_DISABLEPIN = idisablePIN; //9;   // Chip(NOT)enable pin 
  pinMode(M_DISABLEPIN, OUTPUT);
  digitalWrite(M_DISABLEPIN, HIGH);  // standard disable HIGH
  
  M_stepPIN = irunPIN; //7; 
  pinMode(M_stepPIN, OUTPUT);
  
  stepcount = 0;
  mdir = HIGH;   //erst mal richtung open keine Ahnung ob das smart ist.
  usstep = 1000;  //STandard initwert 1000us steps 
  hilostep = LOW;
  //prevMicros = 0;
  //currentMicros = 0;
}
  
void MyMOTO::stepdrive(boolean direx) //, long ln_ussteptime)/Function setzt Motor einen Schritt weiter OHNE DELAY
{
    //currentMicros = micros();
    mdir = direx;
    //usstep = ln_ussteptime;
      // if((currentMicros - prevMicros) >= usstep) 
       //{
        //prevMicros = currentMicros;
        digitalWrite(M_DISABLEPIN, LOW);
        digitalWrite(M_DIRPIN, mdir);
        // fast ohne delay hinbekommen...
        digitalWrite(M_stepPIN, hilostep);
        delayMicroseconds(usstep);
        digitalWrite(M_stepPIN, !hilostep);
        
        //delayMicroseconds(usstep/2);
        //digitalWrite(M_stepPIN, hilostep);
        digitalWrite(M_DISABLEPIN, HIGH);
        
        if (mdir == HIGH) stepcount++;
        if (mdir == LOW) stepcount--;
       //}
}

void MyMOTO::setstepcount(long newstepscount)
{
  stepcount = newstepscount;
}

long MyMOTO::readstepcount()
{
  return stepcount;
}