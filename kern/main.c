#include <stdio.h>
#include <string.h>
#include <taihen.h>
#include <vitasdkkern.h>
#include "f00d.h"
#include "f00dbridge.h"

// last captured GC
static char LAST_CAPTURED_CMD20_INPUT[0x116];
static uint32_t LAST_CAPTURED_CMD20_KEYID = 0;
static uint8_t HAS_CAPTURED_CMD20 = 0;

// hooks for capturing GC comms
static int ksceSblSmCommCallFuncHook = -1;
static tai_hook_ref_t ksceSblSmCommCallFuncHookRef;

// bypass blackfin mitigation
static int sceKernelGetSystemTimeWideHook = -1;
static tai_hook_ref_t sceKernelGetSystemTimeWideHookRef;


void DecryptSecondaryKey0(void* data, uint32_t key_id, void* packet9, void* out) {
	char k_data[0x20];
	char k_packet9[0x30];
	char k_out[0x10];
	ksceKernelMemcpyUserToKernel(k_data, (uintptr_t)data, sizeof(k_data));
	ksceKernelMemcpyUserToKernel(k_packet9, (uintptr_t)packet9, sizeof(k_data));

	decrypt_secondary_key0(k_data, key_id, k_packet9, k_out);

	ksceKernelMemcpyKernelToUser(out, (uintptr_t)k_out, sizeof(k_out));
}

int GetLastCmd20Input(char* cmd20_input) {
	ksceKernelMemcpyKernelToUser(cmd20_input, (uintptr_t)LAST_CAPTURED_CMD20_INPUT, sizeof(LAST_CAPTURED_CMD20_INPUT));
	
	// remove the flag saying we have captured one so another GC can have data captured.
	memset(LAST_CAPTURED_CMD20_INPUT, 0x00, sizeof(LAST_CAPTURED_CMD20_INPUT));
	HAS_CAPTURED_CMD20 = 0;
	
	return 0;
}

int GetLastCmd20KeyId() {
	return LAST_CAPTURED_CMD20_KEYID;
}

int HasCmd20Captured() {
	return HAS_CAPTURED_CMD20;
}

uint64_t return_0() {
	ksceDebugPrintf("ret 0\n");
	return 0;
}

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

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *args)
{
	
	memset(LAST_CAPTURED_CMD20_INPUT, 0x00, sizeof(LAST_CAPTURED_CMD20_INPUT));
	
	// undo cobra blackfin patch
	sceKernelGetSystemTimeWideHook = taiHookFunctionImportForKernel(KERNEL_PID,
		&sceKernelGetSystemTimeWideHookRef, 
		"SceSblGcAuthMgr",
		0xE2C40624, // SceThreadmgrForDriver
		0xF4EE4FA9, // sceKernelGetSystemTimeWide
		return_0);
	
	
	ksceDebugPrintf("sceKernelGetSystemTimeWideHook 0x%04X\n", sceKernelGetSystemTimeWideHook);
	ksceDebugPrintf("sceKernelGetSystemTimeWideHookRef 0x%04X\n", sceKernelGetSystemTimeWideHookRef);
	
	// capture sm communications
	ksceSblSmCommCallFuncHook = taiHookFunctionImportForKernel(KERNEL_PID,
		&ksceSblSmCommCallFuncHookRef, 
		"SceSblGcAuthMgr", 
		0xCD3C89B6, // SceSblSmCommForKernel
		0xDB9FC204, // ksceSblSmCommCallFunc
		ksceSblSmCommCallFunc_patch);
	
	ksceDebugPrintf("ksceSblSmCommCallFuncHook 0x%04X\n", ksceSblSmCommCallFuncHook);
	ksceDebugPrintf("ksceSblSmCommCallFuncHookRef 0x%04X\n", ksceSblSmCommCallFuncHookRef);
	
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
	if (sceKernelGetSystemTimeWideHook >= 0) taiHookReleaseForKernel(sceKernelGetSystemTimeWideHook, sceKernelGetSystemTimeWideHookRef);
	if (ksceSblSmCommCallFuncHook >= 0)		 taiHookReleaseForKernel(ksceSblSmCommCallFuncHook, 	 ksceSblSmCommCallFuncHookRef);


	return SCE_KERNEL_STOP_SUCCESS;
}
