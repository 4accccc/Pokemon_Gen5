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
#include "field/fldeff_gyoe.h"
#include "field/fieldmap_call.h"  //FIELDMAP_IsReady


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
  //フィールドアクターから常に参照され続けるワーク　※その為、ビットフィールド禁止
  VecFx32 pos;
  BOOL vanish;
  u16 dir;
  u16 obj_code;
  
  //comm_player.cでのみ使用するもの
  u8 occ:1;         ///<この構造体全体のデータ有効・無効(TRUE:有効)
  u8 push_flag:1;
  u8 player_form:4; ///<PLAYER_MOVE_FORM
  u8 :2;
  u8 padding[3];
  
  FLDEFF_TASK *fldeff_gyoe_task;  ///<「！」エフェクト動作タスク
}COMM_PLAYER;

///自分自身の座標データなど
typedef struct{
  VecFx32 pos;
  u16 dir;
  u8 player_form:4; ///<PLAYER_MOVE_FORM
  u8 vanish:1;
  u8  :5;
  u8 padding;
}MINE_PLAYER;

///通信プレイヤー制御システムワーク
typedef struct _COMM_PLAYER_SYS{
  GAMESYS_WORK *gsys;
  FIELD_COMM_ACTOR_CTRL *act_ctrl;
  COMM_PLAYER act[COMM_PLAYER_MAX];
  MINE_PLAYER mine;     ///<自分自身の座標データなど
  HEAPID heap_id;
  u8 max;
  u8 update_stop;       ///<TRUE:座標更新を行わない
  u8 dash_flag;
  u8 padding;
}COMM_PLAYER_SYS;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _FieldCommActorSys_CheckCreate(GAMESYS_WORK *gsys, COMM_PLAYER_SYS_PTR cps);



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
COMM_PLAYER_SYS_PTR CommPlayer_Init(int max, GAMESYS_WORK *gsys, HEAPID heap_id, BOOL dash_flag)
{
  COMM_PLAYER_SYS_PTR cps;

  OS_TPrintf("通信プレイヤー制御システムの生成\n");
  
  cps = GFL_HEAP_AllocClearMemory(heap_id, sizeof(COMM_PLAYER_SYS));
  
  GF_ASSERT(max <= COMM_PLAYER_MAX);
  
  cps->max = max;
  cps->gsys = gsys;
  cps->heap_id = heap_id;
  cps->dash_flag = dash_flag;

  return cps;
}

//==================================================================
/**
 * 通信プレイヤー制御システム破棄
 *
 * @param   cps		
 */
//==================================================================
void CommPlayer_Exit(GAMESYS_WORK *gsys, COMM_PLAYER_SYS_PTR cps)
{
  int i;
  
  OS_TPrintf("通信プレイヤー制御システムの破棄\n");

  if(cps->act_ctrl != NULL){  //NULLの場合はPUSH状態の為、既に削除されている
    for(i = 0; i < COMM_PLAYER_MAX; i++){
      CommPlayer_Del(cps, i);
    }
    
    if(GAMESYSTEM_CheckFieldMapWork(gsys) == TRUE){ //念のためフィールド存在チェック
      FIELD_COMM_ACTOR_CTRL_Delete(cps->act_ctrl);
    }
    else{
      GF_ASSERT(0); //フィールドOverlayが存在しない為、FIELD_COMM_ACTOR_CTRL_Deleteが呼び出せない
    }
  }
  
  GFL_HEAP_FreeMemory(cps);
}

//==================================================================
/**
 * 通信プレイヤー制御システム更新処理
 *
 * @param   cps		
 */
//==================================================================
void CommPlayer_Update(GAMESYS_WORK *gsys, COMM_PLAYER_SYS_PTR cps)
{
  int index;
  
  _FieldCommActorSys_CheckCreate(gsys, cps);
  
  //フィールドエフェクトを実行しているものがあれば、終了状態を監視し、ワークの初期化等を行う
  if(GAMESYSTEM_CheckFieldMapWork(cps->gsys) == TRUE){
    for(index = 0; index < COMM_PLAYER_MAX; index++){
      if(cps->act[index].occ == TRUE && cps->act[index].fldeff_gyoe_task != NULL){
        if(FLDEFF_GYOE_CheckEnd(cps->act[index].fldeff_gyoe_task) == TRUE){
          cps->act[index].fldeff_gyoe_task = NULL;
        }
      }
    }
  }
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
void CommPlayer_Add(COMM_PLAYER_SYS_PTR cps, int index, u16 obj_code, const COMM_PLAYER_PACKAGE *pack)
{
  OS_TPrintf("通信プレイヤーAdd index=%d\n", index);
  
  if(cps->act_ctrl == NULL){
    OS_TPrintf("act_ctrl=NULL\n");
    return;
  }

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

  cps->act[index].pos = pack->pos;
  cps->act[index].dir = pack->dir;
  cps->act[index].vanish = pack->vanish;
  cps->act[index].obj_code = obj_code;
  cps->act[index].push_flag = FALSE;
  cps->act[index].occ = TRUE;
  cps->act[index].fldeff_gyoe_task = NULL;
  
  FIELD_COMM_ACTOR_CTRL_AddActor(cps->act_ctrl, index, obj_code, &cps->act[index].dir, 
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
  
  if(cps->act_ctrl == NULL){
    return;
  }
  
  OS_TPrintf("通信プレイヤーDel index=%d\n", index);
  cps->act[index].fldeff_gyoe_task = NULL;
  FIELD_COMM_ACTOR_CTRL_DeleteActro(cps->act_ctrl, index);
  cps->act[index].occ = FALSE;
}

//==================================================================
/**
 * 通信プレイヤーPush処理
 *
 * @param   cps		
 */
//==================================================================
void CommPlayer_Push(COMM_PLAYER_SYS_PTR cps)
{
  int i;
  
  GF_ASSERT(GAMESYSTEM_CheckFieldMapWork(cps->gsys) == TRUE);
  
  if(cps->act_ctrl == NULL){
    return;
  }
  
  for(i = 0; i < COMM_PLAYER_MAX; i++){
    if(cps->act[i].occ == TRUE){
      CommPlayer_Del(cps, i);
      cps->act[i].push_flag = TRUE;
      OS_TPrintf("CommPlayer Push! %d\n", i);
    }
  }
  FIELD_COMM_ACTOR_CTRL_Delete(cps->act_ctrl);
  cps->act_ctrl = NULL;
}

//==================================================================
/**
 * 通信プレイヤーPop処理
 *
 * @param   cps		
 */
//==================================================================
void CommPlayer_Pop(COMM_PLAYER_SYS_PTR cps)
{
  int i;
  COMM_PLAYER_PACKAGE pack;
  FIELDMAP_WORK *fieldWork;
  MMDLSYS *fldMdlSys;

  GF_ASSERT(GAMESYSTEM_CheckFieldMapWork(cps->gsys) == TRUE);
  GF_ASSERT(cps->act_ctrl == NULL);

  fieldWork = GAMESYSTEM_GetFieldMapWork(cps->gsys);
  fldMdlSys = FIELDMAP_GetMMdlSys(fieldWork);
  cps->act_ctrl = FIELD_COMM_ACTOR_CTRL_Create(cps->max, fldMdlSys, cps->heap_id, cps->dash_flag);
  
  GFL_STD_MemClear(&pack, sizeof(COMM_PLAYER_PACKAGE));
  for(i = 0; i < COMM_PLAYER_MAX; i++){
    if(cps->act[i].push_flag == TRUE){
      pack.pos = cps->act[i].pos;
      pack.dir = cps->act[i].dir;
      pack.vanish = FALSE;  //cps->act[i].vanish;
      pack.player_form = cps->act[i].player_form;
      CommPlayer_Add(cps, i, cps->act[i].obj_code, &pack);
      OS_TPrintf("CommPlayer Pop! %d\n", i);
    }
  }
}

//==================================================================
/**
 * 座標更新のポーズフラグをセットする
 *
 * @param   cps		
 * @param   stop_flag		TRUE:座標更新を行わない
 */
//==================================================================
void CommPlayer_FlagSet_UpdateStop(COMM_PLAYER_SYS_PTR cps, BOOL stop_flag)
{
  cps->update_stop = stop_flag;
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
 * 通信プレイヤーのOBJコードを取得
 *
 * @param   cps		
 * @param   index		
 *
 * @retval  u16		OBJコード
 */
//==================================================================
u16 CommPlayer_GetObjCode(COMM_PLAYER_SYS_PTR cps, int index)
{
  GF_ASSERT(cps != NULL && index < COMM_PLAYER_MAX && cps->act[index].occ == TRUE);
  return cps->act[index].obj_code;
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
 */
//==================================================================
BOOL CommPlayer_Mine_DataUpdate(COMM_PLAYER_SYS_PTR cps, COMM_PLAYER_PACKAGE *pack)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(cps->gsys);
  MINE_PLAYER *mine = &cps->mine;
  PLAYER_WORK * player;
  VecFx32 draw_pos;
  u16 dir;
  PLAYER_MOVE_FORM form;
  MMDL *player_mmdl;
  BOOL vanish = FALSE;
  
  if(cps->update_stop == TRUE){
    return FALSE;
  }

  player = GAMEDATA_GetMyPlayerWork(gamedata);
  
  form = PLAYERWORK_GetMoveForm( player );

  if( FIELD_STATUS_GetProcAction( GAMEDATA_GetFieldStatus(gamedata) ) == PROC_ACTION_MAPCHG ){
    //マップチェンジ中はプレイヤー座標が左上を指してしまうタイミングがある為、
    ///非表示にする。座標も現在のは送らない
    vanish = TRUE;
    dir = mine->dir;
    draw_pos = mine->pos;
    form = mine->player_form;
  }
  else if(GAMESYSTEM_CheckFieldMapWork(cps->gsys) == TRUE 
      && FIELDMAP_IsReady(GAMESYSTEM_GetFieldMapWork(cps->gsys)) == TRUE){
    FIELDMAP_WORK *fieldWork;
    FIELD_PLAYER * fld_player;
    MMDL *player_mmdl;
    u32 attr;
    PLAYER_DRAW_FORM draw_form;
    
    fieldWork = GAMESYSTEM_GetFieldMapWork(cps->gsys);
    fld_player = FIELDMAP_GetFieldPlayer(fieldWork);
    player_mmdl = FIELD_PLAYER_GetMMdl(fld_player);
    draw_form = FIELD_PLAYER_GetDrawForm( fld_player );
    
    if(draw_form == PLAYER_DRAW_FORM_ITEMGET){  //跳ねるアニメをするのでこの時だけ実座標
      FIELD_PLAYER_GetPos(fld_player, &draw_pos);
    }
    else{
      MMDL_GetDrawVectorPos(player_mmdl, &draw_pos);
    }
    dir = FIELD_PLAYER_GetDir(fld_player);
  }
  else{
    const VecFx32 *p_pos = PLAYERWORK_getPosition(player);
    draw_pos = *p_pos;
    dir = PLAYERWORK_getDirection_Type(player);
  }
  
  pack->dir = dir;
  pack->pos = draw_pos;
  pack->vanish = vanish;
  pack->player_form = form;

  if(dir != mine->dir || form != mine->player_form 
      || vanish != mine->vanish || GFL_STD_MemComp(&draw_pos, &mine->pos, sizeof(VecFx32)) != 0){
    mine->dir = dir;
    mine->pos = draw_pos;
    mine->player_form = form;
    mine->vanish = vanish;
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 指定座標(グリッド)位置にいる通信アクターのIndexを返す
 *
 * @param   cps		      
 * @param   gx		      調べるグリッド座標X
 * @param   gy		      調べるグリッド座標Y
 * @param   gz		      調べるグリッド座標Z
 * @param   out_index		Index格納先
 * @param   search_start_no 検索開始Index番号(最初は0初期化した値を入れておくこと)
 *
 * @retval  BOOL		TRUE:通信アクターが指定座標にいる
 */
//==================================================================
BOOL CommPlayer_SearchGridPos(COMM_PLAYER_SYS_PTR cps, s16 gx, s16 gy, s16 gz, u32 *out_index, u32 *search_start_no)
{
  if(cps->act_ctrl == NULL){
    return FALSE;
  }
  return FIELD_COMM_ACTOR_CTRL_SearchGridPos(cps->act_ctrl, gx, gy, gz, out_index, search_start_no );
}

//==================================================================
/**
 * 「！」エフェクトタスクをセット
 *
 * @param   cps		
 * @param   index		管理Index
 *
 * @retval  BOOL		TRUE:セット成功。　FALSE:セット失敗(既に起動中なども含む)
 */
//==================================================================
BOOL CommPlayer_SetGyoeTask(COMM_PLAYER_SYS_PTR cps, int index)
{
  COMM_PLAYER *act = &cps->act[index];
  MMDL *mmdl;
  FIELDMAP_WORK *fieldWork;
  FLDEFF_CTRL *fectrl;
  
  if(cps->act_ctrl == NULL || act->occ == FALSE || act->fldeff_gyoe_task != NULL){
    return FALSE;
  }
  
  fieldWork = GAMESYSTEM_GetFieldMapWork(cps->gsys);
  fectrl = FIELDMAP_GetFldEffCtrl(fieldWork);
  mmdl = FIELD_COMM_ACTOR_CTRL_GetMMdl(cps->act_ctrl, index);
  
  act->fldeff_gyoe_task = FLDEFF_GYOE_SetMMdlNonDepend(fectrl, mmdl, FLDEFF_GYOETYPE_GYOE, FALSE);
  return TRUE;
}

//==================================================================
/**
 * 対象プレイヤーのMMDLポインタを取得
 *
 * @param   cps		
 * @param   index		管理Index
 *
 * @retval  MMDL *		
 */
//==================================================================
MMDL * CommPlayer_GetMmdl(COMM_PLAYER_SYS_PTR cps, int index)
{
  GF_ASSERT(cps->act_ctrl != NULL);
  GF_ASSERT(cps->act[index].occ == TRUE);
  return FIELD_COMM_ACTOR_CTRL_GetMMdl(cps->act_ctrl, index);
}

//--------------------------------------------------------------
/**
 * フィールドが存在していれば通信プレイヤーシステムを作成する
 *
 * @param   gsys		
 * @param   cps		
 */
//--------------------------------------------------------------
static void _FieldCommActorSys_CheckCreate(GAMESYS_WORK *gsys, COMM_PLAYER_SYS_PTR cps)
{
  FIELDMAP_WORK *fieldWork;
  MMDLSYS *fldMdlSys;

  if(cps->act_ctrl != NULL){
    return;
  }
  
  fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  if(fieldWork == NULL || FIELDMAP_IsReady(fieldWork) == FALSE){
    return;
  }
  
  fldMdlSys = FIELDMAP_GetMMdlSys(fieldWork);
  cps->act_ctrl = FIELD_COMM_ACTOR_CTRL_Create(cps->max, fldMdlSys, cps->heap_id, cps->dash_flag);
}
