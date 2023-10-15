//#define DEBUG
//#define TEST

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
// analog pins
const int sensor = A0;      // input from tank sensor
const int unused1 = A1;
const int unused2 = A2;
const int gled = A3;        // Green LED
const int yled = A4;        // Yellow LED
const int rled = A5;        // Red LED

// Algorithm constants
// Total period over which to average the tank level for each update, in milliseconds
const int averagePeriodMs = 1000;
// The number or readings to take during the averaging period
const int numReadings = 10;  
// Number of ms to delay to allow ADC to stabilize before taking each reading.
const int delayMs = averagePeriodMs / numReadings;     
// Number of voltage buckets and outputs to track
const int numBuckets = 10;  
// The voltage limits for each bucket. vLimit[bucketNumber] is the lower limit for each bucket, and 
// vLimit[bucketNumber+1] is the upper limit for each bucket.
const float vLimit[numBuckets+1] = { 0.1, 0.5, 1.0, 1.2, 1.75, 2.2, 2.4, 3.4, 4.0, 4.6, 6.0 }; 
// The led color to be illuminated when the voltage falls within the limits for each bucket.
const int ledColor[numBuckets] = { rled, yled, yled, gled, gled, gled, gled, gled, gled, gled };
// The pin to be energized when the voltage falls within the limits for each bucket. This pin will
// energize the appropriate relay to set the desired resistance.
const int relay[numBuckets] = { D2, D3, D4, D5, D6, D7, D8, D9, D10, D11 };

// Error constants
// Voltage below lower limit.
const int ERR_LOW_VOLTAGE = -1;
// Voltage above upper limit.
const int ERR_HIGH_VOLTAGE = -2;
// Pin to raise on high voltage error
const int ERR_HIGH_VOLT_PIN = D13;

// ADC to Voltage conversion constants
// See https://www.arduino.cc/reference/en/language/functions/analog-io/analogread/
// In particular:
//    Reads the value from the specified analog pin. Arduino boards contain a multichannel, 
//    10-bit analog to digital converter. This means that it will map input voltages between 
//    0 and the operating voltage(5V or 3.3V) into integer values between 0 and 1023. On an 
//    Arduino UNO, for example, this yields a resolution between readings of: 5 volts / 1024 
//    units or, 0.0049 volts (4.9 mV) per unit.
//
// The Arduino board voltage
const float BOARD_VOLTAGE = 5.0;  // 5.0 is the voltage for the Arduino UNO.
// The factor used to convert Arduino analogRead() inputs directly to voltage.
const float ADC_TO_VOLTAGE_FACTOR = BOARD_VOLTAGE / 1024;

// Returns the bucket index for a given voltage v by walking the list of vLimits. If v
// is not within the limits defined by vLimit, either below or equal to vLimit[0] or above vLimit[numBuckets+1],
// then either ERR_LOW_VOLTAGE or ERR_HIGH_VOLTAGE are returned respectively. All valid bucket indexes
// are non-negative, and all errors returned are negative. 
int getBucketForVoltage(float v) {
  #ifdef TEST
    v = round( v * 100.0 ) / 100.0; // deals with float weirdness during tests
  #endif
  if ( v <= vLimit[0] ) {
    #ifdef DEBUG
      Serial.print("getBucketForVoltage returning ERR_LOW_VOLTAGE for "); Serial.println(v);
    #endif
    return ERR_LOW_VOLTAGE;
  }

  // Loop over all the vLimits
  for (int i=0; i<=numBuckets; i++) {
    if (v > vLimit[i] && v <= vLimit[i+1]) {
        #ifdef DEBUG
          Serial.print("getBucketForVoltage returning bucket "); Serial.print(i); Serial.print(" for voltage "); Serial.println(v);
        #endif
      return i;
    }
  }

  #ifdef DEBUG
    Serial.print("getBucketForVoltage returning ERR_HIGH_VOLTAGE for "); Serial.println(v);
  #endif
  return ERR_HIGH_VOLTAGE;
}

// Reads the voltage from the sensor pin numReadings times, with a delayMs delay between each reading. The 
// intention behind this delay is to allow the ADC to stabilize between readings, and the multiple readings 
// are averaged to present a more stabilized reading in the face of tank level fluctuations due to motion
// within the tank.
float readAvgVoltage(int numReadings, int delayMs) { 
  int total = 0;               // Sum of all readings
  float avgVoltage = 0;        // Averaged value

  // totals all ADC outputs in the range of 0-1023
  for (int i = 0; i < numReadings; i++) {
    delay(delayMs);                    // delay before reading the sensor, so all readings have time to settle
    total += analogRead(sensor);    
  }
  // converts the total of all digital ADC outputs into a total mV value, converting from int to float,
  // and divides by the number of readings to obtain an average mV value.
  avgVoltage = (total * ADC_TO_VOLTAGE_FACTOR) / numReadings;
  #ifdef DEBUG
    Serial.print("readAvgVoltage() returning ");
    Serial.println( avgVoltage);
  #endif
  return avgVoltage;
}

// Debugging method to log all digital writes.
void logWrite(int pin, int value) {
  #ifdef DEBUG
      // Serial.print("digitalWrite( pin " ); Serial.print(pin); Serial.print(", "); Serial.print(value); Serial.println( ")");
  #endif
  digitalWrite(pin, value);
}

// Manipulate the UI appropriately to indicate an error has occurred. Currently only turns the red LED on
// then off once. 
void alarm() {                 // flash red led if volume falls below 7%
  #ifdef DEBUG
    Serial.println( "Alarm triggered!" );
  #endif
  logWrite(rled, HIGH);
  delay (1000);
  logWrite(rled, LOW);
  delay (1000);
  #ifdef DEBUG
    Serial.println( "Alarm exiting." );
  #endif
}

// This is the arduino entry point. It is called repeatedly from the OS. Each invocation will read
// the average voltage over the averagePeriodMs time, determine which bucket the voltage falls into,
// and check for and handle ERR_LOW_VOLTAGE and ERR_HIGH_VOLTAGE. 
void loop() { 
  #ifdef DEBUG
    Serial.println ("loop()");
  #endif
  float v = readAvgVoltage(numReadings, delayMs);
  int vBucket = getBucketForVoltage(v);

  // walk the buckets, set each relay state, and set each LED state. If an error has been returned
  // then all relays and leds will be brought low.
  for(int b=0; b<numBuckets; b++) {
    bool inBucket = v==vBucket;
    // set the relay HIGH if we're in the current voltage bucket, otherwise set LOW
    logWrite( relay[b], inBucket ? HIGH : LOW );
    // set each led to HIGH if the color matches the current bucket, otherwise set LOW
    int currentColor = ledColor[b];
    logWrite( rled, (inBucket && rled == currentColor) ? HIGH : LOW );
    logWrite( yled, (inBucket && yled == currentColor) ? HIGH : LOW );
    logWrite( gled, (inBucket && gled == currentColor) ? HIGH : LOW );
  }

  if( vBucket < 0 ) { // an error must be handled.
    switch( vBucket ) {
      case ERR_LOW_VOLTAGE : {
        alarm();
        break;
      }
      case ERR_HIGH_VOLTAGE : {
        #ifdef DEBUG
          Serial.println("ERR_HIGH_VOLTAGE detected.");
        #endif
        logWrite (ERR_HIGH_VOLT_PIN, HIGH);      // sensor voltage too high
        break;
      }
      default : {
        #ifdef DEBUG
          Serial.print("Unknown error detected: "); Serial.println(vBucket);
        #endif
      }
    }
  }
}

// Setup() initializes all the pins and enables serial i/o for debugging.
void setup() {
  #if defined DEBUG || defined TEST
    Serial.begin(9600);           // initialize serial interface
    Serial.println ("Entering setup. Serial initialized.");
  #endif

  #ifdef TEST
    Serial.println( "Executing tests...");
    testGetBucketForVoltage();
  #endif

  // initialize pins
  // all relay output pins
  for( int i=0; i<numBuckets; i++) {
    pinMode( relay[i], OUTPUT);
  }
  pinMode(sensor, INPUT);       // Tank sensor input
  // LEDs
  pinMode(rled, OUTPUT);        
  pinMode(yled, OUTPUT);        
  pinMode(gled, OUTPUT);
  // high voltage error pin
  pinMode(ERR_HIGH_VOLT_PIN, OUTPUT);
  #ifdef DEBUG
    Serial.println ("Exiting setup.");
  #endif
}

#ifdef TEST
// All test methods below.
void testGetBucketForVoltage() {
  #ifdef DEBUG
    Serial.println("testGetBucketForVoltage()");
  #endif

//const float vLimit[numBuckets+1] = { 0.1, 0.5, 1.0, 1.2, 1.75, 2.2, 2.4, 3.4, 4.0, 4.6, 6.0 }; 

  int testCaseLen = 11;
  float testCase[] = {0.0, 0.1, 0.3, 0.5, 0.6, 1.0, 1.1, 1.2, 3.0, 6.0, 7.0 };
  int expected[] =   { -1,  -1,   0,   0,   1,   1,   2,   2,   6,  9,  -2 };

  bool pass = true;
  for(int i=0; i<testCaseLen; i++) {
    int actual = getBucketForVoltage(testCase[i]);
    if(expected[i] != actual) {
      Serial.print( "   Case "); Serial.print(i); Serial.print(" expected "); Serial.print(expected[i]); Serial.print(" != "); Serial.println(actual);
      pass = false;
    }
  }
  Serial.print( "testGetBucketForVoltage: "); Serial.println(pass ? "PASS" : "FAIL");
}
#endif
