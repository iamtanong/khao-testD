#include <Arduino.h>
// Define Pin
#define MOISTURE_SENSOR_PIN A0
#define SERVO_PIN 9
#define VIBRATION_SENSOR_PIN 2
#define GAS_SENSOR_PIN A1

// Functoin pre-declaration
void readMoisture();
void readDistance();
void readGas();
void readVibration();
void spinBase(int distance);
void openGate();

void setup()
{
  // communicate with esp32
  Serial.begin(9600);

  // Servo
  pinMode(sensorPin, INPUT);
  sg90.attach(7);
  sg90.write(0); // set zero the gate


}

void loop()
{
  String waste_type = "";

  // Step 1 : Vibration detection || Button pushing
  int vibrationValue = readVibration();

  if( vibrationValue >= 700){ // the sensor detects high value of vibration || button was pushed

    // Step 2 : Read and consider the moisture value
    int moistureValue = readMoisture();

    if( moistureValue > 300 ){ // It's wet if if the value is more than 300
      // // Step 3A : Do the flow for wet category rubbish



    }else{
      // Step 3B : Trigger ESP32 board to run the flow if the waste is not wet
      Serial.println("Start");

      

      waste_type = Serial.readString();
      while (waste_type == ""){ //wait until the task on ESP32 finished
        waste_type = Serial.readString();
        delay(100);
      }

      if( waste_type == "Plastic" ){



      }else if( waste_type == "Paper" ){

      }

    }

  
  }
  
  
  // Waiting stage
  delay(500);
}

void readMoisture()
{
}

void readDistance()
{
}

// *** Discontinued ***
// void readGas()
// {
// }

void readVibration()
{
}

void spinBase(int distance)
{
}

void openGate()
{
}


// ********** communication ************

  // if (Serial.available() > 0) {
  //   // Read the incoming data
  //     String received = Serial.readStringUntil('\n'); // Read until newline character
  //     Serial.println("Received: " + received);       // Print received data
  // }

  // delay(1000);