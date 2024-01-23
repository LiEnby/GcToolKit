#include <stdint.h>
#include <stdio.h>
#include <taihen.h>
#include <vitasdkkern.h>
#include "cmd56.h"

// static variables
static uint8_t LAST_CAPTURED_CMD20_INPUT[0x116];
static uint32_t LAST_CAPTURED_CMD20_KEYID = 0;
static uint8_t HAS_CAPTURED_CMD20 = 0;

// hooks for capturing GC comms
static int ksceSblSmCommCallFuncHook = -1;
static tai_hook_ref_t ksceSblSmCommCallFuncHookRef;

int ksceSblSmCommCallFunc_patch(SceSblSmCommId id, SceUInt32 service_id, SceUInt32 *service_result, SceSblSmCommGcData *data, SceSize size) {
	ksceDebugPrintf("cmd = %x\n", data->command);
	
	if(data->command == 0x20) {
		ksceDebugPrintf("copy auth input, data->data %x\n", data->data);
		memcpy(LAST_CAPTURED_CMD20_INPUT, data->data, sizeof(LAST_CAPTURED_CMD20_INPUT));
		
		ksceDebugPrintf("keyid %x\n", data->key_id);
		LAST_CAPTURED_CMD20_KEYID = data->key_id;
		HAS_CAPTURED_CMD20 = 1;
	}
	
	return TAI_CONTINUE(int, ksceSblSmCommCallFuncHookRef, id, service_id, service_result, data, size);
}

void cmd56_patch() {
	memset(LAST_CAPTURED_CMD20_INPUT, 0x00, sizeof(LAST_CAPTURED_CMD20_INPUT));
	
	// capture sm communications
	ksceSblSmCommCallFuncHook = taiHookFunctionImportForKernel(KERNEL_PID,
		&ksceSblSmCommCallFuncHookRef, 
		"SceSblGcAuthMgr", 
		0xCD3C89B6, // SceSblSmCommForKernel
		0xDB9FC204, // ksceSblSmCommCallFunc
		ksceSblSmCommCallFunc_patch);
	
	ksceDebugPrintf("ksceSblSmCommCallFuncHook 0x%04X\n", ksceSblSmCommCallFuncHook);
	ksceDebugPrintf("ksceSblSmCommCallFuncHookRef 0x%04X\n", ksceSblSmCommCallFuncHookRef);
}

void cmd56_unpatch() {
	if (ksceSblSmCommCallFuncHook >= 0)		 taiHookReleaseForKernel(ksceSblSmCommCallFuncHook, 	 ksceSblSmCommCallFuncHookRef);
}

// user syscalls

int ResetCmd20Input() {
	// remove the flag saying we have captured one so another GC can have data captured.
	memset(LAST_CAPTURED_CMD20_INPUT, 0x00, sizeof(LAST_CAPTURED_CMD20_INPUT));
	HAS_CAPTURED_CMD20 = 0;
	return 0;
}

int GetLastCmd20Input(char* cmd20_input) {
	ksceKernelMemcpyKernelToUser(cmd20_input, (const void*)LAST_CAPTURED_CMD20_INPUT, sizeof(LAST_CAPTURED_CMD20_INPUT));
		
	return 0;
}

int GetLastCmd20KeyId() {
	return LAST_CAPTURED_CMD20_KEYID;
}

int HasCmd20Captured() {
	return HAS_CAPTURED_CMD20;
}

