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

#define BUFLEN_FMISSION_TITLE (15*2+EOM_SIZE) //15文字*2+EOM
#define BUFLEN_FMISSION_EXP   (18*3*2+EOM_SIZE) //(18文字*3行)*2+EOM

///外部参照用
typedef struct _FEST_MISSION FEST_MISSION;

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
 * @brief ミッション　スコアインクリメント
 */
//---------------------------------------------------------
extern BOOL FEST_MISSION_ScoreCalc( FEST_MISSION* fmwk, u16 param, u16 inc, u32* out_bonus );

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

