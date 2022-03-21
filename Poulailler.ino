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
 *
 * Notez-bien : using String to concatenate messages is generally a BAD IDEA
 * as leading to memory fragmentation.
 * Now, it's harmless here as the ESP is reset at every deepsleep 
 * wakeup b/w samples acquisition
 */

#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

#include <Maison.h>		// My very own environment (WIFI_*, MQTT_*, ...)

#include <LFUtilities.h>	// Fake but needed to load LFUtilities
#include <LFUtilities/Duration.h>	// Duration measurement 

	/****
	* Configuration management
	*****/
#include <KeepInRTC.h>
KeepInRTC kir;	// RTC memory management

#include "Context.h"
Context ctx(kir);

#include <OWBus/OWDevice.h>

#include <LFUtilities/TemporalConsign.h>
TemporalConsign delaySampleNest(kir);		// Delay b/w 2 nest sample.
TemporalConsign stayWakedInteractive(kir);	// How long to stay waked in interactive mode

	/***
	* Networks
	****/
#include "NetMQTT.h"

WiFiClient clientWiFi;
NetMQTT nMQTT(
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

	/***
	* Handle MQTT
	***/
bool func_status( const String & ){
	String msg = "Délai acquisition : ";
	msg += delaySampleNest.getConsign();
	msg += "\nEveil suite à commande : ";
	msg += stayWakedInteractive.getConsign();
	msg += ctx.getDebug() ? "\nMessages de Debug" : "\nPas de message de Debug";
#ifdef DEV
	msg += "\nFlash : ";
	msg += ESP.getFlashChipSize();
	msg += " (real : ";
	msg += ESP.getFlashChipRealSize();
	msg += ")";
#endif
	msg += "\nSketch : ";
	msg += 	ESP.getSketchSize();
	msg += ", Libre : ";
	msg += ESP.getFreeSketchSpace();
	msg += "\nHeap :";
	msg += ESP.getFreeHeap();
	msg += "\nAdresse IP :";
	msg += WiFi.localIP().toString();

/*
	msg += OTA ? "\nOTA en attente": "\nOTA désactivé";
*/
	nMQTT.logMsg( msg );
	return true;
}

bool func_1wscan( const String & ){
	String msg = "Sondes sur le bus : ";
	msg += ctx.getOWBus().getDeviceCount();

	msg += "\nAdresses individuelles :";
	OWBus::Address addr;
	ctx.getOWBus().search_reset();
	while( ctx.getOWBus().search_next( addr ) ){
		msg += "\n\t";
		msg += addr.toString().c_str();
		msg += " : ";
		if(!addr.isValid( ctx.getOWBus() ))
			msg += "Adresse Invalide\n";
		else {
			OWDevice probe( ctx.getOWBus(), addr );
			msg += probe.getFamily();
			msg += probe.isParasitePowered() ? " (Parasite)" : " (Externe)";
		}
	}

	nMQTT.logMsg( msg );
	return true;
}

const struct _command {
	const char *nom;
	const char *desc;
	bool (*func)( const String & );	// true : stay awake, reset the timer
} commands[] = {
	{ "status", "Configuration courante", func_status },
	{ "1Wscan",	"Liste les sondes présentent sur le bus 1-wire", func_1wscan },

/*
	{ "delai", "Délai entre chaque échantillons (secondes)", func_delai },
	{ "attente", "Attend <n> secondes l'arrivée de nouvelles commandes", func_att },
	{ "dodo", "Sort du mode interactif et place l'ESP en sommeil", func_dodo },
	{ "reste", "Reste encore <n> secondes en mode interactif", func_reste },
	{ "debug", "Active (1) ou non (0) les messages de debug", func_debug },
	{ "OTA", "Active l'OTA jusqu'au prochain reboot", func_OTA },
*/
	{ NULL, NULL, NULL }
};

void handleMQTT(char* topic, byte* payload, unsigned int length){
	String cmd;
	for(unsigned int i=0;i<length;i++)
		cmd += (char)payload[i];

#	ifdef SERIAL_ENABLED
	Serial.print( "Message [" );
	Serial.print( topic );
	Serial.print( "] : '" );
	Serial.print( cmd );
	Serial.println( "'" );
#	endif

	/* Split the command and its argument */
	const int idx = cmd.indexOf(' ');
	String arg;
	if(idx != -1){
		arg = cmd.substring(idx + 1);
		cmd = cmd.substring(0, idx);
	}

	bool keepinteractive = true;	// We stay in interactive mode
	if(cmd == "?"){	// return the list of known commands
		String rep;
		if( arg.length() ) { // Looking for a specific command
			rep = arg + " : ";

			for( const struct _command *c = commands; c->nom; c++ ){
				if( arg == c->nom && c->desc ){
					rep += c->desc;
					break;	// Found it, returning
				}
			}
		} else {
			rep = "Known commands :";

			for( const struct _command *c = commands; c->nom; c++ ){
				rep += ' ';
				rep += c->nom;
			}
		}

		nMQTT.logMsg( rep );
	} else {	// Launch a command
		for( const struct _command *c = commands; c->nom; c++ ){
			if( cmd == c->nom && c->func ){
				keepinteractive = c->func( arg );
				break;
			}
		}
	}

	if(keepinteractive){ // Reset the waked timer
		stayWakedInteractive.setNext( millis() + stayWakedInteractive.getConsign() * 1e3 );
	} else { // We have to go to sleep
		stayWakedInteractive.setNext(0);
	}
}

	/***
	* Let's go
	****/

void setup(){
#ifdef SERIAL_ENABLED
	Serial.begin(115200);
	delay(100);

	Serial.println("Hello");
#else
	pinMode(LED_BUILTIN, OUTPUT);
#endif

	if( ctx.begin() | delaySampleNest.begin(DEF_NESTSLEEP) | stayWakedInteractive.begin(DEF_WAKED) ){	// single non logical or, otherwise other begin() will never be called
#ifdef SERIAL_ENABLED
		Serial.println("Default value");
#endif
	}

	if( !nMQTT.connectWiFi() ){	// Will connect to MQTT as well
#ifdef SERIAL_ENABLED
		Serial.println("Unable to connect to the network ... will be back !");
#endif
		ctx.deepSleep( delaySampleNest.getConsign() );	// sleep till next try
	}
	nMQTT.begin( MQTT_Command.c_str(), handleMQTT );
}

void loop(){
	nMQTT.loop();

	if( stayWakedInteractive.getNext() < millis() ){	// No activities for too long
#ifdef SERIAL_ENABLED
		Serial.println( "Dodo ..." );
#endif
		long next = delaySampleNest.getNext() - millis();	// Time of the next sample
		if(next > 0){
#ifdef SERIAL_ENABLED
			Serial.print("Sleep for ");
			Serial.print(next);
			Serial.println("ms ...");
#endif
//			ESP.deepSleep( next * 1e3 ); // because deepsleep is in uS
			delay(10e3);	// Fake for testing purgpose
		} else
			delay(5e3);	// Wait 5s before next mqtt checking
	}
}
