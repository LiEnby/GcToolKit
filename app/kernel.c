#include <vitasdk.h>
#include <taihen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vitasdk.h>

int kernel_started() {
	char buffer[0x8];
	memset(buffer, 0x00, sizeof(buffer));
	
	SceUID uid = _vshKernelSearchModuleByName("f00dbridge", buffer);
	sceClibPrintf("_vshKernelSearchModuleByName = %x\n", uid);
	
	
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

		// load psp2spl
		snprintf(kplugin_path, sizeof(kplugin_path), "ux0:app/%s/psp2spl.skprx", titleid);
		SceUID uid = taiLoadStartKernelModule(kplugin_path, 0, NULL, 0);
		sceClibPrintf("start %s = %x\n", kplugin_path, uid);
		
		// load f00dbridge
		snprintf(kplugin_path, sizeof(kplugin_path), "ux0:app/%s/kplugin.skprx", titleid);
		uid = taiLoadStartKernelModule(kplugin_path, 0, NULL, 0);
		sceClibPrintf("start %s = %x\n", kplugin_path, uid);
		if(uid < 0) return;
		
		// restart this application
		strncpy(kplugin_path, "app0:/eboot.bin", sizeof(kplugin_path));
		sceAppMgrLoadExec(kplugin_path, NULL, NULL);
	}
	
}