#include <Arduino.h>

//Definitions of Pins//
const int tempSensorPin = 34;
const int heaterPin     = 12;

//Temperature thresholds as required//
const float TARGET_TEMP = 60.0;
const float MIN_TEMP    = 40.0;

//Heater state definition as required//
enum HeaterState {
  IDLE,
  HEATING,
  STABILIZING,
  TARGET_REACHED,
  OVERHEAT
};

HeaterState currentState = IDLE;
//Reading temperature from ADC//
float readTemperature() {
  int raw = analogRead(tempSensorPin);
  return map(raw, 0, 4095, 0, 100); 
}

// Prining current system status as per conditions//
void logStatus(float temp, HeaterState state) {
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" °C | State: ");
  switch (state) {
    case IDLE:           Serial.println("Idle"); break;
    case HEATING:        Serial.println("Heating"); break;
    case STABILIZING:    Serial.println("Stabilizing"); break;
    case TARGET_REACHED: Serial.println("Target Reached"); break;
    case OVERHEAT:       Serial.println("Overheat"); break;
  }
}

//FreeRTOS Task for Heater Logic//
void heaterControlTask(void *parameter) {
  while (true) {
    float temp = readTemperature();

    if (temp < MIN_TEMP) {
      currentState = HEATING;
      digitalWrite(heaterPin, HIGH);
    }
    else if (temp < TARGET_TEMP) {
      currentState = STABILIZING;
      digitalWrite(heaterPin, HIGH);
    }
    else if (temp < TARGET_TEMP + 10) {
      currentState = TARGET_REACHED;
      digitalWrite(heaterPin, LOW);
    }
    else {
      currentState = OVERHEAT;
      digitalWrite(heaterPin, LOW);
    }

    logStatus(temp, currentState);

    vTaskDelay(1000 / portTICK_PERIOD_MS);  //Run every 1 second//
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(tempSensorPin, INPUT);
  pinMode(heaterPin, OUTPUT);
  digitalWrite(heaterPin, LOW);

  // Create a FreeRTOS task for heater control
  xTaskCreate(
    heaterControlTask,   // Task function
    "HeaterTask",        // Name
    4096,                // Stack size
    NULL,                // Parameters
    1,                   // Priority
    NULL                 // Task handle
  );
}

void loop() {
  //Nothing to be noted here, everything is handled by FreeRTOS//
}
