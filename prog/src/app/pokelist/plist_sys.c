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
#include "pokeicon/pokeicon.h"

#include "plist_sys.h"
#include "plist_plate.h"

#include "test/ariizumi/ari_debug.h"

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
  
  PLIST_InitGraphic( work );
  PLIST_LoadResource( work );
  PLIST_InitCell( work );
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
  
  PLIST_PLATE_ChangeColor( work , work->plateWork[0] , PPC_NORMAL_SELECT );
  work->vBlankTcb = GFUser_VIntr_CreateTCB( PLIST_VBlankFunc , work , 8 );

  //まとめて読み込む
  GFL_BG_LoadScreenV_Req(PLIST_BG_MAIN_BG);
  GFL_BG_LoadScreenV_Req(PLIST_BG_PLATE);
  GFL_BG_LoadScreenV_Req(PLIST_BG_SUB_BG);

  //フェードないので仮処理
  GX_SetMasterBrightness(0);  
  GXS_SetMasterBrightness(0);

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
  
  for( i=0;i<PLIST_LIST_MAX;i++ )
  {
    PLIST_PLATE_DeletePlate( work , work->plateWork[i] );
  }

  PLIST_TermMessage( work );
  PLIST_TermCell( work );
  PLIST_ReleaseResource( work );
  PLIST_TermGraphic( work );

  return TRUE;
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
const BOOL PLIST_UpdatePokeList( PLIST_WORK *work )
{
  u8 i;

  for( i=0;i<PLIST_LIST_MAX;i++ )
  {
    PLIST_PLATE_UpdatePlate( work , work->plateWork[i] );
  }

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    return TRUE;
  }

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
  //OBJの更新
  GFL_CLACT_SYS_VBlankFunc();
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
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x08000,0x8000,
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
      GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x00000,0x05000,
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
  }
  
  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
    //TODO 適当
    work->cellUnit  = GFL_CLACT_UNIT_Create( 10 , 0, work->heapId );
    GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit );
  }
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
    GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x18000,0x8000,
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
  GFL_CLACT_UNIT_Delete( work->cellUnit );
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
  //セルの生成
  GFL_CLWK_DATA cellInitData;
  cellInitData.pos_x = 64;
  cellInitData.pos_y = 32;
  cellInitData.anmseq = PCA_NORMAL_A;
  cellInitData.softpri = 0;
  cellInitData.bgpri = 3;
  work->clwkCursor[0] = GFL_CLACT_WK_Create( work->cellUnit ,
            work->cellRes[PCR_NCG_CURSOR],
            work->cellRes[PCR_PLT_OBJ_COMMON],
            work->cellRes[PCR_ANM_CURSOR],
            &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

  cellInitData.pos_x = 192;
  cellInitData.pos_y = 40;
  cellInitData.anmseq = PCA_NORMAL_B;
  work->clwkCursor[1] = GFL_CLACT_WK_Create( work->cellUnit ,
            work->cellRes[PCR_NCG_CURSOR],
            work->cellRes[PCR_PLT_OBJ_COMMON],
            work->cellRes[PCR_ANM_CURSOR],
            &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );

  GFL_CLACT_WK_SetAutoAnmFlag( work->clwkCursor[0] , TRUE );
  GFL_CLACT_WK_SetAutoAnmFlag( work->clwkCursor[1] , TRUE );
  GFL_CLACT_WK_SetDrawEnable( work->clwkCursor[1] , FALSE );
}

//--------------------------------------------------------------------------
//  セル削除
//--------------------------------------------------------------------------
static void PLIST_TermCell( PLIST_WORK *work )
{
  GFL_CLACT_WK_Remove( work->clwkCursor[0] );
  GFL_CLACT_WK_Remove( work->clwkCursor[1] );
  
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
