//==============================================================================
/**
 * @file    union_char.c
 * @brief   ユニオンルームでの人物関連
 * @author  matsuda
 * @date    2009.07.06(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "union_types.h"
#include "net_app/union/union_main.h"
#include "net_app/union/union_beacon_tool.h"
#include "fieldmap/zone_id.h"
#include "net_app/union/union_chara.h"
#include "union_local.h"
#include "union_msg.h"
#include "union_tool.h"


//==============================================================================
//  型定義
//==============================================================================
///ビーコンPCの動作関数型
typedef BOOL (*UNICHARA_FUNC)(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, u8 *seq);

///ビーコンPCの動作テーブル型
typedef struct{
  UNICHARA_FUNC func_init;
  UNICHARA_FUNC func_update;
  UNICHARA_FUNC func_exit;
}UNICHARA_FUNC_DATA;


//==============================================================================
//  定数定義
//==============================================================================
///ビーコンPC動作サブプロセスNo
enum{
  BPC_SUBPROC_INIT,     ///<初期化処理
  BPC_SUBPROC_UPDATE,   ///<更新処理
  BPC_SUBPROC_EXIT,     ///<終了処理
};


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static BOOL UnionCharaFunc(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara);
static BOOL BeaconPC_UpdateLife(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *bpc);

static BOOL UnicharaSeq_NormalUpdate(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, u8 *seq);


//==============================================================================
//  データ
//==============================================================================
///キャラクタの配置座標
static const struct{
  s16 x;    //グリッド座標：X
  s16 z;    //グリッド座標：Z
}UnionCharPosTbl[] = {
  {10 + 3, 14 - 4},   //index 0
  {10 - 6, 14 - 2},   //index 1
  {10 - 3, 14 - 4},   //index 2
  {10 + 3, 14 - 9},   //index 3
  {10 + 6, 14 - 2},   //index 4
  {10 - 6, 14 - 7},   //index 5
  {10 + 6, 14 - 7},   //index 6
  {10 + 0, 14 - 7},   //index 7
  {10 - 3, 14 - 9},   //index 8
  {10 + 0, 14 - 2},   //index 9
};
SDK_COMPILER_ASSERT(NELEMS(UnionCharPosTbl) == UNION_RECEIVE_BEACON_MAX);

///リンク時のキャラクタの親からのオフセット配置座標
static const struct{
  s16 x;    //グリッド座標：X
  s16 z;    //グリッド座標：Z
}LinkParentOffsetPosTbl[] = {
  {0, 0},             //親
  {1, 0},             //子1
  {-1, 0},            //子2
  {0, -1},            //子3
  {0, 1},             //子4
};


///ユニオンキャラクターのMMDLヘッダーデータ
static const MMDL_HEADER UnionChar_MMdlHeader = 
{
  0,
  0,
  MV_DIR_RND,
  0,	///<イベントタイプ
  0,	///<イベントフラグ
  0,	///<イベントID
  DIR_DOWN,	///<指定方向
  0,	///<指定パラメタ 0
  0,	///<指定パラメタ 1
  0,	///<指定パラメタ 2
  0,	///<X方向移動制限
  0,	///<Z方向移動制限
  0,	///<グリッドX
  0,	///<グリッドZ
  0,	///<Y値 fx32型
};

///ビーコンPCの動作テーブル
static const UNICHARA_FUNC_DATA UnicharaFuncTbl[] = {
  {//BPC_EVENT_STATUS_NORMAL
    NULL,
    UnicharaSeq_NormalUpdate,
    NULL,
  },
  {//BPC_EVENT_STATUS_ENTER
    NULL,
    NULL,
    NULL,
  },
  {//BPC_EVENT_STATUS_LEAVE
    NULL,
    NULL,
    NULL,
  },
};
SDK_COMPILER_ASSERT(NELEMS(UnicharaFuncTbl) == BPC_EVENT_STATUS_MAX);



//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * ビーコンPC：イベントリクエスト
 *
 * @param   bpc		          ビーコンPCへのポインタ
 * @param   event_status		BPC_EVENT_STATUS_???
 *
 * @retval  TRUE:リクエスト正常受付。　FALSE:リクエスト拒否
 */
//--------------------------------------------------------------
static BOOL UNION_CHAR_EventReq(UNION_CHARACTER *unichara, int event_status)
{
  unichara->next_event_status = event_status;
  return TRUE;
}

//==================================================================
/**
 * ユニオンキャラクター：登録
 *
 * @param   unisys		      
 * @param   gdata		        ゲームデータへのポインタ
 * @param   trainer_view		見た目
 * @param   sex		          性別
 * @param   chara_index		  キャラクターIndex
 *
 * @retval  MMDL *		      
 */
//==================================================================
static MMDL * UNION_CHAR_AddOBJ(UNION_SYSTEM_PTR unisys, GAMEDATA *gdata, u8 trainer_view, u16 chara_index)
{
  MMDL *addmdl;
  MMDLSYS *mdlsys;
  MMDL_HEADER head;
  u16 oya_chara_index, child_chara_index;
  
  mdlsys = GAMEDATA_GetMMdlSys( gdata );
  oya_chara_index = chara_index >> 8;
  child_chara_index = chara_index & 0x00ff;
  
  head = UnionChar_MMdlHeader;
  head.id = chara_index;
  head.obj_code = UnionView_GetObjCode(trainer_view);
  MMDLHEADER_SetGridPos( &head,
      UnionCharPosTbl[oya_chara_index].x + LinkParentOffsetPosTbl[child_chara_index].x,
      UnionCharPosTbl[oya_chara_index].z + LinkParentOffsetPosTbl[child_chara_index].z,
      0 );
  OS_TPrintf("obj_code = %d, id = %d\n", head.obj_code, chara_index);
  addmdl = MMDLSYS_AddMMdl(mdlsys, &head, ZONE_ID_UNION);
  return addmdl;
}

//==================================================================
/**
 * ユニオンキャラのCharacterIndexを取得する
 *
 * @param   pc		
 * @param   unichara		
 *
 * @retval  u16		
 */
//==================================================================
static u16 UNION_CHARA_GetCharaIndex(UNION_BEACON_PC *pc, UNION_CHARACTER *unichara)
{
  u16 chara_index;
  
  chara_index = (pc->buffer_no << 8) | unichara->child_no;
  return chara_index;
}

//==================================================================
/**
 * CharacterIndexから、親なのか、子なのかを判別する
 *
 * @param   chara_index		キャラクタインデックス
 *
 * @retval  UNION_CHARA_INDEX_PARENT or UNION_CHARA_INDEX_CHILD
 */
//==================================================================
UNION_CHARA_INDEX UNION_CHARA_CheckCharaIndex(u16 chara_index)
{
  u16 oya_chara_index, child_chara_index;
  
  oya_chara_index = chara_index >> 8;
  child_chara_index = chara_index & 0x00ff;
  
  GF_ASSERT(oya_chara_index < UNION_RECEIVE_BEACON_MAX);
  GF_ASSERT(child_chara_index < UNION_CONNECT_PLAYER_NUM);
  
  if(child_chara_index == 0){
    return UNION_CHARA_INDEX_PARENT;
  }
  return UNION_CHARA_INDEX_CHILD;
}

//==================================================================
/**
 * CharacterIndexから、親番号を取得する
 *
 * @param   chara_index		
 *
 * @retval  u16		
 */
//==================================================================
u16 UNION_CHARA_GetCharaIndex_to_ParentNo(u16 chara_index)
{
  return chara_index >> 8;
}

//==================================================================
/**
 * ユニオンキャラクタのMMDL*を取得する
 *
 * @param   unisys		
 * @param   pc		      親PCのPCパラメータ
 * @param   unichara		対象のユニオンキャラクターへのポインタ
 *
 * @retval  MMDL *		
 */
//==================================================================
static MMDL * UNION_CHARA_GetMmdl(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *pc, UNION_CHARACTER *unichara)
{
  u16 chara_index;
  MMDLSYS *mdlsys;
  MMDL *mmdl;
  
  //ポインタを持つ手法だと画面切り替えでアドレスが変わるので毎回サーチにしている
  chara_index = UNION_CHARA_GetCharaIndex(pc, unichara);
  mdlsys = GAMEDATA_GetMMdlSys( unisys->uniparent->game_data );
  mmdl = MMDLSYS_SearchOBJID(mdlsys, chara_index);
  GF_ASSERT(mmdl != NULL);
  return mmdl;
}

//==================================================================
/**
 * ユニオンキャラグループ：キャラクター登録
 *
 * @param   unisys		
 * @param   pc		    
 */
//==================================================================
static UNION_CHARACTER * UNION_CHARA_AddChar(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *pc, UNION_MEMBER *member, int child_no)
{
  int i;
  UNION_CHARACTER *unichara;
  
  unichara = unisys->character;
  for(i = 0; i < UNION_CHARACTER_MAX; i++){
    if(unichara->occ == FALSE){
      unichara->parent_pc = pc;
      unichara->trainer_view = member->trainer_view;
      unichara->sex = member->sex;
      unichara->child_no = child_no;
      unichara->occ = TRUE;
      
      UNION_CHAR_AddOBJ(unisys, unisys->uniparent->game_data, 
        member->trainer_view, UNION_CHARA_GetCharaIndex(pc, unichara));
      
      return unichara;
    }
    unichara++;
  }
  GF_ASSERT(0); //キャラクタバッファの空きがない
  return NULL;
}

//==================================================================
/**
 * ユニオンキャラクターを削除する
 *
 * @param   unisys		
 * @param   pc		
 * @param   unichara		
 */
//==================================================================
static void UNION_CHARA_DeleteOBJ(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *pc, UNION_CHARACTER *unichara)
{
  OS_TPrintf("ユニオンキャラ削除 chara_index = %x\n", UNION_CHARA_GetMmdl(unisys, pc, unichara));
  MMDL_Delete(UNION_CHARA_GetMmdl(unisys, pc, unichara));
  GFL_STD_MemClear(unichara, sizeof(UNION_CHARACTER));
}

//==================================================================
/**
 * 新規データがあればキャラクターをAdd
 *
 * @param   unisys		
 * @param   pc		
 */
//==================================================================
static void UNION_CHARA_CheckOBJ_Entry(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *pc)
{
  int i;
  UNION_CHARACTER *unichara;
  UNION_BEACON *beacon;
  
  beacon = &pc->beacon;
  for(i = 0; i < UNION_CONNECT_PLAYER_NUM; i++){
    unichara = pc->chara_group.character[i];
    if(unichara == NULL && beacon->party.member[i].occ == TRUE){
      pc->chara_group.character[i] = UNION_CHARA_AddChar(unisys, pc, &beacon->party.member[i], i);
      UNION_CHAR_EventReq(pc->chara_group.character[i], BPC_EVENT_STATUS_ENTER);
    }
  }
}

//==================================================================
/**
 * ユニオンキャラクター：更新処理
 *
 * @param   unisys		
 */
//==================================================================
void UNION_CHAR_Update(UNION_SYSTEM_PTR unisys, GAMEDATA *gdata)
{
  int i;
  UNION_BEACON_PC *bpc;
  BOOL ret;
  MMDLSYS *mdlsys;
  UNION_CHARACTER *unichara;
  int del_count, child_no;
  
  mdlsys = GAMEDATA_GetMMdlSys( gdata );

  bpc = unisys->receive_beacon;
  for(i = 0; i < UNION_RECEIVE_BEACON_MAX; i++){
    if(bpc->beacon.data_valid == UNION_BEACON_VALID){
      //寿命更新
      if(BeaconPC_UpdateLife(unisys, bpc) == FALSE){
        OS_TPrintf("PC:寿命が尽きた group=%d\n", i);
      }
      
      //新規キャラ登録チェック
      UNION_CHARA_CheckOBJ_Entry(unisys, bpc);
      
      del_count = 0;
      for(child_no = 0; child_no < UNION_CONNECT_PLAYER_NUM; child_no++){
        unichara = bpc->chara_group.character[child_no];
        //動作実行
        if(unichara != NULL){
          ret = UnionCharaFunc(unisys, unichara);
          if(ret == FALSE){
            UNION_CHARA_DeleteOBJ(unisys, bpc, unichara);
            bpc->chara_group.character[child_no] = NULL;
            del_count++;
          }
        }
        else{
          del_count++;
        }
      }
      
      //全てのキャラが存在しなくなったならDelete
      if(del_count >= UNION_CONNECT_PLAYER_NUM){
        GFL_STD_MemClear(bpc, sizeof(UNION_BEACON_PC));
        OS_TPrintf("group %d のキャラが全て居なくなったので初期化\n", i);
      }
    }
    bpc++;
  }
}

//--------------------------------------------------------------
/**
 * ビーコンPC動作アップデート
 *
 * @param   bpc		  ビーコンPCへのポインタ
 *
 * @retval  BOOL		TRUE:動作継続。　FALSE:動作終了(モデルを削除してください)
 */
//--------------------------------------------------------------
static BOOL UnionCharaFunc(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara)
{
  BOOL next_seq = TRUE;
  UNICHARA_FUNC func;
  
  switch(unichara->func_proc){
  case BPC_SUBPROC_INIT:
    func = UnicharaFuncTbl[unichara->event_status].func_init;
    break;
  case BPC_SUBPROC_UPDATE:
    func = UnicharaFuncTbl[unichara->event_status].func_update;
    break;
  case BPC_SUBPROC_EXIT:
    func = UnicharaFuncTbl[unichara->event_status].func_exit;
    break;
  }

  if(func != NULL){
    next_seq = func(unisys, unichara, &unichara->func_seq);
  }
  
  if(next_seq == TRUE){
    unichara->func_seq = 0;
    unichara->func_proc++;
    if(unichara->func_proc > BPC_SUBPROC_EXIT){
      if(unichara->event_status == BPC_EVENT_STATUS_LEAVE){
        return FALSE; //退出完了
      }
      unichara->func_proc = 0;
      unichara->event_status = unichara->next_event_status;
      unichara->next_event_status = BPC_EVENT_STATUS_NORMAL;
    }
  }
  
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 寿命更新処理
 *
 * @param   unisys		
 * @param   bpc		
 *
 * @retval  BOOL		TRUE:寿命はまだある。　FALSE:寿命が尽きた
 */
//--------------------------------------------------------------
static BOOL BeaconPC_UpdateLife(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *bpc)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  
  if(situ->mycomm.talk_pc == bpc || situ->mycomm.calling_pc == bpc || situ->mycomm.answer_pc == bpc || situ->mycomm.connect_pc == bpc){
    if(bpc->life == 0){
      bpc->life = 1;
    }
    return TRUE;  //話しかけ中、接続相手は寿命は減らない
  }
  
  if(bpc->life <= 0){
    return FALSE;
  }
  else{
    bpc->life--;
  }
 
  return TRUE;
}

//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * 何もしていない状態：更新
 *
 * @param   unisys		
 * @param   bpc		
 * @param   mmdl		
 * @param   seq		
 *
 * @retval  static BOOL		
 */
//==================================================================
static BOOL UnicharaSeq_NormalUpdate(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, u8 *seq)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  UNION_BEACON *beacon;
  MMDL *mmdl;
  
  beacon = &unichara->parent_pc->beacon;
  
  //寿命チェック
  if(unichara->parent_pc->life == 0){
    UNION_CHAR_EventReq(unichara, BPC_EVENT_STATUS_LEAVE);
    return TRUE;
  }
  
  //自分の存在チェック
  if(unichara->parent_pc->beacon.party.member[unichara->child_no].occ == FALSE){
    UNION_CHAR_EventReq(unichara, BPC_EVENT_STATUS_LEAVE);
    return TRUE;
  }

  //アピール番号が一致していればジャンプ
  if(situ->appeal_no != UNION_APPEAL_NULL 
      && (situ->appeal_no == UnionTool_PlayCategory_to_AppealNo(beacon->play_category)
      || situ->appeal_no == beacon->appeal_no)   //アピール番号一致
      && unichara->child_no == 0                //親だけ
      && UnionMsg_GetMemberMax(beacon->play_category) > beacon->connect_num){ //接続人数未満
    mmdl = UNION_CHARA_GetMmdl(unisys, unichara->parent_pc, unichara);
    if(MMDL_CheckPossibleAcmd(mmdl) == TRUE){
      MMDL_SetAcmd(mmdl, AC_STAY_JUMP_D_8F);
    }
  }
  
  return FALSE;
}

