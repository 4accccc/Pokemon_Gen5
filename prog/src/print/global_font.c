//============================================================================================
/**
 * @file  global_font.c
 * @brief グローバルフォントデータ
 * @author  taya
 * @date  2008.11.19
 */
//============================================================================================
#include <assert.h>
#include <heapsys.h>

#include "print\global_font.h"

#include "arc_def.h"
#include "font\font.naix"


//--------------------------------------------------------------
/**
 *  グローバルメッセージデータ
 */
//--------------------------------------------------------------
const GFL_FONT* GlobalFont_Default = NULL;


//=============================================================================================
/**
 * グローバルメッセージデータハンドラを生成（プログラム起動後に１度だけ呼ぶ）
 *
 * @param   heapID    生成用ヒープID
 */
//=============================================================================================
void GLOBALFONT_Init( HEAPID heapID )
{
  GF_ASSERT(GlobalFont_Default == NULL);

  OS_TPrintf("GLOBANFONT_Init : HeapSize = %08x ->", GFL_HEAP_GetHeapFreeSize( heapID ));

  {
    GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
  }

  OS_TPrintf(" %08x\n", GFL_HEAP_GetHeapFreeSize( heapID ));
}

