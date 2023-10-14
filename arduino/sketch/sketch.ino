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

const int numBuckets = 10;   // Number of buckets to switch. This is should match 
                            // the number of buckets defined in inBucket() below, 
                            // and be less than the number of relay pins defined 
                            // above.

// LED OUTPUT pins
const int gled = A3;        // Green LED
const int yled = A4;        // Yellow LED
const int rled = A5;        // Red LED

// INPUT pin from tank sensor
const int sensor = A0;

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

// This is the arduino entry point. It is called repeatedly from the OS. When a sensor reading 
// is obtained below the threshhold, alarm() will be called, and then all relays will be reset.
// The next call will check the sensor reading again. If in range, then operation proceeds 
// normally.
void loop() { 
  Serial.println ("loop()");
  int avgVolts = readAvgVoltage();

  if (avgVolts <= .10) { 
    alarm();
  }

  setRelays(avgVolts);
}

// READS AND AVERAGES SENSOR INPUT
float readAvgVoltage() { 
  const int numReadings = 10;  // Number of inputs to average
  int total = 0;               // Sum of all readings
  float average = 0;           // Averaged value

  for (int i = 0; i < numReadings; i++) {
    total += scaleInputToVoltage( analogRead(sensor) );
    
    // This 100ms delay allows the ADC to stabilize between readings.
    // In this loop, it will be called 10 times, creating an average
    // over a full second (100ms * 10 iterations = 1000ms)
    delay(100);                    
  }
  average = total / numReadings;
  Serial.print("getSensorAverage() returning ");
  Serial.println( average);
  return average;
}

// See https://www.arduino.cc/reference/en/language/functions/analog-io/analogread/
// In particular:
//    Reads the value from the specified analog pin. Arduino boards contain a multichannel, 
//    10-bit analog to digital converter. This means that it will map input voltages between 
//    0 and the operating voltage(5V or 3.3V) into integer values between 0 and 1023. On an 
//    Arduino UNO, for example, this yields a resolution between readings of: 5 volts / 1024 
//    units or, 0.0049 volts (4.9 mV) per unit.
//
float scaleInputToVoltage( int v ) {
  return v * 0.0049;
}

// This walks the list of buckets, to set the relay and leds associated with each bucket.
void setRelays(int avgVolts) {
  Serial.print("setRelay(): "); Serial.println(avgVolts);
  // Loop over all the relays, from 1 to numRelays inclusive
  for (int i=1; i<=numBuckets; i++) {
    bool isInBucket = inBucket(i, avgVolts);
    digitalWrite( getRelayForBucket(i), (isInBucket ? HIGH : LOW) ); // set the relay for this bucket
    setLED( i, rled, isInBucket );
    setLED( i, yled, isInBucket );
    setLED( i, gled, isInBucket );
  }

  if (avgVolts >5.50) digitalWrite (relay13, HIGH);      // sensor voltage too high
}

// *** FUNCTION TO SET ALARM ***
void alarm() {                 // flash red led if volume falls below 7%
  Serial.println( "Alarm triggered!" );
  digitalWrite(rled, HIGH);
  delay (1000);
  digitalWrite(rled, LOW);
  delay (1000);
}

// Returns the bucket for a given voltage reading. 
bool inBucket(int i, float voltage) {
  // This array contains the values delimiting the various resistors. The first bucket is 
  // greater than the first element value, and less than the second element value. The second 
  // bucket is greater than the second element value, and less than the third element value, 
  // and so on. 
  const float buckets[11] = { 0.1, 0.5, 1.0, 1.2, 1.75, 2.2, 2.4, 3.4, 4.0, 4.6, 6.0 };
  return (voltage > buckets[i] && voltage <= buckets[i+1]);
}

// Returns the pin for the relay assigned to the bucket. This is a bit of a cheat
// since we know that the buckets are aligned to the pins, and offset by one. For
// example, the first bucket is assigned to the second pin, and so on.
int getRelayForBucket(int i) {
  return i+1; 
}

// Encodes the specific LED color states. This is indexed by bucket. If we're setting the LEDs 
// for a bucket that we're not in, then we're clearing them. If we're in the bucket, then we're 
// choosing the specific color for that bucket, and clearing the others.
void setLED(int i, int LED, bool inBucket) {
  int value = LOW;
  if( inBucket ) {
    if( LED = rled ) {
      value = ( i==1 ) ? HIGH : LOW;
    }
    if( LED = yled ) {
      value = ( i>1 && i<4) ? HIGH : LOW;
    }
    if( LED = gled ) {
      value = ( i>3 ) ? HIGH : LOW;
    }
  }
  digitalWrite( LED, value);
}