#include "menu.h"
#include "draw.h"
#include "ctrl.h"
#include "crypto.h"
#include "device.h"
#include "gameinfo.h"
#include "io.h"

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
				  memset(options, 0x00, sizeof(options));
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

#define RETURNOPT() return option;

#define PROCESS_MENU(func, ...) int selected = 0; \
						while (1) { \
						  int total_options = func(&selected, __VA_ARGS__); \
						  int ctrl = get_key(); \
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
						  if(selected > total_options) selected = 0; \
						  if(selected < 0) selected = 0; \
						  sceClibPrintf("selected after adjustment: %x\n", selected); \
					  } \
					  return selected;

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
	wait_for_gc_auth();	
}

int draw_select_output_location(int* selected, char* output_file, uint8_t have_xmc, uint8_t have_usb, uint8_t have_host0, uint8_t save_network) {
	
	start_draw();
	draw_background();
	
	draw_title("Select output location ...");

	char output_txt[128];
	snprintf(output_txt, sizeof(output_txt), "\"%.45s\"", output_file);
	draw_text_center(200, output_txt);
	
	DEFOPT(240);
	
	ADDOPT(1, "Save to \"ux0\"");
	ADDOPT(have_xmc, "Save to Sony Memory Card");
	ADDOPT(have_usb, "Save to USB Drive");
	ADDOPT(have_host0, "Save to Devkit Host0");	
	ADDOPT(save_network, "Save to Network");
	ADDOPT(1, "Change file name");
	ADDOPT(1, "Refresh Devices");
	
	end_draw();
	
	RETURNOPT();
}


void draw_dump_progress(char* device, char* output_filename, uint64_t progress, uint64_t total) {
	start_draw();
	draw_background();
	
	char output_txt[128];
	snprintf(output_txt, sizeof(output_txt), "Backing up %s ...", device);
	draw_title(output_txt);

	snprintf(output_txt, sizeof(output_txt), "Writing \"%.35s\" ...", output_filename);
	draw_text_center(200, output_txt);
	
	draw_progress_bar(210, progress, total);
	snprintf(output_txt, sizeof(output_txt), "%llu / %llu (%i%%)", progress, total, (int)( (((float)progress) / ((float)total)) * 100.0));
	draw_text_center(260, output_txt);
	
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

int do_gc_options() {
	char title_id[64];
	char title[64];
	read_gameinfo(title_id, title);
	
	uint8_t have_grw0 = has_grw0();
	remove_illegal_chars(title);
	
	PROCESS_MENU(draw_gc_options, title, have_grw0, 1);
}
int do_gc_full_dump(char* output_file) {
	umount_gro0();
	umount_grw0();

	lock_shell();
	GcKeys keys;
	extract_gc_keys(&keys);
	int res = dump_device("sdstor0:gcd-lp-ign-entire", output_file, &keys, draw_dump_progress);
	unlock_shell();

	return res;
}

void do_confirm_message(char* title, char* msg) {
	draw_confirmation_message(title, msg);
	get_key();
}

int do_device_dump(char* input_device, char* output_file) {
	umount_gro0();
	umount_grw0();

	lock_shell();
	int res = dump_device(input_device, output_file, NULL, draw_dump_progress);
	unlock_shell();

	return res;
}


int do_select_output_location(char* format, char* output) {
	
	mount_devices();
	
	uint8_t save_network = 0;

	uint8_t xmc_exist = file_exist("xmc0:");
	uint8_t uma_exist = file_exist("uma0:");
	uint8_t host_exist = file_exist("host0:");
	
	PROCESS_MENU(draw_select_output_location, output, xmc_exist, uma_exist, host_exist, save_network);	
}