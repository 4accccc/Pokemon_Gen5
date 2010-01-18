//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_appbar.c
 *	@brief	赤外線チェック用下画面バー
 *	@author	Toru=Nagihashi
 *	@data		2009.07.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>

//archive
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_irc_compatible.h"
#include "app/app_taskmenu.h"
#include "irccompatible_gra.naix"

//mine
#include "net_app/irc_appbar.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	APPBAR
//=====================================
//リソース
enum
{	
	APPBAR_RES_COMMON_PLT,
	APPBAR_RES_COMMON_CHR,
	APPBAR_RES_COMMON_CEL,

	APPBAR_RES_BAR_PLT,
	APPBAR_RES_BAR_CHR,
	APPBAR_RES_BAR_CEL,

	APPBAR_RES_MAX
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	下画面バー(アプリケーションバーと勝手に命名)
//=====================================
struct _APPBAR_WORK
{
  APP_TASKMENU_WIN_WORK   *p_win;
  APP_TASKMENU_RES        *p_menu_res;
	GFL_CLWK	*p_clwk[APPBAR_ICON_MAX];
	u32				res[APPBAR_RES_MAX];
	u32				bg_frm;
	GFL_ARCUTIL_TRANSINFO				chr_pos;
	s32				trg;
	s32				cont;
	u32				tbl_len;
};


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//リソース
static void AppBar_LoadResource( APPBAR_WORK *p_wk, CLSYS_DRAW_TYPE	clsys_draw_type, PALTYPE bgplttype, APP_COMMON_MAPPING mapping, u8 bg_plt, u8 obj_plt, HEAPID heapID );
static void AppBar_UnLoadResource( APPBAR_WORK *p_wk );
//obj
static void AppBar_CreateObj( APPBAR_WORK *p_wk, GFL_CLUNIT* p_unit, APPBAR_OPTION_MASK mask, CLSYS_DEFREND_TYPE	clsys_def_type, HEAPID heapID );
static void AppBar_DeleteObj( APPBAR_WORK *p_wk );
//etc
static void ARCHDL_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h,  u8 plt, BOOL compressedFlag, HEAPID heapID );

//=============================================================================
/**
 *					データ
 */
//=============================================================================


//=============================================================================
/**
 *					GLOBAL
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief	初期化拡張版（アイコン位置指定版）
 *
 *	@param	const APPBAR_SETUP *cp_setup_tbl	設定テーブル
 *	@param	tbl_len	テーブル数
 *	@param	p_unit	OBJ設定UNIT
 *	@param	bar_frm	使用BG番号	（同時に上下画面を調べています）
 *	@param	bg_plt	使用BG用PLTNUM	
 *	@param	obj_plt	使用OBJ用PLTNUM
 *	@param	mapping	OBJマッピングモード
 *	@param	heapID	ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
APPBAR_WORK * APPBAR_Init( APPBAR_OPTION_MASK mask, GFL_CLUNIT* p_unit, u8 bar_frm, u8 bg_plt, u8 obj_plt, APP_COMMON_MAPPING mapping, GFL_FONT* p_font, PRINT_QUE* p_que, HEAPID heapID )
{	
	APPBAR_WORK	*p_wk;

	GF_ASSERT( p_unit );

	//ワーク作成、初期化
	{	
		u32 size;

		size	= sizeof(APPBAR_WORK);
		p_wk	= GFL_HEAP_AllocMemory( heapID, size );
		GFL_STD_MemClear( p_wk, size );
		p_wk->bg_frm	= bar_frm;
		p_wk->trg			= APPBAR_SELECT_NONE;
		p_wk->cont		= APPBAR_SELECT_NONE;
	}

	{	
		CLSYS_DRAW_TYPE			clsys_draw_type;
		CLSYS_DEFREND_TYPE	clsys_def_type;
		PALTYPE							bgplttype;

		//OBJ読み込む場所をチェック
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

		//リソース読み込み
		AppBar_LoadResource( p_wk, clsys_draw_type, bgplttype, mapping, bg_plt, obj_plt, heapID );

    //taskmenu用リソース読み込み
    p_wk->p_menu_res  = APP_TASKMENU_RES_Create( bar_frm, bg_plt, p_font, p_que, heapID );

		//CLWK読み込み
		AppBar_CreateObj( p_wk, p_unit, mask, clsys_def_type, heapID );

    //ウィンドウ読み込み
    {
      GFL_MSGDATA *p_msg;
      APP_TASKMENU_ITEMWORK item;

      p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_irc_compatible_dat, heapID );

      GFL_STD_MemClear( &item, sizeof(APP_TASKMENU_ITEMWORK) );
      item.msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
      item.str      = GFL_MSG_CreateString( p_msg, COMPATI_BTN_000 );
      item.type     = APP_TASKMENU_WIN_TYPE_RETURN;

      p_wk->p_win       = APP_TASKMENU_WIN_Create( p_wk->p_menu_res, &item, 32-APP_TASKMENU_PLATE_WIDTH, 21, APP_TASKMENU_PLATE_WIDTH, heapID );

      GFL_STR_DeleteBuffer( item.str );
      GFL_MSG_Delete( p_msg );
    }
	}

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	破棄
 *
 *	@param	APPBAR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
void APPBAR_Exit( APPBAR_WORK *p_wk )
{	
  //ウィンドウ破棄
  APP_TASKMENU_WIN_Delete( p_wk->p_win );

	//CLWK
	AppBar_DeleteObj( p_wk );

  //taskmenu用リソース読み込み
  APP_TASKMENU_RES_Delete( p_wk->p_menu_res );

	//リソース破棄
	AppBar_UnLoadResource( p_wk );
	
	//破棄
	GFL_HEAP_FreeMemory( p_wk );

}
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	メイン処理
 *
 *	@param	APPBAR_WORK *p_wk			ワーク
 *	@param	CURSOR_WORK	cp_cursor	カーソルでボタンを押すのを実現させるため
 *
 */
//-----------------------------------------------------------------------------
void APPBAR_Main( APPBAR_WORK *p_wk )
{
  if( APP_TASKMENU_WIN_IsTrg( p_wk->p_win ) )
  { 
    APP_TASKMENU_WIN_SetDecide( p_wk->p_win, TRUE );
    p_wk->trg = APPBAR_ICON_RETURN;
  }
  else
  { 
    p_wk->trg = APPBAR_SELECT_NONE;
  }

  APP_TASKMENU_WIN_Update( p_wk->p_win );
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
APPBAR_ICON APPBAR_GetTrg( const APPBAR_WORK *cp_wk )
{	
	return cp_wk->trg;
}
//=============================================================================
/**
 *			EXTERN
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	リソース読み込み
 *
 *	@param	APPBAR_WORK *p_wk	ワーク
 *	@param	clsys_draw_type		セル読み込みタイプ
 *	@param	PALTYPE bgplttype	パレット読み込み場所
 *	@param	mapping						セルマッピングモード
 *	@param	heapID						読み込みテンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static void AppBar_LoadResource( APPBAR_WORK *p_wk, CLSYS_DRAW_TYPE	clsys_draw_type, PALTYPE bgplttype, APP_COMMON_MAPPING mapping, u8 bg_plt, u8 obj_plt, HEAPID heapID )
{	
#if 0
	ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), heapID );

	//BG
	//領域の確保
	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, APP_COMMON_GetBarPltArcIdx(),
			bgplttype, bg_plt*0x20, APPBAR_BG_PLT_NUM*0x20, heapID );
	//確保した領域に読み込み
	p_wk->chr_pos	= GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, APP_COMMON_GetBarCharArcIdx(), p_wk->bg_frm, APPBAR_BG_CHARAAREA_SIZE, FALSE, heapID );
	GF_ASSERT_MSG( p_wk->chr_pos != GFL_ARCUTIL_TRANSINFO_FAIL, "APPBAR:BGキャラ領域が足りませんでした\n" );
	//スクリーンはメモリ上に置いて、下部32*3だけ書き込み
	ARCHDL_UTIL_TransVramScreenEx( p_handle, APP_COMMON_GetBarScrnArcIdx(), p_wk->bg_frm,
			GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->chr_pos), APPBAR_MENUBAR_X, APPBAR_MENUBAR_Y, 32, 24, 
			APPBAR_MENUBAR_X, APPBAR_MENUBAR_Y, APPBAR_MENUBAR_W, APPBAR_MENUBAR_H,
			bg_plt, FALSE, heapID );


	//OBJ
	//共通アイコンリソース	
	p_wk->res[APPBAR_RES_COMMON_PLT]	= GFL_CLGRP_PLTT_RegisterEx( p_handle,
			APP_COMMON_GetBarIconPltArcIdx(), clsys_draw_type, obj_plt*0x20, 0, APPBAR_OBJ_PLT_NUM, heapID );	
	p_wk->res[APPBAR_RES_COMMON_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
			APP_COMMON_GetBarIconCharArcIdx(), FALSE, clsys_draw_type, heapID );

	p_wk->res[APPBAR_RES_COMMON_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
			APP_COMMON_GetBarIconCellArcIdx(mapping),
			APP_COMMON_GetBarIconAnimeArcIdx(mapping), heapID );

	GFL_ARC_CloseDataHandle( p_handle );
#endif

	ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_IRCCOMPATIBLE, heapID );

  p_wk->res[APPBAR_RES_BAR_PLT]	= GFL_CLGRP_PLTT_RegisterEx( p_handle,
			NARC_irccompatible_gra_shita_bar_NCLR, clsys_draw_type, obj_plt*0x20, 0, 1, heapID );	
	p_wk->res[APPBAR_RES_BAR_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
			NARC_irccompatible_gra_shita_bar_NCGR, FALSE, clsys_draw_type, heapID );

	p_wk->res[APPBAR_RES_BAR_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
			NARC_irccompatible_gra_shita_bar_NCER,
			NARC_irccompatible_gra_shita_bar_NANR, heapID );

	GFL_ARC_CloseDataHandle( p_handle );

}

//----------------------------------------------------------------------------
/**
 *	@brief	リソース破棄
 *	
 *	@param	APPBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void AppBar_UnLoadResource( APPBAR_WORK *p_wk )
{	
#if 0
	//OBJ
	GFL_CLGRP_CELLANIM_Release( p_wk->res[APPBAR_RES_COMMON_CEL] );
	GFL_CLGRP_CGR_Release( p_wk->res[APPBAR_RES_COMMON_CHR] );
	GFL_CLGRP_PLTT_Release( p_wk->res[APPBAR_RES_COMMON_PLT] );

	//BG
	GFL_BG_FreeCharacterArea(p_wk->bg_frm,
			GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->chr_pos),
			GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->chr_pos));
#endif
	GFL_CLGRP_CELLANIM_Release( p_wk->res[APPBAR_RES_BAR_CEL] );
	GFL_CLGRP_CGR_Release( p_wk->res[APPBAR_RES_BAR_CHR] );
	GFL_CLGRP_PLTT_Release( p_wk->res[APPBAR_RES_BAR_PLT] );
}

//----------------------------------------------------------------------------
/**
 *	@brief	OBJ作成
 *
 *	@param	APPBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void AppBar_CreateObj( APPBAR_WORK *p_wk, GFL_CLUNIT* p_unit, APPBAR_OPTION_MASK mask, CLSYS_DEFREND_TYPE	clsys_def_type, HEAPID heapID )
{	
	//CLWKの作成
	int i;
	GFL_CLWK_DATA				cldata;

	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

  cldata.pos_x	= 128;
  cldata.pos_y	= 96;
  cldata.anmseq	= 0;
  cldata.bgpri  = 1;
  p_wk->p_clwk[0]	= GFL_CLACT_WK_Create( p_unit, 
      p_wk->res[APPBAR_RES_BAR_CHR],
      p_wk->res[APPBAR_RES_BAR_PLT],
      p_wk->res[APPBAR_RES_BAR_CEL],
      &cldata,
      clsys_def_type,
      heapID
      );
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ破棄
 *
 *	@param	APPBAR_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void AppBar_DeleteObj( APPBAR_WORK *p_wk )
{	
	int i;
	for( i = 0; i < APPBAR_ICON_MAX; i++ )
	{	
		if( p_wk->p_clwk[i] )
		{	
			GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
		}
	}
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

