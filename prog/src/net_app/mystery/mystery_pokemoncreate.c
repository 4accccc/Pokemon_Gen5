//============================================================================================
/**
 * @file    mystery_pokemoncreate.c
 * @bfief   不思議通信データからポケモンを作る関数
 * @author  k.ohno
 * @date    10.01.27
 */
//============================================================================================


#include <gflib.h>
#include "gamesystem/game_data.h"

#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"
#include "waza_tool/wazano_def.h"
#include "savedata/mystery_data.h"

#include "net_app/mystery.h"
#include "system/gfl_use.h"



POKEMON_PARAM* MYSTERY_PokemonCreate(const GIFT_PRESENT_POKEMON* pGift, HEAPID heapID, GAMEDATA* pGameData)
{
  POKEMON_PARAM* pp;
  u16 level=pGift->level;
  u32 id = pGift->id_no;
  u32 monsno = pGift->mons_no;
  PtlSetupPow pow=0;
  u64 rand;
  u32 buff;
  u8 tokusei = pGift->speabino & 0x7f;
  BOOL tokuseiBit = (pGift->speabino & 0x80) ? TRUE : FALSE;

  
  if(pGift->mons_no > MONSNO_END){  //モンスター番号が存在しない物は作らない
    return NULL;
  }
  if(pGift->mons_no == 0){  //モンスター番号が存在しない物は作らない
    return NULL;
  }
  if(pGift->level > 100){  //レベル100より上は作らない
    return NULL;
  }
  if(pGift->item > ITEM_DATA_MAX){  //アイテム番号間違い
    return NULL;
  }

  
  
  if(pGift->level == 0){  //レベル０指定は乱数
    level = GFUser_GetPublicRand(99)+1;
  }
  if(pGift->id_no == 0){  //ID０指定は自分が親
    id = MyStatus_GetID(GAMEDATA_GetMyStatus(pGameData));
  }



  pp = PP_Create( monsno, level, id, heapID );

  pow = PTL_SETUP_POW_PACK(pGift->hp_rnd,pGift->pow_rnd,
                           pGift->def_rnd,
                           pGift->spepow_rnd,pGift->spedef_rnd,pGift->agi_rnd);


  PP_Put(pp, ID_PARA_speabino, pGift->speabino);  

  if(pow==0){
    pow = PTL_SETUP_POW_AUTO;
  }
  if(pGift->rnd != 0){
    rand = pGift->rnd;
  }
  else if(pGift->rare == 0){
    rand = POKETOOL_CalcPersonalRandEx( id, monsno, pGift->form_no, pGift->sex, tokuseiBit, FALSE);
  }
  else if(pGift->rare == 1){
    rand = PTL_SETUP_ID_AUTO;
  }
  else if(pGift->rare == 2){
    rand = POKETOOL_CalcPersonalRandEx( id, monsno, pGift->form_no, pGift->sex, tokuseiBit, TRUE);
  }

  PP_SetupEx( pp, pGift->mons_no, level, id, pow, rand );

  PP_Put(pp, ID_PARA_item, pGift->item);

  if(pGift->waza1!=0 && pGift->waza1<WAZANO_MAX){
    PP_SetWazaPush(pp, pGift->waza1);
  }
  if(pGift->waza2!=0 && pGift->waza2<WAZANO_MAX){
    PP_SetWazaPush(pp, pGift->waza2);
  }
  if(pGift->waza3!=0 && pGift->waza3<WAZANO_MAX){
    PP_SetWazaPush(pp, pGift->waza3);
  }
  if(pGift->waza4!=0 && pGift->waza4<WAZANO_MAX){
    PP_SetWazaPush(pp, pGift->waza4);
  }
  PP_Put(pp, ID_PARA_form_no, pGift->form_no);  //フォルム番号はすべて許す
  
  if(pGift->get_ball!=0){
    PP_Put(pp, ID_PARA_get_ball, pGift->get_ball);
  }
  if(pGift->get_level!=0){
    PP_Put(pp, ID_PARA_get_level, pGift->get_level);  
  }

  PP_Put(pp, ID_PARA_style, pGift->style);  
  PP_Put(pp, ID_PARA_beautiful, pGift->beautiful);  
  PP_Put(pp, ID_PARA_cute, pGift->cute);  
  PP_Put(pp, ID_PARA_clever, pGift->clever);  
  PP_Put(pp, ID_PARA_strong, pGift->strong);  
  PP_Put(pp, ID_PARA_fur, pGift->strong);  

//  u16 ribbon_no;        //リボンビット 16本  @todo 下山田さん待ち

  if(pGift->version!=0){
    PP_Put(pp, ID_PARA_get_cassette, pGift->version);
  }
    
  PP_Put(pp, ID_PARA_country_code, pGift->country_code);  

  if(pGift->nickname[0]!= GFL_STR_GetEOMCode()){
    PP_Put(pp, ID_PARA_nickname_raw, (u32)pGift->nickname);
  }
  

  PP_Put(pp, ID_PARA_seikaku, pGift->seikaku);  
  PP_Put(pp, ID_PARA_get_place, pGift->get_place);  
  PP_Put(pp, ID_PARA_birth_place, pGift->birth_place);  

  if(pGift->nickname[0]!= GFL_STR_GetEOMCode()){
    PP_Put(pp, ID_PARA_oyaname_raw, (u32)pGift->oyaname);
  }
  if(pGift->oyasex <= PM_NEUTRAL){
    PP_Put(pp, ID_PARA_oyasex, pGift->oyasex);
  }
  else{
    PP_Put(pp, ID_PARA_oyasex, MyStatus_GetMySex(GAMEDATA_GetMyStatus(pGameData)));
  }

  if(pGift->egg){       //タマゴかどうか TRUE＝たまご
    PP_Put(pp, ID_PARA_tamago_flag, TRUE);
  }

	PP_Renew( pp );

  if(PP_Get(pp,ID_PARA_fusei_tamago_flag, NULL)){  //不正ポケモンが出来てしまった場合
    GFL_HEAP_FreeMemory(pp);
    return NULL;
  }
  return pp;
}



