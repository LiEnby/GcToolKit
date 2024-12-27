#include <vitasdkkern.h>
#include <stdint.h>
#include <stdio.h>
#include "gc.h"

int GetCardId(int deviceIndex, void* cardId) {
	sd_context_part_mmc* k_deviceInfo = ksceSdifGetSdContextPartValidateMmc(deviceIndex);	
	if(k_deviceInfo == NULL) return -1;

	char k_cardId[sizeof(k_deviceInfo->ctxb.CID)];
	memset(k_cardId, 0xFF, sizeof(k_cardId));

	memcpy(k_cardId, k_deviceInfo->ctxb.CID, sizeof(k_deviceInfo->ctxb.CID));	
	ksceKernelMemcpyKernelToUser(cardId, (const void*)k_cardId, sizeof(k_cardId));	

	return 0;
}

int GetCardCsd(int deviceIndex, void* cardCsd) {
	sd_context_part_mmc* k_deviceInfo = ksceSdifGetSdContextPartValidateMmc(deviceIndex);	
	if(k_deviceInfo == NULL) return -1;
	
	char k_cardCsd[sizeof(k_deviceInfo->ctxb.CSD)];
	memset(k_cardCsd, 0xFF, sizeof(k_cardCsd));
	
	memcpy(k_cardCsd, k_deviceInfo->ctxb.CSD, sizeof(k_deviceInfo->ctxb.CSD));	
	ksceKernelMemcpyKernelToUser(cardCsd, (const void*)k_cardCsd, sizeof(k_cardCsd));	

	return 0;
}