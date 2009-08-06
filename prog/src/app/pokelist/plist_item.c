//======================================================================
/**
 * @file  plist_item.c
 * @brief ポケモンリスト アイテムチェック系
 * @author  ariizumi
 * @data  09/08/05
 *
 * モジュール名：PLIST_ITEM
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "item/item.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//GSから定義移植
#define RCV_FLG_SLEEP   ( 0x01 )  // 眠り回復
#define RCV_FLG_POISON    ( 0x02 )  // 毒回復
#define RCV_FLG_BURN    ( 0x04 )  // 火傷回復
#define RCV_FLG_ICE     ( 0x08 )  // 氷回復
#define RCV_FLG_PARALYZE  ( 0x10 )  // 麻痺回復
#define RCV_FLG_PANIC   ( 0x20 )  // 混乱回復
#define RCV_FLG_ALL     ( 0x3f )  // 全快

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//GSから定義移植
typedef enum
{
  ITEM_TYPE_BTL_ST_UP = 0,  // 戦闘用ステータスアップ系
  ITEM_TYPE_ALLDETH_RCV,    // 全員瀕死回復
  ITEM_TYPE_LV_UP,      // LvUp系
  ITEM_TYPE_NEMURI_RCV,   // 眠り回復
  ITEM_TYPE_DOKU_RCV,     // 毒回復
  ITEM_TYPE_YAKEDO_RCV,   // 火傷回復
  ITEM_TYPE_KOORI_RCV,    // 氷回復
  ITEM_TYPE_MAHI_RCV,     // 麻痺回復
  ITEM_TYPE_KONRAN_RCV,   // 混乱回復
  ITEM_TYPE_ALL_ST_RCV,   // 全快
  ITEM_TYPE_MEROMERO_RCV,   // メロメロ回復
  ITEM_TYPE_HP_RCV,     // HP回復 ( 瀕死も )
  ITEM_TYPE_HP_UP,      // HP努力値UP
  ITEM_TYPE_ATC_UP,     // 攻撃努力値UP
  ITEM_TYPE_DEF_UP,     // 防御努力値UP
  ITEM_TYPE_AGL_UP,     // 素早さ努力値UP
  ITEM_TYPE_SPA_UP,     // 特攻努力値UP
  ITEM_TYPE_SPD_UP,     // 特防努力値UP

  ITEM_TYPE_HP_DOWN,      // HP努力値DOWN
  ITEM_TYPE_ATC_DOWN,     // 攻撃努力値DOWN
  ITEM_TYPE_DEF_DOWN,     // 防御努力値DOWN
  ITEM_TYPE_AGL_DOWN,     // 素早さ努力値DOWN
  ITEM_TYPE_SPA_DOWN,     // 特攻努力値DOWN
  ITEM_TYPE_SPD_DOWN,     // 特防努力値DOWN

  ITEM_TYPE_EVO,        // 進化系
  ITEM_TYPE_PP_UP,      // ppUp系
  ITEM_TYPE_PP_3UP,     // pp3Up系
  ITEM_TYPE_PP_RCV,     // pp回復系
  ITEM_TYPE_ETC,        // その他
}PLIST_ITEM_USE_TYPE;


//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static u8 PLIST_ITEM_RecoverCheck( u16 item );

//--------------------------------------------------------------------------------------------
/**
 * 回復タイプチェック
 *
 * @param item  アイテム番号
 *
 * @return  回復タイプ
 */
//--------------------------------------------------------------------------------------------
static u8 PLIST_ITEM_RecoverCheck( u16 item )
{
  ITEMDATA * dat;
  s32 prm;

  dat = ITEM_GetItemArcData( item, ITEM_GET_DATA, HEAPID_POKELIST );

  // パラメータを持っていないアイテムはその他
  if( ITEM_GetBufParam( dat, ITEM_PRM_W_TYPE ) != ITEM_WKTYPE_POKEUSE ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_ETC;
  }

  // 戦闘用ステータスアップ系
  if( ITEM_GetBufParam( dat, ITEM_PRM_ATTACK_UP ) != 0 ||
    ITEM_GetBufParam( dat, ITEM_PRM_DEFENCE_UP ) != 0 ||
    ITEM_GetBufParam( dat, ITEM_PRM_SP_ATTACK_UP ) != 0 ||
    ITEM_GetBufParam( dat, ITEM_PRM_SP_DEFENCE_UP ) != 0 ||
    ITEM_GetBufParam( dat, ITEM_PRM_AGILITY_UP ) != 0 ||
    ITEM_GetBufParam( dat, ITEM_PRM_HIT_UP ) != 0 ||
    ITEM_GetBufParam( dat, ITEM_PRM_CRITICAL_UP ) != 0 ){

    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_BTL_ST_UP;
  }

  // 全員瀕死回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_ALL_DEATH_RCV ) != 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_ALLDETH_RCV;
  }

  // LvUp系
  if( ITEM_GetBufParam( dat, ITEM_PRM_LV_UP ) != 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_LV_UP;
  }

  // Status回復系
  prm = ITEM_GetBufParam( dat, ITEM_PRM_SLEEP_RCV );        // 眠り
  prm += ( ITEM_GetBufParam( dat, ITEM_PRM_POISON_RCV ) << 1 ); // 毒
  prm += ( ITEM_GetBufParam( dat, ITEM_PRM_BURN_RCV ) << 2 );   // 火傷
  prm += ( ITEM_GetBufParam( dat, ITEM_PRM_ICE_RCV ) << 3 );    // 氷
  prm += ( ITEM_GetBufParam( dat, ITEM_PRM_PARALYZE_RCV ) << 4 ); // 麻痺
  prm += ( ITEM_GetBufParam( dat, ITEM_PRM_PANIC_RCV ) << 5 );    // 混乱
  switch( prm ){
  case RCV_FLG_SLEEP:   // 眠り
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_NEMURI_RCV;
  case RCV_FLG_POISON:  // 毒
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_DOKU_RCV;
  case RCV_FLG_BURN:    // 火傷
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_YAKEDO_RCV;
  case RCV_FLG_ICE:   // 氷
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_KOORI_RCV;
  case RCV_FLG_PARALYZE:  // 麻痺
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_MAHI_RCV;
  case RCV_FLG_PANIC:   // 混乱
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_KONRAN_RCV;
  case RCV_FLG_ALL:   // 全快
    if( ITEM_GetBufParam( dat, ITEM_PRM_HP_RCV ) != 0 ){
      GFL_HEAP_FreeMemory( dat );
      return ITEM_TYPE_HP_RCV;  // HP回復 ( 瀕死も )
    }else{
      GFL_HEAP_FreeMemory( dat );
      return ITEM_TYPE_ALL_ST_RCV;
    }
  }
  // メロメロ回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_MEROMERO_RCV ) != 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_MEROMERO_RCV;
  }

  // HP回復 ( 瀕死も )
  if( ITEM_GetBufParam( dat, ITEM_PRM_HP_RCV ) != 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_HP_RCV;
  }

  // ドーピング系
  // HP努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_HP_EXP_POINT );
  if( prm > 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_HP_UP;
  }
  if( prm < 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_HP_DOWN;
  }
  // 攻撃努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_POWER_EXP_POINT );
  if( prm > 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_ATC_UP;
  }
  if( prm < 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_ATC_DOWN;
  }
  // 防御努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_DEFENCE_EXP_POINT );
  if( prm > 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_DEF_UP;
  }
  if( prm < 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_DEF_DOWN;
  }
  // 素早さ努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_AGILITY_EXP_POINT );
  if( prm > 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_AGL_UP;
  }
  if( prm < 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_AGL_DOWN;
  }
  // 特攻努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_SP_ATTACK_EXP_POINT );
  if( prm > 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_SPA_UP;
  }
  if( prm < 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_SPA_DOWN;
  }
  // 特防努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_SP_DEFENCE_EXP_POINT );
  if( prm > 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_SPD_UP;
  }
  if( prm < 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_SPD_DOWN;
  }

  // 進化系
  if( ITEM_GetBufParam( dat, ITEM_PRM_EVOLUTION ) != 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_EVO;
  }

  // ppUp系
  if( ITEM_GetBufParam( dat, ITEM_PRM_PP_UP ) != 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_PP_UP;
  }

  // pp3Up系
  if( ITEM_GetBufParam( dat, ITEM_PRM_PP_3UP ) != 0 ){
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_PP_3UP;
  }

  // pp回復系
  if( ITEM_GetBufParam( dat, ITEM_PRM_PP_RCV ) != 0 ||
    ITEM_GetBufParam( dat, ITEM_PRM_ALL_PP_RCV ) != 0 ){

    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_PP_RCV;
  }

  GFL_HEAP_FreeMemory( dat );
  return ITEM_TYPE_ETC;
}
