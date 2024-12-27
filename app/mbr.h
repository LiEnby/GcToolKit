#ifndef MBR_H 
#define MBR_H
#define SECTOR_SIZE (0x200)

enum GcCODE {
	GcCODE_EMPTY = 0x00,
	GcCODE_IDSTORAGE = 0x01,
	GcCODE_SLB2 = 0x02,
	GcCODE_OS0 = 0x03,
	GcCODE_VS0 = 0x04,
	GcCODE_VD0 = 0x05,
	GcCODE_TM0 = 0x06,
	GcCODE_UR0 = 0x07,
	GcCODE_UX0 = 0x08,
	GcCODE_GRO0 = 0x09,
	GcCODE_GRW0 = 0x0A,
	GcCODE_UD0 = 0x0B,
	GcCODE_SA0 = 0x0C,
	GcCODE_MEDIAID = 0x0D,
	GcCODE_PD0 = 0x0E,
	GcCODE_UNUSED = 0x0F
};

enum GcTYPE {
	GcTYPE_FAT16 = 0x06,
	GcTYPE_EXFAT = 0x07,
	GcTYPE_RAW = 0xDA
};

typedef struct GcKEYS{
	uint8_t rifbuf[0x20];
	uint8_t klic[0x20];
} GcKEYS;

typedef struct GcPART {
	uint32_t off;
	uint32_t sz;
	uint8_t code;
	uint8_t type;
	uint8_t active;
	uint32_t flags;
	uint16_t unk;
} GcPART;

typedef struct GcMBR {
	char magic[0x20];
	uint32_t version;
	uint32_t devicesize;
	char unk1[0x28];
	GcPART partitions[0x10];
	char unk2[0x5e];
	char unk3[0x10 * 4];
	uint16_t sig;
} GcMBR;


#endif