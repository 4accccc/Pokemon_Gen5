//==============================================================================
/**
 * @file    union_main.c
 * @brief   ユニオンルーム：常駐物
 * @author  matsuda
 * @date    2009.07.02(木)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "union_types.h"
#include "savedata/mystatus.h"
#include "net_app/union/union_main.h"
#include "union_local.h"
#include "union_receive.h"
#include "union_chara.h"
#include "union_oneself.h"


void Union_Main(GAME_COMM_SYS_PTR game_comm, FIELD_MAIN_WORK *fieldmap)
{
  UNION_SYSTEM_PTR unisys;
  
  if(GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_UNION){
    return;
  }
  
  unisys = GameCommSys_GetAppWork(game_comm);
  GF_ASSERT(unisys != NULL);
  
  //データ受信によるイベント起動
  UnionReceive_BeaconInterpret(unisys);
  
  //キー操作によるイベント起動
  UnionOneself_Update(unisys, fieldmap);
  
  //OBJ反映
  UNION_CHAR_Update(unisys, unisys->uniparent->game_data);
  
  //下画面反映
}

