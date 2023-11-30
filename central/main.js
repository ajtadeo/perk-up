// https://github.com/noble/bleno/blob/master/examples/echo/main.js

var bleno = require("bleno");
var BlenoPrimaryService = bleno.PrimaryService;
var DatetimeCharacteristic = require("./characteristic");

GUI_SERVICE_UUID = "3F8B0000-7E9A-442B-AFBC-6C5FEF789C2D";

bleno.on("stateChange", function(state) {
  console.log('on -> stateChange: ' + state);

  if (state === 'poweredOn') {
    bleno.startAdvertising('GUI Peripheral', [GUI_SERVICE_UUID]);
  } else {
    bleno.stopAdvertising();
  }
})

bleno.on('advertisingStart', function(error) {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));

  if (!error) {
    bleno.setServices([
      new BlenoPrimaryService({
        uuid: GUI_SERVICE_UUID,
        characteristics: [
          new DatetimeCharacteristic()
        ]
      })
    ]);
  }
});