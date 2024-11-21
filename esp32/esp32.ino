#include<WiFi.h>
#include<WebServer.h>

// Reference: https://github.com/kosaladeshan/IOT-Group-Project/blob/main/esp32/esp32.ino

#define RXpin2 16
#define TXpin2 17

const char* ssid = "";
const char* password = "";

void connectWifi();

void setup() {
  // Serial 1
  Serial.begin(115200);

  connectWifi();

  // Serial 2 : for Arduino communicate
  Serial2.begin(9600, SERIAL_8N1, RXpin2, TXpin2);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    // If not connected to wifi, Try reconnect
    Serial.println("WiFi is not connected. Reconnecting...");
    connectWifi();
  }
  else {
    if (Serial2.available() > 0) {
      if (false) {
        // Invalid input from serial monitor
        Serial.println("Invalid input from Serial Monitor.");
        return;
      }

      // ...

    }
  }

  delay(10000);
}

void connectWifi() {
  WiFi.mode(WIFI_OFF);
  delay(10000);
  
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wifi");

  while(WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.println("Attemping to connect...");
  }

  Serial.print("Connect to: "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());

}
