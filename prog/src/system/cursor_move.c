//============================================================================================
/**
 * @file	cursor_move.c
 * @brief	カーソル移動処理
 * @author	Hiroyuki Nakamura
 * @date	08.08.06
 */
//============================================================================================
#include <gflib.h>

#include "sound/pm_sndsys.h"
#include "system/cursor_move.h"


//============================================================================================
//	定数定義
//============================================================================================

// 選択カーソル移動ワーク
struct _CURSOR_MOVE_WORK {
	const GFL_UI_TP_HITTBL * rect;			// タッチテーブル
	const POINTER_WORK * pw;			// カーソル位置ワーク
	BOOL cur_flg;						// カーソルON/OFF
	u8	cur_mode;						// カーソル表示モード
	u8	cur_pos;						// カーソル位置
	u8	old_pos;						// 前回のカーソル位置
	u8	save_pos;						// カーソル位置記憶
	u32	mv_tbl[2];						// 移動テーブル
	const CURSORMOVE_CALLBACK * func;	// コールバック関数
	void * work;						// コールバック関数に渡すワーク
};

// カーソル表示モード
enum {
	CURSOR_MODE_NORMAL = 0,		// 常に表示
	CURSOR_MODE_VANISH,			// キー操作によって表示切替
};

// キー全部
#define	PAD_KEY_ALL	( PAD_KEY_UP | PAD_KEY_DOWN | PAD_KEY_LEFT | PAD_KEY_RIGHT )

#define	DEF_OLDPOS		( 0xff )			// 前回のカーソル位置なし
#define	DEF_MVTBL		( 0xffffffff )		// デフォルトの移動テーブル


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static u32 Main_TouchCheck( CURSOR_MOVE_WORK * wk, int hit );
static u32 Main_CursorOn( CURSOR_MOVE_WORK * wk );
static u32 Main_KeyMove( CURSOR_MOVE_WORK * wk, u8 mv, u8 ret );

static BOOL OldCursorSetCheck( const POINTER_WORK * pw, u8 mv );
static BOOL MoveTableBitCheck( u32 * tbl, u32 pos );



//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク作成
 *
 * @param	rect		タッチ判定テーブル
 * @param	pw			カーソル位置ワーク
 * @param	func		コールバック関数
 * @param	work		コールバック関数に渡されるワーク
 * @param	cur_flg		カーソル表示フラグ
 * @param	cur_pos		カーソル位置
 * @param	heapID		ヒープID
 *
 * @return	カーソル移動ワーク
 *
 *	デフォルトはカーソルを常に表示
 */
//--------------------------------------------------------------------------------------------
CURSOR_MOVE_WORK * CURSORMOVE_Create(
					const GFL_UI_TP_HITTBL * rect,
					const POINTER_WORK * pw,
					const CURSORMOVE_CALLBACK * func,
					void * work,
					BOOL cur_flg,
					u8	cur_pos,
					u32 heapID )
{
	CURSOR_MOVE_WORK * wk = GFL_HEAP_AllocMemory( heapID, sizeof(CURSOR_MOVE_WORK) );

	wk->rect = rect;
	wk->pw   = pw;

	wk->func = func;
	wk->work = work;

	wk->cur_mode = CURSOR_MODE_NORMAL;
	wk->cur_flg  = cur_flg;
	wk->cur_pos  = cur_pos;
	wk->old_pos  = DEF_OLDPOS;
	wk->save_pos = DEF_OLDPOS;

	wk->mv_tbl[0] = DEF_MVTBL;
	wk->mv_tbl[1] = DEF_MVTBL;

	return wk;
}

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動ワーク解放
 *
 * @param	wk		カーソル移動ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CURSORMOVE_Exit( CURSOR_MOVE_WORK * wk )
{
	GFI_HEAP_FreeMemory( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * カーソルモードを非表示ありに設定
 *
 * @param	wk		カーソル移動ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CURSORMOVE_VanishModeSet( CURSOR_MOVE_WORK * wk )
{
	wk->cur_mode = CURSOR_MODE_VANISH;
}

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動（トリガ入力）
 *
 * @param	wk		カーソル移動ワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
u32 CURSORMOVE_Main( CURSOR_MOVE_WORK * wk )
{
	int	hit;
	u16	cnt, flg;
	u8	mv, ret, tmp, old;

/*
	// タッチチェック
	hit = GF_TP_RectHitTrg( wk->rect );

	if( hit != RECT_HIT_NONE ){
		return Main_TouchCheck( wk, hit );
	}
*/
	// タッチチェック
	cnt = 0;
	flg = 0;
	while( 1 ){
		if( wk->rect[cnt].rect.top == GFL_UI_TP_HIT_END ){
			break;
		}
		if( GFL_UI_TP_HitTrg( &wk->rect[cnt] ) == TRUE ){
			hit = Main_TouchCheck( wk, cnt );
			if( hit != CURSORMOVE_NONE ){
				return hit;
			}
			flg = 1;
		}
		cnt++;
	}
	if( flg != 0 ){ return CURSORMOVE_NONE; }

	// カーソル表示チェック
	if( wk->cur_flg == FALSE && wk->cur_mode == CURSOR_MODE_VANISH ){
		if( GFL_UI_KEY_GetTrg() & ( PAD_KEY_ALL | PAD_BUTTON_CANCEL | PAD_BUTTON_DECIDE ) ){
			return Main_CursorOn( wk );
		}
		return CURSORMOVE_NONE;
	}

	// 移動先取得
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		ret = PointerWkMoveSel( wk->pw,NULL,NULL,NULL,NULL,wk->cur_pos,POINT_MV_UP );
		mv  = POINT_MV_UP;
	}else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
		ret = PointerWkMoveSel( wk->pw,NULL,NULL,NULL,NULL,wk->cur_pos,POINT_MV_DOWN );
		mv  = POINT_MV_DOWN;
	}else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		ret = PointerWkMoveSel( wk->pw,NULL,NULL,NULL,NULL,wk->cur_pos,POINT_MV_LEFT );
		mv  = POINT_MV_LEFT;
	}else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		ret = PointerWkMoveSel( wk->pw,NULL,NULL,NULL,NULL,wk->cur_pos,POINT_MV_RIGHT );
		mv  = POINT_MV_RIGHT;
	}else{
		ret = POINT_SEL_NOMOVE;
	}

	// キー移動
	if( ret != POINT_SEL_NOMOVE ){
		return Main_KeyMove( wk, mv, ret );
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
//		Snd_SePlay( SEQ_SE_DP_DECIDE );		// 各アプリでSEが可変になる可能性がある
		return wk->cur_pos;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
//		Snd_SePlay( SEQ_SE_DP_DECIDE );
		return CURSORMOVE_CANCEL;
	}

	return CURSORMOVE_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * 選択カーソル移動（ベタ入力）
 *
 * @param	wk		カーソル移動ワーク
 *
 * @return	動作結果
 *
 *	ボタン（Ａ、Ｂなど）はトリガ入力
 */
//--------------------------------------------------------------------------------------------
u32 CURSORMOVE_MainCont( CURSOR_MOVE_WORK * wk )
{
	int	hit;
	u16	cnt, flg;
	u8	mv, ret, tmp, old;

/*
	// タッチチェック
	hit = GF_TP_RectHitTrg( wk->rect );

	if( hit != RECT_HIT_NONE ){
		return Main_TouchCheck( wk, hit );
	}
*/
	// タッチチェック
	cnt = 0;
	flg = 0;
	while( 1 ){
		if( wk->rect[cnt].rect.top == GFL_UI_TP_HIT_END ){
			break;
		}
		if( GFL_UI_TP_HitTrg( &wk->rect[cnt] ) == TRUE ){
			hit = Main_TouchCheck( wk, cnt );
			if( hit != CURSORMOVE_NONE ){
				return hit;
			}
			flg = 1;
		}
		cnt++;
	}
	if( flg != 0 ){ return CURSORMOVE_NONE; }

	// カーソル表示チェック
	if( wk->cur_flg == FALSE && wk->cur_mode == CURSOR_MODE_VANISH ){
		if( GFL_UI_KEY_GetTrg() & ( PAD_KEY_ALL | PAD_BUTTON_CANCEL | PAD_BUTTON_DECIDE ) ){
			return Main_CursorOn( wk );
		}
		return CURSORMOVE_NONE;
	}

	// 移動先取得
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP  ){
		ret = PointerWkMoveSel( wk->pw,NULL,NULL,NULL,NULL,wk->cur_pos,POINT_MV_UP );
		mv  = POINT_MV_UP;
	}else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		ret = PointerWkMoveSel( wk->pw,NULL,NULL,NULL,NULL,wk->cur_pos,POINT_MV_DOWN );
		mv  = POINT_MV_DOWN;
	}else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
		ret = PointerWkMoveSel( wk->pw,NULL,NULL,NULL,NULL,wk->cur_pos,POINT_MV_LEFT );
		mv  = POINT_MV_LEFT;
	}else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
		ret = PointerWkMoveSel( wk->pw,NULL,NULL,NULL,NULL,wk->cur_pos,POINT_MV_RIGHT );
		mv  = POINT_MV_RIGHT;
	}else{
		ret = POINT_SEL_NOMOVE;
	}

	// キー移動
	if( ret != POINT_SEL_NOMOVE ){
		return Main_KeyMove( wk, mv, ret );
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
//		Snd_SePlay( SEQ_SE_DP_DECIDE );		// 各アプリでSEが可変になる可能性がある
		return wk->cur_pos;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
//		Snd_SePlay( SEQ_SE_DP_DECIDE );
		return CURSORMOVE_CANCEL;
	}

	return CURSORMOVE_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * タッチチェック
 *
 * @param	wk		カーソル移動ワーク
 * @param	hit		タッチ位置
 *
 * @retval	"CURSORMOVE_NONE = 無効な位置"
 * @retval	"CURSORMOVE_NONE != 有効な位置"
 */
//--------------------------------------------------------------------------------------------
static u32 Main_TouchCheck( CURSOR_MOVE_WORK * wk, int hit )
{
	if( MoveTableBitCheck( wk->mv_tbl, hit ) == TRUE ){
		wk->cur_pos  = hit;
		wk->old_pos  = DEF_OLDPOS;
		wk->save_pos = DEF_OLDPOS;
		if( wk->cur_flg == TRUE && wk->cur_mode == CURSOR_MODE_VANISH ){
			wk->cur_flg = FALSE;
			wk->func->off( wk->work, wk->cur_pos, wk->old_pos );
		}
		wk->func->touch( wk->work, wk->cur_pos, wk->old_pos );
		return hit;
	}
	return CURSORMOVE_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル表示
 *
 * @param	wk		カーソル移動ワーク
 *
 * @return	CURSORMOVE_CURSOR_ON
 */
//--------------------------------------------------------------------------------------------
static u32 Main_CursorOn( CURSOR_MOVE_WORK * wk )
{
	wk->cur_flg  = TRUE;
	wk->old_pos  = DEF_OLDPOS;
	wk->save_pos = DEF_OLDPOS;
	wk->func->on( wk->work, wk->cur_pos, wk->old_pos );
	return CURSORMOVE_CURSOR_ON;
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動（キー入力）
 *
 * @param	wk		カーソル移動ワーク
 * @param	mv		移動方向
 * @param	ret		移動先
 *
 * @retval	"CURSORMOVE_CURSOR_MOVE = 移動した"
 * @retval	"CURSORMOVE_NONE = 移動しなかった"
 */
//--------------------------------------------------------------------------------------------
static u32 Main_KeyMove( CURSOR_MOVE_WORK * wk, u8 mv, u8 ret )
{
	u8	tmp, old;

	// 移動先の記憶ビットチェック
	if( ret & CURSORMOVE_RETBIT ){
		if( wk->save_pos != DEF_OLDPOS ){
			ret = wk->save_pos;			// 記憶場所へ
		}else{
			ret ^= CURSORMOVE_RETBIT;	// 移動先へ
		}
	}

	old = 1;
	while(1){
		// 移動先が移動可能
		if( wk->mv_tbl[ret/32] & (1<<(ret%32)) ){
			break;
		}
		old = 0;
		tmp = PointerWkMoveSel( wk->pw,NULL,NULL,NULL,NULL,ret,mv )&(0xff^CURSORMOVE_RETBIT);
		// 移動先が行き止まり or 元の位置に戻ってきたら
		if( tmp == ret || tmp == wk->cur_pos ){
			ret = wk->cur_pos;
			break;
		}
		ret = tmp;
	}

	if( wk->cur_pos != ret ){
		// 移動先の記憶ビットが立ってたら記憶
		if( OldCursorSetCheck(&wk->pw[ret],mv) == TRUE && old != 0 ){
			wk->save_pos = wk->cur_pos;
		}else{
			wk->save_pos = DEF_OLDPOS;
		}

		wk->old_pos = wk->cur_pos;
		wk->cur_pos = ret;
		wk->func->move( wk->work, wk->cur_pos, wk->old_pos );
//		CURSORMOVE_CursorPosSet( wk, wk->cur_pos );
//		Snd_SePlay( SEQ_SE_DP_DECIDE );
		return CURSORMOVE_CURSOR_MOVE;
	}
	return CURSORMOVE_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル位置取得
 *
 * @param	wk		カーソル移動ワーク
 *
 * @return	カーソル位置
 */
//--------------------------------------------------------------------------------------------
u8 CURSORMOVE_PosGet( CURSOR_MOVE_WORK * wk )
{
	return wk->cur_pos;
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル記憶位置取得
 *
 * @param	wk		カーソル移動ワーク
 *
 * @return	カーソル記憶位置
 */
//--------------------------------------------------------------------------------------------
u8 CURSORMOVE_SavePosGet( CURSOR_MOVE_WORK * wk )
{
	return wk->save_pos;
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル位置設定
 *
 * @param	wk		カーソル移動ワーク
 * @param	pos		位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CURSORMOVE_PosSet( CURSOR_MOVE_WORK * wk, u8 pos )
{
	wk->cur_pos  = pos;
	wk->old_pos  = DEF_OLDPOS;
	wk->save_pos = DEF_OLDPOS;
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル位置設定（拡張版）
 *
 * @param	wk			カーソル移動ワーク
 * @param	pos			位置
 * @param	old_pos		前回の位置
 * @param	save_pos	記憶位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CURSORMOVE_PosSetEx( CURSOR_MOVE_WORK * wk, u8 new_pos, u8 old_pos, u8 save_pos )
{
	wk->cur_pos  = new_pos;
	wk->old_pos  = old_pos;
	wk->save_pos = save_pos;
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル表示状態取得
 *
 * @param	wk		カーソル移動ワーク
 *
 * @retval	"TRUE = 表示"
 * @retval	"FALSE = 非表示"
 */
//--------------------------------------------------------------------------------------------
BOOL CURSORMOVE_CursorOnOffGet( CURSOR_MOVE_WORK * wk )
{
	return wk->cur_flg;
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル表示状態設定
 *
 * @param	wk		カーソル移動ワーク
 * @param	flg		TRUE = ON, FALSE = OFF
 *
 * @return	none
 *
 * @li	表示をONにした場合、前回の位置と記憶位置は初期化されます
 * @li	コールバックのold_posにはCURSORMOVE_ONOFF_DIRECTが渡されます
 */
//--------------------------------------------------------------------------------------------
void CURSORMOVE_CursorOnOffSet( CURSOR_MOVE_WORK * wk, BOOL flg )
{
	wk->cur_flg = flg;
	if( wk->cur_flg == TRUE ){
		wk->old_pos  = DEF_OLDPOS;
		wk->save_pos = DEF_OLDPOS;
		wk->func->on( wk->work, wk->cur_pos, CURSORMOVE_ONOFF_DIRECT );
	}else{
		wk->func->off( wk->work, wk->cur_pos, CURSORMOVE_ONOFF_DIRECT );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動テーブル初期化
 *
 * @param	wk		カーソル移動ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CURSORMOVE_MoveTableInit( CURSOR_MOVE_WORK * wk )
{
	wk->mv_tbl[0] = DEF_MVTBL;
	wk->mv_tbl[1] = DEF_MVTBL;
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動テーブル設定
 *
 * @param	wk		カーソル移動ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CURSORMOVE_MoveTableSet( CURSOR_MOVE_WORK * wk, u32 * tbl )
{
	wk->mv_tbl[0] = tbl[0];
	wk->mv_tbl[1] = tbl[1];
}

//--------------------------------------------------------------------------------------------
/**
 * 特定の位置をカーソル移動不可にする
 *
 * @param	wk		カーソル移動ワーク
 * @param	pos		位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CURSORMOVE_MoveTableBitOff( CURSOR_MOVE_WORK * wk, u32 pos )
{
	u32	i = pos / 32;
	pos = pos % 32;

	if( ( wk->mv_tbl[i] & (1<<pos) ) != 0 ){
		wk->mv_tbl[i] = wk->mv_tbl[i] & ( 0xffffffff ^ (1<<pos) );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 特定の位置をカーソル移動可にする
 *
 * @param	wk		カーソル移動ワーク
 * @param	pos		位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CURSORMOVE_MoveTableBitOn( CURSOR_MOVE_WORK * wk, u32 pos )
{
	u32	i = pos / 32;
	pos = pos % 32;

	if( ( wk->mv_tbl[i] & (1<<pos) ) == 0 ){
		wk->mv_tbl[i] = wk->mv_tbl[i] ^ (1<<pos);
	}
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル位置データ取得
 *
 * @param	wk		カーソル移動ワーク
 * @param	pos		カーソル位置
 *
 * @return	カーソル位置データ
 */
//--------------------------------------------------------------------------------------------
const POINTER_WORK * CURSORMOVE_PointerWorkGet( CURSOR_MOVE_WORK * wk, u32 pos )
{
	return &wk->pw[pos];
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動テーブル初期化
 *
 * @param	wk			カーソル移動ワーク
 * @param	rect		タッチ判定テーブル
 * @param	pw			カーソル位置ワーク
 * @param	cur_pos		カーソル位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CURSORMOVE_SceneChange(
		CURSOR_MOVE_WORK * wk, const GFL_UI_TP_HITTBL * rect, const POINTER_WORK * pw, u8 cur_pos )
{
	wk->rect = rect;
	wk->pw   = pw;

	wk->cur_pos  = cur_pos;
	wk->old_pos  = DEF_OLDPOS;
	wk->save_pos = DEF_OLDPOS;

	wk->mv_tbl[0] = DEF_MVTBL;
	wk->mv_tbl[1] = DEF_MVTBL;
}

//--------------------------------------------------------------------------------------------
/**
 * 前のカーソル位置に移動するか
 *
 * @param	wk		カーソル位置ワーク
 * @param	mv		移動方向
 *
 * @retval	"TRUE = 移動する"
 * @retval	"FALSE = 移動しない"
 */
//--------------------------------------------------------------------------------------------
static BOOL OldCursorSetCheck( const POINTER_WORK * pw, u8 mv )
{
	switch( mv ){
	case POINT_MV_UP:
		if( pw->down & CURSORMOVE_RETBIT ){
			return TRUE;
		}
		break;
	case POINT_MV_DOWN:
		if( pw->up & CURSORMOVE_RETBIT ){
			return TRUE;
		}
		break;
	case POINT_MV_LEFT:
		if( pw->right & CURSORMOVE_RETBIT ){
			return TRUE;
		}
		break;
	case POINT_MV_RIGHT:
		if( pw->left & CURSORMOVE_RETBIT ){
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * データ有効/無効チェック
 *
 * @param	tbl		データテーブル
 * @param	pos		位置
 *
 * @retval	"TRUE = 有効"
 * @retval	"FALSE = 無効"
 */
//--------------------------------------------------------------------------------------------
static BOOL MoveTableBitCheck( u32 * tbl, u32 pos )
{
	if( tbl[pos/32] & (1<<(pos%32)) ){
		return TRUE;
	}
	return FALSE;
}
