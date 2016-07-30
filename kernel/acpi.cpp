
#include "include\assembler.h"
#include "include\types.h"

int kprintf(const char *formatString, ...);


// RSDP Root System Description Pointer

struct RSDPDescriptor {
	char	Signature[8];
	u8		Checksum;
	char	OEMID[6];
	u8		Revision;
	u32		RsdtAddress;
} __attribute__ ((packed));

struct RSDPDescriptor20 {
 RSDPDescriptor firstPart;
 
 u32 Length;
 u64 XsdtAddress;
 u8 ExtendedChecksum;
 u8 reserved[3];
} __attribute__ ((packed));


struct ACPISDTHeader {
  char	Signature[4];
  u8	Length;
  u8	Revision;
  u8	Checksum;
  char	OEMID[6];
  char	OEMTableID[8];
  u32	OEMRevision;
  u32	CreatorID;
  u32	CreatorRevision;
};

//struct RSDT {
//  struct ACPISDTHeader h;
//  u32 PointerToOtherSDT[(h.Length - sizeof(h)) / 4];
//};

//struct XSDT {
//  struct ACPISDTHeader h;
//  u64 PointerToOtherSDT[(h.Length - sizeof(h)) / 8];
//};


const u8* memSearchStart = (const u8*)0xe0000;
u32 i;

void search() {
	const char* rsdMagic = "RSD PTR ";
	u64 rsdptr = 0;
	kprintf("ACPI initialization.\n");
	for (u64 adress = 0xe0000; adress < 0xfffff; adress+=0x10)
	{
		if( *(u64*)(adress) == *(u64*)rsdMagic) {
			rsdptr = adress;
			//kprintf("Found RSDPTR at %016x\n", (void*)adress);
			break;
		}
	}
	if(!rsdptr)
	{
		kprintf("RSDPTR not found.\n");
		asm("jmp .;");
	}
	RSDPDescriptor *rsdd = (RSDPDescriptor *) (u64*)rsdptr;
	kprintf("  RSD : %016x",(u64) rsdd);
	kprintf(",Revision:%d",rsdd->Revision);
	kprintf(",RsdtAddress:%016x\n", (u32) rsdd->RsdtAddress); // & ~0xfff);

	u8 checksum=0;
	for(u8 i=0;i<sizeof(RSDPDescriptor);i++) {
		checksum+= *(u8*)(((u8*)rsdd)+i);
	}
	if(checksum) {
		kprintf("Invalid RSD checksum.\n");
		asm("jmp .;");
	}

	asm("jmp .;");

}

//00000000000f62e0: 0x52 0x53 0x44 0x20 0x50 0x54 0x52 0x20
//00000000000f62e8: 0xcf 0x42 0x4f 0x43 0x48 0x53 0x20 0x00
//00000000000f62f0: 0x5c 0x22 0xfe 0x07
