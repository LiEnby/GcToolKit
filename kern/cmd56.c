#include <stdint.h>
#include <stdio.h>
#include <taihen.h>
#include <vitasdkkern.h>
#include "cmd56.h"
#include "log.h"
#include "mod.h"

// from taihen module.h
int module_get_offset(SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr);

// static variables
static uint8_t LAST_CAPTURED_CMD20_INPUT[0x116];
static uint32_t LAST_CAPTURED_CMD20_KEYID = 0;
static uint8_t HAS_CAPTURED_CMD20 = 0;

// prototype keyid check patch
static int proto_keyid_check_inject = -1;

// hooks for capturing GC comms
static int ksceSblSmCommCallFuncHook = -1;
static tai_hook_ref_t ksceSblSmCommCallFuncHookRef;

int (* gc_insert_interupt)(uint32_t param_1, int param_2, int param_3, char* param_4); 

int ksceSblSmCommCallFunc_patch(SceSblSmCommId id, SceUInt32 service_id, SceUInt32 *service_result, SceSblSmCommGcData *data, SceSize size) {
	if(data->command == 0x20) {
		PRINT_STR("copy auth input, data->data %x\n", data->data);
		memcpy(LAST_CAPTURED_CMD20_INPUT, data->data, sizeof(LAST_CAPTURED_CMD20_INPUT));
		
		PRINT_STR("keyid %x\n", data->key_id);
		LAST_CAPTURED_CMD20_KEYID = data->key_id;
		HAS_CAPTURED_CMD20 = 1;
	}
	
	return TAI_CONTINUE(int, ksceSblSmCommCallFuncHookRef, id, service_id, service_result, data, size);
}

int cmd56_patch() {
	memset(LAST_CAPTURED_CMD20_INPUT, 0x00, sizeof(LAST_CAPTURED_CMD20_INPUT));
	
	tai_module_info_t gc_authmgr_info;
	gc_authmgr_info.size = sizeof(tai_module_info_t);
	int gcauthmgr_get_info = taiGetModuleInfoForKernel(KERNEL_PID, "SceSblGcAuthMgr", &gc_authmgr_info);
	int gcauthmgr_version = check_module_version("os0:/kd/gcauthmgr.skprx");
	
	PRINT_STR("get module SceSblGcAuthMgr 0x%04X (version: 0x%x)\n", gcauthmgr_get_info, gcauthmgr_version);
	
	if(gcauthmgr_get_info >= 0) {
		// prototype game carts use key ids != 1,
		// 3.60 firmware checks if ((key_id & 0xffff7fff) == 1)
		// it would be nice if we could dump prototype gamecarts tho ..
	
		uint16_t nop_instruction = 0xBF00;
		
		if(gcauthmgr_version >= 0x363) {
			proto_keyid_check_inject = taiInjectDataForKernel(KERNEL_PID, gc_authmgr_info.modid, 0, 0x95F8, &nop_instruction, sizeof(uint16_t));
		}
		else {
			proto_keyid_check_inject = taiInjectDataForKernel(KERNEL_PID, gc_authmgr_info.modid, 0, 0x9376, &nop_instruction, sizeof(uint16_t));
		}

		
		PRINT_STR("proto_keyid_check_inject 0x%04X\n", proto_keyid_check_inject);
		
		// capture sm communications
		ksceSblSmCommCallFuncHook = taiHookFunctionImportForKernel(KERNEL_PID,
			&ksceSblSmCommCallFuncHookRef, 
			"SceSblGcAuthMgr", 
			0xCD3C89B6, // SceSblSmCommForKernel
			0xDB9FC204, // ksceSblSmCommCallFunc
			ksceSblSmCommCallFunc_patch);
		
		PRINT_STR("ksceSblSmCommCallFuncHook 0x%04X\n", ksceSblSmCommCallFuncHook);
		PRINT_STR("ksceSblSmCommCallFuncHookRef 0x%04X\n", ksceSblSmCommCallFuncHookRef);
	}
	
	tai_module_info_t sdstor_info;
	sdstor_info.size = sizeof(tai_module_info_t);
	int sdstor_get_info = taiGetModuleInfoForKernel(KERNEL_PID, "SceSdstor", &sdstor_info);
	PRINT_STR("get module SceSdstor 0x%04X\n", gcauthmgr_get_info);
	
	if(sdstor_get_info >= 0){
		int res = module_get_offset(KERNEL_PID, sdstor_info.modid, 0, 0x3BE0 | 1, (uintptr_t*)&gc_insert_interupt);
		PRINT_STR("module_get_offset 0x%04X\n", res);
		return res;
	}
	return sdstor_get_info;	
}

int cmd56_unpatch() {
	if (ksceSblSmCommCallFuncHook >= 0)		 taiHookReleaseForKernel(ksceSblSmCommCallFuncHook, ksceSblSmCommCallFuncHookRef);
	if (proto_keyid_check_inject >= 0)		 taiInjectReleaseForKernel(proto_keyid_check_inject);
	return 0;
}

// user syscalls

int kResetCmd20Input() {
	// remove the flag saying we have captured one so another GC can have data captured.
	memset(LAST_CAPTURED_CMD20_INPUT, 0x00, sizeof(LAST_CAPTURED_CMD20_INPUT));
	HAS_CAPTURED_CMD20 = 0;
	return 0;
}

int kGetLastCmd20Input(void* cmd20_input) {
	ksceKernelMemcpyKernelToUser(cmd20_input, (const void*)LAST_CAPTURED_CMD20_INPUT, sizeof(LAST_CAPTURED_CMD20_INPUT));
	return 0;
}

int kGetLastCmd20KeyId() {
	return LAST_CAPTURED_CMD20_KEYID;
}

int kHasCmd20Captured() {
	return HAS_CAPTURED_CMD20;
}
// spent ages trying to find a way to run gc authentication again when already inserted
// then i had a thought ???
// have you tried turning it off and on again ????
int kResetGc() {
	char param_4[0x10];
	memset(param_4, 0x00, sizeof(param_4));
	
	PRINT_STR("Resetting GC ...\n");
	
	// trigger gc remove interupt
	int res = gc_insert_interupt(0, 0x200, 0, param_4);
	PRINT_STR("gc_insert_interupt(0, 0x200, 0, param_4) 0x%04X\n", res);
	if(res < 0) return res;
	
	// power down gc slot
	res = ksceSysconCtrlSdPower(0);
	PRINT_STR("ksceSysconCtrlSdPower(0) 0x%04X\n", res);
	if(res < 0) return res;

	// power up gc slot
	res = ksceSysconCtrlSdPower(1);
	PRINT_STR("ksceSysconCtrlSdPower(1) 0x%04X\n", res);
	if(res < 0) return res;
	
	// trigger gc insert interupt
	res = gc_insert_interupt(1, 0x100000, 0, param_4);
	PRINT_STR("gc_insert_interupt(1, 0x100000, 0, param_4) 0x%04X\n", res);
	if(res < 0) return res;
	
	PRINT_STR("param_4[5] = 0x%04X\n", param_4[5]);
	
	return 0;
}

int kClearCartSecret() {
	return ksceSblGcAuthMgrDrmBBClearCartSecret();
}

int kGetCartSecret(uint8_t* keys) {
	uint8_t k_keys[0x20];
	memset(k_keys, 0x00, sizeof(k_keys));
	
	int res = ksceSblGcAuthMgrDrmBBGetCartSecret(k_keys);
	if(keys != NULL) ksceKernelMemcpyKernelToUser(keys, (const void*)k_keys, sizeof(k_keys));
		
	return res;
}
