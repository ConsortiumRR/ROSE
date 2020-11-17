

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define WIFI_SSID "FiOS-AN65C"
#define WIFI_PASSWORD "top649ale0409rely"

// Raspberri Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 1, 54)
// For a cloud MQTT broker, type the domain name
//#define MQTT_HOST "example.com"
#define MQTT_PORT 1883

#define MQTT_PUB_ONOFF "uselessMachine/node4"

//variable
bool onOff;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long lastTime = 0;
unsigned long timerDelay = 1000;


void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
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

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
 
  connectToWifi();



  pixels.begin();
}

void loop() {

  // Set values to send

  if (digitalRead(switchPin) == HIGH) {
    
    onOff = true;
    
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.ColorHSV(5 * millis()));
      pixels.show();
    }


  }
  else
  {
    pixels.clear();
    pixels.show();
    onOff = false;
   
  }


  if ((millis() - lastTime) > timerDelay) {
    
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_ONOFF, 1, true, String(onOff).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_ONOFF, packetIdPub2);
    Serial.printf("Message: %.2f \n", onOff);
    
    lastTime = millis();
  }



}
