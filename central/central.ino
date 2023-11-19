#include <ArduinoBLE.h>
#include <RTCZero.h>

int times = 0; // keep track of the number of times the coffee maker has been turned on in a given time period

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

}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  //identity check for security reasons + clock & times check to activate coffee maker
  if ( (peripheral.address() == "abc") && (clock) && (!times)){ 
    times++; // do not turn on again
    // turn the coffee maker on
  }
  else {
    times=0;
  }



}
