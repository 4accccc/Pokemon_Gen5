//======================================================================
/**
 * @file  map_attr.c
 * @brief	マップアトリビュート
 * @author kagaya
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "field_g3d_mapper.h"
#include "map_attr.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================

//======================================================================
//  マップアトリビュート
//======================================================================
//--------------------------------------------------------------
/**
 * 指定位置のアトリビュートを取得
 * @param mapper FLDMAPPER
 * @retval MAPATTR MAPATTR_ERROR=取得エラー
 */
//--------------------------------------------------------------
MAPATTR MAPATTR_GetAttribute( FLDMAPPER *mapper, const VecFx32 *pos )
{
  MAPATTR attr = MAPATTR_ERROR;
  FLDMAPPER_GRIDINFO gridInfo;
  
  if( FLDMAPPER_GetGridInfo(mapper,pos,&gridInfo) == TRUE ){
    attr = gridInfo.gridData[0].attr;  
  }
  
  return( attr );
}

//--------------------------------------------------------------
/**
 * MAPATTRからMAPATTR_VALUEを取得
 * @param attr MAPATTR
 * @retval MAPATTR_VALUE
 */
//--------------------------------------------------------------
MAPATTR_VALUE MAPATTR_GetAttrValue( const MAPATTR attr )
{
  MAPATTR_VALUE val = attr & 0xffff;
  return( val );
}

//--------------------------------------------------------------
/**
 * MAPATTRからMAPATTR_FLAGを取得
 * @param attr MAPATTR
 * @retval MAPATTR_FLAG
 */
//--------------------------------------------------------------
MAPATTR_FLAG MAPATTR_GetAttrFlag( const MAPATTR attr )
{
  MAPATTR_FLAG flag = (attr >> 16) & 0xffff;
  return( flag );
}

//======================================================================
//  アトリビュートバリュー　識別
//======================================================================
//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　長い草
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckLongGrass( const MAPATTR_VALUE val )
{
  return (val == 0x06 || val == 0x07);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　上ジャンプアトリビュート
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckJumpUp( const MAPATTR_VALUE val )
{
  return (val == 0x74);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　下ジャンプアトリビュート
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckJumpDown( const MAPATTR_VALUE val )
{
  return (val == 0x75);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　左ジャンプアトリビュート
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckumpLeft( const MAPATTR_VALUE val )
{
  return (val == 0x73);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　右ジャンプアトリビュート
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckJumpRight( const MAPATTR_VALUE val )
{
  return (val == 0x72);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック カウンター
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckCounter( const MAPATTR_VALUE val )
{
  return (val == 0xd4);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック マット
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckMat( const MAPATTR_VALUE val )
{
  return (val == 0xd5);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック パソコン
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckPC( const MAPATTR_VALUE val )
{
  return (val == 0xd6);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 世界地図
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckMap( const MAPATTR_VALUE val )
{
  return (val == 0xd7);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック テレビ
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckTV( const MAPATTR_VALUE val )
{
  return (val == 0xd8);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 本棚その１
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckBookShelf1( const MAPATTR_VALUE val )
{
  return (val == 0xd9);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 本棚その２
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckBookShelf2( const MAPATTR_VALUE val )
{
  return (val == 0xda);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 本棚その３
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckBookShelf3( const MAPATTR_VALUE val )
{
  return (val == 0xdb);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 本棚その４
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckBookShelf4( const MAPATTR_VALUE val )
{
  return (val == 0xdc);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 壷
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckVase( const MAPATTR_VALUE val )
{
  return (val == 0xdd);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック ゴミ箱
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckDustBox( const MAPATTR_VALUE val )
{
  return (val == 0xde);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック  商品棚その１
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckShopShelf1( const MAPATTR_VALUE val )
{
  return (val == 0xdf);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック  商品棚その２
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckShopShelf2( const MAPATTR_VALUE val )
{
  return (val == 0xe0);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック  商品棚その３
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckShopShelf3( const MAPATTR_VALUE val )
{
  return (val == 0xe1);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウントA1
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrassA1( const MAPATTR_VALUE val )
{
  return (val == 0x02);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウントA2
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrassA2( const MAPATTR_VALUE val )
{
  return (val == 0x03);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウントB1
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrassB1( const MAPATTR_VALUE val )
{
  return (val == 0x04);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウントB2
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrassB2( const MAPATTR_VALUE val )
{
  return (val == 0x05);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウントA群(A1,A2)
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrassA( const MAPATTR_VALUE val )
{
  if( MAPATTR_VALUE_CheckEncountGrassA1(val) == TRUE ||
      MAPATTR_VALUE_CheckEncountGrassA2(val) == TRUE ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウントB群(B1,B2)
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrassB( const MAPATTR_VALUE val )
{
  if( MAPATTR_VALUE_CheckEncountGrassB1(val) == TRUE ||
      MAPATTR_VALUE_CheckEncountGrassB2(val) == TRUE ){
    return( TRUE );
  }
  return( FALSE );
}

