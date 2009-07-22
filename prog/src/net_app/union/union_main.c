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
#include "union_subproc.h"


void Union_Main(GAME_COMM_SYS_PTR game_comm, FIELD_MAIN_WORK *fieldmap)
{
  UNION_SYSTEM_PTR unisys;
  
  if(GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_UNION){
    return;
  }
  
  unisys = GameCommSys_GetAppWork(game_comm);
  GF_ASSERT(unisys != NULL);
  
  //PROCがフィールドの時のみ実行する処理
  if(Union_FieldCheck(unisys) == TRUE){
    switch(unisys->my_situation.play_category){
    case UNION_PLAY_CATEGORY_UNION:
    case UNION_PLAY_CATEGORY_TALK:
      if(UnionSubProc_IsExits(unisys) == FALSE){  //サブPROCが無い時のみ実行
        //データ受信によるイベント起動
        UnionReceive_BeaconInterpret(unisys);
        
        //キー操作によるイベント起動
        UnionOneself_Update(unisys, fieldmap);
        
        //OBJ反映
        UNION_CHAR_Update(unisys, unisys->uniparent->game_data);
        
        //下画面反映
      }
      break;
    case UNION_PLAY_CATEGORY_COLOSSEUM:
      if(UnionSubProc_IsExits(unisys) == FALSE){  //サブPROCが無い時のみ実行
        //キー操作によるイベント起動
        UnionOneself_Update(unisys, fieldmap);
      }
      break;
    }
  }
}

//==================================================================
/**
 * PROCがフィールドの状態か調べる
 *
 * @param   unisys		
 *
 * @retval  BOOL		TRUE:フィールドである
 */
//==================================================================
BOOL Union_FieldCheck(UNION_SYSTEM_PTR unisys)
{
  if(GAMESYSTEM_CheckFieldMapWork(unisys->uniparent->gsys) == TRUE){
    if(FIELDMAP_IsReady(GAMESYSTEM_GetFieldMapWork(unisys->uniparent->gsys)) == TRUE){
      return TRUE;
    }
  }
  return FALSE;
}

//==================================================================
/**
 * フィールド削除時に行う処理
 *
 * @param   unisys		
 */
//==================================================================
void Union_FieldDelete(UNION_SYSTEM_PTR unisys)
{
}

//==================================================================
/**
 * 他アプリからフィールドへ復帰してきた時の処理
 *
 * @param   unisys		
 */
//==================================================================
void Union_FieldComeback(UNION_SYSTEM_PTR unisys)
{
}
