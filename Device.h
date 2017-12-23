/* Publish device's figures
 *
 * 23/12/2017 First version
 */

#ifndef DEVICE_H
#define DEVICE_H

	/* Settings */
#define INTERVAL 300	// Interval b/w sample in S (5 minutes)

#include "Context.h"
#include "Repeat.h"

ADC_MODE(ADC_VCC);

class Device : public Repeat {
	Context &context;

public :
	Device(Context &ctx) : Repeat( ctx, INTERVAL * 1e3, true ), context(ctx) {}

	void action( void ){
#ifdef SERIAL_ENABLED
		Serial.print("Power : ");
		Serial.println(ESP.getVcc());
#endif
		context.publish( (MQTT_Topic + "Alim").c_str(), String( ESP.getVcc() ).c_str() );

#ifdef SERIAL_ENABLED
		Serial.print("Memory : ");
		Serial.println(ESP.getFreeHeap());
#endif
		context.publish( (MQTT_Topic + "Memory").c_str(), String( ESP.getFreeHeap() ).c_str() );
	}

};

#endif
