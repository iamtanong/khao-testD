#include <Arduino.h>
// Define Pin
#define MOISTURE_SENSOR_PIN A0
#define SERVO_PIN 9
#define VIBRATION_SENSOR_PIN 2

// Functoin pre-declaration
void readMoisture();
void readDistance();

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
  int rubbish_lvl = 0;
  // Step 1 : Vibration detection || Button pushing
  int vibrationValue = readVibration();

  if( vibrationValue >= 700){ // the sensor detects high value of vibration || button was pushed

    // Step 2 : Read and consider the moisture value
    int moistureValue = readMoisture();

    if( moistureValue > 300 ){ // It's wet if if the value is more than 300
      // Step 3A : Move the bin for wet category rubbish to be at the gate
      spinBase(100);

      // Step 4A : Open and close the gate
      openGate();
      delay(2500);
      //closeGate();
      delay(2500);

      // Step 5A : read the rubbish level in the bin
      rubbish_lvl = readDistance();
      
      // Step 6A : Spin the base back to the origin position 
      spinBase(-100);



    }else{
      // Step 3B : Trigger ESP32 board to run the flow if the waste is not wet
      Serial.println("Start");

      // Step 4B : Waiting for response from ESP32
      waste_type = Serial.readString();
      while (waste_type == ""){ //wait until the task on ESP32 finished
        waste_type = Serial.readString();
        delay(100);
      }

      // Step 5B : Spin the base
      int spinValue = 0;
      if( waste_type == "Plastic" ){
        spinValue = 200;
      }else if( waste_type == "Paper" ){
        spinValue = 300; // these are dummies
      }

      // ******** Do the same steps as wet rubbish ****************
      
      // Step 3A : Move the bin for wet category rubbish to be at the gate
      spinBase(spinValue);

      // Step 4A : Open and close the gate
      openGate();
      delay(2500);
      //closeGate();
      delay(2500);

      // Step 5A : read the rubbish level in the bin
      rubbish_lvl = readDistance();
      
      // Step 6A : Spin the base back to the origin position 
      spinBase(-1 * spinValue);

    }

    // Upload data to cloud. Should we use ESP32 instead?
    Serial.println(waste_type + " " + rubbish_lvl);
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