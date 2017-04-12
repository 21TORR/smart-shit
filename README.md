# Smart Shit

Walking to the toilets just to find them currently occupied is a pretty bad thing to ruin your day. So we decided to bring the toilet status into our intranet using AWS IoT.

## The Hardware

### Cloud Box

The model for the 3D printed box containing the boards is derived from Thingiverse #606055 by mrtial (http://bit.ly/2o5Qejy), repaired to a valid 2-manifold for printing using MakePrintable (http://bit.ly/2o3OLLq) and FreeCad (http://bit.ly/2nTuqFH) and resized to 70%.

### Controller Board

The boards are Adafruit WICED WiFi Feather boards (http://bit.ly/2n7PBXA) that are powerful enough to handle the MQTT/HTTP via TLS 1.2 Certs requests required by AWS IoT.

### Also, we used a switch and some cables...

## The Software

### On the Feather Boards

The Feather Boards have a very simple software setup using the AWS IoT SDK, connecting to the AWS Cloud on Powerup and then watching for the swich to be triggered, updating the status in AWS IoT. The Boards do a reconnect if dropped from Wifi and do a regular status send as a timeout too to catch any lost messages that might have happened due to a connection loss or power outage.
