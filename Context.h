/* Persisting context b/w runs
 *
 * 30/10/2017 First version
 * 20/12/2017 Full redesign
 */
#ifndef CONTEXT_H
#define CONTEXT_H

class Network;

class Context {
	bool RTCvalid;	// Is stored memory valid ?
	uint32_t offset;	// Offset in memory for next data
	class Network *net;

public:
	Context() : RTCvalid(false), net(NULL){
			/* Check if RTC memory contains valuable data */
		uint32_t key;
		if(ESP.rtcUserMemoryRead(0, &key, sizeof(key))){
			if( key == ESP.getFlashChipId() )
				RTCvalid = true;
		}

		offset = sizeof(key);
	}

	void setNetwork( Network *n ){ net = n; }

	bool isValid( void ){ return RTCvalid; }

		/* This method has to be called ONLY after all other stuffs
		 * to be kept are saved
		 */
	void save( void ){
		uint32_t key = ESP.getFlashChipId();
		ESP.rtcUserMemoryWrite(0, &key, sizeof(key));
		RTCvalid = true;
	}

	uint32_t reserveData( uint32_t s ){
		uint32_t start = this->offset;
		this->offset += s;

		return( start );
	}

	void status( void ){
#ifdef DEV_ONLY
#	ifdef SERIAL_ENABLED
		Serial.print("Context : RTC ");
		Serial.println(this->isValid() ? "valid" : "invalid");
		Serial.print("RTC data size : ");
		Serial.println(offset);
#	endif
#endif
	}

	class keepInRTC {
		uint32_t *what;
		uint32_t size;
		uint32_t offset;

	protected:
		keepInRTC( Context &ctx, uint32_t *w, uint32_t s ) : what(w), size(s) {
		/* ->	ctx : context managing the RTC memory
		 * 		w : which data to save
		 * 		s : size of the data to save
		 */
			this->offset = ctx.reserveData( s );

			if(ctx.isValid())	// previous data can be retrieved
				ESP.rtcUserMemoryRead(this->offset, this->what, this->size);
		}

	public:
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
		if(number < 0.0)
			negative = true;

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
