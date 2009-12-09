//============================================================================================
/**
 * @file  mb_seq.h
 * @brief メールボックス画面 メインシーケンス
 * @author  Akito Mori / Hiroyuki Nakamura
 * @date  09.12.03
 */
//============================================================================================
#ifndef __MB_SEQ_H__
#define __MB_SEQ_H__

//============================================================================================
//  定数定義
//============================================================================================

// メインシーケンス定義
enum {
  MBSEQ_MAINSEQ_INIT = 0,       // 初期化
  MBSEQ_MAINSEQ_RELEASE,        // 解放
  MBSEQ_MAINSEQ_WIPE,         // ワイプ
  MBSEQ_MAINSEQ_MSG_WAIT,       // メッセージウェイト
  MBSEQ_MAINSEQ_YESNO,        // はい・いいえ
  MBSEQ_MAINSEQ_BUTTON_ANM,     // ボタンアニメ
  MBSEQ_MAINSEQ_SUBPROC_CALL,     // サブプロセス呼び出し
  MBSEQ_MAINSEQ_SUBPROC_MAIN,     // サブプロセスメイン
  MBSEQ_MAINSEQ_START,        // メールボックス開始
  MBSEQ_MAINSEQ_MAIL_SELECT_MAIN,   // メール選択メイン
  MBSEQ_MAINSEQ_MAILBOX_END_SET,    // メールボックス終了処理
  MBSEQ_MAINSEQ_MAIL_SELECT_PAGE_CHG, // メールリストページ切り替え
  MBSEQ_MAINSEQ_MAIL_MENU_MSG_SET,  // メニューメッセージセット
  MBSEQ_MAINSEQ_MAIL_MENU_SET,    // メニューセット
  MBSEQ_MAINSEQ_MAIL_MENU_MAIN,   // メニューメイン
  MBSEQ_MAINSEQ_MAIL_READ_MAIN,   //「メールをよむ」メイン
  MBSEQ_MAINSEQ_MAIL_READ_END,    //「メールをよむ」終了
  MBSEQ_MAINSEQ_MAILDEL_YESNO_SET,    //「メールをけす」はい・いいえセット
  MBSEQ_MAINSEQ_MAILPOKESET_MSG,      // メールをもたせる？メッセージセット
  MBSEQ_MAINSEQ_MAILPOKESET_YESNO_SET,  //メールをもたせる？はい・いいえセット
  MBSEQ_MAINSEQ_ERASEMAIL_POKELIST_EXIT,    // 削除メール添付ポケモンリストからの戻り先
  MBSEQ_MAINSEQ_ERASEMAIL_POKESET_CANCEL,   // 削除メールポケ添付キャンセル
  MBSEQ_MAINSEQ_ERASEMAIL_POKESET_CANCEL_END, // 削除メールポケ添付キャンセル終了
  MBSEQ_MAINSEQ_MAILWRITE_END,        // メール作成後
  MBSEQ_MAINSEQ_MAILDEL_END,        //「メールをけす」終了
  MBSEQ_MAINSEQ_END         // 終了

};


//============================================================================================
//  プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス制御
 *
 * @param syswk メールボックス画面システムワーク
 * @param seq   メインシーケンス
 *
 * @retval  "TRUE = 処理中"
 * @retval  "FALSE = 終了"
 */
//--------------------------------------------------------------------------------------------
extern BOOL MBSEQ_Main( MAILBOX_SYS_WORK * syswk, int * seq );


#endif  // __MB_SEQ_H__
