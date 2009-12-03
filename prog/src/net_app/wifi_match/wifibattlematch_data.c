//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_data.c
 *	@brief  データバッファ
 *	@author	Toru=Nagihashi
 *	@data		2009.12.01
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID

//外部公開
#include "wifibattlematch_data.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ワーク　不完全型としてしか存在しない
//=====================================
/*
struct _WIFIBATTLEMATCH_DATA_WORK
{ 

};
*/
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  データバッファ作成
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_DATA_WORK *WIFIBATTLEMATCH_DATA_Init( HEAPID heapID )
{ 
  WIFIBATTLEMATCH_DATA_WORK *p_wk;
  u32 size;

  size  = WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE*2;
  p_wk  = GFL_HEAP_AllocMemory( heapID, size );
  GFL_STD_MemClear( p_wk, size );

  return p_wk;
} 
//----------------------------------------------------------------------------
/**
 *	@brief  データバッファ破棄
 *
 *	@param	WIFIBATTLEMATCH_DATA_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_DATA_Exit( WIFIBATTLEMATCH_DATA_WORK *p_wk )
{ 
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分のデータのポインタ取得
 *
 *	@param	WIFIBATTLEMATCH_DATA_WORK *p_wk ワーク
 *
 *	@return データポインタ
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_ENEMYDATA * WIFIBATTLEMATCH_DATA_GetPlayerDataPtr( WIFIBATTLEMATCH_DATA_WORK *p_wk )
{ 
  return (WIFIBATTLEMATCH_ENEMYDATA *)p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  敵データのポインタ取得
 *
 *	@param	WIFIBATTLEMATCH_DATA_WORK *p_wk ワーク
 *
 *	@return データポインタ
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_ENEMYDATA * WIFIBATTLEMATCH_DATA_GetEnemyDataPtr( WIFIBATTLEMATCH_DATA_WORK *p_wk )
{ 
  return (WIFIBATTLEMATCH_ENEMYDATA *)((u8*)p_wk + WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE);
}
