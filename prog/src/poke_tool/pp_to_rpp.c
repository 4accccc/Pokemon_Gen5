//==============================================================================
/**
 * @file    pp_to_rpp.c
 * @brief   戦闘録画用変換関数
 * @author  matsuda
 * @date    2009.11.19(木)
 */
//==============================================================================
#include    <gflib.h>

#include    "waza_tool/wazano_def.h"
#include    "poke_tool/poke_personal.h"
#include    "poke_tool/poke_tool.h"
#include    "poke_tool/monsno_def.h"
#include    "poke_tool/tokusyu_def.h"
#include    "waza_tool/wazadata.h"
#include    "print/global_msg.h"
#include    "print/str_tool.h"
#include    "system/gfl_use.h"

#include    "poke_tool_def.h"
#include    "poke_personal_local.h"
#include    "arc_def.h"
#include    "savedata/mail_util.h"


//--------------------------------------------------------------
/**
 * @brief   POKEMON_PARAMをREC_POKEPARAに変換する
 *
 * @param   pp    変換元データ
 * @param   rec   変換後のデータ代入先
 */
//--------------------------------------------------------------
void POKETOOL_PokePara_to_RecPokePara( POKEMON_PARAM *pp, REC_POKEPARA *rec)
{
  POKEMON_PASO_PARAM *ppp;
  POKEMON_PASO_PARAM1 *ppp1;
  POKEMON_PASO_PARAM2 *ppp2;
  POKEMON_PASO_PARAM3 *ppp3;
  POKEMON_PASO_PARAM4 *ppp4;
  int i;
  u16 sum;

  GFL_STD_MemClear( rec, sizeof(REC_POKEPARA) );

  //暗号復号
  if(pp->ppp.pp_fast_mode==0){
    POKETOOL_decord_data(&pp->pcp,sizeof(POKEMON_CALC_PARAM),pp->ppp.personal_rnd);
    POKETOOL_decord_data(&pp->ppp.paradata,sizeof(POKEMON_PASO_PARAM1)*4,pp->ppp.checksum);
  }

  ppp = &pp->ppp;
  ppp1=(POKEMON_PASO_PARAM1 *)POKETOOL_ppp_get_param_block(ppp,ppp->personal_rnd,ID_POKEPARA1);
  ppp2=(POKEMON_PASO_PARAM2 *)POKETOOL_ppp_get_param_block(ppp,ppp->personal_rnd,ID_POKEPARA2);
  ppp3=(POKEMON_PASO_PARAM3 *)POKETOOL_ppp_get_param_block(ppp,ppp->personal_rnd,ID_POKEPARA3);
  ppp4=(POKEMON_PASO_PARAM4 *)POKETOOL_ppp_get_param_block(ppp,ppp->personal_rnd,ID_POKEPARA4);

  //pokemon_paso_param
  rec->personal_rnd = ppp->personal_rnd;
  rec->pp_fast_mode = 0;//ppp->pp_fast_mode;
  rec->ppp_fast_mode = 0;//ppp->ppp_fast_mode;
  rec->fusei_tamago_flag = ppp->fusei_tamago_flag;
  rec->seikaku = ppp2->seikaku;

  //ppp1
  rec->monsno = ppp1->monsno;
  rec->item = ppp1->item;
  rec->id_no = ppp1->id_no;
  rec->exp = ppp1->exp;
  rec->friend = ppp1->friend;
  rec->speabino = ppp1->speabino;
  rec->hp_exp = ppp1->hp_exp;
  rec->pow_exp = ppp1->pow_exp;
  rec->def_exp = ppp1->def_exp;
  rec->agi_exp = ppp1->agi_exp;
  rec->spepow_exp = ppp1->spepow_exp;
  rec->spedef_exp = ppp1->spedef_exp;
  rec->language = ppp1->country_code;

  //ppp2
  for(i = 0; i < PTL_WAZA_MAX; i++){
    rec->waza[i] = ppp2->waza[i];
    rec->pp[i] = ppp2->pp[i];
    rec->pp_count[i] = ppp2->pp_count[i];
  }
  rec->hp_rnd = ppp2->hp_rnd;
  rec->pow_rnd = ppp2->pow_rnd;
  rec->def_rnd = ppp2->def_rnd;
  rec->agi_rnd = ppp2->agi_rnd;
  rec->spepow_rnd = ppp2->spepow_rnd;
  rec->spedef_rnd = ppp2->spedef_rnd;
  rec->tamago_flag = ppp2->tamago_flag;
  rec->nickname_flag = ppp2->nickname_flag;
  rec->event_get_flag = ppp2->event_get_flag;
  rec->sex = ppp2->sex;
  rec->form_no = ppp2->form_no;

  //ppp3
  for(i = 0; i < MONS_NAME_SIZE+EOM_SIZE; i++){
    rec->nickname[i] = ppp3->nickname[i];
  }

  //ppp4
  for(i = 0; i < PERSON_NAME_SIZE+EOM_SIZE; i++){
    rec->oyaname[i] = ppp4->oyaname[i];
  }
  rec->get_ball = ppp4->get_ball;

  //calc
  rec->condition = pp->pcp.condition;
  rec->level = pp->pcp.level;
  rec->cb_id = pp->pcp.cb_id;
  rec->hp = pp->pcp.hp;
  rec->hpmax = pp->pcp.hpmax;
  rec->pow = pp->pcp.pow;
  rec->def = pp->pcp.def;
  rec->agi = pp->pcp.agi;
  rec->spepow = pp->pcp.spepow;
  rec->spedef = pp->pcp.spedef;
//  rec->cb_core = pp->pcp.cb_core;

  //暗号化
  if(pp->ppp.pp_fast_mode==0){
    POKETOOL_encode_data(&pp->pcp,sizeof(POKEMON_CALC_PARAM),pp->ppp.personal_rnd);
    POKETOOL_encode_data(&pp->ppp.paradata,sizeof(POKEMON_PASO_PARAM1)*4,pp->ppp.checksum);
  }
}
