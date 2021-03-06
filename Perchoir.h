/*	Handle perch figures
 *
 *	17/12/2017	First version
 */

#ifndef PERCHOIR_H
#define PERCHOIR_H

#include <SimpleDHT.h>
#include <string>

	/* Settings */
#include "Parameters.h"
#include "Context.h"
#include "Repeater.h"

class Perchoir : public Repeater {
	float temperature;
	float humidite;
	int err;

	Context &context;

protected :
	bool sample( void ){
		SimpleDHT22 DHT;

		err = DHT.read2(pinDHT, &this->temperature, &this->humidite, NULL);

		return( err == SimpleDHTErrSuccess );
	}

	const char *strerror( void ){
		switch( this->err ){
		case SimpleDHTErrSuccess: return "No error";
		case SimpleDHTErrStartLow: return "Error to wait for start low signal";
		case SimpleDHTErrStartHigh: return "Error to wait for start high signal";
		case SimpleDHTErrDataLow: return "Error to wait for data start low signal";
		case SimpleDHTErrDataRead: return "Error to wait for data read signal";
		case SimpleDHTErrDataEOF: return "Error to wait for data EOF signal";
		case SimpleDHTErrDataChecksum: return "Error to validate the checksum";
		case SimpleDHTErrZeroSamples: return "Both temperature and humidity are zero";
		default: return "Who know ...";
		}
	}

public :
	Perchoir( Context &ctx ) : Repeater( ctx, (INTERVAL_PERCHOIR-10) * 1e3, true ), temperature(0), humidite(0), err(SimpleDHTErrSuccess), context( ctx ) {
	}

	void action( void ){
		if( !this->sample() ){
			context.publish( MQTT_Error, this->strerror() );
#			ifdef SERIAL_ENABLED
			Serial.print("DHT.sample() :");
			Serial.println( this->strerror() );
#			endif
			return;
		}

		String troot = MQTT_Topic + "Perchoir/";

		context.publish( (troot+"Temperature").c_str(), context.toString( this->temperature ).c_str() );
		context.publish( (troot+"Humidite").c_str(), context.toString( this->humidite ).c_str() );

#		ifdef SERIAL_ENABLED
		Serial.print("Perchoir :");
		Serial.print(this->temperature);
		Serial.print("° ");
		Serial.println(this->humidite);
#		endif
	}

	void status( void ){
#ifdef DEV_ONLY
		String msg = "Perchoir\n\tProchain échantillonage : ";
		msg += this->remain();
		msg += "ms\n\t";
		msg += this->temperature;
		msg += "° ";
		msg += this->humidite;
		context.Output(msg);
#endif
	}
};
#endif
