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

#include "zukan_common.h"


#define	BASEBG_SCROLL_WAIT	( 4 )			// ＢＧスクロールウェイト
#define	BASEBG_SCROLL_VAL		( 1 )



// デフォルトリスト作成
void ZKNCOMM_MakeDefaultList( ZUKAN_SAVEDATA * sv, u16 * list )
{
	u32	i;

	for( i=0; i<MONSNO_END; i++ ){
		if( ZUKANSAVE_GetPokeGetFlag( sv, i+1 ) == TRUE ){
			list[i] = ZUKAN_LIST_MONS_GET;
		}else if( ZUKANSAVE_GetPokeSeeFlag( sv, i+1 ) == TRUE ){
			list[i] = ZUKAN_LIST_MONS_SEE;
		}else{
			list[i] = ZUKAN_LIST_MONS_NONE;
		}
	}
}

// ソートデータリセット
void ZKNCOMM_ResetSortData( ZKNCOMM_LIST_SORT * sort )
{
	sort->mode  = ZKNCOMM_LIST_SORT_MODE_ZENKOKU;
/*
	sort->row   = ZKNCOMM_LIST_SORT_ROW_NUMBER;
	sort->name  = ZKNCOMM_LIST_SORT_NONE;
	sort->type1 = ZKNCOMM_LIST_SORT_NONE;
	sort->type2 = ZKNCOMM_LIST_SORT_NONE;
	sort->color = ZKNCOMM_LIST_SORT_NONE;
	sort->form  = ZKNCOMM_LIST_SORT_NONE;
*/
	sort->row   = 3;
	sort->name  = 20;
	sort->type1 = 3;
	sort->type2 = 8;
	sort->color = 6;
	sort->form  = 9;
}


void ZKNCOMM_SetFadeIn( HEAPID heapID )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID );
}

void ZKNCOMM_SetFadeOut( HEAPID heapID )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID );
}

// 背景スクロール
void ZKNCOMM_ScrollBaseBG( u8 mainBG, u8 subBG, u32 * cnt )
{
	*cnt += 1;
	if( *cnt == BASEBG_SCROLL_WAIT ){
		GFL_BG_SetScrollReq( mainBG, GFL_BG_SCROLL_X_INC, BASEBG_SCROLL_VAL );
		GFL_BG_SetScrollReq( subBG, GFL_BG_SCROLL_X_INC, BASEBG_SCROLL_VAL );
		*cnt = 0;
	}
}


GFL_CLWK * ZKNCOMM_CreateClact( GFL_CLUNIT * unit, const ZKNCOMM_CLWK_DATA * prm, HEAPID heapID )
{
	return GFL_CLACT_WK_Create( unit, prm->chrRes, prm->palRes, prm->celRes, &prm->dat, prm->disp, heapID );
}
