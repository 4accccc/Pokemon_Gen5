//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		townmap_data_sys.c
 *	@brief	タウンマップデータ読み込み
 *	@author	Toru=Nagihashi
 *	@data		2009.07.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//lib
#include <gflib.h>

//archive
#include "arc_def.h"
#include "townmap_data.naix"

//mine
#include "app/townmap_data_sys.h"
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
///	データ本体
//=====================================
typedef struct 
{
	u16	param[TOWNMAP_DATA_PARAM_MAX];
} TOWNMAP_DATA_STRUCT;



//-------------------------------------
///	タウンマップデータ
//=====================================
/*
typedef struct _TOWNMAP_DATA TOWNMAP_DATA;
*/

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	マップデータアロケート
 *
 *	@param	HEAPID heapID		ヒープID
 *
 *	@return	マップデータハンドル
 */
//-----------------------------------------------------------------------------
TOWNMAP_DATA *TOWNMAP_DATA_Alloc( HEAPID heapID )
{	
	void *p_handle;
	p_handle	= GFL_ARC_UTIL_Load( ARCID_TOWNMAP_DATA, 
			NARC_townmap_data_townmap_data_dat, FALSE, heapID );

	return p_handle;
}
//----------------------------------------------------------------------------
/**
 *	@brief	マップデータ解放
 *
 *	@param	TOWNMAP_DATA *p_wk	マップデータハンドル
 */
//-----------------------------------------------------------------------------
void TOWNMAP_DATA_Free( TOWNMAP_DATA *p_wk )
{	
	GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	マップデータを取得
 *
 *	@param	const		TOWNMAP_DATA *cp_wk	マップデータハンドル
 *	@param	idx			場所インデックス（MAX　TOWNMAP_DATA_MAX）
 *	@param	param		TOWNMAP_DATA_PARAM列挙
 *
 *	@return	値
 */
//-----------------------------------------------------------------------------
u16 TOWNMAP_DATA_GetParam( const TOWNMAP_DATA *cp_wk, u16 idx, TOWNMAP_DATA_PARAM param )
{	
	GF_ASSERT( cp_wk );
	GF_ASSERT( idx < TOWNMAP_DATA_MAX );
	GF_ASSERT( param < TOWNMAP_DATA_PARAM_MAX );

	{	
		const TOWNMAP_DATA_STRUCT *cp_data;
		cp_data	= (const TOWNMAP_DATA_STRUCT*)((const u8*)(cp_wk) + sizeof(TOWNMAP_DATA_STRUCT) * idx);

		return cp_data->param[ param ];
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  field_townmap.hのFIELD_TOWNMAP_GetRootZoneID関数で得た
 *	ZONEIDを以下に渡すと上記タウンマップデータのidxを返す
 *
 *	@param	const TOWNMAP_DATA *cp_wk   ワーク
 *	@param	zoneID                      タウンマップで使用する用のID
 *
 *	@return タウンマップデータのインデックス
 */
//-----------------------------------------------------------------------------
u16 TOWNMAP_DATA_SearchRootZoneID( const TOWNMAP_DATA *cp_wk, u16 zoneID )
{ 
  int i;
  for( i = 0; i < TOWNMAP_DATA_MAX; i++ )
  { 
    if( TOWNMAP_DATA_GetParam( cp_wk, i, TOWNMAP_DATA_PARAM_ZONE_ID ) )
    { 
      return i;
    }
  }

  GF_ASSERT_MSG( 0, "見つかりません %d\n", zoneID );
  return 0;
}
