//============================================================================================
/**
 * @file		b_app_tool.c
 * @brief		戦闘用アプリ共通処理
 * @author	Hiroyuki Nakamura
 * @date		09.08.25
 */
//============================================================================================
#include <gflib.h>

#include "b_app_tool.h"


//============================================================================================
//	定数定義
//============================================================================================

// カーソル表示ワーク
struct _BAPP_CURSOR_PUT_WORK {
	GFL_CLWK * clwk[BAPPTOOL_CURSOR_MAX];
	HEAPID	heapID;
	BOOL	vanish;
};

#define	BATTLE_CURSOR_ANIME		( 0 )		// カーソルＯＢＪのアニメ番号


//============================================================================================
//	カーソル関連
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル表示ワーク作成
 *
 * @param		heapID		ヒープＩＤ
 *
 * @return	カーソル表示ワーク
 */
//--------------------------------------------------------------------------------------------
BAPP_CURSOR_PUT_WORK * BAPPTOOL_CreateCursor( HEAPID heapID )
{
	BAPP_CURSOR_PUT_WORK * wk = GFL_HEAP_AllocMemory( heapID, sizeof(BAPP_CURSOR_PUT_WORK) );
	wk->heapID = heapID;
	wk->vanish = TRUE;
	return wk;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル表示ワーク開放
 *
 * @param		wk		カーソル表示ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BAPPTOOL_FreeCursor( BAPP_CURSOR_PUT_WORK * wk )
{
	GFL_HEAP_FreeMemory( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル追加
 *
 * @param		wk			カーソル表示ワーク
 * @param		clunit	セルアクターユニット
 * @param		chrRes	キャラリソース
 * @param		palRes	パレットリソース
 * @param		celRes	セルリソース
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BAPPTOOL_AddCursor( BAPP_CURSOR_PUT_WORK * wk, GFL_CLUNIT * clunit, u32 chrRes, u32 palRes, u32 celRes )
{
	GFL_CLWK_DATA	dat;
	u32	i;

	dat.pos_x   = 0;
	dat.pos_y   = 0;
	dat.softpri = 0;
	dat.bgpri   = 1;

	for( i=0; i<BAPPTOOL_CURSOR_MAX; i++ ){
		dat.anmseq = BATTLE_CURSOR_ANIME + i;
		wk->clwk[i] = GFL_CLACT_WK_Create( clunit, chrRes, palRes, celRes, &dat, CLSYS_DRAW_SUB, wk->heapID );
		GFL_CLACT_WK_SetPlttOffs( wk->clwk[i], 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[i], TRUE );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル削除
 *
 * @param		wk			カーソル表示ワーク
 * @param		clunit	セルアクターユニット
 * @param		chrRes	キャラリソース
 * @param		palRes	パレットリソース
 * @param		celRes	セルリソース
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BAPPTOOL_DelCursor( BAPP_CURSOR_PUT_WORK * wk )
{
	u32	i;

	for( i=0; i<BAPPTOOL_CURSOR_MAX; i++ ){
		GFL_CLACT_WK_Remove( wk->clwk[i] );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル表示切り替え
 *
 * @param		wk			カーソル表示ワーク
 * @param		flg			TRUE = 表示, FALSE = 非表示
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BAPPTOOL_VanishCursor( BAPP_CURSOR_PUT_WORK * wk, BOOL flg )
{
	if( wk->vanish != flg ){
		u32	i;

		for( i=0; i<BAPPTOOL_CURSOR_MAX; i++ ){
			GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], flg );
		}
		wk->vanish = flg;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動
 *
 * @param		wk			カーソル表示ワーク
 * @param		dat			座標データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BAPPTOOL_MoveCursorPoint( BAPP_CURSOR_PUT_WORK * wk, const CURSORMOVE_DATA * dat )
{
	GFL_CLACTPOS	pos;
	u8	sx, sy;

	sx = dat->sx >> 1;
	sy = dat->sy >> 1;

	// 左上
	pos.x = dat->px - sx;
	pos.y = dat->py - sy;
	GFL_CLACT_WK_SetPos( wk->clwk[0], &pos, CLSYS_DRAW_SUB );
	// 左下
	pos.x = dat->px - sx;
	pos.y = dat->py + sy;
	GFL_CLACT_WK_SetPos( wk->clwk[1], &pos, CLSYS_DRAW_SUB );
	// 右上
	pos.x = dat->px + sx;
	pos.y = dat->py - sy;
	GFL_CLACT_WK_SetPos( wk->clwk[2], &pos, CLSYS_DRAW_SUB );
	// 右下
	pos.x = dat->px + sx;
	pos.y = dat->py + sy;
	GFL_CLACT_WK_SetPos( wk->clwk[3], &pos, CLSYS_DRAW_SUB );
}


//============================================================================================
//	文字表示関連
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		BMPウィンドウスクリーン転送（単発）
 *
 * @param		util		PRINT_UTIL
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BAPPTOOL_PrintScreenTrans( PRINT_UTIL * util )
{
	GFL_BMPWIN_MakeScreen( util->win );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(util->win) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BMPウィンドウスクリーン転送（テーブル参照）
 *
 * @param		util		PRINT_UTIL
 * @param		tbl			データ番号テーブル
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BAPPTOOL_SetStrScrn( PRINT_UTIL * util, const u8 * tbl )
{
	u32	i = 0;

	while(1){
		if( tbl[i] == BAPPTOOL_SET_STR_SCRN_END ){
			break;
		}
		BAPPTOOL_PrintScreenTrans( &util[tbl[i]] );
		i++;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列転送
 *
 * @param		util		PRINT_UTIL
 * @param		que			PRINT_QUE
 * @param		max			PRINT_UTILの数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BAPPTOOL_PrintUtilTrans( PRINT_UTIL * util, PRINT_QUE * que, u32 max )
{
	u32	i;

	PRINTSYS_QUE_Main( que );
	for( i=0; i<max; i++ ){
		PRINT_UTIL_Trans( &util[i], que );
	}
}
