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
 * MAPATTRからMAPATTR_VALを取得
 * @param attr MAPATTR
 * @retval MAPATTR_VAL
 */
//--------------------------------------------------------------
MAPATTR_VAL MAPATTR_GetAttrValue( const MAPATTR attr )
{
  MAPATTR_VAL val = attr & 0xffff;
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
 * @param val MAPATTR_VAL
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_CheckAttrValueLongGrass( const MAPATTR_VAL val )
{
  if( val == 0x06 || val == 0x07 ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　上ジャンプアトリビュート
 * @param val MAPATTR_VAL
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_CheckAttrValueJumpUp( const MAPATTR_VAL val )
{
  if( val == 0x74 ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　下ジャンプアトリビュート
 * @param val MAPATTR_VAL
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_CheckAttrValueJumpDown( const MAPATTR_VAL val )
{
  if( val == 0x75 ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　左ジャンプアトリビュート
 * @param val MAPATTR_VAL
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_CheckAttrValueJumpLeft( const MAPATTR_VAL val )
{
  if( val == 0x73 ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　右ジャンプアトリビュート
 * @param val MAPATTR_VAL
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_CheckAttrValueJumpRight( const MAPATTR_VAL val )
{
  if( val == 0x72 ){
    return( TRUE );
  }
  return( FALSE );
}
