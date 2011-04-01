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

//アーカイブ
#include "arc_def.h"

#include "system/wipe.h"

//外部公開
#include "app/wbt_list.h"


//@TODO BG読み込み とりあえずマイクテストのリソース
#include "message.naix"
#include "wbt_list_gra.naix"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
// debug表示が必要なときはセット
#define _DEBUG_LOCAL_
//#undef  _DEBUG_LOCAL_

#define _DEBUG_PARAM_
#undef  _DEBUG_PARAM_


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
	BG_FRAME_MIDDLE2_M	= GFL_BG_FRAME1_M,		// テキスト(BG1)(未使用)
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
	PLTID_BG_BACK_M				= 0,
	//サブBG
	PLTID_BG_BACK_S				= 0,		// 下地
	
	//メインOBJ
	PLTID_OBJ_PANEL				= 0,		// 3本使用
	PLTID_OBJ_OAM_MAPMODEL_M	= 6,		// 8本使用
	//サブOBJ
	PLTID_OBJ_PMS_DRAW 			= 0,		// 5本使用
};

enum{
	// トレーナー人数
	WBT_TRAINER_MAX = 8,
};

//-------------------------------------
// 以下に追加
//=====================================
// モード(外部パラメータになるので、削除予定)
enum{
	MODE_ROUND1 = 0,		// 1回戦
	MODE_ROUND2,			// 2回戦
	MODE_ROUND3,			// 3回戦
	//
	MODE_ROUND_MAX,
};

// 段階
enum{
	STEP_INIT	= 0,		// 初期化
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

// タイマ(念のため、2の倍数にしておく)
enum{
	TIMER_PANEL,			// パネルイン/アウト
	TIMER_GAGE,				// ゲージ伸び
	TIMER_BOMB,				// 爆発
	TIMER_WAIT,
	
	TIMER_MAX,
};

//-------------------------------------
/// リソースインデックス
//-------------------------------------
enum{
	RESID_OBJ_PLT = 0,				// パレット
	RESID_OBJ_CHR,					// キャラ
	RESID_OBJ_CEL,					// セルアニメ
	RESID_OBJ_MAX,
};

//-------------------------------------
/// CLWK
//-------------------------------------
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

//-----------------------------------
// 移動のパラメータ用
//-----------------------------------
enum{
	MWORK_PANEL_L	= 0,			// 左移動
	MWORK_PANEL_R,					// 右移動
	//
	MWORK_MAX,
};

//-----------------------------------
// 移動の種類
//-----------------------------------
enum{
	MOVE_PANEL_IN	= 0,			// パネルin
	MOVE_PANEL_OUT,					// パネルout
	//
	MOVE_PANEL_MAX,
};

//-------------------
// 移動のフラグ
//-------------------
enum{
	FLG_MOVE_INIT	= (1<<0),		// 初期化
	FLG_MOVE_END	= (1<<1),		// 終了
};

//-------------------
// OBJのフラグ(dispflag)
//-------------------
enum{
	FLG_OBJ_ANIM	= (1<<0),		// アニメあり
	FLG_OBJ_HFLIP	= (1<<1),		// HFLIP
	FLG_OBJ_VFLIP	= (1<<2),		// VFLIP
};

//-----------------------------------
// 定義
//-----------------------------------
// 最大数
enum{
	WBT_OBJ_MAXNUM = 23,			// obj最大
	WBT_CANVAS_MAXNUM = 2,			// canvas最大
	WBT_BMPOAM_MAXNUM = 8,			// bmpoam最大
};

// スピード
enum{
//	MOVE_IN_SPEED	= 36,			// スクロールイン
//	MOVE_OUT_SPEED	= 72,			// スクロールアウト
	MOVE_IN_SPEED	= 18,			// スクロールイン
	MOVE_OUT_SPEED	= 36,			// スクロールアウト
};

// empty
enum{
	BOMB_EMPTY_ID	= 24,			// 爆発のempty
};

// シーケンス
enum{
	L_WHITE_SEQ		= 0,			// 左、白シーケンス
	L_GRAY_SEQ		= 1,			// 左、灰シーケンス
	R_WHITE_SEQ		= 4,			// 左、白シーケンス
	R_GRAY_SEQ		= 5,			// 左、灰シーケンス
	//
	L_BLUE_SEQ		= 2,			// 左、青シーケンス
	L_RED_SEQ		= 3,			// 左、赤のシーケンス
	R_BLUE_SEQ		= 6,			// 右、青シーケンス
	R_RED_SEQ		= 7,			// 右、赤のシーケンス
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
	s16			sx;			// 開始
	s16			ex;			// 終了
	s16			flag;		// フラグ
	s16			pad;
	//
	s32			rate;		// 加算値(x100)
	s32			calcnum;	// 計算結果(x100)
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
	u8			mesid;			// メッセージid(?)
} WBTLIST_PARAM_WORK;

// 個数
enum{
	PARAM_ROUND1 = 1,
	PARAM_ROUND2 = 5,
	PARAM_ROUND3 = 3,
	//
	PARAM_WORK_MAX	= (PARAM_ROUND1 + PARAM_ROUND2 + PARAM_ROUND3),
};

// パラメータ参照用
enum{
	PRM_EXPPOS = 0,
	PRM_WINPOS,
	PRM_MOVEFLAG,
	PRM_LINEWORK,
	PRM_PATTERN,
	PRM_MESID,
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
	GFL_MSGDATA				*msg[2];					// 0:名詞、1:メッセージ(テストで使用)
	
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
	BMPOAM_ACT_PTR			BmpOamWork[WBT_BMPOAM_MAXNUM];	// Work
	PRINT_UTIL				PrintUtil[WBT_CANVAS_MAXNUM];	// 文字表示用
	PRINT_UTIL				MsgPrintUtil[2];				// 文字表示用
	PRINT_STREAM			*PrintStream;					// PrintStream
	GFL_TCBLSYS				*MsgTcblSys;					// メッセージ用タスク
	STRBUF					*MsgBuffer;						// メッセージ用バッファ
	//
	s32						Mode;							// モード(段階)
	s32						Step;							// ステップ
	s32						DispCount;						// カウンタ
	u32						MoveFlag;						// 移動可能フラグ
	s32						Timer[TIMER_MAX];				// タイマ
	WBTLIST_MOVE_WORK		MoveWork[MWORK_MAX];			// 移動
	WBTLIST_PARAM_WORK		ParamWork[PARAM_WORK_MAX];		// パラメータのワーク
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
// テーブル(2の倍数にしておく)
// ->最終的にはなくす方向
const s16 info_tbl[] = {
	60,						// パネルスクロールイン
	60,						// ゲージ伸び
	60,						// 爆発
	60,
};

// OBJの情報(上側)
// narc, resid, paloffset, palnum
const s16 obj_main_info_tbl[] = {
	NARC_wbt_list_gra_wbt_list_obj_u_NCLR, RESID_OBJ_PLT, 0, 5,	// NCL
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
	{ 8,	0x30,	0,	0,	2,	1,	24,	0,},		// 爆弾
	{ 9,	0x50,	1,	0,	3,	1,	32,	0,},		// ゲージ00
	{ 9,	0x50,	1,	0,	3,	5,	35,	0,},		// ゲージ01
	{ 9,	0x50,	1,	0,	3,	3,	36,	0,},		// ゲージ02
	{ 9,	0x50,	1,	0,	3,	7,	39,	0,},		// ゲージ03
	{ 11,	0x50,	1,	0,	3,	5,	41,	0,},		// ゲージ10
	{ 11,	0x50,	1,	0,	3,	7,	43,	0,},		// ゲージ11

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
	{ GFL_BG_FRAME1_S,	4,	4,	24,	2,	2,},		// 下、up
	{ GFL_BG_FRAME1_S,	10,	17,	11,	2,	2,},		// 下、down

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

// 伸びるゲージ
// 座標indexとフラグとシーケンス(前、後)
const s16 wbt_bar_anim_tbl[] = {
	32, 0x01, 9, 10,		// 左0
	33, 0x05, 9, 10,		// 左1
	34, 0x01, 9, 10,		// 左2
	35, 0x05, 9, 10,		// 左3
	//
	36, 0x03, 9, 10,		// 右0
	37, 0x07, 9, 10,		// 右1
	38, 0x03, 9, 10,		// 右2
	39, 0x07, 9, 10,		// 右3
	//
	40, 0x01,11, 12,		// 左10
	41, 0x05,11, 12,		// 左11
	//
	42, 0x03,11, 12,		// 右10
	43, 0x07,11, 12,		// 右11
};

// パネル移動のx座標(外 -> 中)
const s16 wbt_move_pos_tbl[] = {
	-72,   0,				// 左側(-72 <=> 0)
	220, 148,				// 右側(148 <=> 220)
};

// BMPOAMのオフセット
const s16 wbt_bmpoam_off_tbl[] = {
	36,	8,					// 左側(off)
	8,	8,					// 右側(off)
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
//static void WBTList_OAM_MAPMODEL_CreateCLWK( WBTLIST_MAIN_WORK *wk, u16 tex_idx, u8 ptn_ofs, GFL_CLUNIT *unit, HEAPID heapID );
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
static void wbt_clear_meswin( WBTLIST_MAIN_WORK* wk );

//
// 座標取得
static s16 wbt_get_empty( WBTLIST_MAIN_WORK *wk, const s32 num );
//
// (アプリの)表示関連
static void wbt_set_gageanim( WBTLIST_MAIN_WORK *wk, const s32 wnum, const s32 mode, const s32 umode );
static BOOL wbt_is_gageanim( WBTLIST_MAIN_WORK *wk, const s32 wnum );
static void wbt_disp_bomb( WBTLIST_MAIN_WORK* wk, const s32 num );
static void wbt_init_mode( WBTLIST_MAIN_WORK* wk );
//
// 以下、(アプリの)パラメータ関連
static void wbt_init_param( WBTLIST_MAIN_WORK* wk );
static void wbt_calc_param( WBTLIST_MAIN_WORK* wk );
static s16  wbt_get_param(  WBTLIST_MAIN_WORK* wk, const s32 mode, const s32 off );
static BOOL wbt_chk_dispcount( WBTLIST_MAIN_WORK* wk );
static BOOL wbt_is_dispcount( WBTLIST_MAIN_WORK* wk );

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
	
	//メッセージ
//	wk->msg[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_mictest_dat, wk->heapID );
	wk->msg[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_system_dat, wk->heapID );
	wk->msg[1] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_trmsg_dat, wk->heapID );
//	wk->msg[1] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_mictest_dat, wk->heapID );
	
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
	GFL_MSG_Delete( wk->msg[0] );
	GFL_MSG_Delete( wk->msg[1] );
	
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
			if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
			{
				(*seq) ++;
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
		RES_SCR_M03_ID	= NARC_wbt_list_gra_wbt_list_bg_u_03_NSCR,
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


//=============================================================================
/**
 *	OAM_MAPMODEL
 */
//=============================================================================

#if 0	// 保留
typedef struct {
  u8 id;
  u8 ptn_ofs;
}TRAINER_OBJ_DATA;
static const TRAINER_OBJ_DATA TrainerObjData[WBT_TRAINER_MAX] = {
  { NARC_fldmmdl_mdlres_hero_nsbtx, 3 },
  { NARC_fldmmdl_mdlres_rivel_nsbtx, 3 },
  { NARC_fldmmdl_mdlres_support_nsbtx, 3 },
  { NARC_fldmmdl_mdlres_veteranm_nsbtx, 2 },

  { NARC_fldmmdl_mdlres_leader4_nsbtx, 3 },
  { NARC_fldmmdl_mdlres_trainerw_nsbtx, 2 },
  { NARC_fldmmdl_mdlres_trainerm_nsbtx, 2 },
  { NARC_fldmmdl_mdlres_leader3_nsbtx, 3 },
};
#endif

//-----------------------------------------------------------------------------
/**
 *	@brief  OAMマップモデル 作成 (人物OBJ特化仕様)
 *
 *	@param	WBTLIST_MAIN_WORK *wk ワーク
 *	@param	tex_idx   テクスチャのARC内インデックス
 *	@param  ptn_ofs   テクスチャオフセット（アニメパターンのオフセット）
 *	@param	GFL_CLUNIT *unit	CLUNIT  ユニット
 *	@param	heapID						ヒープID
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
#if 0	// 保留
static void WBTList_OAM_MAPMODEL_CreateCLWK( WBTLIST_MAIN_WORK *wk, u16 tex_idx, u8 ptn_ofs, GFL_CLUNIT *unit, HEAPID heapID )
{	
  u32 i;
  UI_EASY_CLWK_RES_PARAM prm;
  const TRAINER_OBJ_DATA * trset;
  s16 * pos_tbl = GFL_ARC_LoadDataAlloc(
      ARCID_WBTLIST_GRA, NARC_wbt_list_gra_trainer_obj_pos_tblbin, wk->heapID );

  for( i = 0; i < WBT_TRAINER_MAX; i ++ )
  {

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = APP_COMMON_GetArcId();
    prm.pltt_id   = APP_COMMON_GetNull4x4PltArcIdx();
    prm.ncg_id    = APP_COMMON_GetNull4x4CharArcIdx();
    prm.cell_id   = APP_COMMON_GetNull4x4CellArcIdx( APP_COMMON_MAPPING_128K );
    prm.anm_id    = APP_COMMON_GetNull4x4AnimeArcIdx( APP_COMMON_MAPPING_128K );
    prm.pltt_line = PLTID_OBJ_OAM_MAPMODEL_M + i;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num = 1;

    trset = &TrainerObjData[ i ];
    // リソース読み込み
    UI_EASY_CLWK_LoadResource( &wk->clres_oam_mmdl, &prm, unit, heapID );
    // CLWK生成
    wk->clwk_oam_mmdl[i]
      = UI_EASY_CLWK_CreateCLWK( &wk->clres_oam_mmdl, unit,
          pos_tbl[i*2+0], pos_tbl[i*2+1], 0, heapID );

    // テクスチャを転送
    {
      // 人物は基本 4 x 4
      int sx = NSBTX_DEF_SX;
      int sy = NSBTX_DEF_SY;

      CLWK_TransNSBTX( wk->clwk_oam_mmdl[i],
          ARCID_MMDL_RES, trset->id, trset->ptn_ofs, sx, sy, 0, prm.draw_type, heapID );
    }
    wk->clwk_oam_mmdl_count ++;
    //
	// プライオリティをセット(要修正)
	GFL_CLACT_WK_SetSoftPri( wk->clwk_oam_mmdl[i], 0x40 );
  }
  GFL_HEAP_FreeMemory( pos_tbl );
}
#endif

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
#ifdef _DEBUG_PARAM_	// debug
		CLWK_TransNSBTX( wk->clwk_oam_mmdl[i], ARCID_MMDL_RES, adr->id, adr->pattern, NSBTX_DEF_SX, NSBTX_DEF_SY, 0, prm.draw_type, wk->heapID );
#else	// パラメータ参照
		buf[0] = wk->Param->trainer[i].mmdl_id;
		buf[1] = wk->Param->trainer[i].pattern_no;
		CLWK_TransNSBTX( wk->clwk_oam_mmdl[i], ARCID_MMDL_RES, buf[0], buf[1], NSBTX_DEF_SX, NSBTX_DEF_SY, 0, prm.draw_type, wk->heapID );
#endif
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
	STRBUF* strbuf;
	
	// debug
	wbt_debug_param( wk );
	
	// 文字列(test)
	strbuf = GFL_MSG_CreateString( wk->msg[0], 0 );
//	wk->MsgBuffer = GFL_MSG_CreateString( wk->msg[1], 0 );
	
	// 初期化
	// それほど複雑ではないので、
	// 上、下画面をまとめて処理
	wk->Mode = wk->Param->stage;
//	wk->Mode = MODE_ROUND1;		// 外からのパラメータ
//	wk->Mode = MODE_ROUND2;		// 外からのパラメータ
//	wk->Mode = MODE_ROUND3;		// 外からのパラメータ
	wk->Step = STEP_INIT;
	//
	wk->DispCount = 0;
	wk->MoveFlag = 0;
	
	for(i=0;i<TIMER_MAX;i++){
		wk->Timer[i] = 0;
	}
	// empty用バッファ取得(必ず破棄すること)
	// キャラテーブルを借用
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_WBTLIST_GRA, NARC_wbt_list_gra_trainer_obj_pos_tblbin, wk->heapID );
	
	//
	// 文字初期化
	wbt_init_string( wk );
	wbt_disp_string( wk, 0, 96, 0, strbuf );
	wbt_disp_string( wk, 1, 44, 0, strbuf );
	// 削除
//	GFL_STR_DeleteBuffer( strbuf );
	
	//
	// OBJ初期化
	wbt_init_obj( wk );
	
	// 転送
	for(i=0;i<WBT_BMPOAM_MAXNUM;i++){
		GFL_BMP_Clear( wk->BmpData[i], 0 );
#ifdef _DEBUG_PARAM_
		PRINTSYS_PrintQueColor( wk->print_que, wk->BmpData[i], 0, 0, strbuf, wk->font, PRINTSYS_LSB_Make(15,14,0) );
#else
		PRINTSYS_PrintQueColor( wk->print_que, wk->BmpData[i], 0, 0, wk->Param->trainer[i].name, wk->font, PRINTSYS_LSB_Make(15,14,0) );
#endif
		BmpOam_ActorBmpTrans( wk->BmpOamWork[i] );
	}
	// パラメータ参照
	wbt_init_param( wk );
	
	// 削除
//	GFL_STR_DeleteBuffer( strbuf );
	
	// 会話ウィンドウセット(キャラデータを転送)
	BmpWinFrame_GraphicSet( GFL_BG_FRAME0_M, 1, 8, MENU_TYPE_SYSTEM, wk->heapID);
//	BmpWinFrame_GraphicSet( GFL_BG_FRAME0_M, 1, 8, MENU_TYPE_TALK, wk->heapID);
	// canvasの設定
//	wk->MsgPrintUtil[0].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 0, 19, 32, 5, 8, GFL_BMP_CHRAREA_GET_B );
	wk->MsgPrintUtil[0].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 1, 20, 30, 4, 8, GFL_BMP_CHRAREA_GET_B );
	// 表示
//	BmpWinFrame_Write( wk->MsgPrintUtil[0].win, WINDOW_TRANS_OFF, 1, 8 );
	BmpWinFrame_Write( wk->MsgPrintUtil[0].win, WINDOW_TRANS_ON,  1, 8 );
	//
	// set
//	wbt_set_meswin( wk, 0 );
	//
	// 削除
	GFL_STR_DeleteBuffer( strbuf );
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
	s32		buf[2];
	
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
			break;
		
		// LV1(パネルイン)
		case STEP_LV1:
			// 初期化
			wbt_main_move_panel( wk );
			// 移動終了
			if( wbt_is_movework( wk, MWORK_PANEL_L ) == FALSE ){
				wk->Step = STEP_LV2;
				// メッセージ表示
				tmp = wbt_get_param( wk, PRM_MESID, 0 );
				wbt_set_meswin( wk, tmp );
			}
			break;
		
		
		// LV2(メッセージ表示待ち)
		case STEP_LV2:
			// メッセージ待ち
			if( wbt_main_meswin( wk ) == FALSE ){
				//
				// 1回戦 => 終了
				if(wk->Mode == MODE_ROUND1){
					wk->Step = STEP_END;
				}
				// 2、3回戦=>パネルアウト
				else{
					wk->Step = STEP_LV3;
					wbt_init_move_panel( wk, MOVE_PANEL_OUT );
					// meswinクリア
					wbt_clear_meswin( wk );
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
				// 爆発
				tmp = wbt_get_param( wk, PRM_EXPPOS, -1 );
				wbt_disp_bomb( wk, tmp );
				//
#if 0
				// パネル色変え
				tmp = wbt_get_param( wk, PRM_EXPPOS, -1 );
				seq = R_GRAY_SEQ;
				if(tmp < 4){
					seq = L_GRAY_SEQ;
				}
				wbt_chg_objseq( wk, tmp, seq );
#endif
				//
				// 次へ
				wk->Step = STEP_LV4;
//				wbt_init_timer( wk, TIMER_BOMB, info_tbl[0] );
			}
			break;
		
		//
		// LV4(爆破)
		case STEP_LV4:
//			timer = wbt_main_timer( wk, TIMER_BOMB );
//			if(timer <= 0){
			if( wbt_is_anime_end( wk, CLWKID_BOMB ) == FALSE ){
				// 爆発が終了してから色変え
				// パネル色変え
				tmp = wbt_get_param( wk, PRM_EXPPOS, -1 );
				seq = R_GRAY_SEQ;
				if(tmp < 4){
					seq = L_GRAY_SEQ;
				}
				wbt_chg_objseq( wk, tmp, seq );
				
				// 爆破アニメ終了=>ゲージ伸び
				wk->Step = STEP_LV5;
				wbt_init_timer( wk, TIMER_GAGE, info_tbl[2] );
				//
				// ゲージ伸びる処理
				sbuf[0] = wbt_get_param( wk, PRM_LINEWORK, -1 );
				sbuf[1] = wbt_get_param( wk, PRM_PATTERN,  -1 );
				wbt_set_gageanim( wk, sbuf[0], 0, sbuf[1] );
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
			}
			break;
		// ゲージ切り替え後待ち
		case STEP_LV5+10:
			timer = wbt_main_timer( wk, TIMER_GAGE );
			if(timer <= 0){
				// ゲージ伸び終了
				wk->Step = STEP_LV6;
				wbt_init_move_panel( wk, MOVE_PANEL_IN );
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
				tmp = wbt_get_param( wk, PRM_MESID, 0 );
				wbt_set_meswin( wk, tmp );
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
 *	@retval 時間
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
//	STRBUF* strbuf;
	
	// test
//	strbuf = GFL_MSG_CreateString( wk->msg, 0 );
	
	// 表示
	PRINTTOOL_PrintColor(
				&wk->PrintUtil[mode],			// プリントユーティル ( BMPWIN )
				wk->print_que,					// プリントキュー
				x, y,							// 表示基準座標
				strbuf,							// 文字列
				wk->font,						// フォント
				PRINTSYS_LSB_Make(15,14,0),		// カラー
				PRINTTOOL_MODE_CENTER );		// 中央
	// 削除
//	GFL_STR_DeleteBuffer( strbuf );
	// 転送
	GFL_BMPWIN_MakeScreen( wk->PrintUtil[mode].win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[mode].win) );
	//
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
	GFL_BMPWIN_Delete( wk->MsgPrintUtil[0].win );
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
 *	@param	const BOOL		(TRUE:再生中、FALSE:停止、終了)
 *
 *	@retval
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
 *	@retval
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
		buf[2] = wbt_bmpoam_off_tbl[0];		// bmpoam
		buf[3] = wbt_bmpoam_off_tbl[1];
	}else{						// 引っ込む
		buf[0] = wbt_move_pos_tbl[0];		// パネル
		buf[1] = wbt_move_pos_tbl[2];
		buf[2] = wbt_bmpoam_off_tbl[2];		// bmpoam
		buf[3] = wbt_bmpoam_off_tbl[3];
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
	buf[0] = wbt_bmpoam_off_tbl[0];		// bmpoamのオフセット
	buf[1] = wbt_bmpoam_off_tbl[1];
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
	buf[0] = wbt_bmpoam_off_tbl[2];		// bmpoamのオフセット
	buf[1] = wbt_bmpoam_off_tbl[3];
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
	// PrintStream終了時にはdeleteしないとtaskが残ってしまうようだ
	// 念のためcheck
	if( wk->PrintStream ){
		PRINTSYS_PrintStreamDelete( wk->PrintStream );
	}
	if(wk->MsgBuffer){
		GFL_STR_DeleteBuffer( wk->MsgBuffer );
	}
	// メッセージバッファセット
	wk->MsgBuffer = GFL_MSG_CreateString( wk->msg[1], mesid );
	
	// test
//	GFL_FONTSYS_SetColor( 1, 9, 0x0c );
	// stream設定=>ここのstrbufは終わるまで削除してはいけない
	wk->PrintStream = PRINTSYS_PrintStream(
				wk->MsgPrintUtil[0].win, 0, 0, wk->MsgBuffer, wk->font,
				MSGSPEED_GetWait(), wk->MsgTcblSys, 4, wk->heapID, 15 );
//				MSGSPEED_GetWait(), wk->MsgTcblSys, 4, wk->heapID, 0x0c );
	// スクリーン転送らしい
	GFL_BMPWIN_MakeTransWindow( wk->MsgPrintUtil[0].win );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ(処理)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL wbt_main_meswin( WBTLIST_MAIN_WORK* wk )
{
	// 描画終了待ち
	s32		stat;
	
	// 念のため
	if(wk->PrintStream == 0){
		return( FALSE );
	}
	
	stat = PRINTSYS_PrintStreamGetState( wk->PrintStream );
	if( stat == PRINTSTREAM_STATE_DONE ){
		if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE || GFL_UI_TP_GetTrg()){
//			PRINTSYS_PrintStreamDelete( wk->PrintStream );
			return( FALSE );
		}
	}
	else if( stat == PRINTSTREAM_STATE_PAUSE ){
		if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE || GFL_UI_TP_GetTrg()){
			PRINTSYS_PrintStreamReleasePause( wk->PrintStream );
			return( FALSE );
		}
	}
	return( TRUE );
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
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->MsgPrintUtil[0].win ), 15 );
	GFL_BMPWIN_TransVramCharacter( wk->MsgPrintUtil[0].win );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  emptyの取得(使い方注意)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s16 wbt_get_empty( WBTLIST_MAIN_WORK *wk, const s32 num )
{
	s16	*adr;
	
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
	s32		clwnum = CLWKID_BAR_00+wnum;
#if 1
	BOOL	bflg = wbt_is_anime_end( wk, clwnum );
#else
	BOOL	bflg=TRUE;
	
	// 終了判定らしい
	if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[clwnum] ) == FALSE ){
		bflg = FALSE;
	}
#endif
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
//	OS_Printf("anim seq:%d\n",anum);
	GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[CLWKID_BOMB], anum );
	//
	GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[CLWKID_BOMB], TRUE );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[CLWKID_BOMB], TRUE );
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
		// 全バー非表示
		for(i=0;i<6;i++){
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
		//
		// 2回戦までのパネル色戻し
		for(i=0;i<4;i++){
			buf[0] = wk->ParamWork[PARAM_ROUND1+i].exppos;		// 爆発パターン
			tmp = R_GRAY_SEQ;
			if(i < 4){
				tmp = L_GRAY_SEQ;
			}
			wbt_chg_objseq( wk, buf[0], tmp );
		}
		//
		// 全バー非表示
		for(i=0;i<6;i++){
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
		// 全バー非表示
		for(i=0;i<6;i++){
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
	}
	//
	// パラメータ計算
	wbt_calc_param( wk );
	// 移動パラメータセット
	wk->MoveFlag = wbt_get_param( wk, PRM_MOVEFLAG, 0 );
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
	}
	// 1回戦 => 上記の初期化でよい
	if(wk->Mode == MODE_ROUND1){
//		return;
	}
	// 2回戦+3回戦
	if((wk->Mode == MODE_ROUND2)||(wk->Mode == MODE_ROUND3)){
		offset = PARAM_ROUND1;
		for(i=0;i<4;i++){
			// 勝敗チェック
			if(wk->Param->first_stage[i].upside_win == TRUE){
				wk->ParamWork[offset+i].winpos = 2*i;		// 勝った番号
				wk->ParamWork[offset+i].exppos = 2*i+1;		// 負けた番号
				wk->ParamWork[offset+i].pattern = 0;		// ゲージのパターン
			}else{
				wk->ParamWork[offset+i].winpos = 2*i+1;
				wk->ParamWork[offset+i].exppos = 2*i;
				wk->ParamWork[offset+i].pattern = 1;
			}
			// 移動可能フラグ
			wk->ParamWork[offset+i].moveflag  = (1<<(2*i));
			wk->ParamWork[offset+i].moveflag |= (1<<(2*i+1));
			// ライン番号
			wk->ParamWork[offset+i].linework = i;
			// メッセージid
			wk->ParamWork[offset+i].mesid = wk->Param->first_stage[i].msg_type;
		}
		// 最後に自分側の移動可能フラグをセットする
		offset = PARAM_ROUND1+4;
		wk->ParamWork[offset].moveflag  = (1<<(wk->ParamWork[PARAM_ROUND1].winpos));
		wk->ParamWork[offset].moveflag |= (1<<(wk->ParamWork[PARAM_ROUND1+1].winpos));
	}
	// 3回戦
	if(wk->Mode == MODE_ROUND3){
		offset = PARAM_ROUND1+PARAM_ROUND2;
		for(i=0;i<2;i++){
			if( wk->Param->second_stage[i].upside_win == TRUE ){
				// 情報は2回戦での勝った番号から調べる
				wk->ParamWork[offset+i].winpos = wk->ParamWork[PARAM_ROUND1+2*i].winpos;
				wk->ParamWork[offset+i].exppos = wk->ParamWork[PARAM_ROUND1+2*i+1].winpos;
				wk->ParamWork[offset+i].pattern = 0;		// ゲージのパターン
			}else{
				wk->ParamWork[offset+i].winpos = wk->ParamWork[PARAM_ROUND1+2*i+1].winpos;
				wk->ParamWork[offset+i].exppos = wk->ParamWork[PARAM_ROUND1+2*i].winpos;
				wk->ParamWork[offset+i].pattern = 1;		// ゲージのパターン
			}
			// 移動可能フラグ
			wk->ParamWork[offset+i].moveflag  = (1<<(wk->ParamWork[PARAM_ROUND1+2*i].winpos));
			wk->ParamWork[offset+i].moveflag |= (1<<(wk->ParamWork[PARAM_ROUND1+2*i+1].winpos));
			// ライン番号
			wk->ParamWork[offset+i].linework = 4+i;
			// メッセージid
			wk->ParamWork[offset+i].mesid = wk->Param->second_stage[i].msg_type;
		}
		// 最後に自分側の移動可能フラグをセットする
		offset = PARAM_ROUND1+PARAM_ROUND2+2;
		wk->ParamWork[offset].moveflag  = (1<<(wk->ParamWork[PARAM_ROUND1+PARAM_ROUND2].winpos));
		wk->ParamWork[offset].moveflag |= (1<<(wk->ParamWork[PARAM_ROUND1+PARAM_ROUND2+1].winpos));
	}
	
#ifdef _DEBUG_LOCAL_
	// 表示
	for(i=0;i<PARAM_WORK_MAX;i++){
		OS_Printf("bomb,win,flag,linework,pattern,mesid:(%d),%d,%d,%x,%d,%d,%d\n",
			i,
			wk->ParamWork[i].exppos,
			wk->ParamWork[i].winpos,
			wk->ParamWork[i].moveflag,
			wk->ParamWork[i].linework,
			wk->ParamWork[i].pattern,
			wk->ParamWork[i].mesid );
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
	else{
		offset = 0;
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
		if(wk->DispCount >= 4){
			wk->DispCount = 4;
			bflg = TRUE;
		}
	}
	// 3回戦
	else if(wk->Mode == MODE_ROUND3){
		if(wk->DispCount >= 2){
			wk->DispCount = 2;
			bflg = TRUE;
		}
	}
	// 1回戦
	else{
		wk->DispCount = 0;
		bflg = TRUE;
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
		if(wk->DispCount >= 4){
			bflg = TRUE;
		}
	}
	// 3回戦
	else if(wk->Mode == MODE_ROUND3){
		if(wk->DispCount >= 2){
			bflg = TRUE;
		}
	}
	// 1回戦
	else{
		bflg = TRUE;
	}
	// 終了判定
	return( bflg );
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
#ifdef _DEBUG_LOCAL_
	s16			i;
	
	// ??
	OS_Printf( "stage :%d\n",wk->Param->stage );
	for(i=0;i<WBT_TRAINER_NUM;i++){
		OS_Printf( "id,pattern,sex,prio,poketype (%d):%d,%d,%d,%d,%d\n",
					i,wk->Param->trainer[i].mmdl_id, wk->Param->trainer[i].pattern_no, 
					  wk->Param->trainer[i].sex, wk->Param->trainer[i].priority, wk->Param->trainer[i].poketype );
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
#endif
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
#ifdef _DEBUG_LOCAL_
	// debug
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
		wk->Step = STEP_INIT;
		// 表示回数
		wk->DispCount = 0;
		//
		wbt_init_param( wk );
		OS_Printf("debug init \n");
	}
#if 0		// パラメータ参照に変更
	// debug
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		wk->Mode++;
		if(wk->Mode > MODE_ROUND3){
			wk->Mode = MODE_ROUND1;
		}
		wk->Step = STEP_INIT;
		wk->DispCount = 0;
		//
		wbt_init_param( wk );
		OS_Printf("---- round : %d ----\n",wk->Mode);
		OS_Printf("mode,step :%d,%d\n",wk->Mode,wk->Step);
	}
#endif
#endif
}

