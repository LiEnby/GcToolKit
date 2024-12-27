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
#include "log.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vitasdk.h>
#include <stdint.h>

static vita2d_texture* insertgc_tex;
static uint8_t options[0x1000];

#define DEFOPT(y) int option = 0;\
				  int opt_y = y; \
				  int increment_y = 20; \
				  int total = 0; \
				  memset(options, 0x00, sizeof(options))
#define ADDOPT(cond,x) if(cond) { \
					draw_option(opt_y, x, option == *selected); \
					opt_y += increment_y; \
					total++; \
					options[option] = 1; \
					PRINT_STR("options[%i] = 1\n", option); \
				   } else { \
					options[option] = 0; \
					PRINT_STR("options[%i] = 0\n", option); \
				   } \
				   option++ 

#define RETURNOPT() return option
#define CALC_FIRST_OPTION() for(first_option = 0; (options[first_option] != 1 && first_option < sizeof(options)); first_option++)
#define CALC_LAST_OPTION() for(last_option = sizeof(options); (options[last_option] != 1 && last_option > 0); last_option--)
#define WINDOW_SIZE (20)
#define PROCESS_MENU(func, ...) \
					  int window = 0; \
					  int selected = 0; \
					  memset(options, 0x00, sizeof(options));\
					  int total_options = func(&selected, &window, __VA_ARGS__); \
					  int first_option = 0;\
					  int last_option = 0;\
					  CALC_FIRST_OPTION(); \
					  CALC_LAST_OPTION(); \
					  selected = first_option;\
					  \
					while (1) { \
					  total_options = func(&selected, &window, __VA_ARGS__); \
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
					     case SCE_CTRL_CIRCLE: \
							 return OP_CANCELED; \
						 case SCE_CTRL_CROSS: \
							 break; \
					  } \
					  if(ctrl == SCE_CTRL_CROSS) break; \
					  if(selected > last_option)  { \
						selected = last_option; \
						\
						if(total_options > WINDOW_SIZE) \
							window++; \
					  } \
					  if(selected < first_option) { \
						selected = first_option; \
						\
						if(window != first_option) \
							window--; \
					  } \
					  PRINT_STR("selected: %x\n", selected); \
					  PRINT_STR("window: %x\n", window); \
				  }

#define RESTORE_MENU(title, what, dev, to) start_draw(); \
					 draw_background(); \
					 \
					 char output_txt[128]; \
					 snprintf(output_txt, sizeof(output_txt), title " %.30s ...", dev); \
					 draw_title(output_txt); \
					 \
					 snprintf(output_txt, sizeof(output_txt), what " \"%.30s\" ...", to); \
					 draw_text_center(200, output_txt); \
					 \
					 draw_progress_bar(210, progress, total); \
					 snprintf(output_txt, sizeof(output_txt), "%llu / %llu (%i%%)", progress, total, (int)( (((float)progress) / ((float)total)) * 100.0)); \
					 draw_text_center(260, output_txt); \
					 \
					 end_draw()

void init_menus() {
	insertgc_tex = load_texture("app0:/res/insertgc.png");

}

void term_menus() {
	free_texture(insertgc_tex);	
}

void draw_wipe_progress(char* device, char* unused, uint64_t progress, uint64_t total) {
	RESTORE_MENU("Formatting", "Writing", device, device);
}

void draw_restore_progress(char* device, char* input_filename, uint64_t progress, uint64_t total) {
	RESTORE_MENU("Restoring", "Reading", device, input_filename);
}

void draw_dump_progress(char* device, char* output_filename, uint64_t progress, uint64_t total) {
	RESTORE_MENU("Backing up", "Writing", device, output_filename);
}


int draw_gc_options(int* selected, int* window, char* title, uint8_t has_grw0, uint8_t has_mediaid) {
	start_draw();
	draw_background();

	char w_title[128];
	snprintf(w_title, sizeof(w_title), "What to do with %.25s ...", title);
	draw_title(w_title);
	
	DEFOPT(200);

	ADDOPT(1, "Backup Entire Game Cart (.VCI)");
	ADDOPT(1, "Backup Game Cart Keys (.BIN)");

	ADDOPT(has_mediaid, "Backup MediaId Section (.MEDIAID)");
	ADDOPT(has_mediaid, "Restore MediaId Section (.MEDIAID)");		
	ADDOPT(has_mediaid, "Format MediaId Section");

	ADDOPT(has_grw0, "Backup Writable Section (.IMG)");
	ADDOPT(has_grw0, "Restore Writable Section (.IMG)");
	ADDOPT(has_grw0, "Format Writable Section");
	
	ADDOPT(1, "Get Game Cart Info");
	ADDOPT(1, "Swap Game Carts");
	
	end_draw();
	
	RETURNOPT();
}

void draw_insert_gc_menu() {
	start_draw();
	draw_background();
	
	draw_title("Waiting for CMD56 authentication ...");
	draw_texture_center(140, insertgc_tex);
	draw_text_center(360, "Insert a VITA game cart!");	
	end_draw();
}

void do_gc_insert_prompt() {
	draw_insert_gc_menu();
	wait_for_gc_auth();	
}

int draw_select_input_location(int* selected, int* window, uint8_t have_ux0, uint8_t have_xmc, uint8_t have_usb, uint8_t have_host0) {
	
	start_draw();
	draw_background();
	
	draw_title("Select input device ...");
	
	DEFOPT(240);
	
	ADDOPT(have_ux0, "Load from \"ux0\"");
	ADDOPT(have_xmc, "Load from Sony Memory Card");
	ADDOPT(have_usb, "Load from USB Drive");
	ADDOPT(have_host0, "Load from Devkit Host0");
	ADDOPT(1, "Refresh Devices");
	
	end_draw();
	
	RETURNOPT();
}

int draw_select_output_location(int* selected, int* window, char* output_file, uint8_t have_ux0, uint8_t have_xmc, uint8_t have_usb, uint8_t have_host0, uint8_t save_network) {
	
	start_draw();
	draw_background();
	
	draw_title("Select output device ...");

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


int draw_select_file(int* selected, int* window, char* input_folder, char* folders, int total_files) {
	start_draw();
	draw_background();
	
	char title[128];
	snprintf(title, sizeof(title), "Select a file from: %.15s ...", input_folder);
	draw_title(title);
	
	DEFOPT(110);

	// check if window - total_files is less than the window size.	
	// reset window to window_size if it is
	if( *window > (total_files % WINDOW_SIZE) ) {
		*window = (total_files % WINDOW_SIZE);
	}
	
	for(int i = *window; i <= *window + WINDOW_SIZE; i++) {
		if(i >= total_files) break;
		
		char file[MAX_PATH];
		snprintf(file, sizeof(file), "%.45s", folders + (i * MAX_PATH));
		ADDOPT(1, file);
	}
	
	end_draw();
	
	RETURNOPT();		
}


int draw_network_settings(int* selected, int* window, char* ip_address, unsigned short port) {
	
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

void draw_device_info(char* cardId, char* cardCsd, uint8_t vendorId) {
	start_draw();
	draw_background();
	
	draw_title("GC Information");
	
	char msg[0x100];
	snprintf(msg, sizeof(msg), "CID: %s", cardId);
	draw_text_center(200, msg);

	snprintf(msg, sizeof(msg), "CSD: %s", cardCsd);
	draw_text_center(220, msg);

	snprintf(msg, sizeof(msg), "Vendor: %s (0x%02X)", mmc_vendor_id_to_manufacturer(vendorId), vendorId);
	draw_text_center(240, msg);
	
	draw_text_center(280, "Press any button to continue ...");
		
	end_draw();
}


void do_device_info() {
	uint8_t cardId[0x10];
	uint8_t cardCsd[0x10];
	
	memset(cardId, 0xFF, sizeof(cardId));
	memset(cardCsd, 0xFF, sizeof(cardCsd));
	
	// TODO: read cmd56 parms
	uint16_t cardKeyId = 0x1;
	char cardRandom[0x10];
	
	char cardRandomHex[0x100];
	
	char cardIdHex[0x100];
	char cardCsdHex[0x100];
	
	int cidRes = GetCardId(1, cardId);
	int csdRes = GetCardCsd(1, cardCsd);
	
	if(cidRes >= 0 && csdRes >= 0){
		TO_HEX(cardId, sizeof(cardId), cardIdHex, sizeof(cardIdHex));
		TO_HEX(cardCsd, sizeof(cardCsd), cardCsdHex, sizeof(cardCsdHex));
		
		uint8_t vendorId = cardId[0xF];
		
		draw_device_info(cardIdHex, cardCsdHex, vendorId);		
	}
	get_key();
	
}

int do_network_options(char* ip_address, unsigned short port) {	
	PROCESS_MENU(draw_network_settings, ip_address, port);
	return selected;
}

int do_gc_options() {
	char title_id[64];
	char title[64];

	mount_gro0();
	mount_grw0();

	read_gameinfo(title_id, title);
	
	remove_illegal_chars(title);
	
	uint8_t has_grw0 = device_exist(BLOCK_DEVICE_GRW0);
	uint8_t has_mediaid = device_exist(BLOCK_DEVICE_MEDIAID);
	
	PROCESS_MENU(draw_gc_options, title, has_grw0, has_mediaid);
	return selected;
}

int do_select_file(char* folder, char* output, char* extension, uint64_t max_size) {
	int total_files = 0;	
	static char files[MAX_PATH * sizeof(options)];
	
	SearchFilter filter;
	memset(&filter, 0x00, sizeof(SearchFilter));
	filter.max_filesize = max_size;
	filter.file_only = 1;
	strncpy(filter.match_extension, extension, sizeof(filter.match_extension));
	
	int res = get_files_in_folder(folder, files, &total_files, &filter, sizeof(options));
	
	PRINT_STR("get_files_in_folder = %x\n", res);
	if(res < 0) return res;
	if(total_files <= 0) return -2;
	
	PRINT_STR("total_files: %x\n", total_files);
	
	PROCESS_MENU(draw_select_file, folder, files, total_files);
	strncpy(output, files + (selected * MAX_PATH), MAX_PATH);	
	return selected;	
}

void do_ime() {
	for(int i = 0; i < 0x5; i++)
		draw_ime();
}

void do_confirm_message(char* title, char* msg) {
	draw_confirmation_message(title, msg);
	get_key();
}

int do_device_wipe(char* block_device, uint8_t format) {
	umount_gro0();
	umount_grw0();
	
	disable_power_off();

	mount_devices();
	
	lock_shell();
	int res = wipe_device(block_device, draw_wipe_progress);
	
	if(format)
		FormatDevice(block_device);
	
	unlock_shell();

	enable_power_off();

	mount_gro0();
	mount_grw0();
	
	umount_devices();
	return res;
}

int do_device_restore(char* block_device, char* input_file) {
	
	umount_gro0();
	umount_grw0();
	
	disable_power_off();

	mount_devices();
	
	lock_shell();
	int res = restore_device(block_device, input_file, draw_restore_progress);
	unlock_shell();

	enable_power_off();

	mount_gro0();
	mount_grw0();
	
	umount_devices();
	return res;
}

int do_device_dump(char* block_device, char* output_file, uint8_t vci, char* ip_address, unsigned short port) {
	
	GcKEYS keys;
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


int do_select_input_location() {
	
	PRINT_STR("mount_devices\n");
	mount_devices();
	
	uint8_t ux_exist = file_exist("ux0:");
	uint8_t xmc_exist = file_exist("xmc0:");
	uint8_t uma_exist = file_exist("uma0:");
	uint8_t host_exist = file_exist("host0:");
	
	PROCESS_MENU(draw_select_input_location, 
				ux_exist, 
				xmc_exist, 
				uma_exist,
				host_exist);
	
	return selected;
}

int do_error(int error) {
	char msg[0x1028];	
	snprintf(msg, sizeof(msg), "There was an error ( res = 0x%08X )", error);
	do_confirm_message("Error!", msg);
	return 0;
}

int do_select_output_location(char* output, uint64_t device_size) {
	
	PRINT_STR("mount_devices\n");
	mount_devices();
	
	uint8_t save_network = is_connected();
	PRINT_STR("save_network = %x\n", save_network);
	
	uint64_t xmc_size = get_free_space("xmc0:");
	uint64_t uma_size = get_free_space("uma0:");
	uint64_t ux_size  = get_free_space("ux0:");
	
	uint8_t ux_exist = file_exist("ux0:");
	uint8_t xmc_exist = file_exist("xmc0:");
	uint8_t uma_exist = file_exist("uma0:");
	uint8_t host_exist = file_exist("host0:");
	
	
	PRINT_STR("device_size %llx\n", device_size);
	PRINT_STR("xmc_size %llx\n", xmc_size);
	PRINT_STR("uma_size %llx\n", uma_size);
	PRINT_STR("ux_size %llx\n", ux_size);
	
	PROCESS_MENU(draw_select_output_location, output, 
				(ux_exist  && ux_size >= device_size), 
				(xmc_exist && xmc_size >= device_size), 
				(uma_exist && uma_size >= device_size), 
				host_exist, 
				save_network);
	
	return selected;
}