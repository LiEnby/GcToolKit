#include <vitasdk.h>
#define ENABLE_LOGGING 1

#ifdef ENABLE_LOGGING
#define PRINT_STR(...) sceClibPrintf(__VA_ARGS__)
#define PRINT_BUFFER(buffer) for(int i = 0; i < sizeof(buffer); i++) { \
									PRINT_STR("%02X ", (unsigned char)(buffer[i]));	\
							 } \
							 PRINT_STR("\n")
#else
#define PRINT_STR(...) /**/
#define PRINT_BUFFER(buffer) /**/
#endif

#define TO_HEX(in, insz, out, outsz) \
{ \
	unsigned char * pin = in; \
	const char * hex = "0123456789ABCDEF"; \
	char * pout = out; \
	for(; pin < in+insz; pout +=2, pin++){ \
		pout[0] = hex[(*pin>>4) & 0xF]; \
		pout[1] = hex[ *pin     & 0xF]; \
		if (pout + 2 - out > outsz){ \
			break; \
		} \
	} \
	pout[-1] = 0; \
}