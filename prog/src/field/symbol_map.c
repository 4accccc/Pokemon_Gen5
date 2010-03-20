//==============================================================================
/**
 * @file  symbol_map.c
 * @brief シンボルエンカウント用マップ関連
 * @author  tamada GAME FREAK inc.
 * @date  2010.03.19
 */
//==============================================================================

#include <gflib.h>

#include "symbol_map.h"

#include "savedata/save_control.h"  //SAVE_CONTROL_WORK
#include "savedata/symbol_save.h"
#include "savedata/intrude_save.h"

#include "arc/fieldmap/zone_id.h"
#include "field/field_dir.h"

//==============================================================================
//==============================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  SYMMAP_MAX_HEIGHT = 10,
  SYMMAP_MAX_WIDTH = 3,

  SYMMAP_SIZE = SYMMAP_MAX_HEIGHT * SYMMAP_MAX_WIDTH,

  SYMMAP_TOP_ID = 1,    ///<一番奥のID
  SYMMAP_ENT_ID = 6,    ///<入り口のID

  SYMMAP_SMALL_LEVEL_LIMIT = SYMBOL_MAP_LEVEL_SMALL_MAX + 1,
};

//==============================================================================
//==============================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static u8 map_level1_0[SYMMAP_SIZE] = {
  0,  1,  0,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level2_0[SYMMAP_SIZE] = {
  0,  1,  0,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level3_0[SYMMAP_SIZE] = {
  0,  1,  0,
  14, 15, 16,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level4_0[SYMMAP_SIZE] = {
  0,  1,  0,
  17, 18, 19,
  14, 15, 16,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level5_0[SYMMAP_SIZE] = {
  0,  1,  0,
  20, 21, 22,
  17, 18, 19,
  14, 15, 16,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level6_0[SYMMAP_SIZE] = {
  0,  1,  0,
  23, 24, 25,
  20, 21, 22,
  17, 18, 19,
  14, 15, 16,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level7_0[SYMMAP_SIZE] = {
  0,  1,  0,
  26, 27, 28,
  23, 24, 25,
  20, 21, 22,
  17, 18, 19,
  14, 15, 16,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
};

static u8 map_level1_1[SYMMAP_SIZE] = {
  0,  1,  0,
  2,  3,  4,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level2_1[SYMMAP_SIZE] = {
  0,  1,  0,
  2,  3,  4,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level3_1[SYMMAP_SIZE] = {
  0,  1,  0,
  2,  3,  4,
  14, 15, 16,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level4_1[SYMMAP_SIZE] = {
  0,  1,  0,
  2,  3,  4,
  17, 18, 19,
  14, 15, 16,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level5_1[SYMMAP_SIZE] = {
  0,  1,  0,
  2,  3,  4,
  20, 21, 22,
  17, 18, 19,
  14, 15, 16,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
  0,  0,  0,
};
static u8 map_level6_1[SYMMAP_SIZE] = {
  0,  1,  0,
  2,  3,  4,
  23, 24, 25,
  20, 21, 22,
  17, 18, 19,
  14, 15, 16,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
  0,  0,  0,
};
static u8 map_level7_1[SYMMAP_SIZE] = {
  0,  1,  0,
  2,  3,  4,
  26, 27, 28,
  23, 24, 25,
  20, 21, 22,
  17, 18, 19,
  14, 15, 16,
  11, 12, 13,
  8,  9,  10,
  5,  6,  7,
};

//==============================================================================
//==============================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline BOOL hasLeft( const u8 * map, int idx )
{
  return (idx % SYMMAP_MAX_WIDTH != 0 && idx != SYMMAP_TOP_ID );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline BOOL hasRight( const u8 * map, int idx )
{
  return ( idx % SYMMAP_MAX_WIDTH != SYMMAP_MAX_WIDTH - 1 && idx != SYMMAP_TOP_ID );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline BOOL hasUp( const u8 * map, int idx )
{
  return (idx - SYMMAP_MAX_WIDTH >= 0 && map[idx - SYMMAP_MAX_WIDTH] != 0 );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline BOOL hasDown( const u8 * map, int idx )
{
  return ( idx + SYMMAP_MAX_WIDTH < SYMMAP_SIZE && map[idx + SYMMAP_MAX_WIDTH] != 0 );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline u8 getIndex( const u8 * map, u8 now_sid )
{
  int i;
  for ( i = 0; i < SYMMAP_SIZE; i ++ )
  {
    if ( map[i] == now_sid ) return i;
  }
  GF_ASSERT(0);
  return SYMMAP_TOP_ID;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
u16 getSymbolMapZoneID( const u8 * map, u8 now_sid )
{
  int idx = getIndex( map, now_sid );
  BOOL left = hasLeft( map, idx );
  BOOL right = hasRight( map, idx );
  BOOL up = hasUp( map, idx );
  BOOL down = hasDown( map, idx );

  if ( now_sid == SYMMAP_TOP_ID ) return ZONE_ID_PALACE03;

  if ( left && right )
  { //左右ある時は上下もある！
    GF_ASSERT( up && down );
    return ZONE_ID_PALACE07;
  }

  if ( left && !right )
  { //右端の場合
    GF_ASSERT( up || down );  //上下どちらかはある！
    if ( up && down ) return ZONE_ID_PALACE08;
    if ( up )         return ZONE_ID_PALACE10;
    if ( down )       return ZONE_ID_PALACE05;
  }

  if ( !left && right )
  { //左端の場合
    GF_ASSERT( up || down );  //上下どちらかはある！
    if ( up && down ) return ZONE_ID_PALACE06;
    if ( up )         return ZONE_ID_PALACE09;
    if ( down )       return ZONE_ID_PALACE04;
  }
  GF_ASSERT( 0 );
  return ZONE_ID_PALACE03;  //万が一のエラー処理：一番制限されたものを返す
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 getNextMapIndex( const u8 * map, u8 now_sid, u8 dir_id )
{
  switch ( dir_id )
  {
  case DIR_UP:
    GF_ASSERT( hasUp( map, now_sid ) );
    return map[ getIndex( map, now_sid ) - SYMMAP_MAX_WIDTH ];
  case DIR_DOWN:
    GF_ASSERT( hasDown( map, now_sid ) );
    return map[ getIndex( map, now_sid ) + SYMMAP_MAX_WIDTH ];
  case DIR_LEFT:
    GF_ASSERT( hasLeft( map, now_sid ) );
    return map[ getIndex( map, now_sid ) - 1 ];
  case DIR_RIGHT:
    GF_ASSERT( hasRight( map, now_sid ) );
    return map[ getIndex( map, now_sid ) + 1 ];
  }
  GF_ASSERT( 0 );
  return now_sid;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static const u8 * getMapTable( SYMBOL_MAP_LEVEL_LARGE large_lvl, SYMBOL_MAP_LEVEL_SMALL small_lvl )
{
  static const u8 * table[] = {
    map_level1_0,
    map_level2_0,
    map_level3_0,
    map_level4_0,
    map_level5_0,
    map_level6_0,
    map_level7_0,

    map_level1_1,
    map_level2_1,
    map_level3_1,
    map_level4_1,
    map_level5_1,
    map_level6_1,
    map_level7_1,
  };
  int index = small_lvl + SYMMAP_SMALL_LEVEL_LIMIT * large_lvl;
  GF_ASSERT( index < NELEMS(table) );
  return table[index];
}



