//============================================================================================
/**
 * @file	debug_ohno.h
 * @brief	デバッグ用関数定義
 * @author	ohno
 * @date	2006.11.29
 */
//============================================================================================


#ifndef __DEBUG_OHNO_H__
#define __DEBUG_OHNO_H__

#include "net/network_define.h"

typedef BOOL (*NetTestFunc)(void* pCtl);


//------------------------------------------------------------------
typedef struct {
  u32 debug_heap_id;
  GFL_PROCSYS * psys;
  NetTestFunc funcNet;
  int bMoveRecv;
  BOOL bParent;
}DEBUG_OHNO_CONTROL;


enum NetDebugOhnoCommand_e {
  NET_CMD_MOVE = GFL_NET_CMD_DEBUG_OHNO,
  NET_CMD_TALK,
  NET_CMD_FRIENDCODE_DATA,
};


#define _MAXNUM   (4)         // 最大接続人数
#define _MAXSIZE  (80)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数

extern void DebugOhnoInit(HEAPID heap_id);
extern void DebugOhnoMain(void);
extern void DebugOhnoExit(void);


#endif //__DEBUG_OHNO_H__
