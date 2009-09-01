//============================================================================================
/**
 * @file	poke_regulation.c
 * @bfief	レギュレーションに適合するかのツール
 * @author	k.ohno
 * @date	06.05.25
 */
//============================================================================================

#include "common.h"
#include "poketool/poke_regulation.h"
#include "poketool/pokeparty.h"
#include "itemtool/itemsym.h"

#include "poketool/monsno.h"
#include "savedata/regulation_data.h"

#define _POKENO_NONE  (0)          // ポケモン番号でない番号


//------------------------------------------------------------------
/**
 * ポケモンがレギュレーションに適合しているかどうか調べる
 * @param   POKEMON_PARAM
 * @param   REGULATION
 * @return  合致したらTRUE
 */
//------------------------------------------------------------------

BOOL PokeRegulationCheckPokePara(const REGULATION* pReg, POKEMON_PARAM* pp)
{
  u16 mons = (u16)PokeParaGet( pp, ID_PARA_monsno, NULL );
  u16 item = (u16)PokeParaGet( pp, ID_PARA_item, NULL );
  int ans,level,weight;

  if(pReg==NULL){
    return TRUE;
  }
  //LV
  ans = Regulation_GetParam(pReg, REGULATION_LEVEL);
  range = Regulation_GetParam(pReg, REGULATION_LEVEL_RANGE);
  level = PokeParaGet(pp, ID_PARA_level, NULL);
  // レベル制限がある場合検査
  switch(range){
  case REGULATION_LEVEL_RANGE_OVER:
    if(ans > level){
      return FALSE;
    }
    break;
  case REGULATION_LEVEL_RANGE_LESS:
    if(ans < level){
      return FALSE;
    }
    break;
  }
  //たまご参戦不可
  if( PokeParaGet(pp, ID_PARA_tamago_flag, NULL ) != 0 ){
    return FALSE;
  }
  // 参加禁止ポケかどうか
  if(Regulation_CheckParamBit(pReg, REGULATION_VETO_POKE_BIT, mons)){
    return FALSE;
  }
  //持ち込み不可アイテムかどうか
  if(Regulation_CheckParamBit(pReg, REGULATION_VETO_ITEM, item)){
    return FALSE;
  }
  return TRUE;
}

//------------------------------------------------------------------
/**
 * ポケパーティがレギュレーションに適合しているかどうか調べる
 * @param   POKEMON_PARAM
 * @param   REGULATION
 * @param   partyを選択した配列  0以外が選択している
 * @return
 */
//------------------------------------------------------------------

int PokeRegulationMatchFullPokeParty(const REGULATION* pReg, POKEPARTY * party, u8* sel)
{
  POKEMON_PARAM* pp;
  int ans,cnt = 0,j,i,level = 0,form=0;
  u16 monsTbl[6],itemTbl[6],formTbl[6];

  if(pReg==NULL){
    return POKE_REG_OK;
  }
  for(i = 0; i < 6 ;i++){
    monsTbl[i] = _POKENO_NONE;
    formTbl[i] = _POKENO_NONE;
    itemTbl[i] = ITEM_DUMMY_DATA;
    if(sel[i]){
      cnt++;
    }
  }

  //全体数
  ans = Regulation_GetParam(pReg, REGULATION_NUM_LO);
  if(cnt < ans){
    return POKE_REG_NUM_FAILED;  // 数があってない
  }
  ans = Regulation_GetParam(pReg, REGULATION_NUM_HI);
  if(cnt > ans){
    return POKE_REG_NUM_FAILED;  // 数があってない
  }
  for(i = 0; i < 6 ;i++){
    if(sel[i]){
      int pid = sel[i] - 1;
      pp = PokeParty_GetMemberPointer(party, pid);
      if(PokeRegulationCheckPokePara(pReg, pp) == FALSE){
        return POKE_REG_ILLEGAL_POKE; // 個体が引っかかった
      }
      monsTbl[i] = (u16)PokeParaGet( pp, ID_PARA_monsno, NULL );
      itemTbl[i] = (u16)PokeParaGet( pp, ID_PARA_item, NULL );
      formTbl[i] = (u16)PokeParaGet( pp, ID_PARA_form_no, NULL );
      level += PokeParaGet(pp,ID_PARA_level,NULL);
    }
  }
  //合計LV
  ans = Regulation_GetParam(pReg, REGULATION_LEVEL_TOTAL);
  if((level > ans) && (ans != 0)){
    return POKE_REG_TOTAL_LV_FAILED;
  }
  // 同じポケモン
  ans = Regulation_GetParam(pReg, REGULATION_BOTH_POKE);
  if((ans == FALSE) && (cnt > 1)){  // 同じポケモンはだめで 一体以上の場合
    for(i = 0; i < (6-1); i++){
      for(j = i + 1;j < 6; j++){
        if((monsTbl[i] == monsTbl[j]) && (monsTbl[i] != _POKENO_NONE)){
          return POKE_REG_BOTH_POKE;
        }
      }
    }
  }
  // 同じアイテム
  ans = Regulation_GetParam(pReg, REGULATION_BOTH_ITEM);
  if((ans == FALSE) && (cnt > 1)){  // 同じアイテムはだめで 一体以上の場合
    for(i = 0; i < (6-1); i++){
      for(j = i + 1;j < 6; j++){
        if((itemTbl[i] == itemTbl[j]) && (monsTbl[i] != _POKENO_NONE) && (ITEM_DUMMY_DATA != itemTbl[i])){
          return POKE_REG_BOTH_ITEM;
        }
      }
    }
  }
  ans = Regulation_GetParam(pReg, REGULATION_MUST_POKE);
  form = Regulation_GetParam(pReg, REGULATION_MUST_POKE_FORM);
  if((ans) && (cnt > 1)){ //必須ポケモンがいる
    for(i = 0; i < 6; i++){
      if(monsTbl[i] == ans){
        if(formTbl[i] == form){
          break;
        }
      }
    }
  }
  if(i==6){
    return POKE_REG_NO_MASTPOKE;
  }

  return POKE_REG_OK;
}

//------------------------------------------------------------------
/**
 * 再帰的にLVを引き算して目的に達したらTRUE
 * @param   モンスター番号のテーブル
 * @param   レベルが入ってるのテーブル
 * @param   このポケモン検査した場合のMARK
 * @param   totalPokeLv   残りのLV
 * @param   nowPokeIndex  今から計算するポケモン
 * @param   partyNum     CUP参加の残りポケモン数
 * @param   ポケパーティー数
 * @return  合致したらTRUE
 */
//------------------------------------------------------------------

static BOOL _totalLevelCheck(u16* pMonsTbl,u16* pLevelTbl,u16* pMarkTbl,
                             int totalPokeLv,int nowPokeIndex, int partyNum, int cnt)
{
  int total = totalPokeLv;
  int i;
  int party = partyNum;

  if((pMonsTbl[nowPokeIndex] != _POKENO_NONE) && (pMarkTbl[nowPokeIndex] == 0)){
    total -= pLevelTbl[nowPokeIndex];
    if(total < 0){
      return FALSE;
    }
    party--;
    if(party == 0){
      return TRUE;
    }
    pMarkTbl[nowPokeIndex] = 1;  // マークつける
  }
  for(i = nowPokeIndex+1;i < cnt;i++){  // 再帰検査中
    if(_totalLevelCheck(pMonsTbl,pLevelTbl,pMarkTbl, total, i, party, cnt)){
      return TRUE;
    }
  }
  pMarkTbl[nowPokeIndex] = 0;  // マークはずす
  return FALSE;
}

//------------------------------------------------------------------
/**
 * ポケパーティ中にレギュレーションに適合している
 *   パーティーが組めるかどうか調べる 適応外のポケモンがいても大丈夫
 *   手持ちアイテムは検査していない
 * @param   POKEMON_PARAM
 * @param   REGULATION
 * @param   ZKN_HEIGHT_GRAM_PTR
 * @return  _POKE_REG_NUM_FAILED か _POKE_REG_TOTAL_FAILED か _POKE_REG_OK
 */
//------------------------------------------------------------------

int PokeRegulationMatchPartialPokeParty(const REGULATION* pReg, POKEPARTY * party)
{
  POKEMON_PARAM* pp;
  int ans,cnt,j,i,level = 0;
  u16 monsTbl[6],levelTbl[6],markTbl[6];
  int partyNum;

  cnt = PokeParty_GetPokeCount(party);
  partyNum = cnt;

  MI_CpuClear8(markTbl,6*sizeof(u16));

  for(i = 0; i < cnt ;i++){
    pp = PokeParty_GetMemberPointer(party, i);
    monsTbl[i] = (u16)PokeParaGet( pp, ID_PARA_monsno, NULL );
    levelTbl[i] = PokeParaGet(pp,ID_PARA_level,NULL);
    if(PokeRegulationCheckPokePara(pReg, pp ) == FALSE){
      monsTbl[i] = _POKENO_NONE; // 固体が引っかかったので消す
      partyNum--;
    }
  }
  // 同じポケモンがだめな場合
  ans = Regulation_GetParam(pReg, REGULATION_BOTH_POKE);
  if((ans == 0) && (cnt > 1)){  // 同じポケモンはLVの低い方を残す
    for(i = 0; i < (cnt-1); i++){
      for(j = i + 1;j < cnt; j++){
        if((monsTbl[i] == monsTbl[j]) && (monsTbl[i] != _POKENO_NONE)){
          if(levelTbl[i] > levelTbl[j]){
            monsTbl[i] = _POKENO_NONE; // 同じものを消去
          }
          else{
            monsTbl[j] = _POKENO_NONE; // 同じものを消去
          }
          partyNum--;
        }
      }
    }
  }

  //全体数
  ans = Regulation_GetParam(pReg, REGULATION_POKE_NUM);
  if(partyNum < ans){
    return POKE_REG_NUM_FAILED;  // 全体数が足りない
  }
  partyNum = ans;

  //残ったポケモンの合計LV組み合わせ検査
  ans = Regulation_GetParam(pReg, REGULATION_TOTAL_LEVEL);
  if(ans == 0){
    return POKE_REG_OK;  // LV制限なし
  }
  for(i = 0;i < cnt;i++){
    if(_totalLevelCheck(monsTbl,levelTbl,markTbl, ans, i, partyNum, cnt)){
      return POKE_REG_OK;
    }
  }
  return POKE_REG_TOTAL_LV_FAILED;
}


