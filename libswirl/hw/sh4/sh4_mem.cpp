/*
	This file is part of libswirl
*/
#include "license/bsd"


/*
	Mostly buggy, old, glue code that somehow still works
	Most of the work is now delegated on vtlb and only helpers are here
*/
#include "types.h"
#include <memory>

#include "sh4_mem.h"
#include "hw/holly/sh4_mem_area0.h"
#include "sh4_mmr.h"
#include "modules/modules.h"
#include "hw/pvr/pvr_mem.h"
#include "hw/sh4/sh4_core.h"
//#include "hw/sh4/rec_v1/blockmanager.h"
#include "hw/mem/_vmem.h"
#include "modules/mmu.h"

//MEM MAPPINNGG

//AREA 1
_vmem_handler area1_32b;
void map_area1_init(SuperH4* sh4)
{
	area1_32b = _vmem_register_handler(sh4->vram.data, pvr_read_area1_8,pvr_read_area1_16,pvr_read_area1_32,
									pvr_write_area1_8,pvr_write_area1_16,pvr_write_area1_32);
}

void map_area1(SuperH4* sh4, u32 base)
{
	//map vram
	
	//Lower 32 mb map
	//64b interface
	_vmem_map_block(sh4->vram.data,0x04 | base,0x04 | base,VRAM_SIZE-1);
	//32b interface
	_vmem_map_handler(area1_32b,0x05 | base,0x05 | base);
	
	//Upper 32 mb mirror
	//0x0600 to 0x07FF
	_vmem_mirror_mapping(0x06|base,0x04|base,0x02);
}

//AREA 2
void map_area2_init()
{
	//nothing to map :p
}

void map_area2(SuperH4* sh4, u32 base)
{
	//nothing to map :p
}


//AREA 3
void map_area3_init()
{
}

#if BUILD_COMPILER==COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif
void map_area3(SuperH4* sh4, u32 base)
{
	//32x2 or 16x4
	_vmem_map_block_mirror(sh4->mram.data,0x0C | base,0x0F | base,RAM_SIZE);
}
#if BUILD_COMPILER==COMPILER_CLANG
#pragma clang diagnostic pop
#endif

u8 OIX_CACHE[256][32];
u32 OIX_ADDR[256];
bool OIX_DIRTY[256];

template<u32 sz, class T>
T DYNACALL ReadMem_area4(void* ctx, u32 addr)
{
	verify(addr & (1<<25));
	verify((addr & (sz-1)) == 0);
	u32 index = (addr / 32) & 255;
	u32 offset = addr & 31;

	verify(OIX_ADDR[index] == (addr & ~31));

	return *(T*)&OIX_CACHE[index][offset];
}

template<u32 sz, class T>
void DYNACALL WriteMem_area4(void* ctx, u32 addr,T data)
{
	verify(addr & (1<<25));
	verify((addr & (sz-1)) == 0);
	
	u32 index = (addr / 32) & 255;
	u32 offset = addr & 31;

	verify(OIX_ADDR[index] == (addr & ~31));

	*(T*)&OIX_CACHE[index][offset] = data;
	OIX_DIRTY[index] = true;
}

_vmem_handler area4_handler;

//AREA 4
//Map for OIX mode
void map_area4_init(SuperH4* sh4)
{
	area4_handler = _vmem_register_handler_Template(sh4, ReadMem_area4,WriteMem_area4);
}

void map_area4(SuperH4* sh4, u32 base)
{
	_vmem_map_handler(area4_handler,0x10|base,0x11|base);

	//upper 32mb mirror lower 32 mb
	_vmem_mirror_mapping(0x12|base,0x10|base,0x02);
}


//AREA 5	--	Ext. Device
//Read Ext.Device
template <u32 sz,class T>
T DYNACALL ReadMem_extdev_T(void* sh4, u32 addr)
{
	return (T)libExtDevice_ReadMem_A5(addr,sz);
}

//Write Ext.Device
template <u32 sz,class T>
void DYNACALL WriteMem_extdev_T(void* sh4, u32 addr,T data)
{
	libExtDevice_WriteMem_A5(addr,data,sz);
}

_vmem_handler area5_handler;
void map_area5_init()
{
	area5_handler = _vmem_register_handler_Template(nullptr, ReadMem_extdev_T,WriteMem_extdev_T);
}

void map_area5(SuperH4* sh4, u32 base)
{
	//map whole region to plugin handler :)
	_vmem_map_handler(area5_handler,base|0x14,base|0x17);
}

//AREA 6	--	Unassigned 
void map_area6_init()
{
	//nothing to map :p
}
void map_area6(SuperH4* sh4, u32 base)
{
	//nothing to map :p
}


//set vmem to default values
void mem_map_default(SuperH4* sh4)
{
	//vmem - init/reset :)
	_vmem_init();

	
	//*TEMP*
	//setup a fallback handler , that calls old code :)
	//_vmem_handler def_handler =
	//	_vmem_register_handler(ReadMem8_i,ReadMem16_i,ReadMem32_i,WriteMem8_i,WriteMem16_i,WriteMem32_i);
	//_vmem_map_handler(def_handler,0,0xFFFF);

	//U0/P0
	//0x0xxx xxxx	-> normal memmap
	//0x2xxx xxxx	-> normal memmap
	//0x4xxx xxxx	-> normal memmap
	//0x6xxx xxxx	-> normal memmap
	//-----------
	//P1
	//0x8xxx xxxx	-> normal memmap
	//-----------
	//P2
	//0xAxxx xxxx	-> normal memmap
	//-----------
	//P3
	//0xCxxx xxxx	-> normal memmap
	//-----------
	//P4
	//0xExxx xxxx	-> internal area

	//Init Memmaps (register handlers)
	map_area0_init(sh4);
	map_area1_init(sh4);
	map_area2_init();
	map_area3_init();
	map_area4_init(sh4);
	map_area5_init();
	map_area6_init();
	map_area7_init(sh4->sh4mmr.get());

	//0x0-0xD : 7 times the normal memmap mirrors :)
	//some areas can be customised :)
	for (int i=0x0;i<0xE;i+=0x2)
	{
		map_area0(sh4, i<<4); //Bios,Flahsrom,i/f regs,Ext. Device,Sound Ram
		map_area1(sh4, i<<4); //VRAM
		map_area2(sh4, i<<4); //Unassigned
		map_area3(sh4, i<<4); //RAM
		map_area4(sh4, i<<4); //TA
		map_area5(sh4, i<<4); //Ext. Device
		map_area6(sh4, i<<4); //Unassigned
		map_area7(sh4, i<<4); //Sh4 Regs
	}

	//map p4 region :)
	map_p4(sh4, sh4->sh4mmr.get());
}

void mem_Init(SuperH4* sh4)
{
	//Allocate mem for memory/bios/flash
	//mem_b.Init(&sh4_reserved_mem[0x0C000000],RAM_SIZE);
}

//Reset Sysmem/Regs -- Pvr is not changed , bios/flash are not zeroed out
void mem_Reset(SuperH4* sh4, bool Manual)
{
	//mem is reseted on hard restart(power on) , not manual...
	if (!Manual)
	{
		//fill mem w/ 0's
		sh4->mram.Zero();
	}
}

void mem_Term(SuperH4* sh4)
{
	//write back Flash/SRAM
	SaveRomFiles(get_writable_data_path(DATA_PATH));
	
	//mem_b.Term(); // handled by vmem

	//vmem
	_vmem_term();
}

void WriteMemBlock_nommu_dma(u32 dst,u32 src,u32 size)
{
	u32 dst_msk,src_msk;

	void* dst_ptr=_vmem_get_ptr2(dst,dst_msk);
	void* src_ptr=_vmem_get_ptr2(src,src_msk);

	if (dst_ptr && src_ptr)
	{
		memcpy((u8*)dst_ptr+(dst&dst_msk),(u8*)src_ptr+(src&src_msk),size);
	}
	else if (src_ptr)
	{
		WriteMemBlock_nommu_ptr(dst,(u32*)((u8*)src_ptr+(src&src_msk)),size);
	}
	else
	{
		verify(size % 4 == 0);
		for (u32 i=0;i<size;i+=4)
		{
			WriteMem32_nommu(dst+i,ReadMem32_nommu(src+i));
		}
	}
}
void WriteMemBlock_nommu_ptr(u32 dst,u32* src,u32 size)
{
	u32 dst_msk;

	void* dst_ptr=_vmem_get_ptr2(dst,dst_msk);

	if (dst_ptr)
	{
		dst&=dst_msk;
		memcpy((u8*)dst_ptr+dst,src,size);
	}
	else
	{
		for (u32 i = 0; i < size;)
		{
			u32 left = size - i;
			if (left >= 4)
			{
				WriteMem32_nommu(dst + i, src[i >> 2]);
				i += 4;
			}
			else if (left >= 2)
			{
				WriteMem16_nommu(dst + i, ((u16 *)src)[i >> 1]);
				i += 2;
			}
			else
			{
				WriteMem8_nommu(dst + i, ((u8 *)src)[i]);
				i++;
			}
		}
	}
}

void WriteMemBlock_nommu_sq(u32 dst,u32* src)
{
	u32 dst_msk;
	void* dst_ptr=_vmem_get_ptr2(dst,dst_msk);

	if (dst_ptr)
	{
		dst&=dst_msk;
		memcpy((u8*)dst_ptr+dst,src,32);
	}
	else
	{
		for (u32 i=0;i<32;i+=4)
		{
			WriteMem32_nommu(dst+i,src[i>>2]);
		}
	}
}

void WriteMemBlock_ptr(u32 addr,u32* data,u32 size)
{
#ifndef NO_MMU
	die("failed\n");
#endif
	WriteMemBlock_nommu_ptr(addr,data,size);
}

//Get pointer to ram area , 0 if error
//For debugger(gdb) - dynarec
u8* GetMemPtr(u32 Addr,u32 size)
{
	verify((((Addr>>29) &0x7)!=7));
	switch ((Addr>>26)&0x7)
	{
		case 3:
		return &sh4_cpu->mram[Addr & RAM_MASK];
		
		case 0:
		case 1:
		case 2:
		case 4:
		case 5:
		case 6:
		case 7:
		default:
//			EMUERROR("unsupported area : addr=0x%X",Addr);
			return 0;
	}
}

//Get information about an area , eg ram /size /anything
//For dynarec - needs to be done
void GetMemInfo(u32 addr,u32 size)
{
	//needs to be done
}

bool IsOnRam(u32 addr)
{
	if (((addr>>26)&0x7)==3)
	{
		if ((((addr>>29) &0x7)!=7) && (((addr>>29) &0x7)!=3))
		{
			return true;
		}
	}

	return false;
}

u32 GetRamPageFromAddress(u32 RamAddress)
{
	verify(IsOnRam(RamAddress));
	return (RamAddress & RAM_MASK)/REI_PAGE_SIZE;
}
