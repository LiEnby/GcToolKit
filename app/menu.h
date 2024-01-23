void do_gc_insert_prompt();
int do_gc_options();
int do_select_output_location(char* format, char* output);
int do_gc_full_dump(char* output_file);
int do_device_dump(char* input_device, char* output_file);
void do_confirm_message(char* title, char* msg);

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

enum select_output_options {
	DUMP_LOCATION_UX0,
	DUMP_LOCATION_XMC,
	DUMP_LOCATION_UMA,
	DUMP_LOCATION_HOST,
	DUMP_LOCATION_NET,
	CHANGE_FILENAME,
	RELOAD_DEVICES
};