//=============================================================================
/**
 *
 *	@file		fmission_app_local.h
 *	@brief  フェスミッションアプリ ローカルヘッダ
 *	@author Miyuki Iwasawa	
 *	@data	  2011.04.22	
 */
//=============================================================================

#pragma once

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"
#include "print/wordset.h"

//描画設定
#include "fmission_app_gra.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "app/fmission_app.h"

#include "fmission_app_def.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
// いろいろ
enum{
	FMA_MSG_MAXNUM    	= 2,		// msg最大数
	FMA_CANVAS_MAXNUM 	= 30,		// canvas最大数
	FMA_OBJ_MAXNUM		= 20,		// obj最大数
	FMA_CMN_OBJ_MAXNUM	= 2,		// obj最大数
};

// フラグ
enum{
	FLG_FMA_UPDATE		= (1<<0),	// 更新
	FLG_FMA_DISP_1ST	= (1<<1),	// はじめの表示
	FLG_FMA_ONLY		= (1<<2),	// 1人のみの場合
	//
	FLG_FMA_SCROLL_STOP	= (1<<8),	// スクロール停止
	FLG_FMA_TIME_STOP	= (1<<9),	// タイマ停止
	//
	FLG_FMA_LEVEL0		= (1<<16),	// レベル0
	FLG_FMA_LEVEL1		= (1<<17),	// レベル1
	FLG_FMA_LEVEL2		= (1<<18),	// レベル2
	FLG_FMA_LEVEL3		= (1<<19),	// レベル3
	//
};

//------------------
// canvasのid(詳細)
//------------------
enum{
	// 上画面
	FMAI_CANVAS_UP_TITLE		= 0,
	//
	FMAI_CANVAS_UP_TIME_L,
	FMAI_CANVAS_UP_JOIN_L,
	FMAI_CANVAS_UP_CLEAR_L,
	FMAI_CANVAS_UP_SCORE_L,
	FMAI_CANVAS_UP_GAIN_L,
	//
	FMAI_CANVAS_UP_JOIN_R,
	FMAI_CANVAS_UP_SCORE_R,
	FMAI_CANVAS_UP_GAIN_R,
	//
	// 下画面
	FMAI_CANVAS_DOWN_MYSCORE_L,
	FMAI_CANVAS_DOWN_RANK_L,
	FMAI_CANVAS_DOWN_TOPSCORE_L,
	FMAI_CANVAS_DOWN_NAME_L,
	FMAI_CANVAS_DOWN_HIGH_L,
	FMAI_CANVAS_DOWN_EXPLAIN_L,
	//
	FMAI_CANVAS_DOWN_MYSCORE_R,
	FMAI_CANVAS_DOWN_RANK_R,
	FMAI_CANVAS_DOWN_TOPSCORE_R,
	FMAI_CANVAS_DOWN_HIGH_R,
	//
	// スクロール
	FMAI_CANVAS_UP_SCROLL_00,
	FMAI_CANVAS_UP_SCROLL_01,
};

//------------------
// canvasのid(評価)
//------------------
enum{
	// 上画面
	FMAR_CANVAS_UP_TITLE		= 0,
	//
	FMAR_CANVAS_UP_JOIN_L,
	FMAR_CANVAS_UP_CLEAR_L,
	FMAR_CANVAS_UP_SCORE_L,
	FMAR_CANVAS_UP_TIME_L,
	//
	FMAR_CANVAS_UP_JOIN_R,
	FMAR_CANVAS_UP_SCORE_R,
	//
	// 下画面
	FMAR_CANVAS_DOWN_MYSCORE_L,
	FMAR_CANVAS_DOWN_RANK_L,
	FMAR_CANVAS_DOWN_TOPSCORE_L,
	FMAR_CANVAS_DOWN_NAME_L,
	FMAR_CANVAS_DOWN_GAIN_L,
	FMAR_CANVAS_DOWN_BONUS_L,
	FMAR_CANVAS_DOWN_DOUBLE_L,
	//
	FMAR_CANVAS_DOWN_MYSCORE_R,
	FMAR_CANVAS_DOWN_RANK_R,
	FMAR_CANVAS_DOWN_TOPSCORE_R,
	FMAR_CANVAS_DOWN_GAIN_R,
	FMAR_CANVAS_DOWN_BONUS_R,
	FMAR_CANVAS_DOWN_DOUBLE_R,
};

//-----------------------------
// objのres
//-----------------------------
enum{
	// 通常
	FMA_RESID_OBJ_PLT = 0,		// パレット
	FMA_RESID_OBJ_CHR,			// キャラ
	FMA_RESID_OBJ_CEL,			// セルアニメ
	// cmn
	FMA_RESID_CMN_OBJ_PLT,		// パレット
	FMA_RESID_CMN_OBJ_CHR,		// キャラ
	FMA_RESID_CMN_OBJ_CEL,		// セルアニメ
	//
	FMA_RESID_OBJ_MAX,
};

//----------
// clwork
//----------
enum{
	FMA_CLWKMODE_NORMAL = 0,	// 通常
	FMA_CLWKMODE_CMN,			// 共通
};

//-------------------
// clwkのid(詳細画面)
//-------------------
enum{
	FMAI_CLWK_ICON_00 = 0,		// 時間アイコン
	FMAI_CLWK_ICON_01,			// 人数アイコン
	FMAI_CLWK_STAR,				// 星マーク
	FMAI_CLWK_TIME_00,			// 時間(5つ分)
	FMAI_CLWK_TIME_01,			// 時間
	FMAI_CLWK_TIME_02,			// 時間
	FMAI_CLWK_TIME_03,			// 時間
	FMAI_CLWK_TIME_04,			// 時間
	FMAI_CLWK_SCORE_00,			// スコア(4つ分)
	FMAI_CLWK_SCORE_01,			// スコア
	FMAI_CLWK_SCORE_02,			// スコア
	FMAI_CLWK_SCORE_03,			// スコア
};

//-------------------
// clwkのid(評価画面)
//-------------------
enum{
	FMAR_CLWK_ICON_01 = 0,		// 人数アイコン
	FMAR_CLWK_STAR,				// 星マーク
	FMAR_CLWK_TIME_00,			// 時間(5つ分)
	FMAR_CLWK_TIME_01,			// 時間
	FMAR_CLWK_TIME_02,			// 時間
	FMAR_CLWK_TIME_03,			// 時間
	FMAR_CLWK_TIME_04,			// 時間
	FMAR_CLWK_SCORE_00,			// スコア(4つ分)
	FMAR_CLWK_SCORE_01,			// スコア
	FMAR_CLWK_SCORE_02,			// スコア
	FMAR_CLWK_SCORE_03,			// スコア
	FMAR_CLWK_RECORD_00,		// レコード
	FMAR_CLWK_RECORD_01,		// レコード
};

//-------------------
// clwkのid(cmn)
//-------------------
enum{
	FMA_CLWK_CMN_BACK = 0,		// 戻るアイコン
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//-------------------------------------
// 一つ前の情報(まとめ)
//-------------------------------------
typedef struct{
	s32			Timer;			// 時間
	s32			Num;			// 人数
	s32			Score;			// スコア
	s32			Rank;			// ランク
} FMISSION_APP_OLD_WORK;

//-------------------------------------
/// OBJ管理ワーク
//-------------------------------------
typedef struct
{
	u32			res[FMA_RESID_OBJ_MAX]; 			// リソース
	// 共通
	GFL_CLWK	*cmn_clwk[FMA_CMN_OBJ_MAXNUM];		// CLWK(common)
	// 通常
	GFL_CLWK	*clwk[FMA_OBJ_MAXNUM];				// CLWK
} FMISSION_APP_OBJ_WORK;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
	HEAPID					heapID;
	// OBJ
	FMISSION_APP_OBJ_WORK	wk_obj;
	//描画設定
	FMISSION_APP_GRAWK		*graphic;
	//
	GFL_FONT				*font;							// フォント
	PRINT_QUE				*print_que;						// プリントキュー
	GFL_MSGDATA				*MsgData[FMA_MSG_MAXNUM];		// メッセージ
	PRINT_UTIL				PrintUtil[FMA_CANVAS_MAXNUM];	// 文字表示用
	WORDSET					*Wordset;						// wordset
	//
	FMISSION_APP_PARAM 		*Param;				// 参照用として保存	
	//
	FMISSION_DATA_MAN		*FMissionManager;	// manager
	ARCHANDLE				*Handle;
	FMISSION_ARCDATA		*ArcData;
	LINKFEST_MISSION_STATUS	MissionStatus;		// 情報(実体)
	LINKFEST_LOG_TOP_SCORE	*TopScore;			// トップスコア用
	FMISSION_LOG_LATEST		*Latest;			// ログ
	//
	s32						Step;				// 処理の流れ
	s32						Mode;				// モード(詳細、評価)
	u32						Flag;				// フラグ
	s32						BgOffX;				// オフセット
	s32						Count;				// tmp
	s32						LevelTimer;			// タイマ
	s32						Level;				// 段階
	//
	FMISSION_APP_OLD_WORK	OldWork;			// 保存
	//
	s16						*EmptyBuffer;		// 座標用のバッファ(必ず破棄すること)
} FMISSION_APP_WORK;

