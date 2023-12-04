#include <ArduinoBLE.h>

// pin assignment
const int solenoid_switch_pin = 8; // solenoid switch
const int sound_threshhold_pin = 9; // GPIO-IN : is our sound above the threshhold?

// ble
const char SERVICE_UUID[] = "86A90000-3D47-29CA-7B15-ED5A42F8E71B";
const char MOVEMENT_UUID[] = "86A90000-3D47-29CA-7B15-ED5A42F8E71B";
const char SOUND_UUID[] = "86A90000-3D47-29CA-7B15-ED5A42F8E71A";
BLEService sensorService(SERVICE_UUID);

void setup() {
  Serial.begin(9600);
  while(!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }
  
  // set up pin solenoid_switch_pin for GPIO output, this is the pin that will activate our solenoid
  pinMode(solenoid_switch_pin, OUTPUT);

  // start scanning for peripheral
  Serial.println("Bluetooth® Low Energy Central scan");
  BLE.scanForUuid(SERVICE_UUID);
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();
  if (peripheral) {
    Serial.println("Connection Established");
    BLE.stopScan();
    
    controlPeripheral(peripheral); // loop here with peripheral logic

    BLE.scanForUuid(SERVICE_UUID);
  }
}

void controlPeripheral(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the characteristics
  BLECharacteristic movementCharacteristic = peripheral.characteristic(MOVEMENT_UUID);
  BLECharacteristic sound_characteristic = peripheral.characteristic(SOUND_UUID);

  if (!movementCharacteristic) {
    Serial.println("Peripheral does not have movement characteristic!");
    peripheral.disconnect();
    return;
  } else if (!sound_characteristic) {
    Serial.println("Peripheral does not have sound characteristic!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected()) {
    // await movement sensed from peripheral
    byte movementSensed = false;
    movementCharacteristic.readValue(movementSensed);
    if (movementSensed) {
      // actuate solenoid
      digitalWrite(solenoid_switch_pin,1); // solenoid activation
      delay(2000);
      digitalWrite(solenoid_switch_pin,0);
      delay(100);

      // await sound from coffee machine
      int is_sound_on = digitalRead(sound_threshhold_pin);
      int on_count = 0;
      int off_count = 0;
      const int THRESHOLD = 2;
      while (1) {
        is_sound_on = digitalRead(sound_threshhold_pin);
        Serial.println(is_sound_on);
        if (is_sound_on) {
          on_count++;
        } else if (off_count >= 10000) { // large threshold, TODO: figure out what this should be
          break;
        } 
        else {
          on_count = 0;
          off_count++;
        }

        if (on_count >= THRESHOLD) {
          // send signal back that the thing has been pressed
          sound_characteristic.writeValue((byte)1);
          Serial.println("sending press characteristic");
          break;
        }
        delay(10);
      }
    }  
    
    sound_characteristic.writeValue((byte)0);

  }

  Serial.println("Peripheral disconnected");
}
