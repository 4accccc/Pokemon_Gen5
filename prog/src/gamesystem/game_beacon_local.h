//==============================================================================
/**
 * @file    game_beacon_local.h
 * @brief   ゲーム中のビーコン情報管理システム ローカルヘッダ
 * @author  matsuda & iwasawa
 * @date    2010.05.01
 *
 * game_beacon.c game_beacon_nw.c 以外への外部公開禁止！
 */
//==============================================================================

#pragma once


//==============================================================================
//  構造体定義
//==============================================================================
///1件のログデータ
typedef struct{
  GAMEBEACON_INFO info;
  u16 time;                           ///<最新のデータを受け取ってから経過しているフレーム数
  u16 padding;
}GAMEBEACON_LOG;

///送信ビーコン管理
typedef struct{
  GAMEBEACON_INFO info;               ///<送信ビーコン
  s16 life;                           ///<送信寿命
  u8 beacon_update;                   ///<TRUE:送信ビーコン更新
  u8 padding;
}GAMEBEACON_SEND_MANAGER;

///ゲームビーコン管理システムのポインタ
typedef struct{
  GAMEDATA *gamedata;
  GAMEBEACON_SEND_MANAGER send;       ///<送信ビーコン管理
  GAMEBEACON_LOG log[GAMEBEACON_SYSTEM_LOG_MAX]; ///<ログ(チェーンバッファ)
  u32 update_log;                     ///<更新のあったログ番号をbitで管理(32を超えたら変える)
  u32 log_count;                      ///<ログ全体の受信件数をカウント
  s8 start_log;                       ///<ログのチェーン開始位置
  s8 end_log;                         ///<ログのチェーン終端位置
  s8 log_num;                         ///<ログ件数
  u8 new_entry;                       ///<TRUE:新しい人物とすれ違った
  u16 backup_hour;                    ///<プレイ時間バックアップ
  u16 padding;
  
#ifdef PM_DEBUG
  BOOL  deb_beacon_priority_egnore;  //TRUEならビーコンリクエスト時にプライオリティ判定を無視する
#endif
}GAMEBEACON_SYSTEM;

///ゲームビーコン管理システムのポインタ
extern GAMEBEACON_SYSTEM * GameBeaconSys;


//--------------------------------------------------------------
/**
 * 送信ビーコンをセットした時の共通処理
 *
 * @param   send		
 * @param   info		
 */
//--------------------------------------------------------------
extern void GAMEBEACON_SUB_SendBeaconSetCommon(GAMEBEACON_SEND_MANAGER *send);

//--------------------------------------------------------------
/**
 * アクション番号のプライオリティチェック
 *
 * @param   action_no		
 *
 * @retval  BOOL		    TRUE:プライオリティが同等以上。　FALSE:プライオリティが下
 */
//--------------------------------------------------------------
extern BOOL GAMEBEACON_SUB_CheckPriority(u16 action_no);

//==================================================================
/**
 * ポケモン関連ビーコン タイプチェック
 *
 * @param   monsno
 */
//==================================================================
extern BOOL GAMEBEACON_SUB_PokeBeaconTypeIsSpecial(const monsno );

//--------------------------------------------------------------
/**
 * ニックネームのコピー　※文字数がFullの場合はEOMを差し込まないので独自コピー処理
 *
 * @param   nickname		    コピー元ニックネーム
 * @param   dest_nickname		ニックネームコピー先
 */
//--------------------------------------------------------------
extern void GAMEBEACON_SUB_StrbufNicknameCopy(const STRBUF *nickname, STRCODE *dest_nickname);

///////////////////////////////////////////////////////////////////////////
//ビーコンセット系
void BEACONINFO_Set_BattleWildPokeStart(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_BattleWildPokeVictory(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_BattleSpPokeStart(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_BattleSpPokeVictory(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_BattleTrainerStart(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleTrainerVictory(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleLeaderStart(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleLeaderVictory(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleSpTrainerStart(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleSpTrainerVictory(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleChampionStart(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleChampionVictory(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_WildPokemonGet(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_SpecialPokemonGet(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_PokemonLevelUp(GAMEBEACON_INFO *info, const STRBUF *nickname);
void BEACONINFO_Set_PokemonEvolution(GAMEBEACON_INFO *info, u16 monsno, const STRBUF *nickname);
void BEACONINFO_Set_GPower(GAMEBEACON_INFO *info, GPOWER_ID g_power_id);
void BEACONINFO_Set_PlayTime(GAMEBEACON_INFO *info, u32 hour);
void BEACONINFO_Set_SpItemGet(GAMEBEACON_INFO *info, u16 item);
void BEACONINFO_Set_ZukanComplete(GAMEBEACON_INFO *info);
void BEACONINFO_Set_ThankyouOver(GAMEBEACON_INFO *info, u32 thankyou_count);
void BEACONINFO_Set_UnionIn(GAMEBEACON_INFO *info);

#ifdef PM_DEBUG
void BEACONINFO_Set_DistributionPoke(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_DistributionItem(GAMEBEACON_INFO *info, u16 item);
void BEACONINFO_Set_DistributionEtc(GAMEBEACON_INFO *info);
void BEACONINFO_Set_DistributionGPower(GAMEBEACON_INFO *info, GPOWER_ID g_power_id);
#endif
void BEACONINFO_Set_CriticalHit(GAMEBEACON_INFO *info, const STRBUF *nickname);
void BEACONINFO_Set_CriticalDamage(GAMEBEACON_INFO *info, const STRBUF *nickname);
void BEACONINFO_Set_Escape(GAMEBEACON_INFO *info);
void BEACONINFO_Set_HPLittle(GAMEBEACON_INFO *info, const STRBUF *nickname);
void BEACONINFO_Set_PPLittle(GAMEBEACON_INFO *info, const STRBUF *nickname);
void BEACONINFO_Set_Dying(GAMEBEACON_INFO *info, const STRBUF *nickname);
void BEACONINFO_Set_StateIsAbnormal(GAMEBEACON_INFO *info, const STRBUF *nickname);
void BEACONINFO_Set_UseItem(GAMEBEACON_INFO *info, u16 item_no);
void BEACONINFO_Set_FieldSkill(GAMEBEACON_INFO *info, u16 wazano);
void BEACONINFO_Set_SodateyaEgg(GAMEBEACON_INFO *info);
void BEACONINFO_Set_EggHatch(GAMEBEACON_INFO *info, u16 monsno );
void BEACONINFO_Set_Shoping(GAMEBEACON_INFO *info);
void BEACONINFO_Set_Subway(GAMEBEACON_INFO *info);
void BEACONINFO_Set_SubwayStraightVictories(GAMEBEACON_INFO *info, u32 victory_count);
void BEACONINFO_Set_SubwayTrophyGet(GAMEBEACON_INFO *info);
void BEACONINFO_Set_TrialHouse(GAMEBEACON_INFO *info);
void BEACONINFO_Set_TrialHouseRank(GAMEBEACON_INFO *info, u8 rank);
void BEACONINFO_Set_FerrisWheel(GAMEBEACON_INFO *info);
void BEACONINFO_Set_PokeShifter(GAMEBEACON_INFO *info);
void BEACONINFO_Set_Musical(GAMEBEACON_INFO *info, const STRBUF *nickname);
void BEACONINFO_Set_OtherGPowerUse(GAMEBEACON_INFO *info, GPOWER_ID gpower_id);
void BEACONINFO_Set_Thankyou(GAMEBEACON_INFO *info, GAMEDATA *gamedata, u32 target_trainer_id);
void BEACONINFO_Set_FreeWord(GAMEBEACON_INFO *info, const STRBUF *free_word);
void BEACONINFO_Set_ZoneChange(GAMEBEACON_INFO *info, ZONEID zone_id, const GAMEDATA *cp_gamedata);
void BEACONINFO_Set_ThanksRecvCount(GAMEBEACON_INFO *info, u32 count);
void BEACONINFO_Set_SuretigaiCount(GAMEBEACON_INFO *info, u32 count);

void BEACONINFO_Set_Details_BattleWildPoke(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_Details_BattleSpPoke(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_Details_BattleTrainer(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_Details_BattleLeader(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_Details_BattleSpTrainer(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_Details_Walk(GAMEBEACON_INFO *info);
void BEACONINFO_Set_Details_IntroductionPms(GAMEBEACON_INFO *info, const PMS_DATA *pms);

