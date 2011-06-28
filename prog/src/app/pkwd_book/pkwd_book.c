//=============================================================================
/**
 *
 *	@file		pkwd_book.c
 *	@brief  ポケウッド台本(app)
 *	@author
 *	@date		2011.03.02
 *
 * 2011.03.02 ui_templateから派生、作成開始
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

#include "system/wipe.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

#include "system/bmp_oam.h"

//PRINT_QUE
#include "print/printsys.h"
#include "app/app_menu_common.h"
//
#include "pokeicon/pokeicon.h"
#include "arc/app_menu_common.naix"
#include "system/tr2dgra.h"

#include "app/pkwd_book.h"

#include "field/event_pokewood.h"

#include "pkwd_book_graphic.h"
#include "b_scenario_gra.naix"
#include "msg/msg_pw_movietitle.h"		// test

// メッセージ
#include "message.naix"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================

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

// 段階
enum{
	STEP_INIT	= 0,		// 初期化
	STEP_LV1 	= 100,
	STEP_LV2 	= 200,
	STEP_LV3 	= 300,
	STEP_LV4 	= 400,
	//
	STEP_END	= 10000,
};

//
enum{
	TMODE_BOOK	= 0,		///< 台本選択
	TMODE_TRAINER,			///< 詳細(トレーナー)
	TMODE_POKE,				///< 詳細(ポケモン)
	//
	TMODE_MAX,
};

//
enum{
	MODE_END		= -1,	///< 終了
	
	MODE_INIT_BOOK 	= 0,	///< 台本選択
	MODE_MAIN_BOOK,
	MODE_EXIT_BOOK,
	MODE_INIT_TRAINER,		///< 詳細
	MODE_MAIN_TRAINER,
	MODE_EXIT_TRAINER,
	MODE_INIT_POKE,			///< 詳細
	MODE_MAIN_POKE,
	MODE_EXIT_POKE,
	//
	MODE_MAX,
};

// フォント
enum{
	STR_MAIN	= 0,		// 上画面
	STR_SUB,				// 下画面
};

// パレット
// 上(MAIN)：BG  (0-7:下地)
// 上(MAIN)：OBJ ()
//
// 下(SUB) ：BG  (0-7:下地、8:cmn)
// 下(SUB) ：OBJ (0-3:poke),(4-7:cmn),(8-10:type),(11:trainer),(12:font)
enum{
	// 何本目か(bg)
	PLTID_SUB_BG_CMN		= 8,		// cmnのBG
	//
	PLTID_SUB_OBJ_CMN		= 4,		// cmn
	PLTID_SUB_OBJ_TYPE		= 8,		// type
	PLTID_SUB_OBJ_TRAINER 	= 11,		// trainer
	PLTID_SUB_OBJ_FONT		= 12,		// font
	//
	// 何番目か
	MES_CLEAR_COLOR			= 0,		// クリアカラー
};

// 最大値
enum{
	PKWD_BOOK_EMPTY_MAXNUM = 23*2,		// empty
	//
	PKWD_BOOK_TRAINER_OBJ_MAXNUM = 1,	// obj(trainer)
	PKWD_BOOK_CMN_OBJ_MAXNUM	= 3,	// obj(cmn)
	PKWD_BOOK_POKE_OBJ_MAXNUM	= 7,	// obj(poke)
	PKWD_BOOK_TYPE_OBJ_MAXNUM	= 6,	// obj(type)
	//
	PKWD_BOOK_BGNUM_MAXNUM		= 8,	// bg
	//
	PKWD_BOOK_CANVAS_MAXNUM		= 16,	// canvas
	//
	PKWD_BOOK_MES_MAX			= 2,
	//
	PKWD_BOOK_BMPOAM_MAXNUM		= 9,	// bmpoam
	//
	PKWD_POKE_MAX				= 6,	// pokemon max
};

//-------------
// obj関連
//-------------
// OBJのリソースのインデックス
enum{
	// cmn
	RESID_CMN_OBJ_PLT = 0,				// パレット
	RESID_CMN_OBJ_CHR,					// キャラ
	RESID_CMN_OBJ_CEL,					// セルアニメ
	// ポケモン
	RESID_POKE_PLT,						// パレット
	RESID_POKE_CHR_00,					// トレーナー(6体)
	RESID_POKE_CHR_01,
	RESID_POKE_CHR_02,
	RESID_POKE_CHR_03,
	RESID_POKE_CHR_04,
	RESID_POKE_CHR_05,
	RESID_POKE_CHR_10,					// 詳細(1体)
	RESID_POKE_CEL,						// セルアニメ
	// type
	RESID_TYPE_PLT,						// パレット
	RESID_TYPE_CHR_00,					// type
	RESID_TYPE_CHR_01,
	RESID_TYPE_CHR_10,
	RESID_TYPE_CHR_11,
	RESID_TYPE_CHR_12,
	RESID_TYPE_CHR_13,
	RESID_TYPE_CEL,						// セルアニメ
	//
	RESID_TRAINER_PLT,					// パレット
	RESID_TRAINER_CHR,					// キャラ
	RESID_TRAINER_CEL,					// セルアニメ
	//
	RESID_FONT_PLT,						// bmpoam用
	//
	RESID_OBJ_MAX,
};

// clwkのグループ
enum{
	CLWKMODE_CMN	= 0,				// 通常
	CLWKMODE_POKE,						// ポケモン
	CLWKMODE_TYPE,						// タイプ
	CLWKMODE_TRAINER,					// トレーナー
};


// CLWK(CMN)
enum{
	CLWK_CMN_BACK		= 0,			// 戻る
	CLWK_CMN_LEFT,						// 左
	CLWK_CMN_RIGHT,						// 右
};

// SEQ(CMN)
enum{
	SEQ_CMN_BACK 		= 1,			// 戻る(通常)
	SEQ_CMN_LEFT		= 4,			// 左
	SEQ_CMN_RIGHT		= 5,			// 右
	//
	SEQ_CMN_BACK_ANIM	= 9,			// 戻る(on)
	SEQ_CMN_LEFT_ANIM	= 12,			// 左(on)
	SEQ_CMN_RIGHT_ANIM	= 13,			// 右(on)
	//
	SEQ_CMN_LEFT_OFF	= 18,			// 左(off)
	SEQ_CMN_RIGHT_OFF	= 19,			// 右(off)
};

//-------------
// フラグ
//-------------
// OBJのフラグ(PKWD_BOOK_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM		= (1<<0),		// アニメあり
	FLG_OBJ_HFLIP		= (1<<1),		// HFLIP
	FLG_OBJ_VFLIP		= (1<<2),		// VFLIP
};

// PKWD_BOOK_MAIN_WORK.Flag
enum{
	FLG_WIPE_IN_END		= (1<<0),		// wipe(in)終了
	FLG_SET_INIT		= (1<<1),		// 初期化済み
	FLG_CHG_TITLE		= (1<<2),		// 変更
	FLG_MODE_SUB		= (1<<3),		// サブ画面に移行した
};

//-------------
// 文字列
//-------------
// 固定文字列(pwb_fix_mestbl)と対応
enum{
	MES_ID_TOP_01		= 0,			// タイトル名
	MES_ID_TOP_02,						// あらすじ
	MES_ID_TOP_03,						// クリア目標
	//
	MES_ID_INFO_01,						// キャスト
	MES_ID_INFO_02,						// もちもの
	MES_ID_INFO_03,						// とくせい
};

// canvasの番号
enum{
	PWB_CVS_TOP_TITLE = 0,				// TOP
	PWB_CVS_TOP_PLOT,
	PWB_CVS_TOP_GOAL,
	PWB_CVS_TOP_TITLE_MES,
	PWB_CVS_TOP_PLOT_MES,
	PWB_CVS_TOP_GOAL_MES,
	PWB_CSV_TOP_MAX,		// max
	//
	PWB_CVS_BOOK_TITLE = 0,				// BOOK
	PWB_CVS_BOOK_MAX,		// max
	
	PWB_CVS_TRAINER_TITLE = 0,			// TRAINER
	PWB_CVS_TRAINER_NAME,
	PWB_CVS_TRAINER_WORK,
	PWB_CVS_TRAINER_WORKER,
	PWB_CVS_TRAINER_MAX,	// max
	
	PWB_CVS_INFO_CAST = 0,				// INFO
	PWB_CVS_INFO_NAME,
	PWB_CVS_INFO_SEX,
	PWB_CVS_INFO_LEVEL,
	PWB_CVS_INFO_ITEM,
	PWB_CVS_INFO_ITEM01,
	PWB_CVS_INFO_SKILL00,
	PWB_CVS_INFO_SKILL01,
	PWB_CVS_INFO_SKILL02,
	PWB_CVS_INFO_SKILL03,
	PWB_CVS_INFO_STATUS,
	PWB_CVS_INFO_STATUS01,
	PWB_CVS_INFO_MAX,		// max
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//----------------------
// ポケモンの情報(1体)
//----------------------
// PP_Getを初期化時にまとめて処理するために作成
// (すべてが)u32でなくてもよいはずが、
// 返り値を直接セットするため以下のようにした
typedef struct{
	u32					id;				// id
	u32					sex;			// 性別
	u32					level;			// レベル
	u32					item;			// もちもの
	u32					stat;			// 特性
	u32					poke_type[2];	// タイプ
	u32					skill[4];		// 技
	u32					skill_type[4];	// 技対応
} PKWD_BOOK_POKE_WORK;

//-------------------------------------
/// OBJ管理ワーク
//-------------------------------------
typedef struct
{
	u32			res[RESID_OBJ_MAX]; 					// リソース
	// cmn
	GFL_CLWK	*clwk_cmn[PKWD_BOOK_CMN_OBJ_MAXNUM];	// CLWK
	// poke
	GFL_CLWK	*clwk_poke[PKWD_BOOK_POKE_OBJ_MAXNUM];	// CLWK
	// type
	GFL_CLWK	*clwk_type[PKWD_BOOK_TYPE_OBJ_MAXNUM];	// CLWK
	// trainer
	GFL_CLWK	*clwk_tr[PKWD_BOOK_TRAINER_OBJ_MAXNUM];	// CLWK
	//
} PKWD_BOOK_OBJ_WORK;

//--------------------------------------------------------------
///	メインワーク
//--------------------------------------------------------------
typedef struct
{
	HEAPID					heapID;
	// OBJ
	PKWD_BOOK_OBJ_WORK		wk_obj;
	//描画設定
	PKWD_BOOK_GRAPHIC_WORK	*graphic;
	//
	PKWD_BOOK_PARAM 		*Param;						// 参照用として保存
	//
	GFL_FONT				*font;									///< フォント
	GFL_FONT				*sfont;									///< フォント
	PRINT_QUE				*print_que;								///< que
	PRINT_UTIL				PrintUtil[PKWD_BOOK_CANVAS_MAXNUM];		///< 文字表示用(下画面:切り替える)
	PRINT_UTIL				PrintUtil2[PKWD_BOOK_CANVAS_MAXNUM];	///< 文字表示用(上画面)
	GFL_MSGDATA				*MsgData[PKWD_BOOK_MES_MAX];			///< メッセージ
	WORDSET					*Wordset;								///< wordset
	//
	BMPOAM_SYS_PTR			BmpOamSys;								// BMPOAM
	GFL_BMP_DATA			*BmpData[PKWD_BOOK_BMPOAM_MAXNUM];		// Bmp
	BMPOAM_ACT_PTR			BmpOamWork[PKWD_BOOK_BMPOAM_MAXNUM];	// BMPOAM用Work
	//
	s32						Step;						// ステップ
	s32						Mode;						// モード
	s32						SubMode;					// サブモード
	s32						TitleMode;					// タイトル
	s32						NextMode;					// 次
	u32						Flag;						// フラグ
	u32						PFlag;						// フラグ
	u32						PFlag2;						// フラグ(要整理)
	s32						ScenarioCurPos;				// 台本カーソル(0-47)
	s32						ScenarioNum;				// シナリオ番号
	s32						InfoCurPos;					///< 詳細用のカーソル(PokeMax+1)
	//
	s16						*EmptyBuffer;				// 座標用のバッファ(必ず破棄すること)
	//
	s32						PokeMax;					///< ポケモン最大
	s32						PokePageNum;				///< ポケモンページ番号(0-max-1)
	POKEPARTY				*Party;						///< パーティ情報
	PKWD_BOOK_POKE_WORK		PokeWork[PKWD_POKE_MAX];	///< ポケモン情報
} PKWD_BOOK_MAIN_WORK;

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
	u8				res_cnum;		// リソース番号(注意)
}PKWD_BOOK_OBJ_DATA;

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}PKWD_BOOK_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}PKWD_BOOK_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				poff;			// パレットのオフセット
}PKWD_BOOK_BGSCN_DATA;

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
}PKWD_BOOK_BMPOAM_DATA;

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
}PKWD_BOOK_CANVAS_DATA;

//=============================================================================
/**
 *							データ定義(要整理)
 */
//=============================================================================
//---------------
// BG関連(ロゴ)
//---------------
// BGPAL
static const PKWD_BOOK_BGPAL_DATA pwc_bgpal_tbl[] = {
	{ PALTYPE_MAIN_BG,	NARC_b_scenario_gra_pkwd_scenario_NCLR,	0,	0,},	// 上
	{ PALTYPE_SUB_BG,	NARC_b_scenario_gra_pkwd_scenario_NCLR,	0,	0,},	// 下

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const PKWD_BOOK_BGCHR_DATA pwc_bgchr_tbl[] = {
	{ GFL_BG_FRAME3_M,	NARC_b_scenario_gra_pkwd_scenario_NCGR,	0,	0,},	// 上
	{ GFL_BG_FRAME3_S,	NARC_b_scenario_gra_pkwd_scenario_NCGR,	0,	0,},	// 下

	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const PKWD_BOOK_BGSCN_DATA pwc_bgscn_tbl[] = {
	{ GFL_BG_FRAME3_M,	NARC_b_scenario_gra_pkwd_scenario_NSCR,	0,	0,},	// 上
	{ GFL_BG_FRAME3_S,	NARC_b_scenario_gra_pkwd_scenario_NSCR,	0,	0,},	// 下

	{ 0xff, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(cmn)
//------------------
// 情報
static const s16 pwb_obj_cmn_itbl[] = {
	RESID_CMN_OBJ_PLT,
	RESID_CMN_OBJ_CHR,
	RESID_CMN_OBJ_CEL,
	//
	PKWD_BOOK_CMN_OBJ_MAXNUM,
	CLWKMODE_CMN,
};

// OBJのテーブルデータ
static const PKWD_BOOK_OBJ_DATA pwb_obj_cmn_tbl[] = {
	{  1,	0x30,	1,	1,	0,	1,	3,	RESID_CMN_OBJ_CHR,},		// 戻る
	{  4,	0x30,	1,	1,	0,	1,	0,	RESID_CMN_OBJ_CHR,},		// 左矢印
	{  5,	0x30,	1,	1,	0,	1,	1,	RESID_CMN_OBJ_CHR,},		// 右矢印

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// OBJ(poke)
//----------------
// 情報
static const s16 pwb_obj_poke_itbl[] = {
	RESID_POKE_PLT,		// plt res
	0,					// chr res(未使用)
	RESID_POKE_CEL,		// cel res
	//
	PKWD_BOOK_POKE_OBJ_MAXNUM,		// max
	CLWKMODE_POKE,
};

// OBJのテーブルデータ
const PKWD_BOOK_OBJ_DATA pwb_obj_poke_tbl[] = {
	{ 1,	0x28,	1,	0,	0,	0,	4, RESID_POKE_CHR_00,},		// ポケモン00
	{ 1,	0x28,	1,	0,	0,	0,	5, RESID_POKE_CHR_01,},		// ポケモン01
	{ 1,	0x28,	1,	0,	0,	0,	6, RESID_POKE_CHR_02,},		// ポケモン02
	{ 1,	0x28,	1,	0,	0,	0,	7, RESID_POKE_CHR_03,},		// ポケモン03
	{ 1,	0x28,	1,	0,	0,	0,	8, RESID_POKE_CHR_04,},		// ポケモン04
	{ 1,	0x28,	1,	0,	0,	0,	9, RESID_POKE_CHR_05,},		// ポケモン05
	//
	{ 1,	0x28,	1,	0,	0,	0,	10, RESID_POKE_CHR_10,},	// ポケモン10
	
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// OBJ(type)
//----------------
// 情報
static const s16 pwb_obj_type_itbl[] = {
	RESID_TYPE_PLT,		// plt res
	0,					// chr res(未使用)
	RESID_TYPE_CEL,		// cel res
	//
	PKWD_BOOK_TYPE_OBJ_MAXNUM,		// max
	CLWKMODE_TYPE,
};

// OBJのテーブルデータ
const PKWD_BOOK_OBJ_DATA pwb_obj_type_tbl[] = {
	{ 0,	0x30,	1,	0,	0,	1,	11,	RESID_TYPE_CHR_00,},		// タイプ00
	{ 0,	0x30,	1,	0,	0,	1,	12,	RESID_TYPE_CHR_01,},		// タイプ01
	{ 0,	0x30,	1,	0,	0,	1,	13,	RESID_TYPE_CHR_10,},		// 技タイプ00
	{ 0,	0x30,	1,	0,	0,	1,	14,	RESID_TYPE_CHR_11,},		// 技タイプ01
	{ 0,	0x30,	1,	0,	0,	1,	15,	RESID_TYPE_CHR_12,},		// 技タイプ02
	{ 0,	0x30,	1,	0,	0,	1,	16,	RESID_TYPE_CHR_13,},		// 技タイプ03
	
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// OBJ(trainer)
//----------------
// 情報
static const s16 pwb_obj_trainer_itbl[] = {
	RESID_TRAINER_PLT,		// plt res
	0,						// chr res(未使用)
	RESID_TRAINER_CEL,		// cel res
	//
	PKWD_BOOK_TRAINER_OBJ_MAXNUM,		// max
	CLWKMODE_TRAINER,
};

// OBJのテーブルデータ
const PKWD_BOOK_OBJ_DATA pwb_obj_trainer_tbl[] = {
	{ 0,	0x30, 1, 0,	0,	1,	17,	RESID_TRAINER_CHR,},		// トレーナー
	
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// BMPOAMの情報
//------------------
static const PKWD_BOOK_BMPOAM_DATA pwb_bmpoam_tbl[] = {
	{ 2,	2,	0x38,	1,	1,	0,	4,	0,},		// ポケモン00
	{ 2,	2,	0x38,	1,	1,	0,	5,	0,},
	{ 2,	2,	0x38,	1,	1,	0,	6,	0,},
	{ 2,	2,	0x38,	1,	1,	0,	7,	0,},
	{ 2,	2,	0x38,	1,	1,	0,	8,	0,},
	{ 2,	2,	0x38,	1,	1,	0,	9,	0,},
	
	{ 2,	2,	0x38,	1,	1,	0,	20,	0,},
	{ 2,	2,	0x38,	1,	1,	0,	21,	0,},
	{ 2,	2,	0x38,	1,	1,	0,	22,	0,},

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//---------------
// CANVASの設定
//---------------
// 上画面
static const PKWD_BOOK_CANVAS_DATA pwb_canvas_00_tbl[] = {
	{ GFL_BG_FRAME1_M,	2,	1,	28,	2,	10,},		// TITLE
	{ GFL_BG_FRAME1_M,	2,	5,	28,	2,	10,},		// PLOT
	{ GFL_BG_FRAME1_M,	2,	12,	28,	2,	10,},		// GOAL
	{ GFL_BG_FRAME1_M,	3,	3,	28,	2,	10,},		// TITLE_MES
	{ GFL_BG_FRAME1_M,	3,	7,	28,	4,	10,},		// PLOT_MES
	{ GFL_BG_FRAME1_M,	3,	14,	28,	4,	10,},		// GOAL_MES

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};
// BOOK(test)
static const PKWD_BOOK_CANVAS_DATA pwb_canvas_01_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	1,	28,	2,	10,},		// CAST

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};
// トレーナー
static const PKWD_BOOK_CANVAS_DATA pwb_canvas_02_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	1,	28,	2,	10,},		// CAST
	{ GFL_BG_FRAME1_S,	1,	13,	12,	2,	10,},		// 役名(fix)
	{ GFL_BG_FRAME1_S,	1,	15,	12,	2,	10,},		// 職業名
	{ GFL_BG_FRAME1_S,	1,	17,	12,	2,	10,},		// 職業

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};
// ポケ情報
static const PKWD_BOOK_CANVAS_DATA pwb_canvas_03_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	1,	28,	2,	10,},		// CAST
	{ GFL_BG_FRAME1_S,	1,	6,	8,	2,	10,},		// 名前
	{ GFL_BG_FRAME1_S,	9,	6,	1,	2,	10,},		// 性別
	{ GFL_BG_FRAME1_S,	2,	11,	10,	2,	10,},		// レベル
	{ GFL_BG_FRAME1_S,	1,	15,	12,	2,	10,},		// 持ち物(fix)
	{ GFL_BG_FRAME1_S,	1,	17,	12,	2,	10,},		// 持ち物2
	{ GFL_BG_FRAME1_S,	16,	4,	12,	2,	10,},		// 技00
	{ GFL_BG_FRAME1_S,	16,	7,	12,	2,	10,},		// 技01
	{ GFL_BG_FRAME1_S,	16,	10,	12,	2,	10,},		// 技02
	{ GFL_BG_FRAME1_S,	16,	13,	12,	2,	10,},		// 技03
	{ GFL_BG_FRAME1_S,	13,	15,	12,	2,	10,},		// 特性(fix)
	{ GFL_BG_FRAME1_S,	13,	17,	12,	2,	10,},		// 特性2

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

// (canvas)まとめ
static const PKWD_BOOK_CANVAS_DATA *pwb_canvas_tbl[] = {
	pwb_canvas_01_tbl,
	pwb_canvas_02_tbl,
	pwb_canvas_03_tbl,
};

//------------------------
// スクリーンのオフセット
//------------------------
static const s16 pwb_bgscr_tbl[] = {
	0,   0,		// 基本
	256, 0,		// 右
	0,  192,	// 下
};

//-------------
// fixメッセージ
//--------------
static const s16 pwb_fix_mestbl[] = {
	pw_topindex_01,			// TOP:TITLE映画の名前
	pw_topindex_02,			// TOP:PLOTあらすじ
	pw_topindex_03,			// TOP:GOALクリア目標
	pw_castindex_03,		// INFO:キャスト共演者
	pw_castindex_04,		// INFO:もちもの
	pw_castindex_05,		// INFO:とくせい
};

//-------------
// fixメッセージ
//-------------
// main(上)
static const u16 pwb_fmes00_tbl[] = {
	MES_ID_TOP_01,	PWB_CVS_TOP_TITLE, 	0, 0,		// タイトル
	MES_ID_TOP_02,	PWB_CVS_TOP_PLOT, 	0, 0,		// PLOT
	MES_ID_TOP_03,	PWB_CVS_TOP_GOAL, 	0, 0,		// GOAL
	//
	0xffff, 0, 0, 0,		// end
};
// 台本選択(保留)
static const u16 pwb_fmes10_tbl[] = {
	MES_ID_INFO_01,	PWB_CVS_BOOK_TITLE,	0, 0,		// キャスト
	//
	0xffff, 0, 0, 0,		// end
};
// トレーナー
static const u16 pwb_fmes11_tbl[] = {
	MES_ID_INFO_01,	PWB_CVS_TRAINER_TITLE,	1, 0,	// キャスト
	//
	MES_ID_INFO_01, PWB_CVS_TRAINER_NAME,  1, 0,	// キャスト、やくめい(test)
	MES_ID_INFO_02,	PWB_CVS_TRAINER_WORK,  1, 0,	// もちもの、職業名(test)
	MES_ID_INFO_03,	PWB_CVS_TRAINER_WORKER,1, 0,	// とくせい、役職名(test)
	//
	0xffff, 0, 0, 0,		// end
};
// ポケモン
static const u16 pwb_fmes12_tbl[] = {
	MES_ID_INFO_01,	PWB_CVS_INFO_CAST,	 1, 0,		// キャスト
	MES_ID_INFO_02,	PWB_CVS_INFO_ITEM,	 1, 0,		// もちもの
	MES_ID_INFO_03,	PWB_CVS_INFO_STATUS, 1, 0,		// とくせい
	//
	0xffff, 0, 0, 0,		// end
};

// fixメッセージ(まとめ)
static const u16 *pwb_fmes_tbl[] = {
	pwb_fmes00_tbl,
	//
	pwb_fmes10_tbl,
	pwb_fmes11_tbl,
	pwb_fmes12_tbl,
};

//--------------------
// メッセージ(要修正)
//--------------------
static const s16 pwb_info_tbl[] = {
	pw_castindex_06,		// 名前
	pw_castindex_07,		// 男性
	pw_castindex_08,		// 女性
	pw_castindex_09,		// レベル
	pw_castindex_10,		// もちもの
	pw_castindex_11,		// とくせい
	pw_castindex_12,		// 技00
	pw_castindex_13,		// 技01
	pw_castindex_14,		// 技02
	pw_castindex_15,		// 技03
	//
	pw_castindex_16,		// slash
};

//--------------
// TOPメッセージ
//--------------
static const s16 pwb_mes_tbl[] = {
	pw_title_00,			// タイトル
	pw_plot_000,			// プロット
	pw_goal_00,				// ゴール
};

//--------------
// カーソル情報
//--------------
static const s16 pwb_curpos_tbl[] = {
	0, 1,			// 通常
	18,19,			// ポケモン情報時
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT PkwdBookProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PkwdBookProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PkwdBookProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
///	以下に追加
//=====================================
static void pwb_init( PKWD_BOOK_MAIN_WORK* wk );
static void pwb_main( PKWD_BOOK_MAIN_WORK* wk );
static void pwb_exit( PKWD_BOOK_MAIN_WORK* wk );
static void pwb_down( PKWD_BOOK_MAIN_WORK* wk );
static void pwb_key_all( PKWD_BOOK_MAIN_WORK* wk );
//
// emptyの設定
static s16  pwb_get_empty( PKWD_BOOK_MAIN_WORK *wk, const s32 num );
//
// bgの設定
static void pwb_init_bg( PKWD_BOOK_MAIN_WORK* wk );
static void pwb_init_bg2( PKWD_BOOK_MAIN_WORK* wk, const s32 type, const s32 pnum );
//
// objの設定
static void pwb_init_obj_res_font( PKWD_BOOK_MAIN_WORK* wk, const s32 drawtype );
static void pwb_init_obj_res_cmn( PKWD_BOOK_MAIN_WORK* wk, const s32 drawtype );
static void pwb_init_obj_res_poke( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_chg_poke_obj( PKWD_BOOK_MAIN_WORK * wk, const s32 wnum, const s32 id, const s32 sex );
static void pwb_init_obj_res_type( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_chg_type_obj( PKWD_BOOK_MAIN_WORK * wk, const s32 wnum, const s32 mode, const u32 type );
static void pwb_init_obj_res_trainer( PKWD_BOOK_MAIN_WORK* wk, const s32 drawtype, const s32 id );
// 
static void pwb_init_obj_clwk( PKWD_BOOK_MAIN_WORK* wk, const s32 drawtype, const PKWD_BOOK_OBJ_DATA *obj_adr, const s16 *info_adr );
static void pwb_init_obj( PKWD_BOOK_MAIN_WORK* wk );
static void pwb_exit_obj( PKWD_BOOK_MAIN_WORK* wk );
static void pwb_set_obj_visible( PKWD_BOOK_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
static void pwb_chg_objseq( PKWD_BOOK_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
static BOOL pwb_is_anime_end( PKWD_BOOK_MAIN_WORK *wk, const s32 wmode, const s32 wnum );
//
static void pwb_init_obj_bmpoam( PKWD_BOOK_MAIN_WORK* wk, const s32 drawtype, const PKWD_BOOK_BMPOAM_DATA *bmp_adr, const u32 resid );
static void pwb_disp_bmpoam( PKWD_BOOK_MAIN_WORK* wk, const s32 wnum, const STRBUF *strbuf );
static void pwb_set_bmpoam_visible( PKWD_BOOK_MAIN_WORK* wk, const s32 wnum, const BOOL bflg );

//
// 文字列表示
static void pwb_init_string( PKWD_BOOK_MAIN_WORK* wk, const s32 type, const s32 mode );
static void pwb_exit_string( PKWD_BOOK_MAIN_WORK* wk, const s32 type );
static void pwb_main_print( PKWD_BOOK_MAIN_WORK *wk );
static void pwb_disp_string( PKWD_BOOK_MAIN_WORK* wk, const s32 type, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode,
								const PRINTSYS_LSB col );
static u16  pwb_get_putsize( const STRBUF * buf, GFL_FONT * font, u32 mode );
static void pwb_clear_canvas( PKWD_BOOK_MAIN_WORK* wk, const s32 type, const s32 mode );
static void pwb_disp_title_string( PKWD_BOOK_MAIN_WORK* wk );
static void pwb_disp_fix_string( PKWD_BOOK_MAIN_WORK * wk, const s32 type, const s32 mode );
static void pwb_chk_canvas( PKWD_BOOK_MAIN_WORK * wk );
// bgスクロール
static void pwb_set_bgscroll( PKWD_BOOK_MAIN_WORK* wk, const s32 mode );
//----------------
// 以下、各アプリ
//----------------
// 台本選択
static void pwb_init_book( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_main_book( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_exit_book( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_key_book( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_disp_book_string( PKWD_BOOK_MAIN_WORK * wk, const s32 mode );
//
// トレーナー
static void pwb_init_trainer( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_main_trainer( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_exit_trainer( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_key_trainer( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_disp_trainer_string( PKWD_BOOK_MAIN_WORK * wk, const s32 mode );
static void pwb_disp_trainer_obj( PKWD_BOOK_MAIN_WORK * wk, const s32 mode );
//
// ポケモン詳細
static void pwb_init_poke( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_main_poke( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_exit_poke( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_key_poke( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_disp_poke_string( PKWD_BOOK_MAIN_WORK * wk, const s32 mode );
static void pwb_disp_poke_obj( PKWD_BOOK_MAIN_WORK * wk, const s32 mode );
//
// クリア(共通)
static void pwb_clear_obj( PKWD_BOOK_MAIN_WORK * wk, const s32 mode );

//
// main処理
static BOOL pwb_main_loop( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_chk_end( PKWD_BOOK_MAIN_WORK * wk );
//
static void pwb_chk_infocurpos( PKWD_BOOK_MAIN_WORK * wk, const s32 mode );
static void pwb_disp_infocurpos( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_chg_cmn_cursor( PKWD_BOOK_MAIN_WORK * wk, const s32 mode );

// party情報
static void pwb_make_party( PKWD_BOOK_MAIN_WORK * wk, const s32 scenario_id );
static void pwb_delete_party( PKWD_BOOK_MAIN_WORK * wk );
static void pwb_set_pokework( PKWD_BOOK_MAIN_WORK * wk );


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA PkwdBookProcData = 
{
	PkwdBookProc_Init,
	PkwdBookProc_Main,
	PkwdBookProc_Exit,
};

//=============================================================================
//=============================================================================
//------------------
// 関数のテーブル
//------------------
static void (*(pwb_functbl[]))(PKWD_BOOK_MAIN_WORK *) = {
	pwb_init_book,			// book(init)
	pwb_main_book,			// book(main)
	pwb_exit_book,			// book(exit)
	pwb_init_trainer,		// trainer(init)
	pwb_main_trainer,		// trainer(main)
	pwb_exit_trainer,		// trainer(exit)
	pwb_init_poke,			// poke(init)
	pwb_main_poke,			// poke(main)
	pwb_exit_poke,			// poke(exit)
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
 *	@param	*pwk					PKWD_BOOK_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdBookProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_BOOK_MAIN_WORK 	*wk;
	
	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PKWD_BOOK, UI_TEMPLATE_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(PKWD_BOOK_MAIN_WORK), HEAPID_PKWD_BOOK );
	GFL_STD_MemClear( wk, sizeof(PKWD_BOOK_MAIN_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_PKWD_BOOK;
	
	// 参照用
	wk->Param = pwk;
	//
	//描画設定初期化(上:メイン、下:サブ)
	wk->graphic	= PKWD_BOOK_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, wk->heapID );
	//
	// フォント作成
	wk->font  = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	wk->sfont = GFL_FONT_Create( ARCID_FONT, NARC_font_num_gftr,   GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	//
	// PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	//
	// WORDSET生成
	wk->Wordset = WORDSET_Create( wk->heapID );
	
	//----------------
	// 通常はこの中で初期化
	//----------------
	pwb_init( wk );
	
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
 *	@param	*pwk					PKWD_BOOK_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdBookProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_BOOK_MAIN_WORK* wk = mywk;
	
	//----------------
	// 処理を記述
	//----------------
	pwb_exit( wk );
	
	// WORDSET破棄
	WORDSET_Delete( wk->Wordset );
	
	// PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	// FONT
	GFL_FONT_Delete( wk->font );
	GFL_FONT_Delete( wk->sfont );
	
	//描画設定破棄
	PKWD_BOOK_GRAPHIC_Exit( wk->graphic );
	
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
 *	@param	*pwk					PKWD_BOOK_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdBookProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_BOOK_MAIN_WORK* wk = mywk;
	
	switch ( *seq )
	{
		case 0:
			if (WIPE_SYS_EndCheck())
			{
				(*seq) ++;
				// wipe終了
				wk->Flag |= FLG_WIPE_IN_END;
			}
			break;
		case 1:
			if( wk->Step == STEP_END )
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
				return GFL_PROC_RES_FINISH;
			}
			break;
		default:
			break;
	}
	
	//----------------
	// 処理を追加
	//----------------
	pwb_main( wk );
	
	// Queの処理
	pwb_main_print( wk );
	
	//2D描画
	PKWD_BOOK_GRAPHIC_2D_Draw( wk->graphic );
	
	//3D描画
	PKWD_BOOK_GRAPHIC_3D_StartDraw( wk->graphic );
	
	PKWD_BOOK_GRAPHIC_3D_EndDraw( wk->graphic );
	
	return GFL_PROC_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(init)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_init( PKWD_BOOK_MAIN_WORK* wk )
{
	s32		i;
	
	//----------
	// 初期化
	//----------
	wk->Step 		= STEP_INIT;
	wk->Mode		= MODE_INIT_BOOK;
	wk->SubMode		= STEP_INIT;
	wk->TitleMode	= 0;
	wk->NextMode	= 0;
	wk->Flag		= 0;
	wk->PFlag		= 0;
	wk->PFlag2		= 0;
	wk->ScenarioCurPos = 0;
	wk->ScenarioNum	= 0;
	wk->InfoCurPos	= 0;
	wk->Party		= 0;
	wk->PokeMax		= 1;
	wk->PokePageNum	= 0;
	//
	// canvas
	for(i=0;i<PKWD_BOOK_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
		wk->PrintUtil2[i].win = 0;
	}
	//---------------------
	// empty用バッファ取得
	//(必ず破棄すること)
	//---------------------
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_B_SCENARIO_GRA, NARC_b_scenario_gra_pkwd_book_pos_tblbin, wk->heapID );
	
	//---------------------
	// メッセージバッファ取得
	//---------------------
	wk->MsgData[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pw_movietitle_dat, wk->heapID );
	wk->MsgData[1] = 0;
	
	//---------------------
	// パーティ情報セット(test)
	//---------------------
	pwb_make_party( wk, wk->ScenarioNum );
	
	//---------------
	// bgの初期化
	//---------------
	pwb_init_bg(  wk );
	pwb_init_bg2( wk, GFL_BG_FRAME2_S, PLTID_SUB_BG_CMN );
	
	//------------------------
	// フォントパレットセット
	//------------------------
	// パレットは10番に転送
	{
		ARCHANDLE* p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_SUB_BG,  10*0x20, 0x20, wk->heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, 10*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
	
	//---------------
	// stringの初期化
	//---------------
	pwb_init_string( wk, STR_MAIN, 0 );				// 1回のみ
	pwb_init_string( wk, STR_SUB,  TMODE_BOOK );
	
	// fix文字列
	pwb_disp_fix_string( wk, STR_MAIN, 0 );
	//
	pwb_disp_book_string( wk, 0 );
	
	//------------
	// OBJ初期化
	//------------
	pwb_init_obj( wk );
	//
	//------------
	// パレット
	//------------
//	pwb_init_pal_fade( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_main( PKWD_BOOK_MAIN_WORK* wk )
{
	// 下画面
	pwb_down( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(exit)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_exit( PKWD_BOOK_MAIN_WORK* wk )
{
	s32		i;
	
	// party情報の破棄
	pwb_delete_party( wk );
	
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	
	// stringの破棄
	pwb_exit_string( wk, STR_MAIN );
	pwb_exit_string( wk, STR_SUB );
	
	//メッセージ破棄
	for(i=0;i<PKWD_BOOK_MES_MAX;i++){
		if(wk->MsgData[i]){
			GFL_MSG_Delete( wk->MsgData[i] );
			wk->MsgData[i] = 0;
		}
	}
	
	// OBJ破棄
	pwb_exit_obj( wk );
	// パレット破棄
//	pwb_exit_pal_fade( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(下画面)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_down( PKWD_BOOK_MAIN_WORK* wk )
{
	// 入力
	pwb_key_all( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理まとめ(処理の流れmain)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_key_all( PKWD_BOOK_MAIN_WORK* wk )
{
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_INIT+10;
			break;
		//
		// wipe待ち
		case STEP_INIT+10:
			// wipe終了を待ってから開始
			if(wk->Flag & FLG_WIPE_IN_END){
				wk->Step = STEP_LV1;
			}
			break;
		//
		// 本体
		case STEP_LV1:
			bflg = pwb_main_loop( wk );
			if(bflg == FALSE){
				wk->Step = STEP_END;
			}
			break;
		//
		// END
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  emptyの取得
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
static s16 pwb_get_empty( PKWD_BOOK_MAIN_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= PKWD_BOOK_EMPTY_MAXNUM)){
		OS_Printf("empty error :%d\n",num);
		return(0);
	}
	
	adr = wk->EmptyBuffer;
	return( adr[num] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_init_bg( PKWD_BOOK_MAIN_WORK* wk )
{
	u16		*adr;
	s32		i,j,k;
	s32		tmp;
	u32		off;
	const PKWD_BOOK_BGPAL_DATA	*padr = &pwc_bgpal_tbl[0];
	const PKWD_BOOK_BGCHR_DATA	*cadr = &pwc_bgchr_tbl[0];
	const PKWD_BOOK_BGSCN_DATA	*sadr = &pwc_bgscn_tbl[0];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_B_SCENARIO_GRA, wk->heapID );
	
	// パレット転送
	for(i=0;i<PKWD_BOOK_BGNUM_MAXNUM;i++){
		// 終了条件
		if(padr->type == 0xff){
			break;
		}
		tmp = padr->size;
		tmp *= 0x20;		// 転送サイズ
		off = padr->offset;
		off *= 0x20;
		GFL_ARCHDL_UTIL_TransVramPalette( handle, padr->arcid, padr->type, off, tmp, wk->heapID );
		//
		padr++;
	}
	//
	// キャラ転送
	for(i=0;i<PKWD_BOOK_BGNUM_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, wk->heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<PKWD_BOOK_BGNUM_MAXNUM;i++){
		// 終了条件
		if(sadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramScreen( handle, sadr->arcid, sadr->frmnumber, sadr->offset, 0, 0, wk->heapID );
		sadr++;
	}
	// 終了
	GFL_ARC_CloseDataHandle( handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化(cmn)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32			(BG面)
 *	@param	const s32			(パレット番号)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_init_bg2( PKWD_BOOK_MAIN_WORK* wk, const s32 type, const s32 pnum )
{
	u16			*adr;
	s32			i;
	s32			j;
	u32			id;
	// cmnのarcidを取得
	const u32 	arcid    = APP_COMMON_GetArcId();
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( arcid, wk->heapID );
	
	// パレット
	id = APP_COMMON_GetBarPltArcIdx();
	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, id, PALTYPE_SUB_BG, pnum*0x20, 0x20, wk->heapID );
	//
	// キャラクタ
	id = APP_COMMON_GetBarCharArcIdx();
	GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, id, type, 0, 0, 0, wk->heapID );
	//
	// スクリーン
	id = APP_COMMON_GetBarScrnArcIdx();
	GFL_ARCHDL_UTIL_TransVramScreen( p_handle, id, type, 0, 0, 0, wk->heapID );
	//
	// スクリーンバッファ指定
	adr = GFL_BG_GetScreenBufferAdrs( type );
	// 1画面分修正
	for(i=0;i<24;i++){
		for(j=0;j<32;j++){
			// オフセットを加算
			adr[i*32+j] += ((pnum&0x0f)<<12);
		}
	}
	GFL_BG_LoadScreenReq( type );
	//
	GFL_ARC_CloseDataHandle( p_handle );
	//
	// 背景色をセット
//	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  font
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_init_obj_res_font( PKWD_BOOK_MAIN_WORK* wk, const s32 drawtype )
{
	// フォントのパレットは圧縮しているので注意
	ARCHANDLE* p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
	wk->wk_obj.res[ RESID_FONT_PLT ] = GFL_CLGRP_PLTT_RegisterComp( p_handle, NARC_font_talkwin_nclr, drawtype, 0x20*PLTID_SUB_OBJ_FONT, wk->heapID );
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(cmn res)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_init_obj_res_cmn( PKWD_BOOK_MAIN_WORK* wk, const s32 drawtype )
{
	// cmnのidを取得
	const u32 	arcid    = APP_COMMON_GetArcId();
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( arcid, wk->heapID );
    GFL_CLUNIT*	p_clunit = PKWD_BOOK_GRAPHIC_GetClunit( wk->graphic );
	u32			idx[4];
	//
	idx[0] = APP_COMMON_GetBarIconPltArcIdx();		// pal
	idx[1] = APP_COMMON_GetBarIconCharArcIdx();		// ncg
	idx[2] = APP_COMMON_GetBarIconCellArcIdx(  APP_COMMON_MAPPING_128K );	// cell
	idx[3] = APP_COMMON_GetBarIconAnimeArcIdx( APP_COMMON_MAPPING_128K );	// anim
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ RESID_CMN_OBJ_PLT ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, idx[0], drawtype, 0x20*4, 0, 4, wk->heapID );
	wk->wk_obj.res[ RESID_CMN_OBJ_CHR ] = GFL_CLGRP_CGR_Register(      p_handle, idx[1], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ RESID_CMN_OBJ_CEL ] = GFL_CLGRP_CELLANIM_Register( p_handle, idx[2], idx[3], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ポケモンアイコンのグラフィックロード
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_init_obj_res_poke( PKWD_BOOK_MAIN_WORK * wk )
{
	ARCHANDLE	*ah;
	u32			i;
	u32			buf[2];
	POKEMON_PARAM	*pp;
	
	ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, GFL_HEAP_LOWID(wk->heapID) );
	// パレット(0-3)
	wk->wk_obj.res[RESID_POKE_PLT] = GFL_CLGRP_PLTT_RegisterComp( ah, 
												POKEICON_GetPalArcIndex(),
												CLSYS_DRAW_SUB, 0, wk->heapID );
	// セル・アニメ
	wk->wk_obj.res[RESID_POKE_CEL] = GFL_CLGRP_CELLANIM_Register( ah,
												POKEICON_GetCellSubArcIndex(),
												POKEICON_GetAnmSubArcIndex(),
												wk->heapID );
	// キャラ
	for(i=0;i<7;i++){
//		if(i < wk->PokeMax){
			// ポケモン情報
//			pp = PokeParty_GetMemberPointer( wk->dat->party, i );
			// id
//			buf[0] = wk->PokeWork[i].id;
//			buf[0] = 498;
			// sex
//			buf[1] = wk->PokeWork[i].sex;
//			buf[1] = 0;
			//
//			wk->wk_obj.res[RESID_POKE_CHR_00+i] = GFL_CLGRP_CGR_Register( ah,
//												POKEICON_GetCgxArcIndexByMonsNumber( buf[0], 1, buf[1], FALSE ),
//												FALSE, CLSYS_DRAW_SUB, wk->heapID );
//		}else{
			wk->wk_obj.res[RESID_POKE_CHR_00+i] = GFL_CLGRP_CGR_Register( ah,
												POKEICON_GetCgxArcIndexByMonsNumber( 0, 0, 0, 0 ),
												FALSE, CLSYS_DRAW_SUB, wk->heapID );
//		}
	}
	//
	GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ポケモンアイコン切り替え
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 * @param	const s32			ワーク番号(reschrと対応)
 * @param	const s32			id
 * @param	const s32			性別
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_chg_poke_obj( PKWD_BOOK_MAIN_WORK * wk, const s32 wnum, const s32 id, const s32 sex )
{
	NNSG2dCharacterData * dat;
	void * buf;
	ARCHANDLE * ah;
	u8 pal_num = POKEICON_GetPalNum( id, 0, sex, FALSE );
	
	ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, GFL_HEAP_LOWID(wk->heapID) );
	//
	// CGRを置き換える方法
	buf = GFL_ARCHDL_UTIL_LoadOBJCharacter( ah, POKEICON_GetCgxArcIndexByMonsNumber( id, 1, sex, FALSE ), FALSE, &dat, GFL_HEAP_LOWID(wk->heapID) );
	//
	GFL_CLGRP_CGR_Replace( wk->wk_obj.res[RESID_POKE_CHR_00+wnum], dat );
	GFL_HEAP_FreeMemory( buf );
	//
	GFL_ARC_CloseDataHandle( ah );
	//
	// パレット変更(転送した番号(読み込ませた際にセットされる) + offset)
	GFL_CLACT_WK_SetPlttOffs( wk->wk_obj.clwk_poke[wnum], pal_num, CLWK_PLTTOFFS_MODE_PLTT_TOP );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	タイプ/分類アイコンobjの読み込み
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_init_obj_res_type( PKWD_BOOK_MAIN_WORK * wk )
{
	ARCHANDLE	*ah;
	u32			i;
	
	ah = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, GFL_HEAP_LOWID(wk->heapID) );
	// パレット(8-10)
	wk->wk_obj.res[RESID_TYPE_PLT] = GFL_CLGRP_PLTT_RegisterEx( ah,
												NARC_app_menu_common_p_st_type_NCLR,
												CLSYS_DRAW_SUB, PLTID_SUB_OBJ_TYPE*0x20, 0, 3, wk->heapID );
	// セル・アニメ
	wk->wk_obj.res[RESID_TYPE_CEL] = GFL_CLGRP_CELLANIM_Register( ah,
												NARC_app_menu_common_p_st_type_128k_NCER,
												NARC_app_menu_common_p_st_type_128k_NANR,
												wk->heapID );
	// タイプアイコンキャラ
	// (技タイプもこれでよいらしい)
	for(i=0;i<PKWD_BOOK_TYPE_OBJ_MAXNUM;i++){
		wk->wk_obj.res[RESID_TYPE_CHR_00+i] = GFL_CLGRP_CGR_Register( ah, APP_COMMON_GetPokeTypeCharArcIdx(0), FALSE, CLSYS_DRAW_SUB, wk->heapID );
	}
	//
	GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	タイプアイコン切り替え(技タイプも同じでよいらしい)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 * @param	const s32			ワーク番号(reschrと対応)
 * @param	const s32			(0:タイプ、1:技タイプ)
 * @param	const s32			タイプ値
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_chg_type_obj( PKWD_BOOK_MAIN_WORK * wk, const s32 wnum, const s32 mode, const u32 type )
{
	NNSG2dCharacterData * dat;
	void * buf;
	ARCHANDLE * ah;
	
	ah = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, GFL_HEAP_LOWID(wk->heapID) );
	//
	// CGRを置き換える方法
	buf = GFL_ARCHDL_UTIL_LoadOBJCharacter( ah, APP_COMMON_GetPokeTypeCharArcIdx(type), FALSE, &dat, GFL_HEAP_LOWID(wk->heapID) );
	//
	GFL_CLGRP_CGR_Replace( wk->wk_obj.res[RESID_TYPE_CHR_00+wnum], dat );
	GFL_HEAP_FreeMemory( buf );
	//
	GFL_ARC_CloseDataHandle( ah );
	// パレット変更(転送した番号(読み込ませた際にセットされる) + offset)
	GFL_CLACT_WK_SetPlttOffs( wk->wk_obj.clwk_type[wnum], APP_COMMON_GetPokeTypePltOffset(type), CLWK_PLTTOFFS_MODE_PLTT_TOP );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(トレーナーデータ専用)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s32			id
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_init_obj_res_trainer( PKWD_BOOK_MAIN_WORK* wk, const s32 drawtype, const s32 id )
{
	ARCHANDLE*	p_handle = TR2DGRA_OpenHandle( wk->heapID );
	//
	wk->wk_obj.res[ RESID_TRAINER_PLT ] = TR2DGRA_OBJ_PLTT_Register( p_handle, id, drawtype, PLTID_SUB_OBJ_TRAINER*0x20, wk->heapID );
	wk->wk_obj.res[ RESID_TRAINER_CHR ] = TR2DGRA_OBJ_CGR_Register(  p_handle, id, drawtype, wk->heapID );
	wk->wk_obj.res[ RESID_TRAINER_CEL ] = TR2DGRA_OBJ_CELLANM_Register( id, APP_COMMON_MAPPING_128K, drawtype, wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32					drawtype
 *	@param	const PKWD_BOOK_OBJ_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_init_obj_clwk( PKWD_BOOK_MAIN_WORK* wk, const s32 drawtype, const PKWD_BOOK_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16						tmp;
	s16						pos[2];
	s16						buf[6];
	s32 					i;
	BOOL					bflg;
	GFL_CLWK_DATA 			cldata;
	CLWK_FLIP_TYPE			flip;
    GFL_CLUNIT*				p_clunit 	= PKWD_BOOK_GRAPHIC_GetClunit( wk->graphic );
	// アドレスセット
	const PKWD_BOOK_OBJ_DATA *adr 	= obj_adr;
	const s16 				*iadr 	= info_adr;
	GFL_CLWK				**clwk;			// アドレス注意
	
	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
	// チェック
	buf[0] = iadr[0];		// plt
	buf[1] = iadr[1];		// chr(未使用)
	buf[2] = iadr[2];		// cel
	buf[3] = iadr[3];		// max
	buf[4] = iadr[4];		// mode
	//
	// アドレスセット
	// アドレスセット
	if(buf[4] == CLWKMODE_TRAINER){			// trainer
		clwk = &wk->wk_obj.clwk_tr[0];
	}
	else if(buf[4] == CLWKMODE_CMN){		// cmn
		clwk = &wk->wk_obj.clwk_cmn[0];
	}
	else if(buf[4] == CLWKMODE_POKE){		// poke
		clwk = &wk->wk_obj.clwk_poke[0];
	}
	else if(buf[4] == CLWKMODE_TYPE){		// type
		clwk = &wk->wk_obj.clwk_type[0];
	}
	
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		// 決めうち
		pos[0] = pwb_get_empty(wk, tmp*2);
		pos[1] = pwb_get_empty(wk, tmp*2+1);
		cldata.pos_x 	= pos[0];
		cldata.pos_y 	= pos[1];
		cldata.anmseq 	= adr->anmseq;
		cldata.softpri	= adr->softpri;
		cldata.bgpri	= adr->bgpri;
		//
		clwk[i] =   GFL_CLACT_WK_Create( p_clunit, 
											wk->wk_obj.res[ adr->res_cnum ],	// chr
											wk->wk_obj.res[ buf[0] ],			// plt
											wk->wk_obj.res[ buf[2] ],			// cel
											&cldata,
											drawtype,
											wk->heapID );
		//-------------
		// 表示/非表示
		//-------------
		bflg = FALSE;
		if(adr->visible == 1){
			bflg = TRUE;
		}
		GFL_CLACT_WK_SetDrawEnable( clwk[i], bflg );
		//----------
		// フラグ
		//----------
		// アニメ
		bflg = FALSE;
		if(adr->dispflag & FLG_OBJ_ANIM){
			bflg = TRUE;
		}
		GFL_CLACT_WK_SetAutoAnmFlag( clwk[i], bflg );
		//
		// Hフリップ
		if(adr->dispflag & FLG_OBJ_HFLIP){
			GFL_CLACT_WK_SetFlip( clwk[i], CLWK_FLIP_H, TRUE );
		}
		// Vフリップ
		if(adr->dispflag & FLG_OBJ_VFLIP){
			GFL_CLACT_WK_SetFlip( clwk[i], CLWK_FLIP_V, TRUE );
		}
		// 半透明のテスト
//		GFL_CLACT_WK_SetObjMode( clwk[i], GX_OAM_MODE_XLU );
		
		// 次のデータへ
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化まとめ
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_init_obj( PKWD_BOOK_MAIN_WORK* wk )
{
	GFL_CLUNIT*	p_clunit = PKWD_BOOK_GRAPHIC_GetClunit( wk->graphic );
	//
	// リソース読み込み(cmn)
	pwb_init_obj_res_cmn( wk, CLSYS_DRAW_SUB );
	// リソース読み込み(poke)
	pwb_init_obj_res_poke( wk );
	// リソース読み込み(poke)
	pwb_init_obj_res_type( wk );
	// リソース読み込み(trainer)
	pwb_init_obj_res_trainer( wk, CLSYS_DRAW_SUB, 1 );
	// font
	pwb_init_obj_res_font( wk, CLSYS_DRAW_SUB );
	//
	// CLWK登録(cmn)
	pwb_init_obj_clwk( wk, CLSYS_DRAW_SUB, &pwb_obj_cmn_tbl[0], &pwb_obj_cmn_itbl[0] );
	// CLWK登録(cmn)
	pwb_init_obj_clwk( wk, CLSYS_DRAW_SUB, &pwb_obj_poke_tbl[0], &pwb_obj_poke_itbl[0] );
	// CLWK登録(type)
	pwb_init_obj_clwk( wk, CLSYS_DRAW_SUB, &pwb_obj_type_tbl[0], &pwb_obj_type_itbl[0] );
	// CLWK登録(trainer)
	pwb_init_obj_clwk( wk, CLSYS_DRAW_SUB, &pwb_obj_trainer_tbl[0], &pwb_obj_trainer_itbl[0] );
	//
	// 通信アイコンセット(上画面)
	GFL_NET_ReloadIconTopOrBottom( TRUE, wk->heapID );
	//
	// BmpOAM初期化
	wk->BmpOamSys = BmpOam_Init( wk->heapID, p_clunit );
	// BMPOAM登録
	pwb_init_obj_bmpoam( wk, CLSYS_DRAW_SUB, &pwb_bmpoam_tbl[0], RESID_FONT_PLT );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_exit_obj( PKWD_BOOK_MAIN_WORK* wk )
{
	s32		i;
	
	// cmn
	for(i=0;i<PKWD_BOOK_CMN_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk_cmn[i] );
	}
	// poke
	for(i=0;i<PKWD_BOOK_POKE_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk_poke[i] );
	}
	// type
	for(i=0;i<PKWD_BOOK_TYPE_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk_type[i] );
	}
	// trainer
	for(i=0;i<PKWD_BOOK_TRAINER_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk_tr[i] );
	}
	//
	//リソース破棄(cmn)
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_CMN_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_CMN_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_CMN_OBJ_CEL] );
	//
	//リソース破棄(poke)
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_POKE_PLT] );
	for(i=0;i<PKWD_BOOK_POKE_OBJ_MAXNUM;i++){
		GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_POKE_CHR_00+i] );
	}
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_POKE_CEL] );
	//
	//リソース破棄(type)
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_TYPE_PLT] );
	for(i=0;i<PKWD_BOOK_TYPE_OBJ_MAXNUM;i++){
		GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_TYPE_CHR_00+i] );
	}
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_TYPE_CEL] );
	//
	//リソース破棄(trainer)
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_TRAINER_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_TRAINER_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_TRAINER_CEL] );
	//
	//リソース破棄(font)
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_FONT_PLT] );
	//
	// BmpOAM後処理
	for(i=0;i<PKWD_BOOK_BMPOAM_MAXNUM;i++){
		BmpOam_ActorDel( wk->BmpOamWork[i] );
		GFL_BMP_Delete( wk->BmpData[i] );
	}
	BmpOam_Exit( wk->BmpOamSys );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32			(CLWKMODE_*)
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_chg_objseq( PKWD_BOOK_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	if(wmode == CLWKMODE_TRAINER){
		if(wnum >= PKWD_BOOK_TRAINER_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk_tr[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk_tr[wnum], TRUE );
	}
	// CMN
	else if(wmode == CLWKMODE_CMN){
		if(wnum >= PKWD_BOOK_CMN_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk_cmn[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk_cmn[wnum], TRUE );
	}
	// POKE
	else if(wmode == CLWKMODE_POKE){
		if(wnum >= PKWD_BOOK_POKE_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk_poke[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk_poke[wnum], TRUE );
	}
	// TYPE
	else if(wmode == CLWKMODE_TYPE){
		if(wnum >= PKWD_BOOK_TYPE_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk_type[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk_type[wnum], TRUE );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32			(CLWKMODE_*)
 *	@param	const s32			objワーク番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_set_obj_visible( PKWD_BOOK_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// trainer
	if(wmode == CLWKMODE_TRAINER){
		// 表示/非表示
		if(wnum >= PKWD_BOOK_TRAINER_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk_tr[wnum], bflg );
	}
	// cmn
	else if(wmode == CLWKMODE_CMN){
		// 表示/非表示
		if(wnum >= PKWD_BOOK_CMN_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk_cmn[wnum], bflg );
	}
	// poke
	else if(wmode == CLWKMODE_POKE){
		// 表示/非表示
		if(wnum >= PKWD_BOOK_POKE_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk_poke[wnum], bflg );
	}
	// type
	else if(wmode == CLWKMODE_TYPE){
		// 表示/非表示
		if(wnum >= PKWD_BOOK_TYPE_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk_type[wnum], bflg );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのアニメの終了待ち
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32			(CLWKMODE_*)
 *	@param	const s32			objワーク番号
 *
 *	@retval	const BOOL			(TRUE:再生中、FALSE:停止、終了)
 */
//-----------------------------------------------------------------------------
static BOOL pwb_is_anime_end( PKWD_BOOK_MAIN_WORK *wk, const s32 wmode, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// trainer
	if(wmode == CLWKMODE_TRAINER){
		if(wnum >= PKWD_BOOK_TRAINER_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		// 判定
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk_tr[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	// cmn
	else if(wmode == CLWKMODE_CMN){
		if(wnum >= PKWD_BOOK_CMN_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		// 判定
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk_cmn[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	// poke
	else if(wmode == CLWKMODE_POKE){
		if(wnum >= PKWD_BOOK_POKE_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		// 判定
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk_poke[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	// type
	else if(wmode == CLWKMODE_TYPE){
		if(wnum >= PKWD_BOOK_TYPE_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		// 判定
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk_type[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(bmpoam)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32						drawtype
 *	@param	const PKWD_BOOK_BMPOAM_DATA*	アドレス
 *	@param	const u32						resid
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_init_obj_bmpoam( PKWD_BOOK_MAIN_WORK* wk, const s32 drawtype, const PKWD_BOOK_BMPOAM_DATA *bmp_adr, const u32 resid )
{
	s16							tmp;
	s32							i;
	s32							buf[2];
	BMPOAM_ACT_DATA				actdata;
	const PKWD_BOOK_BMPOAM_DATA	*adr  = bmp_adr;
	GFL_CLACTPOS				cpos;
	//
	for(i=0;i<PKWD_BOOK_BMPOAM_MAXNUM;i++){
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
		cpos.x = pwb_get_empty( wk, tmp*2 );
		cpos.y = pwb_get_empty( wk, tmp*2+1 );
		//
		GFL_STD_MemClear( &actdata, sizeof(BMPOAM_ACT_DATA) );
		actdata.bmp			= wk->BmpData[i];
		actdata.x 			= cpos.x;
		actdata.y 			= cpos.y;
		actdata.pltt_index	= wk->wk_obj.res[ resid ];		// plt
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
 *	@brief  OB文字列表示(bmpoam)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32				ワーク番号
 *	@param	const STRBUF*			表示文字列
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_disp_bmpoam( PKWD_BOOK_MAIN_WORK* wk, const s32 wnum, const STRBUF *strbuf )
{
	if(wnum >= PKWD_BOOK_BMPOAM_MAXNUM){
		OS_Printf("bmpoamj max error:%d\n",wnum);
		return;
	}
	GFL_BMP_Clear( wk->BmpData[wnum], 0 );
	PRINTSYS_PrintQueColor( wk->print_que, wk->BmpData[wnum], 0, 0, strbuf, wk->sfont, PRINTSYS_LSB_Make(15,2,0) );
	BmpOam_ActorBmpTrans( wk->BmpOamWork[wnum] );
	//
	// 座標も再セットしておく
	{
		s16	buf[2];
		s16	tmp;
		const PKWD_BOOK_BMPOAM_DATA *adr = &pwb_bmpoam_tbl[wnum];
		//
		tmp = adr->empty_num;
		buf[0] = pwb_get_empty( wk, tmp*2 );
		buf[1] = pwb_get_empty( wk, tmp*2+1 );
		buf[0] -= 32;
		buf[1] -= 8;
		BmpOam_ActorSetPos( wk->BmpOamWork[wnum], buf[0], buf[1] );
	}
	// 表示
	BmpOam_ActorSetDrawEnable( wk->BmpOamWork[wnum], TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OB文字列表示(bmpoam)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const s32				ワーク番号
 *	@param	const BOOL				(表示/非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_set_bmpoam_visible( PKWD_BOOK_MAIN_WORK* wk, const s32 wnum, const BOOL bflg )
{
	BmpOam_ActorSetDrawEnable( wk->BmpOamWork[wnum], bflg );
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(Canvasの作成)
 *
 *	@param	PKWD_BOOK_MAIN_WORK*		wk
 *	@param	const s32					(STR_MAIN or STR_SUB)
 *	@param	const s32					(SUBの(0-2)、MAINでは無視)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_init_string( PKWD_BOOK_MAIN_WORK* wk, const s32 type, const s32 mode )
{
	s32		i;
//	const PKWD_BOOK_CANVAS_DATA *adr = &pwb_canvas_tbl[mode];
//	const PKWD_BOOK_CANVAS_DATA *adr = pwb_canvas_tbl[mode];
	const PKWD_BOOK_CANVAS_DATA *adr;
	
	if(type == STR_MAIN){	// main
		adr = &pwb_canvas_00_tbl[0];
		wk->PFlag2 = 0;
	}else{					// sub
		adr = pwb_canvas_tbl[mode];
		wk->PFlag = 0;
		// 破棄
		pwb_exit_string( wk, STR_SUB );
	}
	//
	for(i=0;i<PKWD_BOOK_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット
		if(type == STR_MAIN){		// main
			wk->PrintUtil2[i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		}else{				// sub
			wk->PrintUtil[i].win  = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		}
		// clear
		pwb_clear_canvas( wk, type, i );
		//
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字列破棄
 *
 *	@param	PKWD_BOOK_MAIN_WORK* 		wk
 *	@param	const s32					(STR_MAIN or STR_SUB)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_exit_string( PKWD_BOOK_MAIN_WORK* wk, const s32 type )
{
	s32		i;
	
	if(type == STR_MAIN){		// main
		for(i=0;i<PKWD_BOOK_CANVAS_MAXNUM;i++){
			if(wk->PrintUtil2[i].win != 0){
				// chr側
				GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil2[i].win ), MES_CLEAR_COLOR );
				// scn側
				GFL_BMPWIN_ClearScreen( wk->PrintUtil2[i].win );
				//
				GFL_BMPWIN_Delete( wk->PrintUtil2[i].win );
				wk->PrintUtil2[i].win = 0;
			}
		}
	}else{				// sub
		for(i=0;i<PKWD_BOOK_CANVAS_MAXNUM;i++){
			if(wk->PrintUtil[i].win != 0){
				// chr側
				GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[i].win ), MES_CLEAR_COLOR );
				// scn側
				GFL_BMPWIN_ClearScreen( wk->PrintUtil[i].win );
				//
				GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
				wk->PrintUtil[i].win = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  Queの処理
 *
 *	@param	PKWD_BOOK_MAIN_WORK*			wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_main_print( PKWD_BOOK_MAIN_WORK *wk )
{
	s32		i;
	BOOL	bflg;
	BOOL	pflg;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<PKWD_BOOK_CANVAS_MAXNUM;i++){
		// sub
		if(wk->PrintUtil[i].win){
			pflg = PRINT_UTIL_Trans( &wk->PrintUtil[i], wk->print_que );
			if(pflg == TRUE){
				if(!(wk->PFlag & (1<<i))){
					// キャラの転送終了後スクリーン作成して転送
					// いろいろ試したが、
					// スクリーンの転送を最後にしないと画面にゴミが入るようだ
					GFL_BMPWIN_MakeScreen( wk->PrintUtil[i].win );
					GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[i].win) );
					wk->PFlag |= (1<<i);
				}
			}
		}
		// main
		if(wk->PrintUtil2[i].win){
			pflg = PRINT_UTIL_Trans( &wk->PrintUtil2[i], wk->print_que );
			if(pflg == TRUE){
				if(!(wk->PFlag2 & (1<<i))){
					// キャラの転送終了後スクリーン作成して転送
					// いろいろ試したが、
					// スクリーンの転送を最後にしないと画面にゴミが入るようだ
					GFL_BMPWIN_MakeScreen( wk->PrintUtil2[i].win );
					GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil2[i].win) );
					wk->PFlag2 |= (1<<i);
				}
			}
		}
	}
	// 待ち
	pwb_chk_canvas( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	PKWD_BOOK_MAIN_WORK* 			wk
 *	@param	const s32				(STR_MAIN or STR_SUB)
 *	@param	const s32				Canvasの番号
 *	@param	const s32				表示座標(x)
 *	@param	const s32				表示座標(y)
 *	@param	const STRBUF* 			表示文字列
 *	@param	const u32				表示位置(左(0)、右(1)、中央(2)など)
 *	@param	const PRINTSYS_LSB		カラー
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_disp_string( PKWD_BOOK_MAIN_WORK* wk, const s32 type, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode,
								const PRINTSYS_LSB col )
{
	u16		siz;
	
	// clear
	pwb_clear_canvas( wk, type, mode );
	
	// 表示位置の補正
	siz = pwb_get_putsize( strbuf, wk->font, pmode );
	if(type == STR_MAIN){		// main
		PRINT_UTIL_PrintColor( &wk->PrintUtil2[mode], wk->print_que, x-siz, y, strbuf, wk->font, col );
	}else{				// sub
		PRINT_UTIL_PrintColor( &wk->PrintUtil[mode],  wk->print_que, x-siz, y, strbuf, wk->font, col );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	文字列表示サイズから各モードの補正値を取得(static)
 *			(ui/printtoolから持ってきた)
 *
 * @param	const STRBUF*	buf		文字列
 * @param	GFL_FONT*		font	フォント
 * @param	u32				mode	表示モード(0-2)
 *
 * @return	補正値
 */
//--------------------------------------------------------------------------------------------
static u16 pwb_get_putsize( const STRBUF * buf, GFL_FONT * font, u32 mode )
{
	// 右詰
	if( mode == 1 ){
		return PRINTSYS_GetStrWidth( buf, font, 0 );
	// 中央
	}else if( mode == 2 ){
		return ( PRINTSYS_GetStrWidth( buf, font, 0 ) / 2 );
	}
	return 0;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア
 *
 *	@param	BSCENARIO_WORK*		wk
 *	@param	const s32			(STR_MAIN or STR_SUB)
 *	@param	const s32			キャンバスの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_clear_canvas( PKWD_BOOK_MAIN_WORK* wk, const s32 type, const s32 mode )
{
	if(type == STR_MAIN){
		if(wk->PrintUtil2[mode].win){
			// clear
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil2[mode].win ), MES_CLEAR_COLOR );
			wk->PrintUtil2[mode].transReq = TRUE;
		}
	}else{
		if(wk->PrintUtil[mode].win){
			// clear
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_CLEAR_COLOR );
			wk->PrintUtil[mode].transReq = TRUE;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BGのスクロール(問題があればvblanck中の処理に移動)
 *
 *	@param	PKWD_BOOK_MAIN_WORK* wk
 *	@param	const 32			(段階:(0-2))
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwb_set_bgscroll( PKWD_BOOK_MAIN_WORK* wk, const s32 mode )
{
	s16		buf[2];
	
	buf[0] = pwb_bgscr_tbl[2*mode];				// x
	buf[1] = pwb_bgscr_tbl[2*mode+1];			// y
	// セット
	GFL_BG_SetScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_SET, buf[0] );
	GFL_BG_SetScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, buf[1] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	fix文字表示
 *
 * @param	PKWD_BOOK_MAIN_WORK*	wk
 * @param	const s32				(STR_MAIN or STR_SUB)
 * @param	const s32				モード(0-2)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_disp_fix_string( PKWD_BOOK_MAIN_WORK * wk, const s32 type, const s32 mode )
{
	u16		buf[4];
	const u16	*adr;
	u16		col;
	s32		i;
	STRBUF	*strbuf;
	
	// 指定の方法注意(要修正)
	adr = pwb_fmes_tbl[type+mode];
	for(i=0;i<PKWD_BOOK_CANVAS_MAXNUM;i++){
		// 終了判定
		if(adr[4*i] == 0xffff){
			break;
		}
		buf[0] = adr[4*i];		// mesid
		buf[1] = adr[4*i+1];	// id
		buf[2] = adr[4*i+2];	// col
		buf[3] = adr[4*i+3];	// num
		// 表示
		col = PRINTSYS_LSB_Make(15,2,0);
		if(buf[2] == 1){
			col = PRINTSYS_LSB_Make(1,2,0);
		}
		strbuf = GFL_MSG_CreateString( wk->MsgData[ buf[3] ], pwb_fix_mestbl[ buf[0] ] );
		pwb_disp_string( wk, type, buf[1], 0, 0, strbuf, 0, col );
		
		GFL_STR_DeleteBuffer( strbuf );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フォント表示待ち
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void pwb_chk_canvas( PKWD_BOOK_MAIN_WORK * wk )
{
	s32		num;
	BOOL	bflg;
	
	// タイトルの切り替えがあったときだけ処理
	if(!(wk->Flag & FLG_CHG_TITLE)){
		return;
	}
	//
	// 処理が終わってからBG切り替えなどを行う
	bflg = PRINTSYS_QUE_IsFinished( wk->print_que );
	if(bflg == TRUE){
		// スクロール切り替え
		pwb_set_bgscroll( wk, wk->TitleMode );
#if 0
		// obj非表示(?)
		pwb_clear_obj( wk );
		if(wk->TitleMode == TMODE_TRAINER){
			pwb_disp_trainer_obj( wk, 0 );
		}else if(wk->TitleMode == TMODE_POKE){
			pwb_disp_poke_obj( wk, 0 );
		}
#endif
		// 切り替え終了
		wk->Flag &= ~FLG_CHG_TITLE;
	}
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	初期化(book)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_init_book( PKWD_BOOK_MAIN_WORK * wk )
{
	// main処理へ
	wk->Mode 		= MODE_MAIN_BOOK;
	wk->SubMode 	= STEP_LV1;
	wk->TitleMode	= TMODE_BOOK;
	//
	// bg初期化(1回目だけは転送しない)
	if(wk->Flag & FLG_SET_INIT){
		// canvas初期化
		pwb_init_string( wk, STR_SUB, TMODE_BOOK );
		// fix文字列
		pwb_disp_book_string( wk, 0 );
		wk->Flag |= FLG_CHG_TITLE;
		// 矢印位置
		pwb_chg_cmn_cursor( wk, 0 );
	}
	// objの初期化
	pwb_clear_obj( wk, 0 );
	//
	wk->Flag |= FLG_SET_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	main(book)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_main_book( PKWD_BOOK_MAIN_WORK * wk )
{
	BOOL	bflg;
	
	switch( wk->SubMode ){
		// 初期化
		case STEP_INIT:
			wk->SubMode = STEP_LV1;
			
		// 通常
		case STEP_LV1:
			pwb_key_book( wk );
			break;
		//
		// 待ち(戻る)
		case STEP_LV2:
			bflg = pwb_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_BACK);
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->NextMode = MODE_END;
			}
			break;
		//
		// アニメ待ち(右)
		case STEP_LV2+10:
			bflg = pwb_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_RIGHT);
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
				wk->Mode++;
			}
			break;
		//
		// アニメ待ち(左)
		case STEP_LV2+20:
			bflg = pwb_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_LEFT);
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
				wk->Mode++;
			}
			break;
		//
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	exit(book)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_exit_book( PKWD_BOOK_MAIN_WORK * wk )
{
	// 終了判定
	pwb_chk_end( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	key(book)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_key_book( PKWD_BOOK_MAIN_WORK * wk )
{
	s32		flg=0;
	
	// debug(終了)
	if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
//		wk->NextMode = MODE_END;
		// 戻るシーケンス変更
		pwb_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_BACK, SEQ_CMN_BACK_ANIM );
		wk->SubMode = STEP_LV2;
	}
	// 上
	if ( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		wk->ScenarioCurPos--;
		if(wk->ScenarioCurPos < 0){
			wk->ScenarioCurPos = 0;
		}
		OS_Printf("scenario :%d\n",wk->ScenarioCurPos);
		flg = 1;
		// test
		wk->ScenarioNum = wk->ScenarioCurPos;
	}
	// 下
	else if ( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
		wk->ScenarioCurPos++;
		if(wk->ScenarioCurPos > 2){
			wk->ScenarioCurPos = 2;
		}
		OS_Printf("scenario :%d\n",wk->ScenarioCurPos);
		flg = 1;
		// test
		wk->ScenarioNum = wk->ScenarioCurPos;
	}
	//
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
		wk->NextMode = MODE_INIT_TRAINER;
		wk->Mode++;
		wk->Flag |= FLG_MODE_SUB;
		//
		//-----------------------------------
		// シナリオに応じたパーティ情報変更
		//-----------------------------------
		pwb_make_party( wk, wk->ScenarioNum );
		wk->InfoCurPos = 0;
	}
	//
	// 上文字列更新
	if(flg == 1){
		pwb_disp_book_string( wk, 1 );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	文字列表示(book)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 * @param	const s32					(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_disp_book_string( PKWD_BOOK_MAIN_WORK * wk, const s32 mode )
{
	s32			i;
	s32			id = wk->ScenarioNum;
//	s32			id = 0;
	STRBUF		*strbuf;
	
	// 初期のみでよいはず
	if(mode == 0){
		// 固定文字列
		pwb_disp_fix_string( wk, STR_SUB, TMODE_BOOK );
		//
	}
	// タイトル、あらすじ、クリア目標
	for(i=0;i<3;i++){
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], pwb_mes_tbl[i]+id );
		pwb_disp_string( wk, STR_MAIN, PWB_CVS_TOP_TITLE_MES+i, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(15,2,0) );
		GFL_STR_DeleteBuffer( strbuf );
	}
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	初期化(trainer)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_init_trainer( PKWD_BOOK_MAIN_WORK * wk )
{
	// main処理へ
	wk->Mode 		= MODE_MAIN_TRAINER;
	wk->SubMode 	= STEP_LV1;
	wk->TitleMode	= TMODE_TRAINER;
	//
	// canvas初期化
	pwb_init_string( wk, STR_SUB, TMODE_TRAINER );
	// 表示まとめ
	pwb_disp_trainer_string( wk, 0 );
	// objの初期化
	pwb_clear_obj( wk, 1 );
	pwb_disp_trainer_obj( wk, 0 );
	// 矢印位置
	pwb_chg_cmn_cursor( wk, 1 );
	//
	wk->Flag |= FLG_CHG_TITLE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	main(trainer)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_main_trainer( PKWD_BOOK_MAIN_WORK * wk )
{
	BOOL	bflg;
	
	switch( wk->SubMode ){
		// 初期化
		case STEP_INIT:
			wk->SubMode = STEP_LV1;
			
		// 通常
		case STEP_LV1:
			pwb_key_trainer( wk );
			break;
		//
		// 待ち(戻る)
		case STEP_LV2:
			bflg = pwb_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_BACK);
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->SubMode = STEP_LV1;
				wk->Flag &= ~FLG_MODE_SUB;
			}
			break;
		//
		// 右シーケンス変更
		case STEP_LV2+10:
			bflg = pwb_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_RIGHT);
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
				wk->Mode++;
				// カーソル処理
				pwb_chk_infocurpos( wk, 0 );
			}
			break;
		//
		// 左シーケンス変更
		case STEP_LV2+20:
			bflg = pwb_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_LEFT);
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
				wk->Mode++;
				// カーソル処理
				pwb_chk_infocurpos( wk, 1 );
			}
			break;
		//
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	exit(trainer)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_exit_trainer( PKWD_BOOK_MAIN_WORK * wk )
{
	// 終了判定
	pwb_chk_end( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	key(trainer)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_key_trainer( PKWD_BOOK_MAIN_WORK * wk )
{
	// 戻る(book)
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		wk->NextMode = MODE_INIT_BOOK;
		// 戻るシーケンス変更
		pwb_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_BACK, SEQ_CMN_BACK_ANIM );
		wk->SubMode = STEP_LV2;
	}
	// 右
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		wk->NextMode = MODE_INIT_POKE;
		// 右シーケンス変更
		pwb_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_RIGHT, SEQ_CMN_RIGHT_ANIM );
		wk->SubMode = STEP_LV2+10;
	}
	// 左
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		wk->NextMode = MODE_INIT_POKE;
		// 左シーケンス変更
		pwb_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_LEFT, SEQ_CMN_LEFT_ANIM );
		wk->SubMode = STEP_LV2+20;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	文字列表示(book)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 * @param	const s32					(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_disp_trainer_string( PKWD_BOOK_MAIN_WORK * wk, const s32 mode )
{
	// 初期のみでよいはず
	if(mode == 0){
		// 固定文字列
		pwb_disp_fix_string( wk, STR_SUB, TMODE_TRAINER );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	obj表示(trainer)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 * @param	const s32					(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_disp_trainer_obj( PKWD_BOOK_MAIN_WORK * wk, const s32 mode )
{
	s32		i;
	BOOL	bflg;
	STRBUF	*strbuf;
	
	for(i=0;i<PKWD_POKE_MAX;i++){
		if(i < wk->PokeMax){
			pwb_chg_poke_obj( wk, i, wk->PokeWork[i].id, wk->PokeWork[i].sex );
			bflg = TRUE;
		}else{
			bflg = FALSE;
		}
		pwb_set_obj_visible( wk, CLWKMODE_POKE, i, bflg );
	}
	// trainer
	pwb_set_obj_visible( wk, CLWKMODE_TRAINER, 0, TRUE );
	//
	strbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
	//
	// ポケモン数字
	for(i=0;i<wk->PokeMax;i++){
		// 1はじまり
		STRTOOL_SetNumber( strbuf, (i+1), 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
		pwb_disp_bmpoam( wk, i, strbuf );
	}
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
	//
	// カーソル数
	pwb_disp_infocurpos( wk );
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	初期化(poke)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_init_poke( PKWD_BOOK_MAIN_WORK * wk )
{
	// main処理へ
	wk->Mode 		= MODE_MAIN_POKE;
	wk->SubMode 	= STEP_LV1;
	wk->TitleMode	= TMODE_POKE;
	//
	// canvas初期化
	pwb_init_string( wk, STR_SUB, TMODE_POKE );
	// 表示まとめ
	pwb_disp_poke_string( wk, 0 );
	// objの初期化
	pwb_clear_obj( wk, 1 );
	pwb_disp_poke_obj( wk, 0 );
	//
	wk->Flag |= FLG_CHG_TITLE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	main(poke)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_main_poke( PKWD_BOOK_MAIN_WORK * wk )
{
	BOOL	bflg;
	
	switch( wk->SubMode ){
		// 初期化
		case STEP_INIT:
			wk->SubMode = STEP_LV1;
			
		// 通常
		case STEP_LV1:
			pwb_key_poke( wk );
			break;
		//
		// 待ち(戻る)
		case STEP_LV2:
			bflg = pwb_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_BACK);
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->SubMode = STEP_LV1;
				wk->Flag &= ~FLG_MODE_SUB;
			}
			break;
		//
		// 右シーケンス
		case STEP_LV2+10:
			bflg = pwb_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_RIGHT);
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
				wk->Mode++;
				// カーソル処理
				pwb_chk_infocurpos( wk, 0 );
				if(wk->InfoCurPos != 0){
					wk->NextMode = MODE_INIT_POKE;
				}
			}
			break;
		//
		// 左シーケンス
		case STEP_LV2+20:
			bflg = pwb_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_LEFT);
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
				wk->Mode++;
				// カーソル処理
				pwb_chk_infocurpos( wk, 1 );
				if(wk->InfoCurPos != 0){
					wk->NextMode = MODE_INIT_POKE;
				}
			}
			break;
		//
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	exit(poke)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_exit_poke( PKWD_BOOK_MAIN_WORK * wk )
{
	// 終了判定
	pwb_chk_end( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	key(poke)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_key_poke( PKWD_BOOK_MAIN_WORK * wk )
{
	// debug(終了)
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		wk->NextMode = MODE_INIT_BOOK;
		// 戻るシーケンス変更
		pwb_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_BACK, SEQ_CMN_BACK_ANIM );
		wk->SubMode = STEP_LV2;
	}
	// 右
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		wk->NextMode = MODE_INIT_TRAINER;
		// 右シーケンス変更
		pwb_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_RIGHT, SEQ_CMN_RIGHT_ANIM );
		wk->SubMode = STEP_LV2+10;
	}
	// 左
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		wk->NextMode = MODE_INIT_TRAINER;
		// 左シーケンス変更
		pwb_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_LEFT, SEQ_CMN_LEFT_ANIM );
		wk->SubMode = STEP_LV2+20;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	文字列表示(poke)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 * @param	const s32					(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_disp_poke_string( PKWD_BOOK_MAIN_WORK * wk, const s32 mode )
{
	s32				tmp;
	s32				i;
	s32				buf[2];
	u32				pcol;
	POKEMON_PARAM	*pp;
	STRBUF			*exp;
	STRBUF			*str;
	//
	// 初期のみでよいはず
	if(mode == 0){
		// 固定文字列
		pwb_disp_fix_string( wk, STR_SUB, TMODE_POKE );
	}
	//--------
	// 更新
	//--------
	// ポケモン情報
	pp = PokeParty_GetMemberPointer( wk->Party, wk->PokePageNum );
	//
	// 1回だけ確保
	exp = GFL_STR_CreateBuffer( 128, wk->heapID );
	//
	// 名前
	buf[0] = wk->PokeWork[wk->PokePageNum].id;
	str = GFL_MSG_CreateString( wk->MsgData[0], pwb_info_tbl[0] );
	WORDSET_RegisterPokeMonsNameNo( wk->Wordset, 0, buf[0] );
	WORDSET_ExpandStr( wk->Wordset, exp, str );
	pwb_disp_string( wk, STR_SUB, PWB_CVS_INFO_NAME, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0) );
	GFL_STR_DeleteBuffer( str );
	//
	// 性別
	tmp = 1;
	pcol   = PRINTSYS_LSB_Make(6,5,0);
	buf[0] = wk->PokeWork[wk->PokePageNum].sex;
	if(buf[0] == PTL_SEX_FEMALE){
		tmp  = 2;
		pcol = PRINTSYS_LSB_Make(4,3,0);
	}
	str = GFL_MSG_CreateString( wk->MsgData[0], pwb_info_tbl[tmp] );
	pwb_disp_string(  wk, STR_SUB, PWB_CVS_INFO_SEX, 0, 0, str, 0, pcol );
	GFL_STR_DeleteBuffer( str );
	//
	// レベル
	buf[0] = wk->PokeWork[wk->PokePageNum].level;
	str = GFL_MSG_CreateString( wk->MsgData[0], pwb_info_tbl[3] );
	WORDSET_RegisterNumber( wk->Wordset, 0, buf[0], 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_ZENKAKU);
	WORDSET_ExpandStr( wk->Wordset, exp, str );
	pwb_disp_string(  wk, STR_SUB, PWB_CVS_INFO_LEVEL, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0) );
	GFL_STR_DeleteBuffer( str );
	//
	// もちもの
	buf[0] = wk->PokeWork[wk->PokePageNum].item;
	str = GFL_MSG_CreateString( wk->MsgData[0], pwb_info_tbl[4] );
	WORDSET_RegisterItemName( wk->Wordset, 0, buf[0] );
	WORDSET_ExpandStr( wk->Wordset, exp, str );
	pwb_disp_string(  wk, STR_SUB, PWB_CVS_INFO_ITEM01, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0) );
	GFL_STR_DeleteBuffer( str );
	//
	// 技
	for(i=0;i<4;i++){
		buf[0] = wk->PokeWork[wk->PokePageNum].skill[i];
		if(buf[0] != 0){
			str = GFL_MSG_CreateString( wk->MsgData[0], pwb_info_tbl[6+i] );
			WORDSET_RegisterWazaName( wk->Wordset, 0, buf[0] );
			WORDSET_ExpandStr( wk->Wordset, exp, str );
			// 表示
			pwb_disp_string(  wk, STR_SUB, PWB_CVS_INFO_SKILL00+i, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0) );
			GFL_STR_DeleteBuffer( str );
		}else{
			pwb_clear_canvas( wk, STR_SUB, PWB_CVS_INFO_SKILL00+i );
		}
	}
	//
	// 特性
	buf[0] = wk->PokeWork[wk->PokePageNum].stat;
	str = GFL_MSG_CreateString( wk->MsgData[0], pwb_info_tbl[5] );
	WORDSET_RegisterTokuseiName( wk->Wordset, 0, buf[0] );
	WORDSET_ExpandStr( wk->Wordset, exp, str );
	pwb_disp_string(  wk, STR_SUB, PWB_CVS_INFO_STATUS01, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0) );
	GFL_STR_DeleteBuffer( str );
	//
	// 破棄
	GFL_STR_DeleteBuffer( exp );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	obj表示(poke)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 * @param	const s32			(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_disp_poke_obj( PKWD_BOOK_MAIN_WORK * wk, const s32 mode )
{
	u32				buf[2];
	s32				i;
	POKEMON_PARAM	*pp;
	BOOL			bflg;
	
	// ポケモン情報
	pp = PokeParty_GetMemberPointer( wk->Party, wk->PokePageNum );
	//
	//--------
	// ポケモン
	//--------
	pwb_chg_poke_obj( wk, 6, wk->PokeWork[wk->PokePageNum].id, wk->PokeWork[wk->PokePageNum].sex );
	pwb_set_obj_visible( wk, CLWKMODE_POKE, 6, TRUE );
	//
	//--------
	// タイプ
	//--------
	// type1
	buf[0] = wk->PokeWork[wk->PokePageNum].poke_type[0];
	pwb_chg_type_obj( wk, 0, 0, buf[0] );
	pwb_set_obj_visible( wk, CLWKMODE_TYPE, 0, TRUE );
	//
	// type2(違う場合のみ表示)
	buf[1] = wk->PokeWork[wk->PokePageNum].poke_type[1];
	if(buf[0] != buf[1]){
		pwb_chg_type_obj( wk, 1, 0, buf[1] );
		bflg = TRUE;
	}else{
		bflg = FALSE;
	}
	pwb_set_obj_visible( wk, CLWKMODE_TYPE, 1, TRUE );
	
//	OS_Printf("type :%d,%d\n",buf[0],buf[1]);
	
	//--------
	// タイプ
	//--------
	for(i=0;i<4;i++){
		// 技
		buf[0] = wk->PokeWork[wk->PokePageNum].skill[i];
		if(buf[0] != 0){
			// 技のタイプ
			pwb_chg_type_obj( wk, 2+i, 1, wk->PokeWork[wk->PokePageNum].skill_type[i] );
			bflg = TRUE;
		}else{
			bflg = FALSE;
		}
		pwb_set_obj_visible( wk, CLWKMODE_TYPE, 2+i, bflg );
	}
	//
	// カーソル数
	pwb_disp_infocurpos( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	objクリア
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void pwb_clear_obj( PKWD_BOOK_MAIN_WORK * wk, const s32 mode )
{
	s32		i;
	
	// 表示したいものはこの後で表示すること
	// poke
	for(i=0;i<PKWD_BOOK_POKE_OBJ_MAXNUM;i++){
		pwb_set_obj_visible( wk, CLWKMODE_POKE, i, FALSE );
	}
	// type
	for(i=0;i<PKWD_BOOK_TYPE_OBJ_MAXNUM;i++){
		pwb_set_obj_visible( wk, CLWKMODE_TYPE, i, FALSE );
	}
	// trainer
	pwb_set_obj_visible( wk, CLWKMODE_TRAINER, 0, FALSE );
	//
	// bmpoam
	for(i=0;i<6;i++){
		pwb_set_bmpoam_visible( wk, i, FALSE );
	}
	//
	// カーソル情報のon/off
	if(mode == 0){
		// bmpoam
		for(i=0;i<3;i++){
			pwb_set_bmpoam_visible( wk, 6+i, FALSE );
		}
	}
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	処理まとめ
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return	BOOL				(TRUE:制御中、FALSE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL pwb_main_loop( PKWD_BOOK_MAIN_WORK * wk )
{
	BOOL	bflg=TRUE;
	BOOL	tflg;
	//
	if((wk->Mode < 0)||(wk->Mode >= MODE_MAX)){
		OS_Printf("mode error\n");
		return( FALSE );
	}
	// 処理の分岐
	pwb_functbl[wk->Mode]( wk );
	//
	//
	// 終了判定
	if(wk->NextMode == MODE_END){
		bflg = FALSE;
	}
	return( bflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	終了処理まとめ
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_chk_end( PKWD_BOOK_MAIN_WORK * wk )
{
	// 次の処理
	if(wk->NextMode != MODE_END){
		wk->Mode = wk->NextMode;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	keyまとめ
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_chk_infocurpos( PKWD_BOOK_MAIN_WORK * wk, const s32 mode )
{
	// 右
	if(mode == 0){
		wk->InfoCurPos++;
		if(wk->InfoCurPos >= wk->PokeMax+1){
			wk->InfoCurPos = 0;
		}
	}
	// 左
	else{
		wk->InfoCurPos--;
		if(wk->InfoCurPos < 0){
			wk->InfoCurPos = wk->PokeMax;
		}
	}
	// トレーナー画面の場合
	if(wk->InfoCurPos == 0){
		wk->PokePageNum = 0;
	}
	// ポケモン画面の場合
	else{
		wk->PokePageNum = wk->InfoCurPos-1;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ページ数表示
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_disp_infocurpos( PKWD_BOOK_MAIN_WORK * wk )
{
	STRBUF	*strbuf;
	STRBUF	*strbuf2;
	//
	strbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
	// 現在
	STRTOOL_SetNumber( strbuf, (wk->InfoCurPos+1), 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	pwb_disp_bmpoam( wk, 6, strbuf );
	// 最大
	STRTOOL_SetNumber( strbuf, (wk->PokeMax+1), 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	pwb_disp_bmpoam( wk, 8, strbuf );
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
	//
	// slash
	strbuf2 = GFL_MSG_CreateString( wk->MsgData[0], pwb_info_tbl[10] );
	pwb_disp_bmpoam( wk, 7, strbuf2 );
	// 破棄
	GFL_STR_DeleteBuffer( strbuf2 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	カーソル位置修正
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 * @param	const s32					(0:通常、1:ポケモン情報時)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_chg_cmn_cursor( PKWD_BOOK_MAIN_WORK * wk, const s32 mode )
{
	s16				i;
	s16				buf[2];
	GFL_CLACTPOS	pos;
	//
	// emptyを取得する
	buf[0] = pwb_curpos_tbl[2*mode+0];
	buf[1] = pwb_curpos_tbl[2*mode+1];
	//
	for(i=0;i<2;i++){
		pos.x = pwb_get_empty( wk, buf[i]*2 );
		pos.y = pwb_get_empty( wk, buf[i]*2+1 );
		// 設定
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk_cmn[CLWK_CMN_LEFT+i], &pos, CLSYS_DEFREND_SUB );
	}
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	パーティを作成(event_pokewood.cのsetupPokemonから持ってきた)
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 * @param	const s32					台本番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_make_party( PKWD_BOOK_MAIN_WORK * wk, const s32 scenario_id )
{
	// ポケウッドのpartyの作成
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData( wk->Param->gamesys );
	const MYSTATUS *  mystatus =  GAMEDATA_GetMyStatus( gamedata );
	//
	// データを破棄
	pwb_delete_party( wk );
	//
	// newされたデータがwk->Partyに格納されるはず
	wk->Party = PokeParty_AllocPartyWork( wk->heapID );
	POKEWOOD_EVENT_setupPokemon( mystatus, wk->Party, wk->heapID, scenario_id, 1 );
	//
	// パラメータセット
	pwb_set_pokework( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	パーティを破棄
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_delete_party( PKWD_BOOK_MAIN_WORK * wk )
{
	if(wk->Party != 0){
		GFL_HEAP_FreeMemory( wk->Party );
		wk->Party = 0;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ポケモンパラメータセット
 *
 * @param	PKWD_BOOK_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwb_set_pokework( PKWD_BOOK_MAIN_WORK * wk )
{
	s32				i;
	s32				j;
	POKEMON_PARAM	*pp;
	
	// 最大数をセット
	wk->PokeMax		= PokeParty_GetPokeCount( wk->Party );
	
	// 初期化(なくてもよいはず)
	for(i=0;i<PKWD_POKE_MAX;i++){
		wk->PokeWork[i].id		= 0;
		wk->PokeWork[i].sex		= 0;
		wk->PokeWork[i].level	= 0;
		wk->PokeWork[i].item	= 0;
		wk->PokeWork[i].poke_type[0] = 0;
		wk->PokeWork[i].poke_type[1] = 0;
		//
		for(j=0;j<4;j++){
			wk->PokeWork[i].skill[j] = 0;
			wk->PokeWork[i].skill_type[j] = 0;
		}
	}
	
	// PP_Get関連は重たい(?)ようなので、
	// 初期化時にあらかじめ取得しておく
	// セット
	for(i=0;i<wk->PokeMax;i++){
		// ポケモン情報
		pp = PokeParty_GetMemberPointer( wk->Party, i );
		// 名前
		wk->PokeWork[i].id = PP_Get( pp, ID_PARA_monsno, 0 );
		// 性別
		wk->PokeWork[i].sex = PP_GetSex( pp );
		// レベル
		wk->PokeWork[i].level = PP_Get( pp, ID_PARA_level, 0 );
		// もちもの
		wk->PokeWork[i].item = PP_Get( pp, ID_PARA_item, 0 );
		// もちもの
		wk->PokeWork[i].stat = PP_Get( pp, ID_PARA_speabino, 0 );
		// タイプ
		wk->PokeWork[i].poke_type[0] = PP_Get( pp, ID_PARA_type1, 0 );
		wk->PokeWork[i].poke_type[1] = PP_Get( pp, ID_PARA_type2, 0 );
		// 技、技タイプ
		for(j=0;j<4;j++){
			wk->PokeWork[i].skill[j] = PP_Get( pp, ID_PARA_waza1+j, 0 );
			if(wk->PokeWork[i].skill[j] != 0){
				wk->PokeWork[i].skill_type[j] = WAZADATA_GetParam( wk->PokeWork[i].skill[j], WAZAPARAM_TYPE );
			}
		}
	}
#if 0
	// debug表示
	for(i=0;i<PKWD_POKE_MAX;i++){
		OS_Printf("id, sex, level, item, stat, poke_type[0], poke_type[1] :%d,%d,%d,%d,%d,%d,%d\n",
			wk->PokeWork[i].id, wk->PokeWork[i].sex, wk->PokeWork[i].level, 
			wk->PokeWork[i].item, wk->PokeWork[i].stat, wk->PokeWork[i].poke_type[0], wk->PokeWork[i].poke_type[1] );
		OS_Printf("skill :%d,%d,%d,%d\n",
			wk->PokeWork[i].skill[0], wk->PokeWork[i].skill[1], wk->PokeWork[i].skill[2], wk->PokeWork[i].skill[3] );
		OS_Printf("skill type :%d,%d,%d,%d\n",
			wk->PokeWork[i].skill_type[0], wk->PokeWork[i].skill_type[1], wk->PokeWork[i].skill_type[2], wk->PokeWork[i].skill_type[3] );
		
	}
#endif
}

