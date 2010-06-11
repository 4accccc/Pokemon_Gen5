//==============================================================================
/**
 * @file  symbol_map.c
 * @brief シンボルエンカウント用マップ関連
 * @author  tamada GAME FREAK inc.
 * @date  2010.03.19
 */
//==============================================================================

#include <gflib.h>


#include "savedata/save_control.h"  //SAVE_CONTROL_WORK
#include "gamesystem/gamesystem.h"
#include "savedata/symbol_save.h"   //SYMBOL_POKEMON
#include "savedata/intrude_save.h"
#include "savedata/symbol_save_notwifi.h"

#include "arc/fieldmap/zone_id.h"
#include "field/field_dir.h"

#include "field/intrude_symbol.h"
#include "field_comm/intrude_work.h"  //Intrude_Check_CommConnect

#include "symbol_map.h"
#include "system/main.h"
//==============================================================================
//==============================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  SYMMAP_MAX_HEIGHT = 10,
  SYMMAP_MAX_WIDTH = 3,

  SYMMAP_SIZE = SYMMAP_MAX_HEIGHT * SYMMAP_MAX_WIDTH,

  SYMMAP_TOP_ID = 1,    ///<一番奥のID
  SYMMAP_ENT_ID = 5,    ///<入り口のID

  SYMMAP_IDX_TOP = 1,   ///<マップ配列での一番奥へのインデックス

  SYMMAP_SMALL_LEVEL_LIMIT = SYMBOL_MAP_LEVEL_SMALL_MAX + 1,
};

SDK_COMPILER_ASSERT( SYMMAP_ENT_ID - 1 == SYMBOL_MAP_ID_ENTRANCE );

//==============================================================================
//==============================================================================
//--------------------------------------------------------------
//  各段階でのマップ構成
//--------------------------------------------------------------
static const u8 map_level1_0[SYMMAP_SIZE] = {
  0,  1,  0,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level2_0[SYMMAP_SIZE] = {
  0,  1,  0,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level3_0[SYMMAP_SIZE] = {
  0,  1,  0,
  15, 14, 16,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level4_0[SYMMAP_SIZE] = {
  0,  1,  0,
  18, 17, 19,
  15, 14, 16,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level5_0[SYMMAP_SIZE] = {
  0,  1,  0,
  21, 20, 22,
  18, 17, 19,
  15, 14, 16,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level6_0[SYMMAP_SIZE] = {
  0,  1,  0,
  24, 23, 25,
  21, 20, 22,
  18, 17, 19,
  15, 14, 16,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level7_0[SYMMAP_SIZE] = {
  0,  1,  0,
  27, 26, 28,
  24, 23, 25,
  21, 20, 22,
  18, 17, 19,
  15, 14, 16,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
};

static const u8 map_level1_1[SYMMAP_SIZE] = {
  0,  1,  0,
  3,  2,  4,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level2_1[SYMMAP_SIZE] = {
  0,  1,  0,
  3,  2,  4,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level3_1[SYMMAP_SIZE] = {
  0,  1,  0,
  3,  2,  4,
  15, 14, 16,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level4_1[SYMMAP_SIZE] = {
  0,  1,  0,
  3,  2,  4,
  18, 17, 19,
  15, 14, 16,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level5_1[SYMMAP_SIZE] = {
  0,  1,  0,
  3,  2,  4,
  21, 20, 22,
  18, 17, 19,
  15, 14, 16,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
  0,  0,  0,
};
static const u8 map_level6_1[SYMMAP_SIZE] = {
  0,  1,  0,
  3,  2,  4,
  24, 23, 25,
  21, 20, 22,
  18, 17, 19,
  15, 14, 16,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
  0,  0,  0,
};
static const u8 map_level7_1[SYMMAP_SIZE] = {
  0,  1,  0,
  3,  2,  4,
  27, 26, 28,
  24, 23, 25,
  21, 20, 22,
  18, 17, 19,
  15, 14, 16,
  12, 11, 13,
  9,  8,  10,
  6,  5,  7,
};

static const u8 * const symmap_tables[] = {
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

//--------------------------------------------------------------
//--------------------------------------------------------------

//==============================================================================
//==============================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline SYMBOL_MAP_ID LSIDtoSYMMAPID( u8 lsid )
{
  return lsid - 1;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline u8 SYMMAPIDtoLSID( SYMBOL_MAP_ID symmap_id )
{
  u8 lsid = symmap_id + 1;
  return lsid;
}
//--------------------------------------------------------------
/// 左側にマップがあるか？
//--------------------------------------------------------------
static inline BOOL hasLeft( const u8 * map, int idx )
{
  return ( (idx % SYMMAP_MAX_WIDTH != 0) && idx != SYMMAP_IDX_TOP );
}
//--------------------------------------------------------------
/// 右側にマップがあるか？
//--------------------------------------------------------------
static inline BOOL hasRight( const u8 * map, int idx )
{
  return ( (idx % SYMMAP_MAX_WIDTH != SYMMAP_MAX_WIDTH - 1) && idx != SYMMAP_IDX_TOP );
}
//--------------------------------------------------------------
/// 上側にマップがあるか？
//--------------------------------------------------------------
static inline BOOL hasUp( const u8 * map, int idx )
{
  return (idx - SYMMAP_MAX_WIDTH >= 0 && map[idx - SYMMAP_MAX_WIDTH] != 0 );
}
//--------------------------------------------------------------
/// 下側にマップがあるか？
//--------------------------------------------------------------
static inline BOOL hasDown( const u8 * map, int idx )
{
  return ( idx + SYMMAP_MAX_WIDTH < SYMMAP_SIZE && map[idx + SYMMAP_MAX_WIDTH] != 0 );
}
//--------------------------------------------------------------
//  SIDからマップテーブルへのインデックスを取得する
//--------------------------------------------------------------
static u8 getIndex( const u8 * map, u8 now_lsid )
{
  int i;
#ifdef  PM_DEBUG
  for ( i = 0; i < SYMMAP_SIZE; i+=3 )
  {
    OS_TPrintf( "%2d %2d %2d\n", map[i+0],map[i+1],map[i+2] );
    GF_ASSERT( map[i+0] == 0 || map[i+1] < map[i+0] );
  }
  OS_TPrintf( "search sid == %d\n", now_lsid );
#endif
  for ( i = 0; i < SYMMAP_SIZE; i ++ )
  {
    if ( map[i] == now_lsid ) return i;
  }
  GF_ASSERT(0);
  return SYMMAP_IDX_TOP;
}

//--------------------------------------------------------------
/// SIDから対応するゾーンIDを取得する
//--------------------------------------------------------------
static u16 getSymbolMapZoneID( const u8 * map, u8 now_lsid )
{
  int idx = getIndex( map, now_lsid );
  BOOL left = hasLeft( map, idx );
  BOOL right = hasRight( map, idx );
  BOOL up = hasUp( map, idx );
  BOOL down = hasDown( map, idx );

  if ( now_lsid == SYMMAP_TOP_ID ) return ZONE_ID_PALACE03;
  if ( now_lsid == SYMMAP_ENT_ID ) return ZONE_ID_PALACE02;

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
/// 現在のSIDと方向から次のマップのSIDを取得する
//--------------------------------------------------------------
static u16 getNextMapLSID( const u8 * map, u8 now_lsid, u8 dir_id )
{
  int index = getIndex( map, now_lsid );
  switch ( dir_id )
  {
  case DIR_UP:
    GF_ASSERT( hasUp( map, index ) );
    return map[ index - SYMMAP_MAX_WIDTH ];
  case DIR_DOWN:
    GF_ASSERT( hasDown( map, index ) );
    return map[ index + SYMMAP_MAX_WIDTH ];
  case DIR_LEFT:
    GF_ASSERT( hasLeft( map, index ) );
    return map[ index - 1 ];
  case DIR_RIGHT:
    GF_ASSERT( hasRight( map, index ) );
    return map[ index + 1 ];
  default:
    GF_ASSERT( 0 );
  }
  return now_lsid;
}

//--------------------------------------------------------------
/**
 * @brief マップ段階の取得
 * @param gsys
 * @param small
 * @param large
 * @retval  BOOL  TRUE:正常の時、FALSE:通信エラー時
 */
//--------------------------------------------------------------
static BOOL getMapLevel( GAMESYS_WORK * gsys, SYMBOL_MAP_LEVEL_SMALL * small, SYMBOL_MAP_LEVEL_LARGE * large )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(gamedata);
  SYMBOL_SAVE_WORK *symbol_save = SymbolSave_GetSymbolData(sv_ctrl);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr( gsys );

  if ( IntrudeSymbol_CheckIntrudeNetID( game_comm, gamedata ) == INTRUDE_NETID_NULL )
  { //自分のマップ
    *small = SymbolSave_GetMapLevelSmall( symbol_save );
    *large = SymbolSave_GetMapLevelLarge( symbol_save );
    return TRUE;
  }
  else
  { //通信相手のマップ
    INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect( game_comm );
    INTRUDE_SYMBOL_WORK * isw;
    if ( intcomm == NULL )
    {
      //通信エラーでも正常動作するように値を入れる
      *small = SYMBOL_MAP_LEVEL_SMALL_1;
      *large = SYMBOL_MAP_LEVEL_LARGE_NONE;
      return FALSE;
    }
    isw = IntrudeSymbol_GetSymbolBuffer( intcomm );
    *small = isw->map_level_small;
    *large = isw->map_level_large;
    return TRUE;
  }
}

//--------------------------------------------------------------
/**
 * @brief マップ構造取得
 * @return  マップ構造のテーブル。取得できなかった場合、NULLを返す
 */
//--------------------------------------------------------------
static const u8 * getMapTable( GAMESYS_WORK * gsys )
{
  SYMBOL_MAP_LEVEL_SMALL small_lvl;
  SYMBOL_MAP_LEVEL_LARGE large_lvl;
  int index;
  BOOL ok_flag;
  ok_flag = getMapLevel( gsys, &small_lvl, &large_lvl);
  if ( ok_flag == FALSE )
  {
    return NULL;
  }
  index = small_lvl + SYMMAP_SMALL_LEVEL_LIMIT * large_lvl;
  GF_ASSERT( index < NELEMS(symmap_tables) );
  return symmap_tables[index];
}


//--------------------------------------------------------------
/**
 * @brief
 * @param heapID
 * @param gsys
 * @param no
 * @return  INTRUDE_SYMBOL_WORK
 *
 */
//--------------------------------------------------------------
INTRUDE_SYMBOL_WORK * SYMBOLMAP_AllocSymbolWork( HEAPID heapID, GAMESYS_WORK * gsys, u32 * no )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(gamedata);
  SYMBOL_SAVE_WORK *symbol_save = SymbolSave_GetSymbolData(sv_ctrl);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr( gsys );

  if ( IntrudeSymbol_CheckIntrudeNetID( game_comm, gamedata ) == INTRUDE_NETID_NULL )
  { //自分のマップ
    SYMBOL_MAP_ID symmap_id = GAMEDATA_GetSymbolMapID( gamedata );
    u8 occ_num;
    INTRUDE_SYMBOL_WORK * isw = GFL_HEAP_AllocClearMemory( heapID, sizeof(INTRUDE_SYMBOL_WORK) );
    *no = SymbolSave_GetMapIDSymbolPokemon( symbol_save,
        isw->spoke_array, SYMBOL_MAP_STOCK_MAX, symmap_id, &occ_num );
    isw->num = occ_num;
    isw->map_level_small = SymbolSave_GetMapLevelSmall( symbol_save );
    isw->map_level_large = SymbolSave_GetMapLevelLarge( symbol_save );
    isw->net_id = INTRUDE_NETID_NULL;
    isw->symbol_map_id = symmap_id;
    return isw;
  }
  else
  { //通信相手のマップ
    INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect( game_comm );
    if ( intcomm == NULL )
    {
      return NULL; //通信エラー
    }
    else
    {
      INTRUDE_SYMBOL_WORK * isw = GFL_HEAP_AllocClearMemory( heapID, sizeof(INTRUDE_SYMBOL_WORK) );
      GFL_STD_MemCopy( IntrudeSymbol_GetSymbolBuffer( intcomm ), isw, sizeof( INTRUDE_SYMBOL_WORK ) );
      *no = SYMBOL_POKE_MAX;
      return isw;
    }
  }
}

//==============================================================================
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief 対応するゾーンのIDを取得
 * @param symmap_id   シンボルマップID
 * @return  u16   ゾーンID
 */
//--------------------------------------------------------------
u16 SYMBOLMAP_GetZoneID( GAMESYS_WORK * gsys, SYMBOL_MAP_ID symmap_id )
{
  const u8 * tbl;
  tbl = getMapTable( gsys );

  if ( tbl ) {
    return getSymbolMapZoneID( tbl, SYMMAPIDtoLSID( symmap_id ) );
  } else {
    return ZONE_ID_PALACE02;  //万が一のエラー処理：入り口タイプのゾーンを返す
  }
}

//--------------------------------------------------------------
/**
 * @brief 移動先のシンボルマップID取得
 * @param now_symmap_id   現在のシンボルマップID
 * @param dir_id          移動先マップのある方向
 * @return  SYMBOL_MAP_ID 移動先のシンボルマップID
 */
//--------------------------------------------------------------
SYMBOL_MAP_ID SYMBOLMAP_GetNextSymbolMapID(
    GAMESYS_WORK * gsys, SYMBOL_MAP_ID now_symmap_id, u16 dir_id )
{
  const u8 * tbl;
  u8 next_lsid;

  tbl = getMapTable( gsys );
  if ( tbl != NULL ) {
    next_lsid = getNextMapLSID( tbl, SYMMAPIDtoLSID(now_symmap_id), dir_id );
    return LSIDtoSYMMAPID( next_lsid );
  } else {
    //万が一のエラー処理：入り口IDを返す
    return LSIDtoSYMMAPID( SYMMAP_ENT_ID );
  }
}

//--------------------------------------------------------------
/**
 * @brief キープゾーンか？のチェック
 * @param symmap_id   シンボルマップID
 * @return BOOL   TRUEのとき、キープゾーンのマップ
 */
//--------------------------------------------------------------
BOOL SYMBOLMAP_IsKeepzoneID( SYMBOL_MAP_ID symmap_id )
{
  return SYMMAPIDtoLSID( symmap_id ) == SYMMAP_TOP_ID;
}

//--------------------------------------------------------------
/**
 * @brief   パレスの森の入口マップか？のチェック
 * @param symmap_id   シンボルマップID
 * @return BOOL   TRUEの時、パレスの森の入り口マップ
 */
//--------------------------------------------------------------
BOOL SYMBOLMAP_IsEntranceID( SYMBOL_MAP_ID symmap_id )
{
  return SYMMAPIDtoLSID( symmap_id ) == SYMMAP_ENT_ID;
}

//--------------------------------------------------------------
/**
 * @brief   大きいポケモン用のマップか？のチェック
 * @param symmap_id   シンボルマップID
 * @return BOOL   TRUEの時、大きいポケモン用のマップ
 */
//--------------------------------------------------------------
BOOL SYMBOLMAP_IsLargePokeID( SYMBOL_MAP_ID symmap_id )
{
   u8 lsid = SYMMAPIDtoLSID( symmap_id );
   if ( lsid == 2 || lsid == 3 || lsid == 4 )
   {
     return TRUE;
   }
   else
   {
     return FALSE;
   }
}

