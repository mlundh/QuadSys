#ifndef QUADFC_MESSAGE_ADDRESSES_H_
#define QUADFC_MESSAGE_ADDRESSES_H_
#include <stdint.h>
#define REGION_MASK (0xFF00)
typedef enum
{
	Broadcast_e = 0x0,
	Unassigned_e = 0x1,
	GS_Broadcast_e = 0x100,
	GS_Dbg_e = 0x101,
	GS_GUI_e = 0x102,
	GS_Param_e = 0x103,
	GS_Log_e = 0x104,
	GS_SerialIO_e = 0x105,
	GS_SetpointGen_e = 0x106,
	GS_Router_e = 0x107,
	GS_AppLog_e = 0x108,
	FC_Broadcast_e = 0x200,
	FC_Param_e = 0x201,
	FC_Log_e = 0x202,
	FC_Dbg_e = 0x203,
	FC_SerialIOrx_e = 0x204,
	FC_SerialIOtx_e = 0x205,
	FC_Ctrl_e = 0x206,
	FC_Led_e = 0x207,
	RC_SetpointGen_e = 0x208,
	RC_SetpointGen2_e = 0x209,
	FC_eventSys_e = 0x20a,

}msgAddr_t;


typedef enum
{
	BC_e = 0x0,
	GS_e = 0x100,
	FC_e = 0x200,

}msgAddrDomain_t;


typedef struct addressMap {
    int key;
    char string[40];
} addressMap_t;

#define NR_ADDRESSES (22)

addressMap_t enumStringMap[22];

char* getStrFromAddr(int);


#endif /* QUADFC_MESSAGE_ADDRESSES_H_ */
