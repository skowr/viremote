#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <Arduino.h>

#include "config.h"


void setup() {                

  // CONFIGURATION

  // return;

  pinMode(FS1000A_DATA_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  bool bol0[strlen(sig0)*4];
  bool bol1[strlen(sig1)*4];
  bool bolPause[strlen(sigPause)*4];  

  hexStringToBooleanArray(sig0, bol0);
  hexStringToBooleanArray(sig1, bol1);
  hexStringToBooleanArray(sigPause, bolPause);

  randomSeed(analogRead(A0));

  if (DEBUG)
  {
    Serial.begin(9600); 
    Serial.println("Startup");
  }


  // READ DELAY TIME

  int counter = 0;
  int factor = 1;
  int buttonstate = 0;  
  int timetowait;
  int intprog;

  blink();
  

  while (counter < READ_PERIOD)
  {
    int i = digitalRead(BUTTON_PIN);

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

  timetowait = random(10,60*factor);
  intprog = timetowait % 8;

  if (DEBUG){    
    Serial.print("End Startup. Waiting ");
    Serial.print(timetowait);
    Serial.print(" seconds (");
    Serial.print(timetowait/60);
    Serial.println(" minutes)");
    Serial.print("Initial program: ");
    Serial.println(intprog);
  }

  blink();
  
  delay(timetowait*1000);

  // PLAY
  if (DEBUG){
    Serial.println("End Waiting. Play");
  }

  while(true)
  {

    // SIGNAL
    for (int i = 0; i <= intprog ;i++)
    {      
      blink();

      trSend(bol0, sizeof(bol0));
      delayMicroseconds(6000);

      for (int i=0; i<PULSE_REPEATS; i++ ){
        trSend(bol1, sizeof(bol1));
        delayMicroseconds(6000);
      }
      
      intprog = 0;
    }

    // WAITING
    timetowait = random(5,30);
    if (DEBUG){    
      Serial.print("Play! Waiting ");
      Serial.print(timetowait);
      Serial.println(" seconds");
    }

    delay(timetowait*1000);
    if (DEBUG){
      Serial.println("Change");
    }
  }
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

void blink()
{
  for (int i = 0; i<5; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }
}

void loop() {

  // NOTHING HERE

}
