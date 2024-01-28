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
#include "device.h"
#include "net.h"
#include "bgm.h"

void get_output_filename(char* output, char* format, int size_output) {
	char title_id[64];
	char title[64];
	read_gameinfo(title_id, title);
		
	snprintf(output, size_output, "%s [%s].%s", title, title_id, format);
	remove_illegal_chars(output);
}


int handle_dump_device(int what, char* block_device, char* outfile, char* ip_address, unsigned short port) {
	int res = -1;
	if(what != DUMP_KEYS_ONLY)
		res = do_device_dump(block_device, outfile, (what == DUMP_WHOLE_GC) ? 1 : 0, ip_address, port);		
	else if(ip_address == NULL)
		res = key_dump(outfile);
	else
		res = key_dump_network(ip_address, port, outfile);
	
	
	return res;
}

int handle_menu_set_network_options(int what, char* block_device, char* outfile) {
	char ip_address[0x1028];
	unsigned short port = DEFAULT_PORT;
	
	memset(ip_address, 0x00, sizeof(ip_address));
	strncpy(ip_address, DEFAULT_IP, sizeof(ip_address));
	
	int selected = -1;
	while(1) {
		selected = do_network_options(ip_address, port);
		switch(selected) {
			case CHANGE_IP:
				open_ime("Enter IP", ip_address, 15);
				sceClibPrintf("ip address: %s\n", ip_address);
				if(!check_ip_address_valid(ip_address))
					strncpy(ip_address, DEFAULT_IP, sizeof(ip_address));
				
				continue;
				break;
			case CHANGE_PORT:
				open_ime_short("Enter PORT", &port);
				continue;
				break;
			case START_DUMP:
				break;
		}
		
		break;
	};
	
	return handle_dump_device(what, block_device, outfile, ip_address, port);
}


	
void handle_menu_set_output(char* fmt, int what) {
	
	sceClibPrintf("handle_menu_set_output\n");
	// determine block device
	char* block_device = NULL;
	char* output_device = NULL;
	
	switch(what) {
		case DUMP_WHOLE_GC:
			block_device = BLOCK_DEVICE_GC;
			break;
		case DUMP_MEDIAID:
			block_device = BLOCK_DEVICE_MEDIAID;
			break;
		case DUMP_GRW0:
			block_device = BLOCK_DEVICE_GRW0;
			break;
		case DUMP_KEYS_ONLY:
			break;

	}

	sceClibPrintf("block_device: %s\n", block_device);

	// get filename
	char output_filename[MAX_PATH*3];
	get_output_filename(output_filename, fmt, MAX_PATH);
	
	sceClibPrintf("output_filename: %s\n", output_filename);

	// get total size
	uint64_t total_device_size = sizeof(GcKeys);
	if(block_device != NULL)
		total_device_size = device_size(block_device);
	sceClibPrintf("total_device_size %llx\n", total_device_size);
	
	int selected = -1;
	while(1) {
		selected = do_select_output_location(fmt, output_filename, total_device_size);
		
		switch(selected) {
			case DUMP_LOCATION_UX0:
				output_device = "ux0:";
				break;
			case DUMP_LOCATION_XMC:
				output_device = "xmc0:";
				break;
			case DUMP_LOCATION_UMA:
				output_device = "uma0:";
				break;
			case DUMP_LOCATION_HOST:
				output_device = "host0:";
				break;
			case DUMP_LOCATION_NET:
				output_device = "";
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
	};
	sceClibPrintf("output_device %s\n", output_device);

	// get outfile
	char out_dumpfile[MAX_PATH*3];
	snprintf(out_dumpfile, sizeof(out_dumpfile), "%s%s", output_device, output_filename);
	sceClibPrintf("what = %x, out_dumpfile = %s\n", what, out_dumpfile);

	int res = -1;
	if(selected != DUMP_LOCATION_NET) {
		res = handle_dump_device(what, block_device, out_dumpfile, NULL, 0);
	}
	else {
		res = handle_menu_set_network_options(what, block_device, output_filename);
	}
	
	char msg[0x1028];	
	if(res < 0) {
		snprintf(msg, sizeof(msg), "There was an error ( res = 0x%08X )", res);
		do_confirm_message("Error!", msg);
	}
	else{
		snprintf(msg, sizeof(msg), "Backup created at: \"%.20s\" ...", out_dumpfile);
		do_confirm_message("Backup Success!", msg);
	}


}
void handle_menu_select_option() {
	
	char* fmt = "";
	int selected = -1;
	while(1) {
		selected = do_gc_options();
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

		break;
	};
	if(selected == DUMP_WHOLE_GC || selected == DUMP_KEYS_ONLY || selected == DUMP_MEDIAID || selected == DUMP_GRW0)
		handle_menu_set_output(fmt, selected);

}

int main() {

	load_kernel_modules();
	init_vita2d();
	init_menus();
	init_network();
	init_sound();
	
	while(1) {
		do_gc_insert_prompt();
		handle_menu_select_option();
	}
	
	term_sound();
	term_menus();
	term_vita2d();
	term_network();
	return 0;
}
