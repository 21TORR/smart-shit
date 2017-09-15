var express = require('express');
var webreader = express();
var server = require('http').createServer(webreader);
var io = require('socket.io').listen(server);
var config = require('./config.json');
var serveStatic = require('serve-static');
var awsIot = require('aws-iot-device-sdk');
var _state = {};

// define thing
var Thing = function(name) {
	'use strict';

	this.device = awsIot.thingShadow({
		keyPath: config.certPath + config.certName + '.private.key',
		certPath: config.certPath + config.certName + '.cert.pem',
		caPath: config.certPath + 'root-CA.crt',
		clientId: config.prefix + name,
		host: config.host,
		region: config.region
	});
	var device = this.device;

	// store device state temporary
	_state[name] = 'connecting...';
	this.device
		.on('connect', function() {
			// register to the  ThingShadow
			device.register(name, [], function() {
				//trigger to get current state -> status event
				device.get(name);
			});
		});
	this.device
		.on('foreignStateChange', function(thingName, opreation, stateObject) {
			// triggerd if state of an Thing Changed
			// emit the change to websockets
			io.sockets.emit('state', {
				id: thingName,
				text: stateObject.state.reported.door
			});

			// store the current state for new connected websockets
			_state[name] = stateObject.state.reported.door;
		});
	this.device
		.on('status', function(thingName, stat, clientToken, stateObject) {
			// triggerd by get(name); in register
			io.sockets.emit('state', {
				id: thingName,
				text: stateObject.state.reported.door
			});

			// store the current state for new connected websockets
			_state[name] = stateObject.state.reported.door;

		});
};
module.exports = Thing;

// app listening on port from config file //config.json
server.listen(config.port);

// app respon on root URL (/)
webreader.get('/', function(req, res) {
	// app serve static files from the pulic directory
	webreader.use(serveStatic(__dirname + '/public'));
	// app send index.html
	res.sendFile(__dirname + '/public/index.html');
});

config.things.forEach(function(thing, i) {
	// create Things
	new Thing(thing.thingName);
});

// triggerd if a websocket is connecting to our server
io.sockets.on('connection', function(socket) {
	// send the temporary stored states to the new connected device
	for (var thing in _state) {
		io.sockets.emit('state', {
			id: thing,
			text: _state[thing]
		});
	}
});