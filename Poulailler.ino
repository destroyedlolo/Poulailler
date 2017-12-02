/*	Poulailler
 * 	
 * 	This is my chicken coop automation
 *
 *	provided by "Laurent Faillie"
 *	Licence : "Creative Commons Attribution-NonCommercial 3.0"
 *	
 *	History :
 *	25/10/2017 - Temperature probe only
 *	12/11/2017 - Switch to OWLib
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

extern "C" {
  #include "user_interface.h"
}

	/******
	 * Parameters
	 ******/

#define DEV_ONLY	/* Developpment only code */
/*#define STATIC_IP*/	/* Use static IP when on domestik network */

	/* LED */
// #define LED_BUILTIN 2	// On my ESP-12
#if 1					// Led is lightning during Wifi / Mqtt connection establishment
#	define LED(x)	{ digitalWrite(LED_BUILTIN, x); }
#else
#	define LED(x)	{ }
#endif

	/* Network */
#include "Maison.h"		// My very own environment (WIFI_*, MQTT_*, ...)

#ifdef STATIC_IP
IPAddress adr_ip(192, 168, 0, 17);	// Static IP to avoid DHCP delay
IPAddress adr_gateway(192, 168, 0, 10);
IPAddress adr_dns(192, 168, 0, 3);
#endif

	/* MQTT */
#define MQTT_CLIENT "Poulailler"
String MQTT_Topic("Poulailler/");	// Topic root

	/* Delais */
#define DELAY	300				// Delay in seconds b/w samples (5 minutes)
#define DELAY_STARTUP	5		// Let a chance to enter in interactive mode at startup ( 5s )
#define DELAY_LIGHT 500			// Delay during light sleep (in ms - 0.5s )
#define FAILUREDELAY	900		// Delay in case of failure (15 minutes)

	/* 1-wire */
#include <OWBus.h>

// #define ONE_WIRE_BUS D1	// Where OW bus is connected to (ESP-12)
#define ONE_WIRE_BUS 2 // Where OW bus is connected to (ESP-201)
OneWire oneWire(ONE_WIRE_BUS);	// Initialize oneWire library
OWBus bus(&oneWire);

	/* Probes */
#include <OWBus/DS18B20.h>

OWDevice *probes[] = {
	new DS18B20( bus, 0x2882b25e09000015, "TestTemp" )
};

	/* End of configuration area
	 * Let's go !
	 */
#include "Duration.h"
#include "CommandLine.h"
#include "Context.h"

ADC_MODE(ADC_VCC);

WiFiClient clientWiFi;
PubSubClient clientMQTT(clientWiFi);

CommandLine cmdline;
Context ctx;
unsigned long last = 0;	// Last time data has been sent

bool startMAISON(){	// Start WiFi with home network
	Serial.println("Home network");

#if STATIC_IP
	WiFi.config(adr_ip, adr_gateway, adr_dns);
#endif
	WiFi.begin( WIFI_SSID, WIFI_PASSWORD );
	for( int i=0; i< 240; i++ ){
		if(WiFi.status() == WL_CONNECTED)
			return true;
		delay(500);
		Serial.print(".");
	}
	return false;
}

bool startDOMOTIQUE(){	// Start WiFi with domotique network
	Serial.println("Domotique network");

	WiFi.begin( DOMO_SSID, DOMO_PASSWORD );
	for( int i=0; i< 240; i++ ){
		if(WiFi.status() == WL_CONNECTED)
			return true;
		delay(500);
		Serial.print("-");
	}
	return false;
}

bool startWiFi( class Duration &d ){
	Serial.println("Starting WiFi");

	d.reInit();
	switch(ctx.getNetwork()){
	case Context::MAISON:
		startMAISON();
		break;
	case Context::DOMOTIQUE:
		startDOMOTIQUE();
		break;
	default :	// Safe scenario
		if(!startDOMOTIQUE())
			startMAISON();
	}
	d.Finished();
	
	Serial.println("ok");
	WiFi.printDiag( Serial );
	Serial.print("AutoConnect : ");
	Serial.println(WiFi.getAutoConnect() ? "Oui" : "Non");

	return(WiFi.status() == WL_CONNECTED);
}

bool startMQTT( class Duration &d ){
	Serial.println("Starting MQTT");

	d.reInit();
	for( int i=0; i< 60; i++ ){
		if(clientMQTT.connect(MQTT_CLIENT)){
			Serial.println("Connected");
			break;
		} else {
			Serial.print("Failure, rc:");
			Serial.println(clientMQTT.state());
			delay(1000);	// New try in 1 second
		}
	}

	return(clientMQTT.connected());
}

void publishMQTTDuration( class Duration &d ){
	Serial.print("MQTT connection duration :");
	Serial.println( *d );
	clientMQTT.publish( (MQTT_Topic + "MQTT/Connection").c_str(), String( *d ).c_str() );
}

void setup() {
	Duration dwifi, dmqtt;

	Serial.begin(115200);
	delay(100);
	pinMode(LED_BUILTIN, OUTPUT);
	clientMQTT.setServer(BROKER_HOST, BROKER_PORT);

	LED(LOW);
	if(!( startWiFi( dwifi ) && startMQTT( dmqtt ) )){
		Serial.println("Can't connect in given time");
		LED(HIGH);
		ESP.deepSleep(FAILUREDELAY * 1e6);
	}
	LED(HIGH);
	Serial.print("WiFi connection duration :");
	Serial.println( *dwifi );
	clientMQTT.publish( (MQTT_Topic + "Wifi").c_str(), String( *dwifi ).c_str() );
	publishMQTTDuration( dmqtt );
}

void publishData(){
	if(!clientMQTT.connected()){
		Duration dmqtt;
		startMQTT( dmqtt );
		publishMQTTDuration( dmqtt );
	}

	Serial.print("Power : ");
	Serial.println(ESP.getVcc());
	clientMQTT.publish( (MQTT_Topic + "Alim").c_str(), String( ESP.getVcc() ).c_str() );

	Serial.print("Memory : ");
	Serial.println(ESP.getFreeHeap());
	clientMQTT.publish( (MQTT_Topic + "Memory").c_str(), String( ESP.getFreeHeap() ).c_str() );

//	clientMQTT.loop();	// For incoming topics

	Serial.print("Number of temperature probe to read : ");
	Serial.println(sizeof(probes)/sizeof(*probes));

	Duration owd;
	bus.launchTemperatureAcquisition( true );	// Launch in parallel temperature acquisition
	delay(1e3);	// Wait a second to let all conversion to finish
	for(int i=0; i < sizeof(probes)/sizeof(*probes); i++){
		if( probes[i]->getOWCapability() & OWDevice::OWCapabilities::TEMPERATURE ){
			DS18B20 *p = static_cast <DS18B20 *> (probes[i]);
			String t = String(p->readLastTemperature());
			clientMQTT.publish( (MQTT_Topic + p->getName()).c_str(), t.c_str() );
			Serial.print( p->getName() + " : " + t + " / " );
			Serial.println( p->getResolution() );
		}
	}
	clientMQTT.publish( (MQTT_Topic + "MQTT").c_str(), String( *owd ).c_str() );
}

void CommandLine::loop(){	// Implement command line
	String cmd = Serial.readString();

	if(cmd == "bye"){
		this->finished();
		return;
	} else if(cmd == "1wscan"){
			/* TO BE REVIEWED with a better 1w handling */
/*
		DallasTemperature bus( &oneWire );
		bus.begin();

		int nbr = bus.getDeviceCount();
		Serial.print( nbr );
		Serial.println(" probe(s) on the bus");
		for(int i = 0; i<nbr; i++){
			DeviceAddress addr;
			bus.getAddress(addr, i);
			Serial.println(Probe::AddressString( addr ));
		}
*/
	} else
		Serial.println("Known commands : 1wscan, bye");

	this->prompt();
}

void loop(){
	if( ctx.isfirstrun() ){	// 1st run, let a chance to enter in interactive mode
		ctx.hasrun();
		publishData();
		last = millis();
		delay( DELAY_STARTUP * 1e3);
	} else if( cmdline.isactive() ){	// Interactive mode
		if( millis() > last + DELAY * 1e3 ){	// Data have to be published
			publishData();
			last = millis();
		}
		
		if( Serial.available() ){	// some data to be processed
			cmdline.loop();	// process commands
		} else {	// nothing to do
			delay(DELAY_LIGHT);
		}
	} else {	// DeepSleep mode
		publishData();

		if( Serial.available() ){
			cmdline.enter();	// Enter in interactive mode
			last = millis();
			return;
		}
		ctx.save();
		Serial.println("Going to sleep ...");
		ESP.deepSleep(DELAY * 1e6);
	}
}
