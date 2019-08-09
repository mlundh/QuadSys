#ifndef QUADGS_MESSAGE_ADDRESSES_H_
#define QUADGS_MESSAGE_ADDRESSES_H_
#include <map>
#include <string>
namespace QuadGS {

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
	FC_Broadcast_e = 0x200,
	FC_Param_e = 0x201,
	FC_Log_e = 0x202,
	FC_Dbg_e = 0x203,
	FC_SerialIOrx_e = 0x204,
	FC_SerialIOtx_e = 0x205,
	FC_Ctrl_e = 0x206,
	FC_Led_e = 0x207,
	RC_SetpointGen_e = 0x208,
	FC_eventSys_e = 0x209,

}msgAddr_t;

typedef enum
{
	BC_e = 0x0,
	GS_e = 0x100,
	FC_e = 0x200,

}msgAddrDomain_t;

static const std::map<int, std::string> msgAddrStr =
{
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
	{0x200, "FC_Broadcast_e"},
	{0x201, "FC_Param_e"},
	{0x202, "FC_Log_e"},
	{0x203, "FC_Dbg_e"},
	{0x204, "FC_SerialIOrx_e"},
	{0x205, "FC_SerialIOtx_e"},
	{0x206, "FC_Ctrl_e"},
	{0x207, "FC_Led_e"},
	{0x208, "RC_SetpointGen_e"},
	{0x209, "FC_eventSys_e"},

};
}
#endif /* QUADGS_MESSAGE_ADDRESSES_H_ */
