#include <util/atomic.h> // this library includes the ATOMIC_BLOCK macro.

volatile int flow_frequency=0; // Measures flow sensor pulses
float vol = 0.0;      // συνολικός όγκος νερού σε λίτρα (lt)
float l_minute = 0.0; // ροή νερού σε lt/min
float l_sec = 0.0;    // ροή νερού σε lt/sec

// Pins
const int flowSensorPin = 2; // Flow Sensor Input
const int buttonPin = 3;
const int ledPin = 7;
const int waterLevelSensorPin = A0;
const int relayPin = 6;

unsigned long currentTime;
unsigned long cloopTime;
int waterLevel = 0;
int pososto = 0;
int buttonState = 0;

char buf[30];       // το string αποστολής
char floatStr1[10]; // προσωρινό string (για τη μετατροπή float σε string)
char floatStr2[10];

void flow() // Interrupt function
{
  flow_frequency++;
}

void setup()
{
  pinMode(flowSensorPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), flow, RISING); // Setup Interrupt
  currentTime = millis();
  cloopTime = currentTime;
}

void loop()
{
  currentTime = millis();

  // Every second, calculate flow
  if(currentTime >= (cloopTime + 1000))
  {
    cloopTime = currentTime;
    
    waterLevel = analogRead(waterLevelSensorPin);
    // Μετατροπή σε ποσοστό (με βάση την ελάχιστη και τη μέγιστη τιμή)
    // map(value, fromLow, fromHigh, toLow, toHigh)
    pososto = map(waterLevel, 400, 650, 0, 100);
    pososto = constrain(pososto, 0, 100);

    // απαραίτητο για την επεξεργασία volatile μεταβλητών
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      //Serial.print(flow_frequency);
      if(flow_frequency != 0)
      {
        // Flow rate pulse characteristics: Frequency (Hz) = 7.5 * Flow rate (L/min)
        l_minute = flow_frequency / 7.5;
        l_sec = l_minute / 60;
        vol = vol + l_sec;
        flow_frequency = 0; // Reset Counter
      }
      else
      {
        l_minute = 0;
        l_sec = 0;
      }
    }

    //Serial.println(pososto);
    //Serial.print("#");
    //Serial.print(l_minute);
    //Serial.print("#");
    //Serial.println(vol);
    // Αποστολή όλων των δεδομένων σε μορφή κειμένου
    dtostrf(l_minute, 0, 3, floatStr1); // μετατροπή του float σε string
    dtostrf(vol, 0, 3, floatStr2);
    sprintf(buf, "%d#%s#%s", pososto, floatStr1, floatStr2);  // συνένωση όλων των δεδομένων σε ένα string, χωρισμένων με #
    Serial.print(buf);
  }

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH)
  {
    digitalWrite(ledPin, HIGH);
    digitalWrite(relayPin, LOW);  // ξεκινά η αντλία
  }
  else
  {
    digitalWrite(ledPin, LOW);
    digitalWrite(relayPin, HIGH); // σταματά η αντλία
  }       

//  delay(50);
} 
