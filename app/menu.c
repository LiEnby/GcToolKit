#include "menu.h"
#include "draw.h"
#include "ctrl.h"
#include "crypto.h"
#include "device.h"
#include "gameinfo.h"
#include "io.h"
#include "net.h"
#include "f00dbridge.h"
#include "kernel.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vitasdk.h>
#include <stdint.h>

static vita2d_texture* insertgc_tex;
static uint8_t options[100];

#define DEFOPT(y) int option = 0;\
				  int opt_y = y; \
				  int increment_y = 20; \
				  int total = 0; \
				  memset(options, 0x00, sizeof(options))
#define ADDOPT(cond,x)if(cond) { \
					draw_option(opt_y, x, option == *selected); \
					opt_y += increment_y; \
					total++; \
					options[option] = 1; \
					sceClibPrintf("options[%i] = 1\n", option); \
				   } else { \
					options[option] = 0; \
					sceClibPrintf("options[%i] = 0\n", option); \
				   } \
				   option++ 

#define RETURNOPT() return option
#define CALC_FIRST_OPTION() for(first_option = 0; (options[first_option] != 1 && first_option < sizeof(options)); first_option++)
#define CALC_LAST_OPTION() for(last_option = sizeof(options); (options[last_option] != 1 && last_option > 0); last_option--)
#define PROCESS_MENU(func, ...) \
					  int selected = 0; \
					  memset(options, 0x00, sizeof(options));\
					  func(&selected, __VA_ARGS__); \
					  int first_option = 0;\
					  int last_option = 0;\
					  CALC_FIRST_OPTION(); \
					  CALC_LAST_OPTION(); \
					  selected = first_option;\
					  \
					while (1) { \
					  int total_options = func(&selected, __VA_ARGS__); \
					  CALC_FIRST_OPTION(); \
					  CALC_LAST_OPTION(); \
					  int ctrl = get_key(); \
					  \
					  switch(ctrl) { \
						 case SCE_CTRL_UP: \
							 do{ \
								selected--; \
							 } while(selected > 0 && options[selected] == 0); \
							 break; \
						 case SCE_CTRL_DOWN: \
							 do{ \
								selected++; \
							 } while(selected < sizeof(options) && options[selected] == 0); \
							 break; \
						 case SCE_CTRL_CROSS: \
							 return selected; \
					  } \
					  sceClibPrintf("selected: %x\n", selected); \
					  if(selected > last_option) selected = first_option; \
					  if(selected < first_option) selected = last_option; \
					  sceClibPrintf("selected after adjustment: %x\n", selected); \
				  } \
				  return selected

void init_menus() {
	insertgc_tex = load_texture("app0:/res/insertgc.png");

}

void term_menus() {
	free_texture(insertgc_tex);	
}


int draw_gc_options(int* selected, char* title, uint8_t has_grw0, uint8_t has_mediaid) {
	start_draw();
	draw_background();

	char w_title[128];
	snprintf(w_title, sizeof(w_title), "What to do with %.25s ...", title);
	draw_title(w_title);
	
	DEFOPT(200);

	ADDOPT(1, "Backup Entire Game Cart (.VCI)");
	ADDOPT(1, "Backup Game Cart Keys (.BIN)");

	ADDOPT(has_mediaid, "Backup MediaId Section (.MEDIAID)");
	ADDOPT(0 && has_mediaid, "Restore MediaId Section (.MEDIAID)");		
	ADDOPT(0 && has_mediaid, "Format MediaId Section");

	ADDOPT(has_grw0, "Backup Writable Section (.IMG)");
	ADDOPT(0 && has_grw0, "Restore Writable Section (.IMG)");
	ADDOPT(0 && has_grw0, "Format Writable Section");
	
	end_draw();
	
	RETURNOPT();
}

void draw_insert_gc_menu() {
	start_draw();
	draw_background();
	
	draw_title("Waiting for CMD56 authentication ...");
	draw_texture_center(140, insertgc_tex);
	draw_text_center(360, "Insert a VITA game cart!");
	draw_text_center(380, "(If there already one eject it and put it back in)");
	
	end_draw();
}

void do_gc_insert_prompt() {
	draw_insert_gc_menu();
	
//	int res = StartGcAuthentication(); // re-do authentication.
//	sceClibPrintf("StartGcAuthentication = %x\n", res);
	
	wait_for_gc_auth();	
}

int draw_select_output_location(int* selected, char* output_file, uint8_t have_ux0, uint8_t have_xmc, uint8_t have_usb, uint8_t have_host0, uint8_t save_network) {
	
	start_draw();
	draw_background();
	
	draw_title("Select output location ...");

	char output_txt[128];
	snprintf(output_txt, sizeof(output_txt), "\"%.45s\"", output_file);
	draw_text_center(200, output_txt);
	
	DEFOPT(240);
	
	ADDOPT(have_ux0, "Save to \"ux0\"");
	ADDOPT(have_xmc, "Save to Sony Memory Card");
	ADDOPT(have_usb, "Save to USB Drive");
	ADDOPT(have_host0, "Save to Devkit Host0");	
	ADDOPT(save_network, "Save to Network");
	ADDOPT(1, "Change file name");
	ADDOPT(1, "Refresh Devices");
	
	end_draw();
	
	RETURNOPT();
}


int draw_network_settings(int* selected, char* ip_address, unsigned short port) {
	
	start_draw();
	draw_background();
	
	draw_title("Enter Network Address ...");

	draw_text_center(200, "Run the \"gc_backup_network\" program");
	draw_text_center(220, "it can be found in the readme for GC Backup.");
	draw_text_center(250, "and enter the IP of the device its running on.");

	char output_txt[128];
	snprintf(output_txt, sizeof(output_txt), "Current Setting: %.15s on port %u", ip_address, port);
	draw_text_center(300, output_txt);
	
	DEFOPT(340);
	
	ADDOPT(1, "Change IP Address");
	ADDOPT(1, "Change Port");
	ADDOPT(1, "Start Network Save");
	
	end_draw();
	
	RETURNOPT();
}


void draw_dump_progress(char* device, char* output_filename, uint64_t progress, uint64_t total) {
	
	start_draw();
	draw_background();
	
	char output_txt[128];
	snprintf(output_txt, sizeof(output_txt), "Backing up %s ...", device);
	draw_title(output_txt);

	snprintf(output_txt, sizeof(output_txt), "Writing \"%.30s\" ...", output_filename);
	draw_text_center(200, output_txt);
	
	draw_progress_bar(210, progress, total);
	snprintf(output_txt, sizeof(output_txt), "%llu / %llu (%i%%)", progress, total, (int)( (((float)progress) / ((float)total)) * 100.0));
	draw_text_center(260, output_txt);
	
	end_draw();
}

void draw_ime() {
	start_draw();
	draw_background();
	
	draw_title("Starting IME Dialog ...");

	draw_text_center(200, "IME Dialog is opening...");
	
	end_draw();
}

void draw_confirmation_message(char* title, char* msg) {
	start_draw();
	draw_background();
	
	draw_title(title);

	draw_text_center(200, msg);

	draw_text_center(250, "Press any button to continue ...");
	
	end_draw();
}

int do_network_options(char* ip_address, unsigned short port) {	
	PROCESS_MENU(draw_network_settings, ip_address, port);
}

int do_gc_options() {
	char title_id[64];
	char title[64];
	read_gameinfo(title_id, title);
	
	uint8_t have_grw0 = has_grw0();
	remove_illegal_chars(title);
	
	PROCESS_MENU(draw_gc_options, title, have_grw0, 1);
}

void do_ime() {
	for(int i = 0; i < 0x5; i++)
		draw_ime();
}

void do_confirm_message(char* title, char* msg) {
	draw_confirmation_message(title, msg);
	get_key();
}

int do_device_dump(char* block_device, char* output_file, uint8_t vci, char* ip_address, unsigned short port) {
	
	GcKeys keys;
	if(vci){
		int res = extract_gc_keys(&keys);
		if(res < 0) return res;
	}
	
	umount_gro0();
	umount_grw0();
	
	disable_power_off();

	mount_devices();
	
	lock_shell();
	int res = -1;
	
	if(ip_address == NULL)
		res = dump_device(block_device, output_file, vci ? &keys : NULL, draw_dump_progress);
	else
		res = dump_device_network(ip_address, port, block_device, output_file, vci ? &keys : NULL, draw_dump_progress);
	
	unlock_shell();

	enable_power_off();

	mount_gro0();
	mount_grw0();
	
	umount_devices();
	return res;
}


int do_select_output_location(char* format, char* output, uint64_t device_size) {
	
	sceClibPrintf("mount_devices\n");
	mount_devices();
	
	uint8_t save_network = is_connected();
	sceClibPrintf("save_network = %x\n", save_network);
	
	uint64_t xmc_size = get_free_space("xmc0:");
	uint64_t uma_size = get_free_space("uma0:");
	uint64_t ux_size  = get_free_space("ux0:");
	
	uint8_t ux_exist = file_exist("ux0:");
	uint8_t xmc_exist = file_exist("xmc0:");
	uint8_t uma_exist = file_exist("uma0:");
	uint8_t host_exist = file_exist("host0:");
	
	
	sceClibPrintf("device_size %llx\n", device_size);
	sceClibPrintf("xmc_size %llx\n", xmc_size);
	sceClibPrintf("uma_size %llx\n", uma_size);
	sceClibPrintf("ux_size %llx\n", ux_size);
	
	PROCESS_MENU(draw_select_output_location, output, 
				(ux_exist  && ux_size >= device_size), 
				(xmc_exist && xmc_size >= device_size ), 
				(uma_exist && uma_size >= device_size), 
				host_exist, 
				save_network);
}