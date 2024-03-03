// #define DEBUG            // Enables serial debug mussaging
// #define LOG_WRITE
// #define TEST             // Executes unit test code at the end of setup

// The hardware test loop will endlessly cycle from bucket 0 to numBuckets
// turning on the appropriate LEDs and relays for each bucket. It will do
// this for all tanks as it steps into each bucket. The loop will pause for 
// each bucket for HWTEST_DELAY milliseconds before continuing to the next 
// bucket. When the last bucket is reached, it will continue again with the 
// first bucket.
#define HWTEST              // Enables hardware test loop
#define HWTEST_DELAY 5000   // number of milliseconds to pause for each bucket

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
const int D14 = 14;
const int D15 = 15;
const int D16 = 16;
const int D17 = 17;
const int D18 = 18;
const int D19 = 19;
const int D20 = 20;
const int D21 = 21;
const int D22 = 22;
const int D23 = 23;
const int D24 = 24;
const int D25 = 25;
const int D26 = 26;
const int D27 = 27;
const int D28 = 28;
const int D29 = 29;
const int D30 = 30;
const int D31 = 31;
const int D32 = 32;
const int D33 = 33;
const int D34 = 34;
const int D35 = 35;
const int D36 = 36;
const int D37 = 37;
const int D38 = 38;
const int D39 = 39;
const int D40 = 40;
const int D41 = 41;
const int D42 = 42;
const int D43 = 43;
const int D44 = 44;
const int D45 = 45;
const int D46 = 46;
const int D47 = 47;
const int D48 = 48;
const int D49 = 49;
const int D50 = 50;
const int D51 = 51;
const int D52 = 52;

// analog pins
const int unused0 = A0;
const int tank1 = A1;      // input from tank 1 sensor
const int tank2 = A2;      // input from tank 2 sensor
const int tank3 = A3;      // input from tank 3 sensor
const int rled1 = A4;        // Red LED, Tank 2
const int gled1 = A5;        // Green LED, Tank 2        
const int yled1 = A6;        // Yellow LED, Tank 2        
const int unused7 = A7;        
const int rled2 = A8;        // Red LED, Tank 3     
const int gled2 = A9;        // Green LED, Tank 3        
const int yled2 = A10;        // Yellow LED, Tank 3
const int unused1 = A11;        
const int pwrled = A12;        
const int rled3 = A13;        // Red LED, Tank 1
const int gled3 = A14;        // Green LED, Tank 1
const int yled3 = A15;        // Yellow LED, Tank 1

// Algorithm constants
// Total period over which to average the tank level for each update, in milliseconds
const int averagePeriodMs = 1000;
// The number or readings to take during the averaging period
const int numReadings = 10;  
// Number of ms to delay to allow ADC to stabilize before taking each reading.
const int delayMs = averagePeriodMs / numReadings;     

// the number of tanks to be monitored
const int numTanks = 3;
// Number of voltage buckets and outputs to track
const int numBuckets = 7; 

// the sensor pin for each tank
const int sensor[numTanks] = { tank1, tank2, tank3 };

// The voltage limits for each bucket. vLimit[bucketNumber] is the lower limit for each bucket, and 
// vLimit[bucketNumber+1] is the upper limit for each bucket.
const float vLimit[numTanks][numBuckets] = {
  { 0.1, 1.0, 1.5, 2.0, 2.5, 3.5, 4.0 },
  { 0.1, 1.0, 1.5, 2.0, 2.5, 3.5, 4.0 },
  { 0.1, 1.0, 1.5, 2.0, 2.5, 3.5, 4.0 }
};

// The led color to be illuminated when the voltage falls within the limits for each bucket.
const int bucketColor[numTanks][numBuckets+1] = {
  { rled1, rled1, yled1, yled1, gled1, gled1, gled1, gled1 },
  { rled2, rled2, yled2, yled2, gled2, gled2, gled2, gled2 },
  { gled3, gled3, gled3, gled3, yled3, yled3, rled3, rled3 }
};

// The pin to be energized when the voltage falls within the limits for each bucket. This pin will
// energize the appropriate relay to set the desired resistance.
const int relay[numTanks][numBuckets+1] = { 
  { D2,  D1,   D3,  D4,  D5,  D6,  D8,  D9 }, 
  { D14, D15, D16, D17, D18, D19, D20, D21 }, 
  { D31, D33, D35, D37, D39, D41, D43, D45 } 
};

// The alarm LED for each tank
const int ledColor[numTanks][3] = {
  { rled1, yled1, gled1 },
  { rled2, yled2, gled2 },
  { rled3, yled3, gled3 }
};
const int red = 0;
const int yellow = 1;
const int green = 2;

// algorithm variables
// The last bucket to be set. Allows us to clear the last bucket when the bucket changes, and to only 
// change led and relay states when the bucket changes.
int lastBucket[numTanks] = { 0, 0, 0};

// The average readings for each tank per loop pass.
float avgVoltage[numTanks] = { 0, 0, 0 };

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
int getBucketForVoltage(float v, int tank) {
  #ifdef TEST
    v = round( v * 100.0 ) / 100.0; // deals with float weirdness during tests
  #endif
  // Loop over all the vLimits
  int n = numBuckets;
  for (int i=0; i<numBuckets; i=i+1) {
    if ( v <= vLimit[tank][i]) {
        #ifdef DEBUG
          Serial.print("getBucketForVoltage returning bucket "); Serial.print(i); Serial.print(" for voltage "); Serial.println(v);
        #endif
      return i;
    }
  }

  #ifdef DEBUG
    Serial.print("getBucketForVoltage returning bucket 7 for voltage "); Serial.println(v);
  #endif
  return numBuckets;
}

// Reads the voltage from the sensor pin numReadings times, with a delayMs delay between each reading. The 
// intention behind this delay is to allow the ADC to stabilize between readings, and the multiple readings 
// are averaged to present a more stabilized reading in the face of tank level fluctuations due to motion
// within the tank.
void readAvgVoltage(int numReadings, int delayMs) { 
  int total[numTanks] = {0, 0, 0};                // Sum of all readings 

  // totals all ADC outputs in the range of 0-1023
  for (int i = 0; i < numReadings; i++) {
    delay(delayMs);                    // delay before reading the sensor, so all readings have time to settle
    for (int p = 0; p < numTanks; p++ ) {
      total[p] += analogRead(sensor[p]);    
    }
  }
  // converts the total of all digital ADC outputs into a total mV value, converting from int to float,
  // and divides by the number of readings to obtain an average mV value.
  for (int p = 0; p < numTanks; p++ ) {
    avgVoltage[p] = (total[p] * ADC_TO_VOLTAGE_FACTOR) / numReadings;
    #ifdef DEBUG
      Serial.print("readAvgVoltage() returning "); Serial.print(avgVoltage[p]); Serial.print(" for pin "); Serial.println(sensor[p]);
    #endif
  }
}

// Debugging method to log all digital writes.
void  logWrite(int pin, int value) {
  #ifdef LOG_WRITE && DEBUG 
      Serial.print("digitalWrite( pin " ); Serial.print(pin); Serial.print(", "); Serial.print(value); Serial.println(")");
  #endif
  digitalWrite(pin, value);
}

// Manipulate the UI appropriately to indicate an error has occurred. Currently only turns the red LED on
// then off once. 
void alarm(int tank) {                 // flash red led if volume falls below 7%
  #ifdef DEBUG
    Serial.println( "Alarm triggered!" );
  #endif
  logWrite(ledColor[tank][red], HIGH);
  delay (1000);
  logWrite(ledColor[tank][red], LOW);
  delay (1000);
  #ifdef DEBUG
    Serial.println( "Alarm exiting." );
  #endif
}

void setBucket(int b, bool inBucket, int tank) {
  #ifdef DEBUG
    Serial.print("setBucket() setting bucket "); Serial.print(b); Serial.print(inBucket ? " to true" : " to false"); Serial.print(" for tank "); Serial.println(tank);
  #endif
  // set the relay HIGH if we're in the current voltage bucket, otherwise set LOW
  logWrite( relay[tank][b], inBucket ? HIGH : LOW );
  // set each led to HIGH if the color matches the current bucket, otherwise set LOW
  int currentColor = bucketColor[tank][b];
  logWrite( ledColor[tank][red], (inBucket && ledColor[tank][red] == currentColor) ? HIGH : LOW );
  logWrite( ledColor[tank][yellow], (inBucket && ledColor[tank][yellow] == currentColor) ? HIGH : LOW );
  logWrite( ledColor[tank][green], (inBucket && ledColor[tank][green] == currentColor) ? HIGH : LOW );
}

// This is the arduino entry point. It is called repeatedly from the OS. Each invocation will read
// the average voltage over the averagePeriodMs time, determine which bucket the voltage falls into,
// and check for and handle ERR_LOW_VOLTAGE and ERR_HIGH_VOLTAGE. 
void loop() { 
  #ifdef DEBUG
    Serial.println ("loop()");
  #endif

  // reads all tank voltages
  readAvgVoltage(numReadings, delayMs);

  for( int tank=0; tank<numTanks; tank++ ) {
    int vBucket = getBucketForVoltage(avgVoltage[tank], tank);

    if( vBucket == lastBucket[tank] ) {
      continue; // do nothing if the bucket hasn't changed.
    }

    if( vBucket < 0 || vBucket > numBuckets ) {
      #ifdef DEBUG
        Serial.print("Unknown error detected: "); Serial.println(vBucket);
      #endif
      continue;
    }

    // normal operation
    #ifdef DEBUG
      Serial.print("Switching bucket state from "); Serial.print(lastBucket[tank]); Serial.print(" to "); Serial.print(vBucket); Serial.print(" for tank "); Serial.println(tank);
    #endif
    setBucket(lastBucket[tank], false, tank);
    setBucket(vBucket, true, tank);
    lastBucket[tank] = vBucket;
  }
}

// Setup() initializes all the pins and enables serial i/o for debugging.
void setup() {
  #if defined DEBUG || defined TEST || defined HWTEST
    Serial.begin(9600);           // initialize serial interface
    Serial.println ("Entering setup. Serial initialized.");
  #endif

  #ifdef TEST
    Serial.println( "Executing tests...");
    testGetBucketForVoltage();
  #endif

  // initialize pins
  // all relay output pins
  for( int tank=0; tank < numTanks; tank++ ) {
    for( int i=0; i<numBuckets; i++) {
      pinMode( relay[tank][i], OUTPUT);
    }
    pinMode(sensor[tank], INPUT);       // Tank sensor input
    // LEDs
    pinMode(ledColor[tank][red], OUTPUT);        
    pinMode(ledColor[tank][yellow], OUTPUT);        
    pinMode(ledColor[tank][green], OUTPUT);

    // high voltage error pin
    pinMode(ERR_HIGH_VOLT_PIN, OUTPUT);
  }
  #ifdef HWTEST
    Serial.println( "Running hardware test loop…");
    runHWTestLoop();
  #endif

  #ifdef DEBUG
    Serial.println ("Exiting setup.");
  #endif
}

#ifdef TEST // All test methods below.

// Checks to be sure that the correct bucket index is returned given the expected range of 
// voltage values.
void testGetBucketForVoltage() {
  #ifdef DEBUG
    Serial.println("testGetBucketForVoltage()");
  #endif

 // voltages! { 0.1, 1.0, 1.5, 2.0, 2.5, 3.5, 4.0 },
 // buckets!  {   0    1.   2.   3.   4.   5.   6.   7}

  int testCaseLen = 11; // length of the testCase array
  float testCase[] = {0.0, 0.1, 0.3, 0.5, 0.6, 1.0, 1.1, 1.2, 3.0, 6.0, 7.0 };  // the test inputs
  int expected[] =   {  0,   0,   1,   1,   1,   1,   2,   2,   5,   7,   7 };  // and expected output values for each input

  bool pass = true;
  for(int i=0; i<testCaseLen; i++) {
    int actual = getBucketForVoltage(testCase[i], 0);
    if(expected[i] != actual) {
      Serial.print( "   Case "); Serial.print(i); Serial.print(" expected "); Serial.print(expected[i]); Serial.print(" != "); Serial.println(actual);
      pass = false;
    }
  }
  Serial.print( "testGetBucketForVoltage: "); Serial.println(pass ? "PASS" : "FAIL");
}
#endif

#ifdef HWTEST
void runHWTestLoop() {
  while(true) {
    for(int b=0; b<numBuckets; b++) {
      Serial.print( "Switching to bucket " ); Serial.println( b );
      for(int t=0; t<numTanks; t++) {
        setBucket(lastBucket[t], false, t);
        setBucket(b, true, t);
        lastBucket[t] = b;
      }
      delay(HWTEST_DELAY);
    }
  }
}
#endif
