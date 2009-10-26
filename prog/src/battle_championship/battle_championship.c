//======================================================================
/**
 * @file	battle_championship.c
 * @brief	大会メニュー
 * @author	ariizumi
 * @data	09/10/08
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

#include "arc_def.h"
#include "battle_championship.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_irc_battle.h"

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

#define BATTLE_CHAMPIONSHIP_BG_PAL_TASKMENU (0xC)
#define BATTLE_CHAMPIONSHIP_BG_PAL_WINFRAME (0xD)
#define BATTLE_CHAMPIONSHIP_BG_PAL_FONT (0xE)

#define BATTLE_CHAMPIONSHIP_WINFRAME_CGX (1)

#define BATTLE_CHAMPIONSHIP_MENU_ITEM_NUM (4)

#define BATTLE_CHAMPIONSHIP_MENU_WIDTH (24)
#define BATTLE_CHAMPIONSHIP_MENU_TOP  (4)
#define BATTLE_CHAMPIONSHIP_MENU_LEFT (4)

#define BATTLE_CHAMPIONSHIP_MEMBER_NUM (2)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//ステート
typedef enum
{
  BCS_FADEIN,
  BCS_FADEIN_WAIT,

  BCS_FIRSTMENU_OPEN,
  BCS_FIRSTMENU_WAIT,
  
  BCS_CHANGEPROC_FADEOUT,
  BCS_CHANGEPROC_FADEOUT_WAIT,
  BCS_CHANGEPROC_WAIT,
  BCS_CHANGEPROC_FADEIN,
  BCS_CHANGEPROC_FADEIN_WAIT,

  BCS_FADEOUT,
  BCS_FADEOUT_WAIT,
}BATTLE_CHAMPIONSHIP_STATE;

//切り替え先Proc種類
typedef enum
{
  BCNP_WIFI_BATTLE,
  BCNP_EVENT_BATTLE,
  BCNP_DIGITAL_MEMBERSHIP,
  
}BATTLE_CHAMPIONSHIP_NEXT_PROC;
//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{
  HEAPID heapId;
  BATTLE_CHAMPIONSHIP_STATE state;
  BATTLE_CHAMPIONSHIP_NEXT_PROC nextProcType;
  
  //大会情報
  BATTLE_CHAMPIONSHIP_DATA csData;
  
  //メッセージ用
  GFL_TCBLSYS     *tcblSys;
  GFL_BMPWIN      *msgWin;
  GFL_FONT        *fontHandle;
  PRINT_STREAM    *printHandle;
  GFL_MSGDATA     *msgHandle;
  STRBUF          *msgStr;
  
  PRINT_QUE       *taskMenuQue;
  
  //taskmenu
  APP_TASKMENU_RES *taskMenuRes;
  APP_TASKMENU_WORK *taskMenuWork;
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
static void BATTLE_CHAMPIONSHIP_HideMessage( BATTLE_CHAMPIONSHIP_WORK *work );
static const BOOL BATTLE_CHAMPIONSHIP_IsFinishMessage( BATTLE_CHAMPIONSHIP_WORK *work );

static void BATTLE_CHAMPIONSHIP_OpenFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work );
static void BATTLE_CHAMPIONSHIP_CloseFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work );
static const BOOL BATTLE_CHAMPIONSHIP_UpdateFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work );

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
  
  work->heapId = HEAPID_BATTLE_CHAMPIONSHIP;

  BATTLE_CHAMPIONSHIP_InitGraphic( work );
  BATTLE_CHAMPIONSHIP_InitMessage( work );
  BATTLE_CHAMPIONSHIP_LoadResource( work );
  
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

  BATTLE_CHAMPIONSHIP_ReleaseResource( work );
  BATTLE_CHAMPIONSHIP_TermMessage( work );
  BATTLE_CHAMPIONSHIP_TermGraphic( work );

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
    WIPE_SYS_Start( WIPE_PATTERN_FSAM , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = BCS_FADEIN_WAIT;
    break;
    
  case BCS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
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
      const BOOL ret = BATTLE_CHAMPIONSHIP_UpdateFirstMenu( work );
      if( ret == TRUE )
      {
        const u8 curPos = APP_TASKMENU_GetCursorPos( work->taskMenuWork );
        switch( curPos )
        {
        case 0: //WiFi大会
          work->nextProcType = BCNP_WIFI_BATTLE;
          //戻すためにステート移項仮
          work->state = BCS_FADEOUT;
          //work->state = BCS_CHANGEPROC_FADEOUT;
          break;
        case 1: //イベント大会
          work->nextProcType = BCNP_EVENT_BATTLE;
          work->state = BCS_CHANGEPROC_FADEOUT;
          break;
        case 2: //デジタル選手証
          work->nextProcType = BCNP_DIGITAL_MEMBERSHIP;
          //戻すためにステート移項仮
          work->state = BCS_FADEOUT;
          //work->state = BCS_CHANGEPROC_FADEOUT;
          break;
        case 3: //やめる
          work->state = BCS_FADEOUT;
          break;
        }
        BATTLE_CHAMPIONSHIP_CloseFirstMenu( work );
      }
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
    
  //フェードしてProcを切り替える処理
  case BCS_CHANGEPROC_FADEOUT:
    WIPE_SYS_Start( WIPE_PATTERN_FSAM , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = BCS_CHANGEPROC_FADEOUT_WAIT;
    break;
    
  case BCS_CHANGEPROC_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      work->state = BCS_CHANGEPROC_WAIT;
    }
    break;
    
  case BCS_CHANGEPROC_WAIT:
    work->state = BCS_CHANGEPROC_FADEIN;
    break;

  case BCS_CHANGEPROC_FADEIN:
    WIPE_SYS_Start( WIPE_PATTERN_FSAM , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = BCS_CHANGEPROC_FADEIN_WAIT;
    break;
    
  case BCS_CHANGEPROC_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      work->state = BCS_FIRSTMENU_OPEN;
    }
    break;
    
  }

  return GFL_PROC_RES_CONTINUE;
//  return GFL_PROC_RES_FINISH;
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
  
  GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
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
      GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000,0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    // BG1 MAIN (文字
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x08000,0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
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
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x06000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    // BG3 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x06000,
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
  //下画面背景
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_battle_championship_connect_NCLR , 
                    PALTYPE_MAIN_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_battle_championship_connect_sub_NCGR ,
                    BATTLE_CHAMPIONSHIP_FRAME_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_battle_championship_connect_sub_NSCR , 
                    BATTLE_CHAMPIONSHIP_FRAME_BG ,  0 , 0, FALSE , work->heapId );
  //上画面背景
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_battle_championship_connect_NCLR , 
                    PALTYPE_SUB_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_battle_championship_connect_NCGR ,
                    BATTLE_CHAMPIONSHIP_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_battle_championship_connect_01_NSCR , 
                    BATTLE_CHAMPIONSHIP_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );

  GFL_ARC_CloseDataHandle( arcHandle );
  
  //TaskMenu用
  work->taskMenuRes = APP_TASKMENU_RES_Create( BATTLE_CHAMPIONSHIP_FRAME_MENU , 
                                               BATTLE_CHAMPIONSHIP_BG_PAL_TASKMENU ,
                                               work->fontHandle , work->taskMenuQue ,
                                               work->heapId );
  
  
}

static void BATTLE_CHAMPIONSHIP_ReleaseResource( BATTLE_CHAMPIONSHIP_WORK *work )
{
  APP_TASKMENU_RES_Delete( work->taskMenuRes );
}

#pragma mark [>message func
static void BATTLE_CHAMPIONSHIP_InitMessage( BATTLE_CHAMPIONSHIP_WORK *work )
{
  //メッセージ
  work->msgWin = GFL_BMPWIN_Create( BATTLE_CHAMPIONSHIP_FRAME_STR , 
                  1 , 19 , 30 , 4 , BATTLE_CHAMPIONSHIP_BG_PAL_FONT ,
                  GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_MakeScreen( work->msgWin );

  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_irc_battle_dat , work->heapId );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , BATTLE_CHAMPIONSHIP_BG_PAL_FONT*16*2, 16*2, work->heapId );
  BmpWinFrame_GraphicSet( BATTLE_CHAMPIONSHIP_FRAME_STR , BATTLE_CHAMPIONSHIP_WINFRAME_CGX , 
                          BATTLE_CHAMPIONSHIP_BG_PAL_WINFRAME , 1 , work->heapId );
  
  work->tcblSys = GFL_TCBL_Init( work->heapId , work->heapId , 3 , 0x100 );
  work->printHandle = NULL;
  work->msgStr = NULL;
  
  work->taskMenuQue = PRINTSYS_QUE_Create( work->heapId );
  
}

static void BATTLE_CHAMPIONSHIP_TermMessage( BATTLE_CHAMPIONSHIP_WORK *work )
{
  PRINTSYS_QUE_Delete( work->taskMenuQue );
  
  GFL_TCBL_Exit( work->tcblSys );
  if( work->msgStr != NULL )
  {
    GFL_STR_DeleteBuffer( work->msgStr );
  }
  if( work->printHandle != NULL )
  {
    PRINTSYS_PrintStreamDelete( work->printHandle );
  }
  GFL_BMPWIN_Delete( work->msgWin );
  GFL_MSG_Delete( work->msgHandle );
  GFL_FONT_Delete( work->fontHandle );
}

static void BATTLE_CHAMPIONSHIP_UpdateMessage( BATTLE_CHAMPIONSHIP_WORK *work )
{
  GFL_TCBL_Main( work->tcblSys );

  if( work->printHandle != NULL  )
  {
    const PRINTSTREAM_STATE state = PRINTSYS_PrintStreamGetState( work->printHandle );
    if( state == PRINTSTREAM_STATE_DONE )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        PRINTSYS_PrintStreamDelete( work->printHandle );
        work->printHandle = NULL;
      }
    }
    else
    if( state == PRINTSTREAM_STATE_PAUSE )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        PRINTSYS_PrintStreamReleasePause( work->printHandle );
      }
    }
  }
}
static void BATTLE_CHAMPIONSHIP_ShowMessage( BATTLE_CHAMPIONSHIP_WORK *work  , const u16 msgNo )
{
  if( work->printHandle != NULL )
  {
    OS_TPrintf( "Message is not finish!!\n" );
    PRINTSYS_PrintStreamDelete( work->printHandle );
    work->printHandle = NULL;
  }
  {
    if( work->msgStr != NULL )
    {
      GFL_STR_DeleteBuffer( work->msgStr );
      work->msgStr = NULL;
    }
    
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgWin ) , 0xF );
    work->msgStr = GFL_MSG_CreateString( work->msgHandle , msgNo );
    work->printHandle = PRINTSYS_PrintStream( work->msgWin , 0,0, work->msgStr ,work->fontHandle ,
                        MSGSPEED_GetWait() , work->tcblSys , 2 , work->heapId , 0xf );
    BmpWinFrame_Write( work->msgWin , WINDOW_TRANS_ON_V , 
                       BATTLE_CHAMPIONSHIP_WINFRAME_CGX , BATTLE_CHAMPIONSHIP_BG_PAL_WINFRAME );
  }
}

static void BATTLE_CHAMPIONSHIP_HideMessage( BATTLE_CHAMPIONSHIP_WORK *work )
{
  if( work->printHandle != NULL )
  {
    OS_TPrintf( "Message is not finish!!\n" );
    PRINTSYS_PrintStreamDelete( work->printHandle );
    work->printHandle = NULL;
  }
  if( work->msgStr != NULL )
  {
    GFL_STR_DeleteBuffer( work->msgStr );
    work->msgStr = NULL;
  }
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgWin ) , 0 );
  BmpWinFrame_Clear( work->msgWin , WINDOW_TRANS_ON_V );
  GFL_BMPWIN_TransVramCharacter( work->msgWin );

}

static const BOOL BATTLE_CHAMPIONSHIP_IsFinishMessage( BATTLE_CHAMPIONSHIP_WORK *work )
{
  if( work->printHandle == NULL )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

#pragma mark [>menu func
static void BATTLE_CHAMPIONSHIP_OpenFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work )
{
  u8 i;
  APP_TASKMENU_ITEMWORK taskMenuItem[BATTLE_CHAMPIONSHIP_MENU_ITEM_NUM];
  APP_TASKMENU_INITWORK taskMenuInit;
  
  taskMenuInit.heapId = work->heapId;
  taskMenuInit.itemWork = taskMenuItem;
  taskMenuInit.itemNum = BATTLE_CHAMPIONSHIP_MENU_ITEM_NUM;
  taskMenuInit.posType = ATPT_LEFT_UP;
  taskMenuInit.charPosX = BATTLE_CHAMPIONSHIP_MENU_LEFT;
  taskMenuInit.charPosY = BATTLE_CHAMPIONSHIP_MENU_TOP;
  taskMenuInit.w = BATTLE_CHAMPIONSHIP_MENU_WIDTH;
  taskMenuInit.h = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;
  //BATTLE_CHAMPIONSHIP_MENU_WIDTH;
  
  for( i=0;i<BATTLE_CHAMPIONSHIP_MENU_ITEM_NUM;i++ )
  {
    taskMenuItem[i].str = GFL_MSG_CreateString( work->msgHandle , IRC_CHAMPIONSHIP_MSG_0+i );
    taskMenuItem[i].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
    taskMenuItem[i].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  }
  
  work->taskMenuWork = APP_TASKMENU_OpenMenu( &taskMenuInit , work->taskMenuRes );

  for( i=0;i<BATTLE_CHAMPIONSHIP_MENU_ITEM_NUM;i++ )
  {
    GFL_STR_DeleteBuffer( taskMenuItem[i].str );
  }
}

static void BATTLE_CHAMPIONSHIP_CloseFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work )
{
  APP_TASKMENU_CloseMenu( work->taskMenuWork );
}

static const BOOL BATTLE_CHAMPIONSHIP_UpdateFirstMenu( BATTLE_CHAMPIONSHIP_WORK *work )
{
  APP_TASKMENU_UpdateMenu( work->taskMenuWork );
  return APP_TASKMENU_IsFinish( work->taskMenuWork );
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
