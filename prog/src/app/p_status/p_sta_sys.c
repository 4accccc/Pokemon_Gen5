//======================================================================
/**
 * @file	plist_sys.c
 * @brief	ポケモンリスト メイン処理
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：PSTATUS
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "poke_tool/poke_tool.h"

#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

#include "p_sta_sys.h"
#include "p_sta_sub.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define PSTATUS_SCROLL_SPD (4) //何フレームに1回スクロールするか？

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_48_HI,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_128_B,       // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_01_CD,        // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_0_F,     // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_128K
};


static void PSTATUS_VBlankFunc(GFL_TCB *tcb, void *wk );

static void PSTATUS_InitGraphic( PSTATUS_WORK *work );
static void PSTATUS_TermGraphic( PSTATUS_WORK *work );
static void PSTATUS_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );

static void PSTATUS_LoadResource( PSTATUS_WORK *work );
static void PSTATUS_ReleaseResource( PSTATUS_WORK *work );
static void PSTATUS_InitMessage( PSTATUS_WORK *work );
static void PSTATUS_TermMessage( PSTATUS_WORK *work );

static void PSTATUS_RefreshDisp( PSTATUS_WORK *work );
const POKEMON_PASO_PARAM* PSTATUS_UTIL_GetCurrentPPP(  PSTATUS_WORK *work );


//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
const BOOL PSTATUS_InitPokeStatus( PSTATUS_WORK *work )
{
  work->dataPos = work->psData->pos;
  work->scrollCnt = 0;
  work->befVCount = OS_GetVBlankCount();
#if PM_DEBUG
  work->debPp = NULL;
#endif

  PSTATUS_InitGraphic( work );
  PSTATUS_LoadResource( work );
  PSTATUS_InitMessage( work );

  work->subWork = PSTATUS_SUB_Init( work );

  work->vBlankTcb = GFUser_VIntr_CreateTCB( PSTATUS_VBlankFunc , work , 8 );

  //フェードないので仮処理
  GX_SetMasterBrightness(0);  
  GXS_SetMasterBrightness(0);
  
  PSTATUS_SUB_DispPage( work , work->subWork );
  
  return TRUE;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
const BOOL PSTATUS_TermPokeStatus( PSTATUS_WORK *work )
{
  GFL_TCB_DeleteTask( work->vBlankTcb );

  PSTATUS_SUB_Term( work , work->subWork );

  PSTATUS_TermMessage( work );
  PSTATUS_ReleaseResource( work );
  PSTATUS_TermGraphic( work );

#if PM_DEBUG
  if( work->debPp != NULL )
  {
    GFL_HEAP_FreeMemory( work->debPp );
  }
#endif
  return TRUE;
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
const BOOL PSTATUS_UpdatePokeStatus( PSTATUS_WORK *work )
{
  PSTATUS_SUB_Main( work , work->subWork );

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
  {
    if( work->dataPos < work->psData->max-1 )
    {
      work->dataPos++;
      PSTATUS_RefreshDisp( work );
    }
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
  {
    if( work->dataPos != 0 )
    {
      work->dataPos--;
      PSTATUS_RefreshDisp( work );
    }
  }


  //3D描画  
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    PSTATUS_SUB_Draw( work , work->subWork );
  }
  GFL_G3D_DRAW_End();

  //背景スクロール
  {
    const u32 vCount = OS_GetVBlankCount();
    work->scrollCnt += vCount - work->befVCount;
    work->befVCount = vCount;
    
    if( work->scrollCnt >= PSTATUS_SCROLL_SPD )
    {
      GFL_BG_SetScrollReq( PSTATUS_BG_MAIN_BG , GFL_BG_SCROLL_X_INC , 1 );
      GFL_BG_SetScrollReq( PSTATUS_BG_MAIN_BG , GFL_BG_SCROLL_Y_INC , 1 );
      GFL_BG_SetScrollReq( PSTATUS_BG_SUB_BG , GFL_BG_SCROLL_X_INC , 1 );
      GFL_BG_SetScrollReq( PSTATUS_BG_SUB_BG , GFL_BG_SCROLL_Y_INC , 1 );
      work->scrollCnt -= PSTATUS_SCROLL_SPD;
    }
  }


  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//	VBlank
//--------------------------------------------------------------
static void PSTATUS_VBlankFunc(GFL_TCB *tcb, void *wk )
{
  PSTATUS_WORK *work = (PSTATUS_WORK*)wk;
  
  //OBJの更新
  GFL_CLACT_SYS_VBlankFunc();

}


#pragma mark [>graphic
//--------------------------------------------------------------
//グラフィック系初期化
//--------------------------------------------------------------
static void PSTATUS_InitGraphic( PSTATUS_WORK *work )
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
        GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    // BG1 MAIN (パラメータ
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000,0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 MAIN (土台
    static const GFL_BG_BGCNT_HEADER header_main2 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x10000,0x8000,
      GX_BG_EXTPLTT_01, 2, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 MAIN (背景
    static const GFL_BG_BGCNT_HEADER header_main3 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000,0x06000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    // BG3 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000,0x06000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    GFL_BG_SetBGMode( &sys_data );

    PSTATUS_SetupBgFunc( &header_main3 , PSTATUS_BG_MAIN_BG , GFL_BG_MODE_TEXT );
    PSTATUS_SetupBgFunc( &header_main2 , PSTATUS_BG_PLATE , GFL_BG_MODE_TEXT );
    PSTATUS_SetupBgFunc( &header_main1 , PSTATUS_BG_PARAM , GFL_BG_MODE_TEXT );
    
    GFL_BG_SetBGControl3D( 0 );
    GFL_BG_SetVisible( PSTATUS_BG_3D , TRUE );
    
    PSTATUS_SetupBgFunc( &header_sub3 , PSTATUS_BG_SUB_BG , GFL_BG_MODE_TEXT );
  }
  
  //WindowMask設定
  {
    G2_SetWnd0Position( 0 , 0 , PSTATUS_MAIN_PAGE_WIDTH*8 , 192 );
    G2_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ , TRUE );
    G2_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ , FALSE );
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2 , GX_BLEND_PLANEMASK_BG3 , 12 , 16 );
    GX_SetVisibleWnd( GX_WNDMASK_W0 );
  }
  
  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
  }
  
  //3D系の初期化
  { //3D系の設定
    static const VecFx32 cam_pos = {FX32_CONST(0.0f),FX32_CONST(0.0f),FX32_CONST(101.0f)};
    static const VecFx32 cam_target = {FX32_CONST(0.0f),FX32_CONST(0.0f),FX32_CONST(-100.0f)};
    static const VecFx32 cam_up = {0,FX32_ONE,0};
    //エッジマーキングカラー
    static  const GXRgb edge_color_table[8]=
      { GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K,
            0, work->heapId, NULL );
 
    //正射影カメラ
    work->camera =  GFL_G3D_CAMERA_Create( GFL_G3D_PRJORTH, 
                       FX32_ONE*12.0f,
                       0,
                       0,
                       FX32_ONE*16.0f,
                       (FX32_ONE),
                       (FX32_ONE*200),
                       NULL,
                       &cam_pos,
                       &cam_up,
                       &cam_target,
                       work->heapId );
    
    GFL_G3D_CAMERA_Switching( work->camera );
    //エッジマーキングカラーセット
    G3X_SetEdgeColorTable( edge_color_table );
    G3X_EdgeMarking( TRUE );
    
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO , GX_BUFFERMODE_Z );
  }
  //Vram転送アニメ
//  NNS_GfdClearVramTransferManagerTask();
}


//--------------------------------------------------------------
//グラフィック系開放
//--------------------------------------------------------------
static void PSTATUS_TermGraphic( PSTATUS_WORK *work )
{
  NNS_GfdClearVramTransferManagerTask();
  
  GFL_G3D_CAMERA_Delete( work->camera );
  GFL_G3D_Exit();

  GFL_CLACT_SYS_Delete();

  GFL_BG_FreeBGControl( PSTATUS_BG_MAIN_BG );
  GFL_BG_FreeBGControl( PSTATUS_BG_PLATE );
  GFL_BG_FreeBGControl( PSTATUS_BG_PARAM );
  GFL_BG_FreeBGControl( PSTATUS_BG_3D );
  GFL_BG_FreeBGControl( PSTATUS_BG_SUB_BG );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();

}

//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void PSTATUS_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

#pragma mark [>Resource
//--------------------------------------------------------------------------
//  リソース読み込み
//--------------------------------------------------------------------------
static void PSTATUS_LoadResource( PSTATUS_WORK *work )
{
  ARCHANDLE *archandle = GFL_ARC_OpenDataHandle( ARCID_P_STATUS , work->heapId );

  //下画面共通パレット
  GFL_ARCHDL_UTIL_TransVramPalette( archandle , NARC_p_status_gra_p_status_d_NCLR , 
                    PALTYPE_MAIN_BG , 0 , 0 , work->heapId );
  //上画面共通パレット
  GFL_ARCHDL_UTIL_TransVramPalette( archandle , NARC_p_status_gra_p_status_d_NCLR , 
                    PALTYPE_SUB_BG , 0 , 0 , work->heapId );
  
  //下画面背景
  GFL_ARCHDL_UTIL_TransVramBgCharacter( archandle , NARC_p_status_gra_p_status_bg_NCGR ,
                    PSTATUS_BG_MAIN_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( archandle , NARC_p_status_gra_p_status_bg_NSCR , 
                    PSTATUS_BG_MAIN_BG ,  0 , 0, FALSE , work->heapId );

  //下画面土台共通キャラ
  GFL_ARCHDL_UTIL_TransVramBgCharacter( archandle , NARC_p_status_gra_p_status_d_NCGR ,
                    PSTATUS_BG_PLATE , 0 , 0, FALSE , work->heapId );

  //上画面背景
  GFL_ARCHDL_UTIL_TransVramBgCharacter( archandle , NARC_p_status_gra_p_status_bg_NCGR ,
                    PSTATUS_BG_SUB_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( archandle , NARC_p_status_gra_p_status_bg_NSCR , 
                    PSTATUS_BG_SUB_BG ,  0 , 0, FALSE , work->heapId );

  PSTATUS_SUB_LoadResource( work , work->subWork , archandle );

  //info部分仮処理
  {
    NNSG2dScreenData *scrData;
    void *scrRes = GFL_ARCHDL_UTIL_LoadScreen( archandle , NARC_p_status_gra_p_status_info_NSCR ,
                      FALSE , &scrData , work->heapId );
    GFL_BG_WriteScreenExpand( PSTATUS_BG_PLATE , 
                      0 , 0 , PSTATUS_MAIN_PAGE_WIDTH , 24 ,
                      scrData->rawData ,
                      0 , 0 , 32 , 32 );
    
    GFL_HEAP_FreeMemory( scrRes );
    GFL_BG_LoadScreenV_Req( PSTATUS_BG_PLATE );
  }

  GFL_ARC_CloseDataHandle(archandle);
}

//--------------------------------------------------------------------------
//  リソース開放
//--------------------------------------------------------------------------
static void PSTATUS_ReleaseResource( PSTATUS_WORK *work )
{

  PSTATUS_SUB_ReleaseResource( work , work->subWork );
  
}

//--------------------------------------------------------------------------
//  メッセージ系初期化
//--------------------------------------------------------------------------
static void PSTATUS_InitMessage( PSTATUS_WORK *work )
{
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  
  //メッセージ
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_pokestatus_dat , work->heapId );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , PSTATUS_BG_PLT_FONT*16*2, 16*2, work->heapId );
  
  work->printQue = PRINTSYS_QUE_Create( work->heapId );
}

//--------------------------------------------------------------------------
//  メッセージ系開放
//--------------------------------------------------------------------------
static void PSTATUS_TermMessage( PSTATUS_WORK *work )
{
  PRINTSYS_QUE_Delete( work->printQue );
  GFL_MSG_Delete( work->msgHandle );
  GFL_FONT_Delete( work->fontHandle );
}

#pragma mark [>util

//--------------------------------------------------------------
//キャラが変わったので表示更新
//--------------------------------------------------------------
static void PSTATUS_RefreshDisp( PSTATUS_WORK *work )
{
  PSTATUS_SUB_ClearPage( work , work->subWork );
  PSTATUS_SUB_DispPage( work , work->subWork );
}

//--------------------------------------------------------------
//現在のPPPを取得
//--------------------------------------------------------------
const POKEMON_PASO_PARAM* PSTATUS_UTIL_GetCurrentPPP( PSTATUS_WORK *work )
{
  switch( work->psData->ppt )
  {
  case PST_PP_TYPE_POKEPARAM:

    break;

  case PST_PP_TYPE_POKEPARTY:
    {
      const POKEMON_PARAM *pp = PokeParty_GetMemberPointer( (POKEPARTY*)work->psData->ppd , work->dataPos );
      return PP_GetPPPPointerConst( pp );
    }
    break;

  case PST_PP_TYPE_POKEPASO:

    break;

#if PM_DEBUG
  case PST_PP_TYPE_DEBUG:
    if( work->debPp != NULL &&
        PP_Get( work->debPp , ID_PARA_monsno , NULL ) != work->dataPos+1 )
    {
      GFL_HEAP_FreeMemory( work->debPp );
      work->debPp = NULL;
    }
    if( work->debPp == NULL )
    {
      work->debPp = PP_Create( work->dataPos+1 , 50 , PTL_SETUP_POW_AUTO , HEAPID_POKE_STATUS );
    }
    return PP_GetPPPPointerConst( work->debPp );
    break;
#endif


  }
  return NULL;
}
