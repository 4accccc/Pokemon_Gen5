//============================================================================================
/**
	* @file	pms_input_prv.h
	* @bfief	簡易会話入力画面（private ヘッダ）
	* @author	taya
	* @date	06.01.24
	*/
//============================================================================================
#ifndef __PMS_INPUT_PRV_H__
#define __PMS_INPUT_PRV_H__

#include "app\pms_input.h"
#include "app\pms_input_data.h"
#include "pms_input_param.h"

//------------------------------------------------------
/**
	* 描画コマンド
	*/
//------------------------------------------------------
enum {
	VCMD_INIT,					///< 画面初期構築
	VCMD_QUIT,					///< 描画終了
	VCMD_FADEIN,				///< フェードイン

	VCMD_UPDATE_EDITAREA,		///< 編集領域更新
	VCMD_KTCHANGE_EDITAREA,		///< 編集領域キー/タッチ切替
	VCMD_KTCHANGE_CATEGORY,		///< カテゴリ領域キー/タッチ切替
	VCMD_KTCHANGE_WORDWIN,		///< ワードウィンドウ領域キー/タッチ切替
	VCMD_EDITAREA_TO_BUTTON,	///< 編集領域からコマンドボタンへ(キー入力)
	VCMD_EDITAREA_TO_BUTTON_TOUCH,	///< 編集領域からコマンドボタンへ(タッチ入力)
	VCMD_BUTTON_TO_EDITAREA,	///< コマンドボタンから編集領域へ
	VCMD_EDITAREA_TO_CATEGORY,	///< 編集領域からカテゴリ選択へ
	VCMD_CATEGORY_TO_EDITAREA,	///< カテゴリ選択から編集領域へ
	VCMD_CATEGORY_TO_WORDWIN,	///< カテゴリ選択から単語選択へ
	VCMD_WORDWIN_TO_CATEGORY,	///< 単語選択からカテゴリ選択へ（単語選択キャンセル後）
	VCMD_WORDTIN_TO_EDITAREA,	///< 単語選択から編集領域へ（単語決定後）
	VCMD_WORDTIN_TO_BUTTON,		///< 単語選択からコマンドボタンへ（単語決定後）

	VCMD_MOVE_EDITAREA_CURSOR,	///< 編集領域カーソル移動
	VCMD_MOVE_BUTTON_CURSOR,	///< ボタンカーソル移動
	VCMD_MOVE_CATEGORY_CURSOR,	///< カテゴリ選択カーソル移動
	VCMD_MOVE_WORDWIN_CURSOR,	///< 単語選択カーソル移動
	VCMD_SCROLL_WORDWIN,		///< 単語ウィンドウスクロール

	VCMD_DISP_MESSAGE_DEFAULT,	///< 初期メッセージ表示
//	VCMD_DISP_MESSAGE_OK,		///< これでいいですか？メッセージ＆メニュー
//	VCMD_DISP_MESSAGE_CANCEL,	///< やめますか？メッセージ＆メニュー
	VCMD_DISP_MESSAGE_WARN,		///< なにかことばをいれてください！メッセージ
  VCMD_TASKMENU_DECIDE,     ///< タスクメニュー決定演出
	VCMD_MOVE_MENU_CURSOR,		///< メニューカーソル移動
	VCMD_ERASE_MENU,			///< メニュー表示消去

	VCMD_CHANGE_CATEGORY_MODE_DISABLE,	///< カテゴリモード切替え、カテゴリ選択状態へ移行しない
	VCMD_CHANGE_CATEGORY_MODE_ENABLE,	///< カテゴリモード切替え、カテゴリ選択状態へ移行する

	VCMD_BUTTON_UP_HOLD,
	VCMD_BUTTON_DOWN_HOLD,
	VCMD_BUTTON_UP_RELEASE,
	VCMD_BUTTON_DOWN_RELEASE,

  VCMD_INPUTWORD_UPDATE,  ///< カテゴリで入力された文字表示を更新
	VCMD_SCROLL_WORDWIN_BAR,		///< 単語ウィンドウスクロールバー

  VCMD_ERASE_BLINK_IN_CATEGORY_INITIAL,  // カテゴリあいうえお入力において、1文字消去したとき、1文字消去ボタンを明滅させる
  VCMD_INPUT_BLINK_IN_CATEGORY,          // カテゴリあいうえお入力において、1文字入力したとき、その文字のところを明滅させる  // カテゴリわざピクチャーなどから選択において、選択したとき、その選択したところを明滅させる
  VCMD_INPUT_BLINK_IN_WORDWIN,          // 単語選択において、選択したとき、その選択したところを明滅させる

  VCMD_MAX
};


//------------------------------------------------------
/**
	* ボタンアクターレイアウト
	*/
//------------------------------------------------------
enum {

	BUTTON_CHANGE_SCRN_XPOS = 0,
	BUTTON_CHANGE_SCRN_YPOS = 21,

	MODEBUTTON_SCRN_WIDTH = 6,
	MODEBUTTON_SCRN_HEIGHT = 3,

	BUTTON_CHANGE_TOUCH_TOP		  = BUTTON_CHANGE_SCRN_YPOS*8,
	BUTTON_CHANGE_TOUCH_BOTTOM  = BUTTON_CHANGE_TOUCH_TOP + (MODEBUTTON_SCRN_HEIGHT*8),
	BUTTON_CHANGE_TOUCH_LEFT	  = BUTTON_CHANGE_SCRN_XPOS*8,
	BUTTON_CHANGE_TOUCH_RIGHT	  = BUTTON_CHANGE_TOUCH_LEFT + (MODEBUTTON_SCRN_WIDTH*8),
};


///カテゴリモード　BGオフセット
enum{
	// BG offset
	GROUPMODE_BG_XOFS = -4,
	INITIALMODE_BG_XOFS = 0,

//	CATEGORY_BG_DISABLE_YOFS = 2,
//	CATEGORY_BG_ENABLE_YOFS = 26,
	CATEGORY_BG_DISABLE_YOFS = 16,
	CATEGORY_BG_ENABLE_YOFS = 16,
};

//-------------------------------------------------------------
/**
	*  各文字のインデックス
	*/
//-------------------------------------------------------------
enum {
	INI_A = 0,
	INI_I,
	INI_U,
	INI_E,
	INI_O,
	INI_KA,
	INI_KI,
	INI_KU,
	INI_KE,
	INI_KO,
	INI_SA,
	INI_SI,
	INI_SU,
	INI_SE,
	INI_SO,
	INI_TA,
	INI_TI,
	INI_TU,
	INI_TE,
	INI_TO,
	INI_NA,
	INI_NI,
	INI_NU,
	INI_NE,
	INI_NO,
	INI_HA,
	INI_HI,
	INI_HU,
	INI_HE,
	INI_HO,
	INI_MA,
	INI_MI,
	INI_MU,
	INI_ME,
	INI_MO,
#if 1
	INI_YA,
	INI_YU,
	INI_YO,
	INI_RA,
	INI_RI,
	INI_RU,
	INI_RE,
	INI_RO,
#else
	INI_RA,
	INI_RI,
	INI_RU,
	INI_RE,
	INI_RO,
	INI_YA,
	INI_YU,
	INI_YO,
#endif
	INI_WA,
	INI_OTHER,
  // ↑50音検索のgmmテーブルと対応しているので順番固定↑
  INI_WO,
  INI_NN,
	INI_BOU,
  INI_SEARCH_MAX = INI_OTHER+1,

	INI_BACK    = CATEGORY_POS_BACK,
	INI_ERASE   = CATEGORY_POS_ERASE,
	INI_SELECT  = CATEGORY_POS_SELECT,
	INI_DIS     = CATEGORY_POS_DISABLE,
};



//------------------------------------------------------
/**
	* メインワーク型宣言
	*/
//------------------------------------------------------
typedef struct _PMS_INPUT_WORK	PMS_INPUT_WORK;

//------------------------------------------------------
/**
	* 描画ワーク型宣言
	*/
//------------------------------------------------------
typedef struct _PMS_INPUT_VIEW	PMS_INPUT_VIEW;

//------------------------------------------------------
/**
	* タスクプライオリティ
	*/
//------------------------------------------------------
enum {
	TASKPRI_MAIN,
	TASKPRI_VIEW_COMMAND,
	TASKPRI_VIEW_MAIN,
};

enum {
	VTASKPRI_COMMAND,
	VTASKPRI_MAIN,
};

enum {
	PMSIV_MSG_GUIDANCE,
	PMSIV_MSG_WARN_INPUT,
	PMSIV_MSG_CONFIRM_DECIDE,
	PMSIV_MSG_CONFIRM_CANCEL,
};

enum {
	BUTTON_POS_DECIDE,
	BUTTON_POS_CANCEL,
};

enum {
	CATEGORY_MODE_GROUP,
	CATEGORY_MODE_INITIAL,
};



// リストの最上段は空欄とするため、総数を＋２　2009/11/14 by nakahiro
#define	PMSI_DUMMY_LABEL_NUM	( 2 )


//------------------------------------------------------
/**
	* システム関連関数プロトタイプ
	*/
//------------------------------------------------------
extern int* PMSI_GetKTModePointer(const PMS_INPUT_WORK* wk);
extern u32 PMSI_GetInputMode( const PMS_INPUT_WORK* wk );
extern u32 PMSI_GetSentenceType( const PMS_INPUT_WORK* wk );
extern u32 PMSI_GetCategoryMode( const PMS_INPUT_WORK* wk );
extern PMS_WORD  PMSI_GetEditWord( const PMS_INPUT_WORK* wk, int pos );
extern STRBUF* PMSI_GetEditSourceString( const PMS_INPUT_WORK* wk, u32 heapID );
extern u32 PMSI_GetGuidanceType( const PMS_INPUT_WORK* wk );
extern u32 PMSI_GetEditAreaCursorPos( const PMS_INPUT_WORK* wk );
extern u32 PMSI_GetButtonCursorPos( const PMS_INPUT_WORK* wk );
extern BOOL PMSI_GetEditAreaOrButton( const PMS_INPUT_WORK* wk );
extern u32 PMSI_GetCategoryCursorPos( const PMS_INPUT_WORK* wk );
extern u32 PMSI_GetCategoryWordMax( const PMS_INPUT_WORK* wk );
extern void PMSI_GetCategoryWord( const PMS_INPUT_WORK* wk, u32 word_num, STRBUF* buf );
extern u32 PMSI_GetWordWinCursorPos( const PMS_INPUT_WORK* wk );
extern int PMSI_GetWordWinScrollVector( const PMS_INPUT_WORK* wk );
extern int PMSI_GetWordWinLinePos( const PMS_INPUT_WORK* wk );
extern BOOL PMSI_GetWordWinUpArrowVisibleFlag( const PMS_INPUT_WORK* wk );
extern BOOL PMSI_GetWordWinDownArrowVisibleFlag( const PMS_INPUT_WORK* wk );
extern int PMSI_GetTalkWindowType( const PMS_INPUT_WORK* wk );
extern u32 PMSI_GetMenuCursorPos( const PMS_INPUT_WORK* wk );
extern BOOL PMSI_GetLockFlag( const PMS_INPUT_WORK* wk );

extern GFL_TCBSYS* PMSI_GetTcbSystem( const PMS_INPUT_WORK* wk );
extern void PMSI_GetInputWord( const PMS_INPUT_WORK* wk, STRBUF* out_buf );
extern void PMSI_ClearInputWord( PMS_INPUT_WORK* wk );

extern BOOL PMSI_CheckInputComplete( const PMS_INPUT_WORK* wk );


//------------------------------------------------------
/**
	* 描画関連関数プロトタイプ
	*/
//------------------------------------------------------
extern PMS_INPUT_VIEW*  PMSIView_Create(const PMS_INPUT_WORK* wk, const PMS_INPUT_DATA* dwk);
extern void PMSIView_Delete( PMS_INPUT_VIEW* vwk );
extern void PMSIView_SetCommand( PMS_INPUT_VIEW* vwk, int cmd );
extern BOOL PMSIView_WaitCommand( PMS_INPUT_VIEW* vwk, int cmd );
extern BOOL PMSIView_WaitCommandAll( PMS_INPUT_VIEW* vwk );
extern u32 PMSIView_GetSentenceEditPosMax( PMS_INPUT_VIEW* wk );
extern void PMSIView_GetSentenceWordArea( PMS_INPUT_VIEW* wk ,GFL_UI_TP_HITTBL* tbl,u8 idx);

extern int PMSIView_WaitYesNo(PMS_INPUT_VIEW* wk);
extern void PMSIView_TouchEditButton( PMS_INPUT_VIEW* wk, int btn_id );

extern void PMSI_GetWorkScrollData( const PMS_INPUT_WORK * wk, u16 * line, u16 * line_max );

extern u32 PMSI_GetSearchResultCount( const PMS_INPUT_WORK* wk );
extern u8 PMSI_GetSearchCharNum( const PMS_INPUT_WORK* wk );
extern void PMSI_GetSearchResultString( const PMS_INPUT_WORK* wk, u32 result_idx, STRBUF* dst_buf );


#endif
