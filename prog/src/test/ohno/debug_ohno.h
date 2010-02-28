//============================================================================================
/**
 * @file	debug_ohno.h
 * @brief	デバッグ用関数定義
 * @author	ohno
 * @date	2006.11.29
 */
//============================================================================================


#pragma once

#include "net/network_define.h"


//------------------------------------------------------------------
typedef struct _DEBUG_OHNO_CONTROL DEBUG_OHNO_CONTROL;


enum NetDebugOhnoCommand_e {
  NET_CMD_MOVE = GFL_NET_CMD_DEBUG_OHNO,
  NET_CMD_TALK,
  NET_CMD_FRIENDCODE_DATA,
};



extern void DebugOhnoInit(HEAPID heap_id);
extern void DebugOhnoMain(void);
extern void DebugOhnoExit(void);

extern const GFL_PROC_DATA NetDeliverySendProcData;
extern const GFL_PROC_DATA NetDeliveryRecvProcData;
extern const GFL_PROC_DATA NetDeliveryIRCSendProcData;
extern const GFL_PROC_DATA NetDeliveryIRCRecvProcData;
extern const GFL_PROC_DATA NetDeliveryTriSendProcData;

