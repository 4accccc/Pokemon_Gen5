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
#include "../land_data_patch.h"
#include "../eventdata_local.h"
#include "arc/fieldmap/eventdata.naix"

//resource
#include "arc_def.h"
#include "resonance_resort_data.naix"

//mine
#include "resort_binary.h"

//outer
#include "resort_mapcreate.h"



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
  RESORT_BINARY_INSTITUTION *p_institution_data;
  RESORT_BINARY_SHOP *p_shop_data;
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

  p_wk->p_institution_data = RESORT_BINARY_INSTITUTION_Init( heapID );
  p_wk->p_shop_data = RESORT_BINARY_SHOP_Init( heapID );

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
  RESORT_BINARY_SHOP_Exit( p_wk->p_shop_data );
  RESORT_BINARY_INSTITUTION_Exit( p_wk->p_institution_data );

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
 *	@param	map_index   マップインデックス
 *	@param	heapID    ヒープID 
 *
 *	@retval build_count 追加後
 */
//-----------------------------------------------------------------------------
extern int RESORT_MAPCREATE_SetUpBlock( RESORT_MAPCREATE *p_wk, NormalVtxFormat* p_attr, FIELD_BMODEL_MAN* p_bm, FLD_G3D_MAP* g3Dmap, int build_count, int map_index, HEAPID heapID )
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

#endif
  if( p_wk->cp_city == NULL || p_wk->cp_people_all == NULL )
  {
    return 0;
  }
  else
  {
  int i;

  FIELD_DATA_PATCH* p_patch = FIELD_DATA_PATCH_Create( LAND_DATA_PATCH_RESORT_INSTITUTION_ON, GFL_HEAP_LOWID(heapID) );

  //住人の数だけ配置を行う
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    u16 grid_x = RESORT_BINARY_INSTITUTION_GetData( p_wk->p_institution_data, i, RESORT_BINARY_INSTITUTION_PARAM_GRID_X );
    u16 grid_z = RESORT_BINARY_INSTITUTION_GetData( p_wk->p_institution_data, i, RESORT_BINARY_INSTITUTION_PARAM_GRID_Z );

    //住民が存在して、住民の施設がブロック内にある場合に配置モデルを3dmdを配置
    //住民が存在しなくてブロック内の場合、ドアを消去
    //ブロック外のときは処理をしない

    if( map_index == (grid_x / 32 + (grid_z / 32)*2 ) )
    {
      const RESORT_RESIDENT_DATA *cp_resident = RESORT_PEOPLE_ALL_DATA_GetResident( (RESORT_PEOPLE_ALL_DATA *)p_wk->cp_people_all, i);
      if( !RESORT_RESIDENT_DATA_IsEmpty( cp_resident ) )
      {
        const RESORT_BINARY_SHOP_DATA *cp_binary_shop = RESORT_BINARY_SHOP_SearchDataByData( p_wk->p_shop_data, (RESORT_RESIDENT_DATA *)cp_resident );
        u16 bmodel = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop, RESORT_BINARY_SHOP_PARAM_OUT_MODEL );
        // g3Dmapローカルのグリッド座標を求める
        u32 local_grid_x = grid_x % 32;
        u32 local_grid_z = grid_z % 32;

        //建物を書き換え
        FIELD_DATA_PATCH_WriteBuildModel( p_patch, bmodel, 0 );

        // アトリビュートのオーバーライト
        FIELD_DATA_PATCH_OverWriteAttrEx( p_patch, p_attr, 0, 0, local_grid_x, local_grid_z, 7, 5 );

        //3dmdを配置
        build_count = FIELD_LAND_DATA_PATCH_AddBuildModel( p_patch, p_bm, g3Dmap, build_count, local_grid_x, local_grid_z );
      }
      else
      {
        //ドアを消す
        //u16 doorID = RESORT_BINARY_INSTITUTION_GetData( p_wk->p_institution_data, i, RESORT_BINARY_INSTITUTION_PARAM_DOOR_ID );
      }
    }
  }

  FIELD_DATA_PATCH_Delete( p_patch );
  }

  return build_count;
}
