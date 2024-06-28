const express = require("express");
const http = require("http");
const socketio = require("socket.io");
const app = express();
const server = http.createServer(app);
const io = socketio(server);

const {
  createTable,
  insertNewData,
 
} = require("./module");

const port = 4000;

app.use(express.static("sos")); 

app.get("/dashboard", (req, res) => { // display method
    res.sendFile("./sos/index.html", { root: __dirname }); // read data then send to server
  });

server.listen(port, () =>
  console.log(`Server listening on port: ${port}`)
);

// Create Table
createTable();

const mqtt = require("mqtt");

// Connect to mqtt
const options = {
  clientID: "",
  username: "",
  password: "",
  host: "192.168.43.203", // homeAP-192.168.0.103 || mobile-192.168.43.203
  port: 1883,
};

const mqttClient = mqtt.connect(options);

// setup the callbacks

mqttClient.on('connect', () => {
	console.log('✅ Connected to MQTT Broker');
  // Client sign up topic publish from Esp8266
	mqttClient.subscribe('sensors', () => {
		mqttClient.on('message', (topic, payload) => {
			let tmp = payload.toString().split(',');
			data = {
				temp: tmp[0],
				hum: tmp[1],
				gas: tmp[2],
        smoke: tmp[3],
				created_at: new Date().toLocaleString(),
        
			};
      
    console.log(data);
    io.sockets.emit('sensorss', data);
  const newDataTemp = Math.round(data.temp);
  const newDataHumidity = Math.round(data.hum);
  const newDataGas = data.gas;
  const newDataSmoke = data.smoke;
  const ss_id = Math.floor(Math.random()*5)+1 ;
  insertNewData(ss_id, newDataTemp, newDataHumidity, newDataGas, newDataSmoke);

		});
	});
  
  
});

io.on("connection", function (socket) {
// server listen data from clients
  socket.on("Led1", function (data) {
    if (data == "on") {
      console.log("Led1 ON");
      mqttClient.publish("d1", "1");
    } else {
      console.log("Led1 OFF");
      mqttClient.publish("d1", "0");
    }
  });

  socket.on("Warning", function (data) {
    if (data == "on") {
      console.log("Warning");
      mqttClient.publish("warning", "1");
    } else {
      console.log("UnWarning");
      mqttClient.publish("warning", "0");
    }
  });
});