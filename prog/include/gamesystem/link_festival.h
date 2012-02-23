//==============================================================================
/**
 * @file    link_festival.h
 * @brief   リンクフェスティバル
 * @author  iwasawa
 * @date    2010.12.16
 */
//==============================================================================

#ifndef __H_LINK_FESTIVAL_H__
#define __H_LINK_FESTIVAL_H__

#include "gamesystem/gamesystem_def.h"
#include "print/wordset.h"
#include "fest_mission.h"
#include "fest_mission_data.h"
#include "fmission_log.h"

/*
  @brief  フラグ領域バッファ長定義
  本当はfield/fes_gimmick_local.h のフラグ領域構造体FES_GIMMICK_WORKのサイズ定義を直接参照したいが
  フィールドローカルの定義とファイルを切り離すことが難しかったので直値で定義している。

  field/fes_gimmik.c で SDK_COMPILER_ASSERT()で必要なサイズが確保されているかチェックしている
 */
#define FMISSION_WORK_AREA_LEN  (0x700)

#define LINKFEST_LOG_MAX  (99) //フェスミッションログMax(自分含めて100がmax)
#define LINKFEST_LOG_END  (LINKFEST_LOG_MAX-1) //フェスミッションログEndIndex
#define LINKFEST_JOIN_MAX (LINKFEST_LOG_MAX+1)  //自分を含めた参加者数Maxは100

#define LINKFEST_LIFE_LIMIT (30) //30秒
#define FES_BGM_RECOVER_NONE  (0xFFFFFFFF)  //BGMのリカバー無しコード

/*
  @brief  フェス状態遷移図(各パラメータのアップデート状態図)

  send  -> フェスビーコン送信
  recv  -> フェスビーコン受信処理実行(ログ収集とランキング更新)
  rank  -> トップスコア情報を含むスコアランキング情報更新
  score -> トータルスコア収集
  bonus -> タイムボーナス加算処理

  No | 状態               | タイミング                   | status | send | recv | rank | score | bonus | 
  ---------------------------------------------------------------------------------------------
  1  | ミッション開始前   | ミッション開始前             | NULL   |  x   |  x   |  x   |  x    |  x    |

  2  | ミッションスタート | スタートイベント開始～       | START  |  o   |  o   |  o   |  o    |  x    |
  3  | スタート演出中     | 演出中～フィールド再生成まで | START  |  o   |  o   |  o   |  o    |  x    |
  
  4  | プレイスタート     | スタートイベント最終frame    | PLAY   |  o   |  o   |  o   |  o    |  o    |
  5  | プレイ中           | 自由行動中                   | PLAY   |  o   |  o   |  o   |  o    |  o    |
  
  6  | タイムアップ       | フィニッシュイベント起動     | FINISH |  o   |  o   |  o   |  o    |  x    |　<<ここで残り時間と現在のスコアを記録
  7  | フィニッシュ演出中 | リザルト画面呼び出しまで     | FINISH |  o   |  o   |  o   |  o    |  x    |
  
  8  | 結果確定           | リザルト画面初期化seq内      | RESULT |  x   |  x   |  x   |  x    |  x    |  <<リザルト画面呼び出しイベント内で
  9  | リザルト画面演出中 |                              | RESULT |  x   |  x   |  x   |  x    |  x    |  <<受信専用モードに移行することで送信が実質止まる
  
  10 | ミッション終了     | リザルト画面抜け直後         | NULL   |  x   |  x   |  x   |  x    |  x    |

 */
typedef enum{
 FMISSION_LIVE_STATUS_NULL,   //非ミッション時
 FMISSION_LIVE_STATUS_START,  //ミッション開始～スタート演出中
 FMISSION_LIVE_STATUS_PLAY,   //ミッションプレイ中
 FMISSION_LIVE_STATUS_FINISH, //ミッションフィニッシュ～
 FMISSION_LIVE_STATUS_RESULT, //ミッション結果確定後
}FMISSION_LIVE_STATUS;

typedef struct _LINK_FESTIVAL LINK_FESTIVAL;

//トップスコア保存用構造体
typedef struct _LINKFEST_LOG_TOP_SCORE{
  STRBUF* name; ///<人物名格納バッファ
  u16 score;    ///<スコア
  u16 tr_id;    ///<トレーナーID
}LINKFEST_LOG_TOP_SCORE;

//ミッションの状態を取得するデータ構造体
typedef struct _LINFEST_MISSION_STATUS{
  u8  entry_num;  ///<参加者数
  u8  my_ranking;       ///<自分のスコアの順位
  u16 my_score;         ///<自分のスコア
  u16 total_score;      ///<現在のスコア
}LINKFEST_MISSION_STATUS;

//フェスミッション結果取得用構造体
typedef struct _LINKFEST_MISSION_RESULT{
  u8    clear_f;  //TRUEならクリアした
  u8    hilink_version;   //<ハイリンクレベルのアップしたバージョン FMISSION_VERSION
  u8    hilink_level_up;  //ハイリンクのレベルアップ値(0ならレベルアップ無し)
  u8    new_record_score_f:1; //スコアのニューレコード
  u8    new_record_my_score_f:1; //マイスコアのニューレコード
  u8    new_record_time_f:1; //残り時間のニューレコード
  u8    :5;

  u32   score;    //スコア
  u16   life;     //クリア時残り時間(秒)
  u16   item;     //ご褒美アイテム
  u16   item_num; //ご褒美アイテムの数
  u16   bonus_num;  //ご褒美アイテムのボーナス数
  u16   sp_item;  //特別ボーナスアイテム(0なら特別ボーナス無し)
  u16   sp_item_num;  //特別ボーナスアイテムの数(0なら特別ボーナス無し)
}LINKFEST_MISSION_RESULT;

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル管理領域生成
 */
//---------------------------------------------------------
extern LINK_FESTIVAL* LINK_FESTIVAL_Create( HEAPID heapID, GAMESYS_WORK* gsys );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル管理領域解放
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_Delete( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル状態アップデート
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_Update( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション参加中か？
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_IsMissionEntry( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションワークポインタ取得
 */
//---------------------------------------------------------
extern FEST_MISSION* LINK_FESTIVAL_GetMissionWork( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションデータアーカイブハンドル取得
 */
//---------------------------------------------------------
extern ARCHANDLE* LINK_FESTIVAL_GetFMissionDataHandle( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 送信用ミッションデータ生成
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_GetBeaconSendParam( LINK_FESTIVAL* wk, FEST_BEACON* dat, u16 act_param );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション　現在の自分のスコアを返す
 *
 * @retval 自分のスコア 
 */
//---------------------------------------------------------
extern u16 LINK_FESTIVAL_GetMyScore( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション残り時間を返す
 *
 * @retval  残り時間：秒単位
 */
//---------------------------------------------------------
extern u16 LINK_FESTIVAL_GetMissionLife( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションの現在の状態を返す
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_GetMissionStatus( LINK_FESTIVAL* wk, LINKFEST_MISSION_STATUS* sp  );


//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションログスタックテーブルポインタ取得
 */
//---------------------------------------------------------
extern FMISSION_LOG_TBL* LINK_FESTIVAL_GetMissionLogStackTbl( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションログビューテーブルポインタ取得
 */
//---------------------------------------------------------
extern FMISSION_LOG_TBL* LINK_FESTIVAL_GetMissionLogViewTbl( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションログGパワーテーブルポインタ取得
 */
//---------------------------------------------------------
extern FMISSION_GPOWER_TBL* LINK_FESTIVAL_GetMissionLogGPowerTbl( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションエントリーデータテーブルポインタ取得
 */
//---------------------------------------------------------
extern FMISSION_ENTRY_TBL* LINK_FESTIVAL_GetMissionLogEntryTbl( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションデータコモンアーカイブパラメータ取得
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_GetMissionArcCommonData( LINK_FESTIVAL* wk, u8 m_id, FMISSION_ARC_COMMON* arc );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションスタート
 *
 * この関数を呼んだ時点でビーコン送信は開始されるが、
 * アップデートは無効のまま(時間カウントが始まらない)ので注意！
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_MissionStart( LINK_FESTIVAL* wk, const FEST_MISSION* mission, BOOL succeed_f );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションプレイスタート
 *
 * MissionStartを呼んだ時点で内部的にはミッションが開始され、ビーコン送信も開始されていますが
 * アップデートは無効のまま(時間カウントが始まらない)なので
 * Start後に、この関数を呼び出してプレイ状態に遷移してください
 *
 * 時間カウントの基準も、この関数を呼び出したタイミングで決定します
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_MissionPlayStart( LINK_FESTIVAL* wk, BOOL succeed_f );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションステータス 設定
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_MissionLiveStatusSet( LINK_FESTIVAL* wk, FMISSION_LIVE_STATUS live_status );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションステータス 取得 
 */
//---------------------------------------------------------
extern FMISSION_LIVE_STATUS LINK_FESTIVAL_MissionLiveStatusGet( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション終了
 *
 * @retval  FES_BGM_RECOVER_NONE  BGMリカバー処理無し
 * @retval  その他 返ったBGMNoでリカバー処理をする
 */
//---------------------------------------------------------
extern u32 LINK_FESTIVAL_MissionEnd( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示しているトータルスコアを取得
 */
//---------------------------------------------------------
extern u16 LINK_FESTIVAL_FInfoTotalScoreGet( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示しているマイスコアを取得
 */
//---------------------------------------------------------
extern u16 LINK_FESTIVAL_FInfoMyScoreGet( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示しているスコアをアップデート
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_FInfoScoreUpdate( LINK_FESTIVAL* wk, u16 now_total_score );

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで表示するタイムボーナス値を取得
 */
//---------------------------------------------------------
extern u16 LINK_FESTIVAL_FInfoTimeBonusGet( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示するタイムボーナス値をリセット
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_FInfoTimeBonusReset( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル　保持しているタウンマップルートIDを更新
 *  @retval TRUE  更新された
 *  @retval FALSE 値は変わらなかった
 *
 *  tmap_root = TOWNMAP_UTIL_GetRootZoneID(cp_gamedata, zone_id);
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_TMapRootUpdate( LINK_FESTIVAL* wk, u16 new_root );

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル ミッションギミックワーク領域バッファへのポインタを返す
 */
//---------------------------------------------------------
extern void* LINK_FESTIVAL_GetMissionGimmickWork( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　現在のスコア
 *
 * @retval  現在のスコア
 */
//---------------------------------------------------------
extern u32 LINK_FESTIVAL_GetNowScore( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログ解析してスコアとミッション成功・失敗を判定
 *
 * @param u32* スコアを取得するu32のワークポインタ
 * @param u32* 取得するアイテム数を返すu32のワークポインタ
 * @retval  TRUE    ミッション成功
 * @retval  FALSE   ミッション失敗
 *
 * エントリーしてない時に呼び出すと不定の値が返るので注意
 * LINK_FESTIVAL_IsMissionEntry(), FEST_MISSION_IsEntry(&wk->mission)でチェックしてから呼ぶこと
 *
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_IsMissionClear( LINK_FESTIVAL* wk, u32* score );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ミッションの終了条件を満たしているか？
 *
 * @retval  TRUE    ミッション終了待ち状態
 * @retval  FALSE   ミッション中
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_IsMissionEnd( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル スコア加算
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_MissionScoreCalc( GAMESYS_WORK* gsys, FMISSION_LOG_ACTION action, u16 param );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション参加人数取得
 */
//---------------------------------------------------------
extern int LINK_FESTIVAL_GetMissionJoinNum( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　無視したミッションのログをストア
 *
 * @param fmwk ミッション情報を格納したFEST_MISSION構造体型ワークへのポインタ
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_EgnoreLogStore( LINK_FESTIVAL* wk, const FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　無視するミッションかチェック
 *
 * @param fmwk フェスビーコン情報を格納したFEST_MISSION構造体型ワークへのポインタ
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_IsEgnoreMission( LINK_FESTIVAL* wk, const FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル トップスコアアップデート
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_TopScoreUpdate( LINK_FESTIVAL* wk, u16 tr_id, u16 score, STRBUF* name );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル トップスコアアップデートチェック
 *
 * トップスコアが更新されたかどうかチェック
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_TopScoreIsUpdate( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 *  @brief リンクフェスティバル 今のトップスコアが自分のものかチェック
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_TopScoreIsMine( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 *  @brief リンクフェスティバル トップスコアを取得
 *
 *  @param  LINKFEST_LOG_TOP_SCORE構造体型ワークポインタ(LINKFEST_LOG_TOP_SCORE_Createで生成してください)
 *
 *  この関数を呼び出すと、アップデートチェック用フラグが落ちるので注意してください
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_TopScoreGet( LINK_FESTIVAL* wk, LINKFEST_LOG_TOP_SCORE* tsp );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル トップスコア格納ワーク生成
 */
//---------------------------------------------------------
extern LINKFEST_LOG_TOP_SCORE* LINKFEST_LOG_TOP_SCORE_Create( HEAPID heapID );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル トップスコア格納ワーク開放
 */
//---------------------------------------------------------
extern void LINKFEST_LOG_TOP_SCORE_Delete( LINKFEST_LOG_TOP_SCORE* sp );

//====================================================
//デバッグルーチン
//====================================================
#ifdef PM_DEBUG

/*
 *  @brief  ビーコン送信インターバル取得
 */
extern int DEB_LINKFEST_GetBeaconWait( LINK_FESTIVAL* wk );

/*
 *  @brief  ビーコン送信インターバル設定
 */
extern void DEB_LINKFEST_SetBeaconWait( LINK_FESTIVAL* wk, int wait );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ライフのデクリメントをとめるフラグをセットする
 */
//---------------------------------------------------------
extern void DEB_LINKFEST_SetTimeStopFlag( LINK_FESTIVAL* wk, BOOL flag );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ライフのデクリメントをとめるフラグをゲット
 */
//---------------------------------------------------------
extern BOOL DEB_LINKFEST_GetTimeStopFlag( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 配信スポットエミュレーションフラグをセットする
 */
//---------------------------------------------------------
extern void DEB_LINKFEST_SetSpotModeFlag( LINK_FESTIVAL* wk, BOOL flag );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル 配信モードエミュレーションフラグをゲット
 */
//---------------------------------------------------------
extern BOOL DEB_LINKFEST_GetSpotModeFlag( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ログスタックチェックスキップフラグをセットする
 */
//---------------------------------------------------------
extern void DEB_LINKFEST_SetLogStackCheckThrowFlag( LINK_FESTIVAL* wk, BOOL flag );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ログスタックチェックスキップフラグをゲットする
 */
//---------------------------------------------------------
extern BOOL DEB_LINKFEST_GetLogStackCheckThrowFlag( LINK_FESTIVAL* wk );

#endif  //PM_DEBUG


#endif //__H_LINK_FESTIVAL_H__

