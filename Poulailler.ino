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
#include <PubSubClient.h>

extern "C" {
  #include "user_interface.h"
}

	/******
	 * Parameters
	 *
	 *	Comment or uncomment to activate or disable some optionnal parts.
	 ******/

#define DEV_ONLY	/* Developpment only code */
/*#define STATIC_IP*/	/* Use static IP when on home network */
/*#define LED_BUILTIN 2*/	/* Workaround for my ESP-12 */

	/* If enabled, use internal LED for debugging purposes */
#if 1		// Led is lightning during Wifi / Mqtt connection establishment
#	define LED(x)	{ digitalWrite(LED_BUILTIN, x); }
#else
#	define LED(x)	{ }
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

	/* Delays */
#define DELAY	300				// Delay in seconds b/w samples (5 minutes)
#define DELAY_STARTUP	5		// Let a chance to enter in interactive mode at startup ( 5s )
#define DELAY_LIGHT 500			// Delay during light sleep (in ms - 0.5s )
#define FAILUREDELAY	900		// Delay b/w 2 network attempt in case of failure (15 minutes)

	/* 1-wire */
#define ONE_WIRE_BUS 2 // Where OW bus is connected to

	/* End of configuration area
	 * Let's go !
	 */
#include "Duration.h"
#include "CommandLine.h"
#include "Context.h"

	/* 1-wire */
#include <OWBus.h>
OneWire oneWire(ONE_WIRE_BUS);	// Initialize oneWire library
OWBus bus(&oneWire);

ADC_MODE(ADC_VCC);

WiFiClient clientWiFi;
PubSubClient clientMQTT(clientWiFi);

Context context;

void setup(){
		/* Hardware configuration */
	Serial.begin(115200);
	delay(100);
	pinMode(LED_BUILTIN, OUTPUT);

	context.Porte::setup();
	context.Porte::action();	// Restore previous movement if the ESP crashed

}

void loop(){
	bool still_busy = false; // Do we have something left to do ?

		/*
		 * Actions to be done 
		 */
	still_busy |= context.Porte::isStillMoving();

		/*
		 * Go to sleep if nothing left to be done
		 */
	if(!still_busy)
		ESP.deepSleep(DELAY * 1e6);
}
