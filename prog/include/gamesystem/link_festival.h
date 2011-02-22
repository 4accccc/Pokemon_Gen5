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

#define LINKFEST_LOG_MAX  (100) //フェスタミッションログMax
#define LINKFEST_LOG_END  (LINKFEST_LOG_MAX-1) //フェスタミッションログEndIndex

typedef struct _LINK_FESTIVAL LINK_FESTIVAL;

//フェスタログデータ構造体
typedef struct _LINKFEST_LOG{
  union{  //8byteのログ領域
    struct{ //スコアログ
      u32 tr_id;
      u32 value;
    }score;

    struct{ //受信ミッションログ
      u8  type;
      u16 param;
      u32 key;
    }mission;
  };
}LINKFEST_LOG;

//フェスタログ受け渡し構造体
typedef struct _LINKFEST_LOG_PARAM{
  u8  type;
  u32 key;
  LINKFEST_LOG prm;
}LINKFEST_LOG_PARAM;




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
 * @brief リンクフェスティバル ミッションワークポインタ取得
 */
//---------------------------------------------------------
extern FEST_MISSION* LINK_FESTIVAL_GetMissionWork( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションスタート
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_MissionStart( LINK_FESTIVAL* wk, const FEST_MISSION* mission );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション終了
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_MissionEnd( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示しているスコアを取得
 */
//---------------------------------------------------------
extern u16 LINK_FESTIVAL_FInfoScoreGet( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 *  @brief  リンクフェスティバル フィールドで現在表示しているスコアをインクリメント
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_FInfoScoreInc( LINK_FESTIVAL* wk );
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
 * @brief リンクフェスティバル　ログ解析してスコアとミッション成功・失敗を判定
 *
 * @param u32* スコアを取得するu32のワークポインタ
 * @retval  TRUE    ミッション成功
 * @retval  FALSE   ミッション失敗
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_GetMissionResult( LINK_FESTIVAL* wk, u32* score );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル スコア加算
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_MissionScoreCalc( GAMESYS_WORK* gsys, u16 param, u16 inc );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッション参加人数取得
 */
//---------------------------------------------------------
extern int LINK_FESTIVAL_GetMissionEntryNum( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログストア
 *
 * @param log 情報を格納したLINKFEST_LOG_PARAM構造体型ワークへのポインタ
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_ScoreLogStore( LINK_FESTIVAL* wk, LINKFEST_LOG_PARAM* src );

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
 * @param fmwk ミッション情報を格納したFEST_MISSION構造体型ワークへのポインタ
 */
//---------------------------------------------------------
extern BOOL LINK_FESTIVAL_IsEgnoreMission( LINK_FESTIVAL* wk, const FEST_MISSION* fmwk );


//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログ構造体クリア
 */
//---------------------------------------------------------
extern void LINKFEST_LOG_Clear( LINKFEST_LOG* log );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ログパラメータ構造体クリア
 */
//---------------------------------------------------------
extern void LINKFEST_LOG_PARAM_Clear( LINKFEST_LOG_PARAM* prm );


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

#endif  //PM_DEBUG


#endif //__H_LINK_FESTIVAL_H__

