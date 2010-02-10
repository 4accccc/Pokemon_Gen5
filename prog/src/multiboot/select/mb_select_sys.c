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
#include "print/global_msg.h"
#include "sound/pm_sndsys.h"
#include "system/wipe.h"
#include "system/net_err.h"
#include "app/app_menu_common.h"
#include "poke_tool/monsno_def.h"

#include "arc_def.h"
#include "mb_select_gra.naix"

#include "multiboot/mb_select_sys.h"
#include "multiboot/mb_util_msg.h"
#include "multiboot/mb_util.h"
#include "multiboot/mb_poke_icon.h"
#include "multiboot/mb_local_def.h"
#include "./mb_sel_local_def.h"
#include "./mb_sel_poke.h"


//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define MB_SELECT_FRAME_MSG (GFL_BG_FRAME0_M)
#define MB_SELECT_FRAME_BAR (GFL_BG_FRAME1_M)
#define MB_SELECT_FRAME_BG  (GFL_BG_FRAME3_M)

#define MB_SELECT_FRAME_SUB_BG  (GFL_BG_FRAME3_S)

#define MB_SEL_TRAY_X (216)
#define MB_SEL_TRAY_Y ( 96)
#define MB_SEL_CHANGE_TRAY_L_X ( 16)
#define MB_SEL_CHANGE_TRAY_R_X (152)
#define MB_SEL_CHANGE_TRAY_Y   ( 20)

//速度を変えるときはちゃんと絵が変わるか確認すること！！！！
#define MB_SEL_CHANGE_PAGE_SPD (8)
#define MB_SEL_CHANGE_DISPLEN (176) //表示されているスクロール幅
#define MB_SEL_CHANGE_PAGE_LEN (MB_SEL_CHANGE_DISPLEN+40+32) //176が表示分の幅 １フレームずつ読むのをずらすため速度*5とアイコン分３２
#define MB_SEL_CHANGE_CNT (MB_SEL_CHANGE_PAGE_LEN/MB_SEL_CHANGE_PAGE_SPD) //176が表示分の幅

#define MB_SEL_RETUN_ICON_CNT (4)
#define MB_SEL_SET_TRAY_ICON_CNT (4)
#define MB_SEL_RETURN_BOX_ICON_CNT (8)

#define MB_SEL_MSG_BOXNAME_LEFT (4)
#define MB_SEL_MSG_BOXNAME_TOP (1)
#define MB_SEL_MSG_BOXNAME_WIDTH (13)
#define MB_SEL_MSG_BOXNAME_HEIGHT (3)
#define MB_SEL_MSG_POKEINFO_LEFT (22)
#define MB_SEL_MSG_POKEINFO_TOP (1)
#define MB_SEL_MSG_POKEINFO_WIDTH (9)
#define MB_SEL_MSG_POKEINFO_HEIGHT (4)

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

  MSS_MAIN,
  MSS_MSG_WAIT,
  
  MSS_CHANGE_PAGE,
  MSS_CONFIRM_INIT,
  MSS_CONFIRM_WAIT,
  MSS_CONFIRM_RETURN,
  
}MB_SELECT_STATE;

typedef enum
{
  MSD_LEFT  ,
  MSD_RIGHT ,
  MSD_MAX   ,
  MSD_NONE   ,
}MB_SEL_DIR;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _MB_SELECT_WORK
{
  HEAPID heapId;
  GFL_TCB *vBlankTcb;
  MB_SELECT_INIT_WORK *initWork;
  
  ARCHANDLE *iconArcHandle;
  
  MB_SELECT_STATE  state;
  BOOL isNetErr;
  
  u8         boxPage;
  u16        changePageCnt;
  MB_SEL_DIR pageMoveDir;
  
  BOOL isHold;
  MB_SEL_POKE *holdPoke;
  int   befTpx,befTpy;

  //確認画面用
  u16 anmCnt;
  
  //メッセージ用
  MB_MSG_WORK *msgWork;
  BOOL isUpdateBoxName;
  BOOL isUpdatePokeInfo;
  GFL_BMPWIN  *msgBoxName;
  GFL_BMPWIN  *msgPokeInfo;

  MB_SEL_POKE *boxPoke[MB_POKE_BOX_POKE];
  MB_SEL_POKE *selPoke[MB_CAP_POKE_NUM];
  MB_SEL_POKE *boxHoldPoke;

  GFL_CLUNIT  *cellUnit;
  u32         cellResIdx[MSCR_MAX];
  GFL_CLWK    *clwkTray;
  GFL_CLWK    *clwkChangeTray[MSD_MAX];
  GFL_CLWK    *clwkRetIcon;
  GFL_CLWK    *clwkMark[POKEPARA_MARKING_ELEMS_MAX];

  GFL_CLWK    *exitWaitCell;  //終了時アニメ終了を待つセル
};


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
static void MB_SELECT_LoadResource( MB_SELECT_WORK *work , ARCHANDLE *arcHandle );
static void MB_SELECT_ReleaseResource( MB_SELECT_WORK *work );
static void MB_SELECT_InitCell( MB_SELECT_WORK *work );
static void MB_SELECT_TermCell( MB_SELECT_WORK *work );

static void MB_SELECT_UpdateUI( MB_SELECT_WORK *work );
static const BOOL MB_SELECT_CheckDropTray( MB_SELECT_WORK *work );
static const BOOL MB_SELECT_CheckDropBox( MB_SELECT_WORK *work );
static void MB_SELECT_UpdateChangePage( MB_SELECT_WORK *work );

static void MB_SELECT_SetBoxName( MB_SELECT_WORK *work , const u8 idx );
static void MB_SELECT_SetPokeInfo( MB_SELECT_WORK *work , const POKEMON_PASO_PARAM *ppp );
static void MB_SELECT_ClearPokeInfo( MB_SELECT_WORK *work );

static void MB_SELECT_InitComfirm( MB_SELECT_WORK *work );
static void MB_SELECT_UpdateInitComfirm( MB_SELECT_WORK *work );
static void MB_SELECT_UpdateConfirm( MB_SELECT_WORK *work );
static void MB_SELECT_UpdateReturnComfirm( MB_SELECT_WORK *work );

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
  u8 i;
  //アイコンでもダミーの取得で使うので外でハンドル確保
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_MB_SELECT , work->heapId );

  MB_SELECT_InitGraphic( work );
  MB_SELECT_LoadResource( work , arcHandle );
  MB_SELECT_InitCell( work );
  
#ifndef MULTI_BOOT_MAKE
  work->msgWork = MB_MSG_MessageInit( work->heapId , MB_SELECT_FRAME_MSG , MB_SELECT_FRAME_MSG , NARC_message_multiboot_child_dat , TRUE );
#else
  work->msgWork = MB_MSG_MessageInit( work->heapId , MB_SELECT_FRAME_MSG , MB_SELECT_FRAME_MSG , NARC_message_dl_multiboot_child_dat , TRUE );
#endif
  work->vBlankTcb = GFUser_VIntr_CreateTCB( MB_SELECT_VBlankFunc , work , 8 );

  //ポケモン設定
  {
    MB_SEL_POKE_INIT_WORK initWork;

    initWork.heapId = work->heapId;
    initWork.arcHandle = arcHandle;
    initWork.iconType  = MSPT_BOX;
    initWork.cellUnit  = work->cellUnit;
    initWork.palResIdx = work->cellResIdx[MSCR_PLT_POKEICON];
    initWork.anmResIdx = work->cellResIdx[MSCR_ANM_POKEICON];
    for( i=0;i<MB_POKE_BOX_POKE;i++ )
    {
      initWork.idx = i;
      work->boxPoke[i] = MB_SEL_POKE_CreateWork( work , &initWork );
    }
    initWork.iconType  = MSPT_TRAY;
    for( i=0;i<MB_CAP_POKE_NUM;i++ )
    {
      initWork.idx = i;
      work->selPoke[i] = MB_SEL_POKE_CreateWork( work , &initWork );
    }
    initWork.iconType  = MSPT_BOX;
    work->boxHoldPoke = MB_SEL_POKE_CreateWork( work , &initWork );
  }

  work->msgBoxName = GFL_BMPWIN_Create( MB_SELECT_FRAME_BAR , 
                                MB_SEL_MSG_BOXNAME_LEFT , 
                                MB_SEL_MSG_BOXNAME_TOP ,
                                MB_SEL_MSG_BOXNAME_WIDTH , 
                                MB_SEL_MSG_BOXNAME_HEIGHT , 
                                MB_MSG_PLT_MAIN_FONT ,
                                GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_TransVramCharacter( work->msgBoxName );
  GFL_BMPWIN_MakeScreen( work->msgBoxName );
  work->msgPokeInfo = GFL_BMPWIN_Create( MB_SELECT_FRAME_MSG , 
                                MB_SEL_MSG_POKEINFO_LEFT , 
                                MB_SEL_MSG_POKEINFO_TOP ,
                                MB_SEL_MSG_POKEINFO_WIDTH , 
                                MB_SEL_MSG_POKEINFO_HEIGHT , 
                                MB_MSG_PLT_MAIN_FONT ,
                                GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_TransVramCharacter( work->msgPokeInfo );
  GFL_BMPWIN_MakeScreen( work->msgPokeInfo );
  GFL_BG_LoadScreenReq(MB_SELECT_FRAME_BAR);
  GFL_BG_LoadScreenReq(MB_SELECT_FRAME_MSG);
  work->isUpdateBoxName = FALSE;
  work->isUpdatePokeInfo = FALSE;

  //最初のBOXの設定
  for( i=0;i<MB_POKE_BOX_POKE;i++ )
  {
    MB_SEL_POKE_SetPPP( work , work->boxPoke[i] , work->initWork->boxData[0][i] );
  }
  MB_SELECT_SetBoxName( work , 0 );

  for( i=0;i<MB_CAP_POKE_NUM;i++ )
  {
    work->initWork->selectPoke[i][0] = 0xFF;
    work->initWork->selectPoke[i][1] = 0xFF;
  }
  work->initWork->isCancel = FALSE;

  work->isNetErr = FALSE;
  work->state = MSS_FADEIN;
  work->pageMoveDir = MSD_NONE;
  work->boxPage = 0;
  work->isHold = FALSE;
  work->holdPoke = NULL;
  work->exitWaitCell = NULL;
  
  GFL_ARC_CloseDataHandle( arcHandle );
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static void MB_SELECT_Term( MB_SELECT_WORK *work )
{
  u8 i;

  GFL_BMPWIN_Delete( work->msgPokeInfo );
  GFL_BMPWIN_Delete( work->msgBoxName );

  MB_SEL_POKE_DeleteWork( work , work->boxHoldPoke );
  for( i=0;i<MB_CAP_POKE_NUM;i++ )
  {
    MB_SEL_POKE_DeleteWork( work , work->selPoke[i] );
  }
  for( i=0;i<MB_POKE_BOX_POKE;i++ )
  {
    MB_SEL_POKE_DeleteWork( work , work->boxPoke[i] );
  }

  GFL_TCB_DeleteTask( work->vBlankTcb );

  MB_MSG_MessageTerm( work->msgWork );
  MB_SELECT_TermCell( work );
  MB_SELECT_ReleaseResource( work );
  MB_SELECT_TermGraphic( work );
}

//--------------------------------------------------------------
//  更新
//--------------------------------------------------------------
static const BOOL MB_SELECT_Main( MB_SELECT_WORK *work )
{
  u8 i;
  BOOL isMovePoke = FALSE;

  //操作可能かチェックするために、先にPOKE更新
  if( work->state != MSS_MSG_WAIT )
  {
    for( i=0;i<MB_POKE_BOX_POKE;i++ )
    {
      if( MB_SEL_POKE_UpdateWork( work , work->boxPoke[i] ) == TRUE )
      {
        isMovePoke = TRUE;
      }
    }
    for( i=0;i<MB_CAP_POKE_NUM;i++ )
    {
      if( MB_SEL_POKE_UpdateWork( work , work->selPoke[i] ) == TRUE )
      {
        isMovePoke = TRUE;
      }
    }
    if( MB_SEL_POKE_UpdateWork( work , work->boxHoldPoke ) == TRUE )
    {
      isMovePoke = TRUE;
    }
  }


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
      work->state = MSS_MAIN;
    }
    break;

  case MSS_FADEOUT:
    if( work->exitWaitCell == NULL ||
        GFL_CLACT_WK_CheckAnmActive( work->exitWaitCell ) == FALSE )
    {
      WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
      work->state = MSS_WAIT_FADEOUT;
    }
    break;
    
  case MSS_WAIT_FADEOUT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return TRUE;
    }
    break;
  
  case MSS_MAIN:
    if( isMovePoke == FALSE )
    {
      MB_SELECT_UpdateUI( work );
    }
    break;
    
  case MSS_MSG_WAIT:
    if( MB_MSG_CheckPrintStreamIsFinish( work->msgWork ) == TRUE )
    {
      if( GFL_UI_TP_GetTrg() == TRUE )
      {
        MB_MSG_MessageHide( work->msgWork );
        work->state = MSS_MAIN;
      }
    }
    break;

  case MSS_CHANGE_PAGE:
    MB_SELECT_UpdateChangePage( work );
    break;

  case MSS_CONFIRM_INIT:
    MB_SELECT_UpdateInitComfirm( work );
    break;

  case MSS_CONFIRM_WAIT:
    MB_SELECT_UpdateConfirm( work );
    break;

  case MSS_CONFIRM_RETURN:
    MB_SELECT_UpdateReturnComfirm( work );
    break;
  }

  //OBJの更新
  GFL_CLACT_SYS_Main();

  MB_MSG_MessageMain( work->msgWork );
  if( work->isUpdateBoxName == TRUE )
  {
    if( MB_MSG_CheckPrintQueIsFinish( work->msgWork ) == TRUE )
    {
      work->isUpdateBoxName = FALSE;
      GFL_BMPWIN_TransVramCharacter( work->msgBoxName );
    }
  }
  if( work->isUpdatePokeInfo == TRUE )
  {
    if( MB_MSG_CheckPrintQueIsFinish( work->msgWork ) == TRUE )
    {
      work->isUpdatePokeInfo = FALSE;
      GFL_BMPWIN_TransVramCharacter( work->msgPokeInfo );
    }
  }


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
    // BG0 MAIN (メッセージ
    static const GFL_BG_BGCNT_HEADER header_main0 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000,0x6000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG1 MAIN (バー
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x10000,0x08000,
      GX_BG_EXTPLTT_23, 2, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
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

    MB_SELECT_SetupBgFunc( &header_main0 , MB_SELECT_FRAME_MSG , GFL_BG_MODE_TEXT );
    MB_SELECT_SetupBgFunc( &header_main1 , MB_SELECT_FRAME_BAR , GFL_BG_MODE_TEXT );
    MB_SELECT_SetupBgFunc( &header_main3 , MB_SELECT_FRAME_BG , GFL_BG_MODE_TEXT );
    MB_SELECT_SetupBgFunc( &header_sub3  , MB_SELECT_FRAME_SUB_BG , GFL_BG_MODE_TEXT );
   
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_NONE , GX_BLEND_PLANEMASK_BG3 , 8 , 8 );
  }

  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    cellSysInitData.CGR_RegisterMax = 64;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    work->cellUnit = GFL_CLACT_UNIT_Create( 48 , 0, work->heapId );
    GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit );

    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
  }
  
}

static void MB_SELECT_TermGraphic( MB_SELECT_WORK *work )
{
  GFL_CLACT_UNIT_Delete( work->cellUnit );
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
static void MB_SELECT_LoadResource( MB_SELECT_WORK *work , ARCHANDLE *arcHandle )
{

  //下画面(MAIN
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_mb_select_gra_box_bgd_NCLR , 
                    PALTYPE_MAIN_BG , MB_SEL_PLT_BG_COMMON*32 , 32 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_mb_select_gra_box_bgd_NCGR ,
                    MB_SELECT_FRAME_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_mb_select_gra_box_bgd_NSCR , 
                    MB_SELECT_FRAME_BG ,  0 , 0, FALSE , work->heapId );
  
  //上画面(SUB
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_mb_select_gra_box_bgd_NCLR , 
                    PALTYPE_SUB_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_mb_select_gra_box_bgd_NCGR ,
                    MB_SELECT_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_mb_select_gra_box_bgd_NSCR , 
                    MB_SELECT_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );

  
  //下画面OBJ
  work->cellResIdx[MSCR_PLT_COMMON] = GFL_CLGRP_PLTT_RegisterComp( arcHandle , 
        NARC_mb_select_gra_box_obj_NCLR , CLSYS_DRAW_MAIN , 
        MB_SEL_PLT_OBJ_COMMON*32 , work->heapId  );
  work->cellResIdx[MSCR_NCG_COMMON] = GFL_CLGRP_CGR_Register( arcHandle , 
        NARC_mb_select_gra_box_obj_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  work->cellResIdx[MSCR_ANM_COMMON] = GFL_CLGRP_CELLANIM_Register( arcHandle , 
        NARC_mb_select_gra_box_obj_NCER , NARC_mb_select_gra_box_obj_NANR, work->heapId  );
  
  //ポケアイコン素材
  {
    const DLPLAY_CARD_TYPE type = work->initWork->cardType;
    work->iconArcHandle = MB_ICON_GetArcHandle( work->heapId , type );
    OS_TPrintf("[%d]\n",GFL_ARC_GetDataFileCntByHandle(work->iconArcHandle));
    if( type == CARD_TYPE_DUMMY )
    {
      work->cellResIdx[MSCR_PLT_POKEICON] = GFL_CLGRP_PLTT_RegisterComp( work->iconArcHandle , 
                                  MB_ICON_GetPltResId( type ) , 
                                  CLSYS_DRAW_MAIN , 
                                  MB_SEL_PLT_OBJ_POKEICON*32 , 
                                  work->heapId  );
    }
    else
    {
      
      work->cellResIdx[MSCR_PLT_POKEICON] = GFL_CLGRP_PLTT_Register( work->iconArcHandle , 
                                  MB_ICON_GetPltResId( type ) , 
                                  CLSYS_DRAW_MAIN , 
                                  MB_SEL_PLT_OBJ_POKEICON*32 , 
                                  work->heapId  );
      
      /*
      void* loadPtr = GFL_ARC_LoadDataAllocByHandle(  work->iconArcHandle ,
                              MB_ICON_GetPltResId( type ),
                              GFL_HEAP_LOWID(work->heapId) );
      NNSG2dPaletteData*  palData;
      GF_ASSERT_MSG( 0 , "Test[%d][%d]\n", GFL_ARC_GetDataFileCntByHandle(work->iconArcHandle)
                                     , GFL_ARC_GetDataSizeByHandle(work->iconArcHandle,MB_ICON_GetPltResId( type )) );
      NNS_G2dGetUnpackedPaletteData( loadPtr, &palData );
      GF_ASSERT_MSG( 0 , "Test[%d]\n",palData->szByte );
      */
    }
    work->cellResIdx[MSCR_ANM_POKEICON] = GFL_CLGRP_CELLANIM_Register( work->iconArcHandle , 
                                MB_ICON_GetCellResId( type ) , 
                                MB_ICON_GetAnmResId( type ), 
                                work->heapId  );
  }
  
  //共通素材
  {
    ARCHANDLE *commonArcHandle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId() , work->heapId );
    
    //バー
    GFL_ARCHDL_UTIL_TransVramPalette( commonArcHandle , APP_COMMON_GetBarPltArcIdx() , 
                      PALTYPE_MAIN_BG , MB_SEL_PLT_BG_BAR*32 , 32 , work->heapId );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( commonArcHandle , APP_COMMON_GetBarCharArcIdx() ,
                      MB_SELECT_FRAME_BAR , 0 , 0, FALSE , work->heapId );
    GFL_ARCHDL_UTIL_TransVramScreen( commonArcHandle , APP_COMMON_GetBarScrnArcIdx() , 
                      MB_SELECT_FRAME_BAR , 0 , 0, FALSE , work->heapId );
    GFL_BG_ChangeScreenPalette( MB_SELECT_FRAME_BAR , 0 , 21 , 32 , 3 , MB_SEL_PLT_BG_BAR );
    GFL_BG_LoadScreenReq( MB_SELECT_FRAME_BAR );

    //バーアイコン
    work->cellResIdx[MSCR_PLT_BAR_ICON] = GFL_CLGRP_PLTT_RegisterEx( commonArcHandle , 
          APP_COMMON_GetBarIconPltArcIdx() , CLSYS_DRAW_MAIN , 
          MB_SEL_PLT_OBJ_BAR_BUTTON*32 , 0 , 
          APP_COMMON_BARICON_PLT_NUM , work->heapId  );
    work->cellResIdx[MSCR_NCG_BAR_ICON] = GFL_CLGRP_CGR_Register( commonArcHandle , 
          APP_COMMON_GetBarIconCharArcIdx() , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
    work->cellResIdx[MSCR_ANM_BAR_ICON] = GFL_CLGRP_CELLANIM_Register( commonArcHandle , 
          APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_128K) , 
          APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_128K), 
          work->heapId  );

    //ポケマーク
    work->cellResIdx[MSCR_PLT_POKE_MARK] = GFL_CLGRP_PLTT_RegisterEx( commonArcHandle , 
          APP_COMMON_GetPokeMarkPltArcIdx() , CLSYS_DRAW_MAIN , 
          MB_SEL_PLT_OBJ_POKE_MARK*32 , 0 , 
          APP_COMMON_BARICON_PLT_NUM , work->heapId  );
    work->cellResIdx[MSCR_NCG_POKE_MARK] = GFL_CLGRP_CGR_Register( commonArcHandle , 
          APP_COMMON_GetPokeMarkCharArcIdx(APP_COMMON_MAPPING_128K) , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
    work->cellResIdx[MSCR_ANM_POKE_MARK] = GFL_CLGRP_CELLANIM_Register( commonArcHandle , 
          APP_COMMON_GetPokeMarkCellArcIdx(APP_COMMON_MAPPING_128K) , 
          APP_COMMON_GetPokeMarkAnimeArcIdx(APP_COMMON_MAPPING_128K), 
          work->heapId  );

    GFL_ARC_CloseDataHandle( commonArcHandle );
  }
}

//--------------------------------------------------------------
//  リソース破棄
//--------------------------------------------------------------
static void MB_SELECT_ReleaseResource( MB_SELECT_WORK *work )
{
  GFL_CLGRP_PLTT_Release( work->cellResIdx[MSCR_PLT_POKEICON] );
  GFL_CLGRP_CELLANIM_Release( work->cellResIdx[MSCR_ANM_POKEICON] );
  
  GFL_CLGRP_PLTT_Release( work->cellResIdx[MSCR_PLT_COMMON] );
  GFL_CLGRP_CGR_Release( work->cellResIdx[MSCR_NCG_COMMON] );
  GFL_CLGRP_CELLANIM_Release( work->cellResIdx[MSCR_ANM_COMMON] );
  
  GFL_CLGRP_PLTT_Release( work->cellResIdx[MSCR_PLT_BAR_ICON] );
  GFL_CLGRP_CGR_Release( work->cellResIdx[MSCR_NCG_BAR_ICON] );
  GFL_CLGRP_CELLANIM_Release( work->cellResIdx[MSCR_ANM_BAR_ICON] );
  
  GFL_CLGRP_PLTT_Release( work->cellResIdx[MSCR_PLT_POKE_MARK] );
  GFL_CLGRP_CGR_Release( work->cellResIdx[MSCR_NCG_POKE_MARK] );
  GFL_CLGRP_CELLANIM_Release( work->cellResIdx[MSCR_ANM_POKE_MARK] );
  
  GFL_ARC_CloseDataHandle( work->iconArcHandle );
}

//--------------------------------------------------------------
//  セルの初期化
//--------------------------------------------------------------
static void MB_SELECT_InitCell( MB_SELECT_WORK *work )
{
  u8 i;
  GFL_CLWK_DATA cellInitData;
  cellInitData.pos_x = MB_SEL_TRAY_X;
  cellInitData.pos_y = MB_SEL_TRAY_Y;
  cellInitData.anmseq = 3;
  cellInitData.softpri = 8;
  cellInitData.bgpri = 2;
  
  work->clwkTray = GFL_CLACT_WK_Create( work->cellUnit ,
            work->cellResIdx[MSCR_NCG_COMMON],
            work->cellResIdx[MSCR_PLT_COMMON],
            work->cellResIdx[MSCR_ANM_COMMON],
            &cellInitData ,CLSYS_DRAW_MAIN , work->heapId );

  GFL_CLACT_WK_SetDrawEnable( work->clwkTray , TRUE );
  
  cellInitData.pos_x = MB_SEL_CHANGE_TRAY_L_X;
  cellInitData.pos_y = MB_SEL_CHANGE_TRAY_Y;
  cellInitData.anmseq = 0;

  work->clwkChangeTray[0] = GFL_CLACT_WK_Create( work->cellUnit ,
            work->cellResIdx[MSCR_NCG_COMMON],
            work->cellResIdx[MSCR_PLT_COMMON],
            work->cellResIdx[MSCR_ANM_COMMON],
            &cellInitData ,CLSYS_DRAW_MAIN , work->heapId );
  
  cellInitData.pos_x = MB_SEL_CHANGE_TRAY_R_X;
  cellInitData.pos_y = MB_SEL_CHANGE_TRAY_Y;
  cellInitData.anmseq = 1;

  work->clwkChangeTray[1] = GFL_CLACT_WK_Create( work->cellUnit ,
            work->cellResIdx[MSCR_NCG_COMMON],
            work->cellResIdx[MSCR_PLT_COMMON],
            work->cellResIdx[MSCR_ANM_COMMON],
            &cellInitData ,CLSYS_DRAW_MAIN , work->heapId );
  
  cellInitData.pos_x = 256-24;
  cellInitData.pos_y = 192-24;
  cellInitData.anmseq = APP_COMMON_BARICON_EXIT;
  cellInitData.bgpri = 2;

  work->clwkRetIcon = GFL_CLACT_WK_Create( work->cellUnit ,
            work->cellResIdx[MSCR_NCG_BAR_ICON],
            work->cellResIdx[MSCR_PLT_BAR_ICON],
            work->cellResIdx[MSCR_ANM_BAR_ICON],
            &cellInitData ,CLSYS_DRAW_MAIN , work->heapId );
  
  for( i=0;i<POKEPARA_MARKING_ELEMS_MAX;i++ )
  {
    cellInitData.pos_x = 256-80 + i*8;
    cellInitData.pos_y = 40;
    cellInitData.anmseq = APP_COMMON_POKE_MARK_CIRCLE_WHITE + i*2;
    cellInitData.bgpri = 1;
    work->clwkMark[i] = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellResIdx[MSCR_NCG_POKE_MARK],
              work->cellResIdx[MSCR_PLT_POKE_MARK],
              work->cellResIdx[MSCR_ANM_POKE_MARK],
              &cellInitData ,CLSYS_DRAW_MAIN , work->heapId );
    GFL_CLACT_WK_SetDrawEnable( work->clwkMark[i] , FALSE );
  }
}

//--------------------------------------------------------------
//  セルの開放
//--------------------------------------------------------------
static void MB_SELECT_TermCell( MB_SELECT_WORK *work )
{
  u8 i;
  for( i=0;i<POKEPARA_MARKING_ELEMS_MAX;i++ )
  {
    GFL_CLACT_WK_Remove( work->clwkMark[i] );
  }

  GFL_CLACT_WK_Remove( work->clwkRetIcon );
  GFL_CLACT_WK_Remove( work->clwkChangeTray[1] );
  GFL_CLACT_WK_Remove( work->clwkChangeTray[0] );
  GFL_CLACT_WK_Remove( work->clwkTray );
}

//--------------------------------------------------------------
//  操作周り更新
//--------------------------------------------------------------
static void MB_SELECT_UpdateUI( MB_SELECT_WORK *work )
{
  //ボタンとの判定
  GFL_UI_TP_HITTBL hitTbl[] =
  {
    { MB_SEL_CHANGE_TRAY_Y-12 , 
      MB_SEL_CHANGE_TRAY_Y+12 , 
      MB_SEL_CHANGE_TRAY_L_X-12 , 
      MB_SEL_CHANGE_TRAY_L_X+12 },
    { MB_SEL_CHANGE_TRAY_Y-12 , 
      MB_SEL_CHANGE_TRAY_Y+12 , 
      MB_SEL_CHANGE_TRAY_R_X-12 , 
      MB_SEL_CHANGE_TRAY_R_X+12 },
    { MB_SEL_POKE_BOX_TOP-8 , 
      MB_SEL_POKE_BOX_TOP+MB_SEL_POKE_BOX_HEIGHT*(MB_SEL_POKE_BOX_Y_NUM-1)+16 ,
      MB_SEL_POKE_BOX_LEFT-16 , 
      MB_SEL_POKE_BOX_LEFT+MB_SEL_POKE_BOX_WIDTH*(MB_SEL_POKE_BOX_X_NUM-1)+16 },
    { 192-24 , 
      192 ,
      256-24 , 
      255 },
    { GFL_UI_TP_HIT_END ,0,0,0 },
  };
  
  u32 tpx,tpy;
  u32 holdX,holdY;
  GFL_UI_TP_GetPointTrg( &tpx,&tpy );
  GFL_UI_TP_GetPointCont( &holdX,&holdY );
  if( work->isHold == FALSE )
  {
    const int ret = GFL_UI_TP_HitTrg( hitTbl );
    
    if( ret != GFL_UI_TP_HIT_NONE )
    {
      //ページ切り替えボタン
      if( ret == 0 )
      {
        //MB_TPrintf("LEFT\n");
        work->pageMoveDir = MSD_LEFT;
        work->state = MSS_CHANGE_PAGE;
        work->changePageCnt = 0;
        if( work->boxPage == 0 )
        {
          MB_SELECT_SetBoxName( work , MB_POKE_BOX_TRAY-1 );
        }
        else
        {
          MB_SELECT_SetBoxName( work , work->boxPage - 1 );
        }
        GFL_CLACT_WK_SetAnmSeq( work->clwkChangeTray[0] , 5 );
        GFL_CLACT_WK_SetAutoAnmFlag( work->clwkChangeTray[0] , TRUE );
      }
      else
      if( ret == 1 )
      {
        //MB_TPrintf("RIGHT\n");
        work->pageMoveDir = MSD_RIGHT;
        work->state = MSS_CHANGE_PAGE;
        work->changePageCnt = 0;
        if( work->boxPage == MB_POKE_BOX_TRAY-1 )
        {
          MB_SELECT_SetBoxName( work , 0 );
        }
        else
        {
          MB_SELECT_SetBoxName( work , work->boxPage + 1 );
        }
        GFL_CLACT_WK_SetAnmSeq( work->clwkChangeTray[1] , 6 );
        GFL_CLACT_WK_SetAutoAnmFlag( work->clwkChangeTray[1] , TRUE );
      }
      else
      if( ret == 2 )
      {
        //ボックス内
        const u8 idxX = (tpx-(MB_SEL_POKE_BOX_LEFT-16))/MB_SEL_POKE_BOX_WIDTH;
        const u8 idxY = (tpy-(MB_SEL_POKE_BOX_TOP - 8))/MB_SEL_POKE_BOX_HEIGHT;
        const u8 idx  = idxX + idxY*MB_SEL_POKE_BOX_X_NUM;
        
        if( MB_SEL_POKE_isValid( work->boxPoke[idx] ) == TRUE &&
            MB_SEL_POKE_GetAlpha( work , work->boxPoke[idx] ) == FALSE )
        {
          GFL_CLACTPOS cellPos;
          cellPos.x = tpx;
          cellPos.y = tpy;

          work->isHold = TRUE;
          //work->holdPoke = work->boxPoke[idx];
          //MB_SEL_POKE_SetPri( work , work->boxPoke[idx] , MSPT_HOLD );
          //MB_SELECT_SetPokeInfo( work , work->initWork->boxData[work->boxPage][idx] );
          work->holdPoke = work->boxHoldPoke;
          MB_SEL_POKE_SetIdx( work->holdPoke , idx );
          MB_SEL_POKE_SetPPP( work , work->holdPoke , work->initWork->boxData[work->boxPage][idx] );
          MB_SEL_POKE_SetPri( work , work->holdPoke , MSPT_HOLD );
          MB_SEL_POKE_SetPos( work , work->holdPoke , &cellPos );
          MB_SELECT_SetPokeInfo( work , work->initWork->boxData[work->boxPage][idx] );
          
          MB_SEL_POKE_SetAlpha( work , work->boxPoke[idx] , TRUE );
        }
      }
      else
      if( ret == 3 )
      {
        //キャンセルボタン
        GFL_CLACT_WK_SetAnmSeq( work->clwkRetIcon , APP_COMMON_BARICON_EXIT_ON );
        GFL_CLACT_WK_SetAutoAnmFlag( work->clwkRetIcon , TRUE );
        work->state = MSS_FADEOUT;
        work->exitWaitCell = work->clwkRetIcon;
        //TODO 確認MSG？
        work->initWork->isCancel = TRUE;
      }
      work->befTpx = tpx;
      work->befTpy = tpy;
    }
    else
    {
      u8 i;
      int trayRet;
      //トレーとの当たり判定
      GFL_UI_TP_HITTBL trayHitTbl[7];

      //判定の作成
      for( i=0;i<MB_CAP_POKE_NUM;i++ )
      {
        trayHitTbl[i].rect.top = MB_SEL_POKE_GetPosY( work->selPoke[i] ) - 8;
        trayHitTbl[i].rect.bottom = trayHitTbl[i].rect.top + 24;
        trayHitTbl[i].rect.left = MB_SEL_POKE_GetPosX( work->selPoke[i] ) - 16;
        trayHitTbl[i].rect.right = trayHitTbl[i].rect.left + 32;
      }
      trayHitTbl[6].circle.code = GFL_UI_TP_HIT_END;
      trayRet = GFL_UI_TP_HitTrg( trayHitTbl );

      if( trayRet != GFL_UI_TP_HIT_NONE )
      {
        if( MB_SEL_POKE_isValid( work->selPoke[trayRet] ) == TRUE )
        {
          work->isHold = TRUE;
          work->holdPoke = work->selPoke[trayRet];
          MB_SEL_POKE_SetPri( work , work->selPoke[trayRet] , MSPT_HOLD );
          MB_SELECT_SetPokeInfo( work , work->initWork->boxData[work->initWork->selectPoke[trayRet][0]][work->initWork->selectPoke[trayRet][1]] );
        }
      }
    }
  }
  else
  {
    //Hold中
    if( GFL_UI_TP_GetCont() == FALSE )
    {
      if( MB_SELECT_CheckDropTray( work ) == FALSE )
      {
        if( MB_SELECT_CheckDropBox( work ) == FALSE )
        {
          //戻る
          const int returnPosX = MB_SEL_POKE_GetPosX( work->holdPoke );
          const int returnPosY = MB_SEL_POKE_GetPosY( work->holdPoke );
          
          if( work->holdPoke == work->boxHoldPoke )
          {
            MB_SEL_POKE_SetMove( work , work->holdPoke , work->befTpx , work->befTpy ,
                                 returnPosX , returnPosY , MB_SEL_RETUN_ICON_CNT , TRUE );
            MB_SEL_POKE_ResetAlphaCnt( work , work->boxPoke[MB_SEL_POKE_GetIdx(work->holdPoke)] , MB_SEL_RETUN_ICON_CNT );
          }
          else
          {
            MB_SEL_POKE_SetMove( work , work->holdPoke , work->befTpx , work->befTpy ,
                                 returnPosX , returnPosY , MB_SEL_RETUN_ICON_CNT , FALSE );
          }

          //トレーの上を通るので例外的に表示優先を変える
          MB_SEL_POKE_SetPri( work , work->holdPoke , MSPT_TRAY );
        }
      }
      else
      {
        if( work->initWork->selectPoke[5][0] != 0xFF )
        {
          //6人決まった！
          MB_SELECT_InitComfirm( work );
        }
      }
      work->isHold = FALSE;
      work->holdPoke = NULL;
      MB_SELECT_ClearPokeInfo( work );
    }
    else
    {
      GFL_CLACTPOS cellPos;
      cellPos.x = holdX;
      cellPos.y = holdY;
      MB_SEL_POKE_SetPos( work , work->holdPoke , &cellPos );
      
      work->befTpx = holdX;
      work->befTpy = holdY;
    }
  }
}
//--------------------------------------------------------------
//  アイコンをトレーに置くチェック＋処理
//--------------------------------------------------------------
static const BOOL MB_SELECT_CheckDropTray( MB_SELECT_WORK *work )
{
  //範囲チェック
  if( work->befTpx > MB_SEL_POKE_TRAY_LEFT-16 &&
      work->befTpx < MB_SEL_POKE_TRAY_LEFT + MB_SEL_POKE_TRAY_WIDTH*(MB_SEL_POKE_TRAY_X_NUM-1) + 16 &&
      work->befTpy > MB_SEL_POKE_TRAY_TOP - 16 &&
      work->befTpy < MB_SEL_POKE_TRAY_TOP + MB_SEL_POKE_TRAY_HEIGHT*(MB_SEL_POKE_TRAY_Y_NUM-1) + 16 + MB_SEL_POKE_TRAY_OFS )
  {
    if( MB_SEL_POKE_GetType( work->holdPoke ) == MSPT_BOX )
    {
      u8 i;
      const u8 idx = MB_SEL_POKE_GetIdx( work->holdPoke );
      POKEMON_PASO_PARAM *ppp = work->initWork->boxData[work->boxPage][idx];
      //TODO 置けるか？チェック！！
      const MB_UTIL_CHECK_PLAY_RET checkRet = MB_UTIL_CheckPlay_PalGate( ppp , work->initWork->cardType );
      
      if( checkRet == MUCPR_OK )
      {
        for( i=0;i<MB_CAP_POKE_NUM;i++ )
        {
          if( work->initWork->selectPoke[i][0] == 0xFF )
          {
            break;
          }
        }
        if( i == MB_CAP_POKE_NUM )
        {
          return FALSE;
        }
        
        work->initWork->selectPoke[i][0] = work->boxPage;
        work->initWork->selectPoke[i][1] = idx;
        
        MB_SEL_POKE_SetPPP( work , work->holdPoke , NULL );
        MB_SEL_POKE_SetPPP( work , work->selPoke[i] , ppp );
        MB_SEL_POKE_SetMove( work , work->selPoke[i] , 
                             work->befTpx , work->befTpy ,
                             MB_SEL_POKE_GetPosX( work->selPoke[i] ),
                             MB_SEL_POKE_GetPosY( work->selPoke[i] ),
                             MB_SEL_SET_TRAY_ICON_CNT , FALSE );
        return TRUE;
      }
      else
      if( checkRet == MUCPR_EGG )
      {
        MB_MSG_MessageCreateWindow( work->msgWork , MMWT_2LINE );
        MB_MSG_MessageDispNoWait( work->msgWork , MSG_MB_CHILD_SEL_02 );
        work->state = MSS_MSG_WAIT;
      }
      else
      if( checkRet == MUCPR_HIDEN )
      {
        MB_MSG_MessageCreateWindow( work->msgWork , MMWT_2LINE );
        MB_MSG_MessageDispNoWait( work->msgWork , MSG_MB_CHILD_SEL_03 );
        work->state = MSS_MSG_WAIT;
      }
      else
      if( checkRet == MUCPR_GIZAMIMI )
      {
        MB_MSG_MessageCreateWindow( work->msgWork , MMWT_2LINE_TALK );
        MB_MSG_MessageDisp( work->msgWork , MSG_MB_CHILD_SEL_06 , work->initWork->msgSpeed );
        MB_MSG_SetDispKeyCursor( work->msgWork , TRUE );
        work->state = MSS_MSG_WAIT;
      }
      else
      if( checkRet == MUCPR_FUSEI )
      {
        MB_MSG_MessageCreateWindow( work->msgWork , MMWT_2LINE );
        MB_MSG_MessageDispNoWait( work->msgWork , MSG_MB_CHILD_SEL_04 );
        work->state = MSS_MSG_WAIT;
      }
    }
  }
  return FALSE;
}

//--------------------------------------------------------------
//  アイコンをボックスに置くチェック＋処理
//--------------------------------------------------------------
static const BOOL MB_SELECT_CheckDropBox( MB_SELECT_WORK *work )
{
  //範囲チェック
  if( work->befTpx > MB_SEL_POKE_BOX_TOP-16 &&
      work->befTpx < MB_SEL_POKE_BOX_TOP+MB_SEL_POKE_BOX_HEIGHT*(MB_SEL_POKE_BOX_Y_NUM-1)+16 &&
      work->befTpy > MB_SEL_POKE_BOX_LEFT-16 &&
      work->befTpy < MB_SEL_POKE_BOX_LEFT+MB_SEL_POKE_BOX_WIDTH*(MB_SEL_POKE_BOX_X_NUM-1)+16 )
  {
    if( MB_SEL_POKE_GetType( work->holdPoke ) == MSPT_TRAY )
    {
      u8 i;
      const u8 idx = MB_SEL_POKE_GetIdx( work->holdPoke );
      const u8 retPage = work->initWork->selectPoke[idx][0];
      const u8 retIdx  = work->initWork->selectPoke[idx][1];

      //最後のやつを、ページ外に戻る時に使うので、先に並び替えを行う
      for( i=idx+1;i<MB_CAP_POKE_NUM;i++ )
      {
        int startX = MB_SEL_POKE_GetPosX( work->selPoke[i] );
        int startY = MB_SEL_POKE_GetPosY( work->selPoke[i] );
        work->initWork->selectPoke[i-1][0] = work->initWork->selectPoke[i][0];
        work->initWork->selectPoke[i-1][1] = work->initWork->selectPoke[i][1];

        //絵を読み直すと重いので、ワークの入れ替え処理
        work->selPoke[i-1] = work->selPoke[i];
        MB_SEL_POKE_SetIdx( work->selPoke[i-1] , i-1 );
        MB_SEL_POKE_SetMove( work , work->selPoke[i-1] , 
                             startX , startY ,
                             MB_SEL_POKE_GetPosX( work->selPoke[i-1] ),
                             MB_SEL_POKE_GetPosY( work->selPoke[i-1] ),
                             MB_SEL_RETURN_BOX_ICON_CNT , FALSE );
      }
      work->initWork->selectPoke[MB_CAP_POKE_NUM-1][0] = 0xFF;
      work->initWork->selectPoke[MB_CAP_POKE_NUM-1][1] = 0xFF;
      work->selPoke[MB_CAP_POKE_NUM-1] = work->holdPoke;
      MB_SEL_POKE_SetIdx( work->selPoke[MB_CAP_POKE_NUM-1] , MB_CAP_POKE_NUM-1 );

      //今見えてるページに帰るところがある
      if( retPage == work->boxPage )
      {
        GFL_CLACTPOS cellPos;
        POKEMON_PASO_PARAM *ppp = work->initWork->boxData[retPage][retIdx];
        cellPos.x = work->befTpx;
        cellPos.y = work->befTpy;

        MB_SEL_POKE_SetPPP( work , work->holdPoke , NULL );
        MB_SEL_POKE_SetPos( work , work->boxHoldPoke , &cellPos );
        MB_SEL_POKE_SetPPP( work , work->boxHoldPoke , ppp );
        MB_SEL_POKE_SetPri( work , work->boxHoldPoke , MSPT_HOLD );
        MB_SEL_POKE_SetMove( work , work->boxHoldPoke , 
                             work->befTpx , work->befTpy ,
                             MB_SEL_POKE_GetPosX( work->boxPoke[retIdx] ),
                             MB_SEL_POKE_GetPosY( work->boxPoke[retIdx] ),
                             MB_SEL_RETURN_BOX_ICON_CNT , TRUE );
        MB_SEL_POKE_ResetAlphaCnt( work , work->boxPoke[retIdx] , MB_SEL_RETURN_BOX_ICON_CNT );
      }
      else
      {
        //移動後にpppにNULLが自動的に入る
        MB_SEL_POKE_SetMove( work , work->holdPoke , 
                             work->befTpx,work->befTpy,
                             -32,work->befTpy,
                             MB_SEL_RETURN_BOX_ICON_CNT , TRUE );
      }
      
      return TRUE;
    }
  }
  
  return FALSE;
}

//--------------------------------------------------------------
//  ページスクロール処理
//--------------------------------------------------------------
static void MB_SELECT_UpdateChangePage( MB_SELECT_WORK *work )
{
  work->changePageCnt++;
  //MB_TPrintf("[%2d]",work->changePageCnt);
  
  if( work->changePageCnt <= MB_SEL_CHANGE_CNT )
  {
    u8 nextPage;
    u8 ix,iy;
    int scrollOfsBase;
    if( work->pageMoveDir == MSD_LEFT )
    {
      scrollOfsBase = work->changePageCnt * MB_SEL_CHANGE_PAGE_SPD;
      if( work->boxPage == 0 )
      {
        nextPage = MB_POKE_BOX_TRAY-1;
      }
      else
      {
        nextPage = work->boxPage-1;
      }
    }
    else
    {
      scrollOfsBase = work->changePageCnt * (-MB_SEL_CHANGE_PAGE_SPD);
      if( work->boxPage == MB_POKE_BOX_TRAY-1 )
      {
        nextPage = 0;
      }
      else
      {
        nextPage = work->boxPage+1;
      }
    }
    
    for( ix=0;ix<MB_SEL_POKE_BOX_X_NUM;ix++ )
    {
      GFL_CLACTPOS cellPos;
      cellPos.x = MB_SEL_POKE_GetPosX( work->boxPoke[ix] ) + scrollOfsBase;
      
      if( cellPos.x < -16 )
      {
        cellPos.x += MB_SEL_CHANGE_PAGE_LEN;
      }
      else
      if( cellPos.x > MB_SEL_CHANGE_PAGE_LEN-16 )
      {
        cellPos.x -= MB_SEL_CHANGE_PAGE_LEN;
      }
      //MB_TPrintf("[%4d]",cellPos.x);
      for( iy=0;iy<MB_SEL_POKE_BOX_Y_NUM;iy++ )
      {
        const u8 idx = ix + iy*MB_SEL_POKE_BOX_X_NUM;
        cellPos.y = MB_SEL_POKE_GetPosY( work->boxPoke[idx] );
        
        MB_SEL_POKE_SetPos( work , work->boxPoke[idx] , &cellPos );
        
        //処理落ち回避のため1フレームずつずらして読む
        if( cellPos.x == MB_SEL_CHANGE_DISPLEN+20 + iy*MB_SEL_CHANGE_PAGE_SPD )
        {
          u8 i;
          for( i=0;i<MB_CAP_POKE_NUM;i++ )
          {
            if( work->initWork->selectPoke[i][0] == nextPage &&
                work->initWork->selectPoke[i][1] == idx )
            {
              break;
            }
          }
          if( i==MB_CAP_POKE_NUM )
          {
            //トレーに移動済みではない
            MB_SEL_POKE_SetAlpha( work , work->boxPoke[idx] , FALSE );
            MB_SEL_POKE_SetPPP( work , work->boxPoke[idx] , 
                                work->initWork->boxData[nextPage][idx] );
          }
          else
          {
            //トレーに移動済み!!
            MB_SEL_POKE_SetAlpha( work , work->boxPoke[idx] , TRUE );
            MB_SEL_POKE_SetPPP( work , work->boxPoke[idx] , 
                                work->initWork->boxData[nextPage][idx] );
          }
          
        }
      }
    }
  }
  else
  {
    if( work->pageMoveDir == MSD_LEFT )
    {
      if( work->boxPage == 0 )
      {
        work->boxPage = MB_POKE_BOX_TRAY-1;
      }
      else
      {
        work->boxPage--;
      }
    }
    else
    {
      if( work->boxPage == MB_POKE_BOX_TRAY-1 )
      {
        work->boxPage = 0;
      }
      else
      {
        work->boxPage++;
      }
    }
    work->state = MSS_MAIN;
  }
  //MB_TPrintf("\n");
}

//--------------------------------------------------------------
//  ボックス名設定
//--------------------------------------------------------------
static void MB_SELECT_SetBoxName( MB_SELECT_WORK *work , const u8 idx )
{
  GFL_FONT *fontHandle = MB_MSG_GetFont( work->msgWork );
  PRINT_QUE *printQue  = MB_MSG_GetPrintQue( work->msgWork );
  STRBUF *str = GFL_STR_CreateBuffer( 32 , work->heapId );
  u32 strWidth;

  GFL_STR_SetStringCode( str , work->initWork->boxName[idx] );
  strWidth = PRINTSYS_GetStrWidth( str , fontHandle , 0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgBoxName ) , 0x0 );
  PRINTSYS_PrintQue( printQue , GFL_BMPWIN_GetBmp( work->msgBoxName ) , 
          (MB_SEL_MSG_BOXNAME_WIDTH*8-strWidth)/2 , 4 , str , fontHandle );
  work->isUpdateBoxName = TRUE;
  GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------
//  ポケモン情報設定
//--------------------------------------------------------------
static void MB_SELECT_SetPokeInfo( MB_SELECT_WORK *work , const POKEMON_PASO_PARAM *ppp )
{
  GFL_FONT *fontHandle = MB_MSG_GetFont( work->msgWork );
  PRINT_QUE *printQue  = MB_MSG_GetPrintQue( work->msgWork );
  GFL_MSGDATA* msgHandle = MB_MSG_GetMsgHandle( work->msgWork );
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgPokeInfo ) , 0x0 );

  if( PPP_Get( ppp , ID_PARA_tamago_flag , NULL ) == TRUE )
  {
    STRBUF *nameStr = GFL_MSG_CreateString( GlobalMsg_PokeName , MONSNO_TAMAGO );
    PRINTSYS_PrintQue( printQue , GFL_BMPWIN_GetBmp( work->msgPokeInfo ) , 
            0 , 0 , nameStr , fontHandle );
    GFL_STR_DeleteBuffer( nameStr );
  }
  else
  {
    //名前
    {
      STRBUF *nameStr = GFL_STR_CreateBuffer( 32 , work->heapId );
      PPP_Get( ppp , ID_PARA_nickname , nameStr );
      PRINTSYS_PrintQue( printQue , GFL_BMPWIN_GetBmp( work->msgPokeInfo ) , 
              0 , 0 , nameStr , fontHandle );
      GFL_STR_DeleteBuffer( nameStr );
    }
    //Lv
    {
      STRBUF *baseStr = GFL_MSG_CreateString( msgHandle , MSG_MB_CHILD_SEL_01 );
      STRBUF *workStr = GFL_STR_CreateBuffer( 16 , work->heapId );
      WORDSET *wordSet = WORDSET_Create( work->heapId );
      const u32 lv = PPP_CalcLevel( ppp );
      WORDSET_RegisterNumber( wordSet , 0 , lv , 3 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( wordSet , workStr , baseStr );
      PRINTSYS_PrintQue( printQue , GFL_BMPWIN_GetBmp( work->msgPokeInfo ) , 
              0 , 16 , workStr , fontHandle );

      GFL_STR_DeleteBuffer( baseStr );
      GFL_STR_DeleteBuffer( workStr );
      WORDSET_Delete( wordSet );
    }
  }
  //マーク
  {
    u8 i;
    u8 bitMask = 1;
    const u32 boxMark = PPP_Get( ppp , ID_PARA_mark , NULL );
    for( i=0;i<POKEPARA_MARKING_ELEMS_MAX;i++ )
    {
      if( boxMark & bitMask )
      {
        GFL_CLACT_WK_SetAnmSeq( work->clwkMark[i] , i*2 + 1 );
      }
      else
      {
        GFL_CLACT_WK_SetAnmSeq( work->clwkMark[i] , i*2 );
      }
      bitMask = bitMask<1;
      GFL_CLACT_WK_SetDrawEnable( work->clwkMark[i] , TRUE );
    }
  }
  
  work->isUpdatePokeInfo = TRUE;
}

//--------------------------------------------------------------
//  ポケモン情報クリア
//--------------------------------------------------------------
static void MB_SELECT_ClearPokeInfo( MB_SELECT_WORK *work )
{
  u8 i;
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgPokeInfo ) , 0x0 );
  GFL_BMPWIN_TransVramCharacter( work->msgPokeInfo );
  for( i=0;i<POKEPARA_MARKING_ELEMS_MAX;i++ )
  {
    GFL_CLACT_WK_SetDrawEnable( work->clwkMark[i] , FALSE );
  }
}

#pragma mark [>confirm func
#define MB_SEL_CONF_MOVE_LEN (16*8)
#define MB_SEL_CONF_MOVE_CNT (32)
#define MB_SEL_CONF_MOVE_SPD (4)
//--------------------------------------------------------------
//  確認画面移行初期化
//--------------------------------------------------------------
static void MB_SELECT_InitComfirm( MB_SELECT_WORK *work )
{
  u8 i;
  work->anmCnt = 0;
  work->state = MSS_CONFIRM_INIT;

  for( i=0;i<MB_CAP_POKE_NUM;i++ )
  {
    const int posX = MB_SEL_POKE_GetPosX( work->selPoke[i] );
    const int posY = MB_SEL_POKE_GetPosY( work->selPoke[i] );
    
    MB_SEL_POKE_SetMove( work , work->selPoke[i] , posX , posY ,
                         posX-MB_SEL_CONF_MOVE_LEN , posY , 
                         MB_SEL_CONF_MOVE_CNT , FALSE );
  }
  GFL_CLACT_WK_SetBgPri( work->clwkTray , 1 );
}

//--------------------------------------------------------------
//  確認画面移行
//--------------------------------------------------------------
static void MB_SELECT_UpdateInitComfirm( MB_SELECT_WORK *work )
{
  work->anmCnt++;
  
  {
    u8 i;
    const int moveLen = -MB_SEL_CONF_MOVE_SPD * work->anmCnt;
    GFL_CLACTPOS cellPos;
    
    cellPos.x = MB_SEL_TRAY_X + moveLen;
    cellPos.y = MB_SEL_TRAY_Y;
    GFL_CLACT_WK_SetPos( work->clwkTray , &cellPos , CLSYS_DRAW_MAIN );
  }
  if( work->anmCnt >= MB_SEL_CONF_MOVE_CNT )
  {
    MB_MSG_MessageCreateWindow( work->msgWork , MMWT_1LINE );
    MB_MSG_MessageDispNoWait( work->msgWork , MSG_MB_CHILD_SEL_05 );
    
    MB_MSG_DispYesNo( work->msgWork , MMYT_MID );
    work->state = MSS_CONFIRM_WAIT;
  }
  
}

//--------------------------------------------------------------
//  確認画面
//--------------------------------------------------------------
static void MB_SELECT_UpdateConfirm( MB_SELECT_WORK *work )
{
  const MB_MSG_YESNO_RET ret = MB_MSG_UpdateYesNo( work->msgWork );
  if( ret == MMYR_RET1 )
  {
    //はい
    work->state = MSS_FADEOUT;
  }
  else
  if( ret == MMYR_RET2 )
  {
    u8 i;
    //いいえ
    work->state = MSS_CONFIRM_RETURN;
    work->anmCnt = 0;
    MB_MSG_ClearYesNo( work->msgWork );
    MB_MSG_MessageHide( work->msgWork );
    for( i=0;i<MB_CAP_POKE_NUM;i++ )
    {
      const int posX = MB_SEL_POKE_GetPosX( work->selPoke[i] );
      const int posY = MB_SEL_POKE_GetPosY( work->selPoke[i] );
      
      MB_SEL_POKE_SetMove( work , work->selPoke[i] , 
                           posX-MB_SEL_CONF_MOVE_LEN , posY ,
                           posX , posY , 
                           MB_SEL_CONF_MOVE_CNT , FALSE );
    }
  }
}

//--------------------------------------------------------------
//  確認画面キャンセル時戻り
//--------------------------------------------------------------
static void MB_SELECT_UpdateReturnComfirm( MB_SELECT_WORK *work )
{
  work->anmCnt++;
  {
    u8 i;
    const int moveLen = -MB_SEL_CONF_MOVE_SPD * (MB_SEL_CONF_MOVE_CNT-work->anmCnt);
    GFL_CLACTPOS cellPos;
    
    cellPos.x = MB_SEL_TRAY_X + moveLen;
    cellPos.y = MB_SEL_TRAY_Y;
    GFL_CLACT_WK_SetPos( work->clwkTray , &cellPos , CLSYS_DRAW_MAIN );
  }
  if( work->anmCnt >= MB_SEL_CONF_MOVE_CNT )
  {
    work->state = MSS_MAIN;
    GFL_CLACT_WK_SetBgPri( work->clwkTray , 2 );
    //6番目をBOXに戻す
    {
      GFL_CLACTPOS cellPos;
      const u8 retPage = work->initWork->selectPoke[5][0];
      const u8 retIdx  = work->initWork->selectPoke[5][1];
      POKEMON_PASO_PARAM *ppp = work->initWork->boxData[retPage][retIdx];
      MB_SEL_POKE_SetPPP( work , work->selPoke[5] , NULL );

      cellPos.x = MB_SEL_POKE_GetPosX( work->selPoke[5] );
      cellPos.y = MB_SEL_POKE_GetPosY( work->selPoke[5] );

      MB_SEL_POKE_SetPos( work , work->boxHoldPoke , &cellPos );
      MB_SEL_POKE_SetPPP( work , work->boxHoldPoke , ppp );
      MB_SEL_POKE_SetPri( work , work->boxHoldPoke , MSPT_HOLD );
      MB_SEL_POKE_SetMove( work , work->boxHoldPoke , 
                           MB_SEL_POKE_GetPosX( work->selPoke[5] ),
                           MB_SEL_POKE_GetPosY( work->selPoke[5] ),
                           MB_SEL_POKE_GetPosX( work->boxPoke[retIdx] ),
                           MB_SEL_POKE_GetPosY( work->boxPoke[retIdx] ),
                           MB_SEL_RETURN_BOX_ICON_CNT , TRUE );
      MB_SEL_POKE_ResetAlphaCnt( work , work->boxPoke[retIdx] , MB_SEL_RETURN_BOX_ICON_CNT );

      work->initWork->selectPoke[5][0] = 0xFF;
      work->initWork->selectPoke[5][1] = 0xFF;
    }
  }
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
    u8 i,j;
    GFL_HEAP_CreateHeap( parentHeap , HEAPID_MB_BOX, 0x40000 );
    
    work = GFL_PROC_AllocWork( proc, sizeof(MB_SELECT_WORK), parentHeap );
    initWork = GFL_HEAP_AllocClearMemory( HEAPID_MB_BOX , sizeof(MB_SELECT_INIT_WORK) );

    initWork->msgSpeed = 1;
    initWork->parentHeap = parentHeap;
    initWork->cardType = CARD_TYPE_DUMMY;
    MB_TPrintf("デバッグポケ作成");
    for( i=0;i<MB_POKE_BOX_TRAY;i++ )
    {
      for( j=0;j<MB_POKE_BOX_POKE;j++ )
      {
        initWork->boxData[i][j] = GFL_HEAP_AllocClearMemory( parentHeap , POKETOOL_GetPPPWorkSize() );
        PPP_Clear( initWork->boxData[i][j] );
        if( i==0 && j == 0 )
        {
          //ぎざみみピチュー
          PPP_Setup( initWork->boxData[i][j] ,
                     172 ,
                     100 ,
                     PTL_SETUP_ID_AUTO );
          //デバッグで例外的に１を入れるためPPP_Put使ってます。
          PPP_Put( initWork->boxData[i][j] , ID_PARA_form_no , 1 );
        }
        else
        if( GFUser_GetPublicRand0(10) == 0 )
        //if( i == 0 || i == 1 || i == 17 )
        {
          PPP_Setup( initWork->boxData[i][j] ,
                     GFUser_GetPublicRand0(493)+1 ,
                     GFUser_GetPublicRand0(100)+1 ,
                     PTL_SETUP_ID_AUTO );
        }
      }
      MB_TPrintf(".%d",i);
    }
    MB_TPrintf("Finish!\n");
    //トレー名セット
    for( i=0;i<MB_POKE_BOX_TRAY;i++ )
    {
      initWork->boxName[i] = GFL_HEAP_AllocClearMemory( parentHeap , 20/*BOX_TRAYNAME_BUFSIZE*/ );
      initWork->boxName[i][0] = L'ぼ';
      initWork->boxName[i][1] = L'っ';
      initWork->boxName[i][2] = L'く';
      initWork->boxName[i][3] = L'す';
      initWork->boxName[i][4] = L'0'+(i+1)/10;
      initWork->boxName[i][5] = L'0'+(i+1)%10;
      initWork->boxName[i][6] = 0xFFFF;
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
    u8 i,j;
    for( i=0;i<MB_POKE_BOX_TRAY;i++ )
    {
      for( j=0;j<MB_POKE_BOX_POKE;j++ )
      {
        GFL_HEAP_FreeMemory( work->initWork->boxData[i][j] );
      }
      GFL_HEAP_FreeMemory( work->initWork->boxName[i] );
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

  if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE &&
      work->isNetErr == FALSE )
  {
    work->isNetErr = TRUE;
    work->state = MSS_FADEOUT;
  }
  
  if( ret == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}

#pragma mark [>outer func
//--------------------------------------------------------------
//  外部提供関数
//--------------------------------------------------------------
ARCHANDLE* MB_SELECT_GetIconArcHandle( MB_SELECT_WORK *work )
{
  return work->iconArcHandle;
}
const DLPLAY_CARD_TYPE MB_SELECT_GetCardType( const MB_SELECT_WORK *work )
{
  return work->initWork->cardType;
}
const HEAPID MB_SELECT_GetHeapId( const MB_SELECT_WORK *work )
{
  return work->heapId;
}


