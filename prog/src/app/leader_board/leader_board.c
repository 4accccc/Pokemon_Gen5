//============================================================================================
/**
 * @file    leader_board.c
 * @brief   リーダーボード画面（Wifiバトルサブウェイダウンロードデータ）
 * @author  Akito Mori
 * @date    10.02.22
 *
 * @todo 2月ROM用の仮データを入れる処理が入っているので、Wifiサーバーが動くようになったら消す事
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"            // HEAPID_LEADERBOARD
#include "system/bgwinfrm.h"        // BGWINFRM_WORK
#include "system/cursor_move.h"     // CURSOR_MOVE
#include "system/gfl_use.h"         // GFUser_VIntr_CreateTCB
#include "system/wipe.h"            // WIPE_SYS_Start,
#include "sound/pm_sndsys.h"        // PMSND_PlaySE
#include "print/wordset.h"          // WORDSET
#include "print/printsys.h"         // PRINT_UTIL,PRINT_QUE...
#include "system/pms_data.h"        // PMS_DATA
#include "app/leader_board.h"       // proc
#include "app/app_menu_common.h"
#include "arc/arc_def.h"            // ARCID_LEADER_BOARD
#include "arc/leader_board.naix"    // leaderboard graphic
#include "battle/bsubway_battle_data.h" // BSUBWAY_LEADER_DATA

#include "font/font.naix"           // NARC_font_large_gftr
#include "arc/message.naix"         // NARC_message_leader_board_dat
#include "msg/msg_leader_board.h"   // 

#include "leader_board_def.h"       // 各種定義等

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
#define LB_BGFRAME_U_MSG    ( GFL_BG_FRAME0_S )
#define LB_BGFRAME_U_BG     ( GFL_BG_FRAME1_S )
#define LB_BGFRAME_D_MSG    ( GFL_BG_FRAME0_M )
#define LB_BGFRAME_D_BUTTON ( GFL_BG_FRAME1_M )
#define LB_BGFRAME_D_BG     ( GFL_BG_FRAME2_M )



//--------------------------------------------------------------------------------------------
// 構造体定義定義
//--------------------------------------------------------------------------------------------
typedef struct {
  ARCHANDLE     *g_handle;  // グラフィックデータファイルハンドル
  
  BGWINFRM_WORK * wfrm;   // BGWINFRM

  BSUBWAY_LEADER_DATA *bSubwayData;   // バトルサブウェイデータ

  GFL_FONT      *font;                // フォントデータ
  GFL_BMPWIN    *win[LB_BMPWIN_ID_MAX];
  PRINT_UTIL    printUtil[LB_BMPWIN_ID_MAX];
  PRINT_QUE     *printQue;
  PRINT_STREAM  *printStream;
  GFL_TCBLSYS   *pMsgTcblSys;

  GFL_MSGDATA   *mman;    // メッセージデータマネージャ
  WORDSET       *wset;    // 単語セット
  STRBUF        *expbuf;  // メッセージ展開領域
  STRBUF        *strbuf[LB_STR_MAX];

  GFL_CLUNIT    *clUnit;             // セルアクターユニット
  GFL_CLUNIT    *pmsUnit;            // 簡易会話描画用セルアクターユニット
  GFL_CLWK      *clwk[LB_OBJ_MAX];   // セルアクターワーク
  u32           clres[LB_RES_MAX];   // セルアクターリソース用インデックス

  CURSORMOVE_WORK *cmwk;            // カーソル移動ワーク

  GFL_TCB         *VblankTcb;       // 登録したVblankFuncのポインタ
  
  LEADERBOARD_PARAM *param;         // 呼び出しパラメータ
  int seq,next_seq;                 // サブシーケンス制御用ワーク
  u16 page,page_max;                // 現在の表示ページ

  int trainer_num;                  // バトルサブウェイデータに存在していた人数

#ifdef PM_DEBUG
  GFL_MSGDATA   *debugname;
#endif
} LEADERBOARD_WORK;

//--------------------------------------------------------------------------------------------
// 実データ定義
//--------------------------------------------------------------------------------------------
// プロセス定義データ
const GFL_PROC_DATA LeaderBoardProcData = {
  LeaderBoardProc_Init,
  LeaderBoardProc_Main,
  LeaderBoardProc_End,
};



//--------------------------------------------------------------------------------------------
// 関数プロトタイプ宣言
//--------------------------------------------------------------------------------------------
/*** 関数プロトタイプ ***/
static void VBlankFunc( GFL_TCB *tcb, void * work );
static void InitWork( LEADERBOARD_WORK *wk, void *pwk );
static void FreeWork( LEADERBOARD_WORK *wk );
static void VramBankSet(void);
static void BgInit(LEADERBOARD_WORK *wk);
static void BgExit(LEADERBOARD_WORK *wk) ;
static void BgGraphicInit(LEADERBOARD_WORK *wk);
static void BgGraphicExit( LEADERBOARD_WORK *wk );
static void ClActInit(LEADERBOARD_WORK *wk);
static void ClActSet(LEADERBOARD_WORK *wk) ;
static void ClActExit(LEADERBOARD_WORK *wk);
static void BmpWinInit(LEADERBOARD_WORK *wk);
static void BmpWinExit(LEADERBOARD_WORK *wk);
static void PrintSystemInit(LEADERBOARD_WORK *wk);
static void PrintSystemDelete(LEADERBOARD_WORK *wk);
static void PrintSystem_Main( LEADERBOARD_WORK *wk );
static void BgFramePrint( LEADERBOARD_WORK *wk, int id, STRBUF *str, int gender );
static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos );
static BOOL LBSEQ_Main( LEADERBOARD_WORK *wk );
static void CursorMoveSys_Init( LEADERBOARD_WORK *wk );
static void CursorMoveSys_Exit( LEADERBOARD_WORK *wk );
static void CursorPosSet( LEADERBOARD_WORK *wk, int pos );
static  int _countup_bsubway( BSUBWAY_LEADER_DATA *bData );
static void Rewrite_UpperInformation( LEADERBOARD_WORK *wk, BSUBWAY_LEADER_DATA *bData, int id );
static  int TouchBar_KeyControl( LEADERBOARD_WORK *wk );
static void ExecFunc( LEADERBOARD_WORK *wk, int touch );
static void _page_max_init( LEADERBOARD_WORK *wk );
static void _page_clact_set( LEADERBOARD_WORK *wk, int page, int max );
static int _page_move_check( LEADERBOARD_WORK *wk, int touch );
static void SetupPage( LEADERBOARD_WORK *wk, int page );
static void NamePlatePrint_1Page( LEADERBOARD_WORK *wk );


#ifdef PM_DEBUG
static void _debug_data_set( LEADERBOARD_WORK *wk, BSUBWAY_LEADER_DATA *bData );
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
GFL_PROC_RESULT LeaderBoardProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  LEADERBOARD_WORK * wk;

  OS_Printf( "↓↓↓↓↓　リーダーボード処理開始　↓↓↓↓↓\n" );

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_LEADERBOARD, 0x20000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(LEADERBOARD_WORK), HEAPID_LEADERBOARD );
  MI_CpuClearFast( wk, sizeof(LEADERBOARD_WORK) );

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
  CursorMoveSys_Init(wk); // カーソル制御システム初期化

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
GFL_PROC_RESULT LeaderBoardProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  LEADERBOARD_WORK * wk = mywk;

  if( LBSEQ_Main( wk ) == FALSE ){
    return GFL_PROC_RES_FINISH;
  }
  
  PrintSystem_Main( wk );         // 文字列描画メイン
  BGWINFRM_MoveMain( wk->wfrm );  // 仮想BGフレームメイン
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
GFL_PROC_RESULT LeaderBoardProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  LEADERBOARD_WORK * wk = mywk;
  
  CursorMoveSys_Exit(wk);
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
 * VBlank関数
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
static void InitWork( LEADERBOARD_WORK *wk, void *pwk )
{
  wk->param     = pwk;
  wk->next_seq  = SUBSEQ_FADEIN_WAIT;

  // グラフィックデータハンドルオープン
  wk->g_handle  = GFL_ARC_OpenDataHandle( ARCID_LEADER_BOARD, HEAPID_LEADERBOARD);

  // メッセージマネージャー確保  
  wk->mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                             NARC_message_leader_board_dat, HEAPID_LEADERBOARD );


  // leader_board.gmmの文字列を全て読み込む
  {
    int i;
    const STRCODE *buf;
    for(i=0;i<LB_STR_MAX;i++){
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
  
  // バトルサブウェイWifiトレーナーデータ取得
  {
    BSUBWAY_WIFI_DATA *wifiData = GAMEDATA_GetBSubwayWifiData( wk->param->gamedata );
    wk->bSubwayData = BSUBWAY_WIFIDATA_GetLeaderDataAlloc( wifiData, HEAPID_LEADERBOARD );
  }


#ifdef PM_DEBUG
  wk->debugname = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                             NARC_message_trname_dat, HEAPID_LEADERBOARD );

  // デバッグ用データセット
  _debug_data_set( wk, wk->bSubwayData );

  GFL_MSG_Delete( wk->debugname );

#endif
  
  // トレーナー数数え上げ
  wk->trainer_num = _countup_bsubway(wk->bSubwayData);

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
static void FreeWork( LEADERBOARD_WORK *wk )
{
  int i;
  
  // バトルサブウェイトレーナーデータ解放
  GFL_HEAP_FreeMemory( wk->bSubwayData );
  
  // 文字列解放
  for(i=0;i<LB_STR_MAX;i++){
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
  LB_BGFRAME_U_MSG,
  LB_BGFRAME_U_BG,
  LB_BGFRAME_D_MSG,
  LB_BGFRAME_D_BUTTON,
  LB_BGFRAME_D_BG,
};

//----------------------------------------------------------------------------------
/**
 * @brief BGシステム初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgInit(LEADERBOARD_WORK *wk)
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
        GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x128,
        GX_BG_EXTPLTT_01,      0, 0, 0, FALSE
      },
      { // メイン画面：ボタン
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
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
static void BgExit(LEADERBOARD_WORK *wk) 
{
  int i;
  for(i=0;i<NELEMS(bgframe_init_tbl);i++){
    GFL_BG_FreeBGControl( bgframe_init_tbl[i] );
  }
  GFL_BG_Exit();

}

// プレートの表示位置テーブル
static const name_plate_tbl[][2]={
  { LB_PLATE0_X,  LB_PLATE0_Y },   { LB_PLATE1_X,  LB_PLATE0_Y },
  { LB_PLATE0_X,  LB_PLATE2_Y },   { LB_PLATE1_X,  LB_PLATE2_Y },
  { LB_PLATE0_X,  LB_PLATE4_Y },   { LB_PLATE1_X,  LB_PLATE4_Y },
  { LB_PLATE0_X,  LB_PLATE6_Y },   { LB_PLATE1_X,  LB_PLATE6_Y },
  { LB_PLATE0_X,  LB_PLATE8_Y },   { LB_PLATE1_X,  LB_PLATE8_Y },
};
//----------------------------------------------------------------------------------
/**
 * @brief BGグラフィック転送
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgGraphicInit(LEADERBOARD_WORK *wk)
{
  ARCHANDLE * handle = wk->g_handle;

  // サブ画面背景転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_leader_board_leaderb_u_lz_NCGR,
                                        LB_BGFRAME_U_BG, 0, 0, TRUE, HEAPID_LEADERBOARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_leader_board_leaderb_u_lz_NSCR,
                                        LB_BGFRAME_U_BG, 0, 0, TRUE, HEAPID_LEADERBOARD );
  GFL_ARCHDL_UTIL_TransVramPalette(     handle, NARC_leader_board_leaderb_u_NCLR, 
                                        PALTYPE_SUB_BG, 0, 0, HEAPID_LEADERBOARD );

  // メイン画面背景転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_leader_board_leaderb_d_lz_NCGR,
                                        LB_BGFRAME_D_BG, 0, 0, TRUE, HEAPID_LEADERBOARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_leader_board_leaderb_d_lz_NSCR,
                                        LB_BGFRAME_D_BG, 0, 0, TRUE, HEAPID_LEADERBOARD );
  GFL_ARCHDL_UTIL_TransVramPalette(     handle, NARC_leader_board_leaderb_d_NCLR, 
                                        PALTYPE_MAIN_BG, 0, 0, HEAPID_LEADERBOARD );

  // メイン画面背景転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_leader_board_leaderb_d_lz_NCGR,
                                        LB_BGFRAME_D_BUTTON, 0, 0, TRUE, HEAPID_LEADERBOARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_leader_board_leaderb_d_lz_NSCR,
                                        LB_BGFRAME_D_BUTTON, 0, 0, TRUE, HEAPID_LEADERBOARD );
  
  // タッチバーBG転送
  {
    ARCHANDLE *c_handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_LEADERBOARD);

    GFL_ARCHDL_UTIL_TransVramBgCharacter( c_handle, APP_COMMON_GetBarCharArcIdx(),
                                          LB_BGFRAME_D_BUTTON, 0, 0, FALSE, HEAPID_LEADERBOARD );
    GFL_ARCHDL_UTIL_TransVramScreen(      c_handle, APP_COMMON_GetBarScrnArcIdx(),
                                          LB_BGFRAME_D_BUTTON, 0, 0, FALSE, HEAPID_LEADERBOARD );
    GFL_ARCHDL_UTIL_TransVramPaletteEx(   c_handle, APP_COMMON_GetBarPltArcIdx(), PALTYPE_MAIN_BG,
                                          0, TOUCHBAR_PAL*32, 32, HEAPID_LEADERBOARD );
    
    // スクリーンのパレット指定を変更
    GFL_BG_ChangeScreenPalette( LB_BGFRAME_D_BUTTON, TOUCHBAR_X, TOUCHBAR_Y, 
                                                     TOUCHBAR_W, TOUCHBAR_H, TOUCHBAR_PAL );
    GFL_BG_LoadScreenReq( LB_BGFRAME_D_BUTTON );

    GFL_ARC_CloseDataHandle( c_handle );
  }
  
  // 会話フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
                                 TALKFONT_PAL_OFFSET, 32, HEAPID_LEADERBOARD );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 
                                 TALKFONT_PAL_OFFSET, 32, HEAPID_LEADERBOARD );

  // 仮想BGフレーム読み込み
  wk->wfrm = BGWINFRM_Create( BGWINFRM_TRANS_VBLANK, 10, HEAPID_LEADERBOARD );
  {
    int i;
    for(i=0;i<10;i++){
      BGWINFRM_Add( wk->wfrm, LB_PLATE_ID+i, LB_BGFRAME_D_BUTTON, LB_PLATE_W, LB_PLATE_H );
      BGWINFRM_FrameSetArcHandle( wk->wfrm, LB_PLATE_ID+i, handle, 
                                  NARC_leader_board_leaderb_win_lz_NSCR, TRUE );
      BGWINFRM_FramePut( wk->wfrm, LB_PLATE_ID+i,
                         name_plate_tbl[i][0],name_plate_tbl[i][1] );
    }
  }

}


//----------------------------------------------------------------------------------
/**
 * @brief BG関連システム終了処理
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgGraphicExit( LEADERBOARD_WORK *wk )
{
  // 仮想BGフレーム解放
  BGWINFRM_Exit( wk->wfrm );
}

//----------------------------------------------------------------------------------
/**
 * @brief セルアクターシステム初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ClActInit(LEADERBOARD_WORK *wk)
{
  ARCHANDLE *c_handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_LEADERBOARD);

  // セルアクター初期化
  GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN, &VramTbl, HEAPID_LEADERBOARD );
  wk->clUnit  = GFL_CLACT_UNIT_Create( LB_CLACT_NUM, 1,  HEAPID_LEADERBOARD );
  wk->pmsUnit = GFL_CLACT_UNIT_Create( LB_PMSDRAW_OBJ_NUM, 1,  HEAPID_LEADERBOARD );

  // OBJ面表示ON
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

  // セルアクターリソース読み込み
  
  // ---リーダーボード用下画面---
  wk->clres[LB_RES_CHR]  = GFL_CLGRP_CGR_Register(      wk->g_handle, 
                                                        NARC_leader_board_leaderb_obj_lz_NCGR, 1, 
                                                        CLSYS_DRAW_MAX, HEAPID_LEADERBOARD );
  wk->clres[LB_RES_PLTT] = GFL_CLGRP_PLTT_Register(     wk->g_handle, 
                                                        NARC_leader_board_leaderb_obj_NCLR, 
                                                        CLSYS_DRAW_MAX, 0, HEAPID_LEADERBOARD );
  wk->clres[LB_RES_CELL] = GFL_CLGRP_CELLANIM_Register( wk->g_handle, 
                                                        NARC_leader_board_leaderb_obj_NCER, 
                                                        NARC_leader_board_leaderb_obj_NANR, 
                                                        HEAPID_LEADERBOARD );

  // 共通メニュー素材
  wk->clres[LB_RES_COMMON_CHR] = GFL_CLGRP_CGR_Register(       c_handle, 
                                                               APP_COMMON_GetBarIconCharArcIdx(), 0, 
                                                               CLSYS_DRAW_MAIN, 
                                                               HEAPID_LEADERBOARD );

  wk->clres[LB_RES_COMMON_PLTT] = GFL_CLGRP_PLTT_RegisterEx(   c_handle, 
                                                               APP_COMMON_GetBarIconPltArcIdx(), 
                                                               CLSYS_DRAW_MAIN, 
                                                               11*32, 0, 3, HEAPID_LEADERBOARD );

  wk->clres[LB_RES_COMMON_CELL] = GFL_CLGRP_CELLANIM_Register( c_handle, 
                                                               APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_32K), 
                                                               APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_32K), 
                                                               HEAPID_LEADERBOARD );
  GFL_ARC_CloseDataHandle( c_handle );

}


static const int clact_dat[][6]={
  //   X      Y  anm, chr,               pltt,               cell
  {  8*8,   2*8,   0, LB_RES_CHR,        LB_RES_PLTT,        LB_RES_CELL,      },// カーソル
  { 28*8,  21*8,   1, LB_RES_COMMON_CHR, LB_RES_COMMON_CHR,  LB_RES_COMMON_CHR },// もどる
  {    8,  21*8,   4, LB_RES_COMMON_CHR, LB_RES_COMMON_CHR,  LB_RES_COMMON_CHR },// 左矢印
  {   32,  21*8,   5, LB_RES_COMMON_CHR, LB_RES_COMMON_CHR,  LB_RES_COMMON_CHR },// 右矢印
};

//----------------------------------------------------------------------------------
/**
 * @brief セルアクター登録
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ClActSet(LEADERBOARD_WORK *wk) 
{

  GFL_CLWK_DATA add;
  int i;

  // セルアクター登録
  for(i=0;i<LB_OBJ_MAX;i++){
    add.pos_x  = clact_dat[i][0];
    add.pos_y  = clact_dat[i][1];
    add.anmseq = clact_dat[i][2];
    add.bgpri  = 0;
    add.softpri  = 0;

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
static void ClActExit(LEADERBOARD_WORK *wk)
{

  GFL_CLGRP_CGR_Release(      wk->clres[LB_RES_COMMON_CHR] );
  GFL_CLGRP_PLTT_Release(     wk->clres[LB_RES_COMMON_PLTT] );
  GFL_CLGRP_CELLANIM_Release( wk->clres[LB_RES_COMMON_CELL] );
  GFL_CLGRP_CGR_Release(      wk->clres[LB_RES_CHR] );
  GFL_CLGRP_PLTT_Release(     wk->clres[LB_RES_PLTT] );
  GFL_CLGRP_CELLANIM_Release( wk->clres[LB_RES_CELL] );


  // セルアクターユニット破棄
  GFL_CLACT_UNIT_Delete( wk->pmsUnit );
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

#include "lb_bmpwin.cdat"


//----------------------------------------------------------------------------------
/**
 * @brief BMPWIN初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BmpWinInit(LEADERBOARD_WORK *wk)
{
  int i;
  // BMPWINシステム開始
  GFL_BMPWIN_Init( HEAPID_LEADERBOARD );
 
  // 上画面のBMPWINを確保
  for(i=0;i<SUB_BMPWIN_NUM;i++){
    const BMPWIN_DAT *windat = Window0BmpDataTable[i];
    wk->win[LB_BMPWIN_S_REKIDAI_NO_TOREINAA+i] = GFL_BMPWIN_Create( 
                                                  windat->frame,
                                                  windat->x, windat->y,
                                                  windat->w, windat->h,
                                                  windat->pal, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[LB_BMPWIN_S_REKIDAI_NO_TOREINAA+i]), 0 );
    GFL_BMPWIN_MakeTransWindow( wk->win[LB_BMPWIN_S_REKIDAI_NO_TOREINAA+i] );
  }
  
  // 下画面のBMPWINを確保
  for(i=0;i<MAIN_BMPWIN_NUM;i++){
    wk->win[LB_BMPWIN_M_NAME0+i] = GFL_BMPWIN_Create( LB_BGFRAME_D_BUTTON,
                                                   2,   1,
                                                  10,   2,
                                                   1,   GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[LB_BMPWIN_M_NAME0+i]), NAME_PLATE_CLEAR_COL );
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief BMPWIN解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BmpWinExit(LEADERBOARD_WORK *wk)
{
  int i;
  
  // 下画面用BMPWINを解放
  for(i=0;i<MAIN_BMPWIN_NUM;i++){
    GFL_BMPWIN_Delete( wk->win[LB_BMPWIN_M_NAME0+i] );
  }
  // 上画面用BMPWINを解放
  for(i=0;i<SUB_BMPWIN_NUM;i++){
    GFL_BMPWIN_Delete( wk->win[LB_BMPWIN_S_REKIDAI_NO_TOREINAA+i] );
  }
  // BMPWINシステム終了
  GFL_BMPWIN_Exit();

}


#define LB_COL_WHITE      ( PRINTSYS_LSB_Make( 15, 14, 0) )    // 白
#define LB_COL_BLACK      ( PRINTSYS_LSB_Make(  1,  2, 0) )    // 黒
#define LB_COL_NAME_WHITE ( PRINTSYS_LSB_Make( 13, 12, 4) )    // 名前プレート用の白
#define LB_COL_NAME_M     ( PRINTSYS_LSB_Make( 13, 12, 4) )    // 男の名前
#define LB_COL_NAME_W     ( PRINTSYS_LSB_Make( 11, 10, 4) )    // 女の名前

//----------------------------------------------------------------------------------
/**
 * @brief 文字描画システム初期化
 *

 */
//----------------------------------------------------------------------------------
static void PrintSystemInit(LEADERBOARD_WORK *wk)
{
  int i;
  // フォント確保
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr ,
                              GFL_FONT_LOADTYPE_FILE , FALSE , HEAPID_LEADERBOARD );

  // 描画待機システム初期化
  wk->printQue = PRINTSYS_QUE_Create( HEAPID_LEADERBOARD );

  // BMPWINとPRINT_UTILを関連付け
  for(i=0;i<LB_BMPWIN_ID_MAX;i++){
    PRINT_UTIL_Setup( &wk->printUtil[i], wk->win[i] );
  }

  // 上画面ラベルを事前に描画しておく
  // 「れきだいの　トレーナー」
  PRINT_UTIL_PrintColor( &wk->printUtil[LB_BMPWIN_S_REKIDAI_NO_TOREINAA], wk->printQue, 
                         0, 0, wk->strbuf[LB_STR_REKIDAI], wk->font, LB_COL_WHITE );
  // 「すんでるばしょ」
  PRINT_UTIL_PrintColor( &wk->printUtil[LB_BMPWIN_S_SUNDERUBASHO], wk->printQue, 
                         0, 0, wk->strbuf[LB_STR_SUNDERU], wk->font, LB_COL_WHITE );
  // 「じこしょうかい」
  PRINT_UTIL_PrintColor( &wk->printUtil[LB_BMPWIN_S_JIKOSHOUKAI], wk->printQue, 
                         0, 0, wk->strbuf[LB_STR_JIKOSHOUKAI], wk->font, LB_COL_WHITE );



  
}

//----------------------------------------------------------------------------------
/**
 * @brief 文字描画システム解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void PrintSystemDelete(LEADERBOARD_WORK *wk)
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
static void PrintSystem_Main( LEADERBOARD_WORK *wk )
{
  int i;

  // 描画待ち
  PRINTSYS_QUE_Main( wk->printQue );

  // 各BMPが描画終了済であれば転送
  for(i=0;i<LB_BMPWIN_ID_MAX;i++){
    PRINT_UTIL_Trans( &wk->printUtil[i], wk->printQue );
  }

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

static BOOL SubSuq_FadeinWait( LEADERBOARD_WORK *wk );
static BOOL SubSuq_Main( LEADERBOARD_WORK *wk );
static BOOL SubSeq_IconWait( LEADERBOARD_WORK *wk );
static BOOL SubSuq_FadeOut( LEADERBOARD_WORK *wk );
static BOOL SubSuq_FadeOutWait( LEADERBOARD_WORK *wk );


static BOOL (*functable[])(LEADERBOARD_WORK *wk) = {
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
static BOOL LBSEQ_Main( LEADERBOARD_WORK *wk )
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
static BOOL SubSuq_FadeinWait( LEADERBOARD_WORK *wk )
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
static BOOL SubSuq_Main( LEADERBOARD_WORK *wk )
{
  u32 ret;
  // タッチバー処理が最優先
  if(TouchBar_KeyControl(wk)==GFL_UI_TP_HIT_NONE){
    // カーソル制御メイン
    ret=CURSORMOVE_Main( wk->cmwk );    
    if(ret==CURSORMOVE_NO_MOVE_LEFT){
      ExecFunc( wk, FUNC_LEFT_PAGE );
    }else if(ret==CURSORMOVE_NO_MOVE_RIGHT){
      ExecFunc( wk, FUNC_RIGHT_PAGE );
    }
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
static BOOL SubSeq_IconWait( LEADERBOARD_WORK *wk )
{
  if( GFL_CLACT_WK_CheckAnmActive( wk->clwk[LB_OBJ_END] )==FALSE){
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
static BOOL SubSuq_FadeOut( LEADERBOARD_WORK *wk )
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
static BOOL SubSuq_FadeOutWait( LEADERBOARD_WORK *wk )
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
static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos );

// メール選択
static const CURSORMOVE_DATA CursorSelTbl[] =
{
  {  64,  16, 0, 0, 0,  2, 0, 1, {   0,  31,   8, 119 },},    // 00: メール１
  { 192,  16, 0, 0, 1,  3, 0, 1, {   0,  31, 136, 244 },},    // 01: メール２
  {  64,  48, 0, 0, 0,  4, 2, 3, {  32,  63,   8, 119 },},    // 02: メール３
  { 192,  48, 0, 0, 1,  5, 2, 3, {  32,  63, 136, 244 },},    // 03: メール４
  {  64,  80, 0, 0, 2,  6, 4, 5, {  64,  95,   8, 119 },},    // 04: メール５
  { 192,  80, 0, 0, 3,  7, 4, 5, {  64,  95, 136, 244 },},    // 05: メール６
  {  64, 112, 0, 0, 4,  8, 6, 7, {  96, 127,   8, 119 },},    // 06: メール７
  { 192, 112, 0, 0, 5,  9, 6, 7, {  96, 127, 136, 244 },},    // 07: メール８
  {  64, 144, 0, 0, 6,  8, 8, 9, { 128, 159,   8, 119 },},    // 08: メール９
  { 192, 144, 0, 0, 7,  9, 8, 9, { 128, 159, 136, 244 },},    // 09: メール１０
  {   0,   0, 0, 0, 0,  0, 0, 0, { GFL_UI_TP_HIT_END, 0, 0, 0 },},

//  { 224, 176, 0, 0, CURSORMOVE_RETBIT|9, 10, 10, 10, { 160, 191, 192, 255 }, },   // 10: やめる
};
static const CURSORMOVE_CALLBACK SelCallBack = {
  CursorMoveCallBack_On,
  CursorMoveCallBack_Off,
  CursorMoveCallBack_Move,
  CursorMoveCallBack_Touch
};



//--------------------------------------------------------------------------------------------
/**
 * カーソル移動初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveSys_Init( LEADERBOARD_WORK *wk )
{
  int pos;

  pos = 0;

  wk->cmwk = CURSORMOVE_Create(
              CursorSelTbl,
              &SelCallBack,
              wk,
              TRUE,
              pos,
              HEAPID_LEADERBOARD );

  CursorPosSet( wk, pos );
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動削除
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveSys_Exit( LEADERBOARD_WORK * wk )
{
  CURSORMOVE_Exit( wk->cmwk );
}

//--------------------------------------------------------------------------------------------
/**
 * コールバック関数：カーソル表示
 *
 * @param work    メールボックス画面システムワーク
 * @param now_pos   現在の位置
 * @param old_pos   前回の位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos )
{
}

//--------------------------------------------------------------------------------------------
/**
 * コールバック関数：カーソル非表示
 *
 * @param work    メールボックス画面システムワーク
 * @param now_pos   現在の位置
 * @param old_pos   前回の位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos )
{
}

//--------------------------------------------------------------------------------------------
/**
 * コールバック関数：カーソル移動
 *
 * @param work    メールボックス画面システムワーク
 * @param now_pos   現在の位置
 * @param old_pos   前回の位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos )
{
  LEADERBOARD_WORK *wk = work;
  
  // 上画面書き換え
  Rewrite_UpperInformation( wk, wk->bSubwayData, wk->page*LB_1PAGE_NUM+now_pos );
  CursorPosSet( work, now_pos );
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//--------------------------------------------------------------------------------------------
/**
 * コールバック関数：タッチ
 *
 * @param work    メールボックス画面システムワーク
 * @param now_pos   現在の位置
 * @param old_pos   前回の位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos )
{
  LEADERBOARD_WORK *wk = work;

  // 上画面書き換え
  Rewrite_UpperInformation( wk, wk->bSubwayData, wk->page*LB_1PAGE_NUM+now_pos );
  CursorPosSet( work, now_pos );
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//--------------------------------------------------------------------------------------------
/**
 * メール選択のカーソル位置セット
 *
 * @param work    メールボックス画面システムワーク
 * @param pos     位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorPosSet( LEADERBOARD_WORK *wk, int pos )
{
  const CURSORMOVE_DATA *cd = CURSORMOVE_GetMoveData( wk->cmwk, pos );
  GFL_CLACTPOS clpos;
  clpos.x = cd->px;
  clpos.y = cd->py;

  GFL_CLACT_WK_SetPos( wk->clwk[LB_OBJ_CURSOR], &clpos, CLSYS_DEFREND_MAIN );

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
 * @brief バトルサブウェイデータの中に何人入っているか取得する
 *
 * @param   bData   BSUBWAY_LEADER_DATAの配列のポインタ
 *
 * @retval  int   トレーナーが存在している人数
 */
//----------------------------------------------------------------------------------
static int _countup_bsubway( BSUBWAY_LEADER_DATA *bData )
{
  int i,result=0;
  for(i=0;i<BSUBWAY_STOCK_WIFI_LEADER_MAX;i++){
    // 名前の１文字目が０じゃ無かったら存在している事にする
//    if(bData[i].name[0]!=GFL_STR_GetEOMCode()){
    if(bData[i].name[0]!=0){
      result++;
    }
  }

  OS_Printf("トレーナーデータは%d任\n", result);
  return result;
}


//----------------------------------------------------------------------------------
/**
 * @brief 上画面情報書き換え
 *
 * @param   wk    LEADERBOARD_WORK
 * @param   bData BSUBWAY_LEADER_DATA
 * @param   id    何番目か
 */
//----------------------------------------------------------------------------------
static void Rewrite_UpperInformation( LEADERBOARD_WORK *wk, BSUBWAY_LEADER_DATA *bData, int id )
{
  int nation     = bData[id].country_code;
  int local      = bData[id].address;
  PMS_DATA *pms  = (PMS_DATA*)bData[id].leader_word;
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
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[LB_BMPWIN_S_RANK]), 0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[LB_BMPWIN_S_TRAIN_NO]), 0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[LB_BMPWIN_S_NAME]), 0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[LB_BMPWIN_S_COUTRY]), 0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[LB_BMPWIN_S_PMS]), 0 );

  // 描画

  // トレーナー数を超えていないなら描画
  if( id<(wk->trainer_num)){
  
    // トレーナー名
    GFL_STR_SetStringCodeOrderLength( strbuf, bData[id].name, PERSON_NAME_SIZE );
    PRINT_UTIL_PrintColor( &wk->printUtil[LB_BMPWIN_S_NAME], wk->printQue, 
                           0, 0, strbuf, wk->font, LB_COL_WHITE );
    // ランクNO
    WORDSET_ExpandStr( wk->wset, wk->expbuf, wk->strbuf[LB_STR_RANk] );
    PRINT_UTIL_PrintColor( &wk->printUtil[LB_BMPWIN_S_RANK], wk->printQue, 
                           0, 0, wk->expbuf, wk->font, LB_COL_WHITE );
    // トレインNO
    WORDSET_ExpandStr( wk->wset, wk->expbuf, wk->strbuf[LB_STR_TRAINNO] );
    PRINT_UTIL_PrintColor( &wk->printUtil[LB_BMPWIN_S_TRAIN_NO], wk->printQue, 
                           0, 0, wk->expbuf, wk->font, LB_COL_WHITE );
    // 国
    WORDSET_ExpandStr( wk->wset, wk->expbuf, wk->strbuf[LB_STR_COUNTRY] );
    PRINT_UTIL_PrintColor( &wk->printUtil[LB_BMPWIN_S_COUTRY], wk->printQue, 
                           0, 0, wk->expbuf, wk->font, LB_COL_WHITE );
    OS_Printf("国文字列の長さは%d code=%d\n", GFL_STR_GetBufferLength(wk->expbuf), nation);
  
    // 住んでる場所
    WORDSET_ExpandStr( wk->wset, wk->expbuf, wk->strbuf[LB_STR_LOCAL] );
    PRINT_UTIL_PrintColor( &wk->printUtil[LB_BMPWIN_S_COUTRY], wk->printQue, 
                           0, 16, wk->expbuf, wk->font, LB_COL_WHITE );
    OS_Printf("住んでる場所文字列の長さは%d code=%d\n", GFL_STR_GetBufferLength(wk->expbuf),local);
    
    
    // 簡易会話を展開
    pmsstr = PMSDAT_ToString( pms, HEAPID_LEADERBOARD );
    PRINT_UTIL_PrintColor( &wk->printUtil[LB_BMPWIN_S_PMS], wk->printQue, 
                           0, 0, pmsstr, wk->font, LB_COL_WHITE );
    GFL_STR_DeleteBuffer( pmsstr );
  }else{
    // 範囲外だったのでクリアした状態を転送
    GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[LB_BMPWIN_S_NAME] );
    GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[LB_BMPWIN_S_COUTRY] );
    GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[LB_BMPWIN_S_PMS] );
    
  }
  
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
static int TouchBar_KeyControl( LEADERBOARD_WORK *wk )
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
static void ExecFunc( LEADERBOARD_WORK *wk, int touch )
{
  switch(touch){
  case FUNC_LEFT_PAGE:
    if(_page_move_check( wk, touch )){
      PMSND_PlaySE( SEQ_SE_SELECT1 );
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[LB_OBJ_LEFT],  12 );
      SetupPage( wk, wk->page );
    }
    break;
  case FUNC_RIGHT_PAGE:
    if(_page_move_check( wk, touch )){
      PMSND_PlaySE( SEQ_SE_SELECT1 );
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[LB_OBJ_RIGHT], 13 );
      SetupPage( wk, wk->page );
    }
    break;
  case FUNC_END:
    wk->seq = SUBSEQ_ICON_WAIT;
    GFL_CLACT_WK_SetAnmSeq( wk->clwk[LB_OBJ_END], 9 );
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
static void _page_max_init( LEADERBOARD_WORK *wk )
{
  wk->page = 0;
  wk->page_max = wk->trainer_num /LB_1PAGE_NUM;
  if(wk->trainer_num%LB_1PAGE_NUM){
    wk->page_max++;
  }
  OS_Printf("page=%d, trainer_num=%d, max=%d\n", wk->page, wk->trainer_num, wk->page_max);
}

//----------------------------------------------------------------------------------
/**
 * @brief   矢印アイコンの状態をページ数で変化させる
 *
 * @param   wk    
 * @param   page    
 * @param   max   
 */
//----------------------------------------------------------------------------------
static void _page_clact_set( LEADERBOARD_WORK *wk, int page, int max )
{
  // １ページしかなかったら矢印表示しない
  if(max==1){
    GFL_CLACT_WK_SetDrawEnable( wk->clwk[LB_OBJ_LEFT],  FALSE );
    GFL_CLACT_WK_SetDrawEnable( wk->clwk[LB_OBJ_RIGHT], FALSE );
  }else{
  // 複数ページ
    if(page==0){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[LB_OBJ_LEFT],  FALSE );
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[LB_OBJ_RIGHT], TRUE );
    }else if(page!=0 && page < (max-1)){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[LB_OBJ_LEFT],  TRUE );
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[LB_OBJ_RIGHT], TRUE );
    }else if(page==(max-1)){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[LB_OBJ_LEFT],  TRUE );
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[LB_OBJ_RIGHT], FALSE );
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
static int _page_move_check( LEADERBOARD_WORK *wk, int touch )
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
static void SetupPage( LEADERBOARD_WORK *wk, int page )
{
  wk->page = page;

  // タッチバー表示設定
  _page_clact_set( wk, page, wk->page_max );

  // プレート状態変更
  NamePlatePrint_1Page( wk );

  // 上画面表示設定
  Rewrite_UpperInformation( wk, wk->bSubwayData, 
                            wk->page*LB_1PAGE_NUM+CURSORMOVE_PosGet( wk->cmwk ) );
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
  num = trainer_num-page*LB_1PAGE_NUM;

  if(num > LB_1PAGE_NUM){
    num = LB_1PAGE_NUM;
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
static void NamePlatePrint_1Page( LEADERBOARD_WORK *wk )
{
  int i,num;
  STRBUF *strbuf = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, HEAPID_LEADERBOARD );

  // 表示する総数取得
  num = _get_print_num( wk->page, wk->page_max, wk->trainer_num );

  // プレート描画
  for(i=0;i<num;i++){
    GFL_STR_SetStringCodeOrderLength( strbuf, wk->bSubwayData[i+wk->page*LB_1PAGE_NUM].name,
                                      PERSON_NAME_SIZE );
    BgFramePrint( wk, i, strbuf, wk->bSubwayData[i+wk->page*LB_1PAGE_NUM].gender );
  }
  // 表示しなかった残りはプレートをOFFにする
  for(;i<LB_1PAGE_NUM;i++){
    BGWINFRM_FrameOff( wk->wfrm, i );
  }
  
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
static void BgFramePrint( LEADERBOARD_WORK *wk, int id, STRBUF *str, int gender )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[LB_BMPWIN_M_NAME0+id]), 
                 NAME_PLATE_CLEAR_COL );
  if(gender==0)  {
    PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp( wk->win[LB_BMPWIN_M_NAME0+id]), 
                                          0, 0, str, wk->font, LB_COL_NAME_M );
  }else{
    PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp( wk->win[LB_BMPWIN_M_NAME0+id]), 
                                          0, 0, str, wk->font, LB_COL_NAME_W );
  }
  GFL_BMPWIN_TransVramCharacter( wk->win[LB_BMPWIN_M_NAME0+id] );
  BGWINFRM_BmpWinOn( wk->wfrm, LB_PLATE_ID+id, wk->win[LB_BMPWIN_M_NAME0+id] );
  BGWINFRM_FrameOn( wk->wfrm,  LB_PLATE_ID+id );
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

static PMS_DATA debug_pms[3]={
  {1,1,{1,1} },{2,2,{2,2} },{3,2,{3,3} },
};

  // デバッグ用データセット
static void _debug_data_set( LEADERBOARD_WORK *wk, BSUBWAY_LEADER_DATA *bData )
{
  int i, num=0;
  STRBUF *str;
  PMS_DATA *pms;

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
    GFL_STR_GetStringCode( str, bData[i].name, 6 );
    GFL_STR_DeleteBuffer( str );
    pms = (PMS_DATA*)bData[i].leader_word;
    *pms = debug_pms[i%3];
    bData[i].country_code = GFUser_GetPublicRand(20)+1;
    bData[i].address      = 0;
    bData[i].gender       = GFUser_GetPublicRand(2);
  }
}

#endif