$(document).ready(function() {
	var socket = io.connect();
	socket.on('state', function(data) {
		var _state = data.text;
		if(_state ==='OPEN'){
			_state = 'FREI';
		}else if(_state === 'LOCKED'){
			_state = 'BESETZT';
		}
		$('#smartShit .state').text(_state);
	});
});