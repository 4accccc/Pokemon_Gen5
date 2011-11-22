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
  u8    new_record_time_f:1; //残り時間のニューレコード
  u8    :6;

  u32   score;    //スコア
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
 * @brief リンクフェスティバル ミッションアップデート　有効無効設定
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_MissionLiveStatusSet( LINK_FESTIVAL* wk, BOOL live_f );

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
extern int LINK_FESTIVAL_GetMissionEntryNum( LINK_FESTIVAL* wk );

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

#endif  //PM_DEBUG


#endif //__H_LINK_FESTIVAL_H__

