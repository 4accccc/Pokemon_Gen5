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
  GAMEBEACON_DETAILS_NO_BATTLE_WILD_POKE,       ///<野生ポケモンと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_SP_POKE,         ///<特別なポケモンと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_TRAINER,         ///<トレーナーと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_JIMLEADER,       ///<ジムリーダーと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_BIGFOUR,         ///<四天王と対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_CHAMPION,        ///<チャンピオンと対戦中
  GAMEBEACON_DETAILS_NO_WALK,                   ///<移動中
  
  GAMEBEACON_DETAILS_NO_MAX,
}GAMEBEACON_DETAILS_NO;


///行動番号
typedef enum{
  GAMEBEACON_ACTION_NULL,                     ///<データ無し
  
  GAMEBEACON_ACTION_SEARCH,                   ///<「ｘｘｘさんをサーチしました！」      1
  GAMEBEACON_ACTION_BATTLE_WILD_POKE_START,   ///<野生のポケモンと対戦を開始しました！  2
  GAMEBEACON_ACTION_BATTLE_WILD_POKE_VICTORY, ///<野生のポケモンに勝利しました！        3
  GAMEBEACON_ACTION_BATTLE_SP_POKE_START,     ///<特別なポケモンと対戦を開始しました！  4
  GAMEBEACON_ACTION_BATTLE_SP_POKE_VICTORY,   ///<特別なポケモンに勝利しました！        5
  GAMEBEACON_ACTION_BATTLE_TRAINER_START,     ///<トレーナーと対戦を開始しました！      6
  GAMEBEACON_ACTION_BATTLE_TRAINER_VICTORY,   ///<トレーナーに勝利しました！            7
  GAMEBEACON_ACTION_BATTLE_LEADER_START,      ///<ジムリーダーと対戦を開始しました！    8
  GAMEBEACON_ACTION_BATTLE_LEADER_VICTORY,    ///<ジムリーダーに勝利しました！          9
  GAMEBEACON_ACTION_BATTLE_BIGFOUR_START,     ///<四天王と対戦を開始しました！          10
  GAMEBEACON_ACTION_BATTLE_BIGFOUR_VICTORY,   ///<四天王に勝利しました！                11
  GAMEBEACON_ACTION_BATTLE_CHAMPION_START,    ///<チャンピオンと対戦を開始しました！    12
  GAMEBEACON_ACTION_BATTLE_CHAMPION_VICTORY,  ///<チャンピオンに勝利しました！          13
  GAMEBEACON_ACTION_POKE_GET,                 ///<ポケモン捕獲                          14
  GAMEBEACON_ACTION_SP_POKE_GET,              ///<特別なポケモン捕獲                    15
  GAMEBEACON_ACTION_POKE_LVUP,                ///<ポケモンレベルアップ                  16
  GAMEBEACON_ACTION_POKE_EVOLUTION,           ///<ポケモン進化                          17
  GAMEBEACON_ACTION_GPOWER,                   ///<Gパワー発動                           18
  GAMEBEACON_ACTION_SP_ITEM_GET,              ///<貴重品ゲット                          19
  GAMEBEACON_ACTION_PLAYTIME,                 ///<一定のプレイ時間を越えた              20
  GAMEBEACON_ACTION_ZUKAN_COMPLETE,           ///<図鑑完成                              21
  GAMEBEACON_ACTION_THANKYOU_OVER,            ///<お礼を受けた回数が規定数を超えた      22
  GAMEBEACON_ACTION_UNION_IN,                 ///<ユニオンルームに入った                23
  GAMEBEACON_ACTION_THANKYOU,                 ///<「ありがとう！」                      24
  GAMEBEACON_ACTION_DISTRIBUTION_POKE,        ///<ポケモン配布中                        25
  GAMEBEACON_ACTION_DISTRIBUTION_ITEM,        ///<アイテム配布中                        26
  GAMEBEACON_ACTION_DISTRIBUTION_ETC,         ///<その他配布中                          27
  
//  GAMEBEACON_ACTION_CONGRATULATIONS,      ///<「おめでとう！」
//  GAMEBEACON_ACTION_UNION_OUT,            ///<ユニオンルーム退室
//  GAMEBEACON_ACTION_ENCOUNT_DOWN,         ///<エンカウント率ダウン
  
  GAMEBEACON_ACTION_MAX,
}GAMEBEACON_ACTION;


///調査隊ランク
typedef enum{
  RESEARCH_TEAM_RANK_0,
  RESEARCH_TEAM_RANK_1,
  RESEARCH_TEAM_RANK_2,
  RESEARCH_TEAM_RANK_3,
  RESEARCH_TEAM_RANK_4,
}RESEARCH_TEAM_RANK;


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
extern void GAMEBEACON_Set_BattleWildPokeStart(u16 monsno);
extern void GAMEBEACON_Set_BattleWildPokeVictory(u16 monsno);
extern void GAMEBEACON_Set_BattleSpPokeStart(u16 monsno);
extern void GAMEBEACON_Set_BattleSpPokeVictory(u16 monsno);
extern void GAMEBEACON_Set_BattleTrainerStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleTrainerVictory(u16 tr_no);
extern void GAMEBEACON_Set_BattleLeaderStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleLeaderVictory(u16 tr_no);
extern void GAMEBEACON_Set_BattleBigFourStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleBigFourVictory(u16 tr_no);
extern void GAMEBEACON_Set_BattleChampionStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleChampionVictory(u16 tr_no);
extern void GAMEBEACON_Set_PokemonGet(u16 monsno);
extern void GAMEBEACON_Set_SpecialPokemonGet(u16 monsno);
extern void GAMEBEACON_Set_PokemonLevelUp(const STRBUF *nickname);
extern void GAMEBEACON_Set_PokemonEvolution(u16 monsno, const STRBUF *nickname);
extern void GAMEBEACON_Set_GPower(GPOWER_ID g_power_id);
extern void GAMEBEACON_Set_SpItemGet(u16 item);
extern void GAMEBEACON_Set_PlayTime(u32 hour);
extern void GAMEBEACON_Set_ZukanComplete(void);
extern void GAMEBEACON_Set_ThankyouOver(u32 thankyou_count);
extern void GAMEBEACON_Set_UnionIn(void);
#ifdef PM_DEBUG //イベント用の配布ROMでしか発信はしないのでテスト用
extern void GAMEBEACON_Set_DistributionPoke(u16 monsno);
extern void GAMEBEACON_Set_DistributionItem(u16 item);
extern void GAMEBEACON_Set_DistributionEtc(void);
#endif
extern void GAMEBEACON_Set_Thankyou(GAMEDATA *gamedata, u32 target_trainer_id);

//=====詳細パラメータセット
extern void GAMEBEACON_Set_ZoneChange(ZONEID zone_id, const GAMEDATA *cp_gamedata);
extern void GAMEBEACON_Set_Details_IntroductionPms(const PMS_DATA *pms);

//--------------------------------------------------------------
//  アンケート
//--------------------------------------------------------------
extern BOOL GAMEBEACON_Get_NewEntry(void);
#ifdef PM_DEBUG
extern void DEBUG_GAMEBEACON_Set_NewEntry(void);
#endif

//--------------------------------------------------------------
//  削除予定
//--------------------------------------------------------------
extern u32 GAMEBEACON_GetMsgID(const GAMEBEACON_INFO *info);
extern void GAMEBEACON_Wordset(const GAMEBEACON_INFO *info, WORDSET *wordset, HEAPID temp_heap_id);
