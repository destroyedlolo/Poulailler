/* Network handling 
 *
 * 05/12/2017 First version
 * 20/12/2017 Full redesign
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient clientWiFi;
PubSubClient clientMQTT(clientWiFi);

#include "Context.h"
#include "Duration.h"

#ifdef DEV_ONLY
extern void handleMQTT(char*, byte*, unsigned int);
#endif

class Network : public Context::keepInRTC {
public:
	enum NetworkMode { 
			FAILURE = 0,	// In case of no network can be reached
			MAISON,			// Only one home network
			DOMOTIQUE,		// Only one Automation network
			SAFEDM, 		// Try Automation network first then home one
			SAFEMD			// Try Home network first then automation one
	};

	const char *toString( enum NetworkMode n ){
		switch(n){
		default : return "Echec";
		case MAISON : return "Maison";
		case DOMOTIQUE : return "Domotique";
		case SAFEDM : return "Securise D->M";
		case SAFEMD : return "Securise M->D";
		};
	}

private:
	bool changed;			// Publish the network as it changed
	unsigned long MQTTFailure;	// Time of MQTT failure

	struct {
		unsigned int attempts;	// If in degraded mode, counter before recovery try
		enum NetworkMode mode, current;	// which mode is in use
	} data;	// Data to be kept

	enum NetworkMode getNominalNetwork( void ){	// Where to connect normally
		switch( this->data.mode ){
		case NetworkMode::SAFEDM :
		case NetworkMode::DOMOTIQUE :
			return NetworkMode::DOMOTIQUE;
		default:
			return NetworkMode::MAISON;
		}
	}

	enum NetworkMode getAlternateNetwork( void ){	// Where to connect alternatively
		switch( this->data.mode ){
		case NetworkMode::SAFEDM :
			return NetworkMode::MAISON;
		case NetworkMode::SAFEMD :
			return NetworkMode::DOMOTIQUE;
		default:
			return NetworkMode::FAILURE;
		}
	}

		/***
		 * Notez-Bien : settings are all externally defined
		 ***/
	bool connectMaison( bool persistant = true ){
#ifdef SERIAL_ENABLED
		Serial.println("Connexion au reseau la maison");
#endif

		if( !persistant )
			WiFi.persistent(false);

#	if STATIC_IP
		WiFi.config(adr_ip, adr_gateway, adr_dns);
#	endif
		WiFi.begin( WIFI_SSID, WIFI_PASSWORD );
		for( int i=0; i< 240; i++ ){
			if(WiFi.status() == WL_CONNECTED){
#ifdef SERIAL_ENABLED
			Serial.println("ok");
#endif
				if( !persistant )
					WiFi.persistent(true);
				return true;
			}
			delay(500);
#ifdef SERIAL_ENABLED
			Serial.print(".");
#endif
		}
		return false;
	}

	bool connectDomotique( bool persistant = true ){
#ifdef SERIAL_ENABLED
		Serial.println("Connexion au reseau Domotique");
#endif

		if( !persistant )
			WiFi.persistent(false);

		WiFi.begin( DOMO_SSID, DOMO_PASSWORD );
		for( int i=0; i< 240; i++ ){
			if(WiFi.status() == WL_CONNECTED){
#ifdef SERIAL_ENABLED
			Serial.println("ok");
#endif
				if( !persistant )
					WiFi.persistent(true);
				return true;
			}
			delay(500);
#ifdef SERIAL_ENABLED
			Serial.print("=");
#endif
		}
		return false;
	}

	bool MQTTConnect( void ){
#ifdef SERIAL_ENABLED
		Serial.println("Connexion MQTT");
#endif

		for( unsigned long t=millis(); millis()< t + MQTTRETRY * 1e3; ){
			if(clientMQTT.connect(MQTT_CLIENT,false)){
#ifdef SERIAL_ENABLED
				Serial.println("connecte");
#endif
				clientMQTT.subscribe(MQTT_Command.c_str(), 1);	// QoS 1 to ensure message delivery at wakeup
				return true;
			} else {
#ifdef SERIAL_ENABLED
				Serial.print(millis() - t);
				Serial.print(" : Echec, rc:");
				Serial.println(clientMQTT.state());
#endif
				delay(1000);	// Test dans 1 seconde
			}
		}

		this->MQTTFailure = millis();
#ifdef SERIAL_ENABLED
		Serial.println("MQTT : Echec pour se connecter");
#endif
		return false;
	}

public:
	void status( void ){
#ifdef DEV_ONLY
		String msg = "Reseau\n\tmode :";
		msg += this->toString(this->data.mode);
		msg += "\n\tcourant :";
		msg += this->toString(this->data.current);
		msg += this->changed ? " (change)" : " (conserve)";
		context.Output(msg);
#endif
	}

	Network( Context &ctx ) : Context::keepInRTC( ctx, (uint32_t *)&data, sizeof(data) ), changed(false), MQTTFailure(0) {
		if( !ctx.isValid() ){	// Default value
			this->data.mode = NetworkMode::SAFEDM;
			this->data.current = this->getNominalNetwork();
			this->changed = true;
			this->save();
		}
		clientMQTT.setServer(BROKER_HOST, BROKER_PORT);
#ifdef DEV_ONLY
		clientMQTT.setCallback( handleMQTT );
#endif
	}

	bool isDegraded( void ){
		return( this->getNominalNetwork() != this->data.current );
	}
	
	void setMode( enum NetworkMode n ){ this->data.mode = n; }
	enum NetworkMode getMode( void ){ return this->data.mode; }

	enum NetworkMode networkConnect( void ){
		if( this->data.current == NetworkMode::FAILURE )
			this->data.current = this->getNominalNetwork();
		else if( this->isDegraded() ){
			if( ! --this->data.attempts ){	// Back to nominal network
				this->data.current = this->getNominalNetwork();
				this->changed = true;		// It will be wrong if the fall back again to alternate network but it's harmless
			}
		}

		if( this->data.current == NetworkMode::MAISON ){
			if( !this->connectMaison() ){
				if( this->getAlternateNetwork() == NetworkMode::DOMOTIQUE && // we were on the nominal network
				this->connectDomotique(false)){	// And we successfully connect to the alternate one
					this->data.attempts = RETRYAFTERSWITCHING;
					this->data.current = NetworkMode::DOMOTIQUE;
					this->changed = true;
				} else {	// Failing
					this->data.attempts = RETRYAFTERFAILURE;
					this->data.current = NetworkMode::FAILURE;
				}
			}
		} else if( this->data.current == NetworkMode::DOMOTIQUE ){
			if( !this->connectDomotique() ){
				if( this->getAlternateNetwork() == NetworkMode::MAISON && // we were on the nominal network
				this->connectMaison(false)){	// And we successfully connect to the alternate one
					this->data.attempts = RETRYAFTERSWITCHING;
					this->data.current = NetworkMode::MAISON;
					this->changed = true;
				} else {	// Failing
					this->data.attempts = RETRYAFTERFAILURE;
					this->data.current = NetworkMode::FAILURE;
				}
			}
		}

		this->status();
		this->save();	// If network has changed
		return(this->data.current);
	}

	bool connect( void ){
		/* <- false if the network can't be connected
		 *
		 * MQTT will connect automatically when publishing
		 */

		Duration dwifi;
		if( this->networkConnect() == NetworkMode::FAILURE )
			return false;
		dwifi.Finished();

#ifdef SERIAL_ENABLED
		Serial.print("Duree connexion WiFi :");
		Serial.println( *dwifi );
#endif
		this->publish( (MQTT_Topic + "Wifi").c_str(), String( *dwifi ).c_str() );
		this->publish( (MQTT_Topic + "Reseau").c_str(), this->toString(this->data.current) );
		if( this->changed )
			this->publish( (MQTT_Topic + "Reseau/Change").c_str(), this->toString(this->data.current) );
		return true;
	}


		/******
		 * MQTT publishing
		 ******/

	void publish( const char *topic, const char *msg ){
		if(!clientMQTT.connected()){
			if( this->MQTTFailure && millis() < this->MQTTFailure + MQTTRETRY * 1e3 )	// Broker connection failed
				return;

			Duration dmqtt;
			if(!this->MQTTConnect())	// Failing to reconnect
				return;
			dmqtt.Finished();

#ifdef SERIAL_ENABLED
			Serial.print("Duree connexion MQTT :");
			Serial.println( *dmqtt );
#endif
			clientMQTT.publish( (MQTT_Topic + "MQTT").c_str(), String( *dmqtt ).c_str() );
		}
		clientMQTT.publish( topic, msg );
	}

	void loop( void ){
		if(clientMQTT.connected())
			clientMQTT.loop();
	}
};
#endif
