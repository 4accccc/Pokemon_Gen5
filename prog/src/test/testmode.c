//===================================================================
/**
 * @file  testmode.c
 */
//===================================================================
#include <wchar.h>
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_menuwork.h"
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"
#include "msg/msg_debugname.h"

#include "test/testmode.h"
#include "title/title.h"
#include "title/game_start.h"
#include "gamesystem/game_init.h"
#include "savedata/mystatus.h"
#include "app/config_panel.h"   //ConfigPanelProcData参照
#include "infowin/infowin.h"
#include "net_app/union/union_beacon_tool.h"
#include "sound/pm_sndsys.h"  //PMSND_PlaySystemSE

#include "debug/gf_mcs.h"
//======================================================================
//  define
//======================================================================

#define BG_PLANE_MENU GFL_BG_FRAME1_M
#define ITEM_NAME_MAX 32
#define TESTMODE_PLT_FONT 15
#define LIST_DISP_MAX 5
//======================================================================
//  enum
//======================================================================
typedef enum
{
  TMS_INIT_MENU,
  TMS_SELECT_ITEM,
  TMS_TERM_MENU,
  TMS_CHECK_NEXT,

  TMS_MAX,
}TESTMODE_STATE;

typedef enum
{
  TMN_CHANGE_PROC,
  TMN_CHANGE_MENU,
  TMN_GAME_START,

  TMN_CANCEL_MENU,  //Bボタン抜け
  TMN_EXIT_PROC,    //何もせずProcを抜ける

  TMN_MAX,

}TESTMODE_NEXTACT;

typedef enum
{
  TMI_NEWGAME,
  TMI_CONTINUE,
  TMI_CONTINUE_NET_OFF,
  TMI_CONTINUE_NET,
  TMI_DEBUG_START,
  TMI_DEBUG_START_NET,
  TMI_DEBUG_START_NET_OFF,
  TMI_DEBUG_SELECT_NAME,

  TMI_MAX,

}TESTMODE_INITGAME_TYPE;

//======================================================================
//  typedef struct
//======================================================================
typedef struct _TESTMODE_MENU_LIST TESTMODE_MENU_LIST;

typedef struct
{
  HEAPID heapId_;
  void  *parentWork_;
  SAVE_CONTROL_WORK * saveControl_;
  TESTMODE_STATE state_;
  TESTMODE_NEXTACT nextAction_;
  u8  refreshCount_;
  RTCDate rtcDate_;
  RTCTime rtcTime_;

  TESTMODE_MENU_LIST *currentMenu_;
  u16 currentItemNum_;
  TESTMODE_MENU_LIST *nextMenu_;
  u16 nextItemNum_;

  BMP_MENULIST_DATA *menuList_;
  BMPMENULIST_WORK  *menuWork_;
  PRINT_UTIL      printUtil_;
  PRINT_UTIL      printUtilSub_;
  PRINT_QUE     *printQue_;
  PRINT_QUE     *printQueSub_;
  GFL_FONT      *fontHandle_;
  GFL_BMPWIN      *bmpWin_;
  GFL_BMPWIN      *bmpWinSub_;  //下の情報画面

  //Proc切り替え用
  FSOverlayID overlayId_;
  const GFL_PROC_DATA *procData_;
  void *procWork_;

  //新規開始用初期化種類
  TESTMODE_INITGAME_TYPE  gameType_;

}TESTMODE_WORK;


//メニュー決定時の処理
//  @value  ワーク・選択されたIndex
//  @return メニューを抜けるか？
typedef BOOL (*TestMode_SelectFunc)( TESTMODE_WORK *work , const int idx );
struct _TESTMODE_MENU_LIST
{
  u16 str_[ITEM_NAME_MAX];
  TestMode_SelectFunc selectFunc_;
};

//======================================================================
//  proto
//======================================================================
static void TESTMODE_InitGraphic( TESTMODE_WORK *work );
static void TESTMODE_InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane );

static void TESTMODE_CreateMenu( TESTMODE_WORK *work , TESTMODE_MENU_LIST *menuList , const u16 itemNum );
static BOOL TESTMODE_UpdateMenu( TESTMODE_WORK *work );

static void TESTMODE_RefreshSubWindow( TESTMODE_WORK *work );

//メニュー決定時の各種処理設定関数
static void TESTMODE_COMMAND_ChangeProc( TESTMODE_WORK *work ,FSOverlayID ov_id, const GFL_PROC_DATA *procdata, void *pwork );
static void TESTMODE_COMMAND_ChangeMenu( TESTMODE_WORK *work , TESTMODE_MENU_LIST *menuList , const u16 itemNum );
static void TESTMODE_COMMAND_StartGame( TESTMODE_WORK *work , TESTMODE_INITGAME_TYPE type );

//メニュー決定時の関数群
static BOOL TESTMODE_ITEM_SelectFuncDebugStart( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncDebugStartComm( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncDebugStartCommOff( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncContinue( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncContinueComm( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncChangeSelectName( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncWatanabe( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncTamada( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncSogabe( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncOhno( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncTaya( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncSample1( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncMatsuda( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncSave( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncSound( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncKagaya( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncAri( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncNagi( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncObata( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncIwasawa( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncSaito( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncMori( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncNakamura( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncHosaka( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_BackTopMenu( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncRTCEdit( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_ChangeRTC( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_ChangeMusicalMenu( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectFuncSelectName( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_ConnectMCS( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_DebugFight( TESTMODE_WORK *work , const int idx );

//------------------------------------------------------------------------
/*
 *  QUICKSTART -> STARTボタン一発で任意のメニューを起動するための関数設定
 */
//------------------------------------------------------------------------
#if defined DEBUG_ONLY_FOR_taya
  #define QuickSelectFunc   TESTMODE_ITEM_DebugFight
#elif defined DEBUG_ONLY_FOR_watanabe
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncWatanabe
#elif defined DEBUG_ONLY_FOR_tamada
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncTamada
#elif defined DEBUG_ONLY_FOR_sogabe
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncSogabe
#elif defined DEBUG_ONLY_FOR_ohno
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncOhno
#elif defined DEBUG_ONLY_FOR_matsuda
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncMatsuda
#elif defined DEBUG_ONLY_FOR_kagaya
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncKagaya
#elif defined DEBUG_ONLY_FOR_ariizumi_nobuhiko
//  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncDressUp
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncAri
#elif defined DEBUG_ONLY_FOR_toru_nagihashi
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncNagi
#elif defined DEBUG_ONLY_FOR_fujiwara
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncSogabe
#elif defined DEBUG_ONLY_FOR_toyama
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncSogabe
#elif defined DEBUG_ONLY_FOR_shota_kageyama
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncSogabe
#elif defined DEBUG_ONLY_FOR_sounduser
  #define QuickSelectFunc   TESTMODE_ITEM_SelectFuncSogabe
#endif


static TESTMODE_MENU_LIST topMenu[] =
{
  //汎用
  {L"デバッグ開始(処理負荷：サーチ)"        ,TESTMODE_ITEM_SelectFuncDebugStart },
  {L"デバッグ開始(処理負荷：接続(要侵入)"  ,TESTMODE_ITEM_SelectFuncDebugStartComm },
  {L"デバッグ開始(通信OFF)"  ,TESTMODE_ITEM_SelectFuncDebugStartCommOff },
  {L"続きから"            ,TESTMODE_ITEM_SelectFuncContinue },
  {L"続きから(通信)"      ,TESTMODE_ITEM_SelectFuncContinueComm },
  {L"名前を選んで開始(処理負荷：サーチ)"    ,TESTMODE_ITEM_SelectFuncChangeSelectName },
  {L"RTC調整"             ,TESTMODE_ITEM_SelectFuncRTCEdit },
  {L"セーブ破かい"        ,TESTMODE_ITEM_SelectFuncSave },
  {L"SOUND"               ,TESTMODE_ITEM_SelectFuncSound },
  {L"ミュージカル"        ,TESTMODE_ITEM_ChangeMusicalMenu },
  {L"MCS常駐接続"         ,TESTMODE_ITEM_ConnectMCS },
  {L"デバッグバトル"      ,TESTMODE_ITEM_DebugFight },

  //個人
  {L"わたなべ　てつや"    ,TESTMODE_ITEM_SelectFuncWatanabe },
  {L"たまだ　そうすけ"    ,TESTMODE_ITEM_SelectFuncTamada },
  {L"そがべ　ひさし"      ,TESTMODE_ITEM_SelectFuncSogabe },
  {L"おおの　かつみ"      ,TESTMODE_ITEM_SelectFuncOhno },
  {L"たや　まさお"        ,TESTMODE_ITEM_SelectFuncTaya },
//  {L"Sample1"       ,TESTMODE_ITEM_SelectFuncSample1 },
  {L"まつだ　よしのり"    ,TESTMODE_ITEM_SelectFuncMatsuda },
  {L"かがや　けいた"      ,TESTMODE_ITEM_SelectFuncKagaya  },
  {L"ありいずみ　のぶひこ",TESTMODE_ITEM_SelectFuncAri },
  {L"なぎはし　とおる"    ,TESTMODE_ITEM_SelectFuncNagi },
  {L"おばた　としひろ"    ,TESTMODE_ITEM_SelectFuncObata},
  {L"いわさわ　みゆき"    ,TESTMODE_ITEM_SelectFuncIwasawa},
  {L"さいとう　のぞむ"    ,TESTMODE_ITEM_SelectFuncSaito},
  {L"もり　あきと"        ,TESTMODE_ITEM_SelectFuncMori},
  {L"なかむら　ひろゆき"  ,TESTMODE_ITEM_SelectFuncNakamura},
  {L"ほさか　げんや"      ,TESTMODE_ITEM_SelectFuncHosaka},
};

static TESTMODE_MENU_LIST menuRTCEdit[] =
{
  {L"月　ふやす"      ,TESTMODE_ITEM_ChangeRTC },
  {L"月　へらす"      ,TESTMODE_ITEM_ChangeRTC },
  {L"日　ふやす"      ,TESTMODE_ITEM_ChangeRTC },
  {L"日　へらす"      ,TESTMODE_ITEM_ChangeRTC },
  {L"時　ふやす"      ,TESTMODE_ITEM_ChangeRTC },
  {L"時　へらす"      ,TESTMODE_ITEM_ChangeRTC },
  {L"分　ふやす"      ,TESTMODE_ITEM_ChangeRTC },
  {L"分　へらす"      ,TESTMODE_ITEM_ChangeRTC },
  {L"秒　ふやす"      ,TESTMODE_ITEM_ChangeRTC },
  {L"秒　へらす"      ,TESTMODE_ITEM_ChangeRTC },

  {L"もどる"        ,TESTMODE_ITEM_BackTopMenu },
};

static TESTMODE_MENU_LIST nameListMax[DEBUG_NAME_MAX];

//--------------------------------------------------------------------------
//  描画周り初期化
//--------------------------------------------------------------------------
static void TESTMODE_InitGraphic( TESTMODE_WORK *work )
{
  static const GFL_DISP_VRAM vramBank = {
    GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_128_B,        // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_128_D,      // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,       // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,     // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_32K,   // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
  };

  static const GFL_BG_SYS_HEADER sysHeader = {
    GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
  };

  static const GFL_BG_BGCNT_HEADER bgCont_Menu = {
  0, 0, 0x800, 0,
  GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
  GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };

  GX_SetMasterBrightness(0);
  GXS_SetMasterBrightness(-16);
  GFL_DISP_GX_SetVisibleControlDirect(0);   //全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);

  GFL_DISP_SetBank( &vramBank );
  GFL_BG_Init( work->heapId_ );
  GFL_BG_SetBGMode( &sysHeader );

  TESTMODE_InitBgFunc( &bgCont_Menu , BG_PLANE_MENU );

  GFL_BMPWIN_Init( work->heapId_ );


}

//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void TESTMODE_InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, GFL_BG_MODE_TEXT );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}


//--------------------------------------------------------------------------
//  メニュー作成
//--------------------------------------------------------------------------
static void TESTMODE_CreateMenu( TESTMODE_WORK *work , TESTMODE_MENU_LIST *menuList  , const u16 itemNum )
{
  { //menu create
    u32 i;
    BMPMENULIST_HEADER head;
    STRCODE workStr[128]; //仮文字列

    work->menuList_ = BmpMenuWork_ListCreate( itemNum , work->heapId_ );
    //リスト作成
    for( i=0;i<itemNum;i++ )
    {
      STRBUF *strbuf;
      const u16 strLen = wcslen(menuList[i].str_);
      //終端コードを追加してからSTRBUFに変換
      GFL_STD_MemCopy( menuList[i].str_ , workStr , strLen*2 );
      workStr[strLen] = GFL_STR_GetEOMCode();

      strbuf = GFL_STR_CreateBuffer( strLen+1 , work->heapId_ );
      GFL_STR_SetStringCode( strbuf , workStr );

      BmpMenuWork_ListAddString( work->menuList_ , strbuf , NULL , work->heapId_ );

      GFL_STR_DeleteBuffer( strbuf );
    }
    {
      BMPMENULIST_HEADER menuHead =
      {
        NULL,     //表示文字データポインタ
        NULL,     /* カーソル移動ごとのコールバック関数 */
        NULL,     /* 一列表示ごとのコールバック関数 */
        NULL,     /* BMPウィンドウデータ */
        0,        /* リスト項目数 */
        LIST_DISP_MAX,  /* 表示最大項目数 */
        0,        /* ラベル表示Ｘ座標 */
        16,       /* 項目表示Ｘ座標 */
        0,        /* カーソル表示Ｘ座標 */
        2,        /* 表示Ｙ座標 */
        1,        /*文字色 */
        15,       /*背景色 */
        2,        /*文字影色 */
        0,        /* 文字間隔Ｘ */
        0,        /* 文字間隔Ｙ */
        BMPMENULIST_LRKEY_SKIP,   /*ページスキップタイプ */
        0,        /* 文字指定(本来は u8 だけど、そんなに作らないと思うので) */
        0,        /* ＢＧカーソル(allow)表示フラグ(0:ON,1:OFF) */
        NULL,     //ワーク
        16,       //文字サイズX
        16,       //文字サイズY
        NULL,     //メッセージバッファ
        NULL,     //プリントユーティリティ
        NULL,     //プリントキュー
        NULL,     //フォントハンドル
      };
      menuHead.list = work->menuList_;
      menuHead.count = itemNum;
      menuHead.win = work->bmpWin_;
      menuHead.print_que = work->printQue_;
      menuHead.print_util = &work->printUtil_;
      menuHead.font_handle = work->fontHandle_;
      work->menuWork_ = BmpMenuList_Set( &menuHead, 0, 0, work->heapId_ );
      BmpMenuList_SetCursorBmp( work->menuWork_ , work->heapId_ );
    }
  }
}

//--------------------------------------------------------------------------
//  メニュー削除
//--------------------------------------------------------------------------
static void TESTMODE_DeleteMenu( TESTMODE_WORK *work )
{
  BmpMenuList_Exit( work->menuWork_ , NULL , NULL );
  BmpMenuWork_ListDelete( work->menuList_ );
}

//--------------------------------------------------------------------------
//  メニュー更新
//--------------------------------------------------------------------------
static BOOL TESTMODE_UpdateMenu( TESTMODE_WORK *work )
{
  u32 ret;

  ret = BmpMenuList_Main(work->menuWork_);
  switch(ret)
  {
  case BMPMENULIST_NULL:  /* 何も選択されていない */
    #ifdef QuickSelectFunc
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
    {
      return QuickSelectFunc( work, 0 );
    }
    #endif
    break;
  case BMPMENULIST_CANCEL:  /* キャンセルされた */
    if( work->currentMenu_ != topMenu && work->parentWork_ == NULL )
    {
      //Proc直予備でTopMenuではないときはTopMenuに戻る
      TESTMODE_ITEM_BackTopMenu(work,0);
    }
    else
    {
      work->nextAction_ = TMN_CANCEL_MENU;
    }
    return TRUE;
    break;
  default:    /* 何かが決定された */
    {
      u16 topPos,curPos;
      BmpMenuList_PosGet( work->menuWork_ , &topPos , &curPos );
      return work->currentMenu_[topPos+curPos].selectFunc_( work , topPos+curPos );
    }
    break;
  }
  return FALSE;
}

//--------------------------------------------------------------------------
//  下のウィンドウの時間更新
//--------------------------------------------------------------------------
static const char BuildDate[]=__DATE__;
static const char BuildTime[]=__TIME__;
static void TESTMODE_RefreshSubWindow( TESTMODE_WORK *work )
{
  //文字列取得用
  const u16 strLen = 64;
  STRBUF *strbuf;
  u16   workStr[strLen];

  GFL_RTC_GetDateTime( &work->rtcDate_ , &work->rtcTime_ );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->bmpWinSub_), 15 );

  strbuf = GFL_STR_CreateBuffer( strLen , work->heapId_ );

//  swprintf( workStr , "%2dがつ%2dにち %2じ%2ふん",
  swprintf( workStr ,strLen, L"ROM作成　　　%s　　　%s",BuildDate,BuildTime );
  workStr[wcslen(workStr)] = GFL_STR_GetEOMCode();
  GFL_STR_SetStringCode( strbuf , workStr );

  PRINT_UTIL_Print( &work->printUtilSub_ , work->printQueSub_ ,
        2 , 2 , strbuf , work->fontHandle_ );
  GFL_STR_ClearBuffer( strbuf );

  swprintf( workStr ,strLen, L"RTC時刻　　　%4d/%02d/%02d　　　%2d:%2d:%2d"
    ,work->rtcDate_.year+2000 , work->rtcDate_.month , work->rtcDate_.day
    ,work->rtcTime_.hour , work->rtcTime_.minute , work->rtcTime_.second );

  workStr[wcslen(workStr)] = GFL_STR_GetEOMCode();
  GFL_STR_SetStringCode( strbuf , workStr );

  PRINT_UTIL_Print( &work->printUtilSub_ , work->printQueSub_ ,
        2 , 18 , strbuf , work->fontHandle_ );

  GFL_STR_DeleteBuffer( strbuf );

  GFL_BG_LoadScreenReq( BG_PLANE_MENU );
  work->refreshCount_ = 0;
}


//--------------------------------------------------------------------------
//  Procの切り替え予約
//  @value  最初にTESTMODE_WORKがある以外はGFL_PROC_SysSetNextProc と同じです
//--------------------------------------------------------------------------
static void TESTMODE_COMMAND_ChangeProc( TESTMODE_WORK *work ,FSOverlayID ov_id, const GFL_PROC_DATA *procdata, void *pwork )
{
  work->overlayId_= ov_id;
  work->procData_ = procdata;
  work->procWork_ = pwork;

  work->nextAction_ = TMN_CHANGE_PROC;
}

//--------------------------------------------------------------------------
//  違うメニューへ移動
//  @value  移動先のメニューリストとリストの個数
//--------------------------------------------------------------------------
static void TESTMODE_COMMAND_ChangeMenu( TESTMODE_WORK *work , TESTMODE_MENU_LIST *menuList , const u16 itemNum )
{
  work->nextMenu_ = menuList;
  work->nextItemNum_ = itemNum;

  work->nextAction_ = TMN_CHANGE_MENU;
}

//--------------------------------------------------------------------------
//  ゲームを開始する
//  @value  移動先のメニューリストとリストの個数
//--------------------------------------------------------------------------
static void TESTMODE_COMMAND_StartGame( TESTMODE_WORK *work , TESTMODE_INITGAME_TYPE type )
{
  work->gameType_ = type;

  work->nextAction_ = TMN_GAME_START;
}




//============================================================================================
//
//
//  プロセスコントロール
//
//
//============================================================================================
const GFL_PROC_DATA TestMainProcData;
FS_EXTERN_OVERLAY(testmode);

//------------------------------------------------------------------
/**
 * @brief   初期化
 *
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT TestModeProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  TESTMODE_WORK *work;
  HEAPID  heapID = HEAPID_TITLE;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, heapID, 0x30000 );

  work = GFL_PROC_AllocWork( proc, sizeof(TESTMODE_WORK), heapID );
  GFL_STD_MemClear(work, sizeof(TESTMODE_WORK));
  work->heapId_ = heapID;
  work->parentWork_ = pwk;
  work->saveControl_ = SaveControl_GetPointer();  //グローバル参照。あまりよくない
  work->nextMenu_ = NULL;
  work->state_  = TMS_INIT_MENU;
  work->nextAction_ = TMN_MAX;
  work->refreshCount_ = 15;

  TESTMODE_InitGraphic( work );

  //フォント用パレット
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , TESTMODE_PLT_FONT * 32, 16*2, work->heapId_ );
  work->fontHandle_ = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId_ );

  work->bmpWin_ = GFL_BMPWIN_Create( BG_PLANE_MENU , 1,1,30,LIST_DISP_MAX*2,TESTMODE_PLT_FONT,GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_MakeScreen( work->bmpWin_ );
  GFL_BMPWIN_TransVramCharacter( work->bmpWin_ );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->bmpWin_), 15 );
  GFL_BG_LoadScreenReq( BG_PLANE_MENU );
  work->printQue_ = PRINTSYS_QUE_Create( work->heapId_ );
  PRINT_UTIL_Setup( &work->printUtil_ , work->bmpWin_ );

  work->bmpWinSub_ = GFL_BMPWIN_Create( BG_PLANE_MENU , 1,18,30,4,TESTMODE_PLT_FONT,GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_MakeScreen( work->bmpWinSub_ );
  GFL_BMPWIN_TransVramCharacter( work->bmpWinSub_ );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->bmpWinSub_), 15 );
  GFL_BG_LoadScreenReq( BG_PLANE_MENU );
  work->printQueSub_ = PRINTSYS_QUE_Create( work->heapId_ );
  PRINT_UTIL_Setup( &work->printUtilSub_ , work->bmpWinSub_ );

  //背景色
  *((u16 *)HW_BG_PLTT) = 0x7d8c;//RGB(12, 12, 31);

  //ワークがNULL→タイトルからProcSetで呼ばれた
  if( pwk == NULL )
  {
    work->currentMenu_ = topMenu;
    work->currentItemNum_ = NELEMS(topMenu);
  }
  else
  {
    TESTMODE_PROC_WORK *parentWork = pwk;

    switch( parentWork->startMode_ )
    {
    case TESTMODE_NAMESELECT: //人名選択
      {
        u16 i;
        GFL_MSGDATA *msgMng = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_debugname_dat, work->heapId_);

        for( i=0;i<DEBUG_NAME_MAX;i++ )
        {
          STRBUF  *str = GFL_STR_CreateBuffer( ITEM_NAME_MAX , work->heapId_ );
          GFL_MSG_GetString( msgMng , i , str );
          GFL_STR_GetStringCode( str , nameListMax[i].str_ , ITEM_NAME_MAX );
          GFL_STR_DeleteBuffer( str );

          nameListMax[i].selectFunc_ = TESTMODE_ITEM_SelectFuncSelectName;
        }
        TESTMODE_COMMAND_ChangeMenu( work , nameListMax , DEBUG_NAME_MAX );

        GFL_MSG_Delete( msgMng );

        work->currentMenu_ = nameListMax;
        work->currentItemNum_ = DEBUG_NAME_MAX;
      }
      break;
    }
  }
  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief   メイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT TestModeProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  TESTMODE_WORK *work = mywk;

  work->refreshCount_++;
  if( work->refreshCount_ > 15 )TESTMODE_RefreshSubWindow(work);

  switch( work->state_ )
  {
  case TMS_INIT_MENU:
    TESTMODE_CreateMenu( work , work->currentMenu_ ,work->currentItemNum_);
    work->state_ = TMS_SELECT_ITEM;
    break;

  case TMS_SELECT_ITEM:
    if( TESTMODE_UpdateMenu( work ) == TRUE )
    {
      work->state_ = TMS_TERM_MENU;
    }
    break;

  case TMS_TERM_MENU:
    TESTMODE_DeleteMenu( work );
    work->currentMenu_ = NULL;
    work->state_ = TMS_CHECK_NEXT;
    break;

  case TMS_CHECK_NEXT:
    //次の処理への分岐
    switch( work->nextAction_ )
    {
    case TMN_CHANGE_PROC:
      return GFL_PROC_RES_FINISH;
      break;

    case TMN_CHANGE_MENU:
      work->currentMenu_ = work->nextMenu_;
      work->currentItemNum_ = work->nextItemNum_;

      work->nextMenu_ = NULL;
      work->state_ = TMS_INIT_MENU;
      break;

    case TMN_CANCEL_MENU:
    case TMN_EXIT_PROC:
    default:
      return GFL_PROC_RES_FINISH;
      break;
    }
    break;
  }

  PRINTSYS_QUE_Main( work->printQue_ );
  PRINT_UTIL_Trans( &work->printUtil_ ,work->printQue_ );
  PRINTSYS_QUE_Main( work->printQueSub_ );
  PRINT_UTIL_Trans( &work->printUtilSub_ ,work->printQueSub_ );

  return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief   終了
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT TestModeProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  TESTMODE_WORK *work = mywk;

  //処理分岐
  switch( work->nextAction_ )
  {
  case TMN_CHANGE_PROC:
    GFL_PROC_SysSetNextProc(work->overlayId_, work->procData_, work->procWork_);
    break;

  case TMN_GAME_START:
    {
      FS_EXTERN_OVERLAY(title);
      GFL_OVERLAY_Load(FS_OVERLAY_ID(title));

      // 輝度フェードアウト
      GFL_FADE_SetMasterBrightReq
        (GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 16, 0);

      switch( work->gameType_ )
      {
      case TMI_NEWGAME:
        GameStart_Beginning();
        break;
      case TMI_CONTINUE:
        GameStart_Continue();
        break;
      case TMI_CONTINUE_NET_OFF:
        GameStart_ContinueNetOff();
        break;
      case TMI_CONTINUE_NET:
        GameStart_ContinueNet();
        break;
      case TMI_DEBUG_START:
        GameStart_Debug();
        break;
      case TMI_DEBUG_START_NET:
        GameStart_DebugNet();
        break;
      case TMI_DEBUG_START_NET_OFF:
        GameStart_DebugNetOff();
        break;
      case TMI_DEBUG_SELECT_NAME:
        GameStart_Debug_SelectName();
        break;
      }
      GFL_OVERLAY_Unload(FS_OVERLAY_ID(title));
    }
    break;

  case TMN_EXIT_PROC:
    //NoAction
    break;

  case TMN_CANCEL_MENU:
  default:
    //ワークがNULL→タイトルからProcSetで呼ばれた
    if( pwk == NULL )
    {
      GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);
    }
    else
    {
      TESTMODE_PROC_WORK *parentWork = pwk;
      switch( parentWork->startMode_ )
      {
      case TESTMODE_NAMESELECT: //人名選択
        parentWork->work_ = (void*)1;
        break;
      }
    }
    break;
  }


  //開放処理
  PRINTSYS_QUE_Clear( work->printQue_ );
  PRINTSYS_QUE_Delete( work->printQue_ );
  PRINTSYS_QUE_Clear( work->printQueSub_ );
  PRINTSYS_QUE_Delete( work->printQueSub_ );

  GFL_BMPWIN_Delete( work->bmpWin_ );
  GFL_BMPWIN_Delete( work->bmpWinSub_ );

  GFL_FONT_Delete( work->fontHandle_ );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();

  GFL_PROC_FreeWork(proc);
  GFL_HEAP_DeleteHeap( HEAPID_TITLE );

  GX_SetMasterBrightness(0);
  GXS_SetMasterBrightness(0);

  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief   プロセス関数テーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA TestMainProcData = {
  TestModeProcInit,
  TestModeProcMain,
  TestModeProcEnd,
};

//--------------------------------------------------------------------------
//  項目決定時コールバック
//  @value  ワーク
//      選択されたIndex
//  @return メニューを抜けるか？(メニュー切り替えも含む
//--------------------------------------------------------------------------
static BOOL TESTMODE_ITEM_SelectFuncDummy( TESTMODE_WORK *work , const int idx )
{
  return FALSE;
}

//------------------------------------------------------------------
//  トップメニュー用
//------------------------------------------------------------------

//デバッグスタート
static BOOL TESTMODE_ITEM_SelectFuncDebugStart( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_StartGame( work , TMI_DEBUG_START );
  return TRUE;
}
//デバッグスタート
static BOOL TESTMODE_ITEM_SelectFuncDebugStartComm( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_StartGame( work , TMI_DEBUG_START_NET );
  return TRUE;
}
//デバッグスタート
static BOOL TESTMODE_ITEM_SelectFuncDebugStartCommOff( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_StartGame( work , TMI_DEBUG_START_NET_OFF );
  return TRUE;
}

//続きから
static BOOL TESTMODE_ITEM_SelectFuncContinue( TESTMODE_WORK *work , const int idx )
{
  if ( SaveData_GetExistFlag( work->saveControl_ ) == TRUE )
  {
    TESTMODE_COMMAND_StartGame( work , TMI_CONTINUE_NET_OFF );
    return TRUE;
  } else {
    PMSND_PlaySystemSE( SEQ_SE_BEEP );
    return FALSE;
  }
}
//続きから
static BOOL TESTMODE_ITEM_SelectFuncContinueComm( TESTMODE_WORK *work , const int idx )
{
  if ( SaveData_GetExistFlag( work->saveControl_ ) == TRUE )
  {
    TESTMODE_COMMAND_StartGame( work , TMI_CONTINUE_NET );
    return TRUE;
  } else {
    PMSND_PlaySystemSE( SEQ_SE_BEEP );
    return FALSE;
  }
}

//人名選択
static BOOL TESTMODE_ITEM_SelectFuncChangeSelectName( TESTMODE_WORK *work , const int idx )
{
  /*
  u16 i;
  GFL_MSGDATA *msgMng = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_debugname_dat, work->heapId_);

  for( i=0;i<DEBUG_NAME_MAX;i++ )
  {
    STRBUF  *str = GFL_STR_CreateBuffer( ITEM_NAME_MAX , work->heapId_ );
    GFL_MSG_GetString( msgMng , i , str );
    GFL_STR_GetStringCode( str , nameListMax[i].str_ , ITEM_NAME_MAX );
    GFL_STR_DeleteBuffer( str );
  }
  TESTMODE_COMMAND_ChangeMenu( work , nameListMax , DEBUG_NAME_MAX );

  GFL_MSG_Delete( msgMng );
  */
  TESTMODE_COMMAND_StartGame( work , TMI_DEBUG_SELECT_NAME );
  return TRUE;
}

FS_EXTERN_OVERLAY(watanabe_sample);
extern const GFL_PROC_DATA DebugWatanabeMainProcData;
static BOOL TESTMODE_ITEM_SelectFuncWatanabe( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(watanabe_sample), &DebugWatanabeMainProcData, NULL);
  return TRUE;
}

static BOOL TESTMODE_ITEM_SelectFuncTamada( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(config_panel), &ConfigPanelProcData, NULL);
  //GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(debug_tamada), &DebugTamadaMainProcData, NULL);
#if 0
  {
    GAME_INIT_WORK * init_param = DEBUG_GetGameInitWork(GAMEINIT_MODE_DEBUG, 1);
    TESTMODE_COMMAND_ChangeProc(work,
      NO_OVERLAY_ID, &GameMainProcData, init_param);
  }
#endif
  return TRUE;
}

extern const GFL_PROC_DATA DebugSogabeMainProcData;
FS_EXTERN_OVERLAY(sogabe_debug);
static BOOL TESTMODE_ITEM_SelectFuncSogabe( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(sogabe_debug), &DebugSogabeMainProcData, NULL);
  return TRUE;
}

FS_EXTERN_OVERLAY(ohno_debug);
extern const GFL_PROC_DATA DebugOhnoListProcData;
static BOOL TESTMODE_ITEM_SelectFuncOhno( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(ohno_debug) , &DebugOhnoListProcData, NULL);
  return TRUE;
}

FS_EXTERN_OVERLAY(taya_debug);
extern const GFL_PROC_DATA DebugTayaMainProcData;
static BOOL TESTMODE_ITEM_SelectFuncTaya( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(taya_debug), &DebugTayaMainProcData, NULL);
  return TRUE;
}

FS_EXTERN_OVERLAY(debug_fight);
extern const GFL_PROC_DATA  DebugFightProcData;
static BOOL TESTMODE_ITEM_DebugFight( TESTMODE_WORK *work , const int idx )
{
  GAMEDATA* gameData = GAMEDATA_Create( GFL_HEAP_LOWID(GFL_HEAPID_APP) );
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(debug_fight), &DebugFightProcData, gameData );
  return TRUE;
}



#if 0
extern const GFL_PROC_DATA TestProg1MainProcData;
static BOOL TESTMODE_ITEM_SelectFuncSample1( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,NO_OVERLAY_ID, &TestProg1MainProcData, NULL);
  return TRUE;
}
#endif

FS_EXTERN_OVERLAY(matsuda_debug);
//extern const GFL_PROC_DATA DebugMatsudaListProcData;
extern const GFL_PROC_DATA DebugSaveProcData;
static BOOL TESTMODE_ITEM_SelectFuncMatsuda( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(matsuda_debug), &DebugSaveProcData, NULL);
  return TRUE;
}

//セーブ破壊
extern const GFL_PROC_DATA DebugSaveProcData;
static BOOL TESTMODE_ITEM_SelectFuncSave( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(matsuda_debug), &DebugSaveProcData, NULL);
  return TRUE;
}

//SOUND
FS_EXTERN_OVERLAY(sound_debug);
extern const GFL_PROC_DATA SoundTest2ProcData;
static BOOL TESTMODE_ITEM_SelectFuncSound( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(sound_debug), &SoundTest2ProcData, NULL);
  return TRUE;
}

//MCS接続
extern void PM_MCS_EnableMcsRecv( void );
static BOOL TESTMODE_ITEM_ConnectMCS( TESTMODE_WORK *work , const int idx )
{
  GFL_MCS_Open();
  TESTMODE_COMMAND_ChangeMenu( work , topMenu , NELEMS(topMenu) );
  return TRUE;
}


static BOOL TESTMODE_ITEM_SelectFuncKagaya( TESTMODE_WORK *work , const int idx )
{
  //かがや
  {
    TESTMODE_COMMAND_StartGame( work , TMI_DEBUG_START );
    /*
    FS_EXTERN_OVERLAY(title);
    GFL_OVERLAY_Load(FS_OVERLAY_ID(title));
    GameStart_Debug();
    GFL_OVERLAY_Unload(FS_OVERLAY_ID(title));
    */
  }
  return TRUE;
}

#include "ariizumi/ari_testmode.cdat"
static BOOL TESTMODE_ITEM_SelectFuncAri( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeMenu( work , menuAriizumi , NELEMS(menuAriizumi) );
  return TRUE;
}

FS_EXTERN_OVERLAY(nagihashi_debug);
extern const GFL_PROC_DATA DebugNagiMainProcData;
static BOOL TESTMODE_ITEM_SelectFuncNagi( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(nagihashi_debug), &DebugNagiMainProcData, NULL);
  return TRUE;
}

FS_EXTERN_OVERLAY(obata_debug);
extern const GFL_PROC_DATA DebugObataMainProcData;
extern const GFL_PROC_DATA DebugObataMenuMainProcData;
static BOOL TESTMODE_ITEM_SelectFuncObata( TESTMODE_WORK *work , const int idx )
{
  //TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(obata_debug), &DebugObataMainProcData, NULL);
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(obata_debug), &DebugObataMenuMainProcData, NULL);
  return TRUE;
}

static BOOL TESTMODE_ITEM_SelectFuncIwasawa( TESTMODE_WORK *work , const int idx )
{
  return TRUE;
}
static BOOL TESTMODE_ITEM_SelectFuncSaito( TESTMODE_WORK *work , const int idx )
{
  return TRUE;
}

#include "mori/d_mori.c"
static BOOL TESTMODE_ITEM_SelectFuncMori( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeMenu( work , menuMori , NELEMS(menuMori)  );
  return TRUE;
}

FS_EXTERN_OVERLAY(nakahiro_debug);
extern const GFL_PROC_DATA DebugNakahiroMainProcData;
static BOOL TESTMODE_ITEM_SelectFuncNakamura( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(nakahiro_debug), &DebugNakahiroMainProcData, NULL);
  return TRUE;
}

#include "hosaka/debug_hosaka_menu.c"
static BOOL TESTMODE_ITEM_SelectFuncHosaka( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeMenu( work , menuHosaka , NELEMS(menuHosaka) );
  return TRUE;
}
static BOOL TESTMODE_ITEM_BackTopMenu( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeMenu( work , topMenu , NELEMS(topMenu) );
  return TRUE;
}

//RTC調整
static BOOL TESTMODE_ITEM_SelectFuncRTCEdit( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeMenu( work , menuRTCEdit , NELEMS(menuRTCEdit) );
  return TRUE;
}


//-----------------------------------------------------------------------

//ミュージカルメニュー
//#include "ariizumi/ari_testmode.cdat" ←の中にある
static BOOL TESTMODE_ITEM_ChangeMusicalMenu( TESTMODE_WORK *work , const int idx )
{
  TESTMODE_COMMAND_ChangeMenu( work , menuMusical , NELEMS(menuMusical) );
  return TRUE;
}

//
static BOOL TESTMODE_ITEM_ChangeRTC( TESTMODE_WORK *work , const int idx )
{
  const int addValue = (idx%2==0 ? 1 : -1 );

  switch( idx )
  {
  case 0: //月
  case 1:
    work->rtcDate_.month += addValue;
    RTC_SetDate( &work->rtcDate_ );
    break;
  case 2: //日
  case 3:
    work->rtcDate_.day += addValue;
    RTC_SetDate( &work->rtcDate_ );
    break;
  case 4: //時
  case 5:
    work->rtcTime_.hour += addValue;
    RTC_SetTime( &work->rtcTime_ );
    break;
  case 6: //分
  case 7:
    work->rtcTime_.minute += addValue;
    RTC_SetTime( &work->rtcTime_ );
    break;
  case 8: //秒
  case 9:
    work->rtcTime_.second += addValue;
    RTC_SetTime( &work->rtcTime_ );
    break;
  }
  TESTMODE_RefreshSubWindow(work);
  return FALSE;
}

//人名選択決定時
static BOOL TESTMODE_ITEM_SelectFuncSelectName( TESTMODE_WORK *work , const int idx )
{
  GFL_MSGDATA *msgMng = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_debugname_dat, work->heapId_);

  STRBUF  *str = GFL_STR_CreateBuffer( ITEM_NAME_MAX , work->heapId_ );
  MYSTATUS    *myStatus;


  GFL_MSG_GetString( msgMng , idx , str );
  //名前のセット
  myStatus = SaveData_GetMyStatus( work->saveControl_ );
  MyStatus_SetMyNameFromString( myStatus , str );
  MyStatus_SetID(myStatus, GFL_STD_MtRand(GFL_STD_RAND_MAX));
  if(MyStatus_GetMySex(myStatus) == PM_MALE){
    MyStatus_SetTrainerView(myStatus, UNION_VIEW_INDEX_MAN_START);
  }
  else{
    MyStatus_SetTrainerView(myStatus, UNION_VIEW_INDEX_WOMAN_START);
  }

  GFL_STR_DeleteBuffer( str );

  GFL_MSG_Delete( msgMng );

  work->nextAction_ = TMN_EXIT_PROC;
  return TRUE;
}





