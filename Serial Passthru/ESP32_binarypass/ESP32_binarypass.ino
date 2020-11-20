#include <HardwareSerial.h>


//---------------------------------------ROBOT-COMS-VARIABLES-----------------------------------------//

bool Mode_pulsein = false; 
bool Mode_binarypass = true;

int numiBits = 4;
int numoBits = 4;

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

//Pulse input mode variables -- Mode_pulsein should be true for using pulsed inputs and false for parallel inputs


int READPIN = 36;
// variable for received pulse duration, target pulse durations and tolerance for target pulse durations
unsigned long pulseDuration;
int pulselengths[] = {5000, 10000, 15000, 20000}; 
int pulsetolerance = 500;

//Serial Pass variables for sending robot signals to other devices
byte prevByte = 0; 
byte inputByte = 0;

int UARTbaud = 9600;  
HardwareSerial UART2(2);

//-------------------------------------------------------------------------------//




void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  UART2.begin(UARTbaud, SERIAL_8N1, 16, 17);

  //set binary out pins to output
  for (int indexPin = 0; indexPin < numoBits; indexPin++) {
    pinMode (doPin[indexPin], OUTPUT);
  }
  // set read pin or binary in pins to input
  if (Mode_pulsein == true){
    pinMode(READPIN, INPUT);
    }
  else{
    for (int indexPin = 0; indexPin < numiBits; indexPin++) {
    pinMode (diPin[indexPin], INPUT);
    }
  }
}

void loop() {
  // go to designated read mode
 if (Mode_pulsein == true){
  ReadPulseIn; 
 }
 else if (Mode_binarypass == true){
  BinaryPass; 
 }
}


void BinaryPass(){
  int message; 
  // check for a new byte val from robot until a new one is sent
  while (inputByte == prevByte){
    for (int i = 0; i<numiBits; i++){
      if (digitalRead(diPin[i]) == 1){
      inputByte |= (1<<i);
      }
    }
  }
  // set byte vals equal
  prevByte = inputByte; 
  message = int(inputByte);
  Serial.print("sending byte:");
  Serial.println(inputByte);
  //send value over Uart 
  UART2.write(message); 

}

void ReadPulseIn(){
  // Mode for reading a pulse on a input pin to receive queries from bot 
  //coded number for type of query received from robot -- def need a more elegant setup for this for different types of queries
  // each query should point to some kind of function like sending binary numbers back to robot
    int inputquery = 0; 

    // read pulse and check if it is any of the target pulse lengths
    pulseDuration = pulseIn(READPIN, HIGH);
    for (int i=0; i<4; i++){
      if (pulseDuration >= pulselengths[i]-pulsetolerance && pulseDuration <= pulselengths[i]+pulsetolerance){
        inputquery = i+1; 
        Serial.print("query received: requesting routine ");
        Serial.println(inputquery);
      }
      else{
        inputquery = 0;
      }
    }
    // initial setup for sendBinary input query
    if (inputquery == 1){
      int sendnum = 1; 
      //need some kind of get the number to send function
      displayBinary(sendnum);  
      delay(500);
      displayBinary(0);    
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
