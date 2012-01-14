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

//通信アイコン
#include "net/wih_dwc.h"

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

#include "app/app_printsys_common.h"
#include "app/app_keycursor.h"

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
	FMA_OBJ_RD_MAXNUM	= 1,	// obj(result)
};

// フラグ(Flag)
enum{
	FLG_FMA_UPDATE		= (1<<0),	// 更新
	FLG_FMA_DISP_1ST	= (1<<1),	// はじめの表示
	FLG_FMA_ONLY		= (1<<2),	// 1人のみの場合
	FLG_FMA_SUCCESS		= (1<<3),	// 成功時
	//
	FLG_FMA_BONUS		= (1<<4),	// ボーナス時
	FLG_FMA_DOUBLE		= (1<<5),	// ダブルスコア時
	FLG_FMA_CLEAR_TIME	= (1<<6),	// クリアタイム表示
	FLG_FMA_GET			= (1<<7),	// 成功して報酬を獲得
	//
	FLG_FMA_SCROLL_STOP	= (1<<8),	// スクロール停止
	FLG_FMA_TIME_STOP	= (1<<9),	// タイマ停止
	FLG_FMA_HILINK		= (1<<10),	// ハイリンクの条件を満たす
	FLG_FMA_GET2		= (1<<11),	// 成功して報酬を獲得2
	//
	FLG_FMA_NEW_TS		= (1<<12),	// トータルスコア(NEWマーク)
	FLG_FMA_NEW_CT		= (1<<13),	// クリアタイム(NEWマーク)
	FLG_FMA_NEW_RD		= (1<<14),	// マイスコア(NEWマーク)
	//
	FLG_FMA_LEVEL0		= (1<<16),	// レベル0
	FLG_FMA_LEVEL1		= (1<<17),	// レベル1
	FLG_FMA_LEVEL2		= (1<<18),	// レベル2
	FLG_FMA_LEVEL3		= (1<<19),	// レベル3
	//
	FLG_FMA_LEVEL4		= (1<<20),	// レベル4
	FLG_FMA_ME_WAIT		= (1<<21),	// ME待ち
	FLG_FMA_ME_MES		= (1<<22),	// MEのメッセージ待ち
	//
	FLG_FMA_MSG_00		= (1<<24),	// 成功or失敗
	FLG_FMA_MSG_01		= (1<<25),	// 手に入れたor失敗
	FLG_FMA_MSG_02		= (1<<26),	// ダブルスコア
	FLG_FMA_MSG_03		= (1<<27),	// 手に入れた2
	//
	FLG_FMA_MSG_04		= (1<<28),	// 共鳴した
	FLG_FMA_MSG_END		= (1<<29),	// 終了
	//
	FLG_FMA_DEBUG		= (1<<31),	// debug
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
	//
	FMAI_CANVAS_UP_JOIN_R,
	FMAI_CANVAS_UP_CLEAR_R,
	FMAI_CANVAS_UP_SCORE_R,
	//
	// 下画面
	FMAI_CANVAS_DOWN_MYSCORE_L,
	FMAI_CANVAS_DOWN_RANK_L,
	FMAI_CANVAS_DOWN_TOPSCORE_L,
	FMAI_CANVAS_DOWN_NAME_L,
	FMAI_CANVAS_DOWN_EXPLAIN_L,
	//
	FMAI_CANVAS_DOWN_MYSCORE_R,
	FMAI_CANVAS_DOWN_RANK_R,
	FMAI_CANVAS_DOWN_TOPSCORE_R,
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
	FMAR_CANVAS_UP_CLEAR_R,
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
	//
	FMAR_CANVAS_DOWN_WIN,
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
	// 通常(main)
	FMA_RESID_RD_OBJ_PLT,		// パレット
	FMA_RESID_RD_OBJ_CHR,		// キャラ
	FMA_RESID_RD_OBJ_CEL,		// セルアニメ
	//
	FMA_RESID_OBJ_MAX,
};

//----------
// clwork
//----------
enum{
	FMA_CLWKMODE_NORMAL = 0,	// 通常
	FMA_CLWKMODE_CMN,			// 共通
	FMA_CLWKMODE_RD,			// 結果
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

//-------------------
// clwkのid(result)
//-------------------
enum{
	FMA_CLWK_RESULT_RECORD = 0,	// レコード
};

//-------------------
// MSG
//-------------------
enum{
	FMA_MSGWIN_00 = 0,			// 達成
	FMA_MSGWIN_01,				// 失敗
	FMA_MSGWIN_02,				// 手に入れた
	FMA_MSGWIN_03,				// いれられない
	FMA_MSGWIN_04,				// ダブルスコア
	FMA_MSGWIN_05,				// 個人
	FMA_MSGWIN_06,				// みんな
	FMA_MSGWIN_02A,				// 手に入れた2
	FMA_MSGWIN_03A,				// いれられない2
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
// スコアログの情報(1名分)
//-------------------------------------
typedef struct{
	STRBUF		*strbuf;		// 名前
	s32			score;			// スコア
	s32			tr_id;			// tr_id
} FMISSION_APP_LOG;

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
	// 結果
	GFL_CLWK	*rd_clwk[FMA_OBJ_RD_MAXNUM];		// CLWK(result down)
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
  //すれ違いアイコン表示更新用
  WIFI_LIST* wifi_list;

	//
	GFL_FONT				*font;							// フォント
	PRINT_QUE				*print_que;						// プリントキュー
	GFL_MSGDATA				*MsgData[FMA_MSG_MAXNUM];		// メッセージ
	PRINT_UTIL				PrintUtil[FMA_CANVAS_MAXNUM];	// 文字表示用
	WORDSET					*Wordset;						// wordset
	PRINT_STREAM			*PrintStream;					// 文字表示用(PrintStream)
	GFL_TCBLSYS				*MsgTcblSys;					// メッセージ用タスク
	STRBUF					*MsgBuffer;						// メッセージ用バッファ
	APP_KEYCURSOR_WORK		*MsgCursorWork;					// ページ送りマーク用ワーク
	APP_PRINTSYS_COMMON_WORK PrintSysWork;					// 汎用PrintStream管理ワーク
	//
	FMISSION_APP_PARAM 		*Param;				// 参照用として保存	
	//
	FMISSION_DATA_MAN		*FMissionManager;	// manager
	ARCHANDLE				*Handle;
	FMISSION_ARCDATA		*ArcData;
	LINKFEST_MISSION_STATUS	MissionStatus;		// 情報(実体)
	LINKFEST_MISSION_RESULT MissionResult;		// 結果(実体)
	LINKFEST_LOG_TOP_SCORE	*TopScore;			// トップスコア用
	FMISSION_LOG_LATEST		*Latest;			// ログ
	FMISSION_LOG_LATEST		LatestLocal;		// ログ(localで使用：実体)
	FMISSION_APP_LOG		ScoreLog;			// スコアログ(1名分)
	//
	s32						Step;				// 処理の流れ
	s32						Mode;				// モード(詳細、評価)
	u32						Flag;				// フラグ
	s32						BgOffX;				// オフセット
	s32						LevelTimer;			// タイマ
	s32						Level;				// 段階
	//
	FMISSION_APP_OLD_WORK	OldWork;			// 保存
	//
	s16						*EmptyBuffer;		// 座標用のバッファ(必ず破棄すること)
} FMISSION_APP_WORK;

