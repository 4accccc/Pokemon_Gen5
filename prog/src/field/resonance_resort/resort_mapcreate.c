//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_mapcreate.c
 *	@brief  レゾナンスリゾートランダムマップ作成
 *	@author	Toru=Nagihashi
 *	@data		2011.03.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include "gflib.h"

//resource
#include "arc_def.h"
#include "resonance_resort_data.naix"

//outer
#include "resort_mapcreate.h"

//=============================================================================
/**
 *					施設データ
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
typedef struct _RESORT_BINARY_INSTITUTION RESORT_BINARY_INSTITUTION;

//-------------------------------------
///	取得データ
//=====================================
typedef enum
{
  RESORT_BINARY_INSTITUTION_PARAM_NUMBER,
  RESORT_BINARY_INSTITUTION_PARAM_GRID_X,
  RESORT_BINARY_INSTITUTION_PARAM_GRID_Z,
  RESORT_BINARY_INSTITUTION_PARAM_DOOR_ID,
  RESORT_BINARY_INSTITUTION_PARAM_MAX,
}RESORT_BINARY_INSTITUTION_PARAM;

//-------------------------------------
///	PUBLIC
//=====================================
static RESORT_BINARY_INSTITUTION * RESORT_BINARY_INSTITUTION_Init( HEAPID heapID );
static void RESORT_BINARY_INSTITUTION_Exit( RESORT_BINARY_INSTITUTION *p_wk );
static u16 RESORT_BINARY_INSTITUTION_GetData( const RESORT_BINARY_INSTITUTION *cp_wk, u32 index, RESORT_BINARY_INSTITUTION_PARAM param );


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
struct _RESORT_MAPCREATE
{
  RESORT_BINARY_INSTITUTION *p_data;
  const RESORT_CITY_DATA *cp_city;
  const RESORT_PEOPLE_ALL_DATA *cp_people_all;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief　マップ生成システム  作成
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
RESORT_MAPCREATE * RESORT_MAPCREATE_Init( HEAPID heapID )
{
  RESORT_MAPCREATE *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_MAPCREATE));

  p_wk->p_data = RESORT_BINARY_INSTITUTION_Init( heapID );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  マップ生成システム  破棄
 *
 *	@param	RESORT_MAPCREATE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_MAPCREATE_Exit( RESORT_MAPCREATE *p_wk )
{
  RESORT_BINARY_INSTITUTION_Exit( p_wk->p_data );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief　マップ生成セットアップ
 *
 *	@param	RESORT_MAPCREATE *p_wk  ワーク
 *	@param	RESORT_CITY_DATA *cp_city 街データ
 *	@param	RESORT_PEOPLE_ALL_DATA *cp_people_all   人物データ
 */
//-----------------------------------------------------------------------------
void RESORT_MAPCREATE_Setup( RESORT_MAPCREATE *p_wk, const RESORT_CITY_DATA *cp_city, const RESORT_PEOPLE_ALL_DATA *cp_people_all )
{
  p_wk->cp_city = cp_city;
  p_wk->cp_people_all = cp_people_all;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップ設定ブロック配置
 *
 *	@param	p_wk     ワーク
 *	@param  p_attr    アトリビュート
 *	@param	p_bm      buildmodel
 *	@param	g3Dmap    マップブロック管理
 *	@param  build_count 配置モデルカウント
 *	@param	block_x   ブロックｘインデックス
 *	@param	block_z   ブロックｚインデックス
 *	@param  score     ブロックスコア
 *	@param	heapID    ヒープID 
 *
 *	@retval build_count 追加後
 */
//-----------------------------------------------------------------------------
int RESORT_MAPCREATE_SetUpBlock( RESORT_MAPCREATE *p_wk, NormalVtxFormat* p_attr, FIELD_BMODEL_MAN* p_bm, FLD_G3D_MAP* g3Dmap, int build_count, u32 block_x, u32 block_z, HEAPID heapID )
{
#if 0
  WFBC_BLOCK_NOW map_data;
  u32 local_grid_x, local_grid_z;
  FIELD_DATA_PATCH* p_patch;
  

  map_data = WFBC_NOW_MAPDATA_GetData( &p_wk->map_data, block_x, block_z );

  
  if( map_data.block_tag.block_no != FIELD_WFBC_BLOCK_NONE )
  {
    NAGI_Printf( "block_tag %d\n", map_data.block_tag.block_no );
    NAGI_Printf( "block pos x %d  z %d\n", map_data.block_tag.pos_x, map_data.block_tag.pos_z );
    NAGI_Printf( "block_x=%d  block_z=%d\n", block_x, block_z );


    // g3Dmapローカルのグリッド座標を求める
    local_grid_x = FIELD_WFBC_BLOCK_TO_GRID_X( block_x ) % 32;
    local_grid_z = FIELD_WFBC_BLOCK_TO_GRID_Z( block_z ) % 32;
    NAGI_Printf( "local_grid_x %d\n", local_grid_x );
    NAGI_Printf( "local_grid_z %d\n", local_grid_z );

    NAGI_Printf( "build_count %d\n", build_count );
    
    // パッチ情報読み込み
    p_patch = FIELD_DATA_PATCH_Create( map_data.block_id, GFL_HEAP_LOWID(heapID) );

    // アトリビュートのオーバーライト
    FIELD_DATA_PATCH_OverWriteAttrEx( p_patch, p_attr, map_data.block_tag.pos_x*8, map_data.block_tag.pos_z*8, local_grid_x, local_grid_z, 8, 8 );

    // 配置上書き
    if( (map_data.block_tag.pos_x == 0) && (map_data.block_tag.pos_z == 0) )
    {
      // 左上のブロック情報のときにのみ設定
      build_count = FIELD_LAND_DATA_PATCH_AddBuildModel( p_patch, p_bm, g3Dmap, build_count, local_grid_x, local_grid_z );
    }

    // パッチ情報破棄
    FIELD_DATA_PATCH_Delete( p_patch );
  }
  u32 local_grid_x, local_grid_z;
  FIELD_DATA_PATCH* p_patch;

  // g3Dmapローカルのグリッド座標を求める
  local_grid_x =  (block_x *8) % 32;
  local_grid_z =  (block_z *8) % 32;
  NAGI_Printf( "local_grid_x %d\n", local_grid_x );
  NAGI_Printf( "local_grid_z %d\n", local_grid_z );

  NAGI_Printf( "build_count %d\n", build_count );

  // パッチ情報読み込み
  p_patch = FIELD_DATA_PATCH_Create( 46, GFL_HEAP_LOWID(heapID) );

  // アトリビュートのオーバーライト
  FIELD_DATA_PATCH_OverWriteAttrEx( p_patch, p_attr, map_data.block_tag.pos_x*8, map_data.block_tag.pos_z*8, local_grid_x, local_grid_z, 8, 8 );

  // 配置上書き
  if( (map_data.block_tag.pos_x == 0) && (map_data.block_tag.pos_z == 0) )
  {
    // 左上のブロック情報のときにのみ設定
    build_count = FIELD_LAND_DATA_PATCH_AddBuildModel( p_patch, p_bm, g3Dmap, build_count, local_grid_x, local_grid_z );
  }

  // パッチ情報破棄
  FIELD_DATA_PATCH_Delete( p_patch );


#endif
  return build_count;
}


//=============================================================================
/**
 *					施設データ
*/
//=============================================================================
//-------------------------------------
///	バイナリの中身
//=====================================
typedef struct
{
  u16 param[ RESORT_BINARY_INSTITUTION_PARAM_MAX ];
} RESORT_BINARY_INSTITUTION_DATA;

//-------------------------------------
///	ワーク
//=====================================
struct _RESORT_BINARY_INSTITUTION
{
  u32 size;
  RESORT_BINARY_INSTITUTION_DATA *p_data;
};

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  施設データ作成
 *
 *	@param	HEAPID heapID  ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static RESORT_BINARY_INSTITUTION * RESORT_BINARY_INSTITUTION_Init( HEAPID heapID )
{
  RESORT_BINARY_INSTITUTION *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_BINARY_INSTITUTION ) );

  p_wk->p_data = GFL_ARC_UTIL_LoadEx( ARCID_RESONANCE_RESORT_DATA, NARC_resonance_resort_data_resort_institution_dat, FALSE, heapID, &p_wk->size );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  施設データ破棄
 *
 *	@param	RESORT_BINARY_INSTITUTION *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_BINARY_INSTITUTION_Exit( RESORT_BINARY_INSTITUTION *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  施設データから取得
 *
 *	@param	const RESORT_BINARY_INSTITUTION *cp_wk  ワーク
 *	@param	index
 *	@param	param 取得する種類
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
static u16 RESORT_BINARY_INSTITUTION_GetData( const RESORT_BINARY_INSTITUTION *cp_wk, u32 index, RESORT_BINARY_INSTITUTION_PARAM param )
{
  GF_ASSERT( index < cp_wk->size / sizeof(RESORT_BINARY_INSTITUTION_DATA) );
  GF_ASSERT( param < RESORT_BINARY_INSTITUTION_PARAM_MAX );
  return cp_wk->p_data[ index ].param[ param ];
}

