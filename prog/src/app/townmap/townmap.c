//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		map_main.c
 *	@brief	タウンマップメイン
 *	@author	Toru=Nagihashi
 *	@data		2009.07.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//lib
#include <gflib.h>

//	constant
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"

//	archive
#include "arc_def.h"
#include "townmap_gra.naix"
#include "message.naix"
#include "font/font.naix"

//	common
#include "app/app_menu_common.h"

//	Module
#include "print/gf_font.h"
#include "print/printsys.h"
#include "field/zonedata.h"
#include "townmap_data_sys.h"

//mine
#include "townmap_grh.h"
#include "app/townmap.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	RULE
//=====================================
#define RULE_PLACE_MSG_MAX	(6)

//-------------------------------------
///	APPBAR
//=====================================
typedef enum
{	
	APPBAR_OPTION_MASK_CLOSE		= 1<<0,	//閉じる
	APPBAR_OPTION_MASK_RETURN		= 1<<1,	//戻る
	APPBAR_OPTION_MASK_CUR_D		= 1<<2,	//カーソル上
	APPBAR_OPTION_MASK_CUR_U		= 1<<3,	//カーソル下
	APPBAR_OPTION_MASK_CUR_L		= 1<<4,	//カーソル左
	APPBAR_OPTION_MASK_CUR_R		= 1<<5,	//カーソル右
	APPBAR_OPTION_MASK_CHECK		= 1<<6,	//チェックボックス
	APPBAR_OPTION_MASK_SCALE		= 1<<7,	//拡大縮小


	APPBAR_OPTION_MASK_SKY	= APPBAR_OPTION_MASK_CLOSE|APPBAR_OPTION_MASK_RETURN,	//空を飛ぶ画面のとき
	APPBAR_OPTION_MASK_TOWN	= APPBAR_OPTION_MASK_CUR_L|APPBAR_OPTION_MASK_CUR_R|
		APPBAR_OPTION_MASK_SCALE|
		APPBAR_OPTION_MASK_CLOSE|APPBAR_OPTION_MASK_RETURN,	//地図画面のとき
} APPBAR_OPTION_MASK;
//選択したもの取得
typedef enum
{
	APPBAR_SELECT_NONE	= GFL_UI_TP_HIT_NONE,
	APPBAR_SELECT_CLOSE		= 0,
	APPBAR_SELECT_RETURN,
	APPBAR_SELECT_CUR_D,
	APPBAR_SELECT_CUR_U,
	APPBAR_SELECT_CUR_L,
	APPBAR_SELECT_CUR_R,
	APPBAR_SELECT_CHECK,
	APPBAR_SELECT_SCALE_UP,
	APPBAR_SELECT_SCALE_DOWN,
} APPBAR_SELECT;
//リソース
enum
{	
	APPBAR_RES_COMMON_PLT,
	APPBAR_RES_COMMON_CHR,
	APPBAR_RES_COMMON_CEL,
	APPBAR_RES_SCALE_PLT,
	APPBAR_RES_SCALE_CHR,
	APPBAR_RES_SCALE_CEL,

	APPBAR_RES_MAX
};
//アイコン種類
enum
{	
	APPBAR_BARICON_CLOSE,
	APPBAR_BARICON_RETURN,
	APPBAR_BARICON_CUR_D,
	APPBAR_BARICON_CUR_U,
	APPBAR_BARICON_CUR_L,
	APPBAR_BARICON_CUR_R,
	APPBAR_BARICON_CHECK,
	APPBAR_BARICON_SCALE_UP,
	APPBAR_BARICON_SCALE_DOWN,
	APPBAR_BARICON_MAX,
};
//位置
//バー
#define APPBAR_MENUBAR_X	(0)
#define APPBAR_MENUBAR_Y	(21)
#define APPBAR_MENUBAR_W	(32)
#define APPBAR_MENUBAR_H	(3)
//CLOSE
#define APPBAR_ICON_Y	(168)
#define APPBAR_ICON_CUR_L_X				(16)
#define APPBAR_ICON_CUR_R_X				(38)
#define APPBAR_ICON_SCALE_DOWN_X	(144)
#define APPBAR_ICON_SCALE_UP_X		(168)
#define APPBAR_ICON_CLOSE_X				(200)
#define APPBAR_ICON_RETURN_X			(232)

#define APPBAR_ICON_W	(24)
#define APPBAR_ICON_H	(24)
//-------------------------------------
///	CURSOR
//=====================================
//CLWK
enum
{	
	CURSOR_CLWK_CURSOR,
	CURSOR_CLWK_RING,
	
	CURSOR_CLWK_MAX
};
//位置
#define CURSOR_POS_DEFAULT_X		(128)
#define CURSOR_POS_DEFAULT_Y		(96)
//速度
#define CURSOR_ADD_SPEED				(3)
//制限
#define CURSOR_MOVE_LIMIT_TOP			(0+8)
#define CURSOR_MOVE_LIMIT_BOTTOM	(192)
#define CURSOR_MOVE_LIMIT_LEFT		(0)
#define CURSOR_MOVE_LIMIT_RIGHT		(256-8)
//-------------------------------------
///		INFO
//=====================================
//文字面範囲
#define INFO_BMPWIN_X	(0)
#define INFO_BMPWIN_Y	(0)
#define INFO_BMPWIN_W	(32)
#define INFO_BMPWIN_H	(24)

//文字位置
#define INFO_STR_PLACE_CENTERING	//OFFにするとINFO_STR_PLACE_Xの定義を使い座標指定します
#define INFO_STR_PLACE_X	(16)

#define INFO_STR_PLACE_Y	(48)
#define INFO_STR_GUIDE_X	(32)
#define INFO_STR_GUIDE_Y	(80)
#define INFO_STR_PLACE1_X	(32)
#define INFO_STR_PLACE1_Y	(128)
#define INFO_STR_PLACE2_X	(32)
#define INFO_STR_PLACE2_Y	(144)
#define INFO_STR_PLACE3_X	(32)
#define INFO_STR_PLACE3_Y	(160)
#define INFO_STR_PLACE4_X	(144)
#define INFO_STR_PLACE4_Y	(128)
#define INFO_STR_PLACE5_X	(144)
#define INFO_STR_PLACE5_Y	(144)
#define INFO_STR_PLACE6_X	(144)
#define INFO_STR_PLACE6_Y	(160)

//-------------------------------------
///	MSGWND
//=====================================
#define MSGWND_BMPWIN_SKY_X	(0)
#define MSGWND_BMPWIN_SKY_Y	(21)
#define MSGWND_BMPWIN_SKY_W	(24)
#define MSGWND_BMPWIN_SKY_H	(3)

#define MSGWND_STR_SKY_X	(0)
#define MSGWND_STR_SKY_Y	(0)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	下画面バー(アプリケーションバーと勝手に命名)
//=====================================
typedef struct 
{
	GFL_CLWK	*p_clwk[APPBAR_BARICON_MAX];
	u32				res[APPBAR_RES_MAX];
	u32				bg_frm;
	GFL_ARCUTIL_TRANSINFO				chr_pos;
	s32				trg;
	s32				cont;
	u32				mode;
} APPBAR_WORK;
#define APPBAR_BG_CHARAAREA_SIZE	(8*8*GFL_BG_1CHRDATASIZ)
//-------------------------------------
///	カーソル処理
//=====================================
typedef struct 
{
	GFL_CLWK	*p_clwk[CURSOR_CLWK_MAX];
	GFL_CLACTPOS	pos;
} CURSOR_WORK;
//-------------------------------------
///	場所処理
//=====================================
typedef struct 
{
	GFL_CLWK						*p_clwk;
	const TOWNMAP_DATA	*cp_data;
	u32									data_idx;
} PLACE_DATA;
typedef struct
{
	GFL_CLUNIT			*p_clunit;	//場所はUNITを変えるので作成する
	GFL_CLSYS_REND	*p_rend;
	PLACE_DATA			*p_place;
	u32							data_num;
} PLACE_WORK;
//-------------------------------------
///	地図処理	
//=====================================
typedef struct 
{
	int dummy;	
} MAP_WORK;
//-------------------------------------
///	上画面情報
//=====================================
typedef struct
{
	GFL_BMPWIN	*p_bmpwin;
	STRBUF			*p_strbuf;			//汎用バッファ
	GFL_FONT		*p_font;				//受け取る
	const GFL_MSGDATA	*cp_place_msg;		//受け取り
	const GFL_MSGDATA	*cp_guide_msg;		//受け取り
} INFO_WORK;
//-------------------------------------
///	メッセージ表示ウィンドウ
//=====================================
typedef struct
{
	GFL_BMPWIN*				p_bmpwin;
	STRBUF*						p_strbuf;
	GFL_FONT*				  p_font;
  const GFL_MSGDATA*			cp_msg;
} MSGWND_WORK;
//-------------------------------------
///	Module操作
//=====================================
typedef struct 
{
	int dummy;	
} CONTROL_WORK;
//-------------------------------------
///	シーケンス管理
//=====================================
typedef struct _SEQ_WORK SEQ_WORK;
typedef void (*SEQ_FUNCTION)( SEQ_WORK *p_wk, int *p_seq, void *p_param_adrs );
struct _SEQ_WORK
{
	SEQ_FUNCTION	seq_function;
	BOOL is_end;
	int seq;
	void *p_param_adrs;
}; 
//-------------------------------------
///	メインワーク
//=====================================
typedef struct 
{
	//グラフィックシステム
	TOWNMAP_GRAPHIC_SYS	*p_grh;

	//アプリケーションバー
	APPBAR_WORK		appbar;

	//データ
	TOWNMAP_DATA	*p_data;

	//カーソル
	CURSOR_WORK		cursor;

	//場所
	PLACE_WORK		place;

	//地図
	MAP_WORK			map;

	//シーケンス
	SEQ_WORK			seq;

	//上画面情報
	INFO_WORK			info;

	//空を飛ぶ？のメッセージ面
	MSGWND_WORK		msgwnd;

	//共通利用システム
	GFL_FONT			*p_font;
	GFL_MSGDATA		*p_place_msg;
	GFL_MSGDATA		*p_guide_msg;

	//引数
	TOWNMAP_PARAM	*p_param;

	//その他
	const PLACE_DATA *cp_pre_data;
	
} TOWNMAP_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT TOWNMAP_PROC_Init(
		GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT TOWNMAP_PROC_Exit(
		GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT TOWNMAP_PROC_Main(
		GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
///	SEQ
//=====================================
static void SEQ_Init( SEQ_WORK *p_wk, void *p_param_adrs, SEQ_FUNCTION seq_function );
static void SEQ_Exit( SEQ_WORK *p_wk );
static void SEQ_Main( SEQ_WORK *p_wk );
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk );
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function );
static void SEQ_End( SEQ_WORK *p_wk );
//-------------------------------------
///	SEQFUNC
//=====================================
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs );
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs );
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs );
//-------------------------------------
///	APPBAR
//=====================================
static void APPBAR_Init( APPBAR_WORK *p_wk, APPBAR_OPTION_MASK mask, GFL_CLUNIT* p_unit, u8 bar_frm, u8 bg_plt, u8 obj_plt, HEAPID heapID );
static void APPBAR_Exit( APPBAR_WORK *p_wk );
static void APPBAR_Main( APPBAR_WORK *p_wk );
static APPBAR_SELECT APPBAR_GetTrg( const APPBAR_WORK *cp_wk );
static APPBAR_SELECT APPBAR_GetCont( const APPBAR_WORK *cp_wk );
static void ARCHDL_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h,  u8 plt, BOOL compressedFlag, HEAPID heapID );
//-------------------------------------
///	CURSOR
//=====================================
static void CURSOR_Init( CURSOR_WORK *p_wk, GFL_CLWK *p_cursor, GFL_CLWK *p_ring, HEAPID heapID );
static void CURSOR_Exit( CURSOR_WORK *p_wk );
static void CURSOR_Main( CURSOR_WORK *p_wk );
static void CURSOR_GetPos( const CURSOR_WORK *cp_wk, GFL_POINT *p_pos );
//-------------------------------------
///	PLACE
//=====================================
static void PLACE_Init( PLACE_WORK *p_wk, const TOWNMAP_DATA *cp_data, const TOWNMAP_GRAPHIC_SYS *p_grh, HEAPID heapID);
static void PLACE_Exit( PLACE_WORK *p_wk );
static void PLACE_ScaleUp( PLACE_WORK *p_wk );
static void PLACE_ScaleDown( PLACE_WORK *p_wk );
static void PLACE_SetVisible( PLACE_WORK *p_wk, BOOL is_visible );
static void PLACE_SetWldPos( PLACE_WORK *p_wk, const GFL_POINT *cp_pos );
static void PLACE_GetWldPos( const PLACE_WORK *cp_wk, GFL_POINT *p_pos );
static const PLACE_DATA* PLACE_Hit( const PLACE_WORK *cp_wk, const CURSOR_WORK *cp_cursor );
static u16 PLACEDATA_GetParam( const PLACE_DATA *cp_wk, TOWNMAP_DATA_PARAM param );
static void PlaceData_Init( PLACE_DATA *p_wk, const TOWNMAP_DATA *cp_data, u32 data_idx, GFL_CLUNIT *p_clunit, u32 chr, u32 cel, u32 plt, HEAPID heapID );
static void PlaceData_Exit( PLACE_DATA *p_wk );
static void PlaceData_SetVisible( PLACE_DATA *p_wk, BOOL is_visible );
static BOOL PlaceData_IsHit( const PLACE_DATA *cp_wk, const GFL_POINT *cp_pos );
//-------------------------------------
///	MAP
//=====================================
static void MAP_Init( MAP_WORK *p_wk, HEAPID heapID );
static void MAP_Exit( MAP_WORK *p_wk );
static void MAP_Main( MAP_WORK *p_wk );
static void MAP_ScaleUp( MAP_WORK *p_wk );
static void MAP_ScaleDown( MAP_WORK *p_wk );
static BOOL MAP_IsScaleEnd( const MAP_WORK *cp_wk );
static void MAP_SetWldPos( MAP_WORK *p_wk, const GFL_POINT *cp_pos );
static void MAP_GetWldPos( const MAP_WORK *cp_wk, GFL_POINT *p_pos );
//-------------------------------------
///	CONTROL
//=====================================
static void CONTROL_Init( CONTROL_WORK *p_wk, HEAPID heapID );
static void CONTROL_Exit( CONTROL_WORK *p_wk );
//-------------------------------------
///	INFO
//=====================================
static void INFO_Init( INFO_WORK *p_wk, u8 frm, GFL_FONT *p_font, const GFL_MSGDATA *cp_place_msg, const GFL_MSGDATA *cp_guide_msg, HEAPID heapID );
static void INFO_Exit( INFO_WORK *p_wk );
static void INFO_Main( INFO_WORK *p_wk );
static void INFO_Update( INFO_WORK *p_wk, const PLACE_DATA *cp_data );
//-------------------------------------
///	MSGWND_WORK
//=====================================
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe, GFL_FONT *p_font, const GFL_MSGDATA *cp_msg, u8 x, u8 y, u8 w, u8 h, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static void MSGWND_Print( MSGWND_WORK* p_wk, u32 strID, u16 x, u16 y );
//=============================================================================
/**
 *						DATA
 */
//=============================================================================
//=============================================================================
/**
 *						GLOBAL
 */
//=============================================================================
//-------------------------------------
///	PROCデータ外部公開
//=====================================
const GFL_PROC_DATA	TownMap_ProcData	=
{
	TOWNMAP_PROC_Init,
	TOWNMAP_PROC_Main,
	TOWNMAP_PROC_Exit,
};
//=============================================================================
/**
 *						PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	PROC初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param_adrs					引数
 *	@param	*p_wk_adrs						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TOWNMAP_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	TOWNMAP_WORK	*p_wk;
	TOWNMAP_PARAM	*p_param	= p_param_adrs;
	u16	data_len;

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TOWNMAP, 0x60000 );

	//ワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(TOWNMAP_WORK), HEAPID_TOWNMAP );
	GFL_STD_MemClear( p_wk, sizeof(TOWNMAP_WORK) );
	p_wk->p_param	= p_param;

	//共通システム作成---------------------
	//フォント
	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
    NARC_font_large_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_TOWNMAP );

	//地名メッセージデータ
	p_wk->p_place_msg = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_place_name_dat, HEAPID_TOWNMAP );
	//ガイドメッセージデータ
	p_wk->p_guide_msg = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_place_name_dat, HEAPID_TOWNMAP );

	//モジュール作成----------------------
	//グラフィック作成
	p_wk->p_grh	= TOWNMAP_GRAPHIC_Init( HEAPID_TOWNMAP );

	//シーケンス作成
	SEQ_Init( &p_wk->seq, p_wk, SEQFUNC_FadeOut );

	//アプリケーションバー作成
	
	{	
		u32 appbar_mode;

		if( p_wk->p_param->mode == TOWNMAP_MODE_SKY )
		{	
			appbar_mode	= APPBAR_OPTION_MASK_SKY;

			//空を飛ぶモードならばメッセージ面追加
			MSGWND_Init( &p_wk->msgwnd,
					TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_FONT_M),
					p_wk->p_font,
					p_wk->p_guide_msg,
					MSGWND_BMPWIN_SKY_X,
					MSGWND_BMPWIN_SKY_Y,
					MSGWND_BMPWIN_SKY_W,
					MSGWND_BMPWIN_SKY_H,
					HEAPID_TOWNMAP
					);
			MSGWND_Print( &p_wk->msgwnd, 0, MSGWND_STR_SKY_X, MSGWND_STR_SKY_Y );
		}
		else
		{	
			appbar_mode	= APPBAR_OPTION_MASK_TOWN;
		}

		APPBAR_Init( &p_wk->appbar,appbar_mode,
				TOWNMAP_GRAPHIC_GetUnit( p_wk->p_grh, TOWNMAP_OBJ_CLUNIT_DEFAULT ),
				TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_BAR_M),
				TOWNMAP_BG_PAL_M_15, TOWNMAP_OBJ_PAL_M_12, HEAPID_TOWNMAP );
	}

	//カーソル作成
	CURSOR_Init( &p_wk->cursor, TOWNMAP_GRAPHIC_GetClwk( p_wk->p_grh, TOWNMAP_OBJ_CLWK_CURSOR ),
			TOWNMAP_GRAPHIC_GetClwk( p_wk->p_grh, TOWNMAP_OBJ_CLWK_RING_CUR ), HEAPID_TOWNMAP );

	//マップデータ作成
	p_wk->p_data	= TOWNMAP_DATA_Alloc( HEAPID_TOWNMAP );

	//場所作成
	PLACE_Init( &p_wk->place, p_wk->p_data, p_wk->p_grh, HEAPID_TOWNMAP );

	//地図作成
	MAP_Init( &p_wk->map, HEAPID_TOWNMAP );

	//上画面情報作成
	INFO_Init( &p_wk->info, TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_FONT_S ),
			p_wk->p_font, p_wk->p_place_msg, p_wk->p_guide_msg, HEAPID_TOWNMAP );


	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	PROC破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param_adrs					引数
 *	@param	*p_wk_adrs						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TOWNMAP_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;

	//モジュール破棄-------------
	
	//上画面情報破棄
	INFO_Exit( &p_wk->info );

	//地図破棄
	MAP_Exit( &p_wk->map );

	//場所破棄
	PLACE_Exit( &p_wk->place );

	//マップデータ破棄
	TOWNMAP_DATA_Free( p_wk->p_data );

	//カーソル破棄
	CURSOR_Exit( &p_wk->cursor );

	//空を飛ぶモードならばメッセージ面破棄
	if( p_wk->p_param->mode == TOWNMAP_MODE_SKY )
	{		
		MSGWND_Exit( &p_wk->msgwnd );
	}
	//アプリケーションバー破棄
	APPBAR_Exit( &p_wk->appbar );

	//シーケンス破棄
	SEQ_Exit( &p_wk->seq );	

	//グラフィック破棄
	TOWNMAP_GRAPHIC_Exit( p_wk->p_grh );

	//共通システム破棄-------------
	//メッセージ破棄
	GFL_MSG_Delete( p_wk->p_guide_msg );
	GFL_MSG_Delete( p_wk->p_place_msg );

	//フォント破棄
	GFL_FONT_Delete( p_wk->p_font );

	//ワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap(HEAPID_TOWNMAP );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	PROCメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param_adrs					引数
 *	@param	*p_wk_adrs						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TOWNMAP_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;
	//シーケンス
	SEQ_Main( &p_wk->seq );

	//描画
	TOWNMAP_GRAPHIC_Draw( p_wk->p_grh );

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
 *						SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	初期化
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	*p_param_adrs		パラメータ
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Init( SEQ_WORK *p_wk, void *p_param_adrs, SEQ_FUNCTION seq_function )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );

	//初期化
	p_wk->p_param_adrs	= p_param_adrs;

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
		p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_param_adrs );
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
 *	@brief	フェードOUT
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_param_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs )
{	
	enum
	{	
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_END,
	};

	TOWNMAP_WORK	*p_wk	= p_param_adrs;

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
		SEQ_SetNext( p_seqwk, SEQFUNC_Main );
		break;

	default:
		GF_ASSERT(0);
	}
	
}
//----------------------------------------------------------------------------
/**
 *	@brief	フェードIN
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_param_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs )
{
	enum
	{
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};	

	TOWNMAP_WORK	*p_wk	= p_param_adrs;

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
		SEQ_End( p_seqwk );
		break;

	default:
		GF_ASSERT(0);
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	ゲーム開始
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_param_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs )
{	
	TOWNMAP_WORK	*p_wk	= p_param_adrs;

	switch( APPBAR_GetTrg( &p_wk->appbar ) )
	{	
	case APPBAR_SELECT_CLOSE:
		SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
		break;
	case APPBAR_SELECT_RETURN:
		SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
		break;
	}

	//カーソルの場所当たり判定
	{	
		const PLACE_DATA *cp_data;
		cp_data	= PLACE_Hit( &p_wk->place, &p_wk->cursor );
		if( cp_data != p_wk->cp_pre_data && cp_data != NULL )
		{	
			INFO_Update( &p_wk->info, cp_data );
		}
		p_wk->cp_pre_data	= cp_data;
		//仮
		if( cp_data != NULL && GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
		{	
			p_wk->p_param->select	= TOWNMAP_SELECT_SKY;
			p_wk->p_param->zoneID	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_ZONE_ID );
			p_wk->p_param->grid.x	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_WARP_X );
			p_wk->p_param->grid.y	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_WARP_Y );
			SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
		}
	}

	APPBAR_Main( &p_wk->appbar );
	CURSOR_Main( &p_wk->cursor );
}
//=============================================================================
/**
 *	APPBAR
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	初期化
 *
 *	@param	APPBAR_WORK *p_wk	ワーク
 *	@param	mask							モードマスク
 *	@param	p_unit						APPBARのOBJ生成用p_unit
 *	@param	bar_frm						使用するBG面（同時にメインかサブかを判定するのにも使用）
 *	@param	bg_plt						使用するBGパレット番号
 *	@param	obj_plt						使用するOBJパレット番号
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void APPBAR_Init( APPBAR_WORK *p_wk, APPBAR_OPTION_MASK mask, GFL_CLUNIT* p_unit, u8 bar_frm, u8 bg_plt, u8 obj_plt, HEAPID heapID )
{	
	CLSYS_DRAW_TYPE	clsys_draw_type;
	CLSYS_DEFREND_TYPE	clsys_def_type;
	PALTYPE							bgplttype;

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(APPBAR_WORK) );
	p_wk->bg_frm	= bar_frm;
	p_wk->trg			= APPBAR_SELECT_NONE;
	p_wk->cont		= APPBAR_SELECT_NONE;
	p_wk->mode		= mask;

	//OBJ読み込む場所をチェック
	{	
		if( bar_frm >= GFL_BG_FRAME0_S )
		{	
			clsys_draw_type	= CLSYS_DRAW_SUB;
			clsys_def_type	= CLSYS_DEFREND_SUB;
			bgplttype				= PALTYPE_SUB_BG;
		}
		else
		{	
			clsys_draw_type	= CLSYS_DRAW_MAIN;
			clsys_def_type	= CLSYS_DEFREND_MAIN;
			bgplttype				= PALTYPE_MAIN_BG;
		}
	}
	
	//共通リソース
	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), heapID );

		//BG
		//領域の確保
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, APP_COMMON_GetBarPltArcIdx(),
				bgplttype, bg_plt*0x20, APP_COMMON_BAR_PLT_NUM*0x20, heapID );

		p_wk->chr_pos	= GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, APP_COMMON_GetBarCharArcIdx(), p_wk->bg_frm, APPBAR_BG_CHARAAREA_SIZE, FALSE, heapID );
		GF_ASSERT( p_wk->chr_pos != GFL_ARCUTIL_TRANSINFO_FAIL );
		//スクリーンはメモリ上に置いて、下部32*3だけ書き込み
		ARCHDL_UTIL_TransVramScreenEx( p_handle, APP_COMMON_GetBarScrnArcIdx(), p_wk->bg_frm,
				GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->chr_pos), 0, 21, 32, 24, 
				APPBAR_MENUBAR_X, APPBAR_MENUBAR_Y, APPBAR_MENUBAR_W, APPBAR_MENUBAR_H,
				bg_plt, FALSE, heapID );

		NAGI_Printf( "pos %d size %d\n", GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->chr_pos), GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->chr_pos) );

		//OBJ
		//共通アイコンリソース
		if( (p_wk->mode & APPBAR_OPTION_MASK_CLOSE)
				|| (p_wk->mode & APPBAR_OPTION_MASK_RETURN)
				|| (p_wk->mode & APPBAR_OPTION_MASK_CUR_D)
				|| (p_wk->mode & APPBAR_OPTION_MASK_CUR_U)
				|| (p_wk->mode & APPBAR_OPTION_MASK_CUR_L)
				|| (p_wk->mode & APPBAR_OPTION_MASK_CUR_R)
				|| (p_wk->mode & APPBAR_OPTION_MASK_CHECK)
				)
		{	
			p_wk->res[APPBAR_RES_COMMON_PLT]	= GFL_CLGRP_PLTT_RegisterEx( p_handle,
				APP_COMMON_GetBarIconPltArcIdx(), clsys_draw_type, obj_plt*0x20, 0, APP_COMMON_BARICON_PLT_NUM, heapID );	
			p_wk->res[APPBAR_RES_COMMON_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
					APP_COMMON_GetBarIconCharArcIdx(), FALSE, clsys_draw_type, heapID );

			p_wk->res[APPBAR_RES_COMMON_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
					APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_128K),
				 	APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_128K), heapID );
		}
		GFL_ARC_CloseDataHandle( p_handle );
	}

	//アプリごとのリソース
	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heapID );
		if( (p_wk->mode & APPBAR_OPTION_MASK_SCALE) )
		{	
			p_wk->res[APPBAR_RES_SCALE_PLT]	= GFL_CLGRP_PLTT_RegisterEx( p_handle,
				NARC_townmap_gra_tmap_bg_d_NCLR, clsys_draw_type, (obj_plt+2)*0x20,
				2, 1, heapID );	

			p_wk->res[APPBAR_RES_SCALE_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
					NARC_townmap_gra_tmap_obj_d_NCGR, FALSE, clsys_draw_type, heapID );

			p_wk->res[APPBAR_RES_SCALE_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
					NARC_townmap_gra_tmap_obj_d_NCER,
				 	NARC_townmap_gra_tmap_obj_d_NANR, heapID );
		}

		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWKの作成
	{
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

		//終了ボタン
 		if( p_wk->mode & APPBAR_OPTION_MASK_CLOSE )
 		{	
			cldata.pos_x	= APPBAR_ICON_CLOSE_X;
			cldata.pos_y	= APPBAR_ICON_Y;
			cldata.anmseq	= APP_COMMON_BARICON_CLOSE;
			p_wk->p_clwk[APPBAR_BARICON_CLOSE]	= GFL_CLACT_WK_Create( p_unit, 
					p_wk->res[APPBAR_RES_COMMON_CHR],
					p_wk->res[APPBAR_RES_COMMON_PLT],
					p_wk->res[APPBAR_RES_COMMON_CEL],
					&cldata,
					clsys_def_type,
					heapID
					);
		}

		//戻るボタン
 		if( p_wk->mode & APPBAR_OPTION_MASK_RETURN )
 		{	
			cldata.pos_x	= APPBAR_ICON_RETURN_X;
			cldata.pos_y	= APPBAR_ICON_Y;
			cldata.anmseq	= APP_COMMON_BARICON_RETURN;
			p_wk->p_clwk[APPBAR_BARICON_RETURN]	= GFL_CLACT_WK_Create( p_unit, 
					p_wk->res[APPBAR_RES_COMMON_CHR],
					p_wk->res[APPBAR_RES_COMMON_PLT],
					p_wk->res[APPBAR_RES_COMMON_CEL],
					&cldata,
					clsys_def_type,
					heapID
					);
		}

		//左ボタン
 		if( p_wk->mode & APPBAR_OPTION_MASK_CUR_L )
 		{	
			cldata.pos_x	= APPBAR_ICON_CUR_L_X;
			cldata.pos_y	= APPBAR_ICON_Y;
			cldata.anmseq	= APP_COMMON_BARICON_CURSOR_LEFT;
			p_wk->p_clwk[APPBAR_BARICON_CUR_L]	= GFL_CLACT_WK_Create( p_unit, 
					p_wk->res[APPBAR_RES_COMMON_CHR],
					p_wk->res[APPBAR_RES_COMMON_PLT],
					p_wk->res[APPBAR_RES_COMMON_CEL],
					&cldata,
					clsys_def_type,
					heapID
					);
		}

		//右ボタン
 		if( p_wk->mode & APPBAR_OPTION_MASK_CUR_R )
 		{	
			cldata.pos_x	= APPBAR_ICON_CUR_R_X;
			cldata.pos_y	= APPBAR_ICON_Y;
			cldata.anmseq	= APP_COMMON_BARICON_CURSOR_RIGHT;
			p_wk->p_clwk[APPBAR_BARICON_CUR_R]	= GFL_CLACT_WK_Create( p_unit, 
					p_wk->res[APPBAR_RES_COMMON_CHR],
					p_wk->res[APPBAR_RES_COMMON_PLT],
					p_wk->res[APPBAR_RES_COMMON_CEL],
					&cldata,
					clsys_def_type,
					heapID
					);
		}

		//特殊リソース破棄
		if( p_wk->mode & APPBAR_OPTION_MASK_SCALE )
		{	
			cldata.pos_x	= APPBAR_ICON_SCALE_UP_X;
			cldata.pos_y	= APPBAR_ICON_Y;
			cldata.anmseq	= 7;
			p_wk->p_clwk[APPBAR_BARICON_SCALE_UP]	= GFL_CLACT_WK_Create( p_unit, 
					p_wk->res[APPBAR_RES_SCALE_CHR],
					p_wk->res[APPBAR_RES_SCALE_PLT],
					p_wk->res[APPBAR_RES_SCALE_CEL],
					&cldata,
					clsys_def_type,
					heapID
					);

			cldata.pos_x	= APPBAR_ICON_SCALE_DOWN_X;
			cldata.pos_y	= APPBAR_ICON_Y;
			cldata.anmseq	= 8;
			p_wk->p_clwk[APPBAR_BARICON_SCALE_DOWN]	= GFL_CLACT_WK_Create( p_unit, 
					p_wk->res[APPBAR_RES_SCALE_CHR],
					p_wk->res[APPBAR_RES_SCALE_PLT],
					p_wk->res[APPBAR_RES_SCALE_CEL],
					&cldata,
					clsys_def_type,
					heapID
					);
			GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk[APPBAR_BARICON_SCALE_DOWN], obj_plt+3, CLWK_PLTTOFFS_MODE_PLTT_TOP );
			GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk[APPBAR_BARICON_SCALE_UP], obj_plt+3, CLWK_PLTTOFFS_MODE_PLTT_TOP );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	破棄
 *
 *	@param	APPBAR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void APPBAR_Exit( APPBAR_WORK *p_wk )
{	
	//CLWK
	{	
		int i;
		for( i = 0; i < APPBAR_BARICON_MAX; i++ )
		{	
			if( p_wk->p_clwk[i] )
			{	
				GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
			}
		}
	}

	//OBJ
	{	
		//特殊リソース破棄
		if( p_wk->mode & APPBAR_OPTION_MASK_SCALE )
		{	
			GFL_CLGRP_CELLANIM_Release( p_wk->res[APPBAR_RES_SCALE_CEL] );
			GFL_CLGRP_CGR_Release( p_wk->res[APPBAR_RES_SCALE_CHR] );
			GFL_CLGRP_PLTT_Release( p_wk->res[APPBAR_RES_SCALE_PLT] );
		}

		//共通リソース破棄
		if( (p_wk->mode & APPBAR_OPTION_MASK_CLOSE)
				|| (p_wk->mode & APPBAR_OPTION_MASK_RETURN)
				|| (p_wk->mode & APPBAR_OPTION_MASK_CUR_D)
				|| (p_wk->mode & APPBAR_OPTION_MASK_CUR_U)
				|| (p_wk->mode & APPBAR_OPTION_MASK_CUR_L)
				|| (p_wk->mode & APPBAR_OPTION_MASK_CUR_R)
				|| (p_wk->mode & APPBAR_OPTION_MASK_CHECK)
				)
 		{	
			GFL_CLGRP_CELLANIM_Release( p_wk->res[APPBAR_RES_COMMON_CEL] );
			GFL_CLGRP_CGR_Release( p_wk->res[APPBAR_RES_COMMON_CHR] );
			GFL_CLGRP_PLTT_Release( p_wk->res[APPBAR_RES_COMMON_PLT] );
		}
	}

	//BG
	{	
		GFL_BG_FreeCharacterArea(p_wk->bg_frm,
				GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->chr_pos),
				GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->chr_pos));
	}

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(APPBAR_WORK) );

}
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	メイン処理
 *
 *	@param	APPBAR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void APPBAR_Main( APPBAR_WORK *p_wk )
{	
	static const GFL_UI_TP_HITTBL	sc_hit_tbl[APPBAR_BARICON_MAX]	=
	{	
		{	
			APPBAR_ICON_Y, APPBAR_ICON_Y + APPBAR_ICON_W, 
			APPBAR_ICON_CLOSE_X, APPBAR_ICON_CLOSE_X + APPBAR_ICON_W
		},
		{	
			APPBAR_ICON_Y, APPBAR_ICON_Y + APPBAR_ICON_W, 
			APPBAR_ICON_RETURN_X, APPBAR_ICON_RETURN_X + APPBAR_ICON_W
		},
		{	
			0
		},
		{	
			0
		},
		{	
			APPBAR_ICON_Y, APPBAR_ICON_Y + APPBAR_ICON_W, 
			APPBAR_ICON_CUR_L_X, APPBAR_ICON_CUR_L_X + APPBAR_ICON_W
		},
		{	
			APPBAR_ICON_Y, APPBAR_ICON_Y + APPBAR_ICON_W, 
			APPBAR_ICON_CUR_R_X, APPBAR_ICON_CUR_R_X + APPBAR_ICON_W
		},
		{	
			APPBAR_ICON_Y, APPBAR_ICON_Y + APPBAR_ICON_W, 
			APPBAR_ICON_SCALE_UP_X, APPBAR_ICON_SCALE_UP_X + APPBAR_ICON_W
		},
		{	
			APPBAR_ICON_Y, APPBAR_ICON_Y + APPBAR_ICON_W, 
			APPBAR_ICON_SCALE_DOWN_X, APPBAR_ICON_SCALE_DOWN_X + APPBAR_ICON_W
		},
		
	};
	
	//タッチの動き
	int i;
	u32 x, y;

	p_wk->trg		= APPBAR_SELECT_NONE;
	p_wk->cont	= APPBAR_SELECT_NONE;
	for( i = 0; i < NELEMS(sc_hit_tbl); i++ )
	{	
		if( p_wk->mode & 1<<i )
		{	
			if( GFL_UI_TP_GetPointTrg( &x, &y ) )
			{	
				if( ((u32)( x - sc_hit_tbl[i].rect.left) <= 
							(u32)(sc_hit_tbl[i].rect.right - sc_hit_tbl[i].rect.left))
						&	((u32)( y - sc_hit_tbl[i].rect.top) <=
							(u32)(sc_hit_tbl[i].rect.bottom - sc_hit_tbl[i].rect.top)))
				{
					p_wk->trg	= i;
				}
			}

			if( GFL_UI_TP_GetPointCont( &x, &y ) )
			{	
				if( ((u32)( x - sc_hit_tbl[i].rect.left) <= 
							(u32)(sc_hit_tbl[i].rect.right - sc_hit_tbl[i].rect.left))
						&	((u32)( y - sc_hit_tbl[i].rect.top) <=
							(u32)(sc_hit_tbl[i].rect.bottom - sc_hit_tbl[i].rect.top)))
				{
					p_wk->cont	= i;
				}
			}
		}
	}

	//手動の動き
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
	{	
		p_wk->trg	= APPBAR_SELECT_RETURN;
	}
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
	{	
		p_wk->trg	= APPBAR_SELECT_CLOSE;
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
	{	
		p_wk->cont	= APPBAR_SELECT_CUR_L;
	}
	else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
	{	
		p_wk->cont	= APPBAR_SELECT_CUR_R;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	選択されたものを取得
 *
 *	@param	const APPBAR_WORK *cp_wk ワーク
 *
 *	@return	APPBAR_SELECT列挙
 */
//-----------------------------------------------------------------------------
static APPBAR_SELECT APPBAR_GetTrg( const APPBAR_WORK *cp_wk )
{	
	return cp_wk->trg;
}
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	選択されたものを取得
 *
 *	@param	const APPBAR_WORK *cp_wk ワーク
 *
 *	@return	APPBAR_SELECT列挙
 */
//-----------------------------------------------------------------------------
static APPBAR_SELECT APPBAR_GetCont( const APPBAR_WORK *cp_wk )
{	
	return cp_wk->cont;
}
//----------------------------------------------------------------------------
/**
 *	@brief	Screenデータの　VRAM転送拡張版（読み込んだスクリーンの一部範囲をバッファに張りつける）
 *
 *	@param	ARCHANDLE *handle	ハンドル
 *	@param	datID							データID
 *	@param	frm								フレーム
 *	@param	chr_ofs						キャラオフセット
 *	@param	src_x							転送元ｘ座標
 *	@param	src_y							転送元Y座標
 *	@param	src_w							転送元幅			//データの全体の大きさ
 *	@param	src_h							転送元高さ		//データの全体の大きさ
 *	@param	dst_x							転送先X座標
 *	@param	dst_y							転送先Y座標
 *	@param	dst_w							転送先幅
 *	@param	dst_h							転送先高さ
 *	@param	plt								パレット番号
 *	@param	compressedFlag		圧縮フラグ
 *	@param	heapID						一時バッファ用ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void ARCHDL_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h, u8 plt, BOOL compressedFlag, HEAPID heapID )
{	
	void *p_src_arc;
	NNSG2dScreenData* p_scr_data;

	//読み込み
	p_src_arc = GFL_ARCHDL_UTIL_Load( handle, datID, compressedFlag, GetHeapLowID(heapID) );

	//アンパック
	if(!NNS_G2dGetUnpackedScreenData( p_src_arc, &p_scr_data ) )
	{	
		GF_ASSERT(0);	//スクリーンデータじゃないよ
	}

	//エラー
	GF_ASSERT( src_w * src_h * 2 <= p_scr_data->szByte );

	//キャラオフセット計算
	if( chr_ofs )
	{	
		int i;
		if( GFL_BG_GetScreenColorMode( frm ) == GX_BG_COLORMODE_16 )
		{
			u16 *p_scr16;
			
			p_scr16	=	(u16 *)&p_scr_data->rawData;
			for( i = 0; i < src_w * src_h ; i++ )
			{
				p_scr16[i]	+= chr_ofs;
			}	
		}
		else
		{	
			GF_ASSERT(0);	//256版は作ってない
		}
	}

	//書き込み
	if( GFL_BG_GetScreenBufferAdrs( frm ) != NULL )
	{	
		GFL_BG_WriteScreenExpand( frm, dst_x, dst_y, dst_w, dst_h,
				&p_scr_data->rawData, src_x, src_y, src_w, src_h );	
		GFL_BG_ChangeScreenPalette( frm, dst_x, dst_y, dst_w, dst_h, plt );
		GFL_BG_LoadScreenReq( frm );
	}
	else
	{	
		GF_ASSERT(0);	//バッファがない
	}

	GFL_HEAP_FreeMemory( p_src_arc );
}	
//=============================================================================
/**
 *	CURSOR
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	カーソル初期化
 *
 *	@param	CURSOR_WORK *p_wk	ワーク
 *	@param	*p_cursor					カーソルCLWK
 *	@param	*p_ring						リングCLWK
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void CURSOR_Init( CURSOR_WORK *p_wk, GFL_CLWK *p_cursor, GFL_CLWK *p_ring, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(CURSOR_WORK) );
	p_wk->p_clwk[CURSOR_CLWK_CURSOR]	= p_cursor;
	p_wk->p_clwk[CURSOR_CLWK_RING]		= p_ring;
	p_wk->pos.x	= CURSOR_POS_DEFAULT_X;
	p_wk->pos.y	= CURSOR_POS_DEFAULT_Y;

	{
		GFL_CLACT_WK_SetPos( p_wk->p_clwk[CURSOR_CLWK_CURSOR], &p_wk->pos, 0 );
		GFL_CLACT_WK_SetPos( p_wk->p_clwk[CURSOR_CLWK_RING], &p_wk->pos, 0 );
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_CURSOR], TRUE );
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_RING], FALSE );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	カーソル破棄
 *
 *	@param	CURSOR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void CURSOR_Exit( CURSOR_WORK *p_wk )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(CURSOR_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	カーソルメイン処理
 *
 *	@param	CURSOR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void CURSOR_Main( CURSOR_WORK *p_wk )
{	
	VecFx32	norm	=  {0,0,0};
	BOOL is_move	= FALSE;

	//カーソル移動方向
	if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
	{	
		norm.y	-= FX32_ONE;
		is_move	= TRUE;
	}
	else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
	{	
		norm.y	+= FX32_ONE;
		is_move	= TRUE;
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
	{	
		norm.x	-= FX32_ONE;
		is_move	= TRUE;
	}
	else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
	{	
		norm.x	+= FX32_ONE;
		is_move	= TRUE;
	}

	//移動計算
	if( is_move )
	{	
		VEC_Normalize( &norm, &norm );
		p_wk->pos.x	+=	(norm.x * CURSOR_ADD_SPEED) >> FX32_SHIFT; 
		p_wk->pos.y	+=	(norm.y *  CURSOR_ADD_SPEED) >> FX32_SHIFT;
		p_wk->pos.x	= MATH_CLAMP( p_wk->pos.x, CURSOR_MOVE_LIMIT_LEFT, CURSOR_MOVE_LIMIT_RIGHT );
		p_wk->pos.y	= MATH_CLAMP( p_wk->pos.y, CURSOR_MOVE_LIMIT_TOP, CURSOR_MOVE_LIMIT_BOTTOM );
		GFL_CLACT_WK_SetPos( p_wk->p_clwk[CURSOR_CLWK_CURSOR], &p_wk->pos, 0 );
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_CURSOR], TRUE );
	}

	//タッチすると消える
	if( GFL_UI_TP_GetTrg() )
	{	
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_CURSOR], FALSE );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	カーソル位置を取得
 *
 *	@param	const CURSOR_WORK *cp_wk	ワーク
 *	@param	*p_pos										位置受け取り
 *
 */
//-----------------------------------------------------------------------------
static void CURSOR_GetPos( const CURSOR_WORK *cp_wk, GFL_POINT *p_pos )
{	
	p_pos->x	= cp_wk->pos.x;
	p_pos->y	= cp_wk->pos.y;
}
//=============================================================================
/**
 *					PLACE
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	場所	初期化
 *
 *	@param	PLACE_WORK *p_wk			ワーク
 *	@param	TOWNMAP_DATA *cp_data	データ
 *	@param	p_grh									グラフィック
 *	@param	heapID								ヒープID
 */
//-----------------------------------------------------------------------------
static void PLACE_Init( PLACE_WORK *p_wk, const TOWNMAP_DATA *cp_data, const TOWNMAP_GRAPHIC_SYS *cp_grh, HEAPID heapID)
{	
	//クリアー
	GFL_STD_MemClear( p_wk, sizeof(PLACE_WORK) );

	//ユニット作成
	//ユーザーレンダラーユニット
	{	
		static const GFL_REND_SURFACE_INIT sc_rend_surface_init =
		{	
			0,0,256,192,
			CLSYS_DRAW_MAIN
		};
		p_wk->p_clunit	= GFL_CLACT_UNIT_Create( 128, 0, heapID );
		p_wk->p_rend		= GFL_CLACT_USERREND_Create( &sc_rend_surface_init, 1, heapID );
		GFL_CLACT_UNIT_SetUserRend( p_wk->p_clunit, p_wk->p_rend );
	}


	//作成するCLWKの数を計算
	{	
		int i;
		u16 type;
		p_wk->data_num	= 0;	//０にしてからカウント
		for( i = 0; i < TOWNMAP_DATA_MAX; i++ )
		{	
			type	= TOWNMAP_DATA_GetParam( cp_data, i, TOWNMAP_DATA_PARAM_PLACE_TYPE );
			//町とダンジョンだけセルを作る
			if( type == TOWNMAP_PLACETYPE_TOWN || type == TOWNMAP_PLACETYPE_DUNGEON	)
			{	
				p_wk->data_num++;
			}
		}
	}
	
	//設定されている数分CLWKを出すため、バッファ作成
	p_wk->p_place	= GFL_HEAP_AllocMemory( heapID, sizeof(PLACE_DATA) * p_wk->data_num );
	GFL_STD_MemClear( p_wk->p_place, sizeof(PLACE_DATA) * p_wk->data_num );

	//CLWK作成
	{	
		int i;
		int place_cnt;
		u32 chr, cel, plt;
		u16 type;
		//リソース取得
		TOWNMAP_GRAPHIC_GetObjResource( cp_grh, &chr, &cel, &plt );
		//初期化
		place_cnt	= 0;
		for( i = 0; i < TOWNMAP_DATA_MAX; i++ )
		{	
			type	= TOWNMAP_DATA_GetParam( cp_data, i, TOWNMAP_DATA_PARAM_PLACE_TYPE );
			//町とダンジョンだけ
			if( type == TOWNMAP_PLACETYPE_TOWN || type == TOWNMAP_PLACETYPE_DUNGEON	)
			{	
				PlaceData_Init( &p_wk->p_place[place_cnt], cp_data, i, p_wk->p_clunit, chr, cel, plt, heapID );
				place_cnt++;
			}
		}
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	場所	破棄
 *
 *	@param	PLACE_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void PLACE_Exit( PLACE_WORK *p_wk )
{	

	//CLWK破棄
	{	
		int i;
		for( i = 0; i < p_wk->data_num; i++  )
		{	
			PlaceData_Exit( &p_wk->p_place[i] );
		}
	}

	//バッファクリア
	GFL_HEAP_FreeMemory( p_wk->p_place );

	//レンダラ破棄
	{	
		GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit );
		GFL_CLACT_USERREND_Delete( p_wk->p_rend );
	}

	//ウニット破棄
	GFL_CLACT_UNIT_Delete( p_wk->p_clunit );

	GFL_STD_MemClear( p_wk, sizeof(PLACE_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	メイン処理
 *
 *	@param	PLACE_WORK *p_wk		ワーク
 *	@param	CURSOR_WORK *cp_wk	カーソル情報
 *
 */
//-----------------------------------------------------------------------------
static void PLACE_Main( PLACE_WORK *p_wk, const CURSOR_WORK *cp_wk )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	スケールアップ開始
 *
 *	@param	PLACE_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void PLACE_ScaleUp( PLACE_WORK *p_wk )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	スケールダウン開始
 *
 *	@param	PLACE_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void PLACE_ScaleDown( PLACE_WORK *p_wk )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	表示、非表示設定
 *
 *	@param	PLACE_WORK *p_wk	ワーク
 *	@param	is_visible				TRUEなら表示FALSEならば非表示
 *
 */
//-----------------------------------------------------------------------------
static void PLACE_SetVisible( PLACE_WORK *p_wk, BOOL is_visible )
{	
	int i;
	for( i = 0; i < p_wk->data_num; i++ )
	{	
		PlaceData_SetVisible( &p_wk->p_place[i], is_visible );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	座標を設定
 *
 *	@param	PLACE_WORK *p_wk		ワーク
 *	@param	GFL_POINT *cp_pos		座標
 *
 */
//-----------------------------------------------------------------------------
static void PLACE_SetWldPos( PLACE_WORK *p_wk, const GFL_POINT *cp_pos )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	座標を取得
 *
 *	@param	const PLACE_WORK *cp_wk	ワーク
 *	@param	*p_pos									受け取り座標
 */
//-----------------------------------------------------------------------------
static void PLACE_GetWldPos( const PLACE_WORK *cp_wk, GFL_POINT *p_pos )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	カーソルが衝突したか
 *
 *	@param	const PLACE_WORK *cp_wk	ワーク
 *	@param	CURSOR_WORK *cp_wk			カーソル
 *
 *	@return	NULLならば当たっていない	それ以外ならば当たった場所のデータ
 */
//-----------------------------------------------------------------------------
static const PLACE_DATA* PLACE_Hit( const PLACE_WORK *cp_wk, const CURSOR_WORK *cp_cursor )
{	
	int i;
	
	GFL_POINT pos;

	CURSOR_GetPos( cp_cursor, &pos );

	for( i = 0; i < cp_wk->data_num; i++ )
	{
		if( PlaceData_IsHit( &cp_wk->p_place[i], &pos )  )
		{	
			return &cp_wk->p_place[i];
		}
	}

	return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データ	データ取得
 *
 *	@param	const PLACE_DATA *cp_wk	ワーク
 *	@param	param										情報の種類
 *
 *	@return	情報
 */
//-----------------------------------------------------------------------------
static u16 PLACEDATA_GetParam( const PLACE_DATA *cp_wk, TOWNMAP_DATA_PARAM param )
{	
	return TOWNMAP_DATA_GetParam( cp_wk->cp_data, cp_wk->data_idx, param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データ作成
 *
 *	@param	PLACE_DATA *p_wk	ワーク
 *	@param	cp_data						データ
 *	@param	data_idx					データ番号
 *	@param	*p_clunit					UNIT
 *	@param	chr								チャラ
 *	@param	cel								セル
 *	@param	plt								パレット
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void PlaceData_Init( PLACE_DATA *p_wk, const TOWNMAP_DATA *cp_data, u32 data_idx, GFL_CLUNIT *p_clunit, u32 chr, u32 cel, u32 plt, HEAPID heapID )
{	
	GFL_CLWK_DATA cldata;

	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(PLACE_DATA) );
	p_wk->cp_data			= cp_data;
	p_wk->data_idx		= data_idx;

	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
	cldata.pos_x	= PLACEDATA_GetParam( p_wk,TOWNMAP_DATA_PARAM_CURSOR_X );
	cldata.pos_y	= PLACEDATA_GetParam( p_wk,TOWNMAP_DATA_PARAM_CURSOR_Y );
	switch( PLACEDATA_GetParam( p_wk,TOWNMAP_DATA_PARAM_PLACE_TYPE ) )
	{	
	case TOWNMAP_PLACETYPE_TOWN:
		cldata.anmseq	= 2;
		break;
	case TOWNMAP_PLACETYPE_DUNGEON:
		cldata.anmseq	= 3;
		break;
	default:
		GF_ASSERT(0);
	}
	p_wk->p_clwk	= GFL_CLACT_WK_Create( p_clunit, chr, plt, cel, &cldata, 0, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データ破棄
 *
 *	@param	PLACE_DATA *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void PlaceData_Exit( PLACE_DATA *p_wk )
{	
	GFL_CLACT_WK_Remove( p_wk->p_clwk );
	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(PLACE_DATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データ表示設定
 *
 *	@param	PLACE_DATA *p_wk	ワーク
 *	@param	is_visible				TRUEで表示	FALSEで非ひょ時
 *
 */
//-----------------------------------------------------------------------------
static void PlaceData_SetVisible( PLACE_DATA *p_wk, BOOL is_visible )
{	
	GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, is_visible );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データと当たったか
 *
 *	@param	const PLACE_DATA *cp_wk	わーく
 *	@param	GFL_POINT *cp_pos				座標
 *
 *	@return	TRUEならば衝突	FALSEならばしていない
 */
//-----------------------------------------------------------------------------
static BOOL PlaceData_IsHit( const PLACE_DATA *cp_wk, const GFL_POINT *cp_pos )
{	
	GFL_COLLISION3D_CIRCLE		circle;
	GFL_COLLISION3D_CYLINDER	cylinder;
	VecFx32	v1, v2;
	fx32 w;

	v1.x	= FX32_CONST( cp_pos->x );
	v1.y	= FX32_CONST( cp_pos->y );
	v1.z	= 0;
	GFL_COLLISION3D_CIRCLE_SetData( &circle, &v1, 0 );


	v1.x	= FX32_CONST( PLACEDATA_GetParam( cp_wk,TOWNMAP_DATA_PARAM_HIT_START_X ) );
	v1.y	= FX32_CONST( PLACEDATA_GetParam( cp_wk,TOWNMAP_DATA_PARAM_HIT_START_Y ) );
	v1.z	= 0;
	v2.x	= FX32_CONST( PLACEDATA_GetParam( cp_wk,TOWNMAP_DATA_PARAM_HIT_END_X ) );
	v2.y	= FX32_CONST( PLACEDATA_GetParam( cp_wk,TOWNMAP_DATA_PARAM_HIT_END_Y ) );
	v2.z	= 0;
	w			= FX32_CONST( PLACEDATA_GetParam( cp_wk,TOWNMAP_DATA_PARAM_HIT_WIDTH ) );
	GFL_COLLISION3D_CYLINDER_SetData( &cylinder, &v1, &v2, w );
	
	return GFL_COLLISION3D_CYLXCIR_Check( &cylinder, &circle, NULL );
}

//=============================================================================
/**
 *		MAP
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	地図	初期化
 *
 *	@param	MAP_WORK *p_wk	ワーク
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MAP_Init( MAP_WORK *p_wk, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MAP_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	破棄
 *
 *	@param	MAP_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MAP_Exit( MAP_WORK *p_wk )
{	
	GFL_STD_MemClear( p_wk, sizeof(MAP_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	メイン処理
 *
 *	@param	MAP_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MAP_Main( MAP_WORK *p_wk )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	拡大開始
 *
 *	@param	MAP_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MAP_ScaleUp( MAP_WORK *p_wk )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	縮小開始
 *
 *	@param	MAP_WORK *p_wk	ワーク 
 *
 */
//-----------------------------------------------------------------------------
static void MAP_ScaleDown( MAP_WORK *p_wk )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	拡大縮小処理終了チェック
 *
 *	@param	MAP_WORK *p_wk	ワーク
 *
 *	@return	TRUE終了	FALSE処理中
 */
//-----------------------------------------------------------------------------
static BOOL MAP_IsScaleEnd( const MAP_WORK *cp_wk )
{	
	return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	座標設定
 *
 *	@param	MAP_WORK *p_wk		ワーク
 *	@param	GFL_POINT *cp_pos 座標
 *
 */
//-----------------------------------------------------------------------------
static void MAP_SetWldPos( MAP_WORK *p_wk, const GFL_POINT *cp_pos )
{	
	
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	座標取得
 *
 *	@param	const MAP_WORK *cp_wk	ワーク
 *	@param	*p_pos								座標受け取り
 *
 */
//-----------------------------------------------------------------------------
static void MAP_GetWldPos( const MAP_WORK *cp_wk, GFL_POINT *p_pos )
{	

}
//=============================================================================
/**
 *		INFO
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	上画面情報	初期化
 *
 *	@param	INFO_WORK *p_wk	ワーク
 *	@param	frm							文字作成用フレーム
 *	@param	*p_font					フォント
 *	@param	*p_place_msg		地名GMM
 *	@param	*p_guide_msg		ガイドGMM
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void INFO_Init( INFO_WORK *p_wk, u8 frm, GFL_FONT *p_font, const GFL_MSGDATA *cp_place_msg, const GFL_MSGDATA *cp_guide_msg, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(INFO_WORK) );
	p_wk->p_font			= p_font;
	p_wk->cp_place_msg	= cp_place_msg;
	p_wk->cp_guide_msg	= cp_guide_msg;

	//文字面作成
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( frm, INFO_BMPWIN_X, INFO_BMPWIN_Y, INFO_BMPWIN_W, INFO_BMPWIN_H,
			TOWNMAP_BG_PAL_S_14, GFL_BMP_CHRAREA_GET_F );

	//汎用文字バッファ
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( 255, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	上画面情報	破棄
 *
 *	@param	INFO_WORK *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void INFO_Exit( INFO_WORK *p_wk )
{
	//汎用文字バッファ破棄
	GFL_STR_DeleteBuffer( p_wk->p_strbuf );

	//文字面削除
	GFL_BMPWIN_Delete( p_wk->p_bmpwin );

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(INFO_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	上画面情報	メイン処理
 *
 *	@param	INFO_WORK *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void INFO_Main( INFO_WORK *p_wk )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	上画面情報	情報アップデート
 *
 *	@param	INFO_WORK *p_wk			わーく
 *	@param	PLACE_DATA *cp_data	表示する場所の情報
 */
//-----------------------------------------------------------------------------
static void INFO_Update( INFO_WORK *p_wk, const PLACE_DATA *cp_data )
{	
	GFL_BMP_DATA	*p_bmp	= GFL_BMPWIN_GetBmp( p_wk->p_bmpwin );

	//クリア
	GFL_BMP_Clear( p_bmp, 0 );

	if( cp_data )
	{	
		u16 msgID;
		u16 zoneID;

		//地名
		zoneID	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_ZONE_ID );
		GF_ASSERT( zoneID != TOWNMAP_DATA_ERROR );
		GFL_MSG_GetString( p_wk->cp_place_msg, ZONEDATA_GetPlaceNameID( zoneID ), p_wk->p_strbuf );
#ifdef INFO_STR_PLACE_CENTERING
		{	
			s16 x	= GFL_BMPWIN_GetSizeX( p_wk->p_bmpwin )*4;
			x	-= PRINTSYS_GetStrWidth( p_wk->p_strbuf, p_wk->p_font, 0 )/2;
			PRINTSYS_Print( p_bmp, x, INFO_STR_PLACE_Y, p_wk->p_strbuf, p_wk->p_font );	
		}
#else
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE_X, INFO_STR_PLACE_Y, p_wk->p_strbuf, p_wk->p_font );	
#endif

		//GUIDE
		msgID	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_GUIDE_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_GUIDE_X, INFO_STR_GUIDE_Y, p_wk->p_strbuf, p_wk->p_font );
		}

		//PLACE1
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE1_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE1_X, INFO_STR_PLACE1_Y, p_wk->p_strbuf, p_wk->p_font );
		}

		//PLACE2
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE2_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE2_X, INFO_STR_PLACE2_Y, p_wk->p_strbuf, p_wk->p_font );
		}
		//PLACE3
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE3_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE3_X, INFO_STR_PLACE3_Y, p_wk->p_strbuf, p_wk->p_font );
		}
		//PLACE4
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE4_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE4_X, INFO_STR_PLACE4_Y, p_wk->p_strbuf, p_wk->p_font );
		}
		//PLACE5
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE5_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE5_X, INFO_STR_PLACE5_Y, p_wk->p_strbuf, p_wk->p_font );
		}
		//PLACE6
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE5_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE6_X, INFO_STR_PLACE6_Y, p_wk->p_strbuf, p_wk->p_font );
		}

	}
	GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_bmpwin );
}
//=============================================================================
/**
 *		MSGWND
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ面	初期化
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	bgframe						BGフレーム
 *	@param	*p_font						フォント
 *	@param	*p_msg						メッセージ
 *	@param	x									座標X
 *	@param	y									座標Y
 *	@param	w									幅
 *	@param	h									高さ
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe, GFL_FONT *p_font, const GFL_MSGDATA *cp_msg, u8 x, u8 y, u8 w, u8 h, HEAPID heapID )
{	
	//クリアー
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
	p_wk->p_font	= p_font;
	p_wk->cp_msg		= cp_msg;

	p_wk->p_bmpwin	= GFL_BMPWIN_Create( bgframe, x, y, w, h, TOWNMAP_BG_PAL_M_14, GFL_BMP_CHRAREA_GET_F );
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( 255, heapID );

}
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ面	破棄
 *
 *	@param	MSGWND_WORK* p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void MSGWND_Exit( MSGWND_WORK* p_wk )
{	
	GFL_STR_DeleteBuffer( p_wk->p_strbuf );
	GFL_BMPWIN_Delete( p_wk->p_bmpwin );
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ面	プリント
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	strID							文字列ID
 *	@param	x									座標X
 *	@param	y									座標Y
 */
//-----------------------------------------------------------------------------
static void MSGWND_Print( MSGWND_WORK* p_wk, u32 strID, u16 x, u16 y )
{	

	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	

	//文字列作成
	GFL_FONTSYS_SetColor( 0xf, 1, 0 );
	GFL_MSG_GetString( p_wk->cp_msg, strID, p_wk->p_strbuf );
	PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), x, y, p_wk->p_strbuf, p_wk->p_font );
	GFL_FONTSYS_SetDefaultColor();

	GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_bmpwin );
}
