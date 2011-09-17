//======================================================================
/**
 * @file  pkwd_scr_define.h
 * @date  2011.07.05
 * @author  tamada GAMEFREAK inc.
 */
//======================================================================

#pragma once

//--------------------------------------------------------------
//  ポケウッド撮影イベント状態定義
//--------------------------------------------------------------
#define   PKWD_REC_SEQ_END              0
#define   PKWD_REC_SEQ_FIRST_MENU       1
#define   PKWD_REC_SEQ_SELECT_SCENARIO  2
#define   PKWD_REC_SEQ_SELECT_POKE      3
#define   PKWD_REC_SEQ_POKELIST         4
#define   PKWD_REC_SEQ_SHOOTING         5
#define   PKWD_REC_SEQ_AFTER            6
#define   PKWD_REC_SEQ_ASK_GO_THEATER   7
#define   PKWD_REC_SEQ_GOODBYE          8

//--------------------------------------------------------------
//  ポケウッド撮影イベント状態定義（撮影後サブイベント用）
//--------------------------------------------------------------
#define   PKWD_REC_SUBSEQ_END           0
#define   PKWD_REC_SUBSEQ_ASK_PLAY      1
#define   PKWD_REC_SUBSEQ_ASK_NOT_PLAY  2
#define   PKWD_REC_SUBSEQ_ASK_DELETE    3
#define   PKWD_REC_SUBSEQ_REC           4



//--------------------------------------------------------------
//  ポケウッド上映イベント状態定義
//--------------------------------------------------------------
#define   PKWD_SHOW_SEQ_END             0
#define   PKWD_SHOW_SEQ_GOODBYE         1
#define   PKWD_SHOW_SEQ_FIRST_MENU      2
#define   PKWD_SHOW_SEQ_SELECT_MOVIE    3
#define   PKWD_SHOW_SEQ_WATCH_MOVIE     4
#define   PKWD_SHOW_SEQ_AFTER           5


