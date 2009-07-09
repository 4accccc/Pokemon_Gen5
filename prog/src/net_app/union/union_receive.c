//==============================================================================
/**
 * @file    ユニオン：受信データ解釈
 * @brief   簡単な説明を書く
 * @author  matsuda
 * @date    2009.07.07(火)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "union_types.h"
#include "savedata/mystatus.h"
#include "net_app/union/union_main.h"
#include "union_local.h"
#include "union_receive.h"
#include "union_chara.h"


//==================================================================
/**
 * 受信ビーコンデータ解釈
 *
 * @param   unisys		
 */
//==================================================================
void UnionReceive_BeaconInterpret(UNION_SYSTEM_PTR unisys)
{
  int i;
  UNION_BEACON_PC *bpc;
  BOOL req_ret;
  
  bpc = unisys->receive_beacon;
  for(i = 0; i < UNION_RECEIVE_BEACON_MAX; i++){
    if(bpc->beacon.data_valid == UNION_BEACON_VALID){
      if(bpc->new_data == TRUE){
        //新規キャラ登録
        UNION_CHAR_AddOBJ(unisys, unisys->uniparent->game_data, bpc->beacon.trainer_view, i);
        req_ret = UNION_CHAR_EventReq(bpc, BPC_EVENT_STATUS_ENTER);
        GF_ASSERT_MSG(req_ret == TRUE, "add union req_faile!");
        bpc->new_data = FALSE;
      }
    }
    bpc++;
  }
}

