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
 *	30/12/2017 - Externalise parameters
 *		---
 *	05/12/2017 - Go to final version
 *
 *		----- V2 ----
 *	08/05/2019 - redesign for v2.
 */
#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

#include <Maison.h>		// My very own environment (WIFI_*, MQTT_*, ...)
#include "Parameters.h"

#include <LFUtilities.h>	// Fake but needed to load LFUtilities
#include <LFUtilities/Duration.h>	// Duration measurement 
#include <LFUtilities/SafeMQTTClient.h>	// Network connection

	/****
	* Configuration management
	*****/
#include <KeepInRTC.h>
KeepInRTC kir;	// RTC memory management

#include "Context.h"
Context ctx(kir);

#include <LFUtilities/TemporalConsign.h>
TemporalConsign delaySampleNest(kir);	// Delay b/w 2 nest sample.

	/***
	* Let's go
	****/
WiFiClient clientWiFi;
SafeMQTTClient sMQTTc(
		clientWiFi,
#ifdef DEV
		WIFI_SSID, WIFI_PASSWORD,	// Connect to my own home network during development phase
#else
		DOMO_SSID, DOMO_PASSWORD,	// "Smart home" network for production
#endif
		BROKER_HOST, BROKER_PORT, MQTT_CLIENT,	// Broker's
		MQTT_CLIENT,	// Messages' root
		false	// Doesn't clean waiting messages
);

void setup(){
#ifdef SERIAL_ENABLED
	Serial.begin(115200);
	delay(100);

	Serial.println("Hello");
#else
	pinMode(LED_BUILTIN, OUTPUT);
#endif

	if( ctx.begin() | delaySampleNest.begin(DEF_NESTSLEEP) /*| ctx.begin() */){	// single non logical or, otherwise other begin() will never be called
#ifdef SERIAL_ENABLED
		Serial.println("Default value");
#endif
	}

	if( !sMQTTc.connectWiFi() ){
#ifdef SERIAL_ENABLED
		Serial.println("Unable to connect to the network ... will be back !");
#endif
		ctx.deepSleep( delaySampleNest.getConsign() );
	}
}

void loop(){
}
