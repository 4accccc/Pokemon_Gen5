//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		rail_attr.c
 *	@brief  レール　アトリビュート  
 *	@author	tomoya takahashi
 *	@date		2009.08.21
 *
 *	モジュール名：RAIL_ATTR
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "field_rail.h"

#include "rail_attr.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

#define RAIL_ATTR_ARCID ( 0 )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	アトリビュートデータヘッダー
//=====================================
typedef struct 
{
  u32 line_num;
} RAIL_ATTR_RESOURCE;


//-------------------------------------
///	ライン　アトリビュート
//=====================================
typedef struct 
{
  u16 x;      // 幅グリッド数
  u16 z;      // 奥行きグリッド数

  MAPATTR attr[];
} RAIL_ATTR_LINE;


//-------------------------------------
///	RAIL_ATTR_DATA
//=====================================
struct _RAIL_ATTR_DATA 
{
  // ファイルヘッダー
  RAIL_ATTR_RESOURCE* p_file;
  
  // ラインアトリビュート情報
  u32 line_max;
  RAIL_ATTR_LINE** pp_line;
};    


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  レールアトリビュートデータ　格納メモリ生成
 *
 *	@param	heapID  ヒープID
 *  
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
RAIL_ATTR_DATA* RAIL_ATTR_Create( u32 heapID )
{
  RAIL_ATTR_DATA* p_work;

  p_work = GFL_HEAP_AllocClearMemory( heapID, sizeof(RAIL_ATTR_DATA) );

  return p_work;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールアトリビュートデータ　格納メモリ破棄
 *
 *
 *	@param	p_work  ワーク
 */
//-----------------------------------------------------------------------------
void RAIL_ATTR_Delete( RAIL_ATTR_DATA* p_work )
{
  RAIL_ATTR_Release( p_work );
  GFL_HEAP_FreeMemory( p_work );
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールアトリビュートデータ  リソース読み込み
 *  
 *	@param	p_work    ワーク
 *	@param	datano    データナンバー
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
void RAIL_ATTR_Load( RAIL_ATTR_DATA* p_work, u32 datano, u32 heapID )
{
  int i;
  u32 size;
  
  GF_ASSERT( p_work );
  GF_ASSERT( p_work->p_file == NULL );
  
  // 情報の読み込み
  p_work->p_file = GFL_ARC_UTIL_Load( RAIL_ATTR_ARCID, datano, FALSE, heapID );

  // ライン分のバッファを生成
  p_work->line_max  = p_work->p_file->line_num;
  p_work->pp_line   = GFL_HEAP_AllocClearMemory( heapID, sizeof(RAIL_ATTR_LINE*)*p_work->line_max );

  // ポインタを設定
  size = sizeof( RAIL_ATTR_RESOURCE );
  for( i=0; i<p_work->line_max; i++ )
  {
    p_work->pp_line[i] = (RAIL_ATTR_LINE*)( ((u8*)p_work->p_file) + size );
    size += 4 + ( sizeof(MAPATTR) * (p_work->pp_line[i]->x * p_work->pp_line[i]->z) );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールアトリビュートデータ　リソース破棄
 *
 *	@param	p_work  ワーク
 */ 
//-----------------------------------------------------------------------------
void RAIL_ATTR_Release( RAIL_ATTR_DATA* p_work )
{
  if( p_work->p_file )
  {
    GF_ASSERT( p_work->pp_line );

    GFL_HEAP_FreeMemory( p_work->pp_line );
    GFL_HEAP_FreeMemory( p_work->p_file );

    p_work->pp_line = NULL;
    p_work->p_file = NULL;
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  レールアトリビュートデータ  読み込み情報の有無　取得
 *
 *	@param	cp_work   ワーク
 *
 *	@retval TRUE  あり
 *	@retval FALSE なし
 */
//-----------------------------------------------------------------------------
BOOL RAIL_ATTR_IsLoadData( const RAIL_ATTR_DATA* cp_work )
{
  GF_ASSERT( cp_work );

  if( cp_work->p_file )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールのアトリビュート取得
 *
 *	@param	cp_work       ワーク
 *	@param	cp_location   レールロケーション
 *
 *	@return アトリビュート
 */
//-----------------------------------------------------------------------------
MAPATTR RAIL_ATTR_GetAttribute( const RAIL_ATTR_DATA* cp_work, const RAIL_LOCATION* cp_location )
{
  const RAIL_ATTR_LINE* cp_lineattr;
  s32 x, z;
  s32 index;
  
  GF_ASSERT( cp_work );
  GF_ASSERT( cp_location );
  GF_ASSERT( cp_location->type == FIELD_RAIL_TYPE_LINE );
  GF_ASSERT( cp_work->line_max > cp_location->rail_index )
  
  // ラインアトリビュートデータの取得
  cp_lineattr = cp_work->pp_line[ cp_location->rail_index ];

  // width_gridは、-～+の数字なので、+のみの値にする。
  // 奇数のことも考慮し、%2もたす。
  // (奇数なら、+1)
  x = cp_location->width_grid + (cp_lineattr->x / 2) + (cp_lineattr->x % 2);
  z = cp_location->line_grid;

  index = (cp_lineattr->x * z) + x;
  
  // アトリビュートインデックスチェック
  GF_ASSERT( index < (cp_lineattr->x * cp_lineattr->z) );


  return cp_lineattr->attr[ index ];
}





