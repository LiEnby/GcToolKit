#include <stdint.h>
#include <stdio.h>
#include "vci.h"

typedef struct GcInfo {
	uint8_t Cid[0x10];
	uint8_t Csd[0x10];
	uint8_t ExtCsd[0x200];
	
	uint8_t ExtCsdRev;
	uint8_t Vendor;
	
	char DeviceName[7];
	uint8_t DeviceRev;
	uint32_t DeviceSerial;
	
	
	uint16_t Month;
	uint16_t Year;
	
	uint16_t KeyId;
	GcCmd56Keys KeySet;
	
} GcInfo;

void mmc_datetime_from_byte(uint8_t rev, uint8_t mdt, uint16_t* year, uint16_t* month);
const char* mmc_vendor_id_to_manufacturer(uint8_t vendorId);
const char* keyid_to_keygroup(uint16_t keyId);
void get_gc_info(GcInfo* info);