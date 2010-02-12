//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_zonefog.c
 *	@brief	ゾーン用フォグ・ライト情報
 *	@author	tomoya takahashi
 *	@date		2009.07.09
 *
 *	モジュール名：FIELD_ZONEFOGLIGHT
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "arc_def.h"
#include "fieldmap/field_fog_table.naix"
#include "fieldmap/field_zone_light.naix"

#include "field_zonefog.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------



//-------------------------------------
///	データ構造体
//=====================================
typedef struct {
	s32 offset;
	u32	slope;
} FOG_DATA;

//-------------------------------------
///	FIELD_ZONEFOGLIGHTシステム
//=====================================
struct _FIELD_ZONEFOGLIGHT	
{
	FOG_DATA* p_data;

	u32 light;

  ARCHANDLE* p_handle;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief	ゾーンフォグ情報管理システム　生成
 *
 *	@param	heapID		ヒープID
 *
 *	@return	ゾーンフォグ情報管理システム
 */
//-----------------------------------------------------------------------------
FIELD_ZONEFOGLIGHT* FIELD_ZONEFOGLIGHT_Create( HEAPID heapID )
{
	FIELD_ZONEFOGLIGHT* p_sys;
  u32 size;

	p_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_ZONEFOGLIGHT) );

	p_sys->light = FIELD_ZONEFOGLIGHT_DATA_NONE;

  p_sys->p_handle = GFL_ARC_OpenDataHandle( ARCID_ZONEFOG_TABLE, heapID );

	return  p_sys;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゾーンフォグ情報管理システム	破棄
 *
 *	@param	p_sys			システム
 */
//-----------------------------------------------------------------------------
void FIELD_ZONEFOGLIGHT_Delete( FIELD_ZONEFOGLIGHT* p_sys )
{
  GFL_ARC_CloseDataHandle( p_sys->p_handle );
  
	FIELD_ZONEFOGLIGHT_Clear( p_sys );
	GFL_HEAP_FreeMemory( p_sys );
}


//----------------------------------------------------------------------------
/**
 *	@brief	ゾーンフォグ情報管理システム	フォグ情報読み込み
 *
 *	@param	p_sys			システム
 *	@param	datano		フォグデータナンバー
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_ZONEFOGLIGHT_Load( FIELD_ZONEFOGLIGHT* p_sys, u32 fogno, u32 lightno, HEAPID heapID )
{
	GF_ASSERT( p_sys );
	GF_ASSERT( p_sys->p_data == NULL );

	if( fogno != FIELD_ZONEFOGLIGHT_DATA_NONE )
	{
		p_sys->p_data = GFL_ARCHDL_UTIL_Load( p_sys->p_handle, fogno, FALSE, heapID );
	}
	p_sys->light	= lightno;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゾーンフォグ情報管理システム	フォグ情報破棄
 *
 *	@param	p_sys			システム
 */
//-----------------------------------------------------------------------------
void FIELD_ZONEFOGLIGHT_Clear( FIELD_ZONEFOGLIGHT* p_sys )
{
	GF_ASSERT( p_sys );

	if( p_sys->p_data )
	{
		GFL_HEAP_FreeMemory( p_sys->p_data );
		p_sys->p_data = NULL;
	}
	p_sys->light = FIELD_ZONEFOGLIGHT_DATA_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	情報があるかチェック
 *
 *	@param	cp_sys システム
 *
 *	@retval	TRUE	ある
 *	@retval	FALSE	なし
 */
//-----------------------------------------------------------------------------
BOOL FIELD_ZONEFOGLIGHT_IsFogData( const FIELD_ZONEFOGLIGHT* cp_sys )
{
	GF_ASSERT( cp_sys );
	if( cp_sys->p_data )
	{
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライト情報があるかチェック
 *
 *	@param	cp_sys		システム
 *
 *	@retval	TRUE	ある
 *	@retval	FALSE	なし
 */
//-----------------------------------------------------------------------------
BOOL FIELD_ZONEFOGLIGHT_IsLightData( const FIELD_ZONEFOGLIGHT* cp_sys )
{
	if( cp_sys->light == FIELD_ZONEFOGLIGHT_DATA_NONE )
	{
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	今の情報のオフセット値取得
 *
 *	@param	cp_sys		システム
 *
 *	@return	オフセット値
 */
//-----------------------------------------------------------------------------
s32 FIELD_ZONEFOGLIGHT_GetOffset( const FIELD_ZONEFOGLIGHT* cp_sys )
{
	GF_ASSERT( cp_sys  );
	if( cp_sys->p_data )
	{
		return cp_sys->p_data->offset;
	}
	return FIELD_ZONEFOGLIGHT_DATA_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	今の情報のスロープ値取得
 *
 *	@param	cp_sys	システム
 *
 *	@return	スロープ
 */
//-----------------------------------------------------------------------------
u32 FIELD_ZONEFOGLIGHT_GetSlope( const FIELD_ZONEFOGLIGHT* cp_sys )
{
	GF_ASSERT( cp_sys  );

	if( cp_sys->p_data )
	{
		return cp_sys->p_data->slope;
	}
	return FIELD_ZONEFOGLIGHT_DATA_NONE;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ライトインデックス	取得
 *
 *	@param	cp_sys	システム
 *
 *	@return	ライトインデックス
 */
//-----------------------------------------------------------------------------
u32 FIELD_ZONEFOGLIGHT_GetLightIndex( const FIELD_ZONEFOGLIGHT* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->light;
}

