//=============================================================================
/**
 *
 *	@file		t_download_sub.c
 *	@brief  トーナメントダウンロード画面(sub)
 *	@author
 *	@date
 *
 *  共通部分をまとめた([TdCmn_*]と命名)
 *
 */
//=============================================================================
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"

#include "print/gf_font.h"
#include "font/font.naix"
#include "print/printsys.h"
//
#include "ui/print_tool.h"
#include "system/bmp_winframe.h"
#include "ui/frame_list.h"
//
#include "sound/pm_sndsys.h"
#include "t_download_snd_def.h"
//
#include "arc_def.h"
//
#include "app/app_menu_common.h"
#include "app/app_taskmenu.h"
#include "app/app_printsys_common.h"
#include "app/app_keycursor.h"
#include "gamesystem/msgspeed.h"
//
#include "system/bmp_oam.h"
#include "system/scroll_bar.h"
//
#include "t_download_graphic.h"
#include "t_download_local.h"
#include "t_download_save.h"

#include "t_download_sub.h"

#include "t_download_gra.naix"

#include "message.naix"
#include "msg/msg_wbt_download.h"

//=============================================================================
//=============================================================================
/**
 *								定義
 */
//=============================================================================
// パレット
// (sub)
// 上：BG (0-3:BG、8:FONT)
// 上：OBJ(14:通信)
//
// (main)
// 下：BG (0-4:BG、7,8,9,11,12:FONT,WIN)
// 下：OBJ(0-5:パネル、11-14:cmn)
enum{
	// (main)
	PLTID_CAPTION_FONT			= 7,	// caption
	PLTID_SYSWIN_BASE			= 8,	// font(syswin)
	PLTID_SYSWIN_FONT			= 9,	// font(syswin)
	PLTID_YESNO_WIN_BASE		= 11,	// win(yes/no window)(注：2本使用する(色変え含む:11,12))
	//
	// (sub)
	PLTID_UP_FONT				= 8,	// font(up)
	//
	// (obj)
	PLTID_OBJ_CMN				= 11,	// cmnのpalのoffset
	//
	// 何番目か
	MES_CLEAR_COLOR				= 0,	// (通常)クリアカラー
	MES_SYSWIN_CLEAR_COLOR		= 15,	// (システムウインドウの)クリアカラー
};

// いろいろ
enum{
	EMPTY_CURPOS				= 18,	// カーソル上の位置(リスト用)
	EMPTY_CURPOS2				= 27,	// カーソル上の位置(3枚パネル用)
	//
	CANVAS_INFO_OFFSET			= 9,	// 詳細用のcanvasのoffset
	CANVAS_INFO_NUM				= 5,	// 詳細用のcanvasの数
	//
	CANVAS_JOIN_OFFSET			= 14,	// joinのcanvasのoffset
	CANVAS_JOIN_NUM				= 3,	// joinのcanvasの数
	//
	TD_ACTIVE_RATE				= 0,	// アクティブの割合(戻し)
	TD_PASSIVE_RATE 			= -8,	// パッシブの割合
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

//-----------------
// シーケンス(cmn)
//-----------------
enum{
	CMN_LEFT_SEQ 		= 4,			// 左
	CMN_RIGHT_SEQ 		= 5,			// 右
	CMN_CROSS_SEQ 		= 0,			// ×
	CMN_RETURN_SEQ 		= 1,			// 戻る
	CMN_UP_SEQ 			= 3,			// 上
	CMN_DOWN_SEQ 		= 2,			// 下
	//
	CMN_LEFT_ON_SEQ 	= 12,			// 左オン
	CMN_RIGHT_ON_SEQ 	= 13,			// 右オン
	CMN_UP_ON_SEQ 		= 11,			// 上オン
	CMN_DOWN_ON_SEQ 	= 10,			// 下オン
	CMN_CROSS_ON_SEQ	= 8,			// ×オン
	CMN_RETURN_ON_SEQ	= 9,			// 戻るオン
	//
	CMN_LEFT_OFF_SEQ 	= 18,			// 左オフ
	CMN_RIGHT_OFF_SEQ 	= 19,			// 右オフ
	CMN_UP_OFF_SEQ 		= 17,			// 上オフ
	CMN_DOWN_OFF_SEQ 	= 16,			// 下オフ
	CMN_CROSS_OFF_SEQ	= 14,			// ×オフ
	CMN_RETURN_OFF_SEQ	= 15,			// 戻るオフ
};

//------------------
// シーケンス(通常)
//------------------
enum{
	OBJ_CUR_SEQ			= 2,			// カーソル
	OBJ_CUR_ON_SEQ		= 3,			// カーソル(on)
	OBJ_BTN00_OFF_SEQ	= 4,			// (00)off
	OBJ_BTN00_ON_SEQ	= 5,			// (00)on
	OBJ_BTN01_OFF_SEQ	= 6,			// (01)off
	OBJ_BTN01_ON_SEQ	= 7,			// (01)on
	OBJ_BTN02_OFF_SEQ	= 8,			// (02)off
	OBJ_BTN02_ON_SEQ	= 9,			// (02)on
};

//--------------------
// スクロール(詳細用)
//--------------------
enum{
	SCRL_TOP_Y			= 32,			// レール上部のY座標
	SCRL_DOWN_Y			= 160,			// レール下部のY座標
	SCRL_BAR_SIZE		= 16,			// バーのサイズ
	//
	SCRL_WAIT_LEN		= 1,			// se用
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}T_DOWNLOAD_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}T_DOWNLOAD_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}T_DOWNLOAD_BGSCN_DATA;

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
}T_DOWNLOAD_CANVAS_DATA;

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
}T_DOWNLOAD_OBJ_DATA;

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
}T_DOWNLOAD_BMPOAM_DATA;

//=============================================================================
/**
 *								データ
 */
//=============================================================================
//---------------
// BG関連
//---------------
// BGPAL
static const T_DOWNLOAD_BGPAL_DATA td_bgpal_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_t_download_gra_t_download_bg_u_NCLR,	0,	4,},		// 上、BG
	{ PALTYPE_MAIN_BG,	NARC_t_download_gra_t_download_bg_d_NCLR,	0,	5,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const T_DOWNLOAD_BGCHR_DATA td_bgchr_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_t_download_gra_t_download_bg_u_NCGR,	0,	0,},		// 上、BG
	{ GFL_BG_FRAME2_S,	NARC_t_download_gra_t_download_bg_u_NCGR,	0,	0,},		// 上、BG
	//
	{ GFL_BG_FRAME3_M,	NARC_t_download_gra_t_download_bg_d_NCGR,	0,	0,},		// 下、BG
	{ GFL_BG_FRAME1_M,	NARC_t_download_gra_t_download_bg_d_NCGR,	0,	0,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const T_DOWNLOAD_BGSCN_DATA td_bgscn_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_t_download_gra_t_download_bg_u_01_NSCR,	0,	0,},		// 上、下地
	{ GFL_BG_FRAME2_S,	NARC_t_download_gra_t_download_bg_u_02_NSCR,	0,	0,},		// 上、中間
	//
	{ GFL_BG_FRAME3_M,	NARC_t_download_gra_t_download_bg_d_01_NSCR,	0,	0,},		// 下、下地
	{ GFL_BG_FRAME1_M,	NARC_t_download_gra_t_download_bg_d_02_NSCR,	0,	0,},		// 下、下地

	{ 0xff, 0, 0, 0,}, // end code
};

//--------
// info用
//--------
// BGCHR
static const T_DOWNLOAD_BGCHR_DATA td_bgchr_00_tbl[] = {
	{ GFL_BG_FRAME2_M,	NARC_t_download_gra_t_download_bg_d_NCGR,	0,	0,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};
// BGSCN
static const T_DOWNLOAD_BGSCN_DATA td_bgscn_00_tbl[] = {
	{ GFL_BG_FRAME2_M,	NARC_t_download_gra_t_download_bg_d_05_NSCR,	0,	0,},	// 下、下地

	{ 0xff, 0, 0, 0,}, // end code
};

//--------
// bgまとめ
//--------
static const T_DOWNLOAD_BGCHR_DATA *td_bgchr_sub_tbl[] = {
	td_bgchr_00_tbl,			// save(dummy)
	td_bgchr_00_tbl,			// select(dummy)
	td_bgchr_00_tbl,			// join(dummy)
	td_bgchr_00_tbl,			// info
};
//
static const T_DOWNLOAD_BGSCN_DATA *td_bgscn_sub_tbl[] = {
	td_bgscn_00_tbl,			// save(dummy)
	td_bgscn_00_tbl,			// select(dummy)
	td_bgscn_00_tbl,			// join(dummy)
	td_bgscn_00_tbl,			// info
};

//---------------
// CANVASの設定
//---------------
// (printutilと対応)
static const T_DOWNLOAD_CANVAS_DATA td_canvas_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	3,	28,	2,	2,},		// 上：トーナメントリスト名
	{ GFL_BG_FRAME1_S,	2,	7,	12,	2,	2,},		// 上：ルール(左)
	{ GFL_BG_FRAME1_S,	15,	7,	16,	2,	2,},		// 上：ルール(右)
	{ GFL_BG_FRAME1_S,	2,	9,	28,	6,	2,},		// 上：レベル制限
	{ GFL_BG_FRAME1_S,	2,	16,	18,	2,	2,},		// 上：トーナメント名
	{ GFL_BG_FRAME1_S,	2,	18,	28,	4,	2,},		// 上：内容
	//
	{ GFL_BG_FRAME0_M,	1,	0,	30,	3,	7,},		// 下：タイトル(注意)
	{ GFL_BG_FRAME0_M,	1,	4,	30,	4,	9,},		// 下：meswin
	{ GFL_BG_FRAME0_M,	1,	19,	30,	4,	9,},		// 下：下win
	//
	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

// (info用)
static const T_DOWNLOAD_CANVAS_DATA td_canvas_00_tbl[] = {
	// 注意
	{ GFL_BG_FRAME0_M,	2,	5,	28,	2,	3,},		// 下：項目00
	{ GFL_BG_FRAME0_M,	2,	8,	28,	2,	3,},		// 下：項目01
	{ GFL_BG_FRAME0_M,	2, 11,	28,	2,	3,},		// 下：項目02
	{ GFL_BG_FRAME0_M,	2, 14,	28,	2,	3,},		// 下：項目03
	{ GFL_BG_FRAME0_M,	2, 17,	28,	2,	3,},		// 下：項目04
	
	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

// (join用)
static const T_DOWNLOAD_CANVAS_DATA td_canvas_01_tbl[] = {
	// 注意
	{ GFL_BG_FRAME2_M,	3,	5,	28,	3,	2,},		// 下：項目00
	{ GFL_BG_FRAME2_M,	3, 10,	28,	3,	2,},		// 下：項目01
	{ GFL_BG_FRAME2_M,	3, 15,	28,	3,	2,},		// 下：項目02
	
	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

static const T_DOWNLOAD_CANVAS_DATA *td_canvas_sub_tbl[] = {
	td_canvas_01_tbl,		// save
	td_canvas_00_tbl,		// select(dummy)
	td_canvas_01_tbl,		// join
	td_canvas_00_tbl,		// info
};

static const s16 td_canvas_off_tbl[] = {
	CANVAS_JOIN_OFFSET, CANVAS_JOIN_NUM,	// save
	CANVAS_INFO_OFFSET, CANVAS_INFO_NUM,	// select(dummy)
	CANVAS_JOIN_OFFSET, CANVAS_JOIN_NUM,	// join
	CANVAS_INFO_OFFSET, CANVAS_INFO_NUM,	// info
};

//------------------
// OBJの情報(通常)
//------------------
// narc, resid, paloffset, palnum
static const s16 td_obj_main_info_tbl[] = {
	NARC_t_download_gra_t_download_obj_d_NCLR, T_DOWNLOAD_RESID_OBJ_PLT, 0, 6, // NCL
	NARC_t_download_gra_t_download_obj_d_NCGR, T_DOWNLOAD_RESID_OBJ_CHR, 0, 0,	// NCG
	NARC_t_download_gra_t_download_obj_d_NCER, T_DOWNLOAD_RESID_OBJ_CEL, 0, 0, // NCE
	NARC_t_download_gra_t_download_obj_d_NANR, -1, 0, 0,			// NAR?
};

// 情報
static const s16 td_obj_main_itbl[] = {
	T_DOWNLOAD_RESID_OBJ_PLT,
	T_DOWNLOAD_RESID_OBJ_CHR,
	T_DOWNLOAD_RESID_OBJ_CEL,
	//
	T_DOWNLOAD_OBJ_MAXNUM,
	T_DOWNLOAD_CLWKMODE_NORMAL,
};

// OBJのテーブルデータ(通常)
static const T_DOWNLOAD_OBJ_DATA td_obj_tbl[] = {
	{ 0,	0x30,	1,	1,	0,	1,	4,	0,},		// スクロールボタン
	{ 1,	0x38,	1,	1,	0,	1,	17,	0,},		// スクロールバー
	{ 2,	0x30,	1,	1,	0,	1,	18,	0,},		// カーソル
	//
	{ 4,	0x30,	1,	0,	0,	1,	24,	0,},		// アイコン00(詳細で使用)
	{ 6,	0x30,	1,	0,	0,	1,	25,	0,},		// アイコン01(詳細で使用)
	{ 8,	0x30,	1,	0,	0,	1,	26,	0,},		// アイコン02(詳細で使用)
	//
	{ 10,	0x30,	3,	0,	0,	1,	27,	0,},		// パネル00
	{ 10,	0x30,	3,	0,	0,	1,	28,	0,},		// パネル01
	{ 10,	0x30,	3,	0,	0,	1,	29,	0,},		// パネル02

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(cmn)
//------------------
// 情報
static const s16 td_obj_cmn_itbl[] = {
	T_DOWNLOAD_RESID_CMN_OBJ_PLT,
	T_DOWNLOAD_RESID_CMN_OBJ_CHR,
	T_DOWNLOAD_RESID_CMN_OBJ_CEL,
	//
	T_DOWNLOAD_CMN_OBJ_MAXNUM,
	T_DOWNLOAD_CLWKMODE_CMN,
};

// OBJのテーブルデータ(cmn)
static const T_DOWNLOAD_OBJ_DATA td_obj_cmn_tbl[] = {
	{ 4,	0x30,	1,	1,	0,	1,	7,	0,},		// 左マーク
	{ 5,	0x30,	1,	1,	0,	1,	8,	0,},		// 右マーク
	{ 0,	0x30,	1,	1,	0,	1,	9,	0,},		// ×マーク
	{ 1,	0x30,	1,	1,	0,	1, 10,	0,},		// 戻るマーク
	//
	{ 2,	0x30,	1,	0,	0,	1,	7,	0,},		// 下マーク(詳細で使用)
	{ 3,	0x30,	1,	0,	0,	1,	8,	0,},		// 上マーク(詳細で使用)

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

// (0x100 = 1)
// (0.25ずつ移動で32dotでスクロール)
static const s16 td_scroll_tbl[] = {
	0x40, 0x2000,
};

//------------------
// メッセージ(上画面)
//------------------
// 本体
static const s16 td_mes_u00_tbl[] = {
	wbt_dl_cap_002,				// 「○○トーナメント」
	wbt_dl_cap_003,				// 「バトルルール」
	wbt_dl_cap_008,				// 「トーナメントないよう」
	wbt_dl_error_01,			// dummy
};

// バトルルール用
static const s16 td_mes_u000_tbl[] = {
	wbt_dl_cap_004,				// 「シングル」
	wbt_dl_cap_005,				// 「ダブル」
	wbt_dl_cap_006,				// 「トリプル」
	wbt_dl_cap_007,				// 「ローテーション」
};

//------------------
// メッセージ(下画面)
//------------------
static const s16 td_mes_title_tbl[] = {
	wbt_dl_cap_009,				// 「ほぞんしたトーナメント」
	wbt_dl_cap_001,				// 「トーナメントダウンロード」
	wbt_dl_cap_010,				// 「でられない ポケモン」
	wbt_dl_cap_011,				// 「もたせられない どうぐ」
	wbt_dl_cap_012,				// 「もたせられない わざ」
};

//-----------------
// windowメッセージ
// (T_DOWNLOAD_WIN_*と対応)
//-----------------
static const s16 td_mes_d00_tbl[] = {
	wbt_dl_list_msg_02,			// 「さんかするトーナメントを選んでください」
	wbt_dl_list_msg_03,			// 「○○トーナメントにさんかしますか」
	wbt_dl_list_msg_01,			// 「○○トーナメントをどうしますか」
	wbt_dl_ev_msg_11,			// 「○○トーナメントをさくじょしますか」
	wbt_dl_ev_msg_12,			// 「○○トーナメントをほんとうにさくじょしますか」
	wbt_dl_ev_msg_02,			// 「保存したいトーナメントを選んでください」
	wbt_dl_ev_msg_05,			// 「このトーナメントはすでに保存しています」
	wbt_dl_ev_msg_000,			// 「トーナメントダウンロードを終了しますか」
	wbt_dl_ev_msg_10,			// 「トーナメントデータをけすのをやめますか」
	wbt_dl_ev_msg_09,			// 「トーナメントデータがいっぱいです、上書きしてください」
	wbt_dl_ev_msg_13,			// 「トーナメントデータをさくじょしました」
	wbt_dl_ev_msg_07,			// 「保存しています」
	wbt_dl_ev_msg_08,			// 「トーナメントデータをほぞんしました」
};

//-----------------
// 選択肢
// (T_DOWNLOAD_YESNO_*と対応)
//-----------------
// 3択メッセージ(join)
static const s16 td_mes_yesno_00_tbl[] = {
	wbt_dl_list_msg_03_01,		// 「さんかする」
	wbt_dl_list_msg_03_02,		// 「くわしくみる」
	wbt_dl_list_msg_03_03,		// 「やめる」
	-1,							// end
};
// 3択メッセージ(select)
static const s16 td_mes_yesno_01_tbl[] = {
	wbt_dl_list_msg_01_03,		// 「ほぞんする」
	wbt_dl_list_msg_03_02,		// 「くわしくみる」
	wbt_dl_list_msg_03_03,		// 「やめる」
	-1,							// end
};
// 2択メッセージ(はい/いいえ)
static const s16 td_mes_yesno_02_tbl[] = {
	wbt_dl_ev_msg_03_yes,		// 「はい」
	wbt_dl_ev_msg_03_no,		// 「いいえ」
	-1,							// end
	-1,							// end
};
// 2択メッセージ(save後)
static const s16 td_mes_yesno_03_tbl[] = {
	wbt_dl_list_msg_03_02,		// 「くわしくみる」
	wbt_dl_list_msg_03_03,		// 「やめる」
	-1,							// end
	-1,							// end
};
// 選択肢メッセージまとめ
static const s16 *td_yesno_mes_tbl[] = {
	td_mes_yesno_00_tbl,		// 参加
	td_mes_yesno_01_tbl,		// 保存
	td_mes_yesno_02_tbl,		// はい/いいえ
	td_mes_yesno_03_tbl,		// セーブ後
};

//-----------------
// info
//-----------------
static const s16 td_mes_info_tbl[] = {
	wbt_dl_list_msg_000,		// ポケモン
	wbt_dl_list_msg_001,		// どうぐ
	wbt_dl_list_msg_002,		// 技
	//
	wbt_dl_cap_013,				// なし
};

//----------------
// リスト処理
//-----------------
// リストタッチデータ
static const FRAMELIST_TOUCH_DATA td_list_tp_tbl[] =
{
  // スライドの数は上からのパネル分(7)にしないどだめらしい
  // (リストの処理で問題が起こるようだ...)
//  { {   0,  23, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 00:(ダミー)
  { {  24,  47, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 00: 
  { {  48,  71, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 01: 
  { {  72,  95, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 02: 
  { {  96, 119, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 03: 
  { { 120, 143, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 04: 
  { { 144, 167, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 05: 
  //
  { {  32, 160, 236, 255 }, FRAMELIST_TOUCH_PARAM_RAIL },     // 06: レール
  //
  { { 168, 192, 136, 160 }, FRAMELIST_TOUCH_PARAM_PAGE_UP },  // 07: 左
  { { 168, 192, 168, 192 }, FRAMELIST_TOUCH_PARAM_PAGE_DOWN },// 08: 右
  //
  { { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};

// スクロールなしのタッチデータ
static const FRAMELIST_TOUCH_DATA td_list_tp2_tbl[] =
{
//  { {   0,  23, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 00: 
  { {  24,  47, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 00: 
  { {  48,  71, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 01: 
  { {  72,  95, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 02: 
  { {  96, 119, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 03: 
  { { 120, 143, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 04: 
  { { 144, 167, 8, 232 }, FRAMELIST_TOUCH_PARAM_ITEM },    // 05: 
  //
  { { 168, 192, 136, 160 }, FRAMELIST_TOUCH_PARAM_PAGE_UP },  // 08: 左
  { { 168, 192, 168, 192 }, FRAMELIST_TOUCH_PARAM_PAGE_DOWN },// 09: 右
  //
  { { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};

// 
static const FRAMELIST_HEADER td_frame_list_header = {
  GFL_BG_FRAME2_M,			// 上
  FRAMELIST_SUB_BG_NONE,	// 下
  
  1,   // 項目フレーム表示開始Ｘ座標
  3,    // 項目フレーム表示開始Ｙ座標(●0以外だとリストが長い場合にうまくいかない?)
  28,   // 項目フレーム表示Ｘサイズ
  3,    // 項目フレーム表示Ｙサイズ
  
  2,    // フレーム内に表示するBMPWINの表示Ｘ座標
  0,    // フレーム内に表示するBMPWINの表示Ｙ座標
  28,   // フレーム内に表示するBMPWINの表示Ｘサイズ
  3,    // フレーム内に表示するBMPWINの表示Ｙサイズ(itemSizY)
  2,    // フレーム内に表示するBMPWINのパレット
  
  {24,12,8,6,4,3,},    // スクロール速度 [0] = 最速 ※itemSizY*8を割り切れる値であること！
   2,   // 選択項目のパレット(●この番号のパレットが選択中となるようだ)
  16,   // スクロールバーのＹサイズ
   7,   // 項目登録数(●初期化前に変更すればよい)
   2,   // 背景登録数
   0,   // 初期カーソル位置
   6,   // カーソル移動範囲
   0,   // 初期スクロール値
  
  td_list_tp_tbl,  // タッチデータ
  
  NULL, // コールバック関数
  NULL, // コールバック関数に渡されるワーク
};

//---------------------------------
// 共通ボタン
// (通常、オン、オフ、0)
//---------------------------------
static const s16 td_cmn_btn_tbl[] = {
	CMN_LEFT_SEQ,   CMN_LEFT_ON_SEQ,  CMN_LEFT_OFF_SEQ,  0,	// 左
	CMN_RIGHT_SEQ,  CMN_RIGHT_ON_SEQ, CMN_RIGHT_OFF_SEQ, 0,	// 右
	CMN_CROSS_SEQ,  CMN_CROSS_ON_SEQ, CMN_CROSS_OFF_SEQ, 0,	// ×
	CMN_RETURN_SEQ, CMN_RETURN_ON_SEQ,CMN_RETURN_OFF_SEQ,0,	// 戻る
	//
	CMN_DOWN_SEQ,   CMN_DOWN_ON_SEQ,  CMN_DOWN_OFF_SEQ,  0,	// 下
	CMN_UP_SEQ,     CMN_UP_ON_SEQ,    CMN_UP_OFF_SEQ,    0,	// 上
};

//---------------------------------
// ボタン
// (通常、選択)
//---------------------------------
static const s16 td_info_btn_tbl[] = {
	OBJ_BTN00_OFF_SEQ, OBJ_BTN00_ON_SEQ,		// 00
	OBJ_BTN01_OFF_SEQ, OBJ_BTN01_ON_SEQ,		// 01
	OBJ_BTN02_OFF_SEQ, OBJ_BTN02_ON_SEQ,		// 02
};

//------------------
// タッチペン領域
//------------------
// セーブ画面
static const GFL_UI_TP_HITTBL td_rect_00_tbl[] = {
  { 168, 192, 232, 255 }, 			// 戻る
  { 168, 192, 136, 160 }, 			// 下
  { 168, 192, 168, 192 }, 			// 上
  //
  {  44,  68,   8, 232 }, 			// パネル00
  {  84, 108,   8, 232 }, 			// パネル01
  { 124, 148,   8, 232 }, 			// パネル02
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// 選択画面
static const GFL_UI_TP_HITTBL td_rect_01_tbl[] = {
  { 168, 192, 200, 224 }, 			// ×
  { 168, 192, 232, 255 }, 			// 戻る
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// 参加画面
static const GFL_UI_TP_HITTBL td_rect_02_tbl[] = {
  { 168, 192, 232, 255 }, 			// 戻る
  { 168, 192, 136, 160 }, 			// 下
  { 168, 192, 168, 192 }, 			// 上
  //
  {  44,  68,   8, 232 }, 			// パネル00
  {  84, 108,   8, 232 }, 			// パネル01
  { 124, 148,   8, 232 }, 			// パネル02
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// 詳細画面
static const GFL_UI_TP_HITTBL td_rect_03_tbl[] = {
  { 168, 192, 200, 224 }, 			// ×
  { 168, 192, 232, 255 }, 			// 戻る
  { 168, 192, 136, 160 }, 			// 下
  { 168, 192, 168, 192 }, 			// 上
  //
  { 168, 192,   8,  32 }, 			// ポケモン
  { 168, 192,  40,  64 }, 			// 道具
  { 168, 192,  72,  96 }, 			// 技
  //
  {  32, 160, 236, 255 }, 			// スクロールバー
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// (タッチペン)まとめ
static const GFL_UI_TP_HITTBL *td_rect_tbl[] = {
	td_rect_00_tbl,
	td_rect_01_tbl,
	td_rect_02_tbl,
	td_rect_03_tbl,
};

//=============================================================================
/**
 *								宣言
 */
//=============================================================================
// bg
static void td_init_bg( T_DOWNLOAD_MAIN_WORK* wk );
//
// string
static void td_init_string( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit_string( T_DOWNLOAD_MAIN_WORK* wk );
//
// obj
static void td_init_obj_clwk( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const T_DOWNLOAD_OBJ_DATA *obj_adr, const s16 *info_adr );
static void td_init_obj_res( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr );
static void td_init_obj_res_cmn( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype );
static void td_init_obj( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit_obj( T_DOWNLOAD_MAIN_WORK* wk );
//
// リスト処理
static void td_listcallback_draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );
static void td_listcallback_move( void *work, u32 pos, BOOL flag );
static void td_listcallback_scroll( void *work, s8 mv );
static void td_liststr_print( T_DOWNLOAD_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg );
static void td_PrintStrBuf( T_DOWNLOAD_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int strIdx, PRINTSYS_LSB col, int y, int pos );
static void td_strbuf_print( T_DOWNLOAD_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int param, int pos );
//
// パレットチェンジ
static void td_init_chgpal( T_DOWNLOAD_MAIN_WORK* wk );
static void td_main_chgpal( T_DOWNLOAD_MAIN_WORK* wk );
//
static void td_set_up_visible( T_DOWNLOAD_MAIN_WORK * wk, const BOOL bflg );
//
static void td_init_fontpal( T_DOWNLOAD_MAIN_WORK* wk );
static void td_chg_fontpal( T_DOWNLOAD_MAIN_WORK* wk, const BOOL bflg );


//=============================================================================
/**
 *								関数
 */
//=============================================================================
//---------------------
// リストのコールバック
//---------------------
static const FRAMELIST_CALLBACK td_list_callback = {
	td_listcallback_draw,
	td_listcallback_move,
	td_listcallback_scroll,
};

//-----------------------------------------------------------------------------
/**
 *	@brief  データ関連初期化
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_init_data( T_DOWNLOAD_MAIN_WORK* wk )
{
	// yes/noウインドウ用
	wk->pAppTask 	= 0;
	wk->pAppTaskRes = APP_TASKMENU_RES_Create( GFL_BG_FRAME0_M, PLTID_YESNO_WIN_BASE, wk->font, wk->print_que, wk->heapID );
	
	// bg初期化
	td_init_bg( wk );
	// 文字列
	td_init_string( wk );
	// obj
	td_init_obj( wk );
	// パレットチェンジ用(obj)
	td_init_chgpal( wk );
	// パレットチェンジ(フォント)
	td_init_fontpal( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  データ関連終了
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_exit_data( T_DOWNLOAD_MAIN_WORK* wk )
{
	// 文字列
	td_exit_string( wk );
	// obj
	td_exit_obj( wk );
	// yes/no
	TdCmn_exit_yesnowin( wk );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_bg( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	s32		tmp;
	const T_DOWNLOAD_BGPAL_DATA	*padr = &td_bgpal_tbl[0];
	const T_DOWNLOAD_BGCHR_DATA	*cadr = &td_bgchr_tbl[0];
	const T_DOWNLOAD_BGSCN_DATA	*sadr = &td_bgscn_tbl[0];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_T_DOWNLOAD_GRA, wk->heapID );
	
	// パレット転送
	for(i=0;i<T_DOWNLOAD_BGNUM_MAXNUM;i++){
		// 終了条件
		if(padr->type == 0xff){
			break;
		}
		tmp = padr->size;
		tmp *= 0x20;		// 転送サイズ
		GFL_ARCHDL_UTIL_TransVramPalette( handle, padr->arcid, padr->type, padr->offset, tmp, wk->heapID );
		padr++;
	}
	//
	// キャラ転送
	for(i=0;i<T_DOWNLOAD_BGNUM_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, wk->heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<T_DOWNLOAD_BGNUM_MAXNUM;i++){
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
 *	@brief  BG関連の初期化
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_init_sub_bg( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s32		i;
	s32		tmp;
	const T_DOWNLOAD_BGCHR_DATA	*cadr = td_bgchr_sub_tbl[mode];
	const T_DOWNLOAD_BGSCN_DATA	*sadr = td_bgscn_sub_tbl[mode];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_T_DOWNLOAD_GRA, wk->heapID );
	//
	// キャラ転送
	for(i=0;i<T_DOWNLOAD_BGNUM_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, wk->heapID );
		cadr++;
	}
	// スクリーン転送
	for(i=0;i<T_DOWNLOAD_BGNUM_MAXNUM;i++){
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

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(現在、Canvasの作成)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_string( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	const T_DOWNLOAD_CANVAS_DATA	*adr = &td_canvas_tbl[0];
	
	for(i=0;i<T_DOWNLOAD_CANVAS_MAXNUM;i++){
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
		//
		wk->PFlag &= ~(1<<i);
	}
	//
	//-----------------
	// fontパレット転送
	//-----------------
	{
		ARCHANDLE	*p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		// Main
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, PLTID_CAPTION_FONT*0x20, 0x20, wk->heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, PLTID_SYSWIN_FONT*0x20, 0x20, wk->heapID );
		// Sub
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_SUB_BG,  PLTID_UP_FONT*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
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
void TdCmn_disp_string( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col )
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
 *	@brief  文字表示終了
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit_string( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	
	for(i=0;i<T_DOWNLOAD_CANVAS_MAXNUM;i++){
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
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				キャンバスの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_clear_canvas( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	// clear
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_CLEAR_COLOR );
	wk->PrintUtil[mode].transReq = TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(追加分)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			(0:詳細、1:3パネル用)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_init_string2( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s16		buf[2];
	s32		i;
	const T_DOWNLOAD_CANVAS_DATA	*adr = td_canvas_sub_tbl[mode];
	
	buf[0] = td_canvas_off_tbl[2*mode];			// offset
	buf[1] = td_canvas_off_tbl[2*mode+1];		// num
	
	for(i=0;i<T_DOWNLOAD_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット
		wk->PrintUtil[buf[0]+i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, 
																	 GFL_BMP_CHRAREA_GET_B );
		// スクリーンは一度でよいはず
		GFL_BMPWIN_MakeScreen( wk->PrintUtil[buf[0]+i].win );
		GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[buf[0]+i].win) );
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示終了(追加分)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_exit_string2( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s16		buf[2];
	s32		i;
	
	buf[0] = td_canvas_off_tbl[2*mode];			// offset
	buf[1] = td_canvas_off_tbl[2*mode+1];		// num
	
	for(i=0;i<buf[1];i++){
		if(wk->PrintUtil[buf[0]+i].win){
			// chr側
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[buf[0]+i].win ), MES_CLEAR_COLOR );
			// scn側
			GFL_BMPWIN_ClearScreen( wk->PrintUtil[buf[0]+i].win );
			// 破棄
			GFL_BMPWIN_Delete( wk->PrintUtil[buf[0]+i].win );
			wk->PrintUtil[buf[0]+i].win = 0;
		}
	}
	if(mode == T_DOWNLOAD_MODE_INFO){
		// スクリーンもクリアしておく
		GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
	}else{
		// スクリーンもクリアしておく
		GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
	}
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const T_DOWNLOAD_OBJ_DATA* アドレス
 *	@param	const s16*				アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj_clwk( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const T_DOWNLOAD_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16						tmp;
	s16						pos[2];
	s16						buf[6];
	s32 					i;
	BOOL					bflg;
	GFL_CLWK_DATA 			cldata;
	CLWK_FLIP_TYPE			flip;
    GFL_CLUNIT*				p_clunit 	= T_DOWNLOAD_GRAPHIC_GetClunit( wk->graphic );
	// アドレスセット
	const T_DOWNLOAD_OBJ_DATA *adr 	= obj_adr;
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
	if(buf[4] == T_DOWNLOAD_CLWKMODE_NORMAL){			// 通常
		clwk = &wk->wk_obj.clwk[0];
	}
	else if(buf[4] == T_DOWNLOAD_CLWKMODE_CMN){		// cmn
		clwk = &wk->wk_obj.cmn_clwk[0];
	}
	
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		pos[0] = TdCmn_get_empty(wk, tmp*2);
		pos[1] = TdCmn_get_empty(wk, tmp*2+1);
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
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj_res( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr )
{
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_T_DOWNLOAD_GRA, wk->heapID );
    GFL_CLUNIT*	p_clunit = T_DOWNLOAD_GRAPHIC_GetClunit( wk->graphic );
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
 *	@brief  OBJ初期化(cmn res)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj_res_cmn( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype )
{
	// cmnのidを取得
	const u32 	arcid    = APP_COMMON_GetArcId();
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( arcid, wk->heapID );
    GFL_CLUNIT*	p_clunit = T_DOWNLOAD_GRAPHIC_GetClunit( wk->graphic );
	u32			idx[4];
	//
	idx[0] = APP_COMMON_GetBarIconPltArcIdx();		// pal
	idx[1] = APP_COMMON_GetBarIconCharArcIdx();		// ncg
	idx[2] = APP_COMMON_GetBarIconCellArcIdx(  APP_COMMON_MAPPING_128K );	// cell
	idx[3] = APP_COMMON_GetBarIconAnimeArcIdx( APP_COMMON_MAPPING_128K );	// anim
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ T_DOWNLOAD_RESID_CMN_OBJ_PLT ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, idx[0], drawtype, 0x20*PLTID_OBJ_CMN, 0, 4, wk->heapID );
	wk->wk_obj.res[ T_DOWNLOAD_RESID_CMN_OBJ_CHR ] = GFL_CLGRP_CGR_Register(      p_handle, idx[1], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ T_DOWNLOAD_RESID_CMN_OBJ_CEL ] = GFL_CLGRP_CELLANIM_Register( p_handle, idx[2], idx[3], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化まとめ
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj( T_DOWNLOAD_MAIN_WORK* wk )
{
    GFL_CLUNIT*	p_clunit = T_DOWNLOAD_GRAPHIC_GetClunit( wk->graphic );
    //
	// リソース読み込み(通常)
	td_init_obj_res( wk, CLSYS_DRAW_MAIN, &td_obj_main_info_tbl[0] );
	// リソース読み込み(cmn)
	td_init_obj_res_cmn( wk, CLSYS_DRAW_MAIN );
	// CLWK登録(通常)
	td_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &td_obj_tbl[0], &td_obj_main_itbl[0] );
	// CLWK登録(cmn)
	td_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &td_obj_cmn_tbl[0], &td_obj_cmn_itbl[0] );
	//
	// 通信アイコンセット(上画面)
	GFL_NET_ReloadIconTopOrBottom( TRUE, wk->heapID );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit_obj( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<T_DOWNLOAD_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk[i] );
	}
	for(i=0;i<T_DOWNLOAD_CMN_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.cmn_clwk[i] );
	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[T_DOWNLOAD_RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[T_DOWNLOAD_RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[T_DOWNLOAD_RESID_OBJ_CEL] );
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[T_DOWNLOAD_RESID_CMN_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[T_DOWNLOAD_RESID_CMN_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[T_DOWNLOAD_RESID_CMN_OBJ_CEL] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_chg_objseq( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	if(wmode == T_DOWNLOAD_CLWKMODE_NORMAL){			// 通常
		if(wnum >= T_DOWNLOAD_CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
	}
	else if(wmode == T_DOWNLOAD_CLWKMODE_CMN){			// 共通
		if(wnum >= T_DOWNLOAD_CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.cmn_clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.cmn_clwk[wnum], TRUE );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_set_obj_visible( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// 表示/非表示
	if(wmode == T_DOWNLOAD_CLWKMODE_NORMAL){		// 通常
		if(wnum >= T_DOWNLOAD_CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
	}
	else if(wmode == T_DOWNLOAD_CLWKMODE_CMN){		// 共通
		if(wnum >= T_DOWNLOAD_CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.cmn_clwk[wnum], bflg );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのアニメの終了待ち
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *
 *	@retval	const BOOL			(TRUE:再生中、FALSE:停止、終了)
 */
//-----------------------------------------------------------------------------
BOOL TdCmn_is_anime_end( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// 終了判定らしい
	if(wmode == T_DOWNLOAD_CLWKMODE_NORMAL){		// 通常
		if(wnum >= T_DOWNLOAD_CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	else if(wmode == T_DOWNLOAD_CLWKMODE_CMN){		// 共通
		if(wnum >= T_DOWNLOAD_CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.cmn_clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	return( bflg );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  emptyの取得
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
s16 TdCmn_get_empty( T_DOWNLOAD_MAIN_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= T_DOWNLOAD_EMPTY_MAXNUM)){
		OS_Printf("empty error :%d\n",num);
		return(0);
	}
	
	adr = wk->EmptyBuffer;
	return( adr[num] );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *  @brief   はい/いいえウインドウ 開始
 *
 *  @param  T_DOWNLOAD_MAIN_WORK *wk
 *  @param  const s32			(番号)
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
void TdCmn_init_yesnowin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	APP_TASKMENU_INITWORK appinit;
	s32		i;
	s32		j;
	const s16 *adr = td_yesno_mes_tbl[mode];
	
	// 個数を求める
	j=0;
	for(i=0;i<3;i++){
		if(adr[i] == -1){
			break;
		}
		j++;
	}
//	OS_Printf("mes num :%d\n",j);
	if(j > 3){
		OS_Printf("mes num error:%d\n",j);
		j = 3;
	}
	
	appinit.heapId		= wk->heapID;
	appinit.itemNum		= j;					// アイテム数
	appinit.itemWork	= &wk->appitem[0];		// 情報セットのバッファ
	//
	appinit.posType		= ATPT_RIGHT_DOWN;
	appinit.charPosX	= 32;
	appinit.charPosY	= 24;
	appinit.w			= APP_TASKMENU_PLATE_WIDTH;
	appinit.h			= APP_TASKMENU_PLATE_HEIGHT;
	//
	for(i=0;i<j;i++){
		wk->appitem[i].str = GFL_STR_CreateBuffer(100, wk->heapID);
		GFL_MSG_GetString( wk->msg[0], adr[i], wk->appitem[i].str);
		wk->appitem[i].msgColor	= PRINTSYS_LSB_Make( 0xe, 0xf, 0 );
		// 戻るマークの対応
		if(i == 2){
			wk->appitem[i].type = APP_TASKMENU_WIN_TYPE_RETURN;
		}else{
			wk->appitem[i].type = APP_TASKMENU_WIN_TYPE_NORMAL;
		}
	}
	wk->pAppTask = APP_TASKMENU_OpenMenu( &appinit, wk->pAppTaskRes );
	//
	for(i=0;i<j;i++){
		GFL_STR_DeleteBuffer( wk->appitem[i].str );
	}
}

//-----------------------------------------------------------------------------
/**
 *  @brief  はい/いいえウィンドウ終了
 *
 *  @param  T_DOWNLOAD_MAIN_WORK	*wk
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
void TdCmn_exit_yesnowin( T_DOWNLOAD_MAIN_WORK* wk )
{
	// 終了処理
	if(wk->pAppTask){
		APP_TASKMENU_CloseMenu( wk->pAppTask );
		wk->pAppTask = NULL;
	}
}

//-----------------------------------------------------------------------------
/**
 *  @brief  はい/いいえウィンドウ処理待ち
 *
 *  @param  T_DOWNLOAD_MAIN_WORK	*wk
 *
 *  @retval s32				(-1:未処理、0:項目番号)
 */
//-----------------------------------------------------------------------------
s32  TdCmn_chk_yesnowin( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		flg=-1;
	s32		pos;
	
	// 念のため
	if(wk->pAppTask == 0){
		return( flg );
	}
	//
	APP_TASKMENU_UpdateMenu( wk->pAppTask );
	
	// 選択中
	if( APP_TASKMENU_IsFinish( wk->pAppTask ) ){
		pos = APP_TASKMENU_GetCursorPos( wk->pAppTask );
		// 項目番号を返す
		return( pos );
	}
	//
	return( flg );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ(セット)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			モード
 *	@param	const s32			メッセージid
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_set_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 id )
{
	// PrintStream終了時にはdeleteしないとtaskが残ってしまうようだ
	if( wk->PrintStream ){
		PRINTSYS_PrintStreamDelete( wk->PrintStream );
		wk->PrintStream = 0;
	}
	// Msgの再構築
	if(wk->MsgBuffer){
		GFL_STR_DeleteBuffer( wk->MsgBuffer );
		wk->MsgBuffer = 0;
	}
	// メッセージバッファ
	wk->MsgBuffer = GFL_MSG_CreateString( wk->msg[0], td_mes_d00_tbl[id] );
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
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval	BOOL				(TRUE:再生中、FALSE:終了)
 */
//-----------------------------------------------------------------------------
BOOL TdCmn_main_meswin( T_DOWNLOAD_MAIN_WORK* wk )
{
	BOOL	bflg;
	
	// 念のため
	if(wk->PrintStream == 0){
		return( FALSE );
	}
	//
	// 文字送りカーソル処理
	APP_KEYCURSOR_Main( wk->MsgCursorWork, wk->PrintStream, wk->PrintUtil[T_DOWNLOAD_MSG_DOWN_MESWIN_01].win );
	// streamの入力処理をまとめて処理してくれるらしい
	// 内部でkey/touchの判定もしてくれるようだ
	bflg = APP_PRINTSYS_COMMON_PrintStreamFunc( &wk->PrintSysWork , wk->PrintStream );
	if( bflg == TRUE ){
		// 終了
		return( FALSE );
	}
	return( TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ全クリア(frmのクリア)(要修正)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			(0:全クリア、1:スクリーンクリア)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_clear_frame( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	// clear
	if(mode == 0){
		GFL_BG_ClearFrame( GFL_BG_FRAME0_M );
	}
	// screen clear
	else{
		GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  winにメッセージ表示
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(canvasの番号)
 *	@param	const s32				番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_disp_winmes( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 id, const s32 smode )
{
	s32			flg=0;
	STRBUF		*strbuf;
	STRBUF		*strbuf2;
	STRBUF		*expbuf;
	const WBTDL_DATA	*wbtl;
	
	//-----------------
	// ウインドウ表示
	//-----------------
	// 会話ウィンドウセット(キャラデータを転送)
	BmpWinFrame_GraphicSet( GFL_BG_FRAME0_M, 1, PLTID_SYSWIN_BASE, MENU_TYPE_SYSTEM, wk->heapID);
	// 会話ウインドウ表示
	BmpWinFrame_Write( wk->PrintUtil[mode].win, WINDOW_TRANS_ON, 1, PLTID_SYSWIN_BASE );
	// 文字スクリーン作成
	GFL_BMPWIN_MakeScreen( wk->PrintUtil[mode].win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[mode].win) );
	// クリア
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_SYSWIN_CLEAR_COLOR );
	GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[mode].win );
	//
	if(smode == 0){			// 一発だし
		// メッセージ表示
		switch(id){
			case T_DOWNLOAD_WIN_JOIN_01:
			case T_DOWNLOAD_WIN_WHAT_00:
			case T_DOWNLOAD_WIN_DEL_00:
			case T_DOWNLOAD_WIN_DEL_01:
				flg = 1;
				break;
		}
		
		// 通常
		if(flg == 0){
			strbuf = GFL_MSG_CreateString( wk->msg[0], td_mes_d00_tbl[id] );
			TdCmn_disp_string( wk, mode, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(1,2,0) );
			GFL_STR_DeleteBuffer( strbuf );
		}else{
			wbtl = wk->CurrentWbt;
			if(wbtl == 0){
				OS_Printf("wbt error\n");
			}
			//
			strbuf = GFL_MSG_CreateString( wk->msg[0], td_mes_d00_tbl[id] );
			// セーブデータからトーナメント名を取得
			strbuf2  = GFL_STR_CreateBuffer( 128, wk->heapID );
			expbuf   = GFL_STR_CreateBuffer( 128, wk->heapID );
			TdSave_get_str( wk, wbtl, T_DOWNLOAD_SAVE_STR_NAME, strbuf2 );
			WORDSET_RegisterWord( wk->Wordset, 0, strbuf2, 0, TRUE, PM_LANG );
			//
			WORDSET_ExpandStr( wk->Wordset, expbuf, strbuf );
			TdCmn_disp_string( wk, mode, 0, 0, expbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(1,2,0) );
			GFL_STR_DeleteBuffer( strbuf );
			GFL_STR_DeleteBuffer( strbuf2 );
			GFL_STR_DeleteBuffer( expbuf );
		}
	}else{					// stream表示
		TdCmn_set_meswin( wk, mode, id );
	}
}

//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  アニメのセット
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(CMNの番号:フラグと対応)
 *	@param	const s32				(0:on、1:off、-1:初期化)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_set_cmn_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 smode )
{
	// アニメ再生
	TdCmn_chg_objseq( wk, T_DOWNLOAD_CLWKMODE_CMN, mode, td_cmn_btn_tbl[mode*4+1+smode] );
	//
	if(smode == 0){
		wk->CmnFlag |= (1<<(mode));
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	背景BGスクロール
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_main_bg_scroll( T_DOWNLOAD_MAIN_WORK * wk )
{
	// 背景スクロール
	int		offy;
	//
	// 0.25ずつ移動、32dotで戻し(0x100 = 1.0)
	wk->ScrollOffy += td_scroll_tbl[0];
	if(wk->ScrollOffy >= td_scroll_tbl[1]){
		wk->ScrollOffy = 0;
	}
	offy = (wk->ScrollOffy>>8)&0xff;
	// セット
	GFL_BG_SetScrollReq( GFL_BG_FRAME3_M, GFL_BG_SCROLL_Y_SET, -offy );
	GFL_BG_SetScrollReq( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, -offy );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	上画面表示まとめ
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 * @param	const s32					(0:セーブ、1:リスト)
 * @param	const s32					(0-2:スロット番号)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_disp_upmes( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 num )
{
	s32		i;
	s32		id;
	STRBUF* strbuf;
	STRBUF* strbuf2;
	STRBUF* expbuf;
	const WBTDL_DATA	*wbtl;
	
	// セーブデータ
	if(mode == T_DOWNLOAD_DATA_SAVE){
		wbtl = wk->WbtSaveData[num];
	}
	// リストデータ
	else{
//		wbtl = &wk->Param->data->data[num];
		wbtl = wk->CheckBuffer[num];
	}
	// いったんクリア
	for(i=0;i<6;i++){
		TdCmn_clear_canvas( wk, T_DOWNLOAD_MSG_UP_TNAME+i );
	}
	
	// トーナメント名
	strbuf  = GFL_STR_CreateBuffer( 128, wk->heapID );
	expbuf  = GFL_STR_CreateBuffer( 128, wk->heapID );
	TdSave_get_str( wk, wbtl, T_DOWNLOAD_SAVE_STR_NAME, strbuf );			// トーナメント名取得
	WORDSET_RegisterWord( wk->Wordset, 0, strbuf, 0, TRUE, PM_LANG );
	strbuf2 = GFL_MSG_CreateString( wk->msg[0], td_mes_u00_tbl[0] );
	WORDSET_ExpandStr( wk->Wordset, expbuf, strbuf2 );
	TdCmn_disp_string( wk, T_DOWNLOAD_MSG_UP_TNAME, 112, 0, expbuf, PRINTTOOL_MODE_CENTER, PRINTSYS_LSB_Make(12,13,0));
	GFL_STR_DeleteBuffer( strbuf );
	GFL_STR_DeleteBuffer( strbuf2 );
	GFL_STR_DeleteBuffer( expbuf );
	//
	// ルール(左)
	strbuf = GFL_MSG_CreateString( wk->msg[0], td_mes_u00_tbl[1] );
	TdCmn_disp_string( wk, T_DOWNLOAD_MSG_UP_RULE_00, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(14,15,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// ルール(右)
	id = WBTDL_DATA_GetBtlRule( wbtl );
	strbuf = GFL_MSG_CreateString( wk->msg[0], td_mes_u000_tbl[(id&0x03)] );
	TdCmn_disp_string( wk, T_DOWNLOAD_MSG_UP_RULE_01, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(14,15,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// ルール説明
	strbuf = GFL_STR_CreateBuffer( 256, wk->heapID );
	TdSave_get_str( wk, wbtl, T_DOWNLOAD_SAVE_STR_EXPLAIN_01, strbuf );		// ルール説明取得
	TdCmn_disp_string( wk, T_DOWNLOAD_MSG_UP_EXP_00, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(14,15,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 内容
	strbuf = GFL_MSG_CreateString( wk->msg[0], td_mes_u00_tbl[2] );
	TdCmn_disp_string( wk, T_DOWNLOAD_MSG_UP_INFO, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(14,15,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 内容説明
	strbuf = GFL_STR_CreateBuffer( 256, wk->heapID );
	TdSave_get_str( wk, wbtl, T_DOWNLOAD_SAVE_STR_EXPLAIN_00, strbuf );		// 内容説明取得
	TdCmn_disp_string( wk, T_DOWNLOAD_MSG_UP_EXP_01, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(14,15,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// カレントをセット
	wk->CurrentWbt = (WBTDL_DATA *)wbtl;
}

//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームリスト初期化
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_init_framelist( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode )
{
	FRAMELIST_HEADER  header = td_frame_list_header;
	ARCHANDLE * arc_handle;
	s32		i;
	s32		flg=0;
	BOOL	bflg;
	//
	header.cbWork = wk;
	//
	// 破棄
	TdCmn_exit_framelist( wk );
	//
	// 読み込み
	arc_handle = GFL_ARC_OpenDataHandle( ARCID_T_DOWNLOAD_GRA, wk->heapID );
	
	//----------------------------
	// オリジナルデータの読み込み
	// (パレットは元BGに含まれている)
	//----------------------------
	GFL_ARCHDL_UTIL_TransVramBgCharacter( arc_handle, NARC_t_download_gra_t_download_bg_d_NCGR, GFL_BG_FRAME2_M, 0, 0, 0, wk->heapID );
	
	//---------------
	// FrameListの設定
	//---------------
	// パラメータ設定
	header.cbFunc  		= &td_list_callback;
	header.itemMax 		= wk->DispMaxNum;			// 重要
	header.initPos		= wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT];
	header.initScroll	= wk->ListScrollPos[T_DOWNLOAD_LIST_CURRENT];
	if(wk->DispMaxNum <= T_DOWNLOAD_DISP_PANEL_MAX){
		header.touch = td_list_tp2_tbl;
	}
	// 生成
	wk->frame_list_work = FRAMELIST_Create( &header, wk->heapID );
	
	// 項目背景設定
	FRAMELIST_LoadFrameGraphicAH( wk->frame_list_work, arc_handle, NARC_t_download_gra_t_download_bg_d_03_NSCR, FALSE, 0 );		// 通常
	FRAMELIST_LoadFrameGraphicAH( wk->frame_list_work, arc_handle, NARC_t_download_gra_t_download_bg_d_04_NSCR, FALSE, 1 );		// 通常
	// 点滅アニメパレット設定
	// (互換性のため設定しておく)=>(objで処理する)
	FRAMELIST_LoadBlinkPalette(   wk->frame_list_work, arc_handle, NARC_t_download_gra_t_download_bg_d_NCLR, 2, 2 );			// 大丈夫?
	GFL_ARC_CloseDataHandle( arc_handle );
	//
	// 項目追加(タイトル文字をセット)
	for(i=0;i<header.itemMax;i++){
		flg = 0;
		bflg = TdSave_chk_save_list( wk, i );
		if(bflg == TRUE){
			flg = 1;
		}
		// 項目を表示
		FRAMELIST_AddItem( wk->frame_list_work, flg, i );
	}
	// 初期化(場所移動)
//	FRAMELIST_Init( wk->frame_list_work );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームMain
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
u32 TdCmn_main_framelist( T_DOWNLOAD_MAIN_WORK * wk )
{
	u32		ret = FRAMELIST_RET_NONE;
	
	// Main
	if( wk->frame_list_work ){
		// PAGEUP、PAGEDOWNの場合は、スクロールが終了時に、
		// パラメータをセットする必要がある(タイミング注意)
		ret = FRAMELIST_Main( wk->frame_list_work );
//		OS_Printf("ret :%x\n",ret);
	}
	
#if 0	// debug
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_Y) ){		//
		u32		buf[3];
		
		buf[0] = FRAMELIST_GetListPos(     wk->frame_list_work );
		buf[1] = FRAMELIST_GetCursorPos(   wk->frame_list_work );
		buf[2] = FRAMELIST_GetScrollCount( wk->frame_list_work );
		//
		OS_Printf("%d,%d,%d\n",buf[0],buf[1],buf[2]);
	}
#endif
	return( ret );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームMainのchk
 *
 * @param	T_DOWNLOAD_MAIN_WORK*	wk
 *
 * @return  s32						(0:なし、1:カーソル移動、2:決定)
 */
//--------------------------------------------------------------------------------------------
s32 TdCmn_is_main_framelist( T_DOWNLOAD_MAIN_WORK * wk, const u32 mode )
{
	s16			flg=0;
	
	switch(mode){
		case 0:								// 項目をタッチ
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
//		case 6:
			flg = 2;
			break;
		//
		case FRAMELIST_RET_PAGE_UP:			// page up
		case FRAMELIST_RET_PAGE_UP_NONE:	// page up 2
			// アニメ再生
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_LEFT, 0 );
			flg = 3;
			break;
		case FRAMELIST_RET_PAGE_DOWN:		// page down
		case FRAMELIST_RET_PAGE_DOWN_NONE:	// page down 2
			// アニメ再生
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RIGHT, 0 );
			flg = 3;
			break;
		//
		case FRAMELIST_RET_JUMP_TOP:		// top
		case FRAMELIST_RET_JUMP_BOTTOM:		// bottom
			flg = 4;
			break;
		//
		case FRAMELIST_RET_MOVE:			// move
		case FRAMELIST_RET_SCROLL:			// move
			flg = 1;
			break;
		default:
			break;
	}
	// リスト移動
	if(flg == 1){
	}
	// 決定
	else if(flg == 2){
		// カーソル位置の取得
		TdCmn_calc_framelist( wk );
		wk->CmnFlag |= T_DOWNLOAD_FLG_UPDATE;
	}
	// LR
	else if(flg == 4){
		// カーソル位置の取得
		TdCmn_calc_framelist( wk );
		wk->CmnFlag |= T_DOWNLOAD_FLG_UPDATE;
	}
	return( flg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレーム破棄
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_exit_framelist( T_DOWNLOAD_MAIN_WORK * wk )
{
	// 破棄
	if( wk->frame_list_work ){
		FRAMELIST_Exit( wk->frame_list_work );
		wk->frame_list_work = 0;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	リストカーソル位置再計算
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_calc_framelist( T_DOWNLOAD_MAIN_WORK * wk )
{
	// 初期化
	if(wk->frame_list_work){
		wk->AllCurPos[T_DOWNLOAD_LIST_CURRENT]      = FRAMELIST_GetListPos(  wk->frame_list_work );		// 通しカーソル位置
		wk->ListCurPos[T_DOWNLOAD_LIST_CURRENT]     = FRAMELIST_GetCursorPos(wk->frame_list_work );		// 画面上の位置
		wk->ListScrollPos[T_DOWNLOAD_LIST_CURRENT]  = FRAMELIST_GetScrollCount(wk->frame_list_work );	// スクロール位置
		//
		// 初期化の時に、表示しているフレーム数
		// putFrameMaxMain(frame_list.c)を設定している
		//
		// 通常の使い方(上を空けない)場合は、
		// 基本8になるはずなので特に問題はない(例：図鑑)
		// 上を空ける場合は、
		// 初期化時に状況によっては変化する可能性がある
		// (pos、scrollなど)そのため
		// 操作手順によっては表示箇所がずれることがある
		//
		// ただし、他でも使用している箇所であり、
		// また、これで完全に直るのかがわからないため、
		// 外部から値をセットする方法で様子を見る
		FRAMELIST_SetFrameMax( wk->frame_list_work, 7 );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームのcallback関数(draw,move,scroll)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
// draw
static void td_listcallback_draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
	T_DOWNLOAD_MAIN_WORK * wk = (T_DOWNLOAD_MAIN_WORK *)work;
	// タイトル表示
	td_liststr_print( wk, pos, print_util, wk->msg[0] );
}

// move
static void td_listcallback_move( void *work, u32 pos, BOOL flag )
{
	T_DOWNLOAD_MAIN_WORK * wk = (T_DOWNLOAD_MAIN_WORK *)work;
	// 再計算(カーソルをobjで処理する際は必要)
	TdCmn_calc_framelist( wk );
	wk->CmnFlag |= T_DOWNLOAD_FLG_UPDATE;
}

// scroll
static void td_listcallback_scroll( void *work, s8 mv )
{
	T_DOWNLOAD_MAIN_WORK * wk = (T_DOWNLOAD_MAIN_WORK *)work;
	// 再計算
//	TdCmn_chk_newmark( wk, 0 );
//	TdCmn_scroll_newmark( wk, mv );
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
static void td_liststr_print( T_DOWNLOAD_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg )
{
	PRINT_QUE *que = FRAMELIST_GetPrintQue( wk->frame_list_work );
	int param = FRAMELIST_GetItemParam( wk->frame_list_work, pos );
	//
	td_strbuf_print( wk, print_util, que, msg, param, pos );
}

static void td_PrintStrBuf( T_DOWNLOAD_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int strIdx, PRINTSYS_LSB col, int y, int pos )
{
	STRBUF*		strbuf;
	STRBUF*		strbuf2;
	STRBUF*		expbuf;
	const WBTDL_DATA*	wbtl;
	
	// アドレスセット
//	wbtl = &wk->Param->data->data[pos];
	wbtl = wk->CheckBuffer[pos];
	
	// トーナメント名
	strbuf  = GFL_STR_CreateBuffer( 128, wk->heapID );
	expbuf  = GFL_STR_CreateBuffer( 128, wk->heapID );
	// セーブデータを取得
	TdSave_get_str( wk, wbtl, T_DOWNLOAD_SAVE_STR_NAME, strbuf );
	WORDSET_RegisterWord( wk->Wordset, 0, strbuf, 0, TRUE, PM_LANG );
	strbuf2 = GFL_MSG_CreateString( wk->msg[0], td_mes_u00_tbl[0] );
	WORDSET_ExpandStr( wk->Wordset, expbuf, strbuf2 );
	PRINT_UTIL_PrintColor( print_util, que, 0, y, expbuf, wk->font, col );
	GFL_STR_DeleteBuffer( strbuf );
	GFL_STR_DeleteBuffer( strbuf2 );
	GFL_STR_DeleteBuffer( expbuf );
}

static void td_strbuf_print( T_DOWNLOAD_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int param, int pos )
{
	// 情報表示
	td_PrintStrBuf( wk, print_util, que, msg, param, PRINTSYS_MACRO_LSB(14,15,0), 4, pos );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	リストカーソル表示(通常とフレームリストで違う)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 * @param	const s32					(0:フレームリスト用、1:3枚用)
 * @param	const s32					(番号)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_disp_panel_cursor( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 num)
{
	GFL_CLACTPOS	pos;
	//
	if(mode == T_DOWNLOAD_PANEL_FRAME){
		pos.x = TdCmn_get_empty( wk, (EMPTY_CURPOS+num)*2 );
		pos.y = TdCmn_get_empty( wk, (EMPTY_CURPOS+num)*2+1 );
	}else{
		pos.x = TdCmn_get_empty( wk, (EMPTY_CURPOS2+num)*2 );
		pos.y = TdCmn_get_empty( wk, (EMPTY_CURPOS2+num)*2+1 );
	}
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_CURSOR], &pos, CLSYS_DEFREND_MAIN );
	// 戻し
	if(!(wk->CmnFlag & T_DOWNLOAD_FLG_CUR_ANIM)){
		TdCmn_chg_objseq( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, OBJ_CUR_SEQ );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	パネル表示(通常とフレームリストのカーソル共通)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 * @param	const s32					(0:通常、1:決定)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_set_panel_cursor( T_DOWNLOAD_MAIN_WORK * wk, const s32 smode )
{
	s32		seq;
	
	wk->CmnFlag &= ~T_DOWNLOAD_FLG_CUR_ANIM;
	seq = OBJ_CUR_SEQ;
	if(smode == 1){
		seq = OBJ_CUR_ON_SEQ;
		wk->CmnFlag |= T_DOWNLOAD_FLG_CUR_ANIM;
	}
	TdCmn_chg_objseq( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, seq );
}

//----------------------------------------------------------------------------------
/**
 * @brief	パレットチェンジ初期化
 *
 * @param	T_DOWNLOAD_MAIN_WORK*	wk
 *
 * @retval
 */
//----------------------------------------------------------------------------------
static void td_init_chgpal( T_DOWNLOAD_MAIN_WORK* wk )
{
	// パレット取得
	NNSG2dPaletteData* pal_data;
	void* buf = GFL_ARC_UTIL_LoadPalette( ARCID_T_DOWNLOAD_GRA, NARC_t_download_gra_t_download_obj_d_NCLR, &pal_data, wk->heapID );
	u16* raw_data = pal_data->pRawData;
	//
	// パレットをコピー
	GFL_STD_MemCopy( &raw_data[0x30], wk->PBufStart, 0x20 );		// 3番
	GFL_STD_MemCopy( &raw_data[0x40], wk->PBufEnd,   0x20 );		// 4番
	// 破棄
	GFL_HEAP_FreeMemory( buf );
	//
	wk->PCount = 0;
	//
	wk->CmnFlag |= T_DOWNLOAD_FLG_PALCHG;
}

//----------------------------------------------------------------------------------
/**
 * @brief	パレットチェンジ(main)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*	wk
 *
 * @retval
 */
//----------------------------------------------------------------------------------
void TdCmn_main_chgpal( T_DOWNLOAD_MAIN_WORK* wk )
{
	u8			cbuf[10];
	s32			i;
	s32			rate;
	fx16		cos;
	
	// 念のため
	if(!(wk->CmnFlag & T_DOWNLOAD_FLG_PALCHG)){
		return;
	}
	
	// 速度
	rate = 0x400;
	
	// 加算(0 .. 1.0)にするようだ
	wk->PCount += rate;
	if(wk->PCount >= 0x10000){
		wk->PCount -= 0x10000;
	}
	cos = ( FX_CosIdx(wk->PCount) + FX16_ONE )/2;
	//
	// カラーの計算
	// (start -> end)への補正で(2)に書き込む
	for(i=0;i<16;i++){
		// 開始
		cbuf[0] = ( wk->PBufStart[i] & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
		cbuf[1] = ( wk->PBufStart[i] & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
		cbuf[2] = ( wk->PBufStart[i] & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;
		// 終了
		cbuf[3] = ( wk->PBufEnd[i]   & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
		cbuf[4] = ( wk->PBufEnd[i]   & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
		cbuf[5] = ( wk->PBufEnd[i]   & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;
		// RGBを求める
		cbuf[6] = cbuf[0] + (((cbuf[3]-cbuf[0])*cos)>>FX16_SHIFT);
		cbuf[7] = cbuf[1] + (((cbuf[4]-cbuf[1])*cos)>>FX16_SHIFT);
		cbuf[8] = cbuf[2] + (((cbuf[5]-cbuf[2])*cos)>>FX16_SHIFT);
		//
		wk->PBufNow[i] = GX_RGB( cbuf[6], cbuf[7], cbuf[8] );
	}
	// Main(4番に転送決めうち)
	NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_OBJ_PLTT_MAIN , 0x20*4, wk->PBufNow, 0x20 );
}

//----------------------------------------------------------------------------------
/**
 * @brief	パレットチェンジ(main)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*	wk
 *
 * @retval
 */
//----------------------------------------------------------------------------------
void TdCmn_set_chgpal( T_DOWNLOAD_MAIN_WORK* wk, const BOOL bflg )
{
	// フラグのon/off
	if(bflg == TRUE){
		wk->CmnFlag |= T_DOWNLOAD_FLG_PALCHG;
	}else{
		wk->CmnFlag &= ~T_DOWNLOAD_FLG_PALCHG;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  カーソル位置修正(Info)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *	@param	const s32			(0-2)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_chg_info_curpos( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s32		i;
	
	// パレットアニメなので
	// シーケンスを切り替えても変にはならないはず
	// 一旦全部off
	for(i=0;i<3;i++){
		TdCmn_chg_objseq( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_ICON_00+i, td_info_btn_tbl[2*i] );
	}
	// 該当をon
	TdCmn_chg_objseq( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_ICON_00+mode, td_info_btn_tbl[2*mode+1] );
	//
	// パレットの点滅を初期化
	wk->PCount = 0;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクロールバー表示チェック
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *	@param	const s32			(0-2)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_chk_info_disp_scrollbar( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	BOOL	bflg=TRUE;
	// 1ページしかない
	// =>スクロールバーを非表示とする
	if(wk->PageMaxNum[mode] == 1){
		bflg = FALSE;
	}
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_BAR, bflg );
	// 矢印も含む
	// 上下矢印非表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_UP,   bflg );
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_CMN, T_DOWNLOAD_CLWKID_CMN_DOWN, bflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	スクロールバー処理
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_main_scrollbar( T_DOWNLOAD_MAIN_WORK * wk )
{
	// 表示
	GFL_CLACTPOS cp_pos;
	
	// 座標の取得
	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_BAR], &cp_pos, CLSYS_DEFREND_MAIN );
	cp_pos.y = FRAMELIST_GetScrollBarPY2( wk->frame_list_work, cp_pos.y );
	//
	// RAILの領域の(+-8補正:アイコンの半分の大きさ補正)
	if( cp_pos.y < 40 ){
		cp_pos.y = 40;
	}else if( cp_pos.y > 152 ){
		cp_pos.y = 152;
	}
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_BAR], &cp_pos, CLSYS_DEFREND_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	詳細情報表示
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 * @param	const s32					(0:セーブ、1:リスト)
 * @param	const s32					(番号)
 * @param	const s32					(種類)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_disp_info_mode( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 num, const s32 snum )
{
	s32		i;
	s32		j;
	s32		flg=0;
	s32		buf[2];
	s32		pnum;
	s32		offset;
	s32		id;
	STRBUF	*exp;
	STRBUF	*strbuf;
	const WBTDL_DATA* wbtdl;
	
	// アドレスをセット
	if(mode == T_DOWNLOAD_DATA_SAVE){
		wbtdl = wk->WbtSaveData[num];
	}else{
//		wbtdl = &wk->Param->data->data[num];
		wbtdl = wk->CheckBuffer[num];
	}
	
	//
	// 1画面をクリアする
	for(i=0;i<CANVAS_INFO_NUM;i++){
		TdCmn_clear_canvas( wk, CANVAS_INFO_OFFSET+i );
	}
	
	// 最大ページ数になっている
	// =>途中までの表示となる
	pnum = 10;
	if(wk->CurrentPageNum[snum] == (wk->PageMaxNum[snum]-1)){
		buf[0] = wk->ItemNum[snum]%10;		// 表示数
		if(buf[0] == 0){
			// 項目数が0の場合
			if(wk->ItemNum[snum] == 0){
				flg = 1;
			}
			pnum = 10;
		}else{
			pnum = buf[0];
		}
	}
	offset = wk->CurrentPageNum[snum]*10;
	//
	// 1回だけでよいはず
	exp = GFL_STR_CreateBuffer( 128, wk->heapID );
	// 各情報を表示
	// (0:ポケモン、1:道具、2:技)
	j=0;
	for(i=0;i<CANVAS_INFO_NUM;i++){
		//----
		// 左
		//----
		// 項目がない場合
		if(flg == 1){
			strbuf = GFL_MSG_CreateString( wk->msg[0], td_mes_info_tbl[3] );
			TdCmn_disp_string( wk, T_DOWNLOAD_MSG_DOWN_INFO_00+i, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(14,15,0));
			GFL_STR_DeleteBuffer( strbuf );
			break;
		}
		// 通常の場合
		strbuf = GFL_MSG_CreateString( wk->msg[0], td_mes_info_tbl[snum] );
		if(snum == 0){			// ポケモン
			// 指定した番号のidを返す
			id = TdSave_get_item_id( wk, wbtdl, T_DOWNLOAD_SAVE_ITEM_POKE, 1+offset+j );
			WORDSET_RegisterPokeMonsNameNo( wk->Wordset, 0, id );
		}else if(snum == 1){	// 道具
			id = TdSave_get_item_id( wk, wbtdl, T_DOWNLOAD_SAVE_ITEM_ITEM, 1+offset+j );
			WORDSET_RegisterItemName( wk->Wordset, 0, id );
		}else{					// 技
			id = TdSave_get_item_id( wk, wbtdl, T_DOWNLOAD_SAVE_ITEM_SKILL, 1+offset+j );
			WORDSET_RegisterWazaName( wk->Wordset, 0, id );
		}
		WORDSET_ExpandStr( wk->Wordset, exp, strbuf );
		TdCmn_disp_string( wk, T_DOWNLOAD_MSG_DOWN_INFO_00+i, 0, 0, exp, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(14,15,0));
		GFL_STR_DeleteBuffer( strbuf );
		j++;
		if(j >= pnum){
			break;
		}
		//----
		// 右
		//----
		strbuf = GFL_MSG_CreateString( wk->msg[0], td_mes_info_tbl[snum] );
		if(snum == 0){			// ポケモン
			id = TdSave_get_item_id( wk, wbtdl, T_DOWNLOAD_SAVE_ITEM_POKE, 1+offset+j );
			WORDSET_RegisterPokeMonsNameNo( wk->Wordset, 0, id );
		}else if(snum == 1){	// 道具
			id = TdSave_get_item_id( wk, wbtdl, T_DOWNLOAD_SAVE_ITEM_ITEM, 1+offset+j );
			WORDSET_RegisterItemName( wk->Wordset, 0, id );
		}else{					// 技
			id = TdSave_get_item_id( wk, wbtdl, T_DOWNLOAD_SAVE_ITEM_SKILL, 1+offset+j );
			WORDSET_RegisterWazaName( wk->Wordset, 0, id );
		}
		WORDSET_ExpandStr( wk->Wordset, exp, strbuf );
		TdCmn_disp_string( wk, T_DOWNLOAD_MSG_DOWN_INFO_00+i, 104, 0, exp, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(14,15,0));
		GFL_STR_DeleteBuffer( strbuf );
		j++;
		if(j >= pnum){
			break;
		}
	}
	GFL_STR_DeleteBuffer( exp );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	詳細情報初期化(判定が必要)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 * @param	const s32					(0:セーブ、1:リスト)
 * @param	const s32					(番号)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_init_info_mode( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 num )
{
	s32		i;
	s32		buf[2];
	const WBTDL_DATA* wbtdl;
	
	// アドレスをセット
	if(mode == T_DOWNLOAD_DATA_SAVE){
		wbtdl = wk->WbtSaveData[num];
	}else{
//		wbtdl = &wk->Param->data->data[num];
		wbtdl = wk->CheckBuffer[num];
	}
	// アイテム数
	wk->ItemNum[0] = TdSave_get_item_num( wk, wbtdl, T_DOWNLOAD_SAVE_ITEM_POKE );
	wk->ItemNum[1] = TdSave_get_item_num( wk, wbtdl, T_DOWNLOAD_SAVE_ITEM_ITEM );
	wk->ItemNum[2] = TdSave_get_item_num( wk, wbtdl, T_DOWNLOAD_SAVE_ITEM_SKILL );
	//
	for(i=0;i<T_DOWNLOAD_INFO_MAX;i++){
		// 0の場合は1ページとしておく
		if(wk->ItemNum[i] == 0){
			wk->PageMaxNum[i] = 1;
		}else{
			buf[0] = wk->ItemNum[i]/10;		// ページ数
			buf[1] = wk->ItemNum[i]%10;		// 端数用
			if(buf[1] != 0){
				buf[0]++;
			}
			// 最大ページ数
			wk->PageMaxNum[i] = buf[0];
		}
	}
	//
	// 初期位置セット
	for(i=0;i<T_DOWNLOAD_INFO_MAX;i++){
		wk->CurrentPageNum[i] = 0;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーの処理
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(番号)
 *
 *	@retval const s32				(カーソル位置)
 */
//-----------------------------------------------------------------------------
s32 TdCmn_calc_scrollbar_info( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s32				num;
	u32				cnt=-1;
	u32				buf[2];
	//
	// タッチした座標を取得
	if( GFL_UI_TP_GetPointCont( &buf[0], &buf[1] ) == TRUE ){
		// 段階が求まる
		num = wk->PageMaxNum[mode];
		cnt = SCROLLBAR_GetCount( num-1, buf[1], SCRL_TOP_Y, SCRL_DOWN_Y, SCRL_BAR_SIZE );
	}
	return(cnt);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーの処理
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(番号)
 *	@param	const s32				(段階)
 *	@param	const s32				(0:段階の座標にセット、1:フリーの座標にセット)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL TdCmn_set_scrollbar_info( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 num, const s32 smode )
{
	GFL_CLACTPOS 	cp_pos;
	s32				tmp;
	u32				buf[2];
	BOOL			bflg=FALSE;
	
	// 座標の取得
	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_BAR], &cp_pos, CLSYS_DEFREND_MAIN );
	//
	// 段階の座標にセット
	if(smode == 0){
		// 表示座標を求める
		cp_pos.y = SCROLLBAR_GetPosY( wk->PageMaxNum[mode]-1, num, SCRL_TOP_Y, SCRL_DOWN_Y, SCRL_BAR_SIZE );
	}
	// フリーの座標にセット
	else{
		// タッチしている
		if( GFL_UI_TP_GetPointCont( &buf[0], &buf[1] ) == TRUE ){
			//
			// 距離の差分を求める
			tmp = cp_pos.y-buf[1];
			if(tmp < 0){
				tmp = -tmp;
			}
			// 一定距離あり、上下端でなければ鳴らす
			if(tmp > SCRL_WAIT_LEN){
				if((cp_pos.y != 40)&&(cp_pos.y != 152)){
					bflg = TRUE;
				}
			}
			cp_pos.y = buf[1];
		}
	}
	//
	// 念のため補正
	if( cp_pos.y < 40 ){
		cp_pos.y = 40;
	}else if( cp_pos.y > 152 ){
		cp_pos.y = 152;
	}
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_BAR], &cp_pos, CLSYS_DEFREND_MAIN );
	//
	return( bflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	タイトル表示
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 * @param	const s32					(種類)
 * @param	const s32					(種類)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_disp_title_info( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 smode )
{
	s32		id;
	STRBUF	*strbuf;
	//
	// クリア
	TdCmn_clear_canvas( wk, T_DOWNLOAD_MSG_DOWN_TITLE );
	//
	wk->PFlag |= (1<<T_DOWNLOAD_MSG_DOWN_TITLE);
	//
	id = 0;
	if(mode == T_DOWNLOAD_MODE_SELECT){
		id = 1;
	}else if(mode == T_DOWNLOAD_MODE_INFO){
		id = 2+smode;
	}
	strbuf = GFL_MSG_CreateString( wk->msg[0], td_mes_title_tbl[id] );
	TdCmn_disp_string( wk, T_DOWNLOAD_MSG_DOWN_TITLE, 0, 4, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン処理
 *
 *	@param	T_DOWNLOAD_MAIN_WORK*		wk
 *	@param	const s32				モード(0:trg、1:cont)
 *	@param	const s32				モード(0-3)
 *
 *	@retval s32 					領域の番号
 */
//-----------------------------------------------------------------------------
s32 TdCmn_chk_tp( T_DOWNLOAD_MAIN_WORK *wk, const s32 tpmode, const s32 mode )
{
	s32	id = GFL_UI_TP_HIT_NONE;
	//
	// タッチペン判定
	if(tpmode == 0){
		id = GFL_UI_TP_HitTrg( td_rect_tbl[mode] );
	}else{
		id = GFL_UI_TP_HitCont( td_rect_tbl[mode] );
	}
	// 該当する箇所があった=>モード切替
	if(id != GFL_UI_TP_HIT_NONE){
		GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
	}
	return( id );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  輝度の変更(要検討)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const BOOL			(TRUE:暗い、FALSE:戻し)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_set_passive( T_DOWNLOAD_MAIN_WORK* wk, const BOOL bflg )
{
	s32		rate;
	
	rate = TD_ACTIVE_RATE;
	if(bflg == TRUE){
		rate = TD_PASSIVE_RATE;
	}
	G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1 |GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, rate );
	
	// パレット変更(caption)
	td_chg_fontpal( wk, bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  輝度の変更(要検討)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const BOOL			(TRUE:暗い、FALSE:戻し)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_set_passive_direct( T_DOWNLOAD_MAIN_WORK* wk, const BOOL bflg )
{
	s32		rate;
	s32		off;
	
	rate = TD_ACTIVE_RATE;
	if(bflg == TRUE){
		rate = TD_PASSIVE_RATE;
	}
	G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1 |GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, rate );
	
	// パレット変更(caption)(直接転送)
	off = 0;
	if(bflg == TRUE){
		off = 0x10;
	}
	GX_LoadBGPltt( &wk->FontPal[off], 0x20*7, 0x20 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字送りカーソルの処理
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_main_keycursor( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	if( wk->PrintUtil[T_DOWNLOAD_MSG_DOWN_MESWIN_01].win ){
		if(mode == 0){		// 一発だし
			APP_KEYCURSOR_MainNotStream( wk->MsgCursorWork, wk->PrintUtil[T_DOWNLOAD_MSG_DOWN_MESWIN_01].win );
		}else{				// stream
			APP_KEYCURSOR_Main( wk->MsgCursorWork, wk->PrintStream, wk->PrintUtil[T_DOWNLOAD_MSG_DOWN_MESWIN_01].win );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメのスピード切り替え(決めうち)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const BOOL			(TRUE:とめる、FALSE:戻す)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_stop_anim_speed( T_DOWNLOAD_MAIN_WORK *wk, const BOOL bflg )
{
	BOOL	eflg;
	
	// スクロールカーソル
	// 表示中 => アニメをとめる
	eflg = GFL_CLACT_WK_GetDrawEnable( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_BAR] );
	if(eflg == TRUE){
		if(bflg == TRUE){		// 止める
			GFL_CLACT_WK_StopAnm( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_BAR] );
		}else{					// 再開
			GFL_CLACT_WK_StartAnm( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_BAR] );
		}
	}
	// カーソル
	eflg = GFL_CLACT_WK_GetDrawEnable( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_CURSOR] );
	if(eflg == TRUE){
		if(bflg == TRUE){		// 止める
			GFL_CLACT_WK_StopAnm( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_CURSOR] );
		}else{					// 再開
			GFL_CLACT_WK_StartAnm( wk->wk_obj.clwk[T_DOWNLOAD_CLWKID_CURSOR] );
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	カーソル表示(パネル3用)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 * @param	const s32					(0:セーブ、1:リスト)
 * @param	const s32					(番号)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_disp_panel_string( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 num )
{
	STRBUF* strbuf;
	STRBUF* strbuf2;
	STRBUF* expbuf;
	const WBTDL_DATA	*wbtl;
	
	// セーブデータ
	if(mode == T_DOWNLOAD_DATA_SAVE){
		wbtl = wk->WbtSaveData[num];
	}
	// リストデータ(要修正)
	else{
//		wbtl = &wk->Param->data->data[num];
		wbtl = wk->CheckBuffer[num];
	}
	
	// いったんクリア
	TdCmn_clear_canvas( wk, CANVAS_JOIN_OFFSET+num );
	
	// トーナメント名
	strbuf = GFL_STR_CreateBuffer( 128, wk->heapID );
	expbuf = GFL_STR_CreateBuffer( 128, wk->heapID );
	// セーブデータを取得
	TdSave_get_str( wk, wbtl, T_DOWNLOAD_SAVE_STR_NAME, strbuf );
	WORDSET_RegisterWord( wk->Wordset, 0, strbuf, 0, TRUE, PM_LANG );
	strbuf2 = GFL_MSG_CreateString( wk->msg[0], td_mes_u00_tbl[0] );
	WORDSET_ExpandStr( wk->Wordset, expbuf, strbuf2 );
	TdCmn_disp_string( wk, CANVAS_JOIN_OFFSET+num, 0, 4, expbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(14,15,2));
	GFL_STR_DeleteBuffer( strbuf );
	GFL_STR_DeleteBuffer( strbuf2 );
	GFL_STR_DeleteBuffer( expbuf );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	上画面表示判定
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 * @param	const BOOL					(TRUE:表示、FALSE:非表示)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void td_set_up_visible( T_DOWNLOAD_MAIN_WORK * wk, const BOOL bflg )
{
	u8	vflg = VISIBLE_ON;
	
	// BG面のon/offで大丈夫だろう
	if(bflg == FALSE){
		vflg = VISIBLE_OFF;
	}
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, vflg );
	GFL_BG_SetVisible( GFL_BG_FRAME2_S, vflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	上画面表示判定(判定込み)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void TdCmn_chk_up_disp( T_DOWNLOAD_MAIN_WORK * wk, const BOOL keyflg, const BOOL tpflg, const BOOL fflg )
{
	BOOL	vflg;
	
	// 判定して表示のみ
	if((keyflg == FALSE)&&(tpflg == FALSE)){
		// タッチモード
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH ){
			vflg = FALSE;
		}
		// キーモード
		else if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_KEY ){
			vflg = TRUE;
		}
	}
	//
	// タッチを押した
	if(tpflg == TRUE){
		GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
		vflg = FALSE;
	}
	// キーを押した
	else if(keyflg == TRUE){
		GFL_UI_SetTouchOrKey( GFL_APP_KTST_KEY );
		vflg = TRUE;
	}
	// 強制
	if(fflg == TRUE){
		vflg = TRUE;
	}
	//
	// 上画面表示
	td_set_up_visible( wk, vflg );
	// カーソル表示
	TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, vflg );
}

//----------------------------------------------------------------------------------
/**
 * @brief	パレットチェンジ(caption用)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*	wk
 *
 * @retval
 */
//----------------------------------------------------------------------------------
static void td_init_fontpal( T_DOWNLOAD_MAIN_WORK* wk )
{
	// BGの枚数の都合上、
	// 輝度の調整では対応できなかったので、
	// パレットを変更することにより対応した
	//
	// パレット取得
	NNSG2dPaletteData* pal_data;
	void* buf = GFL_ARC_UTIL_LoadPalette( ARCID_T_DOWNLOAD_GRA, NARC_t_download_gra_t_download_font_NCLR, &pal_data, wk->heapID );
	u16* raw_data = pal_data->pRawData;
	// パレットをコピー
	GFL_STD_MemCopy( &raw_data[0], wk->FontPal, 0x40 );		// 0番(2本分)
	// 破棄
	GFL_HEAP_FreeMemory( buf );
}

//----------------------------------------------------------------------------------
/**
 * @brief	パレットチェンジ(caption用)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*	wk
 * @param	const BOOL				(TRUE:暗い、FALSE:戻し)
 *
 * @retval
 */
//----------------------------------------------------------------------------------
static void td_chg_fontpal( T_DOWNLOAD_MAIN_WORK* wk, const BOOL bflg )
{
	s32		off;
	
	off = 0;
	if(bflg == TRUE){
		off = 0x10;
	}
	// Main(7番に転送決めうち)
	NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN , 0x20*7, &wk->FontPal[off], 0x20 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  マークの判定
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdCmn_chk_mark_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mmin, const s32 mmax, const s32 nnum )
{
	// 左右ボタン
	if(mode == 0){
		// 念のため
		if(wk->CmnFlag & (T_DOWNLOAD_FLG_ANM_LEFT|T_DOWNLOAD_FLG_ANM_RIGHT)){
			return;
		}
		// 最小チェック
		if((mmin >= nnum)){
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_LEFT, 1 );
		}else{
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_LEFT,  -1 );
		}
		//
		// 最大チェック
		if((nnum >= mmax)){
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RIGHT, 1 );
		}else{
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_RIGHT, -1 );
		}
	}
	else if(mode == 1){
		// 念のため
		if(wk->CmnFlag & (T_DOWNLOAD_FLG_ANM_UP|T_DOWNLOAD_FLG_ANM_DOWN)){
			return;
		}
		// 最小チェック
		if((mmin >= nnum)){
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_UP, 1 );
		}else{
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_UP,  -1 );
		}
		//
		// 最大チェック
		if((nnum >= mmax)){
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_DOWN, 1 );
		}else{
			TdCmn_set_cmn_anim( wk, T_DOWNLOAD_CLWKID_CMN_DOWN, -1 );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメの終了判定
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(0:とび先終了判定あり、1:アニメ終了判定のみ)
 *
 *	@retval BOOL					(TRUE:終了、FALSE:なし)
 */
//-----------------------------------------------------------------------------
BOOL TdCmn_chk_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s16		flg=0;
	s16		wnum=0;
	BOOL	bflg=FALSE;
	BOOL	rflg=FALSE;
	
	// 左アニメ再生中
	if(wk->CmnFlag & T_DOWNLOAD_FLG_ANM_LEFT){
		wnum = T_DOWNLOAD_CLWKID_CMN_LEFT;
		flg = 1;
	}
	// 右アニメ再生中
	else if(wk->CmnFlag & T_DOWNLOAD_FLG_ANM_RIGHT){
		wnum = T_DOWNLOAD_CLWKID_CMN_RIGHT;
		flg = 1;
	}
	// ×アニメ再生中
	else if(wk->CmnFlag & T_DOWNLOAD_FLG_ANM_CROSS){
		wnum = T_DOWNLOAD_CLWKID_CMN_CROSS;
		flg = 1;
	}
	// 戻るアニメ再生中
	else if(wk->CmnFlag & T_DOWNLOAD_FLG_ANM_RETURN){
		wnum = T_DOWNLOAD_CLWKID_CMN_RETURN;
		flg = 1;
	}
	// 下アニメ再生中
	else if(wk->CmnFlag & T_DOWNLOAD_FLG_ANM_DOWN){
		wnum = T_DOWNLOAD_CLWKID_CMN_DOWN;
		flg = 1;
	}
	// 上アニメ再生中
	else if(wk->CmnFlag & T_DOWNLOAD_FLG_ANM_UP){
		wnum = T_DOWNLOAD_CLWKID_CMN_UP;
		flg = 1;
	}

	// アニメ終了判定を行う
	if(flg == 1){
		bflg = TdCmn_is_anime_end( wk, T_DOWNLOAD_CLWKMODE_CMN, wnum );
		if(bflg == FALSE){
			flg = 2;
			// アニメの終了
			// 通常に戻しておく
			// =>最終で待つでもよいはず
			wk->CmnFlag &= ~(1<<wnum);
			//
			// 終了判定ありの場合
			if(mode == 0){
				// 戻るボタン(飛び先変更はある)
				if((wnum == T_DOWNLOAD_CLWKID_CMN_CROSS)||(wnum == T_DOWNLOAD_CLWKID_CMN_RETURN)){
					// ×ボタン終了
					if(wnum == T_DOWNLOAD_CLWKID_CMN_CROSS){
						wk->CmnFlag |= T_DOWNLOAD_FLG_CROSS_END;
					}
					// RETURNボタン終了
					else if(wnum == T_DOWNLOAD_CLWKID_CMN_RETURN){
						wk->CmnFlag |= T_DOWNLOAD_FLG_RET_END;
					}
				}
			}
			rflg = TRUE;
		}
	}
	//
	return( rflg );
}

//----------------------------------------------------------------------------------
/**
 * @brief	カーソル位置情報のon/off(消したときの対応)
 *
 * @param	T_DOWNLOAD_MAIN_WORK*	wk
 * @param	const BOOL				(TRUE:消す、FALSE:戻す)
 * @param	const s32				(番号:0-2)
 *
 * @retval
 */
//----------------------------------------------------------------------------------
void TdCmn_set_del_save( T_DOWNLOAD_MAIN_WORK* wk, const BOOL bflg, const s32 num )
{
	// 消す
	if(bflg == TRUE){
		// 上画面表示
		td_set_up_visible( wk, FALSE );
		// いったんクリア
		TdCmn_clear_canvas( wk, CANVAS_JOIN_OFFSET+num );
		// パネル表示
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_PANEL_00+num, FALSE );
		// カーソル非表示
		TdCmn_set_obj_visible( wk, T_DOWNLOAD_CLWKMODE_NORMAL, T_DOWNLOAD_CLWKID_CURSOR, FALSE );
	}
	// 戻し
	else{
		// 上画面表示
		td_set_up_visible( wk, TRUE );
		//
		// 文字列、パネルは復帰時に別で処理してくれる
	}
}

