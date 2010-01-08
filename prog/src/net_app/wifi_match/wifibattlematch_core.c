//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_core.c
 *	@brief	WIFIのバトルマッチコア画面
 *	@author	Toru=Nagihashi
 *	@data		2009.11.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	ライブラリ
#include <gflib.h>

//	システム
#include "system/main.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "system/gf_date.h"
#include "system/bmp_winframe.h"
#include "pokeicon/pokeicon.h"
#include "gamesystem/game_data.h"
#include "system/net_err.h"

//	アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "wifimatch_gra.naix"
#include "msg/msg_wifi_match.h"

//	文字表示
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"

//WIFIバトルマッチのモジュール
#include "wifibattlematch_graphic.h"
#include "wifibattlematch_view.h"
#include "wifibattlematch_net.h"
#include "wifibattlematch_util.h"

//外部公開
#include "wifibattlematch_core.h"

//-------------------------------------
///	オーバーレイ
//=====================================
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(dpw_common);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	BGフレーム
//=====================================
enum
{
	//メイン画面
	BG_FRAME_M_TEXT	=	GFL_BG_FRAME0_M, 
	BG_FRAME_M_BACK	=	GFL_BG_FRAME1_M, 

	//サブ画面
	BG_FRAME_S_BACK	=	GFL_BG_FRAME1_M, 
} ;

//-------------------------------------
///	パレット
//=====================================
enum
{
	//メイン画面BG
	PLT_BG_M	  =	0,
  PLT_LIST_M  = 13,
  PLT_TEXT_M  = 14,
	PLT_FONT_M	= 15,

	//サブ画面BG
	PLT_BG_S	  = 0,
	PLT_FONT_S	= MATCHINFO_PLT_BG_FONT,

	//メイン画面OBJ
	
	//サブ画面OBJ
} ;

//-------------------------------------
///	キャラ
//=====================================
enum
{ 
  //メイン画面  BG_FRAME_M_TEXT
  CGR_OFS_M_CLEAR = 0,
  CGR_OFS_M_LIST  = 1,
  CGR_OFS_M_TEXT  = 10,
};

//-------------------------------------
///	シンク
//=====================================
#define ENEMYDATA_WAIT_SYNC (180)

//=============================================================================
/**
 *        定義
*/
//=============================================================================
//-------------------------------------
///	メインワーク前方宣言
//=====================================
typedef struct _WIFIBATTLEMATCH_WORK WIFIBATTLEMATCH_WORK;

//-------------------------------------
///	  メインシーケンス定義
//=====================================
typedef BOOL (*WIFIBATTLEMATCH_MAINSEQ_FUNCTION)( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_seq );

//-------------------------------------
///	  サブシーケンス定義
//=====================================
typedef int (*WIFIBATTLEMATCH_SUBSEQ_FUNCTION)( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_seq );

//-------------------------------------
///	メインワーク
//=====================================
struct _WIFIBATTLEMATCH_WORK
{
	//グラフィック設定
	WIFIBATTLEMATCH_GRAPHIC_WORK	*p_graphic;

	//共通で使うフォント
	GFL_FONT			            *p_font;

	//共通で使うキュー
	PRINT_QUE				          *p_que;

	//共通で使うメッセージ
	GFL_MSGDATA	          		*p_msg;

	//共通で使う単語
	WORDSET				          	*p_word;

	//上画面情報
	PLAYERINFO_WORK         	*p_playerinfo;

	//対戦者情報
	MATCHINFO_WORK          	*p_matchinfo;

  //テキスト面
  WBM_TEXT_WORK             *p_text;

  //リスト
  WBM_LIST_WORK             *p_list;

  //ネット
  WIFIBATTLEMATCH_NET_WORK  *p_net;

  //メインシーケンス
  WIFIBATTLEMATCH_MAINSEQ_FUNCTION  main_seq;

  //サブシーケンス
  int sub_seq;
  int next_seq;
  int fade_seq;

  //サーバーからの受信バッファ
  WIFIBATTLEMATCH_GDB_RND_SCORE_DATA rnd_score;

  //ウエイト
  u32 cnt;
} ;

//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	メインシーケンス
//    （モードにより違う）
//=====================================
static BOOL WifiBattleMatch_Random_MainSeq( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_seq );
static BOOL WifiBattleMatch_WiFi_MainSeq( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_seq );
static BOOL WifiBattleMatch_Live_MainSeq( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_seq );
static BOOL WifiBattleMatch_Debug_MainSeq( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_seq );

//-------------------------------------
///	不正情報のチェック
//=====================================
static void WifiBattleMatch_Darty_ModifiEnemyData( WIFIBATTLEMATCH_ENEMYDATA *p_data );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	WifiBattleMaptchCore_ProcData =
{	
	WIFIBATTLEMATCH_CORE_PROC_Init,
	WIFIBATTLEMATCH_CORE_PROC_Main,
	WIFIBATTLEMATCH_CORE_PROC_Exit,
};

//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	コアプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	WIFIBATTLEMATCH_WORK	*p_wk;
  WIFIBATTLEMATCH_CORE_PARAM *p_param  = p_param_adrs;


	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH_CORE, 0x30000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_WORK), HEAPID_WIFIBATTLEMATCH_CORE );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_WORK) );	


	//グラフィック設定
	p_wk->p_graphic	= WIFIBATTLEMATCH_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_WIFIBATTLEMATCH_CORE );

	//共通モジュールの作成
	p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WIFIBATTLEMATCH_CORE );
	p_wk->p_que			= PRINTSYS_QUE_Create( HEAPID_WIFIBATTLEMATCH_CORE );
	p_wk->p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
												NARC_message_wifi_match_dat, HEAPID_WIFIBATTLEMATCH_CORE );
	p_wk->p_word	= WORDSET_CreateEx( WORDSET_DEFAULT_SETNUM, WORDSET_COUNTRY_BUFLEN, HEAPID_WIFIBATTLEMATCH_CORE );

	//読みこみ
	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_WIFIMATCH_GRA, HEAPID_WIFIBATTLEMATCH_CORE );

		//PLT
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifimatch_gra_bg_NCLR,
				PALTYPE_MAIN_BG, 0, 0, HEAPID_WIFIBATTLEMATCH_CORE );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifimatch_gra_bg_NCLR,
				PALTYPE_SUB_BG, 0, 0, HEAPID_WIFIBATTLEMATCH_CORE );

		//CHR
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifimatch_gra_bg_back_NCGR, 
				GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_WIFIBATTLEMATCH_CORE );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifimatch_gra_bg_back_NCGR, 
				GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_WIFIBATTLEMATCH_CORE );

		//SCR
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_wifimatch_gra_back_NSCR,
				GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_WIFIBATTLEMATCH_CORE );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_wifimatch_gra_back_NSCR,
				GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_WIFIBATTLEMATCH_CORE );
	
		GFL_ARC_CloseDataHandle( p_handle );

		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
				PALTYPE_MAIN_BG, PLT_FONT_M*0x20, 0, HEAPID_WIFIBATTLEMATCH_CORE );
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
				PALTYPE_SUB_BG, PLT_FONT_S*0x20, 0, HEAPID_WIFIBATTLEMATCH_CORE );

    
    GFL_BG_FillCharacter( BG_FRAME_M_TEXT, 0, 1,  CGR_OFS_M_CLEAR );
    BmpWinFrame_GraphicSet( BG_FRAME_M_TEXT, CGR_OFS_M_LIST, PLT_LIST_M, MENU_TYPE_SYSTEM, HEAPID_WIFIBATTLEMATCH_CORE );
    TalkWinFrame_GraphicSet( BG_FRAME_M_TEXT, CGR_OFS_M_TEXT, PLT_TEXT_M, MENU_TYPE_SYSTEM, HEAPID_WIFIBATTLEMATCH_CORE );
	}

	//モジュールの作成
	{	
    DWCUserData *p_user_data;
    WIFI_LIST   *p_wifilist;
    p_wifilist  = GAMEDATA_GetWiFiList( p_param->p_param->p_game_data );
    p_user_data = WifiList_GetMyUserInfo( p_wifilist );

    p_wk->p_net   = WIFIBATTLEMATCH_NET_Init( p_user_data, p_wifilist, HEAPID_WIFIBATTLEMATCH_CORE );
    p_wk->p_text  = WBM_TEXT_Init( BG_FRAME_M_TEXT, PLT_FONT_M, PLT_TEXT_M, CGR_OFS_M_TEXT, HEAPID_WIFIBATTLEMATCH_CORE );

	}
  //メインシーケンス
  p_wk->main_seq  = WifiBattleMatch_Random_MainSeq;
	
	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	コアプロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  enum 
  { 
    SEQ_START_DISCONNECT,
    SEQ_WAIT_DISCONNECT,
  };

	WIFIBATTLEMATCH_WORK	      *p_wk	= p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_param_adrs;

  if( p_param->result ==  WIFIBATTLEMATCH_CORE_RESULT_FINISH )
  { 
    switch( *p_seq )
    { 
    case SEQ_START_DISCONNECT:
      WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );
      GFL_NET_Exit( NULL );
      *p_seq  = SEQ_WAIT_DISCONNECT;
      return GFL_PROC_RES_CONTINUE; 

    case SEQ_WAIT_DISCONNECT:
      if( GFL_NET_IsExit() )
      { 
        break;
      }
      return GFL_PROC_RES_CONTINUE;
    }
  }

	//モジュールの破棄
	{	
    WBM_TEXT_Exit( p_wk->p_text );

    if( p_param->result !=  WIFIBATTLEMATCH_CORE_RESULT_FINISH )
    { 
      WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );
    }

    if( p_wk->p_matchinfo )
    { 
      MATCHINFO_Exit( p_wk->p_matchinfo );
    }

    if( p_wk->p_playerinfo )
    { 
      PLAYERINFO_RND_Exit( p_wk->p_playerinfo );
      p_wk->p_playerinfo  = NULL;
    }
	}

  //リソース解放
  GFL_BG_FillCharacterRelease(BG_FRAME_M_TEXT, 1, CGR_OFS_M_CLEAR );

	//共通モジュールの破棄
	WORDSET_Delete( p_wk->p_word );
	GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );

	//グラフィック破棄
	WIFIBATTLEMATCH_GRAPHIC_Exit( p_wk->p_graphic );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFIBATTLEMATCH_CORE );
	
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	メインプロセス処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_CORE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_MAIN,
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
  };


	WIFIBATTLEMATCH_WORK	*p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM *p_param  = p_param_adrs;


  switch( p_wk->fade_seq )
  { 
  case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
      p_wk->fade_seq  = SEQ_FADEIN_WAIT;
    break;
  case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{
      p_wk->fade_seq  = SEQ_MAIN;
    }
    break;
  case SEQ_MAIN:
    //メインシーケンス
    if( p_wk->main_seq( p_wk, p_param, p_seq ) )
    { 
      p_wk->fade_seq  = SEQ_FADEOUT_START;
    }

    //共通のエラー処理（SC,GDB以外）
    if( WIFIBATTLEMATCH_NET_CheckError( p_wk->p_net ) )
    { 
      p_param->result = WIFIBATTLEMATCH_CORE_RESULT_ERR_NEXT_LOGIN;
      p_wk->fade_seq  = SEQ_FADEOUT_START;
    }
    break;

  case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    p_wk->fade_seq  = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{
      return GFL_PROC_RES_FINISH;
    }
  }


	//描画
	WIFIBATTLEMATCH_GRAPHIC_2D_Draw( p_wk->p_graphic );

	//プリント
	PRINTSYS_QUE_Main( p_wk->p_que );

  //テキスト
  WBM_TEXT_Main( p_wk->p_text );

  //文字表示
  if( p_wk->p_playerinfo )
  { 
    PLAYERINFO_PrintMain( p_wk->p_playerinfo, p_wk->p_que );
  }
  if( p_wk->p_matchinfo )
  { 
    MATCHINFO_PrintMain( p_wk->p_matchinfo, p_wk->p_que );
  }

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
 *    ランダムマッチの処理
 */
//=============================================================================
//-------------------------------------
///	ランダムマッチシーケンス
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_RND_SUBSEQ_INIT,         //初期化
  WIFIBATTLEMATCH_RND_SUBSEQ_START,         //開始
  WIFIBATTLEMATCH_RND_SUBSEQ_RATE_START,    //レーティング開始
  WIFIBATTLEMATCH_RND_SUBSEQ_RATE_MATCHING, //レーティングマッチング
  WIFIBATTLEMATCH_RND_SUBSEQ_RATE_BTL_END,  //レーティングバトル終了後
  WIFIBATTLEMATCH_RND_SUBSEQ_FREE_START,    //フリー開始
  WIFIBATTLEMATCH_RND_SUBSEQ_FREE_MATCHING, //フリーマッチング
  WIFIBATTLEMATCH_RND_SUBSEQ_FREE_BTL_END,  //フリーバトル終了後

  WIFIBATTLEMATCH_RND_SUBSEQ_ERR_RETURN_LOGIN,  //ログインまでもどる

  WIFIBATTLEMATCH_RND_SUBSEQ_MAX,
  WIFIBATTLEMATCH_RND_SUBSEQ_EXIT = WIFIBATTLEMATCH_RND_SUBSEQ_MAX,

} WIFIBATTLEMATCH_RND_SUBSEQ;

//-------------------------------------
///	ランダムマッチシーケンス関数
//=====================================
static int WIFIBATTLEMATCH_RND_SUBSEQ_Init( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq );
static int WIFIBATTLEMATCH_RND_SUBSEQ_Start( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq );
//レーティング処理
static int WIFIBATTLEMATCH_RND_SUBSEQ_Rate_Start( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq );
static int WIFIBATTLEMATCH_RND_SUBSEQ_Rate_Matching( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq );
static int WIFIBATTLEMATCH_RND_SUBSEQ_Rate_EndBattle( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq );
//フリー処理
static int WIFIBATTLEMATCH_RND_SUBSEQ_Free_Start( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq );
static int WIFIBATTLEMATCH_RND_SUBSEQ_Free_Matching( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq );
static int WIFIBATTLEMATCH_RND_SUBSEQ_Free_EndBattle( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq );

static int WIFIBATTLEMATCH_RND_SUBSEQ_Err_ReturnLogin( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq );

//-------------------------------------
///	ランダムマッチシーケンス関数テーブル
//=====================================
static const WIFIBATTLEMATCH_SUBSEQ_FUNCTION  sc_rnd_subseq[WIFIBATTLEMATCH_RND_SUBSEQ_MAX]  =
{ 
  WIFIBATTLEMATCH_RND_SUBSEQ_Init,
  WIFIBATTLEMATCH_RND_SUBSEQ_Start,
  WIFIBATTLEMATCH_RND_SUBSEQ_Rate_Start,
  WIFIBATTLEMATCH_RND_SUBSEQ_Rate_Matching,
  WIFIBATTLEMATCH_RND_SUBSEQ_Rate_EndBattle,
  WIFIBATTLEMATCH_RND_SUBSEQ_Free_Start,
  WIFIBATTLEMATCH_RND_SUBSEQ_Free_Matching,
  WIFIBATTLEMATCH_RND_SUBSEQ_Free_EndBattle,
  WIFIBATTLEMATCH_RND_SUBSEQ_Err_ReturnLogin,
};

//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  メインシーケンス
 *
 *	@param	WIFIBATTLEMATCH_WORK  *p_wk     ワーク
 *	@param  WIFIBATTLEMATCH_CORE_PARAM *p_param  引数
 *	@param	int *p_seq                      シーケンス
 *
 *	@return TRUEで終了  FALSEでループ
 */
//-----------------------------------------------------------------------------
static BOOL WifiBattleMatch_Random_MainSeq( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_seq )
{ 
  *p_seq  = sc_rnd_subseq[ *p_seq ]( p_wk, p_param, &p_wk->sub_seq );

  if( *p_seq  == WIFIBATTLEMATCH_RND_SUBSEQ_EXIT )
  { 
    return TRUE;
  }
  else
  { 
    return  FALSE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ初期化
 *
 *	@param	WIFIBATTLEMATCH_WORK *p_wk  ワーク
 *	@param  p_param                     引数
 *	@param	int *p_subseq                  現在のサブシーケンス
 *
 *	@return 次のメインシーケンス
 */
//-----------------------------------------------------------------------------
static int WIFIBATTLEMATCH_RND_SUBSEQ_Init( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq )
{ 

  NAGI_Printf( " mode%d ret%d\n", p_param->mode, p_param->retmode );
  switch( p_param->mode )
  { 
  case WIFIBATTLEMATCH_CORE_MODE_START:
    *p_subseq = 0;
    return WIFIBATTLEMATCH_RND_SUBSEQ_START;

  case WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE:
    if( p_param->retmode == WIFIBATTLEMATCH_CORE_RETMODE_RATE )
    { 
      *p_subseq = 0;
      return WIFIBATTLEMATCH_RND_SUBSEQ_RATE_BTL_END;
    }
    else if( p_param->retmode == WIFIBATTLEMATCH_CORE_RETMODE_FREE )
    { 
      *p_subseq = 0;
      return WIFIBATTLEMATCH_RND_SUBSEQ_FREE_BTL_END;
    }
  }

  GF_ASSERT( 0 );
  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ開始
 *
 *	@param	WIFIBATTLEMATCH_WORK *p_wk  ワーク
 *	@param  p_param                     引数
 *	@param	int *p_subseq                  現在のサブシーケンス
 *
 *	@return 次のメインシーケンス
 */
//-----------------------------------------------------------------------------
static int WIFIBATTLEMATCH_RND_SUBSEQ_Start( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq )
{ 
  enum
  { 
    SEQ_START_INIT,
    SEQ_WAIT_INIT,
    SEQ_START_RECVDATA_SAKE,
    SEQ_WAIT_RECVDATA_SAKE,
    SEQ_CHECK_GPF,
    SEQ_SELECT_MSG,
    SEQ_START_SELECT_MODE,
    SEQ_WAIT_SELECT_MODE,
    SEQ_START_CANCEL_MSG,
    SEQ_START_CANCEL_SELECT,
    SEQ_WAIT_CANCEL_SELECT,


    SEQ_WAIT_MSG,
  };

  switch( *p_subseq )
  { 
    //-------------------------------------
    ///	初期化
    //=====================================
  case SEQ_START_INIT:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_000, p_wk->p_font );
    WIFIBATTLEMATCH_NET_StartInitialize( p_wk->p_net );
    *p_subseq = SEQ_WAIT_INIT;
    break;
    
  case SEQ_WAIT_INIT:
    { 
      DWCGdbError error;
      SAVE_CONTROL_WORK *p_save = GAMEDATA_GetSaveControlWork(p_param->p_param->p_game_data);
      if( WIFIBATTLEMATCH_NET_WaitInitialize( p_wk->p_net, p_save, &error ) )
      { 
        *p_subseq = SEQ_START_RECVDATA_SAKE;
      }
      else
      { 
        WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE type;
        type = WIFIBATTLEMATCH_GDB_GetErrorRepairType( error );
        switch( type )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY:       //もう一度
          *p_subseq = SEQ_START_INIT;
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          return WIFIBATTLEMATCH_RND_SUBSEQ_ERR_RETURN_LOGIN;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:       //電源切断
          NetErr_DispCallFatal();
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	初期データ受信
    //=====================================
  case SEQ_START_RECVDATA_SAKE:
    *p_subseq       = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_WAIT_RECVDATA_SAKE;
    break;

  case SEQ_WAIT_RECVDATA_SAKE:
    *p_subseq       = SEQ_CHECK_GPF;
    break;

  case SEQ_CHECK_GPF:
    if( 1 )
    { 
      *p_subseq = SEQ_SELECT_MSG;
    }
    else
    { 
      *p_subseq = SEQ_SELECT_MSG;
    }
    break;

    //-------------------------------------
    ///	モード選択
    //=====================================
  case SEQ_SELECT_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_001, p_wk->p_font );
    *p_subseq       = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_SELECT_MODE;
    break;

  case SEQ_START_SELECT_MODE:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_003;
      setup.strID[1]= WIFIMATCH_TEXT_004;
      setup.strID[2]= WIFIMATCH_TEXT_005;
      setup.list_max= 3;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_subseq     = SEQ_WAIT_SELECT_MODE;
    }
    break;

  case SEQ_WAIT_SELECT_MODE:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          *p_subseq = 0;
          p_param->retmode = WIFIBATTLEMATCH_CORE_RETMODE_FREE;
          return WIFIBATTLEMATCH_RND_SUBSEQ_FREE_START;
        case 1:
          *p_subseq = 0;
          p_param->retmode = WIFIBATTLEMATCH_CORE_RETMODE_RATE;
          return WIFIBATTLEMATCH_RND_SUBSEQ_RATE_START;
        case 2:
          *p_subseq = SEQ_START_CANCEL_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	やめる
    //=====================================
  case SEQ_START_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_006, p_wk->p_font );
    *p_subseq       = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_CANCEL_SELECT;
    break;

  case SEQ_START_CANCEL_SELECT:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_subseq     = SEQ_WAIT_CANCEL_SELECT;
    }
    break;

  case SEQ_WAIT_CANCEL_SELECT:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          return WIFIBATTLEMATCH_RND_SUBSEQ_EXIT; 

        case 1:
          *p_subseq = SEQ_SELECT_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      *p_subseq = p_wk->next_seq;
    }
    break;
  };

  //今のまま
  return WIFIBATTLEMATCH_RND_SUBSEQ_START;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  レーティング開始
 *
 *	@param	WIFIBATTLEMATCH_WORK *p_wk  ワーク
 *	@param  p_param                     引数
 *	@param	int *p_subseq                  現在のサブシーケンス
 *
 *	@return 次のメインシーケンス
 */
//-----------------------------------------------------------------------------
static int WIFIBATTLEMATCH_RND_SUBSEQ_Rate_Start( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq )
{ 
  enum
  { 
    SEQ_START_RECVRATE_SAKE,
    SEQ_WAIT_RECVRATE_SAKE,
    SEQ_START_POKECHECK_SERVER,
    SEQ_WAIT_POKECHECK_SERVER,
    SEQ_CHECK_POKE,

    SEQ_WAIT_MSG,
  };

  switch( *p_subseq )
  { 
    //-------------------------------------
    ///	レーティングデータ受信
    //=====================================
  case SEQ_START_RECVRATE_SAKE:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_000, p_wk->p_font );
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_RND_SCORE, &p_wk->rnd_score );
    *p_subseq       = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_WAIT_RECVRATE_SAKE;
    break;

  case SEQ_WAIT_RECVRATE_SAKE:
    { 
      DWCGdbError result  = DWC_GDB_ERROR_NONE;
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net, &result ) )
      { 

        //自分のデータを表示
        PLAYERINFO_RANDOMMATCH_DATA info_setup;
        GFL_CLUNIT	*p_unit;

        p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );

        GFL_STD_MemClear( &info_setup, sizeof(PLAYERINFO_RANDOMMATCH_DATA) );
        info_setup.btl_rule = p_param->p_param->btl_rule;
        info_setup.rate     = p_wk->rnd_score.single_rate;
        info_setup.win_cnt  = p_wk->rnd_score.single_win;
        info_setup.lose_cnt = p_wk->rnd_score.single_lose;
        info_setup.btl_cnt  = p_wk->rnd_score.single_win + p_wk->rnd_score.single_lose; //@todo
        if( p_wk->p_playerinfo )
        { 
          PLAYERINFO_RND_Exit( p_wk->p_playerinfo );
          p_wk->p_playerinfo  = NULL;
        }
        {
          MYSTATUS  *p_my;
          p_my  = GAMEDATA_GetMyStatus(p_param->p_param->p_game_data); 
          p_wk->p_playerinfo	= PLAYERINFO_RND_Init( &info_setup, TRUE, p_my, p_unit, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, HEAPID_WIFIBATTLEMATCH_CORE );
        }

        //自分のレートを保存
        { 
          WIFIBATTLEMATCH_ENEMYDATA *p_player;
          p_player  = p_param->p_player_data;
          p_player->rate  = p_wk->rnd_score.single_rate;
        }


        *p_subseq       = SEQ_START_POKECHECK_SERVER;
      }
      else
      { 
        WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE type;
        type = WIFIBATTLEMATCH_GDB_GetErrorRepairType( result );
        switch( type )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY:       //もう一度
          *p_subseq = SEQ_START_RECVRATE_SAKE;
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          return WIFIBATTLEMATCH_RND_SUBSEQ_ERR_RETURN_LOGIN;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:       //電源切断
          NetErr_DispCallFatal();
          break;
        }
      }
    }
    break;

    //-------------------------------------
    /// ポケモン不正チェック
    //=====================================
  case SEQ_START_POKECHECK_SERVER:
    *p_subseq       = SEQ_WAIT_POKECHECK_SERVER;
    break;

  case SEQ_WAIT_POKECHECK_SERVER:
    *p_subseq       = SEQ_CHECK_POKE;
    break;

  case SEQ_CHECK_POKE:
    *p_subseq = 0;
    return WIFIBATTLEMATCH_RND_SUBSEQ_RATE_MATCHING;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      *p_subseq = p_wk->next_seq;
    }
    break;

  }

  //今のまま
  return WIFIBATTLEMATCH_RND_SUBSEQ_RATE_START;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  レーティングのマッチング
 *
 *	@param	WIFIBATTLEMATCH_WORK *p_wk  ワーク
 *	@param  p_param                     引数
 *	@param	int *p_subseq                  現在のサブシーケンス
 *
 *	@return 次のメインシーケンス
 */
//-----------------------------------------------------------------------------
static int WIFIBATTLEMATCH_RND_SUBSEQ_Rate_Matching( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq )
{ 
  enum
  { 
    SEQ_START_MATCHING,
    SEQ_START_MATCHING_MSG,
    SEQ_WAIT_MATCHING,
    SEQ_START_CHECK_CHEAT,
    SEQ_WAIT_CHECK_CHEAT,
    SEQ_START_SENDDATA,
    SEQ_WAIT_SENDDATA,
    SEQ_OK_MATCHING_MSG,
    SEQ_OK_MATCHING_WAIT,
    SEQ_END_MATCHING_MSG,
    SEQ_END_MATCHING,

    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  switch( *p_subseq )
  { 
    //-------------------------------------
    ///	マッチング開始
    //=====================================
  case SEQ_START_MATCHING:
    WIFIBATTLEMATCH_NET_StartMatchMake( p_wk->p_net, p_param->p_param->btl_rule ); 
    *p_subseq = SEQ_START_MATCHING_MSG;
    break;

  case SEQ_START_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_009, p_wk->p_font );
    *p_subseq = SEQ_WAIT_MATCHING;
    break;

  case SEQ_WAIT_MATCHING:
    if( WIFIBATTLEMATCH_NET_GetSeqMatchMake( p_wk->p_net ) < WIFIBATTLEMATCH_NET_SEQ_CONNECT_START )
    { 
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      { 
        *p_subseq = SEQ_START_SELECT_CANCEL_MSG;
      }
    }

    if( WIFIBATTLEMATCH_NET_WaitMatchMake( p_wk->p_net ) )
    { 
      *p_subseq = SEQ_START_CHECK_CHEAT;
    }
    break;

    //-------------------------------------
    ///	見つかった後の相手の不正チェック
    //=====================================
  case SEQ_START_CHECK_CHEAT:


    *p_subseq = SEQ_WAIT_CHECK_CHEAT;
    break;

  case SEQ_WAIT_CHECK_CHEAT:
    *p_subseq = SEQ_START_SENDDATA;
    break;

  case SEQ_START_SENDDATA:
    { 
      WIFIBATTLEMATCH_ENEMYDATA *p_my_data = p_param->p_player_data;
      if( WIFIBATTLEMATCH_NET_StartEnemyData( p_wk->p_net, p_my_data ) )
      { 
        *p_subseq       = SEQ_WAIT_SENDDATA;
      }
    }
    break;

  case SEQ_WAIT_SENDDATA:
    {
      WIFIBATTLEMATCH_ENEMYDATA *p_recv_data;
      if( WIFIBATTLEMATCH_NET_WaitEnemyData( p_wk->p_net, &p_recv_data ) )
      { 
        GFL_CLUNIT  *p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );
        WIFIBATTLEMATCH_ENEMYDATA *p_enemy_data = p_param->p_enemy_data;
        GFL_STD_MemCopy( p_recv_data, p_enemy_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );

        WifiBattleMatch_Darty_ModifiEnemyData( p_enemy_data );

        if( p_wk->p_matchinfo )
        { 
          MATCHINFO_Exit( p_wk->p_matchinfo );
        }
        { 
          p_wk->p_matchinfo		= MATCHINFO_Init( p_enemy_data, p_unit, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, HEAPID_WIFIBATTLEMATCH_CORE );
        }

        *p_subseq       = SEQ_OK_MATCHING_MSG;
      }
    }
    break;

  case SEQ_OK_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_010, p_wk->p_font );
    *p_subseq       = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_OK_MATCHING_WAIT;
    break;

  case SEQ_OK_MATCHING_WAIT:
    if( p_wk->cnt++ > ENEMYDATA_WAIT_SYNC )
    { 
      p_wk->cnt      = 0;
      *p_subseq      = SEQ_END_MATCHING_MSG;
    }
    break;

  case SEQ_END_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_012, p_wk->p_font );
    *p_subseq = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_END_MATCHING;
    break;

  case SEQ_END_MATCHING:
    p_param->result = WIFIBATTLEMATCH_CORE_RESULT_NEXT_BATTLE;
    return WIFIBATTLEMATCH_RND_SUBSEQ_EXIT; 

    //-------------------------------------
    ///	キャンセル処理
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_011, p_wk->p_font );
    *p_subseq       = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_SELECT_CANCEL;
    break;

  case SEQ_START_SELECT_CANCEL:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_subseq     = SEQ_WAIT_SELECT_CANCEL;
    }
    break;

  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          return WIFIBATTLEMATCH_RND_SUBSEQ_EXIT; //終了
        case 1:
          *p_subseq = SEQ_START_MATCHING_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      *p_subseq = p_wk->next_seq;
    }
    break;
  }

  //今のまま
  return WIFIBATTLEMATCH_RND_SUBSEQ_RATE_MATCHING;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  レーティングの戦闘終了後
 *
 *	@param	WIFIBATTLEMATCH_WORK *p_wk  ワーク
 *	@param  p_param                     引数
 *	@param	int *p_subseq                  現在のサブシーケンス
 *
 *	@return 次のメインシーケンス
 */
//-----------------------------------------------------------------------------
static int WIFIBATTLEMATCH_RND_SUBSEQ_Rate_EndBattle( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq )
{ 
 
  enum
  { 
    SEQ_START_MSG,
    SEQ_START_REPORT_ATLAS,
    SEQ_WAIT_REPORT_ATLAS,

    SEQ_START_RECVDATA_SAKE,
    SEQ_WAIT_RECVDATA_SAKE,

    SEQ_START_SAVE_MSG,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,

    SEQ_START_SELECT_BTLREC_MSG,
    SEQ_START_SELECTBTLREC,
    SEQ_WAIT_SELECTBTLREC,

    SEQ_START_DISCONNECT,
    SEQ_WAIT_DISCONNECT,

    SEQ_START_SELECT_CONTINUE_MSG,
    SEQ_START_SELECT_CONTINUE,
    SEQ_WAIT_SELECT_CONTINUE,

    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  switch( *p_subseq )
  { 
    //-------------------------------------
    ///	Atlasへの書き込み
    //=====================================
  case SEQ_START_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_013, p_wk->p_font );
    *p_subseq       = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_REPORT_ATLAS;
    break;
  case SEQ_START_REPORT_ATLAS:
    WIFIBATTLEMATCH_SC_Start( p_wk->p_net, p_param->p_param->btl_rule, p_param->btl_result );
    *p_subseq = SEQ_WAIT_REPORT_ATLAS;
    break;
  case SEQ_WAIT_REPORT_ATLAS:
    { 
      DWCScResult result;
      if( WIFIBATTLEMATCH_SC_Process( p_wk->p_net, &result ) )
      { 
        NAGI_Printf( "送ったよ！%d\n", result );
        *p_subseq = SEQ_START_RECVDATA_SAKE;
      }
      else
      { 
        WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE type;
        type = WIFIBATTLEMATCH_SC_GetErrorRepairType( result );
        switch( type )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY:       //もう一度
          *p_subseq = SEQ_START_REPORT_ATLAS;
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          return WIFIBATTLEMATCH_RND_SUBSEQ_ERR_RETURN_LOGIN;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:       //電源切断
          NetErr_DispCallFatal();
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	Sakeから取得
    //=====================================
  case SEQ_START_RECVDATA_SAKE:
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_RND_SCORE, &p_wk->rnd_score );
    *p_subseq = SEQ_WAIT_RECVDATA_SAKE;
    break;
  case SEQ_WAIT_RECVDATA_SAKE:
    { 
      DWCGdbError result  = DWC_GDB_ERROR_NONE;
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net, &result ) )
      { 
        PLAYERINFO_RANDOMMATCH_DATA info_setup;
        GFL_CLUNIT	*p_unit;

        p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );

        GFL_STD_MemClear( &info_setup, sizeof(PLAYERINFO_RANDOMMATCH_DATA) );
        info_setup.btl_rule = p_param->p_param->btl_rule;
        info_setup.rate     = p_wk->rnd_score.single_rate;
        info_setup.win_cnt  = p_wk->rnd_score.single_win;
        info_setup.lose_cnt = p_wk->rnd_score.single_lose;
        info_setup.btl_cnt  = p_wk->rnd_score.single_win + p_wk->rnd_score.single_lose; //@todo

        //セーブデータ
        RNDMATCH_SetParam( p_param->p_rndmatch, RNDMATCH_TYPE_RATE_SINGLE + info_setup.btl_rule, RNDMATCH_PARAM_IDX_RATE, info_setup.rate );
        RNDMATCH_SetParam( p_param->p_rndmatch, RNDMATCH_TYPE_RATE_SINGLE + info_setup.btl_rule, RNDMATCH_PARAM_IDX_WIN, info_setup.win_cnt );
        RNDMATCH_SetParam( p_param->p_rndmatch, RNDMATCH_TYPE_RATE_SINGLE + info_setup.btl_rule, RNDMATCH_PARAM_IDX_LOSE, info_setup.btl_cnt );

        if( p_wk->p_playerinfo )
        { 
          PLAYERINFO_RND_Exit( p_wk->p_playerinfo );
          p_wk->p_playerinfo  = NULL;
        }

        {
          MYSTATUS  *p_my;
          p_my  = GAMEDATA_GetMyStatus(p_param->p_param->p_game_data); 
          p_wk->p_playerinfo	= PLAYERINFO_RND_Init( &info_setup, TRUE, p_my, p_unit, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, HEAPID_WIFIBATTLEMATCH_CORE );
        }

        *p_subseq       = SEQ_START_DISCONNECT;
      }
      else
      { 
        WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE type;
        type = WIFIBATTLEMATCH_GDB_GetErrorRepairType( result );
        switch( type )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY:       //もう一度
          *p_subseq = SEQ_START_RECVDATA_SAKE;
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          return WIFIBATTLEMATCH_RND_SUBSEQ_ERR_RETURN_LOGIN;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:       //電源切断
          NetErr_DispCallFatal();
          break;
        }
      }
    }
    break;

    //-------------------------------------
    /// 切断処理
    //=====================================
  case SEQ_START_DISCONNECT:
    *p_subseq = SEQ_WAIT_DISCONNECT;
    break;

  case SEQ_WAIT_DISCONNECT:
    if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE ) )
    { 
      *p_subseq = SEQ_START_SELECT_BTLREC_MSG;
    }
    break;

    //-------------------------------------
    /// レーティングセーブ処理
    //=====================================
  case SEQ_START_SAVE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_017, p_wk->p_font );
    *p_subseq       = SEQ_START_SAVE;
    break;
  case SEQ_START_SAVE:
    GAMEDATA_SaveAsyncStart(p_param->p_param->p_game_data);
    *p_subseq       = SEQ_WAIT_SAVE;
    break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(p_param->p_param->p_game_data);
      switch(result){
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
      case SAVE_RESULT_NG:
        *p_subseq       = SEQ_START_SELECT_BTLREC_MSG;
        break;
      }
    }
    break;

    //-------------------------------------
    /// 録画確認
    //=====================================
  case SEQ_START_SELECT_BTLREC_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_014, p_wk->p_font );
    *p_subseq = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_SELECTBTLREC;
    break;

  case SEQ_START_SELECTBTLREC:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_subseq     = SEQ_WAIT_SELECTBTLREC;
    }
    break;

  case SEQ_WAIT_SELECTBTLREC:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          *p_subseq = SEQ_START_SELECT_CONTINUE_MSG;
          break;
        case 1:
          *p_subseq = SEQ_START_SELECT_CONTINUE_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    /// 対戦続行確認
    //=====================================
  case SEQ_START_SELECT_CONTINUE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_015, p_wk->p_font );
    *p_subseq = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_SELECT_CONTINUE;
    break;
  case SEQ_START_SELECT_CONTINUE:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_subseq     = SEQ_WAIT_SELECT_CONTINUE;
    }
    break;
  case SEQ_WAIT_SELECT_CONTINUE:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          *p_subseq = 0;
          return WIFIBATTLEMATCH_RND_SUBSEQ_RATE_START;
        case 1:
          *p_subseq = SEQ_START_SELECT_CANCEL_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    /// ランダムマッチ終了確認
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_016, p_wk->p_font );
    *p_subseq = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_SELECT_CANCEL;
    break;
  case SEQ_START_SELECT_CANCEL:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_subseq     = SEQ_WAIT_SELECT_CANCEL;
    }
    break;
  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          return WIFIBATTLEMATCH_RND_SUBSEQ_EXIT; //終了
        case 1:
          *p_subseq = SEQ_START_SELECT_CONTINUE_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      *p_subseq = p_wk->next_seq;
    }
    break;
  }


  //今のまま
  return WIFIBATTLEMATCH_RND_SUBSEQ_RATE_BTL_END;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  フリーモード開始
 *
 *	@param	WIFIBATTLEMATCH_WORK *p_wk  ワーク
 *	@param  p_param                     引数
 *	@param	int *p_subseq               現在のサブシーケンス
 *
 *	@return 次のメインシーケンス
 */
//-----------------------------------------------------------------------------
static int WIFIBATTLEMATCH_RND_SUBSEQ_Free_Start( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq )
{
  enum
  { 
    SEQ_START_FREE_MSG,
    SEQ_WAIT_MSG,
  };

  switch( *p_subseq )
  { 
    //-------------------------------------
    ///	フリーモード開始メッセージ
    //=====================================
  case SEQ_START_FREE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_002, p_wk->p_font );
    { 
      //自分のデータを表示
      PLAYERINFO_RANDOMMATCH_DATA info_setup;
      GFL_CLUNIT	*p_unit;

      p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );

      GFL_STD_MemClear( &info_setup, sizeof(PLAYERINFO_RANDOMMATCH_DATA) );
      info_setup.btl_rule = p_param->p_param->btl_rule;
      info_setup.rate     = 0;
      info_setup.win_cnt  = RNDMATCH_GetParam( p_param->p_rndmatch, p_param->btl_rule, RNDMATCH_PARAM_IDX_WIN );
      info_setup.lose_cnt = RNDMATCH_GetParam( p_param->p_rndmatch, p_param->btl_rule, RNDMATCH_PARAM_IDX_LOSE );
      info_setup.btl_cnt  = info_setup.win_cnt + info_setup.lose_cnt; //@todo
      if( p_wk->p_playerinfo )
      { 
        PLAYERINFO_RND_Exit( p_wk->p_playerinfo );
        p_wk->p_playerinfo  = NULL;
      }
      {
        MYSTATUS  *p_my;
        p_my  = GAMEDATA_GetMyStatus(p_param->p_param->p_game_data); 
        p_wk->p_playerinfo	= PLAYERINFO_RND_Init( &info_setup, FALSE, p_my, p_unit, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, HEAPID_WIFIBATTLEMATCH_CORE );
      }


    }


    *p_subseq       = SEQ_WAIT_MSG;
    break;

  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    {
      *p_subseq = 0;
      return WIFIBATTLEMATCH_RND_SUBSEQ_FREE_MATCHING;
    }
    break;

  }

  //今のまま
  return WIFIBATTLEMATCH_RND_SUBSEQ_FREE_START;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  フリーモードのマッチング処理
 *
 *	@param	WIFIBATTLEMATCH_WORK *p_wk  ワーク
 *	@param  p_param                     引数
 *	@param	int *p_subseq               現在のサブシーケンス
 *
 *	@return 次のメインシーケンス
 */
//-----------------------------------------------------------------------------
static int WIFIBATTLEMATCH_RND_SUBSEQ_Free_Matching( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq )
{ 

  enum
  { 
    SEQ_START_MATCHING,
    SEQ_START_MATCHING_MSG,
    SEQ_WAIT_MATCHING,
    SEQ_START_CHECK_CHEAT,
    SEQ_WAIT_CHECK_CHEAT,
    SEQ_START_SENDDATA,
    SEQ_WAIT_SENDDATA,
    SEQ_OK_MATCHING_MSG,
    SEQ_OK_MATCHING_WAIT,
    SEQ_END_MATCHING_MSG,
    SEQ_END_MATCHING,

    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  switch( *p_subseq )
  { 
    //-------------------------------------
    ///	マッチング開始
    //=====================================
  case SEQ_START_MATCHING:
    WIFIBATTLEMATCH_NET_StartMatchMake( p_wk->p_net, p_param->p_param->btl_rule ); 
    *p_subseq = SEQ_START_MATCHING_MSG;
    break;

  case SEQ_START_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_009, p_wk->p_font );
    *p_subseq = SEQ_WAIT_MATCHING;
    break;

  case SEQ_WAIT_MATCHING:
    if( WIFIBATTLEMATCH_NET_GetSeqMatchMake( p_wk->p_net ) < WIFIBATTLEMATCH_NET_SEQ_CONNECT_START )
    { 
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      { 
        *p_subseq = SEQ_START_SELECT_CANCEL_MSG;
      }
    }

    if( WIFIBATTLEMATCH_NET_WaitMatchMake( p_wk->p_net ) )
    { 
      *p_subseq = SEQ_START_SENDDATA;
    }
    break;

    //-------------------------------------
    ///	データ送受信
    //=====================================
  case SEQ_START_SENDDATA:
    { 
      WIFIBATTLEMATCH_ENEMYDATA *p_my_data = p_param->p_player_data;
      if( WIFIBATTLEMATCH_NET_StartEnemyData( p_wk->p_net, p_my_data ) )
      { 
        *p_subseq       = SEQ_WAIT_SENDDATA;
      }
    }
    break;

  case SEQ_WAIT_SENDDATA:
    {
      WIFIBATTLEMATCH_ENEMYDATA *p_recv_data;
      if( WIFIBATTLEMATCH_NET_WaitEnemyData( p_wk->p_net, &p_recv_data ) )
      { 
        GFL_CLUNIT  *p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );
        WIFIBATTLEMATCH_ENEMYDATA *p_enemy_data = p_param->p_enemy_data;
        GFL_STD_MemCopy( p_recv_data, p_enemy_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );

        if( p_wk->p_matchinfo )
        { 
          MATCHINFO_Exit( p_wk->p_matchinfo );
        }
        { 
          p_wk->p_matchinfo		= MATCHINFO_Init( p_enemy_data, p_unit, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, HEAPID_WIFIBATTLEMATCH_CORE );
        }

        *p_subseq       = SEQ_OK_MATCHING_MSG;
      }
    }
    break;

  case SEQ_OK_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_010, p_wk->p_font );
    *p_subseq       = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_OK_MATCHING_WAIT;
    break;

  case SEQ_OK_MATCHING_WAIT:
    if( p_wk->cnt++ > ENEMYDATA_WAIT_SYNC )
    { 
      p_wk->cnt      = 0;
      *p_subseq      = SEQ_END_MATCHING_MSG;
    }
    break;

  case SEQ_END_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_012, p_wk->p_font );
    *p_subseq = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_END_MATCHING;
    break;

  case SEQ_END_MATCHING:
    p_param->result = WIFIBATTLEMATCH_CORE_RESULT_NEXT_BATTLE;
    return WIFIBATTLEMATCH_RND_SUBSEQ_EXIT; 

    //-------------------------------------
    ///	キャンセル処理
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_011, p_wk->p_font );
    *p_subseq       = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_SELECT_CANCEL;
    break;

  case SEQ_START_SELECT_CANCEL:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_subseq     = SEQ_WAIT_SELECT_CANCEL;
    }
    break;

  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          return WIFIBATTLEMATCH_RND_SUBSEQ_EXIT; //終了
        case 1:
          *p_subseq = SEQ_START_MATCHING_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      *p_subseq = p_wk->next_seq;
    }
    break;
  }

  //今のまま
  return WIFIBATTLEMATCH_RND_SUBSEQ_FREE_MATCHING;

}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  フリーモードの戦闘終了後
 *
 *	@param	WIFIBATTLEMATCH_WORK *p_wk  ワーク
 *	@param  p_param                     引数
 *	@param	int *p_subseq               現在のサブシーケンス
 *
 *	@return 次のメインシーケンス
 */
//-----------------------------------------------------------------------------
static int WIFIBATTLEMATCH_RND_SUBSEQ_Free_EndBattle( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq )
{ 
  enum
  { 
    SEQ_START_SELECT_BTLREC_MSG,
    SEQ_START_SELECTBTLREC,
    SEQ_WAIT_SELECTBTLREC,

    SEQ_START_DISCONNECT,
    SEQ_WAIT_DISCONNECT,

    SEQ_START_SAVE_MSG,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,

    SEQ_START_SELECT_CONTINUE_MSG,
    SEQ_START_SELECT_CONTINUE,
    SEQ_WAIT_SELECT_CONTINUE,

    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  switch( *p_subseq )
  { 
    //-------------------------------------
    /// 録画確認
    //=====================================
  case SEQ_START_SELECT_BTLREC_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_014, p_wk->p_font );
    *p_subseq = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_SELECTBTLREC;
    break;
  case SEQ_START_SELECTBTLREC:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_subseq     = SEQ_WAIT_SELECTBTLREC;
    }
    break;
  case SEQ_WAIT_SELECTBTLREC:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          *p_subseq = SEQ_START_DISCONNECT;  //@todo
          break;
        case 1:
          *p_subseq = SEQ_START_DISCONNECT;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    /// 切断処理
    //=====================================
  case SEQ_START_DISCONNECT:
    *p_subseq = SEQ_WAIT_DISCONNECT;
    break;

  case SEQ_WAIT_DISCONNECT:
    if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE ) )
    { 

      switch( p_param->btl_result )
      {
      case BTL_RESULT_WIN:
      case BTL_RESULT_RUN_ENEMY:
        RNDMATCH_AddParam( p_param->p_rndmatch, p_param->btl_rule, RNDMATCH_PARAM_IDX_WIN );
        break;
      case BTL_RESULT_LOSE:
      case BTL_RESULT_RUN:
        RNDMATCH_AddParam( p_param->p_rndmatch, p_param->btl_rule, RNDMATCH_PARAM_IDX_LOSE );
        break;
      }


      *p_subseq = SEQ_START_SELECT_CONTINUE_MSG;
    }
    break;

    //-------------------------------------
    /// セーブ処理
    //=====================================
  case SEQ_START_SAVE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_017, p_wk->p_font );
    *p_subseq = SEQ_START_SAVE;
    break;

  case SEQ_START_SAVE:
    GAMEDATA_SaveAsyncStart(p_param->p_param->p_game_data);
    *p_subseq       = SEQ_WAIT_SAVE;
    break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(p_param->p_param->p_game_data);
      switch(result){
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
      case SAVE_RESULT_NG:
        *p_subseq       = SEQ_START_SELECT_CONTINUE_MSG;
        break;
      }
    }
    break;

    //-------------------------------------
    /// 対戦続行確認
    //=====================================
  case SEQ_START_SELECT_CONTINUE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_015, p_wk->p_font );
    *p_subseq = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_SELECT_CONTINUE;
    break;
  case SEQ_START_SELECT_CONTINUE:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_subseq     = SEQ_WAIT_SELECT_CONTINUE;
    }
    break;
  case SEQ_WAIT_SELECT_CONTINUE:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          *p_subseq = 0;
          return WIFIBATTLEMATCH_RND_SUBSEQ_FREE_START;
        case 1:
          *p_subseq = SEQ_START_SELECT_CANCEL_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    /// ランダムマッチ終了確認
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_016, p_wk->p_font );
    *p_subseq = SEQ_WAIT_MSG;
    p_wk->next_seq  = SEQ_START_SELECT_CANCEL;
    break;
  case SEQ_START_SELECT_CANCEL:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_subseq     = SEQ_WAIT_SELECT_CANCEL;
    }
    break;
  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          return WIFIBATTLEMATCH_RND_SUBSEQ_EXIT; //終了
        case 1:
          *p_subseq = SEQ_START_SELECT_CONTINUE_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      *p_subseq = p_wk->next_seq;
    }
    break;
  }


  //今のまま
  return WIFIBATTLEMATCH_RND_SUBSEQ_FREE_BTL_END;
}


//----------------------------------------------------------------------------
/**
 *	@brief  エラーのためログインに戻る
 *
 *	@param	WIFIBATTLEMATCH_WORK *p_wk  ワーク
 *	@param  p_param                     引数
 *	@param	int *p_subseq               現在のサブシーケンス
 *
 *	@return 次のメインシーケンス
 */
//-----------------------------------------------------------------------------
static int WIFIBATTLEMATCH_RND_SUBSEQ_Err_ReturnLogin( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_subseq )
{ 
  p_param->result = WIFIBATTLEMATCH_CORE_RESULT_ERR_NEXT_LOGIN;
  return WIFIBATTLEMATCH_RND_SUBSEQ_EXIT; //終了
}

//=============================================================================
/**
 *  メインシーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会  メインシーケンス
 *
 *	@param	WIFIBATTLEMATCH_WORK  *p_wk     ワーク
 *	@param  WIFIBATTLEMATCH_CORE_PARAM *p_param  引数
 *	@param	int *p_seq                      シーケンス
 *
 *	@return TRUEで終了  FALSEでループ
 */
//-----------------------------------------------------------------------------
static BOOL WifiBattleMatch_WiFi_MainSeq( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_seq )
{ 

  return  FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会  メインシーケンス
 *
 *	@param	WIFIBATTLEMATCH_WORK  *p_wk     ワーク
 *	@param  WIFIBATTLEMATCH_CORE_PARAM *p_param  引数
 *	@param	int *p_seq                      シーケンス
 *
 *	@return TRUEで終了  FALSEでループ
 */
//-----------------------------------------------------------------------------
static BOOL WifiBattleMatch_Live_MainSeq( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_seq )
{ 

  return  FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグ用  メインシーケンス
 *
 *	@param	WIFIBATTLEMATCH_WORK  *p_wk     ワーク
 *	@param  WIFIBATTLEMATCH_CORE_PARAM *p_param  引数
 *	@param	int *p_seq                      シーケンス
 *
 *	@return TRUEで終了  FALSEでループ
 */
//-----------------------------------------------------------------------------
static BOOL WifiBattleMatch_Debug_MainSeq( WIFIBATTLEMATCH_WORK *p_wk, WIFIBATTLEMATCH_CORE_PARAM	*p_param, int *p_seq )
{ 
  switch( *p_seq )
  { 
  case 0:
    WIFIBATTLEMATCH_NET_StartMatchMake( p_wk->p_net, p_param->p_param->btl_rule ); 
    (*p_seq)++;
    break;

  case 1:
    if( WIFIBATTLEMATCH_NET_WaitMatchMake( p_wk->p_net ) )
    { 
      NAGI_Printf( "接続完了！\n" );
      (*p_seq)++;
    }
    break;

  case 2:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {	
      return GFL_PROC_RES_FINISH;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    { 
      WIFIBATTLEMATCH_SC_Start( p_wk->p_net, p_param->p_param->btl_rule, p_param->btl_result );
      NAGI_Printf( "SC開始！\n" );
      (*p_seq)  = 3;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    { 
      NAGI_Printf( "GDB開始！\n" );
      WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_RND_SCORE, &p_wk->rnd_score );
      (*p_seq)  = 4;
    }
    break;

  case 3:
    { 
      DWCScResult result;
      if( WIFIBATTLEMATCH_SC_Process( p_wk->p_net, &result ) )
      { 
        NAGI_Printf( "送ったよ！%d\n", result );
        (*p_seq)  = 2;
      }
      GF_ASSERT( result == DWC_SC_RESULT_NO_ERROR );
    }
    break;

  case 4:
    { 
      DWCGdbError error;
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net, &error ) )
      { 
        NAGI_Printf( "データベースからもらったよ！%d\n", error );
        (*p_seq)  = 2;
      }
      else
      { 
        WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE type;
        type = WIFIBATTLEMATCH_GDB_GetErrorRepairType( error );
        switch( type )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY:       //もう一度
          (*p_seq)  = 2;
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          return WIFIBATTLEMATCH_RND_SUBSEQ_ERR_RETURN_LOGIN;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:       //電源切断
          NetErr_DispCallFatal();
          break;
        }
      }
    }
    break;
  }

  return  FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  対戦者情報が不正だった場合、無理やり正常値にする
 *
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *p_data ワーク
 */
//-----------------------------------------------------------------------------
static void WifiBattleMatch_Darty_ModifiEnemyData( WIFIBATTLEMATCH_ENEMYDATA *p_data )
{
  //名前が不正
  { 
    int i;
    BOOL is_darty = TRUE;
    //EOMが入っていれば正常とみなす
    for( i = 0; i < PERSON_NAME_SIZE+EOM_SIZE; i++ )
    { 
      if( p_data->name[i] == GFL_STR_GetEOMCode() )
      { 
        is_darty  = FALSE;
      }
    }

    if( is_darty )
    { 
      STRBUF* p_strbuf;
      GFL_MSGDATA *p_msg;
      p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
												NARC_message_wifi_match_dat, HEAPID_WIFIBATTLEMATCH_CORE );
      p_strbuf  = GFL_MSG_CreateString( p_msg, HEAPID_WIFIBATTLEMATCH_CORE );

      //規定名に変更
      OS_TPrintf( "対戦者名が不正だったので、無理やり書き換えます\n" );
      GFL_STR_GetStringCode( p_strbuf, p_data->name, PERSON_NAME_SIZE+EOM_SIZE );

      GFL_STR_DeleteBuffer( p_strbuf );
      GFL_MSG_Delete( p_msg );
    }
  }

  //性別が不正
  if( p_data->sex != PTL_SEX_MALE && p_data->sex != PTL_SEX_FEMALE )
  { 
    OS_TPrintf( "性別が不正だったので、無理やり書き換えます %d\n", p_data->sex );
    p_data->sex = PTL_SEX_MALE;
  }

  //トレーナービュー
  //@todo
  if( p_data->trainer_view == 0xFFFF )
  { 
    OS_TPrintf( "見た目が不正だったので、無理やり書き換えます \n" );
    p_data->trainer_view  = 0;
  }

  //場所
  if( p_data->nation >= WIFI_COUNTRY_GetDataLen() )
  { 
    OS_TPrintf( "国が不正だったので、無理やり書き換えます %d %d\n", p_data->nation, p_data->area );
    p_data->nation  = 0;
  }

  //エリア
  { 
    if( p_data->area >= WIFI_COUNTRY_CountryCodeToPlaceIndexMax( p_data->nation ) )
    {
      OS_TPrintf( "地域が不正だったので、無理やり書き換えます %d %d\n", p_data->nation, p_data->area );
      p_data->area  = 0;
    }
  }

  //PMS
  if( !PMSDAT_IsValid( &p_data->pms, HEAPID_WIFIBATTLEMATCH_CORE ) )
  { 
    OS_TPrintf( "PMSが不正だったので、無理やり書き換えます\n" );
    PMSDAT_Init( &p_data->pms, PMS_TYPE_BATTLE_READY ); //何挨拶？ @todo
  }
}
