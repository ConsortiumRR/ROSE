#include <esp_now.h>
#include <WiFi.h>
#include <CircularBuffer.h>

CircularBuffer<byte, 10> buffer;
//------------------------------------------ESP-NOW-VARIABLES-----------------------------------------//

// Below is a data structure is defined which allows the ESP32 to receive wireless messages from the senders
// It can be modified to receive whatever variable types you want - floats, strings, char, etc
// This structure must match the sender structure


typedef struct struct_message {
  byte id;
  bool x;
} struct_message;



// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each unit
struct_message unit1;
struct_message unit2;
struct_message unit3;
struct_message unit4;
struct_message unit5;


// Create an array with all the structures
struct_message unitsStruct[] = {unit1, unit2, unit3, unit4, unit5};


void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {

  memcpy(&myData, incomingData, sizeof(myData));
  
//  Serial.printf("unit ID %u: %u bytes\n", myData.id, len);
  
  // Update the structures with the new incoming data 
  unitsStruct[myData.id - 1].x = myData.x;

  Serial.print(myData.id);
  Serial.print(", ");
  Serial.println(myData.x);
   
  if(myData.x == true){
     buffer.unshift(myData.id);
}
}

//----------------------------------------------------------------------------------------------------//




//---------------------------------------ROBOT-COMS-VARIABLES-----------------------------------------//
int numUnits = 6;
int numBits = 4;

int READPIN = 36;
int bit0 = 23;
int bit1 = 16;
int bit2 = 19;
int bit3 = 18;

int doPin[] = {bit0, bit1, bit2, bit3};
bool queryRecieved = false;

byte unit = 0;

unsigned long pulseDuration;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  //set pins to output and input
  for (int indexPin = 0; indexPin < numBits; indexPin++) 
  {
    pinMode (doPin[indexPin], OUTPUT);
  }

  pinMode(READPIN, INPUT);


  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);


}

void loop() {
 
  
  //messages coming from the robot
  pulseDuration = pulseIn(READPIN, HIGH);

  if(pulseDuration >= 4500 && pulseDuration <= 5500){
    queryRecieved = true;
  }
  
  if (queryRecieved == true) {
    Serial.println("query recieved");

        //sets 'queryRecieved' to false to enforce one input/one output
        queryRecieved == false;
        
        // shift unit number to circular buffer
        unit = buffer[0];

       if (unitsStruct[unit].x == true)
       {
       
        Serial.print("go to unit: ");
        Serial.println(unit);

        Serial.print("buffer order: ");
       
          Serial.print(buffer[0]);
          Serial.print(", ");
          
          Serial.print(buffer[1]);
          Serial.print(", ");
          
          Serial.print(buffer[2]);
          Serial.print(", ");
        }
        Serial.println();
            
        displayBinary(unit);  
        delay(500);
        displayBinary(0);
        
      }
      
 
  
  else if (queryRecieved == false)
  { 
        displayBinary(0);
  }

  
}

void displayBinary(byte number) {

  for (int i = 0; i < numBits; i++) {
    if (bitRead(number, i) == 1) {
      digitalWrite(doPin[i], HIGH);
    }
    else {
      digitalWrite(doPin[i], LOW);
    }
  }
}
