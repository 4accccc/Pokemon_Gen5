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
#include "msg/msg_pokelist.h"

#include "plist_sys.h"
#include "plist_message.h"
#include "plist_item.h"

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
  ITEM_TYPE_HP_RCV,     // HP回復
  ITEM_TYPE_DEATH_RCV,  // 瀕死回復 (WB追加
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

static const u16 PLIST_ITEM_UTIL_GetParamExpSum( POKEMON_PARAM *pp );
static const BOOL PLIST_ITEM_UTIL_CanAddParamExp( POKEMON_PARAM *pp , const int id , u16 item );
static const BOOL PLIST_ITEM_UTIL_CanSubParamExp( POKEMON_PARAM *pp , const int id , u16 item );
static const s32 PLIST_ITEM_UTIL_GetFriendValue( POKEMON_PARAM *pp , u16 itemNo , HEAPID heapId );

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
  if( ITEM_GetBufParam( dat, ITEM_PRM_W_TYPE ) != ITEM_WKTYPE_POKEUSE )
  {
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
    ITEM_GetBufParam( dat, ITEM_PRM_CRITICAL_UP ) != 0 )
  {

    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_BTL_ST_UP;
  }

  // 全員瀕死回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_ALL_DEATH_RCV ) != 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_ALLDETH_RCV;
  }

  // LvUp系
  if( ITEM_GetBufParam( dat, ITEM_PRM_LV_UP ) != 0 )
  {
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
  switch( prm )
  {
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
    if( ITEM_GetBufParam( dat, ITEM_PRM_HP_RCV ) != 0 )
    {
      GFL_HEAP_FreeMemory( dat );
      return ITEM_TYPE_HP_RCV;  // HP回復 ( 瀕死も )
    }
    else
    {
      GFL_HEAP_FreeMemory( dat );
      return ITEM_TYPE_ALL_ST_RCV;
    }
  }
  // メロメロ回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_MEROMERO_RCV ) != 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_MEROMERO_RCV;
  }

  // HP回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_HP_RCV ) != 0 )
  {
    if( ITEM_GetBufParam( dat, ITEM_PRM_DEATH_RCV ) != 0)
    {
      GFL_HEAP_FreeMemory( dat );
      return ITEM_TYPE_DEATH_RCV;
    }
    else
    {
      GFL_HEAP_FreeMemory( dat );
      return ITEM_TYPE_HP_RCV;
    }
  }

  // ドーピング系
  // HP努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_HP_EXP_POINT );
  if( prm > 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_HP_UP;
  }
  if( prm < 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_HP_DOWN;
  }
  // 攻撃努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_POWER_EXP_POINT );
  if( prm > 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_ATC_UP;
  }
  if( prm < 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_ATC_DOWN;
  }
  // 防御努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_DEFENCE_EXP_POINT );
  if( prm > 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_DEF_UP;
  }
  if( prm < 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_DEF_DOWN;
  }
  // 素早さ努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_AGILITY_EXP_POINT );
  if( prm > 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_AGL_UP;
  }
  if( prm < 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_AGL_DOWN;
  }
  // 特攻努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_SP_ATTACK_EXP_POINT );
  if( prm > 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_SPA_UP;
  }
  if( prm < 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_SPA_DOWN;
  }
  // 特防努力値
  prm = ITEM_GetBufParam( dat, ITEM_PRM_SP_DEFENCE_EXP_POINT );
  if( prm > 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_SPD_UP;
  }
  if( prm < 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_SPD_DOWN;
  }

  // 進化系
  if( ITEM_GetBufParam( dat, ITEM_PRM_EVOLUTION ) != 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_EVO;
  }

  // ppUp系
  if( ITEM_GetBufParam( dat, ITEM_PRM_PP_UP ) != 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_PP_UP;
  }

  // pp3Up系
  if( ITEM_GetBufParam( dat, ITEM_PRM_PP_3UP ) != 0 )
  {
    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_PP_3UP;
  }

  // pp回復系
  if( ITEM_GetBufParam( dat, ITEM_PRM_PP_RCV ) != 0 ||
    ITEM_GetBufParam( dat, ITEM_PRM_ALL_PP_RCV ) != 0 )
  {

    GFL_HEAP_FreeMemory( dat );
    return ITEM_TYPE_PP_RCV;
  }

  GFL_HEAP_FreeMemory( dat );
  return ITEM_TYPE_ETC;
}

//--------------------------------------------------------------------------
//  全体復活アイテムか？ と、言うか聖なる灰か？
//--------------------------------------------------------------------------
const BOOL PLIST_ITEM_IsDeathRecoverAllItem( PLIST_WORK *work , u16 itemNo )
{
  if( ITEM_GetParam( itemNo , ITEM_PRM_ALL_DEATH_RCV , work->heapId ) != 0 )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//--------------------------------------------------------------------------
//  対象のポケモンにアイテムが使えるか？
//--------------------------------------------------------------------------
const PLIST_ITEM_USE_CHECK PLIST_ITEM_CanUseRecoverItem( PLIST_WORK *work , u16 itemNo , POKEMON_PARAM *pp )
{
  PLIST_ITEM_USE_TYPE useType = PLIST_ITEM_RecoverCheck( itemNo );
  
  switch( useType )
  {
  case ITEM_TYPE_BTL_ST_UP:  // 戦闘用ステータスアップ系
    return PIUC_NG;
    break;

  case ITEM_TYPE_ALLDETH_RCV:    // 全員瀕死回復
    //ここを通る処理じゃない
    return PIUC_NG;
    break;

  case ITEM_TYPE_LV_UP:      // LvUp系
    if( PP_CalcLevel( pp ) < PTL_LEVEL_MAX )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_NEMURI_RCV:   // 眠り回復
    if( PP_GetSick( pp ) == POKESICK_NEMURI )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_DOKU_RCV:     // 毒回復
    if( PP_GetSick( pp ) == POKESICK_DOKU )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_YAKEDO_RCV:   // 火傷回復
    if( PP_GetSick( pp ) == POKESICK_YAKEDO )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_KOORI_RCV:    // 氷回復
    if( PP_GetSick( pp ) == POKESICK_KOORI )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_MAHI_RCV:     // 麻痺回復
    if( PP_GetSick( pp ) == POKESICK_MAHI )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_KONRAN_RCV:   // 混乱回復
    //バトルのみの異常
    return PIUC_NG;
    break;

  case ITEM_TYPE_ALL_ST_RCV:   // 全快
    if( PP_GetSick( pp ) != POKESICK_NULL )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_MEROMERO_RCV:   // メロメロ回復
    //バトルのみの異常
    return PIUC_NG;
    break;
  
  case ITEM_TYPE_DEATH_RCV:  // 瀕死回復 (WB追加
    if( PP_Get( pp , ID_PARA_hp , NULL ) == 0 )
    {
      return PIUC_OK;
    }
    break;
  
  case ITEM_TYPE_HP_RCV:     // HP回復
    //瀕死は含まない！
    {
      const u32 hp = PP_Get( pp , ID_PARA_hp , NULL );
      const u32 hpmax = PP_Get( pp , ID_PARA_hpmax , NULL );
      if( hp != 0 && hpmax != hp )
      {
        return PIUC_OK;
      }
    }
    break;

  case ITEM_TYPE_HP_UP:      // HP努力値UP
    if( PLIST_ITEM_UTIL_CanAddParamExp( pp , ID_PARA_hp_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_ATC_UP:     // 攻撃努力値UP
    if( PLIST_ITEM_UTIL_CanAddParamExp( pp , ID_PARA_pow_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_DEF_UP:     // 防御努力値UP
    if( PLIST_ITEM_UTIL_CanAddParamExp( pp , ID_PARA_def_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_AGL_UP:     // 素早さ努力値UP
    if( PLIST_ITEM_UTIL_CanAddParamExp( pp , ID_PARA_agi_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_SPA_UP:     // 特攻努力値UP
    if( PLIST_ITEM_UTIL_CanAddParamExp( pp , ID_PARA_spepow_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_SPD_UP:     // 特防努力値UP
    if( PLIST_ITEM_UTIL_CanAddParamExp( pp , ID_PARA_spedef_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_HP_DOWN:      // HP努力値DOWN
    if( PLIST_ITEM_UTIL_CanSubParamExp(  pp , ID_PARA_hp_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    //なつき度変化があれば
    if( PLIST_ITEM_UTIL_GetFriendValue( pp , itemNo , work->heapId ) != 0 &&
        PP_Get( pp , ID_PARA_friend , NULL ) < PTL_FRIEND_MAX )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_ATC_DOWN:     // 攻撃努力値DOWN
    if( PLIST_ITEM_UTIL_CanSubParamExp(  pp , ID_PARA_pow_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    //なつき度変化があれば
    if( PLIST_ITEM_UTIL_GetFriendValue( pp , itemNo , work->heapId ) != 0 &&
        PP_Get( pp , ID_PARA_friend , NULL ) < PTL_FRIEND_MAX )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_DEF_DOWN:     // 防御努力値DOWN
    if( PLIST_ITEM_UTIL_CanSubParamExp(  pp , ID_PARA_def_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    //なつき度変化があれば
    if( PLIST_ITEM_UTIL_GetFriendValue( pp , itemNo , work->heapId ) != 0 &&
        PP_Get( pp , ID_PARA_friend , NULL ) < PTL_FRIEND_MAX )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_AGL_DOWN:     // 素早さ努力値DOWN
    if( PLIST_ITEM_UTIL_CanSubParamExp(  pp , ID_PARA_agi_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    //なつき度変化があれば
    if( PLIST_ITEM_UTIL_GetFriendValue( pp , itemNo , work->heapId ) != 0 &&
        PP_Get( pp , ID_PARA_friend , NULL ) < PTL_FRIEND_MAX )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_SPA_DOWN:     // 特攻努力値DOWN
    if( PLIST_ITEM_UTIL_CanSubParamExp(  pp , ID_PARA_spepow_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    //なつき度変化があれば
    if( PLIST_ITEM_UTIL_GetFriendValue( pp , itemNo , work->heapId ) != 0 &&
        PP_Get( pp , ID_PARA_friend , NULL ) < PTL_FRIEND_MAX )
    {
      return PIUC_OK;
    }
    break;

  case ITEM_TYPE_SPD_DOWN:     // 特防努力値DOWN
    if( PLIST_ITEM_UTIL_CanSubParamExp(  pp , ID_PARA_spedef_exp , itemNo ) == TRUE )
    {
      return PIUC_OK;
    }
    //なつき度変化があれば
    if( PLIST_ITEM_UTIL_GetFriendValue( pp , itemNo , work->heapId ) != 0 &&
        PP_Get( pp , ID_PARA_friend , NULL ) < PTL_FRIEND_MAX )
    {
      return PIUC_OK;
    }
    break;


  case ITEM_TYPE_EVO:        // 進化系
    //ここで行う処理ではない！
    return PIUC_NG;
    break;

  case ITEM_TYPE_PP_UP:      // ppUp系
    //とりあえず技のチェック
    return PIUC_SELECT_SKILL;
    break;

  case ITEM_TYPE_PP_3UP:     // pp3Up系
    //とりあえず技のチェック
    return PIUC_SELECT_SKILL;
    break;

  case ITEM_TYPE_PP_RCV:     // pp回復系
    if( ITEM_GetParam( itemNo , ITEM_PRM_PP_RCV , work->heapId ) != 0 )
    {
      //とりあえず技のチェック
      return PIUC_SELECT_SKILL;
    }
    else
    if( ITEM_GetParam( itemNo , ITEM_PRM_ALL_PP_RCV , work->heapId ) != 0 )
    {
      const BOOL isChangeMode = PP_FastModeOn( pp );
      if( PP_Get( pp , ID_PARA_pp1 , NULL ) != PP_Get( pp , ID_PARA_pp_max1 , NULL ) ||
          PP_Get( pp , ID_PARA_pp2 , NULL ) != PP_Get( pp , ID_PARA_pp_max2 , NULL ) ||
          PP_Get( pp , ID_PARA_pp3 , NULL ) != PP_Get( pp , ID_PARA_pp_max3 , NULL ) ||
          PP_Get( pp , ID_PARA_pp4 , NULL ) != PP_Get( pp , ID_PARA_pp_max4 , NULL ) )
        {
          PP_FastModeOff( pp , isChangeMode );
          return PIUC_OK;
        }
      PP_FastModeOff( pp , isChangeMode );
    }
    GF_ASSERT_MSG(0,"ITEM_TYPE_PP_RCV item param invalid!!\n");
    break;

  case ITEM_TYPE_ETC:        // その他
    return PIUC_NG;
    break;
  }
  return PIUC_NG;
  
}

#pragma mark [> use func
//--------------------------------------------------------------------------
//  全体復活アイテム と、言うか聖なる灰の使用
//--------------------------------------------------------------------------
void PLIST_ITEM_UseAllDeathRecoverItem( PLIST_WORK *work )
{
  /*
  PLIST_ITEM_USE_TYPE useType = PLIST_ITEM_RecoverCheck( itemNo );
  
  switch( useType )
  {
  case ITEM_TYPE_BTL_ST_UP:  // 戦闘用ステータスアップ系
    break;

  case ITEM_TYPE_ALLDETH_RCV:    // 全員瀕死回復
    break;

  case ITEM_TYPE_LV_UP:      // LvUp系
    break;

  case ITEM_TYPE_NEMURI_RCV:   // 眠り回復
    break;

  case ITEM_TYPE_DOKU_RCV:     // 毒回復
    break;

  case ITEM_TYPE_YAKEDO_RCV:   // 火傷回復
    break;

  case ITEM_TYPE_KOORI_RCV:    // 氷回復
    break;

  case ITEM_TYPE_MAHI_RCV:     // 麻痺回復
    break;

  case ITEM_TYPE_KONRAN_RCV:   // 混乱回復
    break;

  case ITEM_TYPE_ALL_ST_RCV:   // 全快
    break;

  case ITEM_TYPE_MEROMERO_RCV:   // メロメロ回復
    break;
  
  case ITEM_TYPE_DEATH_RCV:  // 瀕死回復 (WB追加
    break;
  
  case ITEM_TYPE_HP_RCV:     // HP回復
    break;

  case ITEM_TYPE_HP_UP:      // HP努力値UP
    break;

  case ITEM_TYPE_ATC_UP:     // 攻撃努力値UP
    break;

  case ITEM_TYPE_DEF_UP:     // 防御努力値UP
    break;

  case ITEM_TYPE_AGL_UP:     // 素早さ努力値UP
    break;

  case ITEM_TYPE_SPA_UP:     // 特攻努力値UP
    break;

  case ITEM_TYPE_SPD_UP:     // 特防努力値UP
    break;

  case ITEM_TYPE_HP_DOWN:      // HP努力値DOWN
    break;

  case ITEM_TYPE_ATC_DOWN:     // 攻撃努力値DOWN
    break;

  case ITEM_TYPE_DEF_DOWN:     // 防御努力値DOWN
    break;

  case ITEM_TYPE_AGL_DOWN:     // 素早さ努力値DOWN
    break;

  case ITEM_TYPE_SPA_DOWN:     // 特攻努力値DOWN
    break;

  case ITEM_TYPE_SPD_DOWN:     // 特防努力値DOWN
    break;


  case ITEM_TYPE_EVO:        // 進化系
    break;

  case ITEM_TYPE_PP_UP:      // ppUp系
    break;

  case ITEM_TYPE_PP_3UP:     // pp3Up系
    break;

  case ITEM_TYPE_PP_RCV:     // pp回復系
    break;

  case ITEM_TYPE_ETC:        // その他
    break;
  }
  
  GF_ASSERT_MSG
  PLIST_ITEM_MSG_CanNotUseItem( work );
  */
}

//--------------------------------------------------------------------------
//  通常アイテムの仕様(技単体使用・全体復活以外
//--------------------------------------------------------------------------
void PLIST_ITEM_UseRecoverItem( PLIST_WORK *work )
{
  
}

#pragma mark [> msg func
//--------------------------------------------------------------------------
//  アイテム使えないメッセージの表示
//--------------------------------------------------------------------------
void PLIST_ITEM_MSG_CanNotUseItem( PLIST_WORK *work )
{
  work->plData->ret_mode = PL_RET_BAG;
  
  PLIST_MSG_CreateWordSet( work , work->msgWork );
  PLIST_MSG_AddWordSet_ItemName( work , work->msgWork , 0 , work->plData->item );
  PLIST_MessageWaitInit( work , mes_pokelist_04_45 , TRUE , PSTATUS_MSGCB_ExitCommon );
  PLIST_MSG_DeleteWordSet( work , work->msgWork );
}

#pragma mark [> util
//--------------------------------------------------------------------------
//  基礎ポイントの合計値を求める
//--------------------------------------------------------------------------
static const u16 PLIST_ITEM_UTIL_GetParamExpSum( POKEMON_PARAM *pp )
{
  u16 sum = 0;
  const BOOL isChangeMode = PP_FastModeOn( pp );
  
  sum += PP_Get( pp , ID_PARA_hp_exp , NULL );
  sum += PP_Get( pp , ID_PARA_pow_exp , NULL );
  sum += PP_Get( pp , ID_PARA_def_exp , NULL );
  sum += PP_Get( pp , ID_PARA_agi_exp , NULL );
  sum += PP_Get( pp , ID_PARA_spepow_exp , NULL );
  sum += PP_Get( pp , ID_PARA_spedef_exp , NULL );
  
  PP_FastModeOff( pp , isChangeMode );
  return sum;
}

//--------------------------------------------------------------------------
//  基礎ポイントが上昇できるか？
//--------------------------------------------------------------------------
static const BOOL PLIST_ITEM_UTIL_CanAddParamExp( POKEMON_PARAM *pp , const int id , u16 item )
{
  GF_ASSERT_MSG( id >= ID_PARA_hp_exp && id <= ID_PARA_spedef_exp , "id is invalid!![%d]\n",id );
  
  if( PLIST_ITEM_UTIL_GetParamExpSum(pp) < PARA_EXP_TOTAL_MAX &&
      PP_Get( pp , id , NULL ) < PARA_EXP_ITEM_MAX )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------
//  基礎ポイントが減少できるか？
//--------------------------------------------------------------------------
static const BOOL PLIST_ITEM_UTIL_CanSubParamExp( POKEMON_PARAM *pp , const int id , u16 item )
{
  GF_ASSERT_MSG( id >= ID_PARA_hp_exp && id <= ID_PARA_spedef_exp , "id is invalid!![%d]\n",id );
  
  if( PP_Get( pp , id , NULL ) > 0 )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------
//  現在のなつき度から対応するなつき度増減値を取得
//--------------------------------------------------------------------------
static const s32 PLIST_ITEM_UTIL_GetFriendValue( POKEMON_PARAM *pp , u16 itemNo , HEAPID heapId )
{
  const u32 friend = PP_Get( pp , ID_PARA_friend , NULL );
  if( friend < 100 )
  {
    return ITEM_GetParam( itemNo , ITEM_PRM_FRIEND1 , heapId );
  }
  else
  if( friend < 200 )
  {
    return ITEM_GetParam( itemNo , ITEM_PRM_FRIEND2 , heapId );
  }
  else
  {
    return ITEM_GetParam( itemNo , ITEM_PRM_FRIEND3 , heapId );
  }
}
