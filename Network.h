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

class Network : public Context::keepInRTC {
public:
	enum NetworkMode { 
			FAILURE = 0,	// In case of no network can be reached
			MAISON,			// Only one home network
			DOMOTIQUE,		// Only one Automation network
			SAFEDM, 		// Try Automation network first then home one
			SAFEMD			// Try Home network first then automation one
	};

private:

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
		Serial.println("Connecting to home network");
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
		Serial.println("Connecting to Domotique network");
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
			Serial.print("-");
#endif
		}
		return false;
	}

	void MQTTConnect( void ){
#ifdef SERIAL_ENABLED
		Serial.println("Connecting to MQTT");
#endif

		while(!clientMQTT.connected()){
			if(clientMQTT.connect(MQTT_CLIENT)){
#ifdef SERIAL_ENABLED
				Serial.println("connected");
#endif
				break;
			} else {
#ifdef SERIAL_ENABLED
				Serial.print("Failure, rc:");
				Serial.println(clientMQTT.state());
#endif
				delay(1000);	// Test dans 1 seconde
			}
		}
	}

public:
	void status( void ){
#ifdef DEV_ONLY
#	ifdef SERIAL_ENABLED
		Serial.print("Network\n\tmode :");
		Serial.println( this->data.mode );
		Serial.print("\tcurrent :");
		Serial.println( this->data.current );
#	endif
#endif
	}

	Network( Context &ctx ) : Context::keepInRTC( ctx, (uint32_t *)&data, sizeof(data) ){
		if( !ctx.isValid() ){	// Default value
			this->data.mode = NetworkMode::SAFEMD;
			this->data.current = this->getNominalNetwork();
			this->save();
		}
		clientMQTT.setServer(BROKER_HOST, BROKER_PORT);
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
			if( ! --this->data.attempts )	// Back to nominal network
				this->data.current = this->getNominalNetwork();
		}

		if( this->data.current == NetworkMode::MAISON ){
			if( !this->connectMaison() ){
				if( this->getAlternateNetwork() == NetworkMode::DOMOTIQUE && // we were on the nominal network
				this->connectDomotique(false)){	// And we successfully connect to the alternate one
					this->data.attempts = RETRYAFTERSWITCHING;
					this->data.current = NetworkMode::DOMOTIQUE;
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
		 */
		if( this->networkConnect() == NetworkMode::FAILURE )
			return false;
		
		this->MQTTConnect();
		return true;
	}
};

#endif
