#include <SPI.h>

// #define DEBUG
// #define TEST

// Hardware constants
// digital pins
const int D0 = 0;
const int D1 = 1;
const int D2 = 2;
const int D3 = 3;
const int D4 = 4;
const int D5 = 5;
const int D6 = 6;
const int D7 = 7;
const int D8 = 8;
const int D9 = 9;
const int D10 = 10; 
const int D11 = 11; 
const int D12 = 12;
const int D13 = 13; 

const int CS = D10;
const int SDIO = D11;
const int SCLK = D13;

// analog pins
const int unused = A0;          // input from tank sensor
const int tank1 = A1;      
const int unused2 = A2;
const int unused3 = A3;
const int unused4 = A4;
const int unused5 = A5;        // Red LEDvoid setup() {

// The Arduino board voltage
const float BOARD_VOLTAGE = 5.0;  // 5.0 is the voltage for the Arduino UNO.
// The factor used to convert Arduino analogRead() inputs directly to voltage.
const float ADC_TO_VOLTAGE_FACTOR = BOARD_VOLTAGE / 1024.0;

// eight bit commands
// A A A A  C C D D 
const int addressMask8 = 0xF;
const int commandMask8 = 0x3;
const int dataMask8 =    0x3;

// sixteen bit commands
// A A A A  C C D D    D D D D  D D D D
const long addressMask16 = 0x00F;
const long commandMask16 = 0x003;
const long dataMask16 =    0x3FF;

// MCP4151 commands
const byte CMD_READ = 0x3;
const byte CMD_WRITE = 0x0;
const byte CMD_INCREMENT = 0x1;
const byte CMD_DECREMENT = 0x2;

// MCP4151 CS constants
const byte CS_SELECTED = LOW;
const byte CS_UNSELECTED = HIGH;

// MCP4151 addresses
const byte ADR_W1 = 0x0;  // wiper 1

// MCP4151 SPI Settings
const int SPI_BAUD = 10000000;


// Digital pot number of steps
const int DPOT_STEPS = 256;  // could be 127?
const float DPOT_VOLTS_PER_STEP =  DPOT_STEPS / BOARD_VOLTAGE;

/*
  Shifts the bottom four bits of the address, the bottom two 
  bits of the command, and the bottom two bits of the data into
  a one byte command word of the format:
            A A A A  C C D D
*/
byte oneByteCommand(byte address, byte command, byte data) {
  return -1; // needs to be reimplemented along the lines of twoByteCommand.
}

void debugLog(String name, long value, long masked, long shifted) {
   #ifdef DEBUG
    Serial.print("twoByteCommand(): "); Serial.println(name);
    Serial.print(" value: "); Serial.println(value, HEX); 
    Serial.print(" maskd: "); Serial.println(masked, HEX);
    Serial.print(" shift: "); Serial.println(shifted, HEX);
  #endif 
}
/*
  Shifts the bottom four bits of the address, the bottom two 
  bits of the command, and the bottom ten bits of the data into
  a two byte command word of the format:
            A A A A  C C D D    D D D D  D D D D
*/
int twoByteCommand(byte address, byte command, int data) {

  long maskedAddress = address & addressMask16;
  long maskedCommand = command & commandMask16;
  long maskedData = data & dataMask16;

  long shiftedAddress = maskedAddress << 12;
  long shiftedCommand = maskedCommand << 10;
  long shiftedData = maskedData;

  int result = shiftedAddress | shiftedCommand | shiftedData;
  
  // #ifdef DEBUG
  //   debugLog("address", address, maskedAddress, shiftedAddress);
  //   debugLog("command", command, maskedCommand, shiftedCommand);
  //   debugLog("data", data, maskedData, shiftedData);
  //   Serial.print(" result: "); Serial.println(result, HEX);
  // #endif

  return result;
}

int convertReadingToDPotSetting(int reading) { 
  float voltage = reading * ADC_TO_VOLTAGE_FACTOR;
  int dpotSetting = round(DPOT_VOLTS_PER_STEP * voltage);
  Serial.print ("readTank(): reading:"); Serial.print(reading, DEC); Serial.print(", voltage:"); Serial.print(voltage, 2); Serial.print(", dpotSetting:"); Serial.print(dpotSetting, DEC);
  return dpotSetting;
}

void loop() {
  #ifdef DEBUG
    Serial.println ("Entering loop...");
  #endif

  // read the tank input
  int dpotSetting = convertReadingToDPotSetting( analogRead(tank1) );

  // set the digital potentiometer
  int cmd = twoByteCommand(ADR_W1, CMD_WRITE, dpotSetting);
  Serial.print(", cmd "); Serial.println( cmd, HEX );
  
  #ifdef DEBUG
    Serial.println ("loop(): begin SPI" );
  #endif
  digitalWrite(CS, CS_SELECTED); // select
  SPI.begin();
  SPI.beginTransaction( SPISettings(SPISettings(SPI_BAUD, MSBFIRST, SPI_MODE0)) );
  SPI.transfer16(cmd);
  SPI.endTransaction();
  digitalWrite(CS, CS_UNSELECTED);
  #ifdef DEBUG
    Serial.println ("loop(): end SPI" );
  #endif

  #ifdef DEBUG
    delay(1000);
    Serial.println ("...Exiting loop.");
  #endif
}

void setup() {
  Serial.begin(9600);           // initialize serial interface
  Serial.println ("Entering setup. Serial initialized.");

  #ifdef TEST
    Serial.println( "Executing tests...");
    testTwoByteCommand();
    testConvertReadingToDPotSetting();
  #endif

  // initialize pins
  pinMode(CS, OUTPUT);
  pinMode(SDIO, OUTPUT);
  pinMode(SCLK, OUTPUT);
  //pinMode(tank1, INPUT);  // don't need to set pin mode for analog pins
  //analogReference(EXTERNAL);

  Serial.println ("Exiting setup.");
}


#ifdef TEST

void testTwoByteCommand() {
  const int numTests = 6;
  // address, command, data, result
  int tests[numTests][4] = {
    {0, 0, 0, 0},
    {addressMask16, commandMask16, dataMask16, 0xFFFFFFF},
    {0x1, 0x1, 0x235, 0x1635},
    {0x2, 0x2, 0x235, 0x2A35},
    {0x4, 0x1, 0x235, 0x4635},
    {0x8, 0x2, 0x235, 0x8A35}
  };

  Serial.println("testTwoByteCommand():");

  for( int t=0; t < numTests; t++) {
    byte address = tests[t][0];
    byte command = tests[t][1];
    int data = tests[t][2];
    int correct = tests[t][3];
    int result = twoByteCommand(address, command, data);
    if( result == correct ) {
        Serial.print("passed "); Serial.println(t, DEC);
    } else {
        Serial.print("failed "); Serial.println(t, DEC);
    }
  }
}

void testConvertReadingToDPotSetting() {
  const int numTests = 32;
  // address, command, data, result
  float tests[numTests][2] = {
    { 0, 0 }, { 32, 8 }, { 64, 16 }, { 96, 24 }, { 128, 32 }, { 160, 40 }, { 192, 48 }, { 224, 56 }, { 256, 64 }, { 288, 72 }, { 320, 80 }, { 352, 88 }, { 384, 96 }, { 416, 104 }, { 448, 112 }, { 480, 120 }, { 512, 128 }, { 544, 136 }, { 576, 144 }, { 608, 152 }, { 640, 160 }, { 672, 168 }, { 704, 176 }, { 736, 184 }, { 768, 192 }, { 800, 200 }, { 832, 208 }, { 864, 216 }, { 896, 224 }, { 928, 232 }, { 960, 240 }, { 992, 248 }
  };

  Serial.println("testConvertReadingToDPotSetting():");

  for( int t=0; t < numTests; t++) {
    float read = tests[t][0];
    int correct = tests[t][1];
    int result = convertReadingToDPotSetting(read);
    if( result == correct ) {
        Serial.print("passed "); Serial.println(t, DEC);
    } else {
        Serial.print("failed "); Serial.println(t, DEC);
    }
  }
}

#endif