#include <SPI.h>
#include <mcp4xxx.h>

using namespace icecave::arduino;

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

const int gled = D4;
const int yled = D2;
const int rled = D3;

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

// MCP4151
MCP4XXX* pot1;

// Digital pot number of steps
const int DPOT_STEPS = 255;  
const float DPOT_VOLTS_PER_STEP =  DPOT_STEPS / BOARD_VOLTAGE;

// LED color levels as dpotSetting values
const int GREEN = 155;
const int YELLOW = 104;
const int FLASH = 53;

int convertReadingToDPotSetting(int reading) { 
  float voltage = reading * ADC_TO_VOLTAGE_FACTOR;
  int dpotSetting = round(DPOT_VOLTS_PER_STEP * voltage);
  Serial.print ("readTank(): reading:"); Serial.print(reading, DEC); Serial.print(", voltage:"); Serial.print(voltage, 2); Serial.print(", dpotSetting:"); Serial.print(dpotSetting, DEC);
  return dpotSetting;
}

void light(int led) {
  digitalWrite( gled, gled == led ? HIGH : LOW );
  digitalWrite( yled, yled == led ? HIGH : LOW );
  digitalWrite( rled, rled == led ? HIGH : LOW );
}

void loop() {
  #ifdef DEBUG
    Serial.println ("Entering loop...");
  #endif

  // read the tank input
  int dpotSetting = convertReadingToDPotSetting( analogRead(tank1) );

  if( dpotSetting > GREEN ) {
    light(gled);
  } else {
    if( dpotSetting > YELLOW ) {
      light(yled);
    } else {
      light(rled);
      // need to implement flash here
    }
  }
  
  #ifdef DEBUG
    Serial.println ("loop(): begin SPI" );
  #endif
  // set the digital potentiometer
  pot1->set(dpotSetting);
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
    testConvertReadingToDPotSetting();
  #endif

  // initialize pins
  pinMode(CS, OUTPUT);
  pinMode(SDIO, OUTPUT);
  pinMode(SCLK, OUTPUT);
  pinMode(gled, OUTPUT);
  pinMode(yled, OUTPUT);
  pinMode(rled, OUTPUT);

  // initialize the digital potentiometer connection
  pot1 = new MCP4XXX(CS);
  // Move the wiper to the lowest value
  pot1->set(0);

  //pinMode(tank1, INPUT);  // don't need to set pin mode for analog pins
  //analogReference(EXTERNAL);

  Serial.println ("Exiting setup.");
}


#ifdef TEST

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