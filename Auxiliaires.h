/* Auxiliaires
 * 
 * Auxiliary power, solar light and water checks
 */

#ifndef AUXILIAIRE_H
#define AUXILIAIRE_H

#define AUXPWR_GPIO	0

#include "Context.h"
#include "Repeater.h"

#include <OWBus/DS2413.h>

#define DSADDR	0x3a77553800000091	// Address of the DS2413

class Auxiliaires : public Repeater {
	Context &context;
	DS2413 gpio;
	unsigned long int next;

public:
	Auxiliaires( Context &ctx ) : Repeater( ctx, (INTERVAL_AUX-10) * 1e3, true ),
		context( ctx ), gpio( context.getOWBus(), DSADDR ) { }

	void setup( void ){
		gpio.writePIOs( 0xff );	// Put GPIO as Input
		digitalWrite(AUXPWR_GPIO, 1);	// By default Aux power is disabled
		pinMode(AUXPWR_GPIO, OUTPUT);
	}

	void loop( void ){
		if( !this->isPowered() )	// nothing was on way
			this->Repeater::loop();
		else if( millis() > this->next ){	// all auxiliaries are powerd
				/* Action to be done */
			this->water(true);	// Refresh GPIOs
			this->power(false);	// Save power

				// Publish from saved data
			context.publish( 
				(MQTT_Topic+"Water/level").c_str(), 
				this->water() ? "Enough" : "Empty"
			);
			context.publish(
				(MQTT_Topic+"Sunlight").c_str(),
				this->SunLight() ? "Day" : "Night"
			);
		}
	}

	void action( void ){
		this->power(true);
	}

	void power( bool v ){
		context.Output( v ? "Auxillaries ON" : "Auxillaries OFF" );
		digitalWrite(AUXPWR_GPIO, !v);	// Caution : power is active when GPIO is LOW
		if(v)
			this->next = millis() + DELAY_AUX;	// initialise wakeup timer
	}

	bool isPowered( ){
		return !digitalRead(AUXPWR_GPIO);
	}

	bool SunLight( bool refresh=true ){
		if(refresh)
			gpio.readPIOs();
		return !gpio.getPIOA();
	}

	bool water( bool refresh=true ){
		if(refresh)
			gpio.readPIOs();
		return !gpio.getPIOB();
	}

	void status( void ){
#ifdef DEV_ONLY
		String msg ="Auxillaries : ";
		msg += this->isPowered()? "powered, " : "off, ";
		msg += this->water(true) ? "enough water, " : "lack of water, ";
		msg += this->SunLight() ? "Day" : "Night";

		context.Output(msg);
#endif
	}

};
#endif
