//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mystery.c
 *	@brief  ふしぎなおくりものプロセス
 *	@author	Toru=Nagihashi
 *	@data		2009.12.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "print/wordset.h"
#include "system/bmp_winframe.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "arc\mystery.naix"
#include "msg/msg_mystery.h"
#include "item/item.h"
#include "item_icon.naix"
#include "system/poke2dgra.h"

//セーブデータ
#include "savedata/mystery_data.h"

//外部プロセス
#include "net_app/wifi_login.h"
#include "title/title.h"

//ふしぎなおくりもの内モジュール
#include "mystery_graphic.h"
#include "mystery_util.h"
#include "mystery_album.h"
#include "mystery_net.h"

//デバッグ
#include "mystery_debug.h"
#include "debug/debug_str_conv.h"

//外部公開
#include "net_app/mystery.h"

//-------------------------------------
///	オーバーレイ
//=====================================
FS_EXTERN_OVERLAY(dpw_common);

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#endif //PM_DEBUG


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
	BG_FRAME_M_LIST 	=	GFL_BG_FRAME0_M, 
	BG_FRAME_M_TEXT	  =	GFL_BG_FRAME1_M, 
	BG_FRAME_M_BACK1	=	GFL_BG_FRAME2_M, 
	BG_FRAME_M_BACK2	=	GFL_BG_FRAME3_M, 

	//サブ画面
	BG_FRAME_S_TEXT	  =	GFL_BG_FRAME0_S, 
	BG_FRAME_S_BACK1	=	GFL_BG_FRAME2_S, 
	BG_FRAME_S_BACK2	=	GFL_BG_FRAME3_S, 
} ;

//-------------------------------------
///	パレット
//=====================================
enum
{
	//メイン画面BG
	PLT_BG_BACK_M	=	0,
  PLT_BG_RECV_M = 1,  //6本
  PLT_BG_CARD_M = 8,
  PLT_BG_LIST_M = 13,
  PLT_BG_TEXT_M = 14,
	PLT_BG_FONT_M = 15,

	//サブ画面BG
	PLT_BG_BACK_S	= 0,
  PLT_BG_CARD_S  = 1,
	PLT_BG_FONT_S	= 15,

	//メイン画面OBJ
	PLT_OBJ_CURSOR_M  = 0,
  PLT_OBJ_GIFT_M    = 1,
  PLT_OBJ_CARD_ICON_M       = 2,
  PLT_OBJ_CARD_SILHOUETTE_M = 3,

	//サブ画面OBJ
} ;

//-------------------------------------
///	OBJ_WORK
//=====================================
enum
{ 
  OBJ_RES_PLT_CURSOR  = 0, 

  OBJ_RES_PLT_MAX,

  OBJ_RES_CGX_CURSOR  = 0,
  OBJ_RES_CGX_MAX,

  OBJ_RES_CEL_CURSOR  = 0,
  OBJ_RES_CEL_MAX,
};
enum
{
	OBJ_CLWKID_CURSOR,
	OBJ_CLWKID_MAX,
} ;

//-------------------------------------
///	キャラ
//=====================================
enum
{ 
  //メイン画面  BG_FRAME_M_TEXT
  BG_CGX_OFS_M_CLEAR = 0,
  BG_CGX_OFS_M_LIST  = 1,
  BG_CGX_OFS_M_TEXT  = 10,
};

//-------------------------------------
///	通信方法
//=====================================
typedef enum
{
  MYSTERY_NET_MODE_WIRELESS,
  MYSTERY_NET_MODE_WIFI,
  MYSTERY_NET_MODE_IRC,
} MYSTERY_NET_MODE;

//-------------------------------------
///	デモ
//=====================================
#define MYSTERY_DEMO_GIFT_X             (128)
#define MYSTERY_DEMO_MOVE_GIFT_START_Y  (-96)
#define MYSTERY_DEMO_MOVE_GIFT_END_Y    (120)
#define MYSTERY_DEMO_MOVE_GIFT_DIFF_Y   (MYSTERY_DEMO_MOVE_GIFT_END_Y-MYSTERY_DEMO_MOVE_GIFT_START_Y)
#define MYSTERY_DEMO_MOVE_GIFT_SYNC     (60)
#define MYSTERY_DEMO_INIT_WAIT_SYNC     (60)
#define MYSTERY_DEMO_END_WAIT_SYNC     (60)


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	受信デモワーク
//=====================================
typedef struct 
{
  GFL_CLWK  *p_clwk;
  BOOL      is_end;
  u32       res_plt;
  u32       res_cgx;
  u32       res_cel;
  u32       sync;
  u32       seq;
} MYSTERY_DEMO_WORK;

//-------------------------------------
///	シーケンス管理
//=====================================
typedef struct _SEQ_WORK SEQ_WORK;	//関数型作るため前方宣言
typedef void (*SEQ_FUNCTION)( SEQ_WORK *p_wk, int *p_seq, void *p_wk_adrs );
struct _SEQ_WORK
{
	SEQ_FUNCTION	seq_function;		//実行中のシーケンス関数
	BOOL is_end;									//シーケンスシステム終了フラグ
	int seq;											//実行中のシーケンス関数の中のシーケンス
	void *p_wk_adrs;							//実行中のシーケンス関数に渡すワーク
};

//-------------------------------------
///	BGリソース管理
//=====================================
typedef struct 
{
	int dummy;
} BG_WORK;
//-------------------------------------
///	OBJリソース管理
//=====================================
typedef struct 
{
  u32       res_plt[OBJ_RES_PLT_MAX]; //リソースパレット
  u32       res_cgx[OBJ_RES_CGX_MAX]; //リソースキャラクター
  u32       res_cel[OBJ_RES_CEL_MAX]; //リソースセル
  GFL_CLWK  *p_clwk[OBJ_CLWKID_MAX];  //CLWK
} OBJ_WORK;

//-------------------------------------
///	メインワーク
//=====================================
typedef struct 
{ 
  //カード画面
  MYSTERY_CARD_WORK         *p_card;

  //アルバム
  MYSTERY_ALBUM_WORK        *p_album;

  //デモ
  MYSTERY_DEMO_WORK         demo;

  //選択肢
  MYSTERY_LIST_WORK         *p_list;

  //メニュー
  MYSTERY_MENU_WORK         *p_menu;

  //テキスト
  MYSTERY_TEXT_WORK         *p_text;

  //メイン画面用BMPWINセット
  MYSTERY_MSGWINSET_WORK    *p_winset_m;

  //サブ画面用BMPWINセット
  MYSTERY_MSGWINSET_WORK    *p_winset_s;

  //シーケンス管理
  SEQ_WORK                  seq;

	//OBJリソース
	OBJ_WORK		            	obj;

	//BGリソース
	BG_WORK				            bg;

  //グラフィック設定
  MYSTERY_GRAPHIC_WORK      *p_graphic;

  //共通で使うフォント
	GFL_FONT			            *p_font;

	//共通で使うキュー
	PRINT_QUE				          *p_que;

	//共通で使うメッセージ
	GFL_MSGDATA	          		*p_msg;

	//共通で使う単語
	WORDSET				          	*p_word;

  //通信方法
  MYSTERY_NET_MODE          mode;

  //通信
  MYSTERY_NET_WORK          *p_net;

  //引数
  MYSTERY_PARAM             *p_param;

  //セーブデータ
  MYSTERY_DATA              *p_sv;

  //ログインプロセスへの引数
  WIFILOGIN_PARAM           *p_wifilogin_param;

  //仮データ
  DOWNLOAD_GIFT_DATA        data;
} MYSTERY_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
///	BGリソース
//=====================================
static void BG_Init( BG_WORK *p_wk, HEAPID heapID );
static void BG_Exit( BG_WORK *p_wk );
//-------------------------------------
///	OBJリソース
//=====================================
static void OBJ_Init( OBJ_WORK *p_wk, GFL_CLUNIT *p_clunit, HEAPID heapID );
static void OBJ_Exit( OBJ_WORK *p_wk );
static GFL_CLWK *OBJ_GetClwk( const OBJ_WORK *cp_wk, u32 clwkID );
//-------------------------------------
///	SEQ
//=====================================
static void SEQ_Init( SEQ_WORK *p_wk, void *p_wk_adrs, SEQ_FUNCTION seq_function );
static void SEQ_Exit( SEQ_WORK *p_wk );
static void SEQ_Main( SEQ_WORK *p_wk );
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk );
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function );
static void SEQ_End( SEQ_WORK *p_wk );
//-------------------------------------
///	SEQFUNC
//=====================================
static void SEQFUNC_Start( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_StartSelect( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );  //開始選択
static void SEQFUNC_RecvGift( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );     //贈り物受け取り
static void SEQFUNC_Demo( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );         //受け取りデモ
static void SEQFUNC_CardAlbum( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );    //アルバム
static void SEQFUNC_DeleteCard( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );   //カード１枚削除
static void SEQFUNC_WifiLogin( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_DisConnect( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_End( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//-------------------------------------
///	UTIL  Mystery_utilのモジュールをパックしたもの
//=====================================
//はいいいえなどのリスト
typedef enum
{ 
  UTIL_LIST_TYPE_YESNO,
}UTIL_LIST_TYPE;
static void UTIL_CreateList( MYSTERY_WORK *p_wk, UTIL_LIST_TYPE type, HEAPID heapID );
static void UTIL_DeleteList( MYSTERY_WORK *p_wk );
//メニュー
typedef enum
{ 
  UTIL_MENU_TYPE_TOP,
  UTIL_MENU_TYPE_NETMODE,
  UTIL_MENU_TYPE_GIFT,
}UTIL_MENU_TYPE;
static void UTIL_CreateMenu( MYSTERY_WORK *p_wk, UTIL_MENU_TYPE type, HEAPID heapID );
static void UTIL_DeleteMenu( MYSTERY_WORK *p_wk );
//ガイドテキスト
static void UTIL_CreateGuideText( MYSTERY_WORK *p_wk, HEAPID heapID );
static void UTIL_DeleteGuideText( MYSTERY_WORK *p_wk );
//-------------------------------------
///	デモ
//=====================================
static void MYSTERY_DEMO_Init( MYSTERY_DEMO_WORK *p_wk, GFL_CLUNIT *p_unit, const DOWNLOAD_GIFT_DATA *cp_data, HEAPID heapID );
static void MYSTERY_DEMO_Exit( MYSTERY_DEMO_WORK *p_wk );
static void MYSTERY_DEMO_Main( MYSTERY_DEMO_WORK *p_wk );
static BOOL MYSTERY_DEMO_IsEnd( const MYSTERY_DEMO_WORK *cp_wk );

//=============================================================================
/**
 *					外部データ
*/
//=============================================================================
//-------------------------------------
///	プロセスデータ
//=====================================
const GFL_PROC_DATA MysteryGiftProcData  =
{ 
  WIFIBATTLEMATCH_PROC_Init,
  WIFIBATTLEMATCH_PROC_Main,
  WIFIBATTLEMATCH_PROC_Exit,
};
//=============================================================================
/**
 *          プロセス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ふしぎなおくりもの	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  MYSTERY_WORK  *p_wk;
  MYSTERY_PARAM *p_param  = p_param_adrs;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

  //ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MYSTERYGIFT, 0x30000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(MYSTERY_WORK), HEAPID_MYSTERYGIFT );
	GFL_STD_MemClear( p_wk, sizeof(MYSTERY_WORK) );	
  p_wk->p_param = p_param;
  p_wk->p_sv    = SaveData_GetMysteryData(SaveControl_GetPointer());

#ifdef DEBUG_SET_SAVEDATA
  MYSTERY_DEBUG_SetGiftData( &p_wk->data );
  { 
    int i;
    for( i = 0; i < 6; i++ )
    { 
      MYSTERYDATA_SetCardData( p_wk->p_sv, &p_wk->data.data );
    }
#if 0
    { 
      int i;
      const u8  *cp_data = (const u8*)&p_wk->data;
      NAGI_Printf( "size == %d\n",sizeof(DOWNLOAD_GIFT_DATA) );
      NAGI_Printf( "adrs == 0x%x\n",&p_wk->data );
      NAGI_Printf( "!!!!!!!!!!!!-- binary start --!!!!!!!!!!!!!!!\n\n" );
      for( i = 0; i < sizeof(DOWNLOAD_GIFT_DATA); i++ )
      { 
        NAGI_Printf( "%x", cp_data[i] );
      }
      NAGI_Printf( "\n" );
      NAGI_Printf( "!!!!!!!!!!!!-- binary end --!!!!!!!!!!!!!!!\n\n" );
    }
#endif
  }
#endif 

  //グラフィック設定
	p_wk->p_graphic	= MYSTERY_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_MYSTERYGIFT );

	//リソース読み込み
	BG_Init( &p_wk->bg, HEAPID_MYSTERYGIFT );
	{	
		GFL_CLUNIT	*p_clunit	= MYSTERY_GRAPHIC_GetClunit( p_wk->p_graphic );
		OBJ_Init( &p_wk->obj, p_clunit, HEAPID_MYSTERYGIFT );
	}

  //共通モジュールの初期化
  p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_MYSTERYGIFT );
	p_wk->p_que			= PRINTSYS_QUE_Create( HEAPID_MYSTERYGIFT );
	p_wk->p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_mystery_dat, HEAPID_MYSTERYGIFT );
	p_wk->p_word	= WORDSET_Create( HEAPID_MYSTERYGIFT );

  //モジュール作成
	SEQ_Init( &p_wk->seq, p_wk, SEQFUNC_Start );
  p_wk->p_text  = MYSTERY_TEXT_Init( BG_FRAME_M_TEXT, PLT_BG_FONT_M, PLT_BG_TEXT_M, BG_CGX_OFS_M_TEXT, p_wk->p_que, HEAPID_MYSTERYGIFT );
  p_wk->p_net   = MYSTERY_NET_Init( SaveControl_GetPointer(), HEAPID_MYSTERYGIFT );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ふしぎなおくりもの	メインプロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  MYSTERY_WORK  *p_wk     = p_wk_adrs;
  MYSTERY_PARAM *p_param  = p_param_adrs;

  //モジュール破棄
  MYSTERY_NET_Exit( p_wk->p_net );
  if( p_wk->p_text )
  { 
    MYSTERY_TEXT_Exit( p_wk->p_text );
  }
  SEQ_Exit( &p_wk->seq );

	//共通モジュールの破棄
	WORDSET_Delete( p_wk->p_word );
	GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );

  //リソース破棄
	OBJ_Exit( &p_wk->obj );
	BG_Exit( &p_wk->bg );

  //グラフィック破棄
	MYSTERY_GRAPHIC_Exit( p_wk->p_graphic );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_MYSTERYGIFT );

  //タイトルに戻る
  GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ふしぎなおくりもの	メインプロセス処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  MYSTERY_WORK  *p_wk     = p_wk_adrs;
  MYSTERY_PARAM *p_param  = p_param_adrs;

  //通信処理
  MYSTERY_NET_Main( p_wk->p_net );

  //シーケンス
	SEQ_Main( &p_wk->seq );
  
  //描画
	MYSTERY_GRAPHIC_2D_Draw( p_wk->p_graphic );

  //プリント
	PRINTSYS_QUE_Main( p_wk->p_que );
  if( p_wk->p_text )
  { 
    MYSTERY_TEXT_PrintMain( p_wk->p_text );
  }
  if( p_wk->p_winset_s )
  { 
    MYSTERY_MSGWINSET_PrintMain( p_wk->p_winset_s );
  }
  if( p_wk->p_winset_m )
  { 
    MYSTERY_MSGWINSET_PrintMain( p_wk->p_winset_m );
  }

	//終了
	if( SEQ_IsEnd( &p_wk->seq ) )
	{	
		return GFL_PROC_RES_FINISH;
	}
	else
	{	
		return GFL_PROC_RES_CONTINUE;
	}
}

//=============================================================================
/**
 *		BGリソース読みこみ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BGリソース読みこみ
 *
 *	@param	BG_WORK *p_wk	ワーク
 *	@param	heapID				ヒープID
 */
//-----------------------------------------------------------------------------
static void BG_Init( BG_WORK *p_wk, HEAPID heapID )
{	
	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );
		//PLT
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_mystery_fusigi_bg_00_NCLR,
				PALTYPE_MAIN_BG, PLT_BG_BACK_M*0x20, 0x20, heapID );
  	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_mystery_fushigi_back_NCLR,
				PALTYPE_MAIN_BG, PLT_BG_RECV_M*0x20, 0x20*6, heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_mystery_fusigi_bg_00_NCLR,
				PALTYPE_SUB_BG, PLT_BG_BACK_S*0x20, 0x20, heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_mystery_fusigi_card_NCLR,
				PALTYPE_SUB_BG, PLT_BG_CARD_S*0x20, 0x20, heapID );

		//CHR
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fusigi_bg_00_NCGR, 
				BG_FRAME_M_BACK2, 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fusigi_bg_00_NCGR, 
				BG_FRAME_S_BACK2, 0, 0, FALSE, heapID );

		//SCR
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mystery_fusigi_bg_00_NSCR,
				BG_FRAME_M_BACK2, 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mystery_fusigi_bg_00_NSCR,
				BG_FRAME_S_BACK2, 0, 0, FALSE, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}

	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_FONT, heapID );

		//上下画面フォントパレット
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
				PALTYPE_MAIN_BG, PLT_BG_FONT_M*0x20, 0x20, heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
				PALTYPE_SUB_BG, PLT_BG_FONT_S*0x20, 0x20, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}

  //キャラを単位で読み込み
  { 
    //BG_FRAME_M_TEXT
    GFL_BG_FillCharacter( BG_FRAME_M_TEXT, 0, 1, BG_CGX_OFS_M_CLEAR );
    BmpWinFrame_GraphicSet( BG_FRAME_M_TEXT, BG_CGX_OFS_M_LIST, PLT_BG_LIST_M, MENU_TYPE_SYSTEM, heapID );
    TalkWinFrame_GraphicSet( BG_FRAME_M_TEXT, BG_CGX_OFS_M_TEXT, PLT_BG_TEXT_M, MENU_TYPE_SYSTEM, heapID );

    //BG_FRAME_M_LIST
    GFL_BG_FillCharacter( BG_FRAME_M_LIST, 0, 1, BG_CGX_OFS_M_CLEAR );
    BmpWinFrame_GraphicSet( BG_FRAME_M_LIST, BG_CGX_OFS_M_LIST, PLT_BG_LIST_M, MENU_TYPE_SYSTEM, heapID );
    TalkWinFrame_GraphicSet( BG_FRAME_M_LIST, BG_CGX_OFS_M_LIST, PLT_BG_LIST_M, MENU_TYPE_SYSTEM, heapID );
  }

  //初期は非表示のもの
  GFL_BG_SetVisible( BG_FRAME_S_BACK1, FALSE );
}

//----------------------------------------------------------------------------
/**
 *	@brief	BGリソース破棄
 *
 *	@param	BG_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BG_Exit( BG_WORK *p_wk )
{	
  GFL_BG_FillCharacterRelease( BG_FRAME_M_LIST, 1, BG_CGX_OFS_M_CLEAR );
  GFL_BG_FillCharacterRelease( BG_FRAME_M_TEXT, 1, BG_CGX_OFS_M_CLEAR );
}
//=============================================================================
/**
 *		OBJリソース読みこみ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	OBJリソース読みこみ
 *
 *	@param	OBJ_WORK *p_wk	ワーク
 *	@param	clunit					ユニット
 *	@param	HEAPID heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void OBJ_Init( OBJ_WORK *p_wk, GFL_CLUNIT *p_clunit, HEAPID heapID )
{	
	//リソース読みこみ
	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );

		p_wk->res_plt[ OBJ_RES_PLT_CURSOR ]	= GFL_CLGRP_PLTT_Register( p_handle, 
				NARC_mystery_title_cursol_NCLR, CLSYS_DRAW_MAIN, PLT_OBJ_CURSOR_M*0x20, heapID );
		p_wk->res_cel[ OBJ_RES_CEL_CURSOR ]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_mystery_title_cursol_NCER, NARC_mystery_title_cursol_NANR, heapID );
		p_wk->res_cgx[ OBJ_RES_CGX_CURSOR ]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_mystery_title_cursol_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWK登録
	{
		int i;
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

		p_wk->p_clwk[OBJ_CLWKID_CURSOR]	=		GFL_CLACT_WK_Create( p_clunit,
				p_wk->res_cgx[OBJ_RES_CGX_CURSOR],
				p_wk->res_plt[OBJ_RES_PLT_CURSOR],
				p_wk->res_cel[OBJ_RES_CEL_CURSOR],
				&cldata,
				CLSYS_DEFREND_MAIN,
				heapID );
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[OBJ_CLWKID_CURSOR], FALSE );
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJリソース破棄
 *
 *	@param	BG_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void OBJ_Exit( OBJ_WORK *p_wk )
{	
	//CLWK破棄
	{	
		int i;
		for( i = 0; i < OBJ_CLWKID_MAX; i++ )
		{	
			if( p_wk->p_clwk[i] )
			{	
				GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
			}
		}
	}

	//リソース破棄
	{
    int i;
    for( i = 0; i < OBJ_RES_PLT_MAX; i++ )
    { 
      GFL_CLGRP_PLTT_Release( p_wk->res_plt[i] );
    }
    for( i = 0; i < OBJ_RES_CGX_MAX; i++ )
    { 
      GFL_CLGRP_CGR_Release( p_wk->res_cgx[i] );
    }
    for( i = 0; i < OBJ_RES_CEL_MAX; i++ )
    { 
      GFL_CLGRP_CELLANIM_Release( p_wk->res_cel[i] );
    }
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	CLWK取得
 *
 *	@param	const OBJ_WORK *p_wk		ワーク
 *	@param	clwkID									CLWK番号
 *
 *	@return	CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK *OBJ_GetClwk( const OBJ_WORK *cp_wk, u32 clwkID )
{	
	return cp_wk->p_clwk[ clwkID ];
}


//=============================================================================
/**
 *						SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	初期化
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	*p_param				パラメータ
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Init( SEQ_WORK *p_wk, void *p_wk_adrs, SEQ_FUNCTION seq_function )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );

	//初期化
	p_wk->p_wk_adrs	= p_wk_adrs;

	//セット
	SEQ_SetNext( p_wk, seq_function );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	破棄
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void SEQ_Exit( SEQ_WORK *p_wk )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	メイン処理
 *
 *	@param	SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Main( SEQ_WORK *p_wk )
{	
	if( !p_wk->is_end )
	{	
		p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_wk_adrs );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了取得
 *
 *	@param	const SEQ_WORK *cp_wk		ワーク
 *
 *	@return	TRUEならば終了	FALSEならば処理中
 */	
//-----------------------------------------------------------------------------
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk )
{	
	return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	次のシーケンスを設定
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( SEQ_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}
//=============================================================================
/**
 *					SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	開始
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Start( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  MYSTERY_WORK  *p_wk     = p_wk_adrs;

  //開始時のテキスト
  MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_001, p_wk->p_font );

  UTIL_CreateMenu( p_wk, UTIL_MENU_TYPE_TOP, HEAPID_MYSTERYGIFT );

	SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
}
//----------------------------------------------------------------------------
/**
 *	@brief	フェードイン
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{	
	enum
	{	
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_END;
		}
		break;

	case SEQ_END:
		SEQ_SetNext( p_seqwk, SEQFUNC_StartSelect );
		break;

	default:
		GF_ASSERT(0);
	}
	
}
//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
	enum
	{
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};	

	switch( *p_seq )
	{	
	case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
		*p_seq	= SEQ_FADEIN_WAIT;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_EXIT;
		}
		break;

	case SEQ_EXIT:
		SEQ_SetNext( p_seqwk, SEQFUNC_End );
		break;

	default:
		GF_ASSERT(0);
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	開始選択
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_StartSelect( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{	
  enum
  { 
    SEQ_INIT,
    SEQ_TOP_SELECT,
    SEQ_YESNO_SELECT,
    SEQ_NET_INIT,
    SEQ_NET_WAIT,
    SEQ_NET_SELECT_INIT,
    SEQ_NET_SELECT_WAIT,
    SEQ_NET_YESNO_INIT,
    SEQ_NET_YESNO_WAIT,
    SEQ_NET_EXIT,
    SEQ_NET_EXIT_WAIT,
    SEQ_NET_EXIT_RET,
    SEQ_NET_EXIT_RET_WAIT,
  };

  MYSTERY_WORK  *p_wk     = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    //戻ってきた時のためにメニュー構築
    if( p_wk->p_menu == NULL )
    { 
       UTIL_CreateMenu( p_wk, UTIL_MENU_TYPE_TOP, HEAPID_MYSTERYGIFT ); 
       MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_001, p_wk->p_font );
    }
    *p_seq  = SEQ_TOP_SELECT;
    break;

  case SEQ_TOP_SELECT:
    //おくりものをうけとるorカードアルバムをみるorやめる
    { 
      u32 ret;
      ret = MYSTERY_MENU_Main( p_wk->p_menu ); 
      if( ret != MYSTERY_MENU_SELECT_NULL )
      { 
        if( ret == 0 )
        { 
          MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_002, p_wk->p_font );
          UTIL_CreateList( p_wk, UTIL_LIST_TYPE_YESNO, HEAPID_MYSTERYGIFT );
          *p_seq  = SEQ_YESNO_SELECT;
        }
        else if( ret == 1 )
        { 
          UTIL_DeleteMenu( p_wk );
          SEQ_SetNext( p_seqwk, SEQFUNC_CardAlbum );
        }
        else if( ret == 2 )
        {
          SEQ_SetNext( p_seqwk, SEQFUNC_FadeOut );
        }
      }
    }
    break;
    
  case SEQ_YESNO_SELECT:
    //つうしんを開始しますよろしいですか
    { 
      u32 ret;
      ret = MYSTERY_LIST_Main( p_wk->p_list );
      if( ret != MYSTERY_LIST_SELECT_NULL )
      { 
        UTIL_DeleteList( p_wk );
        if( ret == 0 )
        { 
          //はい
          *p_seq  = SEQ_NET_INIT;
        }
        else if( ret == 1 )
        { 
          //いいえ
          MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_001, p_wk->p_font );
          *p_seq  = SEQ_INIT;
        }
      }
    }
    break;
  
  case SEQ_NET_INIT:
    MYSTERY_NET_ChangeStateReq( p_wk->p_net, MYSTERY_NET_STATE_START_BEACON );
    *p_seq  = SEQ_NET_WAIT;
    break;

  case SEQ_NET_WAIT:
    if( MYSTERY_NET_GetState( p_wk->p_net)  == MYSTERY_NET_STATE_MAIN_BEACON )
    {
      *p_seq  = SEQ_NET_SELECT_INIT;
    }
    break;

  case SEQ_NET_SELECT_INIT:
    //ワイヤレスorWIFIor赤外線

    //再構築
    UTIL_DeleteMenu( p_wk );
    UTIL_CreateMenu( p_wk, UTIL_MENU_TYPE_NETMODE, HEAPID_MYSTERYGIFT );
    //表示変更
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_003, p_wk->p_font );
    *p_seq  = SEQ_NET_SELECT_WAIT;
    break;

  case SEQ_NET_SELECT_WAIT:
    { 
      int i;
      u32 ret;
      GAME_COMM_STATUS_BIT  bit;

      bit = MYSTERY_NET_GetCommStatus( p_wk->p_net );
      if( bit & (GAME_COMM_STATUS_BIT_WIRELESS|GAME_COMM_STATUS_BIT_WIRELESS_TR|GAME_COMM_STATUS_BIT_WIRELESS_UN|GAME_COMM_STATUS_BIT_WIRELESS_FU) )
      { 
        MYSTERY_MENU_BlinkMain( p_wk->p_menu, 0 );
      }
      if( bit & (GAME_COMM_STATUS_BIT_WIFI|GAME_COMM_STATUS_BIT_WIFI_ZONE|GAME_COMM_STATUS_BIT_WIFI_FREE|GAME_COMM_STATUS_BIT_WIFI_LOCK) )
      { 
        MYSTERY_MENU_BlinkMain( p_wk->p_menu, 1 );
      }
      if( bit & GAME_COMM_STATUS_BIT_IRC )
      { 
        MYSTERY_MENU_BlinkMain( p_wk->p_menu, 2 );
      }

      //選択
      ret = MYSTERY_MENU_Main( p_wk->p_menu ); 
      if( ret != MYSTERY_MENU_SELECT_NULL )
      { 
        UTIL_DeleteMenu( p_wk );

        switch( ret )
        { 
        case 0: //ワイヤレス
          p_wk->mode  = MYSTERY_NET_MODE_WIRELESS;
          *p_seq  = SEQ_NET_YESNO_INIT;
          break;
        case 1: //Wi-Fi
          p_wk->mode  = MYSTERY_NET_MODE_WIFI;
          *p_seq  = SEQ_NET_YESNO_INIT;
          break;
        case 2: //赤外線
          p_wk->mode  = MYSTERY_NET_MODE_IRC;
          *p_seq  = SEQ_NET_YESNO_INIT;
          break;
        case 3: //もどる
          *p_seq  = SEQ_NET_EXIT_RET;
          break;
        }
      }
    }
    break;

  case SEQ_NET_YESNO_INIT:
    //ワイヤレスorWIFIor赤外線でせつぞくします
    switch( p_wk->mode )
    {
    case MYSTERY_NET_MODE_WIRELESS:
      MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_007, p_wk->p_font );
      break;

    case MYSTERY_NET_MODE_WIFI:
      *p_seq  = SEQ_NET_EXIT;
      return ;

    case MYSTERY_NET_MODE_IRC:
      MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_009, p_wk->p_font );
      break;
    }
    UTIL_CreateList( p_wk, UTIL_LIST_TYPE_YESNO, HEAPID_MYSTERYGIFT );
    *p_seq  = SEQ_NET_YESNO_WAIT;
    break;

  case SEQ_NET_YESNO_WAIT:
    { 
      u32 ret;
      ret = MYSTERY_LIST_Main( p_wk->p_list );
      if( ret != MYSTERY_LIST_SELECT_NULL )
      { 
        UTIL_DeleteList( p_wk );
        if( ret == 0 )
        { 
          *p_seq  =  SEQ_NET_EXIT;
        }
        else if( ret == 1 )
        { 
          //いいえ
          *p_seq  = SEQ_NET_SELECT_INIT;
        }
      }
    }
    break;

  //切断して先へ
  case SEQ_NET_EXIT:
    if( MYSTERY_NET_GetState( p_wk->p_net)  == MYSTERY_NET_STATE_MAIN_BEACON )
    { 
      MYSTERY_NET_ChangeStateReq( p_wk->p_net, MYSTERY_NET_STATE_END_BEACON );
      *p_seq  = SEQ_NET_EXIT_WAIT;
    }
    break;

  case SEQ_NET_EXIT_WAIT:
    if( MYSTERY_NET_GetState( p_wk->p_net)  == MYSTERY_NET_STATE_WAIT )
    {
      //はい
      if( p_wk->mode == MYSTERY_NET_MODE_WIFI )
      { 
        SEQ_SetNext( p_seqwk, SEQFUNC_WifiLogin );
      }
      else
      { 
        SEQ_SetNext( p_seqwk, SEQFUNC_RecvGift );
      }
    }
    break;

  //切断して戻る
  case SEQ_NET_EXIT_RET:
    if( MYSTERY_NET_GetState( p_wk->p_net)  == MYSTERY_NET_STATE_MAIN_BEACON )
    { 
      MYSTERY_NET_ChangeStateReq( p_wk->p_net, MYSTERY_NET_STATE_END_BEACON );
      *p_seq  = SEQ_NET_EXIT_RET_WAIT;
    }
    else
    { 
      *p_seq  = SEQ_INIT;
    }
    break;

  case SEQ_NET_EXIT_RET_WAIT:
    if( MYSTERY_NET_GetState( p_wk->p_net)  == MYSTERY_NET_STATE_WAIT )
    {
      *p_seq  = SEQ_INIT;
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_RecvGift( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_SEARCH,
    SEQ_NO_GIFT_INIT,
    SEQ_NO_GIFT_WAIT,
    SEQ_SELECT_GIFT_INIT,
    SEQ_SELECT_GIFT_WAIT,
    SEQ_GIFT_YESNO,
    SEQ_CANCEL_GIFT,
  };

  MYSTERY_WORK  *p_wk     = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    if( p_wk->mode  == MYSTERY_NET_MODE_WIFI )
    { 
      MYSTERY_NET_ChangeStateReq( p_wk->p_net, MYSTERY_NET_STATE_WIFI_DOWNLOAD );
    }

    //さがしています
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_010, p_wk->p_font );
    *p_seq  = SEQ_SEARCH;
    break;

  case SEQ_SEARCH:

    if( p_wk->mode  == MYSTERY_NET_MODE_WIFI )
    { 
      if( MYSTERY_NET_GetState( p_wk->p_net)  == MYSTERY_NET_STATE_WAIT )
      {
        BOOL ret;
        ret = MYSTERY_NET_GetDownloadData( p_wk->p_net, &p_wk->data, sizeof(DOWNLOAD_GIFT_DATA) );
        if( ret )
        { 
          static char title_buff[GIFT_DATA_CARD_TITLE_MAX];
          static char text_buff[GIFT_DATA_CARD_TEXT_MAX];
          DEB_STR_CONV_StrcodeToSjis( p_wk->data.data.event_name, title_buff, GIFT_DATA_CARD_TITLE_MAX );
          DEB_STR_CONV_StrcodeToSjis( p_wk->data.event_text, text_buff, GIFT_DATA_CARD_TEXT_MAX );

          NAGI_Printf( "取得しました\n" );
          NAGI_Printf( "TITLE: %s\n", title_buff );
          NAGI_Printf( "TEXT : %s\n", text_buff );
          NAGI_Printf( "EV_ID: %d\n", p_wk->data.data.event_id );
          NAGI_Printf( "TYPE : %d\n",   p_wk->data.data.gift_type );

          *p_seq  = SEQ_SELECT_GIFT_INIT;
        }
        else
        { 
          NAGI_Printf( "取得できなかった\n" );
          *p_seq = SEQ_NO_GIFT_INIT;
        }
     
      }
    }
    else
    { 
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
      { 
        //見つかった  本来Aボタンじゃない
        *p_seq  = SEQ_SELECT_GIFT_INIT;
      }
    }

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL )
    { 
      if( p_wk->mode  == MYSTERY_NET_MODE_WIFI )
      { 
        MYSTERY_NET_ChangeStateReq( p_wk->p_net, MYSTERY_NET_STATE_CANCEL_WIFI_DOWNLOAD );
      }

      //Bキャンセルorタイムアウト
      *p_seq = SEQ_NO_GIFT_INIT;
    }
    break;

  case SEQ_NO_GIFT_INIT:
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_err_003, p_wk->p_font );
    *p_seq  = SEQ_NO_GIFT_WAIT;
    break;

  case SEQ_NO_GIFT_WAIT:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
    { 
      SEQ_SetNext( p_seqwk, SEQFUNC_StartSelect );
    }
    break;

  case SEQ_SELECT_GIFT_INIT:
    MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_011, p_wk->p_font );
    UTIL_CreateMenu( p_wk, UTIL_MENU_TYPE_GIFT, HEAPID_MYSTERYGIFT );
    UTIL_CreateGuideText( p_wk, HEAPID_MYSTERYGIFT );
    *p_seq  = SEQ_SELECT_GIFT_WAIT;
    break;

  case SEQ_SELECT_GIFT_WAIT:
    //うけとるおくりものをえらんでください
    { 
      u32 ret;
      ret = MYSTERY_MENU_Main( p_wk->p_menu ); 
      if( ret != MYSTERY_MENU_SELECT_NULL )
      { 
        if( ret == 0 )
        { 
          UTIL_CreateList( p_wk, UTIL_LIST_TYPE_YESNO, HEAPID_MYSTERYGIFT );
          MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_013, p_wk->p_font );
          *p_seq  = SEQ_GIFT_YESNO;
        }
      }
      else  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL )
      { 
        UTIL_CreateList( p_wk, UTIL_LIST_TYPE_YESNO, HEAPID_MYSTERYGIFT );
        MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_012, p_wk->p_font );
        *p_seq  = SEQ_CANCEL_GIFT;
      }
    }
    break;

  case SEQ_GIFT_YESNO:
    //おくりものをじゅしんしますか
    { 
      u32 ret;
      ret = MYSTERY_LIST_Main( p_wk->p_list );
      if( ret != MYSTERY_LIST_SELECT_NULL )
      { 
        UTIL_DeleteList( p_wk );
        if( ret == 0 )
        { 
          //はい
          UTIL_DeleteMenu(p_wk);
          SEQ_SetNext( p_seqwk, SEQFUNC_Demo );
        }
        else if( ret == 1 )
        { 
          //いいえ
          *p_seq  = SEQ_SELECT_GIFT_INIT;
        }
      }
    }
    break;

  case SEQ_CANCEL_GIFT:
    //おくりもののうけとりをやめますか
    { 
      u32 ret;
      ret = MYSTERY_LIST_Main( p_wk->p_list );
      if( ret != MYSTERY_LIST_SELECT_NULL )
      { 
        UTIL_DeleteList( p_wk );
        if( ret == 0 )
        { 
          //はい
          UTIL_DeleteMenu(p_wk);
          UTIL_DeleteGuideText( p_wk );
          SEQ_SetNext( p_seqwk, SEQFUNC_StartSelect );
        }
        else if( ret == 1 )
        { 
          //いいえ
          *p_seq  = SEQ_SELECT_GIFT_INIT;
        }
      }
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	デモ
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Demo( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_RECV,
    SEQ_MSG_WAIT,
    SEQ_CARD_INIT,
    SEQ_CARD_WAIT,
    SEQ_END,
  };

  MYSTERY_WORK  *p_wk     = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    //おくりものを受信中です
    { 
      GFL_CLUNIT	*p_unit	= MYSTERY_GRAPHIC_GetClunit( p_wk->p_graphic );
      MYSTERY_DEMO_Init( &p_wk->demo, p_unit, &p_wk->data, HEAPID_MYSTERYGIFT );
      MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_014, p_wk->p_font );
      *p_seq  = SEQ_RECV;
    }
    break;

  case SEQ_RECV:
    MYSTERY_DEMO_Main( &p_wk->demo );
    if( MYSTERY_DEMO_IsEnd(&p_wk->demo) )
    { 
      MYSTERY_TEXT_Print( p_wk->p_text, p_wk->p_msg, syachi_mystery_01_015, p_wk->p_font );
      *p_seq  = SEQ_MSG_WAIT;
    }
    break;

  case SEQ_MSG_WAIT:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
    { 
      *p_seq  = SEQ_CARD_INIT;
    }
    break;

  case SEQ_CARD_INIT:
    { 
      MYSTERY_DEMO_Exit( &p_wk->demo );
      MYSTERY_TEXT_Exit( p_wk->p_text );
      p_wk->p_text  = NULL;
      UTIL_DeleteMenu( p_wk );

      GFL_BG_SetVisible( BG_FRAME_M_TEXT, FALSE );
      { 
        MYSTERY_CARD_SETUP setup;
        GFL_STD_MemClear( &setup, sizeof(MYSTERY_CARD_SETUP) );

        setup.p_data   = &p_wk->data.data;
        setup.back_frm = BG_FRAME_M_BACK1;
        setup.font_frm = BG_FRAME_M_LIST;
        setup.back_plt_num  = PLT_BG_CARD_M;
        setup.font_plt_num  = PLT_BG_FONT_M;
        setup.icon_obj_plt_num  = PLT_OBJ_CARD_ICON_M; 
        setup.silhouette_obj_plt_num  = PLT_OBJ_CARD_SILHOUETTE_M; 
        setup.p_clunit  = MYSTERY_GRAPHIC_GetClunit( p_wk->p_graphic );
        setup.p_sv      = p_wk->p_sv;
        setup.p_msg     = p_wk->p_msg; 
        setup.p_font    = p_wk->p_font; 
        setup.p_que     = p_wk->p_que; 
        setup.p_word    = p_wk->p_word;
        p_wk->p_card  = MYSTERY_CARD_Init( &setup, HEAPID_MYSTERYGIFT );
      }
    }
    *p_seq  = SEQ_CARD_WAIT;
    break;

  case SEQ_CARD_WAIT:
    MYSTERY_CARD_Main( p_wk->p_card );
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
    { 
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_END:
    SEQ_SetNext( p_seqwk, SEQFUNC_DisConnect );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	カードアルバム
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_CardAlbum( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  MYSTERY_WORK  *p_wk     = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      UTIL_DeleteMenu( p_wk );
      MYSTERY_TEXT_Exit( p_wk->p_text );
      p_wk->p_text  = NULL;
    }
    {
      MYSTERY_ALBUM_SETUP setup;
      GFL_STD_MemClear( &setup, sizeof(MYSTERY_LIST_SETUP) );
      setup.mode  = MYSTERY_ALBUM_MODE_VIEW;
      setup.p_clunit  = MYSTERY_GRAPHIC_GetClunit( p_wk->p_graphic );
      setup.p_sv      = p_wk->p_sv;
      setup.p_font    = p_wk->p_font;
      setup.p_que     = p_wk->p_que;
      setup.p_word    = p_wk->p_word;
      setup.p_msg     = p_wk->p_msg;
      p_wk->p_album = MYSTERY_ALBUM_Init( &setup, HEAPID_MYSTERYGIFT );
      *p_seq  = SEQ_MAIN;
    }
    break;
  case SEQ_MAIN:
    MYSTERY_ALBUM_Main( p_wk->p_album );
    if( MYSTERY_ALBUM_IsEnd( p_wk->p_album ) )
    { 
      *p_seq  = SEQ_EXIT;
    }
    break;
  case SEQ_EXIT:
    MYSTERY_ALBUM_Exit( p_wk->p_album );
    { 
      ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_MYSTERY, HEAPID_MYSTERYGIFT );
      //PLT
      GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_mystery_fusigi_bg_00_NCLR,
          PALTYPE_MAIN_BG, PLT_BG_BACK_M*0x20, 0x20, HEAPID_MYSTERYGIFT );
      GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_mystery_fushigi_back_NCLR,
          PALTYPE_MAIN_BG, PLT_BG_RECV_M*0x20, 0x20*6, HEAPID_MYSTERYGIFT );
      //CHR
      GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fusigi_bg_00_NCGR, 
          BG_FRAME_M_BACK2, 0, 0, FALSE, HEAPID_MYSTERYGIFT );
      //SCR
      GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mystery_fusigi_bg_00_NSCR,
          BG_FRAME_M_BACK2, 0, 0, FALSE, HEAPID_MYSTERYGIFT );
      GFL_ARC_CloseDataHandle( p_handle );
    }


    SEQ_SetNext( p_seqwk, SEQFUNC_StartSelect );
    p_wk->p_text  = MYSTERY_TEXT_Init( BG_FRAME_M_TEXT, PLT_BG_FONT_M, PLT_BG_TEXT_M, BG_CGX_OFS_M_TEXT, p_wk->p_que, HEAPID_MYSTERYGIFT );
    break;
  }
}

//----------------------------------------------------------------------------
/** 
 *	@brief  WiFiLogin画面の接続
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_WifiLogin( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
    SEQ_DELETE,
    SEQ_PROC_CALL,
    SEQ_PROC_WAIT,
    SEQ_CREATE,
    SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
    SEQ_END,
  };

  MYSTERY_WORK  *p_wk     = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
		*p_seq	= SEQ_FADEIN_WAIT;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_DELETE;
		}
		break;

  case SEQ_DELETE:
    if( p_wk->p_text )
    { 
      MYSTERY_TEXT_Exit( p_wk->p_text );
    }

    //リソース破棄
    OBJ_Exit( &p_wk->obj );
    BG_Exit( &p_wk->bg );

    //グラフィック破棄
    MYSTERY_GRAPHIC_Exit( p_wk->p_graphic );

    *p_seq  = SEQ_PROC_CALL;
    break;

  case SEQ_PROC_CALL:
    p_wk->p_wifilogin_param = GFL_HEAP_AllocMemory( HEAPID_MYSTERYGIFT, sizeof(WIFILOGIN_PARAM) );
    GFL_STD_MemClear( p_wk->p_wifilogin_param, sizeof(WIFILOGIN_PARAM) );
    p_wk->p_wifilogin_param->gamedata     = GAMEDATA_Create(HEAPID_MYSTERYGIFT);
    p_wk->p_wifilogin_param->bDreamWorld  = FALSE;

    GFL_PROC_SysCallProc( FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, p_wk->p_wifilogin_param );
    *p_seq  = SEQ_CREATE;
    break;

  case SEQ_CREATE:
    //グラフィック設定
    p_wk->p_graphic	= MYSTERY_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_MYSTERYGIFT );

    //リソース読み込み
    BG_Init( &p_wk->bg, HEAPID_MYSTERYGIFT );
    {	
      GFL_CLUNIT	*p_clunit	= MYSTERY_GRAPHIC_GetClunit( p_wk->p_graphic );
      OBJ_Init( &p_wk->obj, p_clunit, HEAPID_MYSTERYGIFT );
    }

    p_wk->p_text  = MYSTERY_TEXT_Init( BG_FRAME_M_TEXT, PLT_BG_FONT_M, PLT_BG_TEXT_M, BG_CGX_OFS_M_TEXT, p_wk->p_que, HEAPID_MYSTERYGIFT );

    *p_seq  = SEQ_PROC_WAIT;
    break;

  case SEQ_PROC_WAIT:
    *p_seq  = SEQ_FADEOUT_START;
    break;

  case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_END;
		}
		break;

  case SEQ_END:
    if( p_wk->p_wifilogin_param->result == WIFILOGIN_RESULT_LOGIN )
    { 
      //次へ
      SEQ_SetNext( p_seqwk, SEQFUNC_RecvGift );
    }
    else
    { 
      //キャンセル
      SEQ_SetNext( p_seqwk, SEQFUNC_StartSelect );
    }
    GAMEDATA_Delete( p_wk->p_wifilogin_param->gamedata );
    GFL_HEAP_FreeMemory( p_wk->p_wifilogin_param );
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	切断処理
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_DisConnect( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_WAIT,
    SEQ_END,
  };
  MYSTERY_WORK  *p_wk     = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    switch( p_wk->mode )
    { 
    case MYSTERY_NET_MODE_WIRELESS:
      break;

    case MYSTERY_NET_MODE_WIFI:
      MYSTERY_NET_ChangeStateReq( p_wk->p_net, MYSTERY_NET_STATE_LOGOUT_WIFI );
      break;

    case MYSTERY_NET_MODE_IRC:
      break;
    }
    *p_seq  = SEQ_WAIT;
    break;

  case SEQ_WAIT:
    if( MYSTERY_NET_GetState( p_wk->p_net ) == MYSTERY_NET_STATE_WAIT )
    { 
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_END:
    SEQ_SetNext( p_seqwk, SEQFUNC_FadeOut );
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief	終了処理
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  MYSTERY_WORK  *p_wk     = p_wk_adrs;

  UTIL_DeleteList( p_wk );
  UTIL_DeleteMenu( p_wk );
  UTIL_DeleteGuideText( p_wk );

  if( p_wk->p_card )
  { 
    MYSTERY_CARD_Exit( p_wk->p_card );
  }

  //終了
  SEQ_End( p_seqwk );
}

//=============================================================================
/**
 *      UTIL
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  リスト作成
 *
 *	@param	MYSTERY_WORK *p_wk    ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void UTIL_CreateList( MYSTERY_WORK *p_wk, UTIL_LIST_TYPE type, HEAPID heapID )
{ 
  if( p_wk->p_list == NULL )
  { 
    MYSTERY_LIST_SETUP setup;
    GFL_STD_MemClear( &setup, sizeof(MYSTERY_LIST_SETUP) );
    setup.p_msg   = p_wk->p_msg;
    setup.p_font  = p_wk->p_font;
    setup.p_que   = p_wk->p_que;
    setup.strID[0]= syachi_mystery_01_002_yes;
    setup.strID[1]= syachi_mystery_01_002_no;
    setup.list_max= 2;
    setup.frm     = BG_FRAME_M_LIST;
    setup.font_plt= PLT_BG_FONT_M;
    setup.frm_plt = PLT_BG_TEXT_M;
    setup.frm_chr = BG_CGX_OFS_M_LIST;
    p_wk->p_list  = MYSTERY_LIST_Init( &setup, heapID ); 
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト破棄
 *
 *	@param	MYSTERY_WORK *p_wk    ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_DeleteList( MYSTERY_WORK *p_wk )
{ 
  if( p_wk->p_list )
  { 
    MYSTERY_LIST_Exit( p_wk->p_list );
    p_wk->p_list  = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  メニュー作成
 *
 *	@param	MYSTERY_WORK *p_wk    ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void UTIL_CreateMenu( MYSTERY_WORK *p_wk, UTIL_MENU_TYPE type, HEAPID heapID )
{ 
 //メニュー作成
  if( p_wk->p_menu == NULL )
  { 
    MYSTERY_MENU_SETUP setup;
    GFL_STD_MemClear( &setup, sizeof(MYSTERY_MENU_SETUP) );
    setup.p_font  = p_wk->p_font;
    setup.p_que   = p_wk->p_que;
    setup.p_cursor= OBJ_GetClwk( &p_wk->obj, OBJ_CLWKID_CURSOR );
    setup.frm     = BG_FRAME_M_TEXT;
    setup.font_plt= PLT_BG_FONT_M;
    setup.frm_plt = PLT_BG_TEXT_M;
    setup.frm_chr = BG_CGX_OFS_M_TEXT;

    switch( type )
    { 
    case UTIL_MENU_TYPE_TOP:
      setup.p_msg   = p_wk->p_msg;
      setup.strID[0]= syachi_mystery_menu_001;
      setup.strID[1]= syachi_mystery_menu_002;
      setup.strID[2]= syachi_mystery_menu_003;
      setup.list_max= 3;
      break;

    case UTIL_MENU_TYPE_NETMODE:
      setup.p_msg   = p_wk->p_msg;
      setup.strID[0]= syachi_mystery_menu_004;
      setup.strID[1]= syachi_mystery_menu_005;
      setup.strID[2]= syachi_mystery_menu_006;
      setup.strID[3]= syachi_mystery_menu_007;
      setup.list_max= 4;
      break;
      
    case UTIL_MENU_TYPE_GIFT:
      setup.p_msg   = NULL;
      setup.p_strbuf[0] = GFL_STR_CreateBuffer( GIFT_DATA_CARD_TITLE_MAX+1, heapID );
      GFL_STR_SetStringCodeOrderLength( setup.p_strbuf[0], p_wk->data.data.event_name, GIFT_DATA_CARD_TITLE_MAX );
      setup.list_max    = 1;
      break;
    }

    //メニュー作成
    p_wk->p_menu  = MYSTERY_MENU_Init( &setup, heapID );

    //メニューへ文字列バッファを渡していたら開放
    if( setup.p_msg == NULL )
    { 
      int i;
      for( i  = 0; i < MYSTERY_MENU_WINDOW_MAX; i++ )
      {
        if( setup.p_strbuf[i] )
        { 
          GFL_STR_DeleteBuffer( setup.p_strbuf[i] );
        }
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  メニュー破棄
 *
 *	@param	MYSTERY_WORK *p_wk    ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void UTIL_DeleteMenu( MYSTERY_WORK *p_wk )
{
  if(p_wk->p_menu)
  { 
    MYSTERY_MENU_Exit( p_wk->p_menu );
    p_wk->p_menu  = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ガイド用テキスト文  作成
 *
 *	@param	MYSTERY_WORK *p_wk
 *	@param	heapID 
 */
//-----------------------------------------------------------------------------
static void UTIL_CreateGuideText( MYSTERY_WORK *p_wk, HEAPID heapID )
{ 
  if( p_wk->p_winset_s == NULL )
  { 
    MYSTERY_MSGWINSET_SETUP_TBL tbl[] =
    { 
      //タイトル
      { 
        3,2,28,2,
      },
      //本文
      {
        2,5,28,18,
      },
    };
    tbl[0].p_strbuf = GFL_STR_CreateBuffer( GIFT_DATA_CARD_TITLE_MAX+1, heapID );
    GFL_STR_SetStringCodeOrderLength( tbl[0].p_strbuf, p_wk->data.data.event_name, GIFT_DATA_CARD_TITLE_MAX ); 

    tbl[1].p_strbuf = GFL_STR_CreateBuffer( GIFT_DATA_CARD_TEXT_MAX+1, heapID );
    GFL_STR_SetStringCodeOrderLength( tbl[1].p_strbuf, p_wk->data.event_text, GIFT_DATA_CARD_TEXT_MAX ); 

    p_wk->p_winset_s  = MYSTERY_MSGWINSET_Init( tbl, NELEMS(tbl), BG_FRAME_S_TEXT, PLT_BG_FONT_S, p_wk->p_que, p_wk->p_msg, p_wk->p_font, heapID );

    GFL_STR_DeleteBuffer( tbl[0].p_strbuf );
    GFL_STR_DeleteBuffer( tbl[1].p_strbuf );

    {	
      ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );
      GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fusigi_card_NCGR, 
          BG_FRAME_S_BACK1, 0, 0, FALSE, heapID );
      GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mystery_fusigi_card02_NSCR,
          BG_FRAME_S_BACK1, 0, 0, FALSE, heapID );
      GFL_ARC_CloseDataHandle( p_handle );
    }

    GFL_BG_ChangeScreenPalette( BG_FRAME_S_BACK1, 0, 0, 32, 23, PLT_BG_CARD_S );
    GFL_BG_LoadScreenReq( BG_FRAME_S_BACK1 );

    GFL_BG_SetVisible( BG_FRAME_S_BACK1, TRUE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ガイド用テキスト文  破棄
 *
 *	@param	MYSTERY_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_DeleteGuideText( MYSTERY_WORK *p_wk )
{ 
  if( p_wk->p_winset_s )
  { 
   MYSTERY_MSGWINSET_Exit( p_wk->p_winset_s );
   p_wk->p_winset_s = NULL;

   GFL_BG_SetVisible( BG_FRAME_S_BACK1, FALSE );
  }
}
//=============================================================================
/**
 *      デモ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  受信デモ  初期化
 *
 *	@param	MYSTERY_DEMO_WORK *p_wk ワーク
 *	@param	*p_unit                 CLUNIT
 *	@param  cp_data                 データ
 *	@param	heapID                  ヒープID
 */
//-----------------------------------------------------------------------------
static void MYSTERY_DEMO_Init( MYSTERY_DEMO_WORK *p_wk, GFL_CLUNIT *p_unit, const DOWNLOAD_GIFT_DATA *cp_data, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(MYSTERY_DEMO_WORK) );

  //BG設定
  { 
    ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mystery_fushigi_back_NCGR, 
				BG_FRAME_M_BACK1, 0, 0, FALSE, heapID );
    GFL_ARC_UTIL_TransVramScreen( ARCID_MYSTERY, NARC_mystery_fushigi_back_NSCR,
        BG_FRAME_M_BACK1, 0, 0, FALSE, heapID );
    GFL_ARC_CloseDataHandle( p_handle );

    GFL_BG_ChangeScreenPalette( BG_FRAME_M_BACK1, 0, 0, 32, 24, PLT_BG_RECV_M );

    GFL_BG_LoadScreenReq( BG_FRAME_M_BACK1 );
    GFL_BG_SetVisible( BG_FRAME_M_BACK1, TRUE );
  } 

	//リソース読みこみ
  switch( cp_data->data.gift_type )
  { 
  case MYSTERYGIFT_TYPE_POKEMON:
    {	
      const GIFT_PRESENT_POKEMON  *cp_pokemon = &cp_data->data.data.pokemon;
      ARCHANDLE		*p_handle	= POKE2DGRA_OpenHandle( heapID );

      p_wk->res_plt	= POKE2DGRA_OBJ_PLTT_Register( p_handle, cp_pokemon->mons_no, cp_pokemon->form_no, cp_pokemon->sex, cp_pokemon->rare == 2, POKEGRA_DIR_FRONT, cp_pokemon->egg, CLSYS_DRAW_MAIN, PLT_OBJ_GIFT_M * 0x20, heapID );
 
      p_wk->res_cel	= POKE2DGRA_OBJ_CELLANM_Register( cp_pokemon->mons_no, cp_pokemon->form_no, cp_pokemon->sex, cp_pokemon->rare == 1, POKEGRA_DIR_FRONT, cp_pokemon->egg, APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, heapID );
        
      p_wk->res_cgx	= POKE2DGRA_OBJ_CGR_Register( p_handle, cp_pokemon->mons_no, cp_pokemon->form_no, cp_pokemon->sex, cp_pokemon->rare == 1, POKEGRA_DIR_FRONT, cp_pokemon->egg, CLSYS_DRAW_MAIN, heapID );
 
      GFL_ARC_CloseDataHandle( p_handle );
    }
    break;

  case MYSTERYGIFT_TYPE_ITEM:
    {	
      const GIFT_PRESENT_ITEM  *cp_item = &cp_data->data.data.item;
      ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_ITEMICON, heapID );

      p_wk->res_plt	= GFL_CLGRP_PLTT_Register( p_handle, 
          ITEM_GetIndex( cp_item->itemNo, ITEM_GET_ICON_PAL ), CLSYS_DRAW_MAIN, PLT_OBJ_GIFT_M*0x20, heapID );

      p_wk->res_cel	= GFL_CLGRP_CELLANIM_Register( p_handle,
          NARC_item_icon_itemicon_NCER, NARC_item_icon_itemicon_NANR, heapID );

      p_wk->res_cgx	= GFL_CLGRP_CGR_Register( p_handle,
          ITEM_GetIndex( cp_item->itemNo, ITEM_GET_ICON_CGX ), FALSE, CLSYS_DRAW_MAIN, heapID );

      GFL_ARC_CloseDataHandle( p_handle );
    }
    break;

  case MYSTERYGIFT_TYPE_RULE:
    /* fallthrow */
  case MYSTERYGIFT_TYPE_NUTSET:
    {	
      ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_MYSTERY, heapID );

      p_wk->res_plt	= GFL_CLGRP_PLTT_Register( p_handle, 
          NARC_mystery_fushigi_box_NCLR, CLSYS_DRAW_MAIN, PLT_OBJ_GIFT_M*0x20, heapID );

      p_wk->res_cel	= GFL_CLGRP_CELLANIM_Register( p_handle,
          NARC_mystery_fushigi_box_NCER, NARC_mystery_fushigi_box_NANR, heapID );

      p_wk->res_cgx	= GFL_CLGRP_CGR_Register( p_handle,
          NARC_mystery_fushigi_box_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

      GFL_ARC_CloseDataHandle( p_handle );
    }
    break;

  default:
    GF_ASSERT( 0 );
  }

	//CLWK登録
	{
		int i;
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
    cldata.pos_x  = MYSTERY_DEMO_GIFT_X;
    cldata.pos_y  = MYSTERY_DEMO_MOVE_GIFT_START_Y;
    cldata.bgpri  = BG_FRAME_M_BACK1;

		p_wk->p_clwk	=	GFL_CLACT_WK_Create( p_unit,
				p_wk->res_cgx,
				p_wk->res_plt,
				p_wk->res_cel,
				&cldata,
				CLSYS_DEFREND_MAIN,
				heapID );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信デモ  破棄
 *
 *	@param	MYSTERY_DEMO_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MYSTERY_DEMO_Exit( MYSTERY_DEMO_WORK *p_wk )
{ 

  GFL_BG_SetVisible( BG_FRAME_M_BACK1, FALSE ); 

	//CLWK破棄
	{	
    GFL_CLACT_WK_Remove( p_wk->p_clwk );
	}

	//リソース破棄
	{
    GFL_CLGRP_PLTT_Release( p_wk->res_plt );
    GFL_CLGRP_CGR_Release( p_wk->res_cgx );
    GFL_CLGRP_CELLANIM_Release( p_wk->res_cel );
	}

  GFL_STD_MemClear( p_wk, sizeof(MYSTERY_DEMO_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信デモ  メイン処理
 *
 *	@param	MYSTERY_DEMO_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MYSTERY_DEMO_Main( MYSTERY_DEMO_WORK *p_wk )
{
  enum
  { 
    SEQ_INIT_WAIT,
    SEQ_MOVE,
    SEQ_END_WAIT,
    SEQ_END,
  };

  switch( p_wk->seq )
  {
  case SEQ_INIT_WAIT:
    if( p_wk->sync++ > MYSTERY_DEMO_INIT_WAIT_SYNC )
    { 
      p_wk->sync  = 0;
      p_wk->seq   = SEQ_MOVE;
    }
    break;

  case SEQ_MOVE:
    { 
      s16 pos;

      pos = MYSTERY_DEMO_MOVE_GIFT_START_Y + MYSTERY_DEMO_MOVE_GIFT_DIFF_Y * p_wk->sync / MYSTERY_DEMO_MOVE_GIFT_SYNC;
      GFL_CLACT_WK_SetTypePos( p_wk->p_clwk, pos, CLSYS_DEFREND_MAIN, CLSYS_MAT_Y );

      if( p_wk->sync++ >= MYSTERY_DEMO_MOVE_GIFT_SYNC )
      { 
        p_wk->sync  = 0;
        p_wk->seq   = SEQ_END_WAIT;
      }
    }
    break;

  case SEQ_END_WAIT:
    if( p_wk->sync++ > MYSTERY_DEMO_END_WAIT_SYNC )
    { 
      p_wk->sync  = 0;
      p_wk->seq   = SEQ_END;
    }
    break;

  case SEQ_END:
    p_wk->is_end  = TRUE;
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信デモ  終了チェック
 *
 *	@param	const MYSTERY_DEMO_WORK *cp_wk  ワーク
 *
 *	@return TRUEでデモ終了  FALSEでデモ中
 */
//-----------------------------------------------------------------------------
static BOOL MYSTERY_DEMO_IsEnd( const MYSTERY_DEMO_WORK *cp_wk )
{ 
  return cp_wk->is_end;
}

