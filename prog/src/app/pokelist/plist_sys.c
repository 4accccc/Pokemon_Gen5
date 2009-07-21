//======================================================================
/**
 * @file	plist_sys.c
 * @brief	ポケモンリスト メイン処理
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：PLIST
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"

#include "arc_def.h"
#include "pokelist_gra.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_pokelist.h"

#include "pokeicon/pokeicon.h"
#include "waza_tool/wazano_def.h"

#include "plist_sys.h"
#include "plist_plate.h"
#include "plist_message.h"
#include "plist_menu.h"
#include "plist_snd_def.h"

#include "app/p_status.h" //Proc切り替え用

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define PLIST_BARICON_Y (180)
#define PLIST_BARICON_EXIT_X   (212)  //抜ける(×マーク
#define PLIST_BARICON_RETURN_X_MENU (240)  //戻る(矢印
#define PLIST_BARICON_RETURN_X_BAR (236)  //戻る(矢印

#define PLIST_CHANGE_ANM_COUNT (16)
#define PLIST_CHANGE_ANM_VALUE (1)    //移動量(キャラ単位

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//サブとなるシーケンス(work->subSeq
typedef enum
{
  PSSS_INIT,
  PSSS_MAIN,
  PSSS_TERM,
  
  //入れ替えアニメ用
  PSSS_MOVE_OUT,
  PSSS_MOVE_IN,

  //Proc切り替え用
  PSSS_FADEOUT,
  PSSS_FADEIN,

  PSSS_MAX,
}PLIST_SYS_SUB_SEQ;

//選択状態
typedef enum
{
  PSSEL_DECIDE, //決定を押した(参加選択とか)
  PSSEL_RETURN, //キャンセル(B)
  PSSEL_EXIT, //戻る(X)

  PSSEL_SELECT, //ポケモンを選んだ
  PSSEL_NONE, //選択中

  PSSEL_BUTTON_NUM = PSSEL_EXIT+1,  //TP判定でタッチテーブルに使う
  PSSEL_MAX,  
}PLIST_SYS_SELECT_STATE;


//カーソルのアニメ
enum PLIST_CURCOR_ANIME
{
  PCA_NORMAL_A, //通常0番用
  PCA_NORMAL_B, //通常
  PCA_CHANGE_A, //入れ替え0番用
  PCA_CHANGE_B, //入れ替え
  PCA_SELECT_A, //選択0番用
  PCA_SELECT_B, //選択
  PCA_MENU,   //メニュー用
};

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
  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_128_B,       // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_NONE,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_0_D,        // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_0_F,     // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_128K
};

FS_EXTERN_OVERLAY(poke_status);

static void PLIST_VBlankFunc(GFL_TCB *tcb, void *wk );

//グラフィック系
static void PLIST_InitGraphic( PLIST_WORK *work );
static void PLIST_TermGraphic( PLIST_WORK *work );
static void PLIST_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void PLIST_InitBG0_2DMenu( PLIST_WORK *work );
static void PLIST_InitBG0_3DParticle( PLIST_WORK *work );

static void PLIST_InitCell( PLIST_WORK *work );
static void PLIST_TermCell( PLIST_WORK *work );

static void PLIST_InitMessage( PLIST_WORK *work );
static void PLIST_TermMessage( PLIST_WORK *work );

//リソース系
static void PLIST_LoadResource( PLIST_WORK *work );
static void PLIST_ReleaseResource( PLIST_WORK *work );

//モード別処理系
static void PLIST_InitMode_Select( PLIST_WORK *work );
static void PLIST_InitMode_Menu( PLIST_WORK *work );

//ポケモン選択
static void PLIST_SelectPoke( PLIST_WORK *work );
static void PLIST_SelectPokeInit( PLIST_WORK *work );
static void PLIST_SelectPokeTerm( PLIST_WORK *work );
static void PLIST_SelectPokeTerm_Change( PLIST_WORK *work );
static void PLIST_SelectPokeMain( PLIST_WORK *work );
static void PLIST_SelectPokeUpdateKey( PLIST_WORK *work );
static void PLIST_SelectPokeUpdateTP( PLIST_WORK *work );
static void PLIST_SelectPokeSetCursor( PLIST_WORK *work , const PL_SELECT_POS pos );
static void PLIST_SelectPokeSetCursor_Change( PLIST_WORK *work , const PL_SELECT_POS pos );
static void PLIST_PLATE_SetActivePlatePos( PLIST_WORK *work , const PL_SELECT_POS pos );
//メニュー選択
static void PLIST_SelectMenu( PLIST_WORK *work );
static void PLIST_SelectMenuInit( PLIST_WORK *work );
static void PLIST_SelectMenuTerm( PLIST_WORK *work );
static void PLIST_SelectMenuExit( PLIST_WORK *work );

//入れ替えアニメ
static void PLIST_ChangeAnime( PLIST_WORK *work );
static void PLIST_ChangeAnimeInit( PLIST_WORK *work );
static void PLIST_ChangeAnimeTerm( PLIST_WORK *work );
static void PLIST_ChangeAnimeUpdatePlate( PLIST_WORK *work );

//Proc切り替え
static void PLIST_ChangeProcInit( PLIST_WORK *work , GFL_PROC_DATA *procData , FSOverlayID overlayId , void *parentWork );
static void PLIST_ChangeProcUpdate( PLIST_WORK *work );

//デバッグメニュー
static void PLIST_InitDebug( PLIST_WORK *work );
static void PLIST_TermDebug( PLIST_WORK *work );
//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
const BOOL PLIST_InitPokeList( PLIST_WORK *work )
{
  u8 i;
  u8 partyMax = PokeParty_GetPokeCount( work->plData->pp );
  
  work->ktst = GFL_UI_CheckTouchOrKey();
  work->mainSeq = PSMS_FADEIN;
  work->subSeq = PSSS_INIT;
  work->selectPokePara = NULL;
  work->menuRet = PMIT_NONE;
  work->changeTarget = PL_SEL_POS_MAX;
  work->canExit = FALSE;
  work->isActiveWindowMask = FALSE;
  work->reqChangeProc = FALSE;
  work->changeProcSeq = PSCS_INIT;
  
  PLIST_InitGraphic( work );
  PLIST_LoadResource( work );
  PLIST_InitMessage( work );
  

  //プレートの作成
  for( i=0;i<PLIST_LIST_MAX;i++ )
  {
    if( i<partyMax )
    {
      POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, i);
      work->plateWork[i] = PLIST_PLATE_CreatePlate( work , i , pp );
    }
    else
    {
      work->plateWork[i] = PLIST_PLATE_CreatePlate_Blank( work , i );
    }
  }
  //描画順の関係上ここで
  PLIST_InitCell( work );
  
//  PLIST_PLATE_ChangeColor( work , work->plateWork[0] , PPC_NORMAL_SELECT );
  work->vBlankTcb = GFUser_VIntr_CreateTCB( PLIST_VBlankFunc , work , 8 );

  //メッセージシステム生成
  work->msgWork = PLIST_MSG_CreateSystem( work );

  //メニューシステム生成
  work->menuWork = PLIST_MENU_CreateSystem( work );

  
  //モード別初期化
  PLIST_InitMode_Select( work );

  //まとめて読み込む
  GFL_BG_LoadScreenV_Req(PLIST_BG_MAIN_BG);
  GFL_BG_LoadScreenV_Req(PLIST_BG_PLATE);
  GFL_BG_LoadScreenV_Req(PLIST_BG_SUB_BG);

#if USE_DEBUGWIN_SYSTEM
  PLIST_InitDebug( work );
#endif
  return TRUE;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
const BOOL PLIST_TermPokeList( PLIST_WORK *work )
{
  u8 i;
#if USE_DEBUGWIN_SYSTEM
  PLIST_TermDebug( work );
#endif

  GFL_TCB_DeleteTask( work->vBlankTcb );
  
  PLIST_MENU_DeleteSystem( work , work->menuWork );
  PLIST_MSG_DeleteSystem( work , work->msgWork );

  for( i=0;i<PLIST_LIST_MAX;i++ )
  {
    PLIST_PLATE_DeletePlate( work , work->plateWork[i] );
  }

  PLIST_TermMessage( work );
  PLIST_TermCell( work );
  PLIST_ReleaseResource( work );
  PLIST_TermGraphic( work );

  GFL_UI_SetTouchOrKey( work->ktst );
  
  return TRUE;
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
const BOOL PLIST_UpdatePokeList( PLIST_WORK *work )
{
  u8 i;

  switch( work->mainSeq )
  {
  case PSMS_FADEIN:
//    WIPE_SYS_Start( WIPE_PATTERN_FSAM , WIPE_TYPE_SHUTTERIN_DOWN , WIPE_TYPE_SHUTTERIN_DOWN , 
//                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    WIPE_SYS_Start( WIPE_PATTERN_FSAM , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->mainSeq = PSMS_FADEIN_WAIT;
    break;
  
  case PSMS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      work->mainSeq = PSMS_SELECT_POKE;
    }
    break;
    
  case PSMS_SELECT_POKE:
  case PSMS_CHANGE_POKE:  //基本は一緒だから同じ処理で
    PLIST_SelectPoke( work );
    break;

  case PSMS_CHANGE_ANM:
    PLIST_ChangeAnime( work );
    break;
    
  case PSMS_MENU:
    PLIST_SelectMenu( work );
    break;

  case PSMS_FADEOUT:
//    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_SPLITOUT_VSIDE , WIPE_TYPE_SPLITOUT_VSIDE , 
//                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->mainSeq = PSMS_FADEOUT_WAIT;
    break;
  
  case PSMS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return TRUE;
    }
    break;

  case PSMS_CHANGEPROC:
    PLIST_ChangeProcUpdate( work );
    break;
  }
  
  for( i=0;i<PLIST_LIST_MAX;i++ )
  {
    PLIST_PLATE_UpdatePlate( work , work->plateWork[i] );
  }
  PLIST_MSG_UpdateSystem( work , work->msgWork );

  //メッセージ
  PRINTSYS_QUE_Main( work->printQue );

  //OBJの更新
  GFL_CLACT_SYS_Main();

  return FALSE;
}

//--------------------------------------------------------------
//	VBlankTcb
//--------------------------------------------------------------
static void PLIST_VBlankFunc(GFL_TCB *tcb, void *wk )
{
  PLIST_WORK *work = (PLIST_WORK*)wk;

  if( work->isActiveWindowMask == TRUE )
  {
    GX_SetVisibleWnd( GX_WNDMASK_W0 );
  }
  else
  {
    GX_SetVisibleWnd( GX_WNDMASK_NONE );
  }

  //OBJの更新
  GFL_CLACT_SYS_VBlankFunc();
//  NNS_GfdDoVramTransfer();
}


#pragma mark [>graphic
//--------------------------------------------------------------
//グラフィック系初期化
//--------------------------------------------------------------
static void PLIST_InitGraphic( PLIST_WORK *work )
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
        GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    // BG1 MAIN (パラメータ
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x1000, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x08000,0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 MAIN (プレート
    static const GFL_BG_BGCNT_HEADER header_main2 = {
      0, 0, 0x1000, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x10000,0x8000,
      GX_BG_EXTPLTT_01, 2, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 MAIN (背景
    static const GFL_BG_BGCNT_HEADER header_main3 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000,0x05000,
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

    PLIST_SetupBgFunc( &header_main3 , PLIST_BG_MAIN_BG , GFL_BG_MODE_TEXT );
    PLIST_SetupBgFunc( &header_main2 , PLIST_BG_PLATE , GFL_BG_MODE_TEXT );
    PLIST_SetupBgFunc( &header_main1 , PLIST_BG_PARAM , GFL_BG_MODE_TEXT );

    PLIST_SetupBgFunc( &header_sub3 , PLIST_BG_SUB_BG , GFL_BG_MODE_TEXT );
    
    //とりあえず2Dで初期化
    PLIST_InitBG0_2DMenu( work );
    
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_OBJ , 
                      GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ ,
                      12 , 16 );

    //土台とパラメータは取替えアニメのためスクロールさせる
    GFL_BG_SetScroll( PLIST_BG_PLATE , GFL_BG_SCROLL_X_SET , PLIST_BG_SCROLL_X_CHAR*8 );
    GFL_BG_SetScroll( PLIST_BG_PARAM , GFL_BG_SCROLL_X_SET , PLIST_BG_SCROLL_X_CHAR*8 );
  }
  
  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
  }
  
  //Vram転送アニメ
  //machineuse で初期化してた
//  NNS_GfdInitVramTransferManager( work->transTask , PLIST_VTRANS_TASK_NUM );
  NNS_GfdClearVramTransferManagerTask();
}

//--------------------------------------------------------------
//BG0用初期化(BG0面の2D/3D
//--------------------------------------------------------------
static void PLIST_InitBG0_2DMenu( PLIST_WORK *work )
{
  // BG0 MAIN (メニュー
  static const GFL_BG_BGCNT_HEADER header_main0 = {
    0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x18000,0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
  };
  PLIST_SetupBgFunc( &header_main0 , PLIST_BG_MENU , GFL_BG_MODE_TEXT );
}

static void PLIST_InitBG0_3DParticle( PLIST_WORK *work )
{
  GF_ASSERT_MSG( FALSE ,"まだ作ってない!!\n");
}


//--------------------------------------------------------------
//グラフィック系開放
//--------------------------------------------------------------
static void PLIST_TermGraphic( PLIST_WORK *work )
{
  NNS_GfdClearVramTransferManagerTask();
  GFL_CLACT_SYS_Delete();
  
  GFL_BG_FreeBGControl( PLIST_BG_MAIN_BG );
  GFL_BG_FreeBGControl( PLIST_BG_PLATE );
  GFL_BG_FreeBGControl( PLIST_BG_PARAM );
  GFL_BG_FreeBGControl( PLIST_BG_MENU );
  GFL_BG_FreeBGControl( PLIST_BG_SUB_BG );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();

}

//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void PLIST_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------------------
//  セル作成
//--------------------------------------------------------------------------
static void PLIST_InitCell( PLIST_WORK *work )
{
  //TODO 個数は適当
  work->cellUnit  = GFL_CLACT_UNIT_Create( 10 , PLIST_CELLUNIT_PRI_MAIN, work->heapId );
  GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit );
  //セルの生成
  //カーソル
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 0;
    cellInitData.pos_y = 0;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 3;
    work->clwkCursor[0] = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellRes[PCR_NCG_CURSOR],
              work->cellRes[PCR_PLT_OBJ_COMMON],
              work->cellRes[PCR_ANM_CURSOR],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

    work->clwkCursor[1] = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellRes[PCR_NCG_CURSOR],
              work->cellRes[PCR_PLT_OBJ_COMMON],
              work->cellRes[PCR_ANM_CURSOR],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

    GFL_CLACT_WK_SetAutoAnmFlag( work->clwkCursor[0] , TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( work->clwkCursor[1] , TRUE );
    GFL_CLACT_WK_SetDrawEnable( work->clwkCursor[0] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( work->clwkCursor[1] , FALSE );
  }
  //バーアイコン
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = PLIST_BARICON_RETURN_X_BAR;
    cellInitData.pos_y = PLIST_BARICON_Y;
    cellInitData.anmseq = PBA_RETURN_NORMAL;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    
    //こっちはメニューのときも上に出るのでbgpri0
    work->clwkBarIcon[PBT_RETURN] = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellRes[PCR_NCG_BAR_ICON],
              work->cellRes[PCR_PLT_OBJ_COMMON],
              work->cellRes[PCR_ANM_BAR_ICON],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );
    
    //こっちはメニューのとき消えるのでbgpri1
    cellInitData.pos_x = PLIST_BARICON_EXIT_X-4;
    cellInitData.anmseq = PBA_EXIT_NORMAL;
    cellInitData.bgpri = 1;
    work->clwkBarIcon[PBT_EXIT] = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellRes[PCR_NCG_BAR_ICON],
              work->cellRes[PCR_PLT_OBJ_COMMON],
              work->cellRes[PCR_ANM_BAR_ICON],
              &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );
  }
}

//--------------------------------------------------------------------------
//  セル削除
//--------------------------------------------------------------------------
static void PLIST_TermCell( PLIST_WORK *work )
{
  GFL_CLACT_WK_Remove( work->clwkBarIcon[PBT_RETURN] );
  GFL_CLACT_WK_Remove( work->clwkBarIcon[PBT_EXIT] );
  GFL_CLACT_WK_Remove( work->clwkCursor[0] );
  GFL_CLACT_WK_Remove( work->clwkCursor[1] );

  GFL_CLACT_UNIT_Delete( work->cellUnit );
}

//--------------------------------------------------------------------------
//  メッセージ系初期化
//--------------------------------------------------------------------------
static void PLIST_InitMessage( PLIST_WORK *work )
{
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  work->sysFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_small_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  
  //メッセージ
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_pokelist_dat , work->heapId );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , PLIST_BG_PLT_FONT*16*2, 16*2, work->heapId );
  
  work->printQue = PRINTSYS_QUE_Create( work->heapId );
}

//--------------------------------------------------------------------------
//  メッセージ系開放
//--------------------------------------------------------------------------
static void PLIST_TermMessage( PLIST_WORK *work )
{
  PRINTSYS_QUE_Delete( work->printQue );
  GFL_MSG_Delete( work->msgHandle );
  GFL_FONT_Delete( work->sysFontHandle );
  GFL_FONT_Delete( work->fontHandle );
}

#pragma mark [>resource
//--------------------------------------------------------------------------
//  リソース系読み込み
//--------------------------------------------------------------------------
static void PLIST_LoadResource( PLIST_WORK *work )
{
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_POKELIST , work->heapId );

  ////BGリソース

  //下画面共通パレット
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_pokelist_gra_p_list_NCLR , 
                    PALTYPE_MAIN_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_pokelist_gra_hp_bar_NCLR , 
                    PALTYPE_MAIN_BG , PLIST_BG_PLT_HP_BAR*32 , 32 , work->heapId );

  //下画面背景
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_pokelist_gra_p_list_NCGR ,
                    PLIST_BG_MAIN_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_pokelist_gra_p_list_NSCR , 
                    PLIST_BG_MAIN_BG ,  0 , 0, FALSE , work->heapId );

  //下画面プレート
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_pokelist_gra_list_plate_NCGR ,
                    PLIST_BG_PLATE , 0 , 0, FALSE , work->heapId );
  work->plateScrRes = GFL_ARCHDL_UTIL_LoadScreen( arcHandle , NARC_pokelist_gra_list_plate_NSCR ,
                    FALSE , &work->plateScrData , work->heapId );

  //下画面バー  
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_pokelist_gra_list_bar_NCGR ,
                    PLIST_BG_PARAM , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_pokelist_gra_list_bar_NSCR , 
                    PLIST_BG_PARAM ,  0 , 0, FALSE , work->heapId );

  //上画面背景
  GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_pokelist_gra_list_sub_NCLR , 
                    PALTYPE_SUB_BG , 0 , 0 , work->heapId );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_pokelist_gra_list_sub_NCGR ,
                    PLIST_BG_SUB_BG , 0 , 0, FALSE , work->heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_pokelist_gra_list_sub_NSCR , 
                    PLIST_BG_SUB_BG ,  0 , 0, FALSE , work->heapId );

  ////OBJリソース
  //パレット
  work->cellRes[PCR_PLT_OBJ_COMMON] = GFL_CLGRP_PLTT_Register( arcHandle , 
        NARC_pokelist_gra_list_obj_NCLR , CLSYS_DRAW_MAIN , 
        PLIST_OBJPLT_COMMON*32 , work->heapId  );
  work->cellRes[PCR_PLT_ITEM_ICON] = GFL_CLGRP_PLTT_Register( arcHandle , 
        NARC_pokelist_gra_item_icon_NCLR , CLSYS_DRAW_MAIN , 
        PLIST_OBJPLT_ITEM_ICON*32 , work->heapId  );
  work->cellRes[PCR_PLT_CONDITION] = GFL_CLGRP_PLTT_Register( arcHandle , 
        NARC_pokelist_gra_p_st_ijou_NCLR , CLSYS_DRAW_MAIN , 
        PLIST_OBJPLT_CONDITION*32 , work->heapId  );
  work->cellRes[PCR_PLT_HP_BASE] = GFL_CLGRP_PLTT_Register( arcHandle , 
        NARC_pokelist_gra_hp_dodai_NCLR , CLSYS_DRAW_MAIN , 
        PLIST_OBJPLT_HP_BASE*32 , work->heapId  );
        
  //キャラクタ
  work->cellRes[PCR_NCG_CURSOR] = GFL_CLGRP_CGR_Register( arcHandle , 
        NARC_pokelist_gra_list_cursor_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  work->cellRes[PCR_NCG_BALL] = GFL_CLGRP_CGR_Register( arcHandle , 
        NARC_pokelist_gra_list_ball_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  work->cellRes[PCR_NCG_BAR_ICON] = GFL_CLGRP_CGR_Register( arcHandle , 
        NARC_pokelist_gra_bar_icon_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  work->cellRes[PCR_NCG_ITEM_ICON] = GFL_CLGRP_CGR_Register( arcHandle , 
        NARC_pokelist_gra_item_icon_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  work->cellRes[PCR_NCG_CONDITION] = GFL_CLGRP_CGR_Register( arcHandle , 
        NARC_pokelist_gra_p_st_ijou_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  work->cellRes[PCR_NCG_HP_BASE] = GFL_CLGRP_CGR_Register( arcHandle , 
        NARC_pokelist_gra_hp_dodai_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
  
  //セル・アニメ
  work->cellRes[PCR_ANM_CURSOR] = GFL_CLGRP_CELLANIM_Register( arcHandle , 
        NARC_pokelist_gra_list_cursor_NCER , NARC_pokelist_gra_list_cursor_NANR, work->heapId  );
  work->cellRes[PCR_ANM_BALL] = GFL_CLGRP_CELLANIM_Register( arcHandle , 
        NARC_pokelist_gra_list_ball_NCER , NARC_pokelist_gra_list_ball_NANR, work->heapId  );
  work->cellRes[PCR_ANM_BAR_ICON] = GFL_CLGRP_CELLANIM_Register( arcHandle , 
        NARC_pokelist_gra_bar_icon_NCER , NARC_pokelist_gra_bar_icon_NANR, work->heapId  );
  work->cellRes[PCR_ANM_ITEM_ICON] = GFL_CLGRP_CELLANIM_Register( arcHandle , 
        NARC_pokelist_gra_item_icon_NCER , NARC_pokelist_gra_item_icon_NANR, work->heapId  );
  work->cellRes[PCR_ANM_CONDITION] = GFL_CLGRP_CELLANIM_Register( arcHandle , 
        NARC_pokelist_gra_p_st_ijou_NCER , NARC_pokelist_gra_p_st_ijou_NANR, work->heapId  );
  work->cellRes[PCR_ANM_HP_BASE] = GFL_CLGRP_CELLANIM_Register( arcHandle , 
        NARC_pokelist_gra_hp_dodai_NCER , NARC_pokelist_gra_hp_dodai_NANR, work->heapId  );

  GFL_ARC_CloseDataHandle(arcHandle);
  
  //ポケアイコン用リソース
  //キャラクタは各プレートで
  {
    ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , work->heapId );
    work->cellRes[PCR_PLT_POKEICON] = GFL_CLGRP_PLTT_RegisterComp( arcHandlePoke , 
          POKEICON_GetPalArcIndex() , CLSYS_DRAW_MAIN , 
          PLIST_OBJPLT_POKEICON*32 , work->heapId  );
    work->cellRes[PCR_ANM_POKEICON] = GFL_CLGRP_CELLANIM_Register( arcHandlePoke , 
          POKEICON_GetCellArcIndex() , POKEICON_GetAnmArcIndex(), work->heapId  );
    
    GFL_ARC_CloseDataHandle(arcHandlePoke);
  }
}

static void PLIST_ReleaseResource( PLIST_WORK *work )
{
  u8 i;
  //BG
  GFL_HEAP_FreeMemory( work->plateScrRes );
  
  //OBJ
  for( i=PCR_PLT_START ; i<=PCR_PLT_END ; i++ )
  {
    GFL_CLGRP_PLTT_Release( work->cellRes[i] );
  }
  for( i=PCR_NCG_START ; i<=PCR_NCG_END ; i++ )
  {
    GFL_CLGRP_CGR_Release( work->cellRes[i] );
  }
  for( i=PCR_ANM_START ; i<=PCR_ANM_END ; i++ )
  {
    GFL_CLGRP_CELLANIM_Release( work->cellRes[i] );
  }
}

#pragma mark [>accorde mode
//--------------------------------------------------------------------------
//  モード別の初期化(ポケモン選択
//--------------------------------------------------------------------------
static void PLIST_InitMode_Select( PLIST_WORK *work )
{
  switch( work->plData->mode )
  {
  case PL_MODE_FIELD:
    PLIST_MSG_OpenWindow( work , work->msgWork , PMT_BAR );
    PLIST_MSG_DrawMessageNoWait( work , work->msgWork , mes_pokelist_02_01 );
    work->canExit = TRUE;
    break;
    
  default:
    GF_ASSERT_MSG( NULL , "PLIST mode まだ作ってない！[%d]\n" , work->plData->mode );
    break;
  }
  PLIST_SelectPokeInit( work );
}

//--------------------------------------------------------------------------
//  モード別の初期化(メニュー
//--------------------------------------------------------------------------
static void PLIST_InitMode_Menu( PLIST_WORK *work )
{
  PLIST_MENU_ITEM_TYPE itemArr[8];
  switch( work->plData->mode )
  {
  case PL_MODE_FIELD:
    itemArr[0] = PMIT_STATSU;
    itemArr[1] = PMIT_HIDEN;
    itemArr[2] = PMIT_CHANGE;
    itemArr[3] = PMIT_ITEM;
    itemArr[4] = PMIT_CLOSE;
    itemArr[5] = PMIT_END_LIST;
    
    PLIST_MSG_CreateWordSet( work , work->msgWork );
    PLIST_MSG_AddWordSet_PokeName( work , work->msgWork , 0 , work->selectPokePara );
    
    PLIST_MSG_OpenWindow( work , work->msgWork , PMT_MENU );
    PLIST_MSG_DrawMessageNoWait( work , work->msgWork , mes_pokelist_03_01 );
    
    PLIST_MSG_DeleteWordSet( work , work->msgWork );

    break;
    
  default:
    GF_ASSERT_MSG( NULL , "PLIST mode まだ作ってない！[%d]\n" , work->plData->mode );
    itemArr[0] = PMIT_STATSU;
    itemArr[1] = PMIT_CLOSE;
    itemArr[2] = PMIT_END_LIST;
    break;
  }
  PLIST_MENU_OpenMenu( work , work->menuWork , itemArr );

}

#pragma mark [>PokeSelect

//--------------------------------------------------------------------------
//  ポケモン選択画面
//--------------------------------------------------------------------------
static void PLIST_SelectPoke( PLIST_WORK *work )
{
  switch( work->subSeq )
  {
  case PSSS_INIT:
    work->subSeq = PSSS_MAIN;
    work->selectState = PSSEL_NONE;
    break;
    
  case PSSS_MAIN:
    PLIST_SelectPokeMain( work );
    break;
    
  case PSSS_TERM:
    if( work->mainSeq == PSMS_CHANGE_POKE )
    {
      PLIST_SelectPokeTerm_Change( work );
    }
    else
    {
      PLIST_SelectPokeTerm( work );
    }
    break;
  }
}

//--------------------------------------------------------------------------
//  ポケモン選択画面初期化
//--------------------------------------------------------------------------
static void PLIST_SelectPokeInit( PLIST_WORK *work )
{
  work->pokeCursor = work->plData->ret_sel;
  work->subSeq  = PSSS_INIT;
  work->menuRet = PMIT_NONE;
  
  if( work->ktst == GFL_APP_KTST_KEY || 
      GFL_CLACT_WK_GetDrawEnable( work->clwkCursor[0] ) == TRUE )
  {
    if( work->pokeCursor <= PL_SEL_POS_POKE6 )
    {
      PLIST_SelectPokeSetCursor( work , work->pokeCursor );

      PLIST_PLATE_SetActivePlate( work , work->plateWork[work->pokeCursor] , TRUE );
    }
  }
  else
  {
    GFL_CLACT_WK_SetDrawEnable( work->clwkCursor[0] , FALSE );
  }
  
  GFL_CLACT_WK_SetDrawEnable( work->clwkBarIcon[PBT_EXIT] , work->canExit );
  
}

//--------------------------------------------------------------------------
//  ポケモン選択画面開放
//--------------------------------------------------------------------------
static void PLIST_SelectPokeTerm( PLIST_WORK *work )
{
  PLIST_MSG_CloseWindow( work , work->msgWork );

  switch( work->selectState )
  {
  case PSSEL_SELECT:
    //カーソルを変更
    if( work->pokeCursor == PL_SEL_POS_POKE1 )
    {
      GFL_CLACT_WK_SetAnmSeq( work->clwkCursor[0] , PCA_SELECT_A );
    }
    else
    {
      GFL_CLACT_WK_SetAnmSeq( work->clwkCursor[0] , PCA_SELECT_B );
    }
    GFL_CLACT_WK_SetBgPri( work->clwkCursor[0] , 2 );
    
    work->selectPokePara = PokeParty_GetMemberPointer(work->plData->pp, work->pokeCursor );

    work->plData->ret_sel = work->pokeCursor;
    PLIST_SelectMenuInit( work );
    //中で一緒にメニューを開く
    PLIST_InitMode_Menu( work );
    PMSND_PlaySystemSE( PLIST_SND_DECIDE );
    break;

  case PSSEL_DECIDE:
    work->mainSeq = PSMS_FADEOUT;
    work->plData->ret_sel = PL_SEL_POS_ENTER;
    PMSND_PlaySystemSE( PLIST_SND_DECIDE );
    break;
    
  case PSSEL_RETURN:
    work->mainSeq = PSMS_FADEOUT;
    work->plData->ret_sel = PL_SEL_POS_EXIT;
    PMSND_PlaySystemSE( PLIST_SND_CANCEL );
    break;

  case PSSEL_EXIT:
    work->mainSeq = PSMS_FADEOUT;
    work->plData->ret_sel = PL_SEL_POS_EXIT2;
    PMSND_PlaySystemSE( PLIST_SND_CANCEL );
    break;
  }
}

//--------------------------------------------------------------------------
//  入れ替え用ポケモン選択画面開放
//--------------------------------------------------------------------------
static void PLIST_SelectPokeTerm_Change( PLIST_WORK *work )
{
  PLIST_MSG_CloseWindow( work , work->msgWork );

  //同じプレートを選んだのでキャンセル扱い
  if( work->selectState == PSSEL_SELECT &&
      work->pokeCursor == work->changeTarget )
  {
    work->selectState = PSSEL_RETURN;
  }

  switch( work->selectState )
  {
  case PSSEL_SELECT:
    PLIST_ChangeAnimeInit( work );
    PMSND_PlaySystemSE( PLIST_SND_CHANGE );
    break;
    
  case PSSEL_RETURN:
    work->selectPokePara = NULL;
    work->mainSeq = PSMS_SELECT_POKE;

    PLIST_SelectPokeSetCursor( work , work->pokeCursor );
    PLIST_PLATE_SetActivePlate( work , work->plateWork[work->changeTarget] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( work->clwkCursor[1] , FALSE );
    work->changeTarget = PL_SEL_POS_MAX;
    work->plData->ret_sel = work->pokeCursor;

    PLIST_InitMode_Select( work );
    PMSND_PlaySystemSE( PLIST_SND_CANCEL );
    break;

  }
}

//--------------------------------------------------------------------------
//  ポケモン選択画面更新
//--------------------------------------------------------------------------
static void PLIST_SelectPokeMain( PLIST_WORK *work )
{
  PLIST_SelectPokeUpdateKey( work );
  //キーによる決定が無かったらTPも見る
  if( work->selectState == PSSEL_NONE )
  {
    PLIST_SelectPokeUpdateTP( work );
  }

  if( work->selectState != PSSEL_NONE )
  {
    work->subSeq = PSSS_TERM;
  }
}

//--------------------------------------------------------------------------
//  ポケモン選択画面 キー操作
//--------------------------------------------------------------------------
static void PLIST_SelectPokeUpdateKey( PLIST_WORK *work )
{
  const int trg = GFL_UI_KEY_GetTrg();
  const int repeat = GFL_UI_KEY_GetRepeat();
  if( work->ktst == GFL_APP_KTST_TOUCH && 
      GFL_CLACT_WK_GetDrawEnable( work->clwkCursor[0] ) == FALSE )
  {
    //非表示のときだけ0に表示するように
    if( trg != 0 )
    {
      //カーソルを0の位置に表示
      PLIST_PLATE_SetActivePlate( work , work->plateWork[work->pokeCursor] , FALSE );
      work->pokeCursor = PL_SEL_POS_POKE1;
      PLIST_SelectPokeSetCursor( work , work->pokeCursor );
      PLIST_PLATE_SetActivePlate( work , work->plateWork[work->pokeCursor] , TRUE );
      
      PLIST_PLATE_SetActivePlatePos( work , work->pokeCursor );
      
      work->ktst = GFL_APP_KTST_KEY;
    }
  }
  else
  {
    s8 moveVal = 0;
    if( trg != 0 )
    {
      work->ktst = GFL_APP_KTST_KEY;
    }
    //キー分岐
    if( repeat & PAD_KEY_UP )
    {
      moveVal = -2;
    }
    else
    if( repeat & PAD_KEY_DOWN )
    {
      moveVal = 2;
    }
    else
    if( repeat & PAD_KEY_LEFT )
    {
      moveVal = -1;
    }
    else
    if( repeat & PAD_KEY_RIGHT )
    {
      moveVal = 1;
    }
    else if( trg & PAD_BUTTON_A )
    {
      work->selectState = PSSEL_SELECT;
    }
    else if( trg & PAD_BUTTON_B )
    {
      work->selectState = PSSEL_RETURN;
    }
    else if( trg & PAD_BUTTON_X )
    {
      if( work->canExit == TRUE )
      {
        work->selectState = PSSEL_EXIT;
      }
    }
    
    //十字キー移動処理
    if( moveVal != 0 )
    {
      const PL_SELECT_POS befPos = work->pokeCursor;
      BOOL isFinish = FALSE;
      PMSND_PlaySystemSE( PLIST_SND_CURSOR );
      //プレートがある位置までループ
      while( isFinish == FALSE )
      {
        if( work->pokeCursor + moveVal > PL_SEL_POS_POKE6 )
        {
          work->pokeCursor = work->pokeCursor+moveVal-(PL_SEL_POS_POKE6+1);
        }
        else
        if( work->pokeCursor + moveVal < PL_SEL_POS_POKE1 )
        {
          work->pokeCursor = work->pokeCursor+(PL_SEL_POS_POKE6+1)+moveVal;
        }
        else
        {
          work->pokeCursor += moveVal;
        }

        if( PLIST_PLATE_CanSelect( work , work->plateWork[work->pokeCursor] ) == TRUE )
        {
          isFinish = TRUE;
        }
      }
      //表示周り更新
      if( befPos != work->pokeCursor )
      {
        PLIST_SelectPokeSetCursor( work , work->pokeCursor );
        PLIST_PLATE_SetActivePlate( work , work->plateWork[work->pokeCursor] , TRUE );
        PLIST_PLATE_SetActivePlate( work , work->plateWork[befPos] , FALSE );

        PLIST_PLATE_SetActivePlatePos( work , work->pokeCursor );
      }
    }
  }
}

//--------------------------------------------------------------------------
//  ポケモン選択画面 TP操作
//--------------------------------------------------------------------------
static void PLIST_SelectPokeUpdateTP( PLIST_WORK *work )
{
  //プレートと当たり
  int ret;
  
  {
    //当たり判定作成
    u8 i;
    u8 cnt = 0;
    u8 plateIdx[PLIST_LIST_MAX];
    GFL_UI_TP_HITTBL hitTbl[PLIST_LIST_MAX+1];
    
    for( i=0;i<PLIST_LIST_MAX;i++ )
    {
      if( PLIST_PLATE_CanSelect( work , work->plateWork[i] ) == TRUE )
      {
        plateIdx[cnt] = i;
        PLIST_PLATE_GetPlateRect( work , work->plateWork[i] , &hitTbl[cnt] );
        cnt++;
      }
    }
    hitTbl[cnt].circle.code = GFL_UI_TP_HIT_END;

    ret = GFL_UI_TP_HitTrg( hitTbl );

    if( ret != GFL_UI_TP_HIT_NONE )
    {
      const PL_SELECT_POS befPos = work->pokeCursor;
      work->selectState = PSSEL_SELECT;
      work->pokeCursor = plateIdx[ret];

      PLIST_SelectPokeSetCursor( work , work->pokeCursor );
      PLIST_PLATE_SetActivePlate( work , work->plateWork[work->pokeCursor] , TRUE );
      PLIST_PLATE_SetActivePlate( work , work->plateWork[befPos] , FALSE );

      work->ktst = GFL_APP_KTST_TOUCH;
    }
  }
  
  //リターンキャンセルとかと当たり
  if( ret == GFL_UI_TP_HIT_NONE )
  {
    //FIXME 本当はモード分岐がいる
    GFL_UI_TP_HITTBL hitTbl[PSSEL_BUTTON_NUM+1] =
    {
      { 0,0,0,0 },
      { 0,0,0,0 },
      { 0,0,0,0 },
      { GFL_UI_TP_HIT_END,0,0,0 },
    };
    hitTbl[PSSEL_RETURN].rect.top    = PLIST_BARICON_Y - 12;
    hitTbl[PSSEL_RETURN].rect.bottom = PLIST_BARICON_Y + 12;
    hitTbl[PSSEL_RETURN].rect.left   = PLIST_BARICON_RETURN_X_BAR - 12;
    hitTbl[PSSEL_RETURN].rect.right  = PLIST_BARICON_RETURN_X_BAR + 12;
    if( work->canExit == TRUE )
    {
      hitTbl[PSSEL_EXIT  ].rect.top    = PLIST_BARICON_Y - 12;
      hitTbl[PSSEL_EXIT  ].rect.bottom = PLIST_BARICON_Y + 12;
      hitTbl[PSSEL_EXIT  ].rect.left   = PLIST_BARICON_EXIT_X - 12;
      hitTbl[PSSEL_EXIT  ].rect.right  = PLIST_BARICON_EXIT_X + 12;
    }
    
    ret = GFL_UI_TP_HitTrg( hitTbl );

    if( ret != GFL_UI_TP_HIT_NONE )
    {
      work->selectState = ret;
      work->ktst = GFL_APP_KTST_TOUCH;
    }
  }
  
}

//--------------------------------------------------------------------------
//  ポケモン選択画面　カーソル設定
//--------------------------------------------------------------------------
static void PLIST_SelectPokeSetCursor( PLIST_WORK *work , const PL_SELECT_POS pos )
{
  GFL_CLACTPOS curPos;
  PLIST_PLATE_GetPlatePosition(work , work->plateWork[pos] , &curPos );
  GFL_CLACT_WK_SetPos( work->clwkCursor[0] , &curPos , CLSYS_DRAW_MAIN );
  GFL_CLACT_WK_SetDrawEnable( work->clwkCursor[0] , TRUE );
  GFL_CLACT_WK_SetBgPri( work->clwkCursor[0] , 3 );

  if( pos == PL_SEL_POS_POKE1 )
  {
    GFL_CLACT_WK_SetAnmSeq( work->clwkCursor[0] , PCA_NORMAL_A );
  }
  else
  {
    GFL_CLACT_WK_SetAnmSeq( work->clwkCursor[0] , PCA_NORMAL_B );
  }
}

//--------------------------------------------------------------------------
//  ポケモン選択画面　入れ替えようカーソル設定
//--------------------------------------------------------------------------
static void PLIST_SelectPokeSetCursor_Change( PLIST_WORK *work , const PL_SELECT_POS pos )
{
  GFL_CLACTPOS curPos;
  PLIST_PLATE_GetPlatePosition(work , work->plateWork[pos] , &curPos );
  GFL_CLACT_WK_SetPos( work->clwkCursor[1] , &curPos , CLSYS_DRAW_MAIN );
  GFL_CLACT_WK_SetDrawEnable( work->clwkCursor[1] , TRUE );
  GFL_CLACT_WK_SetBgPri( work->clwkCursor[1] , 2 );

  if( pos == PL_SEL_POS_POKE1 )
  {
    GFL_CLACT_WK_SetAnmSeq( work->clwkCursor[1] , PCA_CHANGE_A );
  }
  else
  {
    GFL_CLACT_WK_SetAnmSeq( work->clwkCursor[1] , PCA_CHANGE_B );
  }
}

static void PLIST_PLATE_SetActivePlatePos( PLIST_WORK *work , const PL_SELECT_POS pos )
{
  const s8 moveVal[7][6][2] =
  {
    {
      { 0,0 },{ 1,0 },
      { 0,1 },{ 1,1 },
      { 0,1 },{ 1,1 },
    },
    {
      { -1,0 },{ 0,0 },
      { -1,1 },{ 0,1 },
      { -1,1 },{ 0,1 },
    },
    {
      { 0,-1 },{ 1,-1 },
      { 0,0 } ,{ 1,0 },
      { 0,1 } ,{ 1,1 },
    },
    {
      { -1,-1 },{ 0,-1 },
      { -1,0 } ,{ 0,0 },
      { -1,1 } ,{ 0,1 },
    },
    {
      { 0,-1 },{ 1,-1 },
      { 0,-1 },{ 1,-1 },
      { 0,0 } ,{ 1,0 },
    },
    {
      { -1,-1 },{ 0,-1 },
      { -1,-1 },{ 0,-1 },
      { -1,0 } ,{ 0,0 },
    },
    {
      { 0,0 },{ 0,0 },
      { 0,0 },{ 0,0 },
      { 0,0 },{ 0,0 },
    },
  };
  u8 i;
#if DEBUG_ONLY_FOR_ariizumi_nobuhiko | DEBUG_ONLY_FOR_ibe_mana
  GFL_BG_ClearScreenCodeVReq( PLIST_BG_PARAM , 0 );
  GFL_BG_ClearScreenCodeVReq( PLIST_BG_PLATE , 0 );
  for( i=0;i<6;i++ )
  {
    if( i%2 == work->pokeCursor%2 )
    {
      PLIST_PLATE_MovePlateXY( work , work->plateWork[i] , /*moveVal[pos][i][0]*/0 , moveVal[pos][i][1] );
    }
    else
    {
      PLIST_PLATE_MovePlateXY( work , work->plateWork[i] , 0 , 0 );
    }
  }
#endif
}

#pragma mark [>MenuSelect
//--------------------------------------------------------------------------
//  メニュー選択画面
//--------------------------------------------------------------------------
static void PLIST_SelectMenu( PLIST_WORK *work )
{
  switch( work->subSeq )
  {
  case PSSS_INIT:
    PLIST_SelectMenuInit( work );
    work->subSeq = PSSS_MAIN;
    break;
    
  case PSSS_MAIN:
    PLIST_MENU_UpdateMenu( work , work->menuWork );
    {
      const PLIST_MENU_ITEM_TYPE ret = PLIST_MENU_IsFinish( work , work->menuWork );
      if( ret != PMIT_NONE )
      {
        work->menuRet = ret;
        work->subSeq = PSSS_TERM;
      }
    }
    break;
    
  case PSSS_TERM:
    PLIST_SelectMenuTerm( work );
    
    PLIST_SelectMenuExit( work );
    break;
  }
  
}

//--------------------------------------------------------------
//	メニュー初期化
//--------------------------------------------------------------
static void PLIST_SelectMenuInit( PLIST_WORK *work )
{
  work->mainSeq = PSMS_MENU;
  work->subSeq  = PSSS_INIT;

  //BG・プレート・パラメータを見えにくくする
  G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 8 );
  //戻るアイコンだけカラー効果がかからないようにWindowを使う
  //FIXME 通信アイコン対応
  G2_SetWnd0Position( PLIST_BARICON_RETURN_X_MENU - 12 , PLIST_BARICON_Y - 12 ,
                      PLIST_BARICON_RETURN_X_MENU + 12 , PLIST_BARICON_Y + 12 );
  G2_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ , FALSE );
  G2_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ , TRUE );
  work->isActiveWindowMask = TRUE;

  {
    GFL_CLACTPOS curPos;
    curPos.x = PLIST_BARICON_RETURN_X_MENU;
    curPos.y = PLIST_BARICON_Y;
    GFL_CLACT_WK_SetPos( work->clwkBarIcon[PBT_RETURN] , &curPos , CLSYS_DRAW_MAIN );
    GFL_CLACT_WK_SetDrawEnable( work->clwkBarIcon[PBT_EXIT] , FALSE );
  }
  GFL_BG_LoadScreenV_Req(PLIST_BG_MENU);
}

//--------------------------------------------------------------
//	メニュー開放
//--------------------------------------------------------------
static void PLIST_SelectMenuTerm( PLIST_WORK *work )
{
  PLIST_MENU_CloseMenu( work , work->menuWork );
  PLIST_MSG_CloseWindow( work , work->msgWork );
  {
    GFL_CLACTPOS curPos;
    curPos.x = PLIST_BARICON_RETURN_X_BAR;
    curPos.y = PLIST_BARICON_Y;
    GFL_CLACT_WK_SetPos( work->clwkBarIcon[PBT_RETURN] , &curPos , CLSYS_DRAW_MAIN );
  }
  
  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_OBJ , 
                    GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ ,
                    12 , 16 );
  work->isActiveWindowMask = FALSE;
  GFL_BG_ClearScreenCodeVReq(PLIST_BG_MENU,0);
  
}

//--------------------------------------------------------------
//	メニュー戻り分岐
//--------------------------------------------------------------
static void PLIST_SelectMenuExit( PLIST_WORK *work )
{
  switch( work->menuRet )
  {
  case PMIT_CLOSE:
    PLIST_SelectPokeSetCursor( work , work->pokeCursor );
    work->selectPokePara = NULL;
    work->mainSeq = PSMS_SELECT_POKE;
    PLIST_InitMode_Select( work );
    break;

  case PMIT_CHANGE:
    PLIST_SelectPokeSetCursor( work , work->pokeCursor );
    PLIST_SelectPokeSetCursor_Change( work , work->pokeCursor );
    work->selectPokePara = NULL;
    work->changeTarget = work->pokeCursor;
    work->mainSeq = PSMS_CHANGE_POKE;
    work->canExit = FALSE;
    
    PLIST_MSG_OpenWindow( work , work->msgWork , PMT_BAR );
    PLIST_MSG_DrawMessageNoWait( work , work->msgWork , mes_pokelist_02_02 );
    PLIST_SelectPokeInit( work );
    
    break;

  case PMIT_ITEM:
    {
      PLIST_MENU_ITEM_TYPE itemArr[4] = {PMIT_GIVE,PMIT_TAKE,PMIT_CLOSE,PMIT_END_LIST};
      PLIST_SelectMenuInit( work );
      PLIST_MSG_OpenWindow( work , work->msgWork , PMT_MENU );
      PLIST_MSG_DrawMessageNoWait( work , work->msgWork , mes_pokelist_03_02 );
      PLIST_MENU_OpenMenu( work , work->menuWork , itemArr );
    }
    break;

  case PMIT_STATSU:
    {
      PSTATUS_DATA *psData = GFL_HEAP_AllocMemory( work->heapId , sizeof(PSTATUS_DATA) );
      
      psData->ppd = (void*)work->plData->pp;
      psData->cfg = work->plData->cfg;

      psData->ppt = PST_PP_TYPE_POKEPARTY;
      psData->max = PokeParty_GetPokeCount( work->plData->pp );;
      psData->mode = PST_MODE_NORMAL;
      psData->pos = work->pokeCursor;
      
      PLIST_ChangeProcInit( work , &PokeStatus_ProcData , FS_OVERLAY_ID(poke_status) , (void*)psData );
    }
    break;
    
  case PMIT_WAZA_1:
  case PMIT_WAZA_2:
  case PMIT_WAZA_3:
  case PMIT_WAZA_4:
  case PMIT_GIVE:
  case PMIT_TAKE:
    OS_TPrintf("まだ作ってない！\n");
    PLIST_SelectPokeSetCursor( work , work->pokeCursor );
    work->selectPokePara = NULL;
    work->mainSeq = PSMS_SELECT_POKE;
    PLIST_InitMode_Select( work );
    break;

  default:
    GF_ASSERT_MSG(0,"まだ作ってない！\n");

    PLIST_SelectPokeSetCursor( work , work->pokeCursor );
    work->selectPokePara = NULL;
    work->mainSeq = PSMS_SELECT_POKE;
    PLIST_InitMode_Select( work );
    break;
  }
}

//入れ替えアニメ
#pragma mark [>ChangeAnm
//--------------------------------------------------------------
//	入れ替えアニメ
//--------------------------------------------------------------
static void PLIST_ChangeAnime( PLIST_WORK *work )
{
  switch( work->subSeq )
  {
  case PSSS_MOVE_OUT:
    PLIST_PLATE_ClearPlate( work , work->plateWork[work->pokeCursor  ] , work->anmCnt );
    PLIST_PLATE_ClearPlate( work , work->plateWork[work->changeTarget] , work->anmCnt );
    work->anmCnt += PLIST_CHANGE_ANM_VALUE;
    PLIST_PLATE_MovePlate( work , work->plateWork[work->pokeCursor  ] , work->anmCnt );
    PLIST_PLATE_MovePlate( work , work->plateWork[work->changeTarget] , work->anmCnt );

    if( work->anmCnt >= PLIST_CHANGE_ANM_COUNT )
    {
      PokeParty_ExchangePosition( work->plData->pp , work->pokeCursor , work->changeTarget , work->heapId);
      {
        POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->pokeCursor);
        PLIST_PLATE_ResetParam(work , work->plateWork[work->pokeCursor] , pp , work->anmCnt );
      }
      {
        POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, work->changeTarget);
        PLIST_PLATE_ResetParam(work , work->plateWork[work->changeTarget] , pp , work->anmCnt );
      }
      //入れ替え
      work->subSeq = PSSS_MOVE_IN;
    }
    break;

  case PSSS_MOVE_IN:
    PLIST_PLATE_ClearPlate( work , work->plateWork[work->pokeCursor  ] , work->anmCnt );
    PLIST_PLATE_ClearPlate( work , work->plateWork[work->changeTarget] , work->anmCnt );
    work->anmCnt -= PLIST_CHANGE_ANM_VALUE;
    PLIST_PLATE_MovePlate( work , work->plateWork[work->pokeCursor  ] , work->anmCnt );
    PLIST_PLATE_MovePlate( work , work->plateWork[work->changeTarget] , work->anmCnt );

    if( work->anmCnt <= 0 )
    {
      //終了
      PLIST_ChangeAnimeTerm( work );
    }
    
    break;
  }
}

//--------------------------------------------------------------
//	入れ替えアニメ初期化
//--------------------------------------------------------------
static void PLIST_ChangeAnimeInit( PLIST_WORK *work )
{
  work->mainSeq = PSMS_CHANGE_ANM;
  work->subSeq = PSSS_MOVE_OUT;
  work->anmCnt = 0;

  GFL_CLACT_WK_SetDrawEnable( work->clwkCursor[0] , FALSE );
  GFL_CLACT_WK_SetDrawEnable( work->clwkCursor[1] , FALSE );
}

//--------------------------------------------------------------
//	入れ替えアニメ開放
//--------------------------------------------------------------
static void PLIST_ChangeAnimeTerm( PLIST_WORK *work )
{
  work->selectPokePara = NULL;
  work->mainSeq = PSMS_SELECT_POKE;

  PLIST_SelectPokeSetCursor( work , work->pokeCursor );
  PLIST_PLATE_SetActivePlate( work , work->plateWork[work->changeTarget] , FALSE );
  GFL_CLACT_WK_SetDrawEnable( work->clwkCursor[1] , FALSE );
  work->changeTarget = PL_SEL_POS_MAX;
  work->plData->ret_sel = work->pokeCursor;

  PLIST_InitMode_Select( work );
  
}

//--------------------------------------------------------------
//	入れ替えアニメプレート更新
//--------------------------------------------------------------
static void PLIST_ChangeAnimeUpdatePlate( PLIST_WORK *work )
{
}

#pragma mark [>changeProc
//--------------------------------------------------------------
//	Proc変更初期化
//--------------------------------------------------------------
static void PLIST_ChangeProcInit( PLIST_WORK *work , GFL_PROC_DATA *procData , FSOverlayID overlayId , void *parentWork )
{
  work->procData = procData;
  work->procOverlayId = overlayId;
  work->procParentWork = parentWork;
  work->changeProcSeq = PSCS_INIT;

  work->mainSeq = PSMS_CHANGEPROC;
  work->subSeq = PSSS_FADEOUT;
}

//--------------------------------------------------------------
//	Proc変更メイン
//--------------------------------------------------------------
static void PLIST_ChangeProcUpdate( PLIST_WORK *work )
{
  switch( work->subSeq )
  {   
  case PSSS_FADEOUT:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->subSeq = PSSS_INIT;
  
    break;
    
  case PSSS_INIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      work->reqChangeProc = TRUE;
    }
    break;
/*    
  case PSSS_MAIN:
    if( work->procParentWork != NULL )
    {
      GFL_HEAP_FreeMemory( work->procParentWork );
    }
    PLIST_InitPokeList( work );
    //上でmainSeqが切り替わるのでここで終了。
    break;
*/    
  }
}

#pragma mark [>util
//--------------------------------------------------------------
//	指定技箇所にフィールド技があるか？
//--------------------------------------------------------------
u32 PLIST_UTIL_CheckFieldWaza( const POKEMON_PARAM *pp , const u8 wazaPos )
{
  const u32 wazaID = PP_Get( pp , ID_PARA_waza1+wazaPos , NULL );
  
  switch( wazaID )
  {
  case WAZANO_IAIGIRI: //いあいぎり(15)
    return PL_RET_IAIGIRI;
    break;
  case WAZANO_SORAWOTOBU: //そらをとぶ(19)
    return PL_RET_SORAWOTOBU;
    break;
  case WAZANO_NAMINORI: //なみのり(57)
    return PL_RET_NAMINORI;
    break;
  case WAZANO_KAIRIKI: //かいりき(70)
    return PL_RET_KAIRIKI;
    break;
  case WAZANO_KIRIBARAI: //きりばらい(432)
    return PL_RET_KIRIBARAI;
    break;
  case WAZANO_IWAKUDAKI: //いわくだき(249)
    return PL_RET_IWAKUDAKI;
    break;
  case WAZANO_TAKINOBORI: //たきのぼり(127)
    return PL_RET_TAKINOBORI;
    break;
  case WAZANO_ROKKUKURAIMU: //ロッククライム(431)
    return PL_RET_ROCKCLIMB;
    break;

  case WAZANO_HURASSYU: //フラッシュ(148)
    return PL_RET_FLASH;
    break;
  case WAZANO_TEREPOOTO: //テレポート(100)
    return PL_RET_TELEPORT;
    break;
  case WAZANO_ANAWOHORU: //あなをほる(91)
    return PL_RET_ANAWOHORU;
    break;
  case WAZANO_AMAIKAORI: //あまいかおり(230)
    return PL_RET_AMAIKAORI;
    break;
  case WAZANO_OSYABERI: //おしゃべり(448)
    return PL_RET_OSYABERI;
    break;
  case WAZANO_MIRUKUNOMI: //ミルクのみ(208)
    return PL_RET_MILKNOMI;
    break;
  case WAZANO_TAMAGOUMI: //タマゴうみ(135)
    return PL_RET_TAMAGOUMI;
    break;

  default:
    return 0;
    break;
  }
}


#pragma mark [>debug

struct _PLIST_DEBUG_WORK
{
  u8 alpha1;
  u8 alpha2;
  u8 blend1;
};

#define PLIST_DEBUG_GROUP_NUMBER (50)
static void PLIST_DEB_Update_Alpha1( void* userWork , DEBUGWIN_ITEM* item );
static void PLIST_DEB_Draw_Alpha1( void* userWork , DEBUGWIN_ITEM* item );
static void PLIST_DEB_Update_Alpha2( void* userWork , DEBUGWIN_ITEM* item );
static void PLIST_DEB_Draw_Alpha2( void* userWork , DEBUGWIN_ITEM* item );
static void PLIST_DEB_Update_Blend1( void* userWork , DEBUGWIN_ITEM* item );
static void PLIST_DEB_Draw_Blend1( void* userWork , DEBUGWIN_ITEM* item );


static void PLIST_InitDebug( PLIST_WORK *work )
{
  work->debWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PLIST_DEBUG_WORK) );
  work->debWork->alpha1 = 12;
  work->debWork->alpha2 = 16;
  work->debWork->blend1 = 0;
  DEBUGWIN_InitProc( PLIST_BG_MENU , work->fontHandle );
  DEBUGWIN_ChangeLetterColor( 31,31,31 );
  
  DEBUGWIN_AddGroupToTop( PLIST_DEBUG_GROUP_NUMBER , "PokeList" , work->heapId );
  
  DEBUGWIN_AddItemToGroupEx( PLIST_DEB_Update_Alpha1   ,PLIST_DEB_Draw_Alpha1   , (void*)work , PLIST_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLIST_DEB_Update_Alpha2   ,PLIST_DEB_Draw_Alpha2   , (void*)work , PLIST_DEBUG_GROUP_NUMBER , work->heapId );
  DEBUGWIN_AddItemToGroupEx( PLIST_DEB_Update_Blend1   ,PLIST_DEB_Draw_Blend1   , (void*)work , PLIST_DEBUG_GROUP_NUMBER , work->heapId );
  
}

static void PLIST_TermDebug( PLIST_WORK *work )
{
  DEBUGWIN_RemoveGroup( PLIST_DEBUG_GROUP_NUMBER );
  DEBUGWIN_ExitProc();
  GFL_HEAP_FreeMemory( work->debWork );
}

//アルファ第1面光度
static void PLIST_DEB_Update_Alpha1( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = (PLIST_WORK*)userWork;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT && 
      work->debWork->alpha1 > 0 )
  {
    work->debWork->alpha1--;
    G2_ChangeBlendAlpha( work->debWork->alpha1 , work->debWork->alpha2 );
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT && 
      work->debWork->alpha1 < 16 )
  {
    work->debWork->alpha1++;
    G2_ChangeBlendAlpha( work->debWork->alpha1 , work->debWork->alpha2 );
    DEBUGWIN_RefreshScreen();
  }
}

static void PLIST_DEB_Draw_Alpha1( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = (PLIST_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "アルファ１[%2d]",work->debWork->alpha1 );
}

//アルファ第2面光度
static void PLIST_DEB_Update_Alpha2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = (PLIST_WORK*)userWork;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT && 
      work->debWork->alpha2 > 0 )
  {
    work->debWork->alpha2--;
    G2_ChangeBlendAlpha( work->debWork->alpha1 , work->debWork->alpha2 );
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT && 
      work->debWork->alpha2 < 16 )
  {
    work->debWork->alpha2++;
    G2_ChangeBlendAlpha( work->debWork->alpha1 , work->debWork->alpha2 );
    DEBUGWIN_RefreshScreen();
  }
}

static void PLIST_DEB_Draw_Alpha2( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = (PLIST_WORK*)userWork;
  DEBUGWIN_ITEM_SetNameV( item , "アルファ２[%2d]",work->debWork->alpha2 );
}

//ブレンド1面
static void PLIST_DEB_Update_Blend1( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = (PLIST_WORK*)userWork;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT || 
      GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    if( work->debWork->blend1 == 0 )
    {
      work->debWork->blend1 = 1;
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2 , 
                        GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ ,
                        work->debWork->alpha1 , work->debWork->alpha2 );
    }
    else
    {
      work->debWork->blend1 = 0;
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_OBJ , 
                        GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ ,
                        work->debWork->alpha1 , work->debWork->alpha2 );
    }
    DEBUGWIN_RefreshScreen();
  }
}

static void PLIST_DEB_Draw_Blend1( void* userWork , DEBUGWIN_ITEM* item )
{
  PLIST_WORK *work = (PLIST_WORK*)userWork;
  if( work->debWork->blend1 == 0 )
  {
    DEBUGWIN_ITEM_SetNameV( item , "ブレンド[BG2+OBJ]");
  }
  else
  {
    DEBUGWIN_ITEM_SetNameV( item , "ブレンド[BG2]");
  }
}
