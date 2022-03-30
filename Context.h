/* Context.h
 *
 * Keep all "dynamic" configuration
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <KeepInRTC.h>
#include <OWBus.h> /* 1-wire */

#include "Parameters.h"

class Context : public KeepInRTC::KeepMe {
	struct {	// Data to be kept b/w run, saved in the RTC
		unsigned long wakeuptime;	// last time we go to sleep (in seconds : will round in 136 years ;) )
		bool verbose;				// Send informational messages
		bool debug;					// Debug mode
	} data;

	OneWire oneWire;
	OWBus bus;

public:
	Context(KeepInRTC &kir) : KeepInRTC::KeepMe( kir, (uint32_t *)&this->data, sizeof(this->data) ), oneWire(ONE_WIRE_BUS), bus(&this->oneWire) {}

		/* Set default values if needed
		 *	<- true if data have been reseted
		 */
	bool begin( void ){
		if( !kir.isValid() ){
			this->data.wakeuptime = 0;
			this->data.verbose = DEF_VERBOSITY;
#		ifdef DEV
			this->data.debug = true;
#		else
			this->data.debug = false;
#endif
			this->save();
			return true;
		}
		return false;
	}

		/* Sleep management
		 *	<- howlong to sleep (in secondes)
		 */
	void deepSleep( unsigned long howlong ){
		this->data.wakeuptime += millis()/1e3;	// Add the seconds already spent
		this->data.wakeuptime += howlong;		// Add sleep duration
		this->save();

		ESP.deepSleep( howlong * 1e6 );
	}

		/* Indicate if informational messages have to be send or not */
	void beVerbose( bool arg ){
		this->data.verbose = arg;
		this->save();
	}

	bool getVerbose( void ){
		return this->data.verbose;
	}

		/* Enable debugging message */
	void setDebug(bool v){
		this->data.debug = v;
		this->save();
	}

	bool getDebug( void ){
		return this->data.debug;
	}

	OWBus &getOWBus( void ){ 
		return this->bus;
	}


	/********
	 * Helpers
	 ********/

	String toString( float number, uint8_t digits=2){	// Largely inspired by Print::printFloat()
		String res;

		if (isnan(number)) return "nan";
		if (isinf(number)) return "inf";

		bool negative = false;
		if(number < 0.0){
			number = -number;
			negative = true;
		}

			// Round correctly so that print(1.999, 2) prints as "2.00"
		float rounding = 0.5;
		for (uint8_t i=0; i<digits; ++i)
			rounding /= 10.0;
		number += rounding;

		unsigned long int_part = (unsigned long)number;
		float remainder = number - (float)int_part;

		do {
			char c = int_part % 10;
			int_part /= 10;

			res = (char)(c+'0') + res;
		} while( int_part );
		
		if( negative )
			res = '-' + res;

		res += '.';

		if( digits ) while(digits--){
			remainder *= 10.0;
			res += (char)((char)remainder + '0');
			remainder -= (int)remainder;
		}

		return res;
	}
};

#endif
