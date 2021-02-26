int pin = D1;
unsigned long duration;

void setup() {
  Serial.begin(115200);
  pinMode(pin, INPUT);
}

void loop() {
  duration = pulseIn(pin, HIGH);
  Serial.println(duration);
}
