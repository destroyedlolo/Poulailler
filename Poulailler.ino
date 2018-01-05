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

#ifdef DEV_ONLY
#include "CommandLine.h"
CommandLine cmdline;

#include <OWBus/OWDevice.h>

void CommandLine::exec( String &cmd ){	// Implement command line
	if(cmd == "bye"){
		this->finished();
		return;
	} else if(cmd == "1wscan"){
		String msg = "Number of probes on the bus : ";
		msg += context.getOWBus().getDeviceCount();

		msg += "\nIndividual address :";
		OWBus::Address addr;
		context.getOWBus().search_reset();
		while( context.getOWBus().search_next( addr ) ){
			msg += "\n\t";
			msg += addr.toString().c_str();
			msg += " : ";
			if(!addr.isValid( context.getOWBus() ))
				msg += "Invalid address\n";
			else {
				OWDevice probe( context.getOWBus(), addr );
				msg += probe.getFamily();
				msg += probe.isParasitePowered() ? " (Parasite)" : " (External)";
			}
		}
		context.Output(msg);
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
	} else {
		String msg("Known commands : Aux on, Aux off, Moteur monte (mm), Moteur descent (md), Moteur stop (ms), pubDev, pubPerch, status, 1wscan, reset, bye");
		context.Output(msg);
	}
	this->prompt();
}

void handleMQTT(char* topic, byte* payload, unsigned int length){
	String msg;
	for(int i=0;i<length;i++)
		msg += (char)payload[i];

#	ifdef SERIAL_ENABLED
	Serial.print( "Message [" );
	Serial.print( topic );
	Serial.print( "] : '" );
	Serial.print( msg );
	Serial.println( "'" );
#	endif
	
	if(!cmdline.isActive())
		cmdline.enter();

	cmdline.exec( msg );
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
	network.loop();
	myESP.loop();
	perchoir.loop();
	auxiliaires.loop();
	still_busy |= auxiliaires.isPowered();	// Waiting for power to stabilize

		/*
		 * Command line if activated
		 */
#ifdef DEV_ONLY
	in_interactive = cmdline.isActive();	// Can we enter in interactive mode

	if(!context.isValid() && (millis()-boottime) < DELAY_STARTUP * 1e3)	// 1st run
		in_interactive = true; // let a chance to enter in interactive mode

#	ifdef SERIAL_ENABLED
	if(!cmdline.isActive() && Serial.available() ){
		cmdline.enter();
		return;
	} else if( Serial.available() )	// Already in interactive mode
		cmdline.readSerial();
#	endif
#endif
			
		/*
		 * Go to sleep if nothing left to be done
		 */
	if(!still_busy){
		unsigned long int howlong = _min( myESP.remain(), perchoir.remain() );

		if( in_interactive ){
			howlong = _min( howlong, DELAY_LIGHT );
			delay(howlong);
		} else {
#		ifdef SERIAL_ENABLED
			Serial.print("Dodo ");
			Serial.print( howlong );
			Serial.println(" mS ...");
#		endif
			context.keepTimeBeforeSleep( howlong );	// In mS
			ESP.deepSleep(howlong * 1e3);			// In uS
		}
	}
}
