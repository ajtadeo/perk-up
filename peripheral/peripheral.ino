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
const char SERVICE_UUID[] = "86A90000-3D47-29CA-7B15-ED5A42F8E71B";
const char MOVEMENT_UUID[] = "86A90000-3D47-29CA-7B15-ED5A42F8E71B";
const char SOUND_UUID[] = "86A90000-3D47-29CA-7B15-ED5A42F8E71A";
BLEService sensorService(SERVICE_UUID);
BLEBoolCharacteristic movementCharacteristic(MOVEMENT_UUID, BLERead);
BLEBoolCharacteristic soundCharacteristic(SOUND_UUID, BLERead | BLEWrite );

// pin assignments
const int LED = LED_BUILTIN;
const int ECHO = 2; // digital pin D2
const int TRIG = 3; // digital pin D3
const int A = 11;
const int B = 10;
const int C = 9;
const int D = 8;
const int E = 7;
const int F = 6;
const int G = 5;
const int D1 = 12;
const int D2 = 20;
const int D3 = 19;
const int D4 = 21;

const int START_BTN = 14;
const int END_BTN = 15;

int count = 0;
bool already_reading_1, already_reading_2 = 0;
byte startHours;
byte endHours;

// ultrasonic sensor
long duration;
int distance; 
const int THRESHOLD = 20; // cm

// state machine
int state;
const int SENSE_MOVEMENT = 0;
const int AWAIT_ACK = 1;
const int IDLE_COFFEE_DONE = 2;
const int IDLE = 3;

// moving buffer
const int BUFFER_SIZE = 10;
int moving_buffer[BUFFER_SIZE] = {100,100,100,100,100};

// RTC
RTCZero rtc;
const byte seconds = 30;
const byte minutes = 59;
const byte hours = 1;
const byte day = 3;
const byte month = 12;
const byte year = 23;
byte currentDay;

void setup() {
  Serial.begin(9600);
  // while (!Serial);

  // pin modes
  pinMode(LED, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(START_BTN, INPUT);
  pinMode(END_BTN, INPUT);
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);

  // initialize time
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);
  currentDay = day;
  startHours = 0;
  endHours = 0;

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
}

void loop() {
  // get button input
  if (!already_reading_1 && digitalRead(START_BTN) == HIGH) {
    already_reading_1 = 1;
    if (startHours == 23) {
      startHours = 0;
    } else {
      startHours = startHours + 1;
    }
  } else if (digitalRead(START_BTN) == LOW){
    already_reading_1 = 0;
  }
  if (!already_reading_2 && digitalRead(END_BTN) == HIGH) {
    already_reading_2 = 1;
    if (endHours == 23) {
      endHours = 0;
    } else {
      endHours = endHours + 1;
    }
  } else if (digitalRead(END_BTN) == LOW){
    already_reading_2 = 0;
  }

  count++; // used for 7 segement display
  setSevenSeg();

  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    Serial.println("Schedule: " + String(startHours) + " - " + String(endHours));

    // clear old values, initialize state to IDLE
    if (central.connected()) {
      movementCharacteristic.writeValue(false);
      offSevenSeg();
      state = IDLE;
      Serial.println("IDLE");
    }

    while(central.connected()) {
      // blink display if schedule is invalid
      if (endHours < startHours || ((endHours == startHours) && (startHours != 0))) {
        offSevenSeg();
        delay(500);
        setSevenSeg();
        delay(500);
        continue;
      }
      // Serial.print(rtc.getHours());
      // Serial.print(":");
      // Serial.print(rtc.getMinutes());
      // Serial.print(":");
      // Serial.println(rtc.getSeconds());
      if (state == SENSE_MOVEMENT) {
        if (is_in_schedule() == true) {
          state = SENSE_MOVEMENT;
        } else {
          state = IDLE_COFFEE_DONE;
          Serial.println("SENSE_MOVEMENT -> IDLE_COFFEE_DONE");
          continue;
        }
        // clear TRIG pin
        digitalWrite(TRIG, LOW);
        delayMicroseconds(2);

        // calculate distance data in cm
        digitalWrite(TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG, LOW);
        duration = pulseIn(ECHO, HIGH);
        distance = duration * 0.034 / 2; // speed of sound = 340 m/s
        shift_buffer_left(distance); // add new distance value to our buffer

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
      } else if (state == AWAIT_ACK) {
        // await ack from central that coffee was dispensed
        byte ack;
        soundCharacteristic.readValue(ack);
        if (ack == true) {
          state = IDLE_COFFEE_DONE;
          Serial.println("AWAIT_ACK -> IDLE_COFFEE_DONE");
        } else {
          state = AWAIT_ACK;
        }
      } else if (state == IDLE_COFFEE_DONE) {
        // coffee dispensed for the day, wait for next day
        movementCharacteristic.writeValue(false);
        if (currentDay != rtc.getDay()) {
          currentDay = rtc.getDay();
          state = IDLE;
          Serial.println("IDLE_COFFEE_DONE -> IDLE");
        } else {
          state = IDLE_COFFEE_DONE;
        }
      } else if (state == IDLE) {
        // wait until current time is within schedule
        movementCharacteristic.writeValue(false);
        byte currentHour = rtc.getHours();
        byte currentSecond = rtc.getSeconds();
        if (is_in_schedule() == true) {
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

bool is_in_schedule() {
  byte currentHour = rtc.getHours();
  byte currentSecond = rtc.getSeconds();
  if (startHours == 0 && endHours == 0) {
    // edge case for 0-0, all day
    return true;
  } else if (currentHour >= startHours && currentHour < endHours) {
    return true;
  } else {
    return false;
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

void setSevenSeg() {
  if (count % 4 == 0) {
    sevenSeg(1, (startHours/10));
  } 
  if (count % 4 == 1) {
    sevenSeg(2, startHours % 10);
  } 
  if (count % 4 == 2) {
    sevenSeg(3, endHours/10);
  }
  if (count % 4 == 3) {
    sevenSeg(4, endHours % 10);
  }
}

void offSevenSeg() {
  sevenSeg(1, -1);
  sevenSeg(2, -1);
  sevenSeg(3, -1);
  sevenSeg(4, -1);
}

void sevenSeg(int number, int digit) {
  // Serial.print(number);
  // Serial.print("\t");
  // Serial.println(digit);
  // select digit
  switch (number) {
    case 1:
      // Serial.println("1");
      digitalWrite(D1, LOW);
      digitalWrite(D2, HIGH);
      digitalWrite(D3, HIGH);
      digitalWrite(D4, HIGH);
      break;
    case 2:
    // Serial.println("2");
      digitalWrite(D1, HIGH);
      digitalWrite(D2, LOW);
      digitalWrite(D3, HIGH);
      digitalWrite(D4, HIGH);    
      break;
    case 3:
    // Serial.println("3");
      digitalWrite(D1, HIGH);
      digitalWrite(D2, HIGH);
      digitalWrite(D3, LOW);
      digitalWrite(D4, HIGH);
      break;
    case 4:
    // Serial.println("4");
      digitalWrite(D1, HIGH);
      digitalWrite(D2, HIGH);
      digitalWrite(D3, HIGH);
      digitalWrite(D4, LOW);
      break;
  }
  // select number
  switch(digit) {
    case 0:
      digitalWrite(A,  HIGH);   
      digitalWrite(B, HIGH);   
      digitalWrite(C, HIGH);   
      digitalWrite(D, HIGH);   
      digitalWrite(E, HIGH);   
      digitalWrite(F, LOW);   
      digitalWrite(G, HIGH);  
      break;
    case 1:
      digitalWrite(A,  LOW);   
      digitalWrite(B, HIGH);   
      digitalWrite(C, HIGH);   
      digitalWrite(D, LOW);   
      digitalWrite(E, LOW);   
      digitalWrite(F,  LOW);   
      digitalWrite(G, LOW); 
      break;
    case 2:
      digitalWrite(A,  HIGH);   
      digitalWrite(B, HIGH);   
      digitalWrite(C, LOW);   
      digitalWrite(D, HIGH);   
      digitalWrite(E, HIGH);   
      digitalWrite(F,  HIGH);   
      digitalWrite(G, LOW); 
      break;
    case 3:
      digitalWrite(A,  HIGH);   
      digitalWrite(B, HIGH);   
      digitalWrite(C, HIGH);   
      digitalWrite(D, HIGH);   
      digitalWrite(E, LOW);   
      digitalWrite(F,  HIGH);   
      digitalWrite(G, LOW); 
      break;
    case 4:
      digitalWrite(A,  LOW);   
      digitalWrite(B, HIGH);   
      digitalWrite(C, HIGH);   
      digitalWrite(D, LOW);   
      digitalWrite(E, LOW);   
      digitalWrite(F,  HIGH);   
      digitalWrite(G, HIGH); 
      break;
    case 5:
      digitalWrite(A,  HIGH);   
      digitalWrite(B, LOW);   
      digitalWrite(C, HIGH);   
      digitalWrite(D, HIGH);   
      digitalWrite(E, LOW);   
      digitalWrite(F,  HIGH);   
      digitalWrite(G, HIGH); 
      break;
    case 6:
      digitalWrite(A,  HIGH);   
      digitalWrite(B, LOW);   
      digitalWrite(C, HIGH);   
      digitalWrite(D, HIGH);   
      digitalWrite(E, HIGH);   
      digitalWrite(F, HIGH);   
      digitalWrite(G, HIGH); 
      break;
    case 7:
      digitalWrite(A,  HIGH);   
      digitalWrite(B, HIGH);   
      digitalWrite(C, HIGH);   
      digitalWrite(D, LOW);   
      digitalWrite(E, LOW);   
      digitalWrite(F,  LOW);   
      digitalWrite(G, LOW); 
      break;
    case 8:
      digitalWrite(A,  HIGH);   
      digitalWrite(B, HIGH);   
      digitalWrite(C, HIGH);   
      digitalWrite(D, HIGH);   
      digitalWrite(E, HIGH);   
      digitalWrite(F,  HIGH);   
      digitalWrite(G, HIGH); 
      break;
    case 9:
      digitalWrite(A,  HIGH);   
      digitalWrite(B, HIGH);   
      digitalWrite(C, HIGH);   
      digitalWrite(D, LOW);   
      digitalWrite(E, LOW);   
      digitalWrite(F,  HIGH);   
      digitalWrite(G, HIGH); 
      break;
    case -1:
      digitalWrite(A,  LOW);   
      digitalWrite(B, LOW);   
      digitalWrite(C, LOW);   
      digitalWrite(D, LOW);   
      digitalWrite(E, LOW);   
      digitalWrite(F,  LOW);   
      digitalWrite(G, LOW);

  }
  return;
}