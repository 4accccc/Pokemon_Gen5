//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		touchbar.c
 *	@brief	アプリ用下画面タッチバー
 *	@author	Toru=Nagihashi
 *	@data		2009.09.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>

//mine
#include "touchbar.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	TOUCHBAR
//=====================================
//リソース
enum
{	
	TOUCHBAR_RES_COMMON_PLT,
	TOUCHBAR_RES_COMMON_CHR,
	TOUCHBAR_RES_COMMON_CEL,

	TOUCHBAR_RES_MAX
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	下画面バー(アプリケーションバーと勝手に命名)
//=====================================
struct _TOUCHBAR_WORK
{
	GFL_CLWK	*p_clwk[APBAR_ICON_REGISTER_MAX];
	u32				res[TOUCHBAR_RES_MAX];
	u32				bg_frm;
	GFL_ARCUTIL_TRANSINFO				chr_pos;
	s32				trg;
	s32				cont;
	u32				tbl_len;
	TOUCHBAR_ITEM_ICON	setup_tbl[0];
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//リソース
static void TouchBar_LoadResource( TOUCHBAR_WORK *p_wk, CLSYS_DRAW_TYPE	clsys_draw_type, PALTYPE bgplttype, APP_COMMON_MAPPING mapping, u8 bg_plt, u8 obj_plt, HEAPID heapID );
static void TouchBar_UnLoadResource( TOUCHBAR_WORK *p_wk );
//obj
static void TouchBar_CreateObj( TOUCHBAR_WORK *p_wk, GFL_CLUNIT* p_unit, const TOUCHBAR_ITEM_ICON *cp_setup_tbl, u16 tbl_len, CLSYS_DEFREND_TYPE	clsys_def_type, HEAPID heapID );
static void TouchBar_DeleteObj( TOUCHBAR_WORK *p_wk );
//etc
static void ARCHDL_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h,  u8 plt, BOOL compressedFlag, HEAPID heapID );

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	アニメシーケンステーブル
//	TOUCHBAR_ICONと対応したテーブル
//=====================================
static const u16 sc_anmseq_tbl[]	=
{	
	APP_COMMON_BARICON_EXIT,
	APP_COMMON_BARICON_RETURN,
	APP_COMMON_BARICON_CURSOR_DOWN,
	APP_COMMON_BARICON_CURSOR_UP,
	APP_COMMON_BARICON_CURSOR_LEFT,
	APP_COMMON_BARICON_CURSOR_RIGHT,
	APP_COMMON_BARICON_CHECK_OFF,
};

//=============================================================================
/**
 *					GLOBAL
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	初期化拡張版（アイコン位置指定版）
 *
 *	@param	const TOUCHBAR_SETUP *cp_setup	設定情報
 *	@param	heapID	ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
extern TOUCHBAR_WORK * TOUCHBAR_Init( TOUCHBAR_SETUP *p_setup, HEAPID heapID )
{	
	TOUCHBAR_WORK	*p_wk;


	//ワーク作成、初期化
	{	
		u32 size;

		size	= sizeof(TOUCHBAR_WORK) + sizeof(TOUCHBAR_ITEM_ICON) * p_setup->item_num;
		p_wk	= GFL_HEAP_AllocMemory( heapID, size );
		GFL_STD_MemClear( p_wk, size );
		p_wk->bg_frm	= p_setup->bar_frm;
		p_wk->trg			= TOUCHBAR_SELECT_NONE;
		p_wk->cont		= TOUCHBAR_SELECT_NONE;
		p_wk->tbl_len	= p_setup->item_num;

		GFL_STD_MemCopy( p_setup->p_item, p_wk->setup_tbl, sizeof(TOUCHBAR_ITEM_ICON)*p_setup->item_num );
	}

	{	
		CLSYS_DRAW_TYPE			clsys_draw_type;
		CLSYS_DEFREND_TYPE	clsys_def_type;
		PALTYPE							bgplttype;

		//OBJ読み込む場所をチェック
		if( p_setup->bar_frm >= GFL_BG_FRAME0_S )
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
		TouchBar_LoadResource( p_wk, clsys_draw_type, bgplttype, 
				p_setup->mapping, p_setup->bg_plt, p_setup->obj_plt, heapID );

		//CLWK読み込み
		TouchBar_CreateObj( p_wk, p_setup->p_unit, 
				p_setup->p_item, p_setup->item_num, clsys_def_type, heapID );
	}

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR	破棄
 *
 *	@param	TOUCHBAR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
void TOUCHBAR_Exit( TOUCHBAR_WORK *p_wk )
{	
	//CLWK
	TouchBar_DeleteObj( p_wk );

	//リソース破棄
	TouchBar_UnLoadResource( p_wk );
	
	//破棄
	GFL_HEAP_FreeMemory( p_wk );

}
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR	メイン処理
 *
 *	@param	TOUCHBAR_WORK *p_wk			ワーク
 *	@param	CURSOR_WORK	cp_cursor	カーソルでボタンを押すのを実現させるため
 *
 */
//-----------------------------------------------------------------------------
void TOUCHBAR_Main( TOUCHBAR_WORK *p_wk )
{	

	
	//タッチの動き
	int i;
	u32 x, y;

	p_wk->trg		= TOUCHBAR_SELECT_NONE;
	p_wk->cont	= TOUCHBAR_SELECT_NONE;
	for( i = 0; i < p_wk->tbl_len; i++ )
	{	
		const TOUCHBAR_ITEM_ICON *cp_setup	= &p_wk->setup_tbl[i];

		if( GFL_UI_TP_GetPointTrg( &x, &y ) )
		{	
			if( ((u32)( x - cp_setup->pos.x) <= (u32)(TOUCHBAR_ICON_WIDTH))
					&	((u32)( y - cp_setup->pos.y) <= (u32)(TOUCHBAR_ICON_HEIGHT)))
			{
				p_wk->trg	= cp_setup->icon;
			}
		}
		else if( GFL_UI_TP_GetPointCont( &x, &y ) )
		{	
			if( ((u32)( x - cp_setup->pos.x) <= (u32)(TOUCHBAR_ICON_WIDTH))
					&	((u32)( y - cp_setup->pos.y) <= (u32)(TOUCHBAR_ICON_HEIGHT)))
			{
				p_wk->cont	= cp_setup->icon;
			}
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR	選択されたものを取得
 *
 *	@param	const TOUCHBAR_WORK *cp_wk ワーク
 *
 *	@return	TOUCHBAR_SELECT列挙
 */
//-----------------------------------------------------------------------------
TOUCHBAR_ICON TOUCHBAR_GetTrg( const TOUCHBAR_WORK *cp_wk )
{	
	return cp_wk->trg;
}
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR	選択されたものを取得
 *
 *	@param	const TOUCHBAR_WORK *cp_wk ワーク
 *
 *	@return	TOUCHBAR_SELECT列挙
 */
//-----------------------------------------------------------------------------
TOUCHBAR_ICON TOUCHBAR_GetCont( const TOUCHBAR_WORK *cp_wk )
{	
	return cp_wk->cont;
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
 *	@param	TOUCHBAR_WORK *p_wk	ワーク
 *	@param	clsys_draw_type		セル読み込みタイプ
 *	@param	PALTYPE bgplttype	パレット読み込み場所
 *	@param	mapping						セルマッピングモード
 *	@param	heapID						読み込みテンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static void TouchBar_LoadResource( TOUCHBAR_WORK *p_wk, CLSYS_DRAW_TYPE	clsys_draw_type, PALTYPE bgplttype, APP_COMMON_MAPPING mapping, u8 bg_plt, u8 obj_plt, HEAPID heapID )
{	
	ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), heapID );

	//BG
	//領域の確保
	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, APP_COMMON_GetBarPltArcIdx(),
			bgplttype, bg_plt*0x20, TOUCHBAR_BG_PLT_NUM*0x20, heapID );
	//確保した領域に読み込み
	p_wk->chr_pos	= GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, APP_COMMON_GetBarCharArcIdx(), p_wk->bg_frm, TOUCHBAR_BG_CHARAAREA_SIZE, FALSE, heapID );
	GF_ASSERT_MSG( p_wk->chr_pos != GFL_ARCUTIL_TRANSINFO_FAIL, "TOUCHBAR:BGキャラ領域が足りませんでした\n" );
	//スクリーンはメモリ上に置いて、下部32*3だけ書き込み
	ARCHDL_UTIL_TransVramScreenEx( p_handle, APP_COMMON_GetBarScrnArcIdx(), p_wk->bg_frm,
			GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->chr_pos), TOUCHBAR_MENUBAR_X, TOUCHBAR_MENUBAR_Y, 32, 24, 
			TOUCHBAR_MENUBAR_X, TOUCHBAR_MENUBAR_Y, TOUCHBAR_MENUBAR_W, TOUCHBAR_MENUBAR_H,
			bg_plt, FALSE, heapID );


	//OBJ
	//共通アイコンリソース	
	p_wk->res[TOUCHBAR_RES_COMMON_PLT]	= GFL_CLGRP_PLTT_RegisterEx( p_handle,
			APP_COMMON_GetBarIconPltArcIdx(), clsys_draw_type, obj_plt*0x20, 0, TOUCHBAR_OBJ_PLT_NUM, heapID );	
	p_wk->res[TOUCHBAR_RES_COMMON_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
			APP_COMMON_GetBarIconCharArcIdx(), FALSE, clsys_draw_type, heapID );

	p_wk->res[TOUCHBAR_RES_COMMON_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
			APP_COMMON_GetBarIconCellArcIdx(mapping),
			APP_COMMON_GetBarIconAnimeArcIdx(mapping), heapID );

	GFL_ARC_CloseDataHandle( p_handle );
}

//----------------------------------------------------------------------------
/**
 *	@brief	リソース破棄
 *	
 *	@param	TOUCHBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void TouchBar_UnLoadResource( TOUCHBAR_WORK *p_wk )
{	
	//OBJ
	GFL_CLGRP_CELLANIM_Release( p_wk->res[TOUCHBAR_RES_COMMON_CEL] );
	GFL_CLGRP_CGR_Release( p_wk->res[TOUCHBAR_RES_COMMON_CHR] );
	GFL_CLGRP_PLTT_Release( p_wk->res[TOUCHBAR_RES_COMMON_PLT] );

	//BG
	GFL_BG_FreeCharacterArea(p_wk->bg_frm,
			GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->chr_pos),
			GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->chr_pos));
}

//----------------------------------------------------------------------------
/**
 *	@brief	OBJ作成
 *
 *	@param	TOUCHBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void TouchBar_CreateObj( TOUCHBAR_WORK *p_wk, GFL_CLUNIT* p_unit, const TOUCHBAR_ITEM_ICON *cp_setup_tbl, u16 tbl_len, CLSYS_DEFREND_TYPE	clsys_def_type, HEAPID heapID )
{	
	//CLWKの作成
	int i;
	GFL_CLWK_DATA				cldata;
	const TOUCHBAR_ITEM_ICON	*cp_setup;

	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

	for( i = 0; i < tbl_len; i++ )
	{	
		cp_setup	= &cp_setup_tbl[i];
		GF_ASSERT( cp_setup->icon < TOUCHBAR_ICON_MAX );
		GF_ASSERT_MSG( i < APBAR_ICON_REGISTER_MAX, "TOUCHBAR：アイコン登録個数が超えています now%d max%d\n", i, APBAR_ICON_REGISTER_MAX );

		cldata.pos_x	= cp_setup->pos.x;
		cldata.pos_y	= cp_setup->pos.y;
		cldata.anmseq	= sc_anmseq_tbl[ cp_setup->icon ];
		//clwkはTOUCHBAR_ICON順にしておく
		GF_ASSERT_MSG( p_wk->p_clwk[cp_setup->icon] == NULL, "TOUCHBAR:SETUP構造体に同じアイコンを定義しています\n" );
		p_wk->p_clwk[cp_setup->icon]	= GFL_CLACT_WK_Create( p_unit, 
				p_wk->res[TOUCHBAR_RES_COMMON_CHR],
				p_wk->res[TOUCHBAR_RES_COMMON_PLT],
				p_wk->res[TOUCHBAR_RES_COMMON_CEL],
				&cldata,
				clsys_def_type,
				heapID
				);
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ破棄
 *
 *	@param	TOUCHBAR_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void TouchBar_DeleteObj( TOUCHBAR_WORK *p_wk )
{	
	int i;
	for( i = 0; i < APBAR_ICON_REGISTER_MAX; i++ )
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


