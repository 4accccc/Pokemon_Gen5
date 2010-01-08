//============================================================================================
/**
 * @file  worldtrade.c
 * @brief 世界交換メイン処理
 * @author  Akito Mori -> toru_nagihashi WBへ移植
 * @date  06.04.16  -> 09.08.04
 */
//============================================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include <dwc.h>
#include "libdpw/dpw_tr.h"
#include "print/wordset.h"
#include "message.naix"
#include "msg/msg_wifi_lobby.h"
#include "system/wipe.h"
#include "gamesystem/msgspeed.h"

#include "print/printsys.h"


#include "savedata/wifilist.h"

#include "poke_tool/monsno_def.h"
#include "system/bmp_menu.h"
#include "net_app/worldtrade.h"
#include "sound/pm_sndsys.h"
#include "savedata/save_control.h"

#include "net/dwc_raputil.h"

#include "worldtrade_local.h"

#include "net_app/connect_anm.h"
#include "net/network_define.h"

//#include "msgdata/msg_ev_win.h"

#define WORLDTRADE_WORDSET_BUFLEN ( 64 )  ///< WORDSet文字列の展開用バッファ長

#define MYDWC_HEAPSIZE    0x20000
#include "worldtrade.naix"          ///< グラフィックアーカイブ定義
#include "poke_icon.naix"

#define BRIGHT_VAL  (-7)          ///< パッシブ状態のための半透明率

//============================================================================================
//  定数定義
//============================================================================================
// ポインタ参照だけできる世界交換ワーク構造体
//typedef struct _WORLDTRADE_WORK WORLDTRADE_WORK;


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
// プロセス定義データ
static GFL_PROC_RESULT WorldTradeProc_Init( GFL_PROC * proc, int * seq, void * param, void * work );
static GFL_PROC_RESULT WorldTradeProc_Main( GFL_PROC * proc, int * seq, void * param, void * work );
static GFL_PROC_RESULT WorldTradeProc_End( GFL_PROC * proc, int * seq, void * param, void * work );


/*** 関数プロトタイプ ***/
static void VBlankFunc( GFL_TCB *, void *work );
static void VramBankSet( const GFL_DISP_VRAM * vram );
static void InitWork( WORLDTRADE_WORK *wk, WORLDTRADE_PARAM *param );
static void FreeWork( WORLDTRADE_WORK *wk );
static void char_pltt_manager_init(void);
static void InitCellActor(WORLDTRADE_WORK *wk, const GFL_DISP_VRAM * vram );
static void SetCellActor(WORLDTRADE_WORK *wk);
static void CursorAppearUpDate(WORLDTRADE_WORK *wk, int arrow);
static void SetCursor_Pos( GFL_CLWK* act, int x, int y );
static void RecordMessagePrint( WORLDTRADE_WORK *wk, int msgno, int wait );
static void FreeFunc(DWCAllocType name, void* ptr,  u32 size);
static void *AllocFunc( DWCAllocType name, u32   size, int align );
static void InitDpw( void *heapPtr, NNSFndHeapHandle heapHandle, DWCAllocEx alloc, DWCFreeEx free  );
static void InitCLACT( WORLDTRADE_WORK *wk );
static void FreeCLACT( WORLDTRADE_WORK *wk );
static void ServerWaitTimeFunc( WORLDTRADE_WORK *wk );
static void BoxPokeNumGet( WORLDTRADE_WORK *wk );
static void WorldTrade_SelBoxCallback( SELBOX_WORK* sbox, u8 cur_pos, void* work, SBOX_CB_MODE mode);
static void WorldTrade_WndSetting(void);

static void NET_InitCallback( void *wk_adrs );
static BOOL NET_IsInit( const WORLDTRADE_WORK *wk );

static NNSFndHeapHandle _wtHeapHandle;


//============================================================================================
//  関数テーブル定義
//============================================================================================

// 世界交換サブプロセス用関数配列定義
static int (*SubProcessTable[][3])(WORLDTRADE_WORK *wk, int seq)={
  { WorldTrade_Enter_Init,   WorldTrade_Enter_Main,    WorldTrade_Enter_End   },
  { WorldTrade_Title_Init,   WorldTrade_Title_Main,    WorldTrade_Title_End   },
  { WorldTrade_MyPoke_Init,  WorldTrade_MyPoke_Main,   WorldTrade_MyPoke_End  },
  { WorldTrade_Partner_Init, WorldTrade_Partner_Main,  WorldTrade_Partner_End },
  { WorldTrade_Search_Init,  WorldTrade_Search_Main,   WorldTrade_Search_End  },
  { WorldTrade_Box_Init,     WorldTrade_Box_Main,      WorldTrade_Box_End     },
  { WorldTrade_Deposit_Init, WorldTrade_Deposit_Main,  WorldTrade_Deposit_End },
  { WorldTrade_Upload_Init,  WorldTrade_Upload_Main,   WorldTrade_Upload_End  },
  { WorldTrade_Status_Init,  WorldTrade_Status_Main,   WorldTrade_Status_End  },
  { WorldTrade_Demo_Init,    WorldTrade_Demo_Main,     WorldTrade_Demo_End  },
};


WORLDTRADE_WORK *debug_worldtrade;

//============================================================================================
//  プロセス関数
//============================================================================================
const GFL_PROC_DATA WorldTrade_ProcData =
{
  WorldTradeProc_Init,
  WorldTradeProc_Main,
  WorldTradeProc_End,
};


//--------------------------------------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT WorldTradeProc_Init( GFL_PROC * proc, int * seq, void * param, void * work )
{
  WORLDTRADE_WORK * wk;

  switch(*seq){
  case 0:
#ifdef PM_DEBUG
    GFL_HEAP_GetHeapFreeSize( GFL_HEAPID_SYSTEM );
    GFL_HEAP_GetHeapFreeSize( GFL_HEAPID_APP );
#endif


    // レコードコーナー用ヒープ作成
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WORLDTRADE, 0x80000 );



#if PL_G292_090323_FIX
    DwcOverlayStart();
    DpwCommonOverlayStart();
        mydwc_initdwc(HEAPID_WORLDTRADE);
#endif

    wk = GFL_PROC_AllocWork( proc, sizeof(WORLDTRADE_WORK), HEAPID_WORLDTRADE );
    GFL_STD_MemFill( wk, 0, sizeof(WORLDTRADE_WORK) );

    debug_worldtrade = wk;



    //task
    wk->task_wk_area  = GFL_HEAP_AllocMemory( HEAPID_WORLDTRADE, GFL_TCB_CalcSystemWorkSize( 8 ) );
    wk->tcbsys        = GFL_TCB_Init( 8, wk->task_wk_area );

    // タッチフォントロード
//    FontProc_LoadFont( FONT_TOUCH, HEAPID_WORLDTRADE );
    {
      WORLDTRADE_PARAM *wt_param  = param;

      WT_PRINT_Init( &wk->print, wt_param->config );
    }
    OS_Printf("-------------------------------プロック初期化\n");

    // 文字列マネージャー生成
    wk->WordSet        = WORDSET_CreateEx( 11, WORLDTRADE_WORDSET_BUFLEN, HEAPID_WORLDTRADE );
    wk->MsgManager       = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_gts_dat, HEAPID_WORLDTRADE );
    wk->LobbyMsgManager  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_lobby_dat, HEAPID_WORLDTRADE );
    wk->SystemMsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_system_dat, HEAPID_WORLDTRADE );
    wk->MonsNameManager  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_monsname_dat, HEAPID_WORLDTRADE );
    wk->CountryNameManager  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_place_msg_world_dat, HEAPID_WORLDTRADE );

  //  sys_KeyRepeatSpeedSet( SYS_KEYREPEAT_SPEED_DEF, SYS_KEYREPEAT_WAIT_DEF );

/* ＧＴＳとWifiバトルタワー接続画面の最初にWIPE_ResetBrightnessを呼んでしまっているために
   バックドロップ面が見えてしまう事があるバグを対処 */
#if AFTERMASTER_070215_GTS_WBTOWER_INIT_FIX
//    WIPE_ResetBrightness( WIPE_DISP_MAIN );
//    WIPE_ResetBrightness( WIPE_DISP_SUB );
#else
    WIPE_ResetBrightness( WIPE_DISP_MAIN );
    WIPE_ResetBrightness( WIPE_DISP_SUB );
#endif

//  //  GFL_UI_TP_Init();           // タッチパネルシステム初期化
//  //  InitTPNoBuff(2);


    // ワーク初期化
    InitWork( wk, param );

    WorldTrade_InitSystem( wk );

    WirelessIconEasy();

    // サウンドデータロード(フィールド)
    Snd_DataSetByScene( SND_SCENE_FIELD, SEQ_BLD_BLD_GTC, 1 );
    Snd_DataSetByScene( SND_SCENE_P2P, SEQ_GS_WIFI_ACCESS, 1 );

    // プラチナで、マップの曲と、画面の曲が変更になったので、
    // 画面から、フィールドに戻る時のためにシーンを変更

    // DWCライブラリ（Wifi）に渡すためのワーク領域を確保
    wk->heapPtr    = GFL_HEAP_AllocMemory(HEAPID_WORLDTRADE, MYDWC_HEAPSIZE + 32);
    wk->heapHandle = NNS_FndCreateExpHeap( (void *)( ((u32)wk->heapPtr + 31) / 32 * 32 ), MYDWC_HEAPSIZE);

    *seq = 1;
    break;

  case 1:
#if 0
    DwcOverlayStart();

    DpwCommonOverlayStart();
#else
    {
      //WIFIのオーバーレイを起動させるだけなので、
      //ほぼデータなし
      static const GFLNetInitializeStruct net_init =
      {
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
#if GFL_NET_WIFI
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        MYDWC_HEAPSIZE,
        TRUE,         //デバッグ用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
        0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
        GFL_HEAPID_APP,
        HEAPID_NETWORK,
        HEAPID_WIFI,
        HEAPID_IRC,
        GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
        _MAXNUM,     // 最大接続人数
        _MAXSIZE,  //最大送信バイト数
        _BCON_GET_NUM,    // 最大ビーコン収集数
        TRUE,
        FALSE,
        GFL_NET_TYPE_WIFI_GTS,
        TRUE,
        WB_NET_WIFIGTS,
#if GFL_NET_IRC
        IRC_TIMEOUT_STANDARD,
#endif  //GFL_NET_IRC
    0,//MP親最大サイズ 512まで
    0,//dummy
      };
      GFL_NET_Init( &net_init, NET_InitCallback, work );
    }
#endif


    // 会話ウインドウのタッチON
    MsgPrintTouchPanelFlagSet( MSG_TP_ON );


    (*seq) = SEQ_INIT_DPW;

    return GFL_PROC_RES_FINISH;
    break;
  }
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief    プロセス関数：メイン
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------

static GFL_PROC_RESULT WorldTradeProc_Main( GFL_PROC * proc, int * seq, void * param, void * work )
{
  WORLDTRADE_WORK * wk  = work;

  //EXITして内部でオーバーレイが解放されるまで呼べる
  if( *seq < SEQ_OUT )
  {
    // 受信強度リンクを反映させる
    DWC_UpdateConnection();
    // 通信状態を確認してアイコンの表示を変える
    WirelessIconEasy_SetLevel(WorldTrade_WifiLinkLevel());

    // Dpw_Tr_Main() だけは例外的にいつでも呼べる
    Dpw_Tr_Main();
  }

  GFL_TCB_Main( wk->tcbsys );
  WT_PRINT_Main( &wk->print );

  switch( *seq ){
  // サブ処理初期化
  case SEQ_INIT_DPW:
    //InitDpw(wk->heapPtr, wk->heapHandle, AllocFunc, FreeFunc );
    if(NET_IsInit(wk)){
      _wtHeapHandle = wk->heapHandle;
      // wifiメモリ管理関数呼び出し
      //DWC_SetMemFunc( AllocFunc, FreeFunc );
      *seq = SEQ_INIT;
    }
    break;
  case SEQ_INIT:
    *seq = (*SubProcessTable[wk->sub_process][0])(wk, *seq);
    WorldTrade_WndSetting();
    if(wk->subprocflag){
      //FreeCLACT( wk );
      OS_Printf("OAMシステム臨時解放");
    }
    break;

  // サブ処理フェードイン待ち
  case SEQ_FADEIN:
    if( WIPE_SYS_EndCheck() ){
      *seq = SEQ_MAIN;
    }
    break;

  // サブ処理メイン
  case SEQ_MAIN:
    *seq = (*SubProcessTable[wk->sub_process][1])(wk, *seq);

    break;

  // サブ処理フェードアウト待ち
  case SEQ_FADEOUT:
    if( WIPE_SYS_EndCheck() ){
      // サブ処理解放(SEQ_INITに行くか、SEQ_OUTにいくかはおまかせ）
      *seq = (*SubProcessTable[wk->sub_process][2])(wk, *seq);
      if(wk->subprocflag){
        //InitCLACT( wk );
        WorldTrade_SubLcdActorAdd( wk, MyStatus_GetMySex(wk->param->mystatus) );
        WorldTrade_SubLcdMatchObjAppear( wk, wk->SearchResult, 0 );
//        WorldTrade_SubLcdBgGraphicSet( wk );    // トレードルーム転送
//        WorldTrade_SubLcdWinGraphicSet( wk );   // トレードルームウインドウ転送
        wk->subprocflag = 0;
        OS_Printf("OAMシステム臨時復帰");
      }
    }
    break;

  // 世界交換終了処理
  case SEQ_OUT:
    GFL_NET_Exit(NULL);
    *seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    if( GFL_NET_IsExit() )
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }
  ServerWaitTimeFunc( wk );
  BoxPokeNumGet( wk );

  if(wk->clactSet!=NULL){
    GFL_CLACT_SYS_Main();
  }

  return GFL_PROC_RES_CONTINUE;
}

#define DEFAULT_NAME_MAX    18

// ダイヤ・パールで変わるんだろう
#define MALE_NAME_START     0
#define FEMALE_NAME_START   18

//--------------------------------------------------------------------------------------------
/**
 * @brief    プロセス関数：終了
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT WorldTradeProc_End( GFL_PROC * proc, int * seq, void * param, void * work )
{
  WORLDTRADE_WORK  *wk    = work;
  int i;

  GFL_HEAP_FreeMemory( wk->heapPtr );

#if 0
  DpwCommonOverlayEnd();
  DwcOverlayEnd();
#else
  //GFL_NET_Exit(NULL);
#endif

  // セルアクターリソース解放
  WirelessIconEasyEnd();



  // タッチパネルシステム終了
//  GFL_UI_TP_Exit();


  // メッセージマネージャー・ワードセットマネージャー解放

  GFL_MSG_Delete( wk->MonsNameManager );
  GFL_MSG_Delete( wk->SystemMsgManager );
  GFL_MSG_Delete( wk->LobbyMsgManager );
  GFL_MSG_Delete( wk->MsgManager );
  GFL_MSG_Delete( wk->CountryNameManager );
  WORDSET_Delete( wk->WordSet );

  WorldTrade_ExitSystem( wk );

  // ワーク解放
  FreeWork( wk );

  // イクニューモン解放
  //CommVRAMDFinalize();

  // メッセージウインドウのタッチOFF
  MsgPrintTouchPanelFlagSet( MSG_TP_OFF );

  // 世界交換パラメータ解放
  GFL_HEAP_FreeMemory( wk->param );


  GFL_PROC_FreeWork( proc );        // GFL_PROCワーク開放

  GFL_TCB_Exit( wk->tcbsys );
  GFL_HEAP_FreeMemory( wk->task_wk_area );

  // タッチフォントアンロード
//  FontProc_UnloadFont( FONT_TOUCH );
  WT_PRINT_Exit( &wk->print );


  GFL_TCB_DeleteTask( wk->vblank_task );

  GFL_HEAP_DeleteHeap( HEAPID_WORLDTRADE );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief    VBlank関数
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB *, void *work )
{
  WORLDTRADE_WORK *wk = work;
  // VBlank中転送ファンクション実行
  if(wk->vfunc){
    wk->vfunc(work);
    wk->vfunc = NULL;
  }
  if(wk->vfunc2){
    wk->vfunc2(work);
  }

  GFL_BG_VBlankFunc();

  // セルアクターVram転送マネージャー実行
  //DoVramTransferManager();
  GFL_CLACT_SYS_VBlankFunc();

  // レンダラ共有OAMマネージャVram転送
  //REND_OAMTrans();

  //ConnectBGPalAnm_VBlank(&wk->cbp);
  //InitでVTask読んでるのでなくなった？

//  OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief    VRAM設定
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void VramBankSet( const GFL_DISP_VRAM * vram )
{

  GFL_DISP_SetBank( vram );

}




//------------------------------------------------------------------
/**
 * @brief    世界交換ワーク初期化
 *
 * @param   wk    WORLDTRADE_WORK*
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void InitWork( WORLDTRADE_WORK *wk, WORLDTRADE_PARAM *param )
{
  GF_ASSERT( param->savedata != NULL );
  GF_ASSERT( param->worldtrade_data != NULL );
  GF_ASSERT( param->systemdata != NULL );
  GF_ASSERT( param->myparty != NULL );
  GF_ASSERT( param->mybox != NULL );
//  GF_ASSERT( param->zukanwork != NULL );
  GF_ASSERT( param->wifilist != NULL );
  GF_ASSERT( param->wifihistory != NULL );
  GF_ASSERT( param->mystatus != NULL );
  GF_ASSERT( param->config != NULL );
  GF_ASSERT( param->record != NULL );
//  GF_ASSERT( param->myitem != NULL );



  // 呼び出し時のパラメータを取得
  wk->param = param;

  wk->sub_process   = WORLDTRADE_ENTER;
    WorldTrade_SubProcessChange( wk, WORLDTRADE_ENTER, 0 );
  wk->TitleCursorPos  = 0;

  wk->Search.characterNo     = 0;
  wk->Search.gender          = PARA_UNK+1;
  wk->Search.level_min       = 0;
  wk->Search.level_max       = 0;
  wk->SearchBackup.characterNo = 0;
  wk->demo_end               = 0;
  wk->BoxTrayNo          = 18;
  wk->boxPokeNum         = 0;
  wk->boxSearchFlag      = 0;
  wk->SubLcdTouchOK      = 0;
  wk->timeWaitWork             = NULL;

  wk->country_code       = 0;
  wk->is_net_init       = FALSE;
}

//------------------------------------------------------------------
/**
 * @brief   ワーク解放
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void FreeWork( WORLDTRADE_WORK *wk )
{


}

//--------------------------------------------------------------
/**
 * @brief    ウィンドウ設定
 *
 * @param   none
 */
//--------------------------------------------------------------
static void WorldTrade_WndSetting(void)
{
  GXS_SetVisibleWnd(GX_WNDMASK_NONE);
  //受信強度アイコンにフェードがかからないようにウィンドウで囲む
  GX_SetVisibleWnd(GX_WNDMASK_W0);
  G2_SetWnd0InsidePlane(
    GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2
    | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ, FALSE);
  G2_SetWndOutsidePlane(
    GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2
    | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ, TRUE);
  G2_SetWnd0Position(256-16, 0, 255, 16);
}




//** CharManager PlttManager用 **//
#define RECORD_CHAR_CONT_NUM        (20)
#define RECORD_CHAR_VRAMTRANS_MAIN_SIZE   (2048)
#define RECORD_CHAR_VRAMTRANS_SUB_SIZE    (2048)
#define RECORD_PLTT_CONT_NUM        (20)

//-------------------------------------
//
//  キャラクタマネージャー
//  パレットマネージャーの初期化
//
//=====================================
static void char_pltt_manager_init(void)
{
#if 0
  // キャラクタマネージャー初期化
  {
    CHAR_MANAGER_MAKE cm = {
      RECORD_CHAR_CONT_NUM,
      RECORD_CHAR_VRAMTRANS_MAIN_SIZE,
      RECORD_CHAR_VRAMTRANS_SUB_SIZE,
      HEAPID_WORLDTRADE
    };
    InitCharManager(&cm);
  }
  // パレットマネージャー初期化
  InitPlttManager(RECORD_PLTT_CONT_NUM, HEAPID_WORLDTRADE);

  // 読み込み開始位置を初期化
  CharLoadStartAll();
  PlttLoadStartAll();
#endif
}


#define TRANS_POKEICON_COLOR_NUM  ( 3*16 )
//------------------------------------------------------------------
/**
 * @brief    レーダー画面用セルアクター初期化
 *
 * @param   wk    レーダー構造体のポインタ
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void InitCellActor(WORLDTRADE_WORK *wk, const GFL_DISP_VRAM * vram )
{
  int i;
  ARCHANDLE* p_handle;

  p_handle = GFL_ARC_OpenDataHandle( ARCID_WORLDTRADE_GRA, HEAPID_WORLDTRADE );

  // 共有OAMマネージャ作成
  // レンダラ用OAMマネージャ作成
  // ここで作成したOAMマネージャをみんなで共有する
  {
    static const GFL_CLSYS_INIT clsys_init =
    {
      0, 0,
      0, NAMEIN_SUB_ACTOR_DISTANCE,
      4, 124,
      4, 124,
      0,
      32,32,32,32,
      16, 16,
    };

    GFL_CLACT_SYS_Create( &clsys_init, vram, HEAPID_WORLDTRADE );
  }

  // セルアクター初期化
  wk->clactSet = GFL_CLACT_UNIT_Create( 72+12, 0, HEAPID_WORLDTRADE );

  //CLACT_U_SetSubSurfaceMatrix( &wk->renddata, 0, NAMEIN_SUB_ACTOR_DISTANCE );


  //---------上画面用-------------------
  //chara読み込み
  wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES] = GFL_CLGRP_CGR_Register( p_handle, NARC_worldtrade_worldtrade_obj_lz_ncgr, 1, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE);

  //pal読み込み
  wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES] = GFL_CLGRP_PLTT_Register(p_handle, NARC_worldtrade_worldtrade_obj_nclr,CLSYS_DRAW_MAIN, /*3*0x20*/0, HEAPID_WORLDTRADE);

  //cell読み込み
  wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES] = GFL_CLGRP_CELLANIM_Register( p_handle, NARC_worldtrade_worldtrade_obj_NCER, NARC_worldtrade_worldtrade_obj_NANR, HEAPID_WORLDTRADE);


  //---------下画面用-------------------


  //chara読み込み
  wk->resObjTbl[SUB_LCD][CLACT_U_CHAR_RES] = GFL_CLGRP_CGR_Register( p_handle, NARC_worldtrade_hero_lz_ncgr, 1, CLSYS_DRAW_SUB, HEAPID_WORLDTRADE);

  //pal読み込み
  wk->resObjTbl[SUB_LCD][CLACT_U_PLTT_RES] = GFL_CLGRP_PLTT_Register(p_handle, NARC_worldtrade_hero_nclr, CLSYS_DRAW_SUB, /*10*0x20*/0, HEAPID_WORLDTRADE);

  //cell読み込み
  wk->resObjTbl[SUB_LCD][CLACT_U_CELL_RES] = GFL_CLGRP_CELLANIM_Register(p_handle, NARC_worldtrade_worldtrade_obj_s_NCER, NARC_worldtrade_worldtrade_obj_s_NANR, HEAPID_WORLDTRADE);


  // ポケモンアイコン用パレットを一気に読み込んでVRAM転送する
  // ポケモンアイコンのパレットを暗くしたデータも作成して転送する
  {
    void              *buf;
    NNSG2dPaletteData *palData;
    int i,r,g,b;
    u16 *pal;

    buf = GFL_ARC_UTIL_LoadPalette( ARCID_POKEICON, NARC_poke_icon_poke_icon_NCLR, &palData, HEAPID_WORLDTRADE );
    DC_FlushRange( palData->pRawData, TRANS_POKEICON_COLOR_NUM*2 );
    GX_LoadOBJPltt( palData->pRawData, POKEICON_PAL_OFFSET*0x20, TRANS_POKEICON_COLOR_NUM*2 );

    pal = (u16*)palData->pRawData;
    for(i=0;i<TRANS_POKEICON_COLOR_NUM;i++){
      r = pal[i]>>10&0x1f;
      g = (pal[i]>>5)&0x1f;
      b = pal[i]&0x1f;

      r /=2;  g /= 2; b /=2;
      pal[i] = (r<<10)|(g<<5)|b;
    }

    DC_FlushRange( palData->pRawData, TRANS_POKEICON_COLOR_NUM*2 );
    GX_LoadOBJPltt( palData->pRawData, (POKEICON_PAL_OFFSET+3)*0x20, TRANS_POKEICON_COLOR_NUM*2 );

    GFL_HEAP_FreeMemory(buf);
  }

  GFL_ARC_CloseDataHandle( p_handle );
}

#define TRAINER_NAME_POS_X    ( 24 )
#define TRAINER_NAME_POS_Y    ( 32 )
#define TRAINER_NAME_POS_SPAN ( 32 )

#define TRAINER_NAME_WIN_X    (  3 )
#define TRAINER1_NAME_WIN_Y   (  6 )
#define TRAINER2_NAME_WIN_Y   (  7 )

static const u16 obj_pos_tbl[][2]={
  {212,64},
  {187,77},
  {236,77},
  {194,120},
  {224,120},
};

//=============================================================================================
/**
 * @brief セルアクターヘッダ作成ルーチン
 *
 * @param   add   セルアクターヘッダデータの書き込み先
 * @param   wk    GTS画面ワーク
 * @param   header  ヘッダーデータ
 * @param   param 描画エリア（MAIN/SUB)
 */
//=============================================================================================
void WorldTrade_MakeCLACT( CLACT_ADD *add, WORLDTRADE_WORK *wk, CLACT_HEADER *header, int param)
{
#if 0 //新clactではヘッダはいらない
  add->ClActSet   = wk->clactSet;
  add->ClActHeader  = header;

  add->mat.z    = 0;
  add->sca.x    = FX32_ONE;
  add->sca.y    = FX32_ONE;
  add->sca.z    = FX32_ONE;
  add->rot    = 0;
  add->pri    = 1;
  add->DrawArea = param;
  add->heap   = HEAPID_WORLDTRADE;
#endif
}

//------------------------------------------------------------------
/**
 * @brief   セルアクター登録
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void SetCellActor(WORLDTRADE_WORK *wk)
{
#if 0 //新clactではヘッダはいらない
  int i;
  // セルアクターヘッダ作成
  GFL_CLACT_WK_SetCellResData or GFL_CLACT_WK_SetTrCellResData or GFL_CLACT_WK_SetMCellResData(&wk->clActHeader_main, 0, 0, 0, 0, CLACT_U_HEADER_DATA_NONE, CLACT_U_HEADER_DATA_NONE,
  0, 0,
  wk->resMan[CLACT_U_CHAR_RES],
  wk->resMan[CLACT_U_PLTT_RES],
  wk->resMan[CLACT_U_CELL_RES],
  wk->resMan[CLACT_U_CELLANM_RES],
  NULL,NULL);
  //まだ何も転送していないから

  // セルアクターヘッダ作成
  GFL_CLACT_WK_SetCellResData or GFL_CLACT_WK_SetTrCellResData or GFL_CLACT_WK_SetMCellResData(&wk->clActHeader_sub, 1, 1, 1, 1, CLACT_U_HEADER_DATA_NONE, CLACT_U_HEADER_DATA_NONE,
  0, 0,
  wk->resMan[CLACT_U_CHAR_RES],
  wk->resMan[CLACT_U_PLTT_RES],
  wk->resMan[CLACT_U_CELL_RES],
  wk->resMan[CLACT_U_CELLANM_RES],
  NULL,NULL);
#endif
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_ON ); //メイン画面OBJ面ＯＮ
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON ); //サブ画面OBJ面ＯＮ
}



// はい・いいえ
#define BMP_YESNO_PX  ( 24 )

#define BMP_YESNO_PY  ( 13 )
#define BMP_YESNO_SX  ( 7 )
#define BMP_YESNO_SY  ( 4 )
#define BMP_YESNO_PAL ( 13 )

#if 0
// はい・いいえ(ウインドウ用）
static const BMPWIN_DAT YesNoBmpWin = {
  GFL_BG_FRAME0_M, BMP_YESNO_PX, BMP_YESNO_PY,
  BMP_YESNO_SX, BMP_YESNO_SY, BMP_YESNO_PAL,
  0, //後で指定する
};
#endif //使っていないらしい


BMPMENU_WORK *WorldTrade_BmpWinYesNoMake( int y, int yesno_bmp_cgx )
{
#if 0
  BMPWIN_DAT yesnowin;

  yesnowin        = YesNoBmpWin;
  yesnowin.pos_y  = y;
  yesnowin.chrnum = yesno_bmp_cgx;

  return BmpYesNoSelectInit( &yesnowin, WORLDTRADE_MENUFRAME_CHR, WORLDTRADE_MENUFRAME_PAL, HEAPID_WORLDTRADE );
#endif
  //使っていないらしい
  return NULL;
}
//==============================================================================
/**
 * @brief   タッチ対応YESNOウインドウ（メインのみ）
 *
 * @param   bgl       GF_BGL_INI
 * @param   y       Y座標
 * @param   yesno_bmp_cgx VRAMオフセット位置
 * @param   pltt      パレット指定
 * @param inPassive   パッシブするか
 *
 * @retval  TOUCH_SW_WORK *   はい・いいえ画面ワーク
 */
//==============================================================================
TOUCH_SW_SYS *WorldTrade_TouchWinYesNoMake( int y, int yesno_bmp_cgx, int pltt, u8 inPassive )
{
  return WorldTrade_TouchWinYesNoMakeEx( y, yesno_bmp_cgx, pltt, GFL_BG_FRAME0_M, inPassive );
}

//==============================================================================
/**
 * @brief   タッチ対応YESNOウインドウ(フレーム指定版)
 *
 * @param   bgl       GF_BGL_INI
 * @param   y           Y座標
 * @param   yesno_bmp_cgx VRAMオフセット位置
 * @param   pltt        パレット指定
 * @param   frame     表示フレーム指定
 * @param inPassive   パッシブするか
 *
 * @retval  TOUCH_SW_SYS *    はい・いいえ画面ワーク
 */
//==============================================================================
TOUCH_SW_SYS *WorldTrade_TouchWinYesNoMakeEx( int y, int yesno_bmp_cgx,
                        int pltt, int frame, u8 inPassive )
{
  TOUCH_SW_SYS *tss = TOUCH_SW_AllocWork( HEAPID_WORLDTRADE );
  TOUCH_SW_PARAM  param;

  if (inPassive){
    u8 target = 0;
    if (frame <= GFL_BG_FRAME3_M){
      target = 1;   //対象はメイン画面
    }
    WorldTrade_SetPassive(target);
  }

  param.bg_frame  = frame;
  param.char_offs = yesno_bmp_cgx;
  param.pltt_offs = pltt;
  param.x         = BMP_YESNO_PX;
  param.y         = y;
  param.kt_st     = GFL_APP_KTST_KEY;
  param.key_pos   = 0;
  param.type      = TOUCH_SW_TYPE_S;

  TOUCH_SW_Init( tss, &param );

  return tss;
}

//------------------------------------------------------------------
/**
 * @brief    YesNoウィンドウ監視
 *
 * @param   wk      WORLDTRADE_WORK*
 *
 * @retval  none
 */
//------------------------------------------------------------------
u32 WorldTrade_TouchSwMain(WORLDTRADE_WORK* wk)
{
  u32 ret = TOUCH_SW_Main( wk->tss );
  if ( (ret==TOUCH_SW_RET_YES) || (ret==TOUCH_SW_RET_NO) ){
    //パッシブ解除
    WorldTrade_ClearPassive();
  }

  return ret;
}


// 選択ボックスの幅
#define WORLDTRADESELBOX_X  ( 32 - (WORLDTRADE_SELBOX_W+2)) // 幅0の時は20
#define WORLDTRADESELBOX_CGX_SIZE ( WORLDTRADE_SELBOX_W*2 )
#define WORLDTRADESEL_MAX (3)

///セレクトボックス　ヘッダデータ構造体
static const SELBOX_HEAD_PRM sbox_sel = {
  FALSE,SBOX_OFSTYPE_CENTER,        ///<ループフラグ、左寄せorセンタリング
  0,                    ///<項目表示オフセットX座標(dot)
  GFL_BG_FRAME0_M,            ///<フレームNo
  10,                   ///<フレームパレットID
  0,0,                  ///oam BG Pri,Software Pri
  0x3000/32,                ///<ウィンドウcgx
  0x3000/32+WORLDTRADESELBOX_CGX_SIZE*WORLDTRADESEL_MAX,  // フレームcgx
  WORLDTRADESELBOX_CGX_SIZE*WORLDTRADESEL_MAX,///<BMPウィンドウ占有キャラクタ領域サイズ(char)
};

//==============================================================================
/**
 * @brief   選択ボックスシステム呼び出し
 *
 * @param   wk    GTS画面ワーク
 * @param   count リストの個数
 * @param   y   表示位置Y座標
 *
 * @retval  SELBOX_WORK*    選択ボックスワークのポインタ
 */
//==============================================================================
SELBOX_WORK* WorldTrade_SelBoxInit( WORLDTRADE_WORK *wk, u8 frm, int count, int y )
{
  SELBOX_WORK* list_wk;

  wk->SelBoxSys = SelectBoxSys_AllocWork( HEAPID_WORLDTRADE ,NULL );

  // SELBOX描画登録
  {
    SELBOX_HEADER head;

    MI_CpuClear8(&head,sizeof(SELBOX_HEADER));

    head.prm      = sbox_sel;
    head.prm.frm  = frm;
    head.list = (const BMPLIST_DATA*)wk->BmpMenuList;
    head.fontHandle = wk->print.font;

    head.count = count;

    list_wk = SelectBoxSetEx( wk->SelBoxSys, &head, NULL, WORLDTRADE_SELBOX_X, y, WORLDTRADE_SELBOX_W, 0, WorldTrade_SelBoxCallback, NULL, TRUE );
  }

  //パッシブ
  WorldTrade_SetPassive(1);

  return list_wk;
}

//--------------------------------------------------------------
/**
 * @brief    選択ボックスシステムのコールバック
 *
 * @param   sbox    選択ボックスワークのポインタ
 * @param   cur_pos   カーソル位置
 * @param   work    選択ボックスに登録してワークポインタ
 * @param   mode    選択モード
 */
//--------------------------------------------------------------
static void WorldTrade_SelBoxCallback(SELBOX_WORK* sbox, u8 cur_pos, void* work, SBOX_CB_MODE mode)
{
    switch(mode){
    case SBOX_CB_MODE_INI:  //初期化時
        break;
    case SBOX_CB_MODE_MOVE: //カーソル移動時
    case SBOX_CB_MODE_DECIDE:   //選択肢決定
    case SBOX_CB_MODE_CANCEL: //キャンセル
        PMSND_PlaySE(SEQ_SE_DP_SELECT);
        break;
    }
}

//==============================================================================
/**
 * @brief   選択ボックスシステム終了
 *
 * @param   wk    GTS画面ワーク
 * @param   count リストの個数
 * @param   y   表示位置Y座標
 *
 * @retval  SELBOX_WORK*    選択ボックスワークのポインタ
 */
//==============================================================================
void WorldTrade_SelBoxEnd( WORLDTRADE_WORK *wk )
{
  SelectBoxExit( wk->SelBoxWork );
  SelectBoxSys_Free( wk->SelBoxSys );
  //パッシブ解除
  WorldTrade_ClearPassive();
}


//------------------------------------------------------------------
/**
 * @brief   次の次のシーケンスを予約しておく
 *
 * @param   wk      GTS画面ワーク
 * @param   to_seq    次のシーケンス
 * @param   next_seq  次の次のシーケンス
 *
 * @retval  none
 */
//------------------------------------------------------------------
void WorldTrade_SetNextSeq( WORLDTRADE_WORK *wk, int to_seq, int next_seq )
{
  wk->subprocess_seq      = to_seq;
  wk->subprocess_nextseq  = next_seq;
}

//=============================================================================================
/**
 * @brief サブプロセス終了時の次の行き先を入力する
 *
 * @param   wk        GTS画面ワーク
 * @param   next_process  次のシーケンス
 */
//=============================================================================================
void WorldTrade_SetNextProcess( WORLDTRADE_WORK *wk, int next_process )
{
  wk->sub_nextprocess = next_process;
}

//==============================================================================
/**
 * @brief   CLACTの座標変更
 *
 * @param   act   アクターのポインタ
 * @param   x   X座標
 * @param   y   Y座標
 *
 * @retval  none
 */
//==============================================================================
void WorldTrade_CLACT_PosChangeSub( GFL_CLWK* act, int x, int y )
{
  GFL_CLACTPOS pos;

  pos.x = x;
  pos.y = y;
  GFL_CLACT_WK_SetPos( act, &pos, CLSYS_DRAW_SUB );

}


/*---------------------------------------------------------------------------*
  メモリ確保関数
 *---------------------------------------------------------------------------*/
static void *AllocFunc( DWCAllocType name, u32   size, int align )
{
#pragma unused( name )
    void * ptr;
    OSIntrMode old;
    old = OS_DisableInterrupts();
    ptr = NNS_FndAllocFromExpHeapEx( _wtHeapHandle, size, align );
    OS_RestoreInterrupts( old );
    if(ptr == NULL){
  }

    return ptr;
}

/*---------------------------------------------------------------------------*
  メモリ開放関数
 *---------------------------------------------------------------------------*/
static void FreeFunc(DWCAllocType name, void* ptr,  u32 size)
{
#pragma unused( name, size )
    OSIntrMode old;

    if ( !ptr ) return;
    old = OS_DisableInterrupts();
    NNS_FndFreeToExpHeap( _wtHeapHandle, ptr );
    OS_RestoreInterrupts( old );
}



//------------------------------------------------------------------
/**
 * @brief   NitroDpw_Tr初期化
 *
 * @param   heapPtr   ヒープに割り当てるメモリバッファ
 * @param   headHandle  NNSからもらうヒープハンドル
 * @param   alloc   アローケーターのハンドル
 * @param   free    アロケーターの解放ハンドル
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void InitDpw( void *heapPtr, NNSFndHeapHandle heapHandle, DWCAllocEx alloc, DWCFreeEx free  )
{


  _wtHeapHandle = heapHandle;

  // イクニューモン転送
//  CommVRAMDInitialize();

  // wifiメモリ管理関数呼び出し
  //DWC_SetMemFunc( alloc, free );

  ;

}






//==============================================================================
/**
 * @brief   WIFI接続状況取得
 *
 * @param   none
 *
 * @retval  int   接続強度取得
 */
//==============================================================================
int WorldTrade_WifiLinkLevel( void )
{
    return WM_LINK_LEVEL_3 - DWC_GetLinkLevel();
}




//==============================================================================
/**
 * @brief   サブプロセスモードの切り替え指定
 *
 * @param   wk      GTS画面ワーク
 * @param   subprccess  指定の画面モード（ENTER,TITLE,SEACHなど）
 * @param   mode    画面モード内の開始条件指定(MODE_...)
 *
 * @retval  none
 */
//==============================================================================
void WorldTrade_SubProcessChange( WORLDTRADE_WORK *wk, int subprccess, int mode )
{
  wk->sub_nextprocess  = subprccess;
  wk->sub_process_mode = mode;
}


//==============================================================================
/**
 * @brief   次のサブプロセスへ遷移する（一つ前を取っておく）
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  none
 */
//==============================================================================
void WorldTrade_SubProcessUpdate( WORLDTRADE_WORK *wk )
{
  wk->old_sub_process = wk->sub_process;
  wk->sub_process     = wk->sub_nextprocess;

}
//==============================================================================
/**
 * @brief   会話スピードをコンフィグデータから取得する
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  int   メッセージスピード
 */
//==============================================================================
int WorldTrade_GetTalkSpeed( WORLDTRADE_WORK *wk )
{
  return MSGSPEED_GetWait();
}

//==============================================================================
/**
 * @brief   会話ウインドウの形状をコンフィグデータから取得する
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  WINTYPE 会話ウインドウのスキン番号
 */
//==============================================================================
WINTYPE WorldTrade_GetMesWinType( WORLDTRADE_WORK *wk )
{
  return CONFIG_GetWindowType( wk->param->config );
}












//------------------------------------------------------------------
/**
 * @brief   セルアクター初期化
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void InitCLACT( WORLDTRADE_WORK *wk )
{
  static const GFL_DISP_VRAM tbl =
  {
    GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット

    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット

    GX_VRAM_OBJ_64_E,       // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

    GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット

    GX_VRAM_TEX_0_B,        // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_01_FG,      // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_32K,
    GX_OBJVRAMMODE_CHAR_1D_32K,
  };

  // VRAM バンク設定
  VramBankSet( &tbl );

  // CellActorシステム初期化
  InitCellActor(wk, &tbl);

  // CellActro表示登録
  SetCellActor(wk);


  // VBlank関数セット
  wk->vblank_task = GFUser_VIntr_CreateTCB(VBlankFunc, wk, 0);

}

//------------------------------------------------------------------
/**
 * @brief   セルアクター解放
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void FreeCLACT( WORLDTRADE_WORK *wk )
{
  int i;


  // ユニオンＯＢＪグラフィックデータ解放
  FreeFieldObjData( wk );

  // キャラ・パレット・セル・セルアニメのリソースマネージャー破棄
  GFL_CLGRP_PLTT_Release( wk->resObjTbl[SUB_LCD][CLACT_U_PLTT_RES] );
  GFL_CLGRP_CGR_Release( wk->resObjTbl[SUB_LCD][CLACT_U_CHAR_RES] );
  GFL_CLGRP_CELLANIM_Release( wk->resObjTbl[SUB_LCD][CLACT_U_CELL_RES] );

  GFL_CLGRP_PLTT_Release( wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES] );
  GFL_CLGRP_CGR_Release( wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES] );
  GFL_CLGRP_CELLANIM_Release( wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES] );

  // セルアクターセット破棄
  GFL_CLACT_UNIT_Delete(wk->clactSet);
  wk->clactSet = NULL;

  //OAMレンダラー破棄
  GFL_CLACT_SYS_Delete();


}


//------------------------------------------------------------------
/**
 * @brief   0になるまで１減らす
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void ServerWaitTimeFunc( WORLDTRADE_WORK *wk )
{

  if(wk->serverWaitTime){
    wk->serverWaitTime--;
  }
}

//==============================================================================
/**
 * @brief   １syncに１トレイずつBOXの数を確認していくスタート
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  none
 */
//==============================================================================
void WorldTrade_BoxPokeNumGetStart( WORLDTRADE_WORK *wk )
{
  wk->boxSearchFlag = 1;
  wk->boxPokeNum    = 0;
}


//------------------------------------------------------------------
/**
 * @brief   １syncに１トレイずつBOXの数を確認していく(常駐処理）
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void BoxPokeNumGet( WORLDTRADE_WORK *wk )
{
  if(wk->boxSearchFlag){
//    wk->boxPokeNum += BOXDAT_GetPokeExistCount( wk->param->mybox, wk->boxSearchFlag-1 );
    wk->boxPokeNum  = 0;
    wk->boxSearchFlag++;
    if(wk->boxSearchFlag==19){
      wk->boxSearchFlag = 0;
      OS_Printf("BOX総数は%d\n",wk->boxPokeNum);
    }

  }

}



//------------------------------------------------------------------
/**
 * @brief   時間アイコン追加
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  none
 */
//------------------------------------------------------------------
void WorldTrade_TimeIconAdd( WORLDTRADE_WORK *wk )
{
  wk->timeWaitWork = TimeWaitIconAdd( wk->MsgWin, WORLDTRADE_MESFRAME_CHR );
}

//------------------------------------------------------------------
/**
 * @brief   時間アイコン消去（NULLチェックする）
 *
 * @param   wk    GTS画面ワーク
 *
 * @retval  none
 */
//------------------------------------------------------------------
void WorldTrade_TimeIconDel( WORLDTRADE_WORK *wk )
{
  if(wk->timeWaitWork!=NULL){
    TimeWaitIconDel(wk->timeWaitWork);
    wk->timeWaitWork = NULL;
  }
}

//==============================================================================
/**
 * @brief   セルアクターの座標変更（簡易処理版）
 *
 * @param   ptr   clact_work_ptr
 * @param   x   X
 * @param   y   Y
 *
 * @retval  none
 */
//==============================================================================
void WorldTrade_CLACT_PosChange( GFL_CLWK* ptr, int x, int y )
{
  GFL_CLACTPOS pos;

  pos.x = x;
  pos.y = y;
  GFL_CLACT_WK_SetPos( ptr, &pos, CLSYS_DRAW_MAIN );

}

//------------------------------------------------------------------
/**
 * @brief パッシブ状態セット
 * @param inIsMain    対象がメイン画面か？  YES:1 NO:0
 * @return  none
 */
//------------------------------------------------------------------
void WorldTrade_SetPassive(u8 inIsMain)
{
  if (inIsMain){
    G2_SetBlendBrightness(
      GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ,
      BRIGHT_VAL
    );
  }else{
    G2S_SetBlendBrightness(
      GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ,
      BRIGHT_VAL
    );
  }
}

//------------------------------------------------------------------
/**
 * @brief パッシブ状態セット  mypoke用
 * @param inIsMain    対象がメイン画面か？  YES:1 NO:0
 * @return  none
 */
//------------------------------------------------------------------
void WorldTrade_SetPassiveMyPoke(u8 inIsMain)
{
  if (inIsMain){
    G2_SetBlendBrightness(
      GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ,
      BRIGHT_VAL
    );
  }else{
    G2S_SetBlendBrightness(
      GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ,
      BRIGHT_VAL
    );
  }
}

//------------------------------------------------------------------
/**
 * @brief パッシブ状態解除
 * @param none
 * @return  none
 */
//------------------------------------------------------------------
void WorldTrade_ClearPassive(void)
{
  G2_BlendNone();
  G2S_BlendNone();
}

//----------------------------------------------------------------------------
/**
 *  @brief  システム系を初期化
 *
 *  @param  WORLDTRADE_WORK *wk ワーク
 *
 */
//-----------------------------------------------------------------------------
void WorldTrade_InitSystem( WORLDTRADE_WORK *wk )
{
  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );


  InitCLACT( wk );

  GFL_BG_Init( HEAPID_WORLDTRADE );
  GFL_BMPWIN_Init( HEAPID_WORLDTRADE );


  // BG SYSTEM
  {
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &BGsys_data );
  }

  WorldTrade_WndSetting();

}

//----------------------------------------------------------------------------
/**
 *  @brief  システム系を破棄
 *
 *  @param  WORLDTRADE_WORK *wk ワーク
 */
//-----------------------------------------------------------------------------
void WorldTrade_ExitSystem( WORLDTRADE_WORK *wk )
{
  FreeCLACT( wk );

  GFL_BMPWIN_Exit();
  GFL_BG_Exit();

  GX_SetVisibleWnd(GX_WNDMASK_NONE);
  GXS_SetVisibleWnd(GX_WNDMASK_NONE);


}

//----------------------------------------------------------------------------
/**
 *  @brief  netの初期化終了コールバック
 *
 *  @param  void *wk_adrs   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void NET_InitCallback( void *wk_adrs )
{
  WORLDTRADE_WORK * wk  = wk_adrs;
  wk->is_net_init = TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  netの初期化終了したかどうか
 *
 *  @param  const WORLDTRADE_WORK *wk   ワーク
 *
 *  @return TRUEならば初期化終了  FALSEならばまだ
 */
//-----------------------------------------------------------------------------
static BOOL NET_IsInit( const WORLDTRADE_WORK *wk )
{
  return wk->is_net_init;
}

