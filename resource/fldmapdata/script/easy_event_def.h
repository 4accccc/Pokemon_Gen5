//======================================================================
/**
 * @file    easy_event_def.h
 * @brief   簡易イベント定義ファイル
 * @date    2009.09.08
 * @author  tamada GAMEFREAK inc.
 *
 *
 * 実際にスクリプト中で使用する定義はscr_seq_def.hに記述されている。
 * こちらの定義を直接呼び出さないこと！！
 */
//======================================================================

//--------------------------------------------------------------
/**
 * レポート呼び出し
 *
 * @param ret_wk      結果を受け取るためのワーク
 */
//--------------------------------------------------------------
  .macro _ASM_REPORT_EVENT_CALL ret_wk
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _PUSH_WORK  SCWK_PARAM2
  _PUSH_WORK  SCWK_PARAM3
  _PUSH_WORK  SCWK_PARAM4
  _PUSH_WORK  SCWK_PARAM5


  _CHG_COMMON_SCR SCRID_REPORT_EVENT_CALL
  _ASM_LDWKVAL  \ret_wk, SCWK_PARAM0


  _POP_WORK   SCWK_PARAM5
  _POP_WORK   SCWK_PARAM4
  _POP_WORK   SCWK_PARAM3
  _POP_WORK   SCWK_PARAM2
  _POP_WORK   SCWK_PARAM1
  _POP_WORK   SCWK_PARAM0

  .endm

//--------------------------------------------------------------
/**
 * フィールド通信切断イベント呼び出し
 *
 * @param ret_wk      結果を受け取るためのワーク
 */
//--------------------------------------------------------------
  .macro _ASM_FIELD_COMM_EXIT_EVENT_CALL ret_wk
  _PUSH_WORK  SCWK_PARAM0

  _CHG_COMMON_SCR SCRID_FIELD_COMM_EXIT_EVENT_CALL
  _ASM_LDWKVAL  \ret_wk, SCWK_PARAM0

  _POP_WORK   SCWK_PARAM0
  .endm

//--------------------------------------------------------------
/**
 * 簡易イベント入手イベント
 *
 * @param itemno        取得するアイテムナンバー
 * @param num           取得するアイテムの数
 * @param flag          アイテム取得したかどうかの判定用フラグ
 * @param before_msg    入手前のメッセージ
 * @param finish_msg    入手後のメッセージ
 * @param after_msg     入手後、再び話かけたときのメッセージ
 *
 * SCWK_PARAM0～SCWK_PARAM6を使用して引数を渡している
 */
//--------------------------------------------------------------
  .macro  _ASM_EASY_TALK_ITEM_EVENT itemno, num, flag, before_msg, finish_msg, after_msg
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _PUSH_WORK  SCWK_PARAM2
  _PUSH_WORK  SCWK_PARAM3
  _PUSH_WORK  SCWK_PARAM4
  _PUSH_WORK  SCWK_PARAM5
  _PUSH_WORK  SCWK_PARAM6

  _ASM_LDWKVAL  SCWK_PARAM0, \itemno
  _ASM_LDWKVAL  SCWK_PARAM1, \num
  _ASM_LDWKVAL  SCWK_PARAM2, \flag
  _ASM_LDWKVAL  SCWK_PARAM3, \before_msg
  _ASM_LDWKVAL  SCWK_PARAM4, \finish_msg
  _ASM_LDWKVAL  SCWK_PARAM5, \after_msg
  _ASM_GET_NOW_MSG_ARCID  SCWK_PARAM6

  _CHG_COMMON_SCR SCRID_EASY_ITEM_EVENT
  
  _POP_WORK  SCWK_PARAM6
  _POP_WORK  SCWK_PARAM5
  _POP_WORK  SCWK_PARAM4
  _POP_WORK  SCWK_PARAM3
  _POP_WORK  SCWK_PARAM2
  _POP_WORK  SCWK_PARAM1
  _POP_WORK  SCWK_PARAM0
  .endm

//--------------------------------------------------------------
// イベント入手イベント「▼」待ちあり
//--------------------------------------------------------------
  .macro  _ASM_ITEM_EVENT_KEYWAIT itemno, num
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _ASM_LDWKVAL  SCWK_PARAM0, \itemno
  _ASM_LDWKVAL  SCWK_PARAM1, \num
  _CHG_COMMON_SCR SCRID_ITEM_EVENT_KEYWAIT
  _POP_WORK   SCWK_PARAM1
  _POP_WORK   SCWK_PARAM0
  .endm


//--------------------------------------------------------------
// イベント入手イベント「▼」待ちなし
//--------------------------------------------------------------
  .macro  _ASM_ITEM_EVENT_NOWAIT  itemno, num
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _ASM_LDWKVAL  SCWK_PARAM0, \itemno
  _ASM_LDWKVAL  SCWK_PARAM1, \num
  _CHG_COMMON_SCR SCRID_ITEM_EVENT_NOWAIT
  _POP_WORK   SCWK_PARAM1
  _POP_WORK   SCWK_PARAM0
  .endm

//======================================================================
//   簡易メッセージコマンド
//======================================================================
//--------------------------------------------------------------
/**
 * 簡易吹き出しメッセージ表示 話し掛けOBJ用
 * @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
  .macro _ASM_EASY_BALLOONWIN_TALKOBJ_MSG msg_id
  _ASM_TALK_START_SE_PLAY
  _ASM_TURN_HERO_SITE   
  _ASM_BALLOONWIN_TALKOBJ_OPEN \msg_id 
  _ASM_LAST_KEYWAIT      
  _ASM_BALLOONWIN_CLOSE
  .endm

//--------------------------------------------------------------
/**
 * 簡易メッセージ表示 BG用
 * @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
  .macro  _ASM_EASY_MSG msg_id
  _ASM_TALK_START_SE_PLAY
  _ASM_TALKWIN_OPEN
  _ASM_TALKMSG_ALLPUT \msg_id
  _ASM_LAST_KEYWAIT
  _ASM_TALKWIN_CLOSE
  .endm


//--------------------------------------------------------------
/**
 * 簡易鳴き声メッセージコマンド
 *
 * @param monsno
 * @param msg_id
 */
//--------------------------------------------------------------
    .macro  _ASM_EASY_TALK_POKEVOICE monsno, msg_id
    _ASM_TALK_START_SE_PLAY
    _ASM_TURN_HERO_SITE   
    _ASM_VOICE_PLAY \monsno
    _ASM_BALLOONWIN_TALKOBJ_OPEN \msg_id 
    _ASM_VOICE_WAIT
    _ASM_LAST_KEYWAIT
    _ASM_BALLOONWIN_CLOSE
    .endm

//--------------------------------------------------------------
/**
 * @brief ショップ呼び出し
 *
 * @param shop_id      ショップID
 */
//--------------------------------------------------------------
  .macro _ASM_SHOP_CALL shop_id
  _PUSH_WORK  SCWK_PARAM0

  _ASM_LDWKVAL  SCWK_PARAM0, \shop_id
  _CHG_COMMON_SCR SCRID_SHOP_COUNTER

  _POP_WORK   SCWK_PARAM0
  .endm

