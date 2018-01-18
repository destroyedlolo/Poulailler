/* Publish device's figures
 *
 * 23/12/2017 First version
 */

#ifndef DEVICE_H
#define DEVICE_H

#include "Parameters.h"
#include "Context.h"
#include "Repeater.h"

class Device : public Repeater {
	Context &context;

	struct {
		unsigned int caliber;
	} data;
	Context::keepInRTC *tokeep;
public :
	Device(Context &ctx) : Repeater( ctx, (INTERVAL_DEVICE-10) * 1e3, true ), context(ctx) {	// '-10' ensures a launch if the "sample" time is the same
		tokeep = new Context::keepInRTC( ctx, (uint32_t *)&data, sizeof(data) );
		if( !ctx.isValid() ){	// Default value
			this->data.caliber = 1024;
			this->tokeep->save();
		}
	}

	void setCaliber( unsigned int v ){
		if( !v || v>1024 )
			v=1024;
		this->data.caliber = v;
		this->tokeep->save();
	}

	unsigned int getCaliber( void ){ return this->data.caliber; }

	void action( void ){
		unsigned int v = analogRead(A0), t = v * 5000 / this->getCaliber();
#ifdef SERIAL_ENABLED
		Serial.print("Power : ");
		Serial.print( v );
		Serial.print(" -> ");
		Serial.println( t );
#endif
		context.publish( (MQTT_Topic + "Alim").c_str(), String( t ).c_str() );

#ifdef SERIAL_ENABLED
		Serial.print("Memory : ");
		Serial.println(ESP.getFreeHeap());
#endif
		context.publish( (MQTT_Topic + "Memory").c_str(), String( ESP.getFreeHeap() ).c_str() );
	}

};

#endif
