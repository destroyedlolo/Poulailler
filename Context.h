/* Persisting context b/w runs
 *
 * 30/10/2017 First version
 * 20/12/2017 Full redesign
 */
#ifndef CONTEXT_H
#define CONTEXT_H

	/* 1-wire */
#include <OWBus.h>

class Network;

class Context {
	bool RTCvalid;	// Is stored memory valid ?
	class Network *net;
	uint32_t offset;	// Offset in memory for next data

	OneWire oneWire;
	OWBus bus;

public:
	enum Steps {
		STARTUP_STARTUP = 0,
		STARTUP_AUXPWR,			// Aux powered
		STARTUP_WAIT4DOOR,		// Wait for door to stops
		WORKING
	};

private:
	struct {
		uint32_t key;				// is RTC valid ?
		unsigned long int timeoffset;	// Offset for millis()
		enum Steps status;			// startup status
		unsigned int daylight : 1;	// day or night
	} keep;

public:
	Context() : RTCvalid(false), net(NULL), oneWire(ONE_WIRE_BUS), bus(&this->oneWire) {
			/* Check if RTC memory contains valuable data */
		if(ESP.rtcUserMemoryRead(0, (uint32_t *)&this->keep, sizeof(this->keep))){
			if( this->keep.key == ESP.getFlashChipId() )
				this->RTCvalid = true;
		}

		if( !this->RTCvalid ){	// Initial values
			keep.timeoffset = 0; // Reset timekeeper
			keep.key = ESP.getFlashChipId();
			keep.status = Steps::STARTUP_STARTUP;

			/* RTCvalid remains invalid in order to let other modules
			 * to initialise themselves to default values
			 */
		}

		offset = sizeof(keep);
	}

	void setStatus( enum Steps s ){
		this->keep.status = s;
		this->save();
	}
	enum Steps getStatus( void ){ return this->keep.status; }

	bool getDaylight( void ){ return this->keep.daylight; }
	void setDaylight( bool v ){
		this->keep.daylight = v;
		this->save();
	}

	void setNetwork( Network *n ){ net = n; }

	OWBus &getOWBus( void ){ return bus; }

	bool isValid( void ){ return RTCvalid; }

		/* This method has to be called ONLY after all other stuffs
		 * to be kept are initialised and saved
		 */
	void save( void ){
		ESP.rtcUserMemoryWrite(0, (uint32_t *)&this->keep, sizeof(this->keep));
//		RTCvalid = true;
	}

		/* Store current time offset before going to deep sleep 
		 *
		 * This methods *MUST* be called just before deepsleep otherwise
		 * the offset will be corrupted
		 *
		 * <- tts : time to stay in deep sleep (ms)
		 */
	void keepTimeBeforeSleep( unsigned long tts ){
		keep.timeoffset += millis() + tts;
		this->save();
	}

	unsigned long getTime( void ){
		return( this->keep.timeoffset + millis() );
	}

	uint32_t reserveData( uint32_t s ){
		uint32_t start = this->offset;
		this->offset += s;

		return( start );
	}

	void status( void ){
#ifdef DEV_ONLY
		String msg = "Context : RTC ";
		msg += this->isValid() ? "valid" : "invalid";
		msg += "\n\tRTC data size : ";
		msg += offset;
		msg += "\n\tTime offset : ";
		msg += this->keep.timeoffset;

		this->Output( msg );
#endif
	}

	void Output( const char *msg ){
#		ifdef SERIAL_ENABLED
		Serial.println( msg );
#		endif
		this->publish( MQTT_Output, msg );
	}

	void Output( String &msg ){
		this->Output( msg.c_str() );
	}

	class keepInRTC {
		uint32_t *what;
		uint32_t size;
		uint32_t offset;

	public:
		keepInRTC( Context &ctx, uint32_t *w, uint32_t s ) : what(w), size(s) {
		/* ->	ctx : context managing the RTC memory
		 * 		w : which data to save
		 * 		s : size of the data to save
		 */
			this->offset = ctx.reserveData( s );

			if(ctx.isValid())	// previous data can be retrieved
				ESP.rtcUserMemoryRead(this->offset, this->what, this->size);
		}

		void save( void ){
			ESP.rtcUserMemoryWrite( this->offset, this->what, this->size );
		}
	};

	/********
	 * Helpers
	 ********/
	String toString( float number, uint8_t digits=2){	// Largely inspired by Print::printFloat()
		String res;

		if (isnan(number)) return "nan";
		if (isinf(number)) return "inf";

		bool negative = false;
		if(number < 0.0){
			number = - number;
			negative = true;
		}

			// Round correctly so that print(1.999, 2) prints as "2.00"
		float rounding = 0.5;
		for (uint8_t i=0; i<digits; ++i)
			rounding /= 10.0;
		number += rounding;

		unsigned long int_part = (unsigned long)number;
		float remainder = number - (double)int_part;

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

			/******
		 * MQTT publishing
		 ******/

	void publish( const char *topic, const char *msg );

	void publish( String &topic, String &msg ){
		this->publish( topic.c_str(), msg.c_str() );
	}

	void publish( String &topic, const char *msg ){
		this->publish( topic.c_str(), msg );
	}


};

#endif
