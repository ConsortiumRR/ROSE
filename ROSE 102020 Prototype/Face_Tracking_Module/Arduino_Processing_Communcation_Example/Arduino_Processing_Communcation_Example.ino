int led_pin = 13;
char state;
void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(9600);
}
void loop() {
  if (Serial.available() > 0) {
    state = Serial.read();
    if (state == '1') {
      digitalWrite(led_pin, HIGH);
    }
    if (state == '0') {
      digitalWrite(led_pin, LOW);
    }
  }
  delay(50);

//digitalWrite(led_pin, HIGH);
//
//delay(1000);
//
//digitalWrite(led_pin, LOW);
//
//delay(1000);
}
