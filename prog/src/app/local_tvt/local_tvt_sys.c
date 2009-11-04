//======================================================================
/**
 * @file	local_tvt_sys.c
 * @brief	非通信テレビトランシーバー メイン
 * @author	ariizumi
 * @data	09/11/02
 *
 * モジュール名：LOCAL_TVT
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"

#include "arc_def.h"
#include "local_tvt.naix"

#include "local_tvt_local_def.h"
#include "local_tvt_chara.h"

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

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_256_AB,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_23_G,     // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_16_F ,       // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_NONE,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_NONE,        // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,     // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_32K,
  GX_OBJVRAMMODE_CHAR_1D_32K
};

static void LOCAL_TVT_Init( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_Term( LOCAL_TVT_WORK *work );
static const BOOL LOCAL_TVT_Main( LOCAL_TVT_WORK *work );

static void LOCAL_TVT_InitGraphic( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_TermGraphic( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void LOCAL_TVT_LoadResource( LOCAL_TVT_WORK *work );
//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
static void LOCAL_TVT_Init( LOCAL_TVT_WORK *work )
{
  u8 i;
  LOCAL_TVT_InitGraphic( work );
  
  work->archandle = GFL_ARC_OpenDataHandle( ARCID_LOCAL_TVT , work->heapId );
  LOCAL_TVT_LoadResource( work );

  for( i=0 ; i<work->initWork->mode ; i++ )
  {
    work->charaWork[i] = LOCAL_TVT_CHARA_Init( work , i );
  }

  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );

  work->transCnt = 0;
  work->bufNo = 1;
  work->transState = LTTS_CHARA1;
  work->transChara = NULL;
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static void LOCAL_TVT_Term( LOCAL_TVT_WORK *work )
{
  u8 i;
  for( i=0 ; i<work->initWork->mode ; i++ )
  {
    LOCAL_TVT_CHARA_Term( work , work->charaWork[i] );
  }
  
  GFL_ARC_CloseDataHandle( work->archandle );
  
  LOCAL_TVT_TermGraphic( work );
}

//--------------------------------------------------------------
//  更新
//--------------------------------------------------------------
static const BOOL LOCAL_TVT_Main( LOCAL_TVT_WORK *work )
{
  u8 i;
  
  //フリップタイミング
  if( work->transCnt < LTVT_TRANS_COUNT_MAX )
  {
    work->transCnt++;
  }
  else
  {
    if( work->transChara == NULL &&
        work->transState == LTTS_TRANS_WAIT )
    {
      GXBGCharBase charaBaseAdr;
      GXBGCharBase bgBaseAdr;
      if( work->bufNo == 0 )
      {
        charaBaseAdr = GX_BG_CHARBASE_0x10000;
        bgBaseAdr = GX_BG_CHARBASE_0x28000;
      }
      else
      {
        charaBaseAdr = GX_BG_CHARBASE_0x1c000;
        bgBaseAdr = GX_BG_CHARBASE_0x34000;
      }
      G2_SetBG2Control256x16Affine( GX_BG_SCRSIZE_256x16PLTT_256x256 , 
                                    GX_BG_AREAOVER_REPEAT ,
                                    GX_BG_SCRBASE_0x6800 ,
                                    charaBaseAdr );
      G2_SetBG3Control256x16Affine( GX_BG_SCRSIZE_256x16PLTT_256x256 , 
                                    GX_BG_AREAOVER_REPEAT ,
                                    GX_BG_SCRBASE_0x7000 ,
                                    bgBaseAdr );
      work->bufNo = !work->bufNo;
      work->transState = LTTS_CHARA1;
      work->transCnt = 0;
    }
  }
  
  //絵の転送
  if( work->transChara != NULL )
  {
    if( LOCAL_TVT_CHARA_isFinishTrans( work , work->transChara ) == TRUE )
    {
      work->transChara = NULL;
    }
  }
  
  if( work->transChara == NULL )
  {
    switch( work->transState )
    {
    case LTTS_CHARA1:
    case LTTS_CHARA2:
    case LTTS_CHARA3:
    case LTTS_CHARA4:
      {
        const u8 charaNo = work->transState - LTTS_CHARA1;
        LOCAL_TVT_CHARA_LoadChara( work , work->charaWork[charaNo] , work->bufNo );
        work->transChara = work->charaWork[charaNo];
        if( work->initWork->mode == LTM_2_MEMBER &&
            work->transState == LTTS_CHARA2 )
        {
          work->transState = LTTS_TRANS_WAIT;
        }
        else
        {
          work->transState++;
        }
      }
      break;
    }
  }

  for( i=0 ; i<work->initWork->mode ; i++ )
  {
    LOCAL_TVT_CHARA_Main( work , work->charaWork[i] );
  }
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START &&
      GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//  グラフィック系初期化
//--------------------------------------------------------------
static void LOCAL_TVT_InitGraphic( LOCAL_TVT_WORK *work )
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
        GX_DISPMODE_GRAPHICS, GX_BGMODE_5, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    // BG1 MAIN (メッセージ
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000,0x6000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 MAIN (キャラ
    static const GFL_BG_BGCNT_HEADER header_main2 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x10000,0x0C000,
      GX_BG_EXTPLTT_23, 2, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 MAIN (背景
    static const GFL_BG_BGCNT_HEADER header_main3 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x28000,0x0C000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    GFL_BG_SetBGMode( &sys_data );

    LOCAL_TVT_SetupBgFunc( &header_main1 , LTVT_FRAME_MESSAGE , GFL_BG_MODE_TEXT );
    LOCAL_TVT_SetupBgFunc( &header_main2 , LTVT_FRAME_CHARA , GFL_BG_MODE_256X16 );
    LOCAL_TVT_SetupBgFunc( &header_main3 , LTVT_FRAME_BG , GFL_BG_MODE_256X16 );
    
  }
  
}

static void LOCAL_TVT_TermGraphic( LOCAL_TVT_WORK *work )
{
  GFL_BG_FreeBGControl( LTVT_FRAME_BG );
  GFL_BG_FreeBGControl( LTVT_FRAME_CHARA );
  GFL_BG_FreeBGControl( LTVT_FRAME_MESSAGE );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}


//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void LOCAL_TVT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}


//--------------------------------------------------------------
//  リソース読み込み
//--------------------------------------------------------------
static void LOCAL_TVT_LoadResource( LOCAL_TVT_WORK *work )
{
  
  GFL_ARCHDL_UTIL_TransVramScreen( work->archandle , 
                                   NARC_local_tvt_local_tvt_scr_NSCR ,
                                   LTVT_FRAME_CHARA ,
                                   0,0,FALSE,work->heapId );
  
  GFL_ARCHDL_UTIL_TransVramScreen( work->archandle , 
                                   NARC_local_tvt_local_tvt_scr_NSCR ,
                                   LTVT_FRAME_BG ,
                                   0,0,FALSE,work->heapId );
}


#pragma mark [>proc func
static GFL_PROC_RESULT LOCAL_TVT_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT LOCAL_TVT_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT LOCAL_TVT_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA LocalTvt_ProcData =
{
  LOCAL_TVT_ProcInit,
  LOCAL_TVT_ProcMain,
  LOCAL_TVT_ProcTerm
};

//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT LOCAL_TVT_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  LOCAL_TVT_INIT_WORK *initWork;
  LOCAL_TVT_WORK *work;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_LOCAL_TVT, 0x40000 );
  work = GFL_PROC_AllocWork( proc, sizeof(LOCAL_TVT_WORK), HEAPID_LOCAL_TVT );
  if( pwk == NULL )
  {
    initWork = GFL_HEAP_AllocMemory( HEAPID_LOCAL_TVT , sizeof( LOCAL_TVT_INIT_WORK ));
    initWork->mode = LTM_2_MEMBER;
    initWork->charaType[0] = 0;
    initWork->charaType[1] = 1;
    initWork->bgType[0] = 0;
    initWork->bgType[1] = 1;
  }
  else
  {
    initWork = pwk;
  }
  work->heapId = HEAPID_LOCAL_TVT;
  work->initWork = initWork;
  
  LOCAL_TVT_Init( work );
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static GFL_PROC_RESULT LOCAL_TVT_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  LOCAL_TVT_WORK *work = mywk;
  
  LOCAL_TVT_Term( work );

  if( pwk == NULL )
  {
    GFL_HEAP_FreeMemory( work->initWork );
  }
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_LOCAL_TVT );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  ループ
//--------------------------------------------------------------
static GFL_PROC_RESULT LOCAL_TVT_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  LOCAL_TVT_WORK *work = mywk;
  const BOOL ret = LOCAL_TVT_Main( work );
  
  if( ret == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}
