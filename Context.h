/* Persisting context b/w runs
 *
 * 30/10/2017 First version
 * 20/12/2017 Full redesign
 */
#ifndef CONTEXT_H
#define CONTEXT_H

class Context {
	bool RTCvalid;	// Is stored memory valid ?
	size_t offset;	// Offset in memory for next data

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

	void save( void ){
		uint32_t key = ESP.getFlashChipId();
		ESP.rtcUserMemoryWrite(0, &key, sizeof(key));
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

};

#endif
