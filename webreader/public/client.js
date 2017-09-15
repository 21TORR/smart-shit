$(document).ready(function() {
	'use strict';
	var socket = io.connect();
	socket.on('state', function(data) {
		var _state = data.text;
		if (_state === 'OPEN') {
			_state = 'FREI';
		} else if (_state === 'LOCKED') {
			_state = 'BESETZT';
		}
		$('.' + data.id + ' .state').text(_state);
	});
});