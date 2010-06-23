//============================================================================================
/**
 * @file		zukan_common.c
 * @brief		図鑑画面 共通処理
 * @author	Hiroyuki Nakamura
 * @date		09.12.15
 *
 *	モジュール名：ZKNCOMM
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/wipe.h"
#include "poke_tool/monsnum_def.h"

#include "zukan_common.h"


//============================================================================================
//	定数定義
//============================================================================================
#define	BASEBG_SCROLL_WAIT	( 4 )			// ＢＧスクロールウェイト
#define	BASEBG_SCROLL_VAL		( 1 )			// ＢＧスクロール値


//--------------------------------------------------------------------------------------------
/**
 * @brief		デフォルトリスト作成
 *
 * @param		sv			図鑑セーブデータ
 * @param		list		リスト作成場所
 * @param		heapID	ヒープＩＤ
 *
 * @return	リストに登録した数
 */
//--------------------------------------------------------------------------------------------
u16 ZKNCOMM_MakeDefaultList( ZUKAN_SAVEDATA * sv, u16 ** list, HEAPID heapID )
{
	u16 * buf;
	u16	siz, max;
	u32	i;

	if( ZUKANSAVE_GetZukanMode( sv ) == TRUE ){
		siz = ZUKAN_GetNumberRow( ZKNCOMM_LIST_SORT_MODE_ZENKOKU, heapID, &buf );
	}else{
		siz = ZUKAN_GetNumberRow( ZKNCOMM_LIST_SORT_MODE_LOCAL, heapID, &buf );
	}
	max = 0;

	for( i=0; i<siz; i++ ){
		if( ZUKANSAVE_GetPokeGetFlag( sv, buf[i] ) == TRUE ||
				ZUKANSAVE_GetPokeSeeFlag( sv, buf[i] ) == TRUE ){
			max = i+1;
		}
	}

	*list = buf;

	return max;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		デフォルトポケモンを登録されているものにする
 *
 * @param		sv		図鑑セーブデータ
 * @param		list	リストデータ
 * @param		max		リスト項目数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNCOMM_InitDefaultMons( ZUKAN_SAVEDATA * sv, u16 * list, u16 max )
{
	u16	def_mons, init_mons;
	u16	i;

	init_mons = 0xffff;
	def_mons  = ZUKANSAVE_GetDefaultMons( sv );

	for( i=0; i<max; i++ ){
		if( ZUKANSAVE_GetPokeSeeFlag( sv, list[i] ) == TRUE ){
			if( list[i] == def_mons ){
				return;
			}
			if( init_mons == 0xffff ){
				init_mons = list[i];
			}
		}
	}

	if( init_mons != 0xffff ){
		ZUKANSAVE_SetDefaultMons( sv, init_mons );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ビクティチェック
 *
 * @param		sv		図鑑セーブデータ
 *
 * @retval	"TRUE = 表示する"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZKNCOMM_CheckLocalListNumberZero( ZUKAN_SAVEDATA * sv )
{
	// 表示モードが全国
	if( ZUKANSAVE_GetZukanMode( sv ) == TRUE ){
		return TRUE;
	}

	// 見た・捕獲した場合
	if( ZUKANSAVE_GetPokeGetFlag( sv, MONSNO_657 ) == TRUE ||
			ZUKANSAVE_GetPokeSeeFlag( sv, MONSNO_657 ) == TRUE ){
		return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ソートデータリセット
 *
 * @param		sv		図鑑セーブデータ
 * @param		sort	ソートデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNCOMM_ResetSortData( const ZUKAN_SAVEDATA * sv, ZKNCOMM_LIST_SORT * sort )
{
	if( ZUKANSAVE_GetZukanMode( sv ) == TRUE ){
		sort->mode = ZKNCOMM_LIST_SORT_MODE_ZENKOKU;
	}else{
		sort->mode = ZKNCOMM_LIST_SORT_MODE_LOCAL;
	}
	sort->row   = ZKNCOMM_LIST_SORT_ROW_NUMBER;
	sort->name  = ZKNCOMM_LIST_SORT_NONE;
	sort->type1 = ZKNCOMM_LIST_SORT_NONE;
	sort->type2 = ZKNCOMM_LIST_SORT_NONE;
	sort->color = ZKNCOMM_LIST_SORT_NONE;
	sort->form  = ZKNCOMM_LIST_SORT_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードインセット
 *
 * @param		heapID		ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNCOMM_SetFadeIn( HEAPID heapID )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウトセット
 *
 * @param		heapID		ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNCOMM_SetFadeOut( HEAPID heapID )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		背景スクロール
 *
 * @param		mainBG		メイン画面のＢＧフレーム
 * @param		subBG			サブ画面のＢＧフレーム
 * @param		cnt				カウンタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNCOMM_ScrollBaseBG( u8 mainBG, u8 subBG, u32 * cnt )
{
	*cnt += 1;
	if( *cnt == BASEBG_SCROLL_WAIT ){
		GFL_BG_SetScrollReq( mainBG, GFL_BG_SCROLL_X_INC, BASEBG_SCROLL_VAL );
		GFL_BG_SetScrollReq( subBG, GFL_BG_SCROLL_X_INC, BASEBG_SCROLL_VAL );
		*cnt = 0;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター追加
 *
 * @param		unit		GFL_CLUNIT
 * @param		prm			セルアクターデータ
 * @param		heapID	ヒープＩＤ
 *
 * @return	GFL_CLWK
 */
//--------------------------------------------------------------------------------------------
GFL_CLWK * ZKNCOMM_CreateClact( GFL_CLUNIT * unit, const ZKNCOMM_CLWK_DATA * prm, HEAPID heapID )
{
	return GFL_CLACT_WK_Create( unit, prm->chrRes, prm->palRes, prm->celRes, &prm->dat, prm->disp, heapID );
}
