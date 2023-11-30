// https://github.com/noble/bleno/blob/master/examples/echo/main.js
// linked example needs to be updated to use ES6 classes
var bleno = require('bleno');

const CHARACTERISTIC_UUID = "3F8B0000-7E9A-442B-AFBC-6C5FEF789C2D";

class DatetimeCharacteristic extends bleno.Characteristic {
  constructor () {
    super({
      uuid: CHARACTERISTIC_UUID,
      properties: ['read'],
      value: null,
    })
    this._value = Buffer.alloc(0);
    this._updateValueCallback = null;
  }

  onReadRequest(offset, callback) {
    console.log('DatetimeCharacteristic - onReadRequest: value = ' + this._value.toString('hex'));
    callback(this.RESULT_SUCCESS, this._value);
  }
}

module.exports = DatetimeCharacteristic