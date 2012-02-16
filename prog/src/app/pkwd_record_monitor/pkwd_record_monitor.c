//=============================================================================
/**
 *
 *	@file		pkwd_record_monitor.c
 *	@brief  ポケウッド記録モニター
 *	@author	ichiraku_katsuhiko
 *	@data		2011.09.12
 *
 *  @note   pkwd_book.c の 挙動とほぼ同じだったので
 *          コピーしてきて使っています。
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// system
#include "system/wipe.h"
#include "system/bmp_winframe.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"
#include "ui/frame_list.h"

// サウンド
#include "sound/pm_sndsys.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

// アプリ共通素材
#include "app/app_menu_common.h"

// pokegra
#include "pokeicon/pokeicon.h"

// trgra
#include "system/tr2dgra.h"

// battle
#include "battle/btl_main_def.h"
#include "battle/pokewood/pokewood_senario.h"

// field
#include "field/event_pokewood.h"

//アーカイブ
#include "arc_def.h"
#include "arc/app_menu_common.naix"
#include "b_scenario_gra.naix"

// メッセージ
#include "message.naix"
#include "msg/msg_pw_movietitle.h"
#include "msg/msg_pokewood_name.h"

// mine
#include "app/pkwd_record_monitor.h"
#include "pkwd_record_monitor_graphic.h"
#include "pkwd_record_monitor_snd_def.h"

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
	PKWD_RECORD_MONITOR_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};

// 段階
enum{
	STEP_INIT	= 0,		// 初期化
  STEP_LINE_ANIME = 1,
  STEP_LINE_ANIME_WAIT = 2,
	STEP_LV1 	= 100,
	STEP_LV2 	= 200,
	STEP_LV3 	= 300,

	//
	STEP_END	= 10000,
};

//
enum{
	TMODE_RECORD_MONITOR	= 0,		///< 台本選択
	//
	TMODE_MAX,
};

//
enum{
	MODE_END		= -1,	///< 終了
	
	MODE_INIT_RECORD_MONITOR 	= 0,	///< 記録モニター
	MODE_MAIN_RECORD_MONITOR,
	MODE_EXIT_RECORD_MONITOR,
	//
	MODE_MAX,
};

// フォント
enum{
	STR_MAIN	= 0,		// 上画面
	STR_SUB,				// 下画面
};

// パレット
// 上(MAIN)：BG  (0:下地),(10:font)
// 上(MAIN)：OBJ (5-7:poke),(8-13:parts),(14:通信アイコン)
//
// 下(SUB) ：BG  (0-1:リストパネル),(6,7:下地),(8:cmn),(10:font)
// 下(SUB) ：OBJ (4-7:cmn),(8-13:obj)
enum{
	// 何本目か
	// (main bg)
	PLTID_MAIN_FONT			= 10,		// font
	// (main obj)
	PLTID_MAIN_POKE			= 5,		// pok
	PLTID_MAIN_OBJ_LIST		= 8,		// obj
	//
	// (sub bg)
	PLTID_SUB_BG_BASE		= 6,		// 通常bg
	PLTID_SUB_BG_CMN		= 8,		// cmn
	PLTID_SUB_FONT			= 10,		// font
	// (sub obj)
	PLTID_SUB_OBJ_CMN		= 4,		// cmn
	PLTID_SUB_OBJ_LIST		= 8,		// list
	//
	// 何番目か
	MES_CLEAR_COLOR			= 0,		// クリアカラー
};

// 最大値
enum{
	PKWD_RECORD_MONITOR_EMPTY_MAXNUM = 33*2,		// empty
	//
	PKWD_RECORD_MONITOR_CMN_OBJ_MAXNUM	= 3,	// obj(cmn)
	PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM	= POKEWOOD_MYPOKE_NUM,	// obj(poke)
	PKWD_RECORD_MONITOR_LIST_OBJ_MAXNUM	= 10,	// obj(list)
	PKWD_RECORD_MONITOR_UP_OBJ_MAXNUM	  = 11+3,	// obj(up)
	//
	PKWD_RECORD_MONITOR_BGNUM_MAXNUM		= 8,	// bg
	//
	PKWD_RECORD_MONITOR_CANVAS_MAXNUM		= 13,	// canvas
	//
	PKWD_RECORD_MONITOR_POKE_MAX			= POKEWOOD_MYPOKE_NUM,	// pokemon max = 6
	//
	PKWD_RECORD_MONITOR_COLOR_NUM			= 4,	// 上画面(色変えの本数)
	//
	PKWD_RECORD_MONITOR_LIST_MAX			= 7,	// 1画面のリスト最大数
	//
	PKWD_RECORD_MONITOR_SCENARIO_MAX		= POKEWOOD_PLAY_MAX,	// シナリオ最大値
	//PKWD_RECORD_MONITOR_SCENARIO_MAX		= 40,	// シナリオ最大値
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
	RESID_POKE_CEL,						// セルアニメ
	//
	RESID_LIST_PLT,						// パレット
	RESID_LIST_CHR,						// キャラ
	RESID_LIST_CEL,						// セルアニメ
	//
	RESID_OBJ_MAX,
};

// clwkのグループ
enum{
	CLWKMODE_CMN	= 0,				// cmn
	CLWKMODE_POKE,						// ポケモン
	CLWKMODE_LIST,						// リスト
  CLWKMODE_UP,              // 上画面のOBJ
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

// CLWK(List)
enum{
	CLWK_LIST_BALL,						// スクロールバーの玉
	//
	CLWK_LIST_NEW_00,					// NEW_00
	CLWK_LIST_NEW_01,					// NEW_01
	CLWK_LIST_NEW_02,					// NEW_02
	CLWK_LIST_NEW_03,					// NEW_03
	CLWK_LIST_NEW_04,					// NEW_04
	CLWK_LIST_NEW_05,					// NEW_05
	CLWK_LIST_NEW_06,					// NEW_06
	CLWK_LIST_NEW_07,					// NEW_07
	CLWK_LIST_NEW_08,					// NEW_08
};

// CLWK(UP)
enum{
  CLWK_UP_CROWN,            // 王冠
  CLWK_UP_KIRA,             // キラ
  CLWK_UP_STAR_00,          // スター
  CLWK_UP_STAR_01,          // スター
  CLWK_UP_STAR_02,          // スター
  CLWK_UP_STAR_03,          // スター
  CLWK_UP_STAR_04,          // スター
  CLWK_UP_STAR_05,          // スター
  CLWK_UP_LINE_RED,   // 赤いライン
  CLWK_UP_LINE_GREEN, // 緑ライン
  CLWK_UP_LINE_BLUE,  // 青いライン
  //
  CLWK_UP_MARK_GOOD,         // 評価アイコン(good)
  CLWK_UP_MARK_BAD,          // 評価アイコン(bad)
  CLWK_UP_MARK_STRANGE,      // 評価アイコン(strange)
};

// CLWK(List)のアニメシーケンス
enum{
  CLWK_LIST_ANM_SEQ_SLIDER = 0,
  CLWK_LIST_ANM_SEQ_NEW,
  CLWK_LIST_ANM_SEQ_PANEL,
  CLWK_LIST_ANM_SEQ_UP,
  CLWK_LIST_ANM_SEQ_CROWN,
  CLWK_LIST_ANM_SEQ_KIRA,
  CLWK_LIST_ANM_SEQ_STAR,
  CLWK_LIST_ANM_SEQ_LINE_RED,   // 赤いライン
  CLWK_LIST_ANM_SEQ_LINE_GREEN, // 緑ライン
  CLWK_LIST_ANM_SEQ_LINE_BLUE,  // 青いライン
  //
  CLWK_LIST_ANM_SEQ_GOOD,		// 評価アイコン(good)
  CLWK_LIST_ANM_SEQ_BAD,		// 評価アイコン(bad)
  CLWK_LIST_ANM_SEQ_STRANGE,	// 評価アイコン(strange)
};


//-------------
// フラグ
//-------------
// OBJのフラグ(PKWD_RECORD_MONITOR_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM		= (1<<0),		// アニメあり
	FLG_OBJ_HFLIP		= (1<<1),		// HFLIP
	FLG_OBJ_VFLIP		= (1<<2),		// VFLIP
};

// PKWD_RECORD_MONITOR_MAIN_WORK.Flag
enum{
	FLG_WIPE_IN_END		= (1<<0),		// wipe(in)終了
	FLG_SET_INIT		= (1<<1),		// 初期化済み
	FLG_CHG_TITLE		= (1<<2),		// 変更
	FLG_MODE_SUB		= (1<<3),		// サブ画面に移行した
	//
	FLG_LEFT_ANIM		= (1<<4),		// 左アニメ
	FLG_RIGHT_ANIM		= (1<<5),		// 右アニメ
	FLG_UPDATE			= (1<<6),		// 更新
	//
	FLG_STR_WAIT		= (1<<8),		// wait
	//
	FLG_FRAME_STAT		= (1<<12),		// フレーム初期化終了
	FLG_FRAME_SET_PAL	= (1<<13),		// パレット転送
	//
	FLG_TP_NUMBER		= (1<<16),		// 番号タッチ
	FLG_ANIM_END		= (1<<17),		// アニメ終了
	FLG_TOUCH_DISP		= (1<<18),		// 非表示状態からの表示
};

//-------------
// 文字列
//-------------
// 固定文字列(pwm_fix_mestbl)と対応
enum{
	MES_ID_TOP_01 = 0,      // 「上映作品の最高記録」
	MES_ID_TOP_02,          // 「興行成績記録」
	MES_ID_TOP_03,          // 「主な出演キャスト」
	//
	MES_ID_DUMMY,						// dummy(空白を表示)
};

// canvasの番号
enum{
	PWB_CVS_TOP_01 = 0,      // 「上映作品の最高記録」
	PWB_CVS_TOP_02,          // 「興行成績記録」
	PWB_CVS_TOP_03,          // 「主な出演キャスト」
	PWB_CVS_TOP_04,          //  メインタイトル
	PWB_CVS_TOP_05,          //  サブタイトル
	PWB_CVS_TOP_06,          // 「タグ:4桁数値 億円」
	PWB_CVS_TOP_07,          // 「タグ：トレーナー名」
	PWB_CSV_TOP_MAX,		// max
};

// マーク(stateflag)
enum{
	FLG_PWRM_GOOD		= (1<<0),	// GOOD
	FLG_PWRM_BAD		= (1<<1),	// BAD
	FLG_PWRM_STRANGE	= (1<<2),	// STRANGE
	//
	FLG_PWRM_NEW		= (1<<3),	// NEW
	FLG_PWRM_UP			= (1<<4),	// UP
	FLG_PWRM_FINISH		= (1<<5),	// すでに見た
};

// 文字色
// 上画面
#define TEXT_COL_MAIN   ( PRINTSYS_LSB_Make(13, 12, 11 ) )   // 黒文字
#define TEXT_COL_FIX    ( PRINTSYS_LSB_Make( 1,  2,  0 ) )   // 固定文字

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
#if 0
//----------------------
// ポケモンの情報(1体)
//----------------------
// PP_Getを初期化時にまとめて処理するために作成
// (すべてが)u32でなくてもよいはずが、
// 返り値を直接セットするため以下のようにした
typedef struct{
	u32					id;				// id
	u32					sex;			// 性別
  u32         form;     // フォルムNo
} PKWD_RECORD_MONITOR_POKE_WORK;
#endif

//-------------------------------------
/// シナリオ開放状況
//-------------------------------------
typedef struct{
	u8					openid;		// 開放のシナリオ番号(0-47)(0xff:初期値)
	u8					color;		// カラー(0-3)
	u8					stateflag;	// マーク(bit判断)
	u8					dummy;
} PKWD_RECORD_MONITOR_FLAG_BUFFER;

//-------------------------------------
/// OBJ管理ワーク
//-------------------------------------
typedef struct
{
	u32			res[RESID_OBJ_MAX]; 					// リソース
	// cmn
	GFL_CLWK	*clwk_cmn[PKWD_RECORD_MONITOR_CMN_OBJ_MAXNUM];	// CLWK
	// poke
	GFL_CLWK	*clwk_poke[PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM];	// CLWK
	// list
	GFL_CLWK	*clwk_list[PKWD_RECORD_MONITOR_LIST_OBJ_MAXNUM];	// CLWK
	// up
  GFL_CLWK  *clwk_up[PKWD_RECORD_MONITOR_UP_OBJ_MAXNUM];  // CLWK
} PKWD_RECORD_MONITOR_OBJ_WORK;

//--------------------------------------------------------------
///	メインワーク
//--------------------------------------------------------------
typedef struct
{
	HEAPID					heapID;
	// OBJ
	PKWD_RECORD_MONITOR_OBJ_WORK		wk_obj;
	//描画設定
	PKWD_RECORD_MONITOR_GRAPHIC_WORK	*graphic;
	//
	PKWD_RECORD_MONITOR_PARAM 		*Param;						// 参照用として保存
	//
	GFL_FONT				*font;									///< フォント
	PRINT_QUE				*print_que;								///< que
	PRINT_UTIL				PrintUtil[PKWD_RECORD_MONITOR_CANVAS_MAXNUM];		///< 文字表示用(下画面:切り替える)
	PRINT_UTIL				PrintUtil2[PKWD_RECORD_MONITOR_CANVAS_MAXNUM];	///< 文字表示用(上画面)
	GFL_MSGDATA				*MsgData;			///< メッセージ
	WORDSET					*Wordset;								///< wordset
	//
	//
	FRAMELIST_WORK			*frame_list_work;			///< リスト
	//
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
	s32						ScenarioMax;				// シナリオ最大
	s32						ListCurPos;					///< リストのトップ(画面上のトップ)
	s32						ListScrollPos;				///< リストのスクロール
	s32						Result;						///< 結果
	s32						OldPos;						///< 一つ前の位置
	//
	s16						*EmptyBuffer;				// 座標用のバッファ(必ず破棄すること)
	//
	PKWD_RECORD_MONITOR_FLAG_BUFFER	FlagBuffer[PKWD_RECORD_MONITOR_SCENARIO_MAX];	///< フラグのバッファ
	//
	u8	          profits[PKWD_RECORD_MONITOR_SCENARIO_MAX];	///< 収益の高いフラグ
	BOOL          first_anime_flag;        // 最初のアニメフラグ
	//
} PKWD_RECORD_MONITOR_MAIN_WORK;

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
}PKWD_RECORD_MONITOR_OBJ_DATA;

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}PKWD_RECORD_MONITOR_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}PKWD_RECORD_MONITOR_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				poff;			// パレットのオフセット
}PKWD_RECORD_MONITOR_BGSCN_DATA;

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
}PKWD_RECORD_MONITOR_CANVAS_DATA;

//=============================================================================
/**
 *							データ定義(要整理)
 */
//=============================================================================
//---------------
// BG関連(ロゴ)
//---------------
// BGPAL
static const PKWD_RECORD_MONITOR_BGPAL_DATA pwc_bgpal_tbl[] = {
	{ PALTYPE_MAIN_BG,	NARC_b_scenario_gra_pkwd_record_monitor_NCLR,	0,	0,},	// 上
	{ PALTYPE_SUB_BG,	NARC_b_scenario_gra_pkwd_scenario2_d_NCLR,	6,	2,},	// 下

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const PKWD_RECORD_MONITOR_BGCHR_DATA pwc_bgchr_tbl[] = {
	{ GFL_BG_FRAME1_M,	NARC_b_scenario_gra_pkwd_record_monitor_NCGR,	0,	0,},	// 上
	{ GFL_BG_FRAME3_S,	NARC_b_scenario_gra_pkwd_scenario2_d_NCGR,	0,	0,},	// 下

	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const PKWD_RECORD_MONITOR_BGSCN_DATA pwc_bgscn_tbl[] = {
	{ GFL_BG_FRAME1_M,	NARC_b_scenario_gra_pkwd_record_monitor_NSCR,	0,	0,},	// 上
	{ GFL_BG_FRAME2_M,	NARC_b_scenario_gra_pkwd_record_monitor2_NSCR,	0,	0,},	// 上
	{ GFL_BG_FRAME3_M,	NARC_b_scenario_gra_pkwd_record_monitor3_NSCR,	0,	0,},	// 上
	{ GFL_BG_FRAME3_S,	NARC_b_scenario_gra_pkwd_scenario2_d_NSCR,	0,	0,},	// 下

	{ 0xff, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(cmn)
//------------------
// 情報
static const s16 pwm_obj_cmn_itbl[] = {
	RESID_CMN_OBJ_PLT,
	RESID_CMN_OBJ_CHR,
	RESID_CMN_OBJ_CEL,
	//
	PKWD_RECORD_MONITOR_CMN_OBJ_MAXNUM,
	CLWKMODE_CMN,
};

// OBJのテーブルデータ
static const PKWD_RECORD_MONITOR_OBJ_DATA pwm_obj_cmn_tbl[] = {
	{  1,	0x30,	1,	1,	0,	1,	3,	RESID_CMN_OBJ_CHR,},		// 戻る
	{  4,	0x30,	1,	1,	0,	1,	0,	RESID_CMN_OBJ_CHR,},		// 左矢印
	{  5,	0x30,	1,	1,	0,	1,	1,	RESID_CMN_OBJ_CHR,},		// 右矢印

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// OBJ(poke)
//----------------
// 情報
static const s16 pwm_obj_poke_itbl[] = {
	RESID_POKE_PLT,		// plt res
	0,					// chr res(未使用)
	RESID_POKE_CEL,		// cel res
	//
	PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM,		// max
	CLWKMODE_POKE,
};

// OBJのテーブルデータ
const PKWD_RECORD_MONITOR_OBJ_DATA pwm_obj_poke_tbl[] = {
	{ 1,	0x28,	1,	0,	0,	0,	4, RESID_POKE_CHR_00,},		// ポケモン00
	{ 1,	0x28,	1,	0,	0,	0,	5, RESID_POKE_CHR_01,},		// ポケモン01
	{ 1,	0x28,	1,	0,	0,	0,	6, RESID_POKE_CHR_02,},		// ポケモン02
	{ 1,	0x28,	1,	0,	0,	0,	7, RESID_POKE_CHR_03,},		// ポケモン03
	{ 1,	0x28,	1,	0,	0,	0,	8, RESID_POKE_CHR_04,},		// ポケモン04
	{ 1,	0x28,	1,	0,	0,	0,	9, RESID_POKE_CHR_05,},		// ポケモン05
	//
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// OBJ(list)
//----------------
// narc, resid, paloffset, palnum
static const s16 pwm_obj_list_info_tbl[] = {
	NARC_b_scenario_gra_pkwd_list_obj_NCLR, RESID_LIST_PLT, PLTID_SUB_OBJ_LIST*0x20, 6, // NCL
	NARC_b_scenario_gra_pkwd_list_obj_NCGR, RESID_LIST_CHR, 0, 0,	// NCG
	NARC_b_scenario_gra_pkwd_list_obj_NCER, RESID_LIST_CEL, 0, 0,	// NCE
	NARC_b_scenario_gra_pkwd_list_obj_NANR, -1, 0, 0,				// NAR?
};

// 情報
static const s16 pwm_obj_list_itbl[] = {
	RESID_LIST_PLT,		// plt res
	RESID_LIST_CHR,		// chr res
	RESID_LIST_CEL,		// cel res
	//
	PKWD_RECORD_MONITOR_LIST_OBJ_MAXNUM,		// max
	CLWKMODE_LIST,
};

// OBJのテーブルデータ
const PKWD_RECORD_MONITOR_OBJ_DATA pwm_obj_list_tbl[] = {
	{ CLWK_LIST_ANM_SEQ_SLIDER,	0x30, 1, 0,	0,	1,	16,	RESID_LIST_CHR,},		// スクロールバー
	//
	{ CLWK_LIST_ANM_SEQ_NEW,	0x30, 2, 0,	0,	1,	17,	RESID_LIST_CHR,},		// NEWマーク
	{ CLWK_LIST_ANM_SEQ_NEW,	0x30, 2, 0,	0,	1,	18,	RESID_LIST_CHR,},		// NEWマーク
	{ CLWK_LIST_ANM_SEQ_NEW,	0x30, 2, 0,	0,	1,	19,	RESID_LIST_CHR,},		// NEWマーク
	{ CLWK_LIST_ANM_SEQ_NEW,	0x30, 2, 0,	0,	1,	20,	RESID_LIST_CHR,},		// NEWマーク
	{ CLWK_LIST_ANM_SEQ_NEW,	0x30, 2, 0,	0,	1,	21,	RESID_LIST_CHR,},		// NEWマーク
	{ CLWK_LIST_ANM_SEQ_NEW,	0x30, 2, 0,	0,	1,	22,	RESID_LIST_CHR,},		// NEWマーク
	{ CLWK_LIST_ANM_SEQ_NEW,	0x30, 2, 0,	0,	1,	23,	RESID_LIST_CHR,},		// NEWマーク
	{ CLWK_LIST_ANM_SEQ_NEW,	0x30, 2, 0,	0,	1,	24,	RESID_LIST_CHR,},		// NEWマーク
	{ CLWK_LIST_ANM_SEQ_NEW,	0x30, 2, 0,	0,	1,	25,	RESID_LIST_CHR,},		// NEWマーク
	
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//---------------
// OBJ(up)
//---------------
static const s16 pwm_obj_up_itbl[] = {
	RESID_LIST_PLT,		// plt res
	RESID_LIST_CHR,		// chr res
	RESID_LIST_CEL,		// cel res
  //
	PKWD_RECORD_MONITOR_UP_OBJ_MAXNUM,		// max
	CLWKMODE_UP,
};

// OBJのテーブルデータ
const PKWD_RECORD_MONITOR_OBJ_DATA pwm_obj_up_tbl[] = {
	{ CLWK_LIST_ANM_SEQ_CROWN, 0x30, 1, 0, 0, 1, 26, RESID_LIST_CHR,}, // 王冠
	{ CLWK_LIST_ANM_SEQ_KIRA,  0x30, 0, 0, 0, 1, 26, RESID_LIST_CHR,}, // キラキラ
	{ CLWK_LIST_ANM_SEQ_STAR,  0x30, 0, 0, 0, 1, 10, RESID_LIST_CHR,}, // スター
	{ CLWK_LIST_ANM_SEQ_STAR,  0x30, 0, 0, 0, 1, 11, RESID_LIST_CHR,}, // スター
	{ CLWK_LIST_ANM_SEQ_STAR,  0x30, 0, 0, 0, 1, 12, RESID_LIST_CHR,}, // スター
	{ CLWK_LIST_ANM_SEQ_STAR,  0x30, 0, 0, 0, 1, 13, RESID_LIST_CHR,}, // スター
	{ CLWK_LIST_ANM_SEQ_STAR,  0x30, 0, 0, 0, 1, 14, RESID_LIST_CHR,}, // スター
	{ CLWK_LIST_ANM_SEQ_STAR,  0x30, 0, 0, 0, 1, 15, RESID_LIST_CHR,}, // スター
	{ CLWK_LIST_ANM_SEQ_LINE_RED, 0x30, 2, 0, 0, 0, 27, RESID_LIST_CHR,}, // 赤いライン
	{ CLWK_LIST_ANM_SEQ_LINE_GREEN, 0x30, 2, 0, 0, 0, 28, RESID_LIST_CHR,}, // 緑ライン
	{ CLWK_LIST_ANM_SEQ_LINE_BLUE, 0x30, 2, 0, 0, 0, 29, RESID_LIST_CHR,}, // 青いライン
	// 評価アイコン
	{ CLWK_LIST_ANM_SEQ_GOOD,   0x30, 0, 0, 0, 1, 30, RESID_LIST_CHR,}, // GOOD
	{ CLWK_LIST_ANM_SEQ_BAD,    0x30, 0, 0, 0, 1, 31, RESID_LIST_CHR,}, // BAD
	{ CLWK_LIST_ANM_SEQ_STRANGE,0x30, 0, 0, 0, 1, 32, RESID_LIST_CHR,}, // STRANGE
	//
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};


//---------------
// CANVASの設定
//---------------
// 上画面
static const PKWD_RECORD_MONITOR_CANVAS_DATA pwm_canvas_00_tbl[] = {
	{ GFL_BG_FRAME0_M,	2,	1,	27,	2,	 1,},		// 「上映作品の最高記録」
	{ GFL_BG_FRAME0_M,	2, 11,	28,	2,	 2,},		//「興行成績記録」
	{ GFL_BG_FRAME0_M,  2, 17,	18,	2,	 3,},		//「主な出演キャスト」
	{ GFL_BG_FRAME0_M,	3,	4,	27,	2,	10,},		// 映画のメインタイトル
	{ GFL_BG_FRAME0_M,	3,	6,	27,	4,	10,},		// 映画のサブタイトル
	{ GFL_BG_FRAME0_M,  9, 14,	21,	2,	10,},		//「XXXX億円」
	{ GFL_BG_FRAME0_M, 20, 17,	10,	2,	10,},		// トレーナー名
	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};
// RECORD_MONITOR
static const PKWD_RECORD_MONITOR_CANVAS_DATA pwm_canvas_01_tbl[] = {
	{ GFL_BG_FRAME0_S,	2,	1,	28,	4,	10,},		// win

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

// (canvas)まとめ
static const PKWD_RECORD_MONITOR_CANVAS_DATA *pwm_canvas_tbl[] = {
	pwm_canvas_01_tbl,
};

//------------------------
// スクリーンのオフセット
//------------------------
static const s16 pwm_bgscr_tbl[] = {
	0,  192,	// 基本
	256, 0,		// 右
	0,   0,		// 下
};

//-------------
// fixメッセージ
//--------------
static const s16 pwm_fix_mestbl[] = {
	pw_record_monitor_01,     // 「上映作品の最高記録」
	pw_record_monitor_02,     // 「興行成績記録」
	pw_record_monitor_04,     // 「主な出演キャスト」
	//
	pw_castindex_06,		      // dummy(空白を表示)
};

//-------------
// fixメッセージ
//-------------
// main(上)
static const u16 pwm_fmes00_tbl[] = {
	MES_ID_TOP_01,	PWB_CVS_TOP_01,		// 「上映作品の最高記録」
	MES_ID_TOP_02,	PWB_CVS_TOP_02,		// 「興行成績記録」 
	MES_ID_TOP_03,	PWB_CVS_TOP_03,		// 「主な出演キャスト」 
	//
	0xffff, 0,// end
};

// fixメッセージ(まとめ)
static const u16 *pwm_fmes_tbl[] = {
	pwm_fmes00_tbl,
	//
};

//--------------
// 単語登録するメッセージ
//--------------
static const u16 pwm_mes_tbl[] = {
  pw_title_00,              // メインタイトル
  pw_moviesubtitle_m_01,    // サブタイトル 男
  pw_moviesubtitle_f_01,    // サブタイトル 女
	pw_record_monitor_03,     // 「タグ:4桁数値 億円」
	pw_record_monitor_05,     // 「タグ：トレーナー名」
};
#define MES_TBL_NUM   ( NELEMS(pwm_mes_tbl) )

//---------------------
// カーソル情報のempty
//---------------------
static const s16 pwm_curpos_tbl[] = {
	0, 1,			// 通常
};

//------------------
// タッチペン領域
//------------------
// RECORD_MONITOR(リスト側がほとんどやってくれる)
static const GFL_UI_TP_HITTBL pwm_rect_00_tbl[] = {
  { 168, 184, 232, 248 }, 			// 戻る
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// (タッチペン)まとめ
static const GFL_UI_TP_HITTBL *pwm_rect_tbl[] = {
	pwm_rect_00_tbl,
};

//---------------------
// リストデータ
//---------------------
// リストタッチデータ
static const FRAMELIST_TOUCH_DATA pwm_list_tp_tbl[] =
{
  { {   0,  23, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 00: 
  { {  24,  47, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 01: 
  { {  48,  71, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 02: 
  { {  72,  95, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 03: 
  { {  96, 119, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 04: 
  { { 120, 143, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 05: 
  { { 144, 167, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 06: 

  { {   4, 164, 232, 255 }, FRAMELIST_TOUCH_PARAM_RAIL },     // 07: レール

  { { 168, 192, 136, 160 }, FRAMELIST_TOUCH_PARAM_PAGE_UP },  // 08: 左
  { { 168, 192, 168, 192 }, FRAMELIST_TOUCH_PARAM_PAGE_DOWN },// 09: 右

  { { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};

// スクロールなしのタッチデータ
static const FRAMELIST_TOUCH_DATA pwm_list_tp2_tbl[] =
{
  { {   0,  23, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 00: 
  { {  24,  47, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 01: 
  { {  48,  71, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 02: 
  { {  72,  95, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 03: 
  { {  96, 119, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 04: 
  { { 120, 143, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 05: 
  { { 144, 167, 16, 231 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 06: 

  { { 168, 192, 136, 160 }, FRAMELIST_TOUCH_PARAM_PAGE_UP },  // 08: 左
  { { 168, 192, 168, 192 }, FRAMELIST_TOUCH_PARAM_PAGE_DOWN },// 09: 右

  { { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};

#include "../pkwd_book/pokewood_category.cdat"

static const FRAMELIST_HEADER pwm_frame_list_header = {
  GFL_BG_FRAME2_S,			// 上
  FRAMELIST_SUB_BG_NONE,	// 下
  
  2,   // 項目フレーム表示開始Ｘ座標
  0,    // 項目フレーム表示開始Ｙ座標(●0以外だとリストが長い場合にうまくいかない?)
  28,   // 項目フレーム表示Ｘサイズ
  3,    // 項目フレーム表示Ｙサイズ
  
  1,    // フレーム内に表示するBMPWINの表示Ｘ座標
  0,    // フレーム内に表示するBMPWINの表示Ｙ座標
  26,   // フレーム内に表示するBMPWINの表示Ｘサイズ
  3,    // フレーム内に表示するBMPWINの表示Ｙサイズ(itemSizY)
  0,    // フレーム内に表示するBMPWINのパレット
  
  {24,12,8,6,4,3,},    // スクロール速度 [0] = 最速 ※itemSizY*8を割り切れる値であること！
   1,   // 選択項目のパレット(●この番号のパレットが選択中となるようだ)
   8,   // スクロールバーのＹサイズ
   7,   // 項目登録数(●初期化前に変更すればよい)
   4,   // 背景登録数
   0,   // 初期カーソル位置
   7,   // カーソル移動範囲
   0,   // 初期スクロール値
  
  pwm_list_tp_tbl,  // タッチデータ
  
  NULL, // コールバック関数
  NULL, // コールバック関数に渡されるワーク
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT PkwdRecordMonitorProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PkwdRecordMonitorProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PkwdRecordMonitorProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
///	以下に追加
//=====================================
static void pwm_init( PKWD_RECORD_MONITOR_MAIN_WORK* wk );
static void pwm_main( PKWD_RECORD_MONITOR_MAIN_WORK* wk );
static void pwm_exit( PKWD_RECORD_MONITOR_MAIN_WORK* wk );
static void pwm_down( PKWD_RECORD_MONITOR_MAIN_WORK* wk );
static void pwm_key_all( PKWD_RECORD_MONITOR_MAIN_WORK* wk );
//
// emptyの設定
static s16  pwm_get_empty( PKWD_RECORD_MONITOR_MAIN_WORK *wk, const s32 num );
//
// bgの設定
static void pwm_init_bg( PKWD_RECORD_MONITOR_MAIN_WORK* wk );
static void pwm_init_bg2( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 type, const s32 pnum );
//
// objの設定
static void pwm_init_obj_res_cmn( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 drawtype );
static void pwm_init_obj_res_poke( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static void pwm_chg_poke_obj( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 wnum, const s32 id, const s32 sex, const u32 form );
static void pwm_init_obj_res( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr );
// 
static void pwm_init_obj_clwk( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 drawtype, const PKWD_RECORD_MONITOR_OBJ_DATA *obj_adr, const s16 *info_adr );
static void pwm_init_obj( PKWD_RECORD_MONITOR_MAIN_WORK* wk );
static void pwm_exit_obj( PKWD_RECORD_MONITOR_MAIN_WORK* wk );
static void pwm_set_obj_visible( PKWD_RECORD_MONITOR_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
static void pwm_chg_objseq( PKWD_RECORD_MONITOR_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
static BOOL pwm_is_anime_end( PKWD_RECORD_MONITOR_MAIN_WORK *wk, const s32 wmode, const s32 wnum );
//
//
// 文字列表示
static void pwm_init_string( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 type, const s32 mode );
static void pwm_exit_string( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 type );
static void pwm_main_print( PKWD_RECORD_MONITOR_MAIN_WORK *wk );
static void pwm_disp_string( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 type, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode,
								const PRINTSYS_LSB col );
static u16  pwm_get_putsize( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const STRBUF * buf, GFL_FONT * font, u32 mode, const u32 canvas_no );
static void pwm_clear_canvas( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 type, const s32 mode, const u32 color);
static void pwm_disp_fix_string( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 type, const s32 mode );
static void pwm_chk_canvas( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
// bgスクロール
static void pwm_set_bgscroll( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 mode );
//----------------
// 以下、各アプリ
//----------------
// 台本選択
static void pwm_init_record_monitor( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static void pwm_main_record_monitor( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static void pwm_exit_record_monitor( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static void pwm_key_record_monitor( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static void pwm_disp_record_monitor_string( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 mode );
static void pwm_chk_mark_anim( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 mmin, const s32 mmax, const s32 nnum );
//
// ポケモン詳細
static void pwm_disp_poke_obj( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
//
// クリア(共通)
static void pwm_clear_obj( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 mode );

//
// main処理
static BOOL pwm_main_loop( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static void pwm_chk_end( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
//
static void pwm_chg_cmn_cursor( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 mode );
static s32  pwm_chk_tp( PKWD_RECORD_MONITOR_MAIN_WORK *wk, const s32 mode );

// フラグチェック
static void pwm_chk_flag( PKWD_RECORD_MONITOR_MAIN_WORK *wk );
static void pwm_chk_endflag( PKWD_RECORD_MONITOR_MAIN_WORK *wk );
static s32  pwm_get_scenario_curpos( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 num );
//
static void pwm_disp_up_bg( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 smode );
static BOOL pwm_chk_1st_anime( PKWD_RECORD_MONITOR_MAIN_WORK * wk );

//
// フレーム関連
static void pwm_init_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static void pwm_exit_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static u32  pwm_main_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static s32  pwm_is_main_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const u32 mode );

static void pwm_listclear( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static void pwm_set_init_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static void pwm_calc_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
//
static void pwm_listcallback_draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );
static void pwm_listcallback_move(void *work, u32 pos, BOOL flag);
static void pwm_listcallback_scroll( void *work, s8 mv );
//
static void _liststr_print(      PKWD_RECORD_MONITOR_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg );
static void _liststr_print_null( PKWD_RECORD_MONITOR_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg );
static void PrintStrBuf(    PKWD_RECORD_MONITOR_MAIN_WORK* wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int strIdx, PRINTSYS_LSB col, int y );
static void _strbuf_print(  PKWD_RECORD_MONITOR_MAIN_WORK* wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int param );
//
//
// スクロールバー
static void pwm_init_scrollbar( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
static void pwm_main_scrollbar( PKWD_RECORD_MONITOR_MAIN_WORK * wk );
//
// newマーク
static void pwm_chk_newmark( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 mode );
static void pwm_scroll_newmark( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s8 rate );

// マーク
static void pwm_disp_mark( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const BOOL bflg, const s32 mode, const s32 pos );

// obj非表示
static void pwm_hide_obj( PKWD_RECORD_MONITOR_MAIN_WORK * wk );


// debug
static void pwm_debug( PKWD_RECORD_MONITOR_MAIN_WORK *wk );


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA PkwdRecordMonitorProcData = 
{
	PkwdRecordMonitorProc_Init,
	PkwdRecordMonitorProc_Main,
	PkwdRecordMonitorProc_Exit,
};

//=============================================================================
//=============================================================================
//------------------
// 関数のテーブル
//------------------
static void (*(pwm_functbl[]))(PKWD_RECORD_MONITOR_MAIN_WORK *) = {
	pwm_init_record_monitor,			// record_monitor(init)
	pwm_main_record_monitor,			// record_monitor(main)
	pwm_exit_record_monitor,			// record_monitor(exit)
};

//---------------------
// リストのコールバック
//---------------------
static const FRAMELIST_CALLBACK pwm_list_callback = {
	pwm_listcallback_draw,
	pwm_listcallback_move,
	pwm_listcallback_scroll,
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
 *	@param	*pwk					PKWD_RECORD_MONITOR_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdRecordMonitorProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_RECORD_MONITOR_MAIN_WORK 	*wk;
	
	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PKWD_RECORD_MONITOR, PKWD_RECORD_MONITOR_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(PKWD_RECORD_MONITOR_MAIN_WORK), HEAPID_PKWD_RECORD_MONITOR );
	GFL_STD_MemClear( wk, sizeof(PKWD_RECORD_MONITOR_MAIN_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_PKWD_RECORD_MONITOR;
	
	// 参照用
	wk->Param = pwk;
	//
	//描画設定初期化(上:メイン、下:サブ)
	wk->graphic	= PKWD_RECORD_MONITOR_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, wk->heapID );
	//
	// フォント作成
	wk->font  = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	//
	// PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	//
	// WORDSET生成
	wk->Wordset = WORDSET_Create( wk->heapID );
	
	//----------------
	// 通常はこの中で初期化
	//----------------
	pwm_init( wk );
	
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
 *	@param	*pwk					PKWD_RECORD_MONITOR_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdRecordMonitorProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_RECORD_MONITOR_MAIN_WORK* wk = mywk;
	
	//----------------
	// 処理を記述
	//----------------
	pwm_exit( wk );
	
	// WORDSET破棄
	WORDSET_Delete( wk->Wordset );
	
	// PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	// FONT
	GFL_FONT_Delete( wk->font );
	
	//描画設定破棄
	PKWD_RECORD_MONITOR_GRAPHIC_Exit( wk->graphic );
	
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
 *	@param	*pwk					PKWD_RECORD_MONITOR_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdRecordMonitorProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_RECORD_MONITOR_MAIN_WORK* wk = mywk;
	
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
	pwm_main( wk );
	
	// Queの処理
	pwm_main_print( wk );
	
	//2D描画
	PKWD_RECORD_MONITOR_GRAPHIC_2D_Draw( wk->graphic );
	
	//3D描画
	PKWD_RECORD_MONITOR_GRAPHIC_3D_StartDraw( wk->graphic );
	
	PKWD_RECORD_MONITOR_GRAPHIC_3D_EndDraw( wk->graphic );
	
	return GFL_PROC_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(init)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_init( PKWD_RECORD_MONITOR_MAIN_WORK* wk )
{
	s32		i;
	s32		tmp;
	
	//----------
	// 初期化
	//----------
	wk->Step 			= STEP_INIT;
	wk->Mode			= MODE_INIT_RECORD_MONITOR;
	wk->SubMode			= STEP_INIT;
	wk->TitleMode		= 0;
	wk->NextMode		= 0;
	wk->Flag			= 0;
	wk->PFlag			= 0;
	wk->PFlag2			= 0;
	wk->ScenarioCurPos 	= 0;
	wk->ScenarioMax 	= 20;
	wk->ListCurPos 		= 0;
	wk->ListScrollPos 	= 0;
	wk->Result			= 0;//PKWD_RECORD_MONITOR_NOSELECT;	// 初期値
	wk->OldPos			= -1;
	//
	wk->frame_list_work = 0;
	wk->first_anime_flag = FALSE;
	//
	for(i=0;i<PKWD_RECORD_MONITOR_SCENARIO_MAX;i++){
		wk->FlagBuffer[i].openid   = 0xff;
		wk->FlagBuffer[i].color    = 0;
		wk->FlagBuffer[i].stateflag= 0;
		wk->profits[i] = 0;
	}
	//
	// canvas
	for(i=0;i<PKWD_RECORD_MONITOR_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
		wk->PrintUtil2[i].win = 0;
	}
	//---------------------
	// empty用バッファ取得
	//(必ず破棄すること)
	//---------------------
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_B_SCENARIO_GRA, NARC_b_scenario_gra_pkwd_record_monitor_pos_tblbin, wk->heapID );
	
	//---------------------
	// メッセージバッファ取得
	//---------------------
	wk->MsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pw_movietitle_dat, wk->heapID );	// いろいろ
	
	pwm_chk_flag( wk );
	// シナリオ番号セット
	tmp = pwm_get_scenario_curpos( wk, wk->ScenarioCurPos );
	
	//---------------
	// bgの初期化
	//---------------
	pwm_init_bg(  wk );
	pwm_init_bg2( wk, GFL_BG_FRAME1_S, PLTID_SUB_BG_CMN );
	
	//------------------------
	// フォントパレットセット
	//------------------------
	// パレットは10番に転送
	{
		ARCHANDLE* p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_SUB_BG,  PLTID_SUB_FONT*0x20,  0x20, wk->heapID );
//		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, PLTID_MAIN_FONT*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
	// main
	{
		ARCHANDLE* p_handle	= GFL_ARC_OpenDataHandle( ARCID_B_SCENARIO_GRA, wk->heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_b_scenario_gra_pkwd_list_fnt_NCLR, PALTYPE_MAIN_BG, PLTID_MAIN_FONT*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}

	//------------
	// OBJ初期化
	//------------
	pwm_init_obj( wk );
	//
	
	//---------------
	// stringの初期化
	//---------------
	pwm_init_string( wk, STR_MAIN, 0 );				// 1回のみ
	pwm_init_string( wk, STR_SUB,  TMODE_RECORD_MONITOR );
	
	// fix文字列
	pwm_disp_fix_string( wk, STR_MAIN, 0 );
	//
	pwm_disp_record_monitor_string( wk, 0 );
	
	//------------
	// パレット
	//------------
	
	// (初期)カラーの変更
	tmp = wk->FlagBuffer[wk->ScenarioCurPos].color;
	
	//------------
	// リスト処理
	//------------
	pwm_init_framelist( wk );
	
	// カーソル後処理
	pwm_chk_mark_anim( wk, 0, wk->ScenarioMax-1, wk->ScenarioCurPos );
	
	//--------------------
	// スクロール切り替え
	//--------------------
	pwm_set_bgscroll( wk, 0 );
	
	//------------
	// 上画面表示
	//------------
//	pwm_disp_up_bg( wk, 0 );
	GFL_BG_SetVisible( GFL_BG_FRAME0_M, FALSE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_main( PKWD_RECORD_MONITOR_MAIN_WORK* wk )
{
	// 下画面
	pwm_down( wk );
	
	// debug
	pwm_debug( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(exit)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_exit( PKWD_RECORD_MONITOR_MAIN_WORK* wk )
{
	s32		i;
	
	// 終了チェック(フラグ後処理)
	pwm_chk_endflag( wk );
	
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	
	// stringの破棄
	pwm_exit_string( wk, STR_MAIN );
	pwm_exit_string( wk, STR_SUB );
	
	//メッセージ破棄
  if(wk->MsgData){
    GFL_MSG_Delete( wk->MsgData );
    wk->MsgData = NULL;
  }
	// リスト破棄
	pwm_exit_framelist( wk );
	
	// OBJ破棄
	pwm_exit_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(下画面)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_down( PKWD_RECORD_MONITOR_MAIN_WORK* wk )
{
	// 入力
	pwm_key_all( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理まとめ(処理の流れmain)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_key_all( PKWD_RECORD_MONITOR_MAIN_WORK* wk )
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
			bflg = pwm_main_loop( wk );
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
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
static s16 pwm_get_empty( PKWD_RECORD_MONITOR_MAIN_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= PKWD_RECORD_MONITOR_EMPTY_MAXNUM)){
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
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_init_bg( PKWD_RECORD_MONITOR_MAIN_WORK* wk )
{
	u16		*adr;
	s32		i,j,k;
	s32		tmp;
	u32		off;
	const PKWD_RECORD_MONITOR_BGPAL_DATA	*padr = &pwc_bgpal_tbl[0];
	const PKWD_RECORD_MONITOR_BGCHR_DATA	*cadr = &pwc_bgchr_tbl[0];
	const PKWD_RECORD_MONITOR_BGSCN_DATA	*sadr = &pwc_bgscn_tbl[0];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_B_SCENARIO_GRA, wk->heapID );
	
	// パレット転送
	for(i=0;i<PKWD_RECORD_MONITOR_BGNUM_MAXNUM;i++){
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
	for(i=0;i<PKWD_RECORD_MONITOR_BGNUM_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, wk->heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<PKWD_RECORD_MONITOR_BGNUM_MAXNUM;i++){
		// 終了条件
		if(sadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramScreen( handle, sadr->arcid, sadr->frmnumber, sadr->offset, 0, 0, wk->heapID );
		sadr++;
	}
	// 終了
	GFL_ARC_CloseDataHandle( handle );
	//
	//--------------------
	// スクリーンの書き換え
	//--------------------
	// スクリーンバッファ指定
	adr = GFL_BG_GetScreenBufferAdrs( GFL_BG_FRAME3_S );
	
	// 3画面分修正
	// 1枚のスクリーン(512x512)に3画面分のデータを入れているため
	for(k=0;k<3;k++){
		for(i=0;i<32;i++){
			for(j=0;j<32;j++){
				// オフセットを加算
				adr[k*32*32+i*32+j] += ((PLTID_SUB_BG_BASE&0x0f)<<12);
			}
		}
	}
	GFL_BG_LoadScreenReq( GFL_BG_FRAME3_S );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化(cmn)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *	@param	const s32			(BG面)
 *	@param	const s32			(パレット番号)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_init_bg2( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 type, const s32 pnum )
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
//	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(cmn res)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_init_obj_res_cmn( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 drawtype )
{
	// cmnのidを取得
	const u32 	arcid    = APP_COMMON_GetArcId();
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( arcid, wk->heapID );
    GFL_CLUNIT*	p_clunit = PKWD_RECORD_MONITOR_GRAPHIC_GetClunit( wk->graphic );
	u32			idx[4];
	//
	idx[0] = APP_COMMON_GetBarIconPltArcIdx();		// pal
	idx[1] = APP_COMMON_GetBarIconCharArcIdx();		// ncg
	idx[2] = APP_COMMON_GetBarIconCellArcIdx(  APP_COMMON_MAPPING_128K );	// cell
	idx[3] = APP_COMMON_GetBarIconAnimeArcIdx( APP_COMMON_MAPPING_128K );	// anim
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ RESID_CMN_OBJ_PLT ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, idx[0], drawtype, 0x20*PLTID_SUB_OBJ_CMN, 0, 4, wk->heapID );
	wk->wk_obj.res[ RESID_CMN_OBJ_CHR ] = GFL_CLGRP_CGR_Register(      p_handle, idx[1], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ RESID_CMN_OBJ_CEL ] = GFL_CLGRP_CELLANIM_Register( p_handle, idx[2], idx[3], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ポケモンアイコンのグラフィックロード
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_init_obj_res_poke( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	ARCHANDLE	*ah;
	u32			i;
	
	ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, GFL_HEAP_LOWID(wk->heapID) );
	// パレット(0-3)
	wk->wk_obj.res[RESID_POKE_PLT] = GFL_CLGRP_PLTT_RegisterComp( ah, 
												POKEICON_GetPalArcIndex(),
												CLSYS_DRAW_MAIN, PLTID_MAIN_POKE*0x20, wk->heapID );
	// セル・アニメ
	wk->wk_obj.res[RESID_POKE_CEL] = GFL_CLGRP_CELLANIM_Register( ah,
												POKEICON_GetCellSubArcIndex(),
												POKEICON_GetAnmSubArcIndex(),
												wk->heapID );
	// キャラ(6体)
	for(i=0;i<PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM;i++){
		wk->wk_obj.res[RESID_POKE_CHR_00+i] = GFL_CLGRP_CGR_Register( ah,
												POKEICON_GetCgxArcIndexByMonsNumber( 0, 0, 0, 0 ),
												FALSE, CLSYS_DRAW_MAIN, wk->heapID );
	}
	//
	GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ポケモンアイコン切り替え(cgrの置き換え)
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 * @param	const s32			ワーク番号(reschrと対応)
 * @param	const s32			id
 * @param	const s32			性別
 * @param const u32     フォルム
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_chg_poke_obj( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 wnum, const s32 id, const s32 sex, const u32 form )
{
	NNSG2dCharacterData * dat;
	void * buf;
	ARCHANDLE * ah;
	u8 pal_num = POKEICON_GetPalNum( id, form, sex, FALSE );
	
	ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, GFL_HEAP_LOWID(wk->heapID) );
	//
	// CGRを置き換える方法
	buf = GFL_ARCHDL_UTIL_LoadOBJCharacter( ah, POKEICON_GetCgxArcIndexByMonsNumber( id, form, sex, FALSE ), FALSE, &dat, GFL_HEAP_LOWID(wk->heapID) );
	//
	GFL_CLGRP_CGR_Replace( wk->wk_obj.res[RESID_POKE_CHR_00+wnum], dat );
	GFL_HEAP_FreeMemory( buf );
	//
	GFL_ARC_CloseDataHandle( ah );
	//
	// パレット変更(転送した番号(読み込ませた際にセットされる) + offset)
	GFL_CLACT_WK_SetPlttOffs( wk->wk_obj.clwk_poke[wnum], pal_num, CLWK_PLTTOFFS_MODE_PLTT_TOP );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(res)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_init_obj_res( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr )
{
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_B_SCENARIO_GRA, wk->heapID );
    GFL_CLUNIT*	p_clunit = PKWD_RECORD_MONITOR_GRAPHIC_GetClunit( wk->graphic );
	const s16 *adr		 = info_adr;		// アドレスセット
	//
	// セルアニメ用の処理
	//wk->wk_obj.res[ adr[0+1] ] = GFL_CLGRP_PLTT_RegisterComp(   p_handle, adr[0], drawtype, adr[2], wk->heapID );
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
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *	@param	const s32					drawtype
 *	@param	const PKWD_RECORD_MONITOR_OBJ_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_init_obj_clwk( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 drawtype, const PKWD_RECORD_MONITOR_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16						tmp;
	s16						pos[2];
	s16						buf[6];
	s32 					i;
	BOOL					bflg;
	GFL_CLWK_DATA 			cldata;
	CLWK_FLIP_TYPE			flip;
    GFL_CLUNIT*				p_clunit 	= PKWD_RECORD_MONITOR_GRAPHIC_GetClunit( wk->graphic );
	// アドレスセット
	const PKWD_RECORD_MONITOR_OBJ_DATA *adr 	= obj_adr;
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
	if(buf[4] == CLWKMODE_CMN){		// cmn
		clwk = &wk->wk_obj.clwk_cmn[0];
	}
	else if(buf[4] == CLWKMODE_POKE){		// poke
		clwk = &wk->wk_obj.clwk_poke[0];
	}
	else if(buf[4] == CLWKMODE_LIST){		// list
		clwk = &wk->wk_obj.clwk_list[0];
	}
	else if(buf[4] == CLWKMODE_UP )    // up
	{
		clwk = &wk->wk_obj.clwk_up[0];
	}
	
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		// 決めうち
		pos[0] = pwm_get_empty(wk, tmp*2);
		pos[1] = pwm_get_empty(wk, tmp*2+1);
		cldata.pos_x 	= pos[0];
		cldata.pos_y 	= pos[1];
    ICHI_Printf("x %d y %d\n", pos[0], pos[1]);
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
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_init_obj( PKWD_RECORD_MONITOR_MAIN_WORK* wk )
{
	GFL_CLUNIT*	p_clunit = PKWD_RECORD_MONITOR_GRAPHIC_GetClunit( wk->graphic );
	//
	// リソース読み込み(cmn)
	pwm_init_obj_res_cmn( wk, CLSYS_DRAW_SUB );
	// リソース読み込み(poke)
	pwm_init_obj_res_poke( wk );
	// リソース読み込み(list)
	pwm_init_obj_res( wk, CLSYS_DRAW_MAX, &pwm_obj_list_info_tbl[0]);
	//
	// CLWK登録(cmn)
	pwm_init_obj_clwk( wk, CLSYS_DRAW_SUB, &pwm_obj_cmn_tbl[0], &pwm_obj_cmn_itbl[0] );
	// CLWK登録(poke)
	pwm_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &pwm_obj_poke_tbl[0], &pwm_obj_poke_itbl[0] );
	// CLWK登録(list)
	pwm_init_obj_clwk( wk, CLSYS_DRAW_SUB, &pwm_obj_list_tbl[0], &pwm_obj_list_itbl[0] );
	// CLWK登録(up)
	pwm_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &pwm_obj_up_tbl[0], &pwm_obj_up_itbl[0] );
	//
	// 通信アイコンセット(上画面)
	GFL_NET_ReloadIconTopOrBottom( TRUE, wk->heapID );
	//
  // 半透明OBJ
  GFL_CLACT_WK_SetObjMode( wk->wk_obj.clwk_up[CLWK_UP_LINE_RED], GX_OAM_MODE_XLU );
  GFL_CLACT_WK_SetObjMode( wk->wk_obj.clwk_up[CLWK_UP_LINE_GREEN], GX_OAM_MODE_XLU );
  GFL_CLACT_WK_SetObjMode( wk->wk_obj.clwk_up[CLWK_UP_LINE_BLUE], GX_OAM_MODE_XLU );

  // アルファ設定
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_NONE|GX_BLEND_PLANEMASK_BG3,
		GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
		8, 8 );

}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_exit_obj( PKWD_RECORD_MONITOR_MAIN_WORK* wk )
{
	s32		i;
	
	// cmn
	for(i=0;i<PKWD_RECORD_MONITOR_CMN_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk_cmn[i] );
	}
	// poke
	for(i=0;i<PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk_poke[i] );
	}
	// list
	for(i=0;i<PKWD_RECORD_MONITOR_LIST_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk_list[i] );
	}
  // up
	for(i=0;i<PKWD_RECORD_MONITOR_UP_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk_up[i] );
	}

	//
	//リソース破棄(cmn)
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_CMN_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_CMN_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_CMN_OBJ_CEL] );
	//
	//リソース破棄(poke)
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_POKE_PLT] );
	for(i=0;i<PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM;i++){
		GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_POKE_CHR_00+i] );
	}
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_POKE_CEL] );
	//
	//リソース破棄(list)
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_LIST_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_LIST_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_LIST_CEL] );
	//
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *	@param	const s32			(CLWKMODE_*)
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_chg_objseq( PKWD_RECORD_MONITOR_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	// CMN
	if(wmode == CLWKMODE_CMN){
		if(wnum >= PKWD_RECORD_MONITOR_CMN_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk_cmn[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk_cmn[wnum], TRUE );
	}
	// POKE
	else if(wmode == CLWKMODE_POKE){
		if(wnum >= PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk_poke[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk_poke[wnum], TRUE );
	}
	// LIST
	else if(wmode == CLWKMODE_LIST){
		if(wnum >= PKWD_RECORD_MONITOR_LIST_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk_list[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk_list[wnum], TRUE );
	}
  // UP
	else if(wmode == CLWKMODE_UP){
		if(wnum >= PKWD_RECORD_MONITOR_UP_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk_up[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk_up[wnum], TRUE );
    GFL_CLACT_WK_ResetAnm( wk->wk_obj.clwk_up[wnum] );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *	@param	const s32			(CLWKMODE_*)
 *	@param	const s32			objワーク番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_set_obj_visible( PKWD_RECORD_MONITOR_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// cmn
	if(wmode == CLWKMODE_CMN){
		// 表示/非表示
		if(wnum >= PKWD_RECORD_MONITOR_CMN_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk_cmn[wnum], bflg );
	}
	// poke
	else if(wmode == CLWKMODE_POKE){
		// 表示/非表示
		if(wnum >= PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk_poke[wnum], bflg );
	}
	// list
	else if(wmode == CLWKMODE_LIST){
		// 表示/非表示
		if(wnum >= PKWD_RECORD_MONITOR_LIST_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk_list[wnum], bflg );
	}
	// up
	else if(wmode == CLWKMODE_UP){
		// 表示/非表示
		if(wnum >= PKWD_RECORD_MONITOR_UP_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk_up[wnum], bflg );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのアニメの終了待ち
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *	@param	const s32			(CLWKMODE_*)
 *	@param	const s32			objワーク番号
 *
 *	@retval	const BOOL			(TRUE:再生中、FALSE:停止、終了)
 */
//-----------------------------------------------------------------------------
static BOOL pwm_is_anime_end( PKWD_RECORD_MONITOR_MAIN_WORK *wk, const s32 wmode, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// cmn
	if(wmode == CLWKMODE_CMN){
		if(wnum >= PKWD_RECORD_MONITOR_CMN_OBJ_MAXNUM){
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
		if(wnum >= PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		// 判定
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk_poke[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	// list
	else if(wmode == CLWKMODE_LIST){
		if(wnum >= PKWD_RECORD_MONITOR_LIST_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		// 判定
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk_list[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	// up
	else if(wmode == CLWKMODE_UP){
		if(wnum >= PKWD_RECORD_MONITOR_UP_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
    }
		// 判定
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk_up[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	return( bflg );
}


//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(Canvasの作成)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *	@param	const s32					(STR_MAIN or STR_SUB)
 *	@param	const s32					(SUBの(0-2)、MAINでは無視)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_init_string( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 type, const s32 mode )
{
	s32		i;
	const PKWD_RECORD_MONITOR_CANVAS_DATA *adr;
	
	if(type == STR_MAIN){	// main(基本1回)
		adr = &pwm_canvas_00_tbl[0];
		wk->PFlag2 = 0;
	}else{					// sub(何度も書き換えがある)
		adr = pwm_canvas_tbl[mode];
		wk->PFlag = 0;
		// 破棄
		pwm_exit_string( wk, STR_SUB );
	}
	//
	for(i=0;i<PKWD_RECORD_MONITOR_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット
		if(type == STR_MAIN){		// main
			wk->PrintUtil2[i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		}else{						// sub
			wk->PrintUtil[i].win  = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		}
		// clear
		pwm_clear_canvas( wk, type, i, MES_CLEAR_COLOR );
		//
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字列破棄
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* 		wk
 *	@param	const s32					(STR_MAIN or STR_SUB)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_exit_string( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 type )
{
	s32		i;
	
	if(type == STR_MAIN){		// main
		for(i=0;i<PKWD_RECORD_MONITOR_CANVAS_MAXNUM;i++){
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
		for(i=0;i<PKWD_RECORD_MONITOR_CANVAS_MAXNUM;i++){
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
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK*			wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_main_print( PKWD_RECORD_MONITOR_MAIN_WORK *wk )
{
	s32		i;
	BOOL	bflg;
	BOOL	pflg;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<PKWD_RECORD_MONITOR_CANVAS_MAXNUM;i++){
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
	pwm_chk_canvas( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* 			wk
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
static void pwm_disp_string( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 type, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode,
								const PRINTSYS_LSB col )
{
	u16		siz;
	
	// clear
	pwm_clear_canvas( wk, type, mode, MES_CLEAR_COLOR );
	
	// 表示位置の補正
	siz = pwm_get_putsize( wk, strbuf, wk->font, pmode, mode );
	if(type == STR_MAIN){		// main
		PRINT_UTIL_PrintColor( &wk->PrintUtil2[mode], wk->print_que, x+siz, y, strbuf, wk->font, col );
	}else{						// sub
		PRINT_UTIL_PrintColor( &wk->PrintUtil[mode],  wk->print_que, x+siz, y, strbuf, wk->font, col );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	文字列表示サイズから各モードの補正値を取得(static)
 *			(ui/printtoolから持ってきた)
 *
 * @param PKWD_RECORD_MONITOR_MAIN_WORK
 * @param	const STRBUF*	buf		文字列
 * @param	GFL_FONT*		font	フォント
 * @param	u32				mode	表示モード(0-2)
 * @param const u32 canvas_no キャンバスの番号
 *
 * @return	補正値
 */
//--------------------------------------------------------------------------------------------
static u16  pwm_get_putsize( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const STRBUF * buf, GFL_FONT * font, u32 mode, const u32 canvas_no )
{
  u32 x = 0;

	// 右詰
	if( mode == 1 ){
    x = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp( wk->PrintUtil2[canvas_no].win ) );
		x -= PRINTSYS_GetStrWidth( buf, font, 0 );
	// 中央
	}else if( mode == 2 ){
    x = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp( wk->PrintUtil2[canvas_no].win ) ) / 2;
		x -= PRINTSYS_GetStrWidth( buf, font, 0 ) / 2;
	}

	return x;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア
 *
 *	@param	BSCENARIO_WORK*		wk
 *	@param	const s32			(STR_MAIN or STR_SUB)
 *	@param	const s32			キャンバスの番号
 *	@param	const u32			クリアカラー
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_clear_canvas( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 type, const s32 mode, const u32 color)
{
	if(type == STR_MAIN){
		if(wk->PrintUtil2[mode].win){
			// clear
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil2[mode].win ), color );
			wk->PrintUtil2[mode].transReq = TRUE;
		}
	}else{
		if(wk->PrintUtil[mode].win){
			// clear
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), color );
			wk->PrintUtil[mode].transReq = TRUE;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BGのスクロール(問題があればvblanck中の処理に移動)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* wk
 *	@param	const 32			(段階:(0-2))
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_set_bgscroll( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 mode )
{
	s16		buf[2];
	
	buf[0] = pwm_bgscr_tbl[2*mode];				// x
	buf[1] = pwm_bgscr_tbl[2*mode+1];			// y
	// セット
	GFL_BG_SetScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_SET, buf[0] );
	GFL_BG_SetScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, buf[1] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	fix文字表示
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*	wk
 * @param	const s32				(STR_MAIN or STR_SUB)
 * @param	const s32				モード(0-2)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_disp_fix_string( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 type, const s32 mode )
{
	u16		buf[2];
	const u16	*adr;
	u16		col;
	s32		i;
	STRBUF	*strbuf;
	
	// 指定の方法注意(要修正)
	adr = pwm_fmes_tbl[type+mode];
	for(i=0;i<PKWD_RECORD_MONITOR_CANVAS_MAXNUM;i++){
		// 終了判定
		if(adr[2*i] == 0xffff){
			break;
		}
		buf[0] = adr[2*i];		// mesid
		buf[1] = adr[2*i+1];	// id
		strbuf = GFL_MSG_CreateString( wk->MsgData, pwm_fix_mestbl[ buf[0] ] );

    if( buf[0] == MES_ID_TOP_01 )
    {
      // 中央揃え
      pwm_disp_string( wk, type, buf[1], 0, 0, strbuf, 2, TEXT_COL_FIX );
    }
    else
    {
      pwm_disp_string( wk, type, buf[1], 0, 0, strbuf, 0, TEXT_COL_FIX );
    }
		
		GFL_STR_DeleteBuffer( strbuf );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フォント表示待ち
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void pwm_chk_canvas( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
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
		pwm_set_bgscroll( wk, wk->TitleMode );
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
 * @brief	初期化(record_monitor)
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_init_record_monitor( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	// main処理へ
	wk->Mode 		= MODE_MAIN_RECORD_MONITOR;
	wk->TitleMode	= TMODE_RECORD_MONITOR;
	//
	// bg初期化(1回目だけは転送しない)
//	if(wk->Flag & FLG_SET_INIT){
//		wk->Flag |= FLG_CHG_TITLE;
//		// 矢印位置
//		pwm_chg_cmn_cursor( wk, 0 );
//		// frame再セット
//		pwm_set_init_framelist( wk );
//	}
	// objの初期化
	pwm_clear_obj( wk, 0 );
  // キーとタッチでもう1回処理
//	pwm_disp_up_bg( wk, 0 );
	//
	// スクロールバー初期化
	pwm_init_scrollbar( wk );
	//
	// 再計算
	pwm_chk_newmark( wk, 1 );
	pwm_scroll_newmark( wk, 0 );
	//
	//
	wk->Flag |= FLG_SET_INIT;
	wk->Flag |= FLG_UPDATE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	main(record_monitor)
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_main_record_monitor( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	s32		tmp;
	s32		flg;
	u32		ret;
	BOOL	bflg;
	
	switch( wk->SubMode ){
		// 初期化
		case STEP_INIT:
			if(!(wk->Flag & FLG_FRAME_STAT)){
				if( FRAMELIST_Init( wk->frame_list_work ) == FALSE ){
					wk->SubMode = STEP_INIT+10;
					wk->Flag |= FLG_FRAME_STAT;
				}
			}else{
//				wk->SubMode = STEP_LV1;
				wk->SubMode = STEP_INIT+10;
			}
			break;
		//
		// ここで上画面のチェックを行う
		case STEP_INIT+10:
			bflg = pwm_chk_1st_anime( wk );
			if(bflg == TRUE){
				wk->SubMode = STEP_LINE_ANIME;
			}else{
				wk->SubMode = STEP_LV1;
			}
			break;
		//
		// ラインのobjアニメ
		case STEP_LINE_ANIME:
			// ラインのOBJアニメ
			GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_OFF );
			GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_ON );
			GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_OFF );
			GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_OFF );
			
			// ラインのOBJ表示
			pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_LINE_RED, TRUE );
			pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_LINE_GREEN, TRUE );
			pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_LINE_BLUE, TRUE );
			
			pwm_chg_objseq( wk, CLWKMODE_UP, CLWK_UP_LINE_RED, CLWK_LIST_ANM_SEQ_LINE_RED );
			pwm_chg_objseq( wk, CLWKMODE_UP, CLWK_UP_LINE_GREEN, CLWK_LIST_ANM_SEQ_LINE_GREEN );
			pwm_chg_objseq( wk, CLWKMODE_UP, CLWK_UP_LINE_BLUE, CLWK_LIST_ANM_SEQ_LINE_BLUE );
			wk->SubMode = STEP_LINE_ANIME_WAIT;
			break;
		case STEP_LINE_ANIME_WAIT:
			// ラインのOBJアニメ終了待ち
			if( pwm_is_anime_end(wk, CLWKMODE_UP, CLWK_UP_LINE_RED) == FALSE
				&& pwm_is_anime_end(wk, CLWKMODE_UP, CLWK_UP_LINE_GREEN) == FALSE
				&& pwm_is_anime_end(wk, CLWKMODE_UP, CLWK_UP_LINE_BLUE) == FALSE
			)
			{
//				GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );
				GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_OFF );
				GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
				GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_ON );
				
				pwm_disp_record_monitor_string( wk, 1 );
				// poke icon 表示
				pwm_disp_poke_obj( wk );
				// フラグセット(すでに見た)
				wk->FlagBuffer[wk->ScenarioCurPos].stateflag |= FLG_PWRM_FINISH;
//				wk->SubMode = STEP_LV1;
				wk->SubMode = STEP_LV3+10;
				wk->Flag |= FLG_TOUCH_DISP;
				//
				wk->Flag |= FLG_ANIM_END;
			}
			break;
		// 通常
		case STEP_LV1:
			pwm_key_record_monitor( wk );
			break;
		//
		// 待ち(戻る)
		case STEP_LV2:
			bflg = pwm_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_BACK);
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->NextMode = MODE_END;
			}
			break;
		//
		// アニメ待ち(右)
		case STEP_LV2+10:
			bflg = pwm_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_RIGHT);
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
				wk->Mode++;
			}
			break;
		//
		// アニメ待ち(左)
		case STEP_LV2+20:
			bflg = pwm_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_LEFT);
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
				wk->Mode++;
			}
			break;
		//
		// リストスクロール待ち
		case STEP_LV3:
			// main
			ret = pwm_main_framelist( wk );
			if(ret == FRAMELIST_RET_NONE){
				// 再計算
				pwm_calc_framelist( wk );
				wk->Flag |= FLG_UPDATE;
//				wk->SubMode = STEP_LV1;
				wk->SubMode = STEP_LV3+5;
				// アニメフラグoff
//				wk->Flag &= ~(FLG_LEFT_ANIM|FLG_RIGHT_ANIM);
			}
			break;
		//
		// 矢印アニメ終了待ち
		case STEP_LV3+5:
			if(wk->Flag & FLG_LEFT_ANIM){
				bflg = pwm_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_LEFT);
				if(bflg == FALSE){
					wk->SubMode = STEP_LV1;
					wk->Flag &= ~(FLG_LEFT_ANIM|FLG_RIGHT_ANIM);
					// カーソル後処理
					pwm_chk_mark_anim( wk, 0, wk->ScenarioMax-1, wk->ScenarioCurPos );
				}
			}
			else if(wk->Flag & FLG_RIGHT_ANIM){
				bflg = pwm_is_anime_end(wk, CLWKMODE_CMN, CLWK_CMN_RIGHT);
				if(bflg == FALSE){
					wk->SubMode = STEP_LV1;
					wk->Flag &= ~(FLG_LEFT_ANIM|FLG_RIGHT_ANIM);
					// カーソル後処理
					pwm_chk_mark_anim( wk, 0, wk->ScenarioMax-1, wk->ScenarioCurPos );
				}
			}else{
				// アニメなしの場合
				wk->SubMode = STEP_LV1;
				// カーソル後処理
				pwm_chk_mark_anim( wk, 0, wk->ScenarioMax-1, wk->ScenarioCurPos );
			}
			break;
		//
		case STEP_LV3+10:
			// 終了してから、上画面を切り替える
			// =>FakeNetModeの時に問題がでてしまうため
			if( PRINTSYS_QUE_IsFinished( wk->print_que ) == TRUE ){
				// 決定時は非表示からタッチの場合がある
				// =>表示タイミングをずらしている
				if(wk->Flag & FLG_TOUCH_DISP){
					pwm_disp_up_bg( wk, 1 );
					wk->Flag &= ~FLG_TOUCH_DISP;
				}
				wk->SubMode = STEP_LV1;
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
 * @brief	exit(record_monitor)
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_exit_record_monitor( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	// 終了判定
	pwm_chk_end( wk );
	// リストクリア
	pwm_listclear( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	key(record_monitor)
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_key_record_monitor( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	s32		flg=0;		// 更新の有無
	s32		mflg=0;		// 決定の有無
	s32		listflg=0;
	s32		kflg=0;		// 入力したか
	s32		nnflg=0;
	s32		id;
	s32		tmp;
	u32		ret;
	BOOL	bflg;
	
	// 処理中は更新しない
	if( PRINTSYS_QUE_IsFinished( wk->print_que ) == FALSE ){
		return;
	}
	
	// main
	ret = pwm_main_framelist( wk );
	listflg = pwm_is_main_framelist( wk, ret );
	//
	// カーソル移動=>STEPが変わる(更新は後)
	if(listflg == 1){
		flg = 2;
		kflg = 1;
	}
	// キーにタッチ=>そのまま(更新)
	else if((listflg == 2)||(listflg == 3)){
		flg = 1;
		kflg = 2;
	}
	
	// 更新あり
	if(wk->Flag & FLG_UPDATE){
		flg = 1;
		wk->Flag &= ~FLG_UPDATE;
	}
	
	if(flg == 0){
		// タッチペンの判定(戻るのみ)
		id = pwm_chk_tp( wk, wk->TitleMode );
		if(id == 0){
			mflg = 1;
		}
		// キー入力
		if(mflg == 0){
			// 戻る
			if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
				mflg = 1;
			}
		}
		// 戻る
		if(mflg == 1){
			// 戻るシーケンス変更
			pwm_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_BACK, SEQ_CMN_BACK_ANIM );
			wk->SubMode = STEP_LV2;
			// SE
			PMSND_PlaySE( SND_PKWD_RECORD_MONITOR_CANCEL );
			//
			// 終了処理
			if(id == 0){		// タッチで終了
				GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
			}else{				// キーで終了
				GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
			}
		}
	}
	// 更新
	if(flg == 1){
		wk->Flag |= FLG_STR_WAIT;
	}
	//
	// 入力判定処理
	if(wk->first_anime_flag == FALSE){
		// キー入力か項目をタッチ
		if(( GFL_UI_KEY_GetTrg() )||(kflg == 2)){
			wk->SubMode = STEP_LINE_ANIME;
			wk->first_anime_flag = TRUE;
		}
	}
	// 上文字列更新
	if(wk->Flag & FLG_STR_WAIT){
		// 通常
		if(wk->Flag & FLG_ANIM_END){
			nnflg = 0;
			if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
				nnflg = 1;
			}
			// 番号をタッチ
			if(wk->Flag & FLG_TP_NUMBER){
				nnflg = 1;
			}
			if(nnflg == 1){
				pwm_disp_record_monitor_string( wk, 1 );
				// poke icon 表示
				pwm_disp_poke_obj( wk );
				// フラグセット(すでに見た)
				wk->FlagBuffer[wk->ScenarioCurPos].stateflag |= FLG_PWRM_FINISH;
				// BG表示
//				pwm_disp_up_bg( wk, 1 );
				wk->SubMode = STEP_LV3+10;
				wk->Flag |= FLG_TOUCH_DISP;
				//
				wk->Flag &= ~FLG_TP_NUMBER;
			}
		}
#if 0
		//
		// 画面に何も出ていない状態ならアニメ開始から入る
		if( wk->first_anime_flag == TRUE)
		{
			// 1回きりにしておく
			if(!(wk->Flag & FLG_ANIM_END)){
				pwm_disp_record_monitor_string( wk, 1 );
				// poke icon 表示
				pwm_disp_poke_obj( wk );
				// フラグセット(すでに見た)
				wk->FlagBuffer[wk->ScenarioCurPos].stateflag |= FLG_PWRM_FINISH;
				//
				wk->Flag |= FLG_ANIM_END;
			}
		}
#endif
		// カラーの変更
		//tmp = wk->FlagBuffer[wk->ScenarioCurPos].color;
		wk->Flag &= ~FLG_STR_WAIT;
	}
	//
	// あまりやりたくないが、
	// 初期のカーソル位置転送
	// (FRAMELIST_Main)を通す必要があるためここでセット
	if(!(wk->Flag & FLG_FRAME_SET_PAL)){
		FRAMELIST_SetCursorPosPalette( wk->frame_list_work );
		wk->Flag |= FLG_FRAME_SET_PAL;
	}
	//
	// スクロールバー処理(要検討)
	pwm_main_scrollbar( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	文字列表示(record_monitor)
 *        上画面の更新
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 * @param	const s32					(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_disp_record_monitor_string( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 mode )
{
	s32			i;
	s32			id;
	s32			num;
	STRBUF	*strbuf;
	STRBUF	*namebuf;
	
	// カーソル位置からシナリオのidを計算
	id = pwm_get_scenario_curpos( wk, wk->ScenarioCurPos );
	
	// 初期のみでよいはず
	if(mode == 0){
    	// トレーナ名
		strbuf  = GFL_MSG_CreateString( wk->MsgData, pwm_mes_tbl[4] );
		namebuf = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, wk->heapID );
		WORDSET_RegisterPlayerName( wk->Wordset, 1, wk->Param->mystatus );
		WORDSET_ExpandStr( wk->Wordset, namebuf, strbuf );
		GFL_STR_DeleteBuffer( strbuf );
		// 表示
		pwm_disp_string( wk, STR_MAIN, PWB_CVS_TOP_07, 0, 0, namebuf, 1, TEXT_COL_MAIN );
		GFL_STR_DeleteBuffer( namebuf );
	}
  //
  // タグ展開
  {
    // ポケウッドのセーブ取得
    const POKEWOOD_BOXOFFICE_RECORD *p_pkwd_record = POKEWOOD_SAVEDATA_GetBoxOfficeData( wk->Param->pokewood_sv, id );

    // メインタイトル
    {
      u32 str_id = pwm_mes_tbl[0] + id;
      strbuf = GFL_MSG_CreateString( wk->MsgData, str_id );
      pwm_disp_string( wk, STR_MAIN, PWB_CVS_TOP_04, 0, 0, strbuf, 0, TEXT_COL_MAIN );
      GFL_STR_DeleteBuffer( strbuf );
    }
    //
    // サブタイトル
    {
      u32 str_id;
      
      // 男女別でラベルがある
      if( MyStatus_GetMySex( wk->Param->mystatus ) == PM_MALE )
      {
        str_id = pwm_mes_tbl[1] + id;
      }
      else
      {
        str_id = pwm_mes_tbl[2] + id;
      }
      //
      {
		STRBUF  *monsbuf = GFL_STR_CreateBuffer( 128, wk->heapID );
		u32 monsno,form,sex;
		//
		strbuf = GFL_MSG_CreateString( wk->MsgData, str_id );
		POKEICON_UnpackResourceData( p_pkwd_record->pokeicon[0], &monsno, &form, &sex );
		WORDSET_RegisterPokeMonsNameNo( wk->Wordset, 0, monsno );
		WORDSET_RegisterPlayerName( wk->Wordset, 1, wk->Param->mystatus );
		WORDSET_ExpandStr( wk->Wordset, monsbuf, strbuf );
		GFL_STR_DeleteBuffer( strbuf );
		//
		pwm_disp_string( wk, STR_MAIN, PWB_CVS_TOP_05, 0, 0, monsbuf, 0, TEXT_COL_MAIN );
		GFL_STR_DeleteBuffer( monsbuf );
      }
    }
    // XXXX億円
	strbuf  = GFL_MSG_CreateString( wk->MsgData, pwm_mes_tbl[3] );
	namebuf = GFL_STR_CreateBuffer( 128, wk->heapID );
	num = p_pkwd_record->profits;
	if(num > POKEWOOD_PROFIT_VALUE_MAX){
		num = POKEWOOD_PROFIT_VALUE_MAX;
		OS_Printf("max over:%d\n",p_pkwd_record->profits);
	}
	else if(num < POKEWOOD_PROFIT_VALUE_MIN){
		num = POKEWOOD_PROFIT_VALUE_MIN;
		OS_Printf("min over:%d\n",p_pkwd_record->profits);
	}
	WORDSET_RegisterNumber( wk->Wordset, 2, num, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->Wordset, namebuf, strbuf );
	GFL_STR_DeleteBuffer( strbuf );
	// 表示
	pwm_disp_string( wk, STR_MAIN, PWB_CVS_TOP_06, 0, 0, namebuf, 1, TEXT_COL_MAIN );
	GFL_STR_DeleteBuffer( namebuf );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  マークの判定(後処理)
 *
 *	@param	PKWD_RECORD_MONITOR_MAIN_WORK* 	wk
 *	@param	const s32 mmin		最小
 *	@param	const s32 mmin		最大
 *	@param	const s32 mmin		現在
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwm_chk_mark_anim( PKWD_RECORD_MONITOR_MAIN_WORK* wk, const s32 mmin, const s32 mmax, const s32 nnum )
{
	// ないはずだが念のため
	if(mmax < 0){
		OS_Printf("max error :%d\n",mmax);
		return;
	}
	//
	// 最小チェック
	if((mmin >= nnum)){
		// off
		pwm_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_LEFT, SEQ_CMN_LEFT_OFF );
	}else{
		// on
		pwm_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_LEFT, SEQ_CMN_LEFT );
	}
	//
	// 最大チェック
	if((nnum >= mmax)){
		// off
		pwm_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_RIGHT, SEQ_CMN_RIGHT_OFF );
	}else{
		// on
		pwm_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_RIGHT, SEQ_CMN_RIGHT );
	}
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
/**
 * @brief	obj表示(poke)
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_disp_poke_obj( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	s32 i;
	s32 id = pwm_get_scenario_curpos( wk, wk->ScenarioCurPos );
  // ポケウッドのセーブ取得
  const POKEWOOD_BOXOFFICE_RECORD *p_pkwd_record = POKEWOOD_SAVEDATA_GetBoxOfficeData( wk->Param->pokewood_sv, id );
	BOOL	bflg;
	
	//--------
	// ポケモン
	//--------
  for( i=0; i<PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM; i++ )
  {
    // ポケモンアイコン用情報取得
    u32 monsno,form,sex;
    POKEICON_UnpackResourceData( p_pkwd_record->pokeicon[i], &monsno, &form, &sex );

    ICHI_Printf("mon_no %d i %d\n", monsno, i);

    if( monsno > 0 )
    {
      pwm_chg_poke_obj( wk, i, monsno, sex, form );
      pwm_set_obj_visible( wk, CLWKMODE_POKE, i, TRUE );

      // スターポケ判定
      if( p_pkwd_record->pokeicon_star_bit & (1<<i) )
      {
        pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_STAR_00+i, TRUE );
      }
      else
      {
        pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_STAR_00+i, FALSE );
      }
    }
    else
    {
      pwm_set_obj_visible( wk, CLWKMODE_POKE, i, FALSE );
      pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_STAR_00+i, FALSE );
    }
  }


	// 王冠判定
  {
    ICHI_Printf("scenario_id %d profits %d \n", id, wk->profits[id]);
    if( wk->profits[ id ] == 1 )
    {
      pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_CROWN, TRUE );
      pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_KIRA, TRUE );
    }
    else
    {
      pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_CROWN, FALSE );
      pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_KIRA, FALSE );
    }
  }
	// 評価マーク表示
	for(i=0;i<3;i++){
		bflg = FALSE;
		if( wk->FlagBuffer[wk->ScenarioCurPos].stateflag & (1<<i) ){
			bflg = TRUE;
		}
		pwm_disp_mark( wk, bflg, i, i );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	objクリア
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void pwm_clear_obj( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 mode )
{
	s32		i;
	
	// 表示したいものはこの後で表示すること
	// poke
	for(i=0;i<PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM;i++){
		pwm_set_obj_visible( wk, CLWKMODE_POKE, i, FALSE );
	}
	//
	// list
	for(i=0;i<PKWD_RECORD_MONITOR_LIST_OBJ_MAXNUM;i++){
		pwm_set_obj_visible( wk, CLWKMODE_LIST, i, FALSE );
	}
	// up
	for(i=0;i<PKWD_RECORD_MONITOR_UP_OBJ_MAXNUM;i++){
		pwm_set_obj_visible( wk, CLWKMODE_UP, i, FALSE );
	}
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	処理まとめ
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return	BOOL				(TRUE:制御中、FALSE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL pwm_main_loop( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	BOOL	bflg=TRUE;
	BOOL	tflg;
	//
	if((wk->Mode < 0)||(wk->Mode >= MODE_MAX)){
		OS_Printf("mode error\n");
		return( FALSE );
	}
	// 処理の分岐
	pwm_functbl[wk->Mode]( wk );
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
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_chk_end( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	// 次の処理
	if(wk->NextMode != MODE_END){
		wk->Mode = wk->NextMode;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	カーソル位置修正
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 * @param	const s32					(0:通常、1:ポケモン情報時)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_chg_cmn_cursor( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 mode )
{
	s16				i;
	s16				buf[2];
	GFL_CLACTPOS	pos;
	//
	// emptyを取得する
	buf[0] = pwm_curpos_tbl[0];		// 左側
	buf[1] = pwm_curpos_tbl[1];		// 右側
	//
	for(i=0;i<2;i++){
		pos.x = pwm_get_empty( wk, buf[i]*2 );
		pos.y = pwm_get_empty( wk, buf[i]*2+1 );
		// 設定
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk_cmn[CLWK_CMN_LEFT+i], &pos, CLSYS_DEFREND_SUB );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン処理
 *
 *	@param	BSCENARIO_WORK*		wk
 *	@param	const s32			段階(0-2)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s32 pwm_chk_tp( PKWD_RECORD_MONITOR_MAIN_WORK *wk, const s32 mode )
{
	s32	id;
	//
	// タッチペン判定
	id = GFL_UI_TP_HitTrg( pwm_rect_tbl[mode] );
	return( id );
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	フラグチェック
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_chk_flag( PKWD_RECORD_MONITOR_MAIN_WORK *wk )
{
	s32		i;
	s32		j;
	u16		max_profits = 0;  // 最高収益
	u16		scenario[PKWD_RECORD_MONITOR_SCENARIO_MAX];  // 最高収益
	u8		profits_idx=0;
	BOOL	bflg;
	//
	// openチェック
	j=0;
	for(i=0;i<PKWD_RECORD_MONITOR_SCENARIO_MAX;i++){
		bflg = POKEWOOD_SAVEDATA_GetPokewoodFlag( wk->Param->pokewood_sv, PWSV_TYPE_SENARIO_THEATERGO, i );
		if(bflg == TRUE){
			const POKEWOOD_BOXOFFICE_RECORD *p_pkwd_record = POKEWOOD_SAVEDATA_GetBoxOfficeData( wk->Param->pokewood_sv, i );
			wk->FlagBuffer[j].openid = i;
			//
			// new or upチェック
			{
				// 閲覧してない場合が0
				if( p_pkwd_record->new_flag == 0 ){
					wk->FlagBuffer[j].stateflag |= FLG_PWRM_NEW;
				}
				if( p_pkwd_record->update_flag == 1 ){
					wk->FlagBuffer[j].stateflag |= FLG_PWRM_UP;
				}
			}
			//
			// 王冠チェック
			{
				if( p_pkwd_record->profits >=  max_profits )
				{
					if( p_pkwd_record->profits >  max_profits )
					{
						profits_idx = 0;
					}
					max_profits = p_pkwd_record->profits;
					// 保存
					scenario[ profits_idx ] = i;
					profits_idx++;
				}
			}
			//----------------------
			// 評価マークのチェック
			//----------------------
			// GOOD
			bflg = POKEWOOD_SAVEDATA_GetPokewoodFlag( wk->Param->pokewood_sv, PWSV_TYPE_SENARIO_SUCCESS, i );
			if(bflg == TRUE){
				wk->FlagBuffer[j].stateflag |= FLG_PWRM_GOOD;
			}
			// BAD
			bflg = POKEWOOD_SAVEDATA_GetPokewoodFlag( wk->Param->pokewood_sv, PWSV_TYPE_SENARIO_BAD, i );
			if(bflg == TRUE){
				wk->FlagBuffer[j].stateflag |= FLG_PWRM_BAD;
			}
			// STRANGE
			bflg = POKEWOOD_SAVEDATA_GetPokewoodFlag( wk->Param->pokewood_sv, PWSV_TYPE_SENARIO_STRANGE, i );
			if(bflg == TRUE){
				wk->FlagBuffer[j].stateflag |= FLG_PWRM_STRANGE;
			}
			// colorチェック
			// カラーは変更しない
			wk->FlagBuffer[j].color = PokewoodCategoryData[i]&0x03;
			//
			j++;
		}
	}
	// シナリオ最大数が求まるはず
	wk->ScenarioMax = j;
	// 最高収益のシナリオにフラグをたてる
	for(i=0;i<profits_idx;i++){
		wk->profits[ scenario[ i ] ] = 1;
		ICHI_Printf(" 収益 %d \n", scenario[i]);
	}
	// 結果
	for(i=0;i<wk->ScenarioMax;i++){
		OS_Printf("flag :%d,%d,%d\n",wk->FlagBuffer[i].openid, wk->FlagBuffer[i].color, wk->FlagBuffer[i].stateflag );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	終了時時のチェック
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_chk_endflag( PKWD_RECORD_MONITOR_MAIN_WORK *wk )
{
	s32		i;
	
	// すでに見たかチェック
	for(i=0;i<wk->ScenarioMax;i++){
		if(wk->FlagBuffer[i].stateflag & FLG_PWRM_FINISH){
			// NewとUpdateの両方を落としてくれる
			POKEWOOD_SAVEDATA_ClearUpdate( wk->Param->pokewood_sv, wk->FlagBuffer[i].openid );
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	カーソル位置のシナリオのidを返す
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 * @param	const s32					カーソル位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static s32 pwm_get_scenario_curpos( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 num )
{
	s32			id;
	
	// リミットチェック
	if(num >= wk->ScenarioMax){
		OS_Printf("scenario max error :%d\n",num);
		return -1;//( PKWD_RECORD_MONITOR_NOSELECT );
	}
	
	// カーソル位置のidをセット
	id = wk->FlagBuffer[num].openid;
	return( id );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief	上画面のBGのon/off
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 * @param	const s32					(0:引き継ぐ、1:表示)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_disp_up_bg( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 smode )
{
	u8		flag = VISIBLE_ON;
	int		mode;
	//
	// 引き継ぐ
	if(smode == 0){
		// 限定で大丈夫なはずなのでここで判定
		mode = GFL_UI_CheckTouchOrKey();
		//
		// タッチはoff
		if(mode == GFL_APP_END_TOUCH){
			flag = VISIBLE_OFF;
		}
		// 一旦消しておく
		GFL_BG_SetVisible( GFL_BG_FRAME0_M, flag );
	}
	// 表示
	else{
//		flag = VISIBLE_ON;
		// フォント面のみを考慮にいれる
		GFL_BG_SetVisible( GFL_BG_FRAME0_M, flag );
	}
	//
	// 非表示の場合は消す
	if(flag == VISIBLE_OFF){
		pwm_hide_obj( wk );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	上画面のBGのアニメをするかチェック
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  BOOL								(TRUE:再生、FALSE:なし)
 */
//--------------------------------------------------------------------------------------------
static BOOL pwm_chk_1st_anime( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	BOOL	bflg=FALSE;
	//
	// 処理済み
	if(wk->first_anime_flag == TRUE){
		return( bflg );
	}
	// チェック
	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
		bflg = TRUE;
		wk->first_anime_flag = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームリスト初期化
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_init_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	FRAMELIST_HEADER  header = pwm_frame_list_header;
	ARCHANDLE * arc_handle;
	s32		i;
	//
	header.cbWork = wk;
	//
	// 破棄
	pwm_exit_framelist( wk );
	//
	// 読み込み
	arc_handle = GFL_ARC_OpenDataHandle( ARCID_B_SCENARIO_GRA, wk->heapID );
	
	//----------------------------
	// オリジナルデータの読み込み
	//----------------------------
	GFL_ARCHDL_UTIL_TransVramBgCharacter( arc_handle, NARC_b_scenario_gra_pkwd_list_NCGR, GFL_BG_FRAME2_S, 0, 0, 0, wk->heapID );
	GFL_ARCHDL_UTIL_TransVramPalette(     arc_handle, NARC_b_scenario_gra_pkwd_list_col_NCLR, PALTYPE_SUB_BG,  0, 0x20, wk->heapID );	// 0番
	
	//---------------
	// FrameListの設定
	//---------------
	// パラメータ設定
	header.cbFunc  = &pwm_list_callback;
	header.itemMax 		= wk->ScenarioMax;			// 重要
	header.initPos		= wk->ListCurPos;
	header.initScroll	= wk->ListScrollPos;
	if(wk->ScenarioMax <= PKWD_RECORD_MONITOR_LIST_MAX){
		header.touch = pwm_list_tp2_tbl;
	}
	
	// 生成
	wk->frame_list_work = FRAMELIST_Create( &header, wk->heapID );
	
	// 項目背景設定
	FRAMELIST_LoadFrameGraphicAH( wk->frame_list_work, arc_handle, NARC_b_scenario_gra_pkwd_list_p1_NSCR, FALSE, 0 );		// 通常
	FRAMELIST_LoadFrameGraphicAH( wk->frame_list_work, arc_handle, NARC_b_scenario_gra_pkwd_list_p2_NSCR, FALSE, 1 );		// 通常
	FRAMELIST_LoadFrameGraphicAH( wk->frame_list_work, arc_handle, NARC_b_scenario_gra_pkwd_list_p3_NSCR, FALSE, 2 );		// 通常
	FRAMELIST_LoadFrameGraphicAH( wk->frame_list_work, arc_handle, NARC_b_scenario_gra_pkwd_list_p4_NSCR, FALSE, 3 );		// 通常
	// 点滅アニメパレット設定
	// (NARC_b_scenario_gra_pkwd_list_col_NCLR)の0番パレット(16色)と1番パレット(16色)間を補間してくれる
	// 補間後のパレットはFRAMELIST_HEADERのselPalの指定で可能
	FRAMELIST_LoadBlinkPalette(   wk->frame_list_work, arc_handle, NARC_b_scenario_gra_pkwd_list_col_NCLR, 0, 1 );

	//
	GFL_ARC_CloseDataHandle( arc_handle );
	//
	// 項目追加(タイトル文字をセット)
	// ここではセットだけのようだ
	for(i=0;i<header.itemMax;i++){
//		FRAMELIST_AddItem( wk->frame_list_work, 0, pw_title_00+wk->FlagBuffer[i].openid );
		FRAMELIST_AddItem( wk->frame_list_work, wk->FlagBuffer[i].color, pw_title_00+wk->FlagBuffer[i].openid );
	}
	// 初期化
	wk->Flag &= ~FLG_FRAME_STAT;
	if( FRAMELIST_Init( wk->frame_list_work ) == FALSE ){
		wk->Flag |= FLG_FRAME_STAT;
	}
	//
	// 背景色をセット
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x679d );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームMain
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static u32 pwm_main_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	u32		ret = FRAMELIST_RET_NONE;
	// Main
	if( wk->frame_list_work ){
		// PAGEUP、PAGEDOWNの場合は、スクロールが終了時に、
		// パラメータをセットする必要がある(タイミング注意)
		ret = FRAMELIST_Main( wk->frame_list_work );
	}
	return( ret );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームMainのchk
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*	wk
 *
 * @return  s32						(0:なし、1:カーソル移動、2:決定)
 */
//--------------------------------------------------------------------------------------------
static s32 pwm_is_main_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const u32 mode )
{
	s16			flg=0;
	s16			mflg=0;
	u32			pnum;
	
	switch(mode){
		case 0:								// 項目をタッチ
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			flg = 2;
			break;
		//
		case FRAMELIST_RET_PAGE_UP:			// page up
		case FRAMELIST_RET_PAGE_UP_NONE:	// page up 2
			wk->Flag |= FLG_LEFT_ANIM;
			flg = 1;
			break;
		case FRAMELIST_RET_PAGE_DOWN:		// page down
		case FRAMELIST_RET_PAGE_DOWN_NONE:	// page down 2
			wk->Flag |= FLG_RIGHT_ANIM;
			flg = 1;
			break;
		//
		case FRAMELIST_RET_JUMP_TOP:		// top
		case FRAMELIST_RET_JUMP_BOTTOM:		// bottom
			flg = 1;
			mflg = 1;
			break;
		//
		case FRAMELIST_RET_MOVE:			// move
		case FRAMELIST_RET_SCROLL:			// move
		case FRAMELIST_RET_RAIL:
			flg = 1;
			break;
		case FRAMELIST_RET_CURSOR_ON:		// カーソルがついた(タッチ/キー)
			flg = 3;
			break;
		default:
			break;
	}
	// リスト移動
	if(flg == 1){
		wk->SubMode = STEP_LV3;
		// 要整理
		if(wk->Flag & FLG_LEFT_ANIM){
			// 左シーケンス変更
			pwm_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_LEFT, SEQ_CMN_LEFT_ANIM );
		}
		else if(wk->Flag & FLG_RIGHT_ANIM){
			// 右シーケンス変更
			pwm_chg_objseq( wk, CLWKMODE_CMN, CLWK_CMN_RIGHT, SEQ_CMN_RIGHT_ANIM );
		}
		//
		// LRの処理
		// スクロールのcallbackは呼ばれないため、
		// newマークが更新されない
		// =>ここで処理を通すようにする
		if(mflg == 1){
			// カーソル位置の再設定(カーソル点滅用)
			pnum = FRAMELIST_GetCursorPos( wk->frame_list_work );
			FRAMELIST_SetCursorPos( wk->frame_list_work, pnum );
			// 再計算(端になるので初期化(1)でよい)
			pwm_chk_newmark( wk, 1 );
		}
	}
	// 決定
	else if((flg == 2)||(flg == 3)){
		// カーソル位置の取得
		pwm_calc_framelist( wk );
		//
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
			// SE
			PMSND_PlaySE( SND_PKWD_RECORD_MONITOR_ARROW );
		}
		// 点滅(消えているはず)
		FRAMELIST_ChangePosPalette( wk->frame_list_work, wk->ListCurPos, 1 );
		// 項目をタッチしたときに点滅させるために、
		// キーで終了したものとする(特別処理)
		if(flg == 2){
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		}
		//
		// アニメ終了してから判定
		if( wk->first_anime_flag == TRUE){
			wk->Flag |= FLG_TP_NUMBER;
		}
		// カーソル後処理
		pwm_chk_mark_anim( wk, 0, wk->ScenarioMax-1, wk->ScenarioCurPos );
	}
	//
	// ここでなくてもいいはず
	// 上画面表示
	if(flg == 1){
		pwm_disp_up_bg( wk, 0 );
	}
	return( flg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレーム破棄
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_exit_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	// 破棄
	if( wk->frame_list_work ){
		FRAMELIST_Exit( wk->frame_list_work );
		wk->frame_list_work = 0;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームクリア
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_listclear( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	GFL_BG_ClearScreen( GFL_BG_FRAME2_S );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
	// 戻り用の再計算
	pwm_calc_framelist( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームリスト初期化
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_set_init_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	// 初期化
	if(wk->frame_list_work){
		// FRAMELIST_Initだけでは、カーソル情報が再セットできずに
		// 復帰させることができない(=>スクロール値がセットできない)
		// そのため、再度初期化しカーソル情報をセットするようにした
		pwm_init_framelist( wk );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	リストカーソル位置再計算
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_calc_framelist( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	// 初期化
	if(wk->frame_list_work){
		wk->ScenarioCurPos = FRAMELIST_GetListPos(  wk->frame_list_work );		// 通しカーソル位置
		wk->ListCurPos     = FRAMELIST_GetCursorPos(wk->frame_list_work );		// 画面上の位置
		wk->ListScrollPos  = FRAMELIST_GetScrollCount(wk->frame_list_work );	// スクロール位置
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームのcallback関数(draw,move,scroll)
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
// draw
static void pwm_listcallback_draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
	PKWD_RECORD_MONITOR_MAIN_WORK * wk = (PKWD_RECORD_MONITOR_MAIN_WORK *)work;
	// タイトル表示
	_liststr_print( wk, pos, print_util, wk->MsgData );
}

// move
static void pwm_listcallback_move(void *work, u32 pos, BOOL flag)
{
//	PKWD_RECORD_MONITOR_MAIN_WORK * wk = (PKWD_RECORD_MONITOR_MAIN_WORK *)work;
//	// 再計算
//	pwm_calc_framelist( wk );
}

// scroll
static void pwm_listcallback_scroll( void *work, s8 mv )
{
	PKWD_RECORD_MONITOR_MAIN_WORK * wk = (PKWD_RECORD_MONITOR_MAIN_WORK *)work;
	// 再計算
	pwm_chk_newmark( wk, 0 );
	pwm_scroll_newmark( wk, mv );
}

//----------------------------------------------------------------------------------
/**
 * @brief コールバック関数共通文字列描画処理
 *
 * @param   wk    
 * @param   pos   
 * @param   print_util    
 * @param   msg   
 */
//----------------------------------------------------------------------------------
static void _liststr_print( PKWD_RECORD_MONITOR_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg )
{
	PRINT_QUE *que = FRAMELIST_GetPrintQue( wk->frame_list_work );
	int param = FRAMELIST_GetItemParam( wk->frame_list_work, pos );
	//
	_strbuf_print( wk, print_util, que, msg, param );
}

static void _liststr_print_null( PKWD_RECORD_MONITOR_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg )
{
	PRINT_QUE *que = FRAMELIST_GetPrintQue( wk->frame_list_work );
	int param = FRAMELIST_GetItemParam( wk->frame_list_work, pos );
	//
	PrintStrBuf( wk, print_util, que, msg, param, PRINTSYS_MACRO_LSB(13,12,0), 4 );
}

static void PrintStrBuf( PKWD_RECORD_MONITOR_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int strIdx, PRINTSYS_LSB col, int y )
{
	int width, bmp_width, x;
	STRBUF * strbuf = GFL_MSG_CreateString( msg, strIdx );
	//
	width     = PRINTSYS_GetStrWidth( strbuf, wk->font, 0 );
	bmp_width = GFL_BMPWIN_GetSizeX( print_util->win ) * 8;
	x         = 8+(bmp_width - width)/2;
	//
	PRINT_UTIL_PrintColor( print_util, que, x, y, strbuf, wk->font, col );
	GFL_STR_DeleteBuffer( strbuf );
}

static void _strbuf_print( PKWD_RECORD_MONITOR_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int param )
{
	PrintStrBuf( wk, print_util, que, msg, param, PRINTSYS_MACRO_LSB(15,14,0), 4 );
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	スクロールバー初期化
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_init_scrollbar( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	BOOL	bflg=TRUE;
	//
	if(wk->ScenarioMax <= PKWD_RECORD_MONITOR_LIST_MAX){
		bflg = FALSE;
	}
	// 表示
	pwm_set_obj_visible( wk, CLWKMODE_LIST, CLWK_LIST_BALL, bflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	スクロールバー処理
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_main_scrollbar( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	// 表示
	GFL_CLACTPOS cp_pos;
	// 念のため
	if(wk->ScenarioMax <= PKWD_RECORD_MONITOR_LIST_MAX){
		return;
	}
	
	// 座標の取得
	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk_list[CLWK_LIST_BALL], &cp_pos, CLSYS_DEFREND_SUB );
	cp_pos.y = FRAMELIST_GetScrollBarPY2( wk->frame_list_work, cp_pos.y );
//	cp_pos.x = 240;		// test
	//
	// RAILの領域の(+-4補正:アイコンの半分の大きさ補正)
	if( cp_pos.y < 8 ){
		cp_pos.y = 8;
	}else if( cp_pos.y > 160 ){
		cp_pos.y = 160;
	}
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk_list[CLWK_LIST_BALL], &cp_pos, CLSYS_DEFREND_SUB );
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	NEWマークチェック
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 * @param	s32							(0:通常、1:初期化)
 *
 * @return  
 */
//--------------------------------------------------------------------------------------------
static void pwm_chk_newmark( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s32 mode )
{
	s16				mbuf[2];
	s32				i;
	s32				off;
	s32				buf[2];
	BOOL			bflg;
	u32				NewFlag;
	u32				UpFlag;
	GFL_CLACTPOS	pos;
	
	// 初期化
	NewFlag = 0;
	UpFlag  = 0;
	if(mode == 1){
		wk->OldPos = -1;
	}
	// 画面上の一番上の位置
	buf[0] = FRAMELIST_GetScrollCount( wk->frame_list_work );
	if(buf[0] == wk->OldPos){
		return;
	}
	// 補正=>
	// スクロール値を参照しているため、
	// 方向によっては座標をずらす必要があったので、こう処理した
	if(buf[0] > wk->OldPos){	// 下方向
		off = 1;
	}else{						// 上方向
		off = -1;
	}
	// 初期化
	if(mode == 1){
		off = 0;
	}
	//
	// (0-6),(-1),(7)の順番
	for(i=0;i<PKWD_RECORD_MONITOR_LIST_MAX;i++){
		if(wk->FlagBuffer[buf[0]+i].stateflag & FLG_PWRM_NEW){
			NewFlag |= (1<<i);
		}
		if(wk->FlagBuffer[buf[0]+i].stateflag & FLG_PWRM_UP){
			UpFlag |= (1<<i);
		}
	}
	//
	// 0でない場合=>上側がある
	if(buf[0] > 0){
		if(wk->FlagBuffer[buf[0]-1].stateflag & FLG_PWRM_NEW){
			NewFlag |= (1<<PKWD_RECORD_MONITOR_LIST_MAX);
		}
		if(wk->FlagBuffer[buf[0]-1].stateflag & FLG_PWRM_UP){
			UpFlag |= (1<<PKWD_RECORD_MONITOR_LIST_MAX);
		}
	}
	//
	// 下側にデータがある場合
	if((buf[0]+PKWD_RECORD_MONITOR_LIST_MAX) < wk->ScenarioMax){
		if(wk->FlagBuffer[buf[0]+PKWD_RECORD_MONITOR_LIST_MAX].stateflag & FLG_PWRM_NEW){
			NewFlag |= (1<<(PKWD_RECORD_MONITOR_LIST_MAX+1));
		}
		if(wk->FlagBuffer[buf[0]+PKWD_RECORD_MONITOR_LIST_MAX].stateflag & FLG_PWRM_UP){
			UpFlag |= (1<<(PKWD_RECORD_MONITOR_LIST_MAX+1));
		}
	}
	//
	// ここまでで、NewFlag,UpFlagがセットされる
	// 表示/非表示までをセット
	for(i=0;i<9;i++){
		bflg = FALSE;
		if(NewFlag & (1<<i)){
			bflg = TRUE;
			// newマーク
			pwm_chg_objseq( wk, CLWKMODE_LIST, CLWK_LIST_NEW_00+i, CLWK_LIST_ANM_SEQ_NEW );
		}
		else if(UpFlag & (1<<i)){
			bflg = TRUE;
			// upマーク
			pwm_chg_objseq( wk, CLWKMODE_LIST, CLWK_LIST_NEW_00+i, CLWK_LIST_ANM_SEQ_UP );
		}
		pwm_set_obj_visible( wk, CLWKMODE_LIST, CLWK_LIST_NEW_00+i, bflg );
	}
	//
	// 座標初期化
	for(i=0;i<9;i++){
		GFL_CLACT_WK_GetPos( wk->wk_obj.clwk_list[CLWK_LIST_NEW_00+i], &pos, CLSYS_DEFREND_SUB );
		pos.x = pwm_get_empty( wk, (17+i)*2 );
		pos.y = pwm_get_empty( wk, (17+i)*2+1 );
		pos.y += off*24;
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk_list[CLWK_LIST_NEW_00+i], &pos, CLSYS_DEFREND_SUB );
	}
	//
	wk->OldPos = buf[0];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	NEWマーク(使い方注意)
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 * @param	const s8					BGのスクロール値
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_scroll_newmark( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const s8 rate )
{
	s32				i;
	GFL_CLACTPOS	pos;
	//
	for(i=0;i<9;i++){
		GFL_CLACT_WK_GetPos( wk->wk_obj.clwk_list[CLWK_LIST_NEW_00+i], &pos, CLSYS_DEFREND_SUB );
		pos.y -= rate;		// bgなので反転
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk_list[CLWK_LIST_NEW_00+i], &pos, CLSYS_DEFREND_SUB );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	マーク表示
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 * @param	const BOOL							(TRUE:表示、FALSE:非表示)
 * @param	const s32 mode						(0:GOOD、1:BAD、2:STRANGE)
 * @param	const s32 pos						(0-2:場所
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_disp_mark( PKWD_RECORD_MONITOR_MAIN_WORK * wk, const BOOL bflg, const s32 mode, const s32 pos )
{
	// 表示/非表示
	pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_MARK_GOOD+pos, bflg );
	if(bflg == TRUE){
		pwm_chg_objseq(  wk, CLWKMODE_UP, CLWK_UP_MARK_GOOD+pos, CLWK_LIST_ANM_SEQ_GOOD+mode );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	obj非表示
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_hide_obj( PKWD_RECORD_MONITOR_MAIN_WORK * wk )
{
	s32 i;
	//
	// ポケモン
	for(i=0;i<PKWD_RECORD_MONITOR_POKE_OBJ_MAXNUM;i++){
		// ポケモン
		pwm_set_obj_visible( wk, CLWKMODE_POKE, i, FALSE );
		// スター
		pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_STAR_00+i, FALSE );
	}
	// 王冠
	pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_CROWN, FALSE );
	pwm_set_obj_visible( wk, CLWKMODE_UP, CLWK_UP_KIRA, FALSE );
	//
	// 評価マーク表示
	for(i=0;i<3;i++){
		pwm_disp_mark( wk, FALSE, i, i );
	}
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	debug
 *
 * @param	PKWD_RECORD_MONITOR_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void pwm_debug( PKWD_RECORD_MONITOR_MAIN_WORK *wk )
{
//#ifdef PM_DEBUG
#if 0		// 未使用
  static int ev1;
  static int ev2;
	
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    BOOL update = FALSE;
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
    {
      ev1++;
      update = TRUE;
      OS_Printf("ev1 %d\n", ev1);
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
    {
      ev1--;
      update = TRUE;
      OS_Printf("ev1 %d\n", ev1);
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
    {
      ev2++;
      update = TRUE;
      OS_Printf("ev2 %d\n", ev2);
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
    {
      ev2--;
      update = TRUE;
      OS_Printf("ev2 %d\n", ev2);
    }

    if( update )
    {
      G2_ChangeBlendAlpha(ev1, ev2);
    }
  }
  
	// 評価マークのonチェック
	{
		s32		i;
		
		// Rを押しながら
		if(GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
			// X:GOOD
			if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_X){
				for(i=0;i<wk->ScenarioMax;i++){
					POKEWOOD_SAVEDATA_SetPokewoodFlag( wk->Param->pokewood_sv, PWSV_TYPE_SENARIO_SUCCESS, wk->FlagBuffer[i].openid );
					wk->FlagBuffer[i].stateflag |= FLG_PWRM_GOOD;
				}
			}
			// Y:BAD
			if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y){
				for(i=0;i<wk->ScenarioMax;i++){
					POKEWOOD_SAVEDATA_SetPokewoodFlag( wk->Param->pokewood_sv, PWSV_TYPE_SENARIO_BAD, wk->FlagBuffer[i].openid );
					wk->FlagBuffer[i].stateflag |= FLG_PWRM_BAD;
				}
			}
			// A:STRANGE
			if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_A){
				for(i=0;i<wk->ScenarioMax;i++){
					POKEWOOD_SAVEDATA_SetPokewoodFlag( wk->Param->pokewood_sv, PWSV_TYPE_SENARIO_STRANGE, wk->FlagBuffer[i].openid );
					wk->FlagBuffer[i].stateflag |= FLG_PWRM_STRANGE;
				}
			}
		}
	}
#endif
}

