//==============================================================================
/**
 * @file    union_tool.c
 * @brief   ユニオンルームで使用するツール類：オーバーレイに配置
 * @author  matsuda
 * @date    2009.07.25(土)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "field\field_player.h"
#include "field\fieldmap.h"
#include "union_types.h"
#include "union_tool.h"

//==============================================================================
//  構造体定義
//==============================================================================
///s16のポイント型
typedef struct{
  s16 x;
  s16 z;
}POINT_S16;

//==============================================================================
//  データ
//==============================================================================
//--------------------------------------------------------------
//  出口座標
//--------------------------------------------------------------
///1vs1の時の立ち位置座標(グリッド座標)
static const POINT_S16 UnionWayOutPos[] = {
  {11, 16},
};


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ユニオンの出口座標にいるかチェック
 *
 * @param   fieldWork		
 *
 * @retval  BOOL		TRUE:出口座標にいる
 */
//==================================================================
BOOL UnionTool_CheckWayOut(FIELDMAP_WORK *fieldWork)
{
  VecFx32 pos;
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldWork);
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork(fieldWork);
  PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork(gsys);
  int i;
  
  FIELD_PLAYER_GetPos(player, &pos);
  pos.x = SIZE_GRID_FX32(pos.x);
  pos.z = SIZE_GRID_FX32(pos.z);

  if(PLAYERWORK_getDirection( plWork ) == DIR_UP){
//    return FALSE;
  }
  
  for(i = 0; i < NELEMS(UnionWayOutPos); i++){
    if(pos.x == UnionWayOutPos[i].x && pos.z == UnionWayOutPos[i].z){
      return TRUE;
    }
  }
  return FALSE;
}

//==================================================================
/**
 * play_categoryをアピール番号に変換
 *
 * @param   situ		
 *
 * @retval  UNION_APPEAL		
 */
//==================================================================
UNION_APPEAL UnionTool_PlayCategory_to_AppealNo(UNION_PLAY_CATEGORY play_category)
{
  //遊んでいるゲームがあるなら、そちらを優先してアピールさせる
  if(play_category >= UNION_PLAY_CATEGORY_COLOSSEUM_START && 
      play_category <= UNION_PLAY_CATEGORY_COLOSSEUM_END){
    return UNION_APPEAL_BATTLE;
  }
  else{
    switch(play_category){
    case UNION_PLAY_CATEGORY_PICTURE:        //お絵かき
      return UNION_APPEAL_PICTURE;
    case UNION_PLAY_CATEGORY_TRADE:          //ポケモン交換
      return UNION_APPEAL_TRADE;
    case UNION_PLAY_CATEGORY_GURUGURU:       //ぐるぐる交換
      return UNION_APPEAL_GURUGURU;
    case UNION_PLAY_CATEGORY_RECORD:         //レコードコーナー
      return UNION_APPEAL_RECORD;
    }
  }
  
  return UNION_APPEAL_NULL;
}
