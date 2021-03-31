#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>

//enter in moisture threshold (if unit returns a value below, bot will water it)
int moistureThreshold = 600;

//===========================================//

//------------ESP Now Init------------------//


// REPLACE WITH HUB MAC Address

uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x85, 0x65, 0x24};

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int id;
  int moistureLevel;
  bool isTooDry;
} struct_message;

// Create a struct_message called test to store variables to be sent
struct_message myData;


// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

//===========================================//


//loop timer planter unit sends data to hub every 5 seconds
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

// tells lights to turn on every 5 minutes
unsigned long pLightsCycle = 0;
unsigned long lightsCycle = 3000;

//pins
int moisturePin = A0;
int relayPin = D5;
int redLed = D6;
int blueLed = D7;

// vars
bool lightStatus = HIGH;

void setup() {

  pinMode(moisturePin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  // Init Serial Monitor
  Serial.begin(115200);


  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Once ESPNow is successfully init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  myData.id = BOARD_ID;
  //==============================================



}

void loop() {

  if ((millis() - lastTime) > timerDelay) {

//checks moisture levels, sends 'true' to hub if moisture is below threshold  
    int moistureLevel = analogRead(moisturePin);
    Serial.print("moistureLevel is: ");
    Serial.println(moistureLevel);

/*
 
* Moisture Sensor info: 
* these capacitive sensors return reverse values: 
* Wet = lower numbers, Dry = higher numbers. 
* A 100% dry sensor returns a value of ~866 at room temperature. 
* A completely submerged sensor returns a value ~528 at room temperature
  
*/
   if (moistureLevel > moistureThreshold)
    {
      myData.isTooDry = true;
      digitalWrite(redLed, HIGH);
      digitalWrite(blueLed, LOW);
    }

    else
    {
      myData.isTooDry = false;
      digitalWrite(redLed, LOW);
      digitalWrite(blueLed, HIGH);
    }
    
 // Sends data via ESP-NOW
    esp_now_send(0, (uint8_t *) &myData, sizeof(myData));

    lastTime = millis();
  }

//every 5 minutes, Unit turns its lights on of off (for demo purposes)

if ((millis() - pLightsCycle) > lightsCycle)
{
  if(lightStatus == HIGH){
    lightStatus = LOW;
  }
  else {
    lightStatus = HIGH;
  }
  
}
digitalWrite(relayPin, HIGH); 
}
