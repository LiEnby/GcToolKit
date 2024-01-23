
#define MAX_PATH (512)

int has_gro0();
int has_grw0();
int file_exist(char* path);
void remove_illegal_chars(char* str);

int wait_for_partition(char* partiton);
int mount_uma();
int mount_xmc();
int mount_imc();

void umount_gro0();
void umount_grw0();

void mount_devices();
void umount_uma();
void umount_xmc();
void umount_imc();
void umount_devices();