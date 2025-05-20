#define KMODULE_NAME "GcKernKit"

void load_kernel_modules();
int kernel_started();

int disable_power_off();
void enable_power_off();

void lock_shell();
void unlock_shell();

void init_shell();
void term_shell();