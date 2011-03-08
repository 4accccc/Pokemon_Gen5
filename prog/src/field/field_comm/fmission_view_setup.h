//==============================================================================
/**
 * @file    beacon_view_setup.c
 * @brief   すれ違い通信&フェスミッション画面　セットアップ系サブルーチン ヘッダ
 * @author  iwasawa 
 * @date    2010.12.20
 */
//==============================================================================

#pragma once

typedef enum{
  EVWAIT_MENU_ANM,
  EVWAIT_TCB_ANM,
  EVWAIT_TMENU_ANM,
}EVWAIT_TYPE;


/*
 *  @brief　一番最初のワーク初期化
 */
extern FMISSION_VIEW* FVS_InitWork( GAMESYS_WORK* gsys, FIELD_SUBSCREEN_WORK* subscreen );

/*
 *  @brief  一番最後のワーク解放
 */
extern void FVS_ReleaseWork( FMISSION_VIEW_PTR wk );


////////////////////////////////////////////////////////////////////////////
//イベントリクエスト系
////////////////////////////////////////////////////////////////////////////

/*
 *  @brief  イベントリクエスト
 */
extern void FVSEvent_Reserve( FMISSION_VIEW_PTR wk, BEACON_DETAIL_EVENT ev_id);

/*
 *  @brief  イベントリクエストリセット
 */
extern void FVSEvent_ReserveReset( FMISSION_VIEW_PTR wk );

/*
 *  @brief  イベントチェックコモン
 */
extern GMEVENT* FVSEvent_CheckCommon( FMISSION_VIEW_PTR wk, BOOL bEvReqOK );

/*
 *  @brief  各種イベント起動前に、アニメなどの待ちをするためのイベント
 */
extern GMEVENT* FVSEvent_EventStart( FMISSION_VIEW_PTR wk, GMEVENT* call_event, EVWAIT_TYPE type, int param );


