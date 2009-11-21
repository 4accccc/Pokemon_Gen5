//======================================================================
/**
 * @file	mb_select_sys.h
 * @brief	マルチブート・ポケモン選択
 * @author	ariizumi
 * @data	09/11/19
 *
 * モジュール名：MB_SELECT
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

#include "multiboot/mb_select_sys.h"
#include "multiboot/mb_util_msg.h"
#include "multiboot/mb_poke_icon.h"
#include "multiboot/mb_local_def.h"


//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define MB_SELECT_FRAME_MSG (GFL_BG_FRAME1_M)
#define MB_SELECT_FRAME_BG  (GFL_BG_FRAME3_M)

#define MB_SELECT_FRAME_SUB_BG  (GFL_BG_FRAME3_S)

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
  
}MB_SELECT_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  HEAPID heapId;
  GFL_TCB *vBlankTcb;
  MB_SELECT_INIT_WORK *initWork;
  
  MB_SELECT_STATE  state;
  
  void  *sndData;
  //メッセージ用
  MB_MSG_WORK *msgWork;

  //仮出し
  GFL_CLUNIT  *cellUnit;
  GFL_CLWK    *pokeIcon[MB_CAP_POKE_NUM];
  u32         iconPalRes;
  u32         iconAnmRes;
  u32         iconCharRes[MB_CAP_POKE_NUM];
  
}MB_SELECT_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void MB_SELECT_Init( MB_SELECT_WORK *work );
static void MB_SELECT_Term( MB_SELECT_WORK *work );
static const BOOL MB_SELECT_Main( MB_SELECT_WORK *work );
static void MB_SELECT_VBlankFunc(GFL_TCB *tcb, void *wk );

static void MB_SELECT_InitGraphic( MB_SELECT_WORK *work );
static void MB_SELECT_TermGraphic( MB_SELECT_WORK *work );
static void MB_SELECT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void MB_SELECT_LoadResource( MB_SELECT_WORK *work );

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
//  初期化
//--------------------------------------------------------------
static void MB_SELECT_Init( MB_SELECT_WORK *work )
{
  work->state = MSS_FADEIN;
  
  MB_SELECT_InitGraphic( work );
  MB_SELECT_LoadResource( work );
  work->msgWork = MB_MSG_MessageInit( work->heapId , MB_SELECT_FRAME_MSG );
  work->vBlankTcb = GFUser_VIntr_CreateTCB( MB_SELECT_VBlankFunc , work , 8 );


  
  {
    //仮
    u8 i;
    const DLPLAY_CARD_TYPE type = work->initWork->cardType;
    ARCHANDLE *arcHandle = MB_ICON_GetArcHandle( work->heapId , type );
    
    work->cellUnit = GFL_CLACT_UNIT_Create( 12 , 0, work->heapId );
    GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit );
    
    work->iconPalRes = GFL_CLGRP_PLTT_RegisterComp( arcHandle , 
                              MB_ICON_GetPltResId( type ) , 
                              CLSYS_DRAW_MAIN , 
                              3*32 , work->heapId  );
    work->iconAnmRes = GFL_CLGRP_CELLANIM_Register( arcHandle , 
                              MB_ICON_GetCellResId( type ) , 
                              MB_ICON_GetAnmResId( type ), 
                              work->heapId  );
    
    for( i=0;i<MB_CAP_POKE_NUM;i++ )
    {
      GFL_CLWK_DATA cellInitData;
      cellInitData.pos_x = 32+i*32;
      cellInitData.pos_y = 84;
      cellInitData.anmseq = 0;
      cellInitData.softpri = 0;
      cellInitData.bgpri = 0;

      PPP_Setup( work->initWork->ppp[i] ,
                 GFUser_GetPublicRand0(493)+1 ,
                 GFUser_GetPublicRand0(100)+1 ,
                 PTL_SETUP_ID_AUTO );
               
      work->iconCharRes[i] = GFL_CLGRP_CGR_Register( arcHandle , 
                                    MB_ICON_GetCharResId(work->initWork->ppp[i],type) , 
                                    FALSE , CLSYS_DRAW_MAIN , work->heapId  );
      
      work->pokeIcon[i] = GFL_CLACT_WK_Create( work->cellUnit ,
                work->iconCharRes[i],
                work->iconPalRes,
                work->iconAnmRes,
                &cellInitData ,CLSYS_DRAW_MAIN , work->heapId );

      GFL_CLACT_WK_SetPlttOffs( work->pokeIcon[i] , 
                                MB_ICON_GetPalNumber(work->initWork->ppp[i],type) ,
                                CLWK_PLTTOFFS_MODE_PLTT_TOP );

    }
    GFL_ARC_CloseDataHandle( arcHandle );
  }
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static void MB_SELECT_Term( MB_SELECT_WORK *work )
{
  {
    //仮
    u8 i;
    for( i=0;i<MB_CAP_POKE_NUM;i++ )
    {
      GFL_CLACT_WK_Remove( work->pokeIcon[i] );
      GFL_CLGRP_CGR_Release( work->iconCharRes[i] );
    }
    GFL_CLGRP_PLTT_Release( work->iconPalRes );
    GFL_CLGRP_CELLANIM_Release( work->iconAnmRes );
    GFL_CLACT_UNIT_Delete( work->cellUnit );
  }

  GFL_TCB_DeleteTask( work->vBlankTcb );

  MB_MSG_MessageTerm( work->msgWork );
  MB_SELECT_TermGraphic( work );
}

//--------------------------------------------------------------
//  更新
//--------------------------------------------------------------
static const BOOL MB_SELECT_Main( MB_SELECT_WORK *work )
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
static void MB_SELECT_VBlankFunc(GFL_TCB *tcb, void *wk )
{
  //OBJの更新
  GFL_CLACT_SYS_VBlankFunc();
}

//--------------------------------------------------------------
//  グラフィック系初期化
//--------------------------------------------------------------
static void MB_SELECT_InitGraphic( MB_SELECT_WORK *work )
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

    MB_SELECT_SetupBgFunc( &header_main1 , MB_SELECT_FRAME_MSG , GFL_BG_MODE_TEXT );
    //MB_SELECT_SetupBgFunc( &header_main2 , LTVT_FRAME_CHARA , GFL_BG_MODE_TEXT );
    MB_SELECT_SetupBgFunc( &header_main3 , MB_SELECT_FRAME_BG , GFL_BG_MODE_TEXT );
    MB_SELECT_SetupBgFunc( &header_sub3  , MB_SELECT_FRAME_SUB_BG , GFL_BG_MODE_TEXT );
    
  }

  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
  }
  
}

static void MB_SELECT_TermGraphic( MB_SELECT_WORK *work )
{
  GFL_CLACT_SYS_Delete();
  GFL_BG_FreeBGControl( MB_SELECT_FRAME_MSG );
  GFL_BG_FreeBGControl( MB_SELECT_FRAME_BG );
  GFL_BG_FreeBGControl( MB_SELECT_FRAME_SUB_BG );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}


//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void MB_SELECT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}


//--------------------------------------------------------------
//  リソース読み込み
//--------------------------------------------------------------
static void MB_SELECT_LoadResource( MB_SELECT_WORK *work )
{
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_MB_CHILD , work->heapId );

  //下画面
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_mb_child_gra_bg_sub_NCLR , 
                    PALTYPE_SUB_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_mb_child_gra_bg_sub_NCGR ,
                    MB_SELECT_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_mb_child_gra_bg_sub_NSCR , 
                    MB_SELECT_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );
  
  //上画面
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_mb_child_gra_bg_main_NCLR , 
                    PALTYPE_MAIN_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_mb_child_gra_bg_main_NCGR ,
                    MB_SELECT_FRAME_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_mb_child_gra_bg_main_NSCR , 
                    MB_SELECT_FRAME_BG ,  0 , 0, FALSE , work->heapId );
  
  GFL_ARC_CloseDataHandle( arcHandle );
}



#pragma mark [>proc func
static GFL_PROC_RESULT MB_SELECT_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MB_SELECT_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MB_SELECT_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA MultiBootSelect_ProcData =
{
  MB_SELECT_ProcInit,
  MB_SELECT_ProcMain,
  MB_SELECT_ProcTerm
};

//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT MB_SELECT_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MB_SELECT_WORK *work;
  MB_SELECT_INIT_WORK *initWork;
  
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
    
    work = GFL_PROC_AllocWork( proc, sizeof(MB_SELECT_WORK), parentHeap );
    initWork = GFL_HEAP_AllocClearMemory( HEAPID_MB_BOX , sizeof(MB_SELECT_INIT_WORK) );

    initWork->parentHeap = parentHeap;
    initWork->cardType = CARD_TYPE_DUMMY;
    for( i=0;i<MB_CAP_POKE_NUM;i++ )
    {
      initWork->ppp[i] = GFL_HEAP_AllocClearMemory( parentHeap , POKETOOL_GetPPPWorkSize() );
    }

    work->initWork = initWork;
  }
  else
  {
    initWork = pwk;
    GFL_HEAP_CreateHeap( initWork->parentHeap , HEAPID_MB_BOX, 0x40000 );
    work = GFL_PROC_AllocWork( proc, sizeof(MB_SELECT_WORK), HEAPID_MULTIBOOT );
    work->initWork = pwk;
  }
  

  work->heapId = HEAPID_MB_BOX;
  
  MB_SELECT_Init( work );
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static GFL_PROC_RESULT MB_SELECT_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MB_SELECT_WORK *work = mywk;
  
  MB_SELECT_Term( work );

  if( pwk == NULL )
  {
    u8 i;
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
static GFL_PROC_RESULT MB_SELECT_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MB_SELECT_WORK *work = mywk;
  const BOOL ret = MB_SELECT_Main( work );
  
  if( ret == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}



