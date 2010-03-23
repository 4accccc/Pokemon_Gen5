//======================================================================
/**
 * @file	cg_help.c
 * @brief	C-Gearヘルプ画面
 * @author	ariizumi
 * @data	10/03/22
 *
 * モジュール名：CG_HELP_
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/net_err.h"
#include "system/bmp_winframe.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/gf_font.h"
#include "app/app_taskmenu.h"


#include "arc_def.h"
#include "cg_comm.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_cg_help.h"

#include "net_app/cg_help.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define CG_HELP_FRAME_ICON  (GFL_BG_FRAME0_S)
#define CG_HELP_FRAME_MSG  (GFL_BG_FRAME1_S)
#define CG_HELP_FRAME_BG1  (GFL_BG_FRAME2_S)
#define CG_HELP_FRAME_BG2  (GFL_BG_FRAME3_S)

#define CG_HELP_PLT_MAIN_TASKMENU (11)  //2本
#define CG_HELP_PLT_MAIN_MSGWIN (13)
#define CG_HELP_PLT_MAIN_FONT   (14)
#define CG_HELP_MSGWIN_CGX    (1)

#define CG_HELP_PAGE_MAX  (7)
//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  CHS_FADEIN,
  CHS_FADEIN_WAIT,
  CHS_FADEOUT,
  CHS_FADEOUT_WAIT,
  CHS_MAIN,
}CG_HELP_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  HEAPID heapId;
  CG_HELP_INIT_WORK *initWork;

  BOOL isNetErr;
  CG_HELP_STATE state;
  GFL_TCB *vBlankTcb;
  u8     page;

  //メッセージ用
  BOOL            isUpdateMsg;
  GFL_BMPWIN      *msgWin;
  GFL_BMPWIN      *infoWin;
  GFL_FONT        *fontHandle;
  GFL_MSGDATA     *msgHandle;
  PRINT_QUE       *printQue;
  APP_TASKMENU_RES  *takmenures;
  APP_TASKMENU_WIN_WORK *nextButton;
  APP_TASKMENU_WIN_WORK *endButton;

  GFL_CLUNIT  *cellUnit;

}CG_HELP_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void CG_HELP_Init( CG_HELP_WORK *work );
static void CG_HELP_Term( CG_HELP_WORK *work );
static const BOOL CG_HELP_Main( CG_HELP_WORK *work );
static void CG_HELP_VBlankFunc(GFL_TCB *tcb, void *wk );

static void CG_HELP_InitGraphic( CG_HELP_WORK *work );
static void CG_HELP_TermGraphic( CG_HELP_WORK *work );
static void CG_HELP_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void CG_HELP_LoadResource( CG_HELP_WORK *work );
static void CG_HELP_ReleaseResource( CG_HELP_WORK *work );
static void CG_HELP_InitMessage( CG_HELP_WORK *work );
static void CG_HELP_TermMessage( CG_HELP_WORK *work );

static void CG_HELP_UpdateUI( CG_HELP_WORK *work );

static void CG_HELP_DispPage( CG_HELP_WORK *work , const u8 page);

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_A,             // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,       // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,         // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_128_B ,           // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,      // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_128_D,        // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_NONE,             // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,         // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_128K
};

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
static void CG_HELP_Init( CG_HELP_WORK *work )
{

  work->state = CHS_FADEIN;

  CG_HELP_InitGraphic( work );
  CG_HELP_LoadResource( work );
  CG_HELP_InitMessage( work );

  work->vBlankTcb = GFUser_VIntr_CreateTCB( CG_HELP_VBlankFunc , work , 8 );
  
  work->page = 0;
  CG_HELP_DispPage( work , work->page );

}

static void CG_HELP_Term( CG_HELP_WORK *work )
{
  GFL_TCB_DeleteTask( work->vBlankTcb );
  

  CG_HELP_TermMessage( work );
  CG_HELP_ReleaseResource( work );
  CG_HELP_TermGraphic( work );
}

static const BOOL CG_HELP_Main( CG_HELP_WORK *work )
{

  switch( work->state )
  {
  case CHS_FADEIN:
    WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );

    work->state = CHS_FADEIN_WAIT;
    break;
    
  case CHS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      work->state = CHS_MAIN;
    }
    break;

  case CHS_FADEOUT:
    if( APP_TASKMENU_WIN_IsFinish( work->endButton ) == TRUE )
    {
      WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
      work->state = CHS_FADEOUT_WAIT;
    }
    break;
    
  case CHS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return TRUE;
    }
    break;
  
  case CHS_MAIN:
    if( work->isUpdateMsg == FALSE )
    {
      CG_HELP_UpdateUI( work );
    }
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
    {
      GFL_BG_LoadScreenV_Req(CG_HELP_FRAME_ICON);
    }
    break;
  }
  

  if( work->isUpdateMsg == TRUE &&
      PRINTSYS_QUE_IsFinished( work->printQue ) == TRUE )
  {
    if( APP_TASKMENU_WIN_IsFinish( work->nextButton ) == TRUE ||
        work->page == 0 )
    {
      GFL_BMPWIN_TransVramCharacter( work->msgWin );
      GFL_BMPWIN_TransVramCharacter( work->infoWin );
      work->isUpdateMsg = FALSE;
      if( work->page == CG_HELP_PAGE_MAX-1 )
      {
        APP_TASKMENU_WIN_Delete( work->nextButton );
        GFL_BG_LoadScreenV_Req(CG_HELP_FRAME_ICON);
        work->nextButton = NULL;
      }
      else
      {
        APP_TASKMENU_WIN_ResetDecide( work->nextButton );
      }
    }
  }

  GFL_BG_SetScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_DEC, 1 );
  
  //OBJの更新
  GFL_CLACT_SYS_Main();
  
  if( work->nextButton != NULL )
  {
    APP_TASKMENU_WIN_Update( work->nextButton );
  }
  APP_TASKMENU_WIN_Update( work->endButton );
  return FALSE;
}

static void CG_HELP_VBlankFunc(GFL_TCB *tcb, void *wk )
{
  //OBJの更新
  GFL_CLACT_SYS_VBlankFunc();
}


static void CG_HELP_InitGraphic( CG_HELP_WORK *work )
{
  GFL_DISP_SetDispOn();
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
    static const GFL_BG_SYS_HEADER sys_data = 
    {
        GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    // BG0 SUB (文字
    static const GFL_BG_BGCNT_HEADER header_sub0 =
    {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000,0x08000,
      GX_BG_EXTPLTT_23, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG1 SUB (Win
    static const GFL_BG_BGCNT_HEADER header_sub1 =
    {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000,0x06000,
      GX_BG_EXTPLTT_23, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 SUB (背景1
    static const GFL_BG_BGCNT_HEADER header_sub2 =
    {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x10000,0x08000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 SUB (背景2
    static const GFL_BG_BGCNT_HEADER header_sub3 =
    {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x18000,0x08000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    GFL_BG_SetBGMode( &sys_data );

    CG_HELP_SetupBgFunc( &header_sub0  , CG_HELP_FRAME_ICON , GFL_BG_MODE_TEXT );
    CG_HELP_SetupBgFunc( &header_sub1  , CG_HELP_FRAME_MSG , GFL_BG_MODE_TEXT );
    CG_HELP_SetupBgFunc( &header_sub2  , CG_HELP_FRAME_BG1 , GFL_BG_MODE_TEXT );
    CG_HELP_SetupBgFunc( &header_sub3  , CG_HELP_FRAME_BG2 , GFL_BG_MODE_TEXT );

    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2 , GX_BLEND_PLANEMASK_BG3 , 15 , 4 );
  }
  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    cellSysInitData.CGR_RegisterMax = 64;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    work->cellUnit = GFL_CLACT_UNIT_Create( 48 , 0, work->heapId );
    GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit );

    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
  }
}

static void CG_HELP_TermGraphic( CG_HELP_WORK *work )
{
  GFL_CLACT_UNIT_Delete( work->cellUnit );
  GFL_CLACT_SYS_Delete();
  GFL_BG_FreeBGControl( CG_HELP_FRAME_ICON );
  GFL_BG_FreeBGControl( CG_HELP_FRAME_MSG );
  GFL_BG_FreeBGControl( CG_HELP_FRAME_BG1 );
  GFL_BG_FreeBGControl( CG_HELP_FRAME_BG2 );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

static void CG_HELP_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

static void CG_HELP_LoadResource( CG_HELP_WORK *work )
{
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_CG_COMM , work->heapId );

  //下画面
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_cg_comm_comm_bg_NCLR , 
                    PALTYPE_SUB_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_cg_comm_comm_bg_NCGR ,
                    CG_HELP_FRAME_BG1 , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_cg_comm_comm_help_NSCR , 
                    CG_HELP_FRAME_BG1 ,  0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_cg_comm_comm_bg_NCGR ,
                    CG_HELP_FRAME_BG2 , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_cg_comm_comm_base_NSCR , 
                    CG_HELP_FRAME_BG2 ,  0 , 0, FALSE , work->heapId );
  
  GFL_ARC_CloseDataHandle( arcHandle );
  
  //OBJ
  {
    arcHandle = GFL_ARC_OpenDataHandle( ARCID_CG_COMM , work->heapId );

    GFL_ARC_CloseDataHandle( arcHandle );
  }
}

static void CG_HELP_ReleaseResource( CG_HELP_WORK *work )
{

}

static void CG_HELP_InitMessage( CG_HELP_WORK *work )
{
  //フォント読み込み
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  
  //メッセージ
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_cg_help_dat , work->heapId );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , CG_HELP_PLT_MAIN_FONT*0x20, 16*2, work->heapId );
  BmpWinFrame_GraphicSet( CG_HELP_FRAME_MSG , CG_HELP_MSGWIN_CGX , CG_HELP_PLT_MAIN_MSGWIN , 0 , work->heapId );
  GFL_FONTSYS_SetDefaultColor();
  
  work->printQue = PRINTSYS_QUE_Create( work->heapId );
  work->takmenures  = APP_TASKMENU_RES_Create( CG_HELP_FRAME_ICON, CG_HELP_PLT_MAIN_TASKMENU, work->fontHandle, work->printQue, work->heapId );

  //説明Win
  work->infoWin = GFL_BMPWIN_Create( CG_HELP_FRAME_MSG , 
                                    2 , 
                                    4 ,
                                    28 , 
                                    4 , 
                                    CG_HELP_PLT_MAIN_FONT ,
                                    GFL_BMP_CHRAREA_GET_B );
  work->msgWin = GFL_BMPWIN_Create( CG_HELP_FRAME_MSG , 
                                    2 , 
                                    9 ,
                                    28 , 
                                    12 , 
                                    CG_HELP_PLT_MAIN_FONT ,
                                    GFL_BMP_CHRAREA_GET_B );
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->infoWin ) , 0x0f );
  GFL_BMPWIN_TransVramCharacter( work->infoWin );
  GFL_BMPWIN_MakeScreen( work->infoWin );
  BmpWinFrame_Write( work->infoWin , WINDOW_TRANS_ON_V , CG_HELP_MSGWIN_CGX , CG_HELP_PLT_MAIN_MSGWIN );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgWin ) , 0x0 );
  GFL_BMPWIN_TransVramCharacter( work->msgWin );
  GFL_BMPWIN_MakeScreen( work->msgWin );
  GFL_BG_LoadScreenReq(CG_HELP_FRAME_MSG);
  
  work->isUpdateMsg = FALSE;
  
  //ボタン作成
  {
    APP_TASKMENU_ITEMWORK initWork;
    initWork.str = GFL_MSG_CreateString( work->msgHandle , CG_HELP_SYS_01 );
    initWork.msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
    initWork.type = APP_TASKMENU_WIN_TYPE_NORMAL;
    work->nextButton = APP_TASKMENU_WIN_Create( work->takmenures , &initWork ,
                                        32-(APP_TASKMENU_PLATE_WIDTH*2) , 21 , APP_TASKMENU_PLATE_WIDTH , work->heapId );
    GFL_STR_DeleteBuffer( initWork.str );

    initWork.str = GFL_MSG_CreateString( work->msgHandle , CG_HELP_SYS_02 );
    initWork.type = APP_TASKMENU_WIN_TYPE_RETURN;
    work->endButton =  APP_TASKMENU_WIN_Create( work->takmenures , &initWork ,
                                        32-APP_TASKMENU_PLATE_WIDTH , 21 , APP_TASKMENU_PLATE_WIDTH , work->heapId );
    GFL_STR_DeleteBuffer( initWork.str );
  }
}

static void CG_HELP_TermMessage( CG_HELP_WORK *work )
{
  if( work->nextButton != NULL )
  {
    APP_TASKMENU_WIN_Delete( work->nextButton );
  }
  APP_TASKMENU_WIN_Delete( work->endButton );
  APP_TASKMENU_RES_Delete( work->takmenures );
  PRINTSYS_QUE_Delete( work->printQue );
  GFL_BMPWIN_Delete( work->msgWin );
  GFL_BMPWIN_Delete( work->infoWin );
  GFL_MSG_Delete( work->msgHandle );
  GFL_FONT_Delete( work->fontHandle );
}

static void CG_HELP_UpdateUI( CG_HELP_WORK *work )
{
  //ボタンとの判定
  GFL_UI_TP_HITTBL hitTbl[] =
  {
    { 21*8 , 
      192 , 
      (32-(APP_TASKMENU_PLATE_WIDTH*2))*8 , 
      (32-(APP_TASKMENU_PLATE_WIDTH  ))*8 } ,
    { 21*8 , 
      192 , 
      (32-APP_TASKMENU_PLATE_WIDTH)*8 , 
      255 } ,
    { GFL_UI_TP_HIT_END ,0,0,0 },
  };
  const int ret = GFL_UI_TP_HitTrg( hitTbl );
  
  switch( ret )
  {
  case 0:
    if( work->page < CG_HELP_PAGE_MAX-1 )
    {
      work->page++;
      CG_HELP_DispPage( work , work->page );
      APP_TASKMENU_WIN_SetDecide( work->nextButton , TRUE );
    }
    break;
  case 1:
    APP_TASKMENU_WIN_SetDecide( work->endButton , TRUE );
    work->state = CHS_FADEOUT;
    break;
  }
}


static void CG_HELP_DispPage( CG_HELP_WORK *work , const u8 page)
{
  STRBUF *str;
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->infoWin ) , 0xf );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgWin ) , 0x0 );
  
  str = GFL_MSG_CreateString( work->msgHandle , CG_HELP_INFO_01 + page );
  PRINTSYS_PrintQue( work->printQue , GFL_BMPWIN_GetBmp( work->infoWin ) , 
          0 , 0 , str , work->fontHandle );
  GFL_STR_DeleteBuffer( str );

  str = GFL_MSG_CreateString( work->msgHandle , CG_HELP_MSG_01 + page );
  PRINTSYS_PrintQue( work->printQue , GFL_BMPWIN_GetBmp( work->msgWin ) , 
          0 , 0 , str , work->fontHandle );
  GFL_STR_DeleteBuffer( str );

  work->isUpdateMsg = TRUE;
}

#pragma mark [>proc func
static GFL_PROC_RESULT CG_HELP_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT CG_HELP_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT CG_HELP_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA CGearHelp_ProcData =
{
  CG_HELP_ProcInit,
  CG_HELP_ProcMain,
  CG_HELP_ProcTerm
};

//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT CG_HELP_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  CG_HELP_INIT_WORK *initWork = pwk;
  CG_HELP_WORK *work;
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP , HEAPID_CG_HELP, 0x60000 );
  work = GFL_PROC_AllocWork( proc, sizeof(CG_HELP_WORK), HEAPID_CG_HELP );
  
  if( pwk == NULL )
  {
    initWork = GFL_HEAP_AllocClearMemory( HEAPID_CG_HELP , sizeof(CG_HELP_INIT_WORK) );
    initWork->myStatus = MyStatus_AllocWork( HEAPID_CG_HELP );
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
    {
      MyStatus_SetMySex( initWork->myStatus , PM_FEMALE );
    }
    else
    {
      MyStatus_SetMySex( initWork->myStatus , PM_MALE );
    }
  }
  
  work->initWork = initWork;
  work->heapId = HEAPID_CG_HELP;
  CG_HELP_Init( work );
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static GFL_PROC_RESULT CG_HELP_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  CG_HELP_WORK *work = mywk;
  
  CG_HELP_Term( work );

  if( pwk == NULL )
  {
    GFL_HEAP_FreeMemory( work->initWork->myStatus );
    GFL_HEAP_FreeMemory( work->initWork );
  }
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_CG_HELP );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  ループ
//--------------------------------------------------------------
static GFL_PROC_RESULT CG_HELP_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  CG_HELP_WORK *work = mywk;
  const BOOL ret = CG_HELP_Main( work );

  if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE &&
      work->isNetErr == FALSE )
  {
    work->isNetErr = TRUE;
    work->state = CHS_FADEOUT;
  }
  
  if( ret == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}
