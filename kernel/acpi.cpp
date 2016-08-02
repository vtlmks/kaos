
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

//struct RSDPDescriptor20 {

struct ACPISDTHeader {		// 64-bit version
	char	Signature[4];
	u32		Length;			// 64-bit, otherwise u8
	u8		Revision;
	u8		Checksum;
	char	OEMID[6];
	char	OEMTableID[8];
	u32		OEMRevision;
	u32		CreatorID;
	u32		CreatorRevision;
} __attribute__ ((packed));


struct MADT {
	u32		LocalControllerAddress;
	u32		Flags;			// 1= Dual 8259 Legacy PICs installed
}__attribute__ ((packed));

struct MADTEntryHeader {
	u8		EntryType;		// {1..3}
	u8		Length;
}__attribute__ ((packed));

struct MADTEntry0 {			// Processor Local APIC
	u8		ACPIProcessorID;
	u8		APICID;
	u32		Flags;			// 1 = Processor enabled
}__attribute__ ((packed));

struct MADTEntry1 {			// I/O APIC
	u8		IOAPICID;
	u8		Reserved;
	u32		IOAPICAddress;
	u32		GlobalSystemInterruptBase;
}__attribute__ ((packed));

struct MADTEntry2 {			// Interrupt Source Override
	u8		BusSource;
	u8		IRQSource;
	u32		GlobalSystemInterrupt;
	u16		Flags;
}__attribute__ ((packed));



void setupACPI() {
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


	ACPISDTHeader* rsdt = (ACPISDTHeader*)  ((u64)rsdd->RsdtAddress);

	u8 checksum=0;
	for(u8 i=0;i<sizeof(RSDPDescriptor);i++) {
		checksum+= *(u8*)(((u8*)rsdd)+i);
	}
	if(checksum) {
		kprintf("Invalid RSD checksum.\n");
		asm("jmp .;");
	}

	u8 ptrs = (rsdt->Length - sizeof(ACPISDTHeader))/4;
	for(u64 p = (u64)rsdt + sizeof(ACPISDTHeader); p<((u64)rsdt) + sizeof(ACPISDTHeader) + (ptrs*4);p+=4)
	{
		u64 ptr = (u64) (*(u32*)p);
		kprintf("Parsing ACPI structures.\n");
		if( *(u32*)(ptr) == *(u32*)"APIC") {
			ACPISDTHeader* madtSdtHeader = (ACPISDTHeader*) ptr;
			//kprintf("MADT length = %d\n", madtSdtHeader->Length);
			
			MADT* madt = (MADT*) (ptr+sizeof(ACPISDTHeader));
			kprintf("LAPIC adress = %08x\n", madt->LocalControllerAddress);
			
			u64 offset = sizeof(ACPISDTHeader) + sizeof(MADT);
			while(offset < madtSdtHeader->Length) 
			{
				MADTEntryHeader* entry = (MADTEntryHeader *) (ptr + offset);
				switch(entry->EntryType) {

					case 0:
						kprintf("  Local APIC offset %d, length %d\n", offset, entry->Length);
						break;

					case 1: {
						MADTEntry1* ioapic = (MADTEntry1*) (ptr+offset+sizeof(MADTEntryHeader));
						kprintf("  IOAPICID %d, address:%08x\n", ioapic->IOAPICID, ioapic->IOAPICAddress);
						break;
					}
					case 2: {
						MADTEntry2* intOverride = (MADTEntry2*) (ptr+offset+sizeof(MADTEntryHeader));
						kprintf("  ISO BusSource:%d, IRQSource:%d, GSI:%08x, Flags:%4x\n", intOverride->BusSource, intOverride->IRQSource, intOverride->GlobalSystemInterrupt, intOverride->Flags);
						break;
					}
					case 6:
						kprintf("  I/O SAPIC\n");
						break;

					default:
						kprintf("  Type %d offset %d, length %d\n", entry->EntryType,offset, entry->Length);
						break;
				}
				offset += entry->Length;
			} 
		}
		else
			kprintf("Found other header %016x %-4s. \n", ptr, (char*) "XXXX");
	}
	//asm("jmp .;");
}

/*
					 T D S R              O B          S H C
000000001ffe3f0c: 0x54445352 0x00000040 0x4f421901 0x20534843
					C P X  B   T  DSR                 CPXB
000000001ffe3f1c: 0x43505842 0x54445352 0x00000001 0x43505842
000000001ffe3f2c: 0x00000001 0x1ffe1df8 0x1ffe1e6c 0x1ffe2130
000000001ffe3f3c: 0x1ffe21a8 0x1ffe21e0 0x1ffe221c 0x1ffe225c
000000001ffe3f4c: 0x00000000 0x00000000 0x00000000 0x00000000


*/

//00000000000f62e0: 0x52 0x53 0x44 0x20 0x50 0x54 0x52 0x20
//00000000000f62e8: 0xcf 0x42 0x4f 0x43 0x48 0x53 0x20 0x00
//00000000000f62f0: 0x5c 0x22 0xfe 0x07
