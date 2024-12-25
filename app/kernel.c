#include <vitasdk.h>
#include <taihen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vitasdk.h>
#include "err.h"
#include "log.h"

static uint8_t disable_power = 0;

int kernel_started() {
	char buffer[0x8];
	memset(buffer, 0x00, sizeof(buffer));
	
	SceUID uid = _vshKernelSearchModuleByName("f00dbridge", buffer);
	PRINT_STR("_vshKernelSearchModuleByName = %x\n", uid);
	
	
	if(uid >= 0) // started already
		return 1;
	else
		return 0; // module not yet running
		
}

void load_kernel_modules() {
	char kplugin_path[0x200];
	memset(kplugin_path,0x00,0x200);
	
	if(kernel_started() == 0) {
		// get current title id
		char titleid[12];
		sceAppMgrAppParamGetString(0, 12, titleid , 256);
		
		// load f00dbridge
		snprintf(kplugin_path, sizeof(kplugin_path), "ux0:app/%s/kplugin.skprx", titleid);
		SceUID uid = taiLoadStartKernelModule(kplugin_path, 0, NULL, 0);
		PRINT_STR("start %s = %x\n", kplugin_path, uid);
		if(uid < 0) return;
		
		// restart this application
		strncpy(kplugin_path, "app0:/eboot.bin", sizeof(kplugin_path));
		sceAppMgrLoadExec(kplugin_path, NULL, NULL);
	}
	
}



int power_tick_thread(size_t args, void* argp) {
	disable_power = 1;
	while(disable_power){
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DEFAULT);
		sceKernelDeleteThread(1000 * 1000 * 1);		
	}
	return 0;
}

void enable_power_off() {
	disable_power = 0;
}

int disable_power_off() {
	int ret = 0;
	SceUID thid = sceKernelCreateThread("PowerTickThread", power_tick_thread, 0x10000100, 0x1000, 0, 0, NULL);
	if(thid < 0) ERROR(thid);

	int startresult = sceKernelStartThread(thid, 0, NULL);
	if(startresult < 0) ERROR(startresult);
	
error:
	if(thid < 0)
		sceKernelDeleteThread(thid);
	return ret;
}


void lock_shell() {
	sceShellUtilInitEvents(0);
	sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU |
					SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU |
					SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION |
					SCE_SHELL_UTIL_LOCK_TYPE_MC_INSERTED |
					SCE_SHELL_UTIL_LOCK_TYPE_MC_REMOVED |
					SCE_SHELL_UTIL_LOCK_TYPE_MUSIC_PLAYER |
					SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
}

void unlock_shell() {
	 sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU |
					SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU |
					SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION |
					SCE_SHELL_UTIL_LOCK_TYPE_MC_INSERTED |
					SCE_SHELL_UTIL_LOCK_TYPE_MC_REMOVED |
					SCE_SHELL_UTIL_LOCK_TYPE_MUSIC_PLAYER |
					SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
}
