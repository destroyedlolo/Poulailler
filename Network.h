/* Network handling 
 *
 * 05/12/2017 First version
 */

#ifndef NETWORK_H
#define NETWORK_H

	/*****
	 * Notez-bien : All fields will be persisted in RTC memory
	 *****/
class Network {
	unsigned int attempts;	// If in degraded mode, counter before recovery try

protected:
	enum NetworkMode { 
		FAILURE = 0,	// In case of no network can be reached
		MAISON,			// Only one home network
		DOMOTIQUE,		// Only one Automation network
		SAFEDM, 		// Try Automation network first then home one
		SAFEMD			// Try Home network first then automation one
	} mode, current;	// which mode is in use

//	virtual void save( void ) = 0;	// Needed to call context's one

	void init( void ){ /* Initial configuration */
		this->mode = NetworkMode::SAFEMD;
		this->current = this->getNominalNetwork();
	}

	void setup( void ){}	/* Setup */

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

	enum NetworkMode getNominalNetwork( void ){	// Where to connect normally
		switch( this->mode ){
		case NetworkMode::SAFEDM :
		case NetworkMode::DOMOTIQUE :
			return NetworkMode::DOMOTIQUE;
		default:
			return NetworkMode::MAISON;
		}
	}

	enum NetworkMode getAlternateNetwork( void ){	// Where to connect alternatively
		switch( this->mode ){
		case NetworkMode::SAFEDM :
			return NetworkMode::MAISON;
		case NetworkMode::SAFEMD :
			return NetworkMode::DOMOTIQUE;
		default:
			return NetworkMode::FAILURE;
		}
	}

	void status( void ){
#ifdef DEV_ONLY
#	ifdef SERIAL_ENABLED
		Serial.print("\nNetwork\n\tmode :");
		Serial.println( this->mode );
		Serial.print("\tcurrent :");
		Serial.println( this->current );
#	endif
#endif
	}

public:
	bool isDegraded( void ){
		return( this->getNominalNetwork() != this->current );
	}
	
	void setMode( enum NetworkMode n ){ this->mode = n; }
	enum NetworkMode getMode( void ){ return this->mode; }

	enum NetworkMode connect( void ){
		if( this->isDegraded() ){
			if( ! --this->attempts )	// Back to nominal network
				this->current = this->getNominalNetwork();
		}

		if( this->current == NetworkMode::MAISON ){
			if( !this->connectMaison() ){
				if( this->getAlternateNetwork() == NetworkMode::DOMOTIQUE && // we were on the nominal network
				this->connectDomotique(false)){	// And we successfully connect to the alternate one
					this->attempts = RETRYAFTERSWITCHING;
					this->current = NetworkMode::DOMOTIQUE;
				} else {	// Failing
					this->attempts = RETRYAFTERFAILURE;
					this->current = NetworkMode::FAILURE;
				}
			}
		} else if( this->current == NetworkMode::DOMOTIQUE ){
			if( !this->connectDomotique() ){
				if( this->getAlternateNetwork() == NetworkMode::MAISON && // we were on the nominal network
				this->connectMaison(false)){	// And we successfully connect to the alternate one
					this->attempts = RETRYAFTERSWITCHING;
					this->current = NetworkMode::MAISON;
				} else {	// Failing
					this->attempts = RETRYAFTERFAILURE;
					this->current = NetworkMode::FAILURE;
				}
			}
		}

		this->status();
//		this->save();
//Serial.println("Sauve !");
		return(this->current);
	}
};

#endif
