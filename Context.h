/* Context that is persisted b/w runs 
 *
 * 30/10/2017 First version
 */

class Context {
public:
	enum Network { SAFE, MAISON, DOMOTIQUE  };

private:
	struct {
		uint32_t crc32;
		struct {
			enum Network which_net;
			bool first;
		} dt;
	} rtcData;

	uint32_t crc32(){ /* from https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/RTCUserMemory/RTCUserMemory.ino */
		uint32_t crc = 0xffffffff;

		const uint8_t *data = (const uint8_t *)&(rtcData.dt);
		for(int i=0; i<sizeof(rtcData.dt); i++){
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

		if(ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData))){
			if( rtcData.crc32 == crc32() ){
#ifdef DEV_ONLY
		Serial.println("Context CRC ok");
#endif
				return;
			}
		}

		Serial.println("Invalid context\nReseting to default");
		rtcData.dt.first = true;
		rtcData.dt.which_net = /* SAFE */ MAISON;	/* Trying without hop */
	}

	void save(){
		rtcData.crc32 = crc32();
		ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData));
#ifdef DEV_ONLY
		Serial.println("Context saved");
		Serial.print("RTC data size : ");
		Serial.println(sizeof(rtcData));
#endif
	}

	bool isfirstrun(){ return rtcData.dt.first; }
	void hasrun() { rtcData.dt.first = false; }

	void setNetwork( enum Network n ){ rtcData.dt.which_net = n; }
	enum Network getNetwork(){ return rtcData.dt.which_net; }
};
