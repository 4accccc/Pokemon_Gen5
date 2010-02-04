//======================================================================
/**
 * @file	irc_battle_subproc.h
 * @brief	赤外線バトル リスト・ステータス呼び出し->WIFI_MATCHへもってきてPROC化
 * @author	ariizumi -> nagihashi
 * @data	09/01/19
 *
 */
//======================================================================
#pragma once
#include "savedata/regulation.h"
#include "poke_tool/poke_tool.h"
#include "gamesystem/gamedata_def.h"
#include "poke_tool/pokeparty.h"
#include "pm_define.h"
//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define WIFIBATTLEMATCH_MEMBER_NUM (2)


//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum 
{
  WIFIBATTLEMATCH_SUBPROC_RESULT_SUCCESS,
  WIFIBATTLEMATCH_SUBPROC_RESULT_ERROR_NEXT_LOGIN,
} WIFIBATTLEMATCH_SUBPROC_RESULT;



//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{ 
  REGULATION          *regulation;                  //[in]レギュレーション
  POKEMON_PASO_PARAM  *ppp[TEMOTI_POKEMAX];         //[in]選ぶポケモン
  const STRCODE   *enemyName;                             //[in]相手の名前
  u8        enemySex;                               //[in]相手の性別
  u8        pad2[3];                                //パディング
  POKEPARTY *enemyPokeParty;                             //[in]相手のポケモン
  GAMEDATA            *gameData;                    //[in]ゲームデータ
  u8                  comm_selected_num;            //[in]すでに選び終わった人数 外部操作
  u8                  dummy[3];
  POKEPARTY           *p_party;                     //[out]出場するポケパーティ(Allocしておくこと)
  WIFIBATTLEMATCH_SUBPROC_RESULT  result;           //[out]終了コード
} WIFIBATTLEMATCH_SUBPROC_PARAM;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	proc data
//--------------------------------------------------------------
extern const GFL_PROC_DATA WifiBattleMatch_Sub_ProcData;
