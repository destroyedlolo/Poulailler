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

public :
	Device(Context &ctx) : Repeater( ctx, (INTERVAL_DEVICE-10) * 1e3, true ), context(ctx) {}	// '-10' ensures a launch if the "sample" time is the same

	void action( void ){
		unsigned int t = analogRead(A0) * 5000 / 1024;
#ifdef SERIAL_ENABLED
		Serial.print("Power : ");
		Serial.print( analogRead(A0) );
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
