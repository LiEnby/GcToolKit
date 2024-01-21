typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

typedef struct bridge_s {
    uint32_t paddr;
    uint32_t args[8];  // 4 regs + 0x10 stack
} bridge_s;

int _start(bridge_s *cfg) {
	int (*func)(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7) = (void*)(cfg->paddr);
	return func(cfg->args[0], cfg->args[1], cfg->args[2], cfg->args[3], cfg->args[4], cfg->args[5], cfg->args[6], cfg->args[7]);
}