/*
  Robotracker.h - Library for monitoring household robos by a magnetic sensor.
  Created by Sandro Antenori, August 2021.
  Released into the public domain.
*/
#ifndef Robotracker_h
#define Robotracker_h

#include "Arduino.h"


class Robotracker{
  int M_PPIN; //magnetic park pin 
  boolean parked;  //last read parked state
  unsigned long goneMillis;
  long runtimesec_high;
  long runtimesec_last;

  public:
  Robotracker(int);
  void setpark(boolean);
  boolean readparked();
  boolean readnewparkpin();
  long readruntimeactual();
  long readruntimesec_last();
  long readruntimesec_high(); 
};
#endif