int ResetGc();
int ClearCartSecret();
int GetCartSecret(char* keys);

int HasCmd20Captured();
int GetLastCmd20KeyId();
int GetLastCmd20Input(char* cmd20_input);
int ResetCmd20Input();

int OpenDevice(char* device, int permission);
int ReadDevice(int device_handle, uint8_t* data, int size);
int WriteDevice(int device_handle, uint8_t* data, int size);
int CloseDevice(int device_handle);
void GetDeviceSize(int device_handle, uint64_t* device_size);

int FormatDevice(char* device);

int GetCardId(int deviceIndex, void* cardId);
int GetCardCsd(int deviceIndex, void* cardCsd);
int GetCardExtCsd(int deviceIndex, void* cardExtCsd);

typedef struct SceSblSmCommGcData {
    int always1;
    int command;
    uint8_t data[2048];
    int key_id;
    int size;
    int always0;
} SceSblSmCommGcData;

typedef struct CommsData { 
    uint8_t packet6[32];
    uint8_t packet9[48];
    uint8_t packet17[32];
    uint8_t packet18[67];
    uint8_t packet19[16];
    uint8_t packet20[83];
} CommsData;