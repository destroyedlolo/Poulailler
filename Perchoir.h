/*	Handle perch figures
 *
 *	17/12/2017	First version
 *	30/03/2022	Redesign for v2
 */

#ifndef PERCH_H
#define PERCH_H

#include <SimpleDHT.h>

	/* Settings */
#include "Parameters.h"
#include "NetMQTT.h"
#include "Context.h"

class Perchoir {
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
	Perchoir( Context &ctx ) : temperature(0), humidite(0), err(SimpleDHTErrSuccess), context( ctx ){
	}

	void action( void ){
		if( !this->sample() ){
			nMQTT.publish( MQTT_Error, this->strerror() );
#			ifdef SERIAL_ENABLED
			if( ctx.getDebug() ){
				Serial.print("DHT.sample() :");
				Serial.println( this->strerror() );
			}
#			endif
			return;
		}

		String troot = MQTT_Topic + "Perchoir/";

		String tpc = troot + "Temperature";
		nMQTT.publish( tpc, context.toString( this->temperature ).c_str() );

		tpc = troot + "Humidite";
		nMQTT.publish( tpc, context.toString( this->humidite ).c_str() );

#		ifdef SERIAL_ENABLED
		if( ctx.getDebug() ){
			Serial.print("Perchoir :");
			Serial.print(this->temperature);
			Serial.print("° ");
			Serial.println(this->humidite);
		}
#		endif
	}

	void status( void ){
		String msg = "Perchoir\n\t";
		msg += this->temperature;
		msg += "° ";
		msg += this->humidite;
		nMQTT.logMsg(msg);
	}

};

#endif
