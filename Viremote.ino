#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "config.h"


/* 
// example configuration for reference. Store usually in config.h // TODO: Update
#define  FS1000A_DATA_PIN    10
#define  BUTTON_CONTROL_PIN  2
#define  BUTTON_OFF_PIN      13

#define  JAM_PULSE 100000
#define  JAM_BLINK 1000000

#define  PULSE_BIT 188 // microseconds
#define  PULSE_REPEATS 30

#define  READ_FREQUENCY_PERIOD 50
#define  READ_PERIOD 5000

#define  DEBUG 1
#define  DEBUG_BUTTON_TEST 0

*/


int counter;
int timetowait;

class Signal
{
private:

  unsigned char hex2Bin(char hex) {
    if (isdigit(hex))
      return hex - '0';
    else
      return hex - 'A' + 10;
  }  

  void hexStr2BoolArray(const char* hexString, bool* booleanArray) {
    for (int i = 0; i < strlen(hexString); i++){
      unsigned char byte = hex2Bin(hexString[i]);
      for (int j = 1; j <= 4; j++){
        booleanArray[i*4+4-j] = ((byte & 1) == 1) ? true : false;
        byte >>= 1;
      }
    }
  }  

public:
  bool* PROGMEM bInitSignal;
  bool* PROGMEM bChgSignal;
  bool* PROGMEM bOnSignal;
  bool* PROGMEM bOffSignal;

  int initSignalSize;
  int chgSignalSize;
  int onSignalSize;
  int offSignalSize;

  int pulseBit;
  int pulseRepeats;
  int delay;
  bool enabled;
  bool initSignal;
  bool onSignal;

  Signal()
  {
    enabled = false;
    initSignal = false;
    onSignal = false;
  }

  
  init(const char* cInit, const char* cOn, const char* cChg, const char* cOff, const int iBit, const int iDelay, const int iRpt, const bool bEnb) {

      pulseBit = iBit;
      pulseRepeats = iRpt;
      enabled = bEnb;
      delay = iDelay;

      int i;    

      i = strlen(cInit);
      if (i>0)
      {
        initSignal = true;
        initSignalSize = i * 4;
        bInitSignal = new bool[initSignalSize];
        hexStr2BoolArray(cInit, bInitSignal);
      }

      i = strlen(cOn);
      if (i>0)
      {
        onSignal = true;
        onSignalSize = i * 4;
        bOnSignal = new bool[onSignalSize];
        hexStr2BoolArray(cOn, bOnSignal);
      }


      onSignalSize = strlen(cOn) * 4;
      bOnSignal = new bool[onSignalSize];
      hexStr2BoolArray(cOn, bOnSignal);

      chgSignalSize = strlen(cChg) * 4;      
      bChgSignal = new bool[chgSignalSize];
      hexStr2BoolArray(cChg, bChgSignal);
      
      offSignalSize = strlen(cOff) * 4;
      bOffSignal = new bool[offSignalSize];
      hexStr2BoolArray(cOff, bOffSignal);
  }
};

Signal* signals[NUMBER_OF_SIGNALS];

void setup() {                

  // CONFIGURATION
  if (DEBUG)
  {
    Serial.begin(9600); 
    Serial.println(F("*** Startup ***"));
    Serial.print(F("Pins Setup: "));
  }

  pinMode(FS1000A_DATA_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_CONTROL_PIN, INPUT);
  pinMode(BUTTON_OFF_PIN, INPUT);

  if (DEBUG)
  {
    Serial.println(F("OK"));
    Serial.print(F("Signals setup: "));
  }

  for(int i = 0; i<NUMBER_OF_SIGNALS; i++)
    signals[i] = new Signal();

  // signals[0]->init("ffffff", "", "f9f3e60c1f307cc18307cf983e60affffffc", "f983e60c1f3e60c1f307cf98307caffffffc", 188, 6000, 3, false);
  signals[1]->init("", "", "36cb6cb", "32cb2cb", 860, 5160, 30, false);
  // signals[2]->init("", "" ,"365b65b", "325b25b", 860, 4300, 30, false);
  
  signals[3]->init(""
    , "70707070707070707ff1ce70739ce0e7070738383839c1c1c1c1c1c1c1c1c1c0e0"
    , "1c1c1c1c1c1c1c1c1c1ffc73839ce70739c1c1ce0e0e0e70739c1ce739c1c0"
    , "1c1c1c1c1c1c1c1c1c1ffc739ce0e739c1c1c1ce0e0e0e70707070738383839c0"
    , 150, 0, 10, true);

  // const char s1[] PROGMEM = "70707070707070707ff1ce70739ce0e7070738383839c1c1c1c1c1c1c1c1c1c0e0";

  // signals[3]->init(s1, s1, s1, 150, 0, 10, true);

  // Get the signal definitions from config.h secret file

  if (DEBUG)
  {
    Serial.println(F("OK"));
    Serial.print(F("Finalize: "));
  }

  randomSeed(analogRead(A0));

  if (DEBUG_BUTTON_TEST)
    return;

  // READ DELAY TIME

  int factor = 1;
  int buttonstate = 0;  
  int intprog;

  blink(); 

  counter = 0;

  if (DEBUG)
  {
    Serial.println(F("OK"));
    Serial.println(F("*** Configuration period start ***"));
  }

  while (counter < READ_PERIOD)
  {
    int i = digitalRead(BUTTON_CONTROL_PIN);

    if (buttonstate != i)
    {
      buttonstate = i;
      if (i==1)
      {
        factor *= 2;
        if (DEBUG){
          Serial.print("Button factor: ");
          Serial.println(factor);
        }
      }
    }

    delay(READ_FREQUENCY_PERIOD);
    counter += READ_FREQUENCY_PERIOD;
  }
  
  blink();

  if (!DEBUG_MODE)
    timetowait = random(10,60*factor);
  else
    timetowait = 2;

  intprog = random(8);

  if (DEBUG){    
    Serial.print(F("End Startup. Waiting "));
    Serial.print(timetowait);
    Serial.print(F(" seconds ("));
    Serial.print(timetowait/60);
    Serial.println(F(" minutes)"));
    Serial.print(F("Initial program: "));
    Serial.println(intprog);
  }

  
  delay(timetowait*1000UL);

  // PLAY
  if (DEBUG){
    Serial.println(F("End Waiting. Play"));
  }

  timetowait = 0;
  counter = 0;

}

// Helper functions
unsigned char hexToBinary(char hex) {
  if (isdigit(hex))
    return hex - '0';
  else
    return hex - 'A' + 10;
}

void hexStringToBooleanArray(const char* hexString, bool booleanArray[]) {
  for (int i = 0; i < strlen(hexString); i++){
    unsigned char byte = hexToBinary(hexString[i]);
    for (int j = 1; j <= 4; j++){
      booleanArray[i*4+4-j] = ((byte & 1) == 1) ? true : false;
      byte >>= 1;
    }
  }
}


// Send signal
void ntrSend(bool* bol, int size, int pulse)
{

  for(int i = 0; i<size; i++){
    if (bol[i])
      digitalWrite(FS1000A_DATA_PIN, HIGH);
    else
      digitalWrite(FS1000A_DATA_PIN, LOW);

    delayMicroseconds(pulse);
  }
  digitalWrite(FS1000A_DATA_PIN, LOW);
}

// Wait given number of samples
/*
void ntrWait(int size, int pulse)
{
  // Pause
  digitalWrite(FS1000A_DATA_PIN, LOW);
  for(int i = 0; i<size; i++)
    delayMicroseconds(pulse);
}
*/


void blink()
{
  for (int i = 0; i<5; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(25);
    digitalWrite(LED_BUILTIN, LOW);
    delay(25);
  }
}

void testButtons()
{
  int i = digitalRead(BUTTON_CONTROL_PIN);
  int j = digitalRead(BUTTON_OFF_PIN);

  Serial.print(F("Control: "));
  if (i == HIGH)
    Serial.print(F("HI"));
  else
    Serial.print(F("LO"));
  Serial.print(F(" | Off: "));

  if (j == HIGH)
    Serial.println(F("HI"));
  else
    Serial.println(F("LO"));


  delay(200);

  blink();
}

void sendSignal(int intprog, Signal* signal)
{
  if (signal->enabled) {

    if (signal->onSignal)
      for (int i=0; i<signal->pulseRepeats; i++ ){
        ntrSend(signal->bOnSignal, signal->onSignalSize, signal->pulseBit);
        delayMicroseconds(signal->delay);
      }      

    for (int j = 0; j <= intprog ;j++)
    {
      if (signal->initSignal)
      {
        ntrSend(signal->bInitSignal, signal->initSignalSize, signal->pulseBit);
        delayMicroseconds(signal->delay);
      }

      for (int i=0; i<signal->pulseRepeats; i++ ){
        ntrSend(signal->bChgSignal, signal->chgSignalSize, signal->pulseBit);
        delayMicroseconds(signal->delay);
      }      
    }
  }
}

/*
void sendSignal(Signal* signal)
{
  sendSignal(1, signal);
}
*/


void sendOff(Signal* signal)
{
  if (signal->enabled) {
    ntrSend(signal->bInitSignal, signal->initSignalSize, signal->pulseBit);
    delayMicroseconds(signal->delay);

    for (int i=0; i<signal->pulseRepeats; i++ ){
      ntrSend(signal->bOffSignal, signal->offSignalSize, signal->pulseBit);
      delayMicroseconds(signal->delay);
    }      
  }
}


void playloop() {

  // Check buttons
  int pinctrl = digitalRead(BUTTON_CONTROL_PIN);
  int pinoff = digitalRead(BUTTON_OFF_PIN);

  // Check the time
  if (counter >= timetowait || pinctrl == HIGH)
  {
    if (DEBUG){
      Serial.println("Change");
    }
    blink();
    int intprog = random(8);
    counter = 0;

      
    
    
    if (!DEBUG_MODE)
    {
      if (timetowait < 25)
        for (int i=0; i<NUMBER_OF_SIGNALS; i++)
          sendOff(signals[i]);
      else
        for (int i=0; i<NUMBER_OF_SIGNALS; i++)      
          sendSignal(intprog, signals[i]);
      timetowait = random(5,60);
    }
    else
    {
      for (int i=0; i<NUMBER_OF_SIGNALS; i++)      
        sendSignal(intprog, signals[i]);

      timetowait = random(2,5);
    }

    if (DEBUG){    
      Serial.print(F("Play! Waiting "));
      Serial.print(timetowait);
      Serial.println(F(" seconds"));
      Serial.print(F("Program jump: "));
      Serial.println(intprog);      
    }
  }


  if (pinoff == HIGH)
  {
    blink();
    for (int i=0; i<NUMBER_OF_SIGNALS; i++)    
      sendOff(signals[i]);
    
    if (DEBUG)
      Serial.println(F("Turn Off"));
    
    delay(3000UL);

  }

  delay(1000UL);
  counter++;

}



void loop() {

  if (DEBUG_BUTTON_TEST)
    testButtons();
  else
    playloop();

}
