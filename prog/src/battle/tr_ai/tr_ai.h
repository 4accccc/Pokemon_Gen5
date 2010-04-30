
//============================================================================================
/**
 * @file  tr_ai.h
 * @brief トレーナーAIプログラム
 * @author  HisashiSogabe
 * @date  06.04.26
 */
//============================================================================================
#pragma once

#include  "battle/btl_common.h"
#include  "battle/btl_main.h"
#include  "../btl_server_flow_def.h"
#include  "tr_ai_def.h"

//=========================================================================
//  AI_THINK_BIT宣言
//=========================================================================
enum
{
  AI_THINK_NO_BASIC = 0,    //基本AI
  AI_THINK_NO_STRONG,       //攻撃型AI
  AI_THINK_NO_EXPERT,       //エキスパートAI
  AI_THINK_NO_HOJO,         //補助技AI
  AI_THINK_NO_GAMBLE,       //ギャンブルAI
  AI_THINK_NO_DAMAGE,       //ダメージ重視AI
  AI_THINK_NO_RELAY,        //リレー型AI
  AI_THINK_NO_DOUBLE,       //ダブルバトルAI
  AI_THINK_NO_HP_CHECK,     //HPで判断AI
  AI_THINK_NO_WEATHER,      //天候技AI
  AI_THINK_NO_HARASS,       //いやがらせAI

  AI_THINK_NO_MV_POKE,      //移動ポケモン
  AI_THINK_NO_SAFARI,       //サファリゾーン
  AI_THINK_NO_GUIDE,        //ガイドバトル

  AI_THINK_NO_MAX,          //
};

enum
{
  AI_THINK_BIT_BASIC    = 1 << AI_THINK_NO_BASIC,     //基本AI
  AI_THINK_BIT_STRONG   = 1 << AI_THINK_NO_STRONG,    //攻撃型AI
  AI_THINK_BIT_EXPERT   = 1 << AI_THINK_NO_EXPERT,    //エキスパートAI
  AI_THINK_BIT_HOJO     = 1 << AI_THINK_NO_HOJO,      //補助技AI
  AI_THINK_BIT_GAMBLE   = 1 << AI_THINK_NO_GAMBLE,    //ギャンブルAI
  AI_THINK_BIT_DAMAGE   = 1 << AI_THINK_NO_DAMAGE,    //ダメージ重視AI
  AI_THINK_BIT_RELAY    = 1 << AI_THINK_NO_RELAY,     //リレー型AI
  AI_THINK_BIT_DOUBLE   = 1 << AI_THINK_NO_DOUBLE,    //ダブルバトルAI
  AI_THINK_BIT_HP_CHECK = 1 << AI_THINK_NO_HP_CHECK,  //HPで判断AI
  AI_THINK_BIT_WEATHER  = 1 << AI_THINK_NO_WEATHER,   //天候技AI
  AI_THINK_BIT_HARASS   = 1 << AI_THINK_NO_HARASS,    //いやがらせAI
  AI_THINK_BIT_MV_POKE  = 1 << AI_THINK_NO_MV_POKE,   //移動ポケモン
  AI_THINK_BIT_SAFARI   = 1 << AI_THINK_NO_SAFARI,    //サファリゾーン
  AI_THINK_BIT_GUIDE    = 1 << AI_THINK_NO_GUIDE,     //ガイドバトル
};

extern  VMHANDLE* TR_AI_Init( const BTL_MAIN_MODULE* wk, BTL_SVFLOW_WORK* svfWork, const BTL_POKE_CONTAINER* pokeCon, u32 ai_bit, HEAPID heapID );
extern  BOOL      TR_AI_Main( VMHANDLE* vmh );
extern  void      TR_AI_Exit( VMHANDLE* vmh );
extern  void      TR_AI_Start( VMHANDLE* vmh, u8* unselect, BtlPokePos pos );
extern  u8        TR_AI_GetSelectWazaPos( VMHANDLE* vmh );
extern  u8        TR_AI_GetSelectWazaDir( VMHANDLE* vmh );
