//======================================================================
/**
 * @file	plist_comm.c
 * @brief	ポケモンリスト通信(バトル同期処理
 * @author	ariizumi
 * @data	10/03/10
 *
 * モジュール名：PLIST_COMM
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "net/network_define.h"

#include "plist_sys.h"
#include "plist_comm.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
enum
{
  PCS_INIT_TIMMING,
  PCS_ADD_COMMAND,
  PCS_UPDATE,
  PCS_TERM_TIMMING,
  PCS_RELEASE_COMMAND,
  PCS_FINISH,
}PLIST_COMM_STATE;


//通信の送信タイプ
typedef enum
{
  PCST_SEND_FLG = GFL_NET_CMD_POKELIST,

  PCST_MAX,
}PLIST_COMM_SEND_TYPE;
//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void PLIST_COMM_PostFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );

static const NetRecvFuncTable PLIST_COMM_RecvTable[] = 
{
  { PLIST_COMM_PostFlg   ,NULL  },
};

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
void PLIST_COMM_InitComm( PLIST_WORK *work )
{
  work->isComm = TRUE;
  work->commState = PCS_INIT_TIMMING;
}

void PLIST_COMM_ReqExitComm( PLIST_WORK *work )
{
  if( work->isComm == TRUE )
  {
    work->commState = PCS_TERM_TIMMING;
  }
}

const BOOL PLIST_COMM_IsExitComm( PLIST_WORK *work )
{
  if( work->isComm == TRUE )
  {
    if( work->commState == PCS_FINISH )
    {
      return TRUE;
    }
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

void PLIST_COMM_UpdateComm( PLIST_WORK *work )
{
  if( work->isComm == TRUE )
  {
    switch( work->commState )
    {
    case PCS_INIT_TIMMING:
      {
        GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
        GFL_NET_HANDLE_TimeSyncStart( selfHandle , PLIST_COMM_TIMMIN_INIT_LIST , WB_NET_POKELIST );
        work->commState = PCS_ADD_COMMAND;
      }
      break;

    case PCS_ADD_COMMAND:
      {
        GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
        if( GFL_NET_HANDLE_IsTimeSync( selfHandle , PLIST_COMM_TIMMIN_INIT_LIST , WB_NET_POKELIST ) == TRUE )
        {
          GFL_NET_AddCommandTable( GFL_NET_CMD_POKELIST , PLIST_COMM_RecvTable , NELEMS(PLIST_COMM_RecvTable) , work );
          work->commState = PCS_UPDATE;
        }
      }
      break;

    case PCS_UPDATE:
      break;

    case PCS_TERM_TIMMING:
      {
        GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
        GFL_NET_HANDLE_TimeSyncStart( selfHandle , PLIST_COMM_TIMMIN_EXIT_LIST , WB_NET_POKELIST );
        work->commState = PCS_RELEASE_COMMAND;
      }
      break;

    case PCS_RELEASE_COMMAND:
      {
        GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
        if( GFL_NET_HANDLE_IsTimeSync( selfHandle , PLIST_COMM_TIMMIN_EXIT_LIST , WB_NET_POKELIST ) == TRUE )
        {
          GFL_NET_DelCommandTable( GFL_NET_CMD_POKELIST );
          work->commState = PCS_FINISH;
        }
      }
      break;
    case PCS_FINISH:

      break;
    }
  }
}


typedef struct
{
  u16 value;
  u8  flg;
}PLIST_COMM_FLG_PACKET;

const BOOL PLIST_COMM_SendFlg( PLIST_WORK *work , const u8 flgType , const u16 flgValue )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  PLIST_COMM_FLG_PACKET pkt;
  pkt.flg = flgType;
  pkt.value = flgValue;
  
  if( work->commState != PCS_UPDATE )
  {
    OS_TPrintf("PLIST_COMM net is not initialize!!!\n");
    return FALSE;
  }
  
  OS_TPrintf("Send Flg[%d:%d].\n",pkt.flg,pkt.value);
  
  {
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , GFL_NET_SENDID_ALLUSER , 
      PCST_SEND_FLG , sizeof( PLIST_COMM_FLG_PACKET ) , 
      (void*)&pkt , TRUE , FALSE , FALSE );
    
    if( ret == FALSE )
    {
      OS_TPrintf("Send Flg failue!\n");
    }
    return ret;
  }
}
//--------------------------------------------------------------
// フラグ受信
//--------------------------------------------------------------
static void PLIST_COMM_PostFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  PLIST_WORK *work = (PLIST_WORK*)pWork;
  PLIST_COMM_FLG_PACKET *pkt = (PLIST_COMM_FLG_PACKET*)pData;
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  
  OS_TPrintf("Post Flg [%d][%d:%d].\n",netID,pkt->flg,pkt->value);

  switch( pkt->flg )
  {
  case PCFT_FINISH_SELECT:
    work->plData->comm_selected_num++;
    break;
  }
}
