/* Auxiliaires
 * 
 * Axillary power, solar light and water checks
 */

#ifndef AUXILIAIRE_H
#define AUXILIAIRE_H

#include "Context.h"

#include <OWBus/DS2413.h>

#define DSADDR	0x3a77553800000091	// Address of the DS2413

class Auxiliaires {
	Context &context;
	DS2413 gpio;

public:
	Auxiliaires( Context &ctx ) : context( ctx ), 
		gpio( context.getOWBus(), DSADDR ) { }

	void power( bool v ){
#		ifdef SERIAL_ENABLED
		Serial.println( v ? "Auxillaries ON" : "Auxillaries OFF" );
#		endif

		gpio.setPIOA(!v);
	}

	bool isPowered( bool refresh=true ){
		if(refresh)
			gpio.readPIOs();
		return !gpio.getFlipFlopA();
	}

	bool SunLight( bool refresh=true ){
		if(refresh)
			gpio.readPIOs();
		return !gpio.getPIOB();
	}

	void status( void ){
#if defined(DEV_ONLY) && defined(SERIAL_ENABLED)
		Serial.print("Auxillaries : ");
		Serial.print( this->isPowered(true)? "powered " : "off ");
		Serial.println(this->SunLight() ? "Day" : "Night");
#endif
	}

};
#endif
