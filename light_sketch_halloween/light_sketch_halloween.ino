#include <Adafruit_CircuitPlayground.h>


int colorPulsePin = A6;
int onOffPin = A1;
unsigned long pulseDuration;

void setup() 
{
  Serial.begin(115200);
  CircuitPlayground.begin();
  

  pinMode(colorPulsePin, INPUT);
  pinMode(onOffPin, INPUT);
  
}

void loop()
{
  
  //pulseDuration = pulseIn(colorPulsePin, HIGH);
   //Serial.println(pulseDuration);
  
  
  if (digitalRead(onOffPin)==LOW)
  {
     for(int x = 0; x<10;x++){
      CircuitPlayground.setPixelColor(x,0,0,0);
     }
     Serial.println("low");
  }
  
  if (digitalRead(onOffPin)==HIGH){
  
  for(int y = 0; y<10; y++)
      { 
        CircuitPlayground.setPixelColor(y,255,140,0);
      }
   Serial.println("high");
    
//     if (4500 > pulseDuration > 5500) //Orange -> 5 millisecond pulse from bot
//     {
//      for(int y = 0; y<10; y++)
//      { 
//        CircuitPlayground.setPixelColor(y,255,140,0);
//      }
//      
//     }
//     
//     if (9500 > pulseDuration > 10500) //yellow -> 10 millisecond pulse from bot
//     {
//      for(int z = 0; z<10 ; z++)
//      { 
//        CircuitPlayground.setPixelColor(z,244,232,104);
//      }
//      
//     }
  }
  

}
