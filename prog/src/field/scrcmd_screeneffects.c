//======================================================================
/**
 * @file	scrcmd_screeneffects.c
 *
 * @brief	スクリプトコマンド：フィールド画面演出系
 * @date  2009.09.22
 * @author	tamada GAMEFREAK inc.
 *
 */
//======================================================================
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

#include "scrcmd_screeneffects.h"

#include "event_pc_recovery.h"  //EVENT_PcRecoveryAnime
#include "field_player.h"   //FIELD_PLAYER
#include "fieldmap.h"       //FIELDMAP_GetFieldPlayer

#include "field_g3d_mapper.h" //FLDMAPPPER
#include "field_buildmodel.h"

#include "field/field_const.h"
#include "sound/pm_sndsys.h"      //PMSND_PlaySE, PMSND_CheckPlaySE

#include "event_fieldmap_control.h"   //for EVENT_Field～
#include "system/main.h"  //for HEAPID_PROC

#include "event_pokecen_pc.h"  // for EVENT_PokecenPcOn, EVENT_PokecenPcOff

#define BRIGHT_FADE_SPPED (1)

typedef enum {
  FADE_IN,
  FADE_OUT,
}FADE_IO;

typedef struct BRIGHT_CNT_WORK_tag
{
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap;
	FIELD_FADE_TYPE fade_type;
  FIELD_FADE_WAIT_FLAG wait_flag;
  FADE_IO   fade_io;
  int Seq;
}BRIGHT_CNT_WORK;

static BOOL EvWaitDispFade( VMHANDLE *core, void *wk );
static BOOL EvWaitMapFade( VMHANDLE *core, void *wk );
static void CreateBrightFadeTask( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, FADE_IO fade_io,
                             FIELD_FADE_TYPE type, FIELD_FADE_WAIT_FLAG wait );
static void BrightCntTcb( GFL_TCB* tcb, void* work );

#ifdef PM_DEBUG
extern BOOL MapFadeReqFlg;  //マップフェードリクエストフラグ  宣言元　script.c
#endif

extern void FIELDMAP_InitBG( FIELDMAP_WORK* fieldWork );
extern void FIELDMAP_InitBGMode( void );
//======================================================================
//  画面フェード
//======================================================================
//--------------------------------------------------------------
/**
 * 画面フェード
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdDispFadeStart( VMHANDLE *core, void *wk )
{
  u16 mode = VMGetU16( core );
  u16 start_evy = VMGetU16( core );
  u16 end_evy = VMGetU16( core );
  u16 wait = VMGetU16( core );
  GFL_FADE_SetMasterBrightReq( mode, start_evy, end_evy, wait );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 画面フェード終了チェック ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitDispFade( VMHANDLE *core, void *wk )
{
  if( GFL_FADE_CheckFade() == TRUE ){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * マップ遷移フェード終了チェック ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitMapFade( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  if( FIELDMAP_CheckMapFadeReqFlg( fieldWork ) == TRUE ){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * マップチェンジ用　季節イン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT 
 *
 * @note
 * コンティニュー時に使用すると、フェードアウトしていないために
 * Assertになってしまう。この使い分けのためのコマンド。
 * 現在のゲーム内季節を表示してからフェードインする。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMapFade_SeasonIn_Force( VMHANDLE *core, void *wk )
{
  GMEVENT* event;
  SCRCMD_WORK*   work       = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scriptWork = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*  gameSystem = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*      gameData   = GAMESYSTEM_GetGameData( gameSystem );
  FIELDMAP_WORK* fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  u16            season     = GAMEDATA_GetSeasonID( gameData );
  
  event = EVENT_FieldFadeIn_Season( gameSystem, fieldmap, season, season ); 
  SCRIPT_CallEvent( scriptWork, event );
#ifdef PM_DEBUG
  //GF_ASSERT_MSG(MapFadeReqFlg,"ERROR:NOT CALL MAP_FADE_OUT");
  MapFadeReqFlg = FALSE;
#endif
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * マップチェンジ用　ブラックイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT 
 *
 * @note
 * コンティニュー時に使用すると、フェードアウトしていないために
 * Assertになってしまう。この使い分けのためのコマンド
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMapFade_BlackIn_Force( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );

  CreateBrightFadeTask( gsys, fieldWork, FADE_IN, FIELD_FADE_BLACK, FIELD_FADE_WAIT );
#ifdef PM_DEBUG
  //GF_ASSERT_MSG(MapFadeReqFlg,"ERROR:NOT CALL MAP_FADE_OUT");
  MapFadeReqFlg = FALSE;
#endif
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * マップチェンジ用　ブラックイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMapFade_BlackIn( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );

  CreateBrightFadeTask( gsys, fieldWork, FADE_IN, FIELD_FADE_BLACK, FIELD_FADE_WAIT );
#ifdef PM_DEBUG
  GF_ASSERT_MSG(MapFadeReqFlg,"ERROR:NOT CALL MAP_FADE_OUT");
  MapFadeReqFlg = FALSE;
#endif
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * マップチェンジ用　ブラックアウト
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMapFade_BlackOut( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );

  CreateBrightFadeTask( gsys, fieldWork, FADE_OUT, FIELD_FADE_BLACK, FIELD_FADE_WAIT );
#ifdef PM_DEBUG
  GF_ASSERT_MSG(!MapFadeReqFlg,"ERROR:ALREADY CALLED MAP_FADE_OUT");
  MapFadeReqFlg = TRUE;
#endif  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * マップチェンジ用　ホワイトイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMapFade_WhiteIn( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );

  CreateBrightFadeTask( gsys, fieldWork, FADE_IN, FIELD_FADE_WHITE, FIELD_FADE_WAIT );
#ifdef PM_DEBUG
  GF_ASSERT_MSG(MapFadeReqFlg,"ERROR:NOT CALL MAP_FADE_OUT");
  MapFadeReqFlg = FALSE;
#endif  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * マップチェンジ用　ホワイトアウト
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMapFade_WhiteOut( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );

  CreateBrightFadeTask( gsys, fieldWork, FADE_OUT, FIELD_FADE_WHITE, FIELD_FADE_WAIT );
#ifdef PM_DEBUG
  GF_ASSERT_MSG(!MapFadeReqFlg,"ERROR:ALREADY CALLED MAP_FADE_OUT");
  MapFadeReqFlg = TRUE;
#endif  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 画面フェード終了チェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdDispFadeCheck( VMHANDLE *core, void *wk )
{
  VMCMD_SetWait( core, EvWaitDispFade );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * マップ遷移用フェード終了チェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMapFadeCheck( VMHANDLE *core, void *wk )
{
  VMCMD_SetWait( core, EvWaitMapFade );
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//
//    アニメーション演出
//
//======================================================================
//--------------------------------------------------------------
/**
 * ポケセン回復アニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo  配置モデルを検索し、その位置にアニメを適用したい。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPokecenRecoverAnime( VMHANDLE * core, void *wk )
{
  GMEVENT * call_event;
  GMEVENT * parent;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 pokecount = SCRCMD_GetVMWorkValue( core, wk );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
  
  parent = SCRIPT_GetEvent( sc );
  call_event = EVENT_PcRecoveryAnime( gsys, parent, pokecount );
  SCRIPT_CallEvent( sc, call_event );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * PC起動アニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo  配置モデルを検索し、その位置にアニメを適用したい。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPokecenPcOn( VMHANDLE * core, void *wk )
{
  GMEVENT* event;
  GMEVENT* parent;
  SCRCMD_WORK*       work = wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*         sc = SCRCMD_WORK_GetScriptWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  
  parent = SCRIPT_GetEvent( sc );
  event = EVENT_PokecenPcOn( parent, gsys, fieldmap );
  SCRIPT_CallEvent( sc, event );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * PC起動中アニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo  配置モデルを検索し、その位置にアニメを適用したい。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPokecenPcRun( VMHANDLE * core, void *wk )
{
  GMEVENT* event;
  GMEVENT* parent;
  SCRCMD_WORK*       work = wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*         sc = SCRCMD_WORK_GetScriptWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  
  parent = SCRIPT_GetEvent( sc );
  event = EVENT_PokecenPcRun( parent, gsys, fieldmap );
  SCRIPT_CallEvent( sc, event );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * PC停止アニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo  配置モデルを検索し、その位置にアニメを適用したい。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPokecenPcOff( VMHANDLE * core, void *wk )
{
  GMEVENT* event;
  GMEVENT* parent;
  SCRCMD_WORK*       work = wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*         sc = SCRCMD_WORK_GetScriptWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  
  parent = SCRIPT_GetEvent( sc );
  event = EVENT_PokecenPcOff( parent, gsys, fieldmap );
  SCRIPT_CallEvent( sc, event );
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static FIELD_BMODEL_MAN * getBModelMan( SCRCMD_WORK * wk )
{
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( wk );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMAPPER * mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );
  FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager( mapper );
  return bmodel_man;
}

//--------------------------------------------------------------
/**
 * @brief 配置モデル実データの取得
 */
//--------------------------------------------------------------
static G3DMAPOBJST * getBModel( FIELD_BMODEL_MAN * man, u16 bm_id, u16 gx, u16 gz )
{
  VecFx32 pos;

  pos.x = GRID_TO_FX32( gx );
  pos.y = 0;
  pos.z = GRID_TO_FX32( gz );

  return FIELD_BMODEL_MAN_SearchObjStatusPos( man, bm_id, &pos );
}

//--------------------------------------------------------------
/**
 * @brief 配置モデル実データのアニメ制御：再生終了状態にする
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBModelDirectAnimeSetFinished( VMHANDLE * core, void *wk )
{
  u16 bm_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 gx = SCRCMD_GetVMWorkValue( core, wk );
  u16 gz = SCRCMD_GetVMWorkValue( core, wk );

  SCRCMD_WORK *work = wk;
  FIELD_BMODEL_MAN * bmodel_man = getBModelMan( work );
  G3DMAPOBJST * entry = getBModel( bmodel_man, bm_id, gx, gz );

  if ( entry )
  {
    G3DMAPOBJST_setAnime( bmodel_man, entry, BMANM_INDEX_DOOR_OPEN, BMANM_REQ_REVERSE_START );
    G3DMAPOBJST_setAnime( bmodel_man, entry, BMANM_INDEX_DOOR_OPEN, BMANM_REQ_STOP );
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 配置モデル実データのアニメ制御：ループ再生状態にする
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBModelDirectAnimeSetLoop( VMHANDLE * core, void *wk )
{
  u16 bm_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 gx = SCRCMD_GetVMWorkValue( core, wk );
  u16 gz = SCRCMD_GetVMWorkValue( core, wk );

  SCRCMD_WORK *work = wk;
  FIELD_BMODEL_MAN * bmodel_man = getBModelMan( work );
  G3DMAPOBJST * entry = getBModel( bmodel_man, bm_id, gx, gz );

  if ( entry )
  {
    G3DMAPOBJST_setAnime( bmodel_man, entry, BMANM_INDEX_DOOR_OPEN, BMANM_REQ_LOOP );
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 配置モデル実データの表示制御ON/OFF
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBModelDirectChangeViewFlag( VMHANDLE * core, void *wk )
{
  u16 bm_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 gx = SCRCMD_GetVMWorkValue( core, wk );
  u16 gz = SCRCMD_GetVMWorkValue( core, wk );
  u16 flag = SCRCMD_GetVMWorkValue( core, wk );

  SCRCMD_WORK *work = wk;
  FIELD_BMODEL_MAN * bmodel_man = getBModelMan( work );
  G3DMAPOBJST * entry = getBModel( bmodel_man, bm_id, gx, gz );

  if (entry)
  {
    G3DMAPOBJST_changeViewFlag( entry, flag );
  }

  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//
//
//    配置モデルアニメ
//
//
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 bmodel_unique_key;

//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL evWaitBModelAnime( VMHANDLE *core, void *wk );

//--------------------------------------------------------------
/**
 * @brief 配置モデルアニメ制御：ハンドル生成処理
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBModelAnimeCreate( VMHANDLE * core, void *wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  u16 bm_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 gx = SCRCMD_GetVMWorkValue( core, wk );
  u16 gz = SCRCMD_GetVMWorkValue( core, wk );
  VecFx32 pos;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMAPPER * mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );
  FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager( mapper );

  FIELD_BMODEL * bmodel;

  pos.x = GRID_TO_FX32( gx );
  pos.y = 0;
  pos.z = GRID_TO_FX32( gz );

  bmodel = FIELD_BMODEL_Create_Search( bmodel_man, bm_id, &pos );
  *ret_wk = FIELD_BMODEL_MAN_GetUniqKey( bmodel_man, bmodel );


#ifdef PM_DEBUG
  
  OS_TPrintf( "======= EV_SEQ_BMODEL_ANIME_CREATE INFO ======\n" );
  FIELD_BMODEL_DEBUG_ScriptPrintInfo( bmodel ); 
  OS_TPrintf( "======= EV_SEQ_BMODEL_ANIME_CREATE INFO END ======\n" );
  
#endif

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   配置モデルアニメ制御：ハンドル削除処理
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBModelAnimeDelete( VMHANDLE * core, void *wk )
{
  u16 anime_id = SCRCMD_GetVMWorkValue( core, wk );

  FIELD_BMODEL_MAN * bmodel_man = getBModelMan( wk );
  FIELD_BMODEL * bmodel;

  bmodel = FIELD_BMODEL_MAN_GetBModelByUniqKey( bmodel_man, anime_id );
  if ( bmodel )
  {
    FIELD_BMODEL_Delete( bmodel );
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 配置モデルアニメ制御：アニメセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBModelAnimeSet( VMHANDLE * core, void *wk )
{
  u16 anime_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 anime_type = SCRCMD_GetVMWorkValue( core, wk );
  FIELD_BMODEL_MAN * bmodel_man = getBModelMan( wk );
  FIELD_BMODEL * bmodel;
  u16 seNo;

  bmodel = FIELD_BMODEL_MAN_GetBModelByUniqKey( bmodel_man, anime_id );
  if (bmodel != NULL)
  {
    FIELD_BMODEL_SetAnime( bmodel, anime_type, BMANM_REQ_START );
    if( FIELD_BMODEL_GetCurrentSENo( bmodel, &seNo ) )
    {
      PMSND_PlaySE( seNo );
    }
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 配置モデルアニメ制御：アニメセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBModelAnimeStop( VMHANDLE * core, void *wk )
{
  u16 anime_id = SCRCMD_GetVMWorkValue( core, wk );
  FIELD_BMODEL_MAN * bmodel_man = getBModelMan( wk );
  FIELD_BMODEL * bmodel;
  u16 seNo;

  bmodel = FIELD_BMODEL_MAN_GetBModelByUniqKey( bmodel_man, anime_id );
  if (bmodel != NULL)
  {
    FIELD_BMODEL_StopCurrentAnime( bmodel );
  }

  return VMCMD_RESULT_CONTINUE;
}


//--------------------------------------------------------------
/**
 * @brief 配置モデルアニメ制御：アニメウェイト
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo
 * オブジェクト管理機構をどうするか？考える
 * キーを持ったリスト構造でワーク管理する？
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBModelAnimeWait( VMHANDLE * core, void *wk )
{
  u16 anime_id = SCRCMD_GetVMWorkValue( core, wk );
  bmodel_unique_key = anime_id;
  VMCMD_SetWait( core, evWaitBModelAnime );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief   配置モデルアニメ終了待ち関数
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param   wk      SCRCMD_WORKへのポインタ
 * @return  BOOL  TRUEのとき終了
 */
//--------------------------------------------------------------
static BOOL evWaitBModelAnime( VMHANDLE *core, void *wk )
{
  FIELD_BMODEL_MAN * bmodel_man = getBModelMan( wk );
  FIELD_BMODEL * bmodel;
  bmodel = FIELD_BMODEL_MAN_GetBModelByUniqKey( bmodel_man, bmodel_unique_key );  //手抜き
  if ( bmodel == NULL ) return TRUE;
  if ( FIELD_BMODEL_WaitCurrentAnime( bmodel ) == FALSE)
  {
    return FALSE;
  }

  if ( FIELD_BMODEL_CheckCurrentSE( bmodel ) == FALSE )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//======================================================================
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	ブライトネスフェードタスク生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap  フィールドマップワークへのポインタ
 * @param fade_io   イン　or アウト
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	none
 */
//------------------------------------------------------------------
static void CreateBrightFadeTask( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, FADE_IO fade_io,
                             FIELD_FADE_TYPE type, FIELD_FADE_WAIT_FLAG wait )
{
  BRIGHT_CNT_WORK *task_wk;
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  //ワーク作成
  task_wk = GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(BRIGHT_CNT_WORK) );
  // TCBを追加
  GFL_TCB_AddTask( tcbsys, BrightCntTcb, task_wk, 0 );
  
  //リクエストフラグ立てる
  FIELDMAP_SetMapFadeReqFlg( fieldmap, TRUE );

  task_wk->Seq  = 0;
  task_wk->gsys      = gsys;
  task_wk->fieldmap  = fieldmap;
	task_wk->fade_type = type;
  task_wk->wait_flag = wait;
  task_wk->fade_io = fade_io;
}

//--------------------------------------------------------------
/**
 * ブライトネスフェードタスク
 * @param   tcb        タスクポインタ
 * @param   work      ワーク 
 * @retval  none
 */
//--------------------------------------------------------------
static void BrightCntTcb( GFL_TCB* tcb, void* work )
{
  BRIGHT_CNT_WORK *wk = work;
  switch(wk->Seq){
  case 0:
    {
      int mode;
      int start, end;
      int fade_speed;

      fade_speed = BRIGHT_FADE_SPPED;
      if ( wk->fade_io == FADE_IN )
      {
        //フェードインのときはＢＧ初期化する
        
        start = 16;
        end = 0;
        
        // BGモード設定と表示設定の復帰
        {
          int mv = GFL_DISP_GetMainVisible();
          FIELDMAP_InitBGMode();
          GFL_DISP_GX_SetVisibleControlDirect( mv );
        }
        FIELDMAP_InitBG(wk->fieldmap);
      }
      else
      {
        start = 0;
        end = 16;
      }

      if ( wk->fade_type ==  FIELD_FADE_BLACK)
      {
        mode = GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB;
      }
      else
      {
        mode = GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN | GFL_FADE_MASTER_BRIGHT_WHITEOUT_SUB;
      }
      GFL_FADE_SetMasterBrightReq(mode, start, end, fade_speed);
    }
    wk->Seq++;
    break;
  case 1:
    {
      BOOL rc = FALSE;
      if( wk->wait_flag == FIELD_FADE_NO_WAIT ) rc = TRUE;
		  if( GFL_FADE_CheckFade() == FALSE ) rc = TRUE;

      if (rc) { //フェード終了
        FIELDMAP_SetMapFadeReqFlg( wk->fieldmap, FALSE ); //リクエスト落す
        GFL_TCB_DeleteTask( tcb );
        GFL_HEAP_FreeMemory( work );
      }
    }
		break;
  }
}

//--------------------------------------------------------------
/**
 * @brief フィールドマップBG初期化
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMapFade_InitBG( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  // BGモード設定と表示設定の復帰
  {
    int mv = GFL_DISP_GetMainVisible();
    FIELDMAP_InitBGMode();
    GFL_DISP_GX_SetVisibleControlDirect( mv );
  }
  FIELDMAP_InitBG(fieldmap);

  return VMCMD_RESULT_SUSPEND;
}



