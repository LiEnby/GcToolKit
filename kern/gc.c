#include <vitasdkkern.h>
#include <stdint.h>
#include <stdio.h>
#include "gc.h"
#include "log.h"

int GetCardId(int deviceIndex, void* cardId) {
	sd_context_part_mmc* k_deviceInfo = ksceSdifGetSdContextPartValidateMmc(deviceIndex);	
	if(k_deviceInfo == NULL) return -1;

	char k_cardId[sizeof(k_deviceInfo->ctxb.CID)];
	memset(k_cardId, 0x00, sizeof(k_cardId));
	
	memcpy(k_cardId, k_deviceInfo->ctxb.CID, sizeof(k_deviceInfo->ctxb.CID));
	// copy bytes in reverse order ?
	// int i2 = 0;
	// for(int i = sizeof(k_deviceInfo->ctxb.CID); i > 0; i--) { 
	// 	k_cardId[i2++] = k_deviceInfo->ctxb.CID[i]; 
	// }
	
	PRINT_STR("cardId: ");
	PRINT_BUFFER(k_cardId);
	
	ksceKernelMemcpyKernelToUser(cardId, (const void*)k_cardId, sizeof(k_cardId));	

	return 0;
}

int GetCardCsd(int deviceIndex, void* cardCsd) {
	sd_context_part_mmc* k_deviceInfo = ksceSdifGetSdContextPartValidateMmc(deviceIndex);	
	if(k_deviceInfo == NULL) return -1;
	
	char k_cardCsd[sizeof(k_deviceInfo->ctxb.CSD)];
	memset(k_cardCsd, 0x00, sizeof(k_cardCsd));
	
	memcpy(k_cardCsd, k_deviceInfo->ctxb.CSD, sizeof(k_deviceInfo->ctxb.CSD));
	// copy bytes in reverse order ?
	//	int i2 = 0;
	//	for(int i = sizeof(k_deviceInfo->ctxb.CSD); i > 0; i--) { 
	//		k_cardCsd[i2++] = k_deviceInfo->ctxb.CSD[i]; 
	//	}
	
	PRINT_STR("cardCsd: ");
	PRINT_BUFFER(k_cardCsd);
	
	ksceKernelMemcpyKernelToUser(cardCsd, (const void*)k_cardCsd, sizeof(k_cardCsd));	

	return 0;
}