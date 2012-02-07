//============================================================================================
/**
 * @file  scrcmd_wbt_hall.c
 * @author  tamada GAMEFREAK inc.
 * @date  2011.02.21
 * @brief WBT用スクリプトコマンド実装：会場内専用
 */
//============================================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "script_local.h" //SCRIPT_GetEvent

#include "scrcmd_wbt.h"

#include "fldmmdl.h"
#include "fieldmap.h" //FIELDMAP_GetMMdlSys

#include "field/wbt.h"
#include "wbt_system.h"

#include "wbt_setup.h"
#include "wbt_tool.h"
#include "event_wbt.h"
#include "wbt_gimmick.h"

#include "system/main.h"
#include "arc/message.naix"
#include "msg/msg_wbt_system.h"

#include "scrcmd_menuwin.h" //EvCmdBalloonWin_Write

#include "msg/script/msg_wbt_hall.h" //msg_wbt_menu_monsname

#include "savedata/etc_save.h"    //ETC_SAVE_WORK


//============================================================================================
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  RIVALNAME_BUF_IDX = 1,    ///<ライバル名を埋め込むバッファの場所は固定
};

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static void wordset_rivalname( void * wk, WORDSET * wordset, u8 idx )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK * sv_ctrl_work = GAMEDATA_GetSaveControlWork( gamedata );
  ETC_SAVE_WORK * etcsave = SaveData_GetEtc( sv_ctrl_work );
  WORDSET_RegisterRivalName( wordset, idx, EtcSave_GetRivalName( etcsave ) );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void wordset_pokename( void * wk, POKEPARTY * party, u8 poke_pos, u8 idx )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET *wordset = SCRIPT_GetWordSet( sc );
  POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, poke_pos );
  WORDSET_RegisterPokeMonsName( wordset, idx, pp );
}

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBT対戦トレーナーの動作モデルコード取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetEnemyMMdlCode( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = WBTSYS_GetOpponentMMdlObjCode( wbtsys );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBT対戦トレーナーのメッセージ表示
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTTrainerMsg( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  SCRIPT_WORK * sc = SCRCMD_WORK_GetScriptWork( work );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u16 type = SCRCMD_GetVMWorkValue( core, wk );
  u16 obj_id = SCRCMD_GetVMWorkValue( core, wk );

  STRBUF *msgbuf = SCRIPT_GetMsgBuffer( sc );

  {
    WORDSET *wordset = SCRIPT_GetWordSet( sc );
    STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );

    WBTSYS_GetTrainerMsg( wbtsys, type, tmpbuf );

    wordset_rivalname( work, wordset, RIVALNAME_BUF_IDX );
    WORDSET_ExpandStr( wordset, msgbuf, tmpbuf );
  }
  TAMADA_Printf("EvCmdWBTTrainerMsg(type = %d, obj_id =%d )\n", type, obj_id );

  //トレーナーに合わせたメッセージを表示する
  return EvCmdBalloonWin_Write( core, wk, msgbuf, obj_id,
      SCRCMD_MSGWIN_DEFAULT, TALKMSGWIN_TYPE_NORMAL );
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTトレーナー対戦呼び出し
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTTrainerBattle( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*          gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  GMEVENT * next_event;
  TAMADA_Printf("EvCmdWBTTrainerBattle\n");
  GAMEDATA_SetLastBattleResult( gamedata, BTL_RESULT_WIN );

  //対戦結果を受け取るワークを設定する
  WBTSYS_SetScriptAnswerWork( wbtsys, ret_wk );

  //ここからトレーナーバトルイベントを呼び出す
  next_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_wbt), WBT_EVENT_TrainerBattle, (void*)wbtsys );
  SCRIPT_CallEvent( sc, next_event );
  return VMCMD_RESULT_SUSPEND;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：前口上メッセージを表示する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTAnnounce( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  STRBUF *msgbuf = SCRIPT_GetMsgBuffer( sc );
  u16 up_down = SCRCMD_GetVMWorkValue( core, wk );

  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );

  {
    WORDSET *wordset = SCRIPT_GetWordSet( sc );
    STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );

    WBTSYS_GetTrainerMsg( wbtsys, WBT_TRMSG_ANNOUNCE, tmpbuf );

    wordset_rivalname( work, wordset, RIVALNAME_BUF_IDX );
    WORDSET_ExpandStr( wordset, msgbuf, tmpbuf );
  }
  TAMADA_Printf("EvCmdWBTAnnounce(%d)\n", up_down );
  return EvCmdplainWin_Write( core, work, msgbuf, TALKMSGWIN_TYPE_NORMAL, up_down );
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：対戦者名のセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetEnemyName( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u8 idx = VMGetU8(core);
  TAMADA_Printf("EvCmdWBTSetEnemyName\n");
  {
    u8 sex = WBTSYS_GetOpponentSex( wbtsys );
    STRBUF * strbuf = SCRIPT_GetMsgTempBuffer( sc );
    WBTSYS_GetOpponentName( wbtsys, strbuf );
    WORDSET_RegisterWord( wordset, idx, strbuf, sex, TRUE, PM_LANG );
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：対戦相手の手持ちデータ生成
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetupEnemyParty( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  //SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  //GAMEDATA*          gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );

  switch ( WBTSYS_GetWBTCup(wbtsys) )
  {
  case WBTCUP_RENTAL:
  case WBTCUP_RENTALMASTER:
    WBTSYS_SetupEnemyRentalParty( wbtsys );
    break;
  default:
    WBTSYS_SetupEnemyParty( wbtsys, gsys, wbt_tr );
  }
  TAMADA_Printf("EvCmdWBTSetupEnemyParty\n");
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：勝利時の残り手持ちポケモン数を取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetSurvivalCount( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u16   stage  = WBTSYS_GetStage( wbtsys );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  u8 count = WBTSYS_GetSurvivalCount( wbtsys, stage );
  if ( WBTSYS_GetPartyMax(wbtsys) == count )
  { //  完全勝利の場合
    count = SCR_WBT_SURVIVAL_PERFECT;
  }
  *ret_wk = count;

  TAMADA_Printf("EvCmdWBTGetSurvivalCount == %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//============================================================================================
//
//    ミックスバトル関連
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTMixInfoWinOpen( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  WBT_MIXWIN * wmw;
  wmw = WBT_MIXWIN_Create( wk, wbtsys );
  WBTSYS_SetMixInfoWinPointer( wbtsys, wmw );
  WBT_MIXWIN_Draw( wmw );

  TAMADA_Printf("EvCmdWBTMixInfoWinOpen\n");
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTMixInfoWinClose( VMHANDLE *core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  WBT_MIXWIN * wmw = WBTSYS_GetMixInfoWinPointer( wbtsys );
  WBT_MIXWIN_Erase( wmw );
  WBT_MIXWIN_Delete( wmw );
  TAMADA_Printf("EvCmdWBTMixInfoWinClose\n");
  return VMCMD_RESULT_CONTINUE;
}

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ミックストーナメント用の手持ち生成
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTMixInit( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  WBTSYS_MixInit( wbtsys );
  TAMADA_Printf("EvCmdWBTMixInit\n");
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ミックスのランダム選択
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTMixRandomSelect( VMHANDLE *core, void *wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u16 target = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS_MixSetRandomSelect( wbtsys, target );
  TAMADA_Printf("EvCmdWBTMixRandomSelect(%d) = %d\n", target, WBTSYS_MixGetSelected(wbtsys,target) );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ミックスの選択位置セット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTMixSetSelect( VMHANDLE *core, void *wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u16 target = SCRCMD_GetVMWorkValue( core, wk );
  u16 select = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS_MixSetSelect( wbtsys, target, select );
  TAMADA_Printf("EvCmdWBTMixSetSelect(%d,%d)\n", target, select );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ミックスの選択ポケモン取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTMixGetSelectedName( VMHANDLE *core, void *wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u8 idx = VMGetU8( core );
  u16 target = SCRCMD_GetVMWorkValue( core, wk );
  u8 selected;
  POKEPARTY * party;

  selected = WBTSYS_MixGetSelected( wbtsys, target );
  if ( target == WBTMIX_TARGET_MINE1 || target == WBTMIX_TARGET_MINE2 )
  {
    party = WBTSYS_GetMyParty( wbtsys );
  }
  else
  {
    party = WBTSYS_GetEnemyParty( wbtsys );
  }
  wordset_pokename( wk, party, selected, idx );
  TAMADA_Printf("EvCmdWBTMixGetSelectedName(%d,%d)\n", idx, target );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ミックスの選択リスト生成、呼び出し
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTMixListStart( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  STRBUF *msgbuf  = SCRIPT_GetMsgBuffer( sc );
  STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );
  int i;
  int max = WBTSYS_GetPartyMax( wbtsys );
  POKEPARTY * npc_party = WBTSYS_GetEnemyParty( wbtsys );
  u8 sel1, sel2;
  sel1 = WBTSYS_MixGetSelected( wbtsys, WBTMIX_TARGET_ENEMY1 );
  sel2 = WBTSYS_MixGetSelected( wbtsys, WBTMIX_TARGET_ENEMY2 );
  for ( i = 0; i < max; i ++ )
  {
    u16 msg_id, value;
    msg_id = msg_wbt_menu_monsname;
    value = i;
    if ( i == sel1 || i == sel2 )
    {
      msg_id = msg_wbt_menu_monsname_00;
      value = max;
    }
    wordset_pokename( wk, npc_party, i, 0 );
    SCRCMD_WORK_AddMenuList( work, msg_id, SCR_BMPMENU_EXMSG_NULL, value, msgbuf, tmpbuf );
  }
  //SCRCMD_WORK_AddMenuList( work, msg_wbt_menu_check, SCR_BMPMENU_EXMSG_NULL, max, msgbuf, tmpbuf );
  TAMADA_Printf("EvCmdWBTMixListStart\n");
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ミックストーナメントでの「かくにん」用メッセージ生成
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTMixSetupCheckMsg( VMHANDLE * core, void *wk)
{
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ミックストーナメント用の手持ち生成
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetupMixParty( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  WBTSYS_SetupMixParty( wbtsys );
  TAMADA_Printf("EvCmdWBTSetupMixParty\n");
  return VMCMD_RESULT_CONTINUE;
}


//============================================================================================
//
//      WBT用コマンド：ギミック関連
//
//============================================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック背景ビジョン初期化
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKVisionChangeInit( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA     *gdata = SCRCMD_WORK_GetGameData( work );
  HEAPID      heap_id = SCRCMD_WORK_GetHeapID( work );
  SCRIPT_WORK*     sc = SCRCMD_WORK_GetScriptWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  STRBUF * strbuf1;
  STRBUF * strbuf2;   // プレイヤー名専用
  u16 trtype1;
  u16 trtype2;
  u16 trtype_enemy, trtype_mine;

  u8 type = SCRCMD_GetVMWorkValue( core, wk );    // ビジョン種類
  OS_Printf("EvCmdWBTGMKVisionChange(%d)\n",type);
  //対戦相手のトレーナータイプ取得
  trtype_enemy = WBTTRAINER_GetBattleTrainerType( WBTSYS_GetOpponentTrainer( wbtsys ) );
  //自分のトレーナータイプ取得
  trtype_mine = WBTTRAINER_GetBattleTrainerType( WBTSYS_GetPlayerTrainer( wbtsys ) );
  

  switch( type ){
  // VSビジョン
  // strbuf1 = 左上の名前
  // strbuf2 = 右下の名前
  case SCR_WBT_VISION_VS:
    strbuf1 = SCRIPT_GetMsgTempBuffer( sc );
    WBTSYS_GetOpponentName( wbtsys, strbuf1 );

    // プレイヤー名作成
    {
      MYSTATUS* myStatus;
      myStatus = GAMEDATA_GetMyStatus( gdata );
      strbuf2  = MyStatus_CreateNameString( myStatus, heap_id );
    }

    // トレーナータイプ
    trtype1 = trtype_enemy;
    trtype2 = trtype_mine;
    break;

  // 優勝ビジョン   プレイヤー勝ち
  // strbuf1 = 優勝者の名前
  // strbuf2 = 未使用
  case SCR_WBT_VISION_WIN_PLAYER:
    // プレイヤー名コピー
    {
      MYSTATUS* myStatus;
      myStatus = GAMEDATA_GetMyStatus( gdata );
      strbuf1 = SCRIPT_GetMsgTempBuffer( sc );
      MyStatus_CopyNameString( myStatus, strbuf1 );
    }
    strbuf2 = NULL;

    // トレーナータイプ
    trtype1 = trtype_mine;
    trtype2 = 0;
    break;
  // 優勝ビジョン   敵が勝ち
  // strbuf1 = 優勝者の名前
  // strbuf2 = 未使用
  case SCR_WBT_VISION_WIN_ENEMY:
    strbuf1 = SCRIPT_GetMsgTempBuffer( sc );
    WBTSYS_GetOpponentName( wbtsys, strbuf1 );
    strbuf2 = NULL;

    // トレーナータイプ
    trtype1 = trtype_enemy;
    trtype2 = 0;
    break;

  default:
    strbuf1 = NULL;
    strbuf2 = NULL;
    trtype1 = 0;
    trtype2 = 0;
    break;
  }

  // ギミック変更
  WBT_GIMMICK_VisionChange( fieldWork, type, strbuf1, strbuf2, trtype1, trtype2 );

  // strbuf2はプレイヤー名で作成しているかもしれないので破棄
  if( strbuf2 ){ GFL_STR_DeleteBuffer( strbuf2 ); }

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック背景ビジョン変更
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKVisionChange( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA     *gdata = SCRCMD_WORK_GetGameData( work );
  HEAPID      heap_id = SCRCMD_WORK_GetHeapID( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );

  u8 type = SCRCMD_GetVMWorkValue( core, wk );    // ビジョン種類

  // ギミック変更
  WBT_GIMMICK_VisionChangeStart( fieldWork, type );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック通路エフェクト開始
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKEffectStart( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  u8 type = SCRCMD_GetVMWorkValue( core, wk );
  u8 pos = SCRCMD_GetVMWorkValue( core, wk );

  if( type >= SCR_WBT_EFFECT_MAX )
  {
    GF_ASSERT_MSG( 0, "通路エフェクト定義エラー" );
    return VMCMD_RESULT_CONTINUE;
  }

  // ギミック開始
  WBT_GIMMICK_EffectStart( fieldWork, type, pos );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック優勝エフェクト開始
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKWinEffectStart( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  // ギミック開始
  WBT_GIMMICK_WinEffectStart( fieldWork );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック照明設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKLightSet( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  u8 on_off = SCRCMD_GetVMWorkValue( core, wk );
  u8 pos = SCRCMD_GetVMWorkValue( core, wk );

  // ギミック開始
  WBT_GIMMICK_LightSet( fieldWork, on_off, pos );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック爆破エフェクト
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKExplosion( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  u8 type = SCRCMD_GetVMWorkValue( core, wk );

  // ギミック開始
  WBT_GIMMICK_Explosion( fieldWork, type );

  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//======================================================================
