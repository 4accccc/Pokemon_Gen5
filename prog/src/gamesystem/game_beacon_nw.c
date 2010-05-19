//==============================================================================
/**
 * @file    game_beacon_nw.c
 * @brief   ゲーム中のビーコン情報管理システム(NOT_WIFIオーバーレイ)
 * @author  matsuda & iwasawa
 * @date    2010.05.01
 */
//==============================================================================
#include <gflib.h>
#include "gamesystem/game_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "msg/msg_beacon_status.h"
#include "savedata/encount_sv.h"
#include "savedata/misc.h"
#include "net/net_whpipe.h"
#include "savedata/my_pms_data.h"
#include "app/townmap_util.h"
#include "net_app\union\union_beacon_tool.h"
#include "app/research_radar/research_data.h"   //QUESTION_NUM_PER_TOPIC
#include "app/research_radar/question_id.h"
#include "app/research_radar/questionnaire_index.h"
#include "savedata/playtime.h"
#include "field/research_team_def.h"

#include "game_beacon_local.h"

//==============================================================================
//  プロトタイプ宣言
//==============================================================================

//==============================================================================
//
//  ビーコンセット
//
//==============================================================================
//==================================================================
/**
 * 送信ビーコンセット：ポケモンと対戦を開始しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_BattlePokeStart(u16 monsno)
{
  if( GAMEBEACON_SUB_PokeBeaconTypeIsSpecial( monsno )){
    GAMEBEACON_Set_BattleSpPokeStart(monsno);
  }else{
    GAMEBEACON_Set_BattleWildPokeStart(monsno);
  }
}

//==================================================================
/**
 * 送信ビーコンセット：ポケモンとの対戦に勝利しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_BattlePokeVictory(u16 monsno)
{
  if( GAMEBEACON_SUB_PokeBeaconTypeIsSpecial( monsno )){
    GAMEBEACON_Set_BattleSpPokeVictory(monsno);
  }else{
    GAMEBEACON_Set_BattleWildPokeVictory(monsno);
  }
}

//==================================================================
/**
 * 送信ビーコンセット：野生ポケモンと対戦を開始しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_BattleWildPokeStart(u16 monsno)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_WILD_POKE_START) == FALSE){ return; };
  BEACONINFO_Set_BattleWildPokeStart(&GameBeaconSys->send.info, monsno);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：野生のポケモンと対戦を開始しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_BattleWildPokeStart(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_WILD_POKE_START;
  info->action.monsno = monsno;
 
  BEACONINFO_Set_Details_BattleWildPoke(info, monsno);
}

//==================================================================
/**
 * 送信ビーコンセット：野生のポケモンとの対戦に勝利しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_BattleWildPokeVictory(u16 monsno)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_WILD_POKE_VICTORY) == FALSE){ return; };
  BEACONINFO_Set_BattleWildPokeVictory(&GameBeaconSys->send.info, monsno);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：野生のポケモンとの対戦に勝利しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_BattleWildPokeVictory(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_WILD_POKE_VICTORY;
  info->action.monsno = monsno;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：特別なポケモンと対戦を開始しました
 * @param   tr_no   ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_BattleSpPokeStart(u16 monsno)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_SP_POKE_START) == FALSE){ return; };
  BEACONINFO_Set_BattleSpPokeStart(&GameBeaconSys->send.info, monsno);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：特別なポケモンと対戦を開始しました
 * @param   tr_no   ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_BattleSpPokeStart(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_SP_POKE_START;
  info->action.monsno = monsno;

  BEACONINFO_Set_Details_BattleSpPoke(info, monsno);
}

//==================================================================
/**
 * 送信ビーコンセット：特別なポケモンとの対戦に勝利しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_BattleSpPokeVictory(u16 monsno)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_SP_POKE_VICTORY) == FALSE){ return; };
  BEACONINFO_Set_BattleSpPokeVictory(&GameBeaconSys->send.info, monsno);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：特別なポケモンとの対戦に勝利しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_BattleSpPokeVictory(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_SP_POKE_VICTORY;
  info->action.monsno = monsno;
  
  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：トレーナーと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleTrainerStart(u16 tr_no)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_TRAINER_START) == FALSE){ return; };
  BEACONINFO_Set_BattleTrainerStart(&GameBeaconSys->send.info, tr_no);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：トレーナーと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleTrainerStart(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_TRAINER_START;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_BattleTrainer(info, tr_no);
}

//==================================================================
/**
 * 送信ビーコンセット：トレーナーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleTrainerVictory(u16 tr_no)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_TRAINER_VICTORY) == FALSE){ return; };
  BEACONINFO_Set_BattleTrainerVictory(&GameBeaconSys->send.info, tr_no);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：トレーナーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleTrainerVictory(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_TRAINER_VICTORY;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：ジムリーダーと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleLeaderStart(u16 tr_no)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_LEADER_START) == FALSE){ return; };
  BEACONINFO_Set_BattleLeaderStart(&GameBeaconSys->send.info, tr_no);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：トレーナーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleLeaderStart(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_LEADER_START;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_BattleLeader(info, tr_no);
}

//==================================================================
/**
 * 送信ビーコンセット：ジムリーダーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleLeaderVictory(u16 tr_no)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_LEADER_VICTORY) == FALSE){ return; };
  BEACONINFO_Set_BattleLeaderVictory(&GameBeaconSys->send.info, tr_no);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ジムリーダーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleLeaderVictory(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_LEADER_VICTORY;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：特別なトレーナーと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleSpTrainerStart(u16 tr_no)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_SP_TRAINER_START) == FALSE){ return; };
  BEACONINFO_Set_BattleSpTrainerStart(&GameBeaconSys->send.info, tr_no);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：特別なトレーナーと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleSpTrainerStart(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_SP_TRAINER_START;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_BattleSpTrainer(info, tr_no);
}

//==================================================================
/**
 * 送信ビーコンセット：特別なトレーナーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleSpTrainerVictory(u16 tr_no)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_SP_TRAINER_VICTORY) == FALSE){ return; };
  BEACONINFO_Set_BattleSpTrainerVictory(&GameBeaconSys->send.info, tr_no);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：特別なトレーナーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleSpTrainerVictory(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_SP_TRAINER_VICTORY;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：チャンピオンと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleChampionStart(u16 tr_no)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_CHAMPION_START) == FALSE){ return; };
  BEACONINFO_Set_BattleChampionStart(&GameBeaconSys->send.info, tr_no);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：チャンピオンと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleChampionStart(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_CHAMPION_START;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_BattleSpTrainer(info, tr_no);
}

//==================================================================
/**
 * 送信ビーコンセット：チャンピオンとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleChampionVictory(u16 tr_no)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_BATTLE_CHAMPION_VICTORY) == FALSE){ return; };
  BEACONINFO_Set_BattleChampionVictory(&GameBeaconSys->send.info, tr_no);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：チャンピオンとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleChampionVictory(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_CHAMPION_VICTORY;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * Gパワー発動
 *
 * @param   g_power_id		GパワーID
 */
//==================================================================
void GAMEBEACON_Set_GPower(GPOWER_ID g_power_id)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_GPOWER) == FALSE){ return; };
  BEACONINFO_Set_GPower(&GameBeaconSys->send.info, g_power_id);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * Gパワー発動
 *
 * @param   g_power_id		GパワーID
 */
//==================================================================
void BEACONINFO_Set_GPower(GAMEBEACON_INFO *info, GPOWER_ID g_power_id)
{
  info->action.action_no = GAMEBEACON_ACTION_GPOWER;
  info->g_power_id = g_power_id;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：フィールドに落ちているアイテムをプレイヤーが入手
 *
 * @param   item		アイテム番号
 */
//==================================================================
void GAMEBEACON_Set_SpItemGet(u16 item)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_SP_ITEM_GET) == FALSE){ return; };
  BEACONINFO_Set_SpItemGet(&GameBeaconSys->send.info, item);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：フィールドに落ちているアイテムをプレイヤーが入手
 *
 * @param   item		アイテム番号
 */
//==================================================================
void BEACONINFO_Set_SpItemGet(GAMEBEACON_INFO *info, u16 item)
{
  info->action.action_no = GAMEBEACON_ACTION_SP_ITEM_GET;
  info->action.itemno = item;

  BEACONINFO_Set_Details_Walk(info);
}
//==================================================================
/**
 * 送信ビーコンセット：ポケモン図鑑が完成
 */
//==================================================================
void GAMEBEACON_Set_ZukanComplete(void)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_ZUKAN_COMPLETE) == FALSE){ return; };
  BEACONINFO_Set_ZukanComplete(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ポケモン図鑑が完成
 */
//==================================================================
void BEACONINFO_Set_ZukanComplete(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_ZUKAN_COMPLETE;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：お礼を受けた回数が規定数を超えた
 * @param   thankyou_count	お礼回数
 */
//==================================================================
void GAMEBEACON_Set_ThankyouOver(u32 thankyou_count)
{
  static const u8 regu_count[] = {10, 30, 50, 100, 200};
  int i;
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_THANKYOU_OVER) == FALSE){ return; };
  for(i = 0; i < NELEMS(regu_count); i++){
    if(thankyou_count == regu_count[i]){
      BEACONINFO_Set_ThankyouOver(&GameBeaconSys->send.info, thankyou_count);
      GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
      return;
    }
  }
}

//==================================================================
/**
 * ビーコンセット：お礼を受けた回数が規定数を超えた
 * @param   thankyou_count	お礼回数
 */
//==================================================================
void BEACONINFO_Set_ThankyouOver(GAMEBEACON_INFO *info, u32 thankyou_count)
{
  info->action.action_no = GAMEBEACON_ACTION_THANKYOU_OVER;
  info->action.thankyou_count = thankyou_count;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：ユニオンルーム入室
 */
//==================================================================
void GAMEBEACON_Set_UnionIn(void)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_UNION_IN) == FALSE){ return; };
  BEACONINFO_Set_UnionIn(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ユニオンルーム入室
 */
//==================================================================
void BEACONINFO_Set_UnionIn(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_UNION_IN;

  BEACONINFO_Set_Details_Walk(info);
}
//==================================================================
/**
 * 送信ビーコンセット：戦闘から逃げ出した
 */
//==================================================================
void GAMEBEACON_Set_Escape(void)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_ESCAPE) == FALSE){ return; };
  BEACONINFO_Set_Escape(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：戦闘から逃げ出した
 */
//==================================================================
void BEACONINFO_Set_Escape(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_ESCAPE;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：戦闘後、先頭のポケモンのHPが5割以上少なくなっている
 *
 * @param   nickname		対象のポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_HPLittle(const STRBUF *nickname)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_HP_LITTLE) == FALSE){ return; };
  BEACONINFO_Set_HPLittle(&GameBeaconSys->send.info, nickname);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * 送信ビーコンセット：戦闘後、先頭のポケモンのHPが5割以上少なくなっている
 *
 * @param   info		
 * @param   nickname		対象のポケモンのニックネーム
 */
//==================================================================
void BEACONINFO_Set_HPLittle(GAMEBEACON_INFO *info, const STRBUF *nickname)
{
  info->action.action_no = GAMEBEACON_ACTION_HP_LITTLE;
  GAMEBEACON_SUB_StrbufNicknameCopy(nickname, info->action.normal.nickname);

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：戦闘後、先頭のポケモンのPPが1つでも0になっている
 *
 * @param   nickname		対象のポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_PPLittle(const STRBUF *nickname)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_PP_LITTLE) == FALSE){ return; };
  BEACONINFO_Set_PPLittle(&GameBeaconSys->send.info, nickname);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * 送信ビーコンセット：戦闘後、先頭のポケモンのPPが1つでも0になっている
 *
 * @param   info		
 * @param   nickname		対象のポケモンのニックネーム
 */
//==================================================================
void BEACONINFO_Set_PPLittle(GAMEBEACON_INFO *info, const STRBUF *nickname)
{
  info->action.action_no = GAMEBEACON_ACTION_PP_LITTLE;
  GAMEBEACON_SUB_StrbufNicknameCopy(nickname, info->action.normal.nickname);

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：戦闘後、戦闘のポケモンが瀕死
 *
 * @param   nickname		対象のポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_Dying(const STRBUF *nickname)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_DYING) == FALSE){ return; };
  BEACONINFO_Set_Dying(&GameBeaconSys->send.info, nickname);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * 送信ビーコンセット：戦闘後、先頭のポケモンが瀕死
 *
 * @param   info		
 * @param   nickname		対象のポケモンのニックネーム
 */
//==================================================================
void BEACONINFO_Set_Dying(GAMEBEACON_INFO *info, const STRBUF *nickname)
{
  info->action.action_no = GAMEBEACON_ACTION_DYING;
  GAMEBEACON_SUB_StrbufNicknameCopy(nickname, info->action.normal.nickname);

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：戦闘後、戦闘のポケモンが状態異常
 *
 * @param   nickname		対象のポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_StateIsAbnormal(const STRBUF *nickname)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_STATE_IS_ABNORMAL) == FALSE){ return; };
  BEACONINFO_Set_StateIsAbnormal(&GameBeaconSys->send.info, nickname);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * 送信ビーコンセット：戦闘後、先頭のポケモンが状態異常
 *
 * @param   info		
 * @param   nickname		対象のポケモンのニックネーム
 */
//==================================================================
void BEACONINFO_Set_StateIsAbnormal(GAMEBEACON_INFO *info, const STRBUF *nickname)
{
  info->action.action_no = GAMEBEACON_ACTION_STATE_IS_ABNORMAL;
  GAMEBEACON_SUB_StrbufNicknameCopy(nickname, info->action.normal.nickname);

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：フィールド技を使用
 *
 * @param   wazano    技番号
 */
//==================================================================
void GAMEBEACON_Set_FieldSkill(u16 wazano)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_FIELD_SKILL) == FALSE){ return; };
  BEACONINFO_Set_FieldSkill(&GameBeaconSys->send.info, wazano);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：フィールド技を使用
 *
 * @param   info		
 */
//==================================================================
void BEACONINFO_Set_FieldSkill(GAMEBEACON_INFO *info, u16 wazano)
{
  info->action.action_no = GAMEBEACON_ACTION_FIELD_SKILL;
  info->action.wazano = wazano;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：タマゴを育て屋から受け取った
 */
//==================================================================
void GAMEBEACON_Set_SodateyaEgg( void )
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_SODATEYA_EGG) == FALSE){ return; };
  BEACONINFO_Set_SodateyaEgg(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：タマゴを育て屋から受け取った
 *
 * @param   info		
 */
//==================================================================
void BEACONINFO_Set_SodateyaEgg(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_SODATEYA_EGG;
  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：タマゴが孵化
 *
 * @param   monsno		孵化したタマゴのポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_EggHatch( u16 monsno )
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_EGG_HATCH) == FALSE){ return; };
  BEACONINFO_Set_EggHatch(&GameBeaconSys->send.info, monsno );
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * 送信ビーコンセット：タマゴが孵化
 *
 * @param   info		
 * @param   monsno		孵化したタマゴのポケモン番号
 */
//==================================================================
void BEACONINFO_Set_EggHatch(GAMEBEACON_INFO *info, u16 monsno )
{
  info->action.action_no = GAMEBEACON_ACTION_EGG_HATCH;
  info->action.monsno = monsno;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：買い物中
 */
//==================================================================
void GAMEBEACON_Set_Shoping(void)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_SHOPING) == FALSE){ return; };
  BEACONINFO_Set_Shoping(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：買い物中
 */
//==================================================================
void BEACONINFO_Set_Shoping(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_SHOPING;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：バトルサブウェイ挑戦中
 */
//==================================================================
void GAMEBEACON_Set_Subway(void)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_SUBWAY) == FALSE){ return; };
  BEACONINFO_Set_Subway(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：バトルサブウェイ挑戦中
 */
//==================================================================
void BEACONINFO_Set_Subway(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_SUBWAY;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：バトルサブウェイ連勝中
 *
 * @param   victory_count		連勝数
 */
//==================================================================
void GAMEBEACON_Set_SubwayStraightVictories(u32 victory_count)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_SUBWAY_STRAIGHT_VICTORIES) == FALSE){ return; };
  BEACONINFO_Set_SubwayStraightVictories(&GameBeaconSys->send.info, victory_count);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：バトルサブウェイ連勝中(ラウンド数)
 *
 * @param   victory_count		勝ち進んだラウンド数
 *
 * 記録として伸びていく<連勝数>ではなく、7戦1セットの何勝目かを1～7で渡してください
 */
//==================================================================
void BEACONINFO_Set_SubwayStraightVictories(GAMEBEACON_INFO *info, u32 victory_count)
{
  info->action.action_no = GAMEBEACON_ACTION_SUBWAY_STRAIGHT_VICTORIES;
  info->action.victory_count = victory_count;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：バトルサブウェイでトロフィー取得
 *
 * @param   victory_count		連勝数
 */
//==================================================================
void GAMEBEACON_Set_SubwayTrophyGet( void )
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_SUBWAY_TROPHY_GET) == FALSE){ return; };
  BEACONINFO_Set_SubwayTrophyGet(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：バトルサブウェイでトロフィーゲット
 *
 * @param   victory_count		連勝数
 */
//==================================================================
void BEACONINFO_Set_SubwayTrophyGet( GAMEBEACON_INFO *info )
{
  info->action.action_no = GAMEBEACON_ACTION_SUBWAY_TROPHY_GET;
  info->action.victory_count = 0;
  
  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：トライアルハウスに挑戦中
 */
//==================================================================
void GAMEBEACON_Set_TrialHouse(void)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_TRIALHOUSE) == FALSE){ return; };
  BEACONINFO_Set_TrialHouse(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：トライアルハウスに挑戦中
 */
//==================================================================
void BEACONINFO_Set_TrialHouse(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_TRIALHOUSE;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：トライアルハウスでランク確定
 *
 * @param   rank      TH_RANK_xxx
 */
//==================================================================
void GAMEBEACON_Set_TrialHouseRank(u8 rank)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_TRIALHOUSE_RANK) == FALSE){ return; };
  BEACONINFO_Set_TrialHouseRank(&GameBeaconSys->send.info, rank);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：トライアルハウスでランク確定
 *
 * @param   info		
 * @param   rank		TH_RANK_xxx
 */
//==================================================================
void BEACONINFO_Set_TrialHouseRank(GAMEBEACON_INFO *info, u8 rank)
{
  info->action.action_no = GAMEBEACON_ACTION_TRIALHOUSE_RANK;
  info->action.trial_house_rank = rank;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：観覧車に乗った
 */
//==================================================================
void GAMEBEACON_Set_FerrisWheel(void)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_FERRIS_WHEEL) == FALSE){ return; };
  BEACONINFO_Set_FerrisWheel(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：観覧車に乗った
 */
//==================================================================
void BEACONINFO_Set_FerrisWheel(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_FERRIS_WHEEL;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：ポケシフター入室
 */
//==================================================================
void GAMEBEACON_Set_PokeShifter(void)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_POKESHIFTER) == FALSE){ return; };
  BEACONINFO_Set_PokeShifter(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ポケシフター入室
 */
//==================================================================
void BEACONINFO_Set_PokeShifter(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_POKESHIFTER;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：ミュージカル入室
 *
 * @param   nickname		参加したポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_Musical(const STRBUF *nickname)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_MUSICAL) == FALSE){ return; };
  BEACONINFO_Set_Musical(&GameBeaconSys->send.info, nickname);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ミュージカル入室
 *
 * @param   info		
 * @param   nickname		参加したポケモンのニックネーム
 */
//==================================================================
void BEACONINFO_Set_Musical(GAMEBEACON_INFO *info, const STRBUF *nickname)
{
  info->action.action_no = GAMEBEACON_ACTION_MUSICAL;
  GAMEBEACON_SUB_StrbufNicknameCopy(nickname, info->action.normal.nickname);

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：他人のプレイヤーから受け取ったGパワーを使用
 *
 * @param   gpower_id		GパワーID
 */
//==================================================================
void GAMEBEACON_Set_OtherGPowerUse(GPOWER_ID gpower_id)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_OTHER_GPOWER_USE) == FALSE){ return; };
  BEACONINFO_Set_OtherGPowerUse(&GameBeaconSys->send.info, gpower_id);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：他人のプレイヤーから受け取ったGパワーを使用
 *
 * @param   info		
 * @param   gpower_id		GパワーID
 */
//==================================================================
void BEACONINFO_Set_OtherGPowerUse(GAMEBEACON_INFO *info, GPOWER_ID gpower_id)
{
  info->action.action_no = GAMEBEACON_ACTION_OTHER_GPOWER_USE;
  info->action.gpower_id = gpower_id;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：「ありがとう！」
 *
 * @param   gamedata		
 * @param   target_trainer_id   お礼をする相手のトレーナーID
 */
//==================================================================
void GAMEBEACON_Set_Thankyou(GAMEDATA *gamedata, u32 target_trainer_id)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_THANKYOU) == FALSE){ return; };
  BEACONINFO_Set_Thankyou(&GameBeaconSys->send.info, gamedata, target_trainer_id);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：「ありがとう！」
 *
 * @param   gamedata		
 * @param   target_trainer_id   お礼をする相手のトレーナーID
 */
//==================================================================
void BEACONINFO_Set_Thankyou(GAMEBEACON_INFO *info, GAMEDATA *gamedata, u32 target_trainer_id)
{
  const MISC *misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(gamedata) );

  STRTOOL_Copy(MISC_CrossComm_GetThankyouMessage(misc), 
  info->action.thanks.thankyou_message, GAMEBEACON_THANKYOU_MESSAGE_LEN);
  info->action.action_no = GAMEBEACON_ACTION_THANKYOU;
  info->action.thanks.target_trainer_id = target_trainer_id;

//  詳細は直前のを維持
}

//==================================================================
/**
 * 送信ビーコンセット：一言メッセージ
 *
 * @param   gamedata		
 * @param   free_word   一言メッセージ
 */
//==================================================================
void GAMEBEACON_Set_FreeWord( const STRBUF *free_word )
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_FREEWORD) == FALSE){ return; };
  BEACONINFO_Set_FreeWord(&GameBeaconSys->send.info, free_word);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：一言メッセージ
 *
 * @param   gamedata		
 * @param   free_word   一言メッセージ
 */
//==================================================================
void BEACONINFO_Set_FreeWord(GAMEBEACON_INFO *info, const STRBUF *free_word)
{
  GFL_STR_GetStringCode(free_word, info->action.freeword_message, GAMEBEACON_FREEWORD_MESSAGE_LEN);
  info->action.action_no = GAMEBEACON_ACTION_FREEWORD;
  
  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信その他セット：お礼を受けた回数
 *
 * @param   count		
 */
//==================================================================
static void GAMEBEACON_Set_ThanksRecvCount(u32 count)
{
  BEACONINFO_Set_ThanksRecvCount(&GameBeaconSys->send.info, count);
}

//==================================================================
/**
 * その他セット：お礼を受けた回数
 *
 * @param   count		
 */
//==================================================================
void BEACONINFO_Set_ThanksRecvCount(GAMEBEACON_INFO *info, u32 count)
{
  info->thanks_recv_count = count;
}

//==================================================================
/**
 * 送信その他セット：すれ違った人数
 *
 * @param   count		
 */
//==================================================================
static void GAMEBEACON_Set_SuretigaiCount(u32 count)
{
  BEACONINFO_Set_SuretigaiCount(&GameBeaconSys->send.info, count);
}

//==================================================================
/**
 * その他セット：すれ違った人数
 *
 * @param   count		
 */
//==================================================================
void BEACONINFO_Set_SuretigaiCount(GAMEBEACON_INFO *info, u32 count)
{
  info->suretigai_count = count;
}

//==============================================================================
//  詳細パラメータ
//==============================================================================

