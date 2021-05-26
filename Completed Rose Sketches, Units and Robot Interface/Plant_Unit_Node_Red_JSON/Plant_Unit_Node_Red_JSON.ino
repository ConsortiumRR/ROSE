
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


//____________________Unit Name____________________________________

const char unitName[] = "unit1";

//JSON object
DynamicJsonDocument  parseJson(200);


// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your automation system
WiFiClient unit1;
PubSubClient client(unit1);

//____________________Subscribed_to_____________________________


const char Input[] = "rose/unit1_from_hub";


//____________________Publishing_to____________________________

const char Output[] = "rose/unit1_to_hub";

//____________________Sensor_Callibration____________________________
/*

   Moisture Sensor info:
   these capacitive sensors return reverse values:
   Wet = lower numbers, Dry = higher numbers.

   Test your sensor before you use it.
   Submerge in cup of water, check Serial, and set "wetVal"
   Dry off, check serial, and set "dryVal"

   For example, the unit 1 sensor when 100% dry returns a value of ~730 at room temperature.
   completely submerged it returns a value ~350 at room temperature.

   However, to prevent confusion the sketch will publish values from 0-100,
   0 being dry, 100 being submerged.
*/

int wetVal = 350;
int dryVal = 730;






//loop timer planter unit publishes data to MQTT every x milliseconds
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

//Indicators and Grow light
bool waterMe;
bool growLight;
int readingInterval = timerDelay;
//pins
int moisturePin = A0;
int relayPin = D5;

int redLed = D6;
int greenLed = D7;



//===========================================//

//------------Router/Server info--------------//
const char* ssid = "CRR_2.4GHz";
const char* password = "abbirb6700";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.50.47";



//------------Connect to Router------------------//


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

//-----------establish MQTT connection and subscribe to topics----------//

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    //change client name to fit correct unit



    if (client.connect(unitName)) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)


      client.subscribe(Input);


    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//------------MQTT receive message callback function------------------//

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.print(messageTemp);
  Serial.println();

  if (topic == Input) {
  // topic variable is "rose/unit1_from_hub"
    
    // Deserialize the JSON document

    DeserializationError error = deserializeJson(parseJson, messageTemp);
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

  }
  waterMe = parseJson["waterMe"];
  growLight = parseJson["growLight"];
  readingInterval = parseJson["readingInterval"];
 

  if (waterMe == true) {
      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);
  }
  
  if (waterMe == false) {
      digitalWrite(redLed, LOW);
      digitalWrite(greenLed, HIGH);
  }
  
  if (growLight == 0){
   digitalWrite(relayPin,LOW);
   Serial.println("growlight off");
  }
  
  if (growLight == 1){
   digitalWrite(relayPin,HIGH);
   Serial.println("growlight on");
  }


  Serial.println();
}






//===========================================//




void setup() {

  // Init Serial Monitor
  Serial.begin(115200);


  //connect to mqtt server, set callback function
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(moisturePin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (!client.loop())
    client.connect(unitName);


  if ((millis() - lastTime) > timerDelay) {

    float moistureLevel = analogRead(moisturePin);
    moistureLevel = map(moistureLevel, dryVal, wetVal, 0, 100);
    Serial.print("moistureLevel is: ");
    Serial.println(moistureLevel);
    static char moistureLvl[7];
    dtostrf(moistureLevel, 6, 2, moistureLvl);
    client.publish(Output, moistureLvl);

    lastTime = millis();
  }

  if (readingInterval != timerDelay){
    timerDelay = readingInterval*1000;
  }
  


}
