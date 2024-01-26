#include <stdint.h>
#include <stdio.h>
#include <taihen.h>
#include <vitasdkkern.h>
#include "cmd56.h"

// static variables
static uint8_t LAST_CAPTURED_CMD20_INPUT[0x116];
static uint32_t LAST_CAPTURED_CMD20_KEYID = 0;
static uint8_t HAS_CAPTURED_CMD20 = 0;

// prototype keyid check patch
static int proto_keyid_check_inject = -1;

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
	
	tai_module_info_t info;
	info.size = sizeof(tai_module_info_t);
	int res = taiGetModuleInfoForKernel(KERNEL_PID, "SceSblGcAuthMgr", &info);
	if(res >= 0) {
		// prototype game carts use key ids != 1,
		// 3.60 firmware checks if ((key_id & 0xffff7fff) == 1)
		// it would be nice if we could dump prototype gamecarts tho ..
	
		uint16_t nop_instruction = 0xBF00;
		proto_keyid_check_inject = taiInjectDataForKernel(KERNEL_PID, info.modid, 0, 0x9376, &nop_instruction, sizeof(uint16_t));
		ksceDebugPrintf("proto_keyid_check_inject 0x%04X\n", proto_keyid_check_inject);
		
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
	
	ksceDebugPrintf("get module info 0x%04X\n", res);
	
}

void cmd56_unpatch() {
	if (ksceSblSmCommCallFuncHook >= 0)		 taiHookReleaseForKernel(ksceSblSmCommCallFuncHook, ksceSblSmCommCallFuncHookRef);
	if (proto_keyid_check_inject >= 0)		 taiInjectReleaseForKernel(proto_keyid_check_inject);

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

int StartGcAuthentication() {
	return ksceSblGcAuthMgrGcAuthCartAuthentication(1);
}

int ClearFinalKeys() {
	return ksceSblGcAuthMgrDrmBBClearCartSecret();
}

int GetFinalKeys(char* keys) {
	char k_keys[0x20];
	memset(k_keys, 0x00, sizeof(k_keys));
	int res = ksceSblGcAuthMgrDrmBBGetCartSecret(k_keys);
	ksceKernelMemcpyKernelToUser(keys, (const void*)k_keys, sizeof(k_keys));
	return res;
}
