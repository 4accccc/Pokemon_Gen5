//======================================================================
/**
 * @file  map_attr.c
 * @brief	マップアトリビュート
 * @author kagaya
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "gamesystem/pm_season.h"
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
 * @param pos アトリビュートを取得したいx,z座標,yは現在位置(もしくは階層判定に用いたい基準値)
 * @retval MAPATTR MAPATTR_ERROR=取得エラー
 */
//--------------------------------------------------------------
MAPATTR MAPATTR_GetAttribute( const FLDMAPPER *mapper, const VecFx32 *pos )
{
  MAPATTR attr = MAPATTR_ERROR;
#if 0 //単層
  FLDMAPPER_GRIDINFO gridInfo;
  
  if( FLDMAPPER_GetGridInfo(mapper,pos,&gridInfo) == TRUE ){
    attr = gridInfo.gridData[0].attr;  
  }
#else //複層対応
  FLDMAPPER_GRIDINFODATA gridData;
  if( FLDMAPPER_GetGridData(mapper,pos,&gridData) == TRUE){
    return gridData.attr;
  }
#endif
  
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

//--------------------------------------------------------------
/**
 * @brief MAPATTRから、アトリビュートバリューの有効/無効を取得
 *
 * @param attr MAPATTR
 * @retval TRUE   有効 
 * @retval FALSE  無効
 */
//--------------------------------------------------------------
BOOL MAPATTR_IsEnable( const MAPATTR attr )
{
  u16 val = MAPATTR_GetAttrValue( attr );

  //アトリビュートそのもの、もしくはValueが無効かチェック
  if( (attr == MAPATTR_ERROR) ||
      (val == MAPATTR_VAL_ERR)){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief MAPATTRから進入可不可状態を取得
 *
 * @param attr MAPATTR
 * @retval TRUE 進入不可
 * @retval FALSE 進入可
 *
 * @com アトリビュート無効値も進入不可と返すため、純粋にHitchフラグのみを返しているのではありません
 */
//--------------------------------------------------------------
BOOL MAPATTR_GetHitchFlag( const MAPATTR attr )
{
  u16 flag = MAPATTR_GetAttrFlag( attr );

  //アトリビュートValueが無効か、進入不可フラグがOnなら通れない
  if( (MAPATTR_IsEnable( attr ) == FALSE) ||
      (flag & MAPATTR_FLAGBIT_HITCH)){
    return TRUE;
  }
  return FALSE;
}


//======================================================================
//  アトリビュートフラグ　識別
//======================================================================

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
  return (val == MATTR_E_LGRASS_LOW || val == MATTR_E_LGRASS_HIGH);
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
  return (val == MATTR_JUMP_UP);
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
  return (val == MATTR_JUMP_DOWN);
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
  return (val == MATTR_JUMP_LEFT);
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
  return (val == MATTR_JUMP_RIGHT);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　流砂
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckSandStream( const MAPATTR_VALUE val )
{
  return (val == MATTR_DRIFT_SAND_01);
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
  return (val == MATTR_COUNTER_01);
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
  return (val == MATTR_MAT_01);
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
  return (val == MATTR_PC_01);
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
  return (val == MATTR_WORLDMAP_01);
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
  return (val == MATTR_TV_01);
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
  return (val == MATTR_BOOKSHELF_01);
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
  return (val == MATTR_BOOKSHELF_02);
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
  return (val == MATTR_BOOKSHELF_03);
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
  return (val == MATTR_BOOKSHELF_04);
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
  return (val == MATTR_VASE_01);
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
  return (val == MATTR_DUST_BOX);
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
  return (val == MATTR_SHOPSHELF_01);
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
  return (val == MATTR_SHOPSHELF_02);
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
  return (val == MATTR_SHOPSHELF_03);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウントA1　草むら弱
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrassA1( const MAPATTR_VALUE val )
{
  return ( (val == MATTR_E_GRASS_LOW) || (val == MATTR_SLIP_GRASS_LOW) );  // 0xa1レール滑り降り草むら
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウントA2 長い草むら弱
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrassA2( const MAPATTR_VALUE val )
{
  return (val == MATTR_E_LGRASS_LOW);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウントB1　草むら強
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrassB1( const MAPATTR_VALUE val )
{
  return ( (val == MATTR_E_GRASS_HIGH) || (val == MATTR_SLIP_GRASS_HIGH) );  // 0xa2レール滑り降り草むら　強
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウントB2 長い草むら強
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrassB2( const MAPATTR_VALUE val )
{
  return (val == MATTR_E_LGRASS_HIGH);
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウント弱　A群(A1,A2)
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
 * アトリビュートバリュー チェック 草むらエンカウント強　B群(B1,B2)
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

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 草むらエンカウント
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGrass( const MAPATTR_VALUE val )
{
  if( MAPATTR_VALUE_CheckEncountGrassA(val) == TRUE ||
      MAPATTR_VALUE_CheckEncountGrassB(val) == TRUE ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 怪力穴
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckKairikiAna( const MAPATTR_VALUE val )
{
  return( val == MATTR_KAIRIKI_ANA );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 岸
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckShore( const MAPATTR_VALUE val )
{
  return( val == MATTR_SHORE_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 水たまり(四季無し)
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckPool( const MAPATTR_VALUE val )
{
  return( val == MATTR_POOL_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー チェック 浅瀬
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckShoal( const MAPATTR_VALUE val )
{
  return( val == MATTR_SHOAL_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　滝
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckWaterFall( const MAPATTR_VALUE val )
{
  return( val == MATTR_WATERFALL_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　雪
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckSnow( const MAPATTR_VALUE val )
{
  return( val == MATTR_SNOW_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　雪系アトリビュート
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckSnowType( const MAPATTR_VALUE val )
{
  if( MAPATTR_VALUE_CheckSnow(val) ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　氷
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckIce( const MAPATTR_VALUE val )
{
  return( val == MATTR_ICE_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　浅い湿原
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckMarsh( const MAPATTR_VALUE val )
{
  return( val == MATTR_MARSH_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　砂丘
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckSand( const MAPATTR_VALUE val )
{
  return( val == MATTR_SAND_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　砂丘
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckDesert( const MAPATTR_VALUE val )
{
  return( val == MATTR_E_DESERT_01 || val == MATTR_DESERT_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　砂系アトリビュート
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckSandType( const MAPATTR_VALUE val )
{
  if( MAPATTR_VALUE_CheckSand(val) || MAPATTR_VALUE_CheckDesert(val) ){
    return( TRUE );
  }
  return( FALSE );
}
//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　水系アトリビュート
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckWaterType( const MAPATTR_VALUE val )
{
  if( (val == MATTR_WATER_01) || (val == MATTR_WATER_S01) || (val == MATTR_SEA_01) || (val == MATTR_DEEP_MARSH_01) ) {
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　水系アトリビュート(四季変化考慮)
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckWaterTypeSeason( const MAPATTR_VALUE val, u8 season )
{
  if( (val == MATTR_WATER_01) || (val == MATTR_SEA_01) || (val == MATTR_DEEP_MARSH_01) ) {
    return( TRUE );
  }
  if( (val == MATTR_WATER_S01) && season != PMSEASON_WINTER ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　通常地面
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckMonoGround( const MAPATTR_VALUE val )
{
  return ( val == MATTR_ZIMEN_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　季節変化地面１
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckSeasonGround1( const MAPATTR_VALUE val )
{
  return ( val == MATTR_ZIMEN_S01 );
}
//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　季節変化地面２
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckSeasonGround2( const MAPATTR_VALUE val )
{
  return ( val == MATTR_ZIMEN_S02 );
}
//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　地面（エンカウント）
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckEncountGround( const MAPATTR_VALUE val )
{
  return ( val == MATTR_E_ZIMEN_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　芝生
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckLawn( const MAPATTR_VALUE val )
{
  return ( val == MATTR_LAWN_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　ビリビリ床
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckElecFloor( const MAPATTR_VALUE val )
{
  return ( val == MATTR_ELECTORIC_FLOOR );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック 電気岩
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckElecRock( const MAPATTR_VALUE val )
{
  return ( val == MATTR_ELECTORIC_ROCK );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　尾瀬
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckOze01( const MAPATTR_VALUE val )
{
  return ( val == MATTR_OZE_01 );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　尾瀬階段
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckOzeStairs( const MAPATTR_VALUE val )
{
  return ( val == MATTR_OZE_STAIRS );
}

//--------------------------------------------------------------
/**
 * アトリビュートバリュー　チェック　深い砂漠
 * @param val MAPATTR_VALUE
 * @retval BOOL FALSE=違う
 */
//--------------------------------------------------------------
BOOL MAPATTR_VALUE_CheckDesertDeep( const MAPATTR_VALUE val )
{
  return ( val == MATTR_E_DESERT_01 );
}

