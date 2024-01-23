#include <vitasdk.h>
#include <string.h>
#include <stdio.h>

#include "gameinfo.h"
#include "io.h"
#include "sfo.h"

#define DEFAULT_TITLEID "NPXS99999"
#define DEFAULT_TITLE "UNKNOWN"


int read_titleid(char* title_id) {
	int dfd = sceIoDopen("gro0:/app");
	if(dfd >= 0){
		SceIoDirent ent;
		int res = sceIoDread(dfd, &ent);
		if(res >= 0) {
			strncpy(title_id, ent.d_name, 12);
			return 0;
		}
	}
	
	if(dfd >= 0)
		sceIoDclose(dfd);
	return dfd;	
	
}

int read_gameinfo(char* title_id, char* title) {
	wait_for_partition("gro0:");
	
	// get title id from gro0:/app folder
	int res = read_titleid(title_id);
	sceClibPrintf("read_title_id: = %x\n", res);

	if(res >= 0) {
		char param_sfo_path[512];
		snprintf(param_sfo_path, sizeof(param_sfo_path), "gro0:/app/%s/sce_sys/param.sfo", title_id);
		res = read_sfo_key("STITLE", title, param_sfo_path);
		if(res == -9) // not found
			res = read_sfo_key("TITLE", title, param_sfo_path);
		
		sceClibPrintf("read_title: = %x\n", res);
	}
	
	if(res < 0) {
		strncpy(title_id, DEFAULT_TITLEID, 64);
		strncpy(title, DEFAULT_TITLE, 64);
	}
	return res;
}