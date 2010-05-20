//======================================================================
/**
 * @file	field_skill_check.h
 * @brief	フィールド技の状態チェック(技忘れよう
 * @author	ariizumi
 * @data	10/03/25
 *
 * モジュール名：FIELD_SKILL_CHECK
 */
//======================================================================
#pragma once
#include "gamesystem/game_data.h"

typedef enum
{
  FSCB_NAMINORI = 1<<0,
  FSCB_DIVING   = 1<<1,
  FSCB_FLASH    = 1<<2,
  FSCB_KAIRIKI  = 1<<3,
}FIELD_SKILL_CHECK_BIT;

typedef enum
{
  FSCR_OK,
  FSCR_HIDEN,       //秘伝技
}FIELD_SKILL_CHECK_RET;

extern const u16 FIELD_SKILL_CHECK_GetUseSkillBit( GAMEDATA *gameData );
//技マシンの所持と使用状況でチェック
extern const FIELD_SKILL_CHECK_RET FIELD_SKILL_CHECK_CheckForgetSkill( GAMEDATA *gameData , const u16 wazaNo , HEAPID heapId );
//技を持ってるかだけチェック
extern const BOOL FIELD_SKILL_CHECK_CanTradePoke( POKEMON_PASO_PARAM *ppp , HEAPID heapId );
