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

#include "net_app/comm_tvt_sys.h"
#include "ctvt_camera.h"
#include "ctvt_comm.h"
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

  u8 connectNum;
  u8 selfIdx;
  BOOL isDouble;
  
  //セル系
  GFL_CLUNIT  *cellUnit;

  //各ワーク
  CTVT_CAMERA_WORK *camWork;
  CTVT_COMM_WORK   *commWork;

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
  GX_OBJVRAMMODE_CHAR_1D_32K,
  GX_OBJVRAMMODE_CHAR_1D_32K
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
  work->camWork = CTVT_CAMERA_Init( work , work->heapId );
  work->commWork = CTVT_COMM_InitSystem( work , work->heapId );
  
  work->vBlankTcb = GFUser_VIntr_CreateTCB( COMM_TVT_VBlankFunc , work , 8 );


  GX_SetMasterBrightness(0);
  GXS_SetMasterBrightness(0);
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
static void COMM_TVT_Term( COMM_TVT_WORK *work )
{
  GFL_TCB_DeleteTask( work->vBlankTcb );
  
  CTVT_COMM_TermSystem( work , work->commWork );
  CTVT_CAMERA_Term( work , work->camWork );
  COMM_TVT_TermGraphic( work );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
static const BOOL COMM_TVT_Main( COMM_TVT_WORK *work )
{
  CTVT_CAMERA_Main( work , work->camWork );
  CTVT_COMM_Main( work , work->commWork );

  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG )
  {
    return TRUE;
  }
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

    // BG3 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x08000,0x08000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    GFL_BG_SetBGMode( &sys_data );

    COMM_TVT_SetupBgFunc( &header_sub3 , CTVT_FRAME_SUB_BG , GFL_BG_MODE_TEXT );
    
    //ダイレクトBMPにGFLが対応してないのでべた書き

    G2_SetBG2ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x10000);
    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x28000);

    GFL_STD_MemClear( G2_GetBG2ScrPtr() , 256*192*sizeof(u16) );
    GFL_STD_MemClear( G2_GetBG3ScrPtr() , 256*192*sizeof(u16) );

    G2S_SetBG2Priority(2);
    G2S_SetBG3Priority(3);

  }
  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );

    work->cellUnit = GFL_CLACT_UNIT_Create( 48 , 0, work->heapId );
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

#pragma mark [>outer func (system
CTVT_CAMERA_WORK* COMM_TVT_GetCameraWork( COMM_TVT_WORK *work )
{
  return work->camWork;
}

#pragma mark [>outer func (value
//--------------------------------------------------------------------------
//  モードとか人数
//--------------------------------------------------------------------------
const HEAPID COMM_TVT_GetHeapId( const COMM_TVT_WORK *work )
{
  return work->heapId;
}
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
  }
  else
  {
    work->initWork = pwk;
  }
  
  COMM_TVT_Init( work );
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
static GFL_PROC_RESULT COMM_TVT_Proc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  COMM_TVT_WORK *work = mywk;

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

  const BOOL ret = COMM_TVT_Main( work );
  
  
  
  if( ret == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}


