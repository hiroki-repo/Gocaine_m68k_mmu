// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

typedef UINT32(typeofmemaccessfrombus)(int, int, int);

UINT32(*memaccessfrombus)(int, int, int);
UINT32 baseaddr4mmu_base[2] = { 0 ,0 };
UINT32* baseaddr4mmu = baseaddr4mmu_base;

typedef void (typeofm68k_set_irq)(unsigned int int_level);

void (*m68k_set_irq)(unsigned int int_level);

typedef void (typeofcallback4m68kfc4ext)(unsigned int new_fc);

void (*callback4m68kfc4ext)(unsigned int new_fc);

unsigned int fc4mac = 0;
UINT32 mmusetting = 0;

#define readmem8(a)  ((memaccessfrombus(a+0,0,1|(1<<8))<<(8*0)))
#define readmem16(a) ((memaccessfrombus(a+0,0,1|(1<<8))<<(8*1))|(memaccessfrombus(a+1,0,1|(1<<8))<<(8*0)))
#define readmem32(a) ((memaccessfrombus(a+0,0,1|(1<<8))<<(8*3))|(memaccessfrombus(a+1,0,1|(1<<8))<<(8*2))|(memaccessfrombus(a+2,0,1|(1<<8))<<(8*1))|(memaccessfrombus(a+3,0,1|(1<<8))<<(8*0)))
#define setmmufaultstat(a) mmusetting &= 0xFFFFFF0F;mmusetting|=(((a&0xF)<<4)&0xF0)

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllexport) void* setmmuvar(unsigned int prm_0,void* prm_1) {
        switch (prm_0&0x7FFFFFFF) {
        case 0:
            if (prm_0 & 0x80000000) { return &memaccessfrombus; } else { memaccessfrombus = (typeofmemaccessfrombus*)prm_1; }
            break;
        case 1:
            if (prm_0 & 0x80000000) { return &m68k_set_irq; } else { m68k_set_irq = (typeofm68k_set_irq*)prm_1; }
            break;
        case 2:
            if (prm_0 & 0x80000000) { return &baseaddr4mmu; } else { baseaddr4mmu = (UINT32*)prm_1; }
            break;
        case 3:
            return &baseaddr4mmu_base;
            break;
        case 4:
            return &fc4mac;
            break;
        case 5:
            return &mmusetting;
            break;
        case 6:
            if (prm_0 & 0x80000000) { return &callback4m68kfc4ext; } else { callback4m68kfc4ext = (typeofcallback4m68kfc4ext*)prm_1; }
            break;
        }
        return nullptr;
    }
    __declspec(dllexport) void callback4m68kfc(unsigned int new_fc) { fc4mac = new_fc; }
    __declspec(dllexport) UINT32 memaccess4emu(int prm_0, int prm_1, int prm_2) {
        UINT32 memoryaddr4access = readmem32(((prm_0 >> 12) * 4) + baseaddr4mmu[((fc4mac & 4) ? 1 : 0)]);
        if (((mmusetting & 1) && ((fc4mac & 4) == 0)) || ((mmusetting&256) && (mmusetting & 1))) {
            if (memoryaddr4access & 1){
                if (fc4mac & (~((memoryaddr4access >> 1) & 3))) { setmmufaultstat(1); m68k_set_irq((mmusetting >> 1) & 7); }
                else {
                    if (((prm_2 & 3) == 0) && ((memoryaddr4access & 8) == 0)) {
                        setmmufaultstat(2);
                        m68k_set_irq((mmusetting >> 1) & 7);
                    }
                    else {
                        return memaccessfrombus((memoryaddr4access & 0xFFFFF000) | (prm_0 & 0xFFF), prm_1, prm_2);
                    }
                }
            }
            else {
                setmmufaultstat(0);
                m68k_set_irq((mmusetting >> 1) & 7);
            }
        }
        else {
             return memaccessfrombus(prm_0, prm_1, prm_2);
        }
    }
#ifdef __cplusplus
}
#endif
