//==============================================================================
/**
 * @file  palace_sys.c
 * @brief フィールド通信：パレス
 * @author  matsuda
 * @date  2009.05.01(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "field_comm_main.h"
#include "field_comm_func.h"
#include "field_comm_data.h"
#include "field_comm_sys.h"
#include "field/field_comm/palace_sys.h"


//==============================================================================
//  定数定義
//==============================================================================
///パレスマップの範囲(この座標外に出た場合、ワープさせる必要がある)
enum{
  PALACE_MAP_RANGE_LEFT_X = 512,
  PALACE_MAP_RANGE_RIGHT_X = 1024,

  PALACE_MAP_RANGE_LEN = 512,     ///<パレスマップのX長
};

//==============================================================================
//  構造体定義
//==============================================================================
///パレスシステムワーク
typedef struct _PALACE_SYS_WORK{
  u8 area;          ///<自機が居るエリア番号
  u8 entry_count;   ///<参加したプレイヤー数(プレイヤーが抜けても減らない)
  u8 padding[2];
}PALACE_SYS_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void PALACE_SYS_PosUpdate(PALACE_SYS_PTR palace, PLAYER_WORK *plwork, FIELD_PLAYER *fldply);


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * パレスシステムワーク確保
 * @param   heap_id		ヒープID
 * @retval  PALACE_SYS_PTR		確保したパレスワークシステムへのポインタ
 */
//==================================================================
PALACE_SYS_PTR PALACE_SYS_Alloc(HEAPID heap_id)
{
  PALACE_SYS_PTR palace;
  
  palace = GFL_HEAP_AllocClearMemory(heap_id, sizeof(PALACE_SYS_WORK));
  palace->entry_count = 1;  //自分
  return palace;
}

//==================================================================
/**
 * パレスシステムワーク解放
 *
 * @param   palace		パレスシステムワークへのポインタ
 */
//==================================================================
void PALACE_SYS_Free(PALACE_SYS_PTR palace)
{
  GFL_HEAP_FreeMemory(palace);
}

//==================================================================
/**
 * パレス監視更新処理
 *
 * @param   palace		
 */
//==================================================================
void PALACE_SYS_Update(PALACE_SYS_PTR palace, PLAYER_WORK *plwork, FIELD_PLAYER *fldply)
{
  int net_num;

  if(palace == NULL){
    return;
  }
  
  //ユーザーの増加確認
  net_num = GFL_NET_GetConnectNum();
  if(net_num > palace->entry_count){
    OS_TPrintf("palace:ユーザーが増えた old=%d, new=%d\n", palace->entry_count, net_num);
    palace->entry_count = net_num;
    //※check　親の場合、ここで増えたentry_countを一斉送信
      ;
  }
  
  //自機の位置確認し、ワープ処理
  PALACE_SYS_PosUpdate(palace, plwork, fldply);
}

//--------------------------------------------------------------
/**
 * 自機の位置確認し、ワープ処理
 *
 * @param   palace		
 * @param   plwork		
 * @param   fldply		
 */
//--------------------------------------------------------------
static void PALACE_SYS_PosUpdate(PALACE_SYS_PTR palace, PLAYER_WORK *plwork, FIELD_PLAYER *fldply)
{
  ZONEID zone_id;
  VecFx32 pos, new_pos;
  s32 offs_left, offs_right;
  int new_area;
  
  zone_id = PLAYERWORK_getZoneID(plwork);
  FIELD_PLAYER_GetPos( fldply, &pos );
  new_pos = pos;
  new_area = palace->area;
  
  offs_left = FX_Whole(pos.x) - PALACE_MAP_RANGE_LEFT_X;
  if(offs_left <= 0){
    new_pos.x = (PALACE_MAP_RANGE_RIGHT_X + offs_left) << FX32_SHIFT;
    new_area--;
  }
  else{
    offs_right = FX_Whole(pos.x) - PALACE_MAP_RANGE_RIGHT_X;
    if(offs_right > 0){
      new_pos.x = (PALACE_MAP_RANGE_LEFT_X + offs_right) << FX32_SHIFT;
      new_area++;
    }
  }

  if(palace->area != new_area){
    if(new_area < 0){
      new_area = palace->entry_count - 1;
    }
    else if(new_area >= palace->entry_count){
      new_area = 0;
    }
    OS_TPrintf("palace:ワープ old_x=%d, new_x=%d, old_area=%d, new_area=%d\n", 
      FX_Whole(pos.x), FX_Whole(new_pos.x), palace->area, new_area);
  }
  FIELD_PLAYER_SetPos( fldply, &new_pos );
  palace->area = new_area;
}

//==================================================================
/**
 * パレスのエリア番号を取得
 *
 * @param   palace		パレスシステムワークへのポインタ
 *
 * @retval  int		エリア番号
 */
//==================================================================
int PALACE_SYS_GetArea(PALACE_SYS_PTR palace)
{
  if(palace == NULL){
    return 0;
  }
  return palace->area;
}

//==================================================================
/**
 * 友達の自機座標をパレスのエリアIDを判定して修正を行う
 *
 * @param   palace		      パレスシステムへのポインタ
 * @param   friend_area	  	友達のパレスエリア
 * @param   my_plwork		    自分のPLAYER_WORK
 * @param   friend_plwork		友達のPLAYER_WORK
 */
//==================================================================
void PALACE_SYS_FriendPosConvert(PALACE_SYS_PTR palace, int friend_area, 
  PLAYER_WORK *my_plwork, PLAYER_WORK *friend_plwork)//, FIELD_PLAYER *friend_fldply)
{
  int left_area, right_area;
  VecFx32 pos;
  
  if(palace == NULL || palace->area == friend_area 
      || PLAYERWORK_getZoneID(my_plwork) != PLAYERWORK_getZoneID(friend_plwork)){
    return;
  }
  
  left_area = palace->area - 1;
  right_area = palace->area + 1;
  if(left_area < 0){
    left_area = palace->entry_count - 1;
  }
  if(right_area >= palace->entry_count){
    right_area = 0;
  }
  
//  FIELD_PLAYER_GetPos( friend_fldply, &pos );
  pos = *(PLAYERWORK_getPosition(friend_plwork));
  if(friend_area == right_area){  //右隣のエリアにいる場合
    pos.x += PALACE_MAP_RANGE_LEN << FX32_SHIFT;
  }
  else if(friend_area == left_area){ //左隣のエリアにいる場合
    pos.x -= PALACE_MAP_RANGE_LEN << FX32_SHIFT;
  }
  else{ //両隣のエリアではない場合
    pos.x += (PALACE_MAP_RANGE_LEN * 2) << FX32_SHIFT;
  }
//  FIELD_PLAYER_SetPos( friend_fldply, &pos );
  PLAYERWORK_setPosition(friend_plwork, &pos);
}

