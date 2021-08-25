/*
  Robotracker.cpp - Library for monitoring household robos by a magnetic sensor.
  Created by Sandro Antenori, August 2021.
  Released into the public domain.
  
  Functions:
  Constructor to connect to a hall sensor input
  setpark is the only active influence, that starts, stops and calculate times 
*/

#include "Arduino.h"
#include "Robotracker.h"


Robotracker::Robotracker(int hallsensorpin)
{
    M_PPIN = hallsensorpin;
    pinMode(M_PPIN, INPUT);
    
    //Initialise 
    parked = digitalRead(M_PPIN);
    goneMillis = 0;
    runtimesec_high = 0; 
    runtimesec_last = 0;
}


void Robotracker::setpark(boolean parkstatus)
{
    //when parked (last status) is FALSE and parkstatus (actual) is TRUE --> 
	// then calculate all times
    //if parked is TRUE and parkstatus FALSE --> robo left the scene, set timer

    if ((parked == LOW) && (parkstatus == HIGH))
    {
       if (((millis() - goneMillis)/1000) < 36000){ //nur Zeiten speichern unter 10h
         runtimesec_last = (millis() - goneMillis)/1000;
         if (runtimesec_last > runtimesec_high)
         {
          runtimesec_high = runtimesec_last;
         }
         parked = parkstatus;
       }
       else{ //Zeiten verwerfen
          parked = parkstatus; 
       }
    }

    if ((parked == HIGH) && (parkstatus == LOW))
    {
       goneMillis = millis();     
       parked = parkstatus; 
    }
}

boolean Robotracker::readparked()
{
  return parked;
}

boolean Robotracker::readnewparkpin()
{
  return digitalRead(M_PPIN);
}

long Robotracker::readruntimesec_last()
{
  return runtimesec_last;
}

long Robotracker::readruntimesec_high()
{
  return runtimesec_high;
}

long Robotracker::readruntimeactual()
{
  if (parked == HIGH){
  return 0;  
  }
  else
  {
  return ((millis()- goneMillis)/1000);
    
  }
}