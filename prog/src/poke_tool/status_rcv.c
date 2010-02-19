//============================================================================================
/**
 * @file  status_rcv.c
 * @brief アイテム使用時のポケモン回復処理
 * @author  Hiroyuki Nakamura
 * @date  05.12.01
 *        09.08.06 GSより移植
 */
//============================================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "waza_tool/waza_tool.h"
#include "waza_tool/wazadata.h"
#include "poke_tool/natsuki.h"
#include "poke_tool/monsno_def.h"
#include "item/item.h"

#include "poke_tool/status_rcv.h"

//============================================================================================
//  定数定義
//============================================================================================
#define TEMP_BUFLEN   ( 8 )   // 計算用ワーク数

#define RCV_25PER   ( 253 )   // 25%回復
#define RCV_HALF    ( 254 )   // 半回復
#define RCV_ALL     ( 255 )   // 全回復

#define PP_RCV_ALL    ( 127 )   // PP全回復

#define PRMEXP_MAX    ( 510 )   // 努力値の合計の最大値
#define ITEM_PRMEXP_MAX ( 100 )   // 道具でアップする努力値の最大値

//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static u8 PP_RcvCheck( POKEMON_PARAM * pp, u32 pos );
static u8 PP_Recover( POKEMON_PARAM * pp, u32 pos, u32 rcv );
static u8 PP_Up( POKEMON_PARAM * pp, u32 pos, u32 cnt );
static void HP_Recover( POKEMON_PARAM * pp, u32 hp, u32 max, u32 rcv );
static s32 PrmExp_Up( s32 exp, s32 other, s32 up );
static u8 Friend_Up( POKEMON_PARAM * pp, s32 now, s32 prm, u16 itemNo , u16 place, u32 heap );
static u8 Friend_UpCheck( POKEMON_PARAM * pp, ITEMDATA * dat );


//--------------------------------------------------------------------------------------------
/**
 * 道具使用チェック処理 ( POKEMON_PARAM )
 *
 * @param pp      回復するポケモンのデータ
 * @param item    使用するアイテム
 * @param pos     回復位置（技位置など）
 * @param heap_id   ヒープID
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
u8 STATUS_RCV_RecoverCheck( POKEMON_PARAM * pp, u16 item, u16 pos, u32 heap_id )
{
  ITEMDATA * dat;
  s32 tmp[TEMP_BUFLEN];

  dat = ITEM_GetItemArcData( item, ITEM_GET_DATA, heap_id );

  // ワークタイプをチェック
  if( ITEM_GetBufParam( dat, ITEM_PRM_W_TYPE ) != 1 ){
    GFL_HEAP_FreeMemory( dat );
    return FALSE;
  }

  tmp[0] = PP_GetSick( pp );

  // 眠り回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_SLEEP_RCV ) != 0 ){
    if( ( tmp[0] == POKESICK_NEMURI ) != 0 ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  }
  // 毒回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_POISON_RCV ) != 0 ){
    if( ( tmp[0] == POKESICK_DOKU ) != 0 ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  }
  // 火傷回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_BURN_RCV ) != 0 ){
    if( ( tmp[0] == POKESICK_YAKEDO ) != 0 ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  }
  // 氷回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_ICE_RCV ) != 0 ){
    if( ( tmp[0] == POKESICK_KOORI ) != 0 ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  }
  // 麻痺回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_PARALYZE_RCV ) != 0 ){
    if( ( tmp[0] == POKESICK_MAHI ) != 0 ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  }

  tmp[0] = PP_Get( pp, ID_PARA_hp, NULL );

  // 瀕死回復 or 全員瀕死回復
  if( ( ITEM_GetBufParam( dat, ITEM_PRM_DEATH_RCV ) != 0 ||
      ITEM_GetBufParam( dat, ITEM_PRM_ALL_DEATH_RCV ) != 0 )
     && ITEM_GetBufParam( dat, ITEM_PRM_LV_UP ) == 0 ){

    if( tmp[0] == 0 ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  // HP回復
  }else if( ITEM_GetBufParam( dat, ITEM_PRM_HP_RCV ) != 0 ){
    if( tmp[0] != 0 && tmp[0] < PP_Get( pp, ID_PARA_hpmax, NULL ) ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  }

  // レベルアップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_LV_UP ) != 0 ){
    if( PP_Get( pp, ID_PARA_level, NULL ) < 100 ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  }
  //FIXME 進化チェック(でも今回違う処理にするかも
/*
  // 進化
  if( ITEM_GetBufParam( dat, ITEM_PRM_EVOLUTION ) != 0 ){
    if( PokeShinkaCheck( NULL, pp, ITEM_SHINKA_CHECK, item ,NULL) != 0 ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  }
*/
  // PPアップ or PPアップ（３段階）
  if( ITEM_GetBufParam( dat, ITEM_PRM_PP_UP ) != 0 ||
    ITEM_GetBufParam( dat, ITEM_PRM_PP_3UP ) != 0 ){

    if( PP_Get( pp, ID_PARA_pp_count1+pos, NULL ) < 3 &&
      WAZADATA_GetMaxPP( PP_Get(pp,ID_PARA_waza1+pos,NULL), 0 ) >= 5 ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  }

  // PP回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_PP_RCV ) != 0 ){
    if( PP_RcvCheck( pp, pos ) == TRUE ){
      GFL_HEAP_FreeMemory( dat );
      return TRUE;
    }
  }
  // PP回復（全ての技）
  if( ITEM_GetBufParam( dat, ITEM_PRM_ALL_PP_RCV ) != 0 ){
    for( tmp[0]=0; tmp[0]<4; tmp[0]++ ){
      if( PP_RcvCheck( pp, tmp[0] ) == TRUE ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }
  }

  tmp[0] = PP_Get( pp, ID_PARA_hp_exp, NULL );
  tmp[1] = PP_Get( pp, ID_PARA_pow_exp, NULL );
  tmp[2] = PP_Get( pp, ID_PARA_def_exp, NULL );
  tmp[3] = PP_Get( pp, ID_PARA_agi_exp, NULL );
  tmp[4] = PP_Get( pp, ID_PARA_spepow_exp, NULL );
  tmp[5] = PP_Get( pp, ID_PARA_spedef_exp, NULL );

  // HP努力値アップ
  if( PP_Get( pp, ID_PARA_monsno, NULL ) != MONSNO_NUKENIN ){
    if( ITEM_GetBufParam( dat, ITEM_PRM_HP_EXP ) != 0 ){
      tmp[6] = ITEM_GetBufParam( dat, ITEM_PRM_HP_EXP_POINT );
      OS_Printf( "HP EXP : now = %d, prm = %d\n", tmp[0], tmp[6] );
      if( tmp[6] > 0 ){
        if( tmp[0] < ITEM_PRMEXP_MAX &&
          (tmp[0]+tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5]) < PRMEXP_MAX ){
          GFL_HEAP_FreeMemory( dat );
          return TRUE;
        }
      }else if( tmp[6] < 0 ){
        if( tmp[0] > 0 ){
          GFL_HEAP_FreeMemory( dat );
          return TRUE;
        }
        if( Friend_UpCheck( pp, dat ) == TRUE ){
          GFL_HEAP_FreeMemory( dat );
          return TRUE;
        }
      }
    }
  }
  // 攻撃努力値アップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_POWER_EXP ) != 0 ){
    tmp[6] = ITEM_GetBufParam( dat, ITEM_PRM_POWER_EXP_POINT );
    if( tmp[6] > 0 ){
      if( tmp[1] < ITEM_PRMEXP_MAX &&
        (tmp[0]+tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5]) < PRMEXP_MAX ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }else if( tmp[6] < 0 ){
      if( tmp[1] > 0 ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
      if( Friend_UpCheck( pp, dat ) == TRUE ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }
  }
  // 防御努力値アップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_DEFENCE_EXP ) != 0 ){
    tmp[6] = ITEM_GetBufParam( dat, ITEM_PRM_DEFENCE_EXP_POINT );
    if( tmp[6] > 0 ){
      if( tmp[2] < ITEM_PRMEXP_MAX &&
        (tmp[0]+tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5]) < PRMEXP_MAX ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }else if( tmp[6] < 0 ){
      if( tmp[2] > 0 ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
      if( Friend_UpCheck( pp, dat ) == TRUE ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }
  }
  // 素早さ努力値アップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_AGILITY_EXP ) != 0 ){
    tmp[6] = ITEM_GetBufParam( dat, ITEM_PRM_AGILITY_EXP_POINT );
    if( tmp[6] > 0 ){
      if( tmp[3] < ITEM_PRMEXP_MAX &&
        (tmp[0]+tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5]) < PRMEXP_MAX ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }else if( tmp[6] < 0 ){
      if( tmp[3] > 0 ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
      if( Friend_UpCheck( pp, dat ) == TRUE ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }
  }
  // 特攻努力値アップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_SP_ATTACK_EXP ) != 0 ){
    tmp[6] = ITEM_GetBufParam( dat, ITEM_PRM_SP_ATTACK_EXP_POINT );
    if( tmp[6] > 0 ){
      if( tmp[4] < ITEM_PRMEXP_MAX &&
        (tmp[0]+tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5]) < PRMEXP_MAX ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }else if( tmp[6] < 0 ){
      if( tmp[4] > 0 ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
      if( Friend_UpCheck( pp, dat ) == TRUE ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }
  }
  // 特防努力値アップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_SP_DEFENCE_EXP ) != 0 ){
    tmp[6] = ITEM_GetBufParam( dat, ITEM_PRM_SP_DEFENCE_EXP_POINT );
    if( tmp[6] > 0 ){
      if( tmp[5] < ITEM_PRMEXP_MAX &&
        (tmp[0]+tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5]) < PRMEXP_MAX ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }else if( tmp[6] < 0 ){
      if( tmp[5] > 0 ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
      if( Friend_UpCheck( pp, dat ) == TRUE ){
        GFL_HEAP_FreeMemory( dat );
        return TRUE;
      }
    }
  }

  GFL_HEAP_FreeMemory( dat );

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 道具使用チェック処理 ( POKEPARTY )
 *
 * @param party   回復するポケモンのデータ
 * @param item    使用するアイテム
 * @param pos1    ポケモンデータの中の位置
 * @param pos2    回復位置（技位置など）
 * @param heap_id   ヒープID
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
u8 STATUS_RCV_PokeParty_RecoverCheck( POKEPARTY * party, u16 item, u8 pos1, u8 pos2, u32 heap_id )
{
  POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, pos1 );

  return STATUS_RCV_RecoverCheck( pp, item, pos2, heap_id );
}


//--------------------------------------------------------------------------------------------
/**
 * 回復処理 ( POKEMON_PARAM )
 *
 * @param pp      回復するポケモンのデータ
 * @param item    使用するアイテム
 * @param pos     回復位置（技位置など）
 * @param place   場所
 * @param heap_id   ヒープID
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
u8 STATUS_RCV_Recover( POKEMON_PARAM * pp, u16 item, u16 pos, u16 place, u32 heap_id )
{
  ITEMDATA * dat;
  s32 tmp[TEMP_BUFLEN];
  u8  use_flg;
  u8  dat_flg;

  dat = ITEM_GetItemArcData( item, ITEM_GET_DATA, heap_id );

  // ワークタイプをチェック
  if( ITEM_GetBufParam( dat, ITEM_PRM_W_TYPE ) != 1 ){
    GFL_HEAP_FreeMemory( dat );
    return FALSE;
  }

  use_flg = FALSE;
  dat_flg = FALSE;

  tmp[0] = PP_Get( pp, ID_PARA_condition, NULL );
  tmp[1] = tmp[0];

  // 眠り回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_SLEEP_RCV ) != 0 ){
    tmp[1] = POKESICK_NULL;
    dat_flg = TRUE;
  }
  // 毒回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_POISON_RCV ) != 0 ){
    tmp[1] = POKESICK_NULL;
    dat_flg = TRUE;
  }
  // 火傷回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_BURN_RCV ) != 0 ){
    tmp[1] = POKESICK_NULL;
    dat_flg = TRUE;
  }
  // 氷回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_ICE_RCV ) != 0 ){
    tmp[1] = POKESICK_NULL;
    dat_flg = TRUE;
  }
  // 麻痺回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_PARALYZE_RCV ) != 0 ){
    tmp[1] = POKESICK_NULL;
    dat_flg = TRUE;
  }
  if( tmp[0] != tmp[1] ){
    PP_Put( pp, ID_PARA_condition, tmp[1] );
    use_flg = TRUE;
  }

  tmp[0] = PP_Get( pp, ID_PARA_hp, NULL );
  tmp[1] = PP_Get( pp, ID_PARA_hpmax, NULL );

  // 瀕死回復 or 全員瀕死回復
  if( ( ITEM_GetBufParam(dat,ITEM_PRM_DEATH_RCV) != 0 ||
      ITEM_GetBufParam(dat,ITEM_PRM_ALL_DEATH_RCV) != 0 )
    && ITEM_GetBufParam( dat, ITEM_PRM_LV_UP ) != 0 ){
    if( tmp[0] == 0 ){
      HP_Recover( pp, tmp[0], tmp[1], ITEM_GetBufParam(dat,ITEM_PRM_HP_RCV_POINT) );
      use_flg = TRUE;
    }
    dat_flg = TRUE;
  // HP回復
  }else if( ITEM_GetBufParam( dat, ITEM_PRM_HP_RCV ) != 0 ){
    if( tmp[0] < tmp[1] ){
      HP_Recover( pp, tmp[0], tmp[1], ITEM_GetBufParam(dat,ITEM_PRM_HP_RCV_POINT) );
      use_flg = TRUE;
    }
    dat_flg = TRUE;
  }

  tmp[2] = PP_Get( pp, ID_PARA_level, NULL );

  // レベルアップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_LV_UP ) != 0 )
  {
    if( tmp[2] < 100 )
    {
      const u32 nextLevelExp = POKETOOL_GetMinExp(PP_Get( pp, ID_PARA_monsno, NULL ),
                                                  PP_Get( pp, ID_PARA_form_no, NULL ),
                                                  tmp[2]+1) ;
      PP_Put( pp, ID_PARA_exp, nextLevelExp );
      PP_Renew(pp);
      if( tmp[0] == 0 )
      {
        tmp[3] = PP_Get( pp, ID_PARA_hpmax, NULL );
        HP_Recover( pp, tmp[0], tmp[3], tmp[3]-tmp[1] );
      }
      use_flg = TRUE;
    }
    dat_flg = TRUE;
  }

  // 進化
  if( ITEM_GetBufParam( dat, ITEM_PRM_EVOLUTION ) != 0 ){
    dat_flg = TRUE;
  }

  // PPアップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_PP_UP ) != 0 ){
    if( PP_Up( pp, pos, 1 ) == TRUE ){
      use_flg = TRUE;
    }
    dat_flg = TRUE;
  // PPアップ（３段階）
  }else if( ITEM_GetBufParam( dat, ITEM_PRM_PP_3UP ) != 0 ){
    if( PP_Up( pp, pos, 3 ) == TRUE ){
      use_flg = TRUE;
    }
    dat_flg = TRUE;
  }

  // PP回復
  if( ITEM_GetBufParam( dat, ITEM_PRM_PP_RCV ) != 0 ){
    if( PP_Recover( pp, pos, ITEM_GetBufParam(dat,ITEM_PRM_PP_RCV_POINT) ) == TRUE ){
      use_flg = TRUE;
    }
    dat_flg = TRUE;
  // PP回復（全ての技）
  }else if( ITEM_GetBufParam( dat, ITEM_PRM_ALL_PP_RCV ) != 0 ){
    for( tmp[0]=0; tmp[0]<4; tmp[0]++ ){
      if( PP_Recover( pp, tmp[0], ITEM_GetBufParam(dat,ITEM_PRM_PP_RCV_POINT) ) == TRUE ){
        use_flg = TRUE;
      }
    }
    dat_flg = TRUE;
  }

  tmp[0] = PP_Get( pp, ID_PARA_hp_exp, NULL );
  tmp[1] = PP_Get( pp, ID_PARA_pow_exp, NULL );
  tmp[2] = PP_Get( pp, ID_PARA_def_exp, NULL );
  tmp[3] = PP_Get( pp, ID_PARA_agi_exp, NULL );
  tmp[4] = PP_Get( pp, ID_PARA_spepow_exp, NULL );
  tmp[5] = PP_Get( pp, ID_PARA_spedef_exp, NULL );

  // HP努力値アップ
  if( PP_Get( pp, ID_PARA_monsno, NULL ) != MONSNO_NUKENIN )
  {
    if( ITEM_GetBufParam( dat, ITEM_PRM_HP_EXP ) != 0 )
    {
      tmp[7] = ITEM_GetBufParam( dat,ITEM_PRM_HP_EXP_POINT );
      tmp[6] = PrmExp_Up( tmp[0], (tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5]), tmp[7] );
      if( tmp[6] != -1 )
      {
        tmp[0] = tmp[6];
        PP_Put( pp, ID_PARA_hp_exp, tmp[0] );
        PP_Renew( pp );
        use_flg = TRUE;
      }
      if( tmp[7] > 0 ){ dat_flg = TRUE; }
    }
  }
  // 攻撃努力値アップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_POWER_EXP ) != 0 ){
    tmp[7] = ITEM_GetBufParam( dat,ITEM_PRM_POWER_EXP_POINT );
    tmp[6] = PrmExp_Up( tmp[1], (tmp[0]+tmp[2]+tmp[3]+tmp[4]+tmp[5]), tmp[7] );
    if( tmp[6] != -1 ){
      tmp[1] = tmp[6];
      PP_Put( pp, ID_PARA_pow_exp, tmp[1] );
      PP_Renew( pp );
      use_flg = TRUE;
    }
    if( tmp[7] > 0 ){ dat_flg = TRUE; }
  }
  // 防御努力値アップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_DEFENCE_EXP ) != 0 ){
    tmp[7] = ITEM_GetBufParam( dat,ITEM_PRM_DEFENCE_EXP_POINT );
    tmp[6] = PrmExp_Up( tmp[2], (tmp[0]+tmp[1]+tmp[3]+tmp[4]+tmp[5]), tmp[7] );
    if( tmp[6] != -1 ){
      tmp[2] = tmp[6];
      PP_Put( pp, ID_PARA_def_exp, tmp[2] );
      PP_Renew( pp );
      use_flg = TRUE;
    }
    if( tmp[7] > 0 ){ dat_flg = TRUE; }
  }
  // 素早さ努力値アップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_AGILITY_EXP ) != 0 ){
    tmp[7] = ITEM_GetBufParam( dat,ITEM_PRM_AGILITY_EXP_POINT );
    tmp[6] = PrmExp_Up( tmp[3], (tmp[0]+tmp[1]+tmp[2]+tmp[4]+tmp[5]), tmp[7] );
    if( tmp[6] != -1 ){
      tmp[3] = tmp[6];
      PP_Put( pp, ID_PARA_agi_exp, tmp[3] );
      PP_Renew( pp );
      use_flg = TRUE;
    }
    if( tmp[7] > 0 ){ dat_flg = TRUE; }
  }
  // 特攻努力値アップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_SP_ATTACK_EXP ) != 0 ){
    tmp[7] = ITEM_GetBufParam( dat,ITEM_PRM_SP_ATTACK_EXP_POINT );
    tmp[6] = PrmExp_Up( tmp[4], (tmp[0]+tmp[1]+tmp[2]+tmp[3]+tmp[5]), tmp[7] );
    if( tmp[6] != -1 ){
      tmp[4] = tmp[6];
      PP_Put( pp, ID_PARA_spepow_exp, tmp[4] );
      PP_Renew( pp );
      use_flg = TRUE;
    }
    if( tmp[7] > 0 ){ dat_flg = TRUE; }
  }
  // 特防努力値アップ
  if( ITEM_GetBufParam( dat, ITEM_PRM_SP_DEFENCE_EXP ) != 0 ){
    tmp[7] = ITEM_GetBufParam( dat,ITEM_PRM_SP_DEFENCE_EXP_POINT );
    tmp[6] = PrmExp_Up( tmp[5], (tmp[0]+tmp[1]+tmp[2]+tmp[3]+tmp[4]), tmp[7] );
    if( tmp[6] != -1 ){
      tmp[5] = tmp[6];
      PP_Put( pp, ID_PARA_spedef_exp, tmp[5] );
      PP_Renew( pp );
      use_flg = TRUE;
    }
    if( tmp[7] > 0 ){ dat_flg = TRUE; }
  }

  if( use_flg == FALSE && dat_flg == TRUE ){
    GFL_HEAP_FreeMemory( dat );
    return FALSE;
  }

  tmp[0] = PP_Get( pp, ID_PARA_friend, NULL );

  // なつき度１
  if( tmp[0] < 100 ){
    if( ITEM_GetBufParam( dat, ITEM_PRM_FRIEND1 ) != 0 ){
      //TODO どうする機嫌値
      //CalcHumor_ItemHumorUp(pp, item);  //機嫌値増減
      if( Friend_Up( pp, tmp[0], ITEM_GetBufParam(dat,ITEM_PRM_FRIEND1_POINT), item ,place, heap_id ) == FALSE ){
        GFL_HEAP_FreeMemory( dat );
        return use_flg;
      }
    }
  // なつき度２
  }else if( tmp[0] >= 100 && tmp[0] < 200 ){
    if( ITEM_GetBufParam( dat, ITEM_PRM_FRIEND2 ) != 0 ){
      //TODO どうする機嫌値
      //CalcHumor_ItemHumorUp(pp, item);  //機嫌値増減
      if( Friend_Up( pp, tmp[0], ITEM_GetBufParam(dat,ITEM_PRM_FRIEND2_POINT), item ,place, heap_id ) == FALSE ){
        GFL_HEAP_FreeMemory( dat );
        return use_flg;
      }
    }
  // なつき度３
  }else if( tmp[0] >= 200 && tmp[0] <= 255 ){
    if( ITEM_GetBufParam( dat, ITEM_PRM_FRIEND3 ) != 0 ){
      //TODO どうする機嫌値
      //CalcHumor_ItemHumorUp(pp, item);  //機嫌値増減
      if( Friend_Up( pp, tmp[0], ITEM_GetBufParam(dat,ITEM_PRM_FRIEND3_POINT), item ,place, heap_id ) == FALSE ){
        GFL_HEAP_FreeMemory( dat );
        return use_flg;
      }
    }
  }

  GFL_HEAP_FreeMemory( dat );
  return use_flg;
}

//--------------------------------------------------------------------------------------------
/**
 * 回復処理 ( POKEPARTY )
 *
 * @param party   回復するポケモンのデータ
 * @param item    使用するアイテム
 * @param pos1    ポケモンデータの中の位置
 * @param pos2    回復位置（技位置など）
 * @param place   場所
 * @param heap_id   ヒープID
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
u8 STATUS_RCV_PokeParty_Recover( POKEPARTY * party, u16 item, u8 pos1, u8 pos2, u16 place, u32 heap_id )
{
  POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, pos1 );

  return STATUS_RCV_Recover( pp, item, pos2, place, heap_id );
}


//--------------------------------------------------------------------------------------------
/**
 * PP回復チェック
 *
 * @param pp    回復するポケモンのデータ
 * @param pos   回復位置（技位置など）
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
static u8 PP_RcvCheck( POKEMON_PARAM * pp, u32 pos )
{
  u16 waza;
  u8  npp;
  u8  ppc;

  waza = (u16)PP_Get( pp, ID_PARA_waza1+pos, NULL );
  if( waza == 0 ){
    return FALSE;
  }
  npp = (u8)PP_Get( pp, ID_PARA_pp1+pos, NULL );
  ppc = (u8)PP_Get( pp, ID_PARA_pp_count1+pos, NULL );

  if( npp < WAZADATA_GetMaxPP( waza, ppc ) ){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * PP回復
 *
 * @param pp    回復するポケモンのデータ
 * @param pos   回復位置（技位置など）
 * @param rcv   回復値
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
static u8 PP_Recover( POKEMON_PARAM * pp, u32 pos, u32 rcv )
{
  u16 waza;
  u8  npp;
  u8  mpp;

  waza = (u16)PP_Get( pp, ID_PARA_waza1+pos, NULL );
  if( waza == 0 ){
    return FALSE;
  }
  npp  = (u8)PP_Get( pp, ID_PARA_pp1+pos, NULL );
  mpp  = (u8)WAZADATA_GetMaxPP( waza, PP_Get(pp,ID_PARA_pp_count1+pos,NULL) );

  if( npp < mpp ){
    if( rcv == PP_RCV_ALL ){
      npp = mpp;
    }else{
      npp += rcv;
      if( npp > mpp ){
        npp = mpp;
      }
    }
    PP_Put( pp, ID_PARA_pp1+pos, npp );
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * PPアップ
 *
 * @param pp    回復するポケモンのデータ
 * @param pos   回復位置（技位置など）
 * @param cnt   アップ値
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
static u8 PP_Up( POKEMON_PARAM * pp, u32 pos, u32 cnt )
{
  u16 waza;
  u8  npp;
  u8  ppc;
  u8  mpp;

  ppc = PP_Get( pp, ID_PARA_pp_count1+pos, NULL );
  if( ppc == 3 ){
    return FALSE;
  }

  waza = (u16)PP_Get( pp, ID_PARA_waza1+pos, NULL );

  if( WAZADATA_GetMaxPP( waza, 0 ) < 5 ){
    return FALSE;
  }

  npp  = (u8)PP_Get( pp, ID_PARA_pp1+pos, NULL );
  mpp  = (u8)WAZADATA_GetMaxPP( waza, ppc );

  if( ppc+cnt > 3 ){
    ppc = 3;
  }else{
    ppc = ppc + cnt;
  }
  npp = npp + WAZADATA_GetMaxPP( waza, ppc ) - mpp;

  PP_Put( pp, ID_PARA_pp_count1+pos, ppc );
  PP_Put( pp, ID_PARA_pp1+pos, npp );

  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * HP回復
 *
 * @param pp    回復するポケモンのデータ
 * @param hp    現在のHP
 * @param max   最大HP
 * @param rcv   回復値
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
static void HP_Recover( POKEMON_PARAM * pp, u32 hp, u32 max, u32 rcv )
{
  if( max == 1 ){
    rcv = 1;
  }else if( rcv == RCV_ALL ){
    rcv = max;
  }else if( rcv == RCV_HALF ){
    rcv = max/2;
  }else if( rcv == RCV_25PER ){
    rcv = max/4;
  }

  if( hp+rcv > max ){
    hp = max;
  }else{
    hp += rcv;
  }

  PP_Put( pp, ID_PARA_hp, hp );
}

//--------------------------------------------------------------------------------------------
/**
 * 努力値Up計算
 *
 * @param exp   現在の努力値
 * @param other その他の努力値の合計
 * @param up    アップする値
 *
 * @retval  "-1 = アップしない"
 * @retval  "0 != 計算結果（セットする努力値）"
 */
//--------------------------------------------------------------------------------------------
static s32 PrmExp_Up( s32 exp, s32 other, s32 up )
{
  if( exp == 0 && up < 0 ){
    return -1;
  }
  if( exp >= ITEM_PRMEXP_MAX && up > 0 ){
    return -1;
  }
  if( (exp+other) >= PRMEXP_MAX && up > 0 ){
    return -1;
  }

  exp += up;
  if( exp > ITEM_PRMEXP_MAX ){
    exp = ITEM_PRMEXP_MAX;
  }else if( exp < 0 ){
    exp = 0;
  }
  if( (exp+other) > PRMEXP_MAX ){
    exp = PRMEXP_MAX - other;
  }
  return exp;
}

//--------------------------------------------------------------------------------------------
/**
 * なつき度が上がるか
 *
 * @param pp    ポケモンのデータ
 * @param prm   変更値
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 *
 * @li  下がる場合は無視
 */
//--------------------------------------------------------------------------------------------
static u8 Friend_UpCheck( POKEMON_PARAM * pp, ITEMDATA * dat )
{
  s32 now;
  s32 prm;

  now = PP_Get( pp, ID_PARA_friend, NULL );
  if( now >= PTL_FRIEND_MAX ){ return FALSE; }

  // なつき度１
  if( now < 100 ){
    if( ITEM_GetBufParam( dat, ITEM_PRM_FRIEND1 ) != 0 ){
      if( ITEM_GetBufParam( dat,ITEM_PRM_FRIEND1_POINT ) > 0 ){
        return TRUE;
      }
    }
    return FALSE;
  // なつき度２
  }else if( now >= 100 && now < 200 ){
    if( ITEM_GetBufParam( dat, ITEM_PRM_FRIEND2 ) != 0 ){
      if( ITEM_GetBufParam( dat,ITEM_PRM_FRIEND2_POINT ) > 0 ){
        return TRUE;
      }
    }
    return FALSE;
  // なつき度３
  }else if( now >= 200 && now < 255 ){
    if( ITEM_GetBufParam( dat, ITEM_PRM_FRIEND3 ) != 0 ){
      if( ITEM_GetBufParam( dat,ITEM_PRM_FRIEND3_POINT ) > 0 ){
        return TRUE;
      }
    }
    return FALSE;
  }

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * なつき度Up
 *
 * @param pp    ポケモンのデータ
 * @param now   現在の値
 * @param prm   変更値
 * @param place 場所
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
static u8 Friend_Up( POKEMON_PARAM * pp, s32 now, s32 prm, u16 itemNo , u16 place, u32 heap )
{
  if( now == PTL_FRIEND_MAX && prm > 0 ){
    return FALSE;
  }
  if( now == 0 && prm < 0 ){
    return FALSE;
  }
/*
  if( prm > 0 ){
    // 捕獲ボール
    if( PP_Get( pp, ID_PARA_get_ball, NULL ) == ITEM_GOOZYASUBOORU ){
      prm++;
    }
    // 捕獲場所
    if( PP_Get( pp, ID_PARA_get_place, NULL ) == place ){
      prm++;
    }
    // 装備効果
    if( ITEM_GetParam(PP_Get(pp,ID_PARA_item,NULL),ITEM_PRM_EQUIP,heap)==SOUBI_NATUKIDOUP ){
      prm = prm * 150 / 100;
    }
  }

  prm += now;
  if( prm > PTL_FRIEND_MAX ){
    prm = PTL_FRIEND_MAX;
  }
  if( prm < 0 ){
    prm = 0;
  }
  PP_Put( pp, ID_PARA_friend, prm );
*/
  NATSUKI_CalcUseItem( pp , itemNo , place , heap );
  return TRUE;
}

//============================================================================================
//============================================================================================
//--------------------------------------------------------------------------------------------
/**ポケモン回復処理(単体・全体
 */
//--------------------------------------------------------------------------------------------
void STATUS_RCV_PokeParam_RecoverAll(POKEMON_PARAM * pp)
{
  int j;
  u32 buf;
  //HP全回復
  buf = PP_Get( pp, ID_PARA_hpmax, NULL );
  PP_Put(pp, ID_PARA_hp, buf);
  //状態異常回復
  buf = 0;
  PP_Put(pp, ID_PARA_condition, buf);
  //PP全回復
  for (j = 0; j < 4; j++) {
    if (PP_RcvCheck(pp,j) == TRUE) {
      PP_Recover(pp, j, PP_RCV_ALL);
    }
  }
}

void STATUS_RCV_PokeParty_RecoverAll(POKEPARTY * party)
{
  int i,total;
  POKEMON_PARAM * pp;

  total = PokeParty_GetPokeCount(party);
  for (i = 0; i < total; i++) {
    pp = PokeParty_GetMemberPointer(party, i);
    if (PP_Get(pp, ID_PARA_poke_exist, NULL) == FALSE) {
      continue;
    }
    STATUS_RCV_PokeParam_RecoverAll( pp );
  }
}




