//======================================================================
/**
 * @file	comm_tvt_sys
 * @brief	通信TVTシステム
 * @author	ariizumi
 * @data	09/12/16
 *
 * モジュール名：COMM_TVT
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/net_err.h"
#include "app/app_menu_common.h"

#include "arc_def.h"
#include "comm_tvt.naix"
#include "message.naix"
#include "font/font.naix"

#include "net_app/comm_tvt_sys.h"
#include "ctvt_camera.h"
#include "ctvt_comm.h"
#include "ctvt_talk.h"
#include "ctvt_draw.h"
#include "ctvt_call.h"
#include "comm_tvt_local_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _COMM_TVT_WORK
{
  HEAPID heapId;
  GFL_TCB *vBlankTcb;

  COMM_TVT_MODE mode;
  COMM_TVT_MODE nextMode;
  BOOL isUpperFade;
  BOOL isCommError;

  u8 connectNum;
  u8 selfIdx;
  BOOL isDouble;
  BOOL isSusspend;
  
  ARCHANDLE *arcHandle;
  //セル系
  u32         cellResIdx[CTOR_MAX];
  GFL_CLUNIT  *cellUnit;

  //メッセージ系
  GFL_FONT        *fontHandle;
  GFL_MSGDATA     *msgHandle;
  PRINT_QUE *printQue;

  //各ワーク
  CTVT_CAMERA_WORK *camWork;
  CTVT_COMM_WORK   *commWork;
  
  CTVT_TALK_WORK   *talkWork;
  CTVT_DRAW_WORK   *drawWork;
  CTVT_CALL_WORK   *callWork;

  DRAW_SYS_WORK   *drawSys;

  COMM_TVT_INIT_WORK *initWork;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void COMM_TVT_Init( COMM_TVT_WORK *work );
static void COMM_TVT_Term( COMM_TVT_WORK *work );
static const BOOL COMM_TVT_Main( COMM_TVT_WORK *work );
static void COMM_TVT_VBlankFunc(GFL_TCB *tcb, void *wk );

static void COMM_TVT_InitGraphic( COMM_TVT_WORK *work );
static void COMM_TVT_TermGraphic( COMM_TVT_WORK *work );
static void COMM_TVT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );

static void COMM_TVT_LoadResource( COMM_TVT_WORK *work );
static void COMM_TVT_ReleaseResource( COMM_TVT_WORK *work );
static void COMM_TVT_InitMessage( COMM_TVT_WORK *work );
static void COMM_TVT_TermMessage( COMM_TVT_WORK *work );

static void COMM_TVT_ChangeMode( COMM_TVT_WORK *work );

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_256_AB,             // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,       // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,         // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_16_F,            // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,      // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_128_D,        // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_NONE,             // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,         // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_128K
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
static void COMM_TVT_Init( COMM_TVT_WORK *work )
{
  work->connectNum = 1;
  work->selfIdx = 0;
  work->isDouble = FALSE;


  COMM_TVT_InitGraphic( work );
  COMM_TVT_LoadResource( work );
  COMM_TVT_InitMessage( work );
    
  work->camWork = CTVT_CAMERA_Init( work , work->heapId );
  work->commWork = CTVT_COMM_InitSystem( work , work->heapId );
  
  work->talkWork = CTVT_TALK_InitSystem( work , work->heapId );
  work->drawWork = CTVT_DRAW_InitSystem( work , work->heapId );
  work->callWork = CTVT_CALL_InitSystem( work , work->heapId );
  
  {
    DRAW_SYS_INIT_WORK initWork;
    initWork.heapId = work->heapId;
    initWork.frame = CTVT_FRAME_MAIN_DRAW;
    initWork.bufferNum = CTVT_DRAW_BUFFER_NUM;
    initWork.areaLeft = 0;
    initWork.areaRight = 256;
    initWork.areaTop = 0;
    initWork.areaBottom = 192;

    
    work->drawSys = DRAW_SYS_CreateSystem( &initWork );
  }
  
  work->vBlankTcb = GFUser_VIntr_CreateTCB( COMM_TVT_VBlankFunc , work , 8 );

  work->mode = CTM_NONE;

  work->isUpperFade = TRUE;
  work->isSusspend = FALSE;

  GFL_NET_WirelessIconEasy_HoldLCD( FALSE , work->heapId );
  GFL_NET_ReloadIcon();
  
  switch( work->initWork->mode )
  {
  case CTM_TEST:   //テスト用(changeOver
    CTVT_COMM_SetMode( work , work->commWork , CCIM_SCAN );
    work->nextMode = CTM_CALL;
    break;
  case CTM_PARENT: //親起動
    CTVT_COMM_SetMode( work , work->commWork , CCIM_SCAN );
    work->nextMode = CTM_CALL;
    break;
  case CTM_CHILD:  //子起動
    CTVT_COMM_SetMode( work , work->commWork , CCIM_CHILD );
    CTVT_COMM_SetMacAddress( work , work->commWork , work->initWork->macAddress );
    work->nextMode = CTM_TALK;
    break;
  case CTM_WIFI:   //Wifi起動
    CTVT_COMM_SetMode( work , work->commWork , CCIM_CONNECTED );
    work->nextMode = CTM_TALK;
    break;
  }


}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
static void COMM_TVT_Term( COMM_TVT_WORK *work )
{
  GFL_TCB_DeleteTask( work->vBlankTcb );
  
  DRAW_SYS_DeleteSystem( work->drawSys );
  CTVT_CALL_TermSystem( work , work->callWork );
  CTVT_DRAW_TermSystem( work , work->drawWork );
  CTVT_TALK_TermSystem( work , work->talkWork );

  CTVT_COMM_TermSystem( work , work->commWork );
  CTVT_CAMERA_Term( work , work->camWork );

  COMM_TVT_TermMessage( work );
  COMM_TVT_ReleaseResource( work );
  COMM_TVT_TermGraphic( work );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
static const BOOL COMM_TVT_Main( COMM_TVT_WORK *work )
{
  CTVT_COMM_Main( work , work->commWork );

  switch( work->mode )
  {
  case CTM_TALK: //会話
    work->nextMode = CTVT_TALK_Main( work , work->talkWork );
    break;
  case CTM_CALL: //呼び出し
    work->nextMode = CTVT_CALL_Main( work , work->callWork );
    break;
  case CTM_DRAW: //落書き
    work->nextMode = CTVT_DRAW_Main( work , work->drawWork );
    break;
  case CTM_END:
    if( CTVT_COMM_IsExit(work,work->commWork) == TRUE )
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
    break;
  }
  
  if( work->mode != work->nextMode )
  {
    COMM_TVT_ChangeMode( work );
  }
  
  CTVT_CAMERA_Main( work , work->camWork );
  
  DRAW_SYS_UpdateSystem( work->drawSys );

  //OBJの更新
  GFL_CLACT_SYS_Main();

  PRINTSYS_QUE_Main( work->printQue );

  return FALSE;
}
//--------------------------------------------------------------
//	VBlankTcb
//--------------------------------------------------------------
static void COMM_TVT_VBlankFunc(GFL_TCB *tcb, void *wk )
{
  COMM_TVT_WORK *work = wk;

  //上画面の更新
  CTVT_CAMERA_VBlank( work , work->camWork );
  //OBJの更新
  GFL_CLACT_SYS_VBlankFunc();
  //お絵描きの更新
  DRAW_SYS_UpdateSystemVBlank( work->drawSys );
}

//--------------------------------------------------------------
//	グラフィック初期化
//--------------------------------------------------------------
static void COMM_TVT_InitGraphic( COMM_TVT_WORK *work )
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
    static const GFL_BG_SYS_HEADER sys_data = {
        GX_DISPMODE_GRAPHICS, GX_BGMODE_5, GX_BGMODE_0, GX_BG0_AS_2D,
    };

    // BG0 MSG (文字
    static const GFL_BG_BGCNT_HEADER header_sub0 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x18000,0x08000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG1 BAR (バー
    static const GFL_BG_BGCNT_HEADER header_sub1 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x00000,0x05000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 SUB (その他(画面別BG
    static const GFL_BG_BGCNT_HEADER header_sub2 = {
      0, 0, 0x1000, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x10000,0x08000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x08000,0x08000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    //Main
    // BG0 MSG (文字
    static const GFL_BG_BGCNT_HEADER header_main0 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000,0x07000,
      GX_BG_EXTPLTT_23, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG1 BAR (バー
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x08000,0x08000,
      GX_BG_EXTPLTT_23, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    GFL_BG_SetBGMode( &sys_data );
    
    COMM_TVT_SetupBgFunc( &header_main0 , CTVT_FRAME_MAIN_MSG  , GFL_BG_MODE_TEXT );
    COMM_TVT_SetupBgFunc( &header_main1 , CTVT_FRAME_MAIN_BAR  , GFL_BG_MODE_TEXT );

    COMM_TVT_SetupBgFunc( &header_sub0 , CTVT_FRAME_SUB_MSG  , GFL_BG_MODE_TEXT );
    COMM_TVT_SetupBgFunc( &header_sub1 , CTVT_FRAME_SUB_BAR  , GFL_BG_MODE_TEXT );
    COMM_TVT_SetupBgFunc( &header_sub2 , CTVT_FRAME_SUB_MISC , GFL_BG_MODE_TEXT );
    COMM_TVT_SetupBgFunc( &header_sub3 , CTVT_FRAME_SUB_BG   , GFL_BG_MODE_TEXT );
    
    //ダイレクトBMPにGFLが対応してないのでべた書き

    G2_SetBG2ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x10000);
    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x28000);

    GFL_STD_MemClear( G2_GetBG2ScrPtr() , 256*192*sizeof(u16) );
    GFL_STD_MemClear( G2_GetBG3ScrPtr() , 256*192*sizeof(u16) );

    G2_SetBG2Priority(2);
    G2_SetBG3Priority(3);
    GFL_BG_SetVisible( CTVT_FRAME_MAIN_DRAW, VISIBLE_ON );
    GFL_BG_SetVisible( CTVT_FRAME_MAIN_CAMERA, VISIBLE_ON );
    
    GFL_BG_SetScroll( CTVT_FRAME_MAIN_BAR , GFL_BG_SCROLL_Y_SET , -24 );
  }
  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );

    work->cellUnit = GFL_CLACT_UNIT_Create( 64 , 0, work->heapId );
    GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit );
  }
}

//--------------------------------------------------------------
//	グラフィック開放
//--------------------------------------------------------------
static void COMM_TVT_TermGraphic( COMM_TVT_WORK *work )
{
  GFL_CLACT_UNIT_Delete( work->cellUnit );
  GFL_CLACT_SYS_Delete();

  GFL_BG_FreeBGControl( CTVT_FRAME_SUB_BG );
  GFL_BG_FreeBGControl( CTVT_FRAME_SUB_MISC );
  GFL_BG_FreeBGControl( CTVT_FRAME_SUB_BAR );
  GFL_BG_FreeBGControl( CTVT_FRAME_SUB_MSG );
  GFL_BG_FreeBGControl( CTVT_FRAME_MAIN_BAR );
  GFL_BG_FreeBGControl( CTVT_FRAME_MAIN_MSG );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void COMM_TVT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------------------
//  リソース読み込み
//--------------------------------------------------------------------------
static void COMM_TVT_LoadResource( COMM_TVT_WORK *work )
{
  work->arcHandle = GFL_ARC_OpenDataHandle( ARCID_COMM_TVT_GRA , work->heapId );
  //下画面
  GFL_ARCHDL_UTIL_TransVramPalette( work->arcHandle , NARC_comm_tvt_tv_t_tuuwa_bg_NCLR , 
                    PALTYPE_SUB_BG , CTVT_PAL_BG_SUB_BG*32 , 32*5 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( work->arcHandle , NARC_comm_tvt_tv_t_common_bg_NCGR ,
                    CTVT_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( work->arcHandle , NARC_comm_tvt_tv_t_common_bg_NSCR , 
                    CTVT_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );

  //下画面 BGMisc(キャラは共通なの先読み
  GFL_ARCHDL_UTIL_TransVramBgCharacter( work->arcHandle , NARC_comm_tvt_tv_t_tuuwa_bg_NCGR ,
                    CTVT_FRAME_SUB_MISC , 0 , 0, FALSE , work->heapId );

  //OBJ
  work->cellResIdx[CTOR_COMMON_M_PLT] = GFL_CLGRP_PLTT_RegisterComp( work->arcHandle , 
        NARC_comm_tvt_tv_t_obj_NCLR , CLSYS_DRAW_MAIN , 
        CTVT_PAL_OBJ_MAIN_COMMON*32 , work->heapId  );
  work->cellResIdx[CTOR_COMMON_M_NCG] = GFL_CLGRP_CGR_Register( work->arcHandle , 
        NARC_comm_tvt_tv_t_upper_obj_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  work->cellResIdx[CTOR_COMMON_M_ANM] = GFL_CLGRP_CELLANIM_Register( work->arcHandle , 
        NARC_comm_tvt_tv_t_upper_obj_NCER , NARC_comm_tvt_tv_t_upper_obj_NANR, work->heapId  );

  work->cellResIdx[CTOR_COMMON_S_PLT] = GFL_CLGRP_PLTT_RegisterComp( work->arcHandle , 
        NARC_comm_tvt_tv_t_obj_NCLR , CLSYS_DRAW_SUB , 
        CTVT_PAL_OBJ_MAIN_COMMON*32 , work->heapId  );
  work->cellResIdx[CTOR_COMMON_S_NCG] = GFL_CLGRP_CGR_Register( work->arcHandle , 
        NARC_comm_tvt_tv_t_lower_obj_NCGR , FALSE , CLSYS_DRAW_SUB , work->heapId  );
  work->cellResIdx[CTOR_COMMON_S_ANM] = GFL_CLGRP_CELLANIM_Register( work->arcHandle , 
        NARC_comm_tvt_tv_t_lower_obj_NCER , NARC_comm_tvt_tv_t_lower_obj_NANR, work->heapId  );

  //共通素材
  {
    ARCHANDLE *commonArcHandle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId() , work->heapId );
    
    //BG
    GFL_ARCHDL_UTIL_TransVramPalette( commonArcHandle , APP_COMMON_GetBarPltArcIdx() , 
                      PALTYPE_MAIN_BG , CTVT_PAL_BG_MAIN_BAR*32 , 32 , work->heapId );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( commonArcHandle , APP_COMMON_GetBarCharArcIdx() ,
                      CTVT_FRAME_MAIN_BAR , 0 , 0, FALSE , work->heapId );
    GFL_ARCHDL_UTIL_TransVramScreen( commonArcHandle , APP_COMMON_GetBarScrnArcIdx() , 
                      CTVT_FRAME_MAIN_BAR ,  0 , 0, FALSE , work->heapId );
    GFL_BG_ChangeScreenPalette( CTVT_FRAME_MAIN_BAR , 0 , 21 , 32 , 3 , CTVT_PAL_BG_MAIN_BAR );
    GFL_BG_LoadScreenReq( CTVT_FRAME_MAIN_BAR );

    GFL_ARCHDL_UTIL_TransVramPalette( commonArcHandle , APP_COMMON_GetBarPltArcIdx() , 
                      PALTYPE_SUB_BG , CTVT_PAL_BG_SUB_BAR*32 , 32 , work->heapId );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( commonArcHandle , APP_COMMON_GetBarCharArcIdx() ,
                      CTVT_FRAME_SUB_BAR , 0 , 0, FALSE , work->heapId );
    GFL_ARCHDL_UTIL_TransVramScreen( commonArcHandle , APP_COMMON_GetBarScrnArcIdx() , 
                      CTVT_FRAME_SUB_BAR ,  0 , 0, FALSE , work->heapId );
    GFL_BG_ChangeScreenPalette( CTVT_FRAME_SUB_BAR , 0 , 21 , 32 , 3 , CTVT_PAL_BG_SUB_BAR );
    GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_BAR );
    
    //OBJ
    work->cellResIdx[CTOR_BAR_BUTTON_M_PLT] = GFL_CLGRP_PLTT_RegisterEx( commonArcHandle , 
          APP_COMMON_GetBarIconPltArcIdx() , CLSYS_DRAW_MAIN , 
          CTVT_PAL_OBJ_SUB_BARICON*32 , 0 , APP_COMMON_BARICON_PLT_NUM , work->heapId  );
    work->cellResIdx[CTOR_BAR_BUTTON_M_NCG] = GFL_CLGRP_CGR_Register( commonArcHandle , 
          APP_COMMON_GetBarIconCharArcIdx() , FALSE , CLSYS_DRAW_MAIN , work->heapId  );

    work->cellResIdx[CTOR_BAR_BUTTON_S_PLT] = GFL_CLGRP_PLTT_RegisterEx( commonArcHandle , 
          APP_COMMON_GetBarIconPltArcIdx() , CLSYS_DRAW_SUB , 
          CTVT_PAL_OBJ_SUB_BARICON*32 , 0 , APP_COMMON_BARICON_PLT_NUM , work->heapId  );
    work->cellResIdx[CTOR_BAR_BUTTON_S_NCG] = GFL_CLGRP_CGR_Register( commonArcHandle , 
          APP_COMMON_GetBarIconCharArcIdx() , FALSE , CLSYS_DRAW_SUB , work->heapId  );
    work->cellResIdx[CTOR_BAR_BUTTON_ANM] = GFL_CLGRP_CELLANIM_Register( commonArcHandle , 
          APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_128K) , 
          APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_128K), work->heapId  );
    
    GFL_ARC_CloseDataHandle( commonArcHandle );
  }

  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_BG );
}

//--------------------------------------------------------------------------
//  リソース開放
//--------------------------------------------------------------------------
static void COMM_TVT_ReleaseResource( COMM_TVT_WORK *work )
{
  u8 i;
  for( i=CTOR_PLT_TOP;i<CTOR_NCG_TOP;i++ )
  {
    GFL_CLGRP_PLTT_Release( work->cellResIdx[i] );
  }
  for( i=CTOR_NCG_TOP;i<CTOR_ANM_TOP;i++ )
  {
    GFL_CLGRP_CGR_Release( work->cellResIdx[i] );
  }
  for( i=CTOR_ANM_TOP;i<CTOR_MAX;i++ )
  {
    GFL_CLGRP_CELLANIM_Release( work->cellResIdx[i] );
  }
  GFL_ARC_CloseDataHandle( work->arcHandle );

}

//--------------------------------------------------------------------------
//  メッセージ系初期化
//--------------------------------------------------------------------------
static void COMM_TVT_InitMessage( COMM_TVT_WORK *work )
{
  //フォント読み込み
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  //メッセージ
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_comm_tvt_dat , work->heapId );
  work->printQue = PRINTSYS_QUE_Create( work->heapId );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , CTVT_PAL_BG_MAIN_FONT*0x20, 16*2, work->heapId );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG  , CTVT_PAL_BG_SUB_FONT *0x20, 16*2, work->heapId );
  GFL_FONTSYS_SetDefaultColor();
}

//--------------------------------------------------------------------------
//  メッセージ系開放
//--------------------------------------------------------------------------
static void COMM_TVT_TermMessage( COMM_TVT_WORK *work )
{
  PRINTSYS_QUE_Delete( work->printQue );
  GFL_MSG_Delete( work->msgHandle );
  GFL_FONT_Delete( work->fontHandle );
}

//--------------------------------------------------------------------------
//  モードの変更
//--------------------------------------------------------------------------
static void COMM_TVT_ChangeMode( COMM_TVT_WORK *work )
{
  //開放
  switch( work->mode )
  {
  case CTM_TALK: //会話
    CTVT_TALK_TermMode( work , work->talkWork );
    break;
  case CTM_CALL: //呼び出し
    CTVT_CALL_TermMode( work , work->callWork );
    break;
  case CTM_DRAW: //落書き
    CTVT_DRAW_TermMode( work , work->drawWork );
    break;
  }

  //初期化
  switch( work->nextMode )
  {
  case CTM_TALK: //会話
    CTVT_TALK_InitMode( work , work->talkWork );
    break;
  case CTM_CALL: //呼び出し
    CTVT_CALL_InitMode( work , work->callWork );
    break;
  case CTM_DRAW: //落書き
    CTVT_DRAW_InitMode( work , work->drawWork );
    break;
  case CTM_END: //終了
    CTVT_COMM_ExitComm( work , work->commWork );
    break;
  }
  
  work->mode = work->nextMode;
}

#pragma mark [>outer func (system
CTVT_CAMERA_WORK* COMM_TVT_GetCameraWork( COMM_TVT_WORK *work )
{
  return work->camWork;
}
CTVT_COMM_WORK* COMM_TVT_GetCommWork( COMM_TVT_WORK *work )
{
  return work->commWork;
}
CTVT_TALK_WORK* COMM_TVT_GetTalkWork( COMM_TVT_WORK *work )
{
  return work->talkWork;
}
CTVT_MIC_WORK* COMM_TVT_GetMicWork( COMM_TVT_WORK *work )
{
  return CTVT_TALK_GetMicWork( work , work->talkWork );
}
DRAW_SYS_WORK* COMM_TVT_GetDrawSys( COMM_TVT_WORK *work )
{
  return work->drawSys;
}
const COMM_TVT_INIT_WORK* COMM_TVT_GetInitWork( const COMM_TVT_WORK *work )
{
  return work->initWork;
}

#pragma mark [>outer func (value
//--------------------------------------------------------------------------
//  システムっぽいもの
//--------------------------------------------------------------------------
const HEAPID COMM_TVT_GetHeapId( const COMM_TVT_WORK *work )
{
  return work->heapId;
}
ARCHANDLE* COMM_TVT_GetArcHandle( const COMM_TVT_WORK *work )
{
  return work->arcHandle;
}
const u32 COMM_TVT_GetObjResIdx( const COMM_TVT_WORK *work , const COMM_TVT_OBJ_RES resIdx )
{
  return work->cellResIdx[resIdx];
}
GFL_CLUNIT* COMM_TVT_GetCellUnit( const COMM_TVT_WORK *work )
{
  return work->cellUnit;
}
const BOOL COMM_TVT_GetUpperFade( const COMM_TVT_WORK *work )
{
  return work->isUpperFade;
}
void COMM_TVT_SetUpperFade( COMM_TVT_WORK *work , const BOOL flg )
{
  work->isUpperFade = flg;
}

GFL_FONT* COMM_TVT_GetFontHandle( COMM_TVT_WORK *work )
{
  return work->fontHandle;
}

GFL_MSGDATA* COMM_TVT_GetMegHandle( COMM_TVT_WORK *work )
{
  return work->msgHandle;
}

PRINT_QUE* COMM_TVT_GetPrintQue( COMM_TVT_WORK *work )
{
  return work->printQue;
}


//--------------------------------------------------------------------------
//  モードとか人数
//--------------------------------------------------------------------------
const u8   COMM_TVT_GetConnectNum( const COMM_TVT_WORK *work )
{
  return work->connectNum;
}
void   COMM_TVT_SetConnectNum( COMM_TVT_WORK *work , const u8 num)
{
  work->connectNum = num;
}

const COMM_TVT_DISP_MODE COMM_TVT_GetDispMode( const COMM_TVT_WORK *work )
{
  const u8 num = COMM_TVT_GetConnectNum( work );
  if( num == 1 )
  {
    return CTDM_SINGLE;
  }
  else
  if( num == 2 )
  {
    return CTDM_DOUBLE;
  }
  return CTDM_FOUR;
}

const BOOL COMM_TVT_IsDoubleMode( const COMM_TVT_WORK *work )
{
  return work->isDouble;
}
void   COMM_TVT_SetDoubleMode( COMM_TVT_WORK *work , const BOOL flg )
{
  work->isDouble = flg;
}
const u8   COMM_TVT_GetSelfIdx( const COMM_TVT_WORK *work )
{
  return work->selfIdx;
}
void COMM_TVT_SetSelfIdx( COMM_TVT_WORK *work , const u8 idx )
{
  work->selfIdx = idx;
}
//--------------------------------------------------------------
//	一時停止(システム側
//--------------------------------------------------------------
const BOOL COMM_TVT_GetSusspend( COMM_TVT_WORK *work )
{
  return work->isSusspend;
}

void COMM_TVT_SetSusspend( COMM_TVT_WORK *work , const BOOL flg )
{
  work->isSusspend = flg;
}

#pragma mark [>util func
//--------------------------------------------------------------------------
//  TWL起動かチェック
//--------------------------------------------------------------------------
const BOOL COMM_TVT_IsTwlMode( void )
{
  return OS_IsRunOnTwl();
}

#pragma mark [>proc func
static GFL_PROC_RESULT COMM_TVT_Proc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT COMM_TVT_Proc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT COMM_TVT_Proc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA COMM_TVT_ProcData =
{
  COMM_TVT_Proc_Init,
  COMM_TVT_Proc_Main,
  COMM_TVT_Proc_Term
};

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT COMM_TVT_Proc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  COMM_TVT_WORK *work;
  
  CTVT_TPrintf("Least heap[%x]\n",GFL_HEAP_GetHeapFreeSize(GFL_HEAPID_APP));
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_COMM_TVT, 0x100000 );
  
  work = GFL_PROC_AllocWork( proc, sizeof(COMM_TVT_WORK), HEAPID_COMM_TVT );
  work->heapId = HEAPID_COMM_TVT;
  if( pwk == NULL )
  {
    work->initWork = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP , sizeof(COMM_TVT_INIT_WORK) );
    work->initWork->gameData = GAMEDATA_Create( GFL_HEAPID_APP );
    work->initWork->mode = CTM_TEST;

    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
    {
      work->initWork->mode = CTM_PARENT;
    }
  }
  else
  {
    work->initWork = pwk;
  }
  work->isCommError = FALSE;
  
  COMM_TVT_Init( work );
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
static GFL_PROC_RESULT COMM_TVT_Proc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  COMM_TVT_WORK *work = mywk;
  
  if( CTVT_COMM_IsExit(work,work->commWork) == FALSE )
  {
    return GFL_PROC_RES_CONTINUE;
  }

  COMM_TVT_Term( work );

  if( pwk == NULL )
  {
    GFL_HEAP_FreeMemory( work->initWork );
  }

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_COMM_TVT );

  CTVT_TPrintf("FinishCamera\n");

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
static GFL_PROC_RESULT COMM_TVT_Proc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  COMM_TVT_WORK *work = mywk;

  if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE &&
      work->isCommError == FALSE )
  {
    NetErr_App_ReqErrorDisp();
    work->isCommError = TRUE;
  }

  if( COMM_TVT_Main( work ) == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}


