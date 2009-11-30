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
#include "mb_capture_gra.naix"

#include "multiboot/mb_util_msg.h"
#include "multiboot/mb_poke_icon.h"
#include "multiboot/mb_local_def.h"
#include "multiboot/mb_capture_sys.h"
#include "./mb_cap_obj.h"
#include "./mb_cap_poke.h"
#include "./mb_cap_down.h"
#include "./mb_cap_ball.h"
#include "./mb_cap_effect.h"
#include "./mb_cap_local_def.h"


//======================================================================
//	define
//======================================================================
#pragma mark [> define

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
struct _MB_CAPTURE_WORK
{
  HEAPID heapId;
  GFL_TCB *vBlankTcb;
  MB_CAPTURE_INIT_WORK *initWork;
  
  MB_CAPTURE_STATE  state;
  
  GFL_G3D_CAMERA    *camera;
  GFL_BBD_SYS     *bbdSys;
  int             bbdRes[MCBR_MAX];
  
  MB_CAP_POKE *pokeWork[MB_CAP_POKE_NUM];
  MB_CAP_OBJ  *objWork[MB_CAP_OBJ_NUM];
  MB_CAP_BALL  *ballWork[MB_CAP_BALL_NUM];
  MB_CAP_EFFECT  *effWork[MB_CAP_EFFECT_NUM];
  MB_CAP_DOWN *downWork;
  
  //上画面系
  BOOL isShotBall;
  int resIdxTarget;
  int objIdxTarget;
  
  VecFx32 targetPos;
  

};


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

static void MB_CAPTURE_InitUpper( MB_CAPTURE_WORK *work );
static void MB_CAPTURE_TermUpper( MB_CAPTURE_WORK *work );
static void MB_CAPTURE_UpdateUpper( MB_CAPTURE_WORK *work );

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_A,             // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,       // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,         // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_64_E ,           // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,      // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_128_D,        // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_0_B,             // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_01_FG,         // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_32K,
  GX_OBJVRAMMODE_CHAR_1D_32K
};


//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
static void MB_CAPTURE_Init( MB_CAPTURE_WORK *work )
{
  u8 i;
  work->state = MSS_FADEIN;
  
  MB_CAPTURE_InitGraphic( work );
  MB_CAPTURE_LoadResource( work );
  work->vBlankTcb = GFUser_VIntr_CreateTCB( MB_CAPTURE_VBlankFunc , work , 8 );

  MB_CAPTURE_InitObject( work );
  MB_CAPTURE_InitPoke( work );
  MB_CAPTURE_InitUpper( work );
  work->downWork = MB_CAP_DOWN_InitSystem( work );
  work->isShotBall = FALSE;

  for( i=0;i<MB_CAP_BALL_NUM;i++ )
  {
    work->ballWork[i] = NULL;
  }
  for( i=0;i<MB_CAP_EFFECT_NUM;i++ )
  {
    work->effWork[i] = NULL;
  }
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static void MB_CAPTURE_Term( MB_CAPTURE_WORK *work )
{
  u8 i;
  for( i=0;i<MB_CAP_BALL_NUM;i++ )
  {
    if( work->ballWork[i] != NULL )
    {
      MB_CAP_BALL_DeleteObject( work , work->ballWork[i] );
    }
  }
  for( i=0;i<MB_CAP_EFFECT_NUM;i++ )
  {
    if( work->effWork[i] != NULL )
    {
      MB_CAP_EFFECT_DeleteObject( work , work->effWork[i] );
    }
  }

  MB_CAP_DOWN_DeleteSystem( work , work->downWork );
  MB_CAPTURE_TermUpper( work );
  MB_CAPTURE_TermObject( work );
  MB_CAPTURE_TermPoke( work );

  GFL_TCB_DeleteTask( work->vBlankTcb );

  for( i=0;i<MCBR_MAX;i++ )
  {
    GFL_BBD_RemoveResource( work->bbdSys , work->bbdRes[i] );
  }

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

  MB_CAP_DOWN_UpdateSystem( work , work->downWork );
  MB_CAPTURE_UpdateUpper( work );

  //3D描画  
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    GFL_BBD_Draw( work->bbdSys , work->camera , NULL );
  }
  GFL_G3D_DRAW_End();

  //OBJの更新
  GFL_CLACT_SYS_Main();

  
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_START &&
      GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT )
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
        GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_1, GX_BG0_AS_3D,
    };
    // BG1 MAIN (枠
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x18000,0x0000,
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

    // BG1 SUB (弓の弦
    static const GFL_BG_BGCNT_HEADER header_sub1 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000,0x06000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub2 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x18000,0x08000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 SUB (弓・アフィン
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x1000, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x08000,0x10000,
      GX_BG_EXTPLTT_23, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    GFL_BG_SetBGMode( &sys_data );

    MB_CAPTURE_SetupBgFunc( &header_main1 , MB_CAPTURE_FRAME_FRAME , GFL_BG_MODE_TEXT );
    MB_CAPTURE_SetupBgFunc( &header_main3 , MB_CAPTURE_FRAME_BG , GFL_BG_MODE_TEXT );
    MB_CAPTURE_SetupBgFunc( &header_sub1  , MB_CAPTURE_FRAME_SUB_BOW_LINE , GFL_BG_MODE_TEXT );
    MB_CAPTURE_SetupBgFunc( &header_sub2  , MB_CAPTURE_FRAME_SUB_BG , GFL_BG_MODE_TEXT );
    MB_CAPTURE_SetupBgFunc( &header_sub3  , MB_CAPTURE_FRAME_SUB_BOW , GFL_BG_MODE_AFFINE );
    
  }

  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
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
  GFL_BG_FreeBGControl( MB_CAPTURE_FRAME_FRAME );
  GFL_BG_FreeBGControl( MB_CAPTURE_FRAME_BG );
  GFL_BG_FreeBGControl( MB_CAPTURE_FRAME_SUB_BOW_LINE );
  GFL_BG_FreeBGControl( MB_CAPTURE_FRAME_SUB_BG );
  GFL_BG_FreeBGControl( MB_CAPTURE_FRAME_SUB_BOW );
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
  //下画面はmb_cap_downで
  
  //上画面
  GFL_ARCHDL_UTIL_TransVramPalette( work->initWork->arcHandle , NARC_mb_capture_gra_cap_bgu_NCLR , 
                    PALTYPE_MAIN_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( work->initWork->arcHandle , NARC_mb_capture_gra_cap_bgu_NCGR ,
                    MB_CAPTURE_FRAME_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( work->initWork->arcHandle , NARC_mb_capture_gra_cap_bgu_field_NSCR , 
                    MB_CAPTURE_FRAME_BG ,  0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( work->initWork->arcHandle , NARC_mb_capture_gra_cap_bgu_gage_NSCR , 
                    MB_CAPTURE_FRAME_FRAME ,  0 , 0, FALSE , work->heapId );

  GFL_BG_LoadScreenV_Req( MB_CAPTURE_FRAME_BG );
  GFL_BG_LoadScreenV_Req( MB_CAPTURE_FRAME_FRAME );
  
  //BBDリソース
  {
    static const u32 resListArr[MCBR_MAX] = {
      NARC_mb_capture_gra_cap_obj_shadow_nsbtx ,
      NARC_mb_capture_gra_cap_obj_ballnormal_nsbtx ,
      NARC_mb_capture_gra_cap_obj_ballbonus_nsbtx ,
      NARC_mb_capture_gra_cap_obj_kusamura1_nsbtx ,
      NARC_mb_capture_gra_cap_obj_kusamura2_nsbtx ,
      NARC_mb_capture_gra_cap_obj_wood_nsbtx ,
      NARC_mb_capture_gra_cap_obj_water_nsbtx ,
      NARC_mb_capture_gra_cap_obj_hit_nsbtx ,
      NARC_mb_capture_gra_cap_obj_kemuri_nsbtx ,
      NARC_mb_capture_gra_cap_obj_getefect_nsbtx ,
      };
    static const u32 resSizeArr[MCBR_MAX] = {
      GFL_BBD_TEXSIZDEF_32x32 ,
      GFL_BBD_TEXSIZDEF_256x32 ,
      GFL_BBD_TEXSIZDEF_256x32 ,
      GFL_BBD_TEXSIZDEF_128x32 ,
      GFL_BBD_TEXSIZDEF_128x32 ,
      GFL_BBD_TEXSIZDEF_128x32 ,
      GFL_BBD_TEXSIZDEF_32x32 ,
      GFL_BBD_TEXSIZDEF_64x32 ,
      GFL_BBD_TEXSIZDEF_256x32 ,
      GFL_BBD_TEXSIZDEF_256x32 ,
      };
    u8 i;
    
    for( i=0;i<MCBR_MAX;i++ )
    {
      GFL_G3D_RES* res = GFL_G3D_CreateResourceHandle( work->initWork->arcHandle , resListArr[i] );
      GFL_G3D_TransVramTexture( res );
      work->bbdRes[i] = GFL_BBD_AddResource( work->bbdSys , 
                                       res , 
                                       GFL_BBD_TEXFMT_PAL16 ,
                                       resSizeArr[i] ,
                                       32 , 32 );
      GFL_BBD_CutResourceData( work->bbdSys , work->bbdRes[i] );
    }
  }
}

//--------------------------------------------------------------
//  オブジェクト作成
//--------------------------------------------------------------
static void MB_CAPTURE_InitObject( MB_CAPTURE_WORK *work )
{
  int i;
  MB_CAP_OBJ_INIT_WORK initWork;
  for( i=0;i<MB_CAP_OBJ_MAIN_NUM;i++ )
  {
    const u8 idxX = (i%MB_CAP_OBJ_X_NUM);
    const u8 idxY = (i/MB_CAP_OBJ_X_NUM);
    initWork.type = MCOT_GRASS;
    MB_CAPTURE_GetGrassObjectPos( idxX , idxY , &initWork.pos );
    work->objWork[i] = MB_CAP_OBJ_CreateObject( work , &initWork );
  }
  for( i=MB_CAP_OBJ_SUB_U_START;i<MB_CAP_OBJ_SUB_D_START;i++ )
  {
    const int idx = i-MB_CAP_OBJ_SUB_U_START;
    initWork.type = MCOT_WOOD;
    initWork.pos.x = FX32_CONST( idx * MB_CAP_OBJ_MAIN_X_SPACE + MB_CAP_OBJ_MAIN_LEFT );
    initWork.pos.y = FX32_CONST( MB_CAP_OBJ_SUB_U_TOP );
    initWork.pos.z = FX32_CONST( MB_CAP_OBJ_BASE_Z + MB_CAP_OBJ_LINEOFS_Z*MB_CAP_OBJ_Y_NUM );
    work->objWork[i] = MB_CAP_OBJ_CreateObject( work , &initWork );
  }
  for( i=MB_CAP_OBJ_SUB_D_START;i<MB_CAP_OBJ_SUB_R_START;i++ )
  {
    const int idx = i-MB_CAP_OBJ_SUB_D_START;
    initWork.type = MCOT_WATER;
    initWork.pos.x = FX32_CONST( idx * MB_CAP_OBJ_MAIN_X_SPACE + MB_CAP_OBJ_MAIN_LEFT );
    initWork.pos.y = FX32_CONST( MB_CAP_OBJ_SUB_D_TOP );
    initWork.pos.z = FX32_CONST( MB_CAP_OBJ_BASE_Z - MB_CAP_OBJ_LINEOFS_Z );
    work->objWork[i] = MB_CAP_OBJ_CreateObject( work , &initWork );
  }
  for( i=MB_CAP_OBJ_SUB_R_START;i<MB_CAP_OBJ_SUB_L_START;i++ )
  {
    const int idx = i-MB_CAP_OBJ_SUB_R_START;
    initWork.type = MCOT_GRASS_SIDE;
    initWork.pos.x = FX32_CONST( MB_CAP_OBJ_SUB_R_LEFT );
    initWork.pos.y = FX32_CONST( idx * MB_CAP_OBJ_MAIN_Y_SPACE + MB_CAP_OBJ_MAIN_TOP );
    initWork.pos.z = FX32_CONST( MB_CAP_OBJ_BASE_Z + (MB_CAP_OBJ_Y_NUM-idx-1)*MB_CAP_OBJ_LINEOFS_Z );
    work->objWork[i] = MB_CAP_OBJ_CreateObject( work , &initWork );
  }
  for( i=MB_CAP_OBJ_SUB_L_START;i<MB_CAP_OBJ_NUM;i++ )
  {
    const int idx = i-MB_CAP_OBJ_SUB_L_START;
    initWork.type = MCOT_GRASS_SIDE;
    initWork.pos.x = FX32_CONST( MB_CAP_OBJ_SUB_L_LEFT );
    initWork.pos.y = FX32_CONST( idx * MB_CAP_OBJ_MAIN_Y_SPACE + MB_CAP_OBJ_MAIN_TOP );
    initWork.pos.z = FX32_CONST( MB_CAP_OBJ_BASE_Z + (MB_CAP_OBJ_Y_NUM-idx-1)*MB_CAP_OBJ_LINEOFS_Z );
    work->objWork[i] = MB_CAP_OBJ_CreateObject( work , &initWork );
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
    MB_CAP_OBJ_DeleteObject( work , work->objWork[i] );
  }
}

//--------------------------------------------------------------
//  ポケモン作成
//--------------------------------------------------------------
static void MB_CAPTURE_InitPoke( MB_CAPTURE_WORK *work )
{
  int i,j;
  MB_CAP_POKE_INIT_WORK initWork;
  initWork.pokeArcHandle = MB_ICON_GetArcHandle(GFL_HEAP_LOWID(work->heapId),work->initWork->cardType);
  
  for( i=0;i<MB_CAP_POKE_NUM;i++ )
  {
    initWork.ppp = work->initWork->ppp[i];
    work->pokeWork[i] = MB_CAP_POKE_CreateObject( work , &initWork );
  }
  GFL_ARC_CloseDataHandle( initWork.pokeArcHandle );

  //出現設定
  for( i=0;i<MB_CAP_POKE_NUM;i++ )
  {
    BOOL isLoop = TRUE;
    while( isLoop == TRUE )
    {
      const u8 idxX = GFUser_GetPublicRand0( MB_CAP_OBJ_X_NUM );
      const u8 idxY = GFUser_GetPublicRand0( MB_CAP_OBJ_Y_NUM );
      isLoop = FALSE;
      for( j=0;j<i;j++ )
      {
        if( MB_CAP_POKE_CheckPos( work->pokeWork[j] , idxX , idxY ) == TRUE )
        {
          isLoop = TRUE;
          break;
        }
      }
      if( isLoop == FALSE )
      {
        MB_CAP_POKE_SetHide( work , work->pokeWork[j] , idxX , idxY );
        OS_TPrintf("[%d][%d]\n",idxX,idxY);
      }
    }
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
    MB_CAP_POKE_DeleteObject( work , work->pokeWork[i] );
  }
}

#pragma mark [>upper func
//--------------------------------------------------------------
//  上画面系初期化
//--------------------------------------------------------------
static void MB_CAPTURE_InitUpper( MB_CAPTURE_WORK *work )
{
  GFL_G3D_RES* res = GFL_G3D_CreateResourceHandle( work->initWork->arcHandle , NARC_mb_capture_gra_cap_obj_target_nsbtx );
  VecFx32 pos = {0,0,0};
  const BOOL flg = TRUE;
  GFL_G3D_TransVramTexture( res );
  work->resIdxTarget = GFL_BBD_AddResource( work->bbdSys , 
                                       res , 
                                       GFL_BBD_TEXFMT_PAL16 ,
                                       GFL_BBD_TEXSIZDEF_32x32 ,
                                       32 , 32 );
  GFL_BBD_CutResourceData( work->bbdSys , work->resIdxTarget );
  
  work->objIdxTarget = GFL_BBD_AddObject( work->bbdSys , 
                                     work->resIdxTarget ,
                                     FX32_ONE , 
                                     FX32_ONE , 
                                     &pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );

  //3D設定の関係で反転させる・・・
  GFL_BBD_SetObjectFlipT( work->bbdSys , work->objIdxTarget , &flg );

}

//--------------------------------------------------------------
//  上画面系開放
//--------------------------------------------------------------
static void MB_CAPTURE_TermUpper( MB_CAPTURE_WORK *work )
{
  GFL_BBD_RemoveObject( work->bbdSys , work->resIdxTarget );
  GFL_BBD_RemoveResource( work->bbdSys , work->objIdxTarget );

}

//--------------------------------------------------------------
//  上画面系更新
//--------------------------------------------------------------
static void MB_CAPTURE_UpdateUpper( MB_CAPTURE_WORK *work )
{
  u8 i;
  u8 activeBallNum = 0;
  const MB_CAP_DOWN_STATE downState = MB_CAP_DOWN_GetState( work->downWork );

  if( downState == MCDS_DRAG )
  {
    const BOOL flg = TRUE;
    VecFx32 pos = {FX32_CONST(128),FX32_CONST(MB_CAP_UPPER_BALL_POS_BASE_Y),FX32_CONST(MB_CAP_UPPER_TARGET_BASE_Z)};
    VecFx32 ofs = {0,0,0};
    
    {
      //距離計算
      const fx32 pullLen = MB_CAP_DOWN_GetPullLen( work->downWork );
      const fx32 rate = FX_Div(pullLen,MB_CAP_DOWN_BOW_PULL_LEN_MAX);
      const fx32 len = (MB_CAP_UPPER_BALL_LEN_MAX-MB_CAP_UPPER_BALL_LEN_MIN)*rate;
      ofs.y = len;
    }
    {
      //回転計算
      const u16 rotAngle = MB_CAP_DOWN_GetRotAngle( work->downWork );
      const fx32 sin = FX_SinIdx( rotAngle );
      const fx32 cos = FX_CosIdx( rotAngle );
      
      ofs.x = -FX_Mul( ofs.y , sin );
      ofs.y = FX_Mul( ofs.y , cos );
    }
    VEC_Subtract( &pos , &ofs , &work->targetPos );
    
    GFL_BBD_SetObjectDrawEnable( work->bbdSys , work->objIdxTarget , &flg );
    GFL_BBD_SetObjectTrans( work->bbdSys , work->objIdxTarget , &work->targetPos );
    work->isShotBall = FALSE;

  }
  else
  if( downState == MCDS_SHOT_WAIT )
  {
    if( work->isShotBall == FALSE )
    {
      u8 i;
      for( i=0;i<MB_CAP_BALL_NUM;i++ )
      {
        if( work->ballWork[i] == NULL )
        {
          MB_CAP_BALL_INIT_WORK initWork;
          
          initWork.isBonus = FALSE;
          initWork.targetPos = work->targetPos;
          initWork.rotAngle = MB_CAP_DOWN_GetShotAngle( work->downWork );
          work->ballWork[i] = MB_CAP_BALL_CreateObject( work , &initWork );
          break;
        }
      }
      work->isShotBall = TRUE;
    }
  }
  else
  {
    const BOOL flg = FALSE;
    GFL_BBD_SetObjectDrawEnable( work->bbdSys , work->objIdxTarget , &flg );
  }

  for( i=0;i<MB_CAP_BALL_NUM;i++ )
  {
    if( work->ballWork[i] != NULL )
    {
      MB_CAP_BALL_UpdateObject( work , work->ballWork[i] );
      if( MB_CAP_BALL_IsFinish( work->ballWork[i] ) == TRUE )
      {
        MB_CAP_BALL_DeleteObject( work , work->ballWork[i] );
        work->ballWork[i] = NULL;
      }
      else
      {
        if( MB_CAP_BALL_GetState( work->ballWork[i] ) == MCBS_FLYING )
        {
          activeBallNum++;
        }
      }
    }
  }
  for( i=0;i<MB_CAP_EFFECT_NUM;i++ )
  {
    if( work->effWork[i] != NULL )
    {
      MB_CAP_EFFECT_UpdateObject( work , work->effWork[i] );
      if( MB_CAP_EFFECT_IsFinish( work->effWork[i] ) == TRUE )
      {
        MB_CAP_EFFECT_DeleteObject( work , work->effWork[i] );
        work->effWork[i] = NULL;
      }
    }
  }

  for( i=0;i<MB_CAP_POKE_NUM;i++ )
  {
    MB_CAP_POKE_UpdateObject( work , work->pokeWork[i] );
  }

  
  if( activeBallNum == 0 &&
      downState == MCDS_SHOT_WAIT &&
      work->isShotBall == TRUE )
  {
    MB_CAP_DOWN_ReloadBall( work->downWork );
    work->isShotBall = FALSE;
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



#pragma mark [>outer func
//--------------------------------------------------------------
//  外部提供関数群
//--------------------------------------------------------------
const HEAPID MB_CAPTURE_GetHeapId( const MB_CAPTURE_WORK *work )
{
  return work->heapId;
}
GFL_BBD_SYS* MB_CAPTURE_GetBbdSys( MB_CAPTURE_WORK *work )
{
  return work->bbdSys;
}
ARCHANDLE* MB_CAPTURE_GetArcHandle( MB_CAPTURE_WORK *work )
{
  return work->initWork->arcHandle;
}
const DLPLAY_CARD_TYPE MB_CAPTURE_GetCardType( const MB_CAPTURE_WORK *work )
{
  return work->initWork->cardType;
}
const int MB_CAPTURE_GetBbdResIdx( const MB_CAPTURE_WORK *work , const MB_CAP_BBD_RES resType )
{
  return work->bbdRes[resType];
}
MB_CAP_OBJ* MB_CAPTURE_GetObjWork( MB_CAPTURE_WORK *work , const u8 idx )
{
  return work->objWork[idx];
}
MB_CAP_POKE* MB_CAPTURE_GetPokeWork( MB_CAPTURE_WORK *work , const u8 idx )
{
  return work->pokeWork[idx];
}

//--------------------------------------------------------------
//  ポケモン捕獲処理
//--------------------------------------------------------------
void MB_CAPTURE_GetPokeFunc( MB_CAPTURE_WORK *work , MB_CAP_BALL *ballWork , const u8 pokeIdx )
{
  MB_CAP_POKE *pokeWork = work->pokeWork[pokeIdx];
  
  MB_CAP_POKE_SetCapture( work , pokeWork );
}

//--------------------------------------------------------------
//  エフェクトを作る
//--------------------------------------------------------------
void MB_CAPTURE_CreateEffect( MB_CAPTURE_WORK *work , VecFx32 *pos , const MB_CAP_EFFECT_TYPE type )
{
  u8 i;
  for( i=0;i<MB_CAP_EFFECT_NUM;i++ )
  {
    if( work->effWork[i] == NULL )
    {
      MB_CAP_EFFECT_INIT_WORK initWork;
      
      initWork.pos = *pos;
      initWork.type = type;
      work->effWork[i] = MB_CAP_EFFECT_CreateObject( work , &initWork );
      break;
    }
  }
  
}


#pragma mark [>outer func
//--------------------------------------------------------------
//  便利関数
//--------------------------------------------------------------
void MB_CAPTURE_GetGrassObjectPos( const u8 idxX , const u8 idxY , VecFx32 *ret )
{
  ret->x = FX32_CONST( idxX * MB_CAP_OBJ_MAIN_X_SPACE + MB_CAP_OBJ_MAIN_LEFT);
  ret->y = FX32_CONST( idxY * MB_CAP_OBJ_MAIN_Y_SPACE + MB_CAP_OBJ_MAIN_TOP);
  ret->z = FX32_CONST( MB_CAP_OBJ_BASE_Z + (MB_CAP_OBJ_Y_NUM-idxY-1)*MB_CAP_OBJ_LINEOFS_Z );
}

//--------------------------------------------------------------
//  当たり判定のチェック
//--------------------------------------------------------------
const BOOL MB_CAPTURE_CheckHit( const MB_CAP_HIT_WORK *work1 , MB_CAP_HIT_WORK *work2 )
{
  if((work1->pos->x - work1->size.x < work2->pos->x + work2->size.x) &&
     (work1->pos->x + work1->size.x > work2->pos->x - work2->size.x) &&
     (work1->pos->y - work1->size.y < work2->pos->y + work2->size.y) &&
     (work1->pos->y + work1->size.y > work2->pos->y - work2->size.y) &&
     (work1->height - work1->size.z < work2->height + work2->size.z) &&
     (work1->height + work1->size.z > work2->height - work2->size.z) )
  {
//    OS_Printf("A[%6.2f][%6.2f][%6.2f]\n",F32_CONST(work1->pos->x),F32_CONST(work1->pos->y),F32_CONST(work1->height));
//    OS_Printf(" [%6.2f][%6.2f][%6.2f]\n",F32_CONST(work1->size.x),F32_CONST(work1->size.y),F32_CONST(work1->size.z));
//    OS_Printf("B[%6.2f][%6.2f][%6.2f]\n",F32_CONST(work2->pos->x),F32_CONST(work2->pos->y),F32_CONST(work2->height));
//    OS_Printf(" [%6.2f][%6.2f][%6.2f]\n",F32_CONST(work2->size.x),F32_CONST(work2->size.y),F32_CONST(work2->size.z));
    return TRUE;
  }
  return FALSE;
}
