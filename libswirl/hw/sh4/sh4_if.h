/*
	This file is part of libswirl
*/
#include "license/bsd"


#pragma once
#include "types.h"
#include <memory>
#include "sh4_mmr.h"

enum Sh4RegType
{
	//GPRs
	reg_r0,
	reg_r1,
	reg_r2,
	reg_r3,
	reg_r4,
	reg_r5,
	reg_r6,
	reg_r7,
	reg_r8,
	reg_r9,
	reg_r10,
	reg_r11,
	reg_r12,
	reg_r13,
	reg_r14,
	reg_r15,

	//FPU, bank 0
	reg_fr_0,
	reg_fr_1,
	reg_fr_2,
	reg_fr_3,
	reg_fr_4,
	reg_fr_5,
	reg_fr_6,
	reg_fr_7,
	reg_fr_8,
	reg_fr_9,
	reg_fr_10,
	reg_fr_11,
	reg_fr_12,
	reg_fr_13,
	reg_fr_14,
	reg_fr_15,

	//FPU, bank 1
	reg_xf_0,
	reg_xf_1,
	reg_xf_2,
	reg_xf_3,
	reg_xf_4,
	reg_xf_5,
	reg_xf_6,
	reg_xf_7,
	reg_xf_8,
	reg_xf_9,
	reg_xf_10,
	reg_xf_11,
	reg_xf_12,
	reg_xf_13,
	reg_xf_14,
	reg_xf_15,

	//GPR Interrupt bank
	reg_r0_Bank,
	reg_r1_Bank,
	reg_r2_Bank,
	reg_r3_Bank,
	reg_r4_Bank,
	reg_r5_Bank,
	reg_r6_Bank,
	reg_r7_Bank,

	//Misc regs
	reg_gbr,
	reg_ssr,
	reg_spc,
	reg_sgr,
	reg_dbr,
	reg_vbr,
	reg_mach,
	reg_macl,
	reg_pr,
	reg_fpul,
	reg_nextpc,
	reg_sr,            //Includes T (combined on read/separated on write)
	reg_old_sr_status, //Only the status bits
	reg_sr_status,     //Only the status bits
	reg_sr_T,          //Only T
	reg_old_fpscr,
	reg_fpscr,
	
	reg_pc_dyn,        //Write only, for dynarec only (dynamic block exit address)

	sh4_reg_count,

	/*
		These are virtual registers, used by the dynarec decoder
	*/
	regv_dr_0,
	regv_dr_2,
	regv_dr_4,
	regv_dr_6,
	regv_dr_8,
	regv_dr_10,
	regv_dr_12,
	regv_dr_14,

	regv_xd_0,
	regv_xd_2,
	regv_xd_4,
	regv_xd_6,
	regv_xd_8,
	regv_xd_10,
	regv_xd_12,
	regv_xd_14,

	regv_fv_0,
	regv_fv_4,
	regv_fv_8,
	regv_fv_12,

	regv_xmtrx,
	regv_fmtrx,

	NoReg=-1
};

//Varius sh4 registers

union sr_status_t
{
	struct
	{
		u32 T_h     : 1;//<<0
		u32 S       : 1;//<<1
		u32 rsvd0   : 2;//<<2
		u32 IMASK   : 4;//<<4
		u32 Q       : 1;//<<8
		u32 M       : 1;//<<9
		u32 rsvd1   : 5;//<<10
		u32 FD      : 1;//<<15
		u32 rsvd2   : 12;//<<16
		u32 BL      : 1;//<<28
		u32 RB      : 1;//<<29
		u32 MD      : 1;//<<20
		u32 rsvd3   : 1;//<<31
	};
	u32 status;
};

//Status register bitfield
struct sr_t
{
	union
	{
		struct
		{
			u32 T_h     : 1;//<<0
			u32 S       : 1;//<<1
			u32 rsvd0   : 2;//<<2
			u32 IMASK   : 4;//<<4
			u32 Q       : 1;//<<8
			u32 M       : 1;//<<9
			u32 rsvd1   : 5;//<<10
			u32 FD      : 1;//<<15
			u32 rsvd2   : 12;//<<16
			u32 BL      : 1;//<<28
			u32 RB      : 1;//<<29
			u32 MD      : 1;//<<20
			u32 rsvd3   : 1;//<<31
		};
		u32 status;
	};
	u32 T;

};

//FPSCR (fpu status and control register) bitfield
struct fpscr_t
{
	union
	{
		u32 full;
		struct
		{
			u32 RM         : 2;
			u32 finexact   : 1;
			u32 funderflow : 1;
			u32 foverflow  : 1;
			u32 fdivbyzero : 1;
			u32 finvalidop : 1;
			u32 einexact   : 1;
			u32 eunderflow : 1;
			u32 eoverflow  : 1;
			u32 edivbyzero : 1;
			u32 einvalidop : 1;
			u32 cinexact   : 1;
			u32 cunderflow : 1;
			u32 coverflow  : 1;
			u32 cdivbyzero : 1;
			u32 cinvalid   : 1;
			u32 cfpuerr    : 1;
			u32 DN         : 1;
			u32 PR         : 1;
			u32 SZ         : 1;
			u32 FR         : 1;
			u32 pad        : 10;
		};
		struct
		{
			u32 _nil   : 2+1+1+1+1+4+8+1;
			u32 PR_SZ : 2;
			u32 nilz  : 11;
		};
	};
};


typedef void RunFP();
typedef void StopFP();
typedef void StartFP();
typedef void StepFP();
typedef void SkipFP();
typedef void ResetFP(bool Manual);
typedef void InitFP();
typedef void TermFP();
typedef bool IsCpuRunningFP();

/*
	The interface stuff should be replaced with something nicer
*/
//sh4 interface

struct MMIODevice;
struct SystemBus;
struct SuperH4Mmr;

enum SuperH4Backends {
    SH4BE_INTERPRETER,
    SH4BE_DYNAREC
};


enum Area0Hanlders {
	A0H_BIOS,
	A0H_FLASH,
	A0H_GDROM,
	A0H_SB,
	A0H_PVR,
	A0H_EXTDEV_006,
	A0H_AICA,
	A0H_RTC,
	A0H_EXTDEV_010,

	A0H_MAPLE,
	A0H_ASIC,
	A0H_SPG,
	A0H_SCPU,
	A0H_DSP,

	A0H_MAX
};

struct SuperH4 {
	VLockedMemory mram;
	VLockedMemory vram;
	VLockedMemory aica_ram;
	unique_ptr<SuperH4Mmr> sh4mmr;

	virtual void SetA0Handler(Area0Hanlders slot, MMIODevice* dev) = 0;
	virtual MMIODevice* GetA0Handler(Area0Hanlders slot) = 0;

	template<class T>
	T* GetA0H(Area0Hanlders slot) {
		return dynamic_cast<T*>(GetA0Handler(slot));
	}

    virtual bool setBackend(SuperH4Backends backend) = 0;

    virtual bool Init() = 0;
    virtual void Reset(bool Manual) = 0;
    virtual void Term() = 0;

    virtual void Run() = 0;
    virtual void Stop() = 0;
    virtual void Start() = 0;
    virtual void Step() = 0;
    virtual void Skip() = 0;

    virtual bool IsRunning() = 0;

    virtual void ResetCache() = 0;

	virtual ~SuperH4() { }

	virtual void serialize(void** data, unsigned int* total_size) { }
	virtual void unserialize(void** data, unsigned int* total_size) { }

	static SuperH4* Create();
};

struct Sh4Context
{
	union
	{
		struct
		{
			f32 xffr[32];
			u32 r[16];

			union
			{
				u64 full;
				struct
				{
					u32 l;
					u32 h;
				};
			} mac;

			u32 r_bank[8];

			u32 gbr,ssr,spc,sgr,dbr,vbr;
			u32 _pr,fpul;
			u32 pc;

			u32 jdyn;

			sr_t sr;
			fpscr_t fpscr;
			sr_status_t old_sr;
			fpscr_t old_fpscr;

			volatile u32 CpuRunning;

			int sh4_sched_next;
			u32 interrupt_pend;
		};
		u64 raw[64-8];
	};

};

u32 sh4context_offset_u32(u32 sh4_reg);
u32 sh4context_offset_regtype(Sh4RegType sh4_reg);

void DYNACALL do_sqw_mmu(u32 dst);
extern "C" void DYNACALL do_sqw_nommu_area_3(u32 dst, u8* sqb);
extern "C" void DYNACALL do_sqw_nommu_area_3_nonvmem(u32 dst, u8* sqb);
void DYNACALL do_sqw_nommu_full(u32 dst, u8* sqb);

typedef void DYNACALL sqw_fp(u32 dst,u8* sqb);
typedef void DYNACALL TaListVoidFP(void* data);

#define FPCB_SIZE (RAM_SIZE_MAX/2)
#define FPCB_MASK (FPCB_SIZE -1)
//#defeine FPCB_PAD 0x40000
#define FPCB_PAD 0x100000
#define FPCB_OFFSET (-(FPCB_SIZE*sizeof(void*) + FPCB_PAD)) 
struct Sh4RCB
{
	void* fpcb[FPCB_SIZE];
	u64 _pad[(FPCB_PAD-sizeof(Sh4Context)-64-sizeof(void*)*2)/8];
	TaListVoidFP* tacmd_voud; //*TODO* remove (not used)
	sqw_fp* do_sqw_nommu;
	u64 sq_buffer[64/8];
	Sh4Context cntx;
};

extern "C" Sh4RCB* p_sh4rcb;
extern u8* sh4_dyna_rcb;

INLINE u32 sh4_sr_GetFull()
{
	return (p_sh4rcb->cntx.sr.status & 0x700083F2) | p_sh4rcb->cntx.sr.T;
}

INLINE void sh4_sr_SetFull(u32 value)
{
	p_sh4rcb->cntx.sr.status=value & 0x700083F2;
	p_sh4rcb->cntx.sr.T=value&1;
}

#define do_sqw_nommu sh4rcb.do_sqw_nommu

template<typename T>
s32 rcb_noffs(T* ptr)  
{ 
	s32 rv= (s32)((u8*)ptr - (u8*)p_sh4rcb-sizeof(Sh4RCB)); 
	verify(rv<0);

	return rv;
}

template<typename T>
s32 rcb_poffs(T* ptr)  
{ 
	s32 rv= (u8*)ptr - (u8*)p_sh4rcb-sizeof(Sh4RCB); 
	verify(rv>=0);

	return rv;
}

#define sh4rcb (*p_sh4rcb)
#define Sh4cntx (sh4rcb.cntx)

struct SuperH4Backend {
    virtual bool Init() = 0;
    virtual void Loop() = 0;
    virtual void ClearCache() = 0;

	virtual ~SuperH4Backend() {  }
};

//Get an interface to sh4 interpreter
SuperH4Backend* Get_Sh4Interpreter();
SuperH4Backend* Get_Sh4Recompiler();

u32* GetRegPtr(u32 reg);
