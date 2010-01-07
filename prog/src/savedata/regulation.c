//============================================================================================
/**
 * @file	regulation.c
 * @brief	バトルレギュレーションデータアクセス用ソース
 * @author	k.ohno
 * @date	2006.1.20
 */
//============================================================================================

#include <gflib.h>

#include "arc_def.h"
#include "system/main.h"
#include "message.naix"
#include "savedata/save_tbl.h"	//SAVEDATA参照のため
#include "buflen.h"
#include "savedata/save_control.h"
#include "savedata/regulation.h"
#include "poke_tool/monsno_def.h"

#include "msg/msg_regulation_cup.h"
#include "msg/msg_regulation_rule.h"

#define HEAPID_SAVE_TEMP        (GFL_HEAPID_APP)


//============================================================================================
//============================================================================================

//============================================================================================
//
//	セーブデータシステムが依存する関数
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief	ワークサイズ取得
 * @return	int		サイズ（バイト単位）
 */
//----------------------------------------------------------
int Regulation_GetWorkSize(void)
{
  return sizeof(REGULATION);
}

//----------------------------------------------------------
/**
 * @brief	ワークサイズ取得
 * @return	int		サイズ（バイト単位）
 */
//----------------------------------------------------------
int RegulationData_GetWorkSize(void)
{
  return sizeof(REGULATION_CARDDATA);
}

//----------------------------------------------------------
/**
 * @brief	バトルレギュレーションワークの確保
 * @param	heapID		メモリ確保をおこなうヒープ指定
 * @return	REGULATION*	取得したワークへのポインタ
 */
//----------------------------------------------------------
REGULATION* Regulation_AllocWork(HEAPID heapID)
{
  REGULATION* reg;
  reg = GFL_HEAP_AllocClearMemory(heapID, sizeof(REGULATION));
  return reg;
}

//----------------------------------------------------------
/**
 * @brief	REGULATIONのコピー
 * @param	from	コピー元REGULATIONへのポインタ
 * @param	to		コピー先REGULATIONへのポインタ
 */
//----------------------------------------------------------
void Regulation_Copy(const REGULATION *pFrom, REGULATION* pTo)
{
  GFL_STD_MemCopy(pFrom, pTo, sizeof(REGULATION));
}

//----------------------------------------------------------
/**
 * @brief	REGULATIONの比較
 * @param	cmp1   比較するREGULATIONへのポインタ
 * @param	cmp2   比較されるREGULATIONへのポインタ
 * @return  一致していたらTRUE
 */
//----------------------------------------------------------
int Regulation_Cmp(const REGULATION* pCmp1,const REGULATION* pCmp2)
{
  if(0 == GFL_STD_MemComp(pCmp1,pCmp2,sizeof(REGULATION)))
  {
    return TRUE;
  }
  return FALSE;
}

//============================================================================================
//
//	REGULATION操作のための関数
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief	バトルレギュレーションデータの初期化
 * @param	pReg		REGULATIONワークへのポインタ
 */
//----------------------------------------------------------
void Regulation_Init(REGULATION* pReg)
{
  GFL_STD_MemClear(pReg, sizeof(REGULATION));
}

//----------------------------------------------------------
/**
 * @brief	デジタル選手証レギュレーションデータの初期化
 * @param	pReg		REGULATIONワークへのポインタ
 */
//----------------------------------------------------------
void RegulationData_Init(REGULATION_CARDDATA* pRegData)
{
  GFL_STD_MemClear(pRegData, sizeof(REGULATION_CARDDATA));
}

//----------------------------------------------------------
/**
 * @brief	カップ名セット
 * @param	pReg	REGULATIONワークポインタ
 * @param	pCupBuf	カップ名が入ったバッファ
 */
//----------------------------------------------------------
//void Regulation_SetCupName(REGULATION* pReg, const STRBUF *pCupBuf)
//{
//  GFL_STR_GetStringCode(pCupBuf, pReg->cupName, (REGULATION_CUPNAME_SIZE + EOM_SIZE));
//}

//----------------------------------------------------------
/**
 * @brief	カップ名取得
 * @param	pReg		    REGULATIONワークポインタ
 * @param	pReturnCupName	カップ名を入れるSTRBUFポインタ
 * @return	none
 */
//----------------------------------------------------------
void Regulation_GetCupName(const REGULATION* pReg,STRBUF* pReturnCupName)
{
 // GFL_STR_SetStringCodeOrderLength(pReturnCupName, pReg->cupName, (REGULATION_CUPNAME_SIZE + EOM_SIZE));
  GFL_MSGDATA* pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
                                          NARC_message_regulation_cup_dat, HEAPID_SAVE_TEMP );
  GFL_MSG_GetString( pMsgData, pReg->cupNo, pReturnCupName );
  GFL_MSG_Delete(pMsgData);
}

//----------------------------------------------------------
/**
 * @brief	カップ名前取得（STRBUFを生成）
 * @param	pReg	ワークへのポインタ
 * @param	heapID	STRBUFを生成するヒープのID
 * @return	STRBUF	名前を格納したSTRBUFへのポインタ
 */
//----------------------------------------------------------
STRBUF* Regulation_CreateCupName(const REGULATION* pReg, HEAPID heapID)
{
  STRBUF* tmpBuf = GFL_STR_CreateBuffer((REGULATION_CUPNAME_SIZE + EOM_SIZE)*GLOBAL_MSGLEN, heapID);
  Regulation_GetCupName(pReg,tmpBuf);
//  GFL_STR_SetStringCode( tmpBuf, pReg->cupName );
  return tmpBuf;

}

//----------------------------------------------------------
/**
 * @brief	ルール名セット
 * @param	pReg	REGULATIONワークポインタ
 * @param	pCupBuf	カップ名が入ったバッファ
 */
//----------------------------------------------------------
//void Regulation_SetRuleName(REGULATION* pReg, const STRBUF *pRuleBuf)
//{
//  GFL_STR_GetStringCode(pRuleBuf, pReg->ruleName, (REGULATION_RULENAME_SIZE + EOM_SIZE));
//}

//----------------------------------------------------------
/**
 * @brief	ルール名取得
 * @param	pReg		    REGULATIONワークポインタ
 * @param	pReturnCupName	ルール名を入れるSTRBUFポインタ
 * @return	none
 */
//----------------------------------------------------------
void Regulation_GetRuleName(const REGULATION* pReg,STRBUF* pReturnRuleName)
{
//  GFL_STR_SetStringCodeOrderLength(pReturnRuleName, pReg->ruleName, (REGULATION_RULENAME_SIZE + EOM_SIZE));

  GFL_MSGDATA* pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
                                          NARC_message_regulation_cup_dat, HEAPID_SAVE_TEMP );
  GFL_MSG_GetString( pMsgData, pReg->ruleNo, pReturnRuleName );
  GFL_MSG_Delete(pMsgData);

}

//----------------------------------------------------------
/**
 * @brief	ルール名前取得（STRBUFを生成）
 * @param	pReg	ワークへのポインタ
 * @param	heapID	STRBUFを生成するヒープのID
 * @return	STRBUF	名前を格納したSTRBUFへのポインタ
 */
//----------------------------------------------------------
STRBUF* Regulation_CreateRuleName(const REGULATION* pReg, HEAPID heapID)
{
  STRBUF* tmpBuf = GFL_STR_CreateBuffer((REGULATION_RULENAME_SIZE + EOM_SIZE)*GLOBAL_MSGLEN, heapID);
  Regulation_GetRuleName(pReg,tmpBuf);

  //  GFL_STR_SetStringCode( tmpBuf, pReg->ruleName );
  return tmpBuf;
}

//----------------------------------------------------------
/**
 * @brief	パラメーターを取得する
 * @param	pReg	REGULATIONワークポインタ
 * @param	type	REGULATION_PARAM_TYPE enum のどれか
 */
//----------------------------------------------------------
int Regulation_GetParam(const REGULATION* pReg, REGULATION_PARAM_TYPE type)
{
  int ret = 0;

  switch(type){
  case REGULATION_CUPNUM:
    ret = pReg->cupNo;
    break;
  case REGULATION_RULENUM:
    ret = pReg->ruleNo;
    break;
  case REGULATION_NUM_LO: //    #参加数下限
    ret = pReg->NUM_LO;
    break;
  case REGULATION_NUM_HI:         //参加数上限
    ret = pReg->NUM_HI;
    break;
  case REGULATION_LEVEL:  //    #参加レベル
    ret = pReg->LEVEL;
    break;
  case REGULATION_LEVEL_RANGE: //    #レベル範囲
    ret = pReg->LEVEL_RANGE;
    break;
  case REGULATION_LEVEL_TOTAL: //    #レベル合計
    ret = pReg->LEVEL_TOTAL;
    break;
  case REGULATION_BOTH_POKE: //    #同じポケモン
    ret = pReg->BOTH_POKE;
    break;
  case REGULATION_BOTH_ITEM:  //    #同じどうぐ
    ret = pReg->BOTH_ITEM;
    break;
  case REGULATION_MUST_POKE:   //    #必須ポケモン
    ret = pReg->MUST_POKE;
    break;
  case REGULATION_SHOOTER:    //    #シューター
    ret = pReg->SHOOTER;
    break;
  case REGULATION_TIME_VS:     //    #対戦時間
    ret = pReg->TIME_VS;
    break;
  case REGULATION_TIME_COMMAND: //    #入力時間
    ret = pReg->TIME_COMMAND;
    break;
  case REGULATION_NICKNAME: //    #ニックネーム表示
    ret = pReg->NICKNAME;
    break;
  case REGULATION_AGE_LO:  //    #年齢制限以上
    ret = pReg->AGE_LO;
    break;
  case REGULATION_AGE_HI:  //    #年齢制限以下
    ret = pReg->AGE_HI;
    break;
  case REGULATION_SHOW_POKE:  //    #ポケモン見せ合い
    ret = pReg->SHOW_POKE;
    break;
  case REGULATION_SHOW_POKE_TIME:  //    #ポケモン見せ合い制限時間
    ret = pReg->SHOW_POKE_TIME;
    break;
  case REGULATION_BATTLETYPE:  //   バトルタイプ
    ret = pReg->BATTLE_TYPE;
    break;
  }
  return ret;
}

//----------------------------------------------------------
/**
 * @brief	パラメーターを書きこむ
 * @param	pReg	REGULATIONワークポインタ
 * @param	type	REGULATION_PARAM_TYPE enum のどれか
 * @param	param	書きこむ値
 * @return  正しく書き込んだらTRUE  不正な値の場合FALSE
 */
//----------------------------------------------------------
void Regulation_SetParam(REGULATION* pReg, REGULATION_PARAM_TYPE type, int param)
{

  switch(type){
  case REGULATION_CUPNUM:
    if(REGULATION_CUP_MAX > param){
      pReg->cupNo = param;
    }
    else{
      GF_ASSERT(0);
    }
    break;
  case REGULATION_RULENUM:
    if(REGULATION_RULE_MAX > param){
      pReg->ruleNo = param;
    }
    else{
      GF_ASSERT(0);
    }
    break;
  case REGULATION_NUM_LO: //    #参加数下限
    if(param >= 6){
      GF_ASSERT(0);
      return;
    }
    pReg->NUM_LO = param;
    break;
  case REGULATION_NUM_HI:         //参加数上限
    if(param >= 6){
      GF_ASSERT(0);
      return;
    }
    pReg->NUM_HI = param;
    break;
  case REGULATION_LEVEL:  //    #参加レベル
    if(param > 100){
      GF_ASSERT(0);
      return;
    }
    pReg->LEVEL = param;
    break;
  case REGULATION_LEVEL_RANGE: //    #レベル範囲
    if(param > REGULATION_LEVEL_RANGE_MAX){
      GF_ASSERT(0);
      return;
    }
    pReg->LEVEL_RANGE = param;
    break;
  case REGULATION_LEVEL_TOTAL: //    #レベル合計
    if(param > 600){
      GF_ASSERT(0);
      return;
    }
    pReg->LEVEL_TOTAL = param;
    break;
  case REGULATION_BOTH_POKE: //    #同じポケモン
    if(param >= 2){
      GF_ASSERT(0);
      return;
    }
    pReg->BOTH_POKE = param;
    break;
  case REGULATION_BOTH_ITEM:  //    #同じどうぐ
    if(param >= 2){
      GF_ASSERT(0);
      return;
    }
    pReg->BOTH_ITEM = param;
    break;
  case REGULATION_MUST_POKE:   //    #必須ポケモン
    if(param >= MONSNO_MAX){ 
      GF_ASSERT(0);
      return;
    }
    pReg->MUST_POKE = param;
    break;
  case REGULATION_MUST_POKE_FORM:   //    #必須ポケモンフォルム
    pReg->MUST_POKE_FORM = param;
    break;
  case REGULATION_SHOOTER:    //    #シューター
    if(param >= 2){
      GF_ASSERT(0);
      return;
    }
    pReg->SHOOTER = param;
    break;
  case REGULATION_TIME_VS:     //    #対戦時間
    if(param > REGULATION_VS_TIME){
      GF_ASSERT(0);
      return;
    }
    pReg->TIME_VS = param;
    break;
  case REGULATION_TIME_COMMAND: //    #入力時間
    if(param > REGULATION_COMMAND_TIME){
      GF_ASSERT(0);
      return;
    }
    pReg->TIME_COMMAND = param;
    break;
  case REGULATION_NICKNAME: //    #ニックネーム表示
    if(param >= 2){
      GF_ASSERT(0);
      return;
    }
    pReg->NICKNAME = param;
    break;
  case REGULATION_AGE_LO:  //    #年齢制限以上
    pReg->AGE_LO = param;
    break;
  case REGULATION_AGE_HI:  //    #年齢制限以下
    pReg->AGE_HI = param;
    break;
  case REGULATION_SHOW_POKE:  //    #ポケモン見せ合い
    if(param >= 2){
      GF_ASSERT(0);
      return;
    }
    pReg->SHOW_POKE = param;
    break;
  case REGULATION_SHOW_POKE_TIME:  //    #ポケモン見せ合い時間
    if(param > REGULATION_SHOW_POKE_TIME_MAX){
      GF_ASSERT(0);
      return;
    }
    pReg->SHOW_POKE_TIME = param;
    break;
  case REGULATION_BATTLETYPE:  //  バトルタイプ
    if(param >= REGULATION_BATTLE_MAX){
      GF_ASSERT(0);
      return;
    }
    pReg->BATTLE_TYPE = param;
    break;
  }
}

//----------------------------------------------------------
/**
 * @brief	パラメータのBITが立っているかどうかを検査
 * @param	pReg	REGULATIONワークポインタ
 * @param	type	REGULATION_PARAM_TYPE enum のどれか
 * @param	no	  ポケモン番号やアイテム番号
 * @return  BITがONでTRUE
 */
//----------------------------------------------------------
BOOL Regulation_CheckParamBit(const REGULATION* pReg, REGULATION_PARAM_TYPE type, int no)
{
  int buffno = no / 8;
  int bitno = no % 8;
  
  switch(type){
  case REGULATION_VETO_POKE_BIT:
    if(pReg->VETO_POKE_BIT[buffno] & (0x1<<bitno)){
      return TRUE;
    }
    break;
  case REGULATION_VETO_ITEM:
    if(pReg->VETO_ITEM[buffno] & (0x1<<bitno)){
      return TRUE;
    }
    break;
  }
  return FALSE;
}

//----------------------------------------------------------
/**
 * @brief	カップ名取得
 * @param	pReg		    REGULATION_CARDDATAワークポインタ
 * @param	pReturnCupName	カップ名を入れるSTRBUFポインタ
 * @return	none
 */
//----------------------------------------------------------

void Regulation_GetCardCupName(const REGULATION_CARDDATA* pReg,STRBUF* pReturnCupName)
{
  GFL_STR_SetStringCodeOrderLength(pReturnCupName, pReg->cupname, (WIFI_PLAYER_TIX_CUPNAME_MOJINUM + EOM_SIZE));
}

//----------------------------------------------------------
/**
 * @brief	カップ名前取得（STRBUFを生成）
 * @param	pReg	REGULATION_CARDDATAワークへのポインタ
 * @param	heapID	STRBUFを生成するヒープのID
 * @return	STRBUF	名前を格納したSTRBUFへのポインタ
 */
//----------------------------------------------------------

STRBUF* Regulation_CreateCardCupName(const REGULATION_CARDDATA* pReg, HEAPID heapID)
{
  STRBUF* tmpBuf = GFL_STR_CreateBuffer((WIFI_PLAYER_TIX_CUPNAME_MOJINUM + EOM_SIZE)*GLOBAL_MSGLEN, heapID);
  GFL_STR_SetStringCode( tmpBuf, pReg->cupname );
  return tmpBuf;
}

//----------------------------------------------------------
/**
 * @brief	デジタル選手証パラメーターを引き出す
 * @param	pReg	REGULATION_CARDDATAワークポインタ
 * @param	type	REGULATION_CARD_PARAM_TYPE enum のどれか
 * @return  パラメータ
 */
//----------------------------------------------------------
int Regulation_GetCardParam(const REGULATION_CARDDATA* pReg, REGULATION_CARD_PARAM_TYPE type)
{
  int ret;
  
  switch(type){
  case REGULATION_CARD_ROMVER:
    ret = pReg->ver;
    break;
  case REGULATION_CARD_CUPNO:
    ret = pReg->no;
    break;
  case REGULATION_CARD_LANGCODE:
    ret = pReg->code;
    break;
  case REGULATION_CARD_START_YEAR:  ///< 開始年：00-99
    ret = pReg->start_year;
    break;
  case REGULATION_CARD_START_MONTH: ///< 開始月：01-12
    ret = pReg->start_month;
    break;
  case REGULATION_CARD_START_DAY:   ///< 開始日：01-31
    ret = pReg->start_day;
    break;
  case REGULATION_CARD_END_YEAR:    ///< 終了年：00-99
    ret = pReg->end_year;
    break;
  case REGULATION_CARD_END_MONTH:   ///< 終了月：01-12
    ret = pReg->end_month;
    break;
  case REGULATION_CARD_END_DAY:     ///< 終了日：01-31
    ret = pReg->end_day;
    break;
  case REGULATION_CARD_STATUS:      ///< 大会状態：０未開催／１開催中／２終了
    ret = pReg->status;
    break;
  }
  return ret;
}


//----------------------------------------------------------
/**
 * @brief	パラメーターを書きこむ
 * @param	pReg	REGULATION_CARDDATAワークポインタ
 * @param	type	REGULATION_CARD_PARAM_TYPE enum のどれか
 * @param	param	書きこむ値
 * @return  none
 */
//----------------------------------------------------------
void Regulation_SetCardParam(REGULATION_CARDDATA* pReg, REGULATION_CARD_PARAM_TYPE type, int param)
{
  switch(type){
  case REGULATION_CARD_ROMVER:
    pReg->ver = param;
    break;
  case REGULATION_CARD_CUPNO:
    pReg->no = param;
    break;
  case REGULATION_CARD_LANGCODE:
    pReg->code = param;
    break;
  case REGULATION_CARD_START_YEAR:  ///< 開始年：00-99
    pReg->start_year = param;
    break;
  case REGULATION_CARD_START_MONTH: ///< 開始月：01-12
    pReg->start_month = param;
    break;
  case REGULATION_CARD_START_DAY:   ///< 開始日：01-31
    pReg->start_day = param;
    break;
  case REGULATION_CARD_END_YEAR:    ///< 終了年：00-99
    pReg->end_year = param;
    break;
  case REGULATION_CARD_END_MONTH:   ///< 終了月：01-12
    pReg->end_month = param;
    break;
  case REGULATION_CARD_END_DAY:     ///< 終了日：01-31
    pReg->end_day = param;
    break;
  case REGULATION_CARD_STATUS:      ///< 大会状態：０未開催／１開催中／２終了
    pReg->status = param;
    break;
  }
}

//----------------------------------------------------------
/**
 * @brief	レギュレーションデータへのポインタ取得
 * @param	pSave    	セーブデータ保持ワークへのポインタ
 * @param	何本目のレギュレーションデータか
 * @return	REGULATION	ワークへのポインタ  無効データの場合NULL
 */
//----------------------------------------------------------
REGULATION* SaveData_GetRegulation(SAVE_CONTROL_WORK* pSave, int regNo)
{
  REGULATION_CARDDATA* pRegData = NULL;

  GF_ASSERT(regNo < REGULATION_MAX_NUM);
  if(regNo < REGULATION_MAX_NUM){
    pRegData = SaveControl_DataPtrGet(pSave, GMDATA_ID_REGULATION_DATA);
    return &pRegData->regulation_buff;
  }
  return NULL;
}

//----------------------------------------------------------
/**
 * @brief	レギュレーションデータの保存
 * @param	pSave    	セーブデータ保持ワークへのポインタ
 * @param	const REGULATION  レギュレーションデータ
 * @return	none
 */
//----------------------------------------------------------
void SaveData_SetRegulation(SAVE_CONTROL_WORK* pSave, const REGULATION* pReg, const int regNo)
{
  REGULATION_CARDDATA* pRegData = NULL;

  pRegData = SaveControl_DataPtrGet(pSave, GMDATA_ID_REGULATION_DATA);
  GF_ASSERT(regNo < REGULATION_MAX_NUM);
  if(regNo < REGULATION_MAX_NUM){
    Regulation_Copy(pReg, &pRegData->regulation_buff);
  }
}

//----------------------------------------------------------
/**
 * @brief	レギュレーションデータへのポインタ取得
 * @param	pSave    	セーブデータ保持ワークへのポインタ
 * @param	何本目のレギュレーションデータか
 * @return	REGULATION	ワークへのポインタ  無効データの場合NULL
 */
//----------------------------------------------------------
REGULATION_CARDDATA* SaveData_GetRegulationCardData(SAVE_CONTROL_WORK* pSave)
{
  return SaveControl_DataPtrGet(pSave, GMDATA_ID_REGULATION_DATA);
}

