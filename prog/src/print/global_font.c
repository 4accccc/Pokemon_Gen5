//============================================================================================
/**
 * @file	global_font.c
 * @brief	グローバルフォントデータ
 * @author	taya
 * @date	2008.11.19
 */
//============================================================================================
#include <assert.h>

#include "print/global_font.h"

#include "arc_def.h"
#include "test_graphic\d_taya.naix"


//--------------------------------------------------------------
/**
 *	グローバルメッセージデータ
 */
//--------------------------------------------------------------
const GFL_FONT* GlobalFont_Default = NULL;


//=============================================================================================
/**
 * グローバルメッセージデータハンドラを生成（プログラム起動後に１度だけ呼ぶ）
 *
 * @param   heapID		生成用ヒープID
 */
//=============================================================================================
void GLOBALFONT_Init( HEAPID heapID )
{
	GF_ASSERT(GlobalFont_Default == NULL);

	{
		ARCHANDLE* arc = GFL_ARC_OpenDataHandle( ARCID_D_TAYA, heapID );
		GFL_FONT_CreateHandle( arc, NARC_d_taya_lc12_2bit_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
	}
}

