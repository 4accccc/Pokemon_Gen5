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
#include "field/zonedata.h"

//resource
#include "arc_def.h"
#include "resonance_resort_data.naix"
#include "arc/fieldmap/eventdata.naix"

//mine
#include "resort_binary.h"
#include "resort_map.h"

//outer
#include "resort_mapcreate.h"



//=============================================================================
/**
 *					定数
*/
//=============================================================================
#define RESORT_MAPCREATE_ZONEID_NULL  (0xFFFF)

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
  HEAPID heapID;
  u32 zoneID;
  const RESORT_CITY_DATA *cp_city;
  const RESORT_PEOPLE_ALL_DATA *cp_people_all;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static int Resort_Mapcreate_SetUpBlockField( RESORT_MAPCREATE *p_wk, NormalVtxFormat* p_attr, FIELD_BMODEL_MAN* p_bm, FLD_G3D_MAP* g3Dmap, int build_count, int map_index, HEAPID heapID );


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
  p_wk->heapID = heapID;
  p_wk->zoneID = RESORT_MAPCREATE_ZONEID_NULL;

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
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief　マップ生成セットアップ
 *
 *	@param	RESORT_MAPCREATE *p_wk  ワーク
 *  @param  u16 zoneID
 *	@param	RESORT_CITY_DATA *cp_city 街データ
 *	@param	RESORT_PEOPLE_ALL_DATA *cp_people_all   人物データ
 */
//-----------------------------------------------------------------------------
void RESORT_MAPCREATE_Setup( RESORT_MAPCREATE *p_wk, u16 zoneID, const RESORT_CITY_DATA *cp_city, const RESORT_PEOPLE_ALL_DATA *cp_people_all )
{
  p_wk->zoneID = zoneID;
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
int RESORT_MAPCREATE_SetUpBlock( RESORT_MAPCREATE *p_wk, NormalVtxFormat* p_attr, FIELD_BMODEL_MAN* p_bm, FLD_G3D_MAP* g3Dmap, int build_count, int map_index, HEAPID heapID )
{
  if( ZONEDATA_IsResonanceResort( p_wk->zoneID ) )
  {
    return Resort_Mapcreate_SetUpBlockField( p_wk, p_attr, p_bm, g3Dmap, build_count, map_index, heapID );
  }
  else
  {
    return build_count;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップ設定ブロック配置 フィールド
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
static int Resort_Mapcreate_SetUpBlockField( RESORT_MAPCREATE *p_wk, NormalVtxFormat* p_attr, FIELD_BMODEL_MAN* p_bm, FLD_G3D_MAP* g3Dmap, int build_count, int map_index, HEAPID heapID )
{
  int i;

  RESORT_BINARY_SHOP *p_shop_data = RESORT_BINARY_SHOP_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  RESORT_BINARY_INSTITUTION *p_institution_data = RESORT_BINARY_INSTITUTION_Init( GFL_HEAP_LOWID(p_wk->heapID) );

  //住人の数だけ配置を行う
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    u16 grid_x = RESORT_BINARY_INSTITUTION_GetData( p_institution_data, i, RESORT_BINARY_INSTITUTION_PARAM_GRID_X );
    u16 grid_z = RESORT_BINARY_INSTITUTION_GetData( p_institution_data, i, RESORT_BINARY_INSTITUTION_PARAM_GRID_Z );

    u16 size_x  = 6;
    u16 size_origin_z = 7;
    u16 size_z  = size_origin_z;

    //住民が存在して、住民の施設がブロック内にある場合に配置モデルを3dmdを配置
    //ブロック外のときは処理をしない

    if( map_index == (grid_z / 32) || map_index == (grid_z+size_z)/32 )
    {
      const RESORT_RESIDENT_DATA *cp_resident = RESORT_PEOPLE_ALL_DATA_GetResident( (RESORT_PEOPLE_ALL_DATA *)p_wk->cp_people_all, i);
      if( !RESORT_RESIDENT_DATA_IsEmpty( cp_resident ) )
      {
        const RESORT_BINARY_SHOP_DATA *cp_binary_shop = RESORT_BINARY_SHOP_SearchDataByData( p_shop_data, (RESORT_RESIDENT_DATA *)cp_resident );
        u16 patch;
        FIELD_DATA_PATCH* p_patch;

        // g3Dmapローカルのグリッド座標を求める
        u32 local_grid_x = grid_x % 32;
        u32 local_grid_z = grid_z % 32;
        u32 read_grid_x = 0;
        u32 read_grid_z = 0;

        BOOL is_build = TRUE;

        if( i % 2 == 1 )
        {
          patch = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop, RESORT_BINARY_SHOP_PARAM_3DMD_L );
        }
        else
        {
          patch = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop, RESORT_BINARY_SHOP_PARAM_3DMD_R );
        }

        p_patch = FIELD_DATA_PATCH_Create( patch, GFL_HEAP_LOWID(heapID) );


        if( map_index == (grid_z / 32) )
        {
          //開始座標が入っていた場合、
          //サイズがオーバーしていれば縮める
          if( local_grid_z + size_z >= 32 )
          {
            size_z  = 32 - local_grid_z;
          }
        }
        else
        {
          //後尾座標が入っていなかった場合
          read_grid_z   = 32 - local_grid_z;
          size_z        = size_z - read_grid_z;
          local_grid_z  = 0;
          is_build  = FALSE;  //0版の施設がマップをまたいでいるので描画しない
        }

        // アトリビュートのオーバーライト
        //NAGI_Printf( "rx%d rz%d lx%d lz%d sx%d sz%d \n", read_grid_x, read_grid_z, local_grid_x, local_grid_z, size_x, size_z );
        FIELD_DATA_PATCH_OverWriteAttrEx( p_patch, p_attr, read_grid_x, read_grid_z, local_grid_x, local_grid_z, size_x, size_z );

        //3dmdを配置
        if( is_build )
        {
          build_count = FIELD_LAND_DATA_PATCH_AddBuildModel( p_patch, p_bm, g3Dmap, build_count, local_grid_x, local_grid_z );
        }

        FIELD_DATA_PATCH_Delete( p_patch );
      }
    }
  }

  RESORT_BINARY_INSTITUTION_Exit( p_institution_data );
  RESORT_BINARY_SHOP_Exit( p_shop_data );

  return build_count;
}


//----------------------------------------------------------------------------
/**
 *	@brief イベントデータの上書きを行う
 *
 *	@param	RESORT_MAPCREATE *p_wk  ワーク
 *	@param	p_evdata                イベントデータ
 */
//-----------------------------------------------------------------------------
void RESORT_MAPCREATE_EventDataOverWrite( RESORT_MAPCREATE *p_wk, EVENTDATA_SYSTEM* p_evdata )
{
  int i;

  RESORT_BINARY_SHOP *p_shop_data = RESORT_BINARY_SHOP_Init( GFL_HEAP_LOWID(p_wk->heapID) );
  RESORT_BINARY_INSTITUTION *p_institution_data = RESORT_BINARY_INSTITUTION_Init( GFL_HEAP_LOWID(p_wk->heapID) );

  //住人の数だけ配置を行う
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( (RESORT_PEOPLE_ALL_DATA *)p_wk->cp_people_all, i);

    u16 bgID = RESORT_BINARY_INSTITUTION_GetData( p_institution_data, i, RESORT_BINARY_INSTITUTION_PARAM_BOARD_ID );

    if( RESORT_RESIDENT_DATA_IsEmpty( p_resident ) )
    {
      //空なら施設がない＝看板がない
      EVENTDATA_MoveBGData( p_evdata, bgID, 0, 0, 0 );
    }
    else
    {
      RESORT_INSTITUTION_DATA *p_insti = RESORT_RESIDENT_DATA_GetInstitution( p_resident );
      u16 id  = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data_one = RESORT_BINARY_SHOP_GetData( p_shop_data,id);
      u16 data_3dmd = RESORT_BINARY_SHOP_DATA_GetData(cp_shop_data_one,RESORT_BINARY_SHOP_PARAM_3DMD_L+!(i%2));

      u16 x;
      u16 z;
      RESORT_MAP_GetBoardGridPosOfs( data_3dmd, &x, &z );

      x += RESORT_BINARY_INSTITUTION_GetData( p_institution_data, i, RESORT_BINARY_INSTITUTION_PARAM_GRID_X );
      z += RESORT_BINARY_INSTITUTION_GetData( p_institution_data, i, RESORT_BINARY_INSTITUTION_PARAM_GRID_Z );

      EVENTDATA_MoveBGData( p_evdata, bgID, x, 0, z );
    }
  }

  RESORT_BINARY_INSTITUTION_Exit( p_institution_data );
  RESORT_BINARY_SHOP_Exit( p_shop_data );
}

