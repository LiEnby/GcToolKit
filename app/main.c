#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <taihen.h>
#include <vitasdk.h>

#include <vita2d.h>

#include "crypto.h"
#include "gameinfo.h"
#include "kernel.h"
#include "draw.h"
#include "menu.h"
#include "ime.h"
#include "io.h"

void get_output_filename(char* output, char* format, int size_output) {
	char title_id[64];
	char title[64];
	read_gameinfo(title_id, title);
		
	snprintf(output, size_output, "%s [%s].%s", title, title_id, format);
	remove_illegal_chars(output);
}

void handle_menu_set_output(char* fmt, int what) {
	
	char* outdrv = "";
	char output_filename[MAX_PATH*2];
	get_output_filename(output_filename, fmt, MAX_PATH);
	while(1) {
		int selected = do_select_output_location(fmt, output_filename);
		
		switch(selected) {
			case DUMP_LOCATION_UX0:
				outdrv = "ux0:";
				break;
			case DUMP_LOCATION_XMC:
				outdrv = "xmc0:";
				break;
			case DUMP_LOCATION_UMA:
				outdrv = "uma0:";
				break;
			case DUMP_LOCATION_HOST:
				outdrv = "host0:";
				break;
			case DUMP_LOCATION_NET:
				break;
			case CHANGE_FILENAME:
				open_ime("Enter filename", output_filename, MAX_PATH);
				remove_illegal_chars(output_filename);
				continue;
				break;
			case RELOAD_DEVICES:
				continue;
				break;
			default:
				break;
		};
		
		break;
	}
	char out_dumpfile[MAX_PATH*3];
	snprintf(out_dumpfile, sizeof(out_dumpfile), "%s/%s", outdrv, output_filename);
	sceClibPrintf("what = %x, out_dumpfile = %s\n", what, out_dumpfile);
	int res = 0;
	switch(what) {
		case DUMP_WHOLE_GC:
			res = do_gc_full_dump(out_dumpfile);
			break;
		case DUMP_KEYS_ONLY:
			key_dump(out_dumpfile);
			break;
		case DUMP_MEDIAID:
			res = do_device_dump("sdstor0:gcd-lp-act-mediaid", out_dumpfile);
			break;
		case DUMP_GRW0:
			res = do_device_dump("sdstor0:gcd-lp-ign-gamerw", out_dumpfile);
			break;
	}

	char msg[0x1028];	
	if(res < 0) {
		snprintf(msg, sizeof(msg), "There was an error creating the backup, ( res = 0x%08X )", res);
		do_confirm_message("Error!", msg);
	}
	else{
		snprintf(msg, sizeof(msg), "Backup created at: \"%.25s\"", out_dumpfile);
		do_confirm_message("Backup Success!", msg);
	}
}
void handle_menu_select_option() {
	
	char* fmt = "";
	
	while(1) {
		int selected = do_gc_options();
		switch(selected) {
			case DUMP_WHOLE_GC:
				fmt = "vci";
				break;
			case DUMP_KEYS_ONLY:
				fmt = "bin";
				break;
			case DUMP_MEDIAID:
				fmt = "mediaid";
				break;
			case WRITE_MEDIAID:
				break;
			case RESET_MEDIAID:
				break;
			case DUMP_GRW0:
				fmt = "img";
				break;
			case WRITE_GRW0:
				break;
			case RESET_GRW0:
				break;
			default:
				break;
		};

		if(selected == DUMP_WHOLE_GC || selected == DUMP_KEYS_ONLY || selected == DUMP_MEDIAID || selected == DUMP_GRW0)
			handle_menu_set_output(fmt, selected);

		break;
	}
}


int main() {

	load_kernel_modules();
	init_vita2d();
	init_menus();
	
	while(1) {
		do_gc_insert_prompt();
		handle_menu_select_option();
	}
	term_menus();
	term_vita2d();
	return 0;
}
