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

#include "sound/pm_sndsys.h"

#include "arc_def.h"
#include "cg_comm.naix"
#include "c_gear.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_cg_help.h"
#include "field/c_gear/c_gear_obj_NANR_LBLDEFS.h"

#include "net_app/cg_help.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define CG_HELP_FRAME_ICON  (GFL_BG_FRAME0_S)
#define CG_HELP_FRAME_MSG  (GFL_BG_FRAME1_S)
#define CG_HELP_FRAME_BG1  (GFL_BG_FRAME2_S)
#define CG_HELP_FRAME_BG2  (GFL_BG_FRAME3_S)

#define CG_HELP_PLT_MAIN_ICONBASE (10)  
#define CG_HELP_PLT_MAIN_TASKMENU (11)  //2本
#define CG_HELP_PLT_MAIN_MSGWIN (13)    //2本
#define CG_HELP_PLT_MAIN_FONT   (15)
#define CG_HELP_MSGWIN_CGX    (1)

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

typedef enum
{
  CHCR_CGEAR_PLT,
  CHCR_CGEAR_NCG,
  CHCR_CGEAR_ANM,
  
  CHCR_MAX
}CG_HELP_CELL_RES;

typedef enum
{
  CG_HELP_PAGE_IR,
  CG_HELP_PAGE_WIFI,
  CG_HELP_PAGE_WIFI2,
  CG_HELP_PAGE_WIRELESS,
  CG_HELP_PAGE_WIRELESS2,
  CG_HELP_PAGE_WIRELESS3,
  CG_HELP_PAGE_WIRELESS4,
  CG_HELP_PAGE_WIRELESS5,
  CG_HELP_PAGE_SURETIGAI,
  CG_HELP_PAGE_RESARCH,
  CG_HELP_PAGE_CUSTOM,
  CG_HELP_PAGE_HELP,

  CG_HELP_PAGE_MAX,
  
  CG_HELP_COMM_PAGE_END = CG_HELP_PAGE_WIRELESS5,
}CG_HELP_PAGE_TYPE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  HEAPID heapId;
  CG_HELP_INIT_WORK *initWork;

  BOOL isNetErr;
  BOOL isFirst;
  CG_HELP_STATE state;
  GFL_TCB *vBlankTcb;
  u8     page;

  //メッセージ用
  BOOL            isUpdateMsg;
  GFL_BMPWIN      *msgWin;
  GFL_BMPWIN      *infoWin;
  //GFL_BMPWIN      *iconWin;
  GFL_FONT        *fontHandle;
  GFL_MSGDATA     *msgHandle;
  PRINT_QUE       *printQue;
  APP_TASKMENU_RES  *takmenures;
  APP_TASKMENU_WIN_WORK *backButton;
  APP_TASKMENU_WIN_WORK *nextButton;
  APP_TASKMENU_WIN_WORK *endButton;
  
  void *commIconPltRes;
  NNSG2dPaletteData *commIconPlt;

  GFL_CLUNIT  *cellUnit;
  u32         cellResIdx[CHCR_MAX];
  GFL_CLWK    *clwkIcon;

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
static void CG_HELP_DispPageIcon( CG_HELP_WORK *work , const u8 page);

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
  GX_OBJVRAMMODE_CHAR_1D_32K,
  GX_OBJVRAMMODE_CHAR_1D_32K
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
  work->isFirst = TRUE;
  CG_HELP_DispPage( work , work->page );
  GFL_NET_ReloadIconTopOrBottom(FALSE , work->heapId );

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
  
  // QUEメイン
  PRINTSYS_QUE_Main( work->printQue );

  //OBJの更新
  GFL_CLACT_SYS_Main();
  
  if( work->nextButton != NULL )
  {
    APP_TASKMENU_WIN_Update( work->nextButton );
  }
  if( work->backButton != NULL )
  {
    APP_TASKMENU_WIN_Update( work->backButton );
  }
  APP_TASKMENU_WIN_Update( work->endButton );

  //APPTASKMENUより後に！
  if( work->isUpdateMsg == TRUE &&
      PRINTSYS_QUE_IsFinished( work->printQue ) == TRUE )
  {
    if( APP_TASKMENU_WIN_IsFinish( work->nextButton ) == TRUE ||
        APP_TASKMENU_WIN_IsFinish( work->backButton ) == TRUE ||
        work->isFirst == TRUE )
    {
      GFL_BMPWIN_TransVramCharacter( work->msgWin );
      GFL_BMPWIN_TransVramCharacter( work->infoWin );
      work->isUpdateMsg = FALSE;
      work->isFirst = FALSE;
      CG_HELP_DispPageIcon( work , work->page );

      if( work->page == CG_HELP_PAGE_MAX-1 )
      {
        APP_TASKMENU_WIN_ResetDecide( work->nextButton );
        APP_TASKMENU_WIN_Hide( work->nextButton );
      }
      else
      {
        APP_TASKMENU_WIN_Show( work->nextButton );
        APP_TASKMENU_WIN_ResetDecide( work->nextButton );
      }
      if( work->page == 0 )
      {
        APP_TASKMENU_WIN_ResetDecide( work->backButton );
        APP_TASKMENU_WIN_Hide( work->backButton );
      }
      else
      {
        APP_TASKMENU_WIN_Show( work->backButton );
        APP_TASKMENU_WIN_ResetDecide( work->backButton );
      }
    }
  }

  GFL_BG_SetScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_DEC, 1 );
  
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
    arcHandle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR , work->heapId );

    if( MyStatus_GetMySex( work->initWork->myStatus ) == PM_MALE )
    {
      work->cellResIdx[CHCR_CGEAR_PLT] = GFL_CLGRP_PLTT_RegisterEx( arcHandle , 
            NARC_c_gear_c_gear_m_obj_NCLR, CLSYS_DRAW_SUB , 
            0 , 0 , 15 , work->heapId  );
    }
    else
    {
      work->cellResIdx[CHCR_CGEAR_PLT] = GFL_CLGRP_PLTT_RegisterEx( arcHandle , 
            NARC_c_gear_c_gear_f_obj_NCLR, CLSYS_DRAW_SUB , 
            0 , 0 , 15 , work->heapId  );
    }
    work->cellResIdx[CHCR_CGEAR_NCG] = GFL_CLGRP_CGR_Register( arcHandle , 
          NARC_c_gear_c_gear_obj_NCGR , FALSE , CLSYS_DRAW_SUB , work->heapId  );
    work->cellResIdx[CHCR_CGEAR_ANM] = GFL_CLGRP_CELLANIM_Register( arcHandle , 
          NARC_c_gear_c_gear_obj_NCER , NARC_c_gear_c_gear_obj_NANR, work->heapId  );
    
    //通信マーク用背景キャラ
    GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_c_gear_c_gear_NCGR ,
                      CG_HELP_FRAME_ICON , 0 , 0, FALSE , work->heapId );
    work->commIconPltRes = GFL_ARCHDL_UTIL_LoadPalette( arcHandle , NARC_c_gear_c_gear_m_NCLR, &work->commIconPlt , work->heapId );
    GFL_ARC_CloseDataHandle( arcHandle );
  }
  //OBJ初期化
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 4*8;
    cellInitData.pos_y = 6*8;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
  
    work->clwkIcon = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellResIdx[CHCR_CGEAR_PLT],
              work->cellResIdx[CHCR_CGEAR_NCG],
              work->cellResIdx[CHCR_CGEAR_ANM],
              &cellInitData ,CLSYS_DRAW_SUB , work->heapId );

    GFL_CLACT_WK_SetDrawEnable( work->clwkIcon , FALSE );
    GFL_CLACT_WK_SetAutoAnmFlag( work->clwkIcon , TRUE );
  }
}

static void CG_HELP_ReleaseResource( CG_HELP_WORK *work )
{
  GFL_HEAP_FreeMemory( work->commIconPltRes );
  GFL_CLACT_WK_Remove( work->clwkIcon );
  GFL_CLGRP_PLTT_Release( work->cellResIdx[CHCR_CGEAR_PLT] );
  GFL_CLGRP_CGR_Release( work->cellResIdx[CHCR_CGEAR_NCG] );
  GFL_CLGRP_CELLANIM_Release( work->cellResIdx[CHCR_CGEAR_ANM] );
}

static void CG_HELP_InitMessage( CG_HELP_WORK *work )
{
  //フォント読み込み
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  GFL_FONTSYS_SetColor( 0xf, 2, 0);
  
  //メッセージ
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_cg_help_dat , work->heapId );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , CG_HELP_PLT_MAIN_FONT*0x20, 16*2, work->heapId );
  
  //BmpWin パレットを2本で読み直す
  BmpWinFrame_GraphicSet( CG_HELP_FRAME_MSG , CG_HELP_MSGWIN_CGX , CG_HELP_PLT_MAIN_MSGWIN , 0 , work->heapId );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FLDMAP_WINFRAME , BmpWinFrame_WinPalArcGet( 0 ) , PALTYPE_SUB_BG , CG_HELP_PLT_MAIN_MSGWIN*0x20 , 16*2*2 , work->heapId );

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
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->infoWin ) , 1 );
  GFL_BMPWIN_TransVramCharacter( work->infoWin );
  GFL_BMPWIN_MakeScreen( work->infoWin );
  BmpWinFrame_Write( work->infoWin , WINDOW_TRANS_ON_V , CG_HELP_MSGWIN_CGX , CG_HELP_PLT_MAIN_MSGWIN+1 );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgWin ) , 0x0 );
  GFL_BMPWIN_TransVramCharacter( work->msgWin );
  GFL_BMPWIN_MakeScreen( work->msgWin );
  GFL_BG_LoadScreenReq(CG_HELP_FRAME_MSG);
  GFL_BG_LoadScreenReq(CG_HELP_FRAME_ICON);
  
  work->isUpdateMsg = FALSE;
  
  //ボタン作成
  {
    APP_TASKMENU_ITEMWORK initWork;
    initWork.str = GFL_MSG_CreateString( work->msgHandle , CG_HELP_SYS_01 );
    initWork.msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
    initWork.type = APP_TASKMENU_WIN_TYPE_NORMAL;
    work->nextButton = APP_TASKMENU_WIN_Create( work->takmenures , &initWork ,
                                        11 , 21 , 11 , work->heapId );
    GFL_STR_DeleteBuffer( initWork.str );

    initWork.str = GFL_MSG_CreateString( work->msgHandle , CG_HELP_SYS_03 );
    work->backButton =  APP_TASKMENU_WIN_Create( work->takmenures , &initWork ,
                                         0 , 21 , 11 , work->heapId );
    GFL_STR_DeleteBuffer( initWork.str );

    initWork.str = GFL_MSG_CreateString( work->msgHandle , CG_HELP_SYS_02 );
    initWork.type = APP_TASKMENU_WIN_TYPE_RETURN;
    work->endButton =  APP_TASKMENU_WIN_Create( work->takmenures , &initWork ,
                                        22 , 21 , 10 , work->heapId );
    GFL_STR_DeleteBuffer( initWork.str );

    APP_TASKMENU_WIN_Hide( work->backButton );
  }
  
  *(u16*)(HW_DB_BG_PLTT+CG_HELP_PLT_MAIN_FONT*32+2) = 0;
}

static void CG_HELP_TermMessage( CG_HELP_WORK *work )
{
  if( work->nextButton != NULL )
  {
    APP_TASKMENU_WIN_Delete( work->nextButton );
  }
  if( work->backButton != NULL )
  {
    APP_TASKMENU_WIN_Delete( work->backButton );
  }
  APP_TASKMENU_WIN_Delete( work->endButton );
  APP_TASKMENU_RES_Delete( work->takmenures );
  PRINTSYS_QUE_Delete( work->printQue );
  GFL_BMPWIN_Delete( work->msgWin );
  GFL_BMPWIN_Delete( work->infoWin );
  GFL_MSG_Delete( work->msgHandle );
  GFL_FONT_Delete( work->fontHandle );
  GFL_FONTSYS_SetDefaultColor();
}

static void CG_HELP_UpdateUI( CG_HELP_WORK *work )
{
  //ボタンとの判定
  GFL_UI_TP_HITTBL hitTbl[] =
  {
    { 21*8 , 
      192 , 
       0*8 , 
      11*8 } ,
    { 21*8 , 
      192 , 
      11*8 , 
      22*8 } ,
    { 21*8 , 
      192 , 
      22*8 , 
      255 } ,
    { GFL_UI_TP_HIT_END ,0,0,0 },
  };
  const int ret = GFL_UI_TP_HitTrg( hitTbl );
  
  switch( ret )
  {
  case 0:
    if( work->page > 0 )
    {
      work->page--;
      CG_HELP_DispPage( work , work->page );
      APP_TASKMENU_WIN_SetDecide( work->backButton , TRUE );
      PMSND_PlaySystemSE( SEQ_SE_DECIDE1 );
    }
    break;
  case 1:
    if( work->page < CG_HELP_PAGE_MAX-1 )
    {
      work->page++;
      CG_HELP_DispPage( work , work->page );
      APP_TASKMENU_WIN_SetDecide( work->nextButton , TRUE );
      PMSND_PlaySystemSE( SEQ_SE_DECIDE1 );
    }
    break;
  case 2:
    APP_TASKMENU_WIN_SetDecide( work->endButton , TRUE );
    work->state = CHS_FADEOUT;
    PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
    break;
  }
  
  if( GAMESYSTEM_IsBatt10Sleep( work->initWork->gameSys ) == TRUE )
  {
    APP_TASKMENU_WIN_SetDecide( work->endButton , TRUE );
    work->state = CHS_FADEOUT;
  }
}


static void CG_HELP_DispPage( CG_HELP_WORK *work , const u8 page)
{
  STRBUF *str;
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->infoWin ) , 1 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgWin ) , 0x0 );
  
  str = GFL_MSG_CreateString( work->msgHandle , CG_HELP_INFO_01 + page );
  PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( work->infoWin ) , 
          40 , 8 , str , work->fontHandle , PRINTSYS_MACRO_LSB(0xf, 2, 0) );
  GFL_STR_DeleteBuffer( str );

  str = GFL_MSG_CreateString( work->msgHandle , CG_HELP_MSG_01 + page );
  PRINTSYS_PrintQueColor( work->printQue , GFL_BMPWIN_GetBmp( work->msgWin ) , 
          0 , 0 , str , work->fontHandle , PRINTSYS_MACRO_LSB(0xf, 2, 0) );
  GFL_STR_DeleteBuffer( str );

  work->isUpdateMsg = TRUE;
}

static void CG_HELP_DispPageIcon( CG_HELP_WORK *work , const u8 page)
{
  static const anmArr[CG_HELP_PAGE_MAX] = 
    {
      NANR_c_gear_obj_CellAnime_IR,
      NANR_c_gear_obj_CellAnime_WIFI,
      NANR_c_gear_obj_CellAnime_WIFI,
      NANR_c_gear_obj_CellAnime_WIRELESS,
      NANR_c_gear_obj_CellAnime_WIRELESS,
      NANR_c_gear_obj_CellAnime_WIRELESS,
      NANR_c_gear_obj_CellAnime_WIRELESS,
      NANR_c_gear_obj_CellAnime_WIRELESS,
      NANR_c_gear_obj_CellAnime_sure_sample,
      NANR_c_gear_obj_CellAnime_radar,
      NANR_c_gear_obj_CellAnime_cus_on,
      NANR_c_gear_obj_CellAnime_on_off};
  GFL_CLACT_WK_SetAnmSeq( work->clwkIcon , anmArr[page] );
  GFL_CLACT_WK_SetDrawEnable( work->clwkIcon , TRUE );
  
  //BMP設定
  if( page <= CG_HELP_COMM_PAGE_END )
  {
    static const colArr[CG_HELP_COMM_PAGE_END+1] = {1,1,1,1,1,1,1,1};
    static const posArr[CG_HELP_COMM_PAGE_END+1] = {0x0c,0x14,0x14,0x10,0x10,0x10,0x10,0x10};
    void *transBase = (void*)((u32)work->commIconPlt->pRawData + 32*(colArr[page]));
    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                        CG_HELP_PLT_MAIN_ICONBASE * 32 ,
                                        transBase , 32 );
    {
      u8 x,y;
      u16 *transBuf = GFL_HEAP_AllocClearMemory( work->heapId , 2*4*4 );
      for( y=0;y<4;y++ )
      {
        for( x=0;x<4;x++ )
        {
          transBuf[y*4+x] = posArr[page]+x + 0x20*y + (CG_HELP_PLT_MAIN_ICONBASE<<12);
        }
      }
      
      GFL_BG_WriteScreen( CG_HELP_FRAME_ICON , transBuf , 2 , 4 , 4 , 4 );
      GFL_HEAP_FreeMemory( transBuf );
    }
  }
  else
  {
    u16 *transBuf = GFL_HEAP_AllocClearMemory( work->heapId , 2*4*4 );
    GFL_BG_WriteScreen( CG_HELP_FRAME_ICON , transBuf , 2 , 4 , 4 , 4 );
    GFL_HEAP_FreeMemory( transBuf );
  }
  GFL_BG_LoadScreenReq(CG_HELP_FRAME_ICON);

  //BMP設定座標設定
  {
    GFL_CLACTPOS pos = {4*8,6*8};
    GFL_CLACT_WK_SetPos( work->clwkIcon , &pos , CLSYS_DRAW_SUB );
  }

  //特殊なセル設定
  if( page == CG_HELP_PAGE_CUSTOM )
  {
    //設定
    GFL_CLACT_WK_SetAutoAnmFlag( work->clwkIcon , FALSE );
    GFL_CLACT_WK_SetAnmIndex( work->clwkIcon , 1 );
  }
  else
  if( page == CG_HELP_PAGE_HELP )
  {
    //HELP
    GFL_CLACT_WK_SetAutoAnmFlag( work->clwkIcon , FALSE );
    GFL_CLACT_WK_SetAnmIndex( work->clwkIcon , 0 );
  }
  else
  {
    GFL_CLACT_WK_SetAutoAnmFlag( work->clwkIcon , TRUE );
  }
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
