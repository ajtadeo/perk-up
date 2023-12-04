const express = require("express");
const { createServer } = require("http");
const { Liquid } = require('liquidjs');
const bodyParser = require("body-parser");
var bleno = require("bleno");

GUI_SERVICE_UUID = "3F8B0000-7E9A-442B-AFBC-6C5FEF789C2D";
const PORT = 8888;
const HOSTNAME = "localhost";

var BlenoPrimaryService = bleno.PrimaryService;
var DatetimeCharacteristic = require("./characteristic");
const service = new BlenoPrimaryService({
  uuid: GUI_SERVICE_UUID,
  characteristics: [
    new DatetimeCharacteristic()
  ]
})
const characteristic = new DatetimeCharacteristic();

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
  const startHour = req.body.startHour;
  const endHour = req.body.endHour;
  const msg = startHour + '-' + endHour;

  // TODO: send message via bluetooth
  // if (characteristic) {
  //   characteristic.
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
    bleno.startAdvertising('GUI Peripheral', [GUI_SERVICE_UUID]);
  } else {
    bleno.stopAdvertising();
  }
})

bleno.on('advertisingStart', function (error) {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));

  if (!error) {
    bleno.setServices([service]);
  }
});