#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <Arduino.h>
#include "config.h"


/* 
// example configuration for reference. Store usually in config.h
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

char *sigInit  = ""; // secret
char *sigOn    = ""; // secret
char *sigOff   = ""; // secret
*/

bool* bolInit = new bool[24];
bool* bolOn = new bool[144];
bool* bolOff = new bool[144];

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

  void hexStr2BoolArray(const char* hexString, bool booleanArray[]) {
    for (int i = 0; i < strlen(hexString); i++){
      unsigned char byte = hex2Bin(hexString[i]);
      for (int j = 1; j <= 4; j++){
        booleanArray[i*4+4-j] = ((byte & 1) == 1) ? true : false;
        byte >>= 1;
      }
    }
  }  

public:
  bool* bInit;
  bool* bOn;
  bool* bOff;
  int pulseBit;
  int pulseRepeats;
  int delay;
  bool enabled;

  
  init(const char* cInit, const char* cOn, const char* cOff, const int iBit, const int iDelay, const int iRpt, const bool bEnb) {

      pulseBit = iBit;
      pulseRepeats = iRpt;
      enabled = bEnb;
      delay = iDelay;

      bInit = new bool[sizeof(cInit)-1];
      bOn = new bool[sizeof(cOn)-1];
      bOff = new bool[sizeof(cOff)-1];

      hexStr2BoolArray(cInit, bInit);
      hexStr2BoolArray(cOn, bOn);
      hexStr2BoolArray(cOff, bOff);
  }
};

Signal signals[NUMBER_OF_SIGNALS];

void setup() {                

  // CONFIGURATION
  if (DEBUG)
  {
    Serial.begin(9600); 
    Serial.println("*** Startup ***");
    Serial.print("Pins Setup: ");
  }

  pinMode(FS1000A_DATA_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_CONTROL_PIN, INPUT);
  pinMode(BUTTON_OFF_PIN, INPUT);

  if (DEBUG)
  {
    Serial.println("OK");
    Serial.print("Signals setup: ");
  }

  // Get the signal definitions from config.h secret file
  signals[0].init(SIG0);
  signals[1].init(SIG1);
  signals[2].init(SIG1);

  hexStringToBooleanArray(sigInit, bolInit);
  hexStringToBooleanArray(sigOn, bolOn);
  hexStringToBooleanArray(sigOff, bolOff);

  if (DEBUG)
  {
    Serial.println("OK");
    Serial.print("Finalize preparation and blink: ");
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
    Serial.println("OK");
    Serial.println("*** Configuration period start ***");
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

  timetowait = random(10,60*factor);
  intprog = random(8);

  if (DEBUG){    
    Serial.print("End Startup. Waiting ");
    Serial.print(timetowait);
    Serial.print(" seconds (");
    Serial.print(timetowait/60);
    Serial.println(" minutes)");
    Serial.print("Initial program: ");
    Serial.println(intprog);
  }

  
  delay(timetowait*1000UL);

  // PLAY
  if (DEBUG){
    Serial.println("End Waiting. Play");
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
void trSend(bool* bol, int size)
{

  for(int i = 0; i<size; i++){
    if (bol[i])
      digitalWrite(FS1000A_DATA_PIN, HIGH);
    else
      digitalWrite(FS1000A_DATA_PIN, LOW);

    delayMicroseconds(PULSE_BIT);
  }
}

// Wait given number of samples
void trWait(int size)
{
  // Pause
  digitalWrite(FS1000A_DATA_PIN, LOW);
  for(int i = 0; i<size; i++)
    delayMicroseconds(PULSE_BIT);
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
}

// Wait given number of samples
void ntrWait(int size, int pulse)
{
  // Pause
  digitalWrite(FS1000A_DATA_PIN, LOW);
  for(int i = 0; i<size; i++)
    delayMicroseconds(pulse);
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

  Serial.print("Control: ");
  if (i == HIGH)
    Serial.print("HI");
  else
    Serial.print("LO");
  Serial.print(" | Off: ");

  if (j == HIGH)
    Serial.println("HI");
  else
    Serial.println("LO") ;


  delay(200);

  blink();
}


void sendOn(int intprog)
{
  blink();
    
  for (int j = 0; j <= intprog ;j++)
  {      
    blink();

    trSend(bolInit, sizeof(bolInit));
    delayMicroseconds(6000);

    for (int i=0; i<PULSE_REPEATS; i++ ){
      trSend(bolOn, sizeof(bolOn));
      delayMicroseconds(6000);
    }      
  }

  blink();
}

void sendOn(int intprog, Signal signal)
{
  blink();
    
  for (int j = 0; j <= intprog ;j++)
  {      
    blink();

    ntrSend(signal.bInit, sizeof(signal.bInit), signal.pulseBit);
    delayMicroseconds(signal.delay);

    for (int i=0; i<signal.pulseRepeats; i++ ){
      ntrSend(signal.bOn, sizeof(signal.bOn), signal.pulseBit);
      delayMicroseconds(signal.delay);
    }      
  }

  blink();
}

void sendOff()
{
  blink();

  trSend(bolInit, sizeof(bolInit));
  delayMicroseconds(6000);

  for (int i=0; i<PULSE_REPEATS; i++ ){
    trSend(bolOff, sizeof(bolOff));
    delayMicroseconds(6000);
  }      
}


void sendOff(Signal signal)
{
  blink();

  ntrSend(signal.bInit, sizeof(signal.bInit), signal.pulseBit);
  delayMicroseconds(signal.delay);

  for (int i=0; i<signal.pulseRepeats; i++ ){
    ntrSend(signal.bOff, sizeof(signal.bOff), signal.pulseBit);
    delayMicroseconds(signal.delay);
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
    int intprog = random(8);

    if (timetowait < 25)
      sendOff();
    else
      sendOn(intprog);
      
    counter = 0;
    timetowait = random(5,60);
    // timetowait = random(2,5);
    if (DEBUG){    
      Serial.print("Play! Waiting ");
      Serial.print(timetowait);
      Serial.println(" seconds");
      Serial.print("Program jump: ");
      Serial.println(intprog);      
    }
  }


  if (pinoff == HIGH)
  {
    sendOff();
    if (DEBUG){
      Serial.println("Turn Off");      
    }
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
