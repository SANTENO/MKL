/*
  TINEMAstepper.h - Library for controlling the NEMA stepper motor with 
  TI stepper control. 
  Created by Sandro Antenori, August 2021.
  Released into the public domain.
*/

#ifndef TINEMAstepper_h
#define TINEMAstepper_h

#include "Arduino.h"



//MyMOTO soll den Motor mithilfe von Schrittangaben und Endmarkern zu Positionen anfahren können.


class MyMOTO{
  boolean mdir;  //Richtung #OPEN HIGH - CLOSE LOW
  boolean hilostep; //step high oder low
                      //long stepcount;
  long usstep;       //erst mal einfach mit vorhandenen Code probieren microseconds per 2 steps cycle (high/low
  int M_M012PIN;     // M2 PIN - Voll oder 1/16 schritt. später evtl zu M1step ändern als Halb zu Viertelschritt
  int M_DIRPIN;      //Direction pin
  int M_DISABLEPIN;  // Chip(NOT)enable pin 
  int M_stepPIN;     
  //unsigned long prevMicros;   //Microseconds counter of last step
  //unsigned long currentMicros; //Microseconds actual
  // long MAXSTEP;

  public:
  MyMOTO(int,int,int, int);
  void stepdrive(boolean);
  //void steptopos(long);  //Funktion sendet Motor auf Position X
  void setstepcount(long);
  long readstepcount();
  long stepcount;
};

#endif