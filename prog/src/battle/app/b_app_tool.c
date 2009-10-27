//============================================================================================
/**
 * @file	b_app_tool.c
 * @brief	戦闘用アプリ共通処理
 * @author	Hiroyuki Nakamura
 * @date	09.08.25
 */
//============================================================================================
#include <gflib.h>

//#include "system/clact_tool.h"
/*↑[GS_CONVERT_TAG]*/
//#include "battle_snd_def.h"


#include "b_app_tool.h"


//============================================================================================
//	定数定義
//============================================================================================

//#define	BAPP_CURSOR_MAX		( 4 )

/*
// 選択カーソル移動ワーク
struct _BAPP_CURSOR_MVWK {
//	BCURSOR_PTR	cursor;			// カーソルデータ
//	const POINTSEL_WORK * cp_wk;	// カーソル位置ワーク
	u8	cur_flg;				// カーソルON/OFF
	u8	cur_pos;				// カーソル位置
	u8	old_pos;				// 前回のカーソル位置
	u32	mv_tbl;					// 移動テーブル
};
*/

struct _BAPP_CURSOR_PUT_WORK {
	GFL_CLWK * clwk[BAPPTOOL_CURSOR_MAX];
//	const BAPP_CURSOR_PUT * put;
	HEAPID	heapID;
	BOOL	vanish;
};


// キー全部
#define	PAD_KEY_ALL	( PAD_KEY_UP | PAD_KEY_DOWN | PAD_KEY_LEFT | PAD_KEY_RIGHT )


//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク作成
 *
 * @param	heap	ヒープID
 *
 * @return	カーソル移動ワーク
 */
//--------------------------------------------------------------------------------------------
/*
BAPP_CURSOR_MVWK * BAPP_CursorMoveWorkAlloc( u32 heap )
{
	BAPP_CURSOR_MVWK * wk = GFL_HEAP_AllocClearMemory( heap, sizeof(BAPP_CURSOR_MVWK) );
	return wk;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク解放
 *
 * @param	heap	ヒープID
 *
 * @return	カーソル移動ワーク
 */
//--------------------------------------------------------------------------------------------
/*
void BAPP_CursorMoveWorkFree( BAPP_CURSOR_MVWK * wk )
{
	GFL_HEAP_FreeMemory( wk );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク取得：カーソルデータ
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @return	カーソルデータ
 */
//--------------------------------------------------------------------------------------------
/*
BCURSOR_PTR BAPP_CursorMvWkGetBCURSOR_PTR( BAPP_CURSOR_MVWK * wk )
{
	return wk->cursor;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク取得：カーソル位置
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @return	カーソル位置
 */
//--------------------------------------------------------------------------------------------
/*
u8 BAPP_CursorMvWkGetPos( BAPP_CURSOR_MVWK * wk )
{
	return wk->cur_pos;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク取得：カーソルON/OFF
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @return	カーソルON/OFF
 */
//--------------------------------------------------------------------------------------------
/*
u8 BAPP_CursorMvWkGetFlag( BAPP_CURSOR_MVWK * wk )
{
	return wk->cur_flg;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワークセット：カーソルON/OFF
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	flg		ON/OFFフラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BAPP_CursorMvWkSetFlag( BAPP_CURSOR_MVWK * wk, u8 flg )
{
	wk->cur_flg = flg;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワークセット：カーソルデータ
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	flg		カーソルデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BAPP_CursorMvWkSetBCURSOR_PTR( BAPP_CURSOR_MVWK * wk, BCURSOR_PTR cur )
{
	wk->cursor = cur;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワークセット：カーソル位置
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	pos		カーソル位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BAPP_CursorMvWkSetPos( BAPP_CURSOR_MVWK * wk, u8 pos )
{
	wk->cur_pos = pos;

	if( wk->cur_flg == 1 ){
		BCURSOR_PosSetON(
			wk->cursor,
			wk->cp_wk[wk->cur_pos].px, wk->cp_wk[wk->cur_pos].sx,
			wk->cp_wk[wk->cur_pos].py, wk->cp_wk[wk->cur_pos].sy );
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル位置初期化
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BAPP_CursorMvWkPosInit( BAPP_CURSOR_MVWK * wk )
{
	wk->cur_pos = 0;
	wk->old_pos = 0xff;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワークセット：カーソル位置ワーク
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	pwk		カーソル位置ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BAPP_CursorMvWkSetPoint( BAPP_CURSOR_MVWK * wk, const POINTSEL_WORK * pwk )
{
	BAPP_CursorMvWkPosInit( wk );
	wk->cp_wk  = pwk;
	wk->mv_tbl = 0xffffffff;

	if( wk->cur_flg == 1 ){
		BCURSOR_PosSetON(
			wk->cursor, wk->cp_wk[0].px, wk->cp_wk[0].sx, wk->cp_wk[0].py, wk->cp_wk[0].sy );
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワークセット：移動テーブル
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	mv_tbl	移動テーブル
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
void BAPP_CursorMvWkSetMvTbl( BAPP_CURSOR_MVWK * wk, u32 mv_tbl )
{
	wk->mv_tbl = mv_tbl;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル表示
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @retval	"TRUE = 表示"
 * @retval	"FALSE = 非表示"
 */
//--------------------------------------------------------------------------------------------
/*
static u8 BAPP_CursorOnOffCheck( BAPP_CURSOR_MVWK * wk )
{
	if( wk->cur_flg == 1 ){
		return TRUE;
	}

	if( GFL_UI_KEY_GetTrg() & ( PAD_KEY_ALL | PAD_BUTTON_CANCEL | PAD_BUTTON_DECIDE ) ){
		wk->cur_flg = 1;
		BCURSOR_PosSetON(
			wk->cursor,
			wk->cp_wk[ wk->cur_pos ].px, wk->cp_wk[ wk->cur_pos ].sx,
			wk->cp_wk[ wk->cur_pos ].py, wk->cp_wk[ wk->cur_pos ].sy );
//	Snd_SePlay( BCURSOR_MOVE_SE );
	}
	return FALSE;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 前のカーソル位置に移動するか
 *
 * @param	wk		選択カーソル移動ワーク
 * @param	mv		移動方向
 *
 * @retval	"TRUE = 移動する"
 * @retval	"FALSE = 移動しない"
 */
//--------------------------------------------------------------------------------------------
/*
static u8 BAPP_OldCursorSetCheck( const POINTSEL_WORK * pw, u8 mv )
{
	switch( mv ){
	case POINTSEL_MV_UP:
		if( pw->down & BAPP_CMV_RETBIT ){
			return TRUE;
		}
		break;
	case POINTSEL_MV_DOWN:
		if( pw->up & BAPP_CMV_RETBIT ){
			return TRUE;
		}
		break;
	case POINTSEL_MV_LEFT:
		if( pw->right & BAPP_CMV_RETBIT ){
			return TRUE;
		}
		break;
	case POINTSEL_MV_RIGHT:
		if( pw->left & BAPP_CMV_RETBIT ){
			return TRUE;
		}
		break;
	}
	return FALSE;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動
 *
 * @param	wk		選択カーソル移動ワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
/*
u32 BAPP_CursorMove( BAPP_CURSOR_MVWK * wk )
{
	u8	lx, ly, rx, ry;
	u8	pm_ret;
	u8	mv;

	if( BAPP_CursorOnOffCheck( wk ) == FALSE ){ return BAPP_CMV_NONE; }

	if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		pm_ret = POINTSEL_MoveVec( wk->cp_wk,NULL,NULL,NULL,NULL,wk->cur_pos,POINTSEL_MV_UP );
		mv     = POINTSEL_MV_UP;
	}else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
		pm_ret = POINTSEL_MoveVec( wk->cp_wk,NULL,NULL,NULL,NULL,wk->cur_pos,POINTSEL_MV_DOWN );
		mv     = POINTSEL_MV_DOWN;
	}else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		pm_ret = POINTSEL_MoveVec( wk->cp_wk,NULL,NULL,NULL,NULL,wk->cur_pos,POINTSEL_MV_LEFT );
		mv     = POINTSEL_MV_LEFT;
	}else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		pm_ret = POINTSEL_MoveVec( wk->cp_wk,NULL,NULL,NULL,NULL,wk->cur_pos,POINTSEL_MV_RIGHT );
		mv     = POINTSEL_MV_RIGHT;
	}else{
		pm_ret = BAPP_CMV_NONE;
	}

	if( pm_ret != POINTSEL_MOVE_NONE ){
		u8	old_save = 1;

		if( pm_ret & BAPP_CMV_RETBIT ){
			if( wk->old_pos != 0xff ){
				pm_ret = wk->old_pos;
			}else{
				pm_ret ^= BAPP_CMV_RETBIT;
			}
		}

		while(1){
			u8	tmp;
			if( wk->mv_tbl & (1<<pm_ret) ){
				break;
			}
			old_save = 0;
			tmp = POINTSEL_MoveVec( wk->cp_wk,NULL,NULL,NULL,NULL,pm_ret,mv )&(0xff^BAPP_CMV_RETBIT);
			if( tmp == pm_ret || tmp == wk->cur_pos ){
				pm_ret = wk->cur_pos;
				break;
			}
			pm_ret = tmp;
		}

		if( wk->cur_pos != pm_ret ){
			POINTSEL_GetPos( &wk->cp_wk[pm_ret], &lx, &ly );
			POINTSEL_GetSize( &wk->cp_wk[pm_ret], &rx, &ry );

			if( BAPP_OldCursorSetCheck(&wk->cp_wk[pm_ret],mv) == TRUE && old_save != 0 ){
				wk->old_pos = wk->cur_pos;
			}else{
				wk->old_pos = 0xff;
			}

			wk->cur_pos = pm_ret;
			BCURSOR_PosSetON( wk->cursor, lx, rx, ly, ry );
//		Snd_SePlay( BCURSOR_MOVE_SE );
		}
		return BAPP_CMV_NONE;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
		return wk->cur_pos;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
//	Snd_SePlay( SEQ_SE_DP_DECIDE );
		return BAPP_CMV_CANCEL;
	}

	return BAPP_CMV_NONE;
}
*/


#define	BATTLE_CURSOR_ANIME		( 0 )

BAPP_CURSOR_PUT_WORK * BAPPTOOL_CreateCursor( HEAPID heapID )
{
	BAPP_CURSOR_PUT_WORK * wk = GFL_HEAP_AllocMemory( heapID, sizeof(BAPP_CURSOR_PUT_WORK) );
	wk->heapID = heapID;
	wk->vanish = TRUE;
	return wk;
}

void BAPPTOOL_FreeCursor( BAPP_CURSOR_PUT_WORK * wk )
{
	GFL_HEAP_FreeMemory( wk );
}

/*
void BAPPTOOL_SetCursorPutData( BAPP_CURSOR_PUT_WORK * wk, const BAPP_CURSOR_PUT * put )
{
	wk->put = put;
}
*/

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

void BAPPTOOL_DelCursor( BAPP_CURSOR_PUT_WORK * wk )
{
	u32	i;

	for( i=0; i<BAPPTOOL_CURSOR_MAX; i++ ){
		GFL_CLACT_WK_Remove( wk->clwk[i] );
	}
}

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

/*
void BAPPTOOL_MoveCursor( BAPP_CURSOR_PUT_WORK * wk, u32 point )
{
	GFL_CLACTPOS	pos;
	u8	sx, sy;

	sx = wk->put[point].sx >> 1;
	sy = wk->put[point].sy >> 1;

	// 左上
	pos.x = wk->put[point].px - sx;
	pos.y = wk->put[point].py - sy;
	GFL_CLACT_WK_SetPos( wk->clwk[0], &pos, CLSYS_DRAW_SUB );
	// 左下
	pos.x = wk->put[point].px - sx;
	pos.y = wk->put[point].py + sy;
	GFL_CLACT_WK_SetPos( wk->clwk[1], &pos, CLSYS_DRAW_SUB );
	// 右上
	pos.x = wk->put[point].px + sx;
	pos.y = wk->put[point].py - sy;
	GFL_CLACT_WK_SetPos( wk->clwk[2], &pos, CLSYS_DRAW_SUB );
	// 右下
	pos.x = wk->put[point].px + sx;
	pos.y = wk->put[point].py + sy;
	GFL_CLACT_WK_SetPos( wk->clwk[3], &pos, CLSYS_DRAW_SUB );
}
*/

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










void BAPPTOOL_PrintScreenTrans( PRINT_UTIL * util )
{
	GFL_BMPWIN_MakeScreen( util->win );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(util->win) );
}

void BAPPTOOL_PrintUtilTrans( PRINT_UTIL * util, PRINT_QUE * que, u32 max )
{
	u32	i;

	PRINTSYS_QUE_Main( que );
	for( i=0; i<max; i++ ){
		PRINT_UTIL_Trans( &util[i], que );
	}
}
