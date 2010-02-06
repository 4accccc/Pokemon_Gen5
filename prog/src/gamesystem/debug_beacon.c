//==============================================================================
/**
 * @file    debug_beacon.c
 * @brief   デバッグ用のビーコン情報作成
 * @author  matsuda
 * @date    2010.02.06(土)
 */
//==============================================================================
#ifdef PM_DEBUG

#include <gflib.h>
#include "gamesystem/game_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "msg/msg_beacon_status.h"
#include "savedata/encount_sv.h"
#include "savedata/misc.h"
#include "net/net_whpipe.h"
#include "savedata/my_pms_data.h"

#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/debug_beacon.h"


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void BEACONINFO_Set_BattleWildPokeStart(GAMEBEACON_INFO *info, u16 monsno);
extern void BEACONINFO_Set_BattleWildPokeVictory(GAMEBEACON_INFO *info, u16 monsno);
extern void BEACONINFO_Set_BattleSpPokeStart(GAMEBEACON_INFO *info, u16 monsno);
extern void BEACONINFO_Set_BattleSpPokeVictory(GAMEBEACON_INFO *info, u16 monsno);
extern void BEACONINFO_Set_BattleTrainerStart(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_BattleTrainerVictory(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_BattleLeaderStart(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_BattleLeaderVictory(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_BattleBigFourStart(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_BattleBigFourVictory(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_BattleChampionStart(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_BattleChampionVictory(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_PokemonGet(GAMEBEACON_INFO *info, u16 monsno);
extern void BEACONINFO_Set_SpecialPokemonGet(GAMEBEACON_INFO *info, u16 monsno);
extern void BEACONINFO_Set_PokemonLevelUp(GAMEBEACON_INFO *info, const STRBUF *nickname);
extern void BEACONINFO_Set_PokemonEvolution(GAMEBEACON_INFO *info, u16 monsno, const STRBUF *nickname);
extern void BEACONINFO_Set_GPower(GAMEBEACON_INFO *info, GPOWER_ID g_power_id);
extern void BEACONINFO_Set_SpItemGet(GAMEBEACON_INFO *info, u16 item);
extern void BEACONINFO_Set_PlayTime(GAMEBEACON_INFO *info, u32 hour);
extern void BEACONINFO_Set_ZukanComplete(GAMEBEACON_INFO *info);
extern void BEACONINFO_Set_ThankyouOver(GAMEBEACON_INFO *info, u32 thankyou_count);
extern void BEACONINFO_Set_UnionIn(GAMEBEACON_INFO *info);
#ifdef PM_DEBUG
extern void BEACONINFO_Set_DistributionPoke(GAMEBEACON_INFO *info, u16 monsno);
extern void BEACONINFO_Set_DistributionItem(GAMEBEACON_INFO *info, u16 item);
extern void BEACONINFO_Set_DistributionEtc(GAMEBEACON_INFO *info);
#endif
extern void BEACONINFO_Set_Thankyou(GAMEBEACON_INFO *info, GAMEDATA *gamedata, u32 target_trainer_id);
extern void BEACONINFO_Set_ZoneChange(GAMEBEACON_INFO *info, ZONEID zone_id);
extern void BEACONINFO_Set_ThanksRecvCount(GAMEBEACON_INFO *info, u32 count);
extern void BEACONINFO_Set_SuretigaiCount(GAMEBEACON_INFO *info, u32 count);

extern void BEACONINFO_Set_Details_BattleWildPoke(GAMEBEACON_INFO *info, u16 monsno);
extern void BEACONINFO_Set_Details_BattleSpPoke(GAMEBEACON_INFO *info, u16 monsno);
extern void BEACONINFO_Set_Details_BattleTrainer(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_Details_BattleLeader(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_Details_BattleBigFour(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_Details_BattleChampion(GAMEBEACON_INFO *info, u16 tr_no);
extern void BEACONINFO_Set_Details_Walk(GAMEBEACON_INFO *info);
extern void BEACONINFO_Set_Details_IntroductionPms(GAMEBEACON_INFO *info, const PMS_DATA *pms);

extern GAMEBEACON_INFO * DEBUG_SendBeaconInfo_GetPtr(void);


//--------------------------------------------------------------
//  プロトタイプ宣言
//--------------------------------------------------------------
static void _SetDebugBeaconInfo(GAMEBEACON_INFO *info, const DEBUG_GAMEBEACON_INFO *debuginfo, const DEBUG_ACTION_DATA *debugaction);


#if 0 //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#include "fieldmap/zone_id.h"
#include "item/itemsym.h"
//--------------------------------------------------------------
//  sample:テスト用仮想人物データ
//--------------------------------------------------------------
static const DEBUG_GAMEBEACON_INFO DebugBeaconTbl[] = {
  {
    3,                            //国コード
    1,                            //地域コード
    99999,                        //すれ違い人数
    GPOWER_ID_2,                  //発動しているGパワー
    VERSION_BLACK,                //PM_VERSION
    ZONE_ID_C02,                  //現在地
    PM_MALE,                      //性別
    LANG_SPAIN,                   //PM_LANG
    1,                            //タウンマップでの座標テーブルIndex
    5,                            //ユニオンルームでの見た目(０～１５)
    RESEARCH_TEAM_RANK_1,         //調査隊ランク
    8,                            //本体情報の色(Index) ０～１５
    12345,                        //お礼を受けた回数
    55332,                        //トレーナーID
    {L'ぶ',L'ら',L'っ',L'く',0xffff},   //トレーナー名
    {L'は',L'じ',L'め',L'ま',L'し',L'て',L'！',L'？',0xffff},   //自己紹介
    {2, 3, 10, 20},               //簡易会話(sentence_type, sentence_id, word_0, word_1)
    {                             //デフォルト行動データ
      GAMEBEACON_ACTION_BATTLE_WILD_POKE_START,
      MONSNO_MYUU,
    },
  },
  {
    5,                            //国コード
    0,                            //地域コード
    888,                          //すれ違い人数
    GPOWER_ID_4,                  //発動しているGパワー
    VERSION_WHITE,                //PM_VERSION
    ZONE_ID_C02GYM0101,           //現在地
    PM_FEMALE,                    //性別
    LANG_JAPAN,                   //PM_LANG
    0,                            //タウンマップでの座標テーブルIndex
    9,                            //ユニオンルームでの見た目(０～１５)
    RESEARCH_TEAM_RANK_3,         //調査隊ランク
    15,                           //本体情報の色(Index) ０～１５
    7,                            //お礼を受けた回数
    104,                          //トレーナーID
    {L'W',L'h',L'i',L't',L'e',0xffff},   //トレーナー名
    {L'H',L'e',L'l',L'l',L'o',L'!',0xffff},   //自己紹介
    {0, 0, 1, 2},                 //簡易会話(sentence_type, sentence_id, word_0, word_1)
    {                             //デフォルト行動データ
      GAMEBEACON_ACTION_SEARCH,
      0,
    },
  },
};

//--------------------------------------------------------------
//  sample:行動データ
//--------------------------------------------------------------
static const DEBUG_ACTION_DATA DebugActionTbl[] = {
  {
    GAMEBEACON_ACTION_SEARCH,
    0,
  },
  {
    GAMEBEACON_ACTION_BATTLE_WILD_POKE_START,
    MONSNO_MYUU,
  },
  {
    GAMEBEACON_ACTION_BATTLE_WILD_POKE_VICTORY,
    MONSNO_MYUU,
  },
  {
    GAMEBEACON_ACTION_BATTLE_TRAINER_START,
    20,
  },
  {
    GAMEBEACON_ACTION_BATTLE_LEADER_START,
    30,
  },
  {
    GAMEBEACON_ACTION_POKE_LVUP,
    L'し',L'ん',L'た',L'ろ',L'う',0xffff,
  },
  {
    GAMEBEACON_ACTION_POKE_EVOLUTION,
    30, L'た',L'ろ',L'う',0xffff,
  },
  {
    GAMEBEACON_ACTION_GPOWER,
    GPOWER_ID_1,
  },
  {
    GAMEBEACON_ACTION_SP_ITEM_GET,
    ITEM_KOORINAOSI,
  },
  {
    GAMEBEACON_ACTION_THANKYOU_OVER,
    64317,
  },
};

#if 0
//受信バッファへ一括セット
for(i = 0; i < NELEMS(DebugBeaconTbl); i++){
  DebugRecvBuf_BeaconSet(&DebugBeaconTbl[i], NULL);
};


//行動アクションを指定して、受信バッファへセット
                          ↓対象の人物データを指定      ↓任意の行動データを指定
  DebugRecvBuf_BeaconSet(&DebugBeaconTbl[select_no], &DebugActionTbl[select_action]);
  

//送信ビーコンを自分ではなく、指定した人に成り代わって送信する
  DebugSendBuf_BeaconSet(&DebugBeaconTbl[select_no]);
  GAMEBEACON_Set_xxx    基本情報が成り代わった後は通常の送信命令を使用する
#endif
#endif  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@



//==============================================================================
//
//  
//
//==============================================================================

//==================================================================
/**
 * 自分の送信ビーコンを仮想人物データに成り代わる
 *
 * @param   debuginfo		仮想人物データ
 */
//==================================================================
void DebugSendBuf_BeaconSet(const DEBUG_GAMEBEACON_INFO *debuginfo)
{
  GAMEBEACON_INFO *send = DEBUG_SendBeaconInfo_GetPtr();
  
  _SetDebugBeaconInfo(send, debuginfo, NULL);
}

//==================================================================
/**
 * ビーコン受信バッファへ仮想人物データからのビーコンとして強制セット
 *
 * @param   debuginfo		  仮想人物データ
 * @param   debugaction		行動データ(NULLの場合は仮想人物データが持つデフォルト行動データを適用)
 */
//==================================================================
void DebugRecvBuf_BeaconSet(const DEBUG_GAMEBEACON_INFO *debuginfo, const DEBUG_ACTION_DATA *debugaction)
{
  GAMEBEACON_INFO info = {0};
  
  _SetDebugBeaconInfo(&info, debuginfo, debugaction);

  //受信バッファへセット
  info.send_counter = GFUser_GetPublicRand0(0xff); //カウンターがずれるようランダムで毎回取得
  if(GAMEBEACON_SetRecvBeacon(&info) == FALSE){
    info.send_counter++; //ランダムが被る可能性があるので失敗の場合はカウンタを進めてもう一回
    GAMEBEACON_SetRecvBeacon(&info);
  }
}

//--------------------------------------------------------------
/**
 * 仮想人物データからビーコン情報作成する
 *
 * @param   info		      データ代入先
 * @param   debuginfo		  仮想人物データ
 * @param   debugaction		行動データ
 */
//--------------------------------------------------------------
static void _SetDebugBeaconInfo(GAMEBEACON_INFO *info, const DEBUG_GAMEBEACON_INFO *debuginfo, const DEBUG_ACTION_DATA *debugaction)
{
  const DEBUG_ACTION_DATA *src_action;
  int i;
  
  info->version_bit = 0xffff;
  info->zone_id = debuginfo->zone_id;
  info->g_power_id = debuginfo->g_power_id;
  info->trainer_id = debuginfo->trainer_id;
  info->favorite_color_index = debuginfo->favorite_color_index;
  info->trainer_view = debuginfo->trainer_view;
  info->sex = debuginfo->sex;
  info->pm_version = debuginfo->pm_version;
  info->language = debuginfo->language;
  info->nation = debuginfo->nation;
  info->area = debuginfo->area;
  info->research_team_rank = debuginfo->research_team_rank;
  info->thanks_recv_count = debuginfo->thanks_recv_count;
  info->suretigai_count = debuginfo->suretigai_count;
  
  BEACONINFO_Set_Details_IntroductionPms(info, &debuginfo->pmsdata);
  
  for(i = 0; i < PERSON_NAME_SIZE; i++){
    info->name[i] = debuginfo->name[i];
  }
  for(i = 0; i < GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN; i++){
    info->self_introduction[i] = debuginfo->self_introduction[i];
  }
  
  //行動データセット
  if(debugaction != NULL){
    src_action = debugaction;
  }
  else{
    src_action = &debuginfo->action_data;
  }
  switch(src_action->action_no){
  case GAMEBEACON_ACTION_SEARCH:                   ///<「ｘｘｘさんをサーチしました！」      1
    info->action.action_no = GAMEBEACON_ACTION_SEARCH;
    break;
  case GAMEBEACON_ACTION_BATTLE_WILD_POKE_START:   ///<野生のポケモンと対戦を開始しました！  2
    BEACONINFO_Set_BattleWildPokeStart(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_WILD_POKE_VICTORY: ///<野生のポケモンに勝利しました！        3
    BEACONINFO_Set_BattleWildPokeVictory(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_SP_POKE_START:     ///<特別なポケモンと対戦を開始しました！  4
    BEACONINFO_Set_BattleSpPokeStart(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_SP_POKE_VICTORY:   ///<特別なポケモンに勝利しました！        5
    BEACONINFO_Set_BattleSpPokeVictory(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_TRAINER_START:     ///<トレーナーと対戦を開始しました！      6
    BEACONINFO_Set_BattleTrainerStart(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_TRAINER_VICTORY:   ///<トレーナーに勝利しました！            7
    BEACONINFO_Set_BattleTrainerVictory(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_LEADER_START:      ///<ジムリーダーと対戦を開始しました！    8
    BEACONINFO_Set_BattleLeaderStart(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_LEADER_VICTORY:    ///<ジムリーダーに勝利しました！          9
    BEACONINFO_Set_BattleLeaderVictory(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_BIGFOUR_START:     ///<四天王と対戦を開始しました！          10
    BEACONINFO_Set_BattleBigFourStart(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_BIGFOUR_VICTORY:   ///<四天王に勝利しました！                11
    BEACONINFO_Set_BattleBigFourVictory(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_CHAMPION_START:    ///<チャンピオンと対戦を開始しました！    12
    BEACONINFO_Set_BattleChampionStart(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_BATTLE_CHAMPION_VICTORY:  ///<チャンピオンに勝利しました！          13
    BEACONINFO_Set_BattleChampionVictory(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_POKE_GET:                 ///<ポケモン捕獲                          14
    BEACONINFO_Set_PokemonGet(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_SP_POKE_GET:              ///<特別なポケモン捕獲                    15
    BEACONINFO_Set_SpecialPokemonGet(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_POKE_LVUP:                ///<ポケモンレベルアップ                  16
    {
      STRBUF *temp_nickname = GFL_STR_CreateBuffer(BUFLEN_POKEMON_NAME, HEAPID_WORLD);
    	GFL_STR_SetStringCodeOrderLength(temp_nickname, src_action->param, BUFLEN_POKEMON_NAME);
      BEACONINFO_Set_PokemonLevelUp(info, temp_nickname);
      GFL_STR_DeleteBuffer(temp_nickname);
    }
    break;
  case GAMEBEACON_ACTION_POKE_EVOLUTION:           ///<ポケモン進化                          17
    {
      STRBUF *temp_nickname = GFL_STR_CreateBuffer(BUFLEN_POKEMON_NAME, HEAPID_WORLD);
    	GFL_STR_SetStringCodeOrderLength(temp_nickname, &src_action->param[1], BUFLEN_POKEMON_NAME);
      BEACONINFO_Set_PokemonEvolution(info, src_action->param[0], temp_nickname);
      GFL_STR_DeleteBuffer(temp_nickname);
    }
    break;
  case GAMEBEACON_ACTION_GPOWER:                   ///<Gパワー発動                           18
    BEACONINFO_Set_GPower(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_SP_ITEM_GET:              ///<貴重品ゲット                          19
    BEACONINFO_Set_SpItemGet(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_PLAYTIME:                 ///<一定のプレイ時間を越えた              20
    BEACONINFO_Set_PlayTime(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_ZUKAN_COMPLETE:           ///<図鑑完成                              21
    BEACONINFO_Set_ZukanComplete(info);
    break;
  case GAMEBEACON_ACTION_THANKYOU_OVER:            ///<お礼を受けた回数が規定数を超えた      22
    BEACONINFO_Set_ThankyouOver(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_UNION_IN:                 ///<ユニオンルームに入った                23
    BEACONINFO_Set_UnionIn(info);
    break;
  case GAMEBEACON_ACTION_THANKYOU:                 ///<「ありがとう！」                      24
    GF_ASSERT(0); //未対応
    //BEACONINFO_Set_Thankyou(GAMEBEACON_INFO *info, GAMEDATA *gamedata, u32 target_trainer_id);
    break;
  case GAMEBEACON_ACTION_DISTRIBUTION_POKE:        ///<ポケモン配布中                        25
    BEACONINFO_Set_DistributionPoke(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_DISTRIBUTION_ITEM:        ///<アイテム配布中                        26
    BEACONINFO_Set_DistributionItem(info, src_action->param[0]);
    break;
  case GAMEBEACON_ACTION_DISTRIBUTION_ETC:         ///<その他配布中                          27
    BEACONINFO_Set_DistributionEtc(info);
    break;
  default:
    GF_ASSERT(0);
    break;
  }
}

#endif  //PM_DEBUG
