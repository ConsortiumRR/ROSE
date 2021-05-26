
//____________________________________________________________________________________________________//
//-----------------------------------------------ROSE-HUB-NODE-RED------------------------------------//
//____________________________________________________________________________________________________//


#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
//____________________Unit_Name____________________________________

char unitName[] = "robotInterface";

DynamicJsonDocument  parseJson(200);

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient roseHub;
PubSubClient client(roseHub);

//____________________Subscribed_to_____________________________

const char Input[] = "rose/hub_to_robotInterface";


//____________________Publishing_to_____________________________
const char Output[] = "rose/robotInterface_to_hub";


//===========================================//

//------------Router/Server info--------------//
char* ssid = "CRR_2.4GHz";
char* password = "abbirb6700";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
char* mqtt_server = "192.168.50.47";



//----------------------------------------------------------------------------------------------------//




//---------------------------------------ROBOT-COMS-VARIABLES-----------------------------------------//

int numBits = 4;

int READPIN = 36;
int bit0 = 23;
int bit1 = 16;
int bit2 = 19;
int bit3 = 18;

int doPin[] = {bit0, bit1, bit2, bit3};

bool getWateringArray = false;
bool wateringArray[3];

unsigned long pulseDuration;




void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  //set pins to output and input
  for (int indexPin = 0; indexPin < numBits; indexPin++) {
    pinMode (doPin[indexPin], OUTPUT);
  }

  pinMode(READPIN, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(unitName)) {
      Serial.println("connected");
      // Subscribe
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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();


  if (String(topic) == Input) {

    DeserializationError error = deserializeJson(parseJson, messageTemp);
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }


  }

  wateringArray[0] = parseJson["wateringArray"][0];
  wateringArray[1] = parseJson["wateringArray"][1];
  wateringArray[2] = parseJson["wateringArray"][2];

  Serial.print("wateringArray 0 = ");
  Serial.println(wateringArray[0]);
  Serial.print("wateringArray 1 = ");
  Serial.println(wateringArray[1]);
  Serial.print("wateringArray 2 = ");
  Serial.println(wateringArray[2]);
}




void loop() {



 if (!client.connected()) {
    reconnect();
  }
  if (!client.loop())
    client.connect(unitName);
  //updating an array of bools. 1 = unit switched on. 0 = unit switched off.


  //messages coming from the robot
  pulseDuration = pulseIn(READPIN, HIGH);

//robot tells interface to get watering array from hub 
  if (pulseDuration >= 5000 && pulseDuration <= 10000) {
      
     Serial.println("get array from node red");
     client.publish(Output, "true");
     
  }
//Robot tells interface to tell it what units need watering. 
  if (pulseDuration >= 10000 && pulseDuration <= 15000) {
   

    for(int i = 0; i<sizeof(wateringArray); i++){ 
        if (wateringArray[i] == true)
        {
           displayBinary(i+1);
           delay(500);
           displayBinary(0);
           wateringArray[i] = false; 
           break;
        }
      
    }
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
