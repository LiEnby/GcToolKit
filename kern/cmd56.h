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

void cmd56_patch();
void cmd56_unpatch();