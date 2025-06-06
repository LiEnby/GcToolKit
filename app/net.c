#include <vitasdk.h>
#include <string.h>
#include <stdlib.h>
#include "err.h"
#include "net.h"
#include "log.h"

static uint8_t init = 0;
uint8_t connected = 0;
static char memory[16 * 1024];


int init_network() {
    int ret = 0;
	
	SceNetInitParam param;
	memset(&param, 0x00, sizeof(SceNetInitParam));
    param.memory = memory;
	param.size = sizeof(memory);
	param.flags = 0;
	
	int loadModule = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	PRINT_STR("sceSysmoduleLoadModule = %x\n", loadModule);
	if(loadModule < 0) ERROR(loadModule);
	int netInit = sceNetInit(&param);
	PRINT_STR("sceNetInit = %x\n", netInit);
	if(netInit < 0) ERROR(netInit);
	int netCtlInit = sceNetCtlInit();
	PRINT_STR("sceNetCtlInit = %x\n", netCtlInit);
	if(netCtlInit < 0) ERROR(netCtlInit);
	
	init = 1;
	
	return 0;
error:
	if(netCtlInit >= 0)
		sceNetCtlTerm();
	if(netInit >= 0)	
		sceNetTerm();
	if(loadModule >= 0)
		sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
	return ret;
}

void term_network() {
	sceNetCtlTerm();
	sceNetTerm();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
	init = 0;
}

uint8_t is_connected() {
	if(!init) return 0;
	int state = 0;
	PRINT_STR("sceNetCtlInetGetState before call %x\n", state);
	sceNetCtlInetGetState(&state);
	PRINT_STR("state = %x\n", state);

	if(state == SCE_NETCTL_STATE_CONNECTED)
		return 1;
	else
		return 0;
}

uint8_t check_ip_address_valid(char* ip_address) {
	uint64_t ip_addr_int = 0;
	PRINT_STR("sceNetPton call\n");
	int res = sceNetInetPton(SCE_NET_AF_INET, ip_address, &ip_addr_int);
	PRINT_STR("sceNetPton call res = %x\n", res);
	if(res < 0) return 0;
	else return 1;
}

int file_send_data(int fstream, void* data, size_t data_sz) {
	return sceNetSend(fstream, data, data_sz, 0);
}
void end_file_send(int fstream) {
	sceNetShutdown(fstream, SCE_NET_SHUT_RDWR);
	sceNetSocketClose(fstream);	
}

int begin_file_send(char* ip_address, unsigned short port, char* filename, uint64_t total_size) {
	int ret = 0;
	
	SceUID socket = sceNetSocket("filesocket", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
	if(socket < 0) ERROR(socket);
	
	// setup socket buffer
	SceNetSockaddrIn sin;
	
	memset(&sin, 0, sizeof(SceNetSockaddrIn));
	sin.sin_len = sizeof(SceNetSockaddrIn);
	sin.sin_family = SCE_NET_AF_INET;
	
	ret = sceNetInetPton(SCE_NET_AF_INET, ip_address, &sin.sin_addr);
	if(ret < 0) ERROR(ret);
	
	sin.sin_port = sceNetHtons(port);

	int connection = sceNetConnect(socket, (SceNetSockaddr*)&sin, sizeof(SceNetSockaddrIn));	
	if(connection < 0) ERROR(connection);
	
	send_file_packet packet;
	memset(&packet, 0x00, sizeof(send_file_packet));
	
	packet.magic = SEND_FILE_MAGIC;
	strncpy(packet.filename, filename, sizeof(packet.filename));
	packet.total_size = total_size;
	
	int send = sceNetSend(socket, &packet, sizeof(send_file_packet), 0);
	if(send < 0) ERROR(send);
	
	return socket;
	
	error:
	if(connection > 0)
		sceNetShutdown(socket, SCE_NET_SHUT_RDWR);
	if(socket > 0)
		sceNetSocketClose(socket);
	return ret;
}


