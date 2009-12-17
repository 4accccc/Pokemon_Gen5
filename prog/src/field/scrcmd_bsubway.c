//======================================================================
/**
 * @file  scrcmd_bsybway.c
 * @brief  バトルサブウェイ　スクリプトコマンド専用ソース
 * @author  Miyuki Iwasawa
 * @date  2006.05.23
 *
 * 2007.05.24  Satoshi Nohara*
 *
 * @note plから移植 kagaya
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "savedata/save_tbl.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h" //SYS_FLAG_SPEXIT_REQUEST
#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "bsubway_scr.h"
#include "bsubway_scr_common.h"
#include "scrcmd_bsubway.h"

#include "event_bsubway.h"

#include "fldmmdl.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
static BOOL bsway_CheckEntryPokeNum(
    u16 num, GAMESYS_WORK *gsys, BOOL item_flag );

//======================================================================
//  proto
//======================================================================

//======================================================================
//  バトルサブウェイ　スクリプト関連
//======================================================================
//--------------------------------------------------------------
/**
 *  @brief  バトルサブウェイ用ワーク作成と初期化
 *  @param  init  初期化モード指定
 *          BSWAY_PLAY_NEW:new game
 *          BSWAY_PLAY_CONTINUE:new game
 *  @param  mode  プレイモード指定:BSWAY_MODE_～
 *  @return 0
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBSubwayWorkCreate( VMHANDLE* core, void *wk )
{
  u16 init = VMGetU16(core);
  u16 mode = VMGetU16(core);
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  BSUBWAY_SCRWORK *bsw_scr;
  bsw_scr = BSUBWAY_SCRWORK_CreateWork( gsys, init, mode );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 *  @brief  バトルサブウェイ制御用ワークポインタ初期化
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBSubwayWorkClear(VMHANDLE* core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  BSUBWAY_SCRWORK_ClearWork( gsys );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 *  @brief  バトルサブウェイ用ワーク解放
 *  @return 0
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBSubwayWorkRelease(VMHANDLE* core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  BSUBWAY_SCRWORK *bsw_scr;
  
  bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  BSUBWAY_SCRWORK_ReleaseWork( gsys, bsw_scr );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 *  @brief  バトルサブウェイ用コマンド群呼び出しインターフェース
 *  @param  com_id    u16:コマンドID
 *  @param  retwk_id  u16:返り値を格納するワークのID
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBSubwayTool( VMHANDLE *core, void *wk )
{
  void **buf;
  BSUBWAY_SCRWORK *bsw_scr;
  BSUBWAY_PLAYDATA *playData;
  BSUBWAY_SCOREDATA *scoreData;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  u16 com_id = VMGetU16( core );
  u16 param = SCRCMD_GetVMWorkValue( core, work );
  u16 retwk_id = VMGetU16( core );
  u16 *ret_wk = SCRIPT_GetEventWork( sc, gdata, retwk_id );
  
  bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  playData = SaveControl_DataPtrGet( save, GMDATA_ID_BSUBWAY_PLAYDATA );
  scoreData = SaveControl_DataPtrGet( save, GMDATA_ID_BSUBWAY_SCOREDATA );
  
  switch( com_id ){
  //-------------------------ワーク非依存--------------------------------
  //参加可能なポケモン数のチェック
  case BSWAY_TOOL_CHK_ENTRY_POKE_NUM:
    if( param == 0 ){
      *ret_wk = bsway_CheckEntryPokeNum( bsw_scr->member_num, gsys, 1 );
    }else{
      *ret_wk = bsway_CheckEntryPokeNum( param, gsys, 1 );
    }
    break;
  //リセット
  case BSWAY_TOOL_SYSTEM_RESET:
    OS_ResetSystem( 0 );
    break;
  //プレイデータクリア
  case BSWAY_TOOL_CLEAR_PLAY_DATA:
    BSUBWAY_PLAYDATA_Init( playData );
    break;
  //セーブしているか？
  case BSWAY_TOOL_IS_SAVE_DATA_ENABLE:
    *ret_wk = BSUBWAY_PLAYDATA_GetSaveFlag( playData );
    break;
  //脱出用に現在のロケーションを記憶する
  case BSWAY_TOOL_PUSH_NOW_LOCATION:
    {
      VecFx32 pos;
      LOCATION loc;
      FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
      FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
      FIELD_PLAYER_GetPos( fld_player, &pos );
      LOCATION_SetDirect( &loc, FIELDMAP_GetZoneID(fieldmap),
          FIELD_PLAYER_GetDir(fld_player),
          pos.x, pos.y, pos.z );

      GAMEDATA_SetSpecialLocation( gdata, &loc );
      EVENTWORK_SetEventFlag(
          GAMEDATA_GetEventWork(gdata), SYS_FLAG_SPEXIT_REQUEST );
    }
    break;
  //記録したロケーションに戻す。
  case BSWAY_TOOL_POP_NOW_LOCATION:
    EVENTWORK_ResetEventFlag(
      GAMEDATA_GetEventWork(gdata), SYS_FLAG_SPEXIT_REQUEST );
    break;
  //連勝レコード数取得
  case BSWAY_TOOL_GET_RENSHOU_RECORD:
    #if 0 //wb null
    *ret_wk = TowerScrTools_GetRenshouRecord(core->fsys->savedata,param);
    #else
    GF_ASSERT( 0 && "BSWAY_TOOL_GET_RENSHOU_RECORD WB未作成" );
    #endif
    break;
  case BSWAY_TOOL_WIFI_RANK_DOWN:
    #if 0 // wb null 
    *ret_wk = TowerScr_SetWifiRank(NULL,core->fsys->savedata,2);
    #else
    GF_ASSERT( 0 && "BSWAY_TOOL_WIFI_RANK_DOWN WB未作成" );
    #endif
    break;
  case BSWAY_TOOL_GET_WIFI_RANK:
    #if 0
    *ret_wk = TowerScr_SetWifiRank(NULL,core->fsys->savedata,0);
    #else
    GF_ASSERT( 0 && "BSWAY_TOOL_GET_WIFI_RANK WB未作成" );
    #endif
    break;
  case BSWAY_TOOL_SET_WIFI_UPLOAD_FLAG:
    #if 0
    TowerScrTools_SetWifiUploadFlag(core->fsys->savedata,param);
    #else
    GF_ASSERT( 0 && "BSWAY_TOOL_SET_WIFI_UPLOAD_FLAG WB未作成" );
    #endif
    break;
  case BSWAY_TOOL_GET_WIFI_UPLOAD_FLAG:
    #if 0
    *ret_wk = TowerScrTools_GetWifiUploadFlag(core->fsys->savedata);
    #else
    GF_ASSERT( 0 && "BSWAY_TOOL_GET_WIFI_UPLOAD_FLAG WB未作成" );
    #endif
    break;
  case BSWAY_TOOL_SET_NG_SCORE:
    #if 0
    *ret_wk = TowerScrTools_SetNGScore(core->fsys->savedata);
    #else
    *ret_wk = BSUBWAY_SCRWORK_SetNGScore( gsys );
    #endif
    break;
  case BSWAY_TOOL_IS_PLAYER_DATA_ENABLE:
    #if 0
    *ret_wk = TowerScrTools_IsPlayerDataEnable(core->fsys->savedata);
    break;
    #else
    GF_ASSERT( 0 && "BSWAY_TOOL_IS_PLAYER_DATA_ENABLE WB未作成" );
    #endif
  case BSWAY_TOOL_WIFI_CONNECT:
    #if 0
    EventCmd_BTowerWifiCall(core->event_work,param,retwk_id,*ret_wk);
    return 1;
    #else
    GF_ASSERT( 0 && "BSWAY_TOOL_WIFI_CONNECT WB未作成コマンドです" );
    #endif
  //---------------------ワーク依存--------------------------------
  case BSWAY_SUB_GET_NOW_ROUND:
    *ret_wk = bsw_scr->now_round;
    break;
  case BSWAY_SUB_INC_ROUND:
    /*
    *ret_wk = TowerScr_IncRound(wk); //dpではランダムの種も更新していた
    */
    bsw_scr->now_win++;
    bsw_scr->now_round++;
    *ret_wk = bsw_scr->now_round;
    break;
  //ポケモンリスト画面呼び出し
  case BSWAY_SUB_SELECT_POKE:
    #if 0
    buf = GetEvScriptWorkMemberAdrs(core->fsys, ID_EVSCR_SUBPROC_WORK);
    TowerScr_SelectPoke(wk,core->event_work,buf);
    #else
    SCRIPT_CallEvent( sc, BSUBWAY_EVENT_SetSelectPokeList(bsw_scr,gsys) );
    #endif
    return VMCMD_RESULT_SUSPEND;
  //選択されたポケモン取得
  case BSWAY_SUB_GET_ENTRY_POKE:
    *ret_wk = BSUBWAY_SCRWORK_GetEntryPoke( bsw_scr, gsys );
    break;
  //参加指定した手持ちポケモンの条件チェック
  case BSWAY_SUB_CHK_ENTRY_POKE:
    #if 0
    *ret_wk = TowerScr_CheckEntryPoke(bsw_scr,core->fsys->savedata);
    #else
    OS_Printf( "BSWAY_SUB_GET_ENTRY_POKE WB未作成" );
    *ret_wk = 0; //0=OK 1=同じポケモン 2=同じアイテム
    #endif
    break;
  case BSWAY_SUB_IS_CLEAR:
    #if 0
    *ret_wk = TowerScr_IsClear(bsw_scr);
    #else
    if( bsw_scr->clear_f ){
      *ret_wk = TRUE;
    }else if( bsw_scr->now_round > BSWAY_CLEAR_WINCNT ){
      //クリアフラグon
      bsw_scr->clear_f = 1;
      *ret_wk = TRUE;
    }else{
      *ret_wk = FALSE;
    }
    #endif
    break;
  case BSWAY_SUB_GET_RENSHOU_CNT:
    if( (u32)bsw_scr->renshou+bsw_scr->now_win > 0xFFFF ){
      *ret_wk = 0xFFFF;
    }else{
      *ret_wk = bsw_scr->renshou + bsw_scr->now_win;
    }
    break;
  case BSWAY_SUB_SET_LOSE_SCORE:
    BSUBWAY_SCRWORK_SetLoseScore( gsys, bsw_scr );
    break;
  case BSWAY_SUB_SET_CLEAR_SCORE:
    #if 0
    TowerScr_SetClearScore(bsw_scr,core->fsys->savedata,core->fsys->fnote);
    #else
    GF_ASSERT( 0 && "BSWAY_SUB_SET_CLEAR_SCORE WB未作成" );
    #endif
    break;
  case BSWAY_SUB_SAVE_REST_PLAY_DATA:
    #if 0
    TowerScr_SaveRestPlayData(bsw_scr);
    #else
    BSUBWAY_SCRWORK_SaveRestPlayData( bsw_scr );
    #endif
    break;
  case BSWAY_SUB_CHOICE_BTL_PARTNER:
    BSUBWAY_SCRWORK_ChoiceBattlePartner( bsw_scr );
    break;
  case BSWAY_SUB_CHOICE_BTL_SEVEN:
    #if 0
    TowerScr_ChoiceBtlSeven(bsw_scr);
    #else
    GF_ASSERT( 0 && "BSWAY_SUB_CHOICE_BTL_SEVEN WB未作成" );
    #endif
    break;
  case BSWAY_SUB_GET_ENEMY_OBJ:
    *ret_wk = BSUBWAY_SCRWORK_GetEnemyObj( bsw_scr, param );
    break;
	case BSWAY_SUB_LOCAL_BTL_CALL:
    {
      FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
      SCRIPT_CallEvent( sc,
          BSUBWAY_EVENT_TrainerBattle(bsw_scr,gsys,fieldmap) );
    }
    return VMCMD_RESULT_SUSPEND;
  //現在のプレイモードを取得
  case BSWAY_SUB_GET_PLAY_MODE:
    *ret_wk = bsw_scr->play_mode;
    break;
  case BSWAY_SUB_SET_LEADER_CLEAR_FLAG:
    bsw_scr->leader_f = param;
    break;
  case BSWAY_SUB_GET_LEADER_CLEAR_FLAG:
    *ret_wk = bsw_scr->leader_f;
    break;
  case BSWAY_SUB_ADD_BATTLE_POINT:
    *ret_wk = BSUBWAY_SCRWORK_AddBattlePoint( bsw_scr );
    break;
  case BSWAY_SUB_GOODS_FLAG_SET:
    #if 0
    TowerScr_GoodsFlagSet(bsw_scr,core->fsys->savedata);
    #else
    GF_ASSERT( 0 && "BSWAY_SUB_GOODS_FLAG_SET WB未作成" );
    #endif
    break;
  case BSWAY_SUB_LEADER_RIBBON_SET:
    #if 0
    *ret_wk = TowerScr_LeaderRibbonSet(bsw_scr,core->fsys->savedata);
    #else
    GF_ASSERT( 0 && "BSWAY_SUB_LEADER_RIBBON_SET WB未作成です" );
    #endif
    break;
  case BSWAY_SUB_RENSHOU_RIBBON_SET:
    #if 0
    *ret_wk = TowerScr_RenshouRibbonSet(bsw_scr,core->fsys->savedata);
    #else
    GF_ASSERT( 0 && "BSWAY_SUB_RENSHOU_RIBBON_SET WB未作成" );
    #endif
    break;
  case BSWAY_SUB_SET_PARTNER_NO:
    bsw_scr->partner = param;
    break;
  case BSWAY_SUB_GET_PARTNER_NO:
    *ret_wk = bsw_scr->partner;
    break;
  case BSWAY_SUB_BTL_TRAINER_SET:
    BSUBWAY_SCRWORK_SetBtlTrainerNo( bsw_scr );
    break;
  case BSWAY_SUB_GET_SELPOKE_IDX:
    #if 0
    *ret_wk = wk->member[param];
    #else
    GF_ASSERT( 0 && "BSWAY_SUB_GET_SELPOKE_IDX WB未作成" );
    #endif
    break;
  //(BTS通信142)変更の対象
  case BSWAY_SUB_WIFI_RANK_UP:
    #if 0
    *ret_wk = TowerScr_SetWifiRank(bsw_scr,core->fsys->savedata,1);
    #else
    GF_ASSERT( 0 && "BSWAY_SUB_WIFI_RANK_UP WB未作成" );
    #endif
    break;
  case BSWAY_TOOL_GET_MINE_OBJ:
    {
      FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
      FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
      int sex = FIELD_PLAYER_GetSex( fld_player );
      *ret_wk = FIELD_PLAYER_GetDrawFormToOBJCode(
          sex, PLAYER_DRAW_FORM_NORMAL );
    }
    break;
  //プレイランダムシードを更新する
  case BSWAY_SUB_UPDATE_RANDOM:
    #if 0 //wb null
    *ret_wk = TowerScr_PlayRandUpdate(bsw_scr,core->fsys->savedata);
    #endif
    break;
  case BSWAY_DEB_IS_WORK_NULL:
    if(bsw_scr == NULL){
      *ret_wk = 1;
    }else{
      *ret_wk = 0;
    }
    break;
  //58:受信バッファクリア
  case BSWAY_SUB_RECV_BUF_CLEAR:
    #if 0
    MI_CpuClear8(wk->recv_buf,BSWAY_SIO_BUF_LEN);
    #else
    GF_ASSERT( 0 && "BSWAY_SUB_RECV_BUF_CLEAR WB未作成" );
    #endif
    break;
  //自機を非表示
  case BSWAY_TOOL_PLAYER_VANISH:
    {
      FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
      FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
      MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
      MMDL_SetStatusBitVanish( mmdl, TRUE );
    }
    break;
  //トレーナー対戦前メッセージ
  case BSWAY_SUB_TRAINER_BEFORE_MSG:
    SCRIPT_CallEvent(
        sc, BSUBWAY_EVENT_TrainerBeforeMsg(bsw_scr,gsys,param) );
    return( VMCMD_RESULT_SUSPEND );
  //エラー
  default:
    OS_Printf( "渡されたcom_id = %d\n", com_id );
    GF_ASSERT( 0 && "com_idが未対応です！" );
//    *ret_wk = 0;
    break;
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  ポケモン選択
//======================================================================


//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * 参加可能なポケモン数のチェック
 * @param  num      参加に必要なポケモン数
 * @param  item_flag  アイテムチェックするかフラグ
 * @retval BOOL TRUE=参加可能
 */
//--------------------------------------------------------------
static BOOL bsway_CheckEntryPokeNum(
    u16 num, GAMESYS_WORK *gsys, BOOL item_flag )
{
  return( TRUE );
}
