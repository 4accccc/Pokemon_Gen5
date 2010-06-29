//===================================================================
/**
 * @file  main.c
 * @brief アプリケーション　サンプルスケルトン
 * @author  GAME FREAK Inc.
 * @date  06.11.08
 *
 * $LastChangedDate$
 * $Id$
 */
//===================================================================
#include "playable_version.h"
#include <nitro.h>
#include <nnsys.h>
#include "gflib.h"
#include "system\machine_use.h"
#include "system\gfl_use.h"
#include "system\main.h"
#include "savedata/save_control.h"
#include "net/wih_dwc.h"
#include "print/printsys.h"
#include "print/global_font.h"
#include "print/global_msg.h"
#include "poke_tool/poke_tool.h"
#include "system/net_err.h"
#include "savedata/config.h"
#include "system/wipe.h"
#include "system/brightness.h"
#include "gamesystem\msgspeed.h"
#include "gamesystem\game_beacon.h"
#include "gamesystem/g_power.h"
#include "savedata/config.h"

#include "sound\pm_sndsys.h"
#include "sound\pm_voice.h"
#include "sound\pm_wb_voice.h"
#ifdef PM_DEBUG
#include "debug/debug_sd_print.h"
#include "test/performance.h"
#include "test/debug_pause.h"
#include "debug/debugwin_sys.h"
#include "debug/debug_ui.h"
#include "debug/debug_ui_input.h"
#include "debug/debug_flg.h"
#endif //PM_DEBUG

#include "title/title.h"

// サウンド読み込みスレッド
extern OSThread soundLoadThread;

static BOOL isARM9preference(void);
static  void  SkeltonHBlankFunc(void);
static  void  SkeltonVBlankFunc(void);
static  void  GameInit(void);
static  void  GameMain(void);
static  void  GameExit(void);
static  void  GameVBlankFunc(void);
static  void  _set_sound_mode( CONFIG *config );

#ifdef  PLAYABLE_VERSION
static void checkOnwerComments( void );
#endif

#ifdef PM_DEBUG
static void DEBUG_StackOverCheck(void);
static void DEBUG_UI_AutoKey(void);

#define DEBUG_MAIN_TIME_AVERAGE (0) // 時間AVERAGE　表示 １：ON　０：OFF

#else

#define DEBUG_MAIN_TIME_AVERAGE (0) // 時間AVERAGE　表示 １：ON　０：OFF

#endif


//------------------------------------------------------------------
//  処理時間AVERAGE
//------------------------------------------------------------------
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

u8 DEBUG_MAIN_UPDATE_TYPE = 0;  ///<FIELDMAP TOP TAIL フレームチェック用


OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Now;    // 現在のチック

static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start;    // 1フレーム開始
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_End;    // 1フレーム開始
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave;      // AVERAGE
static int DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count = 0; // カウンタ
static int DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount =0;    // 16000Over
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max =0;    // 16000Over
#define DEBUG_DEBUG_MAIN_TIME_AVERAGE_AVE_COUNT_TIME  ( 120 )

#define DEBUG_DEBUG_MAIN_TIME_AVERAGE_NOW_TIME_DRAW_KEY  ( PAD_BUTTON_R )

static void DEBUG_MAIN_TIME_AVERAGE_StartFunc( void );
static void DEBUG_MAIN_TIME_AVERAGE_EndFunc( void );
#endif

//------------------------------------------------------------------
//  外部関数宣言
//------------------------------------------------------------------
//extern const  GFL_PROC_DATA CorpProcData;
//FS_EXTERN_OVERLAY(title);
FS_EXTERN_OVERLAY(notwifi);
FS_EXTERN_OVERLAY(intrude_system);

//------------------------------------------------------------------
/**
 * @brief メイン処理
 */
//------------------------------------------------------------------
void NitroMain(void)
{

  // ハード環境システム郡を初期化する関数
  MachineSystem_Init();
  // ＧＦＬ初期化
  GFLUser_Init();

#ifdef  PLAYABLE_VERSION
  checkOnwerComments();
#endif

  //HBLANK割り込み許可
  OS_SetIrqFunction(OS_IE_H_BLANK,SkeltonHBlankFunc);
  //VBLANK割り込み許可
  OS_SetIrqFunction(OS_IE_V_BLANK,SkeltonVBlankFunc);

  (void)OS_EnableIrqMask(OS_IE_H_BLANK|OS_IE_V_BLANK);
  (void)OS_EnableIrq();

  (void)GX_HBlankIntr(TRUE);
  (void)GX_VBlankIntr(TRUE);
  (void)OS_EnableInterrupts();

  // 必要なTCBとか登録して…
  GameInit();

#ifdef PM_DEBUG
  DEB_SD_PRINT_InitSystem( GFL_HEAPID_APP );
  DEBUG_PerformanceInit();
  DEBUG_PAUSE_Init();
  {
    //デバッグし捨ての初期化＋アドレスを渡す
    u8 *charArea;
    u16 *scrnArea,*plttArea;
    NetErr_GetTempArea( &charArea , &scrnArea , &plttArea );
    DEBUGWIN_InitSystem(charArea , scrnArea , plttArea);
  }
#endif

  OS_Printf( "argc=%d, argv[0]=%s,argv[1]=%s\n%d",OS_GetArgc(), OS_GetArgv(0), OS_GetArgv(1));

  while(TRUE){
#ifdef PM_DEBUG
    //  DEBUG_PerformanceStart();
    DEBUG_PerformanceMain();
    DEBUG_PerformanceStartLine(PERFORMANCE_ID_MAIN);

    DEBUG_PerformanceStartTick(PERFORMANCE_ID_MAIN);

#endif //PM_DEBUG

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc();
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

#ifdef PM_DEBUG
    DEBUG_StackOverCheck();
#endif  //PM_DEBUG

#ifdef PM_DEBUG
    DEBUG_UI_AutoKey();
#endif  //PM_DEBUG


    MachineSystem_Main();
    // メイン処理して…
    GFLUser_Main();
    GameMain();


    // 描画に必要な準備して…
    GFLUser_Display();

    // レンダリングエンジンが参照するデータ群をスワップ
    // ※gflibに適切な関数が出来たら置き換えてください
    //G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);

#ifdef PM_DEBUG
    //DEBUG_PerformanceDisp();
    DEBUG_PerformanceEndTick(PERFORMANCE_ID_MAIN);

    DEBUG_PerformanceEndLine(PERFORMANCE_ID_MAIN);
    DEB_SD_PRINT_UpdateSystem();
#endif //PM_DEBUG

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc();
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

    // VBLANK待ち
    GFL_G3D_SwapBuffers();

    {
      //2010.06.16  tamada
      //Vblank待ち状態のときに通信時・非通信時問わず
      //サウンド分割ロードスレッドは停止するようにした
      BOOL isARM9pref = isARM9preference();
      if( isARM9pref )
      { //非通信時はARM9優先にする
        MI_SetMainMemoryPriority( MI_PROCESSOR_ARM9 );
      }

      if( OS_IsThreadTerminated( &soundLoadThread ) == FALSE )
      { // サウンド読み込みスレッド休止
        OS_SleepThreadDirect( &soundLoadThread, NULL );
      }

      OS_WaitIrq(TRUE, OS_IE_V_BLANK);
      GameVBlankFunc();

      if( OS_IsThreadTerminated( &soundLoadThread ) == FALSE )
      { // サウンド読み込みスレッド起動
        OS_WakeupThreadDirect( &soundLoadThread );
      }

      if( isARM9pref )
      { //非通信時のARM9優先をARM7優先に戻す
        MI_SetMainMemoryPriority( MI_PROCESSOR_ARM7 );
      }
    }

    
  }//while

  GameExit();
}


//------------------------------------------------------------------
/**
 * brief  ARM9に切り替えるかどうか
 */
//------------------------------------------------------------------
static BOOL isARM9preference(void)
{
  if(GFL_NET_SystemGetConnectNum() > 1){  //接続中
    return FALSE;
  }
  if(GFL_NET_IsIrcConnect()){   //赤外線
    return FALSE;
  }
  if(GFL_NET_IsWifiConnect()){  //WIFI接続
    return FALSE;
  }
  return TRUE;
}



//------------------------------------------------------------------
/**
 * brief  HBlank割り込み処理
 */
//------------------------------------------------------------------
static  void  SkeltonHBlankFunc(void)
{
  GFLUser_HIntr();
}

//------------------------------------------------------------------
/**
 * brief  VBlank割り込み処理
 */
//------------------------------------------------------------------
static  void  SkeltonVBlankFunc(void)
{
  GFLUser_VIntr();

  OS_SetIrqCheckFlag(OS_IE_V_BLANK);
  MI_WaitDma(GX_DEFAULT_DMAID);
}

//--------------------------------------------------------------
/**
 * VBlank期間中に行う処理
 */
//--------------------------------------------------------------
static void GameVBlankFunc(void)
{
#ifdef PM_DEBUG
  DEBUG_PerformanceStartLine(PERFORMANCE_ID_VBLANK);
#endif

  MachineSystem_VIntr();
  GFLUser_VTiming();

#ifdef PM_DEBUG
  DEBUG_PerformanceEndLine(PERFORMANCE_ID_VBLANK);
#endif
}

//------------------------------------------------------------------
/**
 * @brief   ゲームごとの初期化処理
 */
//------------------------------------------------------------------
static  void  GameInit(void)
{
  
  /* ユーザーレベルで必要な初期化をここに記述する */
  //WIFIで必要ないプログラムオーバーレイの最初のロード
  GFL_OVERLAY_Load( FS_OVERLAY_ID( notwifi ) );
  GFL_OVERLAY_Load( FS_OVERLAY_ID( intrude_system ) );


  // 通信のデバッグプリントを行う定義
#ifdef PM_DEBUG
#if defined(DEBUG_ONLY_FOR_ohno)||defined(DEBUG_ONLY_FOR_matsuda)||defined(DEBUG_ONLY_FOR_toru_nagihashi)||defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)||defined(DEBUG_ONLY_FOR_none)
  GFL_NET_DebugPrintOn();
#endif
#endif  //PM_DEBUG

  /* 文字描画システム初期化 */
  PRINTSYS_Init( GFL_HEAPID_SYSTEM );

  // 通信ブート処理 VBlank割り込み後に行うためここに記述、第二引数は表示用関数ポインタ
  GFL_NET_Boot( GFL_HEAPID_APP, NULL, GFL_HEAPID_APP, HEAPID_NETWORK_FIX);  //ここでエラー表示する事があるので文字描画システムは手前
  // AP情報の取得
  WIH_DWC_CreateCFG(HEAPID_NETWORK_FIX);
  WIH_DWC_ReloadCFG();

  /* 起動プロセスの設定 */
#if 0
  TestModeSet();  //←サンプルデバッグモード
#else
  GFL_PROC_SysCallProc(NO_OVERLAY_ID, &TitleControlProcData, NULL);
//  GFL_PROC_SysCallProc(FS_OVERLAY_ID(title), &CorpProcData, NULL);
#endif

  
  //セーブ関連初期化(フラッシュを認識しない場合、警告画面を出す為、文字描画システムが出来てから)
  SaveControl_SystemInit(HEAPID_SAVE);
  CONFIG_ApplyMojiMode( SaveData_GetConfig( SaveControl_GetPointer() ) );
  MSGSPEED_InitSystem( SaveControl_GetPointer() );

  //通信エラー画面管理システム初期化
  NetErr_SystemInit();
  NetErr_SystemCreate(HEAPID_NET_ERR);

  /* グローバルメッセージデータ＆フォント初期化 */
  GLOBALMSG_Init( GFL_HEAPID_SYSTEM );

  /* poketoolシステム初期化 */
  POKETOOL_InitSystem( GFL_HEAPID_SYSTEM );

  //サウンドの設定
  PMSND_Init();
  //鳴き声システムの設定
  PMVOICE_Init(GFL_HEAPID_APP, PMV_GetVoiceWaveIdx, PMV_CustomVoiceWave);

  // サウンドモード（ステレオ・モノラル）設定
  _set_sound_mode( SaveData_GetConfig( SaveControl_GetPointer()) );

  //キーコントロールモード設定
  CONFIG_SYSTEM_KyeControlTblSetting();
  
  //ゲーム中ビーコンスキャン制御
  GAMEBEACON_Init(GFL_HEAPID_APP);
  
  //Gパワー
  GPower_SYSTEM_Init();
}

//------------------------------------------------------------------
/**
 * @brief   ゲームごとのメイン処理
 */
//------------------------------------------------------------------
static  void  GameMain(void)
{
  /* ユーザーレベルで必要なメイン処理をここに記述する */

  BrightnessChgMain();
  WIPE_SYS_Main();

  NetErr_Main();
#if PM_DEBUG
  if((GFL_UI_KEY_GetCont() & PAD_BUTTON_L) && (GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG)){
    NetErr_DEBUG_ErrorSet();
  }
#endif
  PMSND_Main();
  PMVOICE_Main();

  GAMEBEACON_Update();
  GPOWER_SYSTEM_Update();

}

//------------------------------------------------------------------
/**
 * @brief   ゲームごとの終了処理
 */
//------------------------------------------------------------------
static  void  GameExit(void)
{
#ifdef PM_DEBUG
  //デバッグシステム
  DEBUGWIN_ExitSystem();
#endif //PM_DEBUG

  GAMEBEACON_Exit();
  WIH_DWC_DeleteCFG();
  PMVOICE_Exit();
  PMSND_Exit();
}


//----------------------------------------------------------------------------------
/**
 * @brief サウンド設定
 *
 * @param   config    
 */
//----------------------------------------------------------------------------------
static void _set_sound_mode( CONFIG *config )
{
  if(CONFIG_GetSoundMode(config)==SOUNDMODE_STEREO)
  {
    PMSND_SetStereo(TRUE);  // ステレオ
  }
  else
  {
    PMSND_SetStereo(FALSE); // モノラル
  }
}

//--------------------------------------------------------------
/**
 * @brief   スタック溢れチェック
 */
//--------------------------------------------------------------
#ifdef PM_DEBUG
static void DEBUG_StackOverCheck(void)
{
  OSStackStatus stack_status;

  stack_status = OS_GetStackStatus(OS_GetCurrentThread());
  switch(stack_status){
  case OS_STACK_OVERFLOW:
    GF_ASSERT("スタック最下位のマジックナンバーが書き換えられています\n");
    break;
  case OS_STACK_UNDERFLOW:
    GF_ASSERT("スタック最上位のマジックナンバーが書き換えられています\n");
    break;
  case OS_STACK_ABOUT_TO_OVERFLOW:
    OS_TPrintf("スタック溢れの警戒水準に達しています\n");
    break;
  }

  stack_status = OS_GetIrqStackStatus();
  switch(stack_status){
  case OS_STACK_OVERFLOW:
    GF_ASSERT("IRQスタック最下位のマジックナンバーが書き換えられています\n");
    break;
  case OS_STACK_UNDERFLOW:
    GF_ASSERT("IRQスタック最上位のマジックナンバーが書き換えられています\n");
    break;
  case OS_STACK_ABOUT_TO_OVERFLOW:
    OS_TPrintf("IRQスタック溢れの警戒水準に達しています\n");
    break;
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief  オートキーチェック
 */
//-----------------------------------------------------------------------------
static void DEBUG_UI_AutoKey( void  )
{
  
  if( (OS_GetConsoleType() & (OS_CONSOLE_ISDEBUGGER|OS_CONSOLE_TWLDEBUGGER)) ){
    // 自動 UPDOWN
    if( DEBUG_FLG_GetFlg( DEBUG_FLG_AutoUpDown ) ){
      DEBUG_UI_SetUp( DEBUG_UI_AUTO_UPDOWN, DEBUG_UI_PLAY_LOOP );
    }else if( DEBUG_FLG_GetFlg( DEBUG_FLG_AutoLeftRight ) ){
      DEBUG_UI_SetUp( DEBUG_UI_AUTO_LEFTRIGHT, DEBUG_UI_PLAY_LOOP );
    }else if( DEBUG_FLG_GetFlg( DEBUG_FLG_AutoInputData ) ){
      DEBUG_UI_SetUp( DEBUG_UI_AUTO_USER_INPUT, DEBUG_UI_PLAY_LOOP );
    }else{
      if( (DEBUG_UI_GetType() == DEBUG_UI_AUTO_UPDOWN) ||
          (DEBUG_UI_GetType() == DEBUG_UI_AUTO_LEFTRIGHT) ||
          (DEBUG_UI_GetType() == DEBUG_UI_AUTO_USER_INPUT) ){
        DEBUG_UI_SetUp( DEBUG_UI_NONE, 0 );
      }
    }

    // Inputサンプリング
    if( DEBUG_FLG_GetFlg( DEBUG_FLG_SampleInputData ) ){
      DEBUG_UI_INPUT_Start();
    }else{
      DEBUG_UI_INPUT_End();
    }
    DEBUG_UI_INPUT_Update();
  }

}

#endif  //PM_DEBUG



#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

static void DEBUG_MAIN_TIME_AVERAGE_StartFunc( void )
{
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start = OS_GetTick(); 
}

static void DEBUG_MAIN_TIME_AVERAGE_EndFunc( void )
{
  OSTick now_time;
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_End = OS_GetTick(); 

  now_time = OS_TicksToMicroSeconds(DEBUG_DEBUG_MAIN_TIME_AVERAGE_End - DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start);

  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Now = now_time;

  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave += now_time;
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count ++;
  if( now_time > 16000 ){
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount ++;
#if DEBUG_MAIN_TIME_AVERAGE
    OS_TPrintf( " update %d", DEBUG_MAIN_UPDATE_TYPE );
    OS_TPrintf( " over tick %ld", now_time );
    OS_TPrintf( " ave tick %d\n", (DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave / DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count) );
#endif
  }
  if( DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max < now_time ){
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max = now_time;
  }
  if( DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count >= DEBUG_DEBUG_MAIN_TIME_AVERAGE_AVE_COUNT_TIME ){

#if DEBUG_MAIN_TIME_AVERAGE
    OS_TPrintf( " ave tick %d", 
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave / DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count );
    OS_TPrintf( " Over 120::%d",
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount );
    OS_TPrintf( " Max %d\n",
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max );
#endif

    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max       = 0;
  }

#if DEBUG_MAIN_TIME_AVERAGE
  if( DEBUG_DEBUG_MAIN_TIME_AVERAGE_NOW_TIME_DRAW_KEY & GFL_UI_KEY_GetCont() ){
    OS_TPrintf( " update[%d] tick %ld\n", DEBUG_MAIN_UPDATE_TYPE, now_time );
  }
#endif

}

#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

#ifdef  PLAYABLE_VERSION
//ぽけもんぼーだぶりゆ
static const u16 passward[11] = {
  0x307d,   //ぽ
  0x3051,   //け
  0x3082,   //も
  0x3093,   //ん
  0x307c,   //ぼ
  0x30fc,   //ー
  0x3060,   //だ
  0x3076,   //ぶ
  0x308a,   //り
  0x3086,   //ゆ
  0x0000,   //終端文字
};

//------------------------------------------------------------------
/**
 * @brief 指定パスワードを本体コメントに記入しないと起動しない
 */
//------------------------------------------------------------------
static void checkOnwerComments( void )
{
  int i;
  BOOL result = TRUE;
  OSOwnerInfo owner_info;
  OS_GetOwnerInfo( &owner_info );

  for ( i = 0; i < NELEMS(passward); i ++ )
  {
    OS_Printf("%04x : %04x\n", owner_info.nickName[i], passward[i] );
    if ( owner_info.nickName[ i ] == passward[ i ] )
    {
      if ( passward[ i ] == 0x0000 )
      {
        break;  //終端文字なのでチェック打ち切り
      }
      continue; //次の文字をチェック
    }
    result = FALSE;
  }
  if ( owner_info.nickNameLength != i )
  {
    result = FALSE;
  }
  if ( OS_GetConsoleType() & (OS_CONSOLE_ISDEBUGGER|OS_CONSOLE_TWLDEBUGGER) )
  { //デバッガ上で動作しているときはチェック無効
    return;
  }
  if ( result == FALSE )
  {
    GF_PANIC( "password error!\n" );
  }
}

#endif  //  PLAYABLE_VERSION



