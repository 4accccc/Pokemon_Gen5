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

#define BGM_FADEOUT_FRAME (6)
#define BGM_FADEIN_FRAME  (60)

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
 * レポート呼び出し( ワイヤレスセーブモードのチェック有 )
 *
 * @param ret_wk      結果を受け取るためのワーク
 */
//--------------------------------------------------------------
  .macro _ASM_REPORT_EVENT_CALL_WIRELESS ret_wk  
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _PUSH_WORK  SCWK_PARAM2
  _PUSH_WORK  SCWK_PARAM3
  _PUSH_WORK  SCWK_PARAM4
  _PUSH_WORK  SCWK_PARAM5


  _CHG_COMMON_SCR SCRID_REPORT_EVENT_CALL_WIRELESS
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

//--------------------------------------------------------------
// イベント入手イベント（隠しアイテム、エフェクトエンカウント共通）
//--------------------------------------------------------------
  .macro  _ASM_ITEM_EVENT_FIELD itemno, num
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _ASM_LDWKVAL  SCWK_PARAM0, \itemno
  _ASM_LDWKVAL  SCWK_PARAM1, \num
  _CHG_COMMON_SCR SCRID_ITEM_EVENT_FIELD
  _POP_WORK   SCWK_PARAM1
  _POP_WORK   SCWK_PARAM0
  .endm

//--------------------------------------------------------------
// イベント入手イベント（同上、ただし「▼」待ちあり）
//--------------------------------------------------------------
  .macro  _ASM_ITEM_EVENT_FIELD_KEYWAIT itemno, num
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _ASM_LDWKVAL  SCWK_PARAM0, \itemno
  _ASM_LDWKVAL  SCWK_PARAM1, \num
  _CHG_COMMON_SCR SCRID_ITEM_EVENT_FIELD_KEYWAIT
  _POP_WORK   SCWK_PARAM1
  _POP_WORK   SCWK_PARAM0
  .endm

//--------------------------------------------------------------
// 隠しイベント入手イベント「▼」待ちあり
//--------------------------------------------------------------
  .macro  _ASM_ITEM_EVENT_HIDEITEM itemno, num, flag_no
  _ASM_LDWKVAL  SCWK_PARAM0, \itemno
  _ASM_LDWKVAL  SCWK_PARAM1, \num
  _ASM_LDWKVAL  SCWK_PARAM2, \flag_no
  _CHG_COMMON_SCR SCRID_ITEM_EVENT_HIDEITEM
  .endm

//--------------------------------------------------------------
/**
 * @brief イベント入手成功イベント「▼」待ちあり
 */
//--------------------------------------------------------------
  .macro  _ASM_ITEM_EVENT_SUCCESS_KEYWAIT itemno, num
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _ASM_LDWKVAL  SCWK_PARAM0, \itemno
  _ASM_LDWKVAL  SCWK_PARAM1, \num
  _CHG_COMMON_SCR SCRID_ITEM_EVENT_SUCCESS_KEYWAIT
  _POP_WORK   SCWK_PARAM1
  _POP_WORK   SCWK_PARAM0
  .endm

//--------------------------------------------------------------
/**
 * @brief イベント入手成功イベント「▼」待ちなし
 */
//--------------------------------------------------------------
  .macro  _ASM_ITEM_EVENT_SUCCESS_NOWAIT itemno, num
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _ASM_LDWKVAL  SCWK_PARAM0, \itemno
  _ASM_LDWKVAL  SCWK_PARAM1, \num
  _CHG_COMMON_SCR SCRID_ITEM_EVENT_SUCCESS_NOWAIT
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
  _ASM_BALLOONWIN_TALKOBJ_MSG \msg_id 
  _ASM_LAST_KEYWAIT      
  _ASM_BALLOONWIN_CLOSE
  .endm

//--------------------------------------------------------------
/**
 * 簡易メッセージ表示 BG用
 * @param msg_id 表示するメッセージID
 */
//--------------------------------------------------------------
  .macro _ASM_EASY_MSG msg_id
  _ASM_TALK_START_SE_PLAY
  _ASM_SYSWIN_MSG_ALLPUT \msg_id,\POS_DOWN
  _ASM_LAST_KEYWAIT
  _ASM_SYSWIN_CLOSE
  .endm

//--------------------------------------------------------------
/**
 * 簡易BGウィンドウメッセージ表示
 * @param msg_id 表示するメッセージID
 * @param bg_type 表示するタイプ TYPE_INFO,TYPE_TOWN,TYPE_POST,TYPE_ROAD
 */
//--------------------------------------------------------------
  .macro _ASM_EASY_BGWIN_MSG msg_id, bg_type
  _ASM_TALK_START_SE_PLAY
  _ASM_BGWIN_MSG \msg_id, \bg_type
  _ASM_BGWIN_CLOSE
  .endm

//--------------------------------------------------------------
/**
 * 簡易BGウィンドウメッセージ表示     海底神殿　暗号用
 * @param msg_id 表示するメッセージID
 * @param bg_type 表示するタイプ TYPE_INFO,TYPE_TOWN,TYPE_POST,TYPE_ROAD
 */
//--------------------------------------------------------------
  .macro _ASM_EASY_BGWIN_SEATEMPLE_MSG msg_id, bg_type
  _ASM_TALK_START_SE_PLAY
  _ASM_BGWIN_SEATEMPLE_MSG \msg_id, \bg_type
  _ASM_BGWIN_CLOSE
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
    _ASM_VOICE_PLAY \monsno, 0
    _ASM_BALLOONWIN_TALKOBJ_MSG \msg_id 
    _ASM_VOICE_WAIT
    _ASM_LAST_KEYWAIT
    _ASM_BALLOONWIN_CLOSE
    .endm

//======================================================================
//  ショップ
//======================================================================
//--------------------------------------------------------------
/**
 * @brief ショップ呼び出し
 *
 * @param shop_id      ショップID
 */
//--------------------------------------------------------------
  .macro _ASM_SHOP_CALL shop_id,greeting_f
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1

  _ASM_LDWKVAL  SCWK_PARAM0, \shop_id
  _ASM_LDWKVAL  SCWK_PARAM1, \greeting_f
  _CHG_COMMON_SCR SCRID_SHOP_COUNTER

  _POP_WORK   SCWK_PARAM1
  _POP_WORK   SCWK_PARAM0
  .endm


//======================================================================
//    3Dデモ系
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 3Dデモ簡易呼び出し
 */
//--------------------------------------------------------------
  .macro  _ASM_CALL_3D_DEMO demo_no, scene_id
  _ASM_FIELD_CLOSE
  _ASM_DEMO_SCENE \demo_no, \scene_id
  _ASM_FIELD_OPEN
  .endm

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップブラックアウト呼び出し
 */
//--------------------------------------------------------------
#define _DEMO_FADEOUT_BLACK_CALL() _ASM_DEMO_FADEOUT_BLACK_CALL

  .macro  _ASM_DEMO_FADEOUT_BLACK_CALL
  _ASM_MAP_FADE_BLACK_OUT
  _ASM_MAP_FADE_END_CHECK
  .endm

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップブラックイン呼び出し
 */
//--------------------------------------------------------------
#define _DEMO_FADEIN_BLACK_CALL() _ASM_DEMO_FADEIN_BLACK_CALL

  .macro  _ASM_DEMO_FADEIN_BLACK_CALL
  _ASM_MAP_FADE_BLACK_IN
  _ASM_MAP_FADE_END_CHECK
  .endm

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップブラックアウト　BGMPush付き
 */
//--------------------------------------------------------------
#define _DEMO_FADEOUT_BLACK_WITH_BGM_CALL() _ASM_DEMO_FADEOUT_BLACK_WITH_BGM_CALL

  .macro  _ASM_DEMO_FADEOUT_BLACK_WITH_BGM_CALL
  _ASM_MAP_FADE_BLACK_OUT
  _ASM_BGM_PUSH BGM_FADEOUT_FRAME 
  _ASM_MAP_FADE_END_CHECK
  .endm

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップブラックイン BGMPop付き
 */
//--------------------------------------------------------------
#define _DEMO_FADEIN_BLACK_WITH_BGM_CALL() _ASM_DEMO_FADEIN_BLACK_WITH_BGM_CALL

  .macro  _ASM_DEMO_FADEIN_BLACK_WITH_BGM_CALL
  _ASM_MAP_FADE_BLACK_IN
  _ASM_BGM_POP  0,BGM_FADEIN_FRAME 
  _ASM_MAP_FADE_END_CHECK
  .endm

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップホワイトアウト呼び出し
 */
//--------------------------------------------------------------
#define _DEMO_FADEOUT_WHITE_CALL() _ASM_DEMO_FADEOUT_WHITE_CALL

  .macro  _ASM_DEMO_FADEOUT_WHITE_CALL
  _ASM_MAP_FADE_WHITE_OUT
  _ASM_MAP_FADE_END_CHECK
  .endm

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップホワイトイン呼び出し
 */
//--------------------------------------------------------------
#define _DEMO_FADEIN_WHITE_CALL() _ASM_DEMO_FADEIN_WHITE_CALL

  .macro  _ASM_DEMO_FADEIN_WHITE_CALL
  _ASM_MAP_FADE_WHITE_IN
  _ASM_MAP_FADE_END_CHECK
  .endm

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップホワイトアウト　BGMPush付き
 */
//--------------------------------------------------------------
#define _DEMO_FADEOUT_WHITE_WITH_BGM_CALL() _ASM_DEMO_FADEOUT_WHITE_WITH_BGM_CALL

  .macro  _ASM_DEMO_FADEOUT_WHITE_WITH_BGM_CALL
  _ASM_MAP_FADE_WHITE_OUT
  _ASM_BGM_PUSH BGM_FADEOUT_FRAME 
  _ASM_MAP_FADE_END_CHECK
  .endm

//--------------------------------------------------------------
/**
 *  @brief  3Dデモ専用マップホワイトイン BGMPop付き
 */
//--------------------------------------------------------------
#define _DEMO_FADEIN_WHITE_WITH_BGM_CALL() _ASM_DEMO_FADEIN_WHITE_WITH_BGM_CALL

  .macro  _ASM_DEMO_FADEIN_WHITE_WITH_BGM_CALL
  _ASM_MAP_FADE_WHITE_IN
  _ASM_BGM_POP  0,BGM_FADEIN_FRAME 
  _ASM_MAP_FADE_END_CHECK
  .endm

//======================================================================
//    簡易アプリ操作
//======================================================================

//--------------------------------------------------------------
/**
 * @brief テレビトランシーバーデモ簡易呼び出し
 */
//--------------------------------------------------------------
  .macro  _ASM_CALL_TVT_DEMO demo_no
  _ASM_FIELD_CLOSE
  _ASM_TVT_DEMO \demo_no
  _ASM_FIELD_OPEN
  .endm

//--------------------------------------------------------------
/**
 *  対戦受付　レギュレーションに合うパーティが見つからなかったときのメッセージ表示 
 *
 * @param ret_wk      結果を受け取るためのワーク
 */
//--------------------------------------------------------------
  .macro _ASM_BTL_UTIL_PARTY_REGULATION_NG_MSG_CALL regulation
  _PUSH_WORK  SCWK_PARAM0
  
  _ASM_LDWKVAL  SCWK_PARAM0, \regulation
  _CHG_COMMON_SCR SCRID_BTL_UTIL_PARTY_REGULATION_NG_MSG_CALL
  
  _POP_WORK   SCWK_PARAM0
  .endm

//======================================================================
//  表彰状＆路線図アプリ呼び出し
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 表彰状＆路線図アプリ呼び出し
 *
 * @param demo_id
 * @param pattern
 */
//--------------------------------------------------------------
  .macro _ASM_ZUKAN_AWARD_PROC_CALL demo_id,pattern
  
  _MAP_FADE_BLACK_OUT()
  _MAP_FADE_END_CHECK()
  _FIELD_CLOSE()
  
  _ASM_CALL_ZUKAN_AWARD \demo_id,\pattern

  _FIELD_OPEN()
  _MAP_FADE_BLACK_IN()
  _MAP_FADE_END_CHECK()
 
  .endm

//======================================================================
//  
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 最初の３体選択
 *
 * @param ret_wk
 */
//--------------------------------------------------------------
  .macro _ASM_FIRST_POKE_SELECT_CALL ret_wk
  
  _ASM_FIELD_CLOSE
  _ASM_CALL_FIRST_POKE_SELECT \ret_wk
  _ASM_FIELD_OPEN
 
  .endm

//======================================================================
//観覧車トレーナー関連  
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 観覧車トレーナー 登録ID取得(0～7の登録ID tr_idとは別)
 *
 * @param ret_wk
 */
//--------------------------------------------------------------
  .macro _ASM_GET_WHEEL_TRAINER_ENTRY_ID ret_wk
  
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  
  _GET_SEASON_ID( SCWK_PARAM0 )
  _GET_MY_SEX( SCWK_PARAM1 )

  _ASM_LDWKVAL  \ret_wk, SCWK_PARAM0
  _ASM_MUL_WK   \ret_wk,2
  _ASM_ADD_WK   \ret_wk,SCWK_PARAM1
  
  _POP_WORK  SCWK_PARAM1
  _POP_WORK  SCWK_PARAM0
  .endm

//--------------------------------------------------------------
/**
 * @brief 観覧車トレーナーメッセージ
 *
 * @param ret_wk
 */
//--------------------------------------------------------------
  .macro _ASM_WHEEL_TRAINER_MSG msg_id, id, f_2nd
  
  _PUSH_WORK  SCWK_PARAM0
  _PUSH_WORK  SCWK_PARAM1
  _PUSH_WORK  SCWK_PARAM2

  _ASM_LDWKVAL  SCWK_PARAM0, \msg_id
  _ASM_LDWKVAL  SCWK_PARAM1, \id
  _ASM_LDWKVAL  SCWK_PARAM2, \f_2nd
  
  _ASM_MUL_WK SCWK_PARAM1,10
  _ASM_MUL_WK SCWK_PARAM2,5
  _ASM_ADD_WK SCWK_PARAM0,SCWK_PARAM1
  _ASM_ADD_WK SCWK_PARAM0,SCWK_PARAM2
 
  _ASM_PLAYER_NAME  0
  _ASM_BALLOONWIN_TALKOBJ_MSG SCWK_PARAM0
 
  _POP_WORK  SCWK_PARAM2
  _POP_WORK  SCWK_PARAM1
  _POP_WORK  SCWK_PARAM0
  .endm

