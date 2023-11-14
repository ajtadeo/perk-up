#include <ArduinoBLE.h>

/*
Perk Up! Peripheral Bluetooth Arduino

This program provides peripheral advertising via bluetooth to connect to the central arduino.
When motion is detected from the ultrasonic sensor, data is sent from the peripheral arduino to the central arduino.

Resources
- ArduinoBLE peripheral LED.ino 
- ArduinoBLE central LEDControl.ino
- https://howtomechatronics.com/tutorials/arduino/ultrasonic-sensor-hc-sr04/
*/

// bluetooth peripheral
BLEService sensorService("86A90000-3D47-29CA-7B15-ED5A42F8E71B");
BLEBoolCharacteristic movementCharacteristic("86A90000-3D47-29CA-7B15-ED5A42F8E71B", BLERead);

// pin assignments
const int LED = LED_BUILTIN;
const int ECHO = 2; // digital pin D2
const int TRIG = 3; // digital pin D3

// ultrasonic sensor
long duration;
int distance; 

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // pin modes
  pinMode(LED, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(TRIG, OUTPUT);

  // BLE initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  // set advertised local name and service UUID
  BLE.setLocalName("Perk Up Peripheral 1");
  BLE.setAdvertisedService(sensorService);

  // add service
  sensorService.addCharacteristic(movementCharacteristic);
  BLE.addService(sensorService);

  // set initial value
  movementCharacteristic.writeValue(false);

  // advertise to central connections
  BLE.advertise();
  Serial.println("Advertising 'Perk Up Peripheral 1'...");
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    
    while(central.connected()) {
      // clear TRIG pin
      digitalWrite(TRIG, LOW);
      delayMicroseconds(2);

      // calculate distance data in cm
      digitalWrite(TRIG, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIG, LOW);
      duration = pulseIn(ECHO, HIGH);
      distance = duration * 0.034 / 2; // speed of sound = 340 m/s

      // set characteristic = true if motion detected
      Serial.print("Distance: ");
      Serial.println(distance);
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());

  }
}
