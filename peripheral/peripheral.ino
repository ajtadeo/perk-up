#include <ArduinoBLE.h>
#include <RTCZero.h>

int times = 0;
RTCZero rtc;

//rtc values
/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 8;

/* Change these values to set the current initial date */
const byte day = 15;
const byte month = 6;
const byte year = 15;

/*
Perk Up! Peripheral Bluetooth Arduino

This program provides peripheral advertising via bluetooth to connect to the central arduino.
When motion is detected from the ultrasonic sensor, data is sent from the peripheral arduino to the central arduino.

Resources
- ArduinoBLE peripheral LED.ino 
- ArduinoBLE central LEDControl.ino
- https://howtomechatronics.com/tutorials/arduino/ultrasonic-sensor-hc-sr04/
- https://www.youtube.com/watch?v=u_cJCtaEmyA&ab_channel=BinaBhatt (how to use breadboard power supply module)
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
const int THRESHOLD = 10; // 100cm

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
  Serial.print("Advertising 'Perk Up Peripheral 1' with UUID: ");
  Serial.print(sensorService.uuid());
  Serial.println("...");

  //rtc interface

  rtc.begin(); // initialize RTC

  rtc.setHours(hours);
  rtc.setMinutes(minutes);
  rtc.setSeconds(seconds);

  // Set the date
  rtc.setDay(day);
  rtc.setMonth(month);
  rtc.setYear(year);
}

void loop() {
  // int clock = rtc.getHours();
  // if ((clock >=7 ) && (clock <=11) && (times==0)) {
  //   times++;
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

        // TODO: only send messages one time (via times) during set time period
        if (distance < THRESHOLD){
          int clock = rtc.getHours();
          // if ((clock >=7 ) && (clock <=11) && (times==0)) {
            Serial.println("SENDING MESSAGE TO CENTRAL!!!!");
            // times++;
            movementCharacteristic.writeValue(true);
          // }
        } else {
          Serial.print("Distance: ");
          Serial.println(distance);
          movementCharacteristic.writeValue(false);
        }
      }

      Serial.print("Disconnected from central: ");
      Serial.println(central.address());
      times = 0;
    }
  // }
  // else {
  //   if ((times == 1) && (clock <7) && (clock > 11)) {
  //      times = 0;
  //   }
  // }
}
