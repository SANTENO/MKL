#include <Robotracker.h>

/* MKL Motorklappe
 * Function: 
 Opens and closes a door or a lid of a furniture to hide household
 cleaning robots. This door open and close function is protected by a
 power measurement to prevent harm on things, people and animals.
 The MKL program also monitors the time those robots are outside for
 the actual current time, the last time and a "highscore".
 Times above 10 hours are ignored. 
 A display control, button up and down control and a FIRE button are the
 user interface.
 Author: Sandro Antenori
  
31.12.2021 --> GITHUB inclusion
 */



#include "Wire.h"               //I2C interface 
#include "Adafruit_SSD1306.h"   //OLED DISPLAY - quite large 
#include "HX711.h"              //loadcell control
#include "Robotracker.h"        //own library
#include "TINEMAstepper.h"      //also own library TI control for NEMA stepper motor.

//function (pre)declaration
char * sint (char * timetext, long t); //a conversion from time in seconds to xmxx or xhxx.
void homeclose();                      //the initialization sequence
void showDisplay();                    // a refresh on the display in one function.


//Endschalter 
#define SWTCH_HOME A2  ///homing switch - CLOSE END wird homing..

//man Schalter
#define BTN_UP 2
#define BTN_DOWN 3
#define REL1 5
#define REL2 11
#define BTN_FIRE 10

#define OPEN HIGH
#define CLOSE LOW
#define MAXSTEP 10200

boolean manualmode = LOW;
boolean robigone = LOW;
boolean setfire = LOW;    //Status for launch of vaccum robot
int count = 0;
const int LOADCELL_DOUT_PIN = 12;   //weiss
const int LOADCELL_SCK_PIN = 13;    //blau
const long LOADCELL_OFFSET = 20000;
const long LOADCELL_DIVIDER = 500;
const long MAX_PWR = 2000;
const long BTNTIME = 200;
//const long MAXTIME = 35999; //10h Maximale Zeit als Indikator - 9:59:59

char Sauglast[]="0m00";
char Saugact[]="0m00";
char Saughigh[]="0m00";
char Wischlast[]="0m00";
char Wischact[]="0m00";
char Wischhigh[]="0m00";


int iteration = 100;
long reading=0;
unsigned long prevMicros =0;   //Microseconds counter of last step
unsigned long currentMicros=0; //Microseconds actual
int countdown = 120;
unsigned long prevMillis_hx711 = 0;
unsigned long curMillis = 0;
long mysteps = 0;
int Maschinenstatus = 1;

// =====Objects ======== 

HX711 loadcell;
Adafruit_SSD1306 display(4);

MyMOTO MMklappe1(8,6,9,7);  
Robotracker robiSaug(A0);
Robotracker robiWisch(A1);


void setup()   {
  Serial.begin(38400);
  // setup end switches
  pinMode(SWTCH_HOME, INPUT);        //Endschalter Hall Sensor, hat Pullup intern
  // setup manual buttons
  pinMode(BTN_UP, INPUT_PULLUP);    // großer Schalter hoch
  pinMode(BTN_DOWN, INPUT_PULLUP);  //großer Schalter runter
                                    //andere Schalter sind Teil von myRobo..
  pinMode(BTN_FIRE, INPUT_PULLUP);  //roter Taster neben Display
  pinMode(REL1, OUTPUT);            //REL1 enables Power supply for stepper motors and solenoid source
  pinMode(REL2, OUTPUT);            //REL2 is the switch for the solenoid to turn on (needs REL1 for power)

  digitalWrite(REL1,LOW);           //zuerst einschalten, hoomeclose geht ja gleich los..!
  digitalWrite(REL2,HIGH);          //REL2 SOLENOID FInger OFF
 
  // setup Display ======= WELCOME screen ============
  display.begin(SSD1306_SWITCHCAPVCC, 60); 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30,10);
  display.println("MKL V2.1"); 
  display.display();  
  delay(100);

  //Waagebalken initialisieren...
  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_scale(LOADCELL_DIVIDER);
  loadcell.set_offset(LOADCELL_OFFSET);

  //setup Klappe schliessen zuerst, Display, Waage und Schritte nullen
  homeclose();
  loadcell.tare();
  mysteps = 0;   //Nullstellung
  MMklappe1.setstepcount(0); 
  //Voila - alles initialisiert, Klappe geschlossen und Kalibriert.
  Maschinenstatus = 2; 
}

void homeclose(){
  long usstep = 1000;
  while (digitalRead(SWTCH_HOME) == 1){ //Homing switch geht von aktiv auf low bei Erkennung
    currentMicros = micros();
    if((currentMicros - prevMicros) >= usstep) 
     {
     prevMicros = currentMicros;
     MMklappe1.stepdrive(CLOSE);    
     mysteps = MMklappe1.readstepcount(); 
     }
  }
}

//besorgt aktuelle Daten und gibt Sie auf dem Display aus.
void showDisplay(){
       display.clearDisplay();
       display.setCursor(1,4);
       display.print("M");
       display.print(Maschinenstatus);
       if (manualmode == HIGH) {
         display.print("m!");
       }
       else{
         if ((countdown < 120) && (countdown >0)){
            display.drawRect(8,56, 61, 6, SSD1306_WHITE);
            display.fillRect(9,57, countdown/2, 4, SSD1306_INVERSE);
         } 
       }
       display.print(" ");
       display.print(mysteps); 
       display.print("|");
       display.println(reading);
       
       // ====SAUGrobbi=====:
       display.setCursor(1,15); 
       display.print("Saugi: ");
       if (robiSaug.readparked()==0){
          sint(Saugact, robiSaug.readruntimeactual());
          display.println(Saugact);
          display.drawCircle((46 + display.width()/2), (display.height()/2)-9, 13, SSD1306_WHITE);
       }
       else{
          display.println(" ");
          display.fillCircle((46 + display.width()/2), (display.height()/2)-9, 13, SSD1306_INVERSE);
       }
       display.print("l:");
       display.print(Sauglast);
       display.print(" h:");
       display.println(Saughigh);
       
       
       //====Wischrobbi========:
       display.setCursor(1,36); 
       display.print("Consuela:");
       if (robiWisch.readparked()==0){
          sint(Wischact,robiWisch.readruntimeactual());
          display.println(Wischact);
          display.drawRoundRect((36 + display.width()/2),(display.height()/2)+12, 20, 20,3, SSD1306_WHITE);
       }
       else{
          display.println(" ");
          display.fillRoundRect((36 + display.width()/2),(display.height()/2)+12, 20, 20,3, SSD1306_INVERSE);
       }
       display.print("l:");  display.print(Wischlast);
       display.print(" h:"); display.println(Wischhigh);       
       display.display();
}



void loop() {
  curMillis = millis();  
  switch (Maschinenstatus){  
  case 2: //Klappe zu - Display zeigen- TODO
    digitalWrite(REL1,HIGH);  // Power Supply ausschalten
    reading = 3000 + loadcell.get_units(4);
    showDisplay();
    //delay(50); //blockiert eh eine Zeit, sollte reichen um hier nicht zu schnell zu sein
    while (digitalRead(BTN_UP) == 0){
        Maschinenstatus = 3;
        manualmode = HIGH;  
        //loadcell.power_up();
        digitalWrite(REL1,LOW);  // Power Supply einschalten
        delay(BTNTIME);
    }
    if (digitalRead(BTN_FIRE) == 0){
        Maschinenstatus = 9;
        break;
    }
    if (robigone == HIGH){
      Maschinenstatus = 3; //Trigger falls ein Robi wegfährt
      digitalWrite(REL1,LOW);  // Power Supply einschalten
      delay(BTNTIME);
    }
    countdown = 120; // erst nach dem vollstaendigen Schliessen wird Countdown wieder ermoeglicht
    break;
  case 3: //OPENING
     mysteps = MMklappe1.readstepcount();
     while (digitalRead(BTN_DOWN) == 0){ //Manuell entgegengedrueckt gegen oeffnen
       Maschinenstatus=6;
       break;
     }
     if ((iteration  + mysteps) > MAXSTEP){
         iteration = MAXSTEP - mysteps;
     }
     for (int i=0; i < iteration; i++){
      delayMicroseconds(500);
      MMklappe1.stepdrive(OPEN);
     } 
     mysteps = MMklappe1.readstepcount(); 
     if ((curMillis - prevMillis_hx711) >= 100) {
      prevMillis_hx711 = curMillis;
        reading = 3000 + loadcell.get_units(2);
        //Serial.print(mysteps, DEC);
        //Serial.print("\t");
        //Serial.println(reading, DEC);
      }
      if (reading < MAX_PWR){  // umgekehrte Logik, reading geht von 3000 runter bis 1600
        //Serial.println("Klemmschutz!!");
        Maschinenstatus=7;
      }
      if (mysteps >= MAXSTEP){
        Maschinenstatus=4;
      }
      break;
  case 4:  //OPEN
    reading = 3000 + loadcell.get_units(4);

    showDisplay();
    if ((setfire == HIGH)||(digitalRead(BTN_FIRE) == 0)){
       countdown = 120;
       digitalWrite(REL1,LOW); //sollte ja noch an sein..
       //Serial.println("Setfire HIGH -- starte Robbi!"); //eigener Modus 10 um von dort in Ruhe den Saugrobbi zu starten und andere Interaktionen zu blockieren.
       digitalWrite(REL2,LOW); //Solenoid Finger AN!!
       delay(1000);
       digitalWrite(REL2,HIGH); //Solenoid Finger AUSN!!
       delay(500);
       digitalWrite(REL2,LOW); //Solenoid Finger AN!!
       delay(1000);
       digitalWrite(REL2,HIGH); //Solenoid Finger AUSN!!
      
       
       setfire = LOW;
    }
    digitalWrite(REL1,HIGH);  // Power Supply ausschalten-- nach FIRE...
    while (digitalRead(BTN_DOWN) == 0){
        robigone = LOW;  //Fake setzen, sodass Tür nicht wieder nach Schliessen wieder hochgeht
        Maschinenstatus = 5;
        digitalWrite(REL1,LOW);  // Power Supply einschalten 
        delay(BTNTIME);
        manualmode = LOW;
    }
    if ((robiSaug.readparked() == 1) && (robiWisch.readparked() == 1)){
      countdown--;
      robigone = LOW;
    }
    if ((countdown <=0) && manualmode == LOW){
      Maschinenstatus = 5;
      digitalWrite(REL1,LOW);  // Power Supply einschalten
      delay(BTNTIME);
    }
    break;
  case 5: //CLOSING
    mysteps = MMklappe1.readstepcount();
    while (digitalRead(BTN_UP) == 0){
       Maschinenstatus=6;
       break;
     }
     
     iteration = 100;
     if (mysteps < iteration){
         iteration = mysteps;
     }
     for (int i=0; i < iteration; i++){
      delayMicroseconds(500);
      MMklappe1.stepdrive(CLOSE);
     } 
     //homing switch kann ja ansprechen, also hier auf Null setzen!
     if (digitalRead(SWTCH_HOME) == 0){
        MMklappe1.setstepcount(0); //mysteps=0;
        Maschinenstatus=2;
      }     
     mysteps = MMklappe1.readstepcount(); 
     
     if ((curMillis - prevMillis_hx711) >= 100) {
      prevMillis_hx711 = curMillis;
        reading = 3000 + loadcell.get_units(2);
        //Serial.print(mysteps, DEC);
        //Serial.print("\t");
        //Serial.println(reading, DEC);
      }
      if (reading > 3000 + MAX_PWR){  // reading von 3000 plus Delta wäre gefährlich
        //Serial.println("Klemmschutz!!");
        Maschinenstatus=8;
      }
      if (mysteps <= 0){
        Maschinenstatus=2;
      }
    break;
  case 6: //stop mode - drive manually up and down
    manualmode = HIGH;
    showDisplay();
    // Rauffahren manuell  
   
    while ((digitalRead(BTN_UP) == 0)  )   { //Hochfahren bei gedrücktem Schalter
      mysteps = MMklappe1.readstepcount();
      if (mysteps <= MAXSTEP){
         MMklappe1.stepdrive(OPEN); 
      }
      else{
        Maschinenstatus=4; 
      }
    }
  
   //runterfahren manuell 
   while (digitalRead(BTN_DOWN) == 0){                    //RUNTERFAHREN bei gedrückten Schalter
      if ((mysteps >0) && (digitalRead(SWTCH_HOME) == 1)) 
      {     
         MMklappe1.stepdrive(CLOSE);    
         mysteps = MMklappe1.readstepcount();
      }
      if (digitalRead(SWTCH_HOME) == 0){
        MMklappe1.setstepcount(0); //mysteps=0;
        manualmode = LOW;
        Maschinenstatus=2;
      }
      if (mysteps == 0){
        manualmode = LOW;
        Maschinenstatus=2;
      }
    }// statements
    break;
  case 7: //Error in open - 5 Versuche 
    reading = 3000 + loadcell.get_units(4);
    showDisplay();
    if (reading > MAX_PWR + 100){
       //recovery 
      Maschinenstatus = 3; 
    }
    break;
  case 8: //Error in close, einfach stoppen
    Maschinenstatus = 6;
    break;
  case 9: 
     //Serial.print("Mode 9!");
     count = 0;
     if (digitalRead(BTN_FIRE) == 0){
        display.clearDisplay();
        display.setCursor(1,4);
        display.println("HOLD FIRE!!");
        display.drawRect(7,41, 84+1, 12, SSD1306_WHITE); 
     }    
     while (digitalRead(BTN_FIRE) == 0){  //FIRE is ON at 0
        Maschinenstatus = 1;
        count++;
        if (count <=12){
           display.fillRect(8,42, count*7, 10, SSD1306_WHITE);
           display.display();
           //Serial.print("Mode 9! -count: ");
           //Serial.println(count);          
        }
        else{
           delay(100);
           Maschinenstatus = 3;
           setfire = HIGH;
           //Serial.print("Switch to Mode 3! -count: ");
           //Serial.println(count); 
           digitalWrite(REL1,LOW);
           break;
        }
     }
     if (count <= 12) Maschinenstatus=2;
     break;

  default:
    //Serial.print("default Mode:");
    //Serial.println(Maschinenstatus, DEC);
  break;
  }

  Serial.println(Maschinenstatus);
  
    if (robiSaug.readparked() == robiSaug.readnewparkpin())
    {
      // do nothing
    }
    else
    {
      //set parked...
      countdown = 120;
      robiSaug.setpark(robiSaug.readnewparkpin());
      //Serial.println("Robi_Saug changed");
      if (robiSaug.readparked()== LOW) robigone= HIGH;
      sint(Sauglast,robiSaug.readruntimesec_last());
      sint(Saughigh,robiSaug.readruntimesec_high());
      
    }
    
    if (robiWisch.readparked() == robiWisch.readnewparkpin())
    {
      // do nothing
    }
    else
    {
      //set parked...
      countdown = 120; // Reset countdown 
      robiWisch.setpark(robiWisch.readnewparkpin());
      if (robiWisch.readparked()== LOW) robigone= HIGH;
      //Serial.println("RobiWisch changed");
      sint(Wischlast,robiWisch.readruntimesec_last());
      sint(Wischhigh,robiWisch.readruntimesec_high());
    }
    

//     if (digitalRead(BTN_FIRE) == 0){
//        Serial.println("FIRE ON");
//     }
//     else{
//        //Serial.println("FIRE OFF");
//     }
     
    


  
} //void loop



//format the seconds into 4 char array with xhxx or xmxx --> 10h is maximum time anyway when a robot is declared "lost" 
char * sint (char * timetext, long t){
      int i = 0;
    if (t < (10*60)){
        i = t%10;  //einzelne sekunde
        timetext[3] = '0' + i;
        i= t%60;  //alle sekunden
        i = i/10; //zehnersekunde
        timetext[2] = '0' +i;
        timetext[1] = 'm';
        t= t-t%60; //alle sekunden abschneiden
        i = t/60;
        timetext[0] = '0' +i;
     }
     else{ //1h00(3600)  bis 9h59 (36000-1)
        timetext[1] = 'h';
        if (t >= 36000){
          timetext[3] = 'X';
          timetext[2] = 'X';
          
          timetext[0] = 'X';
        }
        else{
          //timetext[1] = 'h';
          t = t - (t%60); //sekunden brauchen wir gar nicht..
          t = t/60;
          i = t%10; //einzel Minute
          timetext[3] = '0' + i;
          i = t%60; 
          i = i/10;
          timetext[2] = '0' +i;
          t= t-t%60; //alle Minuten abschneiden
          i = t/60;
          timetext[0] = '0' +i;
          }
   } 
      return timetext;    
}
