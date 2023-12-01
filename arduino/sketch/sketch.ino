#include <SPI.h>

#define DEBUG

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
const int tank1 = A0;          // input from tank sensor
const int unused1 = A1;      
const int unused2 = A2;
const int unused3 = A3;
const int unused4 = A4;
const int unused5 = A5;        // Red LEDvoid setup() {

// The Arduino board voltage
const float BOARD_VOLTAGE = 5.0;  // 5.0 is the voltage for the Arduino UNO.
// The factor used to convert Arduino analogRead() inputs directly to voltage.
const float ADC_TO_VOLTAGE_FACTOR = BOARD_VOLTAGE / 1024;

// eight bit commands
// A A A A  C C D D 
const int addressMask8 = 0xF0;
const int commandMask8 = 0XC;
const int dataMask8 = 0x3;

// sixteen bit commands
// A A A A  C C D D    D D D D  D D D D
const int addressMask16 = 0xF000;
const int commandMask16 = 0XC00;
const int dataMask16 = 0x3FF;

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
const byte DPOT_STEPS = 256;  // could be 127?
const float DPOT_VOLTS_PER_STEP =  DPOT_STEPS / BOARD_VOLTAGE;

/*
  Shifts the bottom four bits of the address, the bottom two 
  bits of the command, and the bottom two bits of the data into
  a one byte command word of the format:
            A A A A  C C D D
*/
byte oneByteCommand(byte address, byte command, byte data) {
// shift each field into the right position
  int shiftedAddress = address << 4;
  int shiftedCommand = command << 2;

  // mask each field so it can't stomp on the other fields
  shiftedAddress = shiftedAddress & addressMask8;
  shiftedCommand = shiftedCommand & commandMask8;
  int maskedData = data && dataMask8;

  // bitwise OR to get them all into the two byte word
  return shiftedAddress | shiftedCommand | maskedData;
}

/*
  Shifts the bottom four bits of the address, the bottom two 
  bits of the command, and the bottom ten bits of the data into
  a two byte command word of the format:
            A A A A  C C D D    D D D D  D D D D
*/
int twoByteCommand(byte address, byte command, int data) {
  // shift each field into the right position
  int shiftedAddress = address << 12;
  int shiftedCommand = command << 10;

  // mask each field so it can't stomp on the other fields
  shiftedAddress = shiftedAddress & addressMask16;
  shiftedCommand = shiftedCommand & commandMask16;
  int maskedData = data && dataMask16;

  // bitwise OR to get them all into the two byte word
  return shiftedAddress | shiftedCommand | maskedData;
}

void loop() {
  #ifdef DEBUG
    Serial.println ("Entering loop...");
  #endif

  // read the tank input
  int reading = analogRead(tank1);
  float voltage = reading * ADC_TO_VOLTAGE_FACTOR;
  int dpotSetting = round( DPOT_VOLTS_PER_STEP * voltage );
  
  #ifdef DEBUG
    Serial.print("loop(): ");
    Serial.print("reading: ");
    Serial.print(reading);
    Serial.print(", voltage: ");
    Serial.print(voltage);
    Serial.print("dpotSetting: ");
    Serial.println(dpotSetting);
  #endif

  // // set the digital potentiometer
  // int cmd = twoByteCommand(ADR_W1, CMD_WRITE, dpotSetting);
  // #ifdef DEBUG
  //   Serial.print("loop(): constructed cmd "); Serial.println( cmd, HEX );
  // #endif

  // #ifdef DEBUG
  //   Serial.println ("loop(): begin SPI" );
  // #endif
  // digitalWrite(CS, CS_SELECTED); // select
  // SPI.begin();
  // SPI.beginTransaction( SPISettings(SPISettings(SPI_BAUD, MSBFIRST, SPI_MODE0)) );
  // SPI.transfer16(cmd);
  // SPI.endTransaction();
  // digitalWrite(CS, CS_UNSELECTED);
  // #ifdef DEBUG
  //   Serial.println ("loop(): end SPI" );
  // #endif

  // #ifdef DEBUG
  //   delay(1000);
  //   Serial.println ("...Exiting loop.");
  // #endif
}

void setup() {
  #if defined DEBUG || defined TEST
    Serial.begin(9600);           // initialize serial interface
    Serial.println ("Entering setup. Serial initialized.");
  #endif

  #ifdef TEST
    Serial.println( "Executing tests...");
  #endif

  // initialize pins
  pinMode(CS, OUTPUT);
  pinMode(SDIO, OUTPUT);
  pinMode(SCLK, OUTPUT);
  //pinMode(tank1, INPUT);  // don't need to set pin mode for analog pins
  //analogReference(EXTERNAL);

  #ifdef DEBUG
    Serial.println ("Exiting setup.");
  #endif

}