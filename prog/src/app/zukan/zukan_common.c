//============================================================================================
/**
 * @file		zukan_common.c
 * @brief		ê}ä”âÊñ  ã§í èàóù
 * @author	Hiroyuki Nakamura
 * @date		09.12.15
 *
 *	ÉÇÉWÉÖÅ[ÉãñºÅFZKNCOMM
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/wipe.h"

#include "zukan_common.h"


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
