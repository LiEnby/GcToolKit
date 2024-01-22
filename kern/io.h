int OpenDevice(char* device);
int ReadDevice(int device_handle, uint8_t* data, int size);
int CloseDevice(int device_handle);
uint64_t GetDeviceSize(int device_handle);