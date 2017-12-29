/* Auxiliaires
 * 
 * Auxiliary power, solar light and water checks
 */

#ifndef AUXILIAIRE_H
#define AUXILIAIRE_H

#define WATER_GPIO	4
#define AUXPWR_GPIO	0

#include "Context.h"

#include <OWBus/DS2413.h>

#define DSADDR	0x3a77553800000091	// Address of the DS2413

class Auxiliaires {
	Context &context;
	DS2413 gpio;

public:
	Auxiliaires( Context &ctx ) : context( ctx ), 
		gpio( context.getOWBus(), DSADDR ) { }

	void setup( void ){
		gpio.writePIOs( 0xff );
		pinMode(WATER_GPIO, INPUT);
		digitalWrite(AUXPWR_GPIO, 1);	// By default Aux power is disabled
		pinMode(AUXPWR_GPIO, OUTPUT);
	}

	void power( bool v ){
#		ifdef SERIAL_ENABLED
		Serial.println( v ? "Auxillaries ON" : "Auxillaries OFF" );
#		endif

		digitalWrite(AUXPWR_GPIO, !v);	// Caution : power is active when GPIO is LOW
	}

	bool isPowered( ){
		return !digitalRead(AUXPWR_GPIO);
	}

	bool SunLight( bool refresh=true ){
		if(refresh)
			gpio.readPIOs();
		return !gpio.getPIOB();
	}

	bool water( void ){
		return digitalRead( WATER_GPIO );
	}

	void status( void ){
#if defined(DEV_ONLY) && defined(SERIAL_ENABLED)
		Serial.print("Auxillaries : ");
		Serial.print( this->isPowered(true)? "powered, " : "off, ");
		Serial.print( this->water() ? "enought water, " : "lack of water, ");
		Serial.println(this->SunLight() ? "Day" : "Night");

#endif
	}

};
#endif
