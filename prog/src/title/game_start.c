//==============================================================================
/**
 * @file  game_start.c
 * @brief 「さいしょから」or「つづきから」を選択後の処理
 * @author  matsuda
 * @date  2009.01.07(水)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include "system\main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "title/game_start.h"
#include "title/title.h"
#include "savedata/situation.h"
#include "savedata/c_gear_data.h"
#include "app/name_input.h"
#include "test/testmode.h"
#include "select_moji_mode.h"
#include "debug_message.naix"
#include "msg/debug/msg_debugname.h"
#include "net_app/union/union_beacon_tool.h"
#include "system/gfl_use.h"
#include "demo/demo3d.h"
#include "field/zonedata.h"
#include "message.naix"
#include "msg/msg_intro.h"

#include "arc_def.h"  //ARCID_MESSAGE

#include "../../resource/fldmapdata/zonetable/zone_id.h"  // for ZONE_ID_T01

#include "savedata/save_control.h" // for
#include "savedata/save_control_intr.h" // for
#include "savedata/config.h"

//BGMメモリにおく
#include "sound/sound_manager.h"

#include "sound/pm_voice.h"   // for 
#include "savedata/save_outside.h"
#include "savedata/etc_save.h"

//==============================================================================
//  
//==============================================================================
FS_EXTERN_OVERLAY(title);
FS_EXTERN_OVERLAY(outside_save);

typedef enum
{
  CONTINUE_MODE_SELECT,
  CONTINUE_MODE_COMM_OFF,
  CONTINUE_MODE_COMM_ON,
}CONTINUE_MODE_TYPE;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT GameStart_FirstProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_FirstProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT GameStart_FirstProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_ContinueProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_ContinueProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_ContinueProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_DebugProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_DebugProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_DebugProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_DebugSelectNameProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_DebugSelectNameProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_DebugSelectNameProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static void _mystatus_info_set( MYSTATUS *my, STRBUF *name, int sex );

static void OutSideSave_ErasePermanent( SAVE_CONTROL_WORK *sv_ctrl );
static void OutSideSave_ConvertNormal( SAVE_CONTROL_WORK *sv_ctrl );

static void UnicodeToStrcode( const u16 * cp_src, u16 src_len, STRBUF *p_dst, u16 len );

//==============================================================================
//  データ
//==============================================================================
///最初から始める
static const GFL_PROC_DATA GameStart_FirstProcData = {
  GameStart_FirstProcInit,
  GameStart_FirstProcMain,
  GameStart_FirstProcEnd,
};

///続きから始める
static const GFL_PROC_DATA GameStart_ContinueProcData = {
  GameStart_ContinueProcInit,
  GameStart_ContinueProcMain,
  GameStart_ContinueProcEnd,
};

///デバッグ開始
static const GFL_PROC_DATA GameStart_DebugProcData = {
  GameStart_DebugProcInit,
  GameStart_DebugProcMain,
  GameStart_DebugProcEnd,
};

///じんめいせんたく開始
static const GFL_PROC_DATA GameStart_DebugSelectNameProcData = {
  GameStart_DebugSelectNameProcInit,
  GameStart_DebugSelectNameProcMain,
  GameStart_DebugSelectNameProcEnd,
};

//==============================================================================
//  定数定義
//==============================================================================
enum{
  DEBUG_COMM_SEARCH_ONLY,
  DEBUG_COMM_ON,
  DEBUG_COMM_OFF,
  DEBUG_COMM_SEARCH_ONLY_AND_BODYNAME,
};

//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   「最初から始める」を選択
 */
//--------------------------------------------------------------
void GameStart_Beginning(void)
{
  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_FirstProcData, NULL);
}

//--------------------------------------------------------------
/**
 * @brief   「続きから始める」を選択
 */
//--------------------------------------------------------------
void GameStart_Continue(void)
{
  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_ContinueProcData, (void*)CONTINUE_MODE_SELECT);
}

//--------------------------------------------------------------
/**
 * @brief   「続きから始める(通信なし)」を選択
 */
//--------------------------------------------------------------
void GameStart_ContinueNetOff(void)
{
  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_ContinueProcData, (void*)CONTINUE_MODE_COMM_OFF);
}

//--------------------------------------------------------------
/**
 * @brief   「続きから始める(常時通信)」を選択
 */
//--------------------------------------------------------------
void GameStart_ContinueNet(void)
{
  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_ContinueProcData, (void*)CONTINUE_MODE_COMM_ON);
}

//--------------------------------------------------------------
/**
 * @brief   「デバッグ開始」を選択
 */
//--------------------------------------------------------------
void GameStart_Debug(void)
{
#ifdef PM_DEBUG
  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_DebugProcData, (void*)DEBUG_COMM_SEARCH_ONLY);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   「デバッグ開始(常時通信)」を選択
 */
//--------------------------------------------------------------
void GameStart_DebugNet(void)
{
#ifdef PM_DEBUG
  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_DebugProcData, (void*)DEBUG_COMM_ON);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   「デバッグ開始(通信OFF)」を選択
 */
//--------------------------------------------------------------
void GameStart_DebugNetOff(void)
{
#ifdef PM_DEBUG
  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_DebugProcData, (void*)DEBUG_COMM_OFF);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   testmenuの「じんめいせんたく」を選択
 */
//--------------------------------------------------------------
void GameStart_Debug_SelectName(void)
{
#ifdef PM_DEBUG
  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_DebugSelectNameProcData, NULL);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   testmenuの「デバッグ開始（本体名）」を選択
 */
//--------------------------------------------------------------
void GameStart_Debug_BodyName(void)
{
#ifdef PM_DEBUG
  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_DebugProcData, (void*)DEBUG_COMM_SEARCH_ONLY_AND_BODYNAME);
#endif
}


//==============================================================================
//
//  最初から始める
//
//==============================================================================
#include "demo/intro.h"
typedef struct
{
  SELECT_MODE_INIT_WORK selModeParam;
  INTRO_PARAM introParam;
  NAMEIN_PARAM *nameInParam;
  INTR_SAVE_CONTROL* intr_save;
  GFL_PROCSYS* procsys_up;
  SOUNDMAN_PRESET_HANDLE* bgm_handle;
  DEMO3D_PARAM demo3dParam;
  u32 voice_load_id;
  MYSTATUS *mystatus;
  CONFIG *config;
  SAVE_CONTROL_WORK *sv_ctrl;   // swan追加
  STRCODE rival_name[PERSON_NAME_SIZE + EOM_SIZE];    // 16 セーブする時までの保存用
}GAMESTART_FIRST_WORK;

#define USE_INTRSAVE //INTRSAVE有効無効切り替えフラグ

//--------------------------------------------------------------
/**
 * @brief   
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT GameStart_FirstProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMESTART_FIRST_WORK*   work;
  SAVE_CONTROL_WORK *sv_ctrl = SaveControl_GetPointer();
  
  // ワーク アロケート
  work = GFL_PROC_AllocWork( proc , sizeof(GAMESTART_FIRST_WORK) , GFL_HEAPID_APP );
  
  // セーブシステム作成
#ifdef USE_INTRSAVE
  //既存セーブの有無を調べる為、セーブデータのワークエリアの処理化をするよりも先に実行
  work->intr_save = IntrSave_Init( GFL_HEAPID_APP, sv_ctrl );
#endif

  //もしセーブ破壊されていた場合、
  //管理外セーブを消す
  OutSideSave_ErasePermanent( sv_ctrl );

  // セーブデータ:ワークエリアのの初期化
  SaveControl_ClearData( sv_ctrl );
  
  work->sv_ctrl  = sv_ctrl;
  work->mystatus = MyStatus_AllocWork(GFL_HEAPID_APP);
  work->config = CONFIG_AllocWork(GFL_HEAPID_APP);
  GFL_STD_MemClear( &work->rival_name, 16);

  // SELMODE 初期化
  work->selModeParam.type       = SMT_START_GAME;
  work->selModeParam.configSave = work->config;
  work->selModeParam.mystatus   = work->mystatus;

  //主人公の性別は、性別設定が終わってから入れる
  work->nameInParam = NAMEIN_AllocParam( GFL_HEAPID_APP , NAMEIN_MYNAME , 0, 0, NAMEIN_PERSON_LENGTH , NULL, NULL );

  work->procsys_up = GFL_PROC_LOCAL_boot( GFL_HEAPID_APP );

  //BGMハンドル
  //イントロデモ中に鳴らすものだが、
  //ここでメモリ上においておく、さもないと、
  //イントロデモ中のアプリ終了時にメモリが開放されてしまい、名前入力中にBGMがならない
  { 
    u32 se_tbl[30]; //適当
    u32 namein_se_num;
    u32 intro_se_num;
    u32 i = 0;

    GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );  
    namein_se_num = NAMEIN_SE_PresetNum;
    for( i = 0; i < namein_se_num; i++ )
    { 
      se_tbl[i] = NAMEIN_SE_PresetData[i];
    }
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );
    
    GFL_OVERLAY_Load( FS_OVERLAY_ID(intro) ); 
    intro_se_num = Intro_Se_PresetNum;
    for( i = 0; i < intro_se_num; i++ )
    { 
      se_tbl[i+namein_se_num] = Intro_Se_PresetData[i];
    }
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(intro) );

    work->bgm_handle  = SOUNDMAN_PresetSoundTbl( se_tbl, namein_se_num + intro_se_num );

    work->voice_load_id = PMVOICE_LoadOnly( MONSNO_TIRATIINO, 0, 64, FALSE, 0, 0, FALSE, NULL );
  }

  // イントロデモのパラメータ初期化
  work->introParam.mystatus   = work->mystatus;
  work->introParam.config     = work->config;
  work->introParam.scene_id   = INTRO_SCENE_ID_00;
  work->introParam.intr_save  = work->intr_save;
  work->introParam.voice_load_id = work->voice_load_id;
  work->introParam.rival_name = work->rival_name;

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_FirstProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  enum 
  {
    SEQ_INIT,
    SEQ_INTRO_WAIT,
    SEQ_INPUT_NAME,
    SEQ_INPUT_NAME_WAIT,
    SEQ_INPUT_NAME_RETAKE_YESNO,
    SEQ_INPUT_NAME_RETAKE_CHECK,
    SEQ_INPUT_NAME_RETAKE_CHECK_WAIT,
    // SWAN追加
    SEQ_INPUT_RIVAL_NAME,
    SEQ_INPUT_RIVAL_NAME_WAIT,
    SEQ_INPUT_RIVAL_NAME_RETAKE_YESNO,
    SEQ_INPUT_RIVAL_NAME_RETAKE_CHECK,
    SEQ_INPUT_RIVAL_NAME_RETAKE_CHECK_WAIT,
    SEQ_END,
  };
  
  GAMESTART_FIRST_WORK *work = mywk;
  GFL_PROC_MAIN_STATUS up_status;

#ifdef USE_INTRSAVE
  IntrSave_Main( work->intr_save );
#endif

  up_status = GFL_PROC_LOCAL_Main( work->procsys_up );

  switch(*seq){
  case SEQ_INIT:
    //漢字選択
//  GFL_PROC_SysCallProc(NO_OVERLAY_ID, &SelectModeProcData,&work->selModeParam);
    //イントロデモ
    GFL_PROC_LOCAL_CallProc( work->procsys_up, FS_OVERLAY_ID(intro), &ProcData_Intro, &work->introParam );
//  GFL_PROC_SysCallProc(FS_OVERLAY_ID(intro), &ProcData_Intro, &work->introParam );
    (*seq) = SEQ_INTRO_WAIT;
    break;
  case SEQ_INTRO_WAIT :
    if( up_status != GFL_PROC_MAIN_VALID )
    {
      (*seq) = SEQ_INPUT_NAME;
    }
    break;
  case SEQ_INPUT_NAME:
    //名前入力
    work->nameInParam->hero_sex   = MyStatus_GetMySex(work->selModeParam.mystatus);
    work->nameInParam->p_intr_sv  = work->intr_save;
    //GFL_PROC_SysCallProc(FS_OVERLAY_ID(namein), &NameInputProcData,(void*)work->nameInParam);
    GFL_PROC_LOCAL_CallProc( work->procsys_up, FS_OVERLAY_ID(namein), &NameInputProcData, work->nameInParam );
    (*seq) = SEQ_INPUT_NAME_WAIT;
    break;
  case SEQ_INPUT_NAME_WAIT :
    if( up_status != GFL_PROC_MAIN_VALID )
    {
      (*seq) = SEQ_INPUT_NAME_RETAKE_YESNO;
    }
    break;
  case SEQ_INPUT_NAME_RETAKE_YESNO:
    //自分情報のセット
    _mystatus_info_set( work->mystatus, work->nameInParam->strbuf, MyStatus_GetMySex(work->mystatus) );

    //イントロデモ 名前入力の判定
    work->introParam.scene_id = INTRO_SCENE_ID_05_RETAKE_YESNO;
    
//  GFL_PROC_SysCallProc(FS_OVERLAY_ID(intro), &ProcData_Intro, &work->introParam );
    GFL_PROC_LOCAL_CallProc( work->procsys_up, FS_OVERLAY_ID(intro), &ProcData_Intro, &work->introParam );
    (*seq) = SEQ_INPUT_NAME_RETAKE_CHECK_WAIT;
    break;
  case SEQ_INPUT_NAME_RETAKE_CHECK_WAIT :
    if( up_status != GFL_PROC_MAIN_VALID )
    {
      (*seq) = SEQ_INPUT_NAME_RETAKE_CHECK;
    }
    break;
  case SEQ_INPUT_NAME_RETAKE_CHECK:
    // 名前入力復帰判定
    if( work->introParam.retcode == INTRO_RETCODE_INPUT_RIVAL )
    {
      (*seq) = SEQ_INPUT_RIVAL_NAME;
    }
    else
    {
      // 名前入力に復帰
      (*seq) = SEQ_INPUT_NAME;
    }
    break;

//== SWAN 追加==========================

  // ライバル名入力プロセス呼び出し
  case SEQ_INPUT_RIVAL_NAME:
    // 名前入力
    {
      STRBUF *p_default_str;
      GFL_MSGDATA *msg;
      msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_intro_dat, GFL_HEAPID_APP );
      p_default_str = GFL_MSG_CreateString( msg, msg_intro_rival_name01 );
      // デフォルメ名を設定
      GFL_STR_CopyBuffer( work->nameInParam->strbuf, p_default_str );
      GFL_STR_DeleteBuffer( p_default_str );
      GFL_MSG_Delete( msg );

      work->nameInParam->mode = NAMEIN_RIVALNAME;
      GFL_PROC_LOCAL_CallProc( work->procsys_up, FS_OVERLAY_ID(namein), &NameInputProcData, work->nameInParam );
      (*seq) = SEQ_INPUT_RIVAL_NAME_WAIT;
    }
    break;
  // ライバル名入力プロセス終了待ち
  case SEQ_INPUT_RIVAL_NAME_WAIT:
    if( up_status != GFL_PROC_MAIN_VALID )
    {
      (*seq) = SEQ_INPUT_RIVAL_NAME_RETAKE_YESNO;
    }
    break;
  // ライバル名が決定かどうかのシーン呼び出し
  case SEQ_INPUT_RIVAL_NAME_RETAKE_YESNO:
    {
      // ライバル名を保存しておく
      {
        GFL_STR_GetStringCode( work->nameInParam->strbuf, work->rival_name, PERSON_NAME_SIZE + EOM_SIZE );
      }

      //イントロデモ 名前入力の判定
      work->introParam.scene_id = INTRO_SCENE_ID_06_RETAKE_YESNO;

      GFL_PROC_LOCAL_CallProc( work->procsys_up, FS_OVERLAY_ID(intro), &ProcData_Intro, &work->introParam );
      (*seq) = SEQ_INPUT_RIVAL_NAME_RETAKE_CHECK_WAIT;
    }
    break;
  // ライバル名決定待ち
  case SEQ_INPUT_RIVAL_NAME_RETAKE_CHECK_WAIT:
    if( up_status != GFL_PROC_MAIN_VALID )
    {
      (*seq) = SEQ_INPUT_RIVAL_NAME_RETAKE_CHECK;
    }
    break;
  // ライバル名決定
  case SEQ_INPUT_RIVAL_NAME_RETAKE_CHECK:
    {
      // 名前入力復帰判定
      if( work->introParam.retcode == INTRO_RETCODE_NORMAL )
      {
        (*seq) = SEQ_END;
      }
      else
      {
        // 名前入力に復帰
        (*seq) = SEQ_INPUT_RIVAL_NAME;
      }
    }
    break;
  case SEQ_END:
    // 終了
    return GFL_PROC_RES_FINISH;
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_FirstProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SAVE_CONTROL_WORK *sv_ctrl = SaveControl_GetPointer();
  GAMESTART_FIRST_WORK *work = mywk;
  GAME_INIT_WORK * init_param;
  VecFx32 pos = {0,0,0};
  
  SOUNDMAN_ReleasePresetData( work->bgm_handle );

  // セーブシステム削除
#ifdef USE_INTRSAVE
  IntrSave_Exit( work->intr_save );
#endif
  // Introデモで変更してきたデータをセーブデータへ反映
  {
    MyStatus_Copy(work->mystatus, SaveData_GetMyStatus(sv_ctrl));
    CONFIG_Copy(work->config, SaveData_GetConfig(sv_ctrl));
  }
  // ライバル名セット
  {
    ETC_SAVE_WORK *etcsave = SaveData_GetEtc( sv_ctrl );
    // セーブデータに保存
    EtcSave_SetRivalName( etcsave, work->rival_name );
  }
  GFL_HEAP_FreeMemory(work->mystatus);
  GFL_HEAP_FreeMemory(work->config);
  
  GFL_PROC_LOCAL_Exit( work->procsys_up );

  //管理外セーブにデータが存在していれば通常セーブ領域へ移す
  OutSideSave_ConvertNormal( sv_ctrl );

  init_param = GAMEINIT_GetGameInitWork(GAMEINIT_MODE_FIRST, 0, &pos, 0 );

  NAMEIN_FreeParam(work->nameInParam);
  GFL_PROC_FreeWork( proc );
  
  GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &GameMainProcData, init_param);

  return GFL_PROC_RES_FINISH;
}



//==============================================================================
//
//  続きから始める
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT GameStart_ContinueProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMESTART_FIRST_WORK *work = GFL_PROC_AllocWork( proc , sizeof(GAMESTART_FIRST_WORK) , GFL_HEAPID_APP );
  work->selModeParam.type = SMT_CONTINUE_GAME;
  work->selModeParam.configSave = SaveData_GetConfig( SaveControl_GetPointer() );
  work->selModeParam.miscSave = SaveData_GetMisc( SaveControl_GetPointer() );
  work->selModeParam.mystatus = NULL;
  
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_ContinueProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMESTART_FIRST_WORK *work = mywk;
  CONTINUE_MODE_TYPE continueType = (CONTINUE_MODE_TYPE)pwk;
  switch(*seq){
  case 0:
    //通信選択
    if( continueType == CONTINUE_MODE_SELECT )
    {
      GFL_PROC_SysCallProc(NO_OVERLAY_ID, &SelectModeProcData,&work->selModeParam);
    }
    else
    if( continueType == CONTINUE_MODE_COMM_OFF )
    {
      CONFIG_SetNetworkSearchMode( work->selModeParam.configSave, NETWORK_SEARCH_OFF );
    }
    else
    if( continueType == CONTINUE_MODE_COMM_ON )
    {
      CONFIG_SetNetworkSearchMode( work->selModeParam.configSave, NETWORK_SEARCH_ON );
    }
    (*seq)++;
    break;
  case 1:
    return GFL_PROC_RES_FINISH;
    break;
  }
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_ContinueProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMESTART_FIRST_WORK *work = mywk;
  GAME_INIT_WORK * init_param;
  PLAYERWORK_SAVE plsv;
  SAVE_CONTROL_WORK *sv_ctrl = SaveControl_GetPointer();
  
  // 直前の選択肢で選んだ通信モードを取得する
  int search_mode_temp = CONFIG_GetNetworkSearchMode( work->selModeParam.configSave );
  // タイトルメニューの項目情報取得
  u8  title_menu_flag = MISC_GetStartMenuFlagAll( work->selModeParam.miscSave );
  u8  title_msg_flag = MISC_GetStartMenuMessageFlag(work->selModeParam.miscSave);

  SaveControl_Load(sv_ctrl);
  CONFIG_ApplyMojiMode( SaveData_GetConfig( sv_ctrl ) );  //ロードしたコンフィグに従って文字モード設定
  SaveData_SituationLoad_PlayerWorkSave(sv_ctrl, &plsv);

  init_param = GAMEINIT_GetGameInitWork(
  GAMEINIT_MODE_CONTINUE, plsv.zoneID, &plsv.position, plsv.direction );
  
  GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &GameMainProcData, init_param);

  // セーブデータで通信モードが上書きされてしまっているのでとっておいた通信モードを再度格納する
  CONFIG_SetNetworkSearchMode( work->selModeParam.configSave, search_mode_temp );
  // セーブデータでタイトルメニューの項目情報が上書きされてしまっているので再設定
  MISC_SetStartMenuFlagAll( work->selModeParam.miscSave, title_menu_flag );
  MISC_SetStartMenuMessageFlag(work->selModeParam.miscSave, title_msg_flag );

  GFL_PROC_FreeWork( proc );
  
  return GFL_PROC_RES_FINISH;

}



//==============================================================================
//
//  デバッグスタート
//
//==============================================================================
extern BOOL DebugScanOnly;
//--------------------------------------------------------------
/**
 * @brief   
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT GameStart_DebugProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SAVE_CONTROL_WORK *sv_ctrl = SaveControl_GetPointer();
  //もしセーブ破壊されていた場合、
  //管理外セーブを消す
  OutSideSave_ErasePermanent( sv_ctrl );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_DebugProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_DebugProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#ifdef PM_DEBUG

  BOOL is_body_name = FALSE;

  GAME_INIT_WORK * init_param;
  VecFx32 pos = {0,0,0};
  int always_net;
  int sex = 0;
  SAVE_CONTROL_WORK *sv_ctrl = SaveControl_GetPointer();
  
  always_net = (int)pwk;   //TRUE:常時通信で「続きから」

  if( always_net == DEBUG_COMM_SEARCH_ONLY_AND_BODYNAME )
  {
    is_body_name = TRUE;
    always_net = DEBUG_COMM_SEARCH_ONLY;
  }

  SaveControl_ClearData(sv_ctrl);  //セーブデータクリア
  
  if( is_body_name )
  {
    STRBUF *namebuf = GFL_STR_CreateBuffer( 32, GFL_HEAPID_APP );
    int sex = PM_MALE;

#if (defined(SDK_TWL))
    OSOwnerInfoEx info;
    OS_GetOwnerInfoEx(&info);
#else
    OSOwnerInfo owner_info;
    OS_GetOwnerInfo(&owner_info);
#endif


    //名前を本体からコンバート
    {
      UnicodeToStrcode( info.nickName, info.nickNameLength, namebuf, 5 + EOM_SIZE );
    }

    //性別を色からコンバート
    {
      if( info.favoriteColor <= 7 )
      {
        sex = PM_MALE;
      }
      else
      {
        sex = PM_FEMALE;
      }
    }

    {
      MYSTATUS    *myStatus = SaveData_GetMyStatus( sv_ctrl );
      _mystatus_info_set(myStatus, namebuf, sex);
    }

    GFL_STR_DeleteBuffer(namebuf);
  }
  else
  {//名前のセット
    MYSTATUS    *myStatus;
    GFL_MSGDATA *msgman;
    STRBUF *namebuf;
    u32 msg_id;
    
    msgman = GFL_MSG_Create( 
      GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_debugname_dat, GFL_HEAPID_APP );
#if 0
  #if PM_VERSION == VERSION_B2
    namebuf = GFL_MSG_CreateString( msgman, DEBUG_NAME_BLACK );
  #else
    namebuf = GFL_MSG_CreateString( msgman, DEBUG_NAME_WHITE );
  #endif
#else //侵入用に名前固定ではなく、ランダムで決定するようにした
    if(GFUser_GetPublicRand(2) & 1){
      namebuf = GFL_MSG_CreateString( msgman, DEBUG_NAME_RAND_M_000 + GFUser_GetPublicRand(8) );
      sex = PM_MALE;
    }
    else{
      namebuf = GFL_MSG_CreateString( msgman, DEBUG_NAME_RAND_W_000 + GFUser_GetPublicRand(8) );
      sex = PM_FEMALE;
    }
#endif
    myStatus = SaveData_GetMyStatus( sv_ctrl );
    _mystatus_info_set(myStatus, namebuf, sex);

    
    GFL_STR_DeleteBuffer(namebuf);
    GFL_MSG_Delete(msgman);
  }
#ifdef PM_DEBUG
  {
    // ライバル名のセット
    STRBUF *namebuf;
    GFL_MSGDATA *msgman;
    ETC_SAVE_WORK *etcsave = SaveData_GetEtc( sv_ctrl );

    msgman = GFL_MSG_Create( 
      GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_debugname_dat, GFL_HEAPID_APP );

    namebuf = GFL_MSG_CreateString( msgman, DEBUG_NAME_RIVAL_TMP );

    // セーブデータに保存
    {
      const STRCODE *strcode = GFL_STR_GetStringCodePointer(namebuf);
      EtcSave_SetRivalName( etcsave, strcode );
    }

    GFL_STR_DeleteBuffer(namebuf);
    GFL_MSG_Delete(msgman);
  }
#endif
  { 
    //常時通信モードのセット
    CONFIG *config;
    NETWORK_SEARCH_MODE mode;

    config  = SaveData_GetConfig( sv_ctrl );
    switch(always_net){
    case DEBUG_COMM_SEARCH_ONLY:
      mode = NETWORK_SEARCH_ON;
      DebugScanOnly = TRUE;
      break;
    case DEBUG_COMM_ON:
      mode = NETWORK_SEARCH_ON;
      DebugScanOnly = FALSE;
      break;
    case DEBUG_COMM_OFF:
    default:
      mode = NETWORK_SEARCH_OFF;
      DebugScanOnly = FALSE;
      break;
    }
    CONFIG_SetNetworkSearchMode( config, mode );
    //CGEARON
    CGEAR_SV_SetCGearONOFF(CGEAR_SV_GetCGearSaveData(sv_ctrl),TRUE);
  }

  //初回ならば、管理外セーブにデータが存在していれば通常セーブ領域へ移す
  OutSideSave_ConvertNormal( sv_ctrl );
  
  init_param = GAMEINIT_GetGameInitWork(GAMEINIT_MODE_DEBUG, ZONE_ID_C12, &pos, 0 );
  GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &GameMainProcData, init_param);
#endif

  return GFL_PROC_RES_FINISH;
}


//==============================================================================
//
//  じんめいせんたくでスタート
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT GameStart_DebugSelectNameProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#ifdef PM_DEBUG
  TESTMODE_PROC_WORK *work;
  work = GFL_PROC_AllocWork( proc, sizeof(TESTMODE_PROC_WORK), GFL_HEAPID_APP );
  work->startMode_ = TESTMODE_NAMESELECT;
  work->work_ = (void*)0;
  SaveControl_ClearData(SaveControl_GetPointer());  //セーブデータクリア
#endif
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_DebugSelectNameProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#ifdef PM_DEBUG
  FS_EXTERN_OVERLAY(testmode);

  switch(*seq){
  case 0:
    GFL_PROC_SysCallProc(FS_OVERLAY_ID(testmode), &TestMainProcData, (void*)mywk);
    (*seq)++;
    break;
  case 1:
    return GFL_PROC_RES_FINISH;
  }
#endif
  return GFL_PROC_RES_CONTINUE;

}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_DebugSelectNameProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#ifdef PM_DEBUG
  TESTMODE_PROC_WORK *work = mywk;
  SAVE_CONTROL_WORK *sv_ctrl = SaveControl_GetPointer();
  
  if( (int)work->work_ == 0 )
  {
    GAME_INIT_WORK * init_param;
    VecFx32 pos = {0,0,0};
    
    { 
      //常時通信モードのセット
      CONFIG *config;

      config  = SaveData_GetConfig( sv_ctrl );
      CONFIG_SetNetworkSearchMode( config, NETWORK_SEARCH_OFF );
      DebugScanOnly = TRUE;
    }
    //CGEARON
    CGEAR_SV_SetCGearONOFF(CGEAR_SV_GetCGearSaveData(sv_ctrl),TRUE);
    
    init_param = GAMEINIT_GetGameInitWork(GAMEINIT_MODE_DEBUG, 0, &pos, 0 );
    GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &GameMainProcData, init_param);
  }
  else
  {
    GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(testmode), &TestMainProcData, NULL );
  }
  GFL_PROC_FreeWork(proc);
#endif
  return GFL_PROC_RES_FINISH;
}


//----------------------------------------------------------------------------------
/**
 * @brief MYSTATUSに情報を登録する
 *
 * @param   my    MYSTATUS
 * @param   name  入力済の自分の名前の文字列
 * @param   sex   性別(PM_MALE, PM_FEMALE)
 */
//----------------------------------------------------------------------------------
static void _mystatus_info_set( MYSTATUS *my, STRBUF *name, int sex )
{
  // 名前
  MyStatus_SetMyNameFromString( my , name );
  // 性別
  MyStatus_SetMySex(my, sex);
  // トレーナーID
  MyStatus_SetID(my, GFUser_GetPublicRand0(0xFFFFFFFF));
  // ユニオン見た目設定
  MyStatus_SetTrainerView( my, 
                           GFL_STD_MtRand(MYSTATUS_UNIONVIEW_MAX)+
                           MyStatus_GetMySex(my)*MYSTATUS_UNIONVIEW_MAX );
  OS_Printf("TrainerView = %d\n", MyStatus_GetTrainerView( my ));
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  セーブが壊れていたら管理外を消しに行く
 *
 *	@param	SAVE_CONTROL_WORK *sv_ctrl 内部セーブ
 */
//-----------------------------------------------------------------------------
static void OutSideSave_ErasePermanent( SAVE_CONTROL_WORK *sv_ctrl )
{
  //もしセーブ破壊されていた場合、
  //管理外セーブを消す
  if( SaveControl_GetLoadResult(sv_ctrl) & FST_NORMAL_BREAK_BIT )
  {
    GFL_OVERLAY_Load( FS_OVERLAY_ID(outside_save) );
    {
      OUTSIDE_SAVE_CONTROL * outsv = OutsideSave_SystemLoad(GFL_HEAPID_APP);
      if( OutsideSave_GetPermanentExistFlag(outsv) )
      {
        OS_TPrintf( "通常セーブが壊れていたので、パーマネント消します\n" );
        OutsideSave_FlashPermanentErase(GFL_HEAPID_APP);
      }
      OutsideSave_SystemUnload(outsv);
    }
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(outside_save) );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  管理外セーブを内部セーブに移す（内部で移す移さないの判定つき）
 *
 *	@param	SAVE_CONTROL_WORK *sv_ctrl 内部セーブ
 */
//-----------------------------------------------------------------------------
static void OutSideSave_ConvertNormal( SAVE_CONTROL_WORK *sv_ctrl )
{
  //管理外セーブにデータが存在していれば通常セーブ領域へ移す
  //
  //移す条件：さいしょからを選んだ時かつすでセーブが書かれていないとき
	if(SaveControl_NewDataFlagGet(sv_ctrl) == TRUE
      && SaveData_GetExistFlag(sv_ctrl) == FALSE ) 
  {
    OUTSIDE_SAVE_CONTROL *outsv;
    GFL_OVERLAY_Load( FS_OVERLAY_ID(outside_save) );

    outsv = OutsideSave_SystemLoad(GFL_HEAPID_APP);

    if(OutsideSave_GetExistFlag(outsv))
    {
      OutsideSave_MysteryData_Outside_to_Normal(outsv, sv_ctrl);
    }
    if( OutsideSave_GetPermanentExistFlag(outsv) )
    {
      OutsideSave_KeySystem_Outside_to_Normal(outsv, sv_ctrl);
    }

    OutsideSave_SystemUnload(outsv);
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(outside_save) );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  ユニコードをSTRCODEに変換
 *
 *	@param	const u16 * cp_scr  ユニコード文字列
 *	@param	*p_dst  コンバート先（STRCODEをSTRBUFにセット）
 */
//-----------------------------------------------------------------------------
static void UnicodeToStrcode( const u16 * cp_src, u16 src_len, STRBUF *p_dst, u16 len )
{
  int i;
  BOOL is_eom = FALSE;

  u16 src[ 32 ];

  if( src_len < len )
  {
    len = src_len;
  }

  GF_ASSERT( len < NELEMS(src) );
  GFL_STD_MemCopy( cp_src, src, sizeof(u16)*len );

  for( i = 0; i < len; i++ )
  {
    if( L'ぁ' <= src[i] && src[i] <= L'ん' )
    {
    }
    else if( L'ァ' <= src[i] && src[i] <= L'ヴ' )
    {
    }
    else if( L'0' <= src[i] && src[i] <= L'9' )
    {
      src[i]  = L'０' + src[i] - L'0';
    }
    else if( L'０' <= src[i] && src[i] <= L'９' )
    {
    }
    else if( L'a' <= src[i] && src[i] <= L'z' )
    {
      src[i]  = L'ａ' + src[i] - L'a';
    }
    else if( L'A' <= src[i] && src[i] <= L'Z' )
    {
      src[i]  = L'Ａ' + src[i] - L'A';
    }
    else if( L'ａ' <= src[i] && src[i] <= L'ｚ' )
    {
    }
    else if( L'Ａ' <= src[i] && src[i] <= L'Ｚ' )
    {
    }
    else if( L'ー' == src[i] )
    {
    }
    else if( src[i] == 0 )
    {
      src[i]  = GFL_STR_GetEOMCode();
    }
    else
    {
      //他の文字は全部ピリオドにするよ
      src[i]  = L'．';
    }
  }

  if( len == 0 )
  {
    src[0]  = L'な';
    src[1]  = L'な';
    src[2]  = L'し';
    src[3]  = GFL_STR_GetEOMCode();
  }
  else
  {
    src[len-1]  = GFL_STR_GetEOMCode();
  }

  GFL_STR_SetStringCode( p_dst, src );
}
