/* Context that is persisted b/w runs 
 *
 * 30/10/2017 First version
 */
#ifndef CONTEXT_H
#define CONTEXT_H

#include "Network.h"
#include "Porte.h"

	/*****
	 * Notez-bien : All fields will be persisted in RTC memory
	 *****/
class Context : public Network, Porte {
	uint32_t crc;

	uint32_t crc32(){ /* from https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/RTCUserMemory/RTCUserMemory.ino */
		uint32_t crc = 0xffffffff;

		const uint8_t *data = (const uint8_t *)this;
		for(unsigned int i=0; i<sizeof(*this); i++){
			uint8_t c = *data++;
			for (uint32_t i = 0x80; i > 0; i >>= 1) {
				bool bit = crc & 0x80000000;
				if(c & i)
					bit = !bit;
				crc <<= 1;
				if(bit)
					crc ^= 0x04c11db7;
			}
		}
		return crc;
	}

public:
	Context(){
		if(ESP.rtcUserMemoryRead(0, (uint32_t*)this, sizeof(*this))){
			if( this->crc == crc32() ){
#ifdef DEV_ONLY
		Serial.println("Context CRC ok");
#endif
				return;
			}
		}

		Serial.println("Invalid context\nReseting to default");
		Network::init();
		Porte::init();
	}

	void save( void ){
		this->crc = crc32();
		ESP.rtcUserMemoryWrite(0, (uint32_t*)this, sizeof(*this));
#ifdef DEV_ONLY
		Serial.println("Context saved");
		Serial.print("RTC data size : ");
		Serial.println(sizeof(*this));
#endif
	}
};

#endif
