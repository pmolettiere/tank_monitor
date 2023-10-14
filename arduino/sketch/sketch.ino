// Hardware constants
// relay OUTPUT pins
const int relay2 = 2;        
const int relay3 = 3;
const int relay4 = 4;
const int relay5 = 5;
const int relay6 = 6;
const int relay7 = 7;
const int relay8 = 8;
const int relay9 = 9;
const int relay10 = 10;
const int relay11 = 11;
const int relay12 = 12;
const int relay13 = 13;
// LED OUTPUT pins
const int gled = A3;        // Green LED
const int yled = A4;        // Yellow LED
const int rled = A5;        // Red LED
// INPUT pin from tank sensor
const int sensor = A0;

// Algorithm constants
// Period over which to average the tank level for each update, in milliseconds
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
const int pin[numBuckets] = { relay2, relay3, relay4, relay5, relay6, relay7, relay8, relay9, relay10, relay11 };

// Error constants
// Voltage below lower limit.
const int ERR_LOW_VOLTAGE = -1;
// Voltage above upper limit.
const int ERR_HIGH_VOLTAGE = -2;

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
// is not within the limits defined by vLimit, either below vLimit[0] or above vLimit[numBuckets+1],
// then either ERR_LOW_VOLTAGE or ERR_HIGH_VOLTAGE are returned respectively. All valid bucket indexes
// are non-negative, and all errors returned are negative. 
int getBucketForVoltage(float v) {
  if ( v < vLimit[0] ) return ERR_LOW_VOLTAGE;

  // Loop over all the vLimits
  for (int i=1; i<=numBuckets; i++) {
    if (v > vLimit[i] && v <= vLimit[i+1]) {
      return i;
    }
  }

  return ERR_HIGH_VOLTAGE;
}

// Reads the voltage from the sensor pin numReadings times, with a delayMs delay between each reading. The 
// intention behind this delay is to allow the ADC to stabilize between readings, and the multiple readings 
// are averaged to present a more stabilized reading in the face of tank level fluctuations to due motion
// within the tank.
float readAvgVoltage(int numReadings, int delayMs) { 
  int total = 0;               // Sum of all readings
  float avgVoltage = 0;           // Averaged value

  for (int i = 0; i < numReadings; i++) {
    total += analogRead(sensor);    
    delay(delayMs);                    
  }
  avgVoltage = (total * ADC_TO_VOLTAGE_FACTOR) / numReadings;
  Serial.print("getSensorAverage() returning ");
  Serial.println( avgVoltage);
  return avgVoltage;
}

// Manipulate the UI appropriately to indicate an error has occurred. Currently only turns the red LED on
// then off once. 
void alarm() {                 // flash red led if volume falls below 7%
  Serial.println( "Alarm triggered!" );
  digitalWrite(rled, HIGH);
  delay (1000);
  digitalWrite(rled, LOW);
  delay (1000);
}

// This is the arduino entry point. It is called repeatedly from the OS. When a sensor reading 
// is obtained below the threshhold, alarm() will be called, and then all relays will be reset.
// The next call will check the sensor reading again. If in range, then operation proceeds 
// normally.
void loop() { 
  Serial.println ("loop()");
  float v = readAvgVoltage(numReadings, delayMs);
  int vBucket = getBucketForVoltage(v);

  // For a low voltage error, call alarm to indicate the error, the continue to clear all relays and leds
  if (vBucket == ERR_LOW_VOLTAGE) { 
    alarm();
  }

  // walk the buckets, set each relay state, and set each LED state
  for(int b=0; b<numBuckets; b++) {
    bool inBucket = v==vBucket;
    // set the relay HIGH if we're in the current voltage bucket, otherwise set LOW
    digitalWrite( pin[b], inBucket ? HIGH : LOW );
    // set each led to HIGH if the color matches the current bucket, otherwise set LOW
    digitalWrite( rled, rled == ledColor[b] ? HIGH : LOW );
    digitalWrite( yled, yled == ledColor[b] ? HIGH : LOW );
    digitalWrite( yled, gled == ledColor[b] ? HIGH : LOW );
  }

  // For a high voltage error, all relays and LEDs will have been cleared, then we raise pin 13 for some reason
  if (vBucket == ERR_HIGH_VOLTAGE ) {
    digitalWrite (relay13, HIGH);      // sensor voltage too high
  }
}

// Setup() initializes all the pins and enables serial i/o for debugging.
void setup() {
  Serial.println ("SETUP");
  Serial.begin(9600);           // initialize serial interface

  // initialize pins
  pinMode(relay2, OUTPUT);      // Relays
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(relay5, OUTPUT);
  pinMode(relay6, OUTPUT);
  pinMode(relay7, OUTPUT);
  pinMode(relay8, OUTPUT);
  pinMode(relay9, OUTPUT);
  pinMode(relay10, OUTPUT);
  pinMode(relay11, OUTPUT);
  pinMode(relay12, OUTPUT);      
  pinMode(relay13, OUTPUT);
  
  pinMode(A3, OUTPUT);           // LEDs
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);

  pinMode(A0, INPUT);            // Tank sensor input
}
