var express = require('express'),
	webreader = express(),
	server = require('http').createServer(webreader),
	io = require('socket.io').listen(server),
	config = require('./config.json');

server.listen(config.port);

webreader.configure(function() {
	webreader.use(express.static(__dirname + '/public'));
});

webreader.get('/', function(req, res) {
	res.sendfile(__dirname + '/public/index.html');
});

var awsIot = require('aws-iot-device-sdk');

var device = awsIot.thingShadow({
	keyPath: config.certPath + config.thingName + '.private.key',
	certPath: config.certPath + config.thingName + '.cert.pem',
	caPath: config.certPath + 'root-CA.crt',
	clientId: config.prefix + config.thingName,
	region: config.region
});

device
	.on('connect', function() {
		device.register(config.thingName, [], function() {
			io.sockets.on('connection', function(socket) {
				//trigger to get current state
				device.get(config.thingName);
			});
		});

	});
device
	.on('foreignStateChange', function(thingName, opreation, stateObject) {
		io.sockets.emit('state', {
			text: stateObject.state.reported.door
		});
	});
device
	.on('status', function(thingName, stat, clientToken, stateObject) {
		io.sockets.emit('state', {
			text: stateObject.state.reported.door
		});
	});