#include <adafruit_feather.h>
#include <adafruit_mqtt.h>
#include <libmaple/pwr.h>
#include <libmaple/scb.h>

#include "certificates.h"
#include "privatekey.h"
#include "wlan.h"
#include "config.h"

const int ledPin = PA15;
const int buttonPin = PC3;
const int vbatPin = PA1;

int   vbatADC   = 0;         // The raw ADC value off the voltage div
float vbatLSB   = 0.80566F;  // mV per LSB to convert raw values to volts 
int vbatMV = 0;              // The ADC equivalent in millivolts

AdafruitMQTT mqtt;

int timeout = 0;

String ver = "0.3";

int reconnect_counter = 0;

#define AWS_IOT_MQTT_TOPIC             "$aws/things/" AWS_IOT_MY_THING_NAME "/shadow/update"

/**************************************************************************/
/*!
    @brief  The setup function runs once when the board comes out of reset
*/
/**************************************************************************/
void setup()
{
  //Specify the Port usages
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(vbatPin, INPUT_ANALOG);

  //Show that we're alive
  digitalWrite(ledPin, HIGH);

  //If connected to a PC we're going to output some debug information so we open a serial here
  Serial.begin(115200);

  //Connect to the Wifi
  while ( !connectAP() )
  {
    Serial.println("Connecting...");
    delay(200); // delay between each attempt
  }

  reconnect_counter = 0;

  Feather.printNetwork();

  // MQTT should ignore all errors and never halt the system, errors will be taken care of with the timeouts if something is not recoverable by the MQTT broker itself
  mqtt.err_actions(false, false);

  // Set ClientID
  mqtt.clientID(AWS_IOT_MQTT_CLIENT_ID);

  // Set the disconnect callback handler
  mqtt.setDisconnectCallback(disconnect_callback);

  // default RootCA include certificate to verify AWS (
  Feather.useDefaultRootCA(true);

  // Setting Indentity with AWS Private Key & Certificate
  mqtt.tlsSetIdentity(aws_private_key, local_cert, LOCAL_CERT_LEN);

  // Connect with SSL/TLS
  mqtt.connectSSL(AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT, true, 3600);

  sendToAWS("CONNECTED");
}

/**************************************************************************/
/*!
    @brief  This loop function runs over and over again
*/
/**************************************************************************/
void loop()
{
  //We just started the loop, let's assume the door is currently open
  //So we wait for it to be closed again the button/switch has a pullup, so as long as it is not pressed it's going to head HIGH state
  digitalWrite(ledPin, LOW);
  
  while(digitalRead(buttonPin))
  {
    //If we have looped for a while let's send the current state to AWS, just to be safe if anything happened
    //Or if someone poked the shadow in AWS and forgot to put it back
    if(timeout++ > 300)
    {
      timeout = 0;
      sendToAWS("OPEN");
    }
    delay(1000);
  }

  //Ok so the door is no longer open, send this information out to AWS
  digitalWrite(ledPin, HIGH);
  sendToAWS("LOCKED");

  //And now we wait until the business is finished and the door is opened again, let's blink
  // the led just for fun...
  while(!digitalRead(buttonPin))
  {
    if(timeout++ > 300)
    {
      timeout = 0;
      sendToAWS("LOCKED");
    }
    delay(1000);
    digitalWrite(ledPin, !digitalRead(ledPin));
  }

  //Ok so the door is open again, send this to AWS and then go back to the start of the loop
  digitalWrite(ledPin, HIGH);
  sendToAWS("OPEN");
}

void sendToAWS(String statestr)
{
  if(!Feather.connected() || reconnect_counter > 100)
  {
    setup();
  }
  
  //Get Analog Read from VBAT Pin
  //Actually for the 21TORR solution where everything is powered by wire this is not necessary
  //But we built this into the code when we planned to use battery power at the first stage of the experiment
  //This requires the solder bridge on the back of the feather board for the vbat voltage divider to be connected
  vbatADC = analogRead(vbatPin);
  //From the ADC Value calculate the Voltage, this needs to be multiplied by two as this is half of the voltage divider measured here
  vbatMV = (int)(((float)vbatADC * vbatLSB) * 2.0F);

  //Build the string to send to AWS
  String state = "{ \"state\": {\"reported\": { ";

  state += "\"door\": \"";
  state += statestr;
  state += "\", ";

  state += "\"vbat\": \"";

  state += vbatMV;

  state += "\", ";

  state += "\"version\": \"";

  state += ver;
  state += " compiled: ";
  state += __DATE__;

  state += "\"";
    
  state += "} } }";

  char schar[500];
  state.toCharArray(schar, 500);

  //Actually send this into the mqtt broker queue, if something fails, go to setup again
  if(!mqtt.publish(AWS_IOT_MQTT_TOPIC, schar, MQTT_QOS_AT_MOST_ONCE))
  {
    setup();
  }

  //Give the transmission some time, actually this should not be necessary but if we push multiple messages too
  //fast into the queue it could cause the messages to arrive in the wrong order at AWS and leave the shadow in a wrong state
  delay(2000);
}

/**************************************************************************/
/*!
    @brief  Connect to defined Access Point
*/
/**************************************************************************/
bool connectAP(void)
{
  // Attempt to connect to an AP
  //Serial.print("Please wait while connecting to: '" WLAN_SSID "' ... ");
  
  if ( Feather.connect(WLAN_SSID, WLAN_PASS) )
  {
    //Serial.println("Connected!");
  }
  else
  {
    //Serial.printf("Failed! %s (%d)", Feather.errstr(), Feather.errno());
    //Serial.println();
  }
  //Serial.println();

  return Feather.connected();
}

void disconnect_callback(void)
{
  //If there is a disconnect, just setup again...
  setup();
}
