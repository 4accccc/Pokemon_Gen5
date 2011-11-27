//==============================================================================
/**
 * @file    fest_mission.h
 * @brief   リンクフェスティバル ミッション管理
 * @author  iwasawa
 * @date    2010.12.16
 */
//==============================================================================

#ifndef __H_FEST_MISSION_H__
#define __H_FEST_MISSION_H__

#define OSPRINT_LINKFEST_PAGE (2) //OS_FPrintf出力ページ指定

#define FMISSION_FWORD_MESSAGE_LEN  (8) //フェスミッション用フリーワード文字列長 _EOM抜き8文字
#define BUFLEN_FMISSION_TITLE (15*2+EOM_SIZE) //15文字*2+EOM
#define BUFLEN_FMISSION_EXP   (18*4*2+EOM_SIZE) //(18文字*4行)*2+EOM

///外部参照用
typedef union  _FEST_BEACON_ID  FEST_BEACON_ID;
typedef struct _FEST_BEACON FEST_BEACON;
typedef struct _FEST_MISSION FEST_MISSION;

#include "gamesystem/fmission_data_id.h"
#include "gamesystem/fmission_data_def.h"
#include "gamesystem/fmission_log.h"

//---------------------------------------------------------
/*
 * @brief FEST_MISSION型ワークを生成
 */
//---------------------------------------------------------
extern FEST_MISSION* FEST_MISSION_AllocWork( HEAPID heapID );

//---------------------------------------------------------
/*
 * @brief ミッション管理領域クリア
 */
//---------------------------------------------------------
extern void FEST_MISSION_Clear( FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッション開始処理
 */
//---------------------------------------------------------
extern void FEST_MISSION_Start( FEST_MISSION* fmwk, const FEST_MISSION* src, BOOL succeed_f );

//---------------------------------------------------------
/*
 * @brief ミッションプレイ開始処理
 *
 * LINK_FESTIVAL_MissionPlayStart()の内部からだけ呼び出しOK
 */
//---------------------------------------------------------
extern void FEST_MISSION_PlayStart( FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッション終了処理
 */
//---------------------------------------------------------
extern void FEST_MISSION_End( FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッションRTCセット
 *
 * RTCパラメータを現パラメータでセット
 */
//---------------------------------------------------------
extern void FEST_MISSION_SetRTC( FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッションRTC 残り時間取得
 *
 * 現在のRTCを取得し、start_rtcとの差分から残り時間を取得する
 */
//---------------------------------------------------------
extern u32 FEST_MISSION_GetLife( const FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッションRTC エントリーに十分な残り時間が残っているか？
 *
 * 現在のRTCを取得し、start_rtcとの差分から残り時間を取得
 * エントリー条件に十分な残り時間があるかを返す
 */
//---------------------------------------------------------
extern BOOL FEST_MISSION_IsLifeLeftForEntry( const FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッション状態監視処理
 */
//---------------------------------------------------------
extern void FEST_MISSION_Update( FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッション終了監視(ミッション中でない場合、常にFalse)
 *
 * @retval  TRUE   ミッション終了
 * @retval  FALSE  ミッション中でないorミッション継続中
 */
//---------------------------------------------------------
extern BOOL FEST_MISSION_IsEnd( const FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッションが生きているか取得
 *
 * @retval  TRUE   ミッション中且つライフ有
 * @retval  FALSE  ミッション中でないorミッションのライフ切れ
 */
//---------------------------------------------------------
extern BOOL FEST_MISSION_IsLive( const FEST_MISSION* fmwk );

/*
 * @brief ミッションに参加しているか？
 *
 * @retval  TRUE   ミッション中
 * @retval  FALSE  ミッション中でない
 */
//---------------------------------------------------------
extern BOOL FEST_MISSION_IsEntry( const FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッション　スコアインクリメントチェック
 */
//---------------------------------------------------------
extern BOOL FEST_MISSION_IsScoreUp( FEST_MISSION* fmwk, FMISSION_LOG_ACTION action, u16 value );

//---------------------------------------------------------
/*
 * @brief ミッション　スコアインクリメント
 */
//---------------------------------------------------------
extern BOOL FEST_MISSION_ScoreCalc( FEST_MISSION* fmwk,  FMISSION_LOG_ACTION action, u16 param, u16 inc );

//---------------------------------------------------------
/*
 * @brief ミッション　タイムボーナス計算
 */
//---------------------------------------------------------
extern u32 FEST_MISSION_TimeBonusCalc( FEST_MISSION* fmwk,  u16 score_diff );

//---------------------------------------------------------
/*
 * @brief フェスビーコン比較
 */
//---------------------------------------------------------
extern BOOL FEST_MISSION_IsSame( const FEST_MISSION* f1, const FEST_MISSION* f2 );

//=================================================
//以下デバッグルーチン
//=================================================

#ifdef PM_DEBUG

//---------------------------------------------------------
/*
 * @brief ミッション寿命強制破棄
 */
//---------------------------------------------------------
extern void FEST_MISSION_DEBUG_KillMission( FEST_MISSION* fmwk );

#endif  //PM_DEBUG


#endif //__H_FEST_MISSION_H__

