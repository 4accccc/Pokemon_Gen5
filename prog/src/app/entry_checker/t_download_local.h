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

#include "app/t_download.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
// 最大値
enum{
	T_DOWNLOAD_CANVAS_MAXNUM 	= 9,				// canvas
	T_DOWNLOAD_BMPOAM_MAXNUM	= 4,				// bmpoam
	T_DOWNLOAD_EMPTY_MAXNUM		= 17*2,				// empty
	//
	T_DOWNLOAD_CMN_OBJ_MAXNUM	= 4,				// common(obj)
	T_DOWNLOAD_OBJ_MAXNUM		= 6,				// obj
	//
	T_DOWNLOAD_BGNUM_MAXNUM		= 8,				// bg
	//
	T_DOWNLOAD_MSG_MAX			= 2,				// message
	//
	T_DOWNLOAD_SAVE_BUFFER_MAX	= 3,				// セーブ個数最大
	T_DOWNLOAD_ITEM_BUFFER_MAX	= 30,				// バッファ最大数
};

// OBJのリソースのインデックス
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

// タッチペンエリア
// (excelのテーブルと対応)
enum{
	T_DOWNLOAD_AREA_TD_TP_LEFT = 0,					// 左矢印
	T_DOWNLOAD_AREA_TD_TP_RIGHT,					// 右矢印
	T_DOWNLOAD_AREA_TD_TP_CROSS,					// ×
	T_DOWNLOAD_AREA_TD_TP_RETURN,					// RETURN
	T_DOWNLOAD_AREA_TD_TP_SCROLL,					// スクロール領域
	//
	T_DOWNLOAD_AREA_TD_TP_CMN_MAX,					// ここまではデータにセット
	//
	T_DOWNLOAD_AREA_TD_TP_PANEL3_0 = T_DOWNLOAD_AREA_TD_TP_CMN_MAX, // パネル3行
	T_DOWNLOAD_AREA_TD_TP_PANEL3_1,
	T_DOWNLOAD_AREA_TD_TP_PANEL3_2,
	//
	T_DOWNLOAD_AREA_TD_TP_PANEL4_0,					// パネル4行
	T_DOWNLOAD_AREA_TD_TP_PANEL4_1,
	T_DOWNLOAD_AREA_TD_TP_PANEL4_2,
	T_DOWNLOAD_AREA_TD_TP_PANEL4_3,
	//
	T_DOWNLOAD_AREA_TD_TP_MAX,
};

// clwork
enum{
	T_DOWNLOAD_CLWKMODE_NORMAL = 0,					// 通常(down)
	T_DOWNLOAD_CLWKMODE_CMN,						// 共通(down)
};

// CLWK(下画面)
enum
{
	T_DOWNLOAD_CLWKID_PANEL_00 = 0,					// パネル(00)
	T_DOWNLOAD_CLWKID_PANEL_01,						// パネル
	T_DOWNLOAD_CLWKID_PANEL_02,						// パネル
	T_DOWNLOAD_CLWKID_PANEL_03,						// パネル
	T_DOWNLOAD_CLWKID_BAR,							// スクロールバー
	T_DOWNLOAD_CLWKID_CURSOR,						// カーソル(仮)
	//
	T_DOWNLOAD_CLWKID_MAX,
};

// CLWK(下画面共通)
enum{
	T_DOWNLOAD_CLWKID_CMN_LEFT = 0,					// 左
	T_DOWNLOAD_CLWKID_CMN_RIGHT,					// 右
	T_DOWNLOAD_CLWKID_CMN_CROSS,					// ×
	T_DOWNLOAD_CLWKID_CMN_RETURN,					// 戻る
	//
	T_DOWNLOAD_CLWKID_CMN_MAX,
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

//-------------------------------------
/// アイテム情報
//-------------------------------------
typedef struct{
	u8				Num;			// 番号(0xffなし)
	s8				Mode;			// 状態
} T_DOWNLOAD_ITEM;

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
//	APP_KEYCURSOR_WORK		*MsgCursorWork;					// ページ送りマーク用ワーク
	APP_PRINTSYS_COMMON_WORK PrintSysWork;					// 汎用PrintStream管理ワーク
	//
	APP_TASKMENU_WORK		*pAppTask;
	APP_TASKMENU_ITEMWORK	appitem[2];
	APP_TASKMENU_RES		*pAppTaskRes;
	//
	BMPOAM_SYS_PTR			BmpOamSys;								// BMPOAM
	GFL_BMP_DATA			*BmpData[T_DOWNLOAD_BMPOAM_MAXNUM];		// Bmp
	BMPOAM_ACT_PTR			BmpOamWork[T_DOWNLOAD_BMPOAM_MAXNUM];	// BMPOAM用Work
	//
	s32						Step;							// ステップ
	s32						Mode;							// モード
	u32						Flag;							// フラグ
	s32						CurPos;							// カーソル位置(view)
	s32						TopNum;							// 先頭番号(select)
	s32						SubNum;							// カーソル位置(select)
	s32						SaveBuffer[T_DOWNLOAD_SAVE_BUFFER_MAX];	// セーブ領域
	T_DOWNLOAD_ITEM			ItemBuffer[T_DOWNLOAD_ITEM_BUFFER_MAX];	// アイテム領域
	//
	s16						*EmptyBuffer;					// 座標用のバッファ(必ず破棄すること)
	s16						*ParamBuffer;					// パラメータのバッファ(必ず破棄すること)
	//
	GFL_UI_TP_HITTBL		HitBuffer[T_DOWNLOAD_AREA_TD_TP_MAX+1];	// タッチペン用(+1は終了コードを入れるため)
} T_DOWNLOAD_MAIN_WORK;

