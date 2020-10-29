#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <RtcDS3231.h>

RtcDS3231<TwoWire> Rtc(Wire);

//--------------User Variables----------------//

//SET THE TIMES FOR GROW LIGHT TO TURN ON AND OFF.
//24h clock. formatted 'HHMM'. ie. 5:30PM = 1730.

int growLightOnTime = 1430;
int growLightOffTime = 1720;

//enter in moisture threshold (if unit returns a value below, bot will water it)
int moistureThreshold = 400;

//===========================================//

//------------ESP Now Init------------------//


// REPLACE WITH HUB MAC Address
uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x85, 0x70, 0x14};

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int id;
  float moistureLevel;
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
unsigned long timerDelay = 5000;

//pins
int moisturePin = A0;
int relayPin = D5;




void setup() {

  pinMode(moisturePin, INPUT);
  pinMode(relayPin, OUTPUT);

  // Init Serial Monitor
  Serial.begin(115200);


  //----------------RTC related setup-------------------

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!Rtc.IsDateTimeValid())
  {
    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  RtcDateTime now = Rtc.GetDateTime();

  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  //relay port. Turns to high when RTC readings are between "growLightOnTime" and "growLightOffTime"
  pinMode(D2, OUTPUT);

  //==============================================

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

  //==============================================



}

void loop() {

  if ((millis() - lastTime) > timerDelay) {

//checks moisture levels, sends 'true' to hub if moisture is below threshold  
    int moistureLevel = analogRead(moisturePin);
    Serial.print("moistureLevel is - ");
    Serial.println(moistureLevel);

    if (moistureLevel < moistureThreshold)
    {
      myData.id = BOARD_ID;
      myData.isTooDry = true;
    }

    else
    {
      myData.id = BOARD_ID;
      myData.isTooDry = false;
    }
    
 // Sends data via ESP-NOW
    esp_now_send(0, (uint8_t *) &myData, sizeof(myData));



  /*  checks if time is within hours given by the unit  */
  //======================================================
  /* plug RTC in to D1 and D2 on the esp8266 (SDA and SCL) */
  
    RtcDateTime now = Rtc.GetDateTime();
    
    int nowTime = now.Hour() * 100 + now.Minute();
    

    if (nowTime == growLightOnTime) 
    {
      digitalWrite(relayPin, HIGH);
    }
    if (nowTime == growLightOffTime)
    {
      digitalWrite(relayPin, LOW);
    }
//=======================================================+

    lastTime = millis();
  }



}
