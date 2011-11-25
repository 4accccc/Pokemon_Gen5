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

#include "gamesystem/gamesystem.h"

typedef enum{
  FM_CHECK_SCORE_FAILED,  //ミッションにエントリーしていないのでチェックできなかった
  FM_CHECK_SCORE_SUCCESS,  //スコアチェックが出来た(ミッションエントリー中)
  FM_CHECK_SCORE_SUCCESS_UP,  //スコアがあがった
}FM_CHECK_SCORE_RET;

#define GAMEBEACON_BUFLEN_FOR_ACTION  (10)  //アクションで送れる文字数バッファのサイズ(バッファが20byteなのでEOM抜き10文字まで)

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
  GAMESYS_WORK* gsys;
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
  u8 resort_enable;                   ///<リゾート用のデータ配信許可フラグ
  u8 resort_extend_index;             ///<リゾート現在送信中の拡張インデックス
  void *p_resort_buff;                ///<リゾート用拡張バッファ
  u8 resort_extend_count;            ///<リゾート用拡張切り替えカウント
  u8 resort_set_request;              ///<リゾート用データセットリクエスト

  u8  cgear_info_flags;               ///<調査隊の調査依頼を達成していることをCギアに通知(GAMEBEACON_CGEAR_INFO_FLAG)
  u8 resort_recv_request;             ///<リゾート用データ受信リクエスト

  GAMEBEACON_TYPE  beacon_type;       ///<typeがGAMEBEACON_TYPE_COMMONの時BWと互換性のあるビーコンを送信する
                                      ///<swanとbwではアンケートに互換性がないため、ビーコン送受信の1ターンごとに
                                      ///<swan専用ビーコンとbw互換のビーコンを交互に切り替えて発信する。
  
  STRBUF* sbuf_person_name;           ///<トレーナー名を格納可能なバッファ。フェスログ生成時に使用
  void  *p_resort_beacon_adrs;             ///<リゾート用ビーコンバッファアドレス
#ifdef PM_DEBUG
  BOOL  deb_beacon_priority_egnore;  //TRUEならビーコンリクエスト時にプライオリティ判定を無視する
#endif
}GAMEBEACON_SYSTEM;

///ゲームビーコン管理システムのポインタ
extern GAMEBEACON_SYSTEM * GameBeaconSys;


//--------------------------------------------------------------
/**
 * フェスミッションビーコンかどうかチェック 
 * @param   info	ビーコン情報へのポインタ
 *
 * @retval  TRUE  フェスミッションビーコン
 * @retval  FALSE フェスミッションビーコンではない
 */
//--------------------------------------------------------------
extern BOOL GAMEBEACON_Check_FestMission(const GAMEBEACON_INFO *info);

//--------------------------------------------------------------
/**
 * 送信ビーコンをセットした時の共通処理(基本全バージョン受け取り可)
 *
 * @param   send		
 * @param   info		
 */
//--------------------------------------------------------------
extern void GAMEBEACON_SUB_SendBeaconSetCommon(GAMEBEACON_SEND_MANAGER *send);

//--------------------------------------------------------------
/**
 * 送信ビーコンをセットした時の共通処理(SWAN以降のみ受け取り可)
 *
 *以以降降 @param   send		
 * @param   info		
 */
//--------------------------------------------------------------
extern void GAMEBEACON_SUB_SendBeaconSetCommonSwan(GAMEBEACON_SEND_MANAGER *send);

//--------------------------------------------------------------
/**
 * @brief フェスミッションビーコン送信用　ビーコンデータ生成共通処理
 *
 * INFOパラメータの生成→プレイヤーログ生成→プライオリティチェック→ビーコン送信リクエスト
 * までを一気に行う
 *
 * @param   send		
 * @param   action
 * @param   param
 */
//--------------------------------------------------------------
extern void GAMEBEACON_SUB_SendBeaconSetCommonForFMission(GAMEBEACON_SEND_MANAGER *send, u16 action, u32 param );
extern void GAMEBEACON_SUB_SendBeaconSetCommonForFMissionEx(GAMEBEACON_SEND_MANAGER *send, u16 action, u32 param1, u32 param2 );

//--------------------------------------------------------------
/**
 * @brief フェスミッションのエントリーチェックとスコア計算を行う
 *
 * @param   action      LINK_FESTIVAL_MissionScoreCalc()に引き渡す actionナンバー
 * @param   param       LINK_FESTIVAL_MissionScoreCalc()に引き渡すパラメータ
 *
 * @retval  FM_CHECK_SCORE_RET型
 */
//--------------------------------------------------------------
extern FM_CHECK_SCORE_RET GAMEBEACON_SUB_FMissionCheckScore( FMISSION_LOG_ACTION action, u16 param );

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
 * @brief 文字数がFullの場合はEOMを差し込まないビーコンアクション用独自コピー処理
 * ※EOM込みのバッファ長を確保できるものは、GFL_STR_GetStringCode()を使うこと
 *
 * @param   src_buf		  コピー元STRBUF
 * @param   dest		    コピー先STRCODEバッファ
 * @param   len	  	    コピーしたいSTRCODEバッファ長(=EOM抜きの文字数 GAMEBEACON_BUFLEN_FOR_ACTIONまで)
 */
//--------------------------------------------------------------
extern void GAMEBEACON_SUB_GetStringCodeForAction(const STRBUF *src, STRCODE *dest, u32 len );

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
void BEACONINFO_Set_FreeWord(GAMEBEACON_INFO *info, const STRBUF *free_word, u8 namein_shortcut);
void BEACONINFO_Set_ZoneChange(GAMEBEACON_INFO *info, ZONEID zone_id, const GAMEDATA *cp_gamedata);
void BEACONINFO_Set_ThanksRecvCount(GAMEBEACON_INFO *info, u32 count);
void BEACONINFO_Set_SuretigaiCount(GAMEBEACON_INFO *info, u32 count);
void BEACONINFO_Set_Pokewood(GAMEBEACON_INFO *info, u8 scenario_no, GAMEBEACON_ACTION action );
void BEACONINFO_Set_WBT(GAMEBEACON_INFO *info, WBTCUP tournament_id, u8 round, GAMEBEACON_ACTION action );
void BEACONINFO_Set_JoinAvenueShopping(GAMEBEACON_INFO *info );
void BEACONINFO_Set_InfinityDungeon(GAMEBEACON_INFO *info, u8 area_no, u16 zone_id, GAMEBEACON_ACTION action );

void BEACONINFO_Set_FMissionEventClear(GAMEBEACON_INFO *info, u16 ev_id );

void BEACONINFO_Set_Details_BattleWildPoke(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_Details_BattleSpPoke(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_Details_BattleTrainer(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_Details_BattleLeader(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_Details_BattleSpTrainer(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_Details_Walk(GAMEBEACON_INFO *info);
void BEACONINFO_Set_Details_IntroductionPms(GAMEBEACON_INFO *info, const PMS_DATA *pms);

