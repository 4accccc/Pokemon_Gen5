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

typedef enum{
 FEST_MISSION_NULL,
 FEST_MISSION_WILDBTL_WIN,
 FEST_MISSION_ID_MAX,
 FEST_MISSION_ID_END = FEST_MISSION_ID_MAX-1,
}FEST_MISSION_ID;

typedef enum{
  FMISSION_TYPE_WILDBTL_WIN,
  FMISSION_TYPE_MAX,
}FMISSION_TYPE_ID;

typedef struct{
  u8  time;
  u8  reward;
  u16 quota;  //ノルマ 
  u16 param;
}FEST_MISSION_PARAM;

typedef struct{
  u8  id;     ///<ミッションID
  u8  time;   ///<設定時間(分)
  u16 reward; ///<報酬
  u16 quota; ///<ノルマ
  u16 param;  ///<ミッション汎用パラメータ
  u32 key;    ///<ミッション固有マジックナンバー

  u32 life; ///<ミッション残り時間(frm)

  u32 my_score; ///<自分のスコア
}FEST_MISSION;

//---------------------------------------------------------
/*
 * @brief ミッション管理領域クリア
 */
//---------------------------------------------------------
extern void FEST_MISSION_Clear( FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッションパラメータセット
 */
//---------------------------------------------------------
extern void FEST_MISSION_ParamSet( FEST_MISSION* fmwk, FEST_MISSION_ID id, u32 reward, u16 quota, u16 param, u32 time );

//---------------------------------------------------------
/*
 * @brief ミッション開始処理
 */
//---------------------------------------------------------
extern void FEST_MISSION_Start( FEST_MISSION* fmwk, const FEST_MISSION* src );

//---------------------------------------------------------
/*
 * @brief ミッション終了処理
 */
//---------------------------------------------------------
extern void FEST_MISSION_End( FEST_MISSION* fmwk );

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
extern BOOL FEST_MISSION_IsEnd( FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッションが生きているか取得
 *
 * @retval  TRUE   ミッション中且つライフ有
 * @retval  FALSE  ミッション中でないorミッションのライフ切れ
 */
//---------------------------------------------------------
extern BOOL FEST_MISSION_IsLive( FEST_MISSION* fmwk );

/*
 * @brief ミッションに参加しているか？
 *
 * @retval  TRUE   ミッション中
 * @retval  FALSE  ミッション中でない
 */
//---------------------------------------------------------
extern BOOL FEST_MISSION_IsEntry( FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッション　スコアインクリメント
 */
//---------------------------------------------------------
extern void FEST_MISSION_ScoreCalc( FEST_MISSION* fmwk, FMISSION_TYPE_ID type, u32 param, u32 value );

//---------------------------------------------------------
/*
 * @brief ミッションパラメータ取得
 */
//---------------------------------------------------------
extern void FEST_MISSION_ParamDataGet( FEST_MISSION_ID id, FEST_MISSION* dest );

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

