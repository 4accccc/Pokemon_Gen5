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
#define GAMEBEACON_INFO_TBL_END   (GAMEBEACON_INFO_TBL_MAX-1)

///詳細No
typedef enum{
  GAMEBEACON_DETAILS_NO_BATTLE_TRAINER,         ///<トレーナーと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_JIMLEADER,       ///<ジムリーダーと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_BIGFOUR,         ///<四天王と対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_CHAMPION,        ///<チャンピオンと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_POKEMON,         ///<野生ポケモンと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_SPECIAL_POKEMON, ///<特別なポケモンと対戦中
  GAMEBEACON_DETAILS_NO_WALK,                   ///<移動中
  
  GAMEBEACON_DETAILS_NO_MAX,
}GAMEBEACON_DETAILS_NO;


///行動番号
typedef enum{
  GAMEBEACON_ACTION_NULL,                     ///<データ無し
  
  GAMEBEACON_ACTION_SEARCH,                   ///<「ｘｘｘさんをサーチしました！」
  GAMEBEACON_ACTION_BATTLE_LEADER_START,      ///<ジムリーダーと対戦を開始しました！
  GAMEBEACON_ACTION_BATTLE_LEADER_VICTORY,    ///<ジムリーダーに勝利しました！
  GAMEBEACON_ACTION_BATTLE_BIGFOUR_START,     ///<四天王と対戦を開始しました！
  GAMEBEACON_ACTION_BATTLE_BIGFOUR_VICTORY,   ///<四天王に勝利しました！
  GAMEBEACON_ACTION_BATTLE_CHAMPION_START,    ///<チャンピオンと対戦を開始しました！
  GAMEBEACON_ACTION_BATTLE_CHAMPION_VICTORY,  ///<チャンピオンに勝利しました！
  GAMEBEACON_ACTION_POKE_GET,                 ///<ポケモン捕獲
  GAMEBEACON_ACTION_SP_POKE_GET,              ///<特別なポケモン捕獲
  GAMEBEACON_ACTION_POKE_LVUP,                ///<ポケモンレベルアップ
  GAMEBEACON_ACTION_POKE_EVOLUTION,           ///<ポケモン進化
  GAMEBEACON_ACTION_GPOWER,                   ///<Gパワー発動
  GAMEBEACON_ACTION_SP_ITEM_GET,              ///<貴重品ゲット
  GAMEBEACON_ACTION_PLAYTIME,                 ///<一定のプレイ時間を越えた
  GAMEBEACON_ACTION_ZUKAN_COMPLETE,           ///<図鑑完成
  GAMEBEACON_ACTION_THANKYOU_OVER,            ///<お礼を受けた回数が規定数を超えた
  GAMEBEACON_ACTION_UNION_IN,                 ///<ユニオンルームに入った
  GAMEBEACON_ACTION_DISTRIBUTION_POKE,        ///<ポケモン配布中
  GAMEBEACON_ACTION_DISTRIBUTION_ITEM,        ///<アイテム配布中
  GAMEBEACON_ACTION_DISTRIBUTION_ETC,         ///<その他配布中
  GAMEBEACON_ACTION_THANKYOU,                 ///<「ありがとう！」
  
//  GAMEBEACON_ACTION_CONGRATULATIONS,      ///<「おめでとう！」
//  GAMEBEACON_ACTION_UNION_OUT,            ///<ユニオンルーム退室
//  GAMEBEACON_ACTION_ENCOUNT_DOWN,         ///<エンカウント率ダウン
  
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
extern BOOL GAMEBEACON_Set_SearchUpdateFlag(const GAMEBEACON_INFO *info);

//--------------------------------------------------------------
//  ビーコンセット
//--------------------------------------------------------------
extern void GAMEBEACON_Set_BattleLeaderStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleLeaderVictory(u16 tr_no);
extern void GAMEBEACON_Set_BattleBigFourStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleBigFourVictory(u16 tr_no);
extern void GAMEBEACON_Set_BattleChampionStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleChampionVictory(u16 tr_no);
extern void GAMEBEACON_Set_PokemonGet(const STRBUF *nickname);
extern void GAMEBEACON_Set_SpecialPokemonGet(const STRBUF *nickname);
extern void GAMEBEACON_Set_PokemonLevelUp(const STRBUF *nickname);
extern void GAMEBEACON_Set_PokemonEvolution(const STRBUF *nickname);
extern void GAMEBEACON_Set_GPower(GPOWER_ID g_power_id);
extern void GAMEBEACON_Set_Thankyou(GAMEDATA *gamedata, u32 target_trainer_id);
extern void GAMEBEACON_Set_UnionIn(void);

//=====詳細パラメータセット
extern void GAMEBEACON_Set_EncountDown(void);
extern void GAMEBEACON_Set_ZoneChange(ZONEID zone_id);
extern void GAMEBEACON_Set_ThanksRecvCount(u16 count);
extern void GAMEBEACON_Set_SuretigaiCount(u16 count);
extern void GAMEBEACON_Set_BattleTrainer(u16 trainer_code);
extern void GAMEBEACON_Set_BattlePokemon(u16 monsno);

//--------------------------------------------------------------
//  削除予定
//--------------------------------------------------------------
extern u32 GAMEBEACON_GetMsgID(const GAMEBEACON_INFO *info);
extern void GAMEBEACON_Wordset(const GAMEBEACON_INFO *info, WORDSET *wordset, HEAPID temp_heap_id);
