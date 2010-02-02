//============================================================================================
/**
 * @file  event_mapchange.c
 * @brief マップ遷移関連イベント
 * @date  2008.11.04
 * @author  tamada GAME FREAK inc.
 *
 * @todo  FIELD_STATUS_SetFieldInitFlagをどこかに機能としてまとめるべきか検討
 */
//============================================================================================
#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/iss_sys.h"
#include "gamesystem/pm_weather.h"

#include "system/main.h"

#include "field/zonedata.h"
#include "field/fieldmap.h"
#include "field/location.h"
#include "field/rail_location.h"  //RAIL_LOCATION
#include "field/eventdata_sxy.h"
#include "field/fldmmdl.h"
#include "field/fieldskill_mapeff.h"
#include "field/field_status_local.h" //FIELD_STATUS

#include "event_fieldmap_control.h"
#include "event_mapchange.h"
#include "sound/pm_sndsys.h"    //サウンドシステム参照

#include "event_fldmmdl_control.h"
#include "./event_fieldmap_control_local.h" // event_mapchange内限定のFieldOpen Close
#include "field_place_name.h"   //FIELD_PLACE_NAME_DisplayForce

#include "script.h"

#include "fieldmap/zone_id.h"   //※check　ユニオンのSubScreen設定暫定対処の為

#include "warpdata.h"
#include "move_pokemon.h"
#include "field_sound.h"
#include "effect_encount.h"

#include "event_entrance_in.h"
#include "event_entrance_out.h"
#include "event_appear.h"
#include "event_disappear.h"

#include "field/field_flagcontrol.h" //FIELD_FLAGCONT_INIT～

#include "savedata/gimmickwork.h"   //for GIMMICKWORK

#include "net_app/union/union_main.h" // for UNION_CommBoot

#include "savedata/gametime.h"  // for GMTIME
#include "gamesystem/pm_season.h"  // for PMSEASON_TOTAL
#include "ev_time.h"  //EVTIME_Update
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  //SYS_FLAG_SPEXIT_REQUEST
#include "../../../resource/fldmapdata/flagwork/work_define.h"  //WK_SYS_SCENE_COMM

#include "../../../resource/fldmapdata/script/pokecen_scr_def.h"  //SCRID_POKECEN_ELEVATOR_OUT
#ifdef PM_DEBUG
#include "../gamesystem/debug_data.h"
FS_EXTERN_OVERLAY(debug_data);
#include "debug/debug_flg.h"
#endif

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void MAPCHG_setupMapTools( GAMESYS_WORK * gsys, const LOCATION * loc_req );
static void MAPCHG_releaseMapTools( GAMESYS_WORK * gsys );

static void MAPCHG_updateGameData( GAMESYS_WORK * gsys, const LOCATION * loc_req );

static void MAPCHG_loadMMdl( GAMEDATA * gamedata, const LOCATION *loc_req );
static void MAPCHG_loadMMdlWFBC( GAMEDATA * gamedata, const LOCATION *loc );
static void MAPCHG_releaseMMdl( GAMEDATA * gamedata );

static void MAPCHG_SetUpWfbc( GAMEDATA * gamedata, const LOCATION *loc );


static void MAPCHG_setupFieldSkillMapEff( GAMEDATA * gamedata, const LOCATION *loc_req );

static void AssignGimmickID(GAMEDATA * gamedata, int inZoneID);

static GMEVENT_RESULT EVENT_MapChangeNoFade(GMEVENT * event, int *seq, void*work);



//============================================================================================
//
//  イベント：ゲーム開始
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  GAMEDATA * gamedata;
  GAMEINIT_MODE game_init_mode;
  //const GAME_INIT_WORK * game_init_work;
  LOCATION loc_req;
}FIRST_MAPIN_WORK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FirstMapIn(GMEVENT * event, int *seq, void *work)
{
  FIRST_MAPIN_WORK * fmw = work;
  GAMESYS_WORK * gsys = fmw->gsys;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
  EVENTWORK * ev = GAMEDATA_GetEventWork( gamedata );
  FIELDMAP_WORK * fieldmap;
  switch (*seq) {
  case 0:
    // VRAM全クリア
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    switch(fmw->game_init_mode){
    case GAMEINIT_MODE_FIRST:
      SCRIPT_CallGameStartInitScript( gsys, GFL_HEAPID_APP );
      FIELD_STATUS_SetFieldInitFlag( GAMEDATA_GetFieldStatus(gamedata), TRUE );
      //新しいマップモードなど機能指定を行う
      MAPCHG_setupMapTools( gsys, &fmw->loc_req );
      //新しいマップID、初期位置をセット
      MAPCHG_updateGameData( gsys, &fmw->loc_req );
      break;

    case GAMEINIT_MODE_DEBUG:
      SCRIPT_CallDebugGameStartInitScript( gsys, GFL_HEAPID_APP );
      FIELD_STATUS_SetFieldInitFlag( GAMEDATA_GetFieldStatus(gamedata), TRUE );
      //新しいマップモードなど機能指定を行う
      MAPCHG_setupMapTools( gsys, &fmw->loc_req );
      //新しいマップID、初期位置をセット
      MAPCHG_updateGameData( gsys, &fmw->loc_req );
      break;

    case GAMEINIT_MODE_CONTINUE:
      if (EVENTWORK_CheckEventFlag( ev, SYS_FLAG_SPEXIT_REQUEST ) )
      { //特殊接続リクエスト：
        const LOCATION * spLoc = GAMEDATA_GetSpecialLocation( gamedata );
        fmw->loc_req = *spLoc;
    //    EVENTWORK_ResetEventFlag( ev, SYS_FLAG_SPEXIT_REQUEST );
        FIELD_STATUS_SetFieldInitFlag( GAMEDATA_GetFieldStatus(gamedata), TRUE );
        MAPCHG_releaseMapTools( gsys );
        //新しいマップモードなど機能指定を行う
        MAPCHG_setupMapTools( gsys, &fmw->loc_req );
        //新しいマップID、初期位置をセット
        MAPCHG_updateGameData( gsys, &fmw->loc_req );
      }
      else
      {
        //新しいマップモードなど機能指定を行う
        MAPCHG_setupMapTools( gsys, &fmw->loc_req );
        //イベント時間更新
        EVTIME_Update( gamedata );
        //コンティニューによるフラグ落とし処理
        FIELD_FLAGCONT_INIT_Continue( gamedata, fmw->loc_req.zone_id );
        //天気ロード決定
        PM_WEATHER_UpdateSaveLoadWeatherNo( gamedata, fmw->loc_req.zone_id );
      }
      break;

    default:
      GF_ASSERT(0);
    } 

    (*seq)++;
    break;
    
  case 1:
    // BGM再生開始 ( フェードイン )
    {
      u32 soundIdx;
      
      soundIdx = FSND_GetFieldBGM( gamedata, fmw->loc_req.zone_id );
      GMEVENT_CallEvent( event, 
          EVENT_FSND_ChangeBGM( gsys, soundIdx, FSND_FADE_NONE, FSND_FADE_NORMAL ) );
    }
    (*seq)++;
    break;

  case 2:
    GMEVENT_CallEvent(event, EVENT_FieldOpen_FieldProcOnly(gsys));
    (*seq)++;
    break;

  case 3:
    if ( fmw->game_init_mode == GAMEINIT_MODE_CONTINUE 
        &&EVENTWORK_CheckEventFlag( ev, SYS_FLAG_SPEXIT_REQUEST ) )
    {
      EVENTWORK_ResetEventFlag( ev, SYS_FLAG_SPEXIT_REQUEST );
      if ( *(EVENTWORK_GetEventWorkAdrs( ev, WK_SYS_SCENE_COMM )) != 0 )
      {
        SCRIPT_CallScript( event, SCRID_POKECEN_ELEVATOR_OUT_CONTINUE, NULL, NULL, HEAPID_FIELDMAP );
        *seq = 5;
        break;
      }
    }
    (*seq) ++;
    break;

  case 4:
    fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
    // 画面フェードイン
    if( fmw->game_init_mode == GAMEINIT_MODE_DEBUG )
    { // デバッグ開始
      GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black( gsys, fieldmap, FIELD_FADE_WAIT ) );
    }
    else
    { // はじめから / つづきから
      u8 season = GAMEDATA_GetSeasonID( gamedata );
      GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Season( gsys, fieldmap, season, season ) );
    }
    (*seq)++;
    break;

  case 5:
    // 地名を表示する
    fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
    if( FIELDMAP_GetPlaceNameSys(fieldmap) )
    {
      FIELD_PLACE_NAME_DisplayForce( FIELDMAP_GetPlaceNameSys(fieldmap), fmw->loc_req.zone_id );
    }
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドの新規開始初期化処理
 *
 *	@param	gsys  ゲームシステム
 *
 *	GAME_DATA の情報で、新規ゲーム開始時に行いたい処理を記述してください。
 *
 *	このタイミングで呼ばれる処理は、OVERLAY　FIELD_INITに配置してください。
 */
//-----------------------------------------------------------------------------
static void GAME_FieldFirstInit( GAMESYS_WORK * gsys )
{
  GAMEDATA* gamedata = GAMESYSTEM_GetGameData(gsys);

  // WFBC街自分の場所初期化
  FIELD_WFBC_CORE_SetUp( GAMEDATA_GetMyWFBCCoreData(gamedata), GAMEDATA_GetMyStatus(gamedata), GFL_HEAPID_APP ); //heap はテンポラリ用
  FIELD_WFBC_CORE_SetUpZoneData( GAMEDATA_GetMyWFBCCoreData(gamedata) );
}

//------------------------------------------------------------------
/**
 * @brief ゲーム開始処理
 *
 * @todo
 * コンティニュー処理を内部で切り分けているので
 * きちんと関数分割する必要がある
 */
//------------------------------------------------------------------
GMEVENT * EVENT_CallGameStart(GAMESYS_WORK * gsys, GAME_INIT_WORK * game_init_work)
{
  FIRST_MAPIN_WORK * fmw;
  GMEVENT * event;

  event = GMEVENT_Create(gsys, NULL, EVENT_FirstMapIn, sizeof(FIRST_MAPIN_WORK));
  fmw   = GMEVENT_GetEventWork(event);

  fmw->gsys     = gsys;
  fmw->gamedata = GAMESYSTEM_GetGameData(gsys);
  //fmw->game_init_work = game_init_work;
  fmw->game_init_mode = game_init_work->mode;

  switch(game_init_work->mode){
  case GAMEINIT_MODE_CONTINUE:
    LOCATION_SetDirect(&fmw->loc_req, game_init_work->mapid, game_init_work->dir, 
      game_init_work->pos.x, game_init_work->pos.y, game_init_work->pos.z);
    break;
  case GAMEINIT_MODE_FIRST:
    LOCATION_SetGameStart(&fmw->loc_req);
    GAME_FieldFirstInit( gsys );  // フィールド情報の初期化
    break;
#ifdef PM_DEBUG
  case GAMEINIT_MODE_DEBUG:
     DEBUG_FLG_FlgOn( DEBUG_FLG_ShortcutBtlIn );
    GFL_OVERLAY_Load( FS_OVERLAY_ID(debug_data));
    //適当に手持ちポケモンをAdd
    DEBUG_MyPokeAdd( GAMESYSTEM_GetGameData(gsys), GFL_HEAPID_APP );
    //デバッグアイテム追加
    DEBUG_MYITEM_MakeBag( GAMESYSTEM_GetGameData(gsys), GFL_HEAPID_APP );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(debug_data));
    
    LOCATION_DEBUG_SetDefaultPos(&fmw->loc_req, game_init_work->mapid);

    GAME_FieldFirstInit( gsys );  // フィールド情報の初期化
    break;
#endif //PM_DEBUG
  }

  return event;
}

//============================================================================================
//
//    イベント：ゲーム終了
//
//============================================================================================
typedef struct {
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
}GAME_END_WORK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT GameEndEvent(GMEVENT * event, int *seq, void *work)
{
  GAME_END_WORK * gew = work;
  switch (*seq) {
  case 0:
    //フィールドマップをフェードアウト
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gew->gsys, gew->fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    //通信が動いている場合は終了させる
    if(GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gew->gsys)) != GAME_COMM_NO_NULL){
      GameCommSys_ExitReq(GAMESYSTEM_GetGameCommSysPtr(gew->gsys));
    }
    (*seq)++;
    break;
  case 1:
    //通信終了待ち
    if(GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gew->gsys)) != GAME_COMM_NO_NULL){
      break;
    }
    //フィールドマップを終了待ち
    GMEVENT_CallEvent(event, EVENT_FieldClose_FieldProcOnly(gew->gsys, gew->fieldmap));
    (*seq)++;
    break;
  case 2:
    //プロセスを終了し、イベントも終了させるとゲームを終わる
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief デバッグ用：ゲーム終了
 *
 * @note
 * 実際にはゲームを終了させるというインターフェイスは
 * ポケモンに存在しないので、この関数はあくまでデバッグ用。
 */
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_CallGameEnd( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
  GMEVENT * event = GMEVENT_Create(gsys, NULL, GameEndEvent, sizeof(GAME_END_WORK));
  GAME_END_WORK * gew = GMEVENT_GetEventWork(event);
  gew->gsys = gsys;
  gew->fieldmap = fieldmap;
  return event;
}

//============================================================================================
//
//  イベント：マップ切り替え
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief マップ遷移処理イベント用ワーク
 */
//------------------------------------------------------------------
typedef struct 
{
  GAMESYS_WORK*  gameSystem;
  GAMEDATA*      gameData;
  FIELDMAP_WORK* fieldmap;

  u16 before_zone_id;   ///<遷移前マップID
  LOCATION loc_req;     ///<遷移先指定
  EXIT_TYPE exit_type;
  u8 mapchange_type; ///<チェンジタイプ EV_MAPCHG_TYPE
  VecFx32 stream_pos;  ///<流砂中心座標 ( 流砂遷移時にのみ使用 )

  BOOL seasonUpdateEnable; ///<季節の更新を許可するかどうか ( イベント呼び出し側で指定 )
  BOOL seasonUpdateOccur;  ///<季節を更新するかどうか ( イベント内で決定 )
  u16  prevSeason;         ///<遷移前の季節
  u16  nextSeason;         ///<遷移後の季節

} MAPCHANGE_WORK;

typedef MAPCHANGE_WORK* MAPCHANGE_WORK_PTR;

#define INVALID_SEASON_ID (0xff)  ///<季節ID無効値

typedef enum{
  EV_MAPCHG_NORMAL,
  EV_MAPCHG_FLYSKY,
  EV_MAPCHG_ESCAPE,
  EV_MAPCHG_TELEPORT,
} EV_MAPCHG_TYPE;


//------------------------------------------------------------------
/**
 * @brief 季節を更新するかどうかを決定する
 *
 * @param work
 */
//------------------------------------------------------------------
static void SetSeasonUpdate( MAPCHANGE_WORK* work )
{
  BOOL nextZoneIsOutdoor;
  u16 prevSeason, nextSeason;

  // 遷移前・遷移後の季節を決定
  prevSeason = GAMEDATA_GetSeasonID( work->gameData );
  nextSeason = PMSEASON_GetRealTimeSeasonID();
  
  // 遷移先が屋外かどうかを判定
  {
    HEAPID heapID;
    u16 areaID;
    AREADATA* areaData;

    heapID   = FIELDMAP_GetHeapID( work->fieldmap );
    areaID   = ZONEDATA_GetAreaID( work->loc_req.zone_id );
    areaData = AREADATA_Create( heapID, areaID, PMSEASON_SPRING );
    nextZoneIsOutdoor = ( AREADATA_GetInnerOuterSwitch(areaData) != 0 );

    AREADATA_Delete( areaData );
  }

  // 季節が変化 && 遷移先が屋外
  if( (nextSeason != prevSeason) && (nextZoneIsOutdoor) )
  { 
    work->seasonUpdateOccur = TRUE; 
    work->prevSeason = prevSeason;
    work->nextSeason = nextSeason;
  }
}

//------------------------------------------------------------------
/**
 * @brief マップチェンジ コア処理
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_MapChangeCore( GMEVENT* event, int* seq, void* wk )
{
  MAPCHANGE_WORK* work       = *( (MAPCHANGE_WORK_PTR*)wk );
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;
  GAMEDATA*       gameData   = work->gameData;

  switch( *seq )
  {
  case 0:
    //フィールドマップを終了待ち
    GMEVENT_CallEvent( event, EVENT_FieldClose_FieldProcOnly(gameSystem, fieldmap) );
    (*seq)++;
    break;
  case 1:
    //マップモードなど機能指定を解除する
    MAPCHG_releaseMapTools( gameSystem );

    { 
      FIELD_STATUS_SetFieldInitFlag( GAMEDATA_GetFieldStatus(gameData), TRUE );
    }

    // 季節の更新
    if( (work->seasonUpdateEnable) && (work->seasonUpdateOccur) )
    {
      GAMEDATA_SetSeasonID( gameData, work->nextSeason );
      OBATA_Printf( "update season %d ==> %d\n", work->prevSeason, work->nextSeason );
    }

    //新しいマップモードなど機能指定を行う
    MAPCHG_setupMapTools( gameSystem, &work->loc_req );
    
    //新しいマップID、初期位置をセット
    MAPCHG_updateGameData( gameSystem, &work->loc_req );

    //タイプに応じたフラグ初期化
    switch(work->mapchange_type){
    case EV_MAPCHG_FLYSKY:
      FIELD_FLAGCONT_INIT_FlySky( gameData, work->loc_req.zone_id );
      break;
    case EV_MAPCHG_ESCAPE:
      FIELD_FLAGCONT_INIT_Escape( gameData, work->loc_req.zone_id );
      break;
    case EV_MAPCHG_TELEPORT:
      FIELD_FLAGCONT_INIT_Teleport( gameData, work->loc_req.zone_id );
      break;
    }
    (*seq)++;
    break;
  case 2:
    //フィールドマップを開始待ち
    GMEVENT_CallEvent( event, EVENT_FieldOpen_FieldProcOnly(gameSystem) );
    (*seq) ++;
    break;
  case 3:
    // BGM フェード完了待ち
    GMEVENT_CallEvent( event, EVENT_FSND_WaitBGMFade(gameSystem) );
    (*seq) ++;
    break;
  case 4:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT* EVENT_MapChangeCore( MAPCHANGE_WORK* mcw, EV_MAPCHG_TYPE type )
{
  GMEVENT* event;
  MAPCHANGE_WORK_PTR* workPtr;

  event = GMEVENT_Create( mcw->gameSystem, NULL, EVENT_FUNC_MapChangeCore, sizeof(MAPCHANGE_WORK_PTR) );
  workPtr  = GMEVENT_GetEventWork( event );
  *workPtr = mcw;
  mcw->mapchange_type = type;
  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChange( GMEVENT* event, int* seq, void* wk )
{
  MAPCHANGE_WORK* work       = wk;
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;
  GAMEDATA*       gameData   = work->gameData;

  switch(*seq)
  {
  case 0:
    // 季節更新の有無を決定
    SetSeasonUpdate( work );
    // 動作モデルの移動を止める
    {
      MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDLSYS_PauseMoveProc( fmmdlsys );
    }
    // 入口進入イベント
    GMEVENT_CallEvent( event, 
        EVENT_EntranceIn( event, gameSystem, gameData, fieldmap, 
                          work->loc_req, work->exit_type, work->seasonUpdateOccur ) );
    (*seq)++;
    break;
  case 1:
    // マップチェンジ・コア・イベント
    GMEVENT_CallEvent( event, 
        EVENT_MapChangeCore( work, EV_MAPCHG_NORMAL ) );
    (*seq)++;
    break;
  case 2:
    // 入口退出イベント
    GMEVENT_CallEvent( event, 
        EVENT_EntranceOut( event, gameSystem, gameData, fieldmap, work->loc_req, 
                           work->seasonUpdateOccur, 
                           PMSEASON_GetNextSeasonID(work->prevSeason), work->nextSeason ) );
    (*seq)++;
    break;
  case 3:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief 瞬間マップチェンジ ( デバッグ用 )
 */
//------------------------------------------------------------------
static GMEVENT_RESULT DEBUG_EVENT_QuickMapChange( GMEVENT* event, int* seq, void* wk )
{
  MAPCHANGE_WORK* work       = wk;
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;
  GAMEDATA*       gameData   = work->gameData;
  FIELD_SOUND*    fieldSound = GAMEDATA_GetFieldSound( gameData );

  switch(*seq)
  {
  case 0:
    // フェードアウト
    GMEVENT_CallEvent( event, DEBUG_EVENT_QuickFadeOut( gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 1:
    // マップチェンジ・コア・イベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( work, EV_MAPCHG_NORMAL ) );
    (*seq)++;
    break;
  case 2:
    // BGM変更
    FSND_StandByNextMapBGM( fieldSound, gameData, work->before_zone_id, work->loc_req.zone_id );
    FSND_PlayStartBGM( fieldSound );
    (*seq)++;
    break;
  case 3:
    // フェードイン
    GMEVENT_CallEvent( event, DEBUG_EVENT_QuickFadeIn( gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 4:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief フェードなしマップチェンジ
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeNoFade( GMEVENT* event, int* seq, void* wk )
{
  MAPCHANGE_WORK* work       = wk;
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;
  GAMEDATA*       gameData   = work->gameData;
  FIELD_SOUND*    fieldSound = GAMEDATA_GetFieldSound( work->gameData );

  switch(*seq) 
  {
  case 0:
    // 動作モデル停止
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gameSystem, fieldmap) );
    (*seq)++;
    break;
  case 1:
    // BGM更新リクエスト
    FSND_StandByNextMapBGM( fieldSound, gameData, work->before_zone_id, work->loc_req.zone_id );
    FSND_PlayStartBGM( fieldSound );
    (*seq)++;
    break;
  case 2:
    // マップチェンジ コアイベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( work, work->mapchange_type ) );
    (*seq)++;
    break;
  case 3:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief BGMを変更しないマップチェンジ
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeBGMKeep( GMEVENT* event, int* seq, void* wk )
{
  MAPCHANGE_WORK*  work       = wk;
  GAMESYS_WORK*    gameSystem = work->gameSystem;
  GAMEDATA*        gameData   = work->gameData;
  FIELDMAP_WORK*   fieldmap   = work->fieldmap;

  switch( *seq )
  {
  case 0:
    // 動作モデル停止
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll( gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 1:
    // 画面フェードアウト ( クロスフェード )
    GMEVENT_CallEvent( event, EVENT_FieldFadeOut_Cross( gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 2:
    // マップチェンジ コア イベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( work, work->mapchange_type ) );
    (*seq)++;
    break;
  case 3:
    // 画面フェードイン ( クロスフェード )
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Cross( gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 4:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief 流砂によるマップチェンジ
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeBySandStream( GMEVENT* event, int* seq, void* wk )
{
  MAPCHANGE_WORK* work       = wk;
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;

  switch( *seq )
  {
  case 0:
    // 動作モデル停止
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gameSystem, fieldmap) );
    (*seq)++;
    break;
  case 1: 
    // 流砂退場イベント
    GMEVENT_CallEvent( event, 
        EVENT_DISAPPEAR_FallInSand( event, gameSystem, fieldmap, &work->stream_pos ) );
    (*seq)++;
    break;
  case 2: 
    // マップチェンジ・コア・イベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( work, EV_MAPCHG_NORMAL ) );
    (*seq)++;
    break;
  case 3: 
    // 流砂登場イベント
    GMEVENT_CallEvent( event, EVENT_APPEAR_Fall( event, gameSystem, fieldmap ) );
    (*seq) ++;
    break;
  case 4:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief 「あなぬけのヒモ」によるマップチェンジ
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeByAnanukenohimo( GMEVENT* event, int* seq, void* wk )
{
  MAPCHANGE_WORK* work       = wk;
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;

  switch( *seq )
  {
  case 0:
    // 動作モデル停止
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll( gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 1: 
    // 退場イベント
    GMEVENT_CallEvent( event, 
        EVENT_DISAPPEAR_Ananukenohimo( event, gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 2: 
    // マップチェンジ コアイベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( work, EV_MAPCHG_ESCAPE ) );
    (*seq)++;
    break;
  case 3: 
    // 登場イベント
    GMEVENT_CallEvent( event, EVENT_APPEAR_Ananukenohimo( event, gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 4:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief 「あなをほる」によるマップチェンジ
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeByAnawohoru( GMEVENT* event, int* seq, void* wk )
{
  MAPCHANGE_WORK* work       = wk;
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;

  switch( *seq )
  {
  case 0:
    // 動作モデル停止
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gameSystem, fieldmap) );
    (*seq) ++;
    break;
  case 1: 
    // 退場イベント
    GMEVENT_CallEvent( event, EVENT_DISAPPEAR_Anawohoru( event, gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 2: // マップチェンジ コアイベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( work, EV_MAPCHG_ESCAPE ) );
    (*seq)++;
    break;
  case 3: 
    // 登場イベント
    GMEVENT_CallEvent( event, EVENT_APPEAR_Anawohoru( event, gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 4:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief 「テレポート」によるマップチェンジ
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeByTeleport( GMEVENT* event, int* seq, void* wk )
{
  MAPCHANGE_WORK* work       = wk;
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;

  switch( *seq )
  {
  case 0:
    // 動作モデル停止
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll( gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 1: 
    // 退場イベント
    GMEVENT_CallEvent( event, EVENT_DISAPPEAR_Teleport( event, gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 2: 
    // マップチェンジ コアイベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( work, EV_MAPCHG_TELEPORT ) );
    (*seq)++;
    break;
  case 3: 
    // 登場イベント
    GMEVENT_CallEvent( event, EVENT_APPEAR_Teleport( event, gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 4:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief ユニオンルームからのマップチェンジ
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeFromUnion( GMEVENT * event, int *seq, void * wk )
{
  MAPCHANGE_WORK* work       = wk;
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;

  switch( *seq )
  {
  case 0:
    // 退場イベント
    GMEVENT_CallEvent( event, EVENT_DISAPPEAR_Teleport( event, gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 1:
    // マップチェンジ コアイベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( work, EV_MAPCHG_NORMAL ) );
    (*seq)++;
    break;
  case 2:
    // エレベータからの登場スクリプト実行
    SCRIPT_CallScript( event, SCRID_POKECEN_ELEVATOR_OUT, NULL, NULL, HEAPID_FIELDMAP );
    (*seq)++;
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief ユニオンルームへのマップチェンジ
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeToUnion( GMEVENT* event, int* seq, void* wk )
{
  MAPCHANGE_WORK* work       = wk;
  GAMESYS_WORK*   gameSystem = work->gameSystem;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;
  GAMEDATA*       gameData   = work->gameData;

  switch( *seq )
  {
  case 0:
    // 動作モデル停止
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll( gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 1: 
    // マップチェンジ コアイベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( work, EV_MAPCHG_NORMAL ) );
    (*seq)++;
    break;
  case 2: 
    // ユニオン通信起動
    UNION_CommBoot( gameSystem );
    (*seq)++;
    break;
  case 3:
    // 入口退出イベント
    GMEVENT_CallEvent( event, EVENT_APPEAR_Fall( event, gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 4:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief マップチェンジ イベントワークの初期化
 */
//------------------------------------------------------------------
static void MAPCHANGE_WORK_init( MAPCHANGE_WORK* work, GAMESYS_WORK* gameSystem )
{
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gameSystem ); 

  work->gameSystem         = gameSystem;
  work->gameData           = GAMESYSTEM_GetGameData( gameSystem );
  work->fieldmap           = fieldmap;
  work->before_zone_id     = FIELDMAP_GetZoneID( fieldmap );
  work->seasonUpdateEnable = FALSE;
  work->seasonUpdateOccur  = FALSE;
  work->prevSeason         = 0;
  work->nextSeason         = 0;

  //コモン処理としてここに書く
  EFFECT_ENC_EffectAnmPauseSet( FIELDMAP_GetEncount( work->fieldmap), TRUE );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_ChangeMapPos( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap,
                             u16 zoneID, const VecFx32* pos, u16 dir, BOOL seasonUpdateEnable )
{
  MAPCHANGE_WORK* work;
  GMEVENT* event;

  event = GMEVENT_Create( gameSystem, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem );
  LOCATION_SetDirect( &(work->loc_req), zoneID, dir, pos->x, pos->y, pos->z ); 
  work->exit_type          = EXIT_TYPE_NONE;
  work->seasonUpdateEnable = seasonUpdateEnable;

  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* DEBUG_EVENT_ChangeMapRailLocation( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap,
                                            u16 zoneID, const RAIL_LOCATION* rail_loc, u16 dir )
{
  MAPCHANGE_WORK* work;
  GMEVENT* event;

  event = GMEVENT_Create(gameSystem, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK));
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  LOCATION_SetDirectRail( &(work->loc_req), zoneID, dir, 
                          rail_loc->rail_index, rail_loc->line_grid, rail_loc->width_grid);
  work->exit_type = EXIT_TYPE_NONE;
  
  return event;
} 

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* DEBUG_EVENT_ChangeMapDefaultPos( GAMESYS_WORK* gameSystem, 
                                          FIELDMAP_WORK* fieldmap, u16 zoneID )
{
  MAPCHANGE_WORK* work;
  GMEVENT* event;

  if( zoneID >= ZONEDATA_GetZoneIDMax() )
  {
    GF_ASSERT(0);
    zoneID = 0;
  }

  event = GMEVENT_Create( gameSystem, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  LOCATION_DEBUG_SetDefaultPos( &(work->loc_req), zoneID );
  work->exit_type = EXIT_TYPE_NONE;

  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* DEBUG_EVENT_QuickChangeMapDefaultPos( GAMESYS_WORK * gameSystem,
                                                FIELDMAP_WORK* fieldmap, u16 zoneID )
{
  MAPCHANGE_WORK* work;
  GMEVENT* event;

  if( zoneID >= ZONEDATA_GetZoneIDMax() )
  {
    GF_ASSERT(0);
    zoneID = 0;
  }

  event = GMEVENT_Create( gameSystem, NULL, DEBUG_EVENT_QuickMapChange, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  LOCATION_DEBUG_SetDefaultPos( &(work->loc_req), zoneID );
  work->exit_type = EXIT_TYPE_NONE;

  return event;
}

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（ 流砂 ）
 * @param gameSystem          ゲームシステムへのポインタ
 * @param fieldmap      フィールドシステムへのポインタ
 * @param disappearPos 流砂中心点の座標
 * @param zoneID       遷移するマップのZONE指定
 * @param appearPos        遷移先での座標
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
GMEVENT* EVENT_ChangeMapBySandStream( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap,  
                                      const VecFx32* disappearPos, 
                                      u16 zoneID, const VecFx32* appearPos )
{
  MAPCHANGE_WORK* work;
  GMEVENT* event;

  event = GMEVENT_Create( gameSystem, NULL, EVENT_MapChangeBySandStream, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  LOCATION_SetDirect( &(work->loc_req), zoneID, DIR_DOWN, appearPos->x, appearPos->y, appearPos->z);
  VEC_Set( &(work->stream_pos), disappearPos->x, disappearPos->y, disappearPos->z );
  work->exit_type = EXIT_TYPE_NONE;

  return event;
}

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（ あなぬけのヒモ ）
 * @param gameSystem ゲームシステムへのポインタ
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
GMEVENT* EVENT_ChangeMapByAnanukenohimo( FIELDMAP_WORK* fieldWork, GAMESYS_WORK* gameSystem )
{
  GMEVENT* event;
  MAPCHANGE_WORK* work;

  event = GMEVENT_Create( gameSystem, NULL, EVENT_MapChangeByAnanukenohimo, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  work->loc_req      = *(GAMEDATA_GetEscapeLocation( work->gameData ));
  work->loc_req.type = LOCATION_TYPE_DIRECT;
  work->exit_type    = EXIT_TYPE_NONE;

  return event;
}

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（ あなをほる )
 * @param gameSystem ゲームシステムへのポインタ
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
GMEVENT* EVENT_ChangeMapByAnawohoru( GAMESYS_WORK* gameSystem )
{
  GMEVENT* event;
  MAPCHANGE_WORK* work;

  event = GMEVENT_Create( gameSystem, NULL, EVENT_MapChangeByAnawohoru, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  work->loc_req      = *(GAMEDATA_GetEscapeLocation( work->gameData ));
  work->loc_req.type = LOCATION_TYPE_DIRECT;
  work->exit_type    = EXIT_TYPE_NONE;

  return event;
}

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（ テレポート )
 * @param gameSystem ゲームシステムへのポインタ
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
GMEVENT* EVENT_ChangeMapByTeleport( GAMESYS_WORK* gameSystem )
{
  GMEVENT* event;
  MAPCHANGE_WORK* work;
  GAMEDATA* gameData;
  u16 warpID;

  // ワープID取得
  gameData = GAMESYSTEM_GetGameData( gameSystem );
  warpID   = GAMEDATA_GetWarpID( gameData );

  event = GMEVENT_Create( gameSystem, NULL, EVENT_MapChangeByTeleport, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  WARPDATA_GetWarpLocation( warpID, &(work->loc_req) );
  LOCATION_DEBUG_SetDefaultPos( &(work->loc_req), work->loc_req.zone_id );
  work->loc_req.type = LOCATION_TYPE_DIRECT;
  work->exit_type    = EXIT_TYPE_NONE;

  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_ChangeMapToUnion( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap )
{
  MAPCHANGE_WORK* work;
  GMEVENT* event;

  event = GMEVENT_Create( gameSystem, NULL, EVENT_MapChangeToUnion, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  LOCATION_DEBUG_SetDefaultPos( &(work->loc_req), ZONE_ID_UNION );
  work->exit_type = EXIT_TYPE_NONE;

  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_ChangeMapFromUnion( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap )
{
  MAPCHANGE_WORK* work;
  GMEVENT* event;
  const LOCATION* spLoc;

  spLoc = GAMEDATA_GetSpecialLocation( GAMESYSTEM_GetGameData( gameSystem ) );

  event = GMEVENT_Create( gameSystem, NULL, EVENT_MapChangeFromUnion, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  work->loc_req = *spLoc;

  return event;
} 

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_ChangeMapByConnect( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap,
                                   const CONNECT_DATA* connectData, LOC_EXIT_OFS exitOfs )
{
  MAPCHANGE_WORK* work;
  GMEVENT* event;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  MAPCHANGE_WORK_init( work, gameSystem );
  work->seasonUpdateEnable = TRUE;

  if( CONNECTDATA_IsSpecialExit( connectData ) )
  {
    //特殊接続先が指定されている場合、記憶しておいた場所に飛ぶ
    const LOCATION * sp = GAMEDATA_GetSpecialLocation( gameData );
    work->loc_req = *sp;
  }
  else
  {
    CONNECTDATA_SetNextLocation( connectData, &(work->loc_req), exitOfs );
  }
  work->exit_type = CONNECTDATA_GetExitType( connectData );

  {
    //フィールド→屋内への移動の際は脱出先位置を記憶しておく
    const LOCATION* ent = GAMEDATA_GetEntranceLocation( gameData );
    if( ZONEDATA_IsFieldMatrixID(ent->zone_id) == TRUE
        && ZONEDATA_IsFieldMatrixID(work->loc_req.zone_id) == FALSE )
    {
      GAMEDATA_SetEscapeLocation( gameData, ent );
    }
  }
  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT* EVENT_ChangeMapPosNoFadeCore( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap,
                                              EV_MAPCHG_TYPE type, u16 zoneID, const VecFx32* pos, u16 dir )
{
  MAPCHANGE_WORK* work;
  GMEVENT* event;

  event = GMEVENT_Create(gameSystem, NULL, EVENT_MapChangeNoFade, sizeof(MAPCHANGE_WORK));
  work  = GMEVENT_GetEventWork(event);

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  LOCATION_SetDirect( &(work->loc_req), zoneID, dir, pos->x, pos->y, pos->z );
  work->exit_type      = EXIT_TYPE_NONE;
  work->mapchange_type = type; 

  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_ChangeMapPosNoFade( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap,
                                   u16 zoneID, const VecFx32* pos, u16 dir )
{
  return EVENT_ChangeMapPosNoFadeCore( gameSystem, fieldmap, EV_MAPCHG_NORMAL, zoneID, pos, dir );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_ChangeMapSorawotobu( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap,
                                    u16 zoneID, const VecFx32* pos, u16 dir )
{
  return EVENT_ChangeMapPosNoFadeCore( gameSystem, fieldmap, EV_MAPCHG_FLYSKY, zoneID, pos, dir );
}

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成 (BGM変更なし)
 * @param gameSystem ゲームシステムへのポインタ
 * @param fieldmap   フィールドシステムへのポインタ
 * @param zoneID     遷移するマップのZONE指定
 * @param pos        遷移するマップでの座標指定
 * @param dir        遷移するマップでの方向指定
 * @return GMEVENT  生成したマップ遷移イベント
 */
//------------------------------------------------------------------
GMEVENT * EVENT_ChangeMapBGMKeep( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap,
                                  u16 zoneID, const VecFx32* pos, u16 dir )
{
  MAPCHANGE_WORK* work;
  GMEVENT* event;

  event = GMEVENT_Create( gameSystem, NULL, EVENT_MapChangeBGMKeep, sizeof(MAPCHANGE_WORK) );
  work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  MAPCHANGE_WORK_init( work, gameSystem ); 
  LOCATION_SetDirect( &(work->loc_req), zoneID, dir, pos->x, pos->y, pos->z );
  work->exit_type = EXIT_TYPE_NONE;
  
  return event;
}



//------------------------------------------------------------------
//  ※マップIDをインクリメントしている。最大値になったら先頭に戻る
//------------------------------------------------------------------
GMEVENT* DEBUG_EVENT_ChangeToNextMap( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GAMEDATA* gamedata = GAMESYSTEM_GetGameData(gsys);
  PLAYER_WORK* myplayer = GAMEDATA_GetMyPlayerWork(gamedata);
  ZONEID next = PLAYERWORK_getZoneID(myplayer);
  next ++;
  if (next >= ZONEDATA_GetZoneIDMax()) {
    next = 0;
  }
  return DEBUG_EVENT_ChangeMapDefaultPos(gsys, fieldmap, next);
}

//============================================================================================
//
//
//  マップ遷移初期化処理
//
//
//============================================================================================
//-----------------------------------------------------------------------------
/**
 *      全滅時の処理
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	MAPCHANGE_GAMEOVER
//=====================================
typedef struct {
  GAMESYS_WORK * gsys;
} MAPCHANGE_GAMEOVER;


//--------------------------------------------------------------
/**
 * @brief 全滅時のマップ遷移処理（フィールド非生成時）
 */
//--------------------------------------------------------------
static void MAPCHG_GameOver( GAMESYS_WORK * gsys )
{
  LOCATION loc_req;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
  u16 warp_id = GAMEDATA_GetWarpID( gamedata );
  //復活ポイントを取得
  WARPDATA_GetRevivalLocation( warp_id, &loc_req );
  {
    LOCATION esc;
    //エスケープポイントを再設定
    WARPDATA_GetEscapeLocation( warp_id, &esc );
    GAMEDATA_SetEscapeLocation( gamedata, &esc );
  }

  //マップモードなど機能指定を解除する
  MAPCHG_releaseMapTools( gsys );

  { 
    FIELD_STATUS_SetFieldInitFlag( GAMEDATA_GetFieldStatus(gamedata), TRUE );
  }
  //新しいマップモードなど機能指定を行う
  MAPCHG_setupMapTools( gsys, &loc_req );
  
  //新しいマップID、初期位置をセット
  MAPCHG_updateGameData( gsys, &loc_req );

  //イベント時間更新
  EVTIME_Update( gamedata );

  //ゲームオーバー時のフラグのクリア
  FIELD_FLAGCONT_INIT_GameOver( gamedata, loc_req.zone_id );
}


//----------------------------------------------------------------------------
/**
 *	@brief    ゲームオーバーイベント
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT GMEVENT_GameOver(GMEVENT * event, int * seq, void *work)
{
  MAPCHANGE_GAMEOVER* p_wk = work;

  switch( *seq )
  {
  // 遷移先を設定
  case 0:
    MAPCHG_GameOver( p_wk->gsys );
    (*seq) ++;
    break;

  // フィールドマップ開始
  case 1:
    GMEVENT_CallEvent( event, EVENT_FieldOpen_FieldProcOnly(p_wk->gsys) );
    (*seq) ++;
    break;

  case 2:
		return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  全滅時のマップ遷移処理
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_CallGameOver( GAMESYS_WORK * gsys )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	GMEVENT * event;
  MAPCHANGE_GAMEOVER* p_wk;
  event = GMEVENT_Create( gsys, NULL, GMEVENT_GameOver, sizeof(MAPCHANGE_GAMEOVER) );
  p_wk = GMEVENT_GetEventWork( event );
  p_wk->gsys = gsys;

  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static u16 GetDirValueByDirID(EXIT_DIR dir_id)
{
  switch (dir_id) {
  default:
  case EXIT_DIR_UP: return 0x0000;
  case EXIT_DIR_LEFT: return 0x4000;
  case EXIT_DIR_DOWN: return 0x8000;
  case EXIT_DIR_RIGHT:return 0xc000;
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void MakeNewLocation(const EVENTDATA_SYSTEM * evdata, const LOCATION * loc_req,
    LOCATION * loc_tmp)
{
  BOOL result;

  //直接指定の場合はコピーするだけ
  if (loc_req->type == LOCATION_TYPE_DIRECT) {
    *loc_tmp = *loc_req;
    return;
  }
  //開始位置セット
  result = EVENTDATA_SetLocationByExitID(evdata, loc_tmp, loc_req->exit_id, loc_req->exit_ofs );

  if (!result) {
    //デバッグ用処理：本来は不要なはず
    OS_Printf("connect: debug default position\n");
    LOCATION_DEBUG_SetDefaultPos(loc_tmp, loc_req->zone_id);
  }
}

//------------------------------------------------------------------
/**
 * @brief Map切り替え時のデータ更新処理
 *
 * @todo  歩いてマップをまたいだときの処理との共通部分をきちんと処理すること
 */
//------------------------------------------------------------------
static void MAPCHG_updateGameData( GAMESYS_WORK * gsys, const LOCATION * loc_req )
{
  LOCATION loc;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
  PLAYER_WORK * mywork = GAMEDATA_GetMyPlayerWork(gamedata);
  EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData(gamedata);
  FIELD_STATUS* fldstatus = GAMEDATA_GetFieldStatus(gamedata);

  { 
    char buf[ZONEDATA_NAME_LENGTH*2];
    ZONEDATA_DEBUG_GetZoneName(buf, loc_req->zone_id);
    TAMADA_Printf("MAPCHG_updateGameData:%s\n", buf);
  }

  
  //開始位置セット
  MakeNewLocation(evdata, loc_req, &loc);

  //特殊接続出入口に出た場合は、前のマップの出入口位置を記憶しておく
  if (loc.type == LOCATION_TYPE_SPID) {
    //special_location = entrance_location;
    GAMEDATA_SetSpecialLocation(gamedata, GAMEDATA_GetEntranceLocation(gamedata));
  }

  //取得したLOCATION情報を自機へ反映
  {
    u16 direction;
    PLAYERWORK_setZoneID(mywork, loc.zone_id);
    direction = GetDirValueByDirID(loc.dir_id);
    PLAYERWORK_setDirection(mywork, direction);

    if( LOCATION_GetPosType( &loc ) == LOCATION_POS_TYPE_3D ){
      PLAYERWORK_setPosition(mywork, &loc.location_pos.pos);
      PLAYERWORK_setPosType( mywork, LOCATION_POS_TYPE_3D );
    }else{
      PLAYERWORK_setRailPosition(mywork, &loc.location_pos.railpos);
      PLAYERWORK_setPosType( mywork, LOCATION_POS_TYPE_RAIL );
    }
  }

  // ISSにゾーン切り替えを通知
  {
    ISS_SYS* iss = GAMESYSTEM_GetIssSystem( gsys );
    ISS_SYS_ZoneChange( iss, loc.zone_id );
  }
  
  //開始位置を記憶しておく
  GAMEDATA_SetStartLocation(gamedata, &loc);

  //ワープ先登録
  {
    u16 warp_id;
    warp_id = WARPDATA_SearchByRoomID( loc.zone_id );
    if (warp_id) {
      GAMEDATA_SetWarpID( gamedata, warp_id );
    }
  }
  //ギミックアサイン
  AssignGimmickID(gamedata, loc.zone_id);

  //特殊スクリプト呼び出し：ゾーン切り替え
  SCRIPT_CallZoneChangeScript( gsys, HEAPID_PROC );

  //イベント時間更新
  EVTIME_Update( gamedata );

  //マップ遷移時のフラグ初期化
  FIELD_FLAGCONT_INIT_MapJump( gamedata, loc.zone_id );

  //マップ到着フラグセット
  ARRIVEDATA_SetArriveFlag( gamedata, loc.zone_id );

  //新規ゾーンに配置する動作モデルを追加
  MAPCHG_loadMMdl( gamedata, loc_req );

  //フィールド技　マップ効果
  MAPCHG_setupFieldSkillMapEff( gamedata, &loc );

  // WFBCの設定
  MAPCHG_SetUpWfbc( gamedata, &loc );

  // 天気更新
  PM_WEATHER_UpdateZoneChangeWeatherNo( gamedata, loc.zone_id );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void MAPCHG_loadMMdl( GAMEDATA * gamedata, const LOCATION *loc_req )
{
  EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData(gamedata);
  u16 count = EVENTDATA_GetNpcCount( evdata );
  
  if( count ){
    EVENTWORK *evwork =  GAMEDATA_GetEventWork( gamedata );
    MMDLSYS *fmmdlsys = GAMEDATA_GetMMdlSys( gamedata );
    const MMDL_HEADER *header = EVENTDATA_GetNpcData( evdata );
    MMDLSYS_SetMMdl( fmmdlsys, header, loc_req->zone_id, count, evwork );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  WFBCのモデルを読み込む
 */
//-----------------------------------------------------------------------------
static void MAPCHG_loadMMdlWFBC( GAMEDATA * gamedata, const LOCATION *loc )
{
  if( ZONEDATA_IsWfbc( loc->zone_id ) )
  {
    const FIELD_STATUS* cp_fs = GAMEDATA_GetFieldStatus( gamedata );
    MAPMODE mapmode = FIELD_STATUS_GetMapMode( cp_fs );
    MMDLSYS *fmmdlsys = GAMEDATA_GetMMdlSys( gamedata );
    EVENTWORK *evwork =  GAMEDATA_GetEventWork( gamedata );
    MMDL_HEADER*p_header;
    const FIELD_WFBC_CORE* cp_wfbc;
    u32  count;

    if( mapmode == MAPMODE_NORMAL )
    {
      cp_wfbc = GAMEDATA_GetMyWFBCCoreData( gamedata );
    }
    else
    {
      // 通信相手のデータに変更
      cp_wfbc = GAMEDATA_GetWFBCCoreData( gamedata, GAMEDATA_WFBC_ID_COMM );
    }

    // 人物
    {
      p_header = FIELD_WFBC_CORE_MMDLHeaderCreateHeapLo( cp_wfbc, mapmode, GFL_HEAPID_APP );

      count = FIELD_WFBC_CORE_GetPeopleNum( cp_wfbc, mapmode );


      // 
      if( p_header && (count > 0) )
      {
        TOMOYA_Printf( "WFBC MMDL SetUp\n" );
        MMDLSYS_SetMMdl( fmmdlsys, p_header, loc->zone_id, count, evwork );
        GFL_HEAP_FreeMemory( p_header );
      }
    }

    // アイテム
    {
      const FIELD_WFBC_CORE_ITEM* cp_item;

      cp_item = GAMEDATA_GetWFBCItemData( gamedata );

      // アイテム数取得
      count = WFBC_CORE_ITEM_GetNum( cp_item );

      // 配置モデル作成
      p_header = FIELD_WFBC_CORE_ITEM_MMDLHeaderCreateHeapLo( cp_item, mapmode, cp_wfbc->type, GFL_HEAPID_APP );
      
      // 
      if( p_header && (count > 0) )
      {
        TOMOYA_Printf( "WFBC ITEM SetUp\n" );
        MMDLSYS_SetMMdl( fmmdlsys, p_header, loc->zone_id, count, evwork );
        GFL_HEAP_FreeMemory( p_header );
      }
    }
  }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void MAPCHG_releaseMMdl( GAMEDATA * gamedata )
{
    MMDLSYS_DeleteMMdl( GAMEDATA_GetMMdlSys(gamedata) );
}


//----------------------------------------------------------------------------
/**
 *	@brief  WFBCのセットアップ処理
 *
 *	@param	gamedata
 *	@param	loc 
 */
//-----------------------------------------------------------------------------
static void MAPCHG_SetUpWfbc( GAMEDATA * gamedata, const LOCATION *loc )
{
  if( !ZONEDATA_IsWfbc( loc->zone_id ) )
  {
    return ;
  }

  MAPCHG_loadMMdlWFBC( gamedata, loc );

  // まちに入った！加算
  {
    FIELD_WFBC_CORE* p_wfbc;
    const FIELD_STATUS* cp_fs = GAMEDATA_GetFieldStatus( gamedata );
    MAPMODE mapmode = FIELD_STATUS_GetMapMode( cp_fs );


    if( mapmode == MAPMODE_NORMAL )
    {
      p_wfbc = GAMEDATA_GetMyWFBCCoreData( gamedata );

      // 街に入った加算
      FIELD_WFBC_CORE_CalcMoodInTown( p_wfbc );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  フィールド技　画面効果マスクの設定
 */
//-----------------------------------------------------------------------------
static void MAPCHG_setupFieldSkillMapEff( GAMEDATA * gamedata, const LOCATION *loc_req )
{
  FIELD_STATUS * fldstatus =  GAMEDATA_GetFieldStatus( gamedata ); 
  u32 fs_effmsk = ZONEDATA_GetFieldSkillMapEffMsk( loc_req->zone_id );

  //フラッシュ　システムフラグを見て、マスクを書き換える
  if( FIELDSKILL_MAPEFF_MSK_IS_ON(fs_effmsk, FIELDSKILL_MAPEFF_MSK_FLASH_NEAR) )
  {
    if( FIELD_STATUS_IsFieldSkillFlash(fldstatus) )
    {
      // フラッシュ技使用後に変換
      FIELDSKILL_MAPEFF_MSK_OFF(fs_effmsk, FIELDSKILL_MAPEFF_MSK_FLASH_NEAR);
      FIELDSKILL_MAPEFF_MSK_ON(fs_effmsk, FIELDSKILL_MAPEFF_MSK_FLASH_FAR);
      
    }
  }
  FIELD_STATUS_SetFieldSkillMapEffectMsk( fldstatus, fs_effmsk );
}

//--------------------------------------------------------------
/**
 * @brief
 * @param gamedata
 *
 * @todo  マップをまたいでor画面暗転中もずっと維持されるメモリ状態などはここで設定
 * @todo  下記の※checkの対処を行う
 */
//--------------------------------------------------------------
static void MAPCHG_setupMapTools( GAMESYS_WORK * gsys, const LOCATION * loc_req )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
  EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData(gamedata);
  //MAPCHGモジュール群はFieldMapが存在しないときに呼ばれる
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  GF_ASSERT( fieldmap == NULL );

  //※check　スクリプトでマップ作成前に実行できるタイミングが出来れば、そこで行うようにしたい
//  if(ZONEDATA_IsPalaceField(loc_req->zone_id) || ZONEDATA_IsBingo(loc_req->zone_id)){
  if(GAMEDATA_GetIntrudeReverseArea(gamedata) == TRUE){
    FIELD_STATUS_SetMapMode( GAMEDATA_GetFieldStatus( gamedata ), MAPMODE_INTRUDE );
    //GAMEDATA_SetMapMode(gamedata, MAPMODE_INTRUDE);
  }
  else{
    FIELD_STATUS_SetMapMode( GAMEDATA_GetFieldStatus( gamedata ), MAPMODE_NORMAL );
  }
  //※check　ユニオンルームへの移動を受付スクリプトで制御するようになったらサブスクリーンモードの
  //         変更もそのスクリプト内で行うようにする
  switch(loc_req->zone_id){
  case ZONE_ID_UNION:
    GAMEDATA_SetSubScreenMode(gamedata, FIELD_SUBSCREEN_UNION);
    break;
  case ZONE_ID_CLOSSEUM:
  case ZONE_ID_CLOSSEUM02:
    GAMEDATA_SetSubScreenMode(gamedata, FIELD_SUBSCREEN_NOGEAR);
    break;
  default:
    GAMEDATA_SetSubScreenMode(gamedata, FIELD_SUBSCREEN_NORMAL);
    break;
  }

  //イベント起動データの読み込み
  EVENTDATA_SYS_Load(evdata, loc_req->zone_id, GAMEDATA_GetSeasonID(gamedata) );

  // イベント追加登録

  //マトリックスデータの更新
  {
    MAP_MATRIX * matrix = GAMEDATA_GetMapMatrix( gamedata );
    u16 matID = ZONEDATA_GetMatrixID( loc_req->zone_id );
    MAP_MATRIX_Init( matrix, matID, loc_req->zone_id );
    MAP_MATRIX_CheckReplace( matrix, gamedata );
  }

  //フィールド技 マップ効果
  MAPCHG_setupFieldSkillMapEff( gamedata, loc_req );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void MAPCHG_releaseMapTools( GAMESYS_WORK * gsys )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
  //MAPCHGモジュール群はFieldMapが存在しないときに呼ばれる
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  GF_ASSERT( fieldmap == NULL );

  //配置していた動作モデルを削除
  MAPCHG_releaseMMdl( gamedata );

}

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
void MAPCHANGE_setPlayerVanish(FIELDMAP_WORK * fieldmap, BOOL vanish_flag)
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
  MMDL *fmmdl = FIELD_PLAYER_GetMMdl( player );
  MMDL_SetStatusBitVanish( fmmdl, vanish_flag );
}

//---------------------------------------------------------------------------
/**
 * @brief ギミックのアサイン
 * 
 * @param   inZoneID    ゾーンＩＤ
 * 
 * @return  none
 */
//---------------------------------------------------------------------------
static void AssignGimmickID(GAMEDATA * gamedata, int inZoneID)
{
  typedef struct GMK_ASSIGN_DATA_tag
  {
    u32 ZoneID;
    u16 ResNum;
    u16 ObjNum;
  }GMK_ASSIGN_DATA;

  GIMMICKWORK *work;
  //ギミックワーク取得
  work = GAMEDATA_GetGimmickWork(gamedata);

  //マップジャンプのときのみ初期化する（歩いてゾーンが切り替わった場合は初期化しない）
  GIMMICKWORK_Init(work);

  //ギミックデータ検索
  {
    u32 num, i;
    ARCHANDLE *handle;
    GMK_ASSIGN_DATA *data;
    handle = GFL_ARC_OpenDataHandle( ARCID_GIMMICK_ASSIGN, GFL_HEAP_LOWID(GFL_HEAPID_APP) );
    data = (GMK_ASSIGN_DATA *)GFL_ARC_LoadDataAllocByHandle( handle, 0, GFL_HEAP_LOWID(GFL_HEAPID_APP) );

    num = GFL_ARC_GetDataSizeByHandle( handle, 0 ) / sizeof(GMK_ASSIGN_DATA);

//    OS_Printf("gimmick_num = %d\n",num);

    for (i=0;i<num;i++){
      if ( data[i].ZoneID == inZoneID){
        //ギミック発見。アサインする
        GIMMICKWORK_Assign(work, i+1);
        break;
      }
    }

    GFL_HEAP_FreeMemory( data );
    GFL_ARC_CloseDataHandle( handle );
  }
}

