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
#include "plist_plate.h"
#include "plist_message.h"
#include "plist_item.h"
#include "poke_tool/status_rcv.h"

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
  
  //メッセージのステータスと順番依存！！
  ITEM_TYPE_HP_UP,      // HP努力値UP
  ITEM_TYPE_ATC_UP,     // 攻撃努力値UP
  ITEM_TYPE_DEF_UP,     // 防御努力値UP
  ITEM_TYPE_AGL_UP,     // 素早さ努力値UP
  ITEM_TYPE_SPA_UP,     // 特攻努力値UP
  ITEM_TYPE_SPD_UP,     // 特防努力値UP

  //メッセージのステータスと順番依存！！
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

static void PSTATUS_HPANMCB_ReturnRecoverHp( PLIST_WORK *work );
static void PSTATUS_HPANMCB_ReturnRecoverAllDeath( PLIST_WORK *work );
static void PSTATUS_MSGCB_RecoverAllDeath_NextPoke( PLIST_WORK *work );

static const u16 PLIST_ITEM_UTIL_GetParamExpSum( POKEMON_PARAM *pp );
static const BOOL PLIST_ITEM_UTIL_CanAddParamExp( POKEMON_PARAM *pp , const int id , u16 item );
static const BOOL PLIST_ITEM_UTIL_CanSubParamExp( POKEMON_PARAM *pp , const int id , u16 item );
static const s32 PLIST_ITEM_UTIL_GetFriendValue( POKEMON_PARAM *pp , u16 itemNo , HEAPID heapId );
static void PLIST_ITEM_UTIL_ItemUseMessageCommon( PLIST_WORK *work , u16 msgId );
static void PLIST_ITEM_UTIL_ItemUseMessageParamExp( PLIST_WORK *work , u16 msgId , u16 statusID );

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
//  技選択が必要か？
//--------------------------------------------------------------------------
const BOOL PLIST_ITEM_IsNeedSelectSkill( PLIST_WORK *work , u16 itemNo )
{
  PLIST_ITEM_USE_TYPE useType = PLIST_ITEM_RecoverCheck( itemNo );
  
  switch( useType )
  {
  case ITEM_TYPE_PP_UP:      // ppUp系
    return TRUE;
    break;

  case ITEM_TYPE_PP_3UP:     // pp3Up系
    return TRUE;
    break;

  case ITEM_TYPE_PP_RCV:     // pp回復系
    //単体使用か？
    if( ITEM_GetParam( itemNo , ITEM_PRM_PP_RCV , work->heapId ) != 0 )
    {
      return TRUE;
    }
    break;  
  }
  return FALSE;
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
//  全体復活アイテム対象がいるか(-1で居ない
//--------------------------------------------------------------------------
const int PLIST_ITEM_CanUseDeathRecoverAllItem( PLIST_WORK *work )
{
  u8 i;
  const u8 partyMax = PokeParty_GetPokeCount( work->plData->pp );
  for( i=0;i<partyMax;i++ )
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, i);
    const BOOL canUse = STATUS_RCV_RecoverCheck( pp , work->plData->item , 0 , work->heapId );
    if( canUse == TRUE )
    {
      return i;
    }
  }
  return -1;
}

//--------------------------------------------------------------------------
//  アイテム使用時、技選択のMSG番号の取得
//--------------------------------------------------------------------------
u32 PLIST_ITEM_GetWazaListMessage( PLIST_WORK *work , u16 itemNo )
{
  PLIST_ITEM_USE_TYPE useType = PLIST_ITEM_RecoverCheck( itemNo );
  
  switch( useType )
  {
  case ITEM_TYPE_PP_UP:      // ppUp系
  case ITEM_TYPE_PP_3UP:     // pp3Up系
    return mes_pokelist_03_05;  //どの技を増やす？
    break;

  case ITEM_TYPE_PP_RCV:     // pp回復系
    return mes_pokelist_03_04;  //どの技を回復する？
    break;  
  }
  return mes_pokelist_03_04;
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

//--------------------------------------------------------------------------
//  アイテム使った時の処理
//--------------------------------------------------------------------------
void PLIST_ITEM_MSG_UseItemFunc( PLIST_WORK *work )
{
  PLIST_ITEM_USE_TYPE useType = PLIST_ITEM_RecoverCheck( work->plData->item );
  
  switch( useType )
  {
  case ITEM_TYPE_BTL_ST_UP:  // 戦闘用ステータスアップ系
    GF_ASSERT_MSG( 0,"使ってないはず。\n");
    break;

  case ITEM_TYPE_ALLDETH_RCV:    // 全員瀕死回復
    work->mainSeq = PSMS_INIT_HPANIME;
    work->befHp = PLIST_PLATE_GetDispHp( work , work->plateWork[work->pokeCursor] );
    work->hpAnimeCallBack = PSTATUS_HPANMCB_ReturnRecoverAllDeath;

    break;

  case ITEM_TYPE_LV_UP:      // LvUp系
    break;

  case ITEM_TYPE_NEMURI_RCV:   // 眠り回復
    GF_ASSERT_MSG( 0,"使ってないはず。\n");
    break;

  case ITEM_TYPE_DOKU_RCV:     // 毒回復
    PLIST_ITEM_UTIL_ItemUseMessageCommon( work , mes_pokelist_04_15 );
    break;

  case ITEM_TYPE_YAKEDO_RCV:   // 火傷回復
    PLIST_ITEM_UTIL_ItemUseMessageCommon( work , mes_pokelist_04_17 );
    break;

  case ITEM_TYPE_KOORI_RCV:    // 氷回復
    PLIST_ITEM_UTIL_ItemUseMessageCommon( work , mes_pokelist_04_18 );
    break;

  case ITEM_TYPE_MAHI_RCV:     // 麻痺回復
    PLIST_ITEM_UTIL_ItemUseMessageCommon( work , mes_pokelist_04_16 );
    break;

  case ITEM_TYPE_KONRAN_RCV:   // 混乱回復
    GF_ASSERT_MSG( 0,"使ってないはず。\n");
    break;

  case ITEM_TYPE_ALL_ST_RCV:   // 全快
    PLIST_ITEM_UTIL_ItemUseMessageCommon( work , mes_pokelist_04_21 );
    break;

  case ITEM_TYPE_MEROMERO_RCV:   // メロメロ回復
    GF_ASSERT_MSG( 0,"使ってないはず。\n");
    break;

  case ITEM_TYPE_HP_RCV:     // HP回復
  case ITEM_TYPE_DEATH_RCV:  // 瀕死回復 (WB追加
    work->mainSeq = PSMS_INIT_HPANIME;
    work->befHp = PLIST_PLATE_GetDispHp( work , work->plateWork[work->pokeCursor] );
    work->hpAnimeCallBack = PSTATUS_HPANMCB_ReturnRecoverHp;

    break;
    
  case ITEM_TYPE_HP_UP:      // HP努力値UP
  case ITEM_TYPE_ATC_UP:     // 攻撃努力値UP
  case ITEM_TYPE_DEF_UP:     // 防御努力値UP
  case ITEM_TYPE_AGL_UP:     // 素早さ努力値UP
  case ITEM_TYPE_SPA_UP:     // 特攻努力値UP
  case ITEM_TYPE_SPD_UP:     // 特防努力値UP
    {
      u16 statusId = useType-ITEM_TYPE_HP_UP;
      
      PLIST_ITEM_UTIL_ItemUseMessageParamExp( work , mes_pokelist_04_25 , statusId );
    }
    break;

  case ITEM_TYPE_HP_DOWN:      // HP努力値DOWN
  case ITEM_TYPE_ATC_DOWN:     // 攻撃努力値DOWN
  case ITEM_TYPE_DEF_DOWN:     // 防御努力値DOWN
  case ITEM_TYPE_AGL_DOWN:     // 素早さ努力値DOWN
  case ITEM_TYPE_SPA_DOWN:     // 特攻努力値DOWN
  case ITEM_TYPE_SPD_DOWN:     // 特防努力値DOWN
    {
      u16 statusId = useType-ITEM_TYPE_HP_DOWN;
      
      if( PLIST_ITEM_UTIL_CanSubParamExp( work->selectPokePara , ID_PARA_hp_exp+statusId , work->plData->item ) == FALSE )
      {
        //基礎ポイントは下がらない！
        PLIST_ITEM_UTIL_ItemUseMessageParamExp( work , mes_pokelist_04_53 , statusId );
      }
      else
      if( PP_Get( work->selectPokePara , ID_PARA_friend , NULL ) == PTL_FRIEND_MAX )
      {
        //なつき度は上がらない！
        PLIST_ITEM_UTIL_ItemUseMessageParamExp( work , mes_pokelist_04_52 , statusId );
      }
      else
      {
        PLIST_ITEM_UTIL_ItemUseMessageParamExp( work , mes_pokelist_04_51 , statusId );
      }
    }
    break;

  case ITEM_TYPE_EVO:        // 進化系
    break;
  case ITEM_TYPE_PP_UP:      // ppUp系
  case ITEM_TYPE_PP_3UP:     // pp3Up系
    PLIST_ITEM_UTIL_ItemUseMessageCommon( work , mes_pokelist_04_22 );
    break;
  case ITEM_TYPE_PP_RCV:     // pp回復系
    PLIST_ITEM_UTIL_ItemUseMessageCommon( work , mes_pokelist_04_19 );
    break;
  case ITEM_TYPE_ETC:        // その他
    break;
  }
  
}

#pragma mark [> HPAnime CallBack
static void PSTATUS_HPANMCB_ReturnRecoverHp( PLIST_WORK *work )
{
  u16 nowHp = PLIST_PLATE_GetDispHp( work , work->plateWork[work->pokeCursor] );
  
  work->plData->ret_mode = PL_RET_BAG;
  PLIST_MSG_CreateWordSet( work , work->msgWork );
  PLIST_MSG_AddWordSet_PokeName( work , work->msgWork , 0 , work->selectPokePara );
  if( work->befHp != 0 )
  {
    PLIST_MSG_AddWordSet_Value( work , work->msgWork , 1 , nowHp-work->befHp , 3 );
    PLIST_MessageWaitInit( work , mes_pokelist_04_14 , TRUE , PSTATUS_MSGCB_ExitCommon );
  }
  else
  {
    //瀕死から復活
    PLIST_MessageWaitInit( work , mes_pokelist_04_20 , TRUE , PSTATUS_MSGCB_ExitCommon );
  }
  PLIST_MSG_DeleteWordSet( work , work->msgWork );
}

static void PSTATUS_HPANMCB_ReturnRecoverAllDeath( PLIST_WORK *work )
{
  
  work->plData->ret_mode = PL_RET_BAG;
  PLIST_MSG_CreateWordSet( work , work->msgWork );
  PLIST_MSG_AddWordSet_PokeName( work , work->msgWork , 0 , work->selectPokePara );
  if( PLIST_ITEM_CanUseDeathRecoverAllItem( work ) != -1 )
  {
    //他に対象がいるので続く
    PLIST_MessageWaitInit( work , mes_pokelist_04_20 , TRUE , PSTATUS_MSGCB_RecoverAllDeath_NextPoke );
  }
  else
  {
    //他に対象がいないので終了
    PLIST_MessageWaitInit( work , mes_pokelist_04_20 , TRUE , PSTATUS_MSGCB_ExitCommon );
  }
  PLIST_MSG_DeleteWordSet( work , work->msgWork );
}

static void PSTATUS_MSGCB_RecoverAllDeath_NextPoke( PLIST_WORK *work )
{
  const int target = PLIST_ITEM_CanUseDeathRecoverAllItem( work );

  PLIST_MSG_CloseWindow( work , work->msgWork );
  PLIST_PLATE_ChangeColor( work , work->plateWork[work->pokeCursor] , PPC_NORMAL );

  work->pokeCursor = target;
  work->selectPokePara = PokeParty_GetMemberPointer(work->plData->pp, target);

  PLIST_ITEM_MSG_UseItemFunc( work );
  STATUS_RCV_Recover( work->selectPokePara , work->plData->item , 0 , work->plData->place , work->heapId );
  PLIST_PLATE_ReDrawParam( work , work->plateWork[work->pokeCursor] );
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

//--------------------------------------------------------------------------
//  メッセージ表示汎用(WORDSET１にニックネーム 表示して終了
//--------------------------------------------------------------------------
static void PLIST_ITEM_UTIL_ItemUseMessageCommon( PLIST_WORK *work , u16 msgId )
{
  work->plData->ret_mode = PL_RET_BAG;
  PLIST_MSG_CreateWordSet( work , work->msgWork );
  PLIST_MSG_AddWordSet_PokeName( work , work->msgWork , 0 , work->selectPokePara );
  PLIST_MessageWaitInit( work , msgId , TRUE , PSTATUS_MSGCB_ExitCommon );
  PLIST_MSG_DeleteWordSet( work , work->msgWork );
}

//--------------------------------------------------------------------------
//  メッセージ表示努力値(WORDSET１にニックネーム WORDSET２に努力値種類 表示して終了
//--------------------------------------------------------------------------
static void PLIST_ITEM_UTIL_ItemUseMessageParamExp( PLIST_WORK *work , u16 msgId , u16 statusID )
{
  work->plData->ret_mode = PL_RET_BAG;
  PLIST_MSG_CreateWordSet( work , work->msgWork );
  PLIST_MSG_AddWordSet_PokeName( work , work->msgWork , 0 , work->selectPokePara );
  PLIST_MSG_AddWordSet_StatusName( work , work->msgWork , 1 , statusID );
  PLIST_MessageWaitInit( work , msgId , TRUE , PSTATUS_MSGCB_ExitCommon );
  PLIST_MSG_DeleteWordSet( work , work->msgWork );
  
}
