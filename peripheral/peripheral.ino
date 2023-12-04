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

#include <ArduinoBLE.h>
#include <RTCZero.h>

// bluetooth peripheral
BLEService sensorService("86A90000-3D47-29CA-7B15-ED5A42F8E71B");
BLEBoolCharacteristic movementCharacteristic("86A90000-3D47-29CA-7B15-ED5A42F8E71B", BLERead);
BLEBoolCharacteristic soundCharacteristic("86A90000-3D47-29CA-7B15-ED5A42F8E71A", BLERead | BLEWrite );

// pin assignments
const int LED = LED_BUILTIN;
const int ECHO = 2; // digital pin D2
const int TRIG = 3; // digital pin D3

// ultrasonic sensor
long duration;
int distance; 
const int THRESHOLD = 20; // cm

// state machine
int state;
const int SENSE_MOVEMENT = 0;
const int AWAIT_ACK = 1;
const int IDLE = 2;

// moving buffer
const int BUFFER_SIZE = 10;
int moving_buffer[BUFFER_SIZE] = {100,100,100,100,100};

// RTC
RTCZero rtc;
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 15;
const byte day = 3;
const byte month = 12;
const byte year = 23;
const byte schedStartHour = 15;
const byte schedEndHour = 16;
const int ACK_TIMEOUT = 10; // seconds

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
  BLE.setLocalName("Perk Up Peripheral");
  BLE.setAdvertisedService(sensorService);

  // add service
  sensorService.addCharacteristic(movementCharacteristic);
  sensorService.addCharacteristic(soundCharacteristic);
  BLE.addService(sensorService);

  // set initial characteristic values
  movementCharacteristic.writeValue(false);
  soundCharacteristic.writeValue(false);

  // advertise to central connections
  BLE.advertise();
  Serial.print("Advertising 'Perk Up Peripheral' with UUID: ");
  Serial.print(sensorService.uuid());
  Serial.println("...");

  // initialize time
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  // initialize state
  state = IDLE;
  Serial.println("IDLE");
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    
    while(central.connected()) {
      if (state == SENSE_MOVEMENT) {
        // clear TRIG pin
        digitalWrite(TRIG, LOW);
        delayMicroseconds(2);

        // calculate distance data in cm
        digitalWrite(TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG, LOW);
        duration = pulseIn(ECHO, HIGH);
        distance = duration * 0.034 / 2; // speed of sound = 340 m/s
        shift_buffer_left(distance); // add new distance value to our moving average filter

        // distance == 0 when ultrasonic sensor is not turned on
        if (check_buffer() && distance != 0 && distance <= THRESHOLD){
          Serial.println("SENDING MESSAGE TO CENTRAL!!!!");
          movementCharacteristic.writeValue(true);
          state = AWAIT_ACK;
          Serial.println("SENSE_MOVEMENT -> AWAIT_ACK");
        } else {
          movementCharacteristic.writeValue(false);
          state = SENSE_MOVEMENT;
        }
        // Serial.println(distance);
      } else if (state == AWAIT_ACK) {
        // await ack from central that coffee was dispensed
        byte ack;
        soundCharacteristic.readValue(ack);
        if (ack == true) {
          state = IDLE;
          Serial.println("AWAIT_ACK -> IDLE");
        } else {
          state = AWAIT_ACK;
        }
      } else if (state == IDLE) {
        // do nothing, wait until next time period
        byte currentHour = rtc.getHours();
        byte currentSecond = rtc.getSeconds();
        if (currentHour >= schedStartHour && currentHour < schedEndHour) {
          state = SENSE_MOVEMENT;
          Serial.println("IDLE -> SENSE_MOVEMENT");
        } else {
          state = IDLE;
        }
      }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    state = IDLE;
  }
}

void shift_buffer_left(int new_value) {
  // shift 0->3, 1-4
  for (int i = 0 ; i < BUFFER_SIZE-1; ++i) {
    moving_buffer[i+1] = moving_buffer[i];
  }

  moving_buffer[0] = new_value;
}

// checks if every item in the moving buffer is the same
bool check_buffer() {
  int sample = moving_buffer[0];
  for (int i = 1; i < BUFFER_SIZE; i++) {
    if (moving_buffer[i] != sample) {
      return false;
    }
  }
  return true;
}