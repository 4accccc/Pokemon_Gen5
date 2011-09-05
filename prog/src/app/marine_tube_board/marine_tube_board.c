//=============================================================================
/**
 *
 *	@file		marine_tube_board.c
 *	@brief  マリンチューブ掲示板
 *	@author	ichiraku_katsuhiko
 *	@data		2011.09.05
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"

//アプリ共通素材
#include "app/app_menu_common.h"
#include "app/app_nogear_subscreen.h"
#include "app/app_printsys_common.h"
#include "app/app_keycursor.h"

//アーカイブ
#include "arc_def.h"

//描画設定
#include "marine_tube_board_graphic.h"

// mine
#include "app/marine_tube_board.h"
#include "msg/msg_marine_tube.h"    // GMM
#include "message.naix"
#include "marine_tube_gra.naix" // arc


//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  MARINE_TUBE_BOARD_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};

//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_BACK_M	= GFL_BG_FRAME1_M,
  BG_FRAME_NOGEAR_S = GFL_BG_FRAME0_S, 
  BG_FRAME_TEXT_S = GFL_BG_FRAME1_S, 
};

//-------------------------------------
///	パレット
//=====================================
enum
{	
	//メインBG
	PLTID_BG_BACK_M	    = 0,
	//サブBG
	PLTID_BG_BACK_S     =	0,  // NOGEAR
  PLTID_BG_WINFRAME_S = 14, // WINDOWフレーム
  PLTID_BG_FONT_S     = 15, // フォント色

	//メインOBJ
	PLTID_OBJ_M_	      = 0, // 
	//サブOBJ
  PLTID_OBJ_S_        = 0, // 
};


// フェード
#define FADE_IN_WAIT  (0)
#define FADE_OUT_WAIT (0)

//-------------------------------------
///	MSGWND
//=====================================
// フレームを読み込んだキャラ位置
#define MSGWND_FRAME_CHR  (1)
// BMPWIN
#define MSGWND_BMPWIN_X (1)
#define MSGWND_BMPWIN_Y (19)
#define MSGWND_BMPWIN_W (30)
#define MSGWND_BMPWIN_H (4)

#define STRBUF_SIZE     (512) // メッセージバッファサイズ


//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//-------------------------------------
///	メッセージウィンドウ
//=====================================
typedef struct {
  GFL_MSGDATA       *p_msg;     // メッセージデータ
  GFL_FONT          *p_font;    // フォント
  PRINT_UTIL        util;       // 文字描画
  PRINT_QUE         *p_que;     // 文字キュー
  GFL_BMPWIN*       p_bmpwin;   // BMPWIN
  WORDSET           *p_word;    // 単語
  STRBUF*           p_strbuf;   // 文字バッファ
  u16               clear_chr;  // クリアキャラ
  u16               dummy;
} MSGWND_WORK;

//-------------------------------------
///	プリントストリーム
//=====================================
typedef struct {
  PRINT_STREAM      *p_stream;
  GFL_TCBLSYS       *p_tcblsys;
  int               seq;
  u16               wait_max;
  u16               wait_count;
  APP_PRINTSYS_COMMON_WORK  print_wk;
  APP_KEYCURSOR_WORK        *p_cursor;
} PS_WORK;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

	//描画設定
	MARINE_TUBE_BOARD_GRAPHIC_WORK	*graphic;

  // 下画面ウィンドウ
  MSGWND_WORK   msgwnd;

  // 下画面ウィンドウ プリントストリーム
  PS_WORK       ps;

  //=============================
  // 共通モジュール
	GFL_FONT									*p_font;      // フォント
	GFL_MSGDATA								*p_msg;       // メッセージ
	PRINT_QUE									*p_que;       // キュー
  WORDSET                   *p_word;      // 単語

} MARINE_TUBE_BOARD_MAIN_WORK;


//=============================================================================
/**
 *							データ定義
 */
//=============================================================================


//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT MarineTubeBoardProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT MarineTubeBoardProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT MarineTubeBoardProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void MarineTubeBoard_BG_LoadResource( const u16 type, HEAPID heapID );

//-------------------------------------
///	MSGWND
//=====================================
static void MSGWND_Init( MSGWND_WORK *p_wk, GFL_FONT *p_font, GFL_MSGDATA *p_msg, PRINT_QUE *p_que, WORDSET *p_word, HEAPID heapID );
static void MGSWND_Exit( MSGWND_WORK *p_wk );
static void MSGWND_Print( MSGWND_WORK *p_wk, u32 msgID );
static BOOL MSGWND_PrintMain( MSGWND_WORK* p_wk );

//-------------------------------------
///	PS(PrintStream)
//=====================================
static void PS_Init( PS_WORK *p_wk, HEAPID heapID );
static void PS_Exit( PS_WORK *p_wk );
static void PS_Print( PS_WORK *p_wk, MSGWND_WORK *p_msgwnd, u32 msgID, HEAPID heapID );
static BOOL PS_PrintMain( PS_WORK *p_wk, MSGWND_WORK *p_msgwnd );

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA MarineTubeBoardProcData = 
{
	MarineTubeBoardProc_Init,
	MarineTubeBoardProc_Main,
	MarineTubeBoardProc_Exit,
};
//=============================================================================
/**
 *								PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					MARINE_TUBE_BOARD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MarineTubeBoardProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	MARINE_TUBE_BOARD_MAIN_WORK *wk;
	MARINE_TUBE_BOARD_PARAM *param = (MARINE_TUBE_BOARD_PARAM*)pwk;
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( param->gamesys );

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MARINE_TUBE_BOARD, MARINE_TUBE_BOARD_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(MARINE_TUBE_BOARD_MAIN_WORK), HEAPID_MARINE_TUBE_BOARD );
  GFL_STD_MemClear( wk, sizeof(MARINE_TUBE_BOARD_MAIN_WORK) );

  // 初期化
  wk->heapID = HEAPID_MARINE_TUBE_BOARD;
	
  // 共通モジュールの作成
	wk->p_font  = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												         GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
  wk->p_que		= PRINTSYS_QUE_Create( wk->heapID );
	wk->p_msg   = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			                          NARC_message_marine_tube_dat, wk->heapID );
  wk->p_word  = WORDSET_Create( wk->heapID );

	//描画設定初期化
	wk->graphic	= MARINE_TUBE_BOARD_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, wk->heapID );

  // サブBG(NOGEAR)
  {
    const MYSTATUS*  mystatus  = GAMEDATA_GetMyStatus( p_gdata );  // 
    //APP_NOGEAR_SUBSCREEN_Init();  // 描画設定でNOGEARの設定もしている
    APP_NOGEAR_SUBSCREEN_Trans( wk->heapID, mystatus->sex );  // PM_MALE or PM_FEMALE  // include/pm_version.h
  }

	//BGリソース読み込み
	MarineTubeBoard_BG_LoadResource( param->type, wk->heapID );

  // モジュール作成
  MSGWND_Init( &wk->msgwnd, wk->p_font, wk->p_msg, wk->p_que, wk->p_word, wk->heapID );
  PS_Init( &wk->ps, wk->heapID );

  // フェードイン(ただちに真っ黒)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 16, 0 );

  // 通信アイコン
  GFL_NET_ReloadIconTopOrBottom( FALSE, wk->heapID );

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					MARINE_TUBE_BOARD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MarineTubeBoardProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	MARINE_TUBE_BOARD_MAIN_WORK* wk = mywk;

  // サブBG
  //APP_NOGEAR_SUBSCREEN_Exit();
  
  // モジュールの破棄
  PS_Exit( &wk->ps );
  MGSWND_Exit( &wk->msgwnd );

  // 共通モジュールの破棄
  WORDSET_Delete( wk->p_word );
	GFL_MSG_Delete( wk->p_msg );
  PRINTSYS_QUE_Clear( wk->p_que );
	PRINTSYS_QUE_Delete( wk->p_que );
	GFL_FONT_Delete( wk->p_font );

	//描画設定破棄
  GFL_BG_FillCharacterRelease( BG_FRAME_TEXT_S, 1, 0 );
	MARINE_TUBE_BOARD_GRAPHIC_Exit( wk->graphic );

	//PROC用メモリ解放
  {
    HEAPID heapID = wk->heapID;
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( heapID );
  }

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					MARINE_TUBE_BOARD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MarineTubeBoardProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	MARINE_TUBE_BOARD_MAIN_WORK* wk = mywk;
	MARINE_TUBE_BOARD_PARAM *param = (MARINE_TUBE_BOARD_PARAM*)pwk;
  BOOL b_ui_end = FALSE;

  enum{
    SEQ_START,
    SEQ_FADE_IN,
    SEQ_PRINT,
    SEQ_MAIN,
    SEQ_FADE_OUT,
    SEQ_END,
  };

  // キー・タッチの判定
  {
    u32 x, y;
    if( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) )
    {
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      b_ui_end = TRUE;
    }
    else if( GFL_UI_TP_GetPointTrg( &x, &y ) )
    {
      GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
      b_ui_end = TRUE;
    }
  }

  // シーケンス
  switch( *seq ){
  case SEQ_START:
    {
      // フェードイン(黒→見える)
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );
      *seq = SEQ_FADE_IN;
    }
    break;
  case SEQ_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 掲示板メッセージ
        PS_Print( &wk->ps, &wk->msgwnd, msg_marine_tube_01 + param->type, wk->heapID );

        *seq = SEQ_PRINT;
      }
    }
    break;
  case SEQ_PRINT:
    {
      if( PS_PrintMain( &wk->ps, &wk->msgwnd ) == TRUE )
      {
        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      if( b_ui_end )
      {
        *seq = SEQ_FADE_OUT;
        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FADE_OUT_WAIT );
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        *seq = SEQ_END;
      }
    }
    break;
  case SEQ_END:
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  default:
    GF_ASSERT(0);
  }

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->p_que );

	//2D描画
	MARINE_TUBE_BOARD_GRAPHIC_2D_Draw( wk->graphic );

	//3D描画
	//MARINE_TUBE_BOARD_GRAPHIC_3D_StartDraw( wk->graphic );
	//MARINE_TUBE_BOARD_GRAPHIC_3D_EndDraw( wk->graphic );

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *								static関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース読み込み
 *
 *	@param	type    どのBGを読み込むか
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void MarineTubeBoard_BG_LoadResource( const u16 type, HEAPID heapID )
{
  //	----- MAIN -----
  {
    ARCHANDLE	*handle;
    ARCDATID  pal_m_id;
    ARCDATID  chr_m_id;
    ARCDATID  scr_m_id;

    handle	= GFL_ARC_OpenDataHandle( ARCID_MARINE_TUBE_GRA, heapID );

    if( type == 0 )
    {
      pal_m_id = NARC_marine_tube_gra_marine_tube_bg1_NCLR;
      chr_m_id = NARC_marine_tube_gra_marine_tube_bg1_NCGR;
      scr_m_id = NARC_marine_tube_gra_marine_tube_bg1_NSCR;
    }
    else
    {
      pal_m_id = NARC_marine_tube_gra_marine_tube_bg2_NCLR;
      chr_m_id = NARC_marine_tube_gra_marine_tube_bg2_NCGR;
      scr_m_id = NARC_marine_tube_gra_marine_tube_bg2_NSCR;
    }

    GFL_ARCHDL_UTIL_TransVramPalette( handle, pal_m_id, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 0, heapID );

    GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, chr_m_id, BG_FRAME_BACK_M, 0, 0, 0, heapID );
    GFL_ARCHDL_UTIL_TransVramScreen(      handle, scr_m_id, BG_FRAME_BACK_M, 0, 0, 0, heapID );		

    GFL_ARC_CloseDataHandle( handle );

    GFL_BG_LoadScreenReq( BG_FRAME_BACK_M );
  }

  // ---- FONT ----
  { 
    ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, heapID );

    //上画面フォントパレット
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_SUB_BG, PLTID_BG_FONT_S*0x20, 0x20, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }
  {
    //上画面テキスト枠
    GFL_BG_FillCharacter( BG_FRAME_TEXT_S, 0, 1, 0 );
    BmpWinFrame_GraphicSet(
        BG_FRAME_TEXT_S, MSGWND_FRAME_CHR, PLTID_BG_WINFRAME_S, MENU_TYPE_SYSTEM, heapID );
  }
}


//=============================================================================
/**
 *    MSGWND
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージウィンドウ  初期化
 *
 *	@param	MSGWND_WORK* p_wk ワーク
 *	@param	*p_font           フォント
 *	@param	*p_msg            メッセージデータ
 *	@param	*p_que            キュー
 *	@param	*p_word           単語
 *	@param	heapID            ヒープID
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK *p_wk, GFL_FONT *p_font, GFL_MSGDATA *p_msg, PRINT_QUE *p_que, WORDSET *p_word, HEAPID heapID )
{
  // 初期化
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );

  p_wk->clear_chr = 0xF;
  p_wk->p_font    = p_font;
  p_wk->p_msg     = p_msg;
  p_wk->p_que     = p_que;
  p_wk->p_word    = p_word;

  // バッファ作成
  p_wk->p_strbuf    = GFL_STR_CreateBuffer( STRBUF_SIZE, heapID );

  // BMPWIN作成
  p_wk->p_bmpwin  = GFL_BMPWIN_Create( BG_FRAME_TEXT_S, MSGWND_BMPWIN_X, MSGWND_BMPWIN_Y, MSGWND_BMPWIN_W, MSGWND_BMPWIN_H, PLTID_BG_FONT_S, GFL_BMP_CHRAREA_GET_B );

  // BMPWINFRAME
  BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_OFF, MSGWND_FRAME_CHR, PLTID_BG_WINFRAME_S );

  // プリントキュー設定
  PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );

  // 転送
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_bmpwin ), p_wk->clear_chr );
  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージウィンドウ  破棄
 *
 *	@param	MSGWND_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MGSWND_Exit( MSGWND_WORK *p_wk )
{
  // BMPWIN破棄
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );

  // バッファ破棄
  GFL_STR_DeleteBuffer( p_wk->p_strbuf );

  // クリア
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージウィンドウ  プリント開始
 *
 *	@param	MSGWND_WORK *p_wk ワーク
 *	@param	msgID             メッセージID
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void MSGWND_Print( MSGWND_WORK *p_wk, u32 msgID )
{
  // 描画残り待ち
  while( PRINTSYS_QUE_IsExistTarget( p_wk->p_que, GFL_BMPWIN_GetBmp(p_wk->p_bmpwin) ) )
  {
    PRINTSYS_QUE_Main( p_wk->p_que );
  }

  // クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_bmpwin ), p_wk->clear_chr );

  // 文字列作成
  GFL_MSG_GetString( p_wk->p_msg, msgID, p_wk->p_strbuf );

  // 文字列描画
  PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_wk->p_font );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージウィンドウ  QUEの描画待ち
 *
 *  @param  const MSGWND_WORK* cp_wk  ワーク
 *
 *  @return TRUEでストリーム終了  FALSEは処理中
 */
//-----------------------------------------------------------------------------
static BOOL MSGWND_PrintMain( MSGWND_WORK* p_wk )
{ 
  return PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );
}

//=============================================================================
/**
 *    PS(PrintStream)
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  PS  初期化
 *  @param  PS_WORK* p_wk ワーク
 *  @param  heapID        ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void PS_Init( PS_WORK *p_wk, HEAPID heapID )
{
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(PS_WORK) );

  // GFL_TCBLシステム
  p_wk->p_tcblsys = GFL_TCBL_Init( heapID, heapID, 2, 0 );

  p_wk->p_cursor = APP_KEYCURSOR_Create( 0xf, TRUE, TRUE, heapID );

}

//----------------------------------------------------------------------------
/**
 *  @brief  PS  破棄
 *
 *  @param  PS_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PS_Exit( PS_WORK *p_wk )
{
  // プリントストリーム
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
  }

  APP_KEYCURSOR_Delete( p_wk->p_cursor );

  // GFL_TCBLシステム
  GFL_TCBL_Exit( p_wk->p_tcblsys );

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(PS_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  PS  メッセージをプリントストリーム表示
 *
 *	@param	PS_WORK *p_wk     ワーク
 *	@param	*p_msgwnd         メッセージウィンドウワーク
 *	@param	next_seq_function 次シーケンス関数
 *	@param	msgID             メッセージID
 *	@param	heapID            ヒープID
 */
//-----------------------------------------------------------------------------
static void PS_Print( PS_WORK *p_wk, MSGWND_WORK *p_msgwnd, u32 msgID, HEAPID heapID )
{
  // 文字列作成
  GFL_MSG_GetString( p_msgwnd->p_msg, msgID, p_msgwnd->p_strbuf );

  // 設定をクリア
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
  }
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_msgwnd->p_bmpwin ), p_msgwnd->clear_chr );
	GFL_BMPWIN_TransVramCharacter( p_msgwnd->p_bmpwin );

  // 文字描画関連の設定を行う
  p_wk->p_stream = PRINTSYS_PrintStream( p_msgwnd->p_bmpwin, 0, 0, p_msgwnd->p_strbuf,
      p_msgwnd->p_font, MSGSPEED_GetWait(),
      p_wk->p_tcblsys, 2, heapID, p_msgwnd->clear_chr );
	GFL_BMPWIN_MakeScreen( p_msgwnd->p_bmpwin );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(p_msgwnd->p_bmpwin) );

  //PRINTSYS_PrintStream_StartTempSpeedMode( p_wk->p_stream, 0 );

  APP_PRINTSYS_COMMON_PrintStreamInit( &p_wk->print_wk, APP_PRINTSYS_COMMON_TYPE_BOTH );
  // その他設定
  p_wk->wait_max  = 0;
  p_wk->wait_count= 0;
  p_wk->seq = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  PS  メッセージを表示
 *
 *	@param	PS_WORK *p_wk     ワーク
 *
 *	@return 表示したらTRUE
 */
//-----------------------------------------------------------------------------
static BOOL PS_PrintMain( PS_WORK *p_wk, MSGWND_WORK *p_msgwnd )
{
  GFL_TCBL_Main( p_wk->p_tcblsys );

  if( p_wk->p_stream != NULL && p_msgwnd->p_bmpwin != NULL  )
  {
    APP_KEYCURSOR_Main( p_wk->p_cursor , p_wk->p_stream , p_msgwnd->p_bmpwin );
    if( APP_PRINTSYS_COMMON_PrintStreamFunc( &p_wk->print_wk, p_wk->p_stream ) == TRUE )
    {
      PRINTSYS_PrintStreamDelete( p_wk->p_stream );
      p_wk->p_stream = NULL;
      return TRUE;
    }
  }

  return FALSE;
}


