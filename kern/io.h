int kOpenDevice(char* device, int permission);
int kReadDevice(int device_handle, uint8_t* data, int size);
int kWriteDevice(int device_handle, uint8_t* data, int size)
int kCloseDevice(int device_handle);
uint64_t kGetDeviceSize(int device_handle);