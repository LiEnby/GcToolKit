#include <stdint.h>

void do_gc_insert_prompt();
int do_gc_options();
int do_select_output_location(char* output, uint64_t device_size);
int do_select_input_location();
int do_select_file(char* folder, char* output);
int do_device_dump(char* block_device, char* output_file, uint8_t vci, char* ip_address, unsigned short port);
int do_device_wipe(char* block_device, uint8_t format);
int do_device_restore(char* block_device, char* input_file);
void do_confirm_message(char* title, char* msg);
int do_network_options(char* ip_address, unsigned short port);
void do_ime();
int do_error(int error);

void init_menus();
void term_menus();

enum insert_menu_options {
	DUMP_WHOLE_GC,
	DUMP_KEYS_ONLY,
	DUMP_MEDIAID,
	WRITE_MEDIAID,
	RESET_MEDIAID,
	DUMP_GRW0,
	WRITE_GRW0,
	RESET_GRW0
	
};

enum select_network_options {
	CHANGE_IP,
	CHANGE_PORT,
	START_DUMP
};

enum select_input_options {
	RESTORE_LOCATION_UX0,
	RESTORE_LOCATION_XMC,
	RESTORE_LOCATION_UMA,
	RESTORE_LOCATION_HOST,
	I_RELOAD_DEVICES
};

enum select_output_options {
	DUMP_LOCATION_UX0,
	DUMP_LOCATION_XMC,
	DUMP_LOCATION_UMA,
	DUMP_LOCATION_HOST,
	DUMP_LOCATION_NET,
	CHANGE_FILENAME,
	O_RELOAD_DEVICES
};