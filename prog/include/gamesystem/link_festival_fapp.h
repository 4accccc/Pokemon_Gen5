//==============================================================================
/**
 * @file    link_festival_fapp.h
 * @brief   リンクフェスティバル 詳細画面アプリでしか呼び出さない関数をFMISSION_APPオーバーレイに追い出し
 * @author  iwasawa
 * @date    2011.09.25
 */
//==============================================================================

#pragma once

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ミッション結果を取得＆レコードをセーブに反映
 *
 * @param fest_sys  LINK_FESTIVAL*
 * @param pl  LINKFEST_MISSION_RESULT構造体型へのポインタ
 *
 *  ミッション達成回数などをインクリメントするので複数回呼び出さないこと
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_GetMissionResult( LINK_FESTIVAL* wk, LINKFEST_MISSION_RESULT* pl);

