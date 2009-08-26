//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_nogrid_mapper.c
 *	@brief  ノーグリッド動作　マッパー
 *	@author	tomoya takahashi
 *	@date		2009.08.25
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "field_nogrid_mapper.h"

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
///	ノーグリッド動作マッパーワーク
//=====================================
struct _FLDNOGRID_MAPPER 
{
  // 各モジュール
  FIELD_RAIL_MAN*       p_railMan;
  FIELD_RAIL_LOADER*    p_railLoader;
  RAIL_ATTR_DATA*       p_attr;
  FLD_SCENEAREA*        p_areaMan;
  FLD_SCENEAREA_LOADER* p_areaLoader;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッド動作マッパー　ワーク生成
 *
 *	@param	heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
FLDNOGRID_MAPPER* FLDNOGRID_MAPPER_Create( u32 heapID, FIELD_CAMERA* p_camera )
{
  FLDNOGRID_MAPPER* p_mapper;

  p_mapper = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDNOGRID_MAPPER) );

  // 各モジュールワーク生成
  p_mapper->p_railMan       = FIELD_RAIL_MAN_Create( heapID, FIELD_RAIL_WORK_MAX, p_camera ); 
  p_mapper->p_railLoader    = FIELD_RAIL_LOADER_Create( heapID );
  p_mapper->p_areaMan       = FLD_SCENEAREA_Create( heapID, p_camera );
  p_mapper->p_areaLoader    = FLD_SCENEAREA_LOADER_Create( heapID );
  p_mapper->p_attr          = RAIL_ATTR_Create( heapID );

  return p_mapper;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッド動作マッパー破棄
 *
 *	@param	p_mapper  マッパーワーク
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_Delete( FLDNOGRID_MAPPER* p_mapper )
{
  GF_ASSERT( p_mapper );
  
  RAIL_ATTR_Delete( p_mapper->p_attr );
  FLD_SCENEAREA_Delete( p_mapper->p_areaMan );
  FLD_SCENEAREA_LOADER_Delete( p_mapper->p_areaLoader );
  FIELD_RAIL_MAN_Delete( p_mapper->p_railMan );
  FIELD_RAIL_LOADER_Delete( p_mapper->p_railLoader );

  GFL_HEAP_FreeMemory( p_mapper );
}



//----------------------------------------------------------------------------
/**
 *	@brief  データの反映
 *
 *	@param	p_mapper  マッパー
 *	@param	cp_data   データ
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_ResistData( FLDNOGRID_MAPPER* p_mapper, const FLDNOGRID_RESISTDATA* cp_data, u32 heapID )
{
  GF_ASSERT( p_mapper );
  
  // 破棄処理
  FLDNOGRID_MAPPER_Release( p_mapper );

  // 情報ロード
  FIELD_RAIL_LOADER_Load( p_mapper->p_railLoader, cp_data->railDataID, heapID );
  FLD_SCENEAREA_LOADER_Load( p_mapper->p_areaLoader, cp_data->areaDataID, heapID );
  RAIL_ATTR_Load( p_mapper->p_attr, cp_data->attrDataID, heapID );

  // 構築
  FIELD_RAIL_MAN_Load( p_mapper->p_railMan, FIELD_RAIL_LOADER_GetData( p_mapper->p_railLoader ) );
  FLD_SCENEAREA_Load( p_mapper->p_areaMan, 
      FLD_SCENEAREA_LOADER_GetData( p_mapper->p_areaLoader ),
      FLD_SCENEAREA_LOADER_GetDataNum( p_mapper->p_areaLoader ),
      FLD_SCENEAREA_LOADER_GetFunc( p_mapper->p_areaLoader ) );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッド動作　情報破棄
 *
 *	@param	p_mapper  マッパー
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_Release( FLDNOGRID_MAPPER* p_mapper )
{
  GF_ASSERT( p_mapper );

  FLD_SCENEAREA_Release( p_mapper->p_areaMan );
  FIELD_RAIL_LOADER_Clear( p_mapper->p_railLoader );
  FLD_SCENEAREA_LOADER_Clear( p_mapper->p_areaLoader );
  RAIL_ATTR_Release( p_mapper->p_attr );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッド動作　メイン処理
 *
 *	@param	p_mapper  マッパーワーク
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_Main( FLDNOGRID_MAPPER* p_mapper )
{
  VecFx32 rail_pos;

  // レールシステムメイン
  FIELD_RAIL_MAN_Update( p_mapper->p_railMan );
  FIELD_RAIL_MAN_UpdateCamera( p_mapper->p_railMan );
  FIELD_RAIL_MAN_GetBindWorkPos( p_mapper->p_railMan, &rail_pos );
  FLD_SCENEAREA_Update( p_mapper->p_areaMan, &rail_pos );
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールワークの生成
 *
 *	@param	p_mapper  マッパー
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_WORK* FLDNOGRID_MAPPER_CreateRailWork( FLDNOGRID_MAPPER* p_mapper )
{
  GF_ASSERT( p_mapper );
  return FIELD_RAIL_MAN_CreateWork( p_mapper->p_railMan );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールワークの破棄
 *
 *	@param	p_mapper    マッパー
 *	@param	p_railWork  破棄するレールワーク
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_DeleteRailWork( FLDNOGRID_MAPPER* p_mapper, FIELD_RAIL_WORK* p_railWork )
{
  GF_ASSERT( p_mapper );
  FIELD_RAIL_MAN_DeleteWork( p_mapper->p_railMan, p_railWork );
}



//----------------------------------------------------------------------------
/**
 *	@brief  レールアトリビュート情報の取得
 *
 *	@param	cp_mapper     マッパー
 *	@param	cp_location   レールロケーション
 *
 *	@return マップアトリビュート
 */
//-----------------------------------------------------------------------------
MAPATTR FLDNOGRID_MAPPER_GetAttr( const FLDNOGRID_MAPPER* cp_mapper, const RAIL_LOCATION* cp_location )
{
  GF_ASSERT( cp_mapper );
  return RAIL_ATTR_GetAttribute( cp_mapper->p_attr, cp_location );
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールマネージャモジュールの取得
 *
 *	@param	cp_mapper   マッパー
 *
 *	@return モジュール
 */
//-----------------------------------------------------------------------------
const FIELD_RAIL_MAN* FLDNOGRID_MAPPER_GetRailMan( const FLDNOGRID_MAPPER* cp_mapper )
{
  GF_ASSERT( cp_mapper );
  return cp_mapper->p_railMan;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エリアマネージャモジュールの取得
 *
 *	@param	cp_mapper   マッパー
 *
 *	@return モジュール
 */
//-----------------------------------------------------------------------------
const FLD_SCENEAREA* FLDNOGRID_MAPPER_GetSceneAreaMan( const FLDNOGRID_MAPPER* cp_mapper )
{
  GF_ASSERT( cp_mapper );
  return cp_mapper->p_areaMan;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールローダーモジュールの取得
 *
 *	@param	cp_mapper   マッパー
 *
 *	@return モジュール
 */
//-----------------------------------------------------------------------------
const FIELD_RAIL_LOADER* FLDNOGRID_MAPPER_GetRailLoader( const FLDNOGRID_MAPPER* cp_mapper )
{
  GF_ASSERT( cp_mapper );
  return cp_mapper->p_railLoader;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エリアローダーのモジュール取得
 *
 *	@param	cp_mapper   マッパー
 *
 *	@return モジュール
 */
//-----------------------------------------------------------------------------
const FLD_SCENEAREA_LOADER* FLDNOGRID_MAPPER_GetSceneAreaLoader( const FLDNOGRID_MAPPER* cp_mapper )
{
  GF_ASSERT( cp_mapper );
  return cp_mapper->p_areaLoader;
}






