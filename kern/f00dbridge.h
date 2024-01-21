int HasCmd20Captured();
int GetLastCmd20KeyId();
int GetLastCmd20Input(char* cmd20_input);
void DecryptSecondaryKey0(void* data, uint32_t key_id, void* packet9, void* out);
typedef struct SceSblSmCommGcData {
    int always1;
    int command;
    uint8_t data[2048];
    int key_id;
    int size;
    int always0;
} SceSblSmCommGcData;