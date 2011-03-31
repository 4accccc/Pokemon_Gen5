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
#include "gamesystem/gamesystem_def.h"
#include "gamesystem/gamedata_def.h"
#include "gamesystem/fest_mission.h"
#include "print/wordset.h"
#include "field/gpower_id.h"
#include "gamesystem/playerwork.h"  //typedef ZONEID
#include "savedata/questionnaire_save.h"


//==============================================================================
//  定数定義
//==============================================================================
///GAMEBEACON_INFO_TBLで記録するログ件数
#define GAMEBEACON_INFO_TBL_MAX   (30)
#define GAMEBEACON_INFO_TBL_END   (GAMEBEACON_INFO_TBL_MAX-1)

///受け取り可能バージョン定義
#define GAMEBEACON_RECV_VERSION_FULL  (0xFFFF)  //全バージョン受け取り可
#define GAMEBEACON_RECV_VERSION_SWAN  (0xFFFC)  //スワン以降の全バージョン受け取り可(BW受け取り不可)

///詳細No
typedef enum{
  GAMEBEACON_DETAILS_NO_BATTLE_WILD_POKE,       ///<野生ポケモンと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_SP_POKE,         ///<特別なポケモンと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_TRAINER,         ///<トレーナーと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_JYMLEADER,       ///<ジムリーダーと対戦中
  GAMEBEACON_DETAILS_NO_BATTLE_SP_TRAINER,      ///<特別なトレーナーと対戦中
  GAMEBEACON_DETAILS_NO_WALK,                   ///<移動中
  
  GAMEBEACON_DETAILS_NO_MAX,
}GAMEBEACON_DETAILS_NO;


///行動番号
/*
 *  行動番号の増やし方
 *
 *  1, enum GAMEBEACON_ACTION に定義を追加(BW時点での予約No50までは挿入不可)
 *  2, src/gamesystem/game_beacon_accessor.c DATA_BeaconDataType[]にデータ追加
 *  3, resource/message/src/beacon_status.gmm msg_beacon_xxx にアクションメッセージを追加
 *  4, resource/cross_comm/ライブ通信設定データ.xls にプライオリティデータを追加
 *  5, src/field/event_debug_livecomm.c DATA_DebugBeaconParam[]にデータ追加
 */
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
  GAMEBEACON_ACTION_BATTLE_SP_TRAINER_START,  ///<特別なトレーナーと対戦を開始しました！      10
  GAMEBEACON_ACTION_BATTLE_SP_TRAINER_VICTORY,///<特別なトレーナーに勝利しました！            11
  GAMEBEACON_ACTION_BATTLE_CHAMPION_START,    ///<チャンピオンと対戦を開始しました！(欠番)    12
  GAMEBEACON_ACTION_BATTLE_CHAMPION_VICTORY,  ///<チャンピオンに勝利しました！(欠番)          13
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
  GAMEBEACON_ACTION_CRITICAL_HIT,             ///<急所に攻撃をあてた                    28
  GAMEBEACON_ACTION_CRITICAL_DAMAGE,          ///<急所に攻撃を受けた                    29
  GAMEBEACON_ACTION_ESCAPE,                   ///<戦闘から逃げ出した                    30
  GAMEBEACON_ACTION_HP_LITTLE,                ///<HPが残り少ない                        31
  GAMEBEACON_ACTION_PP_LITTLE,                ///<PPが残り少ない                        32
  GAMEBEACON_ACTION_DYING,                    ///<先頭のポケモンが瀕死                  33
  GAMEBEACON_ACTION_STATE_IS_ABNORMAL,        ///<先頭のポケモンが状態異常              34
  GAMEBEACON_ACTION_USE_ITEM,                 ///<アイテムを使用                        35
  GAMEBEACON_ACTION_FIELD_SKILL,              ///<フィールド技を使用                    36
  GAMEBEACON_ACTION_SODATEYA_EGG,             ///<育て屋から卵を引き取った              37
  GAMEBEACON_ACTION_EGG_HATCH,                ///<タマゴが孵化した                      38
  GAMEBEACON_ACTION_SHOPING,                  ///<買い物中                              39
  GAMEBEACON_ACTION_SUBWAY,                   ///<バトルサブウェイ挑戦中                40
  GAMEBEACON_ACTION_SUBWAY_STRAIGHT_VICTORIES,  ///<バトルサブウェイ連勝中              41
  GAMEBEACON_ACTION_SUBWAY_TROPHY_GET,          ///<バトルサブウェイトロフィーゲット    42
  GAMEBEACON_ACTION_TRIALHOUSE,               ///<トライアルハウスに挑戦中              43
  GAMEBEACON_ACTION_TRIALHOUSE_RANK,          ///<トライアルハウスでランク確定          44
  GAMEBEACON_ACTION_FERRIS_WHEEL,             ///<観覧車に乗った                        45
  GAMEBEACON_ACTION_POKESHIFTER,              ///<ポケシフターに入った                  46
  GAMEBEACON_ACTION_MUSICAL,                  ///<ミュージカル挑戦中                    47
  GAMEBEACON_ACTION_OTHER_GPOWER_USE,         ///<他人のGパワーを使用                   48
  GAMEBEACON_ACTION_FREEWORD,                 ///<一言メッセージ                        49
  GAMEBEACON_ACTION_DISTRIBUTION_GPOWER,      ///<Gパワー配布                           50
 
  //スワンで追加されたビーコン
  GAMEBEACON_ACTION_FMISSION,                     ///<フェスミッション受信                  51

  GAMEBEACON_ACTION_FMISSION_POKE_ENCOUNT,        ///<フェスミッション 野生ポケモンエンカウント   52
  GAMEBEACON_ACTION_FMISSION_POKE_WIN,            ///<フェスミッション 野生ポケモン倒した   53
  GAMEBEACON_ACTION_FMISSION_POKE_CAPTURE,        ///<フェスミッション 野生ポケモン捕まえた 54
  GAMEBEACON_ACTION_FMISSION_POKE_ESCAPE,         ///<フェスミッション 野生ポケモン逃げた&逃げられた 55
  GAMEBEACON_ACTION_FMISSION_TR_ENCOUNT,          ///<フェスミッション トレーナーエンカウント 56
  GAMEBEACON_ACTION_FMISSION_GAMEOVER,            ///<フェスミッション ゲームオーバー 57
  GAMEBEACON_ACTION_FMISSION_ITEM_USE,            ///<フェスミッション アイテム使用   　 　58
  GAMEBEACON_ACTION_FMISSION_GPOWER,              ///<フェスミッション デルパワー使用    　59
  GAMEBEACON_ACTION_FMISSION_EVENT_CLEAR,         ///<フェスミッション イベントクリア    　60
  
  GAMEBEACON_ACTION_FMISSION_FREEWORD,            ///<フェスミッション 一言メッセージ受信   61
  GAMEBEACON_ACTION_FMISSION_POKE_DOWN,           ///<フェスミッション 先頭ポケモンが瀕死   62 

  GAMEBEACON_ACTION_MAX,
}GAMEBEACON_ACTION;

#define GAMEBEACON_ACTION_FEST_BEACON_START ( GAMEBEACON_ACTION_FMISSION )
#define GAMEBEACON_ACTION_FEST_BEACON_END   ( GAMEBEACON_ACTION_FMISSION_POKE_DOWN )
#define GAMEBEACON_ACTION_FEST_BEACON_NUM   ( (GAMEBEACON_FEST_BEACON_END-GAMEBEACON_ACTION_FEST_BEACON_START)+1)
#define GAMEBEACON_ACTION_FEST_BEACON_WP_START ( GAMEBEACON_ACTION_FMISSION ) //ミッションパラメータ持ち
#define GAMEBEACON_ACTION_FEST_BEACON_WP_END ( GAMEBEACON_ACTION_FMISSION_EVENT_CLEAR )  //ミッションパラメータ持ち

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
extern void GAMEBEACON_SetGameSystemWork( GAMESYS_WORK* gsys );
extern void GAMEBEACON_Update(void);
extern void GAMEBEACON_Setting(GAMEDATA *gamedata);
extern void GAMEBEACON_SendDataCopy(GAMEBEACON_INFO *info);
extern void GAMEBEACON_SendBeaconUpdate(void);

//--------------------------------------------------------------
//  ビーコン情報取得
//--------------------------------------------------------------
extern BOOL GAMEBEACON_SetRecvBeacon(const GAMEBEACON_INFO *info);
extern const GAMEBEACON_INFO * GAMEBEACON_Get_BeaconLogDirect(int log_no);
extern const GAMEBEACON_INFO * GAMEBEACON_Get_BeaconLog(int log_no);
extern u32 GAMEBEACON_Get_LogCount(void);
extern int GAMEBEACON_Get_UpdateLogNo(int *start_log_no);
extern void GAMEBEACON_Reset_UpdateFlag(int log_no);
extern BOOL GAMEBEACON_Set_SearchUpdateFlag(const GAMEBEACON_INFO *info);
extern u16 GAMEBEACON_Get_RecvBeaconTime(u16 trainer_id);

extern BOOL GAMEBEACON_Check_FestMission(const GAMEBEACON_INFO *info);
extern BOOL GAMEBEACON_Check_FestMissionWithParam(const GAMEBEACON_INFO *info);

//--------------------------------------------------------------
//  ビーコンセット
//--------------------------------------------------------------
extern void GAMEBEACON_Set_BattlePokeStart(u16 monsno);
extern void GAMEBEACON_Set_BattlePokeVictory(u16 monsno);
extern void GAMEBEACON_Set_BattleWildPokeStart(u16 monsno);
extern void GAMEBEACON_Set_BattleWildPokeVictory(u16 monsno);
extern void GAMEBEACON_Set_BattleSpPokeStart(u16 monsno);
extern void GAMEBEACON_Set_BattleSpPokeVictory(u16 monsno);
extern void GAMEBEACON_Set_BattleTrainerStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleTrainerVictory(u16 tr_no);
extern void GAMEBEACON_Set_BattleLeaderStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleLeaderVictory(u16 tr_no);
extern void GAMEBEACON_Set_BattleSpTrainerStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleSpTrainerVictory(u16 tr_no);
extern void GAMEBEACON_Set_BattleChampionStart(u16 tr_no);
extern void GAMEBEACON_Set_BattleChampionVictory(u16 tr_no);
extern void GAMEBEACON_Set_PokemonGet(u16 monsno);
extern void GAMEBEACON_Set_WildPokemonGet(u16 monsno);
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
extern void GAMEBEACON_Set_DistributionGPower(GPOWER_ID gpower_id);
#endif
extern void GAMEBEACON_Set_CriticalHit(const STRBUF *nickname);
extern void GAMEBEACON_Set_CriticalDamage(const STRBUF *nickname);
extern void GAMEBEACON_Set_Escape( u16 monsno, BOOL run_enemy_f );
extern void GAMEBEACON_Set_HPLittle(const STRBUF *nickname);
extern void GAMEBEACON_Set_PPLittle(const STRBUF *nickname);
extern void GAMEBEACON_Set_Dying(const STRBUF *nickname);
extern void GAMEBEACON_Set_StateIsAbnormal(const STRBUF *nickname);
extern void GAMEBEACON_Set_UseItem(u16 item_no);
extern void GAMEBEACON_Set_FieldSkill(u16 wazano);
extern void GAMEBEACON_Set_SodateyaEgg(void);
extern void GAMEBEACON_Set_EggHatch(u16 monsno);
extern void GAMEBEACON_Set_Shoping(void);
extern void GAMEBEACON_Set_Subway(void);
extern void GAMEBEACON_Set_SubwayStraightVictories(u32 victory_count);
extern void GAMEBEACON_Set_SubwayTrophyGet(void);
extern void GAMEBEACON_Set_TrialHouse(void);
extern void GAMEBEACON_Set_TrialHouseRank(u8 rank);
extern void GAMEBEACON_Set_FerrisWheel(void);
extern void GAMEBEACON_Set_PokeShifter(void);
extern void GAMEBEACON_Set_Musical(const STRBUF *nickname);
extern void GAMEBEACON_Set_OtherGPowerUse(GPOWER_ID gpower_id);
extern void GAMEBEACON_Set_Thankyou(GAMEDATA *gamedata, u32 target_trainer_id);
extern void GAMEBEACON_Set_FreeWord(const STRBUF *free_word);

extern void GAMEBEACON_Set_FestMission( FEST_MISSION* mission );
extern void GAMEBEACON_Set_FMissionWildPokemonGet(u16 monsno);
extern void GAMEBEACON_Set_FMissionWildPokemonKnockOut(u16 monsno);
extern void GAMEBEACON_Set_FMissionEventClear( u16 ev_id );

extern void GAMEBEACON_Set_ResortDataEnable( BOOL is_enable );
extern void GAMEBEACON_Set_ResortRecvBuffer( void *p_buff );
#ifdef PM_DEBUG
extern void * GAMEBEACON_DEBUG_GetResortRecvBuffer( void );
#endif //PM_DEBUG


//=====詳細パラメータセット
extern void GAMEBEACON_Set_ZoneChange(ZONEID zone_id, const GAMEDATA *cp_gamedata);
extern void GAMEBEACON_Set_Details_IntroductionPms(const PMS_DATA *pms);

//--------------------------------------------------------------
//  送信バッファ更新
//--------------------------------------------------------------
extern void GAMEBEACON_SendDataUpdate_NationArea(u8 nation, u8 area);
extern void GAMEBEACON_SendDataUpdate_ResearchTeamRank(u8 research_team_rank);
extern void GAMEBEACON_SendDataUpdate_TrCardIntroduction(const PMS_DATA *pms);
extern void GAMEBEACON_SendDataUpdate_SelfIntroduction(void);
extern void GAMEBEACON_SendDataUpdate_TrainerView(int trainer_view);

//--------------------------------------------------------------
//  アンケート
//--------------------------------------------------------------
extern BOOL GAMEBEACON_Get_NewEntry(void);
extern void GAMEBEACON_SendDataUpdate_Questionnaire(QUESTIONNAIRE_ANSWER_WORK *my_ans);
#ifdef PM_DEBUG
extern void DEBUG_GAMEBEACON_Set_NewEntry(void);
extern GAMEBEACON_INFO * DEBUG_SendBeaconInfo_GetPtr(void);
extern void DEBUG_GetBeaconRecvLogCopy( void* log_buf, s8* log_num, s8* start_log, s8* end_log );
extern void DEBUG_SendBeaconCancel(void);
extern void DEBUG_SendBeaconPriorityEgnoreFlagSet( BOOL flag );
extern void DEBUG_RecvBeaconBufferClear(void);
#endif

//--------------------------------------------------------------
//  マジコン対策
//--------------------------------------------------------------
extern void GAMEBEACON_AMPROTECT_SetTask(void);
