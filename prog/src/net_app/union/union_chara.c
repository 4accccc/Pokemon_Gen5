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
#include "net_app/union_eff.h"
#include "field/fieldmap.h"
#include "savedata/etc_save.h"


//==============================================================================
//  型定義
//==============================================================================
///ビーコンPCの動作関数型
typedef BOOL (*UNICHARA_FUNC)(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap, u8 *seq);

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

///スクリプトのOBJIDと被らないようにプログラムでずらすオフセット
#define UNION_CHARINDEX_OFFSET    (32)


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static BOOL UnionCharaFunc(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap);
static BOOL BeaconPC_UpdateLife(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *bpc);

static BOOL UnicharaSeq_NormalUpdate(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap, u8 *seq);
static BOOL UnicharaSeq_EnterUpdate(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap, u8 *seq);
static BOOL UnicharaSeq_LeaveUpdate(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap, u8 *seq);
static BOOL UnicharaSeq_TalkingUpdate(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap, u8 *seq);
static BOOL UNION_CHARA_CheckMemberSameValid(UNION_SYSTEM_PTR unisys, const UNION_BEACON_PC *target_pc, const u8 *entry_mac_address);


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
    UnicharaSeq_EnterUpdate,
    NULL,
  },
  {//BPC_EVENT_STATUS_LEAVE
    NULL,
    UnicharaSeq_LeaveUpdate,
    NULL,
  },
  {//BPC_EVENT_STATUS_TALKING
    NULL,
    UnicharaSeq_TalkingUpdate,
    NULL,
  },
};
SDK_COMPILER_ASSERT(NELEMS(UnicharaFuncTbl) == BPC_EVENT_STATUS_MAX);



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ユニオンキャラクターワークを初期化
 *
 * @param   unisys		
 */
//==================================================================
void UNION_CHARA_WorkInit(UNION_SYSTEM_PTR unisys)
{
  GFL_STD_MemClear(unisys->character, sizeof(UNION_CHARACTER) * UNION_CHARACTER_MAX);
}

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
  child_chara_index = (chara_index & 0x00ff) - UNION_CHARINDEX_OFFSET;
  
  head = UnionChar_MMdlHeader;
  head.id = chara_index;
  head.obj_code = UnionView_GetObjCode(trainer_view);
  if(child_chara_index != 0){
    head.move_code = MV_DOWN;  //子は下向き固定
  }
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
  
  chara_index = (pc->buffer_no << 8) | (unichara->child_no + UNION_CHARINDEX_OFFSET);
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
  child_chara_index = (chara_index & 0x00ff) - UNION_CHARINDEX_OFFSET;
  
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
 * CharacterIndexから性別を取得する
 *
 * @param   unisys		
 * @param   chara_index		キャラクタインデックス
 *
 * @retval  int		
 */
//==================================================================
int UNION_CHARA_GetCharaIndex_to_Sex(UNION_SYSTEM_PTR unisys, u16 chara_index)
{
  int i;
  UNION_CHARACTER *unichara;

  unichara = unisys->character;
  for(i = 0; i < UNION_CHARACTER_MAX; i++){
    if(unichara->occ == TRUE && unichara->parent_pc != NULL){
      if(UNION_CHARA_GetCharaIndex(unichara->parent_pc, unichara) == chara_index){
        return unichara->sex;
      }
    }
    unichara++;
  }
  GF_ASSERT(0);
  return PM_MALE;
}

//==================================================================
/**
 * CharacterIndexが通信プレイヤーかNPCかを判定する
 *
 * @param   chara_index		
 *
 * @retval  BOOL		TRUE:通信プレイヤー　　FALSE:NPC
 */
//==================================================================
BOOL UNION_CHARA_CheckCommPlayer(u16 chara_index)
{
  u16 child_chara_index;
  
  child_chara_index = (chara_index & 0x00ff);
  if(child_chara_index < UNION_CHARINDEX_OFFSET){
    return FALSE;
  }
  return TRUE;
}

//==================================================================
/**
 * 話しかけられる状態かチェックする
 *
 * @param   chara_index		
 *
 * @retval  BOOL		TRUE:話しかけ可能　FALSE:不可
 */
//==================================================================
BOOL UNION_CHARA_TalkCheck(UNION_SYSTEM_PTR unisys, u16 chara_index)
{
  UNION_CHARACTER *unichara;
  int i;
  
  unichara = unisys->character;
  for(i = 0; i < UNION_CHARACTER_MAX; i++){
    if(unichara->occ == TRUE && unichara->parent_pc != NULL && unichara->parent_pc->life > 0){ //NULLチェックは念のため
      if(((unichara->parent_pc->buffer_no << 8) | (unichara->child_no + UNION_CHARINDEX_OFFSET)) == chara_index){
        if(unichara->event_status == BPC_EVENT_STATUS_LEAVE 
            || unichara->event_status == BPC_EVENT_STATUS_ENTER
            || unichara->next_event_status == BPC_EVENT_STATUS_LEAVE 
            || unichara->next_event_status == BPC_EVENT_STATUS_ENTER){
          return FALSE;
        }
        return TRUE;
      }
      else{
        OS_TPrintf("chara_index = %d, buf_no=%d, child_no=%d\n", chara_index, unichara->parent_pc->buffer_no, unichara->child_no);
      }
    }
    unichara++;
  }
  return FALSE;
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
  GF_ASSERT_HEAVY(mmdl != NULL);
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
static UNION_CHARACTER * UNION_CHARA_AddChar(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *pc, UNION_MEMBER *member, int child_no, BOOL enable, MMDL **dest_mmdl_ptr)
{
  int i;
  UNION_CHARACTER *unichara;
  MMDL *mmdl;
  
  unichara = unisys->character;
  for(i = 0; i < UNION_CHARACTER_MAX; i++){
    if(unichara->occ == FALSE){
      unichara->parent_pc = pc;
      unichara->trainer_view = member->trainer_view;
      unichara->sex = member->sex;
      unichara->child_no = child_no;
      unichara->occ = TRUE;
      
      mmdl = UNION_CHAR_AddOBJ(unisys, unisys->uniparent->game_data, 
        member->trainer_view, UNION_CHARA_GetCharaIndex(pc, unichara));
      if(enable == FALSE){
        MMDL_SetStatusBitVanish( mmdl, TRUE );
      }
      *dest_mmdl_ptr = mmdl;
      return unichara;
    }
    unichara++;
  }
  GF_ASSERT_HEAVY(0); //キャラクタバッファの空きがない
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
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(unisys->uniparent->gsys);
  MMDL *mmdl = UNION_CHARA_GetMmdl(unisys, pc, unichara);
  
  OS_TPrintf("ユニオンキャラ削除 chara_index = %x\n", mmdl);
  if(fieldWork != NULL && mmdl != NULL){
    //unieffシステムで常時監視で削除しているが、同一フレーム内で削除＞登録、
    //とされると削除出来ないタイミングがある為、退出時に自分で確実に削除
    UNION_EFF_SYSTEM *unieff = FIELDMAP_GetUnionEffSystem(fieldWork);
    UnionEff_App_SearchDeleteFriendMark( unieff, mmdl );
  }
  MMDL_Delete(mmdl);
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
static void UNION_CHARA_CheckOBJ_Entry(UNION_SYSTEM_PTR unisys, ETC_SAVE_WORK *etc_save, WIFI_LIST *wifilist, UNION_EFF_SYSTEM *unieff, UNION_BEACON_PC *pc)
{
  int i;
  UNION_CHARACTER *unichara;
  UNION_BEACON *beacon;
  MMDL *mmdl;
  u8 my_mac[6];
  
  if(pc->life == 0){
    return;
  }
  
  OS_GetMacAddress(my_mac);
  beacon = &pc->beacon;
  for(i = 0; i < UNION_CONNECT_PLAYER_NUM; i++){
    unichara = pc->chara_group.character[i];
    if(unichara == NULL && beacon->party.member[i].occ == TRUE){
      if(GFL_STD_MemComp(my_mac, beacon->party.member[i].mac_address, 6) == 0){
        continue; //自分は画面に出さない
      }
      if(UNION_CHARA_CheckMemberSameValid(unisys, pc, beacon->party.member[i].mac_address) == FALSE){
        continue;
      }
      pc->chara_group.character[i] = UNION_CHARA_AddChar(unisys, pc, &beacon->party.member[i], i, FALSE, &mmdl);
      pc->chara_group.character[i]->event_status = BPC_EVENT_STATUS_ENTER;
      UNION_CHAR_EventReq(pc->chara_group.character[i], BPC_EVENT_STATUS_NORMAL);
      if(i == 0){ //主キャラのみ友達マークチェック
        if(UnionTool_CheckDwcFriend(wifilist, beacon) == TRUE){
          UnionEff_App_ReqFriendMark(unieff, mmdl, FRIENDMARK_TYPE_FRIEND);
        }
        else if(EtcSave_CheckAcquaintance(etc_save, beacon->trainer_id) == TRUE){
          UnionEff_App_ReqFriendMark(unieff, mmdl, FRIENDMARK_TYPE_ACQUAINTANCE);
        }
    #if 0
      #ifdef PM_DEBUG   //ランダムで友達か知り合いマークを表示するデバッグ機能
        else if(GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
          UnionEff_App_ReqFriendMark(unieff, mmdl, GFUser_GetPublicRand(2));
        }
      #endif
    #endif
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 新たに追加しようとしている子メンバーが他のグループに存在していれば、そのグループを解散させる
 *
 * @param   unisys		
 * @param   target_pc		        追加しようとしている子が在籍しているグループのポインタ
 * @param   entry_mac_address		追加しようとしている子のMacAddress
 *
 * @retval  BOOL		TRUE:他グループには存在していない
 *                  FALSE:他グループに存在している
 */
//--------------------------------------------------------------
static BOOL UNION_CHARA_CheckMemberSameValid(UNION_SYSTEM_PTR unisys, const UNION_BEACON_PC *target_pc, const u8 *entry_mac_address)
{
  UNION_BEACON_PC *bpc;
  int i, member_no;
  
  bpc = unisys->receive_beacon;
  for(i = 0; i < UNION_RECEIVE_BEACON_MAX; i++){
    if(bpc != target_pc){
      if(bpc->beacon.data_valid == UNION_BEACON_VALID){
        for(member_no = 0; member_no < UNION_CONNECT_PLAYER_NUM; member_no++){
          if(bpc->beacon.party.member[member_no].occ == TRUE
              && GFL_STD_MemComp(entry_mac_address, bpc->beacon.party.member[member_no].mac_address, 6) == 0){
            //同じ人物発見
            if(bpc->life > 0){
              bpc->life = 1;
            }
            return FALSE;
          }
        }
      }
    }
    bpc++;
  }
  return TRUE;
}

//==================================================================
/**
 * フォーカス対象のキャラクターがいればフォーカスエフェクトをリクエスト
 *
 * @param   unisys		
 * @param   pc		
 */
//==================================================================
static BOOL UNION_CHARA_CheckFocusOBJ(UNION_SYSTEM_PTR unisys, UNION_BEACON_PC *pc)
{
  int i;
  UNION_CHARACTER *unichara;
  UNION_BEACON *beacon;
  
  beacon = &pc->beacon;
  for(i = 0; i < UNION_CONNECT_PLAYER_NUM; i++){
    unichara = pc->chara_group.character[i];
    if(unichara == NULL){
      continue;
    }
    
    if(beacon->party.member[i].occ == TRUE
        && GFL_STD_MemComp(beacon->party.member[i].mac_address, 
        unisys->my_situation.focus_mac_address, 6) == 0){
      if(GAMESYSTEM_CheckFieldMapWork(unisys->uniparent->gsys) == TRUE){
        FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(unisys->uniparent->gsys);
        UNION_EFF_SYSTEM *unieff = FIELDMAP_GetUnionEffSystem(fieldWork);
        
        if(unieff != NULL){
          u16 char_index = UNION_CHARA_GetCharaIndex(pc, unichara);
          MMDLSYS *mdlsys = GAMEDATA_GetMMdlSys( unisys->uniparent->game_data );
          MMDL *focus_mmdl = MMDLSYS_SearchOBJID( mdlsys, char_index );
        
          if(focus_mmdl != NULL){
            UnionEff_App_ReqFocus(unieff, focus_mmdl);
          }
        }
      }
      GFL_STD_MemClear(unisys->my_situation.focus_mac_address, 6);
      return TRUE;
    }
  }
  return FALSE;
}

static void _ReqFocusOBJ(UNION_SYSTEM_PTR unisys, u16 char_index)
{
}


//==================================================================
/**
 * ユニオンキャラクター：更新処理
 *
 * @param   unisys		
 */
//==================================================================
void UNION_CHAR_Update(UNION_SYSTEM_PTR unisys, GAMEDATA *gdata, FIELDMAP_WORK *fieldmap)
{
  int i;
  UNION_BEACON_PC *bpc;
  BOOL ret;
  MMDLSYS *mdlsys;
  UNION_CHARACTER *unichara;
  int del_count, child_no;
  const u8 focus_null_mac[6] = {0,0,0,0,0,0};
  BOOL focus_check = FALSE;
  ETC_SAVE_WORK *etc_save = SaveData_GetEtc( GAMEDATA_GetSaveControlWork(gdata) );
  WIFI_LIST* wifilist = GAMEDATA_GetWiFiList(gdata);
  UNION_EFF_SYSTEM *unieff = FIELDMAP_GetUnionEffSystem(fieldmap);
  
  mdlsys = GAMEDATA_GetMMdlSys( gdata );

  if(GFL_STD_MemComp(focus_null_mac, &unisys->my_situation.focus_mac_address, 6) != 0){
    focus_check = TRUE;
  }

  bpc = unisys->receive_beacon;
  for(i = 0; i < UNION_RECEIVE_BEACON_MAX; i++){
    if(bpc->beacon.data_valid == UNION_BEACON_VALID){
      //寿命更新
      if(BeaconPC_UpdateLife(unisys, bpc) == FALSE){
        //OS_TPrintf("PC:寿命が尽きた group=%d\n", i);
      }
      
      //新規キャラ登録チェック
      UNION_CHARA_CheckOBJ_Entry(unisys, etc_save, wifilist, unieff, bpc);

      //フォーカスキャラチェック
      if(focus_check == TRUE){
        if(UNION_CHARA_CheckFocusOBJ(unisys, bpc) == TRUE){
          focus_check = FALSE;
        }
      }
      
      del_count = 0;
      for(child_no = 0; child_no < UNION_CONNECT_PLAYER_NUM; child_no++){
        unichara = bpc->chara_group.character[child_no];
        //動作実行
        if(unichara != NULL){
          ret = UnionCharaFunc(unisys, unichara, fieldmap);
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
      if(del_count >= UNION_CONNECT_PLAYER_NUM && bpc->life == 0){
        GFL_STD_MemClear(bpc, sizeof(UNION_BEACON_PC));
        OS_TPrintf("group %d のキャラが全て居なくなったので初期化\n", i);
      }
    }
    bpc++;
  }
  GFL_STD_MemClear(unisys->my_situation.focus_mac_address, 6);
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
static BOOL UnionCharaFunc(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap)
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
    next_seq = func(unisys, unichara, fieldmap, &unichara->func_seq);
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

//==================================================================
/**
 * ユニオンキャラクター：画面復帰時の友達マークの再設定
 *
 * @param   unisys		
 */
//==================================================================
void UNION_CHAR_MarkRecover(UNION_SYSTEM_PTR unisys, GAMEDATA *gdata, FIELDMAP_WORK *fieldmap)
{
  int i;
  UNION_BEACON_PC *bpc;
  UNION_CHARACTER *unichara;
  ETC_SAVE_WORK *etc_save = SaveData_GetEtc( GAMEDATA_GetSaveControlWork(gdata) );
  WIFI_LIST* wifilist = GAMEDATA_GetWiFiList(gdata);
  UNION_EFF_SYSTEM *unieff = FIELDMAP_GetUnionEffSystem(fieldmap);
  MMDL *mmdl;
  
  bpc = unisys->receive_beacon;
  for(i = 0; i < UNION_RECEIVE_BEACON_MAX; i++){
    if(bpc->beacon.data_valid == UNION_BEACON_VALID){
      unichara = bpc->chara_group.character[0]; //主キャラのみチェック
      if(unichara != NULL){
        mmdl = UNION_CHARA_GetMmdl(unisys, unichara->parent_pc, unichara);
        if(mmdl != NULL){
          if(UnionTool_CheckDwcFriend(wifilist, &bpc->beacon) == TRUE){
            UnionEff_App_ReqFriendMark(unieff, mmdl, FRIENDMARK_TYPE_FRIEND);
          }
          else if(EtcSave_CheckAcquaintance(etc_save, bpc->beacon.trainer_id) == TRUE){
            UnionEff_App_ReqFriendMark(unieff, mmdl, FRIENDMARK_TYPE_ACQUAINTANCE);
          }
        }
      }
    }
    bpc++;
  }
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
static BOOL UnicharaSeq_NormalUpdate(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap, u8 *seq)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  UNION_BEACON *beacon;
  MMDL *mmdl;
  UNION_APPEAL play_appeal_no;
  
  beacon = &unichara->parent_pc->beacon;
  
  //寿命チェック
  if(unichara->parent_pc->life == 0){
    UNION_CHAR_EventReq(unichara, BPC_EVENT_STATUS_LEAVE);
    return TRUE;
  }
  
  //自分の存在チェック
  if(unichara->parent_pc->beacon.party.member[unichara->child_no].occ == FALSE
      || unichara->parent_pc->beacon.party.member[unichara->child_no].trainer_view != unichara->trainer_view){
    UNION_CHAR_EventReq(unichara, BPC_EVENT_STATUS_LEAVE);
    return TRUE;
  }
  
  //話しかけ相手ならばプレイヤーの方向を向く
  play_appeal_no = UnionTool_PlayCategory_to_AppealNo(beacon->play_category);
  if(situ->mycomm.talk_obj_id == UNION_CHARA_GetCharaIndex(unichara->parent_pc, unichara)){
    UNION_CHAR_EventReq(unichara, BPC_EVENT_STATUS_TALKING);
    return TRUE;
  }
  else if(situ->appeal_no != UNION_APPEAL_NULL //アピール番号が一致していればジャンプ
      && (situ->appeal_no == play_appeal_no
      || (play_appeal_no == UNION_APPEAL_NULL && situ->appeal_no == beacon->appeal_no)) //アピール一致
      && unichara->child_no == 0                //親だけ
#ifdef BUGFIX_BTS7943_20100806
      && UnionMsg_GetMemberMax(beacon->play_category) > beacon->connect_num   //接続人数未満
      && beacon->entry_ng == FALSE){            //乱入禁止になっていない
#else
      && UnionMsg_GetMemberMax(beacon->play_category) > beacon->connect_num){ //接続人数未満
#endif
    mmdl = UNION_CHARA_GetMmdl(unisys, unichara->parent_pc, unichara);
    if(MMDL_CheckPossibleAcmd(mmdl) == TRUE){
      MMDL_SetAcmd(mmdl, AC_STAY_JUMP_D_8F);
    }
  }
  else if(unichara->child_no == 0 && beacon->connect_num > 1){//親で誰かと繋がっている時は向き固定
    mmdl = UNION_CHARA_GetMmdl(unisys, unichara->parent_pc, unichara);
    if(MMDL_GetMoveCode( mmdl ) == MV_DIR_RND){
      MMDL_ChangeMoveCode( mmdl, MV_DOWN );
    }
  }
  else if(unichara->child_no == 0 && beacon->connect_num <= 1){
    mmdl = UNION_CHARA_GetMmdl(unisys, unichara->parent_pc, unichara);
    if(MMDL_GetMoveCode( mmdl ) == MV_DOWN){
      MMDL_ChangeMoveCode( mmdl, MV_DIR_RND );
    }
  }
  
  return FALSE;
}

//==================================================================
/**
 * 入室：更新
 *
 * @param   unisys		
 * @param   bpc		
 * @param   mmdl		
 * @param   seq		
 *
 * @retval  static BOOL		
 */
//==================================================================
static BOOL UnicharaSeq_EnterUpdate(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap, u8 *seq)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  UNION_BEACON *beacon;
  MMDL *mmdl;

  mmdl = UNION_CHARA_GetMmdl(unisys, unichara->parent_pc, unichara);
  
  switch(*seq){
  case 0:
    if(MMDL_CheckPossibleAcmd(mmdl) == TRUE){
      MMDL_GRIDPOS grid_pos, player_pos;
      FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);

      MMDL_GetGridPos( FIELD_PLAYER_GetMMdl( player ), &player_pos );
      MMDL_GetGridPos( mmdl, &grid_pos );
      if(grid_pos.gx != player_pos.gx || grid_pos.gz != player_pos.gz){
        MMDL_SetAcmd(mmdl, AC_WARP_DOWN);
        MMDL_SetStatusBitVanish( mmdl, FALSE );
        (*seq)++;
      }
    }
    break;
  case 1:
    if(MMDL_CheckEndAcmd(mmdl) == TRUE){
      MMDL_EndAcmd(mmdl);
      return TRUE;
    }
    break;
  }
  return FALSE;
}

//==================================================================
/**
 * 退室：更新
 *
 * @param   unisys		
 * @param   bpc		
 * @param   mmdl		
 * @param   seq		
 *
 * @retval  static BOOL		
 */
//==================================================================
static BOOL UnicharaSeq_LeaveUpdate(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap, u8 *seq)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  UNION_BEACON *beacon;
  MMDL *mmdl;

  mmdl = UNION_CHARA_GetMmdl(unisys, unichara->parent_pc, unichara);
  
  switch(*seq){
  case 0:
    if(MMDL_CheckPossibleAcmd(mmdl) == TRUE){
      MMDL_SetAcmd(mmdl, AC_WARP_UP);
      (*seq)++;
    }
    break;
  case 1:
    if(MMDL_CheckEndAcmd(mmdl) == TRUE){
      MMDL_EndAcmd(mmdl);
      return TRUE;
    }
    break;
  }
  return FALSE;
}

//==================================================================
/**
 * お話中：更新
 *
 * @param   unisys		
 * @param   bpc		
 * @param   mmdl		
 * @param   seq		
 *
 * @retval  static BOOL		
 */
//==================================================================
static BOOL UnicharaSeq_TalkingUpdate(UNION_SYSTEM_PTR unisys, UNION_CHARACTER *unichara, FIELDMAP_WORK *fieldmap, u8 *seq)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  MMDL *mmdl;

  mmdl = UNION_CHARA_GetMmdl(unisys, unichara->parent_pc, unichara);

  switch(*seq){
  case 0:
    if(MMDL_CheckPossibleAcmd(mmdl) == TRUE)
    {
      FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
    	u16 dir = MMDL_GetDirDisp( FIELD_PLAYER_GetMMdl( player ) );
    	u16 movecode, anmcode;
    	switch(dir){
      case DIR_UP:
        movecode = MV_DOWN;
        anmcode = AC_DIR_D;
        break;
      case DIR_DOWN:
        movecode = MV_UP;
        anmcode = AC_DIR_U;
        break;
      case DIR_LEFT:
        movecode = MV_RIGHT;
        anmcode = AC_DIR_R;
        break;
      case DIR_RIGHT:
      default:
        movecode = MV_LEFT;
        anmcode = AC_DIR_L;
        break;
      }
      IWASAWA_Printf( "before mv_code = %d\n", MMDL_GetMoveCode( mmdl ));
      //MMDL_ChangeMoveCode( mmdl, movecode );
      MMDL_SetAcmd(mmdl, anmcode);
      (*seq)++;
    }
    break;
  case 1:
    if(MMDL_EndAcmd(mmdl) == TRUE){ //アニメコマンド終了待ち
      MMDL_OnStatusBit(mmdl, MMDL_STABIT_PAUSE_DIR);
      (*seq)++;
    }
    break;
  case 2:
    if(situ->mycomm.talk_obj_id != UNION_CHARA_GetCharaIndex(unichara->parent_pc, unichara)){
      //MMDL_ChangeMoveCode( mmdl, MV_DIR_RND );
      IWASAWA_Printf( "after mv_code = %d\n", MMDL_GetMoveCode( mmdl ));
      MMDL_OffStatusBit(mmdl, MMDL_STABIT_PAUSE_DIR);
      return TRUE;
    }
    break;
  }
  return FALSE;
}

