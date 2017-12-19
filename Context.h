/* Context that is persisted b/w runs 
 *
 * 30/10/2017 First version
 */
#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdio.h>

#include "Network.h"
#include "Porte.h"
#include "MQTTcon.h"

	/*****
	 * Notez-bien : All fields will be persisted in RTC memory
	 *****/
class Context : public Network, public Porte, public MQTTcon {
	uint32_t crc;
	bool fromRTC;	// Indicate if the contect came from RTC or has been cleared

	uint32_t crc32( void ){ /* from https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/RTCUserMemory/RTCUserMemory.ino */
		uint32_t crc = 0xffffffff, ans_crc = this->crc;
		this->crc = 0;	// Because initial calculation has been made without crc

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

		this->crc = ans_crc;
		return crc;
	}

public:
		/***
		 * Notez-bien : Following methods are called before Serial initialised
		 ***/
	Context() : crc(0) {
		if(ESP.rtcUserMemoryRead(0, (uint32_t*)this, sizeof(*this))){
			if( this->crc == this->crc32() ){
				this->fromRTC = true;
				return;
			}
		}

		this->fromRTC = false;
		this->Network::init();
		this->Porte::init();
		this->MQTTcon::init();

		this->save();	// Save default configuration
	}

	void save( void ){
		this->crc = this->crc32();
		ESP.rtcUserMemoryWrite(0, (uint32_t*)this, sizeof(*this));
	}

	/********
	 * Following must be called after serial initialisation
	 ********/

	void setup( void ){
	/* Hardward setup
	 */
		this->Network::setup();
		this->Porte::setup();
		this->MQTTcon::setup();
	}

	void connect(){
		if( this->Network::connect() != NetworkMode::FAILURE ){
			this->MQTTcon::connect();
			this->save();
		}
	}

	void status( void ){
#ifdef DEV_ONLY
#	ifdef SERIAL_ENABLED
		Serial.print("Context : ");
		Serial.println(this->fromRTC ? "from RTC" : "Cleared");
		Serial.print("RTC data size : ");
		Serial.println(sizeof(*this));
#	endif

		Network::status();
#endif
	}


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
