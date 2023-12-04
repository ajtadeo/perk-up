const express = require("express");
const { createServer } = require("http");
const { Liquid } = require('liquidjs');
const bodyParser = require("body-parser");
var bleno = require("@abandonware/bleno");

const GUI_SERVICE_UUID = "3F8B0000-7E9A-442B-AFBC-6C5FEF789C2D";
const CHARACTERISTIC_UUID = "3F8B0000-7E9A-442B-AFBC-6C5FEF789C2D";
const PORT = 8888;
const HOSTNAME = "localhost";
var start;
var end;

// var BlenoPrimaryService = bleno.PrimaryService;
// const service = new BlenoPrimaryService({
//   uuid: GUI_SERVICE_UUID,
//   characteristics: [
//     new DatetimeCharacteristic()
//   ]
// })

// var DatetimeCharacteristic = bleno.Characteristic;
// const characteristic = new DatetimeCharacteristic(

// );

// WEB SERVER
const app = express();
const server = createServer(app);
const engine = new Liquid();
app.engine("liquid", engine.express());
app.set("views", __dirname + "/views");
app.set("view engine", "liquid");
app.use(express.static(__dirname + "/static"));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

app.get("/", (req, res) => {
  res.render("index")
});

app.post("/update_schedule", (req, res) => {
  start = parseInt(req.body.startHour);
  end = parseInt(req.body.endHour);
  res.redirect("/");
  // TODO: send message via bluetooth
  // if (characteristic) {
  //   write
  //   characteristic.updateValue(Buffer.from(msg));
  //   console.log("Bluetooth message sent:", msg);
  //   res.redirect("/");
  // } else {
  //   console.error("Datetime characteristic not found.");
  //   res.status(500).send("Internal Server Error");
  // }
});

server.listen(PORT, HOSTNAME, () => {
  console.log(`App running on ${HOSTNAME}:${PORT}`);
}).on("error", (err) => {
  console.log("Error: Express server not configured correctly.")
  console.log(err);
});

// BLUETOOTH
// https://github.com/noble/bleno/blob/master/examples/echo/main.js
bleno.on("stateChange", function (state) {
  console.log('on -> stateChange: ' + state);

  if (state === 'poweredOn') {
    bleno.startAdvertising('GUI', [GUI_SERVICE_UUID]);
  } else {
    bleno.stopAdvertising();
  }
})

bleno.on('advertisingStart', function (error) {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));

  if (error) {
    console.log(error)
  } else {
    bleno.setServices([
      new bleno.PrimaryService(
        {
          uuid: GUI_SERVICE_UUID,
          
          characteristics: [
            new bleno.Characteristic({
              uuid: CHARACTERISTIC_UUID,
              value: null,
              properties: ['read'],
              onReadRequest: function (offset, callback) {
                console.log("READ request received");
                // read the temperature value from the sensor
                // this.value = start;
                this.value = 0;
                console.log('Start value: ' + this.value);
                callback(this.RESULT_SUCCESS, Buffer.alloc((this.value ? this.value.toString() : "")));
              }
            })
          ]
        })
    ])
  }
});