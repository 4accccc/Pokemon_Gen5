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
extern u32  RECORD_GetThScore(RECORD * rec);
extern void RECORD_SetThRank(RECORD * rec, u8 inRank);
extern u8   RECORD_GetThRank(RECORD * rec);
extern void RECORD_1day_Update( RECORD *rec );


#ifdef PM_DEBUG
//スコアのセット
extern void RECORD_Score_DebugSet(RECORD * rec, const u32 inScore);
extern void RECORD_DebugSet( RECORD *rec );
#endif


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
#define RECID_REPORT_COUNT      (LARGE_REC_START+ 0)   ///< @レポート回数 1
#define RECID_CLEAR_TIME        (LARGE_REC_START+ 1)   ///< @クリアタイム
#define RECID_RIDE_CYCLE        (LARGE_REC_START+ 2)   ///< @自転車に乗った回数
#define RECID_BATTLE_COUNT      (LARGE_REC_START+ 3)   ///< @全戦闘回数
#define RECID_BTL_ENCOUNT       (LARGE_REC_START+ 4)   ///< @エンカウント戦闘回数
#define RECID_BTL_TRAINER       (LARGE_REC_START+ 5)   ///< @トレーナー戦闘回数
#define RECID_CAPTURE_POKE      (LARGE_REC_START+ 6)   ///< @捕獲したポケモンの数
#define RECID_FISHING_SUCCESS   (LARGE_REC_START+ 7)   ///< @釣りあげたポケモンの回数
#define RECID_TAMAGO_HATCHING   (LARGE_REC_START+ 8)  ///< @タマゴを孵した回数
#define RECID_POKE_EVOLUTION    (LARGE_REC_START+ 9)  ///< @進化させたポケモンの引数 10
#define RECID_RECOVERY_PC       (LARGE_REC_START+10)  ///< @ポケセンで休んだ回数
#define RECID_COMM_TRADE        (LARGE_REC_START+11)  ///< @ワイヤレス通信交換をした回数
#define RECID_COMM_BATTLE       (LARGE_REC_START+12)  ///< @ワイヤレス通信対戦をした回数
#define RECID_COMM_BTL_WIN      (LARGE_REC_START+13)  ///< @ワイヤレス通信対戦の勝利数
#define RECID_COMM_BTL_LOSE     (LARGE_REC_START+14)  ///< @ワイヤレス通信対戦の敗北数
#define RECID_WIFI_TRADE        (LARGE_REC_START+15)  ///< @WiFi通信交換をした回数
#define RECID_WIFI_BATTLE       (LARGE_REC_START+16)  ///< @WiFi通信対戦をした回数
#define RECID_WIFI_BTL_WIN      (LARGE_REC_START+17)  ///< @WiFi通信対戦の勝利数
#define RECID_WIFI_BTL_LOSE     (LARGE_REC_START+18)  ///< @WiFi通信対戦の敗北数
#define RECID_WIFI_BTL_DRAW     (LARGE_REC_START+19)  ///< @WiFi通信対戦の引き分け数 20
#define RECID_SHOPPING_CNT      (LARGE_REC_START+20)  ///< @買い物をした回数
#define RECID_SHOPPING_MONEY    (LARGE_REC_START+21)  ///< @買い物をした合計金額
#define RECID_WATCH_TV          (LARGE_REC_START+22)  ///< @テレビを見た回数
#define RECID_SODATEYA_CNT      (LARGE_REC_START+23)  ///< @育て屋にポケモンを預けた回数
#define RECID_KILL_POKE         (LARGE_REC_START+24)  ///< @倒したポケモンの数
#define RECID_DAYMAX_EXP        (LARGE_REC_START+25)  ///< @1日に稼いだ経験値最大値
#define RECID_DAYCNT_EXP        (LARGE_REC_START+26)  ///< @1日に稼いだ経験値
#define RECID_GTS_PUT           (LARGE_REC_START+27)  ///< @GTSにポケモンを預けた回数
#define RECID_MAIL_WRITE        (LARGE_REC_START+28)  ///< @メールを書いた回数
#define RECID_NICKNAME          (LARGE_REC_START+29)  ///< @ポケモンにニックネームを付けた回数 30
#define RECID_PREMIUM_BALL      (LARGE_REC_START+30)  ///< @もらったプレミアボールの数
#define RECID_LEADERHOUSE_BATTLE  (LARGE_REC_START+31)  ///< @リーダーハウスで戦った回数
#define RECID_WIN_BP            (LARGE_REC_START+32)  ///< @勝ち取ったBP
#define RECID_USE_BP            (LARGE_REC_START+33)  ///< @使ったBP
#define RECID_SURECHIGAI_NUM    (LARGE_REC_START+34)  ///< @すれちがい通信をした回数※未使用
#define RECID_IRC_TRADE         (LARGE_REC_START+35)  ///< @赤外線通信交換をした回数
#define RECID_IRC_BATTLE        (LARGE_REC_START+36)  ///< @赤外線通信対戦をした回数
#define RECID_IRC_BTL_WIN       (LARGE_REC_START+37)  ///< @赤外線通信対戦の勝利数
#define RECID_IRC_BTL_LOSE      (LARGE_REC_START+38)  ///< @赤外線通信対戦の敗北数
#define RECID_PALACE_TIME       (LARGE_REC_START+39)  ///< @パレス訪問時間合計 40(未使用）
#define RECID_PALACE_MISSION     (LARGE_REC_START+40) ///< @パレスクリアしたミッションの数(未使用）

#define RECID_LARGE_42   ( LARGE_REC_START+41 )   ///< 4バイトレコード 未使用領域の定義
#define RECID_LARGE_43   ( LARGE_REC_START+42 )   ///< 次回作用に…
#define RECID_LARGE_44   ( LARGE_REC_START+43 )
#define RECID_LARGE_45   ( LARGE_REC_START+44 )
#define RECID_LARGE_46   ( LARGE_REC_START+45 )
#define RECID_LARGE_47   ( LARGE_REC_START+46 )
#define RECID_LARGE_48   ( LARGE_REC_START+47 )
#define RECID_LARGE_49   ( LARGE_REC_START+48 )
#define RECID_LARGE_50   ( LARGE_REC_START+49 )
#define RECID_LARGE_51   ( LARGE_REC_START+50 )
#define RECID_LARGE_52   ( LARGE_REC_START+51 )
#define RECID_LARGE_53   ( LARGE_REC_START+52 )
#define RECID_LARGE_54   ( LARGE_REC_START+53 )
#define RECID_LARGE_55   ( LARGE_REC_START+54 )
#define RECID_LARGE_56   ( LARGE_REC_START+55 )
#define RECID_LARGE_57   ( LARGE_REC_START+56 )
#define RECID_LARGE_58   ( LARGE_REC_START+57 )
#define RECID_LARGE_59   ( LARGE_REC_START+58 )
#define RECID_LARGE_60   ( LARGE_REC_START+59 )
#define RECID_LARGE_61   ( LARGE_REC_START+60 )
#define RECID_LARGE_62   ( LARGE_REC_START+61 )
#define RECID_LARGE_63   ( LARGE_REC_START+62 )
#define RECID_LARGE_64   ( LARGE_REC_START+63 )
#define RECID_LARGE_65   ( LARGE_REC_START+64 )
#define RECID_LARGE_66   ( LARGE_REC_START+65 )
#define RECID_LARGE_67   ( LARGE_REC_START+66 )
#define RECID_LARGE_68   ( LARGE_REC_START+67 )
#define RECID_LARGE_69   ( LARGE_REC_START+68 )
#define RECID_LARGE_70   ( LARGE_REC_START+69 )

#define LARGE_REC_NUM    ( LARGE_REC_START+70 )  ///< WBで41+1個使用。残り29個
#define LARGE_REC_MAX    ( LARGE_REC_NUM ) 

//以下は、2バイトしかカウンタを持たない
#define SMALL_REC_START   LARGE_REC_MAX

#define RECID_DENDOU_CNT          (SMALL_REC_START+ 0) ///<@殿堂入りした回数  71
#define RECID_RECOVERY_HOME       (SMALL_REC_START+ 1) ///<@自分の家で休んだ回数
#define RECID_WAZA_HANERU         (SMALL_REC_START+ 2) ///< @「はねる」を使った回数
#define RECID_WAZA_WARUAGAKI      (SMALL_REC_START+ 3) ///< @「わるあがき」を使った回数
#define RECID_WAZA_MUKOU          (SMALL_REC_START+ 4)  ///< @無効果わざを出した回数
#define RECID_TEMOTI_KIZETU       (SMALL_REC_START+ 5)  ///< @気絶させた手持ちの引数
#define RECID_TEMOTI_MAKIZOE      (SMALL_REC_START+ 6)  ///< @味方にダメージを与えた回数
#define RECID_NIGERU_SIPPAI       (SMALL_REC_START+ 7)  ///< @逃げそこなった回数 
#define RECID_NIGERARETA          (SMALL_REC_START+ 8)  ///< @逃げられたポケモンの数
#define RECID_FISHING_FAILURE     (SMALL_REC_START+ 9)  ///< @つり損なった回数 80
#define RECID_DAYMAX_KILL         (SMALL_REC_START+10)  ///< @1日に倒したポケモンの最高数
#define RECID_DAYCNT_KILL         (SMALL_REC_START+11)  ///< @1日に倒したポケモンの数
#define RECID_DAYMAX_CAPTURE      (SMALL_REC_START+12)  ///< @1日に捕獲したポケモンの最高数
#define RECID_DAYCNT_CAPTURE      (SMALL_REC_START+13)  ///< @1日に捕獲したポケモンの数
#define RECID_DAYMAX_TRAINER_BATTLE   (SMALL_REC_START+14)  ///< @1日にトレーナー戦した最高回数
#define RECID_DAYCNT_TRAINER_BATTLE   (SMALL_REC_START+15)  ///< @1日にトレーナー戦した回数
#define RECID_DAYMAX_EVOLUTION    (SMALL_REC_START+16)  ///< @1日にポケモン進化させた最高回数
#define RECID_DAYCNT_EVOLUTION    (SMALL_REC_START+17)  ///< @1日にポケモン進化させた回数
#define RECID_FOSSIL_RESTORE      (SMALL_REC_START+18)  ///< @ポケモンの化石を復元した回数
#define RECID_GURUGURU_COUNT      (SMALL_REC_START+19)  ///< @ぐるぐる交換をした回数 90
#define RECID_RANDOM_FREE_SINGLE_WIN   (SMALL_REC_START+20)  ///< ↑(未使用）
#define RECID_RANDOM_FREE_SINGLE_LOSE  (SMALL_REC_START+21)  ///< ランダムマッチ周りの勝敗カウントの(未使用）
#define RECID_RANDOM_FREE_DOUBLE_WIN   (SMALL_REC_START+22)  ///< つもりだったが別の場所で(未使用）
#define RECID_RANDOM_FREE_DOUBLE_LOSE  (SMALL_REC_START+23)  ///< 確保されていたので未使用(未使用）
#define RECID_RANDOM_FREE_ROTATE_WIN   (SMALL_REC_START+24)  ///< (未使用）
#define RECID_RANDOM_FREE_ROTATE_LOSE  (SMALL_REC_START+25)  ///< (未使用）
#define RECID_RANDOM_FREE_TRIPLE_WIN   (SMALL_REC_START+26)  ///< (未使用）
#define RECID_RANDOM_FREE_TRIPLE_LOSE  (SMALL_REC_START+27)  ///< (未使用）
#define RECID_RANDOM_FREE_SHOOTER_WIN  (SMALL_REC_START+28)  ///< (未使用）
#define RECID_RANDOM_FREE_SHOOTER_LOSE (SMALL_REC_START+29)  ///< (未使用）
#define RECID_RANDOM_RATE_SINGLE_WIN   (SMALL_REC_START+30)  ///< (未使用）
#define RECID_RANDOM_RATE_SINGLE_LOSE  (SMALL_REC_START+31)  ///< (未使用）
#define RECID_RANDOM_RATE_DOUBLE_WIN   (SMALL_REC_START+32)  ///< (未使用）
#define RECID_RANDOM_RATE_DOUBLE_LOSE  (SMALL_REC_START+33)  ///< (未使用）
#define RECID_RANDOM_RATE_ROTATE_WIN   (SMALL_REC_START+34)  ///< (未使用）
#define RECID_RANDOM_RATE_ROTATE_LOSE  (SMALL_REC_START+35)  ///< (未使用）
#define RECID_RANDOM_RATE_TRIPLE_WIN   (SMALL_REC_START+36)  ///< (未使用）
#define RECID_RANDOM_RATE_TRIPLE_LOSE  (SMALL_REC_START+37)  ///< (未使用）
#define RECID_RANDOM_RATE_SHOOTER_WIN  (SMALL_REC_START+38)  ///< (未使用）
#define RECID_RANDOM_RATE_SHOOTER_LOSE (SMALL_REC_START+39)  ///< ↓(未使用）
#define RECID_USE_SHOOTER_COUNT        (SMALL_REC_START+40)  ///< @シューター使用回数
#define RECID_EFFECT_ENCOUNT_NUM       (SMALL_REC_START+41)  ///< @エフェクトエンカウントをした回数
#define RECID_AFFINITY_CHECK_NUM       (SMALL_REC_START+42)  ///< @相性チェックをした回数
#define RECID_MUSICAL_PLAY_NUM         (SMALL_REC_START+43)  ///< @ミュージカル参加回数
#define RECID_MUSICAL_WINNER_NUM       (SMALL_REC_START+44)  ///< @ミュージカル勝利回数
#define RECID_MUSICAL_COMM_NUM         (SMALL_REC_START+45)  ///< @ミュージカル通信参加回数
#define RECID_MUSICAL_COMM_WINNER_NUM  (SMALL_REC_START+46)  ///< @ミュージカル通信勝利回数
#define RECID_MUSICAL_TOTAL_POINT      (SMALL_REC_START+47)  ///< @ミュージカル通算取得得点
#define RECID_PDW_SLEEP_POKEMON        (SMALL_REC_START+48)  ///< @PDWでポケモンを寝かせた回数
#define RECID_POKESHIFTER_COUNT        (SMALL_REC_START+49)  ///< @ポケシフターをした回数 120
#define RECID_TRIALHOUSE_COUNT         (SMALL_REC_START+50)  ///< @トライアルハウスをした回数
#define RECID_SURECHIGAI_THANKS        (SMALL_REC_START+51)  ///< @すれちがい通信でお礼を受けた回数※未使用
#define RECID_TRIALHOUSE_RANK          (SMALL_REC_START+52)  ///< @トライアルハウス最高ランク
#define RECID_TRIALHOUSE_SCORE         (SMALL_REC_START+53)  ///< @トライアルハウス最高スコア 124

#define RECID_SMALL_54                 (SMALL_REC_START+54)  ///< 2バイトレコード　未使用領域定義
#define RECID_SMALL_55                 (SMALL_REC_START+55)  ///< 次回作用
#define RECID_SMALL_56                 (SMALL_REC_START+56)
#define RECID_SMALL_57                 (SMALL_REC_START+57)
#define RECID_SMALL_58                 (SMALL_REC_START+58)
#define RECID_SMALL_59                 (SMALL_REC_START+59)
#define RECID_SMALL_60                 (SMALL_REC_START+60)
#define RECID_SMALL_61                 (SMALL_REC_START+61)
#define RECID_SMALL_62                 (SMALL_REC_START+62)
#define RECID_SMALL_63                 (SMALL_REC_START+63)
#define RECID_SMALL_64                 (SMALL_REC_START+64)
#define RECID_SMALL_65                 (SMALL_REC_START+65)
#define RECID_SMALL_66                 (SMALL_REC_START+66)
#define RECID_SMALL_67                 (SMALL_REC_START+67)
#define RECID_SMALL_68                 (SMALL_REC_START+68)
#define RECID_SMALL_69                 (SMALL_REC_START+69)
#define RECID_SMALL_70                 (SMALL_REC_START+70)
#define RECID_SMALL_71                 (SMALL_REC_START+71)
#define RECID_SMALL_72                 (SMALL_REC_START+72)
#define RECID_SMALL_73                 (SMALL_REC_START+73)
#define RECID_SMALL_74                 (SMALL_REC_START+74)
#define RECID_SMALL_75                 (SMALL_REC_START+75)
#define RECID_SMALL_76                 (SMALL_REC_START+76)
#define RECID_SMALL_77                 (SMALL_REC_START+77)
#define RECID_SMALL_78                 (SMALL_REC_START+78)
#define RECID_SMALL_79                 (SMALL_REC_START+79)
#define RECID_SMALL_80                 (SMALL_REC_START+80)
#define RECID_SMALL_81                 (SMALL_REC_START+81)
#define RECID_SMALL_82                 (SMALL_REC_START+82)
#define RECID_SMALL_83                 (SMALL_REC_START+83)
#define RECID_SMALL_84                 (SMALL_REC_START+84)
#define RECID_SMALL_85                 (SMALL_REC_START+85)
#define RECID_SMALL_86                 (SMALL_REC_START+86)
#define RECID_SMALL_87                 (SMALL_REC_START+87)
#define RECID_SMALL_88                 (SMALL_REC_START+88)
#define RECID_SMALL_89                 (SMALL_REC_START+89)
#define RECID_SMALL_90                 (SMALL_REC_START+90)
#define RECID_SMALL_91                 (SMALL_REC_START+91)
#define RECID_SMALL_92                 (SMALL_REC_START+92)
#define RECID_SMALL_93                 (SMALL_REC_START+93)
#define RECID_SMALL_94                 (SMALL_REC_START+94)
#define RECID_SMALL_95                 (SMALL_REC_START+95)


#define SMALL_REC_NUM                   (96)  // 54+42 42箇所分余り    
#define SMALL_REC_MAX                   (SMALL_REC_NUM+LARGE_REC_MAX)




// --------------------------------------------------------------------------------------
// MAX確認用フラグ定義
// 0: 999,999,999
// 1:     999,999
// 2:      65,536
// 3:       9,999
// 4:         999
// 5:           7
// --------------------------------------------------------------------------------------

#define RECID_WALK_COUNT_FLAG       ( 0 )  ///< 歩いた歩数  999,999,999
#define RECID_REPORT_COUNT_FLAG     ( 0 )  ///< レポート回数 999,999,999
#define RECID_CLEAR_TIME_FLAG       ( 1 )  ///< クリアタイム 999,59,59
#define RECID_RIDE_CYCLE_FLAG       ( 0 )  ///< 自転車に乗った回数 999,999,999
#define RECID_BATTLE_COUNT_FLAG     ( 0 )  ///< 全戦闘回数 999,999,999
#define RECID_BTL_ENCOUNT_FLAG      ( 0 )  ///< エンカウント戦闘回数 999,999,999
#define RECID_BTL_TRAINER_FLAG      ( 0 )  ///< トレーナー戦闘回数 999,999,999
#define RECID_CAPTURE_POKE_FLAG     ( 1 )  ///< 捕獲したポケモンの数 999,999
#define RECID_FISHING_SUCCESS_FLAG  ( 1 )  ///< 釣りあげたポケモンの回数 999,999
#define RECID_TAMAGO_HATCHING_FLAG  ( 1 )  ///< タマゴを孵した回数 999,999
#define RECID_POKE_EVOLUTION_FLAG   ( 0 )  ///< 進化させたポケモンの引数  999,999,999
#define RECID_RECOVERY_PC_FLAG      ( 0 )  ///< ポケセンで休んだ回数  999,999,999
#define RECID_COMM_TRADE_FLAG       ( 0 )  ///< ワイヤレス通信交換をした回数  999,999,999
#define RECID_COMM_BATTLE_FLAG      ( 0 )  ///< ワイヤレス通信対戦をした回数  999,999,999
#define RECID_COMM_BTL_WIN_FLAG     ( 0 )  ///< ワイヤレス通信対戦の勝利数 999,999,999
#define RECID_COMM_BTL_LOSE_FLAG    ( 0 )  ///< ワイヤレス通信対戦の敗北数 999,999,999
#define RECID_WIFI_TRADE_FLAG       ( 0 )  ///< WiFi通信交換をした回数 999,999,999
#define RECID_WIFI_BATTLE_FLAG      ( 0 )  ///< WiFi通信対戦をした回数 999,999,999
#define RECID_WIFI_BTL_WIN_FLAG     ( 0 )  ///< WiFi通信対戦の勝利数 999,999,999
#define RECID_WIFI_BTL_LOSE_FLAG    ( 0 )  ///< WiFi通信対戦の敗北数 999,999,999
#define RECID_WIFI_BTL_DRAW_FLAG    ( 0 )  ///< WiFi通信対戦の引き分け数 999,999,999
#define RECID_SHOPPING_CNT_FLAG     ( 0 )  ///< 買い物をした回数 999,999,999
#define RECID_SHOPPING_MONEY_FLAG   ( 0 )  ///< 買い物をした合計金額 999,999,999
#define RECID_WATCH_TV_FLAG         ( 0 )  ///< テレビを見た回数 999,999,999
#define RECID_SODATEYA_CNT_FLAG     ( 0 )  ///< 育て屋にポケモンを預けた回数 999,999,999
#define RECID_KILL_POKE_FLAG        ( 0 )  ///< 倒したポケモンの数 999,999,999
#define RECID_DAYMAX_EXP_FLAG       ( 0 )  ///< 1日に稼いだ経験値最大値 999,999,999
#define RECID_DAYCNT_EXP_FLAG       ( 0 )  ///< 1日に稼いだ経験値 999,999,999
#define RECID_GTS_PUT_FLAG          ( 1 )  ///< GTSにポケモンを預けた回数 999,999
#define RECID_MAIL_WRITE_FLAG       ( 1 )  ///< メールを書いた回数 999,999
#define RECID_NICKNAME_FLAG         ( 1 )  ///< ポケモンにニックネームを付けた回数 999,999
#define RECID_PREMIUM_BALL_FLAG     ( 1 )  ///< もらったプレミアボールの数 999,999
#define RECID_LEADERHOUSE_BATTLE_FLAG ( 0 )  ///< リーダーハウスで戦った回数  999,999,999
#define RECID_WIN_BP_FLAG           ( 0 )  ///< 勝ち取ったBP 999,999,999
#define RECID_USE_BP_FLAG           ( 0 )  ///< 使ったBP 999,999,999
#define RECID_SURECHIGAI_NUM_FLAG   ( 0 )  ///< すれちがい通信をした回数 99,999※未使用
#define RECID_IRC_TRADE_FLAG        ( 0 )  ///< 赤外線通信交換をした回数 999,999,999
#define RECID_IRC_BATTLE_FLAG       ( 0 )  ///< 赤外線通信対戦をした回数 999,999,999
#define RECID_IRC_BTL_WIN_FLAG      ( 0 )  ///< 赤外線通信対戦の勝利数 999,999,999
#define RECID_IRC_BTL_LOSE_FLAG     ( 0 )  ///< 赤外線通信対戦の敗北数 999,999,999
#define RECID_PALACE_TIME_FLAG      ( 1 )  ///< パレス訪問時間合計 999,59,59
#define RECID_PALACE_MISSION_FLAG   ( 1 )  ///< パレスクリアしたミッションの数 999,59,59

#define RECID_LARGE_42_FLAG   ( 1 )   ///< 4バイトレコード 未使用領域のMAX定義
#define RECID_LARGE_43_FLAG   ( 1 )   ///< 0 origin
#define RECID_LARGE_44_FLAG   ( 1 )
#define RECID_LARGE_45_FLAG   ( 1 )
#define RECID_LARGE_46_FLAG   ( 1 )
#define RECID_LARGE_47_FLAG   ( 1 )
#define RECID_LARGE_48_FLAG   ( 1 )
#define RECID_LARGE_49_FLAG   ( 1 )
#define RECID_LARGE_50_FLAG   ( 1 )
#define RECID_LARGE_51_FLAG   ( 1 )
#define RECID_LARGE_52_FLAG   ( 1 )
#define RECID_LARGE_53_FLAG   ( 1 )
#define RECID_LARGE_54_FLAG   ( 1 )
#define RECID_LARGE_55_FLAG   ( 1 )
#define RECID_LARGE_56_FLAG   ( 1 )
#define RECID_LARGE_57_FLAG   ( 1 )
#define RECID_LARGE_58_FLAG   ( 1 )
#define RECID_LARGE_59_FLAG   ( 1 )
#define RECID_LARGE_60_FLAG   ( 1 )
#define RECID_LARGE_61_FLAG   ( 1 )
#define RECID_LARGE_62_FLAG   ( 1 )
#define RECID_LARGE_63_FLAG   ( 1 )
#define RECID_LARGE_64_FLAG   ( 1 )
#define RECID_LARGE_65_FLAG   ( 1 )
#define RECID_LARGE_66_FLAG   ( 1 )
#define RECID_LARGE_67_FLAG   ( 1 )
#define RECID_LARGE_68_FLAG   ( 1 )
#define RECID_LARGE_69_FLAG   ( 1 )
#define RECID_LARGE_70_FLAG   ( 1 )


#define RECID_DENDOU_CNT_FLAG         ( 3 ) ///<殿堂入りした回数          
#define RECID_RECOVERY_HOME_FLAG      ( 3 ) ///<自分の家で休んだ回数
#define RECID_WAZA_HANERU_FLAG        ( 3 ) ///< 「はねる」を使った回数
#define RECID_WAZA_WARUAGAKI_FLAG     ( 3 ) ///< 「わるあがき」を使った回数
#define RECID_WAZA_MUKOU_FLAG         ( 3 )  ///< 無効果わざを出した回数
#define RECID_TEMOTI_KIZETU_FLAG      ( 3 )  ///< 気絶させた手持ちの引数
#define RECID_TEMOTI_MAKIZOE_FLAG     ( 3 )  ///< 味方にダメージを与えた回数
#define RECID_NIGERU_SIPPAI_FLAG      ( 3 )  ///< 逃げそこなった回数
#define RECID_NIGERARETA_FLAG         ( 3 )  ///< 逃げられたポケモンの数  
#define RECID_FISHING_FAILURE_FLAG    ( 3 )  ///< つり損なった回数        80
#define RECID_DAYMAX_KILL_FLAG        ( 3 )  ///< 1日に倒したポケモンの最高数
#define RECID_DAYCNT_KILL_FLAG        ( 3 )  ///< 1日に倒したポケモンの数
#define RECID_DAYMAX_CAPTURE_FLAG     ( 3 )  ///< 1日に捕獲したポケモンの最高数
#define RECID_DAYCNT_CAPTURE_FLAG     ( 3 )  ///< 1日に捕獲したポケモンの数
#define RECID_DAYMAX_TRAINER_BATTLE_FLAG  ( 3 )  ///< 1日にトレーナー戦した最高回数
#define RECID_DAYCNT_TRAINER_BATTLE_FLAG  ( 3 )  ///< 1日にトレーナー戦した回数
#define RECID_DAYMAX_EVOLUTION_FLAG   ( 3 )  ///< 1日にポケモン進化させた最高回数
#define RECID_DAYCNT_EVOLUTION_FLAG   ( 3 )  ///< 1日にポケモン進化させた回数
#define RECID_FOSSIL_RESTORE_FLAG     ( 3 )  ///< ポケモンの化石を復元した回数  
#define RECID_GURUGURU_COUNT_FLAG     ( 3 )  ///< ぐるぐる交換をした回数        90
#define RECID_RANDOM_FREE_SINGLE_WIN_FLAG  ( 2 )  ///< 
#define RECID_RANDOM_FREE_SINGLE_LOSE_FLAG ( 2 )  ///< 
#define RECID_RANDOM_FREE_DOUBLE_WIN_FLAG  ( 2 )  ///< 
#define RECID_RANDOM_FREE_DOUBLE_LOSE_FLAG ( 2 )  ///< 
#define RECID_RANDOM_FREE_ROTATE_WIN_FLAG  ( 2 )  ///< 
#define RECID_RANDOM_FREE_ROTATE_LOSE_FLAG ( 2 )  ///< 
#define RECID_RANDOM_FREE_TRIPLE_WIN_FLAG  ( 2 )  ///< 
#define RECID_RANDOM_FREE_TRIPLE_LOSE_FLAG ( 2 )  ///< 
#define RECID_RANDOM_FREE_SHOOTER_WIN_FLAG ( 2 )  ///< 
#define RECID_RANDOM_FREE_SHOOTER_LOSE_FLAG ( 2 )  ///<
#define RECID_RANDOM_RATE_SINGLE_WIN_FLAG  ( 2 )  ///< 
#define RECID_RANDOM_RATE_SINGLE_LOSE_FLAG ( 2 )  ///< 
#define RECID_RANDOM_RATE_DOUBLE_WIN_FLAG  ( 2 )  ///< 
#define RECID_RANDOM_RATE_DOUBLE_LOSE_FLAG ( 2 )  ///< 
#define RECID_RANDOM_RATE_ROTATE_WIN_FLAG  ( 2 )  ///< 
#define RECID_RANDOM_RATE_ROTATE_LOSE_FLAG ( 2 )  ///< 
#define RECID_RANDOM_RATE_TRIPLE_WIN_FLAG  ( 2 )  ///< 
#define RECID_RANDOM_RATE_TRIPLE_LOSE_FLAG ( 2 )  ///< 
#define RECID_RANDOM_RATE_SHOOTER_WIN_FLAG ( 2 )  ///< 
#define RECID_RANDOM_RATE_SHOOTER_LOSE_FLAG ( 2 )  ///< ランダムレートシューター敗北数 110
#define RECID_USE_SHOOTER_COUNT_FLAG       ( 3 )  ///< シューター使用回数
#define RECID_EFFECT_ENCOUNT_NUM_FLAG      ( 3 )  ///< エフェクトエンカウントをした回数
#define RECID_AFFINITY_CHECK_NUM_FLAG      ( 3 )  ///< 相性チェックをした回数
#define RECID_MUSICAL_PLAY_NUM_FLAG        ( 3 )  ///< ミュージカル参加回数
#define RECID_MUSICAL_WINNER_NUM_FLAG      ( 3 )  ///< ミュージカル勝利回数
#define RECID_MUSICAL_COMM_NUM_FLAG        ( 3 )  ///< ミュージカル通信参加回数
#define RECID_MUSICAL_COMM_WINNER_NUM_FLAG ( 3 )  ///< ミュージカル通信勝利回数
#define RECID_MUSICAL_TOTAL_POINT_FLAG     ( 2 )  ///< ミュージカル通算取得得点
#define RECID_PDW_SLEEP_POKEMON_FLAG       ( 3 )  ///< PDWでポケモンを寝かせた回数 
#define RECID_POKESHIFTER_COUNT_FLAG       ( 4 )  ///< ポケシフターをした回数 120
#define RECID_TRIALHOUSE_COUNT_FLAG        ( 3 )  ///< トライアルハウスをした回数
#define RECID_SURECHIGAI_THANKS_FLAG       ( 3 )  ///< すれちがい通信でお礼を受けた回数※未使用
#define RECID_TRIALHOUSE_RANK_FLAG         ( 5 )  ///< トライアルハウス最高ランク
#define RECID_TRIALHOUSE_SCORE_FLAG        ( 3 )  ///< トライアルハウス最高スコア 124

#define RECID_SMALL_54_FLAG                ( 2 )  ///< 2バイトレコード　未使用領域MAX定義
#define RECID_SMALL_55_FLAG                ( 2 )  ///< 0 origin
#define RECID_SMALL_56_FLAG                ( 2 )
#define RECID_SMALL_57_FLAG                ( 2 )
#define RECID_SMALL_58_FLAG                ( 2 )
#define RECID_SMALL_59_FLAG                ( 2 )
#define RECID_SMALL_60_FLAG                ( 2 )
#define RECID_SMALL_61_FLAG                ( 2 )
#define RECID_SMALL_62_FLAG                ( 2 )
#define RECID_SMALL_63_FLAG                ( 2 )
#define RECID_SMALL_64_FLAG                ( 2 )
#define RECID_SMALL_65_FLAG                ( 2 )
#define RECID_SMALL_66_FLAG                ( 2 )
#define RECID_SMALL_67_FLAG                ( 2 )
#define RECID_SMALL_68_FLAG                ( 2 )
#define RECID_SMALL_69_FLAG                ( 2 )
#define RECID_SMALL_70_FLAG                ( 2 )
#define RECID_SMALL_71_FLAG                ( 2 )
#define RECID_SMALL_72_FLAG                ( 2 )
#define RECID_SMALL_73_FLAG                ( 2 )
#define RECID_SMALL_74_FLAG                ( 2 )
#define RECID_SMALL_75_FLAG                ( 2 )
#define RECID_SMALL_76_FLAG                ( 2 )
#define RECID_SMALL_77_FLAG                ( 2 )
#define RECID_SMALL_78_FLAG                ( 2 )
#define RECID_SMALL_79_FLAG                ( 2 )
#define RECID_SMALL_80_FLAG                ( 2 )
#define RECID_SMALL_81_FLAG                ( 2 )
#define RECID_SMALL_82_FLAG                ( 2 )
#define RECID_SMALL_83_FLAG                ( 2 )
#define RECID_SMALL_84_FLAG                ( 2 )
#define RECID_SMALL_85_FLAG                ( 2 )
#define RECID_SMALL_86_FLAG                ( 2 )
#define RECID_SMALL_87_FLAG                ( 2 )
#define RECID_SMALL_88_FLAG                ( 2 )
#define RECID_SMALL_89_FLAG                ( 2 )
#define RECID_SMALL_90_FLAG                ( 2 )
#define RECID_SMALL_91_FLAG                ( 2 )
#define RECID_SMALL_92_FLAG                ( 2 )
#define RECID_SMALL_93_FLAG                ( 2 )
#define RECID_SMALL_94_FLAG                ( 2 )
#define RECID_SMALL_95_FLAG                ( 2 )

#endif  /* __RECORD_H__ */

