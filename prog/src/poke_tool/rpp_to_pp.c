//==============================================================================
/**
 * @file    rpp_to_pp.c
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
 * @brief   REC_POKEPARAをPOKEMON_PARAMに変換する
 *
 * @param   rec   変換元データ
 * @param   pp    変換後のデータ代入先
 */
//--------------------------------------------------------------
void POKETOOL_RecPokePara_to_PokePara( REC_POKEPARA *rec, POKEMON_PARAM *pp)
{
  POKEMON_PASO_PARAM  *ppp;
  POKEMON_PASO_PARAM1 *ppp1;
  POKEMON_PASO_PARAM2 *ppp2;
  POKEMON_PASO_PARAM3 *ppp3;
  POKEMON_PASO_PARAM4 *ppp4;
  int i;

  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);
//  PokeParaInit(pp);
  GFL_STD_MemClear(pp,sizeof(POKEMON_PARAM));
  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);

  ppp = &pp->ppp;
  ppp1=(POKEMON_PASO_PARAM1 *)POKETOOL_ppp_get_param_block(ppp, rec->personal_rnd, ID_POKEPARA1);
  ppp2=(POKEMON_PASO_PARAM2 *)POKETOOL_ppp_get_param_block(ppp, rec->personal_rnd, ID_POKEPARA2);
  ppp3=(POKEMON_PASO_PARAM3 *)POKETOOL_ppp_get_param_block(ppp, rec->personal_rnd, ID_POKEPARA3);
  ppp4=(POKEMON_PASO_PARAM4 *)POKETOOL_ppp_get_param_block(ppp, rec->personal_rnd, ID_POKEPARA4);
  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);

  //pokemon_paso_param
  ppp->personal_rnd = rec->personal_rnd;
  ppp->pp_fast_mode = 0;  //rec->pp_fast_mode;
  ppp->ppp_fast_mode = 0; //rec->ppp_fast_mode;
  ppp->fusei_tamago_flag = rec->fusei_tamago_flag;
  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);
  //ppp1
  ppp1->monsno = rec->monsno;
  ppp1->item = rec->item;
  ppp1->id_no = rec->id_no;
  ppp1->exp = rec->exp;
  ppp1->friend = rec->friend;
  ppp1->speabino = rec->speabino;
  ppp1->hp_exp = rec->hp_exp;
  ppp1->pow_exp = rec->pow_exp;
  ppp1->def_exp = rec->def_exp;
  ppp1->agi_exp = rec->agi_exp;
  ppp1->spepow_exp = rec->spepow_exp;
  ppp1->spedef_exp = rec->spedef_exp;
  ppp1->country_code = rec->language;
  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);
  //ppp2
  for(i = 0; i < PTL_WAZA_MAX; i++){
    ppp2->waza[i] = rec->waza[i];
    ppp2->pp[i] = rec->pp[i];
    ppp2->pp_count[i] = rec->pp_count[i];
  }
  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);

  ppp2->hp_rnd = rec->hp_rnd;
  ppp2->pow_rnd = rec->pow_rnd;
  ppp2->def_rnd = rec->def_rnd;
  ppp2->agi_rnd = rec->agi_rnd;
  ppp2->spepow_rnd = rec->spepow_rnd;
  ppp2->spedef_rnd = rec->spedef_rnd;
  ppp2->tamago_flag = rec->tamago_flag;
  ppp2->nickname_flag = rec->nickname_flag;
  ppp2->event_get_flag = rec->event_get_flag;
  ppp2->sex = rec->sex;
  ppp2->form_no = rec->form_no;
  ppp2->seikaku = rec->seikaku;
  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);

  //ppp3
  for(i = 0; i < MONS_NAME_SIZE+EOM_SIZE; i++){
    ppp3->nickname[i] = rec->nickname[i];
  }
  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);

  //ppp4
  for(i = 0; i < PERSON_NAME_SIZE+EOM_SIZE; i++){
    ppp4->oyaname[i] = rec->oyaname[i];
  }
  ppp4->get_ball    = rec->get_ball;
  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);

  //calc
  pp->pcp.condition = rec->condition;
  pp->pcp.level = rec->level;
  pp->pcp.cb_id = rec->cb_id;
  pp->pcp.hp = rec->hp;
  pp->pcp.hpmax = rec->hpmax;
  pp->pcp.pow = rec->pow;
  pp->pcp.def = rec->def;
  pp->pcp.agi = rec->agi;
  pp->pcp.spepow = rec->spepow;
  pp->pcp.spedef = rec->spedef;
//  pp->pcp.cb_core = rec->cb_core;
  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);


  //暗号化
  POKETOOL_encode_data(&pp->pcp,sizeof(POKEMON_CALC_PARAM),pp->ppp.personal_rnd);
  pp->ppp.checksum=POKETOOL_make_checksum(&pp->ppp.paradata,sizeof(POKEMON_PASO_PARAM1)*4);
  POKETOOL_encode_data(&pp->ppp.paradata,sizeof(POKEMON_PASO_PARAM1)*4,pp->ppp.checksum);
  TAYA_Printf("RPPtoPP : line=%d\n", __LINE__);

}
