//============================================================================================
/**
 * @file    badge_view.c
 * @brief   バッジ閲覧画面
 * @author  Akito Mori
 * @date    10.03.02
 *
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"            // HEAPID_LEADERBOARD
#include "system/cursor_move.h"     // CURSOR_MOVE
#include "system/gfl_use.h"         // GFUser_VIntr_CreateTCB
#include "system/wipe.h"            // WIPE_SYS_Start,
#include "sound/pm_sndsys.h"        // PMSND_PlaySE
#include "print/wordset.h"          // WORDSET
#include "print/printsys.h"         // PRINT_UTIL,PRINT_QUE...
#include "app/leader_board.h"       // proc
#include "app/app_menu_common.h"
#include "arc/arc_def.h"            // ARCID_LEADER_BOARD
#include "arc/trainer_case.naix"    // leaderboard graphic

#include "font/font.naix"           // NARC_font_large_gftr
#include "arc/message.naix"         // NARC_message_leader_board_dat
#include "msg/msg_trainercard.h"

#include "badge_view_def.h"       // 各種定義等

#ifdef PM_DEBUG
#define SPECIAL_FEBRUARY
#endif

//--------------------------------------------------------------------------------------------
// 定数定義
//--------------------------------------------------------------------------------------------


// シーケンス制御
enum{
  SUBSEQ_FADEIN_WAIT=0,
  SUBSEQ_MAIN,
  SUBSEQ_ICON_WAIT,
  SUBSEQ_FADEOUT,
  SUBSEQ_FADEOUT_WAIT,
};


// BG指定
#define BV_BGFRAME_U_MSG    ( GFL_BG_FRAME0_S )
#define BV_BGFRAME_U_BG     ( GFL_BG_FRAME1_S )
#define BV_BGFRAME_D_MSG    ( GFL_BG_FRAME0_M )
#define BV_BGFRAME_D_BUTTON ( GFL_BG_FRAME1_M )
#define BV_BGFRAME_D_BG     ( GFL_BG_FRAME2_M )



//--------------------------------------------------------------------------------------------
// 構造体定義定義
//--------------------------------------------------------------------------------------------
typedef struct {
  ARCHANDLE     *g_handle;  // グラフィックデータファイルハンドル
  
  GFL_FONT      *font;                // フォントデータ
  GFL_BMPWIN    *InfoWin;
  PRINT_UTIL    printUtil;
  PRINT_QUE     *printQue;
  PRINT_STREAM  *printStream;
  GFL_TCBLSYS   *pMsgTcblSys;

  GFL_MSGDATA   *mman;    // メッセージデータマネージャ
  WORDSET       *wset;    // 単語セット
  STRBUF        *expbuf;  // メッセージ展開領域
  STRBUF        *strbuf[BV_STR_MAX];

  GFL_CLUNIT    *clUnit;             // セルアクターユニット
  GFL_CLWK      *clwk[BV_OBJ_MAX];   // セルアクターワーク
  u32           clres[BV_RES_MAX];   // セルアクターリソース用インデックス

  GFL_TCB         *VblankTcb;       // 登録したVblankFuncのポインタ
  
  LEADERBOARD_PARAM *param;         // 呼び出しパラメータ
  int seq,next_seq;                 // サブシーケンス制御用ワーク
  u16 page,page_max;                // 現在の表示ページ

  int trainer_num;                  // バトルサブウェイデータに存在していた人数

#ifdef PM_DEBUG
  GFL_MSGDATA   *debugname;
#endif
} BADGEVIEW_WORK;

GFL_PROC_RESULT BadgeViewProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
GFL_PROC_RESULT BadgeViewProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
GFL_PROC_RESULT BadgeViewProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk );


//--------------------------------------------------------------------------------------------
// 実データ定義
//--------------------------------------------------------------------------------------------
// プロセス定義データ
const GFL_PROC_DATA BadgeViewProcData = {
  BadgeViewProc_Init,
  BadgeViewProc_Main,
  BadgeViewProc_End,
};



//--------------------------------------------------------------------------------------------
// 関数プロトタイプ宣言
//--------------------------------------------------------------------------------------------
/*** 関数プロトタイプ ***/
static void VBlankFunc( GFL_TCB *tcb, void * work );
static void InitWork( BADGEVIEW_WORK *wk, void *pwk );
static void FreeWork( BADGEVIEW_WORK *wk );
static void VramBankSet(void);
static void BgInit(BADGEVIEW_WORK *wk);
static void BgExit(BADGEVIEW_WORK *wk) ;
static void BgGraphicInit(BADGEVIEW_WORK *wk);
static void BgGraphicExit( BADGEVIEW_WORK *wk );
static void ClActInit(BADGEVIEW_WORK *wk);
static void ClActSet(BADGEVIEW_WORK *wk) ;
static void ClActExit(BADGEVIEW_WORK *wk);
static void BmpWinInit(BADGEVIEW_WORK *wk);
static void BmpWinExit(BADGEVIEW_WORK *wk);
static void PrintSystemInit(BADGEVIEW_WORK *wk);
static void PrintSystemDelete(BADGEVIEW_WORK *wk);
static void PrintSystem_Main( BADGEVIEW_WORK *wk );
static void BgFramePrint( BADGEVIEW_WORK *wk, int id, STRBUF *str, STRBUF *str2, STRBUF *str3 );
static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos );
static BOOL LBSEQ_Main( BADGEVIEW_WORK *wk );
static void Rewrite_UpperInformation( BADGEVIEW_WORK *wk, int id );
static  int TouchBar_KeyControl( BADGEVIEW_WORK *wk );
static void ExecFunc( BADGEVIEW_WORK *wk, int touch );
static void _page_max_init( BADGEVIEW_WORK *wk );
static void _page_clact_set( BADGEVIEW_WORK *wk, int page, int max );
static int _page_move_check( BADGEVIEW_WORK *wk, int touch );
static void SetupPage( BADGEVIEW_WORK *wk, int page );
static void NamePlatePrint_1Page( BADGEVIEW_WORK *wk );
static void CellActorPosSet( BADGEVIEW_WORK *wk, int id, int x, int y );


#ifdef PM_DEBUG
static void _debug_data_set( BADGEVIEW_WORK *wk );
#endif


//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：初期化
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT BadgeViewProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  BADGEVIEW_WORK * wk;

  OS_Printf( "↓↓↓↓↓　リーダーボード処理開始　↓↓↓↓↓\n" );

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_LEADERBOARD, 0x20000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(BADGEVIEW_WORK), HEAPID_LEADERBOARD );
  MI_CpuClearFast( wk, sizeof(BADGEVIEW_WORK) );

  WIPE_SetBrightness( WIPE_DISP_MAIN,WIPE_FADE_BLACK );
  WIPE_SetBrightness( WIPE_DISP_SUB,WIPE_FADE_BLACK );

  InitWork( wk, pwk );  // ワーク初期化
  VramBankSet();        // VRAM設定
  BgInit(wk);           // BGシステム初期化
  BgGraphicInit(wk);    // BGグラフィック転送
  ClActInit(wk);        // セルアクターシステム初期化
  ClActSet(wk);         // セルアクター登録
  BmpWinInit(wk);       // BMPWIN初期化
  PrintSystemInit(wk);  // 描画システム初期化

  // 初期ページ表示
  _page_clact_set( wk, wk->page, wk->page_max );
  SetupPage( wk, wk->page );


  // フェードイン開始
  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
                  WIPE_FADE_BLACK, 16, 1, HEAPID_LEADERBOARD );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：メイン
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT BadgeViewProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  BADGEVIEW_WORK * wk = mywk;

  if( LBSEQ_Main( wk ) == FALSE ){
    return GFL_PROC_RES_FINISH;
  }
  
  PrintSystem_Main( wk );         // 文字列描画メイン
  GFL_CLACT_SYS_Main();           // セルアクターメイン
  
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：終了
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT BadgeViewProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  BADGEVIEW_WORK * wk = mywk;
  
  PrintSystemDelete(wk);  // 描画システム解放
  BmpWinExit(wk);         // BMPWIN解放
  ClActExit(wk);          // セルアクターシステム解放
  BgGraphicExit(wk);      // BGグラフィック関連終了
  BgExit(wk);             // BGシステム終了
  FreeWork(wk);           // ワーク解放

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_CheckHeapSafe( HEAPID_LEADERBOARD );
  GFL_HEAP_DeleteHeap( HEAPID_LEADERBOARD );


  OS_Printf( "↑↑↑↑↑　リーダーボード処理終了　↑↑↑↑↑\n" );

  return GFL_PROC_RES_FINISH;
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
/**
 * 関数
 *
 * @param none
 *
 * @return  none
 */
//----------------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB *tcb, void * work )
{
  // セルアクター
  GFL_CLACT_SYS_VBlankFunc();

  // BG転送  
  GFL_BG_VBlankFunc();
  
}


//----------------------------------------------------------------------------------
/**
 * @brief ワーク初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void InitWork( BADGEVIEW_WORK *wk, void *pwk )
{
  wk->param     = pwk;
  wk->next_seq  = SUBSEQ_FADEIN_WAIT;

  // グラフィックデータハンドルオープン
  wk->g_handle  = GFL_ARC_OpenDataHandle( ARCID_TRAINERCARD, HEAPID_LEADERBOARD);

  // メッセージマネージャー確保  
  wk->mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                             NARC_message_trainercard_dat, HEAPID_LEADERBOARD );


  // leader_board.gmmの文字列を全て読み込む
  {
    int i;
    const STRCODE *buf;
    for(i=0;i<BV_STR_MAX;i++){
      wk->strbuf[i] = GFL_MSG_CreateString( wk->mman, i );
      buf = GFL_STR_GetStringCodePointer( wk->strbuf[i] );
      OS_Printf("str%d=%02d, %02d, %02d, %02d, %02d,\n", i,buf[0],buf[1],buf[2],buf[3],buf[4]);
    }
  }
  // 展開用文字列確保
  wk->expbuf = GFL_STR_CreateBuffer( BUFLEN_PMS_WORD*2, HEAPID_LEADERBOARD );

  // ワードセットワーク確保
  wk->wset = WORDSET_CreateEx( 8, WORDSET_COUNTRY_BUFLEN, HEAPID_LEADERBOARD );
  
  
  // VBlank関数セット
  wk->VblankTcb = GFUser_VIntr_CreateTCB( VBlankFunc, wk, 0 );  
  


#ifdef PM_DEBUG
  wk->debugname = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                             NARC_message_trname_dat, HEAPID_LEADERBOARD );

  // デバッグ用データセット
  _debug_data_set( wk );

  GFL_MSG_Delete( wk->debugname );

#endif
  
  // トレーナー数数え上げ
  wk->trainer_num = 8;

  // 最大ページ数取得
  _page_max_init(wk);
}


//----------------------------------------------------------------------------------
/**
 * @brief ワーク解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void FreeWork( BADGEVIEW_WORK *wk )
{
  int i;
  
  
  // 文字列解放
  for(i=0;i<BV_STR_MAX;i++){
    GFL_STR_DeleteBuffer( wk->strbuf[i] );
  }
  GFL_STR_DeleteBuffer( wk->expbuf );

  // ワードセット解放
  WORDSET_Delete( wk->wset );

  // メッセージデータ解放
  GFL_MSG_Delete( wk->mman );


  // Vblank期間中のBG転送終了
  GFL_TCB_DeleteTask( wk->VblankTcb );

  GFL_ARC_CloseDataHandle( wk->g_handle );
}

//============================
// VRAM割り振り
//============================
static const GFL_DISP_VRAM VramTbl = {
  GX_VRAM_BG_128_B,           // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット

  GX_VRAM_OBJ_64_E,           // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_0_A,            // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_01_FG,      // テクスチャパレットスロット

  GX_OBJVRAMMODE_CHAR_1D_32K, // メインＯＢＪ領域
  GX_OBJVRAMMODE_CHAR_1D_32K, // サブＯＢＪ領域
};

//----------------------------------------------------------------------------------
/**
 * @brief VRAM設定
 *
 * @param   none    
 */
//----------------------------------------------------------------------------------
static void VramBankSet(void)
{
  GFL_DISP_SetBank( &VramTbl );

  // メイン画面を下に表示
  GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

}


// 初期化を行うBG面
static const int bgframe_init_tbl[]={
  BV_BGFRAME_U_MSG,
  BV_BGFRAME_U_BG,
  BV_BGFRAME_D_MSG,
  BV_BGFRAME_D_BUTTON,
  BV_BGFRAME_D_BG,
};

//----------------------------------------------------------------------------------
/**
 * @brief BGシステム初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgInit(BADGEVIEW_WORK *wk)
{
  int i;

  // BG SYSTEM初期化
  GFL_BG_Init( HEAPID_LEADERBOARD );

  // BGモード設定
  { 
    GFL_BG_SYS_HEADER sysh = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &sysh );
  }

  {
    // 各BG初期化設定
    const GFL_BG_BGCNT_HEADER header[]={
      { // サブ画面：文字面
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01,      0, 0, 0, FALSE
      },
      { // サブ画面：背景
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x128,
        GX_BG_EXTPLTT_01,    1, 0, 0, FALSE
      },
      { // メイン画面：文字
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01,      0, 0, 0, FALSE
      },
      { // メイン画面：ボタン
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x128,
        GX_BG_EXTPLTT_01,      2, 0, 0, FALSE
      },
      { // メイン画面：背景
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x128,
        GX_BG_EXTPLTT_01,      3, 0, 0, FALSE
      },
    };

    GF_ASSERT( NELEMS(header)==NELEMS(bgframe_init_tbl) );

    // BG設定・スクリーンクリア・０キャラ目クリア
    for(i=0;i<NELEMS(bgframe_init_tbl);i++)
    {
      GFL_BG_SetBGControl( bgframe_init_tbl[i], &header[i], GFL_BG_MODE_TEXT );
      GFL_BG_ClearScreen(  bgframe_init_tbl[i] );
      GFL_BG_SetClearCharacter( bgframe_init_tbl[i], 0x20, 0, HEAPID_LEADERBOARD );
      GFL_BG_SetVisible(  bgframe_init_tbl[i], VISIBLE_ON     );
    }
    
    // 残りのBG面をOFFに
    GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_OFF );
  }
}
//----------------------------------------------------------------------------------
/**
 * @brief  BGシステム終了
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgExit(BADGEVIEW_WORK *wk) 
{
  int i;
  for(i=0;i<NELEMS(bgframe_init_tbl);i++){
    GFL_BG_FreeBGControl( bgframe_init_tbl[i] );
  }
  GFL_BG_Exit();

}

//----------------------------------------------------------------------------------
/**
 * @brief BGグラフィック転送
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgGraphicInit(BADGEVIEW_WORK *wk)
{
  ARCHANDLE * handle = wk->g_handle;

  // サブ画面背景転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_trainer_case_badge_bg_02_NCGR,
                                        BV_BGFRAME_U_BG, 0, 0, FALSE, HEAPID_LEADERBOARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_trainer_case_badge_bg03_NSCR,
                                        BV_BGFRAME_U_BG, 0, 0, FALSE, HEAPID_LEADERBOARD );
  GFL_ARCHDL_UTIL_TransVramPalette(     handle, NARC_trainer_case_badge_bg02_NCLR, 
                                        PALTYPE_SUB_BG, 0, 0, HEAPID_LEADERBOARD );

  // メイン画面背景転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_trainer_case_badge_bg01_NCGR,
                                        BV_BGFRAME_D_BG, 0, 0, FALSE, HEAPID_LEADERBOARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_trainer_case_badge_bg02_NSCR,
                                        BV_BGFRAME_D_BG, 0, 0, FALSE, HEAPID_LEADERBOARD );
  GFL_ARCHDL_UTIL_TransVramPalette(     handle, NARC_trainer_case_badge_bg01_NCLR, 
                                        PALTYPE_MAIN_BG, 0, 0, HEAPID_LEADERBOARD );

  // メイン画面背景転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_trainer_case_badge_bg01_NCGR,
                                        BV_BGFRAME_D_MSG, 0, 0, FALSE, HEAPID_LEADERBOARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_trainer_case_badge_bg01_NSCR,
                                        BV_BGFRAME_D_MSG, 0, 0, FALSE, HEAPID_LEADERBOARD );
  
  // タッチバーBG転送
  {
    ARCHANDLE *c_handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_LEADERBOARD);

    GFL_ARCHDL_UTIL_TransVramBgCharacter( c_handle, APP_COMMON_GetBarCharArcIdx(),
                                          BV_BGFRAME_D_BUTTON, 0, 0, FALSE, HEAPID_LEADERBOARD );
    GFL_ARCHDL_UTIL_TransVramScreen(      c_handle, APP_COMMON_GetBarScrnArcIdx(),
                                          BV_BGFRAME_D_BUTTON, 0, 0, FALSE, HEAPID_LEADERBOARD );
    GFL_ARCHDL_UTIL_TransVramPaletteEx(   c_handle, APP_COMMON_GetBarPltArcIdx(), PALTYPE_MAIN_BG,
                                          0, TOUCHBAR_PAL*32, 32, HEAPID_LEADERBOARD );
    
    // スクリーンのパレット指定を変更
    GFL_BG_ChangeScreenPalette( BV_BGFRAME_D_BUTTON, TOUCHBAR_X, TOUCHBAR_Y,
                                                     TOUCHBAR_W, TOUCHBAR_H, TOUCHBAR_PAL );
    GFL_BG_LoadScreenReq( BV_BGFRAME_D_BUTTON );

    GFL_ARC_CloseDataHandle( c_handle );
  }
  
  // 会話フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
                                 TALKFONT_PAL_OFFSET, 32, HEAPID_LEADERBOARD );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 
                                 TALKFONT_PAL_OFFSET, 32, HEAPID_LEADERBOARD );


}


//----------------------------------------------------------------------------------
/**
 * @brief BG関連システム終了処理
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgGraphicExit( BADGEVIEW_WORK *wk )
{
}

//----------------------------------------------------------------------------------
/**
 * @brief セルアクターシステム初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ClActInit(BADGEVIEW_WORK *wk)
{
  ARCHANDLE *c_handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_LEADERBOARD);

  // セルアクター初期化
  GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN, &VramTbl, HEAPID_LEADERBOARD );
  wk->clUnit  = GFL_CLACT_UNIT_Create( BV_CLACT_NUM, 1,  HEAPID_LEADERBOARD );

  // OBJ面表示ON
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

  // セルアクターリソース読み込み
  
  // ---リーダーボード用下画面---
  wk->clres[BV_RES_CHR]  = GFL_CLGRP_CGR_Register(      wk->g_handle, 
                                                        NARC_trainer_case_badge_obj01_NCGR, 0, 
                                                        CLSYS_DRAW_MAIN, HEAPID_LEADERBOARD );
  wk->clres[BV_RES_PLTT] = GFL_CLGRP_PLTT_Register(     wk->g_handle, 
                                                        NARC_trainer_case_badge_obj01_NCLR, 
                                                        CLSYS_DRAW_MAIN, 0, HEAPID_LEADERBOARD );
  wk->clres[BV_RES_CELL] = GFL_CLGRP_CELLANIM_Register( wk->g_handle, 
                                                        NARC_trainer_case_badge_obj01_NCER, 
                                                        NARC_trainer_case_badge_obj01_NANR, 
                                                        HEAPID_LEADERBOARD );

  // 共通メニュー素材
  wk->clres[BV_RES_COMMON_CHR] = GFL_CLGRP_CGR_Register(       c_handle, 
                                                               APP_COMMON_GetBarIconCharArcIdx(), 0, 
                                                               CLSYS_DRAW_MAIN, 
                                                               HEAPID_LEADERBOARD );

  wk->clres[BV_RES_COMMON_PLTT] = GFL_CLGRP_PLTT_RegisterEx(   c_handle, 
                                                               APP_COMMON_GetBarIconPltArcIdx(), 
                                                               CLSYS_DRAW_MAIN, 
                                                               11*32, 0, 3, HEAPID_LEADERBOARD );

  wk->clres[BV_RES_COMMON_CELL] = GFL_CLGRP_CELLANIM_Register( c_handle, 
                                                               APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_32K), 
                                                               APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_32K), 
                                                               HEAPID_LEADERBOARD );
  GFL_ARC_CloseDataHandle( c_handle );

}


static const int clact_dat[][6]={
  //   X      Y  anm, chr,               pltt,               cell
  {  1*8,  21*8,  11, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// カードに戻る
  { 28*8,  21*8,   1, BV_RES_COMMON_CHR, BV_RES_COMMON_CHR,  BV_RES_COMMON_CHR },// もどる
  { 24*8,  21*8,   0, BV_RES_COMMON_CHR, BV_RES_COMMON_CHR,  BV_RES_COMMON_CHR },// ×
  { 20*8,  21*8+4,   7, BV_RES_COMMON_CHR, BV_RES_COMMON_CHR,  BV_RES_COMMON_CHR },// Ｙチェック
  {  2*8,  16*8,   0, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// バッジ0
  {  6*8,  16*8,   1, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// バッジ1
  { 10*8,  16*8,   2, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// バッジ2
  { 14*8,  16*8,   3, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// バッジ3
  { 18*8,  16*8,   4, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// バッジ4
  { 22*8,  16*8,   5, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// バッジ5
  { 26*8,  16*8,   6, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// バッジ6
  { 30*8,  16*8,   7, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// バッジ7
  {  (4*0+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ0-１
  {  (4*0+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ0-２
  {  (4*0+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ0-３
  {  (4*1+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ1-１
  {  (4*1+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ1-２
  {  (4*1+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ1-３
  {  (4*2+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ2-１
  {  (4*2+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ2-２
  {  (4*2+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ2-３
  {  (4*3+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ3-１
  {  (4*3+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ3-２
  {  (4*3+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ3-３
  {  (4*4+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ4-１
  {  (4*4+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ4-２
  {  (4*4+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ4-３
  {  (4*5+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ5-１
  {  (4*5+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ5-２
  {  (4*5+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ5-３
  {  (4*6+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ6-１
  {  (4*6+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ6-２
  {  (4*6+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ6-３
  {  (4*7+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ7-１
  {  (4*7+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ7-２
  {  (4*7+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,      },// キラキラ7-３
};

//----------------------------------------------------------------------------------
/**
 * @brief セルアクター登録
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ClActSet(BADGEVIEW_WORK *wk) 
{

  GFL_CLWK_DATA add;
  int i;

  // セルアクター登録
  for(i=0;i<BV_OBJ_MAX;i++){
    add.pos_x  = clact_dat[i][0];
    add.pos_y  = clact_dat[i][1];
    add.anmseq = clact_dat[i][2];
    add.bgpri    = 0;
    add.softpri  = BV_OBJ_MAX-i;

    wk->clwk[i] = GFL_CLACT_WK_Create( wk->clUnit,
                                       wk->clres[clact_dat[i][3]],
                                       wk->clres[clact_dat[i][4]],
                                       wk->clres[clact_dat[i][5]],
                                       &add, CLSYS_DEFREND_MAIN, HEAPID_LEADERBOARD );
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[i], TRUE );
  }
  
}



//----------------------------------------------------------------------------------
/**
 * @brief セルアクターシステム解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ClActExit(BADGEVIEW_WORK *wk)
{

  GFL_CLGRP_CGR_Release(      wk->clres[BV_RES_COMMON_CHR] );
  GFL_CLGRP_PLTT_Release(     wk->clres[BV_RES_COMMON_PLTT] );
  GFL_CLGRP_CELLANIM_Release( wk->clres[BV_RES_COMMON_CELL] );
  GFL_CLGRP_CGR_Release(      wk->clres[BV_RES_CHR] );
  GFL_CLGRP_PLTT_Release(     wk->clres[BV_RES_PLTT] );
  GFL_CLGRP_CELLANIM_Release( wk->clres[BV_RES_CELL] );


  // セルアクターユニット破棄
  GFL_CLACT_UNIT_Delete( wk->clUnit );

  //OAMレンダラー破棄
  GFL_CLACT_SYS_Delete();

}

// BMPWIN初期化用構造体
typedef struct{
  u32 frame;
  u8  x,y,w,h;
  u16 pal, chr;
}BMPWIN_DAT;


static const BMPWIN_DAT bmpwin_dat={
  BV_BGFRAME_U_MSG,
   2, 10,
  28, 12,
  4,  
};

//----------------------------------------------------------------------------------
/**
 * @brief BMPWIN初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BmpWinInit(BADGEVIEW_WORK *wk)
{
  int i;
  const BMPWIN_DAT *windat;

  // BMPWINシステム開始
  GFL_BMPWIN_Init( HEAPID_LEADERBOARD );
 
  // サブ画面のBMPWINを確保
  windat = &bmpwin_dat;
  wk->InfoWin = GFL_BMPWIN_Create( windat->frame,
                                   windat->x, windat->y,
                                   windat->w, windat->h,
                                   windat->pal, GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin), 0x0f0f );
  GFL_BMPWIN_MakeTransWindow( wk->InfoWin );
  

}

//----------------------------------------------------------------------------------
/**
 * @brief BMPWIN解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BmpWinExit(BADGEVIEW_WORK *wk)
{
  
  // サブ画面用BMPWINを解放
  GFL_BMPWIN_Delete( wk->InfoWin );

  // BMPWINシステム終了
  GFL_BMPWIN_Exit();

}


#define BV_COL_BLACK      ( PRINTSYS_LSB_Make(  1,  2, 15) )    // 黒

//----------------------------------------------------------------------------------
/**
 * @brief 文字描画システム初期化
 *

 */
//----------------------------------------------------------------------------------
static void PrintSystemInit(BADGEVIEW_WORK *wk)
{
  int i;
  // フォント確保
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr ,
                              GFL_FONT_LOADTYPE_FILE , FALSE , HEAPID_LEADERBOARD );

  // 描画待機システム初期化
  wk->printQue = PRINTSYS_QUE_Create( HEAPID_LEADERBOARD );

  // BMPWINとPRINT_UTILを関連付け
  PRINT_UTIL_Setup( &wk->printUtil, wk->InfoWin );
  
}

//----------------------------------------------------------------------------------
/**
 * @brief 文字描画システム解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void PrintSystemDelete(BADGEVIEW_WORK *wk)
{
  
  // メッセージ表示用システム解放
  PRINTSYS_QUE_Delete( wk->printQue );

  // フォント解放
  GFL_FONT_Delete( wk->font );
  
}

//----------------------------------------------------------------------------------
/**
 * @brief 文字描画システムメイン
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void PrintSystem_Main( BADGEVIEW_WORK *wk )
{
  int i;

  // 描画待ち
  PRINTSYS_QUE_Main( wk->printQue );

  // 描画終了済であれば転送
  PRINT_UTIL_Trans( &wk->printUtil, wk->printQue );

}















//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// サブシーケンス
//----------------------------------------------------------------------------------

static BOOL SubSuq_FadeinWait( BADGEVIEW_WORK *wk );
static BOOL SubSuq_Main( BADGEVIEW_WORK *wk );
static BOOL SubSeq_IconWait( BADGEVIEW_WORK *wk );
static BOOL SubSuq_FadeOut( BADGEVIEW_WORK *wk );
static BOOL SubSuq_FadeOutWait( BADGEVIEW_WORK *wk );


static BOOL (*functable[])(BADGEVIEW_WORK *wk) = {
  SubSuq_FadeinWait,      // SUBSEQ_FADEIN_WAIT;
  SubSuq_Main,            // SUBSEQ_MAIN 
  SubSeq_IconWait,        // SUBSEQ_ICON_WAIT,
  SubSuq_FadeOut,         // SUBSEQ_FADEOUT
  SubSuq_FadeOutWait,     // SUBSEQ_FADEOUT_WAIT
};

//----------------------------------------------------------------------------------
/**
 * @brief サブシーケンス制御メイン
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL LBSEQ_Main( BADGEVIEW_WORK *wk )
{
  // サブシーケンス実行
  return functable[wk->seq](wk);
}


//----------------------------------------------------------------------------------
/**
 * @brief 【SUBSEQ】フェードイン待ち
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SubSuq_FadeinWait( BADGEVIEW_WORK *wk )
{
  if( WIPE_SYS_EndCheck() ){
    wk->seq = SUBSEQ_MAIN;
  }
  return TRUE;
}


//----------------------------------------------------------------------------------
/**
 * @brief 【SUBSEQ】メイン処理
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SubSuq_Main( BADGEVIEW_WORK *wk )
{
  u32 ret;
  // タッチバー処理
  if(TouchBar_KeyControl(wk)==GFL_UI_TP_HIT_NONE){

  }
  return TRUE;
}


//----------------------------------------------------------------------------------
/**
 * @brief 戻るアイコンのアニメ待ち
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_IconWait( BADGEVIEW_WORK *wk )
{
  if( GFL_CLACT_WK_CheckAnmActive( wk->clwk[BV_OBJ_END] )==FALSE){
    wk->seq = SUBSEQ_FADEOUT;
  }
  return TRUE;
}

//----------------------------------------------------------------------------------
/**
 * @brief 【SUBSEQ】フェードアウト
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SubSuq_FadeOut( BADGEVIEW_WORK *wk )
{
  // フェードイン開始
  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
                  WIPE_FADE_BLACK, 16, 1, HEAPID_LEADERBOARD );
  wk->seq = SUBSEQ_FADEOUT_WAIT;

  return TRUE;
}


//----------------------------------------------------------------------------------
/**
 * @brief 【SUBSEQ】フェードアウト待ち
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SubSuq_FadeOutWait( BADGEVIEW_WORK *wk )
{
  if( WIPE_SYS_EndCheck() ){
    wk->seq = 0;
    return FALSE;
  }
  return TRUE;
}














//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// インターフェース処理
//----------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
/**
 * セルタクター座標位置セット
 *
 * @param work    
 * @param id    セルアクターインデックス
 * @param x     位置X
 * @param y     位置Y
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CellActorPosSet( BADGEVIEW_WORK *wk, int id, int x, int y )
{
  GFL_CLACTPOS clpos;
  clpos.x = x;
  clpos.y = y;

  GFL_CLACT_WK_SetPos( wk->clwk[id], &clpos, CLSYS_DEFREND_MAIN );

}





//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// 機能関数


//----------------------------------------------------------------------------------
/**
 * @brief 上画面情報書き換え
 *
 * @param   wk    BADGEVIEW_WORK
 * @param   id    何番目か
 */
//----------------------------------------------------------------------------------
static void Rewrite_UpperInformation( BADGEVIEW_WORK *wk,  int id )
{
  int nation     = 0;
  int local      = 0;
  STRBUF *strbuf = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, HEAPID_LEADERBOARD );
  STRBUF *pmsstr;
  
  // 各種情報登録

  // ランクＮＯ
  WORDSET_RegisterNumber( wk->wset, 0, wk->param->rank_no,  2, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU);
  // トレインNO
  WORDSET_RegisterNumber( wk->wset, 1, wk->param->train_no, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU);
  WORDSET_RegisterCountryName(    wk->wset, 2, nation );          // 国
  WORDSET_RegisterLocalPlaceName( wk->wset, 3, nation, local );   // 地方


  // 描画領域クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin), 0x0f0f );

  // 描画

  PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, 
                         0, 0, strbuf, wk->font, BV_COL_BLACK );
  
  GFL_STR_DeleteBuffer( strbuf );

}


static const GFL_UI_TP_HITTBL TouchButtonHitTbl[] =
{
  { 168, 191,   8,  8+24 },   // FUNC_LEFT_PAGE:  左矢印
  { 168, 191,  32,  32+24 },  // FUNC_RIGHT_PAGE: 右矢印
  { 168, 191, 232,  255 },    // FUNC_END:        やめる
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};

//=============================================================================================
/**
 * @brief タッチバー制御
 *
 * @param   wk    
 *
 * @retval  staitc BOOL   
 */
//=============================================================================================
static int TouchBar_KeyControl( BADGEVIEW_WORK *wk )
{
  int  touch;
  touch = GFL_UI_TP_HitTrg( TouchButtonHitTbl );

  // キーでキャンセル
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
    PMSND_PlaySE( SEQ_SE_CANCEL1 );
    touch = FUNC_END;
  }
  
  // 左ページ・右ページ・戻る機能の呼び出し
  ExecFunc(wk, touch);
  
  return touch;
}


//----------------------------------------------------------------------------------
/**
 * @brief 各機能（左ページ・右ページ・戻る）の実行
 *
 * @param   wk    
 * @param   touch   
 */
//----------------------------------------------------------------------------------
static void ExecFunc( BADGEVIEW_WORK *wk, int touch )
{
  switch(touch){
  case FUNC_LEFT_PAGE:
    if(_page_move_check( wk, touch )){
      PMSND_PlaySE( SEQ_SE_SELECT1 );
      SetupPage( wk, wk->page );
    }
    break;
  case FUNC_RIGHT_PAGE:
    if(_page_move_check( wk, touch )){
      PMSND_PlaySE( SEQ_SE_SELECT1 );
      SetupPage( wk, wk->page );
    }
    break;
  case FUNC_END:
    wk->seq = SUBSEQ_ICON_WAIT;
    GFL_CLACT_WK_SetAnmSeq( wk->clwk[BV_OBJ_END], 9 );
    break;
  }

}


//----------------------------------------------------------------------------------
/**
 * @brief ページ総数算出
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void _page_max_init( BADGEVIEW_WORK *wk )
{
  wk->page = 0;
  wk->page_max = wk->trainer_num /BV_1PAGE_NUM;
  if(wk->trainer_num%BV_1PAGE_NUM){
    wk->page_max++;
  }
  OS_Printf("page=%d, trainer_num=%d, max=%d\n", wk->page, wk->trainer_num, wk->page_max);
}

#define ISSHU_BADGE_NUM   ( 8 )

//----------------------------------------------------------------------------------
/**
 * @brief   矢印アイコンの状態をページ数で変化させる
 * @todo ジムバッジフラグを見る
 *
 *
 * @param   wk    
 * @param   page    
 * @param   max   
 */
//----------------------------------------------------------------------------------
static void _page_clact_set( BADGEVIEW_WORK *wk, int page, int max )
{
  int i;
  // １ページしかなかったら矢印表示しない
  for(i=0;i<ISSHU_BADGE_NUM;i++){
    if(1){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BV_OBJ_BADGE0+i],  TRUE );
    }else{
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BV_OBJ_BADGE0+i],  FALSE );
    }
  }

  for(i=0;i<ISSHU_BADGE_NUM*3;i++){
    if(1){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BV_OBJ_KIRAKIRA0_0+i],  TRUE );
    }else{
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BV_OBJ_KIRAKIRA0_0+i],  FALSE );
    }
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief 移動可能かチェック
 *
 * @param   wk    
 * @param   touch   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _page_move_check( BADGEVIEW_WORK *wk, int touch )
{
  int result=FALSE;

  // 左移動の時
  if(touch==FUNC_LEFT_PAGE){
    if( wk->page>0 ){   // 0ページでなければ－１
      wk->page--;
      result=TRUE;
    }

  // 右移動の時
  }else if(touch==FUNC_RIGHT_PAGE){
    if(wk->page < wk->page_max-1){  // MAXページよりも小さければ＋１
      wk->page++;
      result=TRUE;
    }
  }

  return result;
}


//----------------------------------------------------------------------------------
/**
 * @brief 初期ページ表示
 *
 * @param   wk    
 * @param   page    
 */
//----------------------------------------------------------------------------------
static void SetupPage( BADGEVIEW_WORK *wk, int page )
{
  wk->page = page;

  // タッチバー表示設定
  _page_clact_set( wk, page, wk->page_max );

  // プレート状態変更
  NamePlatePrint_1Page( wk );

}
//----------------------------------------------------------------------------------
/**
 * @brief   表示するトレーナー数を決める
 *
 * @param   page          現在のページ数
 * @param   max           最大ページ数
 * @param   trainer_num   トレーナー全員の数
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _get_print_num( int page, int max, int trainer_num )
{

  int num;
  num = trainer_num-page*BV_1PAGE_NUM;

  if(num > BV_1PAGE_NUM){
    num = BV_1PAGE_NUM;
  }

  OS_Printf("1ページ表示総数=%d\n", num);
  return num;
}


//----------------------------------------------------------------------------------
/**
 * @brief 1ページ分の名前プレートを描画する
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void NamePlatePrint_1Page( BADGEVIEW_WORK *wk )
{
  int i,num;
  STRBUF *strbuf = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, HEAPID_LEADERBOARD );

  // 表示する総数取得
  num = _get_print_num( wk->page, wk->page_max, wk->trainer_num );

  // プレート描画
  BgFramePrint( wk, i, strbuf, strbuf, strbuf );

  GFL_STR_DeleteBuffer( strbuf );
}

//----------------------------------------------------------------------------------
/**
 * @brief BGFRAMEに文字列を描画し、画面に反映
 *
 * @param   wk      
 * @param   id      指定プレート
 * @param   str     名前
 * @param   gender  性別
 */
//----------------------------------------------------------------------------------
static void BgFramePrint( BADGEVIEW_WORK *wk, int id, STRBUF *str, STRBUF *str2, STRBUF *str3 )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin), 0x0f0f );
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp( wk->InfoWin), 0,  0, str, wk->font, BV_COL_BLACK );
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp( wk->InfoWin), 0, 32, str2, wk->font, BV_COL_BLACK );
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp( wk->InfoWin), 0, 64, str3, wk->font, BV_COL_BLACK );
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->InfoWin );
  

}





//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// デバッグ用
//----------------------------------------------------------------------------------
#ifdef PM_DEBUG


  // デバッグ用データセット
static void _debug_data_set( BADGEVIEW_WORK *wk )
{
  int i, num=0;
  STRBUF *str;

#ifdef SPECIAL_FEBRUARY
  num = 23;
#endif

  if(GFL_UI_KEY_GetCont() & PAD_KEY_UP){
    num = 5;
  }else if(GFL_UI_KEY_GetCont() & PAD_BUTTON_X){
    num = 13;
  }else if(GFL_UI_KEY_GetCont() & PAD_KEY_LEFT){
    num = 20;
  }else if(GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
    num = 25;
  }else if(GFL_UI_KEY_GetCont() & PAD_BUTTON_Y){
    num = 30;
  }

  
  for(i=0;i<num;i++){
    str = GFL_MSG_CreateString( wk->debugname, 20+i);
    GFL_STR_DeleteBuffer( str );
  }
}

#endif