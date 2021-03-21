#include "../inc/msgAddr.h"

addressMap_t enumStringMap[] = {
	{0x0, "Broadcast_e"},
	{0x1, "Unassigned_e"},
	{0x100, "GS_Broadcast_e"},
	{0x101, "GS_Dbg_e"},
	{0x102, "GS_GUI_e"},
	{0x103, "GS_Param_e"},
	{0x104, "GS_Log_e"},
	{0x105, "GS_SerialIO_e"},
	{0x106, "GS_SetpointGen_e"},
	{0x107, "GS_Router_e"},
	{0x108, "GS_AppLog_e"},
	{0x200, "FC_Broadcast_e"},
	{0x201, "FC_Param_e"},
	{0x202, "FC_Log_e"},
	{0x203, "FC_Dbg_e"},
	{0x204, "FC_SerialIOrx_e"},
	{0x205, "FC_SerialIOtx_e"},
	{0x206, "FC_Ctrl_e"},
	{0x207, "FC_Led_e"},
	{0x208, "RC_SetpointGen_e"},
	{0x209, "RC_SetpointGen2_e"},
	{0x20a, "FC_eventSys_e"},

};

char* getStrFromAddr(int address)
{
	for(int i = 0; i < NR_ADDRESSES; i++)
	{
		if(address == enumStringMap[i].key)
		{
			return enumStringMap[i].string;
		}
	}
    return enumStringMap[0].string;
}