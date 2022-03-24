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

#include "Parameters.h"

extern "C" {
  #include "user_interface.h"
}
#ifdef DEV
ADC_MODE(ADC_VCC);	// Read internal Vcc
#endif

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

	/* Notez-bien : time are stored in Seconds */
#include <LFUtilities/TemporalConsign.h>
TemporalConsign delaySample(kir);		// Delay b/w 2 nest sample.
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
	* Commande line
	****/

#include "CommandLine.h"
CommandLine cmdline;

bool func_status( const String & ){
	String msg = "Délai acquisition : ";
	msg += delaySample.getConsign();
	msg += "S\nEveil suite à commande : ";
	msg += stayWakedInteractive.getConsign();
	msg += ctx.getDebug() ? "S\nMessages de Debug" : "ms\nPas de message de Debug";
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

bool func_reboot( const String & ){
	ESP.restart();
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
	{ "reboot", "Redemarre l'ESP", func_reboot },
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

void CommandLine::exec( String &cmd ){	// Commands parsing
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

	if(keepinteractive) // Reset the waked timer
		stayWakedInteractive.restart( millis() / 1000 );
	else { // We have to go to sleep
		stayWakedInteractive.setNext(0);
		this->finished();
	}
}

	/***
	* Handle MQTT
	***/
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

	cmdline.exec( cmd );
}


	/***
	* Let's go
	****/

void setup(){
	bool wakeup = true;		// wakeup or starting ?

#ifdef SERIAL_ENABLED
	Serial.begin(115200);
	delay(100);

	Serial.println("Hello");
#else
	pinMode(LED_BUILTIN, OUTPUT);
#endif

	if( ctx.begin() | delaySample.begin(DEF_SLEEP) | stayWakedInteractive.begin(DEF_WAKED) ){	// single non logical or, otherwise other begin() will never be called
#ifdef SERIAL_ENABLED
		Serial.println("Default value");
		Serial.printf("Command : '%s'\n", MQTT_Command.c_str());
		Serial.printf("Messages : '%s'\n", nMQTT.getMQTTMessageTopic());
		wakeup = false;
#endif
	} else {
// TODO : initialise delaySample with current Context.wakeuptime
	}

	LED(LOW);
	if( !nMQTT.connectWiFi() ){	// Will connect to MQTT as well
#ifdef SERIAL_ENABLED
		Serial.println("Unable to connect to the network ... will be back !");
#endif
		ctx.deepSleep( delaySample.getConsign() );	// sleep till next try
	}
	nMQTT.begin( MQTT_Command.c_str(), handleMQTT );
	LED(HIGH);

	if( !wakeup ){	// starting, let a chance for interactive command
#ifdef SERIAL_ENABLED
		Serial.println("starting");
#endif
		stayWakedInteractive.restart( millis()/1000 );	// reset the delay to the current time
	}
}

int getAlim( void ){
#ifdef DEV
	return( ESP.getVcc() );
#else
	return( analogRead(A0)*5 );
#endif
}

void loop(){
	nMQTT.loop();

	// Acquire samples ?
	if( delaySample.isExhausted(millis()/1000) ){
		int vcctab[3];	// store different samples of Vcc
		int nbre = 0;

		vcctab[nbre++] = getAlim();

#ifdef SERIAL_ENABLED
		Serial.println("Sampling ...");
#endif

		vcctab[nbre++] = getAlim();
	
		// TODO : add probes code

		vcctab[nbre++] = getAlim();

		// sort Vcc values
		for( int i = 0; i<nbre; i++ )
			for( int j = 0; j<nbre; j++ ){
				if(vcctab[i]>vcctab[j]){
					int t = vcctab[i];
			    	vcctab[i] = vcctab[j];
				    vcctab[j] = t;
				}
		}

		if( ctx.getDebug() ){
			String msg(nbre);
			msg += " Vcc sample :";
			for( int i=0; i<nbre; i++ ){
				msg += " ";
				msg += vcctab[i];
			}
			nMQTT.logMsg( msg );
		}

#ifdef SERIAL_ENABLED
		Serial.print("selected Vcc :");
		Serial.println( vcctab[nbre/2] );
#endif
		nMQTT.publish( MQTT_VCC, vcctab[nbre/2] );

		// reset sampling counter
		delaySample.setNext();
	}


#ifdef SERIAL_ENABLED
	if( Serial.available() ){
		if( !cmdline.isActive() ){	// entering interactive mode
			cmdline.enter();
			return;
		} else	// Process commands
			cmdline.readSerial();
	}
#endif

		// ready to go to sleep ?
		// check if we are still waiting for commands
	if(  !stayWakedInteractive.isExhausted(millis()/1000) ){
#ifdef SERIAL_ENABLED
		if( ctx.getDebug() )
			Serial.printf("Interactive (%d)...\n", stayWakedInteractive.getNext() - millis()/1000);
#endif
			delay(500);	// Wait 0.5s before next mqtt checking
	} else {	// No activities for a long time, going to sleep

			// Determine when we'll have to do next sample
		long next = delaySample.getNext() - millis()/1000;
#ifdef SERIAL_ENABLED
		Serial.printf( "Dodo ...(next : %lu, millis %lu [%ld])\n",
			delaySample.getNext(), millis()/1000, next
		);
#endif

		if(next > 0){
#ifdef SERIAL_ENABLED
			Serial.printf("Sleep for %ld sec\n", next);
#endif
			nMQTT.disconnect();
			ctx.deepSleep( next );
		} else {	// It's already time for the next sample
#ifdef SERIAL_ENABLED
			Serial.println("No, it's already time for the next sample");
#endif
			delay(500);	// Wait 0.5s before next mqtt checking
		}
	}
}
