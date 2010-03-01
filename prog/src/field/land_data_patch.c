//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		land_data_patch.h
 *	@brief  マップの基本的なアトリビュート、配置データを書き換え、追加設定
 *	@author	tomoya takahashi
 *	@date		2009.11.30
 *
 *	モジュール名：FIELD_DATA_PATCH
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "arc/arc_def.h"
#include "arc/fieldmap/field_landdata_patch.naix"
#include "arc/fieldmap/land_data_patch_def.h"

#include "field/field_const.h"

#include "land_data_patch.h"

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
///	LAND_DATA_PATCH
//=====================================
typedef struct 
{
  u16 ofs_attr;   // アトリビュートへのオフセット
  u16 ofs_3dmd;   // 配置情報へのオフセット
} LAND_DATA_PATCH_HEADER;


//-------------------------------------
///	  LAND_DATAパッチ情報
//=====================================
struct _FIELD_DATA_PATCH
{
  LAND_DATA_PATCH_HEADER* p_file;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
// アトリビュート情報の取
static NormalVtxFormat* FIELD_LAND_DATA_PATCH_GetAttrData( const FIELD_DATA_PATCH* cp_sys );
static LayoutFormat* FIELD_LAND_DATA_PATCH_Get3DmdData( const FIELD_DATA_PATCH* cp_sys );


//----------------------------------------------------------------------------
/**
 *	@brief  マップ情報の部分書き換えパッチ  生成
 *
 *	@param	data_id   データID
 *	@param	heapID    ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
FIELD_DATA_PATCH* FIELD_DATA_PATCH_Create( u32 data_id, HEAPID heapID )
{
  FIELD_DATA_PATCH* p_wk;
  
  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_DATA_PATCH) );

  GF_ASSERT( data_id < LAND_DATA_PATCH_MAX );
  p_wk->p_file = GFL_ARC_UTIL_Load( ARCID_LAND_DATA_PATCH, data_id, FALSE, heapID );

  TOMOYA_Printf( "load_id %d\n", data_id );

  TOMOYA_Printf( "attr ofs 0x%x\n", p_wk->p_file->ofs_attr );
  TOMOYA_Printf( "3dmd ofs 0x%x\n", p_wk->p_file->ofs_3dmd );
  
  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップ情報の部分書き換えパッチ  破棄
 *
 *	@param	p_sys   システム
 */
//-----------------------------------------------------------------------------
void FIELD_DATA_PATCH_Delete( FIELD_DATA_PATCH* p_sys )
{
  GFL_HEAP_FreeMemory( p_sys->p_file );
  GFL_HEAP_FreeMemory( p_sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップ情報の部分書き換えパッチ　アトリビュート上書き
 *
 *	@param	cp_sys    システム
 *	@param	p_buff    アトリビュートバッファ
 */
//-----------------------------------------------------------------------------
void FIELD_DATA_PATCH_OverWriteAttr( const FIELD_DATA_PATCH* cp_sys, NormalVtxFormat* p_buff, u32 grid_x, u32 grid_z )
{
  int i, j;
  int write_index;
  int read_index;
  const NormalVtxFormat* cp_attr;
  NormalVtxSt* p_write_buff;
  const NormalVtxSt* cp_read_buff;
  u32 addr;

  // 読み込みバッファ取得
  cp_attr = FIELD_LAND_DATA_PATCH_GetAttrData( cp_sys );
  
  addr = (u32)p_buff;
  addr += sizeof(NormalVtxFormat);
  p_write_buff = (NormalVtxSt*)(addr);

  addr = (u32)cp_attr;
  addr += sizeof(NormalVtxFormat);
  cp_read_buff = (const NormalVtxSt*)(addr); //ヘッダー分ずらす

  TOMOYA_Printf( "attr height %d width %d\n", cp_attr->height, cp_attr->width );
  
  // 情報を上書き
  for( i=0; i<cp_attr->height; i++ )
  {
    // 横ライン一気に書き込み
    // 書き出し先
    write_index = ((grid_z + i) * p_buff->width) + (grid_x);
    GF_ASSERT( (grid_z + i) < p_buff->height );
    GF_ASSERT( (grid_x + cp_attr->width) <= p_buff->width );
      
    // 読み込み先
    read_index = (i*cp_attr->width);

    GFL_STD_MemCopy( &cp_read_buff[read_index], &p_write_buff[write_index], sizeof(NormalVtxSt) * cp_attr->width );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  サイズ分のアトリビュートを上書きする
 *
 *	@param	cp_sys          システム
 *	@param	p_buff          バッファ
 *	@param	read_grid_x     読み込みｘグリッド
 *	@param	read_grid_z     読み込みｚグリッド
 *	@param	write_grid_x    書き込みｘグリッド
 *	@param	write_grid_z    書き込みｚグリッド
 *	@param  size_grid_x     上書きｘグリッドサイズ
 *	@param  size_grid_z     上書きｚグリッドサイズ
 *	
 */
//-----------------------------------------------------------------------------
void FIELD_DATA_PATCH_OverWriteAttrEx( const FIELD_DATA_PATCH* cp_sys, NormalVtxFormat* p_buff, u32 read_grid_x, u32 read_grid_z, u32 write_grid_x, u32 write_grid_z, u32 size_grid_x, u32 size_grid_z )
{
  int i, j;
  int write_index;
  int read_index;
  const NormalVtxFormat* cp_attr;
  NormalVtxSt* p_write_buff;
  const NormalVtxSt* cp_read_buff;
  u32 addr;

  // 読み込みバッファ取得
  cp_attr = FIELD_LAND_DATA_PATCH_GetAttrData( cp_sys );
  
  addr = (u32)p_buff;
  addr += sizeof(NormalVtxFormat);
  p_write_buff = (NormalVtxSt*)(addr);

  addr = (u32)cp_attr;
  addr += sizeof(NormalVtxFormat);
  cp_read_buff = (const NormalVtxSt*)(addr); //ヘッダー分ずらす

  TOMOYA_Printf( "attr height %d width %d\n", cp_attr->height, cp_attr->width );
  GF_ASSERT( cp_attr->height >= (read_grid_z + size_grid_z) );
  GF_ASSERT( cp_attr->width >= (read_grid_x + size_grid_x) );

  // 情報を上書き
  for( i=0; i<size_grid_z; i++ )
  {
    // 横ライン一気に書き込み
    // 書き出し先
    write_index = ((write_grid_z + i) * p_buff->width) + (write_grid_x);
    GF_ASSERT( (write_grid_z + i) < p_buff->height );
    GF_ASSERT( (write_grid_x + size_grid_x) <= p_buff->width );
      
    // 読み込み先
    read_index = ((read_grid_z + i)*cp_attr->width) + read_grid_x;

    GFL_STD_MemCopy( &cp_read_buff[read_index], &p_write_buff[write_index], sizeof(NormalVtxSt) * size_grid_x );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  マップ情報の部分書き換えパッチ  配置モデルの追加登録
 *
 *	@param	cp_sys          システム
 *	@param	p_bmodel        配置モデル管理システム
 *	@param	g3Dmap          ３Dマップ情報
 *	@param	count_start     配置モデル登録開始カウント値
 *	@param  grid_x          ｘグリッド
 *	@param  grid_z          ｚグリッド
 *
 *	@return モデル登録　終了カウント値
 */
//-----------------------------------------------------------------------------
int FIELD_LAND_DATA_PATCH_AddBuildModel( const FIELD_DATA_PATCH* cp_sys, FIELD_BMODEL_MAN * p_bmodel, FLD_G3D_MAP * g3Dmap, int count_start, u32 grid_x, u32 grid_z )
{
  const LayoutFormat* cp_3dmd;
  const PositionSt* cp_position;
  PositionSt regist;
  int i;

  // 配置情報を取得
  cp_3dmd = FIELD_LAND_DATA_PATCH_Get3DmdData( cp_sys );
  cp_position = (const PositionSt*)&cp_3dmd->posData;

  TOMOYA_Printf( "3dmd count %d\n", cp_3dmd->count );

  TOMOYA_Printf( "grid x=%d z=%d\n", grid_x, grid_z );

  // 追加登録
  for( i=0; i<cp_3dmd->count; i++ )
  {
    GFL_STD_MemCopy( &cp_position[i], &regist, sizeof(PositionSt) );
    
    regist.xpos += GRID_TO_FX32( grid_x );
    regist.zpos -= GRID_TO_FX32( grid_z );

    TOMOYA_Printf( "bmodel idx = %d\n", regist.resourceID );
    TOMOYA_Printf( "regist pos x=%d z=%d\n", FX_Whole( regist.xpos ), FX_Whole( regist.zpos ) );
    FIELD_BMODEL_MAN_ResistMapObject( p_bmodel, g3Dmap, &regist, count_start + i);
  }

  return count_start + cp_3dmd->count;
}







//-----------------------------------------------------------------------------
/**
 *        private関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  アトリビュート情報を取得
 *
 *	@param	cp_sys  システム
 *
 *	@return アトリビュート情報
 */
//-----------------------------------------------------------------------------
static NormalVtxFormat* FIELD_LAND_DATA_PATCH_GetAttrData( const FIELD_DATA_PATCH* cp_sys )
{
  u32 addr;
  
  addr = (u32)cp_sys->p_file;
  addr += cp_sys->p_file->ofs_attr;

  return (NormalVtxFormat*)(addr);
}

//----------------------------------------------------------------------------
/**
 *	@brief  配置情報を取得
 *
 *	@param	cp_sys  システム
 *
 *	@return 配置情報
 */
//-----------------------------------------------------------------------------
static LayoutFormat* FIELD_LAND_DATA_PATCH_Get3DmdData( const FIELD_DATA_PATCH* cp_sys )
{
  u32 addr;
  
  addr = (u32)cp_sys->p_file;
  addr += cp_sys->p_file->ofs_3dmd;

  return (LayoutFormat*)(addr);
}

