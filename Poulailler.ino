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
	const int idx = cmd.indexOf(' ');
	String arg;
	if(idx != -1){
		arg = cmd.substring(idx + 1);
		cmd = cmd.substring(0, idx);
	}

	if(cmd == "bye"){
		this->finished();
		return;
	} else if(cmd == "1wscan"){
		String msg = "Sondes sur le bus : ";
		msg += context.getOWBus().getDeviceCount();

		msg += "\nAdresses individuelles :";
		OWBus::Address addr;
		context.getOWBus().search_reset();
		while( context.getOWBus().search_next( addr ) ){
			msg += "\n\t";
			msg += addr.toString().c_str();
			msg += " : ";
			if(!addr.isValid( context.getOWBus() ))
				msg += "Adresse Invalide\n";
			else {
				OWDevice probe( context.getOWBus(), addr );
				msg += probe.getFamily();
				msg += probe.isParasitePowered() ? " (Parasite)" : " (Externe)";
			}
		}
		context.Output(msg);
	} else if(cmd == "pub"){
		if( arg == "Dev" )
			myESP.action();
		else if( arg == "Perch" )
			perchoir.action();
		else {
			myESP.action();
			perchoir.action();
		}
	} else if(cmd == "Net"){
		if(arg == "M")
			network.setMode( Network::NetworkMode::MAISON );
		else if(arg == "D")
			network.setMode( Network::NetworkMode::DOMOTIQUE );
		else if(arg == "MD")
			network.setMode( Network::NetworkMode::SAFEMD );
		else
			network.setMode( Network::NetworkMode::SAFEDM );
	} else if(cmd == "Aux")
		auxiliaires.power( arg == "on", true );
	else if(cmd == "AuxInt"){
		if( arg.length() )
			auxiliaires.changeInterval( arg.toInt() );
		else
			context.Output( ( String("Intervalle Aux : ") + String(auxiliaires.getInterval()) ).c_str() );
	} else if(cmd == "AuxStab"){
		if( arg.length() )
			auxiliaires.setWaitTime( arg.toInt() );
		else
			context.Output( ( String("Stabilisation Aux : ") + String(auxiliaires.getWaitTime()) ).c_str() );
	} else if(cmd == "TstAux"){	// Simulate Sunlight aquisition
		auxiliaires.power(true);
		delay( auxiliaires.getWaitTime() );
		auxiliaires.water(true);	// Refresh GPIOs
		auxiliaires.power(false);
		auxiliaires.status(false);
	} else if(cmd == "ESPInt"){
		if( arg.length() )
			myESP.changeInterval( arg.toInt() );
		else
			context.Output( ( String("Intervalle ESP : ") + String(myESP.getInterval()) ).c_str() );
	} else if(cmd == "PerchInt"){
		if( arg.length() )
			perchoir.changeInterval( arg.toInt() );
		else
			context.Output( ( String("Intervalle Perchoir : ") + String(perchoir.getInterval()) ).c_str() );
	} else if(cmd == "PorteOuverte" || cmd == "po")
		porte.action( Porte::Command::OPEN );
	else if(cmd == "PorteFermee" || cmd == "pf")
		porte.action( Porte::Command::CLOSE );
	else if(cmd == "PorteStop" || cmd == "ps")
		porte.action( Porte::Command::STOP );
	else if(cmd == "PorteTimeout" || cmd == "pt")
		if( arg.length() )
			porte.setTimeout( arg.toInt() );
		else
			context.Output( ( String("Timeout porte : ") + String(porte.getTimeout()) ).c_str() );
	else if(cmd == "PorteOk")
		porte.clearErrorCondition();
	else if(cmd == "reset")
		ESP.restart();
	else if(cmd == "statut" || cmd == "status"){
		context.status();
		network.status();
		auxiliaires.status();
		porte.status();
		perchoir.status();
	} else if(cmd == "calVcc"){
#	ifdef SERIAL_ENABLED
		while(!Serial.available()){
			unsigned int v = analogRead(A0), t = v * 5000 / myESP.getCaliber();
			Serial.print("Alimentation : ");
			Serial.print( v );
			Serial.print(" -> ");
			Serial.println( t );
			delay( 500 );
		}
#endif
	} else if(cmd == "maxVcc"){
		if( arg.length() )
			myESP.setCaliber( arg.toInt() );
		else
			context.Output( ( String("Intervalle ESP : ") + String(myESP.getCaliber()) ).c_str() );
	} else {
		String msg("Commandes : Aux {on|off}, Net {M|D|MD|DM},\n"
		"ESPInt [val], PerchInt [val], AuxInt [val], AuxStab [val], TstAux,\n"
		"PorteOuverte (po), PorteFermee (pf), PorteStop (ps), PorteTimeout (pt) {val}, PorteOk\n"
		"calVcc, maxVcc [val]\n"
		"pub [Dev|Perch], statut, 1wscan, reset, bye");
		context.Output(msg);
	}
	this->prompt();
}

void handleMQTT(char* topic, byte* payload, unsigned int length){
	String msg;
	for(unsigned int i=0;i<length;i++)
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

#ifdef SERIAL_ENABLED
	Serial.println("\nValeurs initiales :\n----------");
	context.status();
	network.status();

#	if AUXPWR_GPIO != 0 
	Serial.println(	"****** Mode DEV *******" );
#	endif
#endif

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
		 * Recurrent & automatic tasks
		 * (messages handling, automatic publishing, ...)
		 */
	network.loop();
	myESP.loop();
	perchoir.loop();

		/*
		 * Complexes tasks
		 */
	switch( context.getStatus() ){
	case Context::Steps::STARTUP_STARTUP :
		auxiliaires.power( true );
		context.setStatus( Context::Steps::STARTUP_AUXPWR );
		context.Output("Etape : STARTUP_AUXPWR");
		break;
	case Context::Steps::STARTUP_AUXPWR :
		if( auxiliaires.isStabilised() ){ // Test if it's day or night
			if( auxiliaires.SunLight( true ) ){	// Day
				context.setDaylight( true );
				porte.action( Porte::Command::OPEN );
			} else {	// Night
				context.setDaylight( false );
				porte.action( Porte::Command::CLOSE );
			}
			context.setStatus( Context::Steps::STARTUP_WAIT4DOOR );
			context.Output("Etape : STARTUP_WAIT4DOOR");
		}
		break;
	case Context::Steps::STARTUP_WAIT4DOOR :
		if( !porte.isStillMoving() ){
			context.setStatus( Context::Steps::WORKING );
			context.Output("Démarrage terminé");
		}
		break;
	default:	// Up and runing
		auxiliaires.loop();
		break;
	}

		/*
		 * Is something on way ?
		 */
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
		howlong = _min( howlong, auxiliaires.remain() );

		if( in_interactive ){
			howlong = _min( howlong, DELAY_LIGHT );
			delay(howlong);
		} else if( howlong ){
#		ifdef SERIAL_ENABLED
			Serial.print("Dodo ");
			Serial.print( howlong );
			Serial.println(" mS ...");
#		endif
			context.keepTimeBeforeSleep( howlong );	// In mS
			ESP.deepSleep(howlong * 1e3);			// In uS
		} else {
#		ifdef SERIAL_ENABLED
			Serial.println("Dormir c'est un peu mourrir !");
#		endif
		}
	}
}
