//============================================================================================
/**
 * @file	poke_regulation.c
 * @bfief	レギュレーションに適合するかのツール
 * @author	k.ohno
 * @date	06.05.25
 */
//============================================================================================

#include <gflib.h>
#include "buflen.h"
#include "savedata/save_control.h"
#include "poke_tool/poke_regulation.h"
#include "poke_tool/pokeparty.h"
#include "item/itemsym.h"

#include "poke_tool/monsno_def.h"

#include "arc_def.h"
#include "regulation.naix"

#include "pm_define.h"
#include "msg/msg_regulation.h"
#include "message.naix"

#define _POKENO_NONE  (0)          // ポケモン番号でない番号


//------------------------------------------------------------------
/**
 * @brief  ポケモンがレギュレーションに適合しているかどうか調べる
 * @param   REGULATION     レギュレーション構造体ポインタ
 * @param   POKEMON_PARAM  ポケパラ
 * @return  合致したらTRUE
 */
//------------------------------------------------------------------

BOOL PokeRegulationCheckPokePara(const REGULATION* pReg, POKEMON_PARAM* pp)
{
  u16 mons = (u16)PP_Get( pp, ID_PARA_monsno, NULL );
  u16 item = (u16)PP_Get( pp, ID_PARA_item, NULL );
  int ans,level,weight,range;

  if(pReg==NULL){
    return TRUE;
  }
  //LV
  ans = Regulation_GetParam(pReg, REGULATION_LEVEL);
  range = Regulation_GetParam(pReg, REGULATION_LEVEL_RANGE);
  level = PP_Get(pp, ID_PARA_level, NULL);
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
  if( PP_Get(pp, ID_PARA_tamago_flag, NULL ) != 0 ){
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
 * @brief  ポケパーティがレギュレーションに適合しているかどうか調べる
 * @param   REGULATION     レギュレーション構造体ポインタ
 * @param   POKEMON_PARAM
 * @param   partyを選択した配列  0以外が選択している
 * @return  POKE_REG_RETURN_ENUM
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
  for(i = 0; i < PokeParty_GetPokeCount(party) ;i++){
    if(sel[i]){
      int pid = sel[i] - 1;
      pp = PokeParty_GetMemberPointer(party, pid);
      if(PokeRegulationCheckPokePara(pReg, pp) == FALSE){
        return POKE_REG_ILLEGAL_POKE; // 個体が引っかかった
      }
      monsTbl[i] = (u16)PP_Get( pp, ID_PARA_monsno, NULL );
      itemTbl[i] = (u16)PP_Get( pp, ID_PARA_item, NULL );
      formTbl[i] = (u16)PP_Get( pp, ID_PARA_form_no, NULL );
      level += PP_Get(pp,ID_PARA_level,NULL);
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
 * @brief  再帰的にLVを引き算して目的に達したらTRUE
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
 * @brief ポケパーティ中にレギュレーションに適合している
 *   パーティーが組めるかどうか調べる 適応外のポケモンがいても大丈夫
 *   手持ちアイテムは検査していない
 * @param   POKEMON_PARAM
 * @param   REGULATION
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
    monsTbl[i] = (u16)PP_Get( pp, ID_PARA_monsno, NULL );
    levelTbl[i] = PP_Get(pp,ID_PARA_level,NULL);
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
  ans = Regulation_GetParam(pReg, REGULATION_NUM_LO);
  if(partyNum < ans){
    return POKE_REG_NUM_FAILED;  // 数があってない
  }
  ans = Regulation_GetParam(pReg, REGULATION_NUM_HI);
  if(partyNum > ans){
    return POKE_REG_NUM_FAILED;  // 数があってない
  }
  //partyNum = ans;

  //残ったポケモンの合計LV組み合わせ検査
  ans = Regulation_GetParam(pReg, REGULATION_LEVEL_TOTAL);
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




//------------------------------------------------------------------
/**
 * @brief  ポケモンがレギュレーションに適合しているかどうか調べる シャチ版=見せ合い対応
 * @param   REGULATION     レギュレーション構造体ポインタ
 * @param   POKEMON_PARAM  ポケパラ
 * @param   FailedBit      条件違反をビットで返す
 * @return  合致したらTRUE
 */
//------------------------------------------------------------------

BOOL PokeRegulationCheckPokeParaLookAt(const REGULATION* pReg, POKEMON_PARAM* pp,u32* FailedBit)
{
  u16 mons = (u16)PP_Get( pp, ID_PARA_monsno, NULL );
  u16 item = (u16)PP_Get( pp, ID_PARA_item, NULL );
  int ans,level,weight,range;
  BOOL ret=TRUE;

  if(pReg==NULL){
    return ret;
  }
  //LV
  ans = Regulation_GetParam(pReg, REGULATION_LEVEL);
  range = Regulation_GetParam(pReg, REGULATION_LEVEL_RANGE);
  level = PP_Get(pp, ID_PARA_level, NULL);
  // レベル制限がある場合検査
  switch(range){
  case REGULATION_LEVEL_RANGE_OVER:
    if(ans > level){
      ret = FALSE;
      *FailedBit |= POKEFAILEDBIT_LEVEL;
    }
    break;
  case REGULATION_LEVEL_RANGE_LESS:
    if(ans < level){
      ret = FALSE;
      *FailedBit |= POKEFAILEDBIT_LEVEL;
    }
    break;
  }
#if 0  //タマゴは手持ちにいても良いが　人数には数えない
  //たまご参戦不可
  if( PP_Get(pp, ID_PARA_tamago_flag, NULL ) != 0 ){
    ret = FALSE;
    *FailedBit |= POKEFAILEDBIT_EGG;
  }
#endif
  // 参加禁止ポケかどうか
  if(Regulation_CheckParamBit(pReg, REGULATION_VETO_POKE_BIT, mons)){
    ret = FALSE;
    *FailedBit |= POKEFAILEDBIT_VETO_POKE;
  }
  //持ち込み不可アイテムかどうか
  if(Regulation_CheckParamBit(pReg, REGULATION_VETO_ITEM, item)){
    ret = FALSE;
    *FailedBit |= POKEFAILEDBIT_VETO_ITEM;
  }
  return ret;
}


//------------------------------------------------------------------
/**
 * @brief   ポケパーティがレギュレーションに適合しているかどうか調べる シャチ版=見せ合い対応
            条件にあっていないものは数にかかわらずすべてはじく
            Lv合計は調べていない
 * @param   REGULATION     レギュレーション構造体ポインタ
 * @param   POKEPARTY
 * @param   FailedBit      条件違反をビットで返す
 * @return  POKE_REG_RETURN_ENUM
 */
//------------------------------------------------------------------

int PokeRegulationMatchLookAtPokeParty(const REGULATION* pReg, POKEPARTY * party, u32* FailedBit)
{
  POKEMON_PARAM* pp;
  int ans,cnt = 0,j,i,level = 0,form=0;
  u16 monsTbl[6],itemTbl[6],formTbl[6];
  int ret = POKE_REG_OK;

  if(pReg==NULL){
    return POKE_REG_OK;
  }
  GF_ASSERT(FailedBit);  //if文でいっぱいになるので呼び出し側で必ずセットする事
  for(i = 0; i < 6 ;i++){
    monsTbl[i] = _POKENO_NONE;
    formTbl[i] = _POKENO_NONE;
    itemTbl[i] = ITEM_DUMMY_DATA;
  }

  //全体数
  cnt = PokeParty_GetPokeCountBattleEnable(party);

  ans = Regulation_GetParam(pReg, REGULATION_NUM_LO);
  if(cnt < ans){
    *FailedBit |= POKEFAILEDBIT_NUM;
    ret = POKE_REG_NUM_FAILED;  // 数があってない
  }
  for(i = 0; i < PokeParty_GetPokeCount(party) ;i++){
    pp = PokeParty_GetMemberPointer(party, i);
    if(PokeRegulationCheckPokeParaLookAt(pReg, pp, FailedBit ) == FALSE){
      ret = POKE_REG_ILLEGAL_POKE; // 個体が引っかかった
    }
    monsTbl[i] = (u16)PP_Get( pp, ID_PARA_monsno, NULL );
    itemTbl[i] = (u16)PP_Get( pp, ID_PARA_item, NULL );
    formTbl[i] = (u16)PP_Get( pp, ID_PARA_form_no, NULL );
    level += PP_Get(pp,ID_PARA_level,NULL);
  }
  
  // 同じポケモン
  ans = Regulation_GetParam(pReg, REGULATION_BOTH_POKE);
  if((ans == FALSE) && (cnt > 1)){  // 同じポケモンはだめで 一体以上の場合
    for(i = 0; i < (6-1); i++){
      for(j = i + 1;j < 6; j++){
        if((monsTbl[i] == monsTbl[j]) && (monsTbl[i] != _POKENO_NONE)){
          *FailedBit |= POKEFAILEDBIT_BOTHPOKE;
          ret = POKE_REG_BOTH_POKE;
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
          *FailedBit |= POKEFAILEDBIT_BOTHITEM;
          ret = POKE_REG_BOTH_ITEM;
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
    *FailedBit |= POKEFAILEDBIT_MAST_POKE;
    ret = POKE_REG_NO_MASTPOKE;
  }
  return ret;
}



//------------------------------------------------------------------
/**
 * @brief   ROMからレギュレーションデータを得る
 * @param   regulation_data_no   regulation_def.hの定義で呼び出してください
 * @param   heap  読み込み用領域
 * @return  レギュレーションデータポインタ  GFL_HEAP_FreeMemoryでメモリ開放をしてください
 */
//------------------------------------------------------------------
const REGULATION* PokeRegulation_LoadDataAlloc(int regulation_data_no, HEAPID heapid)
{
  REGULATION* pRegData = GFL_ARC_LoadDataAlloc(ARCID_REGULATION, regulation_data_no, heapid);
  return pRegData;
}

//------------------------------------------------------------------
/**
 * @brief   ROMからレギュレーションデータを得る(外側から代入ワークを渡す)
 * @param   regulation_data_no   regulation_def.hの定義で呼び出してください
 * @param   heap  読み込み用領域
 * @return  レギュレーションデータポインタ  GFL_HEAP_FreeMemoryでメモリ開放をしてください
 */
//------------------------------------------------------------------
void PokeRegulation_LoadData(int regulation_data_no, REGULATION *reg)
{
  GFL_ARC_LoadData(reg, ARCID_REGULATION, regulation_data_no);
}


//==============================================================================
//
//  メッセージ取得
//
//==============================================================================
//--------------------------------------------------------------
/**
 * メッセージ作成：ポケモンの数
 */
//--------------------------------------------------------------
static STRBUF * _RegMsg_CreateNum(const REGULATION* pReg, WORDSET *wordset, REGULATION_PRINT_MSG *rpm, STRBUF *tempbuf, HEAPID heap_id)
{
  STRBUF *strbuf;
  u16 msg_id;
  
  if(pReg->NUM_LO == 1 && pReg->NUM_HI == TEMOTI_POKEMAX){
    msg_id = msg_reg_num_000; //制限無し
  }
  else if(pReg->NUM_LO == pReg->NUM_HI){
    msg_id = msg_reg_num_001; //人数固定
  }
  else if(pReg->NUM_LO > 1 && pReg->NUM_HI == TEMOTI_POKEMAX){
    msg_id = msg_reg_num_002; //xxx匹以上
  }
  else{
    msg_id = msg_reg_num_003; // xxx ～ xxx 匹
  }
  
  WORDSET_RegisterNumber( wordset, 0, pReg->NUM_LO, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wordset, 1, pReg->NUM_HI, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

  strbuf = GFL_STR_CreateBuffer(64, heap_id);
  GFL_MSG_GetString( rpm->msgdata, msg_id, tempbuf );

  WORDSET_ExpandStr( wordset, strbuf, tempbuf );
  return strbuf;
}

//--------------------------------------------------------------
/**
 * メッセージ作成：ポケモンのレベル
 */
//--------------------------------------------------------------
static STRBUF * _RegMsg_CreateLv(const REGULATION* pReg, WORDSET *wordset, REGULATION_PRINT_MSG *rpm, STRBUF *tempbuf, HEAPID heap_id)
{
  STRBUF *strbuf;
  u16 msg_id;
  
  if(pReg->LEVEL_RANGE == REGULATION_LEVEL_RANGE_NORMAL){
    msg_id = msg_reg_num_000; //制限なし
  }
  else if(pReg->LEVEL_RANGE == REGULATION_LEVEL_RANGE_SAME){
    msg_id = msg_reg_lv_001;  //全補正
  }
  else if(pReg->LEVEL_RANGE == REGULATION_LEVEL_RANGE_OVER){
    msg_id = msg_reg_lv_003;  //以上
  }
  else if(pReg->LEVEL_RANGE == REGULATION_LEVEL_RANGE_LESS){
    msg_id = msg_reg_lv_002;  //以下
  }
  else if(pReg->LEVEL_RANGE == REGULATION_LEVEL_RANGE_DRAG_DOWN){
    msg_id = msg_reg_lv_004;  //引き下げ
  }
  else if(pReg->LEVEL_RANGE == REGULATION_LEVEL_RANGE_PULL_UP){
    msg_id = msg_reg_lv_005;  //引き上げ
  }
  else{
    GF_ASSERT(0);
    msg_id = msg_reg_num_000; //制限なし
  }
  WORDSET_RegisterNumber( wordset, 0, pReg->LEVEL, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

  strbuf = GFL_STR_CreateBuffer(64, heap_id);
  GFL_MSG_GetString( rpm->msgdata, msg_id, tempbuf );

  WORDSET_ExpandStr( wordset, strbuf, tempbuf );
  return strbuf;
}

//--------------------------------------------------------------
/**
 * メッセージ作成：特別なポケモン
 */
//--------------------------------------------------------------
static STRBUF * _RegMsg_CreateSpPoke(const REGULATION* pReg, WORDSET *wordset, REGULATION_PRINT_MSG *rpm, STRBUF *tempbuf, HEAPID heap_id)
{
  STRBUF *strbuf;
  u16 msg_id;
  int i;
  
  for(i = 0; i < REG_POKENUM_MAX_BYTE; i++){
    if(pReg->VETO_POKE_BIT[i] != 0){
      break;
    }
  }
  if(i == REG_POKENUM_MAX_BYTE){
    msg_id = msg_reg_sp_000;
  }
  else{
    msg_id = msg_reg_sp_001;
  }
  
  strbuf = GFL_MSG_CreateString( rpm->msgdata, msg_id );
  return strbuf;
}

//--------------------------------------------------------------
/**
 * メッセージ作成：同じポケモン
 */
//--------------------------------------------------------------
static STRBUF * _RegMsg_CreateSamePoke(const REGULATION* pReg, WORDSET *wordset, REGULATION_PRINT_MSG *rpm, STRBUF *tempbuf, HEAPID heap_id)
{
  STRBUF *strbuf;
  u16 msg_id;
  
  if(pReg->BOTH_POKE == TRUE){
    msg_id = msg_reg_samepoke_000;
  }
  else{
    msg_id = msg_reg_samepoke_001;
  }

  strbuf = GFL_MSG_CreateString( rpm->msgdata, msg_id );
  return strbuf;
}

//--------------------------------------------------------------
/**
 * メッセージ作成：同じ道具
 */
//--------------------------------------------------------------
static STRBUF * _RegMsg_CreateSameItem(const REGULATION* pReg, WORDSET *wordset, REGULATION_PRINT_MSG *rpm, STRBUF *tempbuf, HEAPID heap_id)
{
  STRBUF *strbuf;
  u16 msg_id;
  
  if(pReg->BOTH_ITEM == TRUE){
    msg_id = msg_reg_sameitem_000;
  }
  else{
    msg_id = msg_reg_sameitem_001;
  }

  strbuf = GFL_MSG_CreateString( rpm->msgdata, msg_id );
  return strbuf;
}

//--------------------------------------------------------------
/**
 * メッセージ作成：シューターの有無
 */
//--------------------------------------------------------------
static STRBUF * _RegMsg_CreateShooter(const REGULATION* pReg, WORDSET *wordset, REGULATION_PRINT_MSG *rpm, STRBUF *tempbuf, HEAPID heap_id)
{
  STRBUF *strbuf;
  u16 msg_id;
  
  if(rpm->shooter_type == REGULATION_SHOOTER_VALID){
    msg_id = msg_reg_shooter_000;
  }
  else if(rpm->shooter_type == REGULATION_SHOOTER_INVALID){
    msg_id = msg_reg_shooter_001;
  }
  else{
    GF_ASSERT(0);
    msg_id = msg_reg_shooter_000;
  }

  strbuf = GFL_MSG_CreateString( rpm->msgdata, msg_id );
  return strbuf;
}

//==================================================================
/**
 * レギュレーション一覧メニュー表示用メッセージのパッケージを作成する
 *
 * @param   pReg		
 * @param   wordset		
 * @param   heap_id		
 * @param   shooter_manual		REGULATION_SHOOTER_TYPE(レギュレーションがMANUAL以外の時は無視します)
 *
 * @retval  REGULATION_PRINT_MSG *		
 */
//==================================================================
REGULATION_PRINT_MSG * PokeRegulation_CreatePrintMsg(const REGULATION* pReg, WORDSET *wordset, HEAPID heap_id, int shooter_type)
{
  REGULATION_PRINT_MSG *rpm = GFL_HEAP_AllocClearMemory(heap_id, sizeof(REGULATION_PRINT_MSG));
  STRBUF *tempbuf;
  int category;
  static STRBUF * (* const PrerequisiteFunc[])(const REGULATION *, WORDSET *, REGULATION_PRINT_MSG *, STRBUF *, HEAPID) = {
    _RegMsg_CreateNum,
    _RegMsg_CreateLv,
    _RegMsg_CreateSpPoke,
    _RegMsg_CreateSamePoke,
    _RegMsg_CreateSameItem,
    _RegMsg_CreateShooter,
  };
  SDK_COMPILER_ASSERT(NELEMS(PrerequisiteFunc) == REGULATION_CATEGORY_MAX);

  rpm->msgdata = GFL_MSG_Create( 
    GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_regulation_dat, heap_id );
  
  tempbuf = GFL_STR_CreateBuffer(64, heap_id);
  
  if(pReg->SHOOTER == REGULATION_SHOOTER_MANUAL){
    rpm->shooter_type = shooter_type;
  }
  else{
    rpm->shooter_type = pReg->SHOOTER;
  }
  
  for(category = 0; category < REGULATION_CATEGORY_MAX; category++){
    rpm->category[category] = GFL_MSG_CreateString( rpm->msgdata, msg_reg_000 + category );
    rpm->prerequisite[category] = PrerequisiteFunc[category](pReg, wordset, rpm, tempbuf, heap_id);
  }
  
  GFL_STR_DeleteBuffer(tempbuf);
  
  return rpm;
}

//==================================================================
/**
 * レギュレーション一覧メニュー表示用メッセージのパッケージを削除する
 *
 * @param   rpm		
 */
//==================================================================
void PokeRegulation_DeletePrintMsg(REGULATION_PRINT_MSG *rpm)
{
  int category;
  
  for(category = 0; category < REGULATION_CATEGORY_MAX; category++){
    GFL_STR_DeleteBuffer(rpm->category[category]);
    GFL_STR_DeleteBuffer(rpm->prerequisite[category]);
  }
  GFL_MSG_Delete(rpm->msgdata);
  GFL_HEAP_FreeMemory(rpm);
}
