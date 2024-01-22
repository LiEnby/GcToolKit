void DecryptSecondaryKey0(void* data, uint32_t key_id, void* packet9, void* out);


typedef struct CommsData { 
    uint8_t packet6[32];
    uint8_t packet9[48];
    uint8_t packet17[32];
    uint8_t packet18[67];
    uint8_t packet19[16];
    uint8_t packet20[83];
} CommsData;