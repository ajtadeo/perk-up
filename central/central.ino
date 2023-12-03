#include <ArduinoBLE.h>

const int solenoid_switch = 8; // solenoid switch
const int sound_threshhold = 9; // GPIO-IN : is our sound above the threshhold?

//Dummy Services and Initialization
BLEService sensorService("86A90000-3D47-29CA-7B15-ED5A42F8E71B");


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
  BLE.scanForUuid("86A90000-3D47-29CA-7B15-ED5A42F8E71B");

  // set up pin d8 for GPIO output, this is the pin that will activate our solenoid
  pinMode(d8, OUTPUT);


}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();
  if (peripheral) {
    Serial.println("Connection Established");
    BLE.stopScan();
    Serial.println("Stopping scan temporarily");
    controlLed(peripheral);

    BLE.scanForUuid("86A90000-3D47-29CA-7B15-ED5A42F8E71B");
    
    actuate:
    // digitalWrite(d8,0); // solenoid activation
    // delay(2000);
    // digitalWrite(d8,1);
    delay(5000);

    // read sound data, we want the is_sound_on to be high for atleast THRESHOLD cycles to count it
    int is_sound_on = digitalRead(sound_threshhold);
    int on_count = 0;
    int off _count = 0;
    const int THRESHOLD = 5;
    while (1) {
      if (is_sound_on) {
        on_count++;
      } else {
        on_count = 0;
        off_count++;
      }

      if (count >= 5) {
        break;
      }

      if (off_count >= 10000) {
        goto actuate;
      }
      delay(10);
    }
  }

}

void controlLed(BLEDevice peripheral) {
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

  // retrieve the LED characteristic
  BLECharacteristic movementCharacteristic = peripheral.characteristic("86A90000-3D47-29CA-7B15-ED5A42F8E71B");

  if (!movementCharacteristic) {
    Serial.println("Peripheral does not have movement characteristic!");
    peripheral.disconnect();
    return;
  } else if (!movementCharacteristic.canRead()) {
    Serial.println("Peripheral does not have a readable movement characteristic!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected()) {
    byte value = 0;
    movementCharacteristic.readValue(value);
    Serial.println(value);
    // // while the peripheral is connected

    // // read the button pin
    // int buttonState = digitalRead(buttonPin);

    // if (oldButtonState != buttonState) {
    //   // button changed
    //   oldButtonState = buttonState;

    //   if (buttonState) {
    //     Serial.println("button pressed");

    //     // button is pressed, write 0x01 to turn the LED on
    //     ledCharacteristic.writeValue((byte)0x01);
    //   } else {
    //     Serial.println("button released");

    //     // button is released, write 0x00 to turn the LED off
    //     ledCharacteristic.writeValue((byte)0x00);
    //   }
    // }
  }

  Serial.println("Peripheral disconnected");
}
