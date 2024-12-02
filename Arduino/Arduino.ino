#include <Arduino.h>
#include <Stepper.h>
#include <Servo.h>


// Define Pin
#define MOISTURE_SENSOR_PIN A0

#define VIBRATION_PIN A0
#define GAS_SENSOR_PIN A1
#define BUTTON_PIN 12
#define LED_PIN 2
#define pingPin 2
#define inPin 4
// Define the number of steps per revolution for your stepper motor
#define STEPS_PER_REV 2048  // For 28BYJ-48 stepper motor

// Initialize the stepper library with the pins connected to the ULN2003
Stepper stepper(STEPS_PER_REV, 8, 10, 9, 11);  // IN1 = 8, IN3 = 10, IN2 = 9, IN4 = 11

// Functoin pre-declaration
int readMoisture();
float readDistance();
void readGas();
int readVibration();
void spinBase(int distance);
void openGate();

Servo myServo;

// Global var

int rubbish_lvl = 0;
int vibrationValue = 0;
int moistureValue = 0;
float duration, cm;
int curMoisture = 0;
int prevMoisture = 0;

String command, wasteType;
// Extract parts of the string
void parseInput(String input) {
    int firstSpace = input.indexOf(' ');   // Position of first space
    int secondSpace = input.indexOf(' ', firstSpace + 1);  // Position of second space

    if (firstSpace != -1 && secondSpace != -1) {
        command = input.substring(0, firstSpace);                     // Extract "Bin"
        wasteType = input.substring(firstSpace + 1, secondSpace);     // Extract "[waste_type]"
        int number = input.substring(secondSpace + 1).toInt();            // Extract "[number]" and convert to int
    } else {
        Serial.println("Invalid input format.");
    }
}
void processReceivedData(String data) {
    Serial.println("Received: " + data);  // Debug log the received data

    // Check the content of the message
    if (data == "Wet") {
        Serial.println("Processing Wet Rubbish...");
        wasteType = "Wet";  // Update the global waste type
    } else if (data.startsWith("Plastic")) {
        int delimiterIndex = data.indexOf(';');
        if (delimiterIndex != -1) {
            wasteType = "Plastic";  // Update waste type
            int value = data.substring(delimiterIndex + 1).toInt();
            Serial.println("Plastic waste type, value: " + String(value));
        } else {
            wasteType = "Plastic";  // Default handling if no additional info
        }
    } else if (data == "Paper") {
        Serial.println("Processing Paper Rubbish...");
        wasteType = "Paper";  // Update the waste type
    } else if (data == "Start") {
        Serial.println("Starting waste sorting...");
        wasteType = "Start";  // Reset or initiate sorting
    } else {
        Serial.println("Unknown command: " + data);
    }
    data = "";
}


String receivedData = ""; 

void setup()
{
  // communicate with esp32
  Serial.begin(9600);
  Serial.println("Setting up...");

  // Servo
  // pinMode(sensorPin, INPUT);
  myServo.attach(3);
  delay(1000);
  myServo.write(120); // set zero the gate
  delay(3000);

  //vibration
  pinMode(VIBRATION_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(A4,  INPUT);

  // Set the speed of the motor (in RPM)
  stepper.setSpeed(10);  // Adjust speed as needed

}

void loop(){

  String received = "";
  int button_state = digitalRead(BUTTON_PIN);
  Serial.println(button_state);
  
  // Step 1 : Vibration detection || Button pushing
  
  // while(1){
  //   vibrationValue = readVibration();  // Call the function and store the result
  //   Serial.print("Vibration Level: ");
  //   Serial.println(vibrationValue);
  //   delay(200); 
  // }
  
  

  //full flow
    while (Serial.available() > 0) {
        char incomingChar = Serial.read();  // Read one character
        if (incomingChar == '\n') {        // Check for end of message
            receivedData.trim();           // Remove whitespace/newlines
            processReceivedData(receivedData);  // Process the complete message
            receivedData = "";             // Clear the buffer for the next message
        } else {
            receivedData += incomingChar;  // Append to the buffer
        }
    }



  //if( vibrationValue >= 700){ // the sensor detects high value of vibration || button was pushed
  if (button_state == HIGH || receivedData == "rubbish-in"){
    // Step 2 : Read and consider the moisture value

    }else{
      // Step 3B : Trigger ESP32 board to run the flow if the waste is not wet
      Serial.println("Start");
      delay(2000);
      // // Step 4B : Waiting for response from ESP32
      while (wasteType != "Plastic" && wasteType != "Paper" && wasteType != "Wet"){ //wait until the task on ESP32 finished
        wasteType = Serial.readString();
        wasteType.trim();

      }
      delay(500);
      
      Serial.println(wasteType);

      // // Step 5B : Spin the base
      int spinValue = 0;
      if( wasteType == "Plastic" ){
        spinValue = 0;
      }else if( wasteType == "Paper" ){
        spinValue = -650; // these are dummies
      }else if ( wasteType == "Wet"){
        spinValue = 650;
      }

      // // ******** Do the same steps as wet rubbish ****************
      
      // // Step 3A : Move the bin for wet category rubbish to be at the gate
      spinBase(spinValue);

      // // Step 4A : Open and close the gate
      openGate();
      delay(2000);
      //closeGate();
      delay(2000);

      // // Step 5A : read the rubbish level in the bin
      rubbish_lvl = readDistance();
      
      // // Step 6A : Spin the base back to the origin position 
      spinBase(-1 * spinValue);

    }

    // Upload data to cloud. Should we use ESP32 instead?
    Serial.println("Bin " + wasteType + " " + rubbish_lvl);
  



}

int readMoisture()
{
  return analogRead(A0);

}
long microsecondsToCentimeters(long microseconds)
{
// The speed of sound is 340 m/s or 29 microseconds per centimeter.
// The ping travels out and back, so to find the distance of the
// object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

float readDistance()
{

 
  pinMode(pingPin, OUTPUT);
  
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  pinMode(inPin, INPUT);
  duration = pulseIn(inPin, HIGH);
  
  cm = microsecondsToCentimeters(duration);
  
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
  return cm;
  
}

int readVibration() {  // Function returns an integer value
  return analogRead(VIBRATION_PIN); 
}

void spinBase(int distance)
{
  Serial.print("Spinning base by: ");
  Serial.println(distance);
  stepper.step(distance);  // Move one full revolution clockwise
  delay(1000);

}

void openGate()
{
    for (int angle = 120; angle >= 0; angle -= 5) {
    myServo.write(angle);  // Set the servo position

    delay(50);  // Wait 50ms for the servo to reach the position
  }

  delay(500);  // Pause for a second

  // Sweep the servo back from 180 to 0 degrees
  for (int angle = 0; angle <= 120; angle += 5) {
    myServo.write(angle);  // Set the servo position

    delay(50);  // Wait 50ms for the servo to reach the position
  }
 
}

// ********** communication ************

  // if (Serial.available() > 0) {
  //   // Read the incoming data
  //     String received = Serial.readStringUntil('\n'); // Read until newline character
  //     Serial.println("Received: " + received);       // Print received data
  // }

  // delay(1000);