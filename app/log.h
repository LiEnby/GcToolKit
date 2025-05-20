#include <vitasdk.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifdef ENABLE_LOGGING
#define PRINT_STR(...) sceClibPrintf(__VA_ARGS__)
#define PRINT_BUFFER(buffer) for(int i = 0; i < sizeof(buffer); i++) { \
									PRINT_STR("%02X ", (uint8_t)(buffer[i]));	\
							 } \
							 PRINT_STR("\n")
#else
#define PRINT_STR(...) /**/
#define PRINT_BUFFER(buffer) /**/
#endif

#define TO_HEX(inStr, inLen, outStr) \
do { \
	char octet[0x3]; \
	for(int i = 0; i < inLen; i++) { \
		snprintf(octet, sizeof(octet), "%02X", ((uint8_t*)inStr)[i]); \
		memcpy((unsigned char*)outStr + i*2, octet, 0x2); \
	} \
	outStr[(inLen*2)] = 0; \
} while(0)



