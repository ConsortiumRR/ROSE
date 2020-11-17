#include <ESP8266WiFi.h>
#include <espnow.h>

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x85, 0x65, 0x24};

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 2


// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int id;
  bool x;
} struct_message;

// Create a struct_message called test to store variables to be sent
struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

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

//pins
int switchPin = D2;
int ledPin = D1;

//Neopixel stuff

#include <Adafruit_NeoPixel.h>
#define NUMPIXELS 16
Adafruit_NeoPixel pixels(NUMPIXELS, ledPin, NEO_GRB + NEO_KHZ800);

void setup() {

  pinMode(switchPin, INPUT);
  pinMode(ledPin, OUTPUT);

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

  pixels.begin();
}

void loop() {
  
    // Set values to send

    if (digitalRead(switchPin) == HIGH) {

      for(int i=0; i<NUMPIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.ColorHSV(5*millis()));
        pixels.show();
       }
       
      myData.id = BOARD_ID;
      myData.x = true;
    }
    else
    {
      pixels.clear();
      pixels.show();
      myData.id = BOARD_ID;
      myData.x = false;
    }

  
if ((millis() - lastTime) > timerDelay) {
 
    // Send message via ESP-NOW
    esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
    lastTime = millis();
  }
  
delay(10);
  
}
