
//____________________________________________________________________________________________________//
//-----------------------------------------------ROSE-HUB-NODE-RED------------------------------------//
//____________________________________________________________________________________________________//


#include <PubSubClient.h>
#include <WiFi.h>

//____________________Unit_Name____________________________________

const char unitName[] = "roseHub";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient roseHub;
PubSubClient client(roseHub);

//____________________Subscribed_to_____________________________

const char getWaterMeList[] = "rose/hub_to_robotInterface";


//____________________Publishing_to_____________________________
const char sendMeList[] = "rose/robotInterface_to_hub";


//===========================================//

//------------Router/Server info--------------//

const char* ssid = "CRR_2.4GHz";
const char* password = "abbirb6700";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.50.153";






//----------------------------------------------------------------------------------------------------//




//---------------------------------------ROBOT-COMS-VARIABLES-----------------------------------------//
int numUnits = 3;
int numBits = 4;

int READPIN = 36;
int bit0 = 23;
int bit1 = 16;
int bit2 = 19;
int bit3 = 18;

int doPin[] = {bit0, bit1, bit2, bit3};
bool queryRecieved = false;


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

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == waterMe) {
    Serial.print("Changing output to ");
    if(messageTemp == "true"){
      Serial.println("waterMe");
    
    }
    else if(messageTemp == "false"){
      Serial.println("dont waterMe");
     
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("unitName")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(waterMe);
      client.subscribe(getMe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  //updating an array of bools. 1 = unit switched on. 0 = unit switched off.
  
  
  //messages coming from the robot
  pulseDuration = pulseIn(READPIN, HIGH);

  if(pulseDuration >= 4500 && pulseDuration <= 5500){
    queryRecieved = true;
  }
  
  if (queryRecieved == true) {
    Serial.println("query recieved");

    for(int i = 0; i<numUnits; i++){
      
        
        //sets 'queryRecieved' to false to enforce one input/one output
        queryRecieved == false;

        //prints
        Serial.print("go to unit ");
        Serial.print(i+1);
        Serial.println();
      
        displayBinary(i+1);  
        delay(500);
        displayBinary(0);
        
        break;
      }
    }  
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
