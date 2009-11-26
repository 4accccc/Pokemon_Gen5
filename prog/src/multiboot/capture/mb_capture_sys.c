//======================================================================
/**
 * @file	mb_capture_sys.h
 * @brief	マルチブート・捕獲ゲーム
 * @author	ariizumi
 * @data	09/11/24
 *
 * モジュール名：MB_CAPTURE
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "sound/pm_sndsys.h"
#include "system/wipe.h"

#include "arc_def.h"
#include "mb_child_gra.naix"

#include "multiboot/mb_util_msg.h"
#include "multiboot/mb_poke_icon.h"
#include "multiboot/mb_local_def.h"
#include "multiboot/mb_capture_sys.h"
#include "./mb_cap_obj.h"
#include "./mb_cap_poke.h"
#include "./mb_cap_local_def.h"


//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define MB_CAPTURE_FRAME_MSG (GFL_BG_FRAME1_M)
#define MB_CAPTURE_FRAME_BG  (GFL_BG_FRAME3_M)

#define MB_CAPTURE_FRAME_SUB_BG  (GFL_BG_FRAME3_S)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  MSS_FADEIN,
  MSS_WAIT_FADEIN,
  MSS_FADEOUT,
  MSS_WAIT_FADEOUT,
  
}MB_CAPTURE_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  HEAPID heapId;
  GFL_TCB *vBlankTcb;
  MB_CAPTURE_INIT_WORK *initWork;
  
  MB_CAPTURE_STATE  state;
  
  GFL_G3D_CAMERA    *camera;
  GFL_BBD_SYS     *bbdSys;
  
  MB_CAP_POKE *pokeObj[MB_CAP_POKE_NUM];
  MB_CAP_OBJ  *fieldObj[MB_CAP_OBJ_NUM];
  
}MB_CAPTURE_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void MB_CAPTURE_Init( MB_CAPTURE_WORK *work );
static void MB_CAPTURE_Term( MB_CAPTURE_WORK *work );
static const BOOL MB_CAPTURE_Main( MB_CAPTURE_WORK *work );
static void MB_CAPTURE_VBlankFunc(GFL_TCB *tcb, void *wk );

static void MB_CAPTURE_InitGraphic( MB_CAPTURE_WORK *work );
static void MB_CAPTURE_TermGraphic( MB_CAPTURE_WORK *work );
static void MB_CAPTURE_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void MB_CAPTURE_LoadResource( MB_CAPTURE_WORK *work );

static void MB_CAPTURE_InitObject( MB_CAPTURE_WORK *work );
static void MB_CAPTURE_TermObject( MB_CAPTURE_WORK *work );
static void MB_CAPTURE_InitPoke( MB_CAPTURE_WORK *work );
static void MB_CAPTURE_TermPoke( MB_CAPTURE_WORK *work );

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_A,             // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,       // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,         // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_NONE ,           // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,      // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_128_D,        // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_0_B,             // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_01_FG,         // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_128K
};


//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
static void MB_CAPTURE_Init( MB_CAPTURE_WORK *work )
{
  work->state = MSS_FADEIN;
  
  MB_CAPTURE_InitGraphic( work );
  MB_CAPTURE_LoadResource( work );
  work->vBlankTcb = GFUser_VIntr_CreateTCB( MB_CAPTURE_VBlankFunc , work , 8 );

  MB_CAPTURE_InitObject( work );
  MB_CAPTURE_InitPoke( work );
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static void MB_CAPTURE_Term( MB_CAPTURE_WORK *work )
{
  MB_CAPTURE_TermObject( work );
  MB_CAPTURE_TermPoke( work );

  GFL_TCB_DeleteTask( work->vBlankTcb );

  MB_CAPTURE_TermGraphic( work );
}

//--------------------------------------------------------------
//  更新
//--------------------------------------------------------------
static const BOOL MB_CAPTURE_Main( MB_CAPTURE_WORK *work )
{
  switch( work->state )
  {
  case MSS_FADEIN:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );

    work->state = MSS_WAIT_FADEIN;
    break;
    
  case MSS_WAIT_FADEIN:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        
        work->state = MSS_FADEOUT;
      }
    }
    break;

  case MSS_FADEOUT:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = MSS_WAIT_FADEOUT;
    break;
    
  case MSS_WAIT_FADEOUT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return TRUE;
    }
    break;
  }

  //3D描画  
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    GFL_BBD_Draw( work->bbdSys , work->camera , NULL );
  }
  GFL_G3D_DRAW_End();

  //OBJの更新
  GFL_CLACT_SYS_Main();

  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START &&
      GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    return TRUE;
  }
  return FALSE;
}


//--------------------------------------------------------------
//	VBlankTcb
//--------------------------------------------------------------
static void MB_CAPTURE_VBlankFunc(GFL_TCB *tcb, void *wk )
{
  //OBJの更新
  GFL_CLACT_SYS_VBlankFunc();
}

//--------------------------------------------------------------
//  グラフィック系初期化
//--------------------------------------------------------------
static void MB_CAPTURE_InitGraphic( MB_CAPTURE_WORK *work )
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
        GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    // BG1 MAIN (メッセージ
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000,0x6000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 MAIN (キャラ
    //static const GFL_BG_BGCNT_HEADER header_main2 = {
    //  0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
    //  GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    //  GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x10000,0x0C000,
    //  GX_BG_EXTPLTT_23, 2, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    //};
    // BG3 MAIN (背景
    static const GFL_BG_BGCNT_HEADER header_main3 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x18000,0x08000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    // BG3 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x18000,0x08000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    GFL_BG_SetBGMode( &sys_data );

    MB_CAPTURE_SetupBgFunc( &header_main1 , MB_CAPTURE_FRAME_MSG , GFL_BG_MODE_TEXT );
    //MB_CAPTURE_SetupBgFunc( &header_main2 , LTVT_FRAME_CHARA , GFL_BG_MODE_TEXT );
    MB_CAPTURE_SetupBgFunc( &header_main3 , MB_CAPTURE_FRAME_BG , GFL_BG_MODE_TEXT );
    MB_CAPTURE_SetupBgFunc( &header_sub3  , MB_CAPTURE_FRAME_SUB_BG , GFL_BG_MODE_TEXT );
    
  }

  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
  }
  
  //3Dの初期化
  
  { //3D系の設定
    static const VecFx32 cam_pos = {0,0,FX32_CONST(300.0f)};
    static const VecFx32 cam_target = {0,0,0};
    static const VecFx32 cam_up = {0,FX32_ONE,0};
    //エッジマーキングカラー
    static  const GXRgb stage_edge_color_table[8]=
      { GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT80K,
            0, work->heapId, NULL );
    GFL_BG_SetBGControl3D( 2 ); //NNS_g3dInitの中で表示優先順位変わる・・・
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
    //正射影カメラ
    work->camera =  GFL_G3D_CAMERA_Create( GFL_G3D_PRJORTH, 
                       0,
                       FX32_ONE*192.0f,
                       0,
                       FX32_ONE*256.0f,
                       FX32_ONE,
                       FX32_ONE*350,
                       0,
                       &cam_pos,
                       &cam_up,
                       &cam_target,
                       work->heapId );
    
    GFL_G3D_CAMERA_Switching( work->camera );
    //エッジマーキングカラーセット
    G3X_SetEdgeColorTable( &stage_edge_color_table[0] );
    G3X_EdgeMarking( TRUE );
    G3X_AntiAlias( TRUE );
    G3X_AlphaBlend( TRUE );
    
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO , GX_BUFFERMODE_Z );
  }
  {
    VecFx32 bbdScale = {FX32_CONST(16.0f),FX32_CONST(16.0f),FX32_CONST(16.0f)};
    GFL_BBD_SETUP bbdSetup = {
      128,128,
      {FX32_ONE,FX32_ONE,FX32_ONE},
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      GX_RGB(0,0,0),
      0,
      GFL_BBD_ORIGIN_CENTER,
    };
    //ビルボードシステム構築
    work->bbdSys = GFL_BBD_CreateSys( &bbdSetup , work->heapId );
    GFL_BBD_SetScale( work->bbdSys , &bbdScale );
  }
}

static void MB_CAPTURE_TermGraphic( MB_CAPTURE_WORK *work )
{
  GFL_BBD_DeleteSys( work->bbdSys );
  GFL_G3D_CAMERA_Delete( work->camera );
  GFL_G3D_Exit();

  GFL_CLACT_SYS_Delete();
  GFL_BG_FreeBGControl( MB_CAPTURE_FRAME_MSG );
  GFL_BG_FreeBGControl( MB_CAPTURE_FRAME_BG );
  GFL_BG_FreeBGControl( MB_CAPTURE_FRAME_SUB_BG );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}


//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void MB_CAPTURE_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}


//--------------------------------------------------------------
//  リソース読み込み
//--------------------------------------------------------------
static void MB_CAPTURE_LoadResource( MB_CAPTURE_WORK *work )
{
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_MB_CHILD , work->heapId );

  //下画面
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_mb_child_gra_bg_sub_NCLR , 
                    PALTYPE_SUB_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_mb_child_gra_bg_sub_NCGR ,
                    MB_CAPTURE_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_mb_child_gra_bg_sub_NSCR , 
                    MB_CAPTURE_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );
  
  //上画面
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_mb_child_gra_bg_main_NCLR , 
                    PALTYPE_MAIN_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_mb_child_gra_bg_main_NCGR ,
                    MB_CAPTURE_FRAME_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_mb_child_gra_bg_main_NSCR , 
                    MB_CAPTURE_FRAME_BG ,  0 , 0, FALSE , work->heapId );
  
  GFL_ARC_CloseDataHandle( arcHandle );
}

//--------------------------------------------------------------
//  オブジェクト作成
//--------------------------------------------------------------
static void MB_CAPTURE_InitObject( MB_CAPTURE_WORK *work )
{
  int i;
  MB_CAP_OBJ_INIT_WORK initWork;
  initWork.heapId = work->heapId;
  initWork.bbdSys = work->bbdSys;
  initWork.arcHandle = work->initWork->arcHandle;
  
  for( i=0;i<MB_CAP_OBJ_MAIN_NUM;i++ )
  {
    initWork.type = MCOT_GRASS;
    initWork.pos.x = FX32_CONST( (i%MB_CAP_OBJ_X_NUM) * MB_CAP_OBJ_MAIN_X_SPACE + MB_CAP_OBJ_MAIN_LEFT );
    initWork.pos.y = FX32_CONST( (i/MB_CAP_OBJ_X_NUM) * MB_CAP_OBJ_MAIN_Y_SPACE + MB_CAP_OBJ_MAIN_TOP );
    initWork.pos.z = 0;
    work->fieldObj[i] = MB_CAP_OBJ_CreateObject( &initWork );
  }
  for( i=MB_CAP_OBJ_SUB_U_START;i<MB_CAP_OBJ_SUB_D_START;i++ )
  {
    const int idx = i-MB_CAP_OBJ_SUB_U_START;
    initWork.type = MCOT_WOOD;
    initWork.pos.x = FX32_CONST( idx * MB_CAP_OBJ_MAIN_X_SPACE + MB_CAP_OBJ_MAIN_LEFT );
    initWork.pos.y = FX32_CONST( MB_CAP_OBJ_SUB_U_TOP );
    initWork.pos.z = 0;
    work->fieldObj[i] = MB_CAP_OBJ_CreateObject( &initWork );
  }
  for( i=MB_CAP_OBJ_SUB_D_START;i<MB_CAP_OBJ_SUB_R_START;i++ )
  {
    const int idx = i-MB_CAP_OBJ_SUB_D_START;
    initWork.type = MCOT_WATER;
    initWork.pos.x = FX32_CONST( idx * MB_CAP_OBJ_MAIN_X_SPACE + MB_CAP_OBJ_MAIN_LEFT );
    initWork.pos.y = FX32_CONST( MB_CAP_OBJ_SUB_D_TOP );
    initWork.pos.z = 0;
    work->fieldObj[i] = MB_CAP_OBJ_CreateObject( &initWork );
  }
  for( i=MB_CAP_OBJ_SUB_R_START;i<MB_CAP_OBJ_SUB_L_START;i++ )
  {
    const int idx = i-MB_CAP_OBJ_SUB_R_START;
    initWork.type = MCOT_GRASS_SIDE;
    initWork.pos.x = FX32_CONST( MB_CAP_OBJ_SUB_R_LEFT );
    initWork.pos.y = FX32_CONST( idx * MB_CAP_OBJ_MAIN_Y_SPACE + MB_CAP_OBJ_MAIN_TOP );
    initWork.pos.z = 0;
    work->fieldObj[i] = MB_CAP_OBJ_CreateObject( &initWork );
  }
  for( i=MB_CAP_OBJ_SUB_L_START;i<MB_CAP_OBJ_NUM;i++ )
  {
    const int idx = i-MB_CAP_OBJ_SUB_L_START;
    initWork.type = MCOT_GRASS_SIDE;
    initWork.pos.x = FX32_CONST( MB_CAP_OBJ_SUB_L_LEFT );
    initWork.pos.y = FX32_CONST( idx * MB_CAP_OBJ_MAIN_Y_SPACE + MB_CAP_OBJ_MAIN_TOP );
    initWork.pos.z = 0;
    work->fieldObj[i] = MB_CAP_OBJ_CreateObject( &initWork );
  }
}

//--------------------------------------------------------------
//  オブジェクト開放
//--------------------------------------------------------------
static void MB_CAPTURE_TermObject( MB_CAPTURE_WORK *work )
{
  int i;
  for( i=0;i<MB_CAP_OBJ_NUM;i++ )
  {
    MB_CAP_OBJ_DeleteObject( work->fieldObj[i] );
  }
}

//--------------------------------------------------------------
//  ポケモン作成
//--------------------------------------------------------------
static void MB_CAPTURE_InitPoke( MB_CAPTURE_WORK *work )
{
  int i;
  MB_CAP_POKE_INIT_WORK initWork;
  initWork.heapId = work->heapId;
  initWork.bbdSys = work->bbdSys;
  initWork.arcHandle = work->initWork->arcHandle;
  initWork.pokeArcHandle = MB_ICON_GetArcHandle(GFL_HEAP_LOWID(work->heapId),work->initWork->cardType);
  
  for( i=0;i<MB_CAP_POKE_NUM;i++ )
  {
    initWork.ppp = work->initWork->ppp[i];
    work->pokeObj[i] = MB_CAP_POKE_CreateObject( &initWork );
  }
}

//--------------------------------------------------------------
//  ポケモン開放
//--------------------------------------------------------------
static void MB_CAPTURE_TermPoke( MB_CAPTURE_WORK *work )
{
  int i;
  for( i=0;i<MB_CAP_POKE_NUM;i++ )
  {
    MB_CAP_POKE_DeleteObject( work->pokeObj[i] );
  }
}


#pragma mark [>proc func
static GFL_PROC_RESULT MB_CAPTURE_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MB_CAPTURE_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MB_CAPTURE_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA MultiBootCapture_ProcData =
{
  MB_CAPTURE_ProcInit,
  MB_CAPTURE_ProcMain,
  MB_CAPTURE_ProcTerm
};

//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT MB_CAPTURE_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MB_CAPTURE_WORK *work;
  MB_CAPTURE_INIT_WORK *initWork;
  
  if( pwk == NULL )
  {
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
    const HEAPID parentHeap = GFL_HEAPID_APP;
#else                    //DL子機時処理
    const HEAPID parentHeap = HEAPID_MULTIBOOT;
#endif //MULTI_BOOT_MAKE
    u8 i;
    GFL_HEAP_CreateHeap( parentHeap , HEAPID_MB_BOX, 0x40000 );
    
    work = GFL_PROC_AllocWork( proc, sizeof(MB_CAPTURE_WORK), parentHeap );
    initWork = GFL_HEAP_AllocClearMemory( HEAPID_MB_BOX , sizeof(MB_CAPTURE_INIT_WORK) );

    initWork->parentHeap = parentHeap;
    initWork->cardType = CARD_TYPE_DUMMY;
    for( i=0;i<MB_CAP_POKE_NUM;i++ )
    {
      initWork->ppp[i] = GFL_HEAP_AllocClearMemory( parentHeap , POKETOOL_GetPPPWorkSize() );
      PPP_Setup( initWork->ppp[i] , 
                 GFUser_GetPublicRand0(493)+1 ,
                 GFUser_GetPublicRand0(100)+1 ,
                 PTL_SETUP_ID_AUTO );
    }
#ifndef MULTI_BOOT_MAKE  //通常時処理
    initWork->arcHandle = GFL_ARC_OpenDataHandle( ARCID_MB_CAPTER , parentHeap );
#endif //MULTI_BOOT_MAKE
    
    work->initWork = initWork;
  }
  else
  {
    initWork = pwk;
    GFL_HEAP_CreateHeap( initWork->parentHeap , HEAPID_MB_BOX, 0x40000 );
    work = GFL_PROC_AllocWork( proc, sizeof(MB_CAPTURE_WORK), HEAPID_MULTIBOOT );
    work->initWork = pwk;
  }
  

  work->heapId = HEAPID_MB_BOX;
  
  MB_CAPTURE_Init( work );
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static GFL_PROC_RESULT MB_CAPTURE_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MB_CAPTURE_WORK *work = mywk;
  
  MB_CAPTURE_Term( work );

  if( pwk == NULL )
  {
    u8 i;
    GFL_ARC_CloseDataHandle( work->initWork->arcHandle );
    for( i=0;i<MB_CAP_POKE_NUM;i++ )
    {
      GFL_HEAP_FreeMemory( work->initWork->ppp[i] );
    }
    GFL_HEAP_FreeMemory( work->initWork );
  }

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_MB_BOX );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  ループ
//--------------------------------------------------------------
static GFL_PROC_RESULT MB_CAPTURE_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MB_CAPTURE_WORK *work = mywk;
  const BOOL ret = MB_CAPTURE_Main( work );
  
  if( ret == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}



