//=============================================================================
/**
 *
 *	@file		t_download_local.h
 *	@brief  トーナメントダウンロード画面(local)
 *	@author
 *	@date
 *
 */
//=============================================================================
#pragma once

#include "system/time_icon.h"

#include "savedata/wbt_download.h"
#include "app/t_download.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
//---------
// モード
// (途中で切り替わることもある)
//---------
enum{
	T_DOWNLOAD_MODE_SAVE = 0,						// セーブ画面
	T_DOWNLOAD_MODE_SELECT,							// 選択画面
	T_DOWNLOAD_MODE_JOIN,							// 参加画面
	T_DOWNLOAD_MODE_INFO,							// 詳細画面
	//
	T_DOWNLOAD_MODE_MAX,
};

//--------
// 最大値
//--------
enum{
	T_DOWNLOAD_CANVAS_MAXNUM 	= 17,				// canvas
	T_DOWNLOAD_BMPOAM_MAXNUM	= 4,				// bmpoam
	T_DOWNLOAD_EMPTY_MAXNUM		= 30*2,				// empty
	//
	T_DOWNLOAD_CMN_OBJ_MAXNUM	= 6,				// common(obj)
	T_DOWNLOAD_OBJ_MAXNUM		= 9,				// obj
	//
	T_DOWNLOAD_BGNUM_MAXNUM		= 8,				// bg
	//
	T_DOWNLOAD_MSG_MAX			= 2,				// message
	//
	T_DOWNLOAD_DISP_PANEL_MAX	= 6,				// パネル最大
};

//-----------------------------
// OBJのリソースのインデックス
//-----------------------------
enum{
	// common
	T_DOWNLOAD_RESID_CMN_OBJ_PLT = 0,				// パレット
	T_DOWNLOAD_RESID_CMN_OBJ_CHR,					// キャラ
	T_DOWNLOAD_RESID_CMN_OBJ_CEL,					// セルアニメ
	// 通常
	T_DOWNLOAD_RESID_OBJ_PLT,						// パレット
	T_DOWNLOAD_RESID_OBJ_CHR,						// キャラ
	T_DOWNLOAD_RESID_OBJ_CEL,						// セルアニメ
	//
	T_DOWNLOAD_RESID_OBJ_MAX,
};

//-------------------------
// タッチペンエリア
//-------------------------
enum{
	T_DOWNLOAD_AREA_TD_TP_CROSS = 0,				// ×
	T_DOWNLOAD_AREA_TD_TP_RETURN,					// RETURN
	T_DOWNLOAD_AREA_TD_TP_DOWN,						// 下矢印
	T_DOWNLOAD_AREA_TD_TP_UP,						// 上矢印
	T_DOWNLOAD_AREA_TD_TP_ICON_00,					// アイコン
	T_DOWNLOAD_AREA_TD_TP_ICON_01,					// アイコン
	T_DOWNLOAD_AREA_TD_TP_ICON_02,					// アイコン
	T_DOWNLOAD_AREA_TD_TP_SCROLL,					// スクロール領域
};

// 参加用
enum{
	T_DOWNLOAD_AREA_JOIN_TP_RETURN = 0,				// RETURN
	T_DOWNLOAD_AREA_JOIN_TP_LEFT,
	T_DOWNLOAD_AREA_JOIN_TP_RIGHT,
	T_DOWNLOAD_AREA_JOIN_TP_PANEL_00,
	T_DOWNLOAD_AREA_JOIN_TP_PANEL_01,
	T_DOWNLOAD_AREA_JOIN_TP_PANEL_02,
};
// セーブ用
enum{
	T_DOWNLOAD_AREA_SAVE_TP_RETURN = 0,				// RETURN
	T_DOWNLOAD_AREA_SAVE_TP_LEFT,
	T_DOWNLOAD_AREA_SAVE_TP_RIGHT,
	T_DOWNLOAD_AREA_SAVE_TP_PANEL_00,
	T_DOWNLOAD_AREA_SAVE_TP_PANEL_01,
	T_DOWNLOAD_AREA_SAVE_TP_PANEL_02,
};

//----------
// clwork
//----------
enum{
	T_DOWNLOAD_CLWKMODE_NORMAL = 0,					// 通常(down)
	T_DOWNLOAD_CLWKMODE_CMN,						// 共通(down)
};

//--------------
// CLWK(下画面)
//--------------
enum
{
	T_DOWNLOAD_CLWKID_BAR = 0,						// スクロールバー
	T_DOWNLOAD_CLWKID_BAR_BASE,						// スクローバー下地
	T_DOWNLOAD_CLWKID_CURSOR,						// カーソル
	//
	T_DOWNLOAD_CLWKID_ICON_00,						// アイコン
	T_DOWNLOAD_CLWKID_ICON_01,
	T_DOWNLOAD_CLWKID_ICON_02,
	//
	T_DOWNLOAD_CLWKID_PANEL_00,						// パネル00
	T_DOWNLOAD_CLWKID_PANEL_01,						// パネル01
	T_DOWNLOAD_CLWKID_PANEL_02,						// パネル02
	//
	T_DOWNLOAD_CLWKID_MAX,
};

//------------------
// CLWK(下画面共通)
//------------------
enum{
	T_DOWNLOAD_CLWKID_CMN_LEFT = 0,					// 左
	T_DOWNLOAD_CLWKID_CMN_RIGHT,					// 右
	T_DOWNLOAD_CLWKID_CMN_CROSS,					// ×
	T_DOWNLOAD_CLWKID_CMN_RETURN,					// 戻る
	T_DOWNLOAD_CLWKID_CMN_DOWN,						// 下
	T_DOWNLOAD_CLWKID_CMN_UP,						// 上
	//
	T_DOWNLOAD_CLWKID_CMN_MAX,
};

//---------------
// CmnFlag(共通)
//---------------
enum{
	T_DOWNLOAD_FLG_ANM_LEFT		= (1<<0),			// 左矢印アニメ
	T_DOWNLOAD_FLG_ANM_RIGHT	= (1<<1),			// 右矢印アニメ
	T_DOWNLOAD_FLG_ANM_CROSS	= (1<<2),			// ×アニメ
	T_DOWNLOAD_FLG_ANM_RETURN	= (1<<3),			// 戻るアニメ
	//
	T_DOWNLOAD_FLG_ANM_DOWN		= (1<<4),			// 下矢印アニメ
	T_DOWNLOAD_FLG_ANM_UP		= (1<<5),			// 上矢印アニメ
	T_DOWNLOAD_FLG_RET_END		= (1<<6)	,		// RETURNで終了
	T_DOWNLOAD_FLG_CROSS_END	= (1<<7),			// ×で終了
	//
	T_DOWNLOAD_FLG_UPDATE		= (1<<15),			// 更新
	T_DOWNLOAD_FLG_PALCHG		= (1<<16),			// パレットチェンジ
	T_DOWNLOAD_FLG_CUR_ANIM		= (1<<17),			// カーソルアニメ中
};

//--------------
// Canvas
//--------------
enum{
	T_DOWNLOAD_MSG_UP_TNAME		= 0,				// トーナメント名
	T_DOWNLOAD_MSG_UP_RULE_00,						// ルール(左)
	T_DOWNLOAD_MSG_UP_RULE_01,						// ルール(右)
	T_DOWNLOAD_MSG_UP_EXP_00,						// ルール説明
	T_DOWNLOAD_MSG_UP_INFO,							// 内容
	T_DOWNLOAD_MSG_UP_EXP_01,						// 内容説明
	//
	T_DOWNLOAD_MSG_DOWN_TITLE,						// タイトル
	T_DOWNLOAD_MSG_DOWN_MESWIN_00,					// win00(中央)
	T_DOWNLOAD_MSG_DOWN_MESWIN_01,					// win01(下側)
	//
	T_DOWNLOAD_MSG_DOWN_INFO_00,					// 詳細パネル00
	T_DOWNLOAD_MSG_DOWN_INFO_01,
	T_DOWNLOAD_MSG_DOWN_INFO_02,
	T_DOWNLOAD_MSG_DOWN_INFO_03,
	T_DOWNLOAD_MSG_DOWN_INFO_04,
};

//--------------
// 文字列情報
//--------------
// win情報
enum{
	T_DOWNLOAD_WIN_JOIN_00 = 0,						// 参加する
	T_DOWNLOAD_WIN_JOIN_01,							// 参加しますか
	T_DOWNLOAD_WIN_WHAT_00,							// どうしますか
	T_DOWNLOAD_WIN_DEL_00,							// 削除しますか
	T_DOWNLOAD_WIN_DEL_01,							// 本当に削除しますか
	T_DOWNLOAD_WIN_SEL_DATA,						// 保存したいのを選んでください
	T_DOWNLOAD_WIN_SAVED,							// すでにセーブすみ
	T_DOWNLOAD_WIN_FINISH,							// 終了しますか
	T_DOWNLOAD_WIN_DEL_STOP,						// 消すのをやめますか
	T_DOWNLOAD_WIN_DATA_FULL,						// データがいっぱいです
	T_DOWNLOAD_WIN_DEL_DONE,						// 削除しました
	T_DOWNLOAD_WIN_NOW_SAVING,						// 保存しています
	T_DOWNLOAD_WIN_SAVE_DONE,						// セーブしました
};

// 3択情報
enum{
	T_DOWNLOAD_YESNO_JOIN = 0,						// 参加する
	T_DOWNLOAD_YESNO_SAVE,							// セーブする
	T_DOWNLOAD_YESNO_YESNO,							// yes/no
	T_DOWNLOAD_YESNO_SAVE_AFTER,					// セーブ後
};

//--------------
// リスト情報
//--------------
enum{
	T_DOWNLOAD_LIST_CURRENT = 0,					// カレント
	T_DOWNLOAD_LIST_SAVE,							// セーブ
	T_DOWNLOAD_LIST_SELECT,							// 選択
	//
	T_DOWNLOAD_LIST_MAX,
};

//--------------
// アイコン情報
//--------------
enum{
	T_DOWNLOAD_INFO_POKE = 0,						// ポケモン
	T_DOWNLOAD_INFO_ITEM,							// 道具
	T_DOWNLOAD_INFO_SKILL,							// 技
	//
	T_DOWNLOAD_INFO_MAX,
};

//--------------
// 判定
//--------------
enum{
	T_DOWNLOAD_PANEL_FRAME = 0,						// パネル(リストフレーム)
	T_DOWNLOAD_PANEL_3,								// パネル(3枚表示)
};

//--------------
// 判定
//--------------
enum{
	T_DOWNLOAD_DATA_SAVE = 0,						// セーブデータ参照
	T_DOWNLOAD_DATA_LIST,							// リストデータ参照
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//-------------------------------------
/// OBJ管理ワーク
//-------------------------------------
typedef struct
{
	u32			res[T_DOWNLOAD_RESID_OBJ_MAX]; 			// リソース
	// 共通
	GFL_CLWK	*cmn_clwk[T_DOWNLOAD_CMN_OBJ_MAXNUM];	// CLWK(common)
	// 通常
	GFL_CLWK	*clwk[T_DOWNLOAD_OBJ_MAXNUM];			// CLWK
} T_DOWNLOAD_OBJ_WORK;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
	HEAPID					heapID;
	// OBJ
	T_DOWNLOAD_OBJ_WORK		wk_obj;
	//描画設定
	T_DOWNLOAD_GRAPHIC_WORK	*graphic;
	//
	T_DOWNLOAD_PARAM 		*Param;							// 参照用として保存	
	//フォント
	GFL_FONT				*font;
	//プリントキュー
	PRINT_QUE				*print_que;
	GFL_MSGDATA				*msg[T_DOWNLOAD_MSG_MAX];		// 0:メッセージ,1:未使用
	PRINT_UTIL				PrintUtil[T_DOWNLOAD_CANVAS_MAXNUM];	// 文字表示用
	PRINT_STREAM			*PrintStream;					// 文字表示用(PrintStream)
	GFL_TCBLSYS				*MsgTcblSys;					// メッセージ用タスク
	STRBUF					*MsgBuffer;						// メッセージ用バッファ
	APP_KEYCURSOR_WORK		*MsgCursorWork;					// ページ送りマーク用ワーク
	APP_PRINTSYS_COMMON_WORK PrintSysWork;					// 汎用PrintStream管理ワーク
	WORDSET					*Wordset;						// wordset
	//
	APP_TASKMENU_WORK		*pAppTask;
	APP_TASKMENU_ITEMWORK	appitem[3];
	APP_TASKMENU_RES		*pAppTaskRes;
	//
	TIMEICON_WORK			*TimeIconWork;					// セーブ中のタイムアイコン
	//
	s32						Step;							// ステップ
	s32						Mode;							// モード
	u32						Flag;							// フラグ
	u32						CmnFlag;						// 共通フラグ
	u32						DebugFlag;						// debug
	s32						RetMode;						// 戻りモード
	s32						InfoCurPos;						// 詳細カーソル位置(0-2)
	s32						ScrollOffy;						// BG3のスクロールoffset
	u32						PFlag;							// Canvas用フラグ
	u32						SaveFlag;						// セーブフラグ
	s32						JoinCurPos;						// 参加カーソル
	s32						SaveCurPos;						// セーブカーソル
	s32						NextMode;						// 飛び先
	s32						SaveSrc;						// セーブ元
	s32						SaveDest;						// セーブ先
	//
	s32						SaveMaxNum;						// セーブ数
	s32						ListMaxNum;						// リスト最大数
	//
	// パレットチェンジ用
	s32						PCount;							///< カウンタ
	u16						PBufStart[0x10];				///< 開始
	u16						PBufEnd[0x10];					///< 終了
	u16						PBufNow[0x10];					///< 現在
	u16						FontPal[0x20];					///< パレット用
	//
	// list
	FRAMELIST_WORK			*frame_list_work;					// リスト
	s32						DispMaxNum;							// 表示最大数
	s32						AllCurPos[T_DOWNLOAD_LIST_MAX];		// 通し番号
	s32						ListCurPos[T_DOWNLOAD_LIST_MAX];	// 画面上の位置
	s32						ListScrollPos[T_DOWNLOAD_LIST_MAX];	// スクロールの位置
	// 詳細画面用(3アイコン用)
	s32						ItemNum[T_DOWNLOAD_INFO_MAX];		// 項目数
	s32						PageMaxNum[T_DOWNLOAD_INFO_MAX];	// ページ最大数
	s32						CurrentPageNum[T_DOWNLOAD_INFO_MAX]; // カレントページ数
	//
	s16						*EmptyBuffer;					// 座標用のバッファ(必ず破棄すること)
    u16						*TestBuffer;					// テスト
	//
	WBTDL_DATA				*CurrentWbt;					// カレント(上画面の切り替えタイミング)
	WBTDL_DATA				*WbtSaveData[3];				// セーブデータ用
	//
	WBTDL_DATA				*CheckBuffer[WBTDL_TOTAL_NUM];	// チェック後のデータ
} T_DOWNLOAD_MAIN_WORK;

