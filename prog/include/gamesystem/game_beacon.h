//==============================================================================
/**
 * @file    game_beacon.h
 * @brief   
 * @author  matsuda
 * @date    2009.12.13(日)
 */
//==============================================================================
#pragma once

#include "buflen.h"
#include "gamesystem/gamedata_def.h"
#include "print/wordset.h"
#include "field/gpower_id.h"
#include "gamesystem/playerwork.h"  //typedef ZONEID


//==============================================================================
//  定数定義
//==============================================================================
///GAMEBEACON_INFO_TBLで記録するログ件数
#define GAMEBEACON_INFO_TBL_MAX   (30)

///詳細No
typedef enum{
  GAMEBEACON_DETAILS_NO_BATTLE_TRAINER,         ///<トレーナーと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_JIMLEADER,       ///<ジムリーダーと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_BIGFOUR,         ///<四天王と対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_CHAMPION,        ///<チャンピオンと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_POKEMON,         ///<野生ポケモンと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_REGEND_POKEMON,  ///<伝説ポケモンと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_VISION_POKEMON,  ///<幻ポケモンと対戦中
  GAMEBEACON_DETAILS_NO_ROAD,                   ///<道路を移動中
  GAMEBEACON_DETAILS_NO_TOWN,                   ///<街、ダンジョンを移動中
  GAMEBEACON_DETAILS_NO_JIM,                    ///<ジムを移動中
  
  GAMEBEACON_DETAILS_NO_MAX,
}GAMEBEACON_DETAILS_NO;


///行動番号
typedef enum{
  GAMEBEACON_ACTION_NULL,                 ///<データ無し
  GAMEBEACON_ACTION_APPEAL,               ///<「ちかくにいます」
  GAMEBEACON_ACTION_CONGRATULATIONS,      ///<「おめでとう！」
  GAMEBEACON_ACTION_POKE_EVOLUTION,       ///<ポケモン進化
  GAMEBEACON_ACTION_POKE_LVUP,            ///<ポケモンレベルアップ
  GAMEBEACON_ACTION_POKE_GET,             ///<ポケモン捕獲
  GAMEBEACON_ACTION_UNION_IN,             ///<ユニオンルーム入室
  GAMEBEACON_ACTION_UNION_OUT,            ///<ユニオンルーム退室
  GAMEBEACON_ACTION_ENCOUNT_DOWN,         ///<エンカウント率ダウン
  
  GAMEBEACON_ACTION_MAX,
}GAMEBEACON_ACTION;


//==============================================================================
//  型定義
//==============================================================================
///送受信されるビーコンパラメータ
typedef struct _GAMEBEACON_INFO GAMEBEACON_INFO;

///ビーコンパラメータテーブル
typedef struct _GAMEBEACON_INFO_TBL GAMEBEACON_INFO_TBL;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void GAMEBEACON_Init(HEAPID heap_id);
extern void GAMEBEACON_Exit(void);
extern void GAMEBEACON_Update(void);
extern void GAMEBEACON_Setting(GAMEDATA *gamedata);
extern void GAMEBEACON_SendDataCopy(GAMEBEACON_INFO *info);
extern void GAMEBEACON_SendBeaconUpdate(void);

//--------------------------------------------------------------
//  ビーコン情報取得
//--------------------------------------------------------------
extern BOOL GAMEBEACON_SetRecvBeacon(const GAMEBEACON_INFO *info);
extern const GAMEBEACON_INFO * GAMEBEACON_Get_BeaconLog(int log_no);
extern u32 GAMEBEACON_Get_LogCount(void);
extern int GAMEBEACON_Get_UpdateLogNo(int *start_log_no);
extern void GAMEBEACON_Reset_UpdateFlag(int log_no);

//--------------------------------------------------------------
//  ビーコンセット
//--------------------------------------------------------------
extern void GAMEBEACON_Set_Congratulations(void);
extern void GAMEBEACON_Set_PokemonEvolution(const STRBUF *nickname);
extern void GAMEBEACON_Set_PokemonLevelUp(const STRBUF *nickname);
extern void GAMEBEACON_Set_PokemonGet(const STRBUF *nickname);
extern void GAMEBEACON_Set_UnionIn(void);
extern void GAMEBEACON_Set_UnionOut(void);
extern void GAMEBEACON_Set_EncountDown(void);
extern void GAMEBEACON_Set_ZoneChange(ZONEID zone_id);
extern void GAMEBEACON_Set_GPower(GPOWER_ID g_power_id);
extern void GAMEBEACON_Set_ThanksRecvCount(u16 count);
extern void GAMEBEACON_Set_SuretigaiCount(u16 count);
extern void GAMEBEACON_Set_BattleTrainer(u16 trainer_code);
extern void GAMEBEACON_Set_BattlePokemon(u16 monsno);

//--------------------------------------------------------------
//  削除予定
//--------------------------------------------------------------
extern u32 GAMEBEACON_GetMsgID(const GAMEBEACON_INFO *info);
extern void GAMEBEACON_Wordset(const GAMEBEACON_INFO *info, WORDSET *wordset, HEAPID temp_heap_id);
