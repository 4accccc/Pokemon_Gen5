//============================================================================================
/**
 * @file  record.h
 * @brief
 * @date  2006.03.28
 * @author  tamada/iwasawa
 */
//============================================================================================

#ifndef __RECORD_H__
#define __RECORD_H__

#ifndef __ASM_NO_DEF_

#include "savedata/save_control.h"
#include "score_def.h"

//============================================================================================
//
//  RECORDを操作するのに必要な定義
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief RECORDへの不完全型定義
 */
//----------------------------------------------------------
typedef struct RECORD RECORD;

//----------------------------------------------------------
//----------------------------------------------------------
//レコードデータの取得
extern RECORD * SaveData_GetRecord(SAVE_CONTROL_WORK * sv);

extern void RECORD_Init(RECORD * rec);
extern int RECORD_GetWorkSize(void);

//----------------------------------------------------------
//----------------------------------------------------------
//値をセットする
extern u32 RECORD_Set(RECORD * rec, int id, u32 value);
//現在の値と比較、大きければ値をセットする
extern u32 RECORD_SetIfLarge(RECORD * rec, int id, u32 value);
//値をインクリメントする
extern u32 RECORD_Inc(RECORD * rec, int id);
//値を取得する
extern u32 RECORD_Get(RECORD * rec, int id);
//値を足す
extern u32 RECORD_Add(RECORD * rec, int id, u32 add_value);

extern void RECORD_SetThScore(RECORD * rec, u32 inScore);
extern u32 RECORD_GetThScore(RECORD * rec);
extern void RECORD_SetThRank(RECORD * rec, u8 inRank);
extern u8 RECORD_GetThRank(RECORD * rec);

#ifdef PM_DEBUG
//スコアのセット
extern void RECORD_Score_DebugSet(RECORD * rec, const u32 inScore);
#endif

//----------------------------------------------------------
//----------------------------------------------------------
//スコアの加算処理
extern void RECORD_Score_Add(RECORD * rec, int id);
//スコアを取得する
extern u32 RECORD_Score_Get(RECORD * rec);

//FIXME 図鑑未対応
//図鑑スコア加算
//extern void RECORD_Score_AddZukanScore(RECORD * rec, const ZUKAN_WORK * zw, u16 const inMonsno);

#endif  //__ASM_NO_DEF_


//============================================================================================
//
//  RECORD各項目の定義
//
//============================================================================================
//----------------------------------------------------------
//----------------------------------------------------------
#define REC_FIRST   0

//暗号化から外す為、歩数カウンタを先頭に持ってくる
#define RECID_WALK_COUNT        (REC_FIRST) ///< @歩数

/**
 *  @brief  スコア取得指定ID(4byte)
 *
 *  値の加算には必ずRECORD_Score_Add()関数
 *  値の参照には必ずRECORD_Score_Get()関数を使用すること
 *
 *  RECORD_Add()/RECORD_Get()に直接指定して値を更新/参照することは禁止です
*/

#define LARGE_REC_START   1

//以下は、4バイトのカウンタを持つ
#define RECID_REPORT_COUNT      (LARGE_REC_START+ 0)   ///< @レポート回数
#define RECID_CLEAR_TIME        (LARGE_REC_START+ 1)   ///< @クリアタイム
#define RECID_RIDE_CYCLE        (LARGE_REC_START+ 2)   ///< @自転車に乗った回数
#define RECID_BATTLE_COUNT      (LARGE_REC_START+ 3)   ///< @全戦闘回数
#define RECID_BTL_ENCOUNT       (LARGE_REC_START+ 4)   ///< @エンカウント戦闘回数
#define RECID_BTL_TRAINER       (LARGE_REC_START+ 5)   ///< @トレーナー戦闘回数

#define RECID_CAPTURE_POKE      (LARGE_REC_START+ 6)   ///< @捕獲したポケモンの数
#define RECID_FISHING_SUCCESS   (LARGE_REC_START+ 7)   ///< @釣りあげたポケモンの回数
#define RECID_TAMAGO_HATCHING   (LARGE_REC_START+ 8)  ///< @タマゴを孵した回数
#define RECID_POKE_EVOLUTION    (LARGE_REC_START+ 9)  ///< @進化させたポケモンの引数
#define RECID_RECOVERY_PC       (LARGE_REC_START+10)  ///< @ポケセンで休んだ回数

#define RECID_COMM_TRADE        (LARGE_REC_START+11)  ///< @ワイヤレス通信交換をした回数
#define RECID_COMM_BATTLE       (LARGE_REC_START+12)  ///< @ワイヤレス通信対戦をした回数
#define RECID_COMM_BTL_WIN      (LARGE_REC_START+13)  ///< @ワイヤレス通信対戦の勝利数
#define RECID_COMM_BTL_LOSE     (LARGE_REC_START+14)  ///< @ワイヤレス通信対戦の敗北数

#define RECID_WIFI_TRADE        (LARGE_REC_START+15)  ///< @WiFi通信交換をした回数
#define RECID_WIFI_BATTLE       (LARGE_REC_START+16)  ///< @WiFi通信対戦をした回数
#define RECID_WIFI_BTL_WIN      (LARGE_REC_START+17)  ///< @WiFi通信対戦の勝利数
#define RECID_WIFI_BTL_LOSE     (LARGE_REC_START+18)  ///< @WiFi通信対戦の敗北数
#define RECID_WIFI_BTL_DRAW     (LARGE_REC_START+19)  ///< @WiFi通信対戦の引き分け数

#define RECID_SHOPPING_CNT      (LARGE_REC_START+20)  ///< @買い物をした回数
#define RECID_SHOPPING_MONEY    (LARGE_REC_START+21)  ///< @買い物をした合計金額
#define RECID_WATCH_TV          (LARGE_REC_START+22)  ///< @テレビを見た回数
#define RECID_SODATEYA_CNT      (LARGE_REC_START+23)  ///< @育て屋にポケモンを預けた回数

#define RECID_KILL_POKE         (LARGE_REC_START+24)  ///< @倒したポケモンの数
#define RECID_DAYMAX_EXP        (LARGE_REC_START+25)  ///< @1日に稼いだ経験値最大値
#define RECID_DAYCNT_EXP        (LARGE_REC_START+26)  ///< @1日に稼いだ経験値

#define RECID_GTS_PUT           (LARGE_REC_START+27)  ///< @GTSにポケモンを預けた回数
#define RECID_MAIL_WRITE        (LARGE_REC_START+28)  ///< @メールを書いた回数
#define RECID_NICKNAME          (LARGE_REC_START+29)  ///< @ポケモンにニックネームを付けた回数
#define RECID_PREMIUM_BALL      (LARGE_REC_START+30)  ///< @もらったプレミアボールの数
#define RECID_LEADERHOUSE_BATTLE  (LARGE_REC_START+31)  ///< @リーダーハウスで戦った回数
#define RECID_WIN_BP            (LARGE_REC_START+32)  ///< @勝ち取ったBP
#define RECID_USE_BP            (LARGE_REC_START+33)  ///< @使ったBP

#define RECID_SURECHIGAI_NUM    (LARGE_REC_START+34)  ///< @すれちがい通信をした回数
#define RECID_IRC_TRADE         (LARGE_REC_START+35)  ///< @赤外線通信交換をした回数
#define RECID_IRC_BATTLE        (LARGE_REC_START+36)  ///< @赤外線通信対戦をした回数
#define RECID_IRC_BTL_WIN       (LARGE_REC_START+37)  ///< @赤外線通信対戦の勝利数
#define RECID_IRC_BTL_LOSE      (LARGE_REC_START+38)  ///< @赤外線通信対戦の敗北数
#define RECID_PALACE_TIME       (LARGE_REC_START+39)  ///< @パレス訪問時間合計
#define RECID_PALACE_MISSION     (LARGE_REC_START+40)  ///< @パレスクリアしたミッションの数

#define LARGE_REC_NUM           (70+LARGE_REC_START)  ///< WBで41+1個使用。残り29個
#define LARGE_REC_MAX           (LARGE_REC_NUM) 

//以下は、2バイトしかカウンタを持たない
#define SMALL_REC_START   LARGE_REC_MAX

#define RECID_DENDOU_CNT          (SMALL_REC_START+ 0) ///<@殿堂入りした回数
#define RECID_RECOVERY_HOME       (SMALL_REC_START+ 1) ///<@自分の家で休んだ回数

#define RECID_WAZA_HANERU         (SMALL_REC_START+ 2) ///< @「はねる」を使った回数
#define RECID_WAZA_WARUAGAKI      (SMALL_REC_START+ 3) ///< @「わるあがき」を使った回数

#define RECID_WAZA_MUKOU          (SMALL_REC_START+ 4)  ///< @無効果わざを出した回数
#define RECID_TEMOTI_KIZETU       (SMALL_REC_START+ 5)  ///< @気絶させた手持ちの引数
#define RECID_TEMOTI_MAKIZOE      (SMALL_REC_START+ 6)  ///< @味方にダメージを与えた回数
#define RECID_NIGERU_SIPPAI       (SMALL_REC_START+ 7)  ///< @逃げそこなった回数
#define RECID_NIGERARETA          (SMALL_REC_START+ 8)  ///< @逃げられたポケモンの数
#define RECID_FISHING_FAILURE     (SMALL_REC_START+ 9)  ///< @つり損なった回数
#define RECID_DAYMAX_KILL         (SMALL_REC_START+10)  ///< @1日に倒したポケモンの最高数
#define RECID_DAYCNT_KILL         (SMALL_REC_START+11)  ///< @1日に倒したポケモンの数
#define RECID_DAYMAX_CAPTURE      (SMALL_REC_START+12)  ///< @1日に捕獲したポケモンの最高数
#define RECID_DAYCNT_CAPTURE      (SMALL_REC_START+13)  ///< @1日に捕獲したポケモンの数
#define RECID_DAYMAX_TRAINER_BATTLE   (SMALL_REC_START+14)  ///< @1日にトレーナー戦した最高回数
#define RECID_DAYCNT_TRAINER_BATTLE   (SMALL_REC_START+15)  ///< @1日にトレーナー戦した回数
#define RECID_DAYMAX_EVOLUTION    (SMALL_REC_START+16)  ///< @1日にポケモン進化させた最高回数
#define RECID_DAYCNT_EVOLUTION    (SMALL_REC_START+17)  ///< @1日にポケモン進化させた回数
#define RECID_FOSSIL_RESTORE      (SMALL_REC_START+18)  ///< @ポケモンの化石を復元した回数
#define RECID_GURUGURU_COUNT      (SMALL_REC_START+19)  ///< @ぐるぐる交換をした回数

#define RECID_RANDOM_FREE_SINGLE_WIN   (SMALL_REC_START+20)  ///< @ランダムフリーシングル勝利数
#define RECID_RANDOM_FREE_SINGLE_LOSE  (SMALL_REC_START+21)  ///< @ランダムフリーシングル敗北数
#define RECID_RANDOM_FREE_DOUBLE_WIN   (SMALL_REC_START+22)  ///< @ランダムフリーダブル勝利数
#define RECID_RANDOM_FREE_DOUBLE_LOSE  (SMALL_REC_START+23)  ///< @ランダムフリーダブル敗北数
#define RECID_RANDOM_FREE_ROTATE_WIN   (SMALL_REC_START+24)  ///< @ランダムフリーローテーション勝利数
#define RECID_RANDOM_FREE_ROTATE_LOSE  (SMALL_REC_START+25)  ///< @ランダムフリーローテーション敗北数
#define RECID_RANDOM_FREE_TRIPLE_WIN   (SMALL_REC_START+26)  ///< @ランダムフリートリプル勝利数
#define RECID_RANDOM_FREE_TRIPLE_LOSE  (SMALL_REC_START+27)  ///< @ランダムフリートリプル敗北数
#define RECID_RANDOM_FREE_SHOOTER_WIN  (SMALL_REC_START+28)  ///< @ランダムフリーシューター勝利数
#define RECID_RANDOM_FREE_SHOOTER_LOSE (SMALL_REC_START+29)  ///< @ランダムフリーシューター敗北数
#define RECID_RANDOM_RATE_SINGLE_WIN   (SMALL_REC_START+30)  ///< @ランダムレートシングル勝利数
#define RECID_RANDOM_RATE_SINGLE_LOSE  (SMALL_REC_START+31)  ///< @ランダムレートシングル敗北数
#define RECID_RANDOM_RATE_DOUBLE_WIN   (SMALL_REC_START+32)  ///< @ランダムレートダブル勝利数
#define RECID_RANDOM_RATE_DOUBLE_LOSE  (SMALL_REC_START+33)  ///< @ランダムレートダブル敗北数
#define RECID_RANDOM_RATE_ROTATE_WIN   (SMALL_REC_START+34)  ///< @ランダムレートローテーション勝利数
#define RECID_RANDOM_RATE_ROTATE_LOSE  (SMALL_REC_START+35)  ///< @ランダムレートローテーション敗北数
#define RECID_RANDOM_RATE_TRIPLE_WIN   (SMALL_REC_START+36)  ///< @ランダムレートトリプル勝利数
#define RECID_RANDOM_RATE_TRIPLE_LOSE  (SMALL_REC_START+37)  ///< @ランダムレートトリプル敗北数
#define RECID_RANDOM_RATE_SHOOTER_WIN  (SMALL_REC_START+38)  ///< @ランダムレートシューター勝利数
#define RECID_RANDOM_RATE_SHOOTER_LOSE (SMALL_REC_START+39)  ///< @ランダムレートシューター敗北数
#define RECID_USE_SHOOTER_COUNT        (SMALL_REC_START+40)  ///< @シューター使用回数
#define RECID_SYMBOL_ENCOUNT_NUM       (SMALL_REC_START+41)  ///< @シンボルエンカウントをした回数
#define RECID_AFFINITY_CHECK_NUM       (SMALL_REC_START+42)  ///< @相性チェックをした回数
#define RECID_MUSICAL_PLAY_NUM         (SMALL_REC_START+43)  ///< @ミュージカル参加回数
#define RECID_MUSICAL_WINNER_NUM       (SMALL_REC_START+44)  ///< @ミュージカル勝利回数
#define RECID_MUSICAL_COMM_NUM         (SMALL_REC_START+45)  ///< @ミュージカル通信参加回数
#define RECID_MUSICAL_COMM_WINNER_NUM  (SMALL_REC_START+46)  ///< @ミュージカル通信勝利回数
#define RECID_MUSICAL_TOTAL_POINT      (SMALL_REC_START+47)  ///< @ミュージカル通算取得得点
#define RECID_PDW_SLEEP_POKEMON        (SMALL_REC_START+48)  ///< @PDWでポケモンを寝かせた回数
#define RECID_RENSHOU_SINGLE           (SMALL_REC_START+49)   ///< @タワー:シングルバトル連勝数
#define RECID_RENSHOU_SINGLE_CNT       (SMALL_REC_START+50)  ///< @タワー:シングルバトル連勝数
#define RECID_RENSHOU_S_SINGLE         (SMALL_REC_START+51)   ///< @タワー:スーパーシングルバトル連勝数
#define RECID_RENSHOU_S_SINGLE_CNT     (SMALL_REC_START+52)  ///< @タワー:スーパーシングルバトル連勝数
#define RECID_RENSHOU_DOUBLE           (SMALL_REC_START+53)  ///< @タワー:ダブルバトル連勝数
#define RECID_RENSHOU_DOUBLE_CNT       (SMALL_REC_START+54)  ///< @タワー:ダブルバトル連勝数
#define RECID_RENSHOU_S_DOUBLE         (SMALL_REC_START+55)  ///< @タワー:スーパーダブルバトル連勝数
#define RECID_RENSHOU_S_DOUBLE_CNT     (SMALL_REC_START+56)  ///< @タワー:スーパーダブルバトル連勝数
#define RECID_RENSHOU_MULTI            (SMALL_REC_START+57)  ///< @タワー:マルチバトル連勝数
#define RECID_RENSHOU_MULTI_CNT        (SMALL_REC_START+58)  ///< @タワー:マルチバトル連勝数
#define RECID_RENSHOU_S_MULTI          (SMALL_REC_START+59)  ///< @タワー:スーパーマルチバトル連勝数
#define RECID_RENSHOU_S_MULTI_CNT      (SMALL_REC_START+60)  ///< @タワー:スーパーマルチバトル連勝数
#define RECID_RENSHOU_WIFI             (SMALL_REC_START+61)  ///< @タワー:Wi-Fiバトル最大連勝数
#define RECID_RENSHOU_WIFI_CNT         (SMALL_REC_START+62)  ///< @タワー:Wi-Fiバトル連勝数
#define RECID_SURECHIGAI_THANKS        (SMALL_REC_START+63)  ///< @すれちがい通信でお礼を受けた回数
#define RECID_RENSHOU_WIFI_CNT         (SMALL_REC_START+62)  ///< @タワー:Wi-Fiバトル連勝数
#define RECID_SURECHIGAI_THANKS        (SMALL_REC_START+63)  ///< @すれちがい通信でお礼を受けた回数
#define RECID_TRIALHOUSE_RANK          (SMALL_REC_START+64)  ///< @トライアルハウス最高ランク
#define RECID_TRIALHOUSE_SCORE         (SMALL_REC_START+65)  ///< @トライアルハウス最高スコア


#define SMALL_REC_NUM ( 96 )  // 66+30 30箇所分余り    
#define SMALL_REC_MAX (SMALL_REC_NUM+LARGE_REC_MAX)


#endif  /* __RECORD_H__ */

