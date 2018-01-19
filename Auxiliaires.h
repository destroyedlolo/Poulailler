/* Auxiliaires
 * 
 * Auxiliary power, solar light and water checks
 */

#ifndef AUXILIAIRE_H
#define AUXILIAIRE_H

#include "Context.h"
#include "Repeater.h"

#include <OWBus/DS2413.h>

#define DSADDR	0x3a77553800000091	// Address of the DS2413

class Auxiliaires : public Repeater {
	Context &context;
	DS2413 gpio;
	unsigned long int next;
	bool testmode;					// In test mode

	struct {
		unsigned long int wait4stab;	// Wait for the capacitor to load
	} conf;	// data to be kept
	Context::keepInRTC *agarder;

public:
	Auxiliaires( Context &ctx ) : 
	Repeater( ctx, (INTERVAL_AUX-10) * 1e3, true ),
	context( ctx ), 
	gpio( context.getOWBus(), DSADDR ),
	testmode(false){
		agarder = new Context::keepInRTC( ctx, (uint32_t *)&conf, sizeof(conf) );

		if( !ctx.isValid() ){	// Default value
			conf.wait4stab = DELAY_AUX;
			this->agarder->save();
		}
	}

	void setup( void ){
		gpio.writePIOs( 0xff );	// Put GPIO as Input
		digitalWrite(AUXPWR_GPIO, 1);	// By default Aux power is disabled
		pinMode(AUXPWR_GPIO, OUTPUT);
	}

	unsigned long int getWaitTime( void ){
		return conf.wait4stab;
	}

	void setWaitTime( unsigned long int v ){
		conf.wait4stab = v;
		this->agarder->save();
	}

	void loop( void ){
		if( !this->isPowered() )	// nothing was on way
			this->Repeater::loop();
		else if( (millis() > this->next) && !testmode ){	// all auxiliaries are powerd
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

	void power( bool v, bool tst=false ){
		context.Output( v ? "Auxillaries ON" : "Auxillaries OFF" );
		digitalWrite(AUXPWR_GPIO, !v);	// Caution : power is active when GPIO is LOW
		if(v)
			this->next = millis() + conf.wait4stab;	// initialise wakeup timer

		if(tst)
			this->testmode = v;
	}

	bool isPowered( void ){
		return !digitalRead(AUXPWR_GPIO);
	}

	bool isStabilised( void ){
		return( this->isPowered() && (millis() > this->next) );
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

	void status( bool refresh=true ){
#ifdef DEV_ONLY
		String msg ="Auxillaries : ";
		msg += this->isPowered()? "powered, " : "off, ";
		msg += this->water( refresh ) ? "enough water, " : "lack of water, ";
		msg += this->SunLight(false) ? "Day" : "Night";

		context.Output(msg);
#endif
	}

};
#endif
