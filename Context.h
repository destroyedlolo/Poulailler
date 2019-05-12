/* Context.h
 *
 * Keep all "dynamic" configuration
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <KeepInRTC.h>

#include "Parameters.h"

class Context : public KeepInRTC::KeepMe {
	struct {	// Data to be kept b/w run, saved in the RTC
		unsigned long wakeuptime;	// last time we go to sleep (in seconds : will round in 136 years ;) )
		bool verbose;				// Send informational messages
	} data;

public:
	Context(KeepInRTC &kir) : KeepInRTC::KeepMe( kir, (uint32_t *)&this->data, sizeof(this->data) ) {}

		/* Set default values if needed
		 *	<- true if data have been reseted
		 */
	bool begin( void ){
		if( !kir.isValid() ){
			this->data.wakeuptime = 0;
			this->data.verbose = DEF_VERBOSITY;
			this->save();
			return true;
		}
		return false;
	}

		/* Sleep management */
	void deepSleep( unsigned long howlong ){
		this->data.wakeuptime += millis()/1e3;	// Add the time already spent
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
};

#endif
