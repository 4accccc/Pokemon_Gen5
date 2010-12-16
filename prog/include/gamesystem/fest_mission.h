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

typedef enum{
 FEST_MISSION_NULL,
 FEST_MISSION_WILDBTL_WIN,
}FEST_MISSION_ID;

#define FMISSION_PARAM_MAX  (2)

typedef struct{
  u8  id;     ///<ミッションID
  u8  time;   ///<設定時間(分)
  u16 reward; ///<報酬
  u16 prm[FMISSION_PARAM_MAX]; ///<ミッションパラメータ
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
 * @brief ミッション開始処理
 */
//---------------------------------------------------------
extern void FEST_MISSION_Start( FEST_MISSION* fmwk, FEST_MISSION_ID id, u32 key, u16 param1, u16 param2, u32 life );

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

