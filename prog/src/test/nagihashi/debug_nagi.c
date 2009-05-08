//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		d_nagi.c
 *	@brief	名木橋用デバッグ
 *	@author	Toru=Nagihashi
 *	@data		2009.05.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//lib
#include <gflib.h>
#include <wchar.h>	//wcslen

//contant
#include "system/main.h"	//HEAPID

//module
#include "system/bmp_menulist.h"

//archive
#include "arc_def.h"
#include "font/font.naix"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	リストの最大文字数
//=====================================
#define LISTDATA_STR_LENGTH	(32)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	BG関係
//=====================================
typedef struct
{
	GFL_BMPWIN	*p_bmpwin;
} GRAPHIC_BG_WORK;
//-------------------------------------
///	描画関係
//=====================================
typedef struct 
{
	GRAPHIC_BG_WORK		gbg;
} GRAPHIC_WORK;
//-------------------------------------
///	リスト
//=====================================
typedef struct 
{
	BMPMENULIST_WORK	*p_list;
	BMP_MENULIST_DATA *p_list_data;
	u32		select;
} LIST_WORK;
//-------------------------------------
///	メッセージ
//=====================================
typedef struct {
	GFL_FONT*				  p_font;
  PRINT_STREAM*     p_printstream;
  PRINT_QUE*        p_print_que;
  PRINT_UTIL        print_util;
} MSG_WORK;

//-------------------------------------
///	メインワーク
//=====================================
typedef struct
{
	GRAPHIC_WORK			grp;
	MSG_WORK					msg;
	LIST_WORK					list;
	GFL_BMPWIN				*p_bmpwin;

	BOOL	is_end;
	//Proc切り替え用
	FSOverlayID overlay_Id;
	const GFL_PROC_DATA *p_procdata;
	void	*p_proc_work;
} DEBUG_NAGI_MAIN_WORK;

//-------------------------------------
///	リスト設定テーブル
//=====================================
typedef struct 
{	
	u16	str[LISTDATA_STR_LENGTH];
	u32	param;
} LIST_SETUP_TBL;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Init( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work );
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Exit( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work );
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Main( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work );
//汎用
static void DEBUG_NAGI_COMMAND_ChangeProc( DEBUG_NAGI_MAIN_WORK * p_wk, FSOverlayID ov_id, const GFL_PROC_DATA *p_procdata, void *p_work );
static void DEBUG_NAGI_COMMAND_ChangeMenu( DEBUG_NAGI_MAIN_WORK * p_wk, const LIST_SETUP_TBL *cp_tbl, u32 tbl_max );
static void DEBUG_NAGI_COMMAND_End( DEBUG_NAGI_MAIN_WORK *p_wk );
//grp
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk );
//BG
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
//LIST
static void LIST_Init( LIST_WORK *p_wk, const LIST_SETUP_TBL *cp_tbl, u32 tbl_max, MSG_WORK *p_msg, GFL_BMPWIN*	p_bmpwin, HEAPID heapID );
static void LIST_Exit( LIST_WORK *p_wk );
static void LIST_Main( LIST_WORK *p_wk );
static BOOL LIST_IsDecide( const LIST_WORK *cp_wk, u32 *p_select );

//MSG_WORK
static void MSG_Init( MSG_WORK *p_wk, HEAPID heapID );
static void MSG_Exit( MSG_WORK *p_wk );
static PRINT_UTIL * MSG_GetPrintUtil( MSG_WORK *p_wk, GFL_BMPWIN*	p_bmpwin );
static GFL_FONT*	MSG_GetFont( const MSG_WORK *cp_wk );
static PRINT_STREAM * MSG_GetPrintStream( const MSG_WORK *cp_wk );
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk );
//LISTFUNC
typedef void (*LISTDATA_FUNCTION)( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_Test( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_Return( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_NextListHome( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_NextListPage1( DEBUG_NAGI_MAIN_WORK *p_wk );

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	外部参照PROCデータ
//=====================================
const GFL_PROC_DATA	DebugNagiMainProcData	= 
{
	DEBUG_PROC_NAGI_Init,
	DEBUG_PROC_NAGI_Main,
	DEBUG_PROC_NAGI_Exit,
};

//-------------------------------------
///	BGシステム
//=====================================
typedef enum 
{
	GRAPHIC_BG_FRAME_TEXT,
	GRAPHIC_BG_FRAME_MAX
} GRAPHIC_BG_FRAME;
static const u32 sc_bgcnt_frame[ GRAPHIC_BG_FRAME_MAX ] = 
{
	GFL_BG_FRAME0_M,
};
static const GFL_BG_BGCNT_HEADER sc_bgcnt_data[ GRAPHIC_BG_FRAME_MAX ] = 
{
	// GRAPHIC_BG_FRAME_TEXT
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	},
};

//-------------------------------------
///	リストで実行するもの
//=====================================
enum
{	
	LISTDATA_SEQ_TEST,
	LISTDATA_SEQ_RETURN,
	LISTDATA_SEQ_NEXT_HOME,
	LISTDATA_SEQ_NEXT_PAGE1,
	LISTDATA_SEQ_MAX,
};
static const LISTDATA_FUNCTION	sc_list_funciton[]	= 
{	
	LISTDATA_Test,
	LISTDATA_Return,
	LISTDATA_NextListHome,
	LISTDATA_NextListPage1,
};

//-------------------------------------
///	リストデータ
//=====================================
static const LIST_SETUP_TBL sc_list_data_home[]	=
{	
	{	
		L"文字列", LISTDATA_SEQ_TEST
	},
	{	
		L"次へ", LISTDATA_SEQ_NEXT_PAGE1
	},
	{	
		L"もどる", LISTDATA_SEQ_RETURN
	}
};

static const LIST_SETUP_TBL sc_list_data_page1[]	=
{	
	{	
		L"進んだよ", LISTDATA_SEQ_TEST
	},
	{	
		L"前へ", LISTDATA_SEQ_NEXT_HOME
	}
};


//----------------------------------------------------------------------------
/**
 *	@brief	メインプロック	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_parent					親ワーク
 *	@param	*p_work						自分のワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Init( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work )
{	
	DEBUG_NAGI_MAIN_WORK	*p_wk;
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NAGI_DEBUG, 0x10000 );
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(DEBUG_NAGI_MAIN_WORK), HEAPID_NAGI_DEBUG );
	GFL_STD_MemClear( p_wk, sizeof(DEBUG_NAGI_MAIN_WORK) );

	GRAPHIC_Init( &p_wk->grp, HEAPID_NAGI_DEBUG );

	MSG_Init( &p_wk->msg, HEAPID_NAGI_DEBUG );

	p_wk->p_bmpwin	= GFL_BMPWIN_Create( sc_bgcnt_frame[GRAPHIC_BG_FRAME_TEXT],
			1, 1, 30, 12, 0, GFL_BMP_CHRAREA_GET_B );
	GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );

	LIST_Init( &p_wk->list, sc_list_data_home, NELEMS(sc_list_data_home), 
			&p_wk->msg, p_wk->p_bmpwin, HEAPID_NAGI_DEBUG );


	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メインプロック	破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_parent					親ワーク
 *	@param	*p_work						自分のワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Exit( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work )
{	
	DEBUG_NAGI_MAIN_WORK	*p_wk;

	p_wk	= p_work;

	if( p_wk->p_procdata )
	{	
		GFL_PROC_SysSetNextProc( p_wk->overlay_Id, p_wk->p_procdata, p_wk->p_proc_work );
	}

	LIST_Exit( &p_wk->list );

	GFL_BMPWIN_Delete( p_wk->p_bmpwin );

	MSG_Exit( &p_wk->msg );

	GRAPHIC_Exit( &p_wk->grp );

	GFL_PROC_FreeWork( p_proc );
	GFL_HEAP_DeleteHeap( HEAPID_NAGI_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メインプロック	メイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_parent					親ワーク
 *	@param	*p_work						自分のワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Main( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_MAIN,
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};

	DEBUG_NAGI_MAIN_WORK	*p_wk;

	p_wk	= p_work;


	switch( *p_seq )
	{	
	case SEQ_INIT:
//		*p_seq	= SEQ_FADEOUT_START;
			*p_seq	= SEQ_MAIN;
		break;

	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_MAIN;
		}
		break;

	case SEQ_MAIN:
		{
			u32 select;
			LIST_Main( &p_wk->list );
			if( LIST_IsDecide( &p_wk->list, &select ) )
			{	
				sc_list_funciton[ select ]( p_wk );
			}

			//終了判定
			if( p_wk->is_end )
			{	
				*p_seq	= SEQ_FADEIN_START;
			}
		}
		break;

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
		return GFL_PROC_RES_FINISH;

	default:
		GF_ASSERT_MSG( 0, "DEBUG_PROC_NAGI_MainのSEQエラー %d", *p_seq );
	}

	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *		操作用
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	プロセス切り替え
 *
 *	@param	DEBUG_NAGI_MAIN_WORK * p_wk	ワーク
 *	@param	ov_id												オーバーレイID
 *	@param	GFL_PROC_DATA *p_procdata		プロセスデータ
 *	@param	*p_work											渡す情報
 *
 */
//-----------------------------------------------------------------------------
static void DEBUG_NAGI_COMMAND_ChangeProc( DEBUG_NAGI_MAIN_WORK * p_wk, FSOverlayID ov_id, const GFL_PROC_DATA *p_procdata, void *p_work )
{	
	p_wk->overlay_Id	= ov_id;
	p_wk->p_procdata	= p_procdata;
	p_wk->p_proc_work	= p_work;
	p_wk->is_end	= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メニューを切り替え
 *
 *	@param	DEBUG_NAGI_MAIN_WORK * p_wk	ワーク
 *	@param	LIST_SETUP_TBL *cp_tbl			リスト用テーブル
 *	@param	tbl_max											リスト用テーブルの要素数
 *
 */
//-----------------------------------------------------------------------------
static void DEBUG_NAGI_COMMAND_ChangeMenu( DEBUG_NAGI_MAIN_WORK * p_wk, const LIST_SETUP_TBL *cp_tbl, u32 tbl_max )
{
	LIST_Exit( &p_wk->list );
	LIST_Init( &p_wk->list, cp_tbl, tbl_max, &p_wk->msg, p_wk->p_bmpwin, HEAPID_NAGI_DEBUG );
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void DEBUG_NAGI_COMMAND_End( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}

//=============================================================================
/**
 *	リスト関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	test
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_Test( DEBUG_NAGI_MAIN_WORK *p_wk )
{
	NAGI_Printf("てすとー\n");
}
//----------------------------------------------------------------------------
/**
 *	@brief	終了
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_Return( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_End( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	最初のリストに
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_NextListHome( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_ChangeMenu( p_wk, sc_list_data_home, NELEMS(sc_list_data_home) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	1ページへ飛ぶ
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_NextListPage1( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_ChangeMenu( p_wk, sc_list_data_page1, NELEMS(sc_list_data_page1) );
}
//=============================================================================
/**
 *				GRAPHIC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	描画関係初期化
 *
 *	@param	GRAPHIC_WORK* p_wk	ワーク
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Init( GRAPHIC_WORK* p_wk, HEAPID heapID )
{
	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );

	// ディスプレイON
	GFL_DISP_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );
	GFL_DISP_SetDispOn();

	// VRAMバンク設定
	{
		static const GFL_DISP_VRAM sc_vramSetTable =
		{
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,         // メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,     // サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_16_I,           // サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,    // サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット
			GX_OBJVRAMMODE_CHAR_1D_128K,		
			GX_OBJVRAMMODE_CHAR_1D_128K,		
		};
		GFL_DISP_SetBank( &sc_vramSetTable );
	}

	//描画モジュール
	GRAPHIC_BG_Init( &p_wk->gbg, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画関係破棄
 *
 *	@param	GRAPHIC_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Exit( GRAPHIC_WORK* p_wk )
{
	GRAPHIC_BG_Exit( &p_wk->gbg );
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画関係描画処理
 *
 *	@param	GRAPHIC_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Draw( GRAPHIC_WORK* p_wk )
{
}
//=============================================================================
/**
 *					GRAPHIC_BG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BG描画	初期化
 *
 *	@param	GRAPHIC_BG_WORK* p_wk	ワーク
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK* p_wk, HEAPID heapID )
{
	int i;

	//ＢＧシステム起動
	GFL_BG_Init( heapID );
	GFL_BMPWIN_Init( heapID );

	//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sc_bg_sys_header = 
		{
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
		};	
		GFL_BG_SetBGMode( &sc_bg_sys_header );
	}

	//ＢＧコントロール設定
	{
		int i;

		for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
		{
			GFL_BG_SetBGControl( sc_bgcnt_frame[i], &sc_bgcnt_data[i], GFL_BG_MODE_TEXT );
			GFL_BG_ClearFrame( sc_bgcnt_frame[i] );
			GFL_BG_SetVisible( sc_bgcnt_frame[i], VISIBLE_ON );
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	BG描画	破棄
 *
 *	@param	GRAPHIC_BG_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK* p_wk )
{	
	int i;

	// BGコントロール破棄
	for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
	{
		GFL_BG_FreeBGControl( sc_bgcnt_frame[i] );
	}

	// BGシステム破棄
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();
}

//=============================================================================
/**
 *					LIST
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	リストシステム　初期化
 *
 *	@param	LIST_WORK *p_wk	ワーク
 *	@param	*p_tbl					設定テーブル
 *	@param	tbl_max					設定テーブル数
 *	@param	*p_msg					メッセージワーク
 *	@param	HEAPID heapID		ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void LIST_Init( LIST_WORK *p_wk, const LIST_SETUP_TBL *cp_tbl, u32 tbl_max, MSG_WORK *p_msg, GFL_BMPWIN*	p_bmpwin, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(LIST_WORK));


	//LISTDATA create
	{	
		int i;
		STRBUF	*p_strbuf;
		STRCODE	str[128];
		u16	strlen;

		p_wk->p_list_data	= BmpMenuWork_ListCreate( tbl_max, heapID );

		p_strbuf = GFL_STR_CreateBuffer( LISTDATA_STR_LENGTH+1, heapID);

		for( i = 0; i < tbl_max; i++ )
		{	
			//
			strlen	= wcslen(cp_tbl[i].str);
			GFL_STD_MemCopy(cp_tbl[i].str, str, strlen*2);
			str[strlen]	= GFL_STR_GetEOMCode();
			GFL_STR_SetStringCode( p_strbuf, str);

			BmpMenuWork_ListAddString( &p_wk->p_list_data[i], p_strbuf, cp_tbl[i].param, heapID );
		}

		GFL_STR_DeleteBuffer( p_strbuf );
	}

	//LIST create
	{	
		BMPMENULIST_HEADER	header;

		GFL_STD_MemClear( &header, sizeof(BMPMENULIST_HEADER));
		header.list				= p_wk->p_list_data;
		header.win				= p_bmpwin;
		header.count			= tbl_max;
		header.line				= 5;
		header.rabel_x		= 0;
		header.data_x			= 16;
		header.cursor_x		= 0;
		header.line_y			= 2;
		header.f_col			= 1;
		header.b_col			= 15;
		header.s_col			= 2;
		header.msg_spc		= 0;
		header.line_spc		= 0;
		header.page_skip	= BMPMENULIST_LRKEY_SKIP;
		header.font				= 0;
		header.c_disp_f		= 0;
		header.work				= NULL;
		header.font_size_x= 16;
		header.font_size_y= 16;
		header.msgdata		= NULL;
		header.print_util	= MSG_GetPrintUtil( p_msg, p_bmpwin );
		header.print_que	= MSG_GetPrintQue( p_msg );
		header.font_handle= MSG_GetFont( p_msg );

		p_wk->p_list	= BmpMenuList_Set( &header, 0, 0, heapID );
		BmpMenuList_SetCursorBmp( p_wk->p_list, heapID );
	}


}

//----------------------------------------------------------------------------
/**
 *	@brief	リストシステム	破棄
 *
 *	@param	LIST_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LIST_Exit( LIST_WORK *p_wk )
{
	BmpMenuList_Exit( p_wk->p_list, NULL, NULL );
	BmpMenuWork_ListDelete( p_wk->p_list_data );
	GFL_STD_MemClear( p_wk, sizeof(LIST_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	リストシステム	メイン処理
 *
 *	@param	LIST_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LIST_Main( LIST_WORK *p_wk )
{	
	p_wk->select	= BmpMenuList_Main(	p_wk->p_list );
}

//----------------------------------------------------------------------------
/**
 *	@brief	リスト決定
 *
 *	@param	const LIST_WORK *cp_wk	ワーク
 *	@param	*p_select								リストのデータ
 *
 *	@retval	TRUEならば決定
 *	@retval	FALSEならば選択中
 */
//-----------------------------------------------------------------------------
static BOOL LIST_IsDecide( const LIST_WORK *cp_wk, u32 *p_select )
{	
	if( cp_wk->select != BMPMENULIST_NULL
		&& cp_wk->select != BMPMENULIST_CANCEL )
	{	
		if( p_select )
		{	
			*p_select	= cp_wk->select;
		}
		return TRUE;
	}

	return FALSE;
}

//=============================================================================
/**
 *					MSG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	MSG関係を設定
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSG_Init( MSG_WORK *p_wk, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSG_WORK) );

	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
    NARC_font_large_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

	p_wk->p_print_que = PRINTSYS_QUE_Create( heapID );

 
	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0, 0, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	MSG関係を破棄
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *	
 */
//-----------------------------------------------------------------------------
static void MSG_Exit( MSG_WORK *p_wk )
{	
	PRINTSYS_QUE_Delete( p_wk->p_print_que );

	GFL_FONT_Delete( p_wk->p_font );
	GFL_STD_MemClear( p_wk, sizeof(MSG_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	PRINT_UTILを設定し取得
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *	@param	BMPWIN
 *
 *	@return	PRINT_UTIL
 */
//-----------------------------------------------------------------------------
static PRINT_UTIL * MSG_GetPrintUtil( MSG_WORK *p_wk, GFL_BMPWIN*	p_bmpwin )
{	
	PRINT_UTIL_Setup( &p_wk->print_util, p_bmpwin );
	return &p_wk->print_util;
}

//----------------------------------------------------------------------------
/**
 *	@brief	FONTを取得
 *
 *	@param	const MSG_WORK *cp_wk		ワーク
 *
 *	@return	FONT
 */
//-----------------------------------------------------------------------------
static GFL_FONT*	MSG_GetFont( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_font;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PRINTSTREAMを取得
 *
 *	@param	const MSG_WORK *cp_wk		ワーク
 *
 *	@return	PRINTSTREAM
 */
//-----------------------------------------------------------------------------
static PRINT_STREAM * MSG_GetPrintStream( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_printstream;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PRINTQUEを取得
 *
 *	@param	const MSG_WORK *cp_wk		ワーク
 *
 *	@return	PRINTQUE
 */
//-----------------------------------------------------------------------------
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_print_que;
}
