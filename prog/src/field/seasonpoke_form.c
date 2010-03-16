//============================================================================================
/**
 * @file  seasonpoke_form.c
 * @bfief 季節ポケモンフォールムチェンジ
 * @author  Saito
 */
//============================================================================================
#include <gflib.h>

#include "seasonpoke_form.h"
#include "gamesystem/pm_season.h"
#include "poke_tool/monsnum_def.h"  // for MONSNO_数字

//--------------------------------------------------------------
/**
 * @brief   手持ち季節ポケモンを指定した季節のフォルムにする
 *
 * @param   ppt		POKEPARTYへのポインタ
 * @param   inSeason    季節ＩＤ
 */
//--------------------------------------------------------------
void SEASONPOKE_FORM_ChangeForm(POKEPARTY *ppt, const u8 inSeason)
{
	int pos, count, monsno;
	POKEMON_PARAM *pp;
	int set_form_no_poke511;
  int set_form_no_poke527;

  //指定四季で分岐 @todo
  switch(inSeason){
  case PMSEASON_SPRING:
    set_form_no_poke511 = FORMNO_511_SPRING;
    set_form_no_poke527 = FORMNO_527_SPRING;
    break;
  case PMSEASON_SUMMER:
    set_form_no_poke511 = FORMNO_511_SUMMER;
    set_form_no_poke527 = FORMNO_527_SUMMER;
    break;
  case PMSEASON_AUTUMN:
    set_form_no_poke511 = FORMNO_511_AUTUMN;
    set_form_no_poke527 = FORMNO_527_AUTUMN;
    break;
  case PMSEASON_WINTER:
    set_form_no_poke511 = FORMNO_511_WINTER;
    set_form_no_poke527 = FORMNO_527_WINTER;
    break;
  default:
    GF_ASSERT_MSG(0,"season error %d",inSeason);
    set_form_no_poke511 = FORMNO_511_SUMMER;
    set_form_no_poke527 = FORMNO_527_SUMMER;
  }
	
	count = PokeParty_GetPokeCount(ppt);
	for(pos = 0; pos < count; pos++)
  {
		pp = PokeParty_GetMemberPointer(ppt,pos);
		monsno = PP_Get(pp, ID_PARA_monsno, NULL);
		if ( monsno == MONSNO_511 ) PP_ChangeFormNo( pp, set_form_no_poke511 );
    else if ( monsno == MONSNO_527 ) PP_ChangeFormNo( pp, set_form_no_poke527 );
	}
}

