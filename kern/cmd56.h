#include <stdint.h>
#include <stdio.h>

typedef struct SceSblSmCommGcData {
    int always1;
    int command;
    uint8_t data[2048];
    int key_id;
    int size;
    int always0;
} SceSblSmCommGcData;

typedef struct GcInteruptInfo{
	SceUID request_id;
	SceUID op_sync_id;
	char unk[0x20];
} GcInteruptInfo;

int cmd56_patch();
int cmd56_unpatch();