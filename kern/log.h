#include <vitasdkkern.h>

#ifdef ENABLE_LOGGING
#define PRINT_STR(...) ksceDebugPrintf(__VA_ARGS__)
#define PRINT_BUFFER(buffer) for(int i = 0; i < sizeof(buffer); i++) { \
									PRINT_STR("%02X ", (unsigned char)(buffer[i]));	\
							 } \
							 PRINT_STR("\n")
#else
#define PRINT_STR(...) /**/
#define PRINT_BUFFER(buffer) /**/
#endif

