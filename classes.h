

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
  bool isOn;

  Signal()
  {
    enabled = false;
    initSignal = false;
    onSignal = false;
    isOn = false;
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

      chgSignalSize = strlen(cChg) * 4;      
      bChgSignal = new bool[chgSignalSize];
      hexStr2BoolArray(cChg, bChgSignal);
      
      offSignalSize = strlen(cOff) * 4;
      bOffSignal = new bool[offSignalSize];
      hexStr2BoolArray(cOff, bOffSignal);

  }


};
