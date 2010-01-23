//======================================================================
/**
 * @file	battle_championship.c
 * @brief	大会メニュー
 * @author	ariizumi
 * @author  Toru=Nagihashi 引継
 * @data	09/10/08 -> 100112
 *
 * モジュール名：BATTLE_CHAMPINONSHIP
 */
//======================================================================
#include <gflib.h>
#include "buflen.h"

#include "app/app_taskmenu.h"
#include "gamesystem/msgspeed.h"
#include "print/printsys.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_regulation.h"
#include "system/bmp_winframe.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/bmp_menulist.h"
#include "app/app_keycursor.h"

#include "arc_def.h"
#include "battle_championship.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_battle_championship.h"

#include "title/title.h"
#include "net_app/irc_battle.h"
#include "net_app/wifibattlematch.h"

#include "battle_championship/battle_championship.h"
#include "battle_championship/battle_championship_def.h"
#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define BATTLE_CHAMPIONSHIP_FRAME_MENU ( GFL_BG_FRAME0_M )
#define BATTLE_CHAMPIONSHIP_FRAME_STR ( GFL_BG_FRAME1_M )
#define BATTLE_CHAMPIONSHIP_FRAME_BG  ( GFL_BG_FRAME3_M )

#define BATTLE_CHAMPIONSHIP_FRAME_SUB_BG  ( GFL_BG_FRAME3_S )

#define BATTLE_CHAMPIONSHIP_BG_PAL_WINFRAME (0xD)
#define BATTLE_CHAMPIONSHIP_BG_PAL_FONT (0xE)

#define BATTLE_CHAMPIONSHIP_FRAME_MENU_CGX (1)
#define BATTLE_CHAMPIONSHIP_FRAME_STR_CGX (1)

#define BATTLE_CHAMPIONSHIP_MENU_ITEM_NUM (4)

#define BATTLE_CHAMPIONSHIP_MENU_WIDTH (24)
#define BATTLE_CHAMPIONSHIP_MENU_TOP  (4)
#define BATTLE_CHAMPIONSHIP_MENU_LEFT (4)

#define BATTLE_CHAMPIONSHIP_MEMBER_NUM (2)
//-------------------------------------
/// LIST
//=====================================
#define BC_LIST_SELECT_NULL  (BMPMENULIST_NULL)
#define BC_LIST_WINDOW_MAX   (5)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//ステート
typedef enum
{
  BCS_FADEIN,
  BCS_FADEIN_WAIT,

  BCS_FIRSTMSG_SHOW,
  BCS_FIRSTMSG_WAIT,

  BCS_FIRSTMENU_OPEN,
  BCS_FIRSTMENU_WAIT,
  
  BCS_INFOMSG_SHOW,
  BCS_INFOMSG_WAIT,

  BCS_FADEOUT,
  BCS_FADEOUT_WAIT,
}BATTLE_CHAMPIONSHIP_STATE;

//切り替え先Proc種類
typedef enum
{
  BCNP_TITLE,
  BCNP_WIFI_BATTLE,
  BCNP_EVENT_BATTLE,
  BCNP_DIGITAL_MEMBERSHIP,
  
}BATTLE_CHAMPIONSHIP_NEXT_PROC;

//-------------------------------------
///	描画方式
//=====================================
typedef enum
{
  BC_TEXT_TYPE_QUE,     //プリントキューを使う
  BC_TEXT_TYPE_STREAM,  //ストリームを使う

  BC_TEXT_TYPE_MAX,    //c内部にて使用
} BC_TEXT_TYPE;



//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//-------------------------------------
///	メッセージウィンドウ
//=====================================
typedef struct _BC_TEXT_WORK BC_TEXT_WORK;

//-------------------------------------
///	選択リスト
//=====================================
typedef struct 
{
  GFL_MSGDATA *p_msg;
  GFL_FONT    *p_font;
  PRINT_QUE   *p_que;
  u32 strID[BC_LIST_WINDOW_MAX];
  u32 list_max;

  u16 frm;
  u16 font_plt;
  u16 frm_plt;
  u16 frm_chr;
} BC_LIST_SETUP;
typedef struct _BC_LIST_WORK BC_LIST_WORK;

//メインワーク
typedef struct
{
  HEAPID heapId;
  BATTLE_CHAMPIONSHIP_STATE state;
  BATTLE_CHAMPIONSHIP_NEXT_PROC nextProcType;
  
  //大会情報
  BATTLE_CHAMPIONSHIP_DATA csData;
  
  //メッセージ用
  BC_TEXT_WORK    *text;
  GFL_FONT        *fontHandle;
  GFL_MSGDATA     *msgHandle;
  PRINT_QUE       *taskMenuQue;
  
  //taskmenu
  BC_LIST_WORK    *list;

}BATTLE_CHAMPIONSHIP_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_128_B,       // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_128_D,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_NONE,        // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,     // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_128K
};

static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

static void BATTLE_CHAMPIONSHIP_ChangeProcInit( BATTLE_CHAMPIONSHIP_WORK *work );
static void BATTLE_CHAMPIONSHIP_ChangeProcTerm( BATTLE_CHAMPIONSHIP_WORK *work );

static void BATTLE_CHAMPIONSHIP_InitGraphic( BATTLE_CHAMPIONSHIP_WORK *work );
static void BATTLE_CHAMPIONSHIP_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void BATTLE_CHAMPIONSHIP_TermGraphic( BATTLE_CHAMPIONSHIP_WORK *work );
static void BATTLE_CHAMPIONSHIP_LoadResource( BATTLE_CHAMPIONSHIP_WORK *work );
static void BATTLE_CHAMPIONSHIP_ReleaseResource( BATTLE_CHAMPIONSHIP_WORK *work );

static void BATTLE_CHAMPIONSHIP_InitMessage( BATTLE_CHAMPIONSHIP_WORK *work );
static void BATTLE_CHAMPIONSHIP_TermMessage( BATTLE_CHAMPIONSHIP_WORK *work );
static void BATTLE_CHAMPIONSHIP_UpdateMessage( BATTLE_CHAMPIONSHIP_WORK *work );
static void BATTLE_CHAMPIONSHIP_ShowMessage( BATTLE_CHAMPIONSHIP_WORK *work  , const u16 msgNo );
static const BOOL BATTLE_CHAMPIONSHIP_IsFinishMessage( BATTLE_CHAMPIONSHIP_WORK *work );

static void BATTLE_CHAMPIONSHIP_OpenFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work );
static void BATTLE_CHAMPIONSHIP_CloseFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work );
static const u32 BATTLE_CHAMPIONSHIP_UpdateFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work );

//-------------------------------------
///	テキスト
//=====================================
extern BC_TEXT_WORK * BC_TEXT_Init( u16 frm, u8 font_plt, PRINT_QUE *p_que, GFL_FONT *p_font, HEAPID heapID );
extern void BC_TEXT_Exit( BC_TEXT_WORK* p_wk );
extern void BC_TEXT_Main( BC_TEXT_WORK* p_wk );
extern void BC_TEXT_Print( BC_TEXT_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, BC_TEXT_TYPE type );
extern void BC_TEXT_PrintBuf( BC_TEXT_WORK* p_wk, const STRBUF *cp_strbuf, BC_TEXT_TYPE type );
extern BOOL BC_TEXT_IsEndPrint( const BC_TEXT_WORK *cp_wk );
extern void BC_TEXT_WriteWindowFrame( BC_TEXT_WORK *p_wk, u16 frm_chr, u8 frm_plt );

//-------------------------------------
///	LIST
//=====================================
extern BC_LIST_WORK * BC_LIST_Init( const BC_LIST_SETUP *cp_setup, HEAPID heapID );
extern void BC_LIST_Exit( BC_LIST_WORK *p_wk );
extern u32 BC_LIST_Main( BC_LIST_WORK *p_wk );


GFL_PROC_DATA BATTLE_CHAMPIONSHIP_ProcData =
{
  BATTLE_CHAMPIONSHIP_ProcInit,
  BATTLE_CHAMPIONSHIP_ProcMain,
  BATTLE_CHAMPIONSHIP_ProcTerm
};

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
#pragma mark [>proc 
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  BATTLE_CHAMPIONSHIP_WORK *work;
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BATTLE_CHAMPIONSHIP, 0x18000 );

  work = GFL_PROC_AllocWork( proc, sizeof(BATTLE_CHAMPIONSHIP_WORK), HEAPID_BATTLE_CHAMPIONSHIP );
  GFL_STD_MemClear( work, sizeof(BATTLE_CHAMPIONSHIP_WORK) );
  work->nextProcType  = BCNP_TITLE;
  work->heapId = HEAPID_BATTLE_CHAMPIONSHIP;

  BATTLE_CHAMPIONSHIP_InitGraphic( work );
  BATTLE_CHAMPIONSHIP_LoadResource( work );
  BATTLE_CHAMPIONSHIP_InitMessage( work );
  
  work->state = BCS_FADEIN;

  //仮データ作成
  {
    BATTLE_CHAMPIONSHIP_SetDebugData( &work->csData , work->heapId );
  }
  
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  BATTLE_CHAMPIONSHIP_WORK *work = mywk;

  //仮データ破棄
  BATTLE_CHAMPIONSHIP_TermDebugData( &work->csData );

  BATTLE_CHAMPIONSHIP_TermMessage( work );
  BATTLE_CHAMPIONSHIP_ReleaseResource( work );
  BATTLE_CHAMPIONSHIP_TermGraphic( work );

  switch( work->nextProcType )
  { 
  default:
    GF_ASSERT(0);
  case BCNP_TITLE:
    GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);
    break;
  case BCNP_WIFI_BATTLE:
    { 
      WIFIBATTLEMATCH_PARAM *p_param;
      p_param = GFL_HEAP_AllocMemory( GFL_HEAPID_APP, sizeof(WIFIBATTLEMATCH_PARAM) );
      GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_PARAM) );
      p_param->mode             = WIFIBATTLEMATCH_MODE_WIFI;
      p_param->is_auto_release  = TRUE;
      GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(wifibattlematch_sys), &WifiBattleMaptch_ProcData, p_param );
    }
    break;
  case BCNP_EVENT_BATTLE:
    GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &IRC_BATTLE_ProcData, NULL);
    break;
  case BCNP_DIGITAL_MEMBERSHIP:
    break;
  }

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_BATTLE_CHAMPIONSHIP );

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  BATTLE_CHAMPIONSHIP_WORK *work = mywk;
  
  switch( work->state )
  {
  case BCS_FADEIN:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = BCS_FADEIN_WAIT;
    break;
    
  case BCS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      work->state = BCS_FIRSTMSG_SHOW;
    }
    break;

  case BCS_FIRSTMSG_SHOW:
    BATTLE_CHAMPIONSHIP_ShowMessage( work, BC_STR_01 );
    work->state = BCS_FIRSTMSG_WAIT;
    break;

  case BCS_FIRSTMSG_WAIT:
    BATTLE_CHAMPIONSHIP_UpdateMessage( work );
    if( BATTLE_CHAMPIONSHIP_IsFinishMessage( work ) == TRUE )
    {
      work->state = BCS_FIRSTMENU_OPEN;
    }
    break;

  case BCS_FIRSTMENU_OPEN:
    BATTLE_CHAMPIONSHIP_OpenFirstMenu( work );
    work->state = BCS_FIRSTMENU_WAIT;
    break;

  case BCS_FIRSTMENU_WAIT:
    {
      const u32 ret = BATTLE_CHAMPIONSHIP_UpdateFirstMenu( work );
      if( ret != BC_LIST_SELECT_NULL )
      {
        switch( ret )
        {
        case 0: //WiFi大会
          work->nextProcType = BCNP_WIFI_BATTLE;
          work->state = BCS_FADEOUT;
          break;
        case 1: //イベント大会
          work->nextProcType = BCNP_EVENT_BATTLE;
          work->state = BCS_FADEOUT;
          break;
        case 2: //デジタル選手証
          work->nextProcType = BCNP_DIGITAL_MEMBERSHIP;
          work->state = BCS_FADEOUT;
          break;
        case 3: //説明をきく
          work->state = BCS_INFOMSG_SHOW;
          break;
        case 4: //やめる
          work->state = BCS_FADEOUT;
          break;
        }
        BATTLE_CHAMPIONSHIP_CloseFirstMenu( work );
      }
    }
    break;

  case BCS_INFOMSG_SHOW:
    BATTLE_CHAMPIONSHIP_ShowMessage( work, BC_STR_02 );
    work->state = BCS_INFOMSG_WAIT;
    break;
  
    break;
  case BCS_INFOMSG_WAIT:
    BATTLE_CHAMPIONSHIP_UpdateMessage( work );
    if( BATTLE_CHAMPIONSHIP_IsFinishMessage( work ) == TRUE )
    {
      work->state = BCS_FIRSTMSG_SHOW;
    }
    break;

  case BCS_FADEOUT:
    WIPE_SYS_Start( WIPE_PATTERN_FSAM , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = BCS_FADEOUT_WAIT;
    break;
    
  case BCS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

#pragma mark [>change proc func
static void BATTLE_CHAMPIONSHIP_ChangeProcInit( BATTLE_CHAMPIONSHIP_WORK *work )
{
  
}

static void BATTLE_CHAMPIONSHIP_ChangeProcTerm( BATTLE_CHAMPIONSHIP_WORK *work )
{
  
}

#pragma mark [>graphic func
static void BATTLE_CHAMPIONSHIP_InitGraphic( BATTLE_CHAMPIONSHIP_WORK *work )
{
  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );
  WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
  WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
  WIPE_ResetWndMask(WIPE_DISP_MAIN);
  WIPE_ResetWndMask(WIPE_DISP_SUB);
  
  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
  GFL_DISP_SetBank( &vramBank );

  //BG系の初期化
  GFL_BG_Init( work->heapId );
  GFL_BMPWIN_Init( work->heapId );

  //Vram割り当ての設定
  {
    static const GFL_BG_SYS_HEADER sys_data = {
        GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };

    // BG0 MAIN (メニュー
    static const GFL_BG_BGCNT_HEADER header_main0 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000,0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    // BG1 MAIN (文字
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000,0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
/*
    // BG2 MAIN (
    static const GFL_BG_BGCNT_HEADER header_main2 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x10000,0x8000,
      GX_BG_EXTPLTT_01, 2, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
*/
    // BG3 MAIN (背景
    static const GFL_BG_BGCNT_HEADER header_main3 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000,0x06000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    // BG3 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x04000,0x06000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    GFL_BG_SetBGMode( &sys_data );

    BATTLE_CHAMPIONSHIP_SetupBgFunc( &header_main3 , BATTLE_CHAMPIONSHIP_FRAME_BG , GFL_BG_MODE_TEXT );
    BATTLE_CHAMPIONSHIP_SetupBgFunc( &header_main1 , BATTLE_CHAMPIONSHIP_FRAME_STR , GFL_BG_MODE_TEXT );
    BATTLE_CHAMPIONSHIP_SetupBgFunc( &header_main0 , BATTLE_CHAMPIONSHIP_FRAME_MENU , GFL_BG_MODE_TEXT );

    BATTLE_CHAMPIONSHIP_SetupBgFunc( &header_sub3 , BATTLE_CHAMPIONSHIP_FRAME_SUB_BG , GFL_BG_MODE_TEXT );
  }
}
//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void BATTLE_CHAMPIONSHIP_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

static void BATTLE_CHAMPIONSHIP_TermGraphic( BATTLE_CHAMPIONSHIP_WORK *work )
{
  GFL_BG_FreeBGControl( BATTLE_CHAMPIONSHIP_FRAME_SUB_BG );
  GFL_BG_FreeBGControl( BATTLE_CHAMPIONSHIP_FRAME_MENU );
  GFL_BG_FreeBGControl( BATTLE_CHAMPIONSHIP_FRAME_STR );
  GFL_BG_FreeBGControl( BATTLE_CHAMPIONSHIP_FRAME_BG );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

static void BATTLE_CHAMPIONSHIP_LoadResource( BATTLE_CHAMPIONSHIP_WORK *work )
{
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_BATTLE_CHAMPIONSHIP , work->heapId );

  ////BGリソース
  //上画面背景
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_battle_championship_connect_NCLR , 
                    PALTYPE_MAIN_BG , 0, 0x20 * 10, work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_battle_championship_connect_sub_NCGR ,
                    BATTLE_CHAMPIONSHIP_FRAME_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_battle_championship_connect_sub_NSCR , 
                    BATTLE_CHAMPIONSHIP_FRAME_BG ,  0 , 0, FALSE , work->heapId );
  //下画面背景
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_battle_championship_connect_NCLR , 
                    PALTYPE_SUB_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_battle_championship_connect_NCGR ,
                    BATTLE_CHAMPIONSHIP_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_battle_championship_connect_01_NSCR , 
                    BATTLE_CHAMPIONSHIP_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );

  GFL_ARC_CloseDataHandle( arcHandle );

  //フレーム枠
  GFL_BG_FillCharacter( BATTLE_CHAMPIONSHIP_FRAME_STR, 0, 1, 0 );
  BmpWinFrame_GraphicSet( BATTLE_CHAMPIONSHIP_FRAME_STR, BATTLE_CHAMPIONSHIP_FRAME_MENU_CGX, BATTLE_CHAMPIONSHIP_BG_PAL_WINFRAME, MENU_TYPE_SYSTEM, work->heapId );
  GFL_BG_FillCharacter( BATTLE_CHAMPIONSHIP_FRAME_MENU, 0, 1, 0 );
  BmpWinFrame_GraphicSet( BATTLE_CHAMPIONSHIP_FRAME_MENU, BATTLE_CHAMPIONSHIP_FRAME_STR_CGX, BATTLE_CHAMPIONSHIP_BG_PAL_WINFRAME, MENU_TYPE_SYSTEM, work->heapId );
  

  //フォント
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , BATTLE_CHAMPIONSHIP_BG_PAL_FONT*0x20, 0x20, work->heapId );
  
}

static void BATTLE_CHAMPIONSHIP_ReleaseResource( BATTLE_CHAMPIONSHIP_WORK *work )
{
  GFL_BG_FillCharacterRelease( BATTLE_CHAMPIONSHIP_FRAME_MENU, 1, 0 );
  GFL_BG_FillCharacterRelease( BATTLE_CHAMPIONSHIP_FRAME_STR, 1, 0 );
}

#pragma mark [>message func
static void BATTLE_CHAMPIONSHIP_InitMessage( BATTLE_CHAMPIONSHIP_WORK *work )
{
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_battle_championship_dat , work->heapId );
  
  work->taskMenuQue = PRINTSYS_QUE_Create( work->heapId );
  
  //メッセージ
  work->text    = BC_TEXT_Init( BATTLE_CHAMPIONSHIP_FRAME_STR, BATTLE_CHAMPIONSHIP_BG_PAL_FONT, work->taskMenuQue, work->fontHandle, work->heapId );
  BC_TEXT_WriteWindowFrame( work->text, BATTLE_CHAMPIONSHIP_FRAME_STR_CGX, BATTLE_CHAMPIONSHIP_BG_PAL_WINFRAME );
}

static void BATTLE_CHAMPIONSHIP_TermMessage( BATTLE_CHAMPIONSHIP_WORK *work )
{
  BC_TEXT_Exit( work->text );

  PRINTSYS_QUE_Delete( work->taskMenuQue );
  
  GFL_MSG_Delete( work->msgHandle );
  GFL_FONT_Delete( work->fontHandle );
}

static void BATTLE_CHAMPIONSHIP_UpdateMessage( BATTLE_CHAMPIONSHIP_WORK *work )
{
  BC_TEXT_Main( work->text );
  PRINTSYS_QUE_Main( work->taskMenuQue );
}
static void BATTLE_CHAMPIONSHIP_ShowMessage( BATTLE_CHAMPIONSHIP_WORK *work  , const u16 msgNo )
{
  BC_TEXT_Print( work->text, work->msgHandle, msgNo, BC_TEXT_TYPE_STREAM );
}

static const BOOL BATTLE_CHAMPIONSHIP_IsFinishMessage( BATTLE_CHAMPIONSHIP_WORK *work )
{
  return BC_TEXT_IsEndPrint( work->text );
}

#pragma mark [>menu func
static void BATTLE_CHAMPIONSHIP_OpenFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work )
{
  BC_LIST_SETUP  setup;
  GFL_STD_MemClear( &setup, sizeof(BC_LIST_SETUP) );
  setup.p_msg   = work->msgHandle;
  setup.p_font  = work->fontHandle;
  setup.p_que   = work->taskMenuQue;
  setup.strID[0]= BC_SELECT_01;
  setup.strID[1]= BC_SELECT_02;
  setup.strID[2]= BC_SELECT_03;
  setup.strID[3]= BC_SELECT_04;
  setup.strID[4]= BC_SELECT_05;
  setup.list_max= 5;
  setup.frm     = BATTLE_CHAMPIONSHIP_FRAME_MENU;
  setup.font_plt= BATTLE_CHAMPIONSHIP_BG_PAL_FONT;
  setup.frm_plt = BATTLE_CHAMPIONSHIP_BG_PAL_WINFRAME;
  setup.frm_chr = BATTLE_CHAMPIONSHIP_FRAME_MENU_CGX;
  work->list  = BC_LIST_Init( &setup, work->heapId );
}

static void BATTLE_CHAMPIONSHIP_CloseFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work )
{
  BC_LIST_Exit( work->list );
}

static const u32 BATTLE_CHAMPIONSHIP_UpdateFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work )
{
  return BC_LIST_Main( work->list );;
}

#pragma mark [>debug
//デバッグ用
void BATTLE_CHAMPIONSHIP_SetDebugData( BATTLE_CHAMPIONSHIP_DATA *csData , const HEAPID heapId )
{
  u8 i;
  csData->name[0] = L'で';
  csData->name[1] = L'ば';
  csData->name[2] = L'っ';
  csData->name[3] = L'ぐ';
  csData->name[4] = L'り';
  csData->name[5] = L'ー';
  csData->name[6] = L'ぐ';
  csData->name[7] = 0xFFFF;
  csData->number = 10;
  csData->league = 8;
  csData->regulation = (REGULATION*)PokeRegulation_LoadDataAlloc( REG_DEBUG_BATTLE , heapId );
  
  for( i=0;i<6;i++ )
  {
    csData->ppp[i] = GFL_HEAP_AllocMemory( heapId , POKETOOL_GetPPPWorkSize() );
    PPP_Clear( csData->ppp[i] );
//    if( i<5 )
    {
      PPP_Setup( csData->ppp[i] , i+1 , 10 , PTL_SETUP_ID_AUTO );
      {
        const u16 oyaName[5] = {L'ブ',L'ラ',L'ッ',L'ク',0xFFFF};
        PPP_Put( csData->ppp[i] , ID_PARA_oyaname_raw , (u32)&oyaName[0] );
        PPP_Put( csData->ppp[i] , ID_PARA_oyasex , PTL_SEX_MALE );
      }
    }
  }
}

void BATTLE_CHAMPIONSHIP_TermDebugData( BATTLE_CHAMPIONSHIP_DATA *csData )
{
  u8 i;
  GFL_HEAP_FreeMemory( csData->regulation );
  for( i=0;i<6;i++ )
  {
    GFL_HEAP_FreeMemory( csData->ppp[i] );
  }
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					テキスト描画構造体
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
#define BC_TEXT_TYPE_NONE  (BC_TEXT_TYPE_MAX)

//-------------------------------------
/// メッセージウィンドウ
//=====================================
struct _BC_TEXT_WORK
{
  GFL_MSGDATA       *p_msg;
  GFL_FONT          *p_font;
  PRINT_STREAM      *p_stream;
  GFL_TCBLSYS       *p_tcbl;
  GFL_BMPWIN*       p_bmpwin;
  STRBUF*           p_strbuf;
  u16               clear_chr;
  u16               heapID;
  PRINT_UTIL        util;
  PRINT_QUE         *p_que;
  u32               print_update;
  BOOL              is_end_print;
  APP_KEYCURSOR_WORK* p_keycursor;
} ;

//-------------------------------------
///	プロトタイプ
//=====================================
static void BC_TEXT_PrintInner( BC_TEXT_WORK* p_wk, BC_TEXT_TYPE type );

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  テキスト  初期化
 *
 *	@param	u16 frm       BG面
 *	@param	font_plt      フォントパレット
 *	@param  PRINT_QUE     プリントQ
 *	@param  GFL_FONT      フォント
 *	@param	heapID        ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BC_TEXT_WORK * BC_TEXT_Init( u16 frm, u8 font_plt, PRINT_QUE *p_que, GFL_FONT *p_font, HEAPID heapID )
{ 
  BC_TEXT_WORK *p_wk;

  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BC_TEXT_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(BC_TEXT_WORK) );
  p_wk->clear_chr = 0xF;
  p_wk->p_font    = p_font;
  p_wk->p_que     = p_que;
  p_wk->print_update  = BC_TEXT_TYPE_NONE;

  //文字送りカーソル作成
  p_wk->p_keycursor  = APP_KEYCURSOR_Create( p_wk->clear_chr, TRUE, FALSE, heapID );

  //バッファ作成
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( 512, heapID );

	//BMPWIN作成
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( frm, 1, 19, 30, 4, font_plt, GFL_BMP_CHRAREA_GET_B );

	//プリントキュー設定
	PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );

	//転送
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );
	GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );

  p_wk->p_tcbl    = GFL_TCBL_Init( heapID, heapID, 1, 32 );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  テキスト  破棄
 *
 *	@param	BC_TEXT_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BC_TEXT_Exit( BC_TEXT_WORK* p_wk )
{ 
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream  = NULL;
  }

  GFL_TCBL_Exit( p_wk->p_tcbl );

  BmpWinFrame_Clear( p_wk->p_bmpwin, WINDOW_TRANS_ON );
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );

  GFL_STR_DeleteBuffer( p_wk->p_strbuf );

  APP_KEYCURSOR_Delete( p_wk->p_keycursor );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  テキスト  描画処理
 *
 *	@param	BC_TEXT_WORK* p_wk   ワーク
 *
 */
//-----------------------------------------------------------------------------
void BC_TEXT_Main( BC_TEXT_WORK* p_wk )
{ 
  switch( p_wk->print_update )
  { 
  default:
    /* fallthor */
  case BC_TEXT_TYPE_NONE:
    break;

  case BC_TEXT_TYPE_QUE:
    p_wk->is_end_print  = PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );
    break;

  case BC_TEXT_TYPE_STREAM:
    if( p_wk->p_stream )
    { 
      PRINTSTREAM_STATE state;
      state  = PRINTSYS_PrintStreamGetState( p_wk->p_stream );

      APP_KEYCURSOR_Main( p_wk->p_keycursor, p_wk->p_stream, p_wk->p_bmpwin );

      switch( state )
      { 
      case PRINTSTREAM_STATE_RUNNING:  ///< 処理実行中（文字列が流れている）

        // メッセージスキップ
        if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) )
        {
          PRINTSYS_PrintStreamShortWait( p_wk->p_stream, 0 );
        }
        break;

      case PRINTSTREAM_STATE_PAUSE:    ///< 一時停止中（ページ切り替え待ち等）

        //改行
        if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) )
        { 
          PRINTSYS_PrintStreamReleasePause( p_wk->p_stream );
        }
        break;

      case PRINTSTREAM_STATE_DONE:     ///< 文字列終端まで表示完了
        p_wk->is_end_print  = TRUE;
        break;
      }
    }
    break;
  }

  GFL_TCBL_Main( p_wk->p_tcbl );

}
//----------------------------------------------------------------------------
/*
 *	@brief  テキスト プリント開始
 *
 *	@param	BC_TEXT_WORK* p_wk ワーク
 *	@param	*p_msg            メッセージ
 *	@param	strID             メッセージID
 *	@param  type              描画タイプ
 */
//-----------------------------------------------------------------------------
void BC_TEXT_Print( BC_TEXT_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, BC_TEXT_TYPE type )
{ 
  //文字列作成
  GFL_MSG_GetString( p_msg, strID, p_wk->p_strbuf );

  //文字描画
  BC_TEXT_PrintInner( p_wk, type );
}

//----------------------------------------------------------------------------
/**
 *	@brief  テキスト  プリント開始  バッファ版
 *
 *	@param	BC_TEXT_WORK* p_wk ワーク
 *	@param	STRBUF *cp_strbuf       文字バッファ
 *	@param	type                    描画タイプ
 *
 */
//-----------------------------------------------------------------------------
void BC_TEXT_PrintBuf( BC_TEXT_WORK* p_wk, const STRBUF *cp_strbuf, BC_TEXT_TYPE type )
{ 
  //文字列作成
  GFL_STR_CopyBuffer( p_wk->p_strbuf, cp_strbuf );

  //文字描画
  BC_TEXT_PrintInner( p_wk, type );
}
//----------------------------------------------------------------------------
/**
 *	@brief  テキスト文字描画プライベート
 *
 *	@param	BC_TEXT_WORK* p_wk ワーク
 *	@param	type              描画タイプ
 *
 */
//-----------------------------------------------------------------------------
static void BC_TEXT_PrintInner( BC_TEXT_WORK* p_wk, BC_TEXT_TYPE type )
{ 
  //一端消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );

  //ストリーム破棄
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream  = NULL;
  }

  //タイプごとの文字描画
  switch( type )
  { 
  case BC_TEXT_TYPE_QUE:     //プリントキューを使う
    PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_wk->p_font );
    p_wk->print_update  = BC_TEXT_TYPE_QUE;
    break;

  case BC_TEXT_TYPE_STREAM:  //ストリームを使う
    p_wk->p_stream  = PRINTSYS_PrintStream( p_wk->p_bmpwin, 0, 0, p_wk->p_strbuf,
        p_wk->p_font, MSGSPEED_GetWait(), p_wk->p_tcbl, 0, p_wk->heapID, p_wk->clear_chr );
    p_wk->print_update  = BC_TEXT_TYPE_STREAM;
    break;
  }

  p_wk->is_end_print  = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  テキスト  プリント終了待ち  （QUEの場合はQUE終了、STREAMのときは最後まで文字描画終了）
 *
 *	@param	const BC_TEXT_WORK *cp_wk ワーク
 *
 *	@return TRUEならば文字描画完了  FALSEならば最中。
 */
//-----------------------------------------------------------------------------
BOOL BC_TEXT_IsEndPrint( const BC_TEXT_WORK *cp_wk )
{ 
  return cp_wk->is_end_print;
}

//----------------------------------------------------------------------------
/**
 *	@brief  テキスト  枠を描画
 *
 *	@param	BC_TEXT_WORK *p_wk   ワーク
 *	@param  フレームのキャラ
 *	@param  フレームのパレット
 */
//-----------------------------------------------------------------------------
void BC_TEXT_WriteWindowFrame( BC_TEXT_WORK *p_wk, u16 frm_chr, u8 frm_plt )
{ 
  BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_ON, frm_chr, frm_plt );
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  リスト
 *				    ・簡単にリストを出すために最大値決めうち
 *            ・表示数＝リスト最大数
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================/
#define BC_LIST_W  (28)
//-------------------------------------
///	選択リスト
//=====================================
struct _BC_LIST_WORK
{ 
  GFL_BMPWIN        *p_bmpwin;
  PRINT_QUE         *p_que;
  PRINT_UTIL        print_util;
  BMPMENULIST_WORK  *p_list;
  BMP_MENULIST_DATA *p_list_data;
};
//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化
 *
 *	@param	BC_LIST_SETUP *cp_setup  リスト設定ワーク
 *	@param	heapID                    heapID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BC_LIST_WORK * BC_LIST_Init( const BC_LIST_SETUP *cp_setup, HEAPID heapID )
{ 
  BC_LIST_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BC_LIST_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(BC_LIST_WORK) );
  p_wk->p_que = cp_setup->p_que;

  //BMPWIN作成
  { 
    //右下、テキストボックスの上に位置するため
    //表示項目から位置、高さを計算
    const u8 w  = BC_LIST_W;
    const u8 h  = cp_setup->list_max * 2;
    const u8 x  = 32 / 2 - w / 2; //1はフレーム分
    const u8 y  = (24 - 6) / 2 - h / 2; //１は自分のフレーム分と6はテキスト分
    p_wk->p_bmpwin  = GFL_BMPWIN_Create( cp_setup->frm, x, y, w, h, cp_setup->font_plt, GFL_BMP_CHRAREA_GET_B );
    BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_OFF, cp_setup->frm_chr, cp_setup->frm_plt );
    GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );

    PRINT_UTIL_Setup( &p_wk->print_util, p_wk->p_bmpwin );
  }
  //リストデータ作成
  { 
    int i;
    p_wk->p_list_data = BmpMenuWork_ListCreate( cp_setup->list_max, heapID );
    for( i = 0; i < cp_setup->list_max; i++ )
    { 
      BmpMenuWork_ListAddArchiveString( p_wk->p_list_data,
          cp_setup->p_msg, cp_setup->strID[ i ], i, heapID );
    }
  }
  //リスト作成
  { 
    static const BMPMENULIST_HEADER sc_def_header =
    { 
      NULL,
      NULL,
      NULL,
      NULL,

      0,  //count
      0,  //line
      0,  //rabel_x
      13, //data_x  
      0,  //cursor_x
      3,  //line_y
      1,  //f
      15, //b
      2,  //s
      0,  //msg_spc
      1,  //line_spc
      BMPMENULIST_NO_SKIP,  //page_skip
      0,  //font
      0,  //c_disp_f
      NULL,
      12,
      12,

      NULL,
      NULL,
      NULL,
      NULL,
      
    };
    BMPMENULIST_HEADER  header;
    header  = sc_def_header;

    header.list         = p_wk->p_list_data;
    header.count        = cp_setup->list_max;
    header.line         = cp_setup->list_max;
    header.win          = p_wk->p_bmpwin;
    header.msgdata      = cp_setup->p_msg;
    header.print_util   = &p_wk->print_util;
    header.print_que    = cp_setup->p_que;
    header.font_handle  = cp_setup->p_font;
    p_wk->p_list  = BmpMenuList_Set( &header, 0, 0, heapID );

    BmpMenuList_SetCursorBmp( p_wk->p_list, heapID );
    BmpMenuList_SetCancelMode( p_wk->p_list, BMPMENULIST_CANCELMODE_NOT );
    
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト破棄処理
 *
 *	@param	BC_LIST_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BC_LIST_Exit( BC_LIST_WORK *p_wk )
{ 
  BmpMenuList_Exit( p_wk->p_list, NULL, NULL );
  BmpMenuWork_ListDelete( p_wk->p_list_data );

  BmpWinFrame_Clear( p_wk->p_bmpwin, WINDOW_TRANS_ON );
  GFL_BMPWIN_ClearScreen( p_wk->p_bmpwin );
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  リストメイン処理
 *
 *	@param	BC_LIST_WORK *p_wk   ワーク
 *
 *	@return 選択していないならばBC_LIST_SELECT_NULL それ以外ならば選択したリストインデックス
 */
//-----------------------------------------------------------------------------
u32 BC_LIST_Main( BC_LIST_WORK *p_wk )
{ 
  PRINT_UTIL_Trans( &p_wk->print_util, p_wk->p_que );
  return BmpMenuList_Main( p_wk->p_list );
}

