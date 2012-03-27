//=============================================================================
/**
 *
 *	@file		fmission_app_sub.c
 *	@brief  フェスミッションアプリ サブ
 *	@author Miyuki Iwasawa	
 *	@data	  2011.04.22	
 */
//=============================================================================
//必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "ui/print_tool.h"

#include "app/app_menu_common.h"

#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"

#include "fmission_app_local.h"
#include "fmission_app_sub.h"

#include "fmission_app.naix"

#include "msg/msg_fmission_app.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
// いろいろ
enum{
	FMA_BGNUM_MAX		= 8,
	FMA_EMPTY_MAXNUM	= 14*2,
	//
	MES_CLEAR_COLOR     = 0,
	MES_SYSWIN_CLEAR_COLOR		= 15,	// (システムウインドウの)クリアカラー
	//
	//
	// (main bg)(0-5:bg、6:cmn、8:font)
	FMA_BG_BASE_PAL		= 0,
	FMA_BG_CMN_PAL		= 6,
	//
	FMA_BG_FONT_PAL		= 8,
	//
	// (sub bg)(0-5:bg、8:font)
	FMA_BG_SUB_BASE_PAL	= 0,
	FMA_BG_SUB_WIN_PAL	= 4,
	FMA_BG_SUB_FONT_PAL	= 8,
	//
	// (obj)(0:cmn、2-4:obj、14:net)
	FMA_OBJ_BASE_PAL	= 0,
	FMA_OBJ_MAIN_PAL	= 2,
	//
	// (sub)(0-1:base)
	FMA_OBJ_SUB_BASE_PAL = 0,
	FMA_OBJ_SUB_CHG_FONT = 1,
};

//--------------
// フラグ
//--------------
// OBJのフラグ(T_DOWNLOAD_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM		= (1<<0),		// アニメあり
	FLG_OBJ_HFLIP		= (1<<1),		// HFLIP
	FLG_OBJ_VFLIP		= (1<<2),		// VFLIP
};

//=============================================================================
/**
 *								定義
 */
//=============================================================================
//-----------------------------------------
// BGPALの定義
//-----------------------------------------
typedef struct{
	u8				type;			///< タイプ(0xffの場合は終了判定)
	u8				arcid;			///< id
	u8				offset;			///< オフセット
	u8				size;			///< サイズ(*0x20)
}FMA_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//-----------------------------------------
typedef struct{
	u8				frmnumber;		///< frame(0xffの場合は終了判定)
	u8				arcid;			///< id
	u8				offset;			///< オフセット
	u8				pad;			///< pad
}FMA_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//-----------------------------------------
typedef struct{
	u8				frmnumber;		///< frame(0xffの場合は終了判定)
	u8				arcid;			///< id
	u8				offset;			///< オフセット
	u8				mode;			///< (0:詳細、1:結果)
}FMA_BGSCN_DATA;

//-----------------------------------------
// 文字用の定義(表示領域の設定:CANVAS)
//------------------------------------------
typedef struct{
  u8				frmnumber;			///< BG上下(frame定義なので注意)(0xffの場合は終了判定)
  u8				x;					///< x座標
  u8				y;					///< y座標
  u8				width;				///< 横サイズ
  u8				height;				///< 縦サイズ
  u8				palnum;				///< パレット番号
}FMA_CANVAS_DATA;

//-----------------------------------------
// OBJ用の定義
//------------------------------------------
typedef struct{
	u8				anmseq;			///< アニメシーケンス(0xff終了コード)
	u8				softpri;		///< OBJ(プライオリティ)
	u8				bgpri;			///< BG(プライオリティ)
	u8				visible;		///< 表示/非表示
	u8				palnum;			///< パレット番号(現在未使用だが、しばらく残し)
	u8				dispflag;		///< 表示フラグ
	u8				empty_num;		///< empty番号(これから座標を参照)
	u8				pad;			///< pad
}FMA_OBJ_DATA;

//=============================================================================
/**
 *								データ
 */
//=============================================================================
//---------------
// ■BG関連
//---------------
// BGPAL
static const FMA_BGPAL_DATA fma_bgpal_00_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_fmission_app_fmapp_bgu_nclr,	0,	2,},		// 上、BG
	{ PALTYPE_MAIN_BG,	NARC_fmission_app_fmapp_bgd_nclr,	0,	2,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};
static const FMA_BGPAL_DATA fma_bgpal_01_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_fmission_app_fmapp_bgu_nclr,	2,	2,},		// 上、BG
	{ PALTYPE_MAIN_BG,	NARC_fmission_app_fmapp_bgd_nclr,	2,	2,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};
// まとめ
static const FMA_BGPAL_DATA *fma_bgpal_tbl[] = {
	fma_bgpal_00_tbl,		// 詳細
	fma_bgpal_01_tbl,		// 評価
};

// BGCHR
static const FMA_BGCHR_DATA fma_bgchr_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_fmission_app_fmapp_bgu_lz_ncgr,	0,	0,},	// 上、BG
	{ GFL_BG_FRAME2_S,	NARC_fmission_app_fmapp_bgu_lz_ncgr,	0,	0,},	// 上、BG
	//
	{ GFL_BG_FRAME3_M,	NARC_fmission_app_fmapp_bgd_lz_ncgr,	0,	0,},	// 下、BG
	{ GFL_BG_FRAME2_M,	NARC_fmission_app_fmapp_bgd_lz_ncgr,	0,	0,},	// 下、BG
	//
	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const FMA_BGSCN_DATA fma_bgscn_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_fmission_app_fmapp_bgu03_lz_nscr,	0,	0,},	// 上、下地
	{ GFL_BG_FRAME2_S,	NARC_fmission_app_fmapp_bgu01_lz_nscr,	0,	0,},	// 上、下地
	{ GFL_BG_FRAME3_M,	NARC_fmission_app_fmapp_bgd03_lz_nscr,	0,	0,},	// 下、下地
	{ GFL_BG_FRAME2_M,	NARC_fmission_app_fmapp_bgd01_lz_nscr,	0,	0,},	// 下、下地
	//
	{ GFL_BG_FRAME3_S,	NARC_fmission_app_fmapp_bgu03_lz_nscr,	0,	1,},	// 上、下地
	{ GFL_BG_FRAME2_S,	NARC_fmission_app_fmapp_bgu02_lz_nscr,	0,	1,},	// 上、下地
	{ GFL_BG_FRAME3_M,	NARC_fmission_app_fmapp_bgd03_lz_nscr,	0,	1,},	// 下、下地
	{ GFL_BG_FRAME2_M,	NARC_fmission_app_fmapp_bgd02_lz_nscr,	0,	1,},	// 下、下地
	//
	{ 0xff, 0, 0, 0,}, // end code
};

//---------------
// ■CANVASの設定
//---------------
//-------------------
// CANVASの設定(詳細)
//-------------------
const FMA_CANVAS_DATA fma_canvas_00_tbl[] = {
  { GFL_BG_FRAME0_S,	1,	0,	24,	3,	FMA_BG_SUB_FONT_PAL,},		// タイトル
  //
  { GFL_BG_FRAME0_S,	5,	 5+2,	12,	2,	FMA_BG_SUB_FONT_PAL,},		// のこり時間(左)
  { GFL_BG_FRAME0_S,	5,	 8+2,	12,	2,	FMA_BG_SUB_FONT_PAL,},		// 参加人数(左)
  { GFL_BG_FRAME0_S,	3,	11+2,	20,	2,	FMA_BG_SUB_FONT_PAL,},		// クリア目標(左)
  { GFL_BG_FRAME0_S,	3,	14+2,	12,	2,	FMA_BG_SUB_FONT_PAL,},		// トータルスコア(左)
  //
  { GFL_BG_FRAME0_S,   23,	 8+2,	 6,	2,	FMA_BG_SUB_FONT_PAL,},		// 参加人数(右)
  { GFL_BG_FRAME0_S,   23,	11+2,	 6,	2,	FMA_BG_SUB_FONT_PAL,},		// クリア目標(右)
  { GFL_BG_FRAME0_S,   23,	14+2,	 6,	2,	FMA_BG_SUB_FONT_PAL,},		// トータルスコア(右)
  //
  //
  { GFL_BG_FRAME0_M,	3,	1,	8,	2,	FMA_BG_FONT_PAL,},			// マイスコア(左)
  { GFL_BG_FRAME0_M,	3,	4,	7,	2,	FMA_BG_FONT_PAL,},			// 順位(左)
  { GFL_BG_FRAME0_M,	3,	7, 10,	2,	FMA_BG_FONT_PAL,},			// トップスコア(左)
  { GFL_BG_FRAME0_M,   13,  7,	8,	2,	FMA_BG_FONT_PAL,},			// 名前(左)
  { GFL_BG_FRAME0_M,	3, 13, 28,	8,	FMA_BG_FONT_PAL,},			// 説明文(左)
  //
  { GFL_BG_FRAME0_M,   21,	1,	8,	2,	FMA_BG_FONT_PAL,},			// マイスコア(右)
  { GFL_BG_FRAME0_M,   10,	4, 19,	2,	FMA_BG_FONT_PAL,},			// 順位(右)
  { GFL_BG_FRAME0_M,   21,	7,  8,	2,	FMA_BG_FONT_PAL,},			// トップスコア(右)
  //
  //
  { GFL_BG_FRAME1_S,    2, 21, 28,	2,	FMA_BG_FONT_PAL,},			// スクロール
  { GFL_BG_FRAME1_S, 2+32, 21, 28,	2,	FMA_BG_FONT_PAL,},			// スクロール
  //
  { 0xff, 0, 0, 0, 0, 0,}, // end code
};

//-------------------
// CANVASの設定(評価)
//-------------------
const FMA_CANVAS_DATA fma_canvas_01_tbl[] = {
  { GFL_BG_FRAME0_S,	1,	0,	24,	3,	FMA_BG_SUB_FONT_PAL,},		// タイトル
  //
  { GFL_BG_FRAME0_S,	5,	5,	12,	2,	FMA_BG_SUB_FONT_PAL,},		// 参加人数(左)
  { GFL_BG_FRAME0_S,	3,	8,	20,	2,	FMA_BG_SUB_FONT_PAL,},		// クリア目標(左)
  { GFL_BG_FRAME0_S,	3,	11,	12,	2,	FMA_BG_SUB_FONT_PAL,},		// トータルスコア(左)
  { GFL_BG_FRAME0_S,	3,	14,	12,	2,	FMA_BG_SUB_FONT_PAL,},		// クリアタイム(左)
  //
  { GFL_BG_FRAME0_S,   23,	 5,	 6,	2,	FMA_BG_SUB_FONT_PAL,},		// 参加人数(右)
  { GFL_BG_FRAME0_S,   23,	 8,	 6,	2,	FMA_BG_SUB_FONT_PAL,},		// クリア目標(右)
  { GFL_BG_FRAME0_S,   23,	11,	 6,	2,	FMA_BG_SUB_FONT_PAL,},		// トータルスコア(右)
  //
  //
  { GFL_BG_FRAME0_M,	3,	1,	8,	2,	FMA_BG_FONT_PAL,},			// マイスコア(左)
  { GFL_BG_FRAME0_M,	3,	4,	7,	2,	FMA_BG_FONT_PAL,},			// 順位(左)
  { GFL_BG_FRAME0_M,	3,	7, 10,	2,	FMA_BG_FONT_PAL,},			// トップスコア(左)
  { GFL_BG_FRAME0_M,   13,  7,	8,	2,	FMA_BG_FONT_PAL,},			// 名前(左)
  { GFL_BG_FRAME0_M,	3, 12,	8,	2,	FMA_BG_FONT_PAL,},			// 報酬(左)
  { GFL_BG_FRAME0_M,	3, 15, 12,	2,	FMA_BG_FONT_PAL,},			// スコアボーナス(左)
  { GFL_BG_FRAME0_M,	3, 18, 10,	2,	FMA_BG_FONT_PAL,},			// ダブルスコア(左)
  //
  { GFL_BG_FRAME0_M,   21,	1,	8,	2,	FMA_BG_FONT_PAL,},			// マイスコア(右)
  { GFL_BG_FRAME0_M,   10,	4, 19,	2,	FMA_BG_FONT_PAL,},			// 順位(右)
  { GFL_BG_FRAME0_M,   21,	7,  8,	2,	FMA_BG_FONT_PAL,},			// トップスコア(右)
  { GFL_BG_FRAME0_M,   13, 12, 16,	2,	FMA_BG_FONT_PAL,},			// 報酬(右)
  { GFL_BG_FRAME0_M,   23, 15,	6,	2,	FMA_BG_FONT_PAL,},			// スコアボーナス(右)
  { GFL_BG_FRAME0_M,   13, 18, 16,	2,	FMA_BG_FONT_PAL,},			// ダブルボーナス(右)
  //
  { GFL_BG_FRAME1_S,   1, 19, 30,	4,	FMA_BG_SUB_FONT_PAL,},		// Window(上)
  //
  { 0xff, 0, 0, 0, 0, 0,}, // end code
};

//-------------------
// CANVASの設定(まとめ)
//-------------------
const FMA_CANVAS_DATA *fma_canvas_all_tbl[] = {
	fma_canvas_00_tbl,		// 詳細
	fma_canvas_01_tbl,		// 評価
};

//--------------------------------
// ■OBJの設定
// (共通のデータであることが前提)
//--------------------------------
//------------------
// OBJの情報
//------------------
// narc, resid, paloffset, palnum
static const s16 fma_obj_main_info_tbl[] = {
	NARC_fmission_app_fmapp_obj_nclr,    FMA_RESID_OBJ_PLT, 0, 3,	// NCL
	NARC_fmission_app_fmapp_obj_lz_ncgr, FMA_RESID_OBJ_CHR, 0, 0,	// NCG
	NARC_fmission_app_fmapp_obj_ncer,    FMA_RESID_OBJ_CEL, 0, 0,	// NCE
	NARC_fmission_app_fmapp_obj_nanr, 	-1, 0, 0,			// NAR?
};

// 下画面(RD)
// narc, resid, paloffset, palnum
static const s16 fma_obj_rd_info_tbl[] = {
	NARC_fmission_app_fmapp_obj_nclr,    FMA_RESID_RD_OBJ_PLT, 0x20*2, 3,	// NCL
	NARC_fmission_app_fmapp_obj_lz_ncgr, FMA_RESID_RD_OBJ_CHR, 0, 0,	// NCG
	NARC_fmission_app_fmapp_obj_ncer,    FMA_RESID_RD_OBJ_CEL, 0, 0,	// NCE
	NARC_fmission_app_fmapp_obj_nanr, 	-1, 0, 0,			// NAR?
};

// 情報
static const s16 fma_obj_main_itbl[] = {
	FMA_RESID_OBJ_PLT,
	FMA_RESID_OBJ_CHR,
	FMA_RESID_OBJ_CEL,
	//
	FMA_OBJ_MAXNUM,
	FMA_CLWKMODE_NORMAL,
};

// 情報(RD)
static const s16 fma_obj_rd_itbl[] = {
	FMA_RESID_RD_OBJ_PLT,
	FMA_RESID_RD_OBJ_CHR,
	FMA_RESID_RD_OBJ_CEL,
	//
	FMA_OBJ_RD_MAXNUM,
	FMA_CLWKMODE_RD,
};

// OBJのテーブルデータ(詳細)
static const FMA_OBJ_DATA fma_obj_info_tbl[] = {
	{ 6,	0x40,	1,	1,	0,	1,	0,	0,},		// 時間アイコン
	{ 7,	0x40,	1,	1,	0,	1,	1,	0,},		// 人数アイコン
	{ 5,	0x40,	1,	1,	0,	1,	2,	0,},		// 星
	{ 0,	0x40,	1,	1,	0,	0,	3,	0,},		// 時間
	{ 0,	0x40,	1,	1,	0,	0,	3,	0,},		// 時間
	{ 1,	0x40,	1,	1,	0,	0,	3,	0,},		// 時間(コロン)
	{ 0,	0x40,	1,	1,	0,	0,	3,	0,},		// 時間
	{ 0,	0x40,	1,	1,	0,	0,	3,	0,},		// 時間
	{ 0,	0x40,	1,	1,	0,	0,	4,	0,},		// スコア
	{ 0,	0x40,	1,	1,	0,	0,	4,	0,},		// スコア
	{ 0,	0x40,	1,	1,	0,	0,	4,	0,},		// スコア
	{ 0,	0x40,	1,	1,	0,	0,	4,	0,},		// スコア
	//
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

// OBJのテーブルデータ(結果)
static const FMA_OBJ_DATA fma_obj_result_tbl[] = {
	{ 7,	0x40,	1,	0,	0,	1,	5,	0,},		// 人数アイコン
	{ 5,	0x40,	1,	0,	0,	1,	6,	0,},		// 星
	{ 0,	0x40,	1,	0,	0,	0,	8,	0,},		// 時間
	{ 0,	0x40,	1,	0,	0,	0,	8,	0,},		// 時間
	{ 1,	0x40,	1,	0,	0,	0,	8,	0,},		// 時間(コロン)
	{ 0,	0x40,	1,	0,	0,	0,	8,	0,},		// 時間
	{ 0,	0x40,	1,	0,	0,	0,	8,	0,},		// 時間
	{ 0,	0x40,	1,	0,	0,	0,	7,	0,},		// スコア
	{ 0,	0x40,	1,	0,	0,	0,	7,	0,},		// スコア
	{ 0,	0x40,	1,	0,	0,	0,	7,	0,},		// スコア
	{ 0,	0x40,	1,	0,	0,	0,	7,	0,},		// スコア
	{ 8,	0x40,	1,	0,	0,	1,	9,	0,},		// レコード(トータルスコア)
	{ 8,	0x40,	1,	0,	0,	1, 10,	0,},		// レコード(クリアタイム)
	//
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

// OBJのテーブルデータ(RD)
static const FMA_OBJ_DATA fma_obj_rd_tbl[] = {
	{ 8,	0x40,	1,	0,	0,	1, 12,	0,},		// レコード(クリアタイム)
	//
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

// OBJのテーブルデータまとめ
static const FMA_OBJ_DATA *fma_obj_tbl[] = {
	fma_obj_info_tbl,			// 詳細
	fma_obj_result_tbl,			// 結果
	fma_obj_rd_tbl,				// 結果(RD)
};

//-----------
// 共通obj
//-----------
// 情報
static const s16 fma_obj_cmn_itbl[] = {
	FMA_RESID_CMN_OBJ_PLT,
	FMA_RESID_CMN_OBJ_CHR,
	FMA_RESID_CMN_OBJ_CEL,
	//
	FMA_OBJ_MAXNUM,
	FMA_CLWKMODE_CMN,
};

// OBJのテーブルデータ(cmn)
static const FMA_OBJ_DATA fma_obj_cmn_info_tbl[] = {
	{ 1,	0x30,	1,	1,	0,	1, 11,	0,},		// 戻るマーク
	//
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//---------------
// 星のシーケンス
//---------------
static const s16 fma_star_seq_tbl[] = {
	2,		// 1つ
	3,		// 2つ
	4,		// 3つ
	5,		// 4つ
};

//----------------
// タッチペン領域
//----------------
static const GFL_UI_TP_HITTBL fma_rect_01_tbl[] = {
  { 168, 192, 232, 255 }, 			// 戻る
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// 文字列
static const s16 fma_meswin_tbl[] = {
	msg_fmission_str_33,		// 達成
	msg_fmission_str_34,		// 失敗
	msg_fmission_str_35,		// 手に入れた
	msg_fmission_str_36,		// これ以上もてない
	msg_fmission_str_37,		// ダブルスコア
	msg_fmission_str_38,		// 個人の思いが..
	msg_fmission_str_39,		// みんなの思いが..
	msg_fmission_str_40,		// ホワイト
	msg_fmission_str_41,		// ブラック
};

//=============================================================================
/**
 *								宣言
 */
//=============================================================================
static void fma_init_bg( FMISSION_APP_WORK* wk, const s32 mode );
static void fma_init_bg2( FMISSION_APP_WORK* wk, const s32 type, const s32 pnum );
static void fma_init_string( FMISSION_APP_WORK* wk, const s32 mode );
//
static void fma_init_obj_clwk( FMISSION_APP_WORK* wk, const s32 drawtype, const FMA_OBJ_DATA *obj_adr, const s16 *info_adr );
static void fma_init_obj_res( FMISSION_APP_WORK* wk, const s32 drawtype, const s16 *info_adr );
static void fma_init_obj_res_cmn( FMISSION_APP_WORK* wk, const s32 drawtype );
static void fma_init_obj( FMISSION_APP_WORK* wk );
static void fma_exit_obj( FMISSION_APP_WORK* wk );
//
static void fma_set_meswin( FMISSION_APP_WORK* wk, const s32 mode, const s32 id );

//=============================================================================
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  データ初期化まとめ(bg+obj+canvas)
 *
 *	@param	FMISSION_APP_WORK* 	wk 
 *	@param	const s32			(0:詳細、1:評価)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FmaCmn_data_init( FMISSION_APP_WORK* wk, const s32 mode )
{
	// BG
	fma_init_bg(  wk, mode );
	fma_init_bg2( wk, GFL_BG_FRAME1_M, FMA_BG_CMN_PAL );
	// 文字列
	fma_init_string( wk, mode );
	// obj
	fma_init_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  データ破棄(bg+obj+canvas)
 *
 *	@param	FMISSION_APP_WORK* 	wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FmaCmn_data_exit( FMISSION_APP_WORK* wk )
{
	s32		i;
	
	// canvas
	for(i=0;i<FMA_CANVAS_MAXNUM;i++){
		if(wk->PrintUtil[i].win){
			GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
			wk->PrintUtil[i].win = 0;
		}
	}
	// PrintStream後処理
	if(wk->MsgBuffer){
		GFL_STR_DeleteBuffer( wk->MsgBuffer );
		wk->MsgBuffer = 0;
	}
	if(wk->PrintStream){
		PRINTSYS_PrintStreamDelete( wk->PrintStream );
		wk->PrintStream = 0;
	}
	
	// obj
	fma_exit_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG初期化
 *
 *	@param	FMISSION_APP_WORK* wk 
 *	@param	const s32			(0:詳細、1:評価)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init_bg( FMISSION_APP_WORK* wk, const s32 mode )
{
	s32		i;
	s32		tmp;
	s32		off;
	const FMA_BGPAL_DATA	*padr = fma_bgpal_tbl[mode];	// まとめ
	const FMA_BGCHR_DATA	*cadr = &fma_bgchr_tbl[0];
	const FMA_BGSCN_DATA	*sadr = &fma_bgscn_tbl[0];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_FMISSION_APP, wk->heapID );
	
	// パレット転送
	for(i=0;i<FMA_BGNUM_MAX;i++){
		// 終了条件
		if(padr->type == 0xff){
			break;
		}
		tmp = padr->size;
		tmp *= 0x20;		// 転送サイズ
		off = padr->offset;
		off *= 0x20;
		GFL_ARCHDL_UTIL_TransVramPaletteEx( handle, padr->arcid, padr->type, off, 0, tmp, wk->heapID );
		padr++;
	}
	//
	// キャラ転送(圧縮)
	for(i=0;i<FMA_BGNUM_MAX;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, TRUE, wk->heapID );
		cadr++;
	}
	
	// スクリーン転送(圧縮)
	for(i=0;i<FMA_BGNUM_MAX;i++){
		// 終了条件
		if(sadr->frmnumber == 0xff){
			break;
		}
		// 同じモードの時のみ転送
		if(sadr->mode == mode){
			GFL_ARCHDL_UTIL_TransVramScreen( handle, sadr->arcid, sadr->frmnumber, sadr->offset, 0, TRUE, wk->heapID );
		}
		sadr++;
	}
	
	// 終了
	GFL_ARC_CloseDataHandle( handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化(cmn)
 *
 *	@param  FMISSION_APP_WORKK* wk
 *	@param	const s32			(BG面)
 *	@param	const s32			(パレット番号)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init_bg2( FMISSION_APP_WORK* wk, const s32 type, const s32 pnum )
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
	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, id, PALTYPE_MAIN_BG, pnum*0x20, 0x20, wk->heapID );
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
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(現在、Canvasの作成)
 *
 *	@param	FMISSION_APP_WORK* 	wk
 *	@param	const s32			(0:詳細、1:評価)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init_string( FMISSION_APP_WORK* wk, const s32 mode )
{
	s32		i;
	const FMA_CANVAS_DATA	*adr = fma_canvas_all_tbl[mode];
	
	for(i=0;i<FMA_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット
		wk->PrintUtil[i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		// スクリーンは一度でよいはず
		GFL_BMPWIN_MakeScreen( wk->PrintUtil[i].win );
		GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[i].win) );
		adr++;
	}
	//
	//-----------------
	// fontパレット転送
	//-----------------
	{
		ARCHANDLE	*p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		// Main
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, FMA_BG_FONT_PAL*0x20, 0x20, wk->heapID );
		// Sub
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_SUB_BG,  FMA_BG_SUB_FONT_PAL*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x)
 *	@param	const s32			表示座標(y)
 *	@param	const STRBUF* 		表示文字列
 *	@param	const u32			表示位置(左、右、中央など)
 *	@param	const PRINTSYS_LSB	カラー
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FmaCmn_disp_string( FMISSION_APP_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col )
{
	// 表示
	PRINTTOOL_PrintColor(
				&wk->PrintUtil[mode],			// プリントユーティル ( BMPWIN )
				wk->print_que,					// プリントキュー
				x, y,							// 表示基準座標
				strbuf,							// 文字列
				wk->font,						// フォント
				col,							// カラー
				pmode );						// 位置
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32				キャンバスの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FmaCmn_clear_canvas( FMISSION_APP_WORK* wk, const s32 mode )
{
	// clear
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_CLEAR_COLOR );
	wk->PrintUtil[mode].transReq = TRUE;
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  emptyの取得
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
s16 FmaCmn_get_empty( FMISSION_APP_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= FMA_EMPTY_MAXNUM)){
		OS_Printf("empty error :%d\n",num);
		return(0);
	}
	
	adr = wk->EmptyBuffer;
	return( adr[num] );
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const FMA_OBJ_DATA* アドレス
 *	@param	const s16*				アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init_obj_clwk( FMISSION_APP_WORK* wk, const s32 drawtype, const FMA_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16						tmp;
	s16						pos[2];
	s16						buf[6];
	s32 					i;
	BOOL					bflg;
	GFL_CLWK_DATA 			cldata;
	CLWK_FLIP_TYPE			flip;
    GFL_CLUNIT*				p_clunit 	= FMISSION_APP_GRA_GetClunit( wk->graphic );
	// アドレスセット
	const FMA_OBJ_DATA *adr 		= obj_adr;
	const s16 				*iadr 	= info_adr;
	GFL_CLWK				**clwk;			// アドレス注意
	
	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
	// チェック
	buf[0] = iadr[0];		// plt
	buf[1] = iadr[1];		// chr
	buf[2] = iadr[2];		// cel
	buf[3] = iadr[3];		// max
	buf[4] = iadr[4];		// mode(0:通常、1:cmn)
	//
	// アドレスセット
	if(buf[4] == FMA_CLWKMODE_NORMAL){			// 通常
		clwk = &wk->wk_obj.clwk[0];
	}
	else if(buf[4] == FMA_CLWKMODE_CMN){		// cmn
		clwk = &wk->wk_obj.cmn_clwk[0];
	}
	else if(buf[4] == FMA_CLWKMODE_RD){		// rd
		clwk = &wk->wk_obj.rd_clwk[0];
	}
	
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		pos[0] = FmaCmn_get_empty(wk, tmp*2);
		pos[1] = FmaCmn_get_empty(wk, tmp*2+1);
		cldata.pos_x 	= pos[0];
		cldata.pos_y 	= pos[1];
		cldata.anmseq 	= adr->anmseq;
		cldata.softpri	= adr->softpri;
		cldata.bgpri	= adr->bgpri;
		//
		clwk[i] =   GFL_CLACT_WK_Create( p_clunit, 
											wk->wk_obj.res[ buf[1] ],			// chr
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
		// 次のデータへ
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(res)
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init_obj_res( FMISSION_APP_WORK* wk, const s32 drawtype, const s16 *info_adr )
{
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_FMISSION_APP, wk->heapID );
    GFL_CLUNIT*	p_clunit = FMISSION_APP_GRA_GetClunit( wk->graphic );
	const s16 *adr		 = info_adr;		// アドレスセット
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ adr[0+1] ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, adr[0], drawtype, adr[2], 0, adr[3], wk->heapID );
	wk->wk_obj.res[ adr[4+1] ] = GFL_CLGRP_CGR_Register(      p_handle, adr[4], TRUE, drawtype, wk->heapID );
	wk->wk_obj.res[ adr[8+1] ] = GFL_CLGRP_CELLANIM_Register( p_handle, adr[8], adr[12], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(cmn res)
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			drawtype
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init_obj_res_cmn( FMISSION_APP_WORK* wk, const s32 drawtype )
{
	// cmnのidを取得
	const u32 	arcid    = APP_COMMON_GetArcId();
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( arcid, wk->heapID );
    GFL_CLUNIT*	p_clunit = FMISSION_APP_GRA_GetClunit( wk->graphic );
	u32			idx[4];
	//
	idx[0] = APP_COMMON_GetBarIconPltArcIdx();		// pal
	idx[1] = APP_COMMON_GetBarIconCharArcIdx();		// ncg
	idx[2] = APP_COMMON_GetBarIconCellArcIdx(  APP_COMMON_MAPPING_128K );	// cell
	idx[3] = APP_COMMON_GetBarIconAnimeArcIdx( APP_COMMON_MAPPING_128K );	// anim
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ FMA_RESID_CMN_OBJ_PLT ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, idx[0], drawtype, 0x20*FMA_OBJ_BASE_PAL, 0, 4, wk->heapID );
	wk->wk_obj.res[ FMA_RESID_CMN_OBJ_CHR ] = GFL_CLGRP_CGR_Register(      p_handle, idx[1], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ FMA_RESID_CMN_OBJ_CEL ] = GFL_CLGRP_CELLANIM_Register( p_handle, idx[2], idx[3], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化まとめ
 *
 *	@param	FMISSION_APP_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init_obj( FMISSION_APP_WORK* wk )
{
	GFL_CLUNIT*	p_clunit = FMISSION_APP_GRA_GetClunit( wk->graphic );
	const FMA_OBJ_DATA	*adr = fma_obj_tbl[wk->Mode];
	//
	// リソース読み込み(通常)
	fma_init_obj_res( wk, CLSYS_DRAW_SUB, &fma_obj_main_info_tbl[0] );
	// リソース読み込み(cmn)
	fma_init_obj_res_cmn( wk, CLSYS_DRAW_MAIN );
	//
	// CLWK登録(通常)
	fma_init_obj_clwk( wk, CLSYS_DRAW_SUB, adr, &fma_obj_main_itbl[0] );
	// CLWK登録(cmn)
	fma_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &fma_obj_cmn_info_tbl[0], &fma_obj_cmn_itbl[0] );
	//
	// 結果表示の場合
	if(wk->Mode == FMISSION_APP_MODE_RESULT){
		// リソース読み込み(RD)
		fma_init_obj_res( wk, CLSYS_DRAW_MAIN, &fma_obj_rd_info_tbl[0] );
		// CLWK登録(RD)
		adr = fma_obj_tbl[2];
		fma_init_obj_clwk( wk, CLSYS_DRAW_MAIN, adr, &fma_obj_rd_itbl[0] );
	}
	// 通信アイコンセット(下画面)
	GFL_NET_ReloadIconTopOrBottom( FALSE, wk->heapID );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	FMISSION_APP_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_exit_obj( FMISSION_APP_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<FMA_OBJ_MAXNUM;i++){
		if(wk->wk_obj.clwk[i]){
			GFL_CLACT_WK_Remove( wk->wk_obj.clwk[i] );
			wk->wk_obj.clwk[i] = 0;
		}
	}
	for(i=0;i<FMA_CMN_OBJ_MAXNUM;i++){
		if(wk->wk_obj.cmn_clwk[i]){
			GFL_CLACT_WK_Remove( wk->wk_obj.cmn_clwk[i] );
			wk->wk_obj.cmn_clwk[i] = 0;
		}
	}
	for(i=0;i<FMA_OBJ_RD_MAXNUM;i++){
		if(wk->wk_obj.rd_clwk[i]){
			GFL_CLACT_WK_Remove( wk->wk_obj.rd_clwk[i] );
			wk->wk_obj.rd_clwk[i] = 0;
		}
	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[FMA_RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[FMA_RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[FMA_RESID_OBJ_CEL] );
	//
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[FMA_RESID_CMN_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[FMA_RESID_CMN_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[FMA_RESID_CMN_OBJ_CEL] );
	//
	// 結果表示の場合
	if(wk->Mode == FMISSION_APP_MODE_RESULT){
		GFL_CLGRP_PLTT_Release( wk->wk_obj.res[FMA_RESID_RD_OBJ_PLT] );
		GFL_CLGRP_CGR_Release(  wk->wk_obj.res[FMA_RESID_RD_OBJ_CHR] );
		GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[FMA_RESID_RD_OBJ_CEL] );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FmaCmn_chg_objseq( FMISSION_APP_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	if(wmode == FMA_CLWKMODE_NORMAL){			// 通常
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
	}
	else if(wmode == FMA_CLWKMODE_CMN){			// 共通
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.cmn_clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.cmn_clwk[wnum], TRUE );
	}
	else if(wmode == FMA_CLWKMODE_RD){			// RD
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.rd_clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.rd_clwk[wnum], TRUE );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FmaCmn_set_obj_visible( FMISSION_APP_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// 表示/非表示
	if(wmode == FMA_CLWKMODE_NORMAL){		// 通常
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
	}
	else if(wmode == FMA_CLWKMODE_CMN){		// 共通
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.cmn_clwk[wnum], bflg );
	}
	else if(wmode == FMA_CLWKMODE_RD){		// RD
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.rd_clwk[wnum], bflg );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのアニメの終了待ち
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *
 *	@retval	const BOOL			(TRUE:再生中、FALSE:停止、終了)
 */
//-----------------------------------------------------------------------------
BOOL FmaCmn_is_anime_end( FMISSION_APP_WORK *wk, const s32 wmode, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// 終了判定
	if(wmode == FMA_CLWKMODE_NORMAL){		// 通常
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	else if(wmode == FMA_CLWKMODE_CMN){		// 共通
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.cmn_clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	else if(wmode == FMA_CLWKMODE_RD){		// RD
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.rd_clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのパレットの切り替え
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *	@param	const s32			ワーク
 *	@param	const s32			カラー
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FmaCmn_chg_pal( FMISSION_APP_WORK *wk, const s32 wmode, const s32 wnum, const s32 cnum )
{
	// 切り替え
	if(wmode == FMA_CLWKMODE_NORMAL){			// 通常
		GFL_CLACT_WK_SetPlttOffs( wk->wk_obj.clwk[wnum], cnum, CLWK_PLTTOFFS_MODE_PLTT_TOP );
	}
	else if(wmode == FMA_CLWKMODE_CMN){			// 共通
		GFL_CLACT_WK_SetPlttOffs( wk->wk_obj.cmn_clwk[wnum], cnum, CLWK_PLTTOFFS_MODE_PLTT_TOP );
	}
	else if(wmode == FMA_CLWKMODE_RD){			// RD
		GFL_CLACT_WK_SetPlttOffs( wk->wk_obj.rd_clwk[wnum], cnum, CLWK_PLTTOFFS_MODE_PLTT_TOP );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  数字表示
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			開始objワーク番号
 *	@param	const s32			値
 *	@param	const s32			桁数(max4)
 *	@param	const s32			モード(0:値のみ,1:0埋め)
 *	@param	const s32,s32		座標
 */
//-----------------------------------------------------------------------------
void FmaCmn_disp_number( FMISSION_APP_WORK *wk, const s32 wmode, const s32 src_wnum, const s32 num, const s32 snum, const s32 smode,
												const s32 x, const s32 y)
{
	s32				i;
	s32				buf[4];
	GFL_CLACTPOS	pos;
	BOOL			bflg;
	s32				tmp=num;		// 値セット
	
	// 初期化
	for(i=0;i<4;i++){
		buf[i] = -1;	// 初期化
	}
	pos.x = x;
	pos.y = y;
	//
	for(i=0;i<snum;i++){
		// 各桁をセット
		buf[i] = tmp%10;
		tmp /= 10;
		if(tmp == 0){
			break;
		}
	}
	//
//	OS_Printf("%d,%d,%d,%d\n",buf[0],buf[1],buf[2],buf[3]);
	// ここまででbuf[]に逆順で値が入っているはず
	for(i=0;i<4;i++){
		// 値のみ表示
		bflg = TRUE;
		if(smode == 0){
			if(buf[i] == -1){
				bflg = FALSE;
				buf[i] = 0;	// 念のため
			}
		}
		// 表示
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[src_wnum+3-i], bflg );
		// フレームを指定(FX32_SHIFT)したがうまくいかないので、
		// アニメのインデックスをセットするようにした
		GFL_CLACT_WK_SetAnmIndex( wk->wk_obj.clwk[src_wnum+3-i], buf[i] );
		// x方向に加算
		pos.x = x + 8*(3-i);
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[src_wnum+3-i], &pos, CLSYS_DEFREND_SUB );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  時間表示(決めうち)
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			開始objワーク番号
 *	@param	const s32			分
 *	@param	const s32			秒
 */
//-----------------------------------------------------------------------------
void FmaCmn_disp_time( FMISSION_APP_WORK *wk, const s32 wmode, const s32 src_wnum, const s32 src0, const s32 src1 )
{
	s32				i;
	s32				buf[4];
	GFL_CLACTPOS	pos;
	s32				srcpos;
	BOOL			bflg;
	s32				tmp;			// 値セット
	
	// 初期化
	for(i=0;i<4;i++){
		buf[i] = 0;
	}
	// 先頭の座標の取得
	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[src_wnum], &pos, CLSYS_DEFREND_SUB );
	srcpos = pos.x;
	//
	tmp = src0;
	for(i=0;i<2;i++){
		// 各桁をセット
		buf[1-i] = tmp%10;
		tmp /= 10;
		if(tmp == 0){
			break;
		}
	}
	tmp = src1;
	for(i=0;i<2;i++){
		// 各桁をセット
		buf[2+1-i] = tmp%10;
		tmp /= 10;
		if(tmp == 0){
			break;
		}
	}
	//
//	OS_Printf("%d,%d,%d,%d\n",buf[0],buf[1],buf[2],buf[3]);
	//
	// ここまででbuf[]に値(01:23)が入っているはず
	// 分
	for(i=0;i<2;i++){
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[src_wnum+i], TRUE );
		// フレームを指定
		GFL_CLACT_WK_SetAnmIndex( wk->wk_obj.clwk[src_wnum+i], buf[i] );
		// x方向に加算
		pos.x = srcpos + 8*i;
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[src_wnum+i], &pos, CLSYS_DEFREND_SUB );
	}
	// コロン
	GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[src_wnum+2], TRUE );
	pos.x = srcpos + 8*2;
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[src_wnum+2], &pos, CLSYS_DEFREND_SUB );
	//
	// 秒
	for(i=0;i<2;i++){
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[src_wnum+3+i], TRUE );
		// フレームを指定
		GFL_CLACT_WK_SetAnmIndex( wk->wk_obj.clwk[src_wnum+3+i], buf[2+i] );
		// x方向に加算
		pos.x = srcpos + 8*(3+i);
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[src_wnum+3+i], &pos, CLSYS_DEFREND_SUB );
	}
}

//--------------------------------------------------------------
/**
 *	星表示
 *
 * @param   FMISSION_APP_WORK		*wk
 */
//--------------------------------------------------------------
void FmaCmn_disp_star( FMISSION_APP_WORK *wk, const s32 cnum )
{
	s32			lv;
	
	// レベルを取得
	lv = wk->Param->fmwk->beacon.m_level;
	// 表示
	GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[cnum], TRUE );
	// 星のシーケンス変更
	FmaCmn_chg_objseq( wk, FMA_CLWKMODE_NORMAL, cnum, fma_star_seq_tbl[lv] );
}

//--------------------------------------------------------------
/**
 *	タッチ判定
 *
 * @param   FMISSION_APP_WORK		*wk
 *
 * @retval  s32						(タッチ領域番号)
 */
//--------------------------------------------------------------
s32 FmaCmn_chk_tp( FMISSION_APP_WORK *wk )
{
	s32		id;
	//
	// タッチあり
	id = GFL_UI_TP_HitTrg( &fma_rect_01_tbl[0] );
	return( id );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ(セット)
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32			モード
 *	@param	const s32			(FMA_MSGWIN_**)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_set_meswin( FMISSION_APP_WORK* wk, const s32 mode, const s32 id )
{
	s32			buf[2];
	STRBUF		*str_tmp;
	STRBUF		*strbuf2;
	MYSTATUS	*mystatus;
	MYITEM		*myitem;
	
	// PrintStream終了時にはdeleteしないとtaskが残ってしまうようだ
	if( wk->PrintStream ){
		PRINTSYS_PrintStreamDelete( wk->PrintStream );
		wk->PrintStream = 0;
	}
	// Msgの再構築
	if( wk->MsgBuffer ){
		GFL_STR_DeleteBuffer( wk->MsgBuffer );
		wk->MsgBuffer = 0;
	}
	//
	// 手に入れた
	if(id == FMA_MSGWIN_02){
		// プレイヤ名
		mystatus = GAMEDATA_GetMyStatus( wk->Param->gdata );
		strbuf2 = GFL_STR_CreateBuffer( 64, wk->heapID );
		MyStatus_CopyNameString( mystatus, strbuf2 );
		WORDSET_RegisterWord( wk->Wordset, 0, strbuf2, 0, TRUE, PM_LANG );
		// アイテム名
		WORDSET_RegisterItemName( wk->Wordset, 1, wk->MissionResult.item );
		// (所持可能な)アイテム数を求める
		buf[0] = FmaCmn_chk_itemget(wk, wk->MissionResult.item);
		buf[1] = wk->MissionResult.item_num+wk->MissionResult.bonus_num;
		if(buf[1] > buf[0]){
			buf[1] = buf[0];
		}
		WORDSET_RegisterNumber( wk->Wordset, 2, buf[1], 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		// セット
		wk->MsgBuffer = GFL_STR_CreateBuffer( 256, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fma_meswin_tbl[id]);
		WORDSET_ExpandStr( wk->Wordset, wk->MsgBuffer, str_tmp );
		GFL_STR_DeleteBuffer(strbuf2);
		GFL_STR_DeleteBuffer(str_tmp);
		//
		// 後処理としてアイテムを実際に加算
		myitem = GAMEDATA_GetMyItem( wk->Param->gdata );
		MYITEM_AddItem( myitem, wk->MissionResult.item, buf[1], wk->heapID );
	}
	// 手に入れられない(アイテム名は持てない)
	else if(id == FMA_MSGWIN_03){
		// アイテム名
		WORDSET_RegisterItemName( wk->Wordset, 0, wk->MissionResult.item );
		// セット
		wk->MsgBuffer = GFL_STR_CreateBuffer( 256, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fma_meswin_tbl[id]);
		WORDSET_ExpandStr( wk->Wordset, wk->MsgBuffer, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	// 手に入れた2(sp)
	else if(id == FMA_MSGWIN_02A){
		// プレイヤ名
		mystatus = GAMEDATA_GetMyStatus( wk->Param->gdata );
		strbuf2 = GFL_STR_CreateBuffer( 64, wk->heapID );
		MyStatus_CopyNameString( mystatus, strbuf2 );
		WORDSET_RegisterWord( wk->Wordset, 0, strbuf2, 0, TRUE, PM_LANG );
		// アイテム名
		WORDSET_RegisterItemName( wk->Wordset, 1, wk->MissionResult.sp_item );
		// (所持可能な)アイテム数を求める
		buf[0] = FmaCmn_chk_itemget(wk, wk->MissionResult.sp_item);
		buf[1] = wk->MissionResult.sp_item_num;
		if(buf[1] > buf[0]){
			buf[1] = buf[0];
		}
		WORDSET_RegisterNumber( wk->Wordset, 2, buf[1], 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		// セット
		wk->MsgBuffer = GFL_STR_CreateBuffer( 256, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fma_meswin_tbl[2]);
		WORDSET_ExpandStr( wk->Wordset, wk->MsgBuffer, str_tmp );
		GFL_STR_DeleteBuffer(strbuf2);
		GFL_STR_DeleteBuffer(str_tmp);
		//
		// 後処理としてアイテムを実際に加算
		myitem = GAMEDATA_GetMyItem( wk->Param->gdata );
		MYITEM_AddItem( myitem, wk->MissionResult.sp_item, buf[1], wk->heapID );
	}
	// 手に入れられない2
	else if(id == FMA_MSGWIN_03A){
		// アイテム名
		WORDSET_RegisterItemName( wk->Wordset, 0, wk->MissionResult.sp_item );
		// セット
		wk->MsgBuffer = GFL_STR_CreateBuffer( 256, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fma_meswin_tbl[3]);
		WORDSET_ExpandStr( wk->Wordset, wk->MsgBuffer, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	// 個人
	else if(id == FMA_MSGWIN_05){
		// プレイヤ名
		mystatus = GAMEDATA_GetMyStatus( wk->Param->gdata );
		strbuf2 = GFL_STR_CreateBuffer( 64, wk->heapID );
		MyStatus_CopyNameString( mystatus, strbuf2 );
		WORDSET_RegisterWord( wk->Wordset, 0, strbuf2, 0, TRUE, PM_LANG );
		GFL_STR_DeleteBuffer(strbuf2);
		// バージョン
		strbuf2 = GFL_MSG_CreateString( wk->MsgData[0], fma_meswin_tbl[7+wk->MissionResult.hilink_version]);
		WORDSET_RegisterWord( wk->Wordset, 1, strbuf2, 0, TRUE, PM_LANG );
		// バージョンレベル
		buf[0] = wk->MissionResult.hilink_level_up;
		WORDSET_RegisterNumber( wk->Wordset, 2, buf[0], 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		// セット
		wk->MsgBuffer = GFL_STR_CreateBuffer( 256, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fma_meswin_tbl[id]);
		WORDSET_ExpandStr( wk->Wordset, wk->MsgBuffer, str_tmp );
		GFL_STR_DeleteBuffer(strbuf2);
		GFL_STR_DeleteBuffer(str_tmp);
	}
	// みんな
	else if(id == FMA_MSGWIN_06){
		// バージョン
		strbuf2 = GFL_MSG_CreateString( wk->MsgData[0], fma_meswin_tbl[7+wk->MissionResult.hilink_version]);
		WORDSET_RegisterWord( wk->Wordset, 0, strbuf2, 0, TRUE, PM_LANG );
		// バージョンレベル
		buf[0] = wk->MissionResult.hilink_level_up;
		WORDSET_RegisterNumber( wk->Wordset, 1, buf[0], 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		// セット
		wk->MsgBuffer = GFL_STR_CreateBuffer( 256, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fma_meswin_tbl[id]);
		WORDSET_ExpandStr( wk->Wordset, wk->MsgBuffer, str_tmp );
		GFL_STR_DeleteBuffer(strbuf2);
		GFL_STR_DeleteBuffer(str_tmp);
	}
	// その他
	else{
		// メッセージバッファ
		wk->MsgBuffer = GFL_MSG_CreateString( wk->MsgData[0], fma_meswin_tbl[id] );
	}
	//
	// stream設定 => ここのwk->MsgBufferは終わるまで削除してはいけない
	wk->PrintStream = PRINTSYS_PrintStream(
				wk->PrintUtil[mode].win, 0, 0, wk->MsgBuffer, wk->font,
				MSGSPEED_GetWait(), wk->MsgTcblSys, 4, wk->heapID, MES_SYSWIN_CLEAR_COLOR );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ(処理)
 *
 *	@param	FMISSION_APP_WORK* wk
 *
 *	@retval	BOOL				(TRUE:再生中、FALSE:終了)
 */
//-----------------------------------------------------------------------------
BOOL FmaCmn_main_meswin( FMISSION_APP_WORK* wk )
{
	BOOL	bflg;
	
	// 念のため
	if(wk->PrintStream == 0){
		return( FALSE );
	}
	//
	// 文字送りカーソル処理
	APP_KEYCURSOR_Main( wk->MsgCursorWork, wk->PrintStream, wk->PrintUtil[FMAR_CANVAS_DOWN_WIN].win );
	// streamの入力処理をまとめて処理してくれる
	// 内部でkey/touchの判定もしてくれる
	bflg = APP_PRINTSYS_COMMON_PrintStreamFunc( &wk->PrintSysWork , wk->PrintStream );
	if( bflg == TRUE ){
		// 終了
		return( FALSE );
	}
	return( TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  winにメッセージ表示
 *
 *	@param	FMISSION_APP_WORK* wk
 *	@param	const s32				(canvasの番号)
 *	@param	const s32				番号
 *	@param	const s32				(0:一発表示、1:stream表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void FmaCmn_disp_winmes( FMISSION_APP_WORK* wk, const s32 mode, const s32 id, const s32 smode )
{
	s32			flg=0;
	STRBUF		*strbuf;
	STRBUF		*strbuf2;
	STRBUF		*expbuf;
	
	//-----------------
	// ウインドウ表示
	//-----------------
	// 会話ウィンドウセット(キャラデータを転送)
	BmpWinFrame_GraphicSet( GFL_BG_FRAME1_S, 1, FMA_BG_SUB_WIN_PAL, MENU_TYPE_SYSTEM, wk->heapID);
	// 会話ウインドウ表示
	BmpWinFrame_Write( wk->PrintUtil[mode].win, WINDOW_TRANS_ON, 1, FMA_BG_SUB_WIN_PAL );
	// 文字スクリーン作成
	GFL_BMPWIN_MakeScreen( wk->PrintUtil[mode].win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[mode].win) );
	// クリア
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_SYSWIN_CLEAR_COLOR );
	GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[mode].win );
	//
	if(smode == 0){			// 一発だし
		// 中央そろい
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fma_meswin_tbl[id] );
		FmaCmn_disp_string( wk, mode, 120, 0, strbuf, PRINTTOOL_MODE_CENTER, PRINTSYS_LSB_Make(1,2,0) );
		GFL_STR_DeleteBuffer( strbuf );
	}
	else{					// stream表示
		fma_set_meswin( wk, mode, id );
	}
}

//--------------------------------------------------------------
/**
 * アイテム取得のチェック
 *
 * @param   FMISSION_APP_WORK		*wk
 * @param   const s32				id
 * @param   const s32				加算しようとしている数
 *
 * @ret     s32						持てる数
 */
//--------------------------------------------------------------
s32 FmaCmn_chk_itemget( FMISSION_APP_WORK *wk, const s32 itemid )
{
	s32		num;
	u16		nownum;
	u32		maxnum;
	
	// 念のため
	if(itemid == 0){
		return( 0 );
	}
	// 所持最大数
	maxnum = MYITEM_GetItemMax( itemid );
	// 現在の所持数
	nownum = MYITEM_GetItemNum( GAMEDATA_GetMyItem(wk->Param->gdata), itemid, wk->heapID );
	// 追加で持てる数
	num = maxnum-nownum;
	//
	return( num );
}

//--------------------------------------------------------------
/**
 * 矢印表示
 *
 * @param   FMISSION_APP_WORK		*wk
 *
 * @ret		
 */
//--------------------------------------------------------------
void FmaCmn_disp_arrow( FMISSION_APP_WORK *wk, const BOOL bflg )
{
	if(bflg == TRUE){
		APP_KEYCURSOR_Write( wk->MsgCursorWork, GFL_BMPWIN_GetBmp(wk->PrintUtil[FMAR_CANVAS_DOWN_WIN].win), 15 );
		GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[FMAR_CANVAS_DOWN_WIN].win );
	}else{
		APP_KEYCURSOR_Clear( wk->MsgCursorWork, GFL_BMPWIN_GetBmp(wk->PrintUtil[FMAR_CANVAS_DOWN_WIN].win), 15 );
		GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[FMAR_CANVAS_DOWN_WIN].win );
	}
}


