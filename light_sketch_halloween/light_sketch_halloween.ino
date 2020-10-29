#include <Adafruit_CircuitPlayground.h>


int colorPulsePin = A6;
int onOffPin = A1;
unsigned long pulseDuration;

void setup() 
{
  CircuitPlayground.begin();

  pinMode(colorPulse, INPUT);
  pinMode(READPIN2, INPUT);
  
}

void loop()
{
  pulseDuration = pulseIn(READPIN2, HIGH);
  
  if (digitalRead(onOffPin)==LOW){
     CircuitPlayground.fill(0);
  }
  else
  {
     if (1500 > pulseDuration > 2500) //Orange -> 2 millisecond pulse from bot
     {
      CircuitPlayground.fill(255,140,0);
     }
     if (2500 > pulseDuration > 3500) //yellow -> 3 millisecond pulse from bot
     {
      CircuitPlayground.fill(244,232,104);
     }
  }
  
strip.show();

}
