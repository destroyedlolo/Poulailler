/* Auxiliaires
 * 
 * Auxiliary power, solar light and water checks
 */

#ifndef AUXILIAIRE_H
#define AUXILIAIRE_H

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
		gpio.writePIOs( 0xff );	// Put GPIO as Input
		digitalWrite(AUXPWR_GPIO, 1);	// By default Aux power is disabled
		pinMode(AUXPWR_GPIO, OUTPUT);
	}

	void power( bool v ){
		context.Output( v ? "Auxillaries ON" : "Auxillaries OFF" );
		digitalWrite(AUXPWR_GPIO, !v);	// Caution : power is active when GPIO is LOW
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
		msg += this->water(true) ? "enought water, " : "lack of water, ";
		msg += this->SunLight() ? "Day" : "Night";

		context.Output(msg);
#endif
	}

};
#endif
