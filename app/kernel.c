#include <vitasdk.h>
#include <taihen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vitasdk.h>
#include "err.h"
#include "log.h"

static uint8_t disable_power = 0;

// the kernel module "GcKernKit" will be attempted to start from the following locations:
// (attempted in the order their listed.)
static const char* load_locations[] = { 
	// memory card
	"ux0:/patch",
	"ux0:/app",
	
	// game cartridge
	"grw0:/patch",
	"gro0:/app",

	// devkit
	"host0:/patch",
	"host0:/app",
	"sd0:/patch"
	"sd0:/app",
	
	// homebrew
	"ur0:/patch",
	"ur0:/app",
	"xmc0:/patch",
	"xmc0:/app",
	"imc0:/patch",			
	"imc0:/app",
	"uma0:/patch",
	"uma0:/app",
	
	// system
	"pd0:/app",
	"vs0:/app",
	
	NULL
};
		
int kernel_started() {
	char buffer[0x8];
	memset(buffer, 0x00, sizeof(buffer));
	
	SceUID uid = _vshKernelSearchModuleByName("GcKernKit", buffer);
	PRINT_STR("_vshKernelSearchModuleByName = %x\n", uid);
	
	return (uid > 0);
		
}

int try_load(const char* install_path) {
	char kplugin_path[0x1028];
	char titleid[12];
	
	memset(titleid, 0x00, sizeof(titleid));
	memset(kplugin_path, 0x00, sizeof(kplugin_path));
	
	sceAppMgrAppParamGetString(0, 12, titleid , 256);

	snprintf(kplugin_path, sizeof(kplugin_path)-1, "%s/%s/kplugin.skprx", install_path, titleid);
	SceUID uid = taiLoadStartKernelModule(kplugin_path, 0, NULL, 0);
	PRINT_STR("%s(%s) = %x\n", __FUNCTION__, kplugin_path, uid);
	
	return uid > 0;
}

void load_kernel_modules() {
	
	if(kernel_started() == 0) {
		
		// try load GcKernKit from all load locations.
		for(int i = 0; load_locations[i] != NULL; i++) {
			if(try_load(load_locations[i])) break;
		}
		
		// restart this application
		sceAppMgrLoadExec("app0:/eboot.bin", NULL, NULL);
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
	sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU |
					SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU |
					SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION |
					SCE_SHELL_UTIL_LOCK_TYPE_MUSIC_PLAYER |
					SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
}

void unlock_shell() {
	 sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU |
					SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU |
					SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION |
					SCE_SHELL_UTIL_LOCK_TYPE_MUSIC_PLAYER |
					SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
}

// disable memory card remove/insert prompt everywhere;
void init_shell() { 
	sceShellUtilInitEvents(0);
	sceShellUtilLock(
		SCE_SHELL_UTIL_LOCK_TYPE_MC_INSERTED |
		SCE_SHELL_UTIL_LOCK_TYPE_MC_REMOVED
	);
}

void term_shell() {
	sceShellUtilUnlock(
		SCE_SHELL_UTIL_LOCK_TYPE_MC_INSERTED |
		SCE_SHELL_UTIL_LOCK_TYPE_MC_REMOVED
	);
}