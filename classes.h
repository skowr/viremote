class Signal
{
private:

/*  unsigned char hex2Bin(char hex) {
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
*/

public:
  bool* PROGMEM bInitSignal;
  bool* PROGMEM bChgSignal;
  bool* PROGMEM bOnSignal;
  bool* PROGMEM bOffSignal;
  bool* PROGMEM bChgSignal2;

  int initSignalSize;
  int chgSignalSize;
  int onSignalSize;
  int offSignalSize;
  int chgSignalSize2;

  int pulseBit;
  int pulseRepeats;
  int delay;
  bool enabled;
  bool initSignal;
  bool onSignal;
  bool isOn;
  bool signalToggling;

  Signal()
  {
    enabled = false;
    initSignal = false;
    onSignal = false;
    isOn = false;
    signalToggling = false;
  }

  Signal(const bool* bInit, 
      int bInitSize,
      const bool* bOn,
      const int bOnSize,
      const bool* bChg,
      const int bChgSize,
      const bool* bOff,
      const int bOffSize,
      const int iBit,
      const int iDelay,
      const int iRpt,
      const bool bEnb) {
  
    Signal();

    pulseBit = iBit;
    pulseRepeats = iRpt;
    enabled = bEnb;
    delay = iDelay;

    bInitSignal = bInit;
    bChgSignal = bChg;
    bOnSignal = bOn;
    bOffSignal = bOff;
    
    initSignalSize = bInitSize;
    onSignalSize = bOnSize;
    chgSignalSize = bChgSize;
    offSignalSize = bOffSize;

    if (initSignalSize>0) 
      initSignal = true;    
    if (onSignalSize>0)
      onSignal = true;

    
  }

};
