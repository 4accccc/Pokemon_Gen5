//==============================================================================
/**
 * @file    comm_player.c
 * @brief   通信プレイヤー制御
 * @author  matsuda
 * @date    2009.07.17(金)
 */
//==============================================================================
#include <gflib.h>
#include "field/field_comm_actor.h"
#include "net_app/union/comm_player.h"
#include "field/fieldmap.h"


//==============================================================================
//  定数定義
//==============================================================================
///通信プレイヤーワーク最大管理数
#define COMM_PLAYER_MAX     (4)   //コロシアムが最大4人の為。net_idが穴抜けでも対応させるため自分分を引いてはいけない

//==============================================================================
//  構造体定義
//==============================================================================
///通信プレイヤーワーク
typedef struct{
  VecFx32 pos;
  BOOL vanish;
  u16 dir;
  u8 occ;         ///<この構造体全体のデータ有効・無効(TRUE:有効)
  u8 padding;
}COMM_PLAYER;

///自分自身の座標データなど
typedef struct{
  VecFx32 pos;
  u16 dir;
  u8 padding[2];
}MINE_PLAYER;

///通信プレイヤー制御システムワーク
typedef struct _COMM_PLAYER_SYS{
  FIELD_MAIN_WORK *fieldWork;
  FIELD_COMM_ACTOR_CTRL *act_ctrl;
  COMM_PLAYER act[COMM_PLAYER_MAX];
  MINE_PLAYER mine;     ///<自分自身の座標データなど
  u8 max;
  u8 padding[3];
}COMM_PLAYER_SYS;


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 通信プレイヤー制御システム初期化
 *
 * @param   max		
 * @param   fmmdlsys		
 * @param   heap_id		
 *
 * @retval  COMM_PLAYER_SYS_PTR		
 */
//==================================================================
COMM_PLAYER_SYS_PTR CommPlayer_Init(int max, FIELD_MAIN_WORK *fieldWork, HEAPID heap_id)
{
  COMM_PLAYER_SYS_PTR cps;
  MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys(fieldWork);

  OS_TPrintf("通信プレイヤー制御システムの生成\n");
  
  cps = GFL_HEAP_AllocClearMemory(heap_id, sizeof(COMM_PLAYER_SYS));
  
  GF_ASSERT(max <= COMM_PLAYER_MAX);
  
  cps->max = max;
  cps->fieldWork = fieldWork;
  cps->act_ctrl = FIELD_COMM_ACTOR_CTRL_Create(max, fldMdlSys, heap_id);
  
  return cps;
}

//==================================================================
/**
 * 通信プレイヤー制御システム破棄
 *
 * @param   cps		
 */
//==================================================================
void CommPlayer_Exit(COMM_PLAYER_SYS_PTR cps)
{
  int i;
  
  OS_TPrintf("通信プレイヤー制御システムの破棄\n");

  for(i = 0; i < COMM_PLAYER_MAX; i++){
    CommPlayer_Del(cps, i);
  }
  FIELD_COMM_ACTOR_CTRL_Delete(cps->act_ctrl);
  
  GFL_HEAP_FreeMemory(cps);
}

//==================================================================
/**
 * 通信プレイヤーアクター登録
 *
 * @param   cps		
 * @param   index		管理Index(基本的にnet_idを渡せばよい。
 *                  穴抜け参加に対応しているのでmaxが2でも0,3といった指定が可能)
 * @param   sex		  性別
 * @param   pack    座標データパッケージ
 */
//==================================================================
void CommPlayer_Add(COMM_PLAYER_SYS_PTR cps, int index, int sex, const COMM_PLAYER_PACKAGE *pack)
{
  int objcode;

  OS_TPrintf("通信プレイヤーAdd index=%d\n", index);

#ifdef PM_DEBUG
  {//最大人数を超えて追加しようとしていないかチェック
    int i, count;
    count = 0;
    for(i = 0; i < COMM_PLAYER_MAX; i++){
      if(cps->act[i].occ == TRUE){
        count++;
      }
    }
    if(count > cps->max){
      GF_ASSERT(0);   //最大人数を超えて追加しようとした
      return;
    }
  }
#endif

  GF_ASSERT(cps->act[index].occ == FALSE);
  if(cps->act[index].occ == TRUE){
    return; //一応
  }

  objcode = (sex == PM_MALE) ? HERO : HEROINE;
  cps->act[index].pos = pack->pos;
  cps->act[index].dir = pack->dir;
  cps->act[index].vanish = pack->vanish;
  cps->act[index].occ = TRUE;
  
  FIELD_COMM_ACTOR_CTRL_AddActor(cps->act_ctrl, index, objcode, &cps->act[index].dir, 
    &cps->act[index].pos, &cps->act[index].vanish);
}

//==================================================================
/**
 * 通信プレイヤーアクター削除
 *
 * @param   cps		
 * @param   index		管理Index
 */
//==================================================================
void CommPlayer_Del(COMM_PLAYER_SYS_PTR cps, int index)
{
  if(cps->act[index].occ == FALSE){
    return;
  }
  
  OS_TPrintf("通信プレイヤーDel index=%d\n", index);
  FIELD_COMM_ACTOR_CTRL_DeleteActro(cps->act_ctrl, index);
  cps->act[index].occ = FALSE;
}

//==================================================================
/**
 * 通信プレイヤー存在チェック
 *
 * @param   cps		
 * @param   index		管理Index
 *
 * @retval  BOOL		TRUE:存在している。　FALSE:非存在
 */
//==================================================================
BOOL CommPlayer_CheckOcc(COMM_PLAYER_SYS_PTR cps, int index)
{
  GF_ASSERT(cps != NULL && index < COMM_PLAYER_MAX);
  return cps->act[index].occ;
}

//==================================================================
/**
 * 通信プレイヤーアクター座標をセットする
 *
 * @param   cps		  
 * @param   index		管理Index
 * @param   pack		座標パッケージ
 */
//==================================================================
void CommPlayer_SetParam(COMM_PLAYER_SYS_PTR cps, int index, const COMM_PLAYER_PACKAGE *pack)
{
  GF_ASSERT(cps->act[index].occ == TRUE);

  cps->act[index].pos = pack->pos;
  cps->act[index].dir = pack->dir;
  cps->act[index].vanish = pack->vanish;
}

//==================================================================
/**
 * 自分座標更新処理
 *
 * @param   cps		  
 * @param   pack		自分座標データ代入先(通信で座標をやり取りする場合、このデータを送ってください)
 *
 * @retval  BOOL		TRUE:更新あり。　FALSE:以前と値が変わっていない
 *
 * packには座標の更新の有無に関わらず常に今の値が代入されます
 * 初回のデータ送信といった、データ更新に限らず送りたい場合でもちゃんとした値が入ります
 */
//==================================================================
BOOL CommPlayer_Mine_DataUpdate(COMM_PLAYER_SYS_PTR cps, COMM_PLAYER_PACKAGE *pack)
{
  MINE_PLAYER *mine = &cps->mine;
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(cps->fieldWork);
  VecFx32 pos;
  u16 dir;
  
  FIELD_PLAYER_GetPos(player, &pos);
  dir = FIELD_PLAYER_GetDir(player);

  pack->dir = dir;
  pack->pos = pos;
  pack->vanish = FALSE;

  if(dir != mine->dir || GFL_STD_MemComp(&pos, &mine->pos, sizeof(VecFx32)) != 0){
    mine->dir = dir;
    mine->pos = pos;
    return TRUE;
  }
  return FALSE;
}

