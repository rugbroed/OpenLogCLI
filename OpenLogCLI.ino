// Author --> Kasper Benjamin Hansen [kbhdk1976@gmail.com]
//
// OpenLogCLI (v0.5) is an Arduino serial monitor supported OpenLog filesystem and command browser using
// CLI (Command Line Interpreter)
// 
// Required: There should be a CONFIG.TXT in the root of the card with the following; "9600,26,3,2".
// 
// Wire your boards like this;
//
// OpenLog.GRN --> Arduino.4
// OpenLog.RXI --> Arduino.5
// OpenLog.TXO --> Arduino.6
// OpenLog.VCC --> Arduino.3,3v
// OpenLog.GND --> Arduino.GND
//
// Start out by uploading this sketch to your Arduino, open the serial monitor and press "?" [enter].
//
// :-) Enjoy

#include <SoftwareSerial.h>
#include <avr/pgmspace.h>˛

#define sensorRX A0

#define openLogRX 6
#define openLogTX 5
#define openLogRST 4
#define ledPWR 13
#define RECEIVE_BUFFER_SIZE 1000
#define TERMINATION "$q"

SoftwareSerial OpenLog(openLogRX, openLogTX);

String inputString;
char c;

void setup() {
  pinMode(sensorRX, INPUT);
  pinMode(ledPWR, OUTPUT);
  pinMode(openLogRST, OUTPUT);
  
  OpenLog.begin(9600);
  Serial.begin(9600);
  
  doResetOpenLog();
  getCommandMode();
  
  //TODO Read termination string from CONFIG.TXT
}

void doCommand(String command) {
  olPrint(command);
  getCommandMode();
}

void olPrint(String s) {
  OpenLog.print(s);
  OpenLog.write(13);
}

void flipMode() {
  Serial.println("Flipping mode.");

  OpenLog.write(26);
  OpenLog.write(26);
  OpenLog.write(26);
  
  getCommandMode();
}

void getCommandMode() {
  Serial.println("Determining mode...");
  
  boolean conti = true;
  while(conti) {
    if(OpenLog.available()) {
      c = OpenLog.read();

      if (c == '>' || c == '<') {
        conti = false;
        break;
      }
    }
  }

  if (c == '>') {
    Serial.println("Mode is COMMAND.");
  }
  else {
    Serial.println("Mode is USAGE.");
  }
}

void doResetOpenLog() {
  digitalWrite(openLogRST, LOW);
  delay(100);
  digitalWrite(openLogRST, HIGH);
  delay(100);
}

char* readStringFromOpenLog() {
  //TODO Make a dynamic buffer
  
  char PROGMEM chars[RECEIVE_BUFFER_SIZE];
  
  int count = 0;
  
  for (int u = 0; u < RECEIVE_BUFFER_SIZE; u++) {
    chars[u] = 0;
  }
  
  for(int timeOut = 0 ; timeOut < 2000 ; timeOut++) {
    
    while(OpenLog.available()) {

      byte intRead = OpenLog.read();
      char c = intRead;
      
      if (c != '\r') {
        if (intRead >= 32 || c == '\n') {
          chars[count] = c;
          count++;
        }
      }

      timeOut = 0;
    }
    delay(1);
  }
  
  return chars;
}

String readFromSerial() {
  if (Serial.available() > 0) {
    String resultString;

    while (Serial.available() > 0) {
      int incomingByte = Serial.read();

      if (incomingByte != '\n' && incomingByte != '\r') {
        char c = (char) incomingByte;
        resultString += c;
        delay(1);
      }
    }

    return resultString;
  }
  else return NULL;
}

void loop() {
  inputString = String(readFromSerial());
  if (inputString != NULL) {
    if (inputString == TERMINATION) {
      flipMode();
    }
    else {
      olPrint(inputString);
  
      char* chars = readStringFromOpenLog();
  
      Serial.println(chars);
      chars = NULL;
    }
  }
}
