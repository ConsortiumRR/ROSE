
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

int numiBits = 4;
int numoBits = 4;
int pBinaryNum = 0;
//input (ABB-> Hub) IO binary pins
int ibit0 = 36;
int ibit1 = 39;
int ibit2 = 34;
int ibit3 = 35;
//output (Hub->ABB) IO binary pins
int obit0 = 23;
int obit1 = 22;
int obit2 = 21;
int obit3 = 18;

int doPin[] = {obit0, obit1, obit2, obit3};
int diPin[] = {ibit0, ibit1, ibit2, ibit3};

bool getWateringArray = false;
bool wateringArray[3];

unsigned long pulseDuration;




void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  //set pins to output and input
  for (int indexPin = 0; indexPin < numiBits; indexPin++) {
    pinMode (doPin[indexPin], OUTPUT);
  }

  for (int indexPin = 0; indexPin < numoBits; indexPin++) {
    pinMode (diPin[indexPin], INPUT);
  }

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


  //robot tells interface to get watering array from hub
  if (readBinary() == 1) {

    Serial.println("get array from node red");
    client.publish(Output, "true");

  }
  //Robot tells interface to tell it what unit needs watering.
  if (readBinary() == 2) {


    for (int i = 0; i < sizeof(wateringArray); i++) {
      if (wateringArray[i] == true)
      {
        displayBinary(i + 1);
        delay(500);
        displayBinary(0);
        wateringArray[i] = false;
        break;
      }

    }
  }
}


int readBinary() {
  // create an int to store robot input
  int binaryNum = 0;
  for (int i = 0; i < numiBits; i++) {
    // read binary pin input and convert to a number
    if (digitalRead(diPin[i]) == HIGH) {
      binaryNum += pow(2, i);
    }
  }

  if (binaryNum != pBinaryNum) {
    return binaryNum;
  }
  else {
    return 0;
  }

}

void displayBinary(byte number) {

  for (int i = 0; i < numoBits; i++) {
    if (bitRead(number, i) == 1) {
      digitalWrite(doPin[i], HIGH);
    }
    else {
      digitalWrite(doPin[i], LOW);
    }
  }
}
