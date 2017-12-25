/*	Poulailler
 * 	
 * 	This is my chicken coop automation
 *
 *	provided by "Laurent Faillie"
 *	Licence : "Creative Commons Attribution-NonCommercial 3.0"
 *	
 *	History :
 *	25/10/2017 - Temperature probe only
 *	12/11/2017 - Switch to OWBus
 *		---
 *	05/12/2017 - Go to final version
 */
#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

	/******
	 * Parameters
	 *
	 *	Comment or uncomment to activate or disable some optionnal parts.
	 ******/

#	define DEV_ONLY	/* Developpment only code */
/*#define STATIC_IP*/	/* Use static IP when on home network */
/*#define LED_BUILTIN 2*/	/* Workaround for my ESP-12 */

	/* If enabled, LED is lighting during network establishment 
	 * NOTEZ-BIEN : on ESP-201 this is mutually exclusif vs serial output
	 */
#if 0		// Led is lightning during Wifi / Mqtt connection establishment
#	define LED(x)	{ digitalWrite(LED_BUILTIN, x); }
#else
#	define LED(x)	{ }
#	define SERIAL_ENABLED
#endif


	/* Network */
#include "Maison.h"		// My very own environment (WIFI_*, MQTT_*, ...)

#ifdef STATIC_IP
	/* Static IP to avoid DHCP querying delay */
IPAddress adr_ip(192, 168, 0, 17);
IPAddress adr_gateway(192, 168, 0, 10);
IPAddress adr_dns(192, 168, 0, 3);
#endif

	/* MQTT */
#define MQTT_CLIENT "Poulailler"
String MQTT_Topic("Poulailler/");	// Topic's root
String MQTT_Error = MQTT_Topic + "Message";

	/* Delays */
#define DELAY	300				// Delay in seconds b/w samples (5 minutes)
/* #define DELAY	60 */
#define DELAY_STARTUP	5		// Let a chance to enter in interactive mode at startup ( 5s )
#define DELAY_LIGHT 500			// Delay during light sleep (in ms - 0.5s )

	// Network related delays
	// Caution to respect delays if in interactive mode
#define FAILUREDELAY	900		// Delay b/w 2 network attempt in case of failure (15 minutes)
#define RETRYAFTERSWITCHING	12	// number of connections before trying the nominal network in case of degraded mode
#define RETRYAFTERFAILURE	3	// number of connections before trying the nominal network in case of faillure

	/* 1-wire */
#define ONE_WIRE_BUS 2 // Where OW bus is connected to

	/* DHT */
#define pinDHT 5	// Where DHT is connected to

	/****************
	 * End of configuration area
	 * Let's go !
	 *******/
#include "Duration.h"
#include "Context.h"
Context context;

#include "Network.h"
Network network( context );

void Context::publish( const char *topic, const char *msg ){
	if( this->net )	// Otherwise, we're silently ignoring the publishing
		this->net->publish( topic, msg );
}

#include "CommandLine.h"
#include "Perchoir.h"
#include "Device.h"

	/* 1-wire */
#include <OWBus.h>
OneWire oneWire(ONE_WIRE_BUS);	// Initialize oneWire library
OWBus bus(&oneWire);

	/* Component */
Device myESP( context );
Perchoir perchoir( context );

void setup(){
		/* Hardware configuration */
#ifdef SERIAL_ENABLED
	Serial.begin(115200);
	delay(100);
#else
	pinMode(LED_BUILTIN, OUTPUT);
#endif

#	ifdef SERIAL_ENABLED
	Serial.println("\nInitial setup :\n----------");
	context.status();
	network.status();
#	endif

	context.save();	// At least, default values have been set
	LED(LOW);
	network.connect();
	context.setNetwork( &network );
	LED(HIGH);
}

void loop(){
	bool still_busy = false; // Do we have something left to do ?

		/*
		 * Components'
		 */
	myESP.loop();
	perchoir.loop();

		/*
		 * Go to sleep if nothing left to be done
		 */
	if(!still_busy){
#	ifdef SERIAL_ENABLED
		Serial.println("Dodo ...");
#	endif
		context.keepTimeBeforeSleep( DELAY * 1e3 );	// In mS
		ESP.deepSleep(DELAY * 1e6);					// In uS
	}
}
