/* Persisting context b/w runs
 *
 * 30/10/2017 First version
 * 20/12/2017 Full redesign
 */
#ifndef CONTEXT_H
#define CONTEXT_H

class Context {
	bool RTCvalid;	// Is stored memory valid ?
	uint32_t offset;	// Offset in memory for next data

public:
	Context() : RTCvalid(false){
			/* Check if RTC memory contains valuable data */
		uint32_t key;
		if(ESP.rtcUserMemoryRead(0, &key, sizeof(key))){
			if( key == ESP.getFlashChipId() )
				RTCvalid = true;
		}

		offset = sizeof(key);
	}

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
	String toString( float f){
		char buff[16];
		snprintf(buff, sizeof(buff), "%f", f);
		return buff;
	}
};

#endif
