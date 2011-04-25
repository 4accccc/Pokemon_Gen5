//=============================================================================
/**
 *
 *	@file		wbt_list.c
 *	@brief  WBTトーナメント表
 *	@author	tamada
 *	@date		2011.03.02
 *
 * 2011.03.02 ui_templateから派生、作成開始
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

//テクスチャをOAMに転送
#include "system/nsbtx_to_clwk.h"
#include "system/bmp_oam.h"
#include "system/bmp_winframe.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//描画設定
#include "wbt_list_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"
// print
#include "ui/print_tool.h"
#include "gamesystem/msgspeed.h"

//マップモデルのnsbtx
#include "fieldmap/fldmmdl_mdlres.naix"

//アプリ共通素材
#include "app/app_menu_common.h"

// サウンド
#include "sound/pm_sndsys.h"
#include "wbt_list_snd_def.h"

//アーカイブ
#include "arc_def.h"

#include "system/wipe.h"

#include "app/app_keycursor.h"
#include "app/app_printsys_common.h"

//外部公開
#include "app/wbt_list.h"

//@TODO BG読み込み とりあえずマイクテストのリソース
#include "message.naix"
#include "wbt_list_gra.naix"

#include "msg/msg_wbt_tournament.h"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
// debug表示が必要なときはセット
#define _DEBUG_LOCAL_
//#undef  _DEBUG_LOCAL_

// Bボタン戻りがある場合(debug)
#define _DEBUG_INPUT_
#undef  _DEBUG_INPUT_

// SOUND
#define _DEBUG_SOUND_
#undef  _DEBUG_SOUND_

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{
	UI_TEMPLATE_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};


//-------------------------------------
///	フレーム(要整理)
//=====================================
enum
{
	BG_FRAME_BACK_M		= GFL_BG_FRAME3_M,		// 下地(BG3)
	BG_FRAME_MIDDLE_M	= GFL_BG_FRAME2_M,		// 中間(BG2)
	BG_FRAME_MIDDLE2_M	= GFL_BG_FRAME1_M,		// テキスト枠(BG1)
	BG_FRAME_TEXT_M		= GFL_BG_FRAME0_M,		// テキスト面(BG0)
	//
	BG_FRAME_BACK_S		= GFL_BG_FRAME2_S,		// 下地(BG2)
	BG_FRAME_MIDDLE_S	= GFL_BG_FRAME1_S,		// 中間(BG1)
	BG_FRAME_TEXT_S 	= GFL_BG_FRAME0_S,		// テキスト面(BG0)
};

//-------------------------------------
///	パレット(要整理)
//=====================================
enum
{
	//メインBG
	PLTID_BG_BACK_M		= 0,
	PLTID_BG_MESWIN		= 8,		// メッセージウインドウ表示
	PLTID_BG_FONT		= 9,		// メッセージ用のフォント
	//
	//サブBG
	PLTID_BG_BACK_S		= 0,		// 下地
	//
	//メインOBJ
	PLTID_OBJ_PANEL		= 0,		// 6本使用
	PLTID_OBJ_MAPMODEL	= 6,		// 8本使用
	//サブOBJ
	PLTID_OBJ_PMS_DRAW 	= 0,		// 5本使用
};

enum{
	// トレーナー人数
	WBT_TRAINER_MAX = 8,
};

//-------------------------------------
// 以下に追加
//=====================================
// モード
enum{
	MODE_ROUND1 = 0,				// 1回戦
	MODE_ROUND2,					// 2回戦
	MODE_ROUND3,					// 3回戦
	//
	MODE_ROUND_MAX,
};

// 段階
enum{
	STEP_INIT	= 0,				// 初期化
	STEP_LV1 	= 100,
	STEP_LV2 	= 200,
	STEP_LV3 	= 300,
	STEP_LV4 	= 400,
	STEP_LV5 	= 500,
	STEP_LV6 	= 600,
	STEP_LV7 	= 700,
	STEP_LV_MAX = 1000,
	//
	STEP_END	= 10000,
};

// OBJのリソースのインデックス
enum{
	RESID_OBJ_PLT = 0,				// パレット
	RESID_OBJ_CHR,					// キャラ
	RESID_OBJ_CEL,					// セルアニメ
	//
	RESID_OBJ_MAX,
};

// CLWK(上画面)
enum
{
	CLWKID_PANEL_00 = 0,			// パネル(上)
	CLWKID_PANEL_01,				// パネル
	CLWKID_PANEL_02,				// パネル
	CLWKID_PANEL_03,				// パネル
	CLWKID_PANEL_04,				// パネル
	CLWKID_PANEL_05,				// パネル
	CLWKID_PANEL_06,				// パネル
	CLWKID_PANEL_07,				// パネル
	CLWKID_PANEL_00A,				// パネル(下:移動)
	CLWKID_PANEL_01A,				// パネル
	CLWKID_PANEL_02A,				// パネル
	CLWKID_PANEL_03A,				// パネル
	CLWKID_PANEL_04A,				// パネル
	CLWKID_PANEL_05A,				// パネル
	CLWKID_PANEL_06A,				// パネル
	CLWKID_PANEL_07A,				// パネル
	CLWKID_BOMB,					// 爆発アニメ
	CLWKID_BAR_00,					// バー0(横)
	CLWKID_BAR_01,					// バー1
	CLWKID_BAR_02,					// バー2
	CLWKID_BAR_03,					// バー3
	CLWKID_BAR_10,					// バー10(縦)
	CLWKID_BAR_11,					// バー11
	//
	CLWKID_MAX,
};

// 移動のパラメータ
enum{
	MWORK_PANEL_L	= 0,			// 左移動
	MWORK_PANEL_R,					// 右移動
	//
	MWORK_MAX,
};

// 移動の種類
enum{
	MOVE_PANEL_IN	= 0,			// パネルin
	MOVE_PANEL_OUT,					// パネルout
	//
	MOVE_PANEL_MAX,
};

// タイマindex(念のため、2の倍数にしておく)
enum{
	TIMER_PANEL,					// パネルイン/アウト
	TIMER_GAGE,						// ゲージ伸び
	TIMER_BOMB,						// 爆発
	TIMER_WAIT,
	
	TIMER_MAX,
};

// タイマ値
enum{
	FRAME_PANEL = 60,
	FRAME_GAGE	= 60,
	FRAME_BOMB	= 10,
	FRAME_WAIT	= 60,
};

// 最大数
enum{
	WBT_OBJ_MAXNUM		= 23,		// obj最大
	WBT_CANVAS_MAXNUM	= 3,		// canvas最大
	WBT_BMPOAM_MAXNUM	= 8,		// bmpoam最大
	//
	WBT_EMPTY_MAXNUM	= 46*2,		// emptyの最大値
	//
	WBT_GAGE_MAXNUM		= 6,		// ゲージ数最大
};

// スピード
enum{
	MOVE_IN_SPEED		= 9,		// スクロールイン
	MOVE_OUT_SPEED		= 9,		// スクロールアウト
//	MOVE_IN_SPEED		= 18,		// スクロールイン
//	MOVE_OUT_SPEED		= 36,		// スクロールアウト
};

// empty
enum{
	BOMB_EMPTY_ID		= 24,		// 爆発のempty
	//
	L_BMPOAM_EMPTY_ID	= 44,		// 左側、BMPOAMのempty
	R_BMPOAM_EMPTY_ID	= 45,		// 右側、BMPOAMのempty
};

// シーケンス
enum{
	L_WHITE_SEQ			= 0,		// 左、白シーケンス
	L_GRAY_SEQ			= 1,		// 左、灰シーケンス
	L_FLASH_SEQ			= 12,		// 左、点滅(終了後灰)シーケンス
	R_WHITE_SEQ			= 4,		// 右、白シーケンス
	R_GRAY_SEQ			= 5,		// 右、灰シーケンス
	R_FLASH_SEQ			= 13,		// 右、点滅(終了後灰)シーケンス
	//
	L_BLUE_SEQ			= 2,		// 左、青シーケンス
	L_RED_SEQ			= 3,		// 左、赤のシーケンス
	R_BLUE_SEQ			= 6,		// 右、青シーケンス
	R_RED_SEQ			= 7,		// 右、赤のシーケンス
	//
	GAGE_GRAY_SEQ		= 14,		// 灰色ゲージ
	//
	L_WIN_SEQ			= 15,		// 左、勝ちシーケンス
	R_WIN_SEQ			= 16,		// 右、勝ちシーケンス
};

// mes関連
enum{
	MES_WIN_TOP_CHR		= 1,		// ウインドウ用の先頭キャラ(0は抜き用のはず)
	MES_STRING_BUFFER	= 200,		// 文字列のバッファ
	MES_CLEAR_COLOR		= 15,		// クリアカラー
	MES_CURSOR_COLOR	= 15,		// 文字送りカーソル抜きカラー(printstreamのwindowのパレット番号と対応)
	//
	MES_RANDOM_MAX		= 5,		// ランダム値(0-4まで選択)
};

//-------------------
// フラグ
//-------------------
// 移動のフラグ(MoveWork.flag)
enum{
	FLG_MOVE_INIT	= (1<<0),		// 初期化
	FLG_MOVE_END	= (1<<1),		// 終了
};

// OBJのフラグ(WBTLIST_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM	= (1<<0),		// アニメあり
	FLG_OBJ_HFLIP	= (1<<1),		// HFLIP
	FLG_OBJ_VFLIP	= (1<<2),		// VFLIP
};

// メッセージのフラグ(MesFlag)
enum{
	FLG_MES_1ST		= (1<<0),		// 1回目終了
	FLG_MES_END		= (1<<1),		// 2回目終了
	//
	FLG_MES_EX1		= (1<<2),		// 1回戦特別(ex1)
	FLG_MES_EX2		= (1<<3),		// 1回戦特別(ex2)
	//
	FLG_MES_LAST	= (1<<4),		// それぞれの最後(特別メッセージに変更になった)
};

// debugのフラグ(DebugFlag)
enum{
	FLG_DBG_MODE	= (1<<0),		// モード(0:デバッグなし、1:あり)
	FLG_DBG_COLOR	= (1<<1),		// 色変え
	//
	FLG_DBG_INPUT	= (1<<4),		// 入力
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
///	BG管理ワーク
//==============================================================
typedef struct 
{
	int dummy;
} WBTLIST_BG_WORK;

//-------------------------------------
/// OBJ管理ワーク
//-------------------------------------
typedef struct
{
	u32			res[RESID_OBJ_MAX]; 		//リソース
	GFL_CLWK	*clwk[WBT_OBJ_MAXNUM];		//CLWK
} WBTLIST_OBJ_WORK;

//-------------------------------------
/// 移動用管理ワーク
/// (どこかにあるはずなので仮作成)
/// (後に修正:固定小数点にする?)
//-------------------------------------
typedef struct
{
	s16			sx;				// 開始
	s16			ex;				// 終了
	s16			flag;			// フラグ
	s16			pad;
	//
	s32			rate;			// 加算値(x100)
	s32			calcnum;		// 計算結果(x100)
} WBTLIST_MOVE_WORK;


//-------------------------------------
/// パラメータ参照用
/// 初期化時にパラメータをセットしてしまう
//-------------------------------------
typedef struct
{
	u8			exppos;			// 爆発位置
	u8			winpos;			// 勝った位置
	u8			moveflag;		// 移動可能フラグ
	u8			linework;		// ラインのワーク
	u8			pattern;		// パターン
	u8			mesid;			// メッセージid
	u8			t_1st;			// トレーナー番号(左)
	u8			t_2nd;			// トレーナー番号(右)
} WBTLIST_PARAM_WORK;

// 個数
enum{
	PARAM_ROUND1 = 3,		// 決めせりふ(1) + トーナメント、ルール名(1) + 1回戦(1)
	PARAM_ROUND2 = 5,		// 人数分の結果(4) + 2回戦(1)
	PARAM_ROUND3 = 3,		// 人数分の結果(2) + 決勝戦(1)
	//
	PARAM_WORK_MAX	= (PARAM_ROUND1 + PARAM_ROUND2 + PARAM_ROUND3),
};

// パラメータ参照用(WBTLIST_PARAM_WORKと対応)
enum{
	PRM_EXPPOS = 0,				// 爆発位置
	PRM_WINPOS,					// 勝った位置
	PRM_MOVEFLAG,				// 移動可能フラグ
	PRM_LINEWORK,				// ラインのワーク
	PRM_PATTERN,				// パターン
	PRM_MESID,					// メッセージid
	PRM_T1ST,					// トレーナー番号(左)
	PRM_T2ND,					// トレーナー番号(右)
};

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
	HEAPID					heapID;
	// BG
	WBTLIST_BG_WORK			wk_bg;
	// OBJ
	WBTLIST_OBJ_WORK		wk_obj;
	//描画設定
	WBTLIST_GRAPHIC_WORK	*graphic;
	//フォント
	GFL_FONT				*font;
	//プリントキュー
	PRINT_QUE				*print_que;
	GFL_MSGDATA				*msg[2];						// 0:メッセージ,1:未使用
	
	// OAMマップモデル
	u8						clwk_oam_mmdl_count;
	UI_EASY_CLWK_RES		clres_oam_mmdl;
	GFL_CLWK				*clwk_oam_mmdl[ WBT_TRAINER_MAX ];
	
	//---------------------
	// 以下に追加記入
	//---------------------
	WBTLIST_PARAM 			*Param;							// 参照用として保存
	
	BMPOAM_SYS_PTR			BmpOamSys;						// BMPOAM
	GFL_BMP_DATA			*BmpData[WBT_BMPOAM_MAXNUM];	// Bmp
	BMPOAM_ACT_PTR			BmpOamWork[WBT_BMPOAM_MAXNUM];	// BMPOAM用Work
	PRINT_UTIL				PrintUtil[WBT_CANVAS_MAXNUM];	// 文字表示用
	PRINT_STREAM			*PrintStream;					// 文字表示用(PrintStream)
	GFL_TCBLSYS				*MsgTcblSys;					// メッセージ用タスク
	STRBUF					*MsgBuffer;						// メッセージ用バッファ
	APP_KEYCURSOR_WORK		*MsgCursorWork;					// ページ送りマーク用ワーク
	APP_PRINTSYS_COMMON_WORK PrintSysWork;					// 汎用PrintStream管理ワーク
	//
	s32						Mode;							// モード(段階)
	s32						Step;							// ステップ
	s32						DispCount;						// カウンタ
	s32						PlayerNum;						// プレイヤの番号
	u32						MoveFlag;						// 移動可能フラグ
	u32						MesFlag;						// メッセージフラグ
	u32						DebugFlag;						// debug
	s32						DebugSound;						// debug(sound)
	s32						RandCount;						// random用カウンタ
	s32						RandBuffer[4];					// random用のバッファ
	s32						Timer[TIMER_MAX];				// タイマ
	WBTLIST_MOVE_WORK		MoveWork[MWORK_MAX];			// 移動用ワーク
	WBTLIST_PARAM_WORK		ParamWork[PARAM_WORK_MAX];		// パラメータのワーク
	u16						PalBuffer[0x10*8];				// 色変え用(u16なので1パレット0x10分でよい)
	u16						TmpBuffer[0x10*8];				// パレット用(連続で転送させるためPalBufferと同じサイズを確保)
	//
	s16						*EmptyBuffer;					// 座標用のバッファ(必ず破棄すること)
} WBTLIST_MAIN_WORK;


//-----------------------------------------
// OBJ用の定義
//------------------------------------------
typedef struct{
	u8				anmseq;			// アニメシーケンス(0xff終了コード)
	u8				softpri;		// OBJ(プライオリティ)
	u8				bgpri;			// BG(プライオリティ)
	u8				visible;		// 表示/非表示
	u8				palnum;			// パレット番号(現在未使用だが、しばらく残し)
	u8				dispflag;		// 表示フラグ
	u8				empty_num;		// empty番号(これから座標を参照)
	u8				pad;
}WBTLIST_OBJ_DATA;

//-----------------------------------------
// 文字用の定義(表示領域の設定:CANVAS)
//------------------------------------------
typedef struct{
	u8				frmnumber;		// BG上下(frame定義なので注意)(0xffの場合は終了判定)
	u8				x;				// 座標
	u8				y;
	u8				width;			// サイズ
	u8				height;
	u8				palnum;			// パレット番号
}WBTLIST_CANVAS_DATA;

//-----------------------------------------
// BMPOAMの定義
//------------------------------------------
typedef struct{
	u8				width;			// サイズ(8x8単位)(0xffの場合は終了判定)
	u8				height;
	u8				softpri;		// プライオリティ(obj)
	u8				bgpri;			// プライオリティ(bg)
	u8				visible;		// 表示/非表示
	u8				paloffset;		// パレットオフセット(nclの何番目か)
	u8				empty_num;		// empty番号
	u8				pad;
}WBTLIST_BMPOAM_DATA;

//-----------------------------------------
// MAPMODELの定義
//------------------------------------------
typedef struct{
	u8				id;				// id(パラメータ参照時は未使用)(0xffの場合終了判定)
	u8				pattern;		// パターン(パラメータ参照時は未使用)
	u8				palnum;			// 転送先パレット番号
	u8				empty_num;		// empty番号
	u8				softpri;		// プライオリティ(obj)
	u8				pad;
}WBTLIST_MAPMODEL_DATA;

//=============================================================================
/**
 *							データ定義(要整理)
 */
//=============================================================================

// OBJの情報(上側)
// narc, resid, paloffset, palnum
const s16 obj_main_info_tbl[] = {
	NARC_wbt_list_gra_wbt_list_obj_u_NCLR, RESID_OBJ_PLT, 0, 6,	// NCL
	NARC_wbt_list_gra_wbt_list_obj_u_NCGR, RESID_OBJ_CHR, 0, 0,	// NCG
	NARC_wbt_list_gra_wbt_list_obj_u_NCER, RESID_OBJ_CEL, 0, 0, // NCE
	NARC_wbt_list_gra_wbt_list_obj_u_NANR, -1, 0, 0,			// NAR?
};

// OBJのテーブルデータ(上側)
const WBTLIST_OBJ_DATA wbt_obj_tbl[] = {
	{ 0,	0x30,	1,	1,	0,	0,	8,	0,},		// 左パネル0
	{ 0,	0x30,	1,	1,	0,	0,	9,	0,},		// 左パネル1
	{ 0,	0x30,	1,	1,	0,	0,	10,	0,},		// 左パネル2
	{ 0,	0x30,	1,	1,	0,	0,	11,	0,},		// 左パネル3
	{ 4,	0x30,	1,	1,	0,	0,	12,	0,},		// 右パネル0
	{ 4,	0x30,	1,	1,	0,	0,	13,	0,},		// 右パネル1
	{ 4,	0x30,	1,	1,	0,	0,	14,	0,},		// 右パネル2
	{ 4,	0x30,	1,	1,	0,	0,	15,	0,},		// 右パネル3
	{ 2,	0x40,	1,	1,	0,	0,	16,	0,},		// 左、移動パネル0
	{ 2,	0x40,	1,	1,	0,	0,	17,	0,},		// 左、移動パネル1
	{ 2,	0x40,	1,	1,	0,	0,	18,	0,},		// 左、移動パネル2
	{ 2,	0x40,	1,	1,	0,	0,	19,	0,},		// 左、移動パネル3
	{ 6,	0x40,	1,	1,	0,	0,	20,	0,},		// 右、移動パネル0
	{ 6,	0x40,	1,	1,	0,	0,	21,	0,},		// 右、移動パネル1
	{ 6,	0x40,	1,	1,	0,	0,	22,	0,},		// 右、移動パネル2
	{ 6,	0x40,	1,	1,	0,	0,	23,	0,},		// 右、移動パネル3
	{ 0,	0x30,	0,	0,	2,	1,	24,	0,},		// 爆弾(未使用)
	{ 8,	0x50,	1,	0,	3,	1,	32,	0,},		// ゲージ00
	{ 8,	0x50,	1,	0,	3,	5,	35,	0,},		// ゲージ01
	{ 8,	0x50,	1,	0,	3,	3,	36,	0,},		// ゲージ02
	{ 8,	0x50,	1,	0,	3,	7,	39,	0,},		// ゲージ03
	{ 10,	0x50,	1,	0,	3,	5,	41,	0,},		// ゲージ10
	{ 10,	0x50,	1,	0,	3,	7,	43,	0,},		// ゲージ11

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

// BMPOAMの設定(上側)
const WBTLIST_BMPOAM_DATA wbt_bmpoam_tbl[] = {
	{ 8,	2,	0x38,	1,	1,	0,	16,	0,},		// 左移動パネル0用
	{ 8,	2,	0x38,	1,	1,	0,	17,	0,},		// 左移動パネル1用
	{ 8,	2,	0x38,	1,	1,	0,	18,	0,},		// 左移動パネル2用
	{ 8,	2,	0x38,	1,	1,	0,	19,	0,},		// 左移動パネル3用
	{ 8,	2,	0x38,	1,	1,	0,	20,	0,},		// 右移動パネル0用
	{ 8,	2,	0x38,	1,	1,	0,	21,	0,},		// 右移動パネル1用
	{ 8,	2,	0x38,	1,	1,	0,	22,	0,},		// 右移動パネル2用
	{ 8,	2,	0x38,	1,	1,	0,	23,	0,},		// 右移動パネル3用

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

// マップモデル(上側)
const WBTLIST_MAPMODEL_DATA wbt_mapmodel_tbl[] = {
	{ NARC_fldmmdl_mdlres_hero_nsbtx,     3,  6, 0, 0x40, 0, },		// 0
	{ NARC_fldmmdl_mdlres_rivel_nsbtx,    3,  7, 1, 0x40, 0, },		// 1
	{ NARC_fldmmdl_mdlres_support_nsbtx,  3,  8, 2, 0x40, 0, },		// 2
	{ NARC_fldmmdl_mdlres_veteranm_nsbtx, 2,  9, 3, 0x40, 0, },		// 3
	//
	{ NARC_fldmmdl_mdlres_leader4_nsbtx,  3, 10, 4, 0x40, 0, },		// 4
	{ NARC_fldmmdl_mdlres_trainerw_nsbtx, 2, 11, 5, 0x40, 0, },		// 5
	{ NARC_fldmmdl_mdlres_trainerm_nsbtx, 2, 12, 6, 0x40, 0, },		// 6
	{ NARC_fldmmdl_mdlres_leader3_nsbtx,  3, 13, 7, 0x40, 0, },		// 7
	//
	{ 0xff, 0, 0, 0, 0, 0, },		// end
};

// CANVASの設定
// (printutilと対応)
const WBTLIST_CANVAS_DATA wbt_canvas_tbl[] = {
	{ GFL_BG_FRAME0_S,	4,	4,	24,	2,	2,},		// 下、up
	{ GFL_BG_FRAME0_S,	4,	17,	24,	2,	2,},		// 下、down
	//
	{ GFL_BG_FRAME0_M,	1,	19,	30,	4,	9,},		// 上、meswin

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

// 伸びるゲージ
// 座標indexとフラグとシーケンス(前、後)
const s16 wbt_bar_anim_tbl[] = {
	32, 0x01, 8,  9,		// 左0
	33, 0x05, 8,  9,		// 左1
	34, 0x01, 8,  9,		// 左2
	35, 0x05, 8,  9,		// 左3
	//
	36, 0x03, 8,  9,		// 右0
	37, 0x07, 8,  9,		// 右1
	38, 0x03, 8,  9,		// 右2
	39, 0x07, 8,  9,		// 右3
	//
	40, 0x01,10, 11,		// 左10
	41, 0x05,10, 11,		// 左11
	//
	42, 0x03,10, 11,		// 右10
	43, 0x07,10, 11,		// 右11
};

// パネル移動のx座標(外 -> 中)
const s16 wbt_move_pos_tbl[] = {
	-72,   0,				// 左側(-72 <=> 0)
	220, 148,				// 右側(220 <=> 148)
};

// 文字表示位置
const u8 wbt_string_pos_tbl[] = {
	96, 0,					// 下画面(上)
	96, 0,					// 下画面(下)
};

// メッセージ(はじめ、最後、0回目(1回戦のみ有効)、1回目(1回戦のみ有効))
const u8 wbt_mes_gmm_tbl[] = {
	msg_wbt_tournament_00, msg_wbt_tournament_00b, msg_wbt_tournament_00, msg_wbt_tournament_00a,// 1回戦
	msg_wbt_tournament_01, msg_wbt_tournament_00c, 0, 0,										 // 2回戦
	msg_wbt_tournament_02, msg_wbt_tournament_00d, 0, 0,										 // 決勝戦
	0,0,0,0,
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT WBTListProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT WBTListProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT WBTListProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void WBTList_BG_LoadResource( WBTLIST_BG_WORK* wk, HEAPID heapID );

//-------------------------------------
///	OAMでマップモデル表示
//=====================================
static void wbt_init_mapmodel( WBTLIST_MAIN_WORK *wk, const WBTLIST_MAPMODEL_DATA *mapmodel_adr );
static void WBTList_OAM_MAPMODEL_DeleteCLWK( WBTLIST_MAIN_WORK* wk );

//-------------------------------------
///	以下に追加
//=====================================
static void wbt_init( WBTLIST_MAIN_WORK* wk );
static void wbt_main( WBTLIST_MAIN_WORK* wk );
static void wbt_exit( WBTLIST_MAIN_WORK* wk );
static BOOL wbt_wait_exit( WBTLIST_MAIN_WORK* wk );
static void wbt_up(   WBTLIST_MAIN_WORK* wk );
static void wbt_down( WBTLIST_MAIN_WORK* wk );
//
static void wbt_key_00( WBTLIST_MAIN_WORK* wk );
static void wbt_key_all( WBTLIST_MAIN_WORK* wk );
//
static void wbt_init_timer( WBTLIST_MAIN_WORK* wk, const s32 tmode, const s32 timer );
static s32  wbt_main_timer( WBTLIST_MAIN_WORK* wk, const s32 tmode );
//
static void wbt_init_string( WBTLIST_MAIN_WORK* wk );
static void wbt_disp_string( WBTLIST_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, STRBUF* strbuf );
static void wbt_exit_string( WBTLIST_MAIN_WORK* wk );
//
static void wbt_init_obj( WBTLIST_MAIN_WORK* wk );
static void wbt_init_obj_res(    WBTLIST_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr );
static void wbt_init_obj_clwk(   WBTLIST_MAIN_WORK* wk, const s32 drawtype, const WBTLIST_OBJ_DATA *obj_adr,    const s16 *info_adr );
static void wbt_init_obj_bmpoam( WBTLIST_MAIN_WORK* wk, const s32 drawtype, const WBTLIST_BMPOAM_DATA *bmp_adr, const s16 *info_adr );
static void wbt_exit_obj( WBTLIST_MAIN_WORK* wk );
static void wbt_chg_objseq( WBTLIST_MAIN_WORK *wk, const s32 wnum, const s32 seq );
static void wbt_set_obj_visible( WBTLIST_MAIN_WORK *wk, const s32 wnum, const BOOL bflg );
static BOOL wbt_is_anime_end( WBTLIST_MAIN_WORK *wk, const s32 wnum );
static void wbt_get_obj_paladr( WBTLIST_MAIN_WORK *wk, const s32 pnum, const s32 poff, const s32 drawtype );
static void wbt_chg_obj_pal( WBTLIST_MAIN_WORK *wk, const s32 wnum, const s32 poff, const s32 mode );
//
// 移動関連
static void wbt_init_movework( WBTLIST_MAIN_WORK* wk, const s32 wnum, const s16 sx, const s16 ex, const s16 frame );
static s16  wbt_main_movework( WBTLIST_MAIN_WORK* wk, const s32 wnum );
static BOOL wbt_is_movework(   WBTLIST_MAIN_WORK* wk, const s32 wnum );
static void wbt_init_move_panel( WBTLIST_MAIN_WORK* wk, const s32 mode );
static void wbt_init_move_panel_pos( WBTLIST_MAIN_WORK* wk, const s32 wnum, const s32 mode );
static void wbt_main_move_panel( WBTLIST_MAIN_WORK* wk );
//
// メッセージウインドウ関連
static void wbt_set_meswin( WBTLIST_MAIN_WORK* wk, const s32 mesid );
static BOOL wbt_main_meswin( WBTLIST_MAIN_WORK* wk );
static s32  wbt_chk_meswin( WBTLIST_MAIN_WORK* wk );
static void wbt_clear_meswin( WBTLIST_MAIN_WORK* wk );

//
// 座標取得
static s16 wbt_get_empty( WBTLIST_MAIN_WORK *wk, const s32 num );
//
// (アプリの)表示関連
static void wbt_set_gageanim( WBTLIST_MAIN_WORK *wk, const s32 wnum, const s32 mode, const s32 umode );
static BOOL wbt_is_gageanim( WBTLIST_MAIN_WORK *wk, const s32 wnum );
static void wbt_disp_bomb( WBTLIST_MAIN_WORK* wk, const s32 num );
static void wbt_chg_gage_color( WBTLIST_MAIN_WORK* wk );
static void wbt_init_mode( WBTLIST_MAIN_WORK* wk );
//
// 以下、(アプリの)パラメータ関連
static void wbt_init_param( WBTLIST_MAIN_WORK* wk );
static void wbt_calc_param( WBTLIST_MAIN_WORK* wk );
static s16  wbt_get_param(  WBTLIST_MAIN_WORK* wk, const s32 mode, const s32 off );
static BOOL wbt_chk_dispcount( WBTLIST_MAIN_WORK* wk );
static BOOL wbt_is_dispcount( WBTLIST_MAIN_WORK* wk );
//
static void wbt_calc_random( WBTLIST_MAIN_WORK* wk );

// 以下(debug)
static void wbt_debug_param( WBTLIST_MAIN_WORK* wk );
static void wbt_debug( WBTLIST_MAIN_WORK* wk );

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA WBTListProcData = 
{
	WBTListProc_Init,
	WBTListProc_Main,
	WBTListProc_Exit,
};
//=============================================================================
/**
 *								PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					WBTLIST_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WBTListProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	WBTLIST_MAIN_WORK 	*wk;
	WBTLIST_PARAM 		*param;
	
	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//引数取得
	param	= pwk;
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UI_DEBUG, UI_TEMPLATE_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(WBTLIST_MAIN_WORK), HEAPID_UI_DEBUG );
	GFL_STD_MemClear( wk, sizeof(WBTLIST_MAIN_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_UI_DEBUG;
	
	// 参照用
	wk->Param = pwk;
	
	//描画設定初期化
	wk->graphic	= WBTLIST_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, wk->heapID );
	
	//フォント作成
	wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	
	// メッセージ
	wk->msg[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_tournament_dat, wk->heapID );
	wk->msg[1] = 0;
	
	//PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	//
	// 会話ウインドウ用
	wk->MsgTcblSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 32 , 32 );
	wk->PrintStream = 0;
	wk->MsgBuffer 	= 0;
	
	//BGリソース読み込み
	WBTList_BG_LoadResource( &wk->wk_bg, wk->heapID );
	
	//----------------
	// 処理を記述
	//----------------
	wbt_init( wk );
	
	// ワイプ
	WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
					WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
	
	return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					WBTLIST_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WBTListProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	WBTLIST_MAIN_WORK* wk = mywk;
	
	//----------------
	// 処理を記述
	//----------------
	wbt_exit( wk );
	
	//OAMマップモデルの破棄
	WBTList_OAM_MAPMODEL_DeleteCLWK( wk );
	
	//メッセージ破棄
	if(wk->msg[0]){
		GFL_MSG_Delete( wk->msg[0] );
	}
	if(wk->msg[1]){
		GFL_MSG_Delete( wk->msg[1] );
	}
	
	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	
	//FONT
	GFL_FONT_Delete( wk->font );
	// 会話ウインドウ用
	GFL_TCBL_Exit( wk->MsgTcblSys );
	
	//描画設定破棄
	WBTLIST_GRAPHIC_Exit( wk->graphic );
	
	//PROC用メモリ解放
	{
		HEAPID heapID = wk->heapID;
		GFL_PROC_FreeWork( proc );
		GFL_HEAP_DeleteHeap( heapID );
	}
	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
	//
	return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					WBTLIST_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WBTListProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	WBTLIST_MAIN_WORK* wk = mywk;
	
	switch ( *seq )
	{
		case 0:
			if (WIPE_SYS_EndCheck())
			{
				(*seq) ++;
			}
			break;
		case 1:
			if(!(wk->DebugFlag & FLG_DBG_INPUT)){
				// 終了判定
				if(wk->Step == STEP_END){
					(*seq) ++;
				}
			}else{
				if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
				{
					(*seq) ++;
				}
			}
			break;
		case 2:
			WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
							WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
			(*seq) ++;
			break;
		//
		// ここで最終終了待ちをしているようだ
		case 3:
			if (WIPE_SYS_EndCheck())
			{
				// 終了待ち(いれても大丈夫?)
				if( wbt_wait_exit( wk ) == TRUE ){
					return GFL_PROC_RES_FINISH;
				}
			}
			break;
		default:
			break;
	}
	
	//----------------
	// 処理を追加
	//----------------
	wbt_main( wk );
	
	// 会話ウインドウ用
	GFL_TCBL_Main( wk->MsgTcblSys );
	
	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );
	
	//2D描画
	WBTLIST_GRAPHIC_2D_Draw( wk->graphic );
	
	//3D描画
	WBTLIST_GRAPHIC_3D_StartDraw( wk->graphic );
	
	WBTLIST_GRAPHIC_3D_EndDraw( wk->graphic );
	
	return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース読み込み
 *
 *	@param	WBTLIST_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void WBTList_BG_LoadResource( WBTLIST_BG_WORK* wk, HEAPID heapID )
{
	//@TODO とりあえずマイクテストのリソース
	//@TODO 複数枚ロードできるようにして切り離す？
	enum
	{
		RES_ARC = ARCID_WBTLIST_GRA,
	
		RES_PAL_S_SIZE = 0x20*3,
		RES_PAL_M_SIZE = 0x20*5,
		
		RES_PAL_S_ID 	= NARC_wbt_list_gra_wbt_list_bg_d_NCLR,
		RES_CHR_S_ID 	= NARC_wbt_list_gra_wbt_list_bg_d_NCGR,
		RES_SCR_S01_ID 	= NARC_wbt_list_gra_wbt_list_bg_d_01_NSCR,
		RES_SCR_S02_ID 	= NARC_wbt_list_gra_wbt_list_bg_d_02_NSCR,
		
		RES_PAL_M_ID	= NARC_wbt_list_gra_wbt_list_bg_u_NCLR,
		RES_CHR_M_ID	= NARC_wbt_list_gra_wbt_list_bg_u_NCGR,
		RES_SCR_M01_ID	= NARC_wbt_list_gra_wbt_list_bg_u_01_NSCR,
		RES_SCR_M02_ID	= NARC_wbt_list_gra_wbt_list_bg_u_02_NSCR,
	};
	
	ARCHANDLE	*handle;
	
	handle	= GFL_ARC_OpenDataHandle( RES_ARC, heapID );
	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_M_ID, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, RES_PAL_M_SIZE, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_S_ID, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_S_SIZE, heapID );
	//	----- SUB -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_S_ID, 	BG_FRAME_BACK_S,   0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(		handle, RES_SCR_S01_ID, BG_FRAME_BACK_S,   0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(		handle, RES_SCR_S02_ID, BG_FRAME_MIDDLE_S, 0, 0, 0, heapID );
	//	----- MAIN -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_M_ID, 	BG_FRAME_BACK_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(		handle, RES_SCR_M01_ID, BG_FRAME_BACK_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(		handle, RES_SCR_M02_ID, BG_FRAME_MIDDLE_M,  0, 0, 0, heapID );
	//
	GFL_ARC_CloseDataHandle( handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OAMマップモデル(新)
 *
 *	@param	WBTLIST_MAIN_WORK *wk ワーク
 *	@param	const WBTLIST_MAPMODEL_DATA	*
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void wbt_init_mapmodel( WBTLIST_MAIN_WORK *wk, const WBTLIST_MAPMODEL_DATA *mapmodel_adr )
{
	s16						tmp;
	s16						buf[2];
	u32						i;
	UI_EASY_CLWK_RES_PARAM	prm;
	GFL_CLUNIT				*unit = WBTLIST_GRAPHIC_GetClunit( wk->graphic );
	GFL_CLACTPOS			cpos;
	const WBTLIST_MAPMODEL_DATA	*adr = mapmodel_adr;		// アドレスセット
	
	// 処理
	for(i=0;i<WBT_TRAINER_MAX;i++){
		// 終了判定
		if(adr->id == 0xff){
			break;
		}
		prm.draw_type		= CLSYS_DRAW_MAIN;				// これもはずしてほうがいいかも..
		prm.comp_flg		= UI_EASY_CLWK_RES_COMP_NONE;
		prm.arc_id			= APP_COMMON_GetArcId();
		prm.pltt_id			= APP_COMMON_GetNull4x4PltArcIdx();
		prm.ncg_id			= APP_COMMON_GetNull4x4CharArcIdx();
		prm.cell_id			= APP_COMMON_GetNull4x4CellArcIdx( APP_COMMON_MAPPING_128K );
		prm.anm_id			= APP_COMMON_GetNull4x4AnimeArcIdx( APP_COMMON_MAPPING_128K );
		prm.pltt_line		= adr->palnum;
		prm.pltt_src_ofs	= 0;
		prm.pltt_src_num	= 1;
		
		// リソース読み込み
		UI_EASY_CLWK_LoadResource( &wk->clres_oam_mmdl, &prm, unit, wk->heapID );
		// 座標の取得
		tmp = adr->empty_num;
		cpos.x = wbt_get_empty( wk, tmp*2 );
		cpos.y = wbt_get_empty( wk, tmp*2+1 );
		// CLWK生成
		wk->clwk_oam_mmdl[i] = UI_EASY_CLWK_CreateCLWK( &wk->clres_oam_mmdl, unit, cpos.x, cpos.y, 0, wk->heapID );
		// テクスチャを転送
		buf[0] = wk->Param->trainer[i].mmdl_id;
		buf[1] = wk->Param->trainer[i].pattern_no;
		CLWK_TransNSBTX( wk->clwk_oam_mmdl[i], ARCID_MMDL_RES, buf[0], buf[1], NSBTX_DEF_SX, NSBTX_DEF_SY, 0, prm.draw_type, wk->heapID );
		wk->clwk_oam_mmdl_count ++;
		// プライオリティ
		GFL_CLACT_WK_SetSoftPri( wk->clwk_oam_mmdl[i], adr->softpri );
		//
		// 次のデータへ
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OAMマップモデル 破棄
 *
 *	@param	WBTLIST_MAIN_WORK *wk ワーク
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void WBTList_OAM_MAPMODEL_DeleteCLWK( WBTLIST_MAIN_WORK* wk )
{
	u32 i;
	
	//CLWK破棄
	for ( i = 0; i < wk->clwk_oam_mmdl_count; i ++ )
	{
		GFL_CLACT_WK_Remove( wk->clwk_oam_mmdl[i] );
	}
	//リソース開放
	UI_EASY_CLWK_UnLoadResource( &wk->clres_oam_mmdl );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(init)
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init( WBTLIST_MAIN_WORK* wk )
{
	s32		i;
	s32		buf[2];
	
	// debug
	wbt_debug_param( wk );
	
	// バッファを確保
	wk->MsgBuffer = GFL_STR_CreateBuffer( MES_STRING_BUFFER, wk->heapID );
	
	// 初期化
	// それほど複雑ではないので、
	// 上、下画面をまとめて処理
	wk->Mode 		= wk->Param->stage;
	wk->Step 		= STEP_INIT;
	wk->DispCount	= 0;
	wk->PlayerNum	= 0;
	wk->MoveFlag	= 0;
	wk->MesFlag		= 0;
	wk->DebugFlag	= 0;
	wk->DebugSound	= 0;
	wk->RandCount	= 0;
	//
	for(i=0;i<4;i++){
		wk->RandBuffer[i] = -1;
	}
	for(i=0;i<TIMER_MAX;i++){
		wk->Timer[i] = 0;
	}
	// empty用バッファ取得(必ず破棄すること)
	// キャラテーブルを借用
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_WBTLIST_GRA, NARC_wbt_list_gra_trainer_obj_pos_tblbin, wk->heapID );
	//
	// 文字初期化
	wbt_init_string( wk );
	// 下画面(上)
	buf[0] = wbt_string_pos_tbl[0];
	buf[1] = wbt_string_pos_tbl[1];
	wbt_disp_string( wk, 0, buf[0], buf[1], wk->Param->tournament_name );
	// 下画面(下)
	buf[0] = wbt_string_pos_tbl[2];
	buf[1] = wbt_string_pos_tbl[3];
	wbt_disp_string( wk, 1, buf[0], buf[1], wk->Param->tournament_rule );
	//
	// OBJ初期化
	wbt_init_obj( wk );
	//
	// 転送
	for(i=0;i<WBT_BMPOAM_MAXNUM;i++){
		GFL_BMP_Clear( wk->BmpData[i], 0 );
		PRINTSYS_PrintQueColor( wk->print_que, wk->BmpData[i], 0, 0, wk->Param->trainer[i].name, wk->font, PRINTSYS_LSB_Make(15,14,0) );
		BmpOam_ActorBmpTrans( wk->BmpOamWork[i] );
	}
	//
	// パラメータ参照
	wbt_init_param( wk );
	
	// パレットは9番に転送
	{
		ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		//上画面フォントパレット
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, PLTID_BG_FONT*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
	
	// 会話ウィンドウセット(キャラデータを転送)
	BmpWinFrame_GraphicSet( GFL_BG_FRAME0_M, MES_WIN_TOP_CHR, PLTID_BG_MESWIN, MENU_TYPE_SYSTEM, wk->heapID);
//	BmpWinFrame_GraphicSet( GFL_BG_FRAME0_M, MES_WIN_TOP_CHR, PLTID_BG_MESWIN, MENU_TYPE_TALK, wk->heapID);
	// 会話ウインドウ表示
	BmpWinFrame_Write( wk->PrintUtil[2].win, WINDOW_TRANS_ON, MES_WIN_TOP_CHR, PLTID_BG_MESWIN );
	//
	// 文字スクリーン作成
	GFL_BMPWIN_MakeScreen( wk->PrintUtil[2].win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[2].win) );
	// 文字領域クリア(キャラをクリア転送)
	wbt_clear_meswin( wk );
	//
	// 汎用PrintStream用(キー入力のみ、とした)
	APP_PRINTSYS_COMMON_PrintStreamInit( &wk->PrintSysWork, APP_PRINTSYS_COMMON_TYPE_KEY );
	// メッセージ送りマーク用(キー入力のみ、とした)
	wk->MsgCursorWork = APP_KEYCURSOR_Create( MES_CURSOR_COLOR, TRUE, FALSE, wk->heapID );
	//
	//
	// DebugFlagの判定
#ifdef PM_DEBUG

	// 入力debug
#ifdef _DEBUG_INPUT_
	wk->DebugFlag |= FLG_DBG_INPUT;
#endif		// _DEBUG_INPUT

	// ここで(FLG_DBG_MODE)の判定
	if(wk->Param->debug_value != 0){
		wk->DebugFlag |= FLG_DBG_MODE;		// test
		if(wk->DebugFlag & FLG_DBG_MODE){
			wk->DebugFlag |= FLG_DBG_COLOR;
		}
	}
#endif		// PM_DEBUG
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_main( WBTLIST_MAIN_WORK* wk )
{
	// 上画面
	wbt_up( wk );
	// 下画面
	wbt_down( wk );
	//
	// debug
	wbt_debug( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(exit)
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_exit( WBTLIST_MAIN_WORK* wk )
{
	// それほど複雑ではないので、
	// 上、下画面をまとめて処理
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	
	// 文字表示用
	wbt_exit_string( wk );
	//
	// OBJ破棄
	wbt_exit_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(完全終了待ち)
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL wbt_wait_exit( WBTLIST_MAIN_WORK* wk )
{
	// 必要であれば追加
	return( TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(上画面)
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_up( WBTLIST_MAIN_WORK* wk )
{
	// 入力
	wbt_key_all( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(下画面)
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_down( WBTLIST_MAIN_WORK* wk )
{
	// 念のため用意しておく
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_key_00( WBTLIST_MAIN_WORK* wk )
{
	s16		sbuf[2];
	s16		seq;
	s16		tmp;
	s32		step = wk->Step;
	s32		timer;
	s32		i;
	s32		mesid;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化(整列状態)
		case STEP_INIT:
			wk->Step = STEP_LV1;
			// 初期化
			wbt_init_move_panel( wk, MOVE_PANEL_IN );
			//
			// 初期状態にする
			wbt_init_mode( wk );
			//
			// 1回戦は特別処理
			if(wk->Mode == MODE_ROUND1){
				wk->Step = STEP_INIT+10;
				// メッセージ表示
				mesid = wbt_chk_meswin( wk );
				wbt_set_meswin( wk, mesid );
			}else{
				// se(panel in)
				PMSND_PlaySE( SND_WBT_LIST_PANELIN );
			}
			//
			break;
		
		// メッセージ表示待ち
		case STEP_INIT+10:
			// メッセージ待ち
			if( wbt_main_meswin( wk ) == FALSE ){
				// カウンタをすすめる(特別処理)
				// 1回戦だけ処理が異なる
				wbt_chk_dispcount( wk );
				// 移動可能パラメータの更新
				wk->MoveFlag = wbt_get_param( wk, PRM_MOVEFLAG, 0 );
				//
				wk->Step = STEP_LV1;
				wbt_init_move_panel( wk, MOVE_PANEL_IN );
				// meswinクリア
				wbt_clear_meswin( wk );
				// se(panel in)
				PMSND_PlaySE( SND_WBT_LIST_PANELIN );
			}
			break;
		
		
		// LV1(パネルイン)
		case STEP_LV1:
			// 初期化
			wbt_main_move_panel( wk );
			// 移動終了
			if( wbt_is_movework( wk, MWORK_PANEL_L ) == FALSE ){
				wk->Step = STEP_LV2;
				// メッセージ表示
				mesid = wbt_chk_meswin( wk );
				wbt_set_meswin( wk, mesid );
			}
			break;
		
		
		// LV2(メッセージ表示待ち)
		case STEP_LV2:
			// メッセージ待ち
			if( wbt_main_meswin( wk ) == FALSE ){
				//
				// 1回戦 => 終了
				if(wk->Mode == MODE_ROUND1){
					// カウンタをすすめる(特別処理)
					// 1回戦だけ処理が異なる
					wbt_chk_dispcount( wk );
					// 移動可能パラメータの更新
					wk->MoveFlag = wbt_get_param( wk, PRM_MOVEFLAG, 0 );
					//
					wk->Step = STEP_LV3+10;
					wbt_init_move_panel( wk, MOVE_PANEL_OUT );
					// meswinクリア
					wbt_clear_meswin( wk );
					// se(panel out)
					PMSND_PlaySE( SND_WBT_LIST_PANELOUT );
				}
				// 2、3回戦=>パネルアウト
				else{
					wk->Step = STEP_LV3;
					wbt_init_move_panel( wk, MOVE_PANEL_OUT );
					// meswinクリア
					wbt_clear_meswin( wk );
					// se(panel in)
					PMSND_PlaySE( SND_WBT_LIST_PANELOUT );
				}
			}
			break;
		
		//
		// LV3(パネルアウト)
		case STEP_LV3:
			// 初期化
			wbt_main_move_panel( wk );
			// 移動終了
			if( wbt_is_movework( wk, MWORK_PANEL_L ) == FALSE ){
				//------------------------
				// 更新(ここでカウンタ+1)
				//------------------------
				wbt_chk_dispcount( wk );
				//
				// 移動可能パラメータの更新
				wk->MoveFlag = wbt_get_param( wk, PRM_MOVEFLAG, 0 );
				//
				// 爆発(保留)
//				tmp = wbt_get_param( wk, PRM_EXPPOS, -1 );
//				wbt_disp_bomb( wk, tmp );
				//
				// パネル点滅
				tmp = wbt_get_param( wk, PRM_EXPPOS, -1 );
				seq = R_FLASH_SEQ;
				if(tmp < 4){
					seq = L_FLASH_SEQ;
				}
				wbt_chg_objseq( wk, tmp, seq );
				//
				// 次へ
				wk->Step = STEP_LV4;
				wbt_init_timer( wk, TIMER_BOMB, FRAME_BOMB );
			}
			break;
		//
		// LV3+10(パネルアウト:MODE_ROUND1用)
		case STEP_LV3+10:
			// 初期化
			wbt_main_move_panel( wk );
			// 移動終了
			if( wbt_is_movework( wk, MWORK_PANEL_L ) == FALSE ){
				wk->Step = STEP_LV3+20;
				// メッセージ表示
				mesid = wbt_chk_meswin( wk );
				wbt_set_meswin( wk, mesid );
			}
			break;
		//
		// LV3+20(メッセージ待ち:MODE_ROUND1用)
		case STEP_LV3+20:
			// メッセージ待ち
			if( wbt_main_meswin( wk ) == FALSE ){
				// 終了
				wk->Step = STEP_END;
			}
			break;
		
		//
		// LV4(爆破)
		case STEP_LV4:
//			timer = wbt_main_timer( wk, TIMER_BOMB );
//			if(timer <= 0){
			// パネル点滅アニメ待ち
			tmp = wbt_get_param( wk, PRM_EXPPOS, -1 );
			if( wbt_is_anime_end( wk, tmp ) == FALSE ){
				// アニメが終了してから色変え
				// キャラのパレット変更
				tmp = wbt_get_param( wk, PRM_EXPPOS, -1 );
				wbt_chg_obj_pal( wk, tmp, PLTID_OBJ_MAPMODEL, 0 );
				//
				// 点滅パネルの色を灰色にする
				tmp = wbt_get_param( wk, PRM_EXPPOS, -1 );
				seq = R_GRAY_SEQ;
				if(tmp < 4){
					seq = L_GRAY_SEQ;
				}
				wbt_chg_objseq( wk, tmp, seq );
				//
				// アニメ終了=>ゲージ伸び
				wk->Step = STEP_LV5;
				wbt_init_timer( wk, TIMER_GAGE, FRAME_GAGE );
				//
				// ゲージ伸びる処理
				sbuf[0] = wbt_get_param( wk, PRM_LINEWORK, -1 );
				sbuf[1] = wbt_get_param( wk, PRM_PATTERN,  -1 );
				wbt_set_gageanim( wk, sbuf[0], 0, sbuf[1] );
				//
				// 反対側のゲージが灰色になる
				wbt_chg_gage_color( wk );
				//
				// 勝ったパネル演出
				tmp = wbt_get_param( wk, PRM_WINPOS, -1 );
				seq = R_WIN_SEQ;
				if(tmp < 4){
					seq = L_WIN_SEQ;
				}
				wbt_chg_objseq( wk, tmp, seq );
				// se(bar)
				PMSND_PlaySE( SND_WBT_LIST_BAR );
			}
			break;
		
		// LV5(ゲージ伸びアニメ待ち)
		case STEP_LV5:
			// タイマは動かしておく
			wbt_main_timer( wk, TIMER_GAGE );
			//
			// ゲージ処理
			sbuf[0] = wbt_get_param( wk, PRM_LINEWORK, -1 );
			sbuf[1] = wbt_get_param( wk, PRM_PATTERN,  -1 );
			if( wbt_is_gageanim( wk, sbuf[0] ) == FALSE ){
				//
				// ゲージ伸び終了
				wk->Step = STEP_LV5+10;
				wbt_set_gageanim( wk, sbuf[0], 1, sbuf[1] );
				//
				// 勝ったパネル演出終了
				// =>通常に戻す
				tmp = wbt_get_param( wk, PRM_WINPOS, -1 );
				seq = R_WHITE_SEQ;
				if(tmp < 4){
					seq = L_WHITE_SEQ;
				}
				wbt_chg_objseq( wk, tmp, seq );
				//
				// se(bar)の音を止める
				{
					SEPLAYER_ID	player_id;
					player_id = PMSND_GetSE_DefaultPlayerID( SND_WBT_LIST_BAR );
					PMSND_StopSE_byPlayerID( player_id ); 
				}
			}
			break;
		// ゲージ切り替え後待ち
		case STEP_LV5+10:
			timer = wbt_main_timer( wk, TIMER_GAGE );
			if(timer <= 0){
				// ゲージ伸び終了
				wk->Step = STEP_LV6;
				wbt_init_move_panel( wk, MOVE_PANEL_IN );
				// se(panel in)
				PMSND_PlaySE( SND_WBT_LIST_PANELIN );
			}
			break;
		
		// LV6(パネルイン)
		case STEP_LV6:
			// 初期化
			wbt_main_move_panel( wk );
			// 移動終了
			if( wbt_is_movework( wk, MWORK_PANEL_L ) == FALSE ){
				wk->Step = STEP_LV7;
				// メッセージ表示
				mesid = wbt_chk_meswin( wk );
				wbt_set_meswin( wk, mesid );
			}
			break;
		
		// LV7(メッセージ待ち)
		case STEP_LV7:
			// メッセージ待ち
			if( wbt_main_meswin( wk ) == FALSE ){
				//
				wk->Step = STEP_LV3;
				// 終了判定
				if( wbt_is_dispcount( wk ) == TRUE ){
					wk->Step = STEP_END;
				}
				// 繰り返し
				else{
					wbt_init_move_panel( wk, MOVE_PANEL_OUT );
					// meswinクリア
					wbt_clear_meswin( wk );
					// se(panel in)
					PMSND_PlaySE( SND_WBT_LIST_PANELOUT );
				}
			}
			break;
		
		// LV END
		case STEP_END:
			break;
		//
		default:
			break;
	}
	//
	// debug
	if(step != wk->Step){
#ifdef _DEBUG_LOCAL_
		OS_Printf("step :%d\n",wk->Step);
#endif
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理まとめ
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_key_all( WBTLIST_MAIN_WORK* wk )
{
	// 1つで大丈夫のようだ
	wbt_key_00( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイマ処理初期化
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32 tmode		タイマの種類
 *	@param	const s32 timer		フレーム数
 *
 *	@retval 
 */
//-----------------------------------------------------------------------------
static void wbt_init_timer( WBTLIST_MAIN_WORK* wk, const s32 tmode, const s32 timer )
{
	wk->Timer[tmode] = timer;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイマ処理
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32 mode		タイマの種類
 *
 *	@retval s32					時間
 */
//-----------------------------------------------------------------------------
static s32 wbt_main_timer( WBTLIST_MAIN_WORK* wk, const s32 tmode )
{
	wk->Timer[tmode]--;
	if(wk->Timer[tmode] <= 0){
		wk->Timer[tmode] = 0;
	}
	return( wk->Timer[tmode] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(現在、Canvasの作成)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_string( WBTLIST_MAIN_WORK* wk )
{
	s32		i;
	const WBTLIST_CANVAS_DATA	*adr = &wbt_canvas_tbl[0];
	
	for(i=0;i<WBT_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット
		wk->PrintUtil[i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x,y)
 *	@param	STRBUF* 			表示文字列
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_disp_string( WBTLIST_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, STRBUF* strbuf )
{
	// 表示
	PRINTTOOL_PrintColor(
				&wk->PrintUtil[mode],			// プリントユーティル ( BMPWIN )
				wk->print_que,					// プリントキュー
				x, y,							// 表示基準座標
				strbuf,							// 文字列
				wk->font,						// フォント
				PRINTSYS_LSB_Make(15,14,0),		// カラー
				PRINTTOOL_MODE_CENTER );		// 中央
	// 転送
	GFL_BMPWIN_MakeScreen( wk->PrintUtil[mode].win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[mode].win) );
	// 転送?
	PRINT_UTIL_Trans( &wk->PrintUtil[mode], wk->print_que );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示終了
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_exit_string( WBTLIST_MAIN_WORK* wk )
{
	s32		i;
	
	for(i=0;i<WBT_CANVAS_MAXNUM;i++){
		GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
	}
	// PrintStream後処理
	if(wk->MsgBuffer){
		GFL_STR_DeleteBuffer( wk->MsgBuffer );
	}
	if(wk->PrintStream){
		PRINTSYS_PrintStreamDelete( wk->PrintStream );
	}
	// 文字送り用
	APP_KEYCURSOR_Delete( wk->MsgCursorWork );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_obj( WBTLIST_MAIN_WORK* wk )
{
	s16			i;
	s16			buf[2];
    GFL_CLUNIT*	p_clunit = WBTLIST_GRAPHIC_GetClunit( wk->graphic );
	// リソース読み込み
	wbt_init_obj_res( wk, CLSYS_DRAW_MAIN, &obj_main_info_tbl[0] );
	// CLWK登録
	wbt_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &wbt_obj_tbl[0], &obj_main_info_tbl[0] );
	// BmpOAM初期化
	wk->BmpOamSys = BmpOam_Init( wk->heapID, p_clunit );
	// BMPOAM登録
	wbt_init_obj_bmpoam( wk, CLSYS_DRAW_MAIN, &wbt_bmpoam_tbl[0], &obj_main_info_tbl[0] );
	// マップモデル
	wbt_init_mapmodel( wk, &wbt_mapmodel_tbl[0] );
	//
	// パレット(すぐ取得でも大丈夫なようだ)
	wbt_get_obj_paladr( wk, WBT_TRAINER_MAX, PLTID_OBJ_MAPMODEL, CLSYS_DRAW_MAIN );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(res)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_obj_res( WBTLIST_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr )
{
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_WBTLIST_GRA, wk->heapID );
    GFL_CLUNIT*	p_clunit = WBTLIST_GRAPHIC_GetClunit( wk->graphic );
	const s16 *adr		 = info_adr;		// アドレスセット
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ adr[0+1] ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, adr[0], drawtype, adr[2], 0, adr[3], wk->heapID );
	wk->wk_obj.res[ adr[4+1] ] = GFL_CLGRP_CGR_Register(      p_handle, adr[4], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ adr[8+1] ] = GFL_CLGRP_CELLANIM_Register( p_handle, adr[8], adr[12], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const WBTLIST_OBJ_DATA* アドレス
 *	@param	const s16*				アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_obj_clwk( WBTLIST_MAIN_WORK* wk, const s32 drawtype, const WBTLIST_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16				tmp;
	s16				pos[2];
	s32 			i;
	BOOL			bflg;
	GFL_CLWK_DATA 	cldata;
	CLWK_FLIP_TYPE	flip;
    GFL_CLUNIT*		p_clunit 	= WBTLIST_GRAPHIC_GetClunit( wk->graphic );
	// アドレスセット
	const WBTLIST_OBJ_DATA *adr = obj_adr;
	const s16 *iadr 			= info_adr;
	
	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
	//
	for(i=0;i<WBT_OBJ_MAXNUM;i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		pos[0] = wbt_get_empty(wk, tmp*2);
		pos[1] = wbt_get_empty(wk, tmp*2+1);
		cldata.pos_x 	= pos[0];
		cldata.pos_y 	= pos[1];
		cldata.anmseq 	= adr->anmseq;
		cldata.softpri	= adr->softpri;
		cldata.bgpri	= adr->bgpri;
		//
		wk->wk_obj.clwk[i] =   GFL_CLACT_WK_Create( p_clunit, 
											wk->wk_obj.res[ iadr[4+1] ],			// chr
											wk->wk_obj.res[ iadr[0+1] ],			// plt
											wk->wk_obj.res[ iadr[8+1] ],			// cel
											&cldata,
											drawtype,
											wk->heapID );
		// パレット(意図的に変更しない限り、セットしなくてもいいようだ(しばらくは残し)
//		GFL_CLACT_WK_SetPlttOffs( wk->wk_obj.clwk[i], adr->palnum, CLWK_PLTTOFFS_MODE_PLTT_TOP );
		//-------------
		// 表示/非表示
		//-------------
		bflg = FALSE;
		if(adr->visible == 1){
			bflg = TRUE;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[i], bflg );
		//----------
		// フラグ
		//----------
		// アニメ
		bflg = FALSE;
		if(adr->dispflag & FLG_OBJ_ANIM){
			bflg = TRUE;
		}
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[i], bflg );
		//
		// Hフリップ
		if(adr->dispflag & FLG_OBJ_HFLIP){
			GFL_CLACT_WK_SetFlip( wk->wk_obj.clwk[i], CLWK_FLIP_H, TRUE );
		}
		// Vフリップ
		if(adr->dispflag & FLG_OBJ_VFLIP){
			GFL_CLACT_WK_SetFlip( wk->wk_obj.clwk[i], CLWK_FLIP_V, TRUE );
		}
		// 次のデータへ
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(bmpoam)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const WBTLIST_BMPOAM_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_obj_bmpoam( WBTLIST_MAIN_WORK* wk, const s32 drawtype, const WBTLIST_BMPOAM_DATA *bmp_adr, const s16 *info_adr )
{
	s16							tmp;
	s32							i;
	s32							buf[2];
	BMPOAM_ACT_DATA				actdata;
	const WBTLIST_BMPOAM_DATA	*adr  = bmp_adr;
	const s16					*iadr = info_adr;
	GFL_CLACTPOS				cpos;
	//
	for(i=0;i<WBT_BMPOAM_MAXNUM;i++){
		// 終了判定
		if(adr->width == 0xff){
			break;
		}
		// サイズ(colormodeは16で大丈夫だろう)
		buf[0] = (s32)(adr->width);
		buf[1] = (s32)(adr->height);
		wk->BmpData[i] = GFL_BMP_Create( buf[0], buf[1], GFL_BMP_16_COLOR, wk->heapID );
		// 左側
		tmp = adr->empty_num;
		cpos.x = wbt_get_empty( wk, tmp*2 );
		cpos.y = wbt_get_empty( wk, tmp*2+1 );
		//
		GFL_STD_MemClear( &actdata, sizeof(BMPOAM_ACT_DATA) );
		actdata.bmp			= wk->BmpData[i];
		actdata.x 			= cpos.x;
		actdata.y 			= cpos.y;
		actdata.pltt_index	= wk->wk_obj.res[ iadr[0+1] ];		// plt
		actdata.soft_pri	= adr->softpri;
		actdata.bg_pri		= adr->bgpri;
		actdata.pal_offset	= adr->paloffset;
		actdata.setSerface	= drawtype;
		actdata.draw_type	= drawtype;		// ?
		// 作成
		wk->BmpOamWork[i]	= BmpOam_ActorAdd( wk->BmpOamSys, &actdata );
		// visible
		if(adr->visible == 0){
			BmpOam_ActorSetDrawEnable( wk->BmpOamWork[i], FALSE );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_exit_obj( WBTLIST_MAIN_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<WBT_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk[i] );
	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_OBJ_CEL] );
	//
	// BmpOAM後処理
	for(i=0;i<WBT_BMPOAM_MAXNUM;i++){
		BmpOam_ActorDel( wk->BmpOamWork[i] );
		GFL_BMP_Delete( wk->BmpData[i] );
	}
	BmpOam_Exit( wk->BmpOamSys );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_chg_objseq( WBTLIST_MAIN_WORK *wk, const s32 wnum, const s32 seq )
{
	// 切り替え
	GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
	// いれても問題ないはず
	GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32		objワーク番号
 *	@param	const BOOL		(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_set_obj_visible( WBTLIST_MAIN_WORK *wk, const s32 wnum, const BOOL bflg )
{
	// 表示/非表示
	GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのアニメの終了待ち
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32		objワーク番号
 *
 *	@retval	const BOOL		(TRUE:再生中、FALSE:停止、終了)
 */
//-----------------------------------------------------------------------------
static BOOL wbt_is_anime_end( WBTLIST_MAIN_WORK *wk, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// 終了判定らしい
	if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[wnum] ) == FALSE ){
		bflg = FALSE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのパレット取得(標準パレットのみ)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32		パレット本数
 *	@param	const s32		パレットオフセット
 *	@param	const s32		上下
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_get_obj_paladr( WBTLIST_MAIN_WORK *wk, const s32 pnum, const s32 poff, const s32 drawtype )
{
	u32	padr = HW_OBJ_PLTT;
	
	if(drawtype == CLSYS_DRAW_SUB){
		padr += 0x400;
	}
	padr += poff*0x20;
	// コピー
	GFL_STD_MemCopy16( (void*)(padr) , wk->PalBuffer , pnum*0x20 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのパレット変更(vblank中にやってくれるらしい)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32		ワーク番号(0-7)(-1は全転送)
 *	@param	const s32		オフセット
 *	@param	const s32		(0:変更、1:戻し)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_chg_obj_pal( WBTLIST_MAIN_WORK *wk, const s32 wnum, const s32 poff, const s32 mode )
{
	u16		tmp;
	u16		buf[4];
	s32		i;
	u32		padr = HW_OBJ_PLTT;
	
	// 全戻し用
	if(wnum == -1){
		padr = 0;
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_OBJ_PLTT_MAIN, (padr+0x20*(poff)), &wk->PalBuffer[0], 0x20*8 );
		return;
	}
	
	// 16色分
	for(i=0;i<16;i++){
		// カラーを取得
		tmp = wk->PalBuffer[wnum*0x10+i];		// palbufferはu16のため
		//
		// カラーの平均を取得
		buf[0] = tmp&0x1f;			// R
		buf[1] = (tmp>>5)&0x1f;		// G
		buf[2] = (tmp>>10)&0x1f;	// B
		// 候補を調べる
		buf[3] = (buf[0]+buf[1]+buf[2])/3;
		// ここまででカラーの平均が出る
		wk->TmpBuffer[wnum*0x10+i] = (buf[3]<<10)+(buf[3]<<5)+buf[3];
	}
	// 転送
	if(mode == 0){		// グレースケールへ
		padr = 0;
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_OBJ_PLTT_MAIN, (padr+0x20*(wnum+poff)), &wk->TmpBuffer[wnum*0x10], 0x20 );
	}else{				// オリジナルカラーへ(debug用)
		padr = 0;
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_OBJ_PLTT_MAIN, (padr+0x20*(wnum+poff)), &wk->PalBuffer[wnum*0x10], 0x20 );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  移動初期化
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32			ワーク番号
 *	@param	const s16			始点、終点
 *	@param	const s16			フレーム数
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_movework( WBTLIST_MAIN_WORK* wk, const s32 wnum, const s16 sx, const s16 ex, const s16 frame )
{
	if(wnum >= MWORK_MAX){
#ifdef _DEBUG_LOCAL_
		OS_Printf("wnum ERROR:%d\n",wnum);
#endif
		return;
	}
	if(frame == 0){
#ifdef _DEBUG_LOCAL_
		OS_Printf("frame ERROR:%d\n",frame);
#endif
		return;
	}
	//
	wk->MoveWork[wnum].sx 		= sx;
	wk->MoveWork[wnum].ex		= ex;
	// 内部の計算は100倍しておく
	// シフトを使ってもよいがとりあえずは保留
	wk->MoveWork[wnum].rate		= (ex-sx)*100/frame;
	wk->MoveWork[wnum].calcnum	= sx*100;
	wk->MoveWork[wnum].flag		= FLG_MOVE_INIT;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  移動処理
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32			ワーク番号
 *
 *	@retval s16					座標(x)
 */
//-----------------------------------------------------------------------------
static s16 wbt_main_movework( WBTLIST_MAIN_WORK* wk, const s32 wnum )
{
	s16		flg;
	s16		tmp;
	s16		rate=0;
	
	// 移動中
	if(wk->MoveWork[wnum].flag & FLG_MOVE_INIT){
		if(!(wk->MoveWork[wnum].flag & FLG_MOVE_END)){
			// 移動
			wk->MoveWork[wnum].calcnum += wk->MoveWork[wnum].rate;
			// 1/100にして判定
			tmp = wk->MoveWork[wnum].calcnum/100;
			// 終了判定
			flg = 0;
			if(wk->MoveWork[wnum].rate < 0){
				flg = 1;
			}
			if(flg == 0){
				// 加算
				if(tmp >= wk->MoveWork[wnum].ex){
					rate = wk->MoveWork[wnum].ex;
					wk->MoveWork[wnum].flag |= FLG_MOVE_END;
				}
			}else{
				// 減算
				if(tmp <= wk->MoveWork[wnum].ex){
					rate = wk->MoveWork[wnum].ex;
					wk->MoveWork[wnum].flag |= FLG_MOVE_END;
				}
			}
			rate = tmp;
			
		}else{
			rate = wk->MoveWork[wnum].ex;
		}
	}
	return( rate );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  移動判定
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32			ワーク番号
 *
 *	@retval BOOL 				(移動中:TRUE、停止、未使用:FALSE)
 */
//-----------------------------------------------------------------------------
static BOOL wbt_is_movework( WBTLIST_MAIN_WORK* wk, const s32 wnum )
{
	BOOL	bflg=FALSE;
	//
	// 移動中
	if(wk->MoveWork[wnum].flag & FLG_MOVE_INIT){
		if(!(wk->MoveWork[wnum].flag & FLG_MOVE_END)){
			bflg = TRUE;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パネル移動初期化
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32 			(0:出る、1:引っ込む)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_move_panel( WBTLIST_MAIN_WORK* wk, const s32 mode )
{
	s32		buf[4];
	s32		rate=MOVE_IN_SPEED;
	//
	if(mode == MOVE_PANEL_IN){	// 出る
		buf[0] = wbt_move_pos_tbl[0];	// 左
		buf[1] = wbt_move_pos_tbl[1];
		buf[2] = wbt_move_pos_tbl[2];	// 右
		buf[3] = wbt_move_pos_tbl[3];
		rate = MOVE_IN_SPEED;
	}else{						// 引っ込む
		buf[0] = wbt_move_pos_tbl[1];	// 左
		buf[1] = wbt_move_pos_tbl[0];
		buf[2] = wbt_move_pos_tbl[3];	// 右
		buf[3] = wbt_move_pos_tbl[2];
		rate = MOVE_OUT_SPEED;
	}
	//
	// 移動のパラメータ計算は同時に行うが
	// 実際の移動はフラグを見て必要なものだけが移動
	//
	// 左側
	wbt_init_movework( wk, MWORK_PANEL_L, buf[0], buf[1], rate );
	// 右側
	wbt_init_movework( wk, MWORK_PANEL_R, buf[2], buf[3], rate );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パネル初期化
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32			ワーク番号
 *	@param	const s32 			(0:出る、1:引っ込む)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_move_panel_pos( WBTLIST_MAIN_WORK* wk, const s32 wnum, const s32 mode )
{
	s32				buf[4];
	GFL_CLACTPOS	pos;
	
	if(mode == MOVE_PANEL_IN){	// 出る
		buf[0] = wbt_move_pos_tbl[1];		// パネル
		buf[1] = wbt_move_pos_tbl[3];
		buf[2] = wbt_get_empty( wk, L_BMPOAM_EMPTY_ID*2 );		// bmpoam
		buf[3] = wbt_get_empty( wk, L_BMPOAM_EMPTY_ID*2+1 );
	}else{						// 引っ込む
		buf[0] = wbt_move_pos_tbl[0];		// パネル
		buf[1] = wbt_move_pos_tbl[2];
		buf[2] = wbt_get_empty( wk, R_BMPOAM_EMPTY_ID*2 );		// bmpoam
		buf[3] = wbt_get_empty( wk, R_BMPOAM_EMPTY_ID*2+1 );
	}
	//
	// 座標の取得
	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[CLWKID_PANEL_00A+wnum], &pos, CLSYS_DEFREND_MAIN );
	// 座標のセット(x方向のみ修正)
	pos.x = buf[1];
	if(wnum < 4){
		pos.x = buf[0];
	}
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[CLWKID_PANEL_00A+wnum], &pos, CLSYS_DEFREND_MAIN );
	// BmpOamの移動
	BmpOam_ActorSetPos( wk->BmpOamWork[wnum], (pos.x+buf[2]), (pos.y+buf[3]) );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パネル移動(要整理)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_main_move_panel( WBTLIST_MAIN_WORK* wk )
{
	s16				i;
	s16				px;
	s16				buf[2];
	GFL_CLACTPOS	pos[2];
	
	// 左側
	px = wbt_main_movework( wk, MWORK_PANEL_L );
	buf[0] = wbt_get_empty( wk, L_BMPOAM_EMPTY_ID*2 );		// bmpoam
	buf[1] = wbt_get_empty( wk, L_BMPOAM_EMPTY_ID*2+1 );
	for(i=0;i<4;i++){
		// 移動許可がない場合はそのまま
		if(!(wk->MoveFlag & (1<<i))){
			continue;
		}
		// 座標の取得
		GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[CLWKID_PANEL_00A+i], &pos[0], CLSYS_DEFREND_MAIN );
		// 座標のセット(x方向のみ修正)
		pos[0].x = px;
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[CLWKID_PANEL_00A+i], &pos[0], CLSYS_DEFREND_MAIN );
		//
		// BmpOamの移動
		BmpOam_ActorSetPos( wk->BmpOamWork[i], (pos[0].x+buf[0]), (pos[0].y+buf[1]) );
	}
	//
	// 右側
	px = wbt_main_movework( wk, MWORK_PANEL_R );
	buf[0] = wbt_get_empty( wk, R_BMPOAM_EMPTY_ID*2 );		// bmpoam
	buf[1] = wbt_get_empty( wk, R_BMPOAM_EMPTY_ID*2+1 );
	for(i=0;i<4;i++){
		// 移動許可がない場合はそのまま
		if(!(wk->MoveFlag & (1<<(4+i)))){
			continue;
		}
		// 座標の取得
		GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[CLWKID_PANEL_04A+i], &pos[1], CLSYS_DEFREND_MAIN );
		// 座標のセット(x方向のみ修正)
		pos[1].x = px;
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[CLWKID_PANEL_04A+i], &pos[1], CLSYS_DEFREND_MAIN );
		//
		// BmpOamの移動
		BmpOam_ActorSetPos( wk->BmpOamWork[4+i], (pos[1].x+buf[0]), (pos[1].y+buf[1]) );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ(セット)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32			メッセージid
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_set_meswin( WBTLIST_MAIN_WORK* wk, const s32 mesid )
{
	s16			flg;
	s16			buf[2];
	WORDSET		*wordset;
	STRBUF		*strbuf;
	
	// WORDSET生成
	wordset = WORDSET_Create( wk->heapID );
	//
	// 1回戦の1,2回目だけ特別処理
	flg = 0;
	if((wk->MesFlag & FLG_MES_EX1)||(wk->MesFlag & FLG_MES_EX2)){
		flg = 1;
	}
	if(wk->MesFlag & FLG_MES_LAST){
		flg = 2;
	}
	//
	if((wk->Mode == MODE_ROUND1)&&(flg == 1)){
		// トーナメント名、ルール名
		WORDSET_RegisterWord( wordset, 0, wk->Param->tournament_name, 0, TRUE, PM_LANG );
		WORDSET_RegisterWord( wordset, 1, wk->Param->tournament_rule, 0, TRUE, PM_LANG );
	}else{
		// 各トレーナーの名前をセット
		buf[0] = wbt_get_param( wk, PRM_T1ST, 0 );		// 左
		buf[1] = wbt_get_param( wk, PRM_T2ND, 0 );		// 右
		// 最後は勝者を表示
		if(wk->MesFlag & FLG_MES_END){
			buf[0] = wbt_get_param( wk, PRM_WINPOS, 0 );
		}
		if(flg == 2){
			// トーナメント名、ルール名
			WORDSET_RegisterWord( wordset, 0, wk->Param->tournament_name, 0, TRUE, PM_LANG );
			WORDSET_RegisterWord( wordset, 1, wk->Param->tournament_rule, 0, TRUE, PM_LANG );	// なくてもよいはず
		}else{
			WORDSET_RegisterWord( wordset, 0, wk->Param->trainer[ buf[0] ].name, 0, TRUE, PM_LANG );
			WORDSET_RegisterWord( wordset, 1, wk->Param->trainer[ buf[1] ].name, 0, TRUE, PM_LANG );
		}
	}
	
	// PrintStream終了時にはdeleteしないとtaskが残ってしまうようだ
	// 念のため0check
	if( wk->PrintStream ){
		PRINTSYS_PrintStreamDelete( wk->PrintStream );
	}
	// Msgの再構築
	if(wk->MsgBuffer){
		GFL_STR_DeleteBuffer( wk->MsgBuffer );
		wk->MsgBuffer = GFL_STR_CreateBuffer( MES_STRING_BUFFER, wk->heapID );
	}
	// メッセージ仮バッファセット
	strbuf = GFL_MSG_CreateString( wk->msg[0], mesid );
	// パラメータを展開しMsgBufferに再セット
	WORDSET_ExpandStr( wordset, wk->MsgBuffer, strbuf );
	//
	// stream設定=>ここのwk->MsgBufferは終わるまで削除してはいけない
	wk->PrintStream = PRINTSYS_PrintStream(
				wk->PrintUtil[2].win, 0, 0, wk->MsgBuffer, wk->font,
				MSGSPEED_GetWait(), wk->MsgTcblSys, 4, wk->heapID, MES_CLEAR_COLOR );
	// スクリーン転送らしい
	GFL_BMPWIN_MakeTransWindow( wk->PrintUtil[2].win );
	
	// WORDSET破棄
	WORDSET_Delete( wordset );
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ(処理)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval	BOOL				(TRUE:再生中、FALSE:終了)
 */
//-----------------------------------------------------------------------------
static BOOL wbt_main_meswin( WBTLIST_MAIN_WORK* wk )
{
	u16		tmp;
	u32		num;
	BOOL	bflg;
	
	// 念のため
	if(wk->PrintStream == 0){
		return( FALSE );
	}
	//
	// 文字送りカーソル処理
	APP_KEYCURSOR_Main( wk->MsgCursorWork, wk->PrintStream, wk->PrintUtil[2].win );
	//
	// streamの入力処理をまとめて処理してくれるらしい
    bflg = APP_PRINTSYS_COMMON_PrintStreamFunc( &wk->PrintSysWork , wk->PrintStream );
    if( bflg == TRUE ){
		// 終了した
		// 2回目が終了=>もれなく終了
		if(wk->MesFlag & FLG_MES_1ST){
			return( FALSE );
		}
		// 1回目が終了=>2回目がある
		wk->MesFlag |= FLG_MES_1ST;
		wk->MesFlag |= FLG_MES_END;
		// meswinクリア
		wbt_clear_meswin( wk );
		// 指定した番号 + (0-(MES_RANDOM_MAX-1))のランダム
//		num = GFUser_GetPublicRand( MES_RANDOM_MAX );
		num = wk->RandBuffer[wk->RandCount];
		wk->RandCount++;
		// 念のため
		if(wk->RandCount >= 3){
			wk->RandCount = 3;
		}
		OS_Printf("rand :%d\n",num);
		//
		tmp = wbt_get_param( wk, PRM_MESID, 0 );
		wbt_set_meswin( wk, (tmp+num) );
		return( TRUE );
	}
	return( TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウフラグチェック
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval s32					mesid
 */
//-----------------------------------------------------------------------------
static s32 wbt_chk_meswin( WBTLIST_MAIN_WORK* wk )
{
	s32		mesid;
	
	// 前半
	mesid = wbt_mes_gmm_tbl[4*wk->Mode];
	
	// メッセージの最後のときは、
	// 次の回戦の情報を表示させるための処理
	wk->MesFlag = 0;
	if(wk->Mode == MODE_ROUND1){		// 1回戦
		// 1回戦だけ特別(1文章のみ表示)
		wk->MesFlag = FLG_MES_1ST;
		if(wk->DispCount == 0){
			// 1回目 => 「トーナメント名、ルール」
			mesid = wbt_mes_gmm_tbl[4*wk->Mode+2];
			wk->MesFlag |= FLG_MES_EX1;
		}
		else if(wk->DispCount == 1){
			// 2回目 => 「決めセリフ」
			mesid = wbt_mes_gmm_tbl[4*wk->Mode+3];
			wk->MesFlag |= FLG_MES_EX2;
		}
		else if(wk->DispCount == (PARAM_ROUND1-1)){
			mesid = wbt_mes_gmm_tbl[4*wk->Mode+1];
			wk->MesFlag |= FLG_MES_LAST;
		}
	}
	else if(wk->Mode == MODE_ROUND2){	// 2回戦
		if(wk->DispCount == (PARAM_ROUND2-1)){
			wk->MesFlag = FLG_MES_1ST;
			mesid = wbt_mes_gmm_tbl[4*wk->Mode+1];
			wk->MesFlag |= FLG_MES_LAST;
		}
	}
	else if(wk->Mode == MODE_ROUND3){	// 3回戦
		if(wk->DispCount == (PARAM_ROUND3-1)){
			wk->MesFlag = FLG_MES_1ST;
			mesid = wbt_mes_gmm_tbl[4*wk->Mode+1];
			wk->MesFlag |= FLG_MES_LAST;
		}
	}
	return( mesid );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウクリア
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_clear_meswin( WBTLIST_MAIN_WORK* wk )
{
	// clear
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[2].win ), MES_CLEAR_COLOR );
	GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[2].win );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  emptyの取得(使い方注意)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
static s16 wbt_get_empty( WBTLIST_MAIN_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= WBT_EMPTY_MAXNUM)){
		OS_Printf("empty error :%d\n",num);
		return(0);
	}
	
	adr = wk->EmptyBuffer;
	return( adr[num] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ゲージアニメの切り替え
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32		アニメ番号(0-3)
 *	@param	const s32		mode(0:伸びアニメ(後アニメ)、1:伸び後アニメ)
 *	@param	const s32		umode(0:上、1:下)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_set_gageanim( WBTLIST_MAIN_WORK *wk, const s32 wnum, const s32 mode, const s32 umode )
{
	s16		buf[2];
	s32		clwnum = CLWKID_BAR_00+wnum;
	GFL_CLACTPOS	pos;
	u16		anum;
	
	// アニメのシーケンスセット
	anum = wbt_bar_anim_tbl[wnum*8+umode*4+2+mode];
//	u16		anum = GFL_CLACT_WK_GetAnmSeq( wk->wk_obj.clwk[clwnum] );
	GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[clwnum], anum );
	//
	GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[clwnum], TRUE );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[clwnum], TRUE );
	
	// empty番号
	buf[0] = wbt_bar_anim_tbl[wnum*8+umode*4];
	// フリップ情報
	buf[1] = wbt_bar_anim_tbl[wnum*8+umode*4+1];
	
	// Hフリップ
	if(buf[1] & FLG_OBJ_HFLIP){
		GFL_CLACT_WK_SetFlip( wk->wk_obj.clwk[clwnum], CLWK_FLIP_H, TRUE );
	}else{
		GFL_CLACT_WK_SetFlip( wk->wk_obj.clwk[clwnum], CLWK_FLIP_H, FALSE );
	}
	// Vフリップ
	if(buf[1] & FLG_OBJ_VFLIP){
		GFL_CLACT_WK_SetFlip( wk->wk_obj.clwk[clwnum], CLWK_FLIP_V, TRUE );
	}else{
		GFL_CLACT_WK_SetFlip( wk->wk_obj.clwk[clwnum], CLWK_FLIP_V, FALSE );
	}
	// 左側
	pos.x = wbt_get_empty( wk, buf[0]*2 );
	pos.y = wbt_get_empty( wk, buf[0]*2+1 );
	//
//	OS_Printf("wnum, mode, umode,clwnum:%d,%d,%d,%d\n",wnum,mode,umode,clwnum);
//	OS_Printf("anum, buf[0], buf[1], pos.x, pos.y:%d,%d,%d,%d,%d\n",anum,buf[0],buf[1],pos.x,pos.y);
	
	// 座標セット
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[clwnum], &pos, CLSYS_DEFREND_MAIN );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ゲージアニメの終了待ち
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32		アニメ番号(0-3,4,5)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL wbt_is_gageanim( WBTLIST_MAIN_WORK *wk, const s32 wnum )
{
	BOOL	bflg = wbt_is_anime_end( wk, CLWKID_BAR_00+wnum );
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  爆発アニメ
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32		表示場所(0-7)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_disp_bomb( WBTLIST_MAIN_WORK* wk, const s32 num )
{
	u16				anum;
	GFL_CLACTPOS	pos;
	
	// 左側
	pos.x = wbt_get_empty( wk, (BOMB_EMPTY_ID+num)*2 );
	pos.y = wbt_get_empty( wk, (BOMB_EMPTY_ID+num)*2+1 );
	// 座標セット
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[CLWKID_BOMB], &pos, CLSYS_DEFREND_MAIN );
	
	// 再度アニメを再生するときは、
	// SetAnimSeqとAutoAnmFlagをする必要があるようだ
	anum = GFL_CLACT_WK_GetAnmSeq( wk->wk_obj.clwk[CLWKID_BOMB] );
	// 表示
	GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[CLWKID_BOMB], anum );
	//
	GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[CLWKID_BOMB], TRUE );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[CLWKID_BOMB], TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ゲージの色を落とす(シーケンスの変更)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_chg_gage_color( WBTLIST_MAIN_WORK* wk )
{
	s16		buf[4];
	
	// 3回戦目以外は処理しない
	if(wk->Mode == MODE_ROUND3){
		// ゲージ伸びる番号を取得
		buf[0] = wbt_get_param( wk, PRM_LINEWORK, -1 );
		buf[1] = wbt_get_param( wk, PRM_PATTERN,  -1 );
		//
		// 灰色にする番号を求める
		if(buf[0] == 4){		// 左側
			if(buf[1] == 0){	// 上側->対象は下側
				buf[2] = 1;
			}else{				// 下側->対象は上側
				buf[2] = 0;
			}
		}else{
			if(buf[1] == 0){	// 上側->対象は下側
				buf[2] = 3;
			}else{				// 下側->対象は上側
				buf[2] = 2;
			}
		}
		// ゲージのシーケンスを変更
		wbt_chg_objseq( wk, CLWKID_BAR_00+buf[2], GAGE_GRAY_SEQ );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  初期状態にする
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_mode( WBTLIST_MAIN_WORK* wk )
{
	s16			buf[2];
	s32			i;
	s32			tmp;
	
	// 2回戦
	if(wk->Mode == MODE_ROUND2){
		// パネル色戻し
		for(i=0;i<8;i++){
			tmp = R_WHITE_SEQ;
			if(i < 4){
				tmp = L_WHITE_SEQ;
			}
			wbt_chg_objseq( wk, i, tmp );
		}
		// キャラのパレット変更(戻し)=>なくてよいはず
		wbt_chg_obj_pal( wk, -1, PLTID_OBJ_MAPMODEL, 1 );
		
		// 全バー非表示
		for(i=0;i<WBT_GAGE_MAXNUM;i++){
			wbt_set_obj_visible( wk, CLWKID_BAR_00+i, FALSE );
		}
		// 全パネルを閉まった状態にする
		for(i=0;i<WBT_TRAINER_MAX;i++){
			wbt_init_move_panel_pos( wk ,i, MOVE_PANEL_OUT );
		}
	}
	// 3回戦
	else if(wk->Mode == MODE_ROUND3){
		// パネル色を白に戻し
		for(i=0;i<8;i++){
			tmp = R_WHITE_SEQ;
			if(i < 4){
				tmp = L_WHITE_SEQ;
			}
			wbt_chg_objseq( wk, i, tmp );
		}
		// キャラのパレット変更=>なくてよいはず
		wbt_chg_obj_pal( wk, -1, PLTID_OBJ_MAPMODEL, 1 );
		
		//
		// 2回戦までのパネル色戻し
		for(i=0;i<4;i++){
			buf[0] = wk->ParamWork[PARAM_ROUND1+i].exppos;		// 爆発パターン
			tmp = R_GRAY_SEQ;
			if(i < 2){
				tmp = L_GRAY_SEQ;
			}
			wbt_chg_objseq( wk, buf[0], tmp );
			// キャラのパレット変更
			wbt_chg_obj_pal( wk, buf[0], PLTID_OBJ_MAPMODEL, 0 );
		}
		//
		// 全バー非表示
		for(i=0;i<WBT_GAGE_MAXNUM;i++){
			wbt_set_obj_visible( wk, CLWKID_BAR_00+i, FALSE );
		}
		// 2回戦までのバー再現
		for(i=0;i<4;i++){
			buf[0] = wk->ParamWork[PARAM_ROUND1+i].linework;	// ラインバッファ
			buf[1] = wk->ParamWork[PARAM_ROUND1+i].pattern;		// パターン
			wbt_set_gageanim( wk, buf[0], 1, buf[1] );
		}
		
		// 全パネルを閉まった状態にする
		for(i=0;i<WBT_TRAINER_MAX;i++){
			wbt_init_move_panel_pos( wk ,i, MOVE_PANEL_OUT );
		}
	}
	// 1回戦
	else{
		// debug用に戻せるようにしておく
		// パネル色戻し
		for(i=0;i<8;i++){
			tmp = R_WHITE_SEQ;
			if(i < 4){
				tmp = L_WHITE_SEQ;
			}
			wbt_chg_objseq( wk, i, tmp );
		}
		// キャラのパレット変更=>なくてよいはず
		wbt_chg_obj_pal( wk, -1, PLTID_OBJ_MAPMODEL, 1 );
		
		// 全バー非表示
		for(i=0;i<WBT_GAGE_MAXNUM;i++){
			wbt_set_obj_visible( wk, CLWKID_BAR_00+i, FALSE );
		}
		//
		// 全パネルを閉まった状態にする
		for(i=0;i<WBT_TRAINER_MAX;i++){
			wbt_init_move_panel_pos( wk ,i, MOVE_PANEL_OUT );
		}
	}
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータ初期化(ここでパラメータを参照)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_param( WBTLIST_MAIN_WORK* wk )
{
	s16		i;
	s16		flg=0;
	s16		seq;
	s16		poff;
	
	// 性別を見てパネルの色(seq)変更
	for(i=0;i<WBT_TRAINER_MAX;i++){
		// 女性以外は青にしておく
		if(i < 4){
			// 左
			seq = L_BLUE_SEQ;
			if( wk->Param->trainer[i].sex == PM_FEMALE ){
				seq = L_RED_SEQ;
			}
		}else{
			// 右
			seq = R_BLUE_SEQ;
			if( wk->Param->trainer[i].sex == PM_FEMALE ){
				seq = R_RED_SEQ;
			}
		}
		wbt_chg_objseq( wk, CLWKID_PANEL_00A+i, seq );
		//
		// パレット変更
		poff = 0;
		if(wk->Param->trainer[i].sex == PM_FEMALE){
			poff = 1;
		}
		BmpOam_ActorSetPaletteOffset( wk->BmpOamWork[i], poff );
		//
		// プレイヤの番号を調べる
		if(wk->Param->trainer[i].is_player == 1){
			if(flg == 0){
				flg = 1;
				wk->PlayerNum = i;
			}
		}
	}
	//
	// パラメータ計算
	wbt_calc_param( wk );
	// 移動パラメータセット
	wk->MoveFlag = wbt_get_param( wk, PRM_MOVEFLAG, 0 );
	//
	// ランダム取得
	wbt_calc_random( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータセット(初期化時に1度)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_calc_param( WBTLIST_MAIN_WORK* wk )
{
	s32			offset;
	s32			i;
	//
	// 初期化
	for(i=0;i<PARAM_WORK_MAX;i++){
		wk->ParamWork[i].exppos		= 0;
		wk->ParamWork[i].winpos		= 0;
		wk->ParamWork[i].moveflag	= 0xff;
		wk->ParamWork[i].linework	= 0;
		wk->ParamWork[i].pattern	= 0;
		wk->ParamWork[i].mesid		= 0;
		wk->ParamWork[i].t_1st		= 0;
		wk->ParamWork[i].t_2nd		= 0;
	}
	// 1回戦 => 上記の初期化でよい
	if(wk->Mode == MODE_ROUND1){
		// 0番目はパネルを出さない
		// 「決めセリフまち」
		offset = 0;
		wk->ParamWork[offset].moveflag	= 0;
		//
		// 1番目はパネルを全部出すので初期化のまま(0xff)でよい
		//
		// 2番目はプレイヤの対戦表示
		offset = 2;
		if((wk->PlayerNum%2) == 0){		// 上側
			wk->ParamWork[offset].t_1st	= wk->PlayerNum;
			wk->ParamWork[offset].t_2nd	= wk->PlayerNum+1;
			//
			// 特殊処理(自分の対戦プレートだけ残す)
			wk->ParamWork[offset].moveflag  = 0xff;
			wk->ParamWork[offset].moveflag &= ~(1<<(wk->PlayerNum));
			wk->ParamWork[offset].moveflag &= ~(1<<(wk->PlayerNum+1));
		}else{							// 下側
			wk->ParamWork[offset].t_1st	= wk->PlayerNum-1;
			wk->ParamWork[offset].t_2nd	= wk->PlayerNum;
			//
			// 特殊処理(自分の対戦プレートだけ残す)
			wk->ParamWork[offset].moveflag  = 0xff;
			wk->ParamWork[offset].moveflag &= ~(1<<(wk->PlayerNum-1));
			wk->ParamWork[offset].moveflag &= ~(1<<(wk->PlayerNum));
		}
//		return;
	}
	// 2回戦+3回戦
	if((wk->Mode == MODE_ROUND2)||(wk->Mode == MODE_ROUND3)){
		offset = PARAM_ROUND1;
		for(i=0;i<4;i++){
			// 勝敗チェック
			if(wk->Param->first_stage[i].upside_win == TRUE){
				wk->ParamWork[offset+i].winpos	= 2*i;		// 勝った番号
				wk->ParamWork[offset+i].exppos	= 2*i+1;	// 負けた番号
				wk->ParamWork[offset+i].pattern = 0;		// ゲージのパターン
			}else{
				wk->ParamWork[offset+i].winpos	= 2*i+1;
				wk->ParamWork[offset+i].exppos	= 2*i;
				wk->ParamWork[offset+i].pattern = 1;
			}
			// 移動可能フラグ
			wk->ParamWork[offset+i].moveflag  = (1<<(2*i));
			wk->ParamWork[offset+i].moveflag |= (1<<(2*i+1));
			// ライン番号
			wk->ParamWork[offset+i].linework = i;
			// メッセージid
			wk->ParamWork[offset+i].mesid = wk->Param->first_stage[i].msg_type;
			//
			wk->ParamWork[offset+i].t_1st = 2*i;
			wk->ParamWork[offset+i].t_2nd = 2*i+1;
		}
		// 最後に自分側の移動可能フラグをセットする
		offset = PARAM_ROUND1+4;
		if(wk->PlayerNum < 4){
			wk->ParamWork[offset].moveflag  = (1<<(wk->ParamWork[PARAM_ROUND1].winpos));
			wk->ParamWork[offset].moveflag |= (1<<(wk->ParamWork[PARAM_ROUND1+1].winpos));
			// 表示用
			wk->ParamWork[offset].t_1st = wk->ParamWork[PARAM_ROUND1].winpos;
			wk->ParamWork[offset].t_2nd = wk->ParamWork[PARAM_ROUND1+1].winpos;
		}else{
			wk->ParamWork[offset].moveflag  = (1<<(wk->ParamWork[PARAM_ROUND1+2].winpos));
			wk->ParamWork[offset].moveflag |= (1<<(wk->ParamWork[PARAM_ROUND1+3].winpos));
			// 表示用
			wk->ParamWork[offset].t_1st = wk->ParamWork[PARAM_ROUND1+2].winpos;
			wk->ParamWork[offset].t_2nd = wk->ParamWork[PARAM_ROUND1+3].winpos;
		}
		// 2回戦
		wk->ParamWork[offset].mesid = wbt_mes_gmm_tbl[4*1+1];
	}
	// 3回戦
	if(wk->Mode == MODE_ROUND3){
		offset = PARAM_ROUND1+PARAM_ROUND2;
		for(i=0;i<2;i++){
			if( wk->Param->second_stage[i].upside_win == TRUE ){
				// 情報は2回戦での勝った番号から調べる
				wk->ParamWork[offset+i].winpos	= wk->ParamWork[PARAM_ROUND1+2*i].winpos;
				wk->ParamWork[offset+i].exppos	= wk->ParamWork[PARAM_ROUND1+2*i+1].winpos;
				wk->ParamWork[offset+i].pattern = 0;		// ゲージのパターン
			}else{
				wk->ParamWork[offset+i].winpos	= wk->ParamWork[PARAM_ROUND1+2*i+1].winpos;
				wk->ParamWork[offset+i].exppos	= wk->ParamWork[PARAM_ROUND1+2*i].winpos;
				wk->ParamWork[offset+i].pattern = 1;		// ゲージのパターン
			}
			// 移動可能フラグ
			wk->ParamWork[offset+i].moveflag  = (1<<(wk->ParamWork[PARAM_ROUND1+2*i].winpos));
			wk->ParamWork[offset+i].moveflag |= (1<<(wk->ParamWork[PARAM_ROUND1+2*i+1].winpos));
			// ライン番号
			wk->ParamWork[offset+i].linework = 4+i;
			// メッセージid
			wk->ParamWork[offset+i].mesid = wk->Param->second_stage[i].msg_type;
			//
			wk->ParamWork[offset+i].t_1st = wk->ParamWork[PARAM_ROUND1+2*i].winpos;
			wk->ParamWork[offset+i].t_2nd = wk->ParamWork[PARAM_ROUND1+2*i+1].winpos;
		}
		// 最後に自分側の移動可能フラグをセットする
		// =>残りが2なのでもれなく出せばよい
		offset = PARAM_ROUND1+PARAM_ROUND2+2;
		wk->ParamWork[offset].moveflag  = (1<<(wk->ParamWork[PARAM_ROUND1+PARAM_ROUND2].winpos));
		wk->ParamWork[offset].moveflag |= (1<<(wk->ParamWork[PARAM_ROUND1+PARAM_ROUND2+1].winpos));
		// 3回戦
		wk->ParamWork[offset].mesid = wbt_mes_gmm_tbl[4*2+1];
		//
		wk->ParamWork[offset].t_1st = wk->ParamWork[PARAM_ROUND1+PARAM_ROUND2].winpos;
		wk->ParamWork[offset].t_2nd = wk->ParamWork[PARAM_ROUND1+PARAM_ROUND2+1].winpos;
	}
	
#ifdef _DEBUG_LOCAL_
	// 表示
	for(i=0;i<PARAM_WORK_MAX;i++){
		OS_Printf("bomb,win,flag,linework,pattern,mesid,1st,2nd:(%d),%d,%d,%x,%d,%d,%d,%d,%d\n",
			i,
			wk->ParamWork[i].exppos,
			wk->ParamWork[i].winpos,
			wk->ParamWork[i].moveflag,
			wk->ParamWork[i].linework,
			wk->ParamWork[i].pattern,
			wk->ParamWork[i].mesid,
			wk->ParamWork[i].t_1st,
			wk->ParamWork[i].t_2nd );
	}
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータ取得
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s16 wbt_get_param( WBTLIST_MAIN_WORK* wk, const s32 mode, const s32 off )
{
	s16		tmp = 0;
	s32		offset=off;		// 加算
	
	if(wk->Mode == MODE_ROUND2){
		offset += (PARAM_ROUND1 + wk->DispCount);
		if(offset >= (PARAM_ROUND1+PARAM_ROUND2)){
			offset = PARAM_ROUND1+PARAM_ROUND2-1;
		}
	}
	else if(wk->Mode == MODE_ROUND3){
		offset += (PARAM_ROUND1 + PARAM_ROUND2 + wk->DispCount);
		if(offset >= (PARAM_ROUND1+PARAM_ROUND2+PARAM_ROUND3)){
			offset = PARAM_ROUND1+PARAM_ROUND2+PARAM_ROUND3-1;
		}
	}
	else if(wk->Mode == MODE_ROUND1){
		offset += wk->DispCount;
		if(offset >= (PARAM_ROUND1)){
			offset = PARAM_ROUND1-1;
		}
	}
	//
	// 取得
	if(mode == PRM_EXPPOS){
		tmp = wk->ParamWork[offset].exppos;
	}else if(mode == PRM_WINPOS){
		tmp = wk->ParamWork[offset].winpos;
	}else if(mode == PRM_MOVEFLAG){
		tmp = wk->ParamWork[offset].moveflag;
	}else if(mode == PRM_LINEWORK){
		tmp = wk->ParamWork[offset].linework;
	}else if(mode == PRM_PATTERN){
		tmp = wk->ParamWork[offset].pattern;
	}else if(mode == PRM_MESID){
		tmp = wk->ParamWork[offset].mesid;
	}else if(mode == PRM_T1ST){
		tmp = wk->ParamWork[offset].t_1st;
	}else if(mode == PRM_T2ND){
		tmp = wk->ParamWork[offset].t_2nd;
	}
	return( tmp );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  表示回数の処理(ここで加算)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL wbt_chk_dispcount( WBTLIST_MAIN_WORK* wk )
{
	BOOL	bflg=FALSE;
	
	wk->DispCount++;
	// 2回戦
	if(wk->Mode == MODE_ROUND2){
		if(wk->DispCount >= (PARAM_ROUND2-1)){
			wk->DispCount = (PARAM_ROUND2-1);
			bflg = TRUE;
		}
	}
	// 3回戦
	else if(wk->Mode == MODE_ROUND3){
		if(wk->DispCount >= (PARAM_ROUND3-1)){
			wk->DispCount = (PARAM_ROUND3-1);
			bflg = TRUE;
		}
	}
	// 1回戦
	else if(wk->Mode == MODE_ROUND1){
		if(wk->DispCount >= (PARAM_ROUND1-1)){
			wk->DispCount = PARAM_ROUND1-1;
			bflg = TRUE;
		}
	}
	// 終了判定
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  表示回数の処理(終了判定)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL wbt_is_dispcount( WBTLIST_MAIN_WORK* wk )
{
	BOOL	bflg=FALSE;
	
	// 2回戦
	if(wk->Mode == MODE_ROUND2){
		if(wk->DispCount >= (PARAM_ROUND2-1)){
			bflg = TRUE;
		}
	}
	// 3回戦
	else if(wk->Mode == MODE_ROUND3){
		if(wk->DispCount >= (PARAM_ROUND3-1)){
			bflg = TRUE;
		}
	}
	// 1回戦
	else if(wk->Mode == MODE_ROUND1){
		if(wk->DispCount >= (PARAM_ROUND1-1)){
			bflg = TRUE;
		}
	}
	// 終了判定
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  乱数の作成
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_calc_random( WBTLIST_MAIN_WORK* wk )
{
	s32		tmp;
	s32		i;
	s32		j;
	s32		k;
	s32		flg;
	//
	k=0;
	// 多めの回数分処理(こないはず)
	for(i=0;i<128;i++){
		flg = 0;
		// (0-4)のランダムをセット
		tmp = GFUser_GetPublicRand( MES_RANDOM_MAX );
		// 同じものがあるかチェック
		for(j=0;j<4;j++){
			if(tmp == wk->RandBuffer[j]){
				flg = 1;
				break;
			}
		}
		// ここまでで同じものがあるかがわかるので、
		// ない場合は要素に追加
		if(flg == 0){
			wk->RandBuffer[k] = tmp;
			k++;
			if(k >= 4){
				break;
			}
		}
	}
	// どうしても見つからなかった
	// (こないはず)
	if(i == 128){
		for(i=0;i<4;i++){
			wk->RandBuffer[i] = i;
		}
		OS_Printf("rand not found\n");
	}
	// ここまででRandBuffer[]に、値がもとまる
	OS_Printf("rand :%d, %d, %d, %d\n", wk->RandBuffer[0], wk->RandBuffer[1], wk->RandBuffer[2], wk->RandBuffer[3] );
	//
	wk->RandCount = 0;
}


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータチェック(debug)
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_debug_param( WBTLIST_MAIN_WORK* wk )
{
#ifdef PM_DEBUG
	
#ifdef _DEBUG_LOCAL_
	s16			i;
	
	OS_Printf( "stage :%d\n",wk->Param->stage );
	for(i=0;i<WBT_TRAINER_NUM;i++){
		OS_Printf( "id,pattern,sex,prio,poketype (%d):%d,%d,%d,%d,%d,%d\n",
					i,wk->Param->trainer[i].mmdl_id, wk->Param->trainer[i].pattern_no, 
					  wk->Param->trainer[i].sex, wk->Param->trainer[i].priority, wk->Param->trainer[i].poketype, wk->Param->trainer[i].is_player);
	}
	//
	for(i=0;i<4;i++){
		OS_Printf("1st :upsidewin,msgtype (%d):%d,%d\n",
					i,wk->Param->first_stage[i].upside_win,wk->Param->first_stage[i].msg_type);
	}
	//
	for(i=0;i<2;i++){
		OS_Printf("2nd :upsidewin,msgtype (%d):%d,%d\n",
					i,wk->Param->second_stage[i].upside_win,wk->Param->second_stage[i].msg_type);
	}
#endif		// _DEBUG_LOCAL_

#endif		// PM_DEBUG
}

//-----------------------------------------------------------------------------
/**
 *	@brief  デバッグ
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_debug( WBTLIST_MAIN_WORK* wk )
{
#ifdef PM_DEBUG
	s32		i;
	s32		tmp=0;
	
	// debug input
	if(wk->DebugFlag & FLG_DBG_INPUT){
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
			wk->Step = STEP_INIT;
			// 表示回数
			wk->DispCount = 0;
			//
			wbt_init_param( wk );
			OS_Printf("debug init \n");
		}
	}
	// debug mode(color)
	if(wk->DebugFlag & FLG_DBG_MODE){
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
			// 色変更
			if(wk->DebugFlag & FLG_DBG_COLOR){
				wk->DebugFlag &= ~FLG_DBG_COLOR;
				tmp = 0;
			}else{
				wk->DebugFlag |= FLG_DBG_COLOR;
				tmp = 1;
			}
			for(i=0;i<8;i++){
				wbt_chg_obj_pal( wk, i, PLTID_OBJ_MAPMODEL, tmp );
			}
		}
	}
#ifdef _DEBUG_SOUND_
	// sound
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){		// マイナス
		wk->DebugSound--;
		if(wk->DebugSound < SEQ_SE_SW_WBT_01){
			wk->DebugSound = SEQ_SE_SW_WBT_01;
		}
		OS_Printf("se :%d\n",wk->DebugSound);
	}
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){	// プラス
		wk->DebugSound++;
		if(wk->DebugSound >= SEQ_SE_SW_WBT_07){
			wk->DebugSound = SEQ_SE_SW_WBT_07;
		}
		OS_Printf("se :%d\n",wk->DebugSound);
	}
	// X
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		PMSND_PlaySE( wk->DebugSound );
	}
	
#endif		// _DEBUG_SOUND_

#endif		// PM_DEBUG

}

