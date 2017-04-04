# Smart Shit

Walking to the toilets just to find them currently occupied is a pretty bad thing to ruin your day. So we decided to bring the toilet status into our intranet using AWS IoT.

## Cloud Model

The printed box containing the boards is derived from Thingiverse #606055 by mrtial (http://bit.ly/2o5Qejy), repaired to a valid 2-manifold for printing using MakePrintable (http://bit.ly/2o3OLLq) and FreeCad (http://bit.ly/2nTuqFH) and resized to 70%.

## Controller Board

The boards are Adafruit WICED WiFi Feather boards that are powerful enough to handle the MQTT/HTTP via TLS 1.2 Certs requests required by AWS IoT.