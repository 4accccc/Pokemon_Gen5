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

//mine
#include "app/townmap_data_sys.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	置き換えデータ
//=====================================
typedef enum
{
  TOWNMAP_REPLACE_PARAM_DST_ZONE_ID,  //置き換え先ゾーングループ
  TOWNMAP_REPLACE_PARAM_SRC_ZONE_ID_00,  //置き換え元ゾーングループ00
  TOWNMAP_REPLACE_PARAM_MAX,
} TOWNMAP_REPLACE_PARAM;

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
///	置き換えデータ本体
//=====================================
typedef struct {
  u16 param[ TOWNMAP_REPLACE_PARAM_MAX ];
} TOWNMAP_REPLACE_STRUCT;


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
			0, FALSE, heapID );

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
 *	@brief  townmap_util.hのFIELD_TOWNMAP_GetRootZoneID関数で得た
 *	ZONEIDを以下に渡すと上記タウンマップデータのidxを返す
 *
 *	@param	const TOWNMAP_DATA *cp_wk   ワーク
 *	@param	zoneID                      タウンマップで使用する用のID
 *
 *	@return タウンマップデータのインデックス  見つからなかった場合TOWNMAP_DATA_ERRORを返す
 */
//-----------------------------------------------------------------------------
u16 TOWNMAP_DATA_SearchRootZoneID( const TOWNMAP_DATA *cp_wk, u16 zoneID )
{ 
  int i;
  for( i = 0; i < TOWNMAP_DATA_MAX; i++ )
  { 
    if( TOWNMAP_DATA_GetParam( cp_wk, i, TOWNMAP_DATA_PARAM_ZONE_ID ) == zoneID )
    { 
      return i;
    }
  }

  return TOWNMAP_DATA_ERROR;
}


//=============================================================================
/**
 *    置き換え
 */
//=============================================================================
#if 0 //cdat化したためなくなりました
//----------------------------------------------------------------------------
/**
 *	@brief  置き換えデータ取得
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ハンドル
 */
//-----------------------------------------------------------------------------
TOWNMAP_REPLACE_DATA *TOWNMAP_REPLACE_DATA_Alloc( HEAPID heapID )
{ 
	void *p_handle;
	p_handle	= GFL_ARC_UTIL_Load( ARCID_TOWNMAP_DATA, 
			1, FALSE, heapID );

	return p_handle;
}
//----------------------------------------------------------------------------
/**
 *	@brief  置き換えデータ破棄
 *
 *	@param	TOWNMAP_REPLACE_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void TOWNMAP_REPLACE_DATA_Free( TOWNMAP_REPLACE_DATA *p_wk )
{ 
	GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  置き換え関数
 *
 *	@param	const TOWNMAP_REPLACE_DATA *cp_wk ワーク
 *	@param	zoneID  置き換え元ゾーンID
 *
 *	@return 置き換え先ゾーンID  もし入っていなければTOWNMAP_DATA_ERROR
 */
//-----------------------------------------------------------------------------
u16 TOWNMAP_REPLACE_DATA_GetReplace( const TOWNMAP_REPLACE_DATA *cp_wk, u16 zoneID )
{ 
  GF_ASSERT( cp_wk );

	{	
    int i;
		const TOWNMAP_REPLACE_STRUCT *cp_data;

    for( i = 0; i < TOWNMAP_REPLACE_MAX; i++ )
    { 
      cp_data	= (const TOWNMAP_REPLACE_STRUCT*)((const u8*)(cp_wk) + sizeof(TOWNMAP_REPLACE_STRUCT) * i);

      if( cp_data->param[ TOWNMAP_REPLACE_PARAM_SRC_ZONE_ID_00 ] == zoneID )
      { 
        return cp_data->param[ TOWNMAP_REPLACE_PARAM_DST_ZONE_ID ];
      }
    }

		return TOWNMAP_DATA_ERROR;
	}
}

#endif
