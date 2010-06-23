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
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "gamesystem\btl_setup.h"
#include "poke_tool/poke_regulation.h"

#include "savedata/save_tbl.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h" //SYS_FLAG_SPEXIT_REQUEST
#include "fieldmap.h"
#include "field_player.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "event_battle.h"   //FIELD_BATTLE_IsLoseResult

#include "bsubway_scr.h"
#include "bsubway_scrwork.h"
#include "scrcmd_bsubway.h"

#include "event_bsubway.h"

#include "fldmmdl.h"

#include "field_gimmick_bsubway.h"

#include "../../../resource/fldmapdata/zonetable/zone_id.h"
#include "../../../resource/fldmapdata/script/c04r0111_def.h"
#include "msg/script/msg_c04r0111.h"

#include "gamesystem/iss_switch_sys.h"

#include "event_wifi_bsubway.h"

#include "savedata/battle_box_save.h"
#include "savedata/battle_rec.h"

#include "field/eventdata_type.h"

#include "net_app/btl_rec_sel.h"
FS_EXTERN_OVERLAY(btl_rec_sel);

#include "net_app/irc_match.h"
FS_EXTERN_OVERLAY(ircbattlematch);
extern const GFL_PROC_DATA IrcBattleMatchProcData;

#include "savedata/etc_save.h"

#include "item/itemsym.h"

#include "system/net_err.h"

//======================================================================
//  define
//======================================================================
#define HOME_NPC_WIFI_MAX (5) ///<WiFiホーム NPC最大数
#define HOME_NO_MAX (22)

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// HOME_NPC_DATA 途中駅NPCデータ
//--------------------------------------------------------------
typedef struct
{
  u16 code;
  u16 npc_home_no;
  s16 gx;
  s16 gy;
  s16 gz;
  u16 msg_id;
  u16 msg_id_item_after;
  u16 item_no;
}HOME_NPC_DATA;

//======================================================================
//  proto
//======================================================================
static BOOL evCommSendData( VMHANDLE *core, void *wk );
static BOOL evCommTimingSync( VMHANDLE *core, void *wk );
static BOOL evCommEntryMenuPerent( VMHANDLE *core, void *wk );
static BOOL evCommEntryMenuChild( VMHANDLE *core, void *wk );
static BOOL evCommRecvData( VMHANDLE *core, void *wk );

static BOOL evBtlRecSave( VMHANDLE *core, void *wk );

static GMEVENT * createEvCommExit(
    GAMESYS_WORK *gsys, BSUBWAY_SCRWORK *bsw_scr );

static GMEVENT * createYesNoEvent( GAMESYS_WORK *gsys,
    SCRCMD_WORK *work, u16 *ret_wk, FLDMENUFUNC_YESNO cursor_pos );

static void bsway_SetHomeNPC(
    BSUBWAY_SCRWORK *bsw_scr, GAMEDATA *gdata,
    MMDLSYS *mmdlsys, FIELDMAP_WORK *fieldmap );
static u16 bsway_GetHomeNPCMsgID( const MMDL *mmdl );
static BOOL bsway_CheckHomeNPCItemMsg(
    BSUBWAY_SCRWORK *bsw_scr, int npc_no );
static void bsway_GetHomeNPCItemData(
    BSUBWAY_SCRWORK *bsw_scr, u16 npc_no,
    u16 *msg_before, u16 *msg_after, u16 *item_no );

static u32 bsw_getRegulationLabel( u32 play_mode );
static u32 bsw_GetNowMapPlayMode( FIELDMAP_WORK *fieldmap );

static const FLDEFF_BTRAIN_TYPE data_TrainModeType[BSWAY_MODE_MAX];
static const VecFx32 data_TrainPosTbl[BTRAIN_POS_MAX];
static const u32 data_PlayModeZoneID[BSWAY_MODE_MAX];
static const VecFx32 data_PlayModeRecoverPos[BSWAY_MODE_MAX];
static const u16 data_ModeBossClearFlag[BSWAY_MODE_MAX];
static const u16 data_ModeBattleMode[BSWAY_MODE_MAX];
static const u32 data_NpcHomeNoToStageNo[HOME_NO_MAX];
const HOME_NPC_DATA data_HomeNpcTbl[];
static const u16 data_HomeNpcTbl_WifiMan[10];
static const u16 data_HomeNpcTbl_WifiWoman[10];
static const MMDL_GRIDPOS data_HomeNpcWiFiPosTbl[HOME_NPC_WIFI_MAX];

#ifdef DEBUG_ONLY_FOR_kagaya
const u32 d_data_NpcHomeNoTbl[];
#endif

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
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  u16 init = VMGetU16(core);
  u16 mode = SCRCMD_GetVMWorkValue( core, work );
  BSUBWAY_SCRWORK *bsw_scr = BSUBWAY_SCRWORK_CreateWork( gsys, init, mode );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 *  @brief  バトルサブウェイ制御用ワークポインタ初期化
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBSubwayWorkClear( VMHANDLE* core, void *wk )
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
VMCMD_RESULT EvCmdBSubwayWorkRelease( VMHANDLE *core, void *wk )
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
  VMCMD_RESULT result = VMCMD_RESULT_CONTINUE;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  EVENTWORK *event = GAMEDATA_GetEventWork( gdata );
  BSUBWAY_SCRWORK *bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  BSUBWAY_PLAYDATA *playData =
    SaveControl_DataPtrGet( save, GMDATA_ID_BSUBWAY_PLAYDATA );
  u32 play_mode = (u32)BSUBWAY_PLAYDATA_GetData(
        playData, BSWAY_PLAYDATA_ID_playmode, NULL );
  BSUBWAY_SCOREDATA *scoreData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_SCOREDATA );
  BSUBWAY_WIFI_DATA *wifiData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_WIFIDATA );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  EV_WIFIBSUBWAY_PARAM wifibsubway_param;
  GMEVENT* next_event;
  u16 cmd_id = VMGetU16( core );
  u16 param0 = SCRCMD_GetVMWorkValue( core, work );
  u16 param1 = SCRCMD_GetVMWorkValue( core, work );
  u16 retwk_id = VMGetU16( core );
  u16 *ret_wk = SCRIPT_GetEventWork( sc, gdata, retwk_id );

#ifdef DEBUG_ONLY_FOR_kagaya
  {
    int i;
    const HOME_NPC_DATA *data = data_HomeNpcTbl;
    
    for( i = 0; data->code != OBJCODEMAX; i++, data++ ){
      if( data->npc_home_no != d_data_NpcHomeNoTbl[i] ){
        OS_Printf(
            "BSW データ異常 NPC_No.%dのホーム番号(%d)は正しくは%dです\n",
            i, data->npc_home_no, d_data_NpcHomeNoTbl[i] );
      }
    }
  }
#endif

#ifdef DEBUG_ONLY_FOR_kagaya  
  if( cmd_id >= BSWTOOL_START_NO && cmd_id < BSWTOOL_END_NO )
  {
    KAGAYA_Printf( "BSUBWAY_TOOL() cmd type BSWTOOL : cmd No %d\n",
        cmd_id - BSWTOOL_START_NO );
  }
  else if(
      cmd_id >= BSWTOOL_WIFI_START_NO && cmd_id < BSWTOOL_WIFI_END_NO )
  {
    KAGAYA_Printf( "BSUBWAY_TOOL() cmd type BSWTOOL_WIFI : cmd No %d\n",
        cmd_id - BSWTOOL_WIFI_START_NO );
  }
  else if(
      cmd_id >= BSWTOOL_DEBUG_START_NO && cmd_id < BSWTOOL_DEBUG_END_NO  )
  {
    KAGAYA_Printf( "BSUBWAY_TOOL() cmd type BSWTOOL_DEBUG : cmd No %d\n",
        cmd_id - BSWTOOL_DEBUG_START_NO );
  }
  else if(
      cmd_id >= BSWSUB_START_NO && cmd_id < BSWSUB_END_NO )
  {
    KAGAYA_Printf( "BSUBWAY_TOOL() cmd type BSWSUB : cmd No %d\n",
        cmd_id - BSWSUB_START_NO );
  }
  else if(
      cmd_id >= BSWSUB_COMM_START_NO && cmd_id < BSWSUB_COMM_END_NO )
  {
    KAGAYA_Printf( "BSUBWAY_TOOL() cmd type BSWSUB_COMM : cmd No %d\n",
        cmd_id - BSWSUB_COMM_START_NO );
  }
  else if(
      cmd_id >= BSWSUB_DEBUG_START_NO && cmd_id < BSWSUB_DEBUG_END_NO )
  {
    KAGAYA_Printf( "BSUBWAY_TOOL() cmd type BSWSUB_DEBUG : cmd No %d\n",
        cmd_id - BSWSUB_DEBUG_START_NO );
  }
  else
  {
    GF_ASSERT( 0 );
  }
#endif
  
  //バグチェック　ワーク依存コマンド
  if( bsw_scr == NULL && cmd_id >= BSWSUB_START_NO ){
    GF_ASSERT( 0 );
    return( VMCMD_RESULT_CONTINUE );
  }
  
  switch( cmd_id ){
  //ゾーンID別プレイモード取得
  case BSWTOOL_GET_ZONE_PLAY_MODE:
    *ret_wk = bsw_GetNowMapPlayMode( fieldmap );
    break;
  //リセット
  case BSWTOOL_SYSTEM_RESET:
    OS_ResetSystem( 0 );
    break;
  //プレイデータクリア
  case BSWTOOL_CLEAR_PLAY_DATA:
    BSUBWAY_PLAYDATA_Init( playData );
    break;
  //セーブされているか
  case BSWTOOL_IS_SAVE_DATA_ENABLE:
    *ret_wk = BSUBWAY_PLAYDATA_GetSaveFlag( playData );
    break;
  //復帰位置設定
  case BSWTOOL_PUSH_NOW_LOCATION:
    {
      u16 dir,exit_dir;
      VecFx32 pos;
      LOCATION loc;
      FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
      
      dir = FIELD_PLAYER_GetDir( fld_player );
      FIELD_PLAYER_GetPos( fld_player, &pos );
      
      switch( dir ){
      case DIR_UP:
        exit_dir = EXIT_DIR_UP;
        break;
      case DIR_DOWN:
        exit_dir = EXIT_DIR_DOWN;
        break;
      case DIR_LEFT:
        exit_dir = EXIT_DIR_LEFT;
        break;
      case DIR_RIGHT:
        exit_dir = EXIT_DIR_RIGHT;
        break;
      default:
        GF_ASSERT( 0 );
        exit_dir = 0;
        break;
      }
      
      LOCATION_SetDirect( &loc, FIELDMAP_GetZoneID(fieldmap),
            exit_dir, pos.x, pos.y, pos.z );
      
      GAMEDATA_SetSpecialLocation( gdata, &loc );
      EVENTWORK_SetEventFlag( event, SYS_FLAG_SPEXIT_REQUEST );
      
      KAGAYA_Printf("復帰位置　方向=%d\n",FIELD_PLAYER_GetDir(fld_player));
    }
    break;
  //復帰位置無効
  case BSWTOOL_POP_NOW_LOCATION:
    EVENTWORK_ResetEventFlag( event, SYS_FLAG_SPEXIT_REQUEST );
    break;
  //セーブされている連勝数を取得
  case BSWTOOL_GET_SAVE_RENSHOU:
    *ret_wk = BSUBWAY_SCOREDATA_GetRenshou( scoreData, param0 );
    
    if( *ret_wk > BSW_RENSHOU_MAX ){
      *ret_wk = BSW_RENSHOU_MAX;
    }
    break;
  //エラー時のスコアセット
  case BSWTOOL_SET_NG_SCORE:
    *ret_wk = BSUBWAY_SCRWORK_SetNGScore( gsys );
    break;
  //連勝数取得
#if 0
  case BSWTOOL_GET_RENSHOU_CNT:
    *ret_wk = BSUBWAY_SCOREDATA_GetRenshou( scoreData, param0 );
    
    if( *ret_wk > BSW_RENSHOU_MAX ){
      *ret_wk = BSW_RENSHOU_MAX;
    }
    break;
#endif
  //自機OBJコード取得
  case BSWTOOL_GET_MINE_OBJ:
    {
      FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
      int sex = FIELD_PLAYER_GetSex( fld_player );
      *ret_wk = FIELD_PLAYER_GetDrawFormToOBJCode(
          sex, PLAYER_DRAW_FORM_NORMAL );
    }
    break;
  //OBJ表示、非表示
  case BSWTOOL_OBJ_VANISH:
    {
      u16 id = param0;
      u16 vanish = param1;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
      if( mmdl == NULL ){
        GF_ASSERT_MSG( 0, "BSW MMDL VANISH ERROR OBJID %d\n", id );
      }else{
        MMDL_SetStatusBitVanish( mmdl, vanish );
      }
    }
    break;
  //現在ラウンド数取得
  case BSWTOOL_GET_NOW_ROUND:
    *ret_wk = BSUBWAY_PLAYDATA_GetRoundNo( playData ) + 1;
    break;
  //ラウンド数増加
  #if 0
  case BSWTOOL_INC_ROUND:
    BSUBWAY_PLAYDATA_IncRoundNo( playData );
    BSUBWAY_SCRWORK_IncNowRenshou( bsw_scr );
    *ret_wk = BSUBWAY_SCOREDATA_GetRenshou( scoreData, play_mode );
    if( (*ret_wk) > BSW_RENSHOU_MAX ){
      *ret_wk = BSW_RENSHOU_MAX;
    }
    break;
  #endif
  //次のラウンド数取得
  case BSWTOOL_GET_NEXT_ROUND:
    *ret_wk = BSUBWAY_PLAYDATA_GetRoundNo( bsw_scr->playData );
    (*ret_wk) += 1; //0 org
    break;
  //ボスクリア済みフラグ取得
  case BSWTOOL_GET_BOSS_CLEAR_FLAG:
    {
      u16 flag = data_ModeBossClearFlag[param0];
      
      if( flag == BSWAY_SCOREDATA_FLAG_MAX ){
        GF_ASSERT( 0 );
        *ret_wk = 1;
      }else{
        *ret_wk = BSUBWAY_SCOREDATA_SetFlag(
            scoreData, flag, BSWAY_SETMODE_get );
      }
    }
    break;
  //ボスクリア済みフラグセット
  case BSWTOOL_SET_BOSS_CLEAR_FLAG:
    {
      u16 flag = data_ModeBossClearFlag[param0];
      
      if( flag == BSWAY_SCOREDATA_FLAG_MAX ){
        GF_ASSERT( 0 );
      }else{
        BSUBWAY_SCOREDATA_SetFlag( scoreData, flag, BSWAY_SETMODE_set );
      }
    }
    break;
  //サポート遭遇フラグ取得
  case BSWTOOL_GET_SUPPORT_ENCOUNT_END:
    *ret_wk = BSUBWAY_SCOREDATA_SetFlag( scoreData,
        BSWAY_SCOREDATA_FLAG_SUPPORT_ENCOUNT_END,
        BSWAY_SETMODE_get );
    break;
  //サポート遭遇フラグセット
  case BSWTOOL_SET_SUPPORT_ENCOUNT_END:
    BSUBWAY_SCOREDATA_SetFlag( scoreData,
        BSWAY_SCOREDATA_FLAG_SUPPORT_ENCOUNT_END,
        BSWAY_SETMODE_set );
    break;
  //列車配置 param0 = 種類(modeから変換) param1 = 座標種類
  case BSWTOOL_SET_TRAIN:
    {
      const VecFx32 *pos;
      FLDEFF_BTRAIN_TYPE type;
      type = data_TrainModeType[param0];
      pos = &data_TrainPosTbl[param1];
      BSUBWAY_GIMMICK_SetTrain( fieldmap, type, pos );
    }
    break;
  //列車アニメ
  case BSWTOOL_SET_TRAIN_ANM:
    {
      FLDEFF_TASK *task = BSUBWAY_GIMMICK_GetTrainTask( fieldmap );
      FLDEFF_BTRAIN_SetAnime( task, param0 );
    }
    break;
  //列車非表示
  case BSWTOOL_SET_TRAIN_VANISH:
    {
      FLDEFF_TASK *task = BSUBWAY_GIMMICK_GetTrainTask( fieldmap );
      FLDEFF_BTRAIN_SetVanishFlag( task, param0 );
    }
    break;
  //BSUBWAY_PLAYDATAからプレイモード取得
  case BSWTOOL_GET_DATA_PLAY_MODE:
    {
      *ret_wk = (u16)BSUBWAY_PLAYDATA_GetData(
        playData, BSWAY_PLAYDATA_ID_playmode, NULL );
    }
    break;
  //指定モードがマルチかどうか
  case BSWTOOL_CHK_MODE_MULTI:
    *ret_wk = FALSE;
    switch( param0 ){
    case BSWAY_MODE_MULTI:
    case BSWAY_MODE_COMM_MULTI:
    case BSWAY_MODE_S_MULTI:
    case BSWAY_MODE_S_COMM_MULTI:
      *ret_wk = TRUE;
      break;
    }
    break;
  //指定OBJの高さ取得をOFF
  case BSWTOOL_OBJ_HEIGHT_OFF:
    {
      u16 id = param0;
      fx32 height = GRID_SIZE_FX32( param1 );
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
      GF_ASSERT( mmdl != NULL );
      if( mmdl != NULL ){
        VecFx32 pos;
        MMDL_GetVectorPos( mmdl, &pos );
        pos.y = height;
        MMDL_InitPosition( mmdl, &pos, MMDL_GetDirDisp(mmdl) );
        MMDL_SetStatusBitHeightGetOFF( mmdl, TRUE );
      }
    }
    break;
  //指定OBJの高さ取得をON
  case BSWTOOL_OBJ_HEIGHT_ON:
    {
      u16 id = param0;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
      GF_ASSERT( mmdl != NULL );
      if( mmdl != NULL ){
        MMDL_SetStatusBitHeightGetOFF( mmdl, FALSE );
      }
    }
    break;
  //指定OBJが存在しているかチェック
  case BSWTOOL_CHK_EXIST_OBJ:
    {
      u16 id = param0;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
      *ret_wk = FALSE;
      if( mmdl != NULL ){
        *ret_wk = TRUE;
      }
    }
    break;
  //途中駅NPCのメッセージID取得
  case BSWTOOL_GET_HOME_NPC_MSGID:
    {
      u16 id = param0;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
      *ret_wk = 0;
      GF_ASSERT( mmdl != NULL );
      
      if( mmdl != NULL ){
        *ret_wk = bsway_GetHomeNPCMsgID( mmdl );
      }
    }
    break;
  //Wifiランク取得
  case BSWTOOL_GET_WIFI_RANK:
    *ret_wk = BSUBWAY_SCRWORK_SetWifiRank(
            scoreData, gsys, BSWAY_SETMODE_get );
    
    if( (s16)*ret_wk <= 0 ){
      *ret_wk = 1;
    }
    break;
  //WiFiランクアップ
  case BSWTOOL_UP_WIFI_RANK:
    *ret_wk = BSUBWAY_SCRWORK_SetWifiRank(
        scoreData, gsys, BSWAY_SETMODE_inc );
    break;
  //Wifiランクダウン
  case BSWTOOL_DOWN_WIFI_RANK:
    *ret_wk = BSUBWAY_SCRWORK_SetWifiRank(
            scoreData, gsys, BSWAY_SETMODE_dec );
    break;
  //ステージ数取得
  case BSWTOOL_GET_STAGE_NO:
    *ret_wk = BSUBWAY_SCOREDATA_GetStageNo_Org0( scoreData, param0 );
    break;
  //パートナーOBJコード取得
  case BSWTOOL_GET_OBJCODE_PARTNER:
    {
      PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
      u32 sex = MyStatus_GetMySex( &player->mystatus );
      
      if( sex == PM_MALE ){
        sex = PM_FEMALE;
        KAGAYA_Printf( "パートナーOBJコード取得 パートナー女\n" );
      }else{
        sex = PM_MALE;
        KAGAYA_Printf( "パートナーOBJコード取得 パートナー男\n" );
      }
      
      *ret_wk = FIELD_PLAYER_GetMoveFormToOBJCode(
          sex, PLAYER_MOVE_FORM_NORMAL );
    }
    break;
  //バトルボックスにポケモンがいるか
  case BSWTOOL_CHK_BTL_BOX_IN:
    {
      BATTLE_BOX_SAVE *bb_save = BATTLE_BOX_SAVE_GetBattleBoxSave( save );
      
      *ret_wk = FALSE;
      
      if( BATTLE_BOX_SAVE_IsIn(bb_save) == TRUE ){
        *ret_wk = TRUE;
      }
    }
    break;
  //デバッグROMか
  case BSWTOOL_CHK_DEBUG_ROM:
    #ifdef PM_DEBUG
    *ret_wk = TRUE;
    #else
    *ret_wk = FALSE;
    #endif
    break;
  //列車配置 トレインタウン専用
  case BSWTOOL_SET_TTOWN_TRAIN:
    {
      const VecFx32 *pos;
      FLDEFF_BTRAIN_TYPE type;
      type = FLDEFF_BTRAIN_TYPE_08;
      pos = &data_TrainPosTbl[0];
      BSUBWAY_GIMMICK_SetTrain( fieldmap, type, pos );
    }
    break;
  //OBJの向きをセットする
  case BSWTOOL_SET_OBJ_DIR:
    {
      u16 obj_id = param0;
      u16 dir = param1;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
      GF_ASSERT( mmdl != NULL );
      
      if( mmdl != NULL ){
        GF_ASSERT( dir < DIR_MAX4 );
        MMDL_SetDirDisp( mmdl, dir );
      }
    }
    break;
  //OBJをポーズする
  case BSWTOOL_OBJ_PAUSE:
    {
      u16 obj_id = param0;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
      GF_ASSERT( mmdl != NULL );
      
      if( mmdl != NULL ){
        MMDL_OnMoveBitMoveProcPause( mmdl );
      }
    }
    break;
  //WiFi ホームNPCの会話メッセージ表示
  case BSWTOOL_WIFI_HOME_NPC_MSG:
    {
      u16 obj_id = param0;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
      u16 leader_no = MMDL_GetParam( mmdl, MMDL_PARAM_0 );
      
      SCRIPT_CallEvent(
          sc, BSUBWAY_EVENT_MsgWifiHomeNPC(gsys,leader_no,obj_id) );
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //ステージ数が存在するかチェック
  case BSWTOOL_CHK_EXIST_STAGE:
    {
      *ret_wk = BSUBWAY_SCOREDATA_CheckExistStageNo( scoreData, param0 );
    }
    break;
  //アイテムをくれるNPCかチェック
  //戻り値TRUEでくれる人
  case BSWTOOL_CHK_HOME_NPC_GETITEM:
    {
      u16 npc_no;
      u16 id = param0;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
      npc_no = MMDL_GetParam( mmdl, MMDL_PARAM_0 );
      
      *ret_wk = FALSE;
      
      if( bsway_CheckHomeNPCItemMsg(bsw_scr,npc_no) == TRUE ){
        *ret_wk = TRUE;
      }
    }
    break;
  //アイテムをくれるNPCのmsg id取得
  case BSWTOOL_GET_HOME_NPC_GETITEM_MSGID_BEFORE:
    {
      u16 npc_no,msg0,msg1,item_no;
      u16 id = param0;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
      npc_no = MMDL_GetParam( mmdl, MMDL_PARAM_0 );
      
      bsway_GetHomeNPCItemData( bsw_scr, npc_no,
        &msg0, &msg1, &item_no );
      *ret_wk = msg0;
    }
    break;
  //アイテムをくれるNPCのmsg id取得
  case BSWTOOL_GET_HOME_NPC_GETITEM_MSGID_AFTER:
    {
      u16 npc_no,msg0,msg1,item_no;
      u16 id = param0;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
      npc_no = MMDL_GetParam( mmdl, MMDL_PARAM_0 );
      
      bsway_GetHomeNPCItemData( bsw_scr, npc_no,
        &msg0, &msg1, &item_no );
      *ret_wk = msg1;
    }
    break;
  //アイテムをくれるNPCのアイテムナンバー取得
  case BSWTOOL_GET_HOME_NPC_GETITEM_ITEMNO:
    {
      u16 npc_no,msg0,msg1,item_no;
      u16 id = param0;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
      npc_no = MMDL_GetParam( mmdl, MMDL_PARAM_0 );
      
      bsway_GetHomeNPCItemData( bsw_scr, npc_no,
        &msg0, &msg1, &item_no );
      *ret_wk = item_no;
    }
    break;
  //歴代トレーナーデータの格納されている人数を返す
  case BSWTOOL_GET_LEADERDATA_DATANUM:
    {
      BSUBWAY_LEADER_DATA *pLeader =
        BSUBWAY_WIFIDATA_GetLeaderDataAlloc( wifiData, HEAPID_PROC );
      *ret_wk = (u16)BSUBWAY_LEADERDATA_GetDataNum( pLeader );
      GFL_HEAP_FreeMemory( pLeader );
    }
    break;
  //はい、いいえウィンドウ param0 = カーソル初期位置 SCR_YES,SCR_NO
  case BSWTOOL_YESNO_WIN:
    {
      FLDMENUFUNC_YESNO cursor_pos = FLDMENUFUNC_YESNO_YES;
      
      if( param0 == SCR_NO ){
        cursor_pos = FLDMENUFUNC_YESNO_NO;
      }
      
      SCRIPT_CallEvent( sc, createYesNoEvent(gsys,work,ret_wk,cursor_pos) );
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //現在の受付マップ別に復帰位置セット
  case BSWTOOL_SET_RECEIPT_MAP_LOCATION:
    {
      LOCATION loc;
      u16 map_play_mode = bsw_GetNowMapPlayMode( fieldmap );
      u32 zone_id = data_PlayModeZoneID[map_play_mode];
      const VecFx32 *pos = &data_PlayModeRecoverPos[map_play_mode];
      LOCATION_SetDirect( &loc, zone_id, EXIT_DIR_RIGHT,
          pos->x, pos->y, pos->z );
      GAMEDATA_SetSpecialLocation( gdata, &loc );
      EVENTWORK_SetEventFlag( event, SYS_FLAG_SPEXIT_REQUEST );
    }
    break;
  //----TOOL Wifi関連
  //Wifiアップロードフラグをセット
  case BSWTOOL_WIFI_SET_UPLOAD_FLAG:
    if( param0 == 0 ){ //リセット
      BSUBWAY_SCOREDATA_SetFlag( scoreData,
          BSWAY_SCOREDATA_FLAG_WIFI_UPLOAD, BSWAY_SETMODE_reset );
    }else{ //セット
      BSUBWAY_SCOREDATA_SetFlag( scoreData,
          BSWAY_SCOREDATA_FLAG_WIFI_UPLOAD, BSWAY_SETMODE_set );
    }
    break;
  //Wifiアップロードフラグを取得
  case BSWTOOL_WIFI_GET_UPLOAD_FLAG:
    *ret_wk = BSUBWAY_SCOREDATA_SetFlag(
          scoreData, BSWAY_SCOREDATA_FLAG_WIFI_UPLOAD, BSWAY_SETMODE_get );
    break;
  //Wifi接続
  case BSWTOOL_WIFI_CONNECT:
    #if 0
    EventCmd_BTowerWifiCall(core->event_work,param,retwk_id,*ret_wk);
    return 1;
    #else
    GF_ASSERT( 0 && "BSWTOOL_WIFI_CONNECT NOT SUPPORT WB" );
    #endif
    break;
  //WiFi 前回の記録をアップロード
  case BSWTOOL_WIFI_UPLOAD_SCORE:
    wifibsubway_param.mode = WIFI_BSUBWAY_MODE_SCORE_UPLOAD;
    wifibsubway_param.ret_wk = ret_wk;

    next_event = GMEVENT_CreateOverlayEventCall( gsys, 
              FS_OVERLAY_ID(event_wifibsubway), 
              WIFI_BSUBWAY_EVENT_CallStart, &wifibsubway_param );
    SCRIPT_CallEvent( sc, next_event );
    result = VMCMD_RESULT_SUSPEND;
    break;
  //WiFi ゲーム情報をダウンロード
  case BSWTOOL_WIFI_DOWNLOAD_GAMEDATA:
    wifibsubway_param.mode = WIFI_BSUBWAY_MODE_GAMEDATA_DOWNLOAD;
    wifibsubway_param.ret_wk = ret_wk;

    next_event = GMEVENT_CreateOverlayEventCall( gsys, 
              FS_OVERLAY_ID(event_wifibsubway), 
              WIFI_BSUBWAY_EVENT_CallStart, &wifibsubway_param );
    SCRIPT_CallEvent( sc, next_event );
    result = VMCMD_RESULT_SUSPEND;
    break;
  //WiFi 歴代情報をダウンロード
  case BSWTOOL_WIFI_DOWNLOAD_SCDATA:
    wifibsubway_param.mode = WIFI_BSUBWAY_MODE_SUCCESSDATA_DOWNLOAD;
    wifibsubway_param.ret_wk = ret_wk;

    next_event = GMEVENT_CreateOverlayEventCall( gsys, 
              FS_OVERLAY_ID(event_wifibsubway), 
              WIFI_BSUBWAY_EVENT_CallStart, &wifibsubway_param );
    SCRIPT_CallEvent( sc, next_event );
    result = VMCMD_RESULT_SUSPEND;
    break;
  //WiFi ランク取得
  case BSWTOOL_WIFI_GET_RANK:
    *ret_wk = BSUBWAY_WIFIDATA_GetPlayerRank( wifiData );
    break;
  //WiFi ダウンロードデータがあるか
  case BSWTOOL_WIFI_CHK_DL_DATA:
    *ret_wk = BSUBWAY_WIFIDATA_CheckPlayerDataEnable( wifiData );
    break;
  //WiFi  ダウンロードプレイを行うかセット
  case BSWTOOL_WIFI_SET_SEL_DL_BTL:
    {
      u8 buf = param0;
      BSUBWAY_PLAYDATA_SetData( playData,
        BSWAY_PLAYDATA_ID_sel_wifi_dl_play, &buf );
    }
    break;
  //WiFi  ダウンロードプレイを行うかチェック
  case BSWTOOL_WIFI_CHK_SEL_DL_BTL:
    {
      *ret_wk = (u16)BSUBWAY_PLAYDATA_GetData( playData,
        BSWAY_PLAYDATA_ID_sel_wifi_dl_play, NULL );
    }
    break;
  //WiFi　歴代情報ダウンロード済みか
  case BSWTOOL_WIFI_CHK_DL_SCDATA:
    {
      *ret_wk = BSUBWAY_WIFIDATA_CheckLeaderDataEnable( wifiData );
    }
    break;
  //WiFi　歴代情報閲覧イベント呼び出し
  case BSWTOOL_WIFI_EV_READ_SCDATA:
    {
      SCRIPT_CallEvent( sc, BSUBWAY_EVENT_CallLeaderBoard(gsys) );
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //WiFi DLした歴代情報のランク取得
  case BSWTOOL_WIFI_GET_DL_SCDATA_RANK:
    *ret_wk = BSUBWAY_WIFIDATA_GetLeaderRank( wifiData );
    break;
  //WiFi DLした歴代情報の部屋番号取得
  case BSWTOOL_WIFI_GET_DL_SCDATA_ROOM:
    *ret_wk = BSUBWAY_WIFIDATA_GetLeaderRoomNo( wifiData );
    break;
  //----TOOL DEBUG
  //DEBUG フラグチェック
  case BSWTOOL_DEBUG_CHK_FLAG:
    #ifdef PM_DEBUG
    {
      u8 flag = BSUBWAY_SCOREDATA_DEBUG_GetFlag( scoreData );
      u16 check_flag = param0;
      
      *ret_wk = FALSE;
      
      if( check_flag && check_flag <= 0xff ){
        if( (flag & check_flag) ){
          *ret_wk = TRUE;
        }
      }else{
        GF_ASSERT( 0 );
      }
    }
    #else
    *ret_wk = FALSE;
    #endif
    break;
  //DEBUG 数値出力
  case BSWTOOL_DEBUG_PRINT_NO:
    #ifdef PM_DEBUG
    {
      KAGAYA_Printf( "========BSW DEBUG PRINT NO %d=======\n", param0 );
    }
    #endif
    break;
  //DEBUG HEAPID_PROC空きサイズ出力
  case BSWTOOL_DEBUG_PRINT_HEAP_SIZE:
    #ifdef PM_DEBUG
    {
      KAGAYA_Printf( "========BSW_DEBUG_PRINT_HEAPID_PROC_FREE 0x%x:No.%d========\n",
        GFL_HEAP_GetHeapFreeSize(HEAPID_PROC), param0 );
      GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_PROC );
    }
    #endif
    break;
  //デバッグROMか
  case BSWTOOL_DEBUG_CHK_DEBUG_ROM:
    #ifdef PM_DEBUG
    *ret_wk = TRUE;
    #else
    *ret_wk = FALSE;
    #endif
    break;
  //----ワーク依存
  //プレイモード別復帰位置セット
  case BSWSUB_SET_PLAY_MODE_LOCATION:
    if( play_mode < BSWAY_MODE_MAX ){
      LOCATION loc;
      u32 zone_id = data_PlayModeZoneID[play_mode];
      const VecFx32 *pos = &data_PlayModeRecoverPos[play_mode];
      LOCATION_SetDirect( &loc, zone_id, EXIT_DIR_RIGHT,
          pos->x, pos->y, pos->z );
      GAMEDATA_SetSpecialLocation( gdata, &loc );
      EVENTWORK_SetEventFlag( event, SYS_FLAG_SPEXIT_REQUEST );
    }else{
      GF_ASSERT( 0 );
    }
    break;
  //ポケモン選択画面へ
  case BSWSUB_SELECT_POKE:
    {
      u8 buf;
      buf = BSUBWAY_PLAYDATA_GetData( playData,
          BSWAY_PLAYDATA_ID_use_battle_box, NULL );
      SCRIPT_CallEvent(
          sc, BSUBWAY_EVENT_SetSelectPokeList(bsw_scr,gsys,buf) );
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //選択ポケモン取得
  case BSWSUB_GET_ENTRY_POKE:
    *ret_wk = BSUBWAY_SCRWORK_GetEntryPoke( bsw_scr, gsys );
    break;
  //参加指定した手持ちポケモンの条件チェック
  case BSWSUB_CHK_ENTRY_POKE:
    #if 0
    *ret_wk = TowerScr_CheckEntryPoke(bsw_scr,core->fsys->savedata);
    #else
    GF_ASSERT( 0 && "BSWSUB_GET_ENTRY_POKE NOT SUPPORT" );
    *ret_wk = 0; //0=OK 1=同じポケモン 2=同じアイテム
    #endif
    break;
  //クリアしているか
  case BSWSUB_IS_CLEAR:
    if( BSUBWAY_SCRWORK_IsClear(bsw_scr) == TRUE ){
      *ret_wk = TRUE;
    }else{
      *ret_wk = FALSE;
    }
    break;
  //敗北時のスコアセット
  case BSWSUB_SET_LOSE_SCORE:
    BSUBWAY_SCRWORK_SetLoseScore( gsys, bsw_scr );
    break;
  //クリア時のスコアセット
  case BSWSUB_SET_CLEAR_SCORE:
    BSUBWAY_SCRWORK_SetClearScore( bsw_scr, gsys );
    break;
  //休む際のプレイデータ処理
  case BSWSUB_SAVE_REST_PLAY_DATA:
    BSUBWAY_SCRWORK_SaveRestPlayData( bsw_scr );
    break;
  //対戦トレーナー抽選
  case BSWSUB_CHOICE_BTL_PARTNER:
    BSUBWAY_SCRWORK_ChoiceBattlePartner( bsw_scr );
    break;
  //敵トレーナーOBJコード取得
  case BSWSUB_GET_ENEMY_OBJ:
    *ret_wk = BSUBWAY_SCRWORK_GetTrainerOBJCode( bsw_scr, param0 );
    break;
  //バトル呼び出し
  case BSWSUB_LOCAL_BTL_CALL:
#ifdef DEBUG_BSW_COMM_MULTI_BTL_SKIP
    if( play_mode == BSWAY_MODE_COMM_MULTI ||
        play_mode == BSWAY_MODE_S_COMM_MULTI ){
      result = VMCMD_RESULT_SUSPEND;
      break;
    }
#endif
    
#ifndef DEBUG_BSW_BTL_SKIP
    SCRIPT_CallEvent(
        sc, BSUBWAY_EVENT_TrainerBattle(bsw_scr,gsys,fieldmap) );
#endif
    result = VMCMD_RESULT_SUSPEND;
    break;
  //現在のプレイモードを取得
  case BSWSUB_GET_PLAY_MODE:
    *ret_wk = bsw_scr->play_mode;
    KAGAYA_Printf( "BSWSUB GET PLAY_MODE = %d\n", *ret_wk );
    break;
  //ボス対戦フラグをセット
  case BSWSUB_SET_PLAY_BTL_BOSS:
    bsw_scr->boss_f = param0;
    break;
  //ボス対戦フラグを取得
  case BSWSUB_GET_PLAY_BTL_BOSS:
    *ret_wk = bsw_scr->boss_f;
    break;
  //バトルポイント加算
  case BSWSUB_ADD_BATTLE_POINT:
    *ret_wk = BSUBWAY_SCRWORK_AddBattlePoint( bsw_scr );
    break;
  //パートナー番号セット
  case BSWSUB_SET_PARTNER_NO:
    bsw_scr->partner = param0;
    break;
  //パートナー番号取得
  case BSWSUB_GET_PARTNER_NO:
    *ret_wk = bsw_scr->partner;
    break;
  //戦闘トレーナーセット
  case BSWSUB_BTL_TRAINER_SET:
    BSUBWAY_SCRWORK_SetBtlTrainerNo( bsw_scr );
    break;
  //選択ポケモン番号取得
  case BSWSUB_GET_SELPOKE_IDX:
    *ret_wk = bsw_scr->member[param0];
    break;
  //受信バッファクリア
  case BSWSUB_RECV_BUF_CLEAR:
    bsw_scr->comm_receive_count = 0; //受信カウントクリア
    MI_CpuClear8( bsw_scr->recv_buf, BSWAY_SIO_BUF_LEN );
    break;
  //トレーナー対戦前メッセージ表示
  case BSWSUB_TRAINER_BEFORE_MSG_UR:
    {
      u16 tr_idx = param0;
      u16 obj_id = param1;
      SCRIPT_CallEvent( sc,
          BSUBWAY_EVENT_TrainerBeforeMsg(
            bsw_scr,gsys,tr_idx,obj_id,SCRCMD_MSGWIN_UPRIGHT) );
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //ゲームクリア時のプレイデータをセーブ
  case BSWSUB_SAVE_GAMECLEAR:
    BSUBWAY_SCRWORK_SaveGameClearPlayData( bsw_scr );
    break;
  //ポケモンメンバーロード
  case BSWSUB_LOAD_POKEMON_MEMBER:
    BSUBWAY_SCRWORK_LoadPokemonMember( bsw_scr, gsys );
    break;
  //バトルボックス使用するか
  case BSWSUB_SET_USE_BTL_BOX_FLAG:
    {
      u8 buf = param0;
      BSUBWAY_PLAYDATA_SetData( playData,
          BSWAY_PLAYDATA_ID_use_battle_box, &buf );
    }
    break;
  //通信マルチモードにワークを変更
  case BSWSUB_CHG_WORK_COMM_MULTI_MODE:
    BSUBWAY_SCRWORK_ChangeCommMultiMode( bsw_scr );
    break;
  //スーパーモードか
  case BSWSUB_CHK_S_MODE:
    if( play_mode == BSWAY_MODE_S_SINGLE ||
        play_mode == BSWAY_MODE_S_DOUBLE ||
        play_mode == BSWAY_MODE_S_MULTI ||
        play_mode == BSWAY_MODE_S_COMM_MULTI ){
      *ret_wk = TRUE;
    }else{
      *ret_wk = FALSE;
    }
    break;
  //ホーム、OBJセット
  case BSWSUB_SET_HOME_OBJ:
    {
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      bsway_SetHomeNPC( bsw_scr, gdata, mmdlsys, fieldmap );
    }
    break;
  //通信中フラグをセット
  case BSWSUB_SET_COMM_FLAG:
    bsw_scr->comm_sio_f = param0;
    break;
  //通信中フラグを取得
  case BSWSUB_GET_COMM_FLAG:
    *ret_wk = bsw_scr->comm_sio_f;
    break;
  //戦闘結果取得
  case BSWSUB_GET_BATTLE_RESULT:
    if( bsw_scr->btl_setup_param == NULL ){
      GF_ASSERT( 0 );
      *ret_wk = BSW_BTL_RESULT_LOSE;
    }else{
      BtlResult res = bsw_scr->btl_setup_param->result;
      BtlCompetitor cp = BTL_COMPETITOR_SUBWAY;
      
      KAGAYA_Printf( "BSW 戦闘結果 %d\n", res );
      
      #ifdef PM_DEBUG
      //BTS 要望422 バトルサブウェイでも強制勝利入れてほしい
      // デバッグ都合上、特定キー押しながら逃げた時に結果コード書き換え
      if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_L|PAD_BUTTON_R) )
      {
        res = BTL_RESULT_WIN;
        
        if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){   // X押し = 負け
          res = BTL_RESULT_LOSE;
        }
      }
      #endif
      
      if( res == BTL_RESULT_WIN ){
        *ret_wk = BSW_BTL_RESULT_WIN;
      }else{
        *ret_wk = BSW_BTL_RESULT_LOSE;
      }
    }
    break;
  //ホームに着いた際に行うワークセット
  case BSWSUB_SET_HOME_WORK:
    BSUBWAY_SCRWORK_SetHomeWork( bsw_scr, gsys );
    break;
  //電車内BGM、ラウンド数別スイッチ設定
  case BSWSUB_SET_TRAIN_BGM_ROUND_SW:
    {
      SWITCH_INDEX idx = SWITCH_00;
      u16 round = BSUBWAY_PLAYDATA_GetRoundNo( playData );
      
      if( round ){ //ラウンドが存在する
#if 0 //これだとゲーム続きからの際はスイッチが一つしか設定されない
        idx += round;

        if( idx < SWITCH_NUM ){
          ISS_SYS *iss = GAMESYSTEM_GetIssSystem( gsys );
          ISS_SWITCH_SYS *iss_sw = ISS_SYS_GetIssSwitchSystem( iss );
          ISS_SWITCH_SYS_SwitchOn( iss_sw, idx ); 
        }
#else
        ISS_SYS *iss = GAMESYSTEM_GetIssSystem( gsys );
        ISS_SWITCH_SYS *iss_sw = ISS_SYS_GetIssSwitchSystem( iss );
        u16 range = idx + round + 1;
        
        for( ; idx < range; idx++ ){
          if( idx < SWITCH_NUM && idx != SWITCH_00 ){
            ISS_SWITCH_SYS_SwitchOn( iss_sw, idx );
          }
        }
#endif
      }
    }
    break;
  //パートナーデータロード
  case BSWSUB_CHOICE_BTL_SEVEN:
    {
      PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
      u32 sex = MyStatus_GetMySex( &player->mystatus );
      
      if( sex == PM_MALE ){
        sex = PM_FEMALE;
        KAGAYA_Printf( "パートナーデータロード パートナー女\n" );
      }else{
        sex = PM_MALE;
        KAGAYA_Printf( "パートナーデータロード パートナー男\n" );
      }
      
      BSUBWAY_SCRWORK_ChoiceBtlSeven( bsw_scr, sex );
    }
    break;
  //選択メンバーのポケモン番号を取得
  case BSWSUB_GET_MEMBER_POKENO:
    GF_ASSERT( param0 < BSUBWAY_STOCK_MEMBER_MAX );
    *ret_wk = bsw_scr->mem_poke[param0];
    break;
  //赤外線通信を行うフラグセット
  case BSWSUB_SET_COMM_IRC_FLAG:
    bsw_scr->comm_irc_f = param0;
    break;
  //赤外線通信を行うフラグ取得
  case BSWSUB_GET_COMM_IRC_FLAG:
    *ret_wk = bsw_scr->comm_irc_f;
    break;
  //手持ち、バトルボックスの事前レギュレーションチェック
  case BSWSUB_CHK_REGULATION:
    *ret_wk = FALSE; //error
    
    {
      int type;
      u32 ret = 0;
      REGULATION *regu;
      POKEPARTY *check_party = NULL, *btl_party = NULL, *my_party = NULL;
      u32 use_bbox = (u32)BSUBWAY_PLAYDATA_GetData(
        playData, BSWAY_PLAYDATA_ID_use_battle_box, NULL );
      
      type = bsw_getRegulationLabel( play_mode );
      regu = (REGULATION*)PokeRegulation_LoadDataAlloc( type, HEAPID_PROC );
      
      if( use_bbox == TRUE ){
        BATTLE_BOX_SAVE *bb_save;
        bb_save = BATTLE_BOX_SAVE_GetBattleBoxSave( save );
        btl_party = BATTLE_BOX_SAVE_MakePokeParty( bb_save, HEAPID_PROC );
        check_party = btl_party;
      }else{
        my_party = GAMEDATA_GetMyPokemon( gdata ); 
        check_party = my_party;
      }
      
      ret = PokeRegulationMatchPartialPokeParty( regu, check_party );
      
      GFL_HEAP_FreeMemory( regu );
      
      if( btl_party != NULL ){
        GFL_HEAP_FreeMemory( btl_party );
      }
      
      if( ret == POKE_REG_OK || ret == POKE_REG_TOTAL_LV_FAILED ){
        *ret_wk = TRUE; //ok
      }
      
#ifdef DEBUG_BSW_REGU_NONE
      *ret_wk = TRUE;
#endif
    }
    break;
  //バトルボックス使用準備
  case BSWSUB_PREPAR_BTL_BOX:
    BSUBWAY_SCRWORK_PreparBattleBox( bsw_scr );
    break;
  //レギュレーションタイプ取得
  case BSWSUB_GET_REGULATION_TYPE:
    *ret_wk = bsw_getRegulationLabel( play_mode );
    break;
  //現在のプレイモードで参加に必要なポケモン数
  case BSWSUB_GET_MEMBER_NUM:
    *ret_wk = BSUBWAY_SCRWORK_GetPlayModeMemberNum( play_mode );
    break;
  //戦闘録画データ存在チェック呼び出し
  //(一時的にですが、大きいヒープを使用するため、
  //BattleRec_Initとの共存は厳しいかもしれません。
  //BattleRec_Init呼び出し前に、先にチェックをしておいて、
  //フラグを別途ワークに保存しておくのをオススメします。)
  //なので存在チェックの前にこれを呼ぶ必要がある。
  case BSWSUB_CALL_BTLREC_EXIST:
    if( bsw_scr->btlrec_exist_f == BSW_BTLREC_EXIST_NG ){
      LOAD_RESULT res;
      bsw_scr->btlrec_exist_f = BSW_BTLREC_EXIST_NON;
      if( BattleRec_DataOccCheck(
            save,HEAPID_PROC,&res,LOADDATA_MYREC) == TRUE ){
        bsw_scr->btlrec_exist_f = BSW_BTLREC_EXIST_EXIST;
      }
    }
    break;
  //戦闘録画データ存在チェック
  case BSWSUB_CHK_BTLREC_EXIST:
    GF_ASSERT( bsw_scr->btlrec_exist_f != BSW_BTLREC_EXIST_NG ); //none call
    
    if( bsw_scr->btlrec_exist_f == BSW_BTLREC_EXIST_NON ){
      *ret_wk = FALSE;
    }else{
      *ret_wk = TRUE;
    }
    break;
  //戦闘後の録画データ格納
  case BSWSUB_STORE_BTLREC:
    GF_ASSERT( bsw_scr->btl_setup_param != NULL );
    {
      BattleRec_LoadToolModule();
      BattleRec_StoreSetupParam( bsw_scr->btl_setup_param );
      BattleRec_UnloadToolModule();
    }
    break;
  //戦闘録画データセーブ
  case BSWSUB_SAVE_BTLREC:
    {
      bsw_scr->btlrec_save_work[0] = 0;
      bsw_scr->btlrec_save_work[1] = 0;
      bsw_scr->btlrec_btlnum = param0;
      VMCMD_SetWait( core, evBtlRecSave );
      bsw_scr->btlrec_exist_f = BSW_BTLREC_EXIST_EXIST;
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //戦闘用ワーク開放
  case BSWSUB_FREE_BTLPRM:
    if( bsw_scr->btl_setup_param != NULL ){
      BattleRec_Exit();
      BATTLE_PARAM_Delete( bsw_scr->btl_setup_param );
      bsw_scr->btl_setup_param = NULL;
    }
    break;
  //戦闘録画処理呼び出し -> 現状未使用となっている
  case BSWSUB_CALL_BTLREC:
    {
      GMEVENT *event;
      
      MI_CpuClear8( &bsw_scr->btl_rec_sel_param, sizeof(BTL_REC_SEL_PARAM) );
      bsw_scr->btl_rec_sel_param.gamedata = gdata;
      bsw_scr->btl_rec_sel_param.b_rec = TRUE;
      bsw_scr->btl_rec_sel_param.b_sync = FALSE;
      bsw_scr->btl_rec_sel_param.battle_mode = data_ModeBattleMode[play_mode];
      bsw_scr->btl_rec_sel_param.fight_count =
        BSUBWAY_SCRWORK_GetNowRenshou( bsw_scr );
      
      if( bsw_scr->btl_rec_sel_param.fight_count > BSW_RENSHOU_MAX ){
        bsw_scr->btl_rec_sel_param.fight_count = BSW_RENSHOU_MAX;
      }
      
      event = EVENT_FieldSubProc_Callback(
          gsys, fieldmap, FS_OVERLAY_ID(btl_rec_sel), 
          &BTL_REC_SEL_ProcData, &bsw_scr->btl_rec_sel_param,
          NULL, NULL );
      SCRIPT_CallEvent( sc, event );
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //ステージ数、ラウンド数をリセット
  case BSWSUB_RESET_STAGE_ROUND:
    {
      //ラウンドリセット
      BSUBWAY_PLAYDATA_ResetRoundNo( playData );
      //ステージ数をエラーに
      BSUBWAY_SCOREDATA_ErrorStageNo( scoreData, play_mode );
    }
    break;
  //電車のゆれを止める
  case BSWSUB_STOP_SHAKE_TRAIN:
    BSUBWAY_GIMMICK_StopTrainShake( fieldmap );
    break;
  //現在ラウンドのトレーナー番号を取得
  //param0=マルチの際は0,1でペアトレーナー切り替え
  case BSWSUB_GET_TRAINER_NUM_ROUND_NOW:
    {
      u16 round = BSUBWAY_PLAYDATA_GetRoundNo( playData );
      
      switch( play_mode ){
      case BSWAY_MODE_MULTI:
      case BSWAY_MODE_S_MULTI:
      case BSWAY_MODE_COMM_MULTI:
      case BSWAY_MODE_S_COMM_MULTI:
        *ret_wk = bsw_scr->trainer[round*2+param0];
        break;
      default:
        *ret_wk = bsw_scr->trainer[round];
      }
    }
    break;
  //トレーナー対戦前メッセージ表示
  case BSWSUB_TRAINER_BEFORE_MSG_DR:
    {
      u16 tr_idx = param0;
      u16 obj_id = param1;
      SCRIPT_CallEvent( sc,
          BSUBWAY_EVENT_TrainerBeforeMsg(
            bsw_scr,gsys,tr_idx,obj_id,SCRCMD_MSGWIN_DOWNRIGHT) );
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //wifi成績、戦闘結果更新
  case BSWSUB_UPDATE_WIFISCORE_BTLRESULT:
    GF_ASSERT( bsw_scr->btl_setup_param != NULL );
    
    if( bsw_scr->btl_setup_param != NULL ){
      BATTLE_SETUP_PARAM *btlParam = bsw_scr->btl_setup_param;
      u8 turn = btlParam->TurnNum;
      u8 down = btlParam->LosePokeNum;
      int damage;

      { //ＨＰ割合計算
        int i;
        int poke_num;
        int hp, hp_max;
        POKEPARTY *party = btlParam->party[BTL_CLIENT_PLAYER];

        //ポケモン数を取得
        poke_num = PokeParty_GetPokeCount(party);
        //ＨＰ格納バッファ初期化
        hp = 0;
        hp_max = 0;

        for (i=0;i<poke_num;i++)
        {
          int pokehp, pokehp_max;
          POKEMON_PARAM *pp;
          pp = PokeParty_GetMemberPointer(party, i); 
          //残りＨＰ取得
          pokehp = PP_Get( pp, ID_PARA_hp, NULL);
          //最大ＨＰ取得
          pokehp_max = PP_Get( pp, ID_PARA_hpmax, NULL);
          //足しこみ
          hp += pokehp;
          hp_max += pokehp_max;
        }
        
        damage = hp_max - hp;
      }
      
      BSUBWAY_PLAYDATA_AddWifiRecord( playData, down, turn, damage );
    }
    break;
  //通信録画済みフラグをセット TRUE=済み
  case BSWSUB_SET_BTLREC_FINISH_FLAG:
    bsw_scr->btlrec_finish_f = param0;
    break;
  //戦闘録画済フラグをゲット
  case BSWSUB_GET_BTLREC_FINISH_FLAG:
    *ret_wk = bsw_scr->btlrec_finish_f;
    break;
  //バトルボックス使用するかフラグを取得
  case BSWSUB_GET_USE_BTL_BOX_FLAG:
    *ret_wk = BSUBWAY_PLAYDATA_GetData(
        playData, BSWAY_PLAYDATA_ID_use_battle_box, NULL );
    break;
  //ラウンド数増加
  case BSWSUB_INC_ROUND:
    BSUBWAY_PLAYDATA_IncRoundNo( playData );
    BSUBWAY_SCRWORK_IncNowRenshou( bsw_scr );
    *ret_wk = BSUBWAY_SCRWORK_GetNowRenshou( bsw_scr );
    
    if( (*ret_wk) > BSW_RENSHOU_MAX ){
      *ret_wk = BSW_RENSHOU_MAX;
    }
    break;
  //現在の連勝数取得
  case BSWSUB_GET_NOW_RENSHOU:
    *ret_wk = BSUBWAY_SCRWORK_GetNowRenshou( bsw_scr );
    
    if( *ret_wk > BSW_RENSHOU_MAX ){
      *ret_wk = BSW_RENSHOU_MAX;
    }
    break;
  //ゲーム開始時のスコア設定
  case BSWSUB_GAME_START_SCORE:
    {
      BOOL exist;
      exist = BSUBWAY_SCOREDATA_CheckExistStageNo( scoreData, play_mode );
      
      if( exist == FALSE ){ //存在しない
        BSUBWAY_SCOREDATA_InitStageNo( scoreData, play_mode );
        BSUBWAY_PLAYDATA_ResetRoundNo( playData );
        BSUBWAY_SCRWORK_ResetNowRenshou( bsw_scr );
      }else{ //存在する
        //特になし(連勝数復元はワーク作成時に行っている)
      }
    }
    break;
  //----ワーク依存　通信関連
  //通信開始
  case BSWSUB_COMM_START:
    if( bsw_scr->comm_irc_f == TRUE ){ //赤外線
      BSUBWAY_COMM_AddCommandTable( bsw_scr );
    }else{ //ワイヤレス
      BSUBWAY_COMM_Init( bsw_scr );
    }
    
    break;
  //通信終了
  case BSWSUB_COMM_END:
    SCRIPT_CallEvent( sc, createEvCommExit(gsys,bsw_scr) );
    result = VMCMD_RESULT_SUSPEND;
    break;
  //通信同期
  case BSWSUB_COMM_TIMSYNC:
    KAGAYA_Printf( "BSUBWAY 通信同期開始 No.%d\n", param0 );
    bsw_scr->comm_timing_no = param0;
    bsw_scr->ret_work_id = retwk_id;
    BSUBWAY_COMM_TimingSyncStart( bsw_scr->comm_timing_no );
    VMCMD_SetWait( core, evCommTimingSync );
    result = VMCMD_RESULT_SUSPEND;
    break;
  //親機選択メニュー表示
  case BSWSUB_COMM_ENTRY_PARENT_MENU:
    {
      bsw_scr->pCommEntryResult = ret_wk;
      bsw_scr->pCommEntryMenu = CommEntryMenu_Setup(
          GAMEDATA_GetMyStatus(gdata),
          fieldmap, 2, 2, HEAPID_PROC,
          COMM_ENTRY_MODE_PARENT,
          COMM_ENTRY_GAMETYPE_SUBWAY, NULL );
      VMCMD_SetWait( core, evCommEntryMenuPerent );
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //子機選択メニュー表示
  case BSWSUB_COMM_ENTRY_CHILD_MENU:
    {
      bsw_scr->pCommEntryResult = ret_wk;
      bsw_scr->pCommEntryMenu = CommEntryMenu_Setup(
          GAMEDATA_GetMyStatus(gdata),
          fieldmap, 2, 2, HEAPID_PROC, 
          COMM_ENTRY_MODE_CHILD,
          COMM_ENTRY_GAMETYPE_SUBWAY, NULL );
      VMCMD_SetWait( core, evCommEntryMenuChild );
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //通信データ送信
  case BSWSUB_COMM_SEND_BUF:
    BSUBWAY_SCRWORK_CreateCommSendData( bsw_scr, param0, param1 );
    VMCMD_SetWait( core, evCommSendData );
    result = VMCMD_RESULT_SUSPEND;
    break;
  //通信データ受信
  case BSWSUB_COMM_RECV_BUF:
    BSUBWAY_SCRWORK_CommRecieveDataStart( bsw_scr, param0 );
    bsw_scr->ret_work_id = retwk_id;
    VMCMD_SetWait( core, evCommRecvData );
    result = VMCMD_RESULT_SUSPEND;
    break;
  //自分の通信IDを取得
  case BSWSUB_COMM_GET_CURRENT_ID:
    *ret_wk = GFL_NET_SystemGetCurrentID();
    break;
  //赤外線通信開始
  case BSWSUB_COMM_IRC_ENTRY:
    {
      GMEVENT *irc_event;
      
      MI_CpuClear8( &bsw_scr->irc_match, sizeof(IRC_MATCH_WORK) );
      bsw_scr->irc_match.gamedata = gdata;
      bsw_scr->irc_match.selectType = EVENTIRCBTL_ENTRYMODE_SUBWAY;
      
      irc_event = EVENT_FieldSubProc_Callback(
          gsys, fieldmap, FS_OVERLAY_ID(ircbattlematch), 
          &IrcBattleMatchProcData, &bsw_scr->irc_match,
          NULL, NULL );
      SCRIPT_CallEvent( sc, irc_event );
    }
    result = VMCMD_RESULT_SUSPEND;
    break;
  //赤外線通信結果
  case BSWSUB_COMM_IRC_ENTRY_RESULT:
    switch( bsw_scr->irc_match.selectType ){
    case EVENTIRCBTL_ENTRYMODE_SUBWAY:
      *ret_wk = BSWAY_COMM_IRC_RESULT_OK;
      break;
    case EVENTIRCBTL_ENTRYMODE_EXIT:
      *ret_wk = BSWAY_COMM_IRC_RESULT_EXIT;
      break;
    case EVENTIRCBTL_ENTRYMODE_RETRY:
      *ret_wk = BSWAY_COMM_IRC_RESULT_RETRY;
      break;
    default:
      GF_ASSERT( 0 );
      *ret_wk = BSWAY_COMM_IRC_RESULT_EXIT;
    }
    break;
  //通信相手をユニオン知り合いグループとして登録。
  //BSWAY_COMM_MYSTATUS_DATA実行済みが条件
  case BSWSUB_COMM_SET_UNION_ACQ:
    {
      ETC_SAVE_WORK *etc = SaveData_GetEtc( save );
      EtcSave_SetAcquaintance( etc, bsw_scr->mystatus_fr.id ) ;
    }
    break;
  //通信親のOBJコードを取得
  //BSWAY_COMM_MYSTATUS_DATA実行済みが条件
  case BSWSUB_COMM_GET_OBJCODE_OYA:
    {
      int sex;
      
      if( GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE ){ //親
        FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
        sex = FIELD_PLAYER_GetSex( fld_player );
      }else{
        sex = bsw_scr->mystatus_fr.sex;
      }
      
      *ret_wk = FIELD_PLAYER_GetDrawFormToOBJCode(
          sex, PLAYER_DRAW_FORM_NORMAL );
    }
    break;
  //通信子のOBJコードを取得
  //BSWAY_COMM_MYSTATUS_DATA実行済みが条件
  case BSWSUB_COMM_GET_OBJCODE_KO:
    {
      int sex;
      
      if( GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE ){ //親
        sex = bsw_scr->mystatus_fr.sex;
      }else{
        FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
        sex = FIELD_PLAYER_GetSex( fld_player );
      }
      
      *ret_wk = FIELD_PLAYER_GetDrawFormToOBJCode(
          sex, PLAYER_DRAW_FORM_NORMAL );
    }
    break;
  //通信エラー画面表示リクエスト
  case BSWSUB_COMM_REQ_ERROR_DISP:
    NetErr_App_ReqErrorDisp();
    break;
  //通信エラー画面常時リクエスト フィールドフリー時
  case BSWSUB_COMM_REQ_ERROR_DISP_FLD:
    GAMESYSTEM_SetFieldCommErrorReq( gsys, TRUE );
    break;
  //通信エラーチェック
  case BSWSUB_COMM_CHK_NET_ERROR:
    *ret_wk = FALSE;
    if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE ){
      *ret_wk = TRUE;
    }
    break;
  //----デバッグ
  //DEBUG 選択ポケモン強制セット
  case BSWSUB_DEBUG_SET_SELECT_POKE:
    #ifdef PM_DEBUG
    {
      u16  i = 0;
      const POKEPARTY *party;
      POKEMON_PARAM *pp;
      party = BSUBWAY_SCRWORK_GetPokePartyUse( bsw_scr );
    
      for( i = 0; i < bsw_scr->member_num; i++ ){ //ポケモン選択の手持ちNo
        bsw_scr->pokelist_select_num[i] = i + 1;
        bsw_scr->member[i] = bsw_scr->pokelist_select_num[i] - 1;
        pp = PokeParty_GetMemberPointer(
            (POKEPARTY*)party, bsw_scr->member[i] );
        bsw_scr->mem_poke[i] = PP_Get( pp, ID_PARA_monsno, NULL );  
        bsw_scr->mem_item[i] = PP_Get( pp, ID_PARA_item, NULL );  
      }
    }
    #endif
    break;
  //----ERROR
  //未対応コマンドエラー
  default:
    GF_ASSERT_MSG( 0, "BSW CMD ID ERROR ID=%d", cmd_id );
    break;
  }
  
  KAGAYA_Printf( "BSUBWAY コマンド完了\n" );
  return( result );
}

//======================================================================
//  通信関連
//======================================================================
//--------------------------------------------------------------
/**
 * 通信データ送信待ち
 */
//--------------------------------------------------------------
static BOOL evCommSendData( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  BSUBWAY_SCRWORK *bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  
  if( BSUBWAY_SCRWORK_CommSendData(bsw_scr) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 通信同期待ち
 * @param core VMHANDLE
 * @param wk script work
 * @retval BOOL
 */
//--------------------------------------------------------------
static BOOL evCommTimingSync( VMHANDLE *core, void *wk )
{
  COMM_ENTRY_RESULT entry_ret;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  BSUBWAY_SCRWORK *bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  u16 retwk_id = bsw_scr->ret_work_id;
  u16 *ret_wk = SCRIPT_GetEventWork( sc, gdata, retwk_id );
  
  if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE ){
    OS_Printf( "BSW NET ERROR!!\n" );
    *ret_wk = FALSE;
    return( TRUE );
  }else if( BSUBWAY_COMM_IsTimingSync(bsw_scr->comm_timing_no) == TRUE ){
    KAGAYA_Printf("BSUBWAY 通信同期完了 No.%d\n",bsw_scr->comm_timing_no);
    *ret_wk = TRUE;
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 通信マルチ受付メニュー　親
 * @param core VMHANDLE
 * @param wk script work
 * @retval BOOL
 */
//--------------------------------------------------------------
static BOOL evCommEntryMenuPerent( VMHANDLE *core, void *wk )
{
  COMM_ENTRY_RESULT entry_ret;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  EVENTWORK *event = GAMEDATA_GetEventWork( gdata );
  BSUBWAY_SCRWORK *bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  BSUBWAY_PLAYDATA *playData =
    SaveControl_DataPtrGet( save, GMDATA_ID_BSUBWAY_PLAYDATA );
  u32 play_mode = (u32)BSUBWAY_PLAYDATA_GetData(
        playData, BSWAY_PLAYDATA_ID_playmode, NULL );
  BSUBWAY_SCOREDATA *scoreData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_SCOREDATA );
  
  entry_ret = CommEntryMenu_Update( bsw_scr->pCommEntryMenu ); 
  
  if( entry_ret != COMM_ENTRY_RESULT_NULL ){
    CommEntryMenu_Exit( bsw_scr->pCommEntryMenu );
    bsw_scr->pCommEntryMenu = NULL;
    
    switch( entry_ret ){
    case COMM_ENTRY_RESULT_SUCCESS:      //メンバーが集まった
      (*bsw_scr->pCommEntryResult) = BSWAY_COMM_PERENT_ENTRY_OK;
      return( TRUE );
    case COMM_ENTRY_RESULT_CANCEL:       //キャンセルして終了
      (*bsw_scr->pCommEntryResult) = BSWAY_COMM_PERENT_ENTRY_CANCEL;
      return( TRUE );
    case COMM_ENTRY_RESULT_ERROR:        //エラーで終了
    default:
      (*bsw_scr->pCommEntryResult) = BSWAY_COMM_PERENT_ENTRY_ERROR;
      return( TRUE );
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 通信マルチ受付メニュー　子
 * @param core VMHANDLE
 * @param wk script work
 * @retval BOOL
 */
//--------------------------------------------------------------
static BOOL evCommEntryMenuChild( VMHANDLE *core, void *wk )
{
  COMM_ENTRY_RESULT entry_ret;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  EVENTWORK *event = GAMEDATA_GetEventWork( gdata );
  BSUBWAY_SCRWORK *bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  BSUBWAY_PLAYDATA *playData =
    SaveControl_DataPtrGet( save, GMDATA_ID_BSUBWAY_PLAYDATA );
  u32 play_mode = (u32)BSUBWAY_PLAYDATA_GetData(
        playData, BSWAY_PLAYDATA_ID_playmode, NULL );
  BSUBWAY_SCOREDATA *scoreData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_SCOREDATA );
  
  entry_ret = CommEntryMenu_Update( bsw_scr->pCommEntryMenu ); 
  
  if( entry_ret != COMM_ENTRY_RESULT_NULL ){
    CommEntryMenu_Exit( bsw_scr->pCommEntryMenu );
    bsw_scr->pCommEntryMenu = NULL;

    switch( entry_ret ){
    case ENTRY_PARENT_ANSWER_OK:      //エントリーOK
      (*bsw_scr->pCommEntryResult) = BSWAY_COMM_CHILD_ENTRY_OK;
      return( TRUE );
    case ENTRY_PARENT_ANSWER_NG:    //エントリーNG
    default:
      (*bsw_scr->pCommEntryResult) = BSWAY_COMM_CHILD_ENTRY_NG ;
      return( TRUE );
    }
  }

  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 通信受信待ち
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL evCommRecvData( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  BSUBWAY_SCRWORK *bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  u16 *ret_wk = SCRIPT_GetEventWork( sc, gdata, bsw_scr->ret_work_id );
  
  if( BSUBWAY_SCRWORK_CommRecieveData(bsw_scr,ret_wk) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//======================================================================
//  バトルサブウェイ　通信終了
//======================================================================
//--------------------------------------------------------------
/// バトルサブウェイ　通信終了イベント用ワーク
//--------------------------------------------------------------
typedef struct
{
  BSUBWAY_SCRWORK *bsw_scr;
}EV_COMM_EXIT_WORK;

//--------------------------------------------------------------
/// バトルサブウェイ　通信終了イベント
//--------------------------------------------------------------
static GMEVENT_RESULT evCommExit( GMEVENT *event, int *seq, void *wk )
{
  EV_COMM_EXIT_WORK *work = wk;
  
  switch( (*seq) ){
  case 0:
    if( GFL_NET_IsParentMachine() == TRUE ){
      GFL_NET_SetNoChildErrorCheck( FALSE ); //子機が不在でもエラーとしない
    }
    
    if( work->bsw_scr->comm_irc_f == TRUE ){ //赤外線通信 コマンド削除
      BSUBWAY_COMM_DeleteCommandTable( work->bsw_scr );
    }
        
    (*seq)++;
    break;   
  case 1:
    if( GFL_NET_IsParentMachine() == TRUE ){
      if( GFL_NET_GetConnectNum() <= 1 ){
        GFL_NET_Exit( NULL );
        (*seq)++;
      }
    }else{
      GFL_NET_Exit( NULL );
      (*seq)++;
    }
    break;
  case 2:
    if( GFL_NET_IsExit() == TRUE ){
      KAGAYA_Printf( "BSW NET EXIT END\n" );
      return( GMEVENT_RES_FINISH );
    }
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　通信終了イベント作成
 * @param gsys  GAMESYS_WORK*
 * @param bsw_scr BSUBWAY_SCRWORK*
 * @retval GMEVNET*
 */
//--------------------------------------------------------------
static GMEVENT * createEvCommExit(
    GAMESYS_WORK *gsys, BSUBWAY_SCRWORK *bsw_scr )
{
  GMEVENT *event;
  EV_COMM_EXIT_WORK *work;
  
  event = GMEVENT_Create(
      gsys, NULL, evCommExit, sizeof(EV_COMM_EXIT_WORK) );
  work = GMEVENT_GetEventWork( event );
  work->bsw_scr = bsw_scr;
  return( event );
}

//======================================================================
//  はい、いいえウィンドウ
//======================================================================
//--------------------------------------------------------------
/// はい・いいえウィンドウ制御イベント用ワーク定義
//--------------------------------------------------------------
typedef struct {
  SCRCMD_WORK * scrcmd_work;
  u16 * ret_wk;
  FLDMENUFUNC * mw;
  FLDMENUFUNC_YESNO cursor_pos;
}YESNO_EV_WORK;

//--------------------------------------------------------------
/// はい・いいえウィンドウ制御イベント
//--------------------------------------------------------------
static GMEVENT_RESULT YesNoSelectEvent(
    GMEVENT * event, int * seq, void * work )
{
  YESNO_EV_WORK * yew = work;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( yew->scrcmd_work );
  
  switch (*seq)
  {
  case 0:
    SCRCMD_WORK_SetWaitCount( yew->scrcmd_work, SELECT_WAIT_VALUE );
    (*seq) ++;
    /* FALL THROUGH */
  case 1:
    //一定時間ウェイト後、選択可能にする
    if ( SCRCMD_WORK_WaitCountDown( yew->scrcmd_work ) == TRUE )
    {
      GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( yew->scrcmd_work );
      FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
      FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG( fieldmap );
      FLDMENUFUNC *mw = FLDMENUFUNC_AddYesNoMenu( msgBG, yew->cursor_pos );
      yew->mw = mw;
      (*seq) ++;
    }
    break;
  case 2:
    {
      FLDMENUFUNC_YESNO ret;
      ret = FLDMENUFUNC_ProcYesNoMenu( yew->mw );
      
      switch ( ret )
      {
      case FLDMENUFUNC_YESNO_NULL:
        break;
      case FLDMENUFUNC_YESNO_YES:
        *yew->ret_wk = SCR_YES;
        (*seq) ++;
        break;
      default:
        *yew->ret_wk = SCR_NO;
        (*seq) ++;
        break;
      }
    }
    break;

  case 3:
    FLDMENUFUNC_DeleteMenu( yew->mw );
    return GMEVENT_RES_FINISH;
  }
  
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * はい、いいえウィンドウイベント作成
 * @param gsys  GAMESYS_WORK*
 * @param work SCRCMD_WORK*
 * @param ret_wk 結果格納先
 * @retval GMEVNET*
 */
//--------------------------------------------------------------
static GMEVENT * createYesNoEvent( GAMESYS_WORK *gsys,
    SCRCMD_WORK *work, u16 *ret_wk, FLDMENUFUNC_YESNO cursor_pos )
{
  GMEVENT * event;
  YESNO_EV_WORK * yew;
  event = GMEVENT_Create( gsys, NULL, YesNoSelectEvent, sizeof(YESNO_EV_WORK) );
  yew = GMEVENT_GetEventWork( event );
  yew->scrcmd_work = work;
  yew->ret_wk = ret_wk;
  yew->cursor_pos = cursor_pos;
  return event;
}

//======================================================================
//  途中駅NPC
//======================================================================
//--------------------------------------------------------------
/**
 * 途中駅NPC配置
 * @param 
 * @retval
 */
//--------------------------------------------------------------
static void bsway_SetHomeNPC(
    BSUBWAY_SCRWORK *bsw_scr, GAMEDATA *gdata,
    MMDLSYS *mmdlsys, FIELDMAP_WORK *fieldmap )
{
  MMDL *mmdl;
  int i = 0;
  int zone_id = FIELDMAP_GetZoneID( fieldmap );
  u16 obj_id = BSW_HOME_OBJID_NPC_FIRST;
  BSWAY_PLAYMODE mode = bsw_scr->play_mode;
  
  if( mode == BSWAY_MODE_WIFI ){
    u32 id;
    u16 code;
    const MMDL_GRIDPOS *pos_tbl = data_HomeNpcWiFiPosTbl;
    SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
    BSUBWAY_WIFI_DATA *wifiData = SaveControl_DataPtrGet(
        save, GMDATA_ID_BSUBWAY_WIFIDATA );
    BSUBWAY_LEADER_DATA *pLeader = BSUBWAY_WIFIDATA_GetLeaderDataAlloc(
        wifiData, HEAPID_PROC );
    BSUBWAY_LEADER_DATA *leader = &pLeader[BSUBWAY_STOCK_WIFI_LEADER_MAX-1];

    for( ; i < HOME_NPC_WIFI_MAX; i++, leader--, obj_id++, pos_tbl++ ){
      id = *(u32*)leader->id_no;
      id %= 10;
      
      if( leader->gender == 0 ){
        code = data_HomeNpcTbl_WifiMan[id];
      }else{
        code = data_HomeNpcTbl_WifiWoman[id];
      }

      mmdl = MMDLSYS_AddMMdlParam( mmdlsys,
          pos_tbl->gx, pos_tbl->gz, (DIR_UP+i) % DIR_MAX4,
          obj_id, code, MV_DIR_RND, zone_id );
      
      MMDL_SetParam( mmdl, i, MMDL_PARAM_0 );
      MMDL_SetEventID( mmdl, SCRID_C04R0111_NPC_TALK );
    }
    
    GFL_HEAP_FreeMemory( pLeader );
  }else{
    const HOME_NPC_DATA *data = data_HomeNpcTbl;
    BSUBWAY_SCOREDATA *bsw_score = bsw_scr->scoreData;
    u16 stage = BSUBWAY_SCOREDATA_GetStageNo_Org0( bsw_score, mode );
    
    KAGAYA_Printf( "BSW ホームNo.%d\n", stage );
    
    switch( mode ){
    case BSWAY_MODE_S_SINGLE:
    case BSWAY_MODE_S_DOUBLE:
    case BSWAY_MODE_S_MULTI:
    case BSWAY_MODE_S_COMM_MULTI:
      stage += 3;
    }
  
    while( data->code != OBJCODEMAX ){
      GF_ASSERT( data->npc_home_no );
      GF_ASSERT( data->npc_home_no <= HOME_NO_MAX );
      
      if( stage == data_NpcHomeNoToStageNo[data->npc_home_no-1] ){
        mmdl = MMDLSYS_AddMMdlParam( mmdlsys,
            data->gx, data->gz, (DIR_UP+i) % DIR_MAX4,
            obj_id, data->code, MV_DIR_RND, zone_id );
      
        MMDL_SetParam( mmdl, i, MMDL_PARAM_0 );
        MMDL_SetParam( mmdl, data->msg_id, MMDL_PARAM_1 );
        MMDL_SetEventID( mmdl, SCRID_C04R0111_NPC_TALK );
      
        if( obj_id < 0xff ){
          obj_id++;
        }else{
          GF_ASSERT( 0 );
        }
      }
      
      data++;
      i++;
    }
  }
}

//--------------------------------------------------------------
/**
 * 途中駅NPC メッセージID取得
 * @param 
 * @retval
 */
//--------------------------------------------------------------
static u16 bsway_GetHomeNPCMsgID( const MMDL *mmdl )
{
  return( MMDL_GetParam(mmdl,MMDL_PARAM_1) );
}

//--------------------------------------------------------------
/**
 * 途中駅NPC アイテム取得メッセージがあるか
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL bsway_CheckHomeNPCItemMsg(
    BSUBWAY_SCRWORK *bsw_scr, int npc_no )
{
  const HOME_NPC_DATA *data = &data_HomeNpcTbl[npc_no];
  
  if( bsw_scr->play_mode == BSWAY_MODE_WIFI ){
    return( FALSE );
  }
  
  if( data->item_no != ITEM_DUMMY_DATA ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 途中駅NPC アイテム取得メッセージなら各データ取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void bsway_GetHomeNPCItemData(
    BSUBWAY_SCRWORK *bsw_scr, u16 npc_no,
    u16 *msg_before, u16 *msg_after, u16 *item_no )
{
  const HOME_NPC_DATA *data = &data_HomeNpcTbl[npc_no];
  
  if( bsw_scr->play_mode == BSWAY_MODE_WIFI ){
    GF_ASSERT( 0 );
    *msg_before = 0;
    *msg_after = 0;
    *item_no = ITEM_MONSUTAABOORU;
    return;
  }
  
  if( data->item_no == ITEM_DUMMY_DATA ){
    GF_ASSERT( 0 );
    *msg_before = 0;
    *msg_after = 0;
    *item_no = ITEM_MONSUTAABOORU;
    return;
  }
  
  *msg_before = data->msg_id;
  *msg_after = data->msg_id_item_after;
  *item_no = data->item_no;
}

//======================================================================
//  通信録画 
//======================================================================
//--------------------------------------------------------------
/**
 * 戦闘録画セーブ待ち
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL evBtlRecSave( VMHANDLE *core, void *wk )
{
  SAVE_RESULT res;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  BSUBWAY_SCRWORK *bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  int btl_number = bsw_scr->btlrec_btlnum;
  BATTLE_MODE mode = BATTLE_MODE_SUBWAY_SINGLE;
  
  switch( bsw_scr->play_mode ){
  case BSWAY_MODE_DOUBLE:
  case BSWAY_MODE_S_DOUBLE:
    mode = BATTLE_MODE_SUBWAY_DOUBLE;
    break;
  case BSWAY_MODE_MULTI:
  case BSWAY_MODE_COMM_MULTI:
  case BSWAY_MODE_S_MULTI:
  case BSWAY_MODE_S_COMM_MULTI:
    mode = BATTLE_MODE_SUBWAY_MULTI;
    break;
  }
  
  if( btl_number > BSW_RENSHOU_MAX ){
    btl_number = BSW_RENSHOU_MAX;
  }
  
  res = BattleRec_Save(
      gdata, HEAPID_PROC, mode, btl_number, LOADDATA_MYREC,
      &bsw_scr->btlrec_save_work[0], &bsw_scr->btlrec_save_work[1] );
  
  if( res == SAVE_RESULT_OK || res == SAVE_RESULT_NG ){
    return( TRUE );
  }
  return( FALSE );
}

//======================================================================
//  parts
//======================================================================
#if 0
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
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  const POKEPARTY *party =  GAMEDATA_GetMyPokemon( gdata );
  
  if( PokeParty_GetPokeCount(party) < num ){
    return FALSE;
  }
  
  return( TRUE );
}
#endif

#if 0
//--------------------------------------------------------------
/**
 * レギュレーションチェック
 * @param mode プレイモード
 * @param gsys GAMESYS_WORK
 * @retval BOOL TRUE=OK
 */
//--------------------------------------------------------------
static BOOL bsway_CheckRegulation(
    int mode, int use_bbox, GAMESYS_WORK *gsys )
{
  int reg_type,ret;
  GAMEDATA *gdata;
  POKEPARTY *party;
  REGULATION *reg_data;
  
  switch( mode ){
  case BSWAY_MODE_SINGLE:
    reg_type = REG_SUBWAY_SINGLE;
    break;
  case BSWAY_MODE_DOUBLE:
    reg_type = REG_SUBWAY_DOUBLE;
    break;
  default:
    GF_ASSERT( 0 );
    reg_type = REG_SUBWAY_DOUBLE;
  }

  reg_data = (REGULATION*)PokeRegulation_LoadDataAlloc(
      reg_type, HEAPID_PROC );
  
  gdata = GAMESYSTEM_GetGameData( gsys );
  party =  GAMEDATA_GetMyPokemon( gdata );
  ret = PokeRegulationMatchPartialPokeParty( reg_data, party );
  GFL_HEAP_FreeMemory( reg_data );
  
  if( ret == POKE_REG_OK || POKE_REG_TOTAL_LV_FAILED ){
    return( TRUE );
  }
  
  return( FALSE );
}
#endif

//--------------------------------------------------------------
/**
 * レギュレーションラベル取得
 * @param play_mode BSWAY_MODE_SINGLE等
 * @retval u32 レギュレーションラベル
 */
//--------------------------------------------------------------
static u32 bsw_getRegulationLabel( u32 play_mode )
{
  u32 type = REG_SUBWAY_SINGLE;
  
  switch( play_mode ){
  case BSWAY_MODE_DOUBLE:
  case BSWAY_MODE_S_DOUBLE:
    type = REG_SUBWAY_DOUBLE;
    break;
  case BSWAY_MODE_MULTI:
  case BSWAY_MODE_S_MULTI:
  case BSWAY_MODE_COMM_MULTI:
  case BSWAY_MODE_S_COMM_MULTI:
    type = REG_SUBWAY_MALTI;
  }
  
  return( type );
}

//--------------------------------------------------------------
/**
 * 現在マップ別プレイモード取得
 */
//--------------------------------------------------------------
static u32 bsw_GetNowMapPlayMode( FIELDMAP_WORK *fieldmap )
{
  u16 zone_id = FIELDMAP_GetZoneID( fieldmap );
  u32 play_mode = BSWAY_MODE_SINGLE;

  switch( zone_id ){
  case ZONE_ID_C04R0102:
    play_mode = BSWAY_MODE_SINGLE;
    break;
  case ZONE_ID_C04R0103:
    play_mode = BSWAY_MODE_S_SINGLE;
    break;
  case ZONE_ID_C04R0104:
    play_mode = BSWAY_MODE_DOUBLE;
    break;
  case ZONE_ID_C04R0105:
    play_mode = BSWAY_MODE_S_DOUBLE;
    break;
  case ZONE_ID_C04R0106:
    play_mode = BSWAY_MODE_MULTI;
    break;
  case ZONE_ID_C04R0107:
    play_mode = BSWAY_MODE_S_MULTI;
    break;
  case ZONE_ID_C04R0108:
    play_mode = BSWAY_MODE_WIFI;
    break;
  default:
    GF_ASSERT( 0 );
  }
  return( play_mode );
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
/// バトルサブウェイ　プレイモード別　列車種類
//--------------------------------------------------------------
static const FLDEFF_BTRAIN_TYPE data_TrainModeType[BSWAY_MODE_MAX] =
{
  FLDEFF_BTRAIN_TYPE_01,
  FLDEFF_BTRAIN_TYPE_03,
  FLDEFF_BTRAIN_TYPE_05,
  FLDEFF_BTRAIN_TYPE_05,
  FLDEFF_BTRAIN_TYPE_07,

  FLDEFF_BTRAIN_TYPE_02,
  FLDEFF_BTRAIN_TYPE_04,
  FLDEFF_BTRAIN_TYPE_06,
  FLDEFF_BTRAIN_TYPE_06,
};

//--------------------------------------------------------------
/// バトルサブウェイ　列車位置テーブル
//--------------------------------------------------------------
static const VecFx32 data_TrainPosTbl[BTRAIN_POS_MAX] =
{
  {
    GRID_SIZE_FX32(45),
    GRID_SIZE_FX32(-2),
    GRID_SIZE_FX32(11) + GRID_HALF_FX32 - NUM_FX32(2),
  },
  {
    GRID_SIZE_FX32(47),
    GRID_SIZE_FX32(-3),
    GRID_SIZE_FX32(9) + GRID_HALF_FX32 - NUM_FX32(1),
  },
};

//--------------------------------------------------------------
/// バトルサブウェイ　モード別ゾーンID
//--------------------------------------------------------------
static const u32 data_PlayModeZoneID[BSWAY_MODE_MAX] =
{
  ZONE_ID_C04R0102,
  ZONE_ID_C04R0104,
  ZONE_ID_C04R0106,
  ZONE_ID_C04R0106,
  ZONE_ID_C04R0108,
  ZONE_ID_C04R0103,
  ZONE_ID_C04R0105,
  ZONE_ID_C04R0107,
  ZONE_ID_C04R0107,
};

//--------------------------------------------------------------
/// バトルサブウェイ　モード別受付復帰位置
//--------------------------------------------------------------
#define GRID_PLUS_HALF(p) (GRID_SIZE_FX32(p)+GRID_HALF_FX32)

static const VecFx32 data_PlayModeRecoverPos[BSWAY_MODE_MAX] =
{
  {GRID_PLUS_HALF(11),0,GRID_PLUS_HALF(15)},
  {GRID_PLUS_HALF(11),0,GRID_PLUS_HALF(15)},
  {GRID_PLUS_HALF(11),0,GRID_PLUS_HALF(15)},
  {GRID_PLUS_HALF(11),0,GRID_PLUS_HALF(15)},
  {GRID_PLUS_HALF(11),0,GRID_PLUS_HALF(15)},
  {GRID_PLUS_HALF(11),0,GRID_PLUS_HALF(15)},
  {GRID_PLUS_HALF(11),0,GRID_PLUS_HALF(15)},
  {GRID_PLUS_HALF(11),0,GRID_PLUS_HALF(15)},
  {GRID_PLUS_HALF(11),0,GRID_PLUS_HALF(15)},
};

//--------------------------------------------------------------
///  バトルサブウェイ　モード別クリアフラグ
//--------------------------------------------------------------
static const u16 data_ModeBossClearFlag[BSWAY_MODE_MAX] =
{
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_SINGLE, //single
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_DOUBLE, //double
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_MULTI, //multi
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_MULTI, //comm_multi
  BSWAY_SCOREDATA_FLAG_MAX, //wifi
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_S_SINGLE, //super single
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_S_DOUBLE, //super double
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_S_MULTI, //super multi
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_S_MULTI, //super comm multi
};

//--------------------------------------------------------------
/// バトルサブウェイ　モード別バトルモード
//--------------------------------------------------------------
static const u16 data_ModeBattleMode[BSWAY_MODE_MAX] =
{
  BATTLE_MODE_SUBWAY_SINGLE, //single
  BATTLE_MODE_SUBWAY_DOUBLE, //double
  BATTLE_MODE_SUBWAY_MULTI, //multi
  BATTLE_MODE_SUBWAY_MULTI, //comm_multi
  BATTLE_MODE_SUBWAY_SINGLE, //wifi
  BATTLE_MODE_SUBWAY_SINGLE, //super single
  BATTLE_MODE_SUBWAY_DOUBLE, //super double
  BATTLE_MODE_SUBWAY_MULTI, //super multi
  BATTLE_MODE_SUBWAY_MULTI, //super comm multi
};

//--------------------------------------------------------------
/// バトルサブウェイ　途中駅NPCデータ
//--------------------------------------------------------------
static const u32 data_NpcHomeNoToStageNo[HOME_NO_MAX] =
{
  1,  //home 1
  2,
  3,
  1+3,  //s1 4
  2+3,  //5
  3+3,
  4+3,
  5+3,
  6+3,
  7+3,  //10
  8+3,
  9+3,
  10+3,
  11+3,
  12+3, //15
  13+3,
  14+3,
  15+3,
  21+3,
  29+3, //20
  43+3,
  143+3, //22
};

#ifdef DEBUG_ONLY_FOR_kagaya
const u32 d_data_NpcHomeNoTbl[] =
{
1,
4,
1,
4,
1,
4,
1,
4,
1,
2,
4,
9,
15,
2,
4,
8,
12,
2,
3,
5,
11,
1,
2,
3,
5,
6,
8,
12,
3,
4,
6,
11,
3,
5,
6,
7,
8,
15,
18,
4,
1,
17,
9,
10,
12,
14,
21,
7,
9,
11,
16,
20,
1,
2,
5,
3,
5,
7,
22,
7,
9,
11,
13,
15,
8,
10,
13,
16,
19,
3,
5,
8,
3,
6,
7,
10,
6,
7,
10,
18,
};
#endif

static const HOME_NPC_DATA data_HomeNpcTbl[] =
{
  //たんぱんこぞう
  {BOY2,1,71,0,17,msg_c04r0111_boy2_1,0,ITEM_DUMMY_DATA}, //0
  {BOY2,4,62,0,14,msg_c04r0111_boy2_2,0,ITEM_DUMMY_DATA},
  //ミニスカート
  {GIRL2,1,66,0,14,msg_c04r0111_girl2_1,0,ITEM_DUMMY_DATA},
  {GIRL2,4,50,0,16,msg_c04r0111_girl2_2,0,ITEM_DUMMY_DATA},
  //塾帰り
  {BOY3,1,23,0,16,msg_c04r0111_boy3_1,0,ITEM_DUMMY_DATA},
  {BOY3,4,41,0,14,msg_c04r0111_boy3_2,0,ITEM_DUMMY_DATA},
  //塾帰り
  {GIRL3,1,35,0,16,msg_c04r0111_girl3_1,0,ITEM_DUMMY_DATA},
  {GIRL3,4,40,0,14,msg_c04r0111_girl3_2,0,ITEM_DUMMY_DATA},
  //バックパッカー
  {BACKPACKERM,1,61,0,13,msg_c04r0111_backpackerm_1,0,ITEM_DUMMY_DATA},
  {BACKPACKERM,2,31,0,14,msg_c04r0111_backpackerm_2,0,ITEM_DUMMY_DATA},
  {BACKPACKERM,4,58,0,15,msg_c04r0111_backpackerm_3,0,ITEM_DUMMY_DATA}, //10
  {BACKPACKERM,9,22,0,13,msg_c04r0111_backpackerm_4,0,ITEM_DUMMY_DATA},
  {BACKPACKERM,15,42,0,17,msg_c04r0111_backpackerm_5,0,ITEM_DUMMY_DATA},
  //バックパッカー
  {BACKPACKERW,2,67,0,17,msg_c04r0111_backpackerw_1,0,ITEM_DUMMY_DATA},
  {BACKPACKERW,4,12,0,16,msg_c04r0111_backpackerw_2,0,ITEM_DUMMY_DATA},
  {BACKPACKERW,8,38,0,14,msg_c04r0111_backpackerw_3,0,ITEM_DUMMY_DATA},
  {BACKPACKERW,12,28,0,16,msg_c04r0111_backpackerw_4,0,ITEM_DUMMY_DATA},
  //おまわりさん
  {POLICEMAN,2,44,0,15,msg_c04r0111_policeman_1,0,ITEM_DUMMY_DATA},
  {POLICEMAN,3,19,0,16,msg_c04r0111_policeman_2,0,ITEM_DUMMY_DATA},
  {POLICEMAN,5,50,0,16,msg_c04r0111_policeman_3,0,ITEM_DUMMY_DATA},
  {POLICEMAN,11,68,0,14,msg_c04r0111_policeman_4,0,ITEM_DUMMY_DATA}, //20
  //げいじゅつか
  {OLDMAN1,1,43,0,14,msg_c04r0111_oldman1_1,0,ITEM_DUMMY_DATA},
  {OLDMAN1,2,22,0,14,msg_c04r0111_oldman1_2,0,ITEM_DUMMY_DATA},
  {OLDMAN1,3,58,0,15,msg_c04r0111_oldman1_3,0,ITEM_DUMMY_DATA},
  {OLDMAN1,5,61,0,16,msg_c04r0111_oldman1_4,0,ITEM_DUMMY_DATA},
  {OLDMAN1,6,21,0,14,msg_c04r0111_oldman1_5,msg_c04r0111_oldman1_5_01,ITEM_POINTOAPPU},
  {OLDMAN1,8,20,0,16,msg_c04r0111_oldman1_6,0,ITEM_DUMMY_DATA},
  {OLDMAN1,12,67,0,14,msg_c04r0111_oldman1_7,0,ITEM_DUMMY_DATA},
  //OL
  {OL,3,67,0,15,msg_c04r0111_ol_1,0,ITEM_DUMMY_DATA},
  {OL,4,82,0,17,msg_c04r0111_ol_2,0,ITEM_DUMMY_DATA},
  {OL,6,60,0,16,msg_c04r0111_ol_3,0,ITEM_DUMMY_DATA}, //30
  {OL,11,34,0,15,msg_c04r0111_ol_4,0,ITEM_DUMMY_DATA},
  //てつどういん
  {RAILMAN,3,45,0,14,msg_c04r0111_railman_1,0,ITEM_DUMMY_DATA},
  {RAILMAN,5,12,0,15,msg_c04r0111_railman_2,0,ITEM_DUMMY_DATA},
  {RAILMAN,6,78,0,15,msg_c04r0111_railman_3,0,ITEM_DUMMY_DATA},
  {RAILMAN,7,19,0,16,msg_c04r0111_railman_4,0,ITEM_DUMMY_DATA},
  {RAILMAN,8,62,0,14,msg_c04r0111_railman_5,0,ITEM_DUMMY_DATA},
  {RAILMAN,15,61,0,17,msg_c04r0111_railman_6,0,ITEM_DUMMY_DATA},
  {RAILMAN,18,67,0,15,msg_c04r0111_railman_7,0,ITEM_DUMMY_DATA},
  //おぼっちゃま
  {BOY4,4,32,0,15,msg_c04r0111_boy4_1,0,ITEM_DUMMY_DATA},
  //おじょうちゃま
  {GIRL4,1,55,0,17,msg_c04r0111_girl4_1,0,ITEM_DUMMY_DATA}, //40
  //パイロット
  {PILOT,17,59,0,15,msg_c04r0111_pilot_1,0,ITEM_DUMMY_DATA},
  //エリートトレーナー
  {TRAINERM,9,69,0,17,msg_c04r0111_trainerm_1,0,ITEM_DUMMY_DATA},
  {TRAINERM,10,81,0,14,msg_c04r0111_trainerm_2,0,ITEM_DUMMY_DATA},
  {TRAINERM,12,49,0,14,msg_c04r0111_trainerm_3,0,ITEM_DUMMY_DATA},
  {TRAINERM,14,63,0,16,msg_c04r0111_trainerm_4,0,ITEM_DUMMY_DATA},
  {TRAINERM,21,70,0,17,msg_c04r0111_trainerm_5,0,ITEM_DUMMY_DATA},
  //エリートトレーナー
  {TRAINERW,7,67,0,17,msg_c04r0111_trainerw_1,msg_c04r0111_trainerw_1_01,ITEM_HUSIGINAAME},

  {TRAINERW,9,45,0,15,msg_c04r0111_trainerw_2,0,ITEM_DUMMY_DATA},
  {TRAINERW,11,23,0,16,msg_c04r0111_trainerw_3,0,ITEM_DUMMY_DATA},
  {TRAINERW,16,17,0,16,msg_c04r0111_trainerw_4,0,ITEM_DUMMY_DATA}, //50
  {TRAINERW,20,45,0,15,msg_c04r0111_trainerw_5,msg_c04r0111_trainerw_5_01,ITEM_SUTAANOMI},
  //スキンヘッズ
  {BADMAN,1,83,0,17,msg_c04r0111_badman_1,0,ITEM_DUMMY_DATA},
  {BADMAN,2,50,0,16,msg_c04r0111_badman_2,0,ITEM_DUMMY_DATA},
  {BADMAN,5,43,0,13,msg_c04r0111_badman_3,0,ITEM_DUMMY_DATA},
  //せいそういん
  {CLEANINGM,3,39,0,16,msg_c04r0111_cleaningm_1,msg_c04r0111_cleaningm_1_01,ITEM_POINTOAPPU},
  {CLEANINGM,5,35,0,16,msg_c04r0111_cleaningm_2,0,ITEM_DUMMY_DATA},
  {CLEANINGM,7,58,0,15,msg_c04r0111_cleaningm_3,0,ITEM_DUMMY_DATA},
  {CLEANINGM,22,50,0,14,msg_c04r0111_cleaningm_4,0,ITEM_DUMMY_DATA},
  //ベテラントレーナー
  {VETERANM,7,39,0,14,msg_c04r0111_veteranm_1,0,ITEM_DUMMY_DATA},
  {VETERANM,9,62,0,16,msg_c04r0111_veteranm_2,0,ITEM_DUMMY_DATA}, //60
  {VETERANM,11,53,0,14,msg_c04r0111_veteranm_3,0,ITEM_DUMMY_DATA},
  {VETERANM,13,59,0,15,msg_c04r0111_veteranm_4,0,ITEM_DUMMY_DATA},
  {VETERANM,15,55,0,14,msg_c04r0111_veteranm_5,0,ITEM_DUMMY_DATA},
  //ベテラントレーナー
  {VETERANW,8,57,0,16,msg_c04r0111_veteranw_1,0,ITEM_DUMMY_DATA},
  {VETERANW,10,39,0,13,msg_c04r0111_veteranw_2,0,ITEM_DUMMY_DATA},
  {VETERANW,13,60,0,15,msg_c04r0111_veteranw_3,0,ITEM_DUMMY_DATA},
  {VETERANW,16,68,0,15,msg_c04r0111_veteranw_4,0,ITEM_DUMMY_DATA},
  {VETERANW,19,64,0,14,msg_c04r0111_veteranw_5,0,ITEM_DUMMY_DATA},
  //マダム
  {LADY,3,25,0,14,msg_c04r0111_lady_1,0,ITEM_DUMMY_DATA},
  {LADY,5,82,0,15,msg_c04r0111_lady_2,0,ITEM_DUMMY_DATA}, //70
  {LADY,8,50,0,15,msg_c04r0111_lady_3,0,ITEM_DUMMY_DATA},
  //ジェントルマン
  {GENTLEMAN,3,26,0,14,msg_c04r0111_gentleman_1,0,ITEM_DUMMY_DATA},
  {GENTLEMAN,6,44,0,14,msg_c04r0111_gentleman_2,0,ITEM_DUMMY_DATA},
  {GENTLEMAN,7,48,0,16,msg_c04r0111_gentleman_3,0,ITEM_DUMMY_DATA},
  {GENTLEMAN,10,58,0,16,msg_c04r0111_gentleman_4,0,ITEM_DUMMY_DATA},
  //ビジネスマン
  {BUSINESSMAN,6,66,0,14,msg_c04r0111_businessman_1,0,ITEM_DUMMY_DATA},
  {BUSINESSMAN,7,55,0,13,msg_c04r0111_businessman_2,0,ITEM_DUMMY_DATA},
  {BUSINESSMAN,10,20,0,16,msg_c04r0111_businessman_3,0,ITEM_DUMMY_DATA},
  {BUSINESSMAN,18,28,0,14,msg_c04r0111_businessman_4,msg_c04r0111_businessman_4_01,ITEM_SANNOMI},
  //終端
  {OBJCODEMAX,0,0,0,0,0,0,ITEM_DUMMY_DATA}, //80
};

//--------------------------------------------------------------
/// バトルサブウェイ　途中駅NPCデータ WiFi用　男
//--------------------------------------------------------------
static const u16 data_HomeNpcTbl_WifiMan[10] =
{
  GENTLEMAN, //ジェントルマン //0
  BOY2, //たんぱんこぞう
  BOY3, //じゅくがえり
  BACKPACKERM, //バックパッカー
  POLICEMAN, //おまわりさん
  OLDMAN1,  //げいじゅつか
  BOY4, //おぼっちゃま
  PILOT, //パイロット
  TRAINERM, //エリートトレーナー
  VETERANM, //ベテラントレーナー //9
};

//--------------------------------------------------------------
/// バトルサブウェイ　途中駅NPCデータ WiFi用　女
//--------------------------------------------------------------
static const u16 data_HomeNpcTbl_WifiWoman[10] =
{
  VETERANW, //ベテラントレーナー //0
  GIRL2, //ミニスカート
  GIRL3, //じゅくがえり
  BACKPACKERW, //バックパッカー
  OL, //ＯＬ
  GIRL4, //おじょうさま
  TRAINERW, //エリートトレーナー
  VETERANW, //ベテラントレーナー
  LADY, //マダム
  TRAINERW, //エリートトレーナー //9
};

//--------------------------------------------------------------
/// バトルサブウェイ　WiFi用　途中駅NPC配置位置
//--------------------------------------------------------------
static const MMDL_GRIDPOS data_HomeNpcWiFiPosTbl[HOME_NPC_WIFI_MAX] =
{
  {70,0,16},
  {61,0,14},
  {47,0,16},
  {37,0,14},
  {26,0,16},
};

//======================================================================
//  デバッグ
//======================================================================
//----
#ifdef PM_DEBUG
//----

#include "../../../resource/fldmapdata/flagwork/work_define.h"

//--------------------------------------------------------------
/**
 * バトルサブウェイ　ワーク作成、データセット
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_DebugCreateWork( GAMESYS_WORK *gsys, u16 mode )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  
  //ワーク作成
  BSUBWAY_SCRWORK *bsw_scr = BSUBWAY_SCRWORK_CreateWork(
      gsys, BSWAY_PLAY_NEW, mode );
  
  //ステージ数リセット
  BSUBWAY_SCOREDATA_InitStageNo( bsw_scr->scoreData, mode );
  BSUBWAY_PLAYDATA_ResetRoundNo( bsw_scr->playData );
  BSUBWAY_SCRWORK_ResetNowRenshou( bsw_scr );
  
  { //手持ち使用
    u8 buf = FALSE;
     BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
          BSWAY_PLAYDATA_ID_use_battle_box, &buf );
  }
  
  //フラグ初期化
  {
    EVENTWORK *ev = GAMEDATA_GetEventWork( gdata );
    u16 *work = EVENTWORK_GetEventWorkAdrs( ev, WK_OTHER_BSUBWAY_RECEIPT );
    *work = BSWAY_SCENE_RECEIPT_ERROR;
    work = EVENTWORK_GetEventWorkAdrs( ev, WK_OTHER_BSUBWAY_TRAIN );
    *work = BSWAY_SCENE_TRAIN_CONTINUE;
    
    EVENTWORK_SetEventFlag( ev, FV_BSUBWAY_RECEIPT_PARTNER );
    EVENTWORK_SetEventFlag( ev, FV_C04R0111_PARTNER );
    EVENTWORK_SetEventFlag( ev, FV_C04R0111_NPC );
  }
  
  //ポケモン選択
  {
    int i;
    
    for( i = 0;  i < bsw_scr->member_num; i++ ){
      bsw_scr->pokelist_select_num[i] = i + 1;
    }
    
    bsw_scr->pokelist_return_mode = PL_RET_NORMAL;
    bsw_scr->pokelist_result_select = PL_SEL_POS_POKE1;
    
    //メンバーロード
    BSUBWAY_SCRWORK_GetEntryPoke( bsw_scr, gsys );
  }
  
  //aiマルチ
  if( mode == BSWAY_MODE_MULTI || mode == BSWAY_MODE_S_MULTI ){
    PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
    u32 sex = MyStatus_GetMySex( &player->mystatus );
    
    if( sex == PM_MALE ){
      sex = PM_FEMALE;
    }else{
      sex = PM_MALE;
    }
    
    bsw_scr->partner = 0;
    BSUBWAY_SCRWORK_ChoiceBtlSeven( bsw_scr, sex );
  }
  
  //対戦トレーナー抽選
  BSUBWAY_SCRWORK_SetBtlTrainerNo( bsw_scr );
  
  //デバッグフラグ初期化
  {
    u8 flag = BSUBWAY_SCOREDATA_DEBUG_GetFlag( bsw_scr->scoreData );
    flag &= ~BSW_DEBUG_FLAG_AUTO; //オートは切っておく
  }
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　ワーク作成、データセット
 * トレーナーNo指定あり。
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_DebugCreateWorkTrNo(
    GAMESYS_WORK *gsys, u16 mode, const u16 *tr_no )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  
  //ワーク作成
  BSUBWAY_SCRWORK *bsw_scr = BSUBWAY_SCRWORK_CreateWork(
      gsys, BSWAY_PLAY_NEW, mode );
  
  //ステージ数リセット
  BSUBWAY_SCOREDATA_InitStageNo( bsw_scr->scoreData, mode );
  BSUBWAY_PLAYDATA_ResetRoundNo( bsw_scr->playData );
  BSUBWAY_SCRWORK_ResetNowRenshou( bsw_scr );
  
  { //手持ち使用
    u8 buf = FALSE;
     BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
          BSWAY_PLAYDATA_ID_use_battle_box, &buf );
  }
  
  //フラグ初期化
  {
    EVENTWORK *ev = GAMEDATA_GetEventWork( gdata );
    u16 *work = EVENTWORK_GetEventWorkAdrs( ev, WK_OTHER_BSUBWAY_RECEIPT );
    *work = BSWAY_SCENE_RECEIPT_ERROR;
    work = EVENTWORK_GetEventWorkAdrs( ev, WK_OTHER_BSUBWAY_TRAIN );
    *work = BSWAY_SCENE_TRAIN_CONTINUE;
    
    EVENTWORK_SetEventFlag( ev, FV_BSUBWAY_RECEIPT_PARTNER );
    EVENTWORK_SetEventFlag( ev, FV_C04R0111_PARTNER );
    EVENTWORK_SetEventFlag( ev, FV_C04R0111_NPC );
  }
  
  //ポケモン選択
  {
    int i;
    
    for( i = 0;  i < bsw_scr->member_num; i++ ){
      bsw_scr->pokelist_select_num[i] = i + 1;
    }
    
    bsw_scr->pokelist_return_mode = PL_RET_NORMAL;
    bsw_scr->pokelist_result_select = PL_SEL_POS_POKE1;
    
    //メンバーロード
    BSUBWAY_SCRWORK_GetEntryPoke( bsw_scr, gsys );
  }
  
  //aiマルチ
  if( mode == BSWAY_MODE_MULTI || mode == BSWAY_MODE_S_MULTI ){
    PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
    u32 sex = MyStatus_GetMySex( &player->mystatus );
    
    if( sex == PM_MALE ){
      sex = PM_FEMALE;
    }else{
      sex = PM_MALE;
    }
    
    bsw_scr->partner = 0;
    BSUBWAY_SCRWORK_ChoiceBtlSeven( bsw_scr, sex );
  }
  
  //対戦トレーナー抽選
  BSUBWAY_SCRWORK_SetBtlTrainerNo( bsw_scr );
  
  //対戦トレーナー　指定番号セット
  {
    int i = 0;
    u16 stage = 0;
    
    if( mode == BSWAY_MODE_MULTI || mode == BSWAY_MODE_COMM_MULTI ||
        mode == BSWAY_MODE_S_MULTI || mode == BSWAY_MODE_S_COMM_MULTI )
    {
      bsw_scr->trainer[i] = tr_no[i];
      bsw_scr->trainer[i*2+1] = tr_no[i*2+1];
    }
    else
    {
      bsw_scr->trainer[i] = tr_no[i];
    }
  }
  
  //デバッグフラグ初期化
  {
    u8 flag = BSUBWAY_SCOREDATA_DEBUG_GetFlag( bsw_scr->scoreData );
    flag &= ~BSW_DEBUG_FLAG_AUTO; //オートは切っておく
  }
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　ワーク　任意のラウンド数に変更
 * @param game_round_now 何戦目にするか
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_DebugFightAnyRound(
    GAMESYS_WORK *gsys, u16 game_round_now )
{
  u16 round,stage,renshou;
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  BSUBWAY_SCRWORK *bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  
  if( bsw_scr == NULL ){
    GF_ASSERT( 0 );
    return;
  }
  
  if( game_round_now == 0 ){
    GF_ASSERT( 0 );
    game_round_now = 1;
  }
  
  round = (game_round_now-1) % 7;
  stage = game_round_now / 7;
  
  if( game_round_now % 7 ){
    stage++;
  }
  
  renshou = game_round_now - 1;
  
#if 0  
  OS_Printf( "BSW ================================================\n" );
  OS_Printf( "BSW DEBUG %d戦目へ(STAGE %d ROUND %d RENSHOU %d\n",
      game_round_now, stage, round, renshou );
  OS_Printf( "BSW ================================================\n" );
#endif
  
  BSUBWAY_PLAYDATA_SetRoundNo( bsw_scr->playData, round );
  BSUBWAY_SCOREDATA_SetStageNo_Org1(
      bsw_scr->scoreData, bsw_scr->play_mode, stage );
  BSUBWAY_SCOREDATA_SetRenshou(
      bsw_scr->scoreData, bsw_scr->play_mode, renshou );
  BSUBWAY_SCRWORK_SetNowRenshou( bsw_scr, renshou );
  
  //対戦トレーナー抽選
  BSUBWAY_SCRWORK_SetBtlTrainerNo( bsw_scr );
}

#if 0
//--------------------------------------------------------------
/**
 * バトルサブウェイ　ワーク　２１戦状態に
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_DebugFight21( GAMESYS_WORK *gsys )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  BSUBWAY_SCRWORK *bsw_scr = GAMEDATA_GetBSubwayScrWork( gdata );
  GF_ASSERT( bsw_scr != NULL );
  
  BSUBWAY_PLAYDATA_SetRoundNo( bsw_scr->playData, 6 );
  BSUBWAY_SCOREDATA_SetStageNo_Org1(
      bsw_scr->scoreData, bsw_scr->play_mode, 3 );
  BSUBWAY_SCOREDATA_SetRenshou( bsw_scr->scoreData, bsw_scr->play_mode, (7*2+6) );

  //対戦トレーナー抽選
  BSUBWAY_SCRWORK_SetBtlTrainerNo( bsw_scr );
}
#endif

//--------------------------------------------------------------
/**
 * バトルサブウェイ　フラグ取得
 */
//--------------------------------------------------------------
u8 BSUBWAY_SCRWORK_DebugGetFlag( GAMESYS_WORK *gsys )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  BSUBWAY_SCOREDATA *scoreData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_SCOREDATA );
  u8 flag = BSUBWAY_SCOREDATA_DEBUG_GetFlag( scoreData );
  return( flag );
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　フラグ設定
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_DebugSetFlag( GAMESYS_WORK *gsys, u8 flag )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  BSUBWAY_SCOREDATA *scoreData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_SCOREDATA );
  BSUBWAY_SCOREDATA_DEBUG_SetFlag( scoreData, flag );
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　wifiランク取得
 */
//--------------------------------------------------------------
int BSUBWAY_SCRWORK_DebugGetWifiRank( GAMESYS_WORK *gsys )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  BSUBWAY_SCOREDATA *scoreData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_SCOREDATA );
  
  return( BSUBWAY_SCOREDATA_GetWifiRank(scoreData) );
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　wifiランクセット
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_DebugSetWifiRank( GAMESYS_WORK *gsys, u8 rank )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  BSUBWAY_SCOREDATA *scoreData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_SCOREDATA );
  
  BSUBWAY_SCOREDATA_SetWifiRank( scoreData, rank );
  BSUBWAY_SCOREDATA_SetWifiLoseCount( scoreData, BSWAY_SETMODE_reset );
  BSUBWAY_SCOREDATA_SetFlag( scoreData,
    BSWAY_SCOREDATA_FLAG_WIFI_LOSE_F, BSWAY_SETMODE_reset );
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　wifiルームデータクリア
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_DebugClearWifiRoomData( GAMESYS_WORK *gsys )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  BSUBWAY_WIFI_DATA *wifiData = SaveControl_DataPtrGet(
    save, GMDATA_ID_BSUBWAY_WIFIDATA );

  BSUBWAY_WIFIDATA_ClearPlayerData( wifiData );
  BSUBWAY_WIFIDATA_ClearRoomDataFlag( wifiData );
}

//--------------------------------------------------------------
/**
 *  バトルサブウェイ　通信マルチ周回数操作
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_DebugSetCommMultiStage(
    GAMESYS_WORK *gsys, u16 play_mode, u16 stage )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  BSUBWAY_SCOREDATA *scoreData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_SCOREDATA );
  BSUBWAY_PLAYDATA *playData =
    SaveControl_DataPtrGet( save, GMDATA_ID_BSUBWAY_PLAYDATA );
  
  if( stage == 0 ){ //0
    BSUBWAY_SCOREDATA_ErrorStageNo( scoreData, play_mode );
  }else{
    BSUBWAY_SCOREDATA_SetStageNo_Org1( scoreData, play_mode, stage+1 );
    BSUBWAY_SCOREDATA_SetRenshou( scoreData, play_mode, stage*7 );
    BSUBWAY_PLAYDATA_ResetRoundNo( playData );
  }
}
//----
#endif //PM_DEBUG
//----
