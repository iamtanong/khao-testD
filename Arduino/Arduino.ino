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
}

void loop()
{
}

void readMoisture()
{
}

void readDistance()
{
}

void readGas()
{
}

void readVibration()
{
}

void spinBase(int distance)
{
}

void openGate()
{
}