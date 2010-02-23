//============================================================================================
/**
 * @file  mb_main.h
 * @brief メールボックス画面 メイン処理
 * @author  Hiroyuki Nakamura
 * @date  09.01.31
 */
//============================================================================================
#ifndef __MB_MAIN_H__
#define __MB_MAIN_H__

#include "savedata/mail_util.h"
#include "system/bmp_winframe.h"
//#include "system/touch_subwindow.h"
#include "print/wordset.h"
#include "system/bgwinfrm.h"
#include "system/cursor_move.h"
#include "system/selbox.h"
#include "system/pms_draw.h"
#include "app/mailbox.h"
#include "app/app_taskmenu.h"

//============================================================================================
//  定数定義
//============================================================================================

// ＢＧフレーム
#define MBMAIN_BGF_MSG_M    ( GFL_BG_FRAME0_M )
#define MBMAIN_BGF_STR_M    ( GFL_BG_FRAME1_M )
#define MBMAIN_BGF_BTN_M    ( GFL_BG_FRAME2_M )
#define MBMAIN_BGF_BG_M     ( GFL_BG_FRAME3_M )
#define MBMAIN_BGF_MAILMES_S  ( GFL_BG_FRAME0_S )
#define MBMAIN_BGF_MAIL_S   ( GFL_BG_FRAME1_S )
#define MBMAIN_BGF_STR_S    ( GFL_BG_FRAME2_S )
#define MBMAIN_BGF_BG_S     ( GFL_BG_FRAME3_S )

// ＢＧパレット
#define MBMAIN_MBG_PAL_YNWIN  ( 10 )    // はい・いいえウィンド（２本）
#define MBMAIN_MBG_PAL_SELBOX ( 12 )
#define MBMAIN_MBG_PAL_TALKWIN  ( 13 )
#define MBMAIN_MBG_PAL_TALKFNT  ( 14 )
#define MBMAIN_MBG_PAL_SYSFNT ( 15 )
#define MBMAIN_SBG_PAL_MAILMSE  ( 1 )   // メールメッセージ
#define MBMAIN_SBG_PAL_SYSFNT ( 15 )

// 会話ウィンドウ定義
#define MBMAIN_TALKWIN_CGX_SIZ  ( TALK_WIN_CGX_SIZ )
#define MBMAIN_TALKWIN_CGX_POS  ( 1024 - MBMAIN_TALKWIN_CGX_SIZ )
// はい・いいえウィンドウ定義
#define MBMAIN_YNWIN_CGX_SIZ  ( TOUCH_SW_USE_CHAR_NUM )
#define MBMAIN_YNWIN_CGX_POS  ( MBMAIN_TALKWIN_CGX_POS - MBMAIN_YNWIN_CGX_SIZ )
// SELBOXウィンドウ定義
#define MBMAIN_SELBOX_CGX_SIZ ( 13 * 13 )
#define MBMAIN_SELBOX_CGX_NUM ( MBMAIN_YNWIN_CGX_POS - MBMAIN_SELBOX_CGX_SIZ )
#define MBMAIN_SELBOX_FRM_NUM ( MBMAIN_SELBOX_CGX_NUM - SBOX_WINCGX_SIZ )


#define MBMAIN_MAILLIST_MAX   ( 10 )    // １ページに表示するメール数
#define MBMAIN_MAILLIST_NULL  ( 0xff )  // メールリストのダミーデータ

// BGWINFRM
enum {
  MBMAIN_BGWF_BUTTON1 = 0,
  MBMAIN_BGWF_SELMAIL = MBMAIN_BGWF_BUTTON1 + MBMAIN_MAILLIST_MAX,

  MBMAIN_BGWF_MAX
};

// BMPWIN
enum {
  MBMAIN_NULL_CHARA_M = 0,
  MBMAIN_NULL_CHARA_S,
  MBMAIN_BMPWIN_ID_MAIL01,  // メールリスト
  MBMAIN_BMPWIN_ID_MAIL02,
  MBMAIN_BMPWIN_ID_MAIL03,
  MBMAIN_BMPWIN_ID_MAIL04,
  MBMAIN_BMPWIN_ID_MAIL05,
  MBMAIN_BMPWIN_ID_MAIL06,
  MBMAIN_BMPWIN_ID_MAIL07,
  MBMAIN_BMPWIN_ID_MAIL08,
  MBMAIN_BMPWIN_ID_MAIL09,
  MBMAIN_BMPWIN_ID_MAIL10,
  MBMAIN_BMPWIN_ID_MAIL11,
  MBMAIN_BMPWIN_ID_MAIL12,
  MBMAIN_BMPWIN_ID_MAIL13,
  MBMAIN_BMPWIN_ID_MAIL14,
  MBMAIN_BMPWIN_ID_MAIL15,
  MBMAIN_BMPWIN_ID_MAIL16,
  MBMAIN_BMPWIN_ID_MAIL17,
  MBMAIN_BMPWIN_ID_MAIL18,
  MBMAIN_BMPWIN_ID_MAIL19,
  MBMAIN_BMPWIN_ID_MAIL20,

  MBMAIN_BMPWIN_ID_SELMAIL,   // 選択メール
  MBMAIN_BMPWIN_ID_CLOSE,     //「とじる」
  MBMAIN_BMPWIN_ID_PAGE,      // ページ

  MBMAIN_BMPWIN_ID_TALK,      // 会話メッセージ

  MBMAIN_BMPWIN_ID_TITLE,     //「メールボックス」
  MBMAIN_BMPWIN_ID_MAILMSG1,    // メールの内容１
  MBMAIN_BMPWIN_ID_MAILMSG2,    // メールの内容２
  MBMAIN_BMPWIN_ID_MAILMSG3,    // メールの内容３
  MBMAIN_BMPWIN_ID_NAME,      // 作成者名

  MBMAIN_BMPWIN_ID_MAX,
};

// OBJ
enum {
  MBMAIN_OBJ_ARROW_L = 0,
  MBMAIN_OBJ_ARROW_R,
  MBMAIN_OBJ_CURSOR,
  MBMAIN_OBJ_RET_BTN,

  MBMAIN_OBJ_POKEICON1,
  MBMAIN_OBJ_POKEICON2,
  MBMAIN_OBJ_POKEICON3,

  MBMAIN_OBJ_MAX
};

// ボタンアニメモード
enum {
  MBMAIN_BTN_ANM_MODE_OBJ = 0,
  MBMAIN_BTN_ANM_MODE_BG
};

// ボタンアニメワーク
typedef struct {
  u8  btn_mode:1;
  u8  btn_id:7;     // OBJ = ID, BG = FRM
  u8  btn_pal1:4;
  u8  btn_pal2:4;
  u8  btn_seq;
  u8  btn_cnt;

  u8  btn_px;     // BGのみ
  u8  btn_py;
  u8  btn_sx;
  u8  btn_sy;
}BUTTON_ANM_WORK;

// メールボックス画面アプリワーク
typedef struct {
  BGWINFRM_WORK * wfrm;   // BGWINFRM
  u8  btn_back_cgx[0x20*2]; // ボタン背景キャラ

  GFL_BMPWIN  *win[MBMAIN_BMPWIN_ID_MAX];
  PRINT_UTIL    printUtil[MBMAIN_BMPWIN_ID_MAX];
  PRINT_QUE     *printQue;
  PRINT_STREAM  *printStream;
  GFL_TCBLSYS   *pMsgTcblSys;

  GFL_MSGDATA *mman;    // メッセージデータマネージャ
  WORDSET     *wset;    // 単語セット
  STRBUF      *expbuf;  // メッセージ展開領域
  u8          msgID;    // 表示中メッセージＩＤ
  u8          msgTrg;   // メッセージトリガーウェイト

  // はい・いいえ関連
//  TOUCH_SW_SYS * tsw;   // タッチウィンドウ
  u32 ynID;             // はい・いいえＩＤ

  // メニューリスト
  SELBOX_SYS * selbox;
  SELBOX_WORK * sbwk;
  BMPLIST_DATA * sblist;
  u32 sb_old_pos;

  // OBJ関連
//  CATS_SYS_PTR  csp;
//  CATS_RES_PTR  crp;
//  CATS_ACT_PTR  cap[MBMAIN_OBJ_MAX];
  // セルアクター
  GFL_CLUNIT   *clUnit;
  GFL_CLWK     *clwk[MBMAIN_OBJ_MAX];
  u32          clres[4][MBMAIN_OBJ_MAX];  // セルアクターリソース用インデックス



  // ポケモンアイコン
  ARCHANDLE * pokeicon_ah;

  CURSORMOVE_WORK * cmwk; // カーソル移動ワーク

  MAIL_DATA * mail[MAIL_STOCK_MAX]; // メールデータ
  u8  mail_list[MAIL_STOCK_MAX];    // メールデータのインデックス

  u16 mail_max;       // 有効メールデータ数
  u16 page_max;       // ページ最大数

  BUTTON_ANM_WORK bawk;   // ボタンアニメワーク
  int wipe_seq;       // ワイプ後のシーケンス

  // メニュー共通リソース
  APP_TASKMENU_RES  *menures;
  // タッチメニュー
  APP_TASKMENU_WORK     *menuwork;
  APP_TASKMENU_ITEMWORK menuitem[4];
  APP_TASKMENU_ITEMWORK yn_menuitem[2];

  // 簡易会話描画用ワーク
  PMS_DRAW_WORK     *pms_draw_work;
  GFL_CLUNIT        *pmsClunit;
  PRINT_QUE         *pmsPrintque;

}MAILBOX_APP_WORK;

// メールボックス画面システムワーク
typedef struct {
  MAIL_BLOCK * sv_mail;   // メールデータ
  MAILBOX_PARAM * dat;

  MAILBOX_APP_WORK * app;

  GFL_PROC *subProcFunc;   // サブプロセス
  void     *subProcWork;   // サブプロセスで使用するワーク
  u32       subProcType;   // サブプロセスの種類
  GFL_FONT *font;

  u16 sel_page;     // 現在のページ
  u8  sel_pos;      // 選択位置
  u8  lst_pos;      // 選択したメールデータの位置

  u8  ret_mode;     // ポケモンリストからの戻り値
  u8  ret_poke:7;     // ポケモンリスト終了時の位置
  u8  check_item:1;   // バッグに戻せたか 0 = 成功, 1 = 失敗
  u16 ret_item;     // 戻すアイテム
  BOOL  mw_ret;     // メール作成結果

  int next_seq;     // 次のシーケンス

  GFL_TCB      *vintr_tcb; // 登録したVblankFuncのポインタ
  GFL_PROCSYS  *mbProcSys; // メールボックス内で実行するPROCのサブシステム

}MAILBOX_SYS_WORK;

typedef int (*pMailBoxFunc)(MAILBOX_SYS_WORK*);

// メニュー番号
enum {
  MBMAIN_MENU_READ = 0,   // メールを読む
  MBMAIN_MENU_DELETE,     // メールを消す
  MBMAIN_MENU_POKESET,    // 持たせる
  MBMAIN_MENU_CANCEL,     // やめる
  MBMAIN_MENU_MAX
};



//============================================================================================
//  プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * VBLANK関数
 *
 * @param work  メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_VBlank( GFL_TCB *tcb, void *work );

//--------------------------------------------------------------------------------------------
/**
 * VRAM設定
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_VramInit(void);

//--------------------------------------------------------------------------------------------
/**
 * ＢＧ初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_BgInit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * ＢＧ解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_BgExit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * ＢＧグラフィック読み込み
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_BgGraphicLoad( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * ウィンドウ読み込み
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_WindowLoad( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * BGWINFRM初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_BgWinFrmInit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * BGWINFRM解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_BgWinFrmExit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * メッセージ関連初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_MsgInit( MAILBOX_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * メッセージ関連解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_MsgExit( MAILBOX_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえ処理初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_YesNoWinInit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえ処理解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_YesNoWinExit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえセット
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_YesNoWinSet( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 選択ボックス作成
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_SelBoxInit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 選択ボックス削除
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_SelBoxExit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 選択ボックス表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_SelBoxPut( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 選択ボックス非表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_SelBoxDel( MAILBOX_SYS_WORK * syswk );


//--------------------------------------------------------------------------------------------
/**
 * メールデータ取得
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_MailDataInit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * メールデータ解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_MailDataExit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * メール選択ボタン初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_MailButtonInit( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 選択したメールのタブ表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_SelMailTabPut( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * ボタンアニメ処理
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @retval  "TRUE = アニメ中"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL MBMAIN_ButtonAnmMain( MAILBOX_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * メールグラフィック読み込み
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void MBMAIN_MailGraphcLoad( MAILBOX_SYS_WORK * syswk );

extern void MBMAIN_PmsDrawInit( MAILBOX_SYS_WORK * syswk );
extern void MBMAIN_PmsDrawExit( MAILBOX_SYS_WORK * syswk );


#endif  // __MB_MAIN_H__
