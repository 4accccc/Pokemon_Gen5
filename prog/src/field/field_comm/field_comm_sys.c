//==============================================================================
/**
 * @file  field_comm_sys.c
 * @brief
 * @author  matsuda
 * @date  2009.04.30(木)
 */
//==============================================================================
#include <gflib.h>
#include "field/field_comm/field_comm_sys.h"
#include "field_comm_func.h"
#include "field_comm_data.h"


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct _COMM_FIELD_SYS{
  FIELD_COMM_FUNC *commFunc_;
  FIELD_COMM_DATA *commData_;
  
  GAME_COMM_SYS_PTR game_comm;
  
  BOOL comm_act_vanish[FIELD_COMM_MEMBER_MAX];   ///<TRUE:非表示
  u8 invalid_netid;           ///<侵入先ROMのnet_id
  u8 exit_recv;               ///<TRUE:終了フラグ
  u8 recv_profile;            ///<プロフィール受信フラグ(bit管理)
  u8 padding;
}COMM_FIELD_SYS;



//==================================================================
/**
 * フィールド通信監視ワークを生成する(通信開始時に生成すればよい)
 *
 * @param   commHeapID
 *
 * @retval  COMM_FIELD_SYS_PTR    生成したフィールド通信監視ワークへのポインタ
 */
//==================================================================
COMM_FIELD_SYS_PTR FIELD_COMM_SYS_Alloc(HEAPID commHeapID, GAME_COMM_SYS_PTR game_comm)
{
  COMM_FIELD_SYS_PTR comm_field;

  comm_field = GFL_HEAP_AllocClearMemory(commHeapID, sizeof(COMM_FIELD_SYS));
  comm_field->commFunc_ = FIELD_COMM_FUNC_InitSystem( commHeapID );
  comm_field->commData_ = FIELD_COMM_DATA_InitSystem( commHeapID );
  comm_field->game_comm = game_comm;
  return comm_field;
}

//==================================================================
/**
 * フィールド通信監視ワークを破棄(通信終了時に破棄)
 *
 * @param   comm_field   フィールド通信監視ワークへのポインタ
 */
//==================================================================
void FIELD_COMM_SYS_Free(COMM_FIELD_SYS_PTR comm_field)
{
  FIELD_COMM_DATA_TermSystem(comm_field->commData_);
  FIELD_COMM_FUNC_TermSystem(comm_field->commFunc_);
  GFL_HEAP_FreeMemory(comm_field);
}

FIELD_COMM_FUNC* FIELD_COMM_SYS_GetCommFuncWork( COMM_FIELD_SYS_PTR commField )
{
  return commField->commFunc_;
}

FIELD_COMM_DATA* FIELD_COMM_SYS_GetCommDataWork( COMM_FIELD_SYS_PTR commField )
{
  return commField->commData_;
}

GAME_COMM_SYS_PTR FIELD_COMM_SYS_GetGameCommSys( COMM_FIELD_SYS_PTR commField )
{
  return commField->game_comm;
}

//==================================================================
/**
 * アクター表示・非表示フラグへのポインタを取得
 *
 * @param   palace		
 * @param   net_id		
 *
 * @retval  BOOL *		
 */
//==================================================================
BOOL * FIELD_COMM_SYS_GetCommActorVanishFlag(COMM_FIELD_SYS_PTR commField, int net_id)
{
  return &commField->comm_act_vanish[net_id];
}

//==================================================================
/**
 * 侵入先ROMのネットIDをセットする
 *
 * @param   commField		    
 * @param   invalid_netid		侵入先ROMのネットID
 */
//==================================================================
void FIELD_COMM_SYS_SetInvalidNetID(COMM_FIELD_SYS_PTR commField, int invalid_netid)
{
  commField->invalid_netid = invalid_netid;
}

//==================================================================
/**
 * 侵入先ROMのネットIDを取得する
 *
 * @param   commField		
 *
 * @retval  int		侵入先ROMのネットID
 */
//==================================================================
int FIELD_COMM_SYS_GetInvalidNetID(COMM_FIELD_SYS_PTR commField)
{
  return commField->invalid_netid;
}

//==================================================================
/**
 * 終了フラグをセットする
 *
 * @param   commField		
 */
//==================================================================
void FIELD_COMM_SYS_SetExitReq(COMM_FIELD_SYS_PTR commField)
{
  commField->exit_recv = TRUE;
}

//==================================================================
/**
 * 終了フラグを取得する
 *
 * @param   commField		
 *
 * @retval  BOOL		TRUE:終了フラグが立っている
 */
//==================================================================
BOOL FIELD_COMM_SYS_GetExitReq(COMM_FIELD_SYS_PTR commField)
{
  return commField->exit_recv;
}

//==================================================================
/**
 * プロフィール受信フラグをセットする
 *
 * @param   commField		
 * @param   net_id		
 */
//==================================================================
void FIELD_COMM_SYS_SetRecvProfile(COMM_FIELD_SYS_PTR commField, int net_id)
{
  commField->recv_profile |= 1 << net_id;
}

//==================================================================
/**
 * プロフィール受信フラグを取得する
 *
 * @param   commField		
 * @param   net_id		
 *
 * @retval  BOOL		TRUE:受信している
 */
//==================================================================
BOOL FIELD_COMM_SYS_GetRecvProfile(COMM_FIELD_SYS_PTR commField, int net_id)
{
  if(commField->recv_profile & (1 << net_id)){
    return TRUE;
  }
  return FALSE;
}
