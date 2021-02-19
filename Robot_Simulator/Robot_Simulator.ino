int pulselengths[] = {5000, 10000, 15000, 20000, 25000}; //set pulse lengths
const char  pulsecommand[] = {"NONE", "FORWARD", "RETRACT", "STOP", "PREHEAT", "OFF"};

int potPin = 2;
int buttonPin = 3;
int pulsePin = 4;


int potReading;
int buttonState;
int lastButtonState = LOW;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;


void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT);
  pinMode(potPin, INPUT);
}

void loop() {

  int button = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (button != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (button != buttonState) {
      buttonState = button;
    }
  }

  if (buttonState == HIGH) {

    potReading = analogRead(potPin);
    potReading = map(potReading, 0, 1023, 0, 5);


    switch (potReading) {

      case  0:
        Serial.println(pulsecommand[potReading]);
        digitalWrite(pulsePin, HIGH);
        delayMicroseconds(potReading);
        digitalWrite(pulsePin, LOW);
        break;

      case  1:
        Serial.println(pulsecommand[potReading]);
        digitalWrite(pulsePin, HIGH);
        delayMicroseconds(potReading);
        digitalWrite(pulsePin, LOW);
        break;

      case  2:
        Serial.println(pulsecommand[potReading]);
        digitalWrite(pulsePin, HIGH);
        delayMicroseconds(potReading);
        digitalWrite(pulsePin, LOW);
        break;

      case  3:
        Serial.println(pulsecommand[potReading]);
        digitalWrite(pulsePin, HIGH);
        delayMicroseconds(potReading);
        digitalWrite(pulsePin, LOW);
        break;

      case  4:
        Serial.println(pulsecommand[potReading]);
        digitalWrite(pulsePin, HIGH);
        delayMicroseconds(potReading);
        digitalWrite(pulsePin, LOW);
        break;

      case  5:
        Serial.println(pulsecommand[potReading]);
        digitalWrite(pulsePin, HIGH);
        delayMicroseconds(potReading);
        digitalWrite(pulsePin, LOW);
        break;
    }
    delay(1000);

  }

  lastButtonState = button;
}
