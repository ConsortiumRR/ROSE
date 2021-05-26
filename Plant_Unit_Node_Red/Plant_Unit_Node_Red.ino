
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//3 mqtt topics involved in this sketch
//esp8266 subscribes to "unit#/lightSwitch" and "unit#/setMoistureThreshold"
//and publishes moisture data to "unit#/moistureLvl"

//modify name of unit, and list topics to subscribe to 

//____________________Unit Name____________________________________

const char unitName[] = "unit2";
//const char unitName[] = "unit2";


// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your automation system
WiFiClient unit2;
PubSubClient client(unit2);

//____________________Subscribed_to_____________________________

const char Light_Switch[] = "unit2/lightSwitch";
const char Moisture_Threshold[] = "unit2/moistureThreshold";
const char Readings_Frequency[] = "unit2/readingsFreq";
const char Wet_Reading[] = "unit2/wetReading";
const char Dry_Reading[] = "unit2/dryReading";
const char Get_Raw_Reading[] = "unit2/getRawReading";

//const char Light_Switch[] = "unit2/lightSwitch";
//const char Moisture_Threshold[] = "unit2/moistureThreshold";

//____________________Publishing_to____________________________

const char Moisture_Level[] = "unit2/moistureLvl";
const char Send_Raw_Reading[] = "unit2/sendRawReading";
//const char Moisture_Level[] = "unit2/moistureLvl";


    /*

      Moisture Sensor info:
      these capacitive sensors return reverse values:
      Wet = lower numbers, Dry = higher numbers.
      
      Test your sensor before you use it.
      For example, the unit 1 sensor when 100% dry returns a value of ~730 at room temperature.
      completely submerged it returns a value ~350 at room temperature.

      However, to prevent confusion the sketch will publish values from 0-100,
      0 being dry, 100 being submerged.
    */


//Sensor Callibration 


int dryVal = 730;
int wetVal = 350;
//int dryVal = 730;
//int wetVal = 350;


//moistureThreshold may be changed later from NodeRed  
int moistureThreshold = 30;


//loop timer planter unit publishes data to MQTT every x milliseconds
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// tells lights to turn on every 5 minutes
//unsigned long pLightsCycle = 0;
//unsigned long lightsCycle = 3000;

//pins
int moisturePin = A0;
int relayPin = D5;
int redLed = D6;
int blueLed = D7;

// vars
bool lightStatus = HIGH;

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
      
      
      client.subscribe(Light_Switch);
      client.subscribe(Moisture_Threshold);
      client.subscribe(Readings_Frequency);
      client.subscribe(Dry_Reading);
      client.subscribe(Wet_Reading);
      client.subscribe(Get_Raw_Reading);


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
  Serial.println();

  if (topic == Light_Switch) {
    Serial.print("Changing Room lamp to ");
    if (messageTemp == "true") {
      digitalWrite(D5, HIGH);

    }
    else if (messageTemp == "false") {
      digitalWrite(D5, LOW);
    } 
  }

  if (topic == Get_Raw_Reading){
    float moistureLevel = analogRead(moisturePin);
    static char moistureLvl[7];
    dtostrf(moistureLevel, 6, 2, moistureLvl);
    client.publish(Send_Raw_Reading, moistureLvl);
    Serial.println(moistureLevel);
    
  }
  if (topic == Moisture_Threshold){
    
      moistureThreshold = messageTemp.toInt();
      Serial.print("threshold changed to:  ");
      Serial.println(moistureThreshold);    
  }
   
  if (topic == Readings_Frequency){
    
      timerDelay = messageTemp.toInt();
      Serial.print("readings frequency in milliseconds:  ");
      Serial.println(timerDelay);    
  }

  if (topic == Wet_Reading){
    
      wetVal = messageTemp.toInt();
      Serial.print("wet reading:  ");
      Serial.println(wetVal);    
  }
  if (topic == Dry_Reading){
    
      dryVal = messageTemp.toInt();
      Serial.print("dry reading:  ");
      Serial.println(dryVal);    
  }
  
  Serial.println();
}






//===========================================//




void setup() {

  pinMode(moisturePin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  // Init Serial Monitor
  Serial.begin(115200);


  //connect to mqtt server, set callback function
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);




}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (!client.loop())
    client.connect(unitName);


if ((millis() - lastTime) > timerDelay) {

    float moistureLevel = analogRead(moisturePin);

    // map function normalizes moisture values with 0 being dry, 100 being submerged.
   
   moistureLevel = map(moistureLevel,dryVal,wetVal,0,100);
    
    Serial.print("moistureLevel is: ");
    Serial.println(moistureLevel);

    static char moistureLvl[7];

    dtostrf(moistureLevel, 6, 2, moistureLvl);
    
    
  
    client.publish(Moisture_Level, moistureLvl);

  

    if (moistureLevel > moistureThreshold)
    {
      digitalWrite(redLed, HIGH);
      digitalWrite(blueLed, LOW);
    }

    else
    {
      digitalWrite(redLed, LOW);
      digitalWrite(blueLed, HIGH);
    }

    lastTime = millis();
  }


}
