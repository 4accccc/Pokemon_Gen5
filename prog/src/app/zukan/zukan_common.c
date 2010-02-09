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

GFL_CLWK * ZKNCOMM_CreateClact( GFL_CLUNIT * unit, const ZKNCOMM_CLWK_DATA * prm, HEAPID heapID )
{
	return GFL_CLACT_WK_Create( unit, prm->chrRes, prm->palRes, prm->celRes, &prm->dat, prm->disp, heapID );
}
