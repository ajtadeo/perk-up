#include <ArduinoBLE.h>

const int d8 = 8;

//Dummy Services and Initialization
BLEService sensorService("86A90000-3D47-29CA-7B15-ED5A42F8E71B");
BLEBoolCharacteristic movementCharacteristic("86A90000-3D47-29CA-7B15-ED5A42F8E71B", BLERead);


void setup() {
  Serial.begin(9600);
  while(!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }
  
  Serial.println("Bluetooth® Low Energy Central scan");

  // start scanning for peripheral
  BLE.scan();

  // set up pin d8 for GPIO output, this is the pin that will activate our solenoid
  pinMode(d8, OUTPUT);


}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  //identity check for security reasons + clock & times check to activate coffee maker
  if ( peripheral.address() == "abc"){ 
    // turn the coffee maker on
    digitalWrite(d8,0); // solenoid activation
    delay(2000);
    digitalWrite(d8,1);
    delay(2000);
  }
  else {
    // do something?
  }



}
