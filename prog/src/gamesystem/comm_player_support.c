//==============================================================================
/**
 * @file    comm_player_support.c
 * @brief   通信プレイヤーからのサポート
 * @author  matsuda
 * @date    2009.12.10(木)
 */
//==============================================================================
#include <gflib.h>
#include "gamesystem/comm_player_support.h"
#include "savedata/mystatus_local.h"


//==============================================================================
//  構造体定義
//==============================================================================
///通信相手からのサポート
struct _COMM_PLAYER_SUPPORT{
  SUPPORT_TYPE type;            ///<サポートの種類
  MYSTATUS mystatus;            ///<サポートしてくれた通信相手のMYSTATUS
};


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * サポートデータAlloc
 *
 * @param   cps		
 */
//==================================================================
COMM_PLAYER_SUPPORT * COMM_PLAYER_SUPPORT_Alloc(HEAPID heap_id)
{
  COMM_PLAYER_SUPPORT *cps;
  
  cps = GFL_HEAP_AllocClearMemory(heap_id, sizeof(COMM_PLAYER_SUPPORT));
  COMM_PLAYER_SUPPORT_Init(cps);
  return cps;
}

//==================================================================
/**
 * サポートデータFreeMemory
 *
 * @param   cps		
 */
//==================================================================
void COMM_PLAYER_SUPPORT_Free(COMM_PLAYER_SUPPORT *cps)
{
  GFL_HEAP_FreeMemory(cps);
}

//==================================================================
/**
 * サポートデータ初期化
 *
 * @param   cps		
 */
//==================================================================
void COMM_PLAYER_SUPPORT_Init(COMM_PLAYER_SUPPORT *cps)
{
  GFL_STD_MemClear(cps, sizeof(COMM_PLAYER_SUPPORT));
  cps->type = SUPPORT_TYPE_NULL;
  MyStatus_Init(&cps->mystatus);
}

//==================================================================
/**
 * サポートパラメータをセットする
 *
 * @param   dest_cps		
 * @param   type		    
 * @param   mystatus		
 */
//==================================================================
void COMM_PLAYER_SUPPORT_SetParam(COMM_PLAYER_SUPPORT *dest_cps, SUPPORT_TYPE type, const MYSTATUS *mystatus)
{
  dest_cps->type = type;
  dest_cps->mystatus = *mystatus;
}

//==================================================================
/**
 * サポートデータからMYSTATUSを取得
 *
 * @param   cps		
 *
 * @retval  const MYSTATUS *		
 */
//==================================================================
const MYSTATUS * COMM_PLAYER_SUPPORT_GetMyStatus(const COMM_PLAYER_SUPPORT *cps)
{
  return &cps->mystatus;
}

//==================================================================
/**
 * サポートデータからサポートタイプを取得
 *
 * @param   cps		
 *
 * @retval  SUPPORT_TYPE		
 */
//==================================================================
SUPPORT_TYPE COMM_PLAYER_SUPPORT_GetSupportType(const COMM_PLAYER_SUPPORT *cps)
{
  return cps->type;
}

