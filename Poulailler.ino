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
 */
#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

	/* Network */
#include <Maison.h>		// My very own environment (WIFI_*, MQTT_*, ...)
#include "Parameters.h"

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
#include "Porte.h"
#include "Auxiliaires.h"

	/* Component */
Device myESP( context );
Perchoir perchoir( context );
Porte porte( context );
Auxiliaires auxiliaires( context );

#if defined(SERIAL_ENABLED) && defined(DEV_ONLY)
#include "CommandLine.h"
CommandLine cmdline;

#include <OWBus/OWDevice.h>

void CommandLine::loop(){	// Implement command line
	String cmd = Serial.readString();

	if(cmd == "bye"){
		this->finished();
		return;
	} else if(cmd == "1wscan"){
		Serial.print("\nNumber of probes on the bus :");
		Serial.println(context.getOWBus().getDeviceCount());

		Serial.println("Individual address :");
		OWBus::Address addr;
		context.getOWBus().search_reset();
		while( context.getOWBus().search_next( addr ) ){
			Serial.print( addr.toString().c_str() );
			Serial.print(" : ");
			if(!addr.isValid( context.getOWBus() ))
				Serial.println("Invalid address");
			else {
				OWDevice probe( context.getOWBus(), addr );
				Serial.println( probe.getFamily() );
				Serial.println( probe.isParasitePowered() ? "\tParasite" : "\tExternal" );
			}
		}
	} else if(cmd == "pubDev")
		myESP.action();
	else if(cmd == "pubPerch")
		perchoir.action();
	else if(cmd == "Aux on")
		auxiliaires.power(1);
	else if(cmd == "Aux off")
		auxiliaires.power(0);
	else if(cmd == "moteur monte" || cmd == "mm")
		porte.action( Porte::Command::OPEN );
	else if(cmd == "moteur descent" || cmd == "md")
		porte.action( Porte::Command::CLOSE );
	else if(cmd == "moteur stop" || cmd == "ms")
		porte.action( Porte::Command::STOP );
	else if(cmd == "reset")
		ESP.restart();
	else if(cmd == "status"){
		context.status();
		network.status();
		auxiliaires.status();
	} else
		Serial.println("Known commands : Aux on, Aux off, Moteur monte (mm), Moteur descent (md), Moteur stop (ms), pubDev, pubPerch, status, 1wscan, reset, bye");

	this->prompt();
}
#endif

unsigned int boottime;

void setup(){
		/* Hardware configuration */
#ifdef SERIAL_ENABLED
	Serial.begin(115200);
	delay(100);
#else
	pinMode(LED_BUILTIN, OUTPUT);
#endif
	porte.setup();
	auxiliaires.setup();

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

	porte.action();	// Potentially, the ESP crashed during a movement, restoring

	boottime = millis();
}

void loop(){
	bool still_busy = false; // Do we have something left to do ?
	bool in_interactive = false;
		/*
		 * Components'
		 */
	myESP.loop();
	perchoir.loop();

		/*
		 * Command line if activated
		 */
#if defined(SERIAL_ENABLED) && defined(DEV_ONLY)
	in_interactive = cmdline.isActive();	// Can we enter in interactive mode

	if(!context.isValid() && (millis()-boottime) < DELAY_STARTUP * 1e3)	// 1st run
		in_interactive = true; // let a chance to enter in interactive mode

	if(!cmdline.isActive() && Serial.available() ){
		cmdline.enter();
		return;
	} else if( Serial.available() )	// Already in interactive mode
		cmdline.loop();
#endif
			
		/*
		 * Go to sleep if nothing left to be done
		 */
	if(!still_busy){
		if( in_interactive )
			delay(DELAY_LIGHT);
		else {
#		ifdef SERIAL_ENABLED
			Serial.println("Dodo ...");
#		endif
			auxiliaires.power(0);	// Switch off auxiliaries
			context.keepTimeBeforeSleep( DELAY * 1e3 );	// In mS
			ESP.deepSleep(DELAY * 1e6);					// In uS
		}
	}
}
