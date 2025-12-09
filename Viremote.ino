#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "classes.h"
#include "config.h"


int counter;
int timetowait;

Signal* signals[NUMBER_OF_SIGNALS];


void display_freeram() {
  Serial.print(F("- SRAM left: "));
  Serial.println(freeRam());
}

int freeRam() {
  extern int __heap_start,*__brkval;
  int v;
  return (int)&v - (__brkval == 0  
    ? (int)&__heap_start : (int) __brkval);  
}

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

  
  signals[0] = new Signal(SIG0Init, SIG0InitSize, SIG0On, SIG0OnSize, SIG0Chg, SIG0ChgSize, SIG0Off, SIG0OffSize, SSS0);
  signals[1] = new Signal(SIG1Init, SIG1InitSize, SIG1On, SIG1OnSize, SIG1Chg, SIG1ChgSize, SIG1Off, SIG1OffSize, SSS1);
  signals[2] = new Signal(SIG2Init, SIG2InitSize, SIG2On, SIG2OnSize, SIG2Chg, SIG2ChgSize, SIG2Off, SIG2OffSize, SSS2);
  signals[3] = new Signal(SIG3Init, SIG3InitSize, SIG3On, SIG3OnSize, SIG3Chg, SIG3ChgSize, SIG3Off, SIG3OffSize, SSS3);

  signals[3]->bChgSignal2 = SIG3Chg2;
  signals[3]->signalToggling = true;
  signals[3]->chgSignalSize2 = SIG3Chg2Size;


  // Get the signal definitions from config.h secret file

  if (DEBUG)
  {
    Serial.println(F("OK"));
    display_freeram();
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

  // Serial.print(F("PLAY, size: "));
  // Serial.println(signal->chgSignalSize);
  // Serial.print(F(" bits: "));
  // Serial.println(signal->pulseBit);

  if (signal->enabled) {

    if (signal->onSignal)
    {
      for (int i=0; i<signal->pulseRepeats; i++ ){
        ntrSend(signal->bOnSignal, signal->onSignalSize, signal->pulseBit);
        delayMicroseconds(signal->delay);
      }      
      delayMicroseconds(5000);
    }

    for (int j = 0; j <= intprog ;j++)
    {
      if (signal->initSignal)
      {
        ntrSend(signal->bInitSignal, signal->initSignalSize, signal->pulseBit);
        delayMicroseconds(signal->delay);
      }

      for (int i=0; i<signal->pulseRepeats; i++ ){
        if (signal->signalToggling) {
          i % 2 == 0 ? ntrSend(signal->bChgSignal, signal->chgSignalSize, signal->pulseBit) : ntrSend(signal->bChgSignal2, signal->chgSignalSize2, signal->pulseBit);
          delayMicroseconds(signal->delay);  
        }
        else {
          ntrSend(signal->bChgSignal, signal->chgSignalSize, signal->pulseBit);
          delayMicroseconds(signal->delay);
        }
      }
      delayMicroseconds(5000);
    }
  }
}

void sendOff(Signal* signal)
{
  if (signal->enabled) {
    if (signal->initSignal)
    {
      ntrSend(signal->bInitSignal, signal->initSignalSize, signal->pulseBit);
      delayMicroseconds(signal->delay);
    }

    for (int i=0; i<signal->pulseRepeats; i++ ){
      ntrSend(signal->bOffSignal, signal->offSignalSize, signal->pulseBit);
      delayMicroseconds(signal->delay);
    }      
    delayMicroseconds(5000);
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
        {          
          sendOff(signals[i]);
        }
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
    {

      sendOff(signals[i]);
    }
    
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
