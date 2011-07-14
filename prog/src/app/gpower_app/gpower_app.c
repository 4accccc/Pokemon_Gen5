//=============================================================================
/**
 *
 *	@file	gpower_app.c
 *	@brief  デルパワー使用画面
 *	@author
 *	@date
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
#include "gpower_app_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"
// print
#include "ui/print_tool.h"
#include "gamesystem/msgspeed.h"
#include "gamesystem/g_power.h"

//アプリ共通素材
#include "app/app_menu_common.h"
#include "app/app_taskmenu.h"

// サウンド
#include "sound/pm_sndsys.h"
#include "gpower_app_snd_def.h"

//アーカイブ
#include "arc_def.h"

#include "system/wipe.h"

#include "app/app_keycursor.h"
#include "app/app_printsys_common.h"

#include "app/gpower_app.h"
#include "gpower_app_gra.naix"

// メッセージ
#include "message.naix"
#include "msg/msg_monolith.h"

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
	STEP_INIT	= 0,					// 初期化
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

//--------------
// パレット
//--------------
// (sub)
// 上：BG  (0-2:BG)
// 上：OBJ (0:キャラ)
// (main)
// 下：BG  (0-2:BG)、(6:cmn)、(8:font)、(10:yesno)
// 下：OBJ (8-11:cmn)
enum{
	// (main bg)
	PLTID_BG_BASE				= 0,
	PLTID_BG_CMN				= 6,
	PLTID_BG_FONT				= 8,
	PLTID_BG_YESNO				= 10,
	// (sub bg)
	PLTID_SUB_BG_BASE			= 0,
	PLTID_SUB_BG_FONT			= 8,
	//
	// (main obj)
	PLTID_OBJ_FONT				= 4,
	PLTID_OBJ_CMN				= 8,		// cmnのpalのoffset
	PLTID_OBJ_CMN_NUM			= 4,		// cmnのpalの本数
	//
	// 何番目か
	MES_CLEAR_COLOR				= 0,		// クリアカラー
};

// 最大値
enum{
	GPOWER_APP_CANVAS_MAXNUM 	= 22,		// canvas
	GPOWER_APP_EMPTY_MAXNUM		= 9*2,		// empty
	//
	GPOWER_APP_CMN_OBJ_MAXNUM	= 1,		// common(obj)
	GPOWER_APP_OBJ_MAXNUM		= 4,		// obj
	GPOWER_APP_BMPOAM_MAXNUM	= 4,		// bmpoam
	//
	GPOWER_APP_BGNUM_MAXNUM		= 8,		// bg
	//
	GPOWER_APP_MES_MAXNUM		= 2,		// message
	//
	GPOWER_APP_POWER_MAXNUM		= 10,		// 発動数最大
};

//-----------------------------
// OBJのリソースのインデックス
//-----------------------------
enum{
	// common
	RESID_CMN_OBJ_PLT = 0,					// パレット
	RESID_CMN_OBJ_CHR,						// キャラ
	RESID_CMN_OBJ_CEL,						// セルアニメ
	// 通常
	RESID_OBJ_PLT,							// パレット
	RESID_OBJ_CHR,							// キャラ
	RESID_OBJ_CEL,							// セルアニメ
	//
	RESID_FONT_PLT,							// パレット
	//
	RESID_OBJ_MAX,
};

//----------
// clwork
//----------
enum{
	CLWKMODE_CMN = 0,						// 共通(cmn)
	CLWKMODE_NORMAL,						// 通常
};

//----------
// cmn
//----------
// CLWK(cmn)
enum {
	CLWKID_CMN_RETURN = 0,					// 戻る
};

// シーケンス(cmn)
enum{
	CMN_RETURN_SEQ 		= 1,				// 戻る
	CMN_RETURN_ON_SEQ	= 9,				// 戻るオン
};

//----------
// normal
//----------
// CLWK
enum {
	CLWKID_POWER_00 = 0,					// パネル00
	CLWKID_POWER_01,						// パネル01
	CLWKID_POWER_02,						// パネル02
	CLWKID_HILINK,							// パネル03(ハイリンク)
};

// シーケンス
enum{
	SEQ_POWER_ON 	= 0,					// デルパワーパネル(on)
	SEQ_HILINK_ON 	= 1,					// ハイリンクパネル(on)
	SEQ_POWER_OFF 	= 2,					// デルパワーパネル(off)
	SEQ_HILINK_OFF 	= 3,					// ハイリンクパネル(off)
};

//-------------------
// canvas
//-------------------
enum{
	CVS_UP_TITLE		= 0,				// 上：タイトル
	CVS_UP_POWER_00		= 1,				// 上：項目(10)
	CVS_UP_POWER_NUM_00 = 11,				// 上：項目数字(10)
	//
	CVS_DOWN_WIN		= 21,				// 下：win
	//
	CVS_DOWN_POWER_00 	= 21,				// 下：パネル(0-2)
	CVS_DOWN_HILINK		= 24,				// 下：ハイリンク
};

//-------------------
// フラグ
//-------------------
// OBJのフラグ(GPOWER_APP_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM		= (1<<0),			// アニメあり
	FLG_OBJ_HFLIP		= (1<<1),			// HFLIP
	FLG_OBJ_VFLIP		= (1<<2),			// VFLIP
};

// GPOWER_APP_MAIN_WORK.Flag
enum{
	FLG_MW_UPDATE		= (1<<0),			// 更新
	FLG_MW_DISP_CURSOR	= (1<<1),			// カーソル表示
	FLG_MW_NO_SELECT	= (1<<2),			// ハイリンク選択不可
	FLG_MW_BATTERY_ERR	= (1<<3),			// バッテリーエラー
	//
	FLG_MW_PALCHG_ON	= (1<<4),			// パレットチェンジ中
	FLG_MW_DECIDE_ANIM	= (1<<5),			// 決定アニメ
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
	u32			res[RESID_OBJ_MAX]; 					// リソース
	// 共通
	GFL_CLWK	*clwk_cmn[GPOWER_APP_CMN_OBJ_MAXNUM];	// CLWK(cmn)
	// 通常
	GFL_CLWK	*clwk[GPOWER_APP_OBJ_MAXNUM];			// CLWK
} GPOWER_APP_OBJ_WORK;

//--------------------------------------------------------------
///	メインワーク
//--------------------------------------------------------------
typedef struct
{
	HEAPID						heapID;
	// OBJ
	GPOWER_APP_OBJ_WORK			wk_obj;
	//描画設定
	GPOWER_APP_GRAPHIC_WORK		*graphic;
	//
	GPOWER_APP_PARAM 			*Param;							// 参照用として保存
	//フォント
	GFL_FONT					*font;
	//プリントキュー
	PRINT_QUE					*print_que;
	GFL_MSGDATA					*MsgData[GPOWER_APP_MES_MAXNUM];		// メッセージ
	PRINT_UTIL					PrintUtil[GPOWER_APP_CANVAS_MAXNUM];	// 文字表示用
	WORDSET						*WordSet;
	//
	BMPOAM_SYS_PTR				BmpOamSys;								// BMPOAM
	GFL_BMP_DATA				*BmpData[GPOWER_APP_BMPOAM_MAXNUM];		// Bmp
	BMPOAM_ACT_PTR				BmpOamWork[GPOWER_APP_BMPOAM_MAXNUM];	// BMPOAM用Work
	//
	APP_TASKMENU_WIN_WORK		*pAppTask[2];
	APP_TASKMENU_ITEMWORK		appitem[2];
	APP_TASKMENU_RES			*pAppTaskRes;
	//
	s32							Step;							// ステップ
	u32							Flag;							// フラグ
	s32							CurPos;							// カーソル
	s32							CurPosMax;						// 最大
	s32							Result;							// 結果
	s32							Timer;							// タイマ
	s32							AnimNum;						// アニメ用
	s32							YesNoCursor;					// yes/no
	u32							DelTamaFlag;					// フラグ
	s32							DelTama[3];						// 必要なデルタマ数
	//
	s16							*EmptyBuffer;					// 座標用のバッファ(必ず破棄すること)
	// パレットチェンジ用
	s32							PCount;							///< カウンタ
	u16							PBufStart[0x10];				///< 開始
	u16							PBufEnd[0x10];					///< 終了
	u16							PBufNow[0x10];					///< 現在
} GPOWER_APP_MAIN_WORK;

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
}GPOWER_APP_OBJ_DATA;

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
}GPOWER_APP_CANVAS_DATA;

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}GPOWER_APP_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}GPOWER_APP_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}GPOWER_APP_BGSCN_DATA;

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
}GPOWER_APP_BMPOAM_DATA;

//=============================================================================
/**
 *							データ定義(要整理)
 */
//=============================================================================
//---------------
// BG関連
//---------------
// BGPAL
static const GPOWER_APP_BGPAL_DATA gpapp_bgpal_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_gpower_app_gra_delpow_bg_NCLR,	0, 3,},		// 上、BG
	{ PALTYPE_MAIN_BG,	NARC_gpower_app_gra_delpow_bg_NCLR,	0, 3,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const GPOWER_APP_BGCHR_DATA gpapp_bgchr_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_gpower_app_gra_delpow_bg_NCGR,	0,	0,},	// 上、BG
	{ GFL_BG_FRAME3_M,	NARC_gpower_app_gra_delpow_bg_NCGR,	0,	0,},	// 下、BG
	{ GFL_BG_FRAME1_M,	NARC_gpower_app_gra_delpow_bg_NCGR, 0,	0,},	// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const GPOWER_APP_BGSCN_DATA gpapp_bgscn_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_gpower_app_gra_delpow_bgu_NSCR, 0,	0,},	// 上、下地
	{ GFL_BG_FRAME3_M,	NARC_gpower_app_gra_delpow_bgd_NSCR, 0,	0,},	// 下、下地
	{ GFL_BG_FRAME1_M,	NARC_gpower_app_gra_delpow_win_NSCR, 0,	0,},	// 下、下地

	{ 0xff, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(通常)
//------------------
// narc, resid, paloffset, palnum
static const s16 gpapp_obj_main_info_tbl[] = {
	NARC_gpower_app_gra_delpow_obj_NCLR, RESID_OBJ_PLT, 0, 3, // NCL
	NARC_gpower_app_gra_delpow_obj_NCGR, RESID_OBJ_CHR, 0, 0, // NCG
	NARC_gpower_app_gra_delpow_obj_NCER, RESID_OBJ_CEL, 0, 0, // NCE
	NARC_gpower_app_gra_delpow_obj_NANR, -1, 0, 0,			  // NAR?
};

// 情報
static const s16 gpapp_obj_main_itbl[] = {
	RESID_OBJ_PLT,
	RESID_OBJ_CHR,
	RESID_OBJ_CEL,
	//
	GPOWER_APP_OBJ_MAXNUM,
	CLWKMODE_NORMAL,
};

// OBJのテーブルデータ(通常)
static const GPOWER_APP_OBJ_DATA gpapp_obj_tbl[] = {
	{ 0,	0x30,	3,	1,	0,	1,	0,	0,},		// パネル0
	{ 0,	0x30,	3,	1,	0,	1,	1,	0,},		// パネル1
	{ 0,	0x30,	3,	1,	0,	1,	2,	0,},		// パネル2
	{ 1,	0x30,	3,	1,	0,	1,	3,	0,},		// パネル3

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(cmn)
//------------------
// 情報
static const s16 gpapp_obj_cmn_itbl[] = {
	RESID_CMN_OBJ_PLT,
	RESID_CMN_OBJ_CHR,
	RESID_CMN_OBJ_CEL,
	//
	GPOWER_APP_CMN_OBJ_MAXNUM,
	CLWKMODE_CMN,
};

// OBJのテーブルデータ(cmn)
static const GPOWER_APP_OBJ_DATA gpapp_obj_cmn_tbl[] = {
	{ 1,	0x30,	2,	1,	0,	1, 4,	0,},		// 戻るマーク

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//---------------
// CANVASの設定
//---------------
// (printutilと対応)
static const GPOWER_APP_CANVAS_DATA gpapp_canvas_tbl[] = {
	{ GFL_BG_FRAME2_S,	1,	0,	30,	3, PLTID_SUB_BG_FONT,},		// 上タイトル
	//
	{ GFL_BG_FRAME2_S,	2,	3,	20,	2, PLTID_SUB_BG_FONT,},		// 上:項目0
	{ GFL_BG_FRAME2_S,	2,	5,	20,	2, PLTID_SUB_BG_FONT,},		// 上:項目1
	{ GFL_BG_FRAME2_S,	2,	7,	20,	2, PLTID_SUB_BG_FONT,},		// 上:項目2
	{ GFL_BG_FRAME2_S,	2,	9,	20,	2, PLTID_SUB_BG_FONT,},		// 上:項目3
	{ GFL_BG_FRAME2_S,	2, 11,	20,	2, PLTID_SUB_BG_FONT,},		// 上:項目4
	{ GFL_BG_FRAME2_S,	2, 13,	20,	2, PLTID_SUB_BG_FONT,},		// 上:項目5
	{ GFL_BG_FRAME2_S,	2, 15,	20,	2, PLTID_SUB_BG_FONT,},		// 上:項目6
	{ GFL_BG_FRAME2_S,	2, 17,	20,	2, PLTID_SUB_BG_FONT,},		// 上:項目7
	{ GFL_BG_FRAME2_S,	2, 19,	20,	2, PLTID_SUB_BG_FONT,},		// 上:項目8
	{ GFL_BG_FRAME2_S,	2, 21,	20,	2, PLTID_SUB_BG_FONT,},		// 上:項目9
	//
	{ GFL_BG_FRAME2_S,	22,	3,	8,	2, PLTID_SUB_BG_FONT,},		// 上:項目0(数字)
	{ GFL_BG_FRAME2_S,	22,	5,	8,	2, PLTID_SUB_BG_FONT,},		// 上:項目1
	{ GFL_BG_FRAME2_S,	22,	7,	8,	2, PLTID_SUB_BG_FONT,},		// 上:項目2
	{ GFL_BG_FRAME2_S,	22,	9,	8,	2, PLTID_SUB_BG_FONT,},		// 上:項目3
	{ GFL_BG_FRAME2_S,	22, 11,	8,	2, PLTID_SUB_BG_FONT,},		// 上:項目4
	{ GFL_BG_FRAME2_S,	22, 13,	8,	2, PLTID_SUB_BG_FONT,},		// 上:項目5
	{ GFL_BG_FRAME2_S,	22, 15,	8,	2, PLTID_SUB_BG_FONT,},		// 上:項目6
	{ GFL_BG_FRAME2_S,	22, 17,	8,	2, PLTID_SUB_BG_FONT,},		// 上:項目7
	{ GFL_BG_FRAME2_S,	22, 19,	8,	2, PLTID_SUB_BG_FONT,},		// 上:項目8
	{ GFL_BG_FRAME2_S,	22, 21,	8,	2, PLTID_SUB_BG_FONT,},		// 上:項目9
	//
	{ GFL_BG_FRAME0_M,	 2,	14,	28,	6, PLTID_BG_FONT,},			// 下:yes/no
	//
	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

//---------------
// メッセージ
//---------------
static const s16 gpapp_mes_tbl[] = {
	msg_mono_pow_100,			// 「発動中のデルパワー」
	msg_mono_pow_101,			// 「なし」
	msg_mono_pow_102,			// 「ハイリンク」
	msg_mono_pow_103,			// 「分:秒」
	msg_mono_pow_104,			// 「をつかいますか?」
	msg_mono_pow_105,			// 「たりません」
	//
	msg_mono_pow_006,			// 「はい」
	msg_mono_pow_007,			// 「いいえ」
};

//------------------
// タッチペン領域
//------------------
static const GFL_UI_TP_HITTBL gpapp_rect_00_tbl[] = {
  {  28,  52,  56, 248 }, 			// 0
  {  60,  84,  32, 224 }, 			// 1
  {  92, 116,   8, 200 }, 			// 2
  { 132, 156, 128, 232 }, 			// ハイリンク
  {  96, 132, 216, 240 }, 			// ハイリンク2
  { 168, 192, 232, 255 }, 			// 戻る
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// yes/no
static const GFL_UI_TP_HITTBL gpapp_rect_01_tbl[] = {
  { 168, 192, 128-32, 172-32 }, 			// はい
  { 168, 192, 192-16, 255-16 }, 			// いいえ
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

//------------------
// BMPOAMの情報
//------------------
static const GPOWER_APP_BMPOAM_DATA gpapp_bmpoam_tbl[] = {
	{ 20,	2,	0x28,	3,	1,	0,	5,	0,},		// パネル00
	{ 20,	2,	0x28,	3,	1,	0,	6,	0,},		// パネル01
	{ 20,	2,	0x28,	3,	1,	0,	7,	0,},		// パネル02
	{ 20,	2,	0x28,	3,	1,	0,	8,	0,},		// パネル03
	//
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//-------------------
// タイマ
//-------------------
// (frame, パレット番号)
static const s16 gpapp_time_tbl[] = {
	1, 0,
	4, 1,
	8, 0,
   12, 1,
   16,-1,	// end
   -1,-1,
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT GPowerAppProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT GPowerAppProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT GPowerAppProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
///	以下に追加
//=====================================
static void gpower_app_init( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_main( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_exit( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_up(   GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_down( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_key_all( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_main_print( GPOWER_APP_MAIN_WORK* wk );
//
static s16  gpower_app_get_empty( GPOWER_APP_MAIN_WORK *wk, const s32 num );
//
static void gpower_app_init_bg(  GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_init_bg2( GPOWER_APP_MAIN_WORK* wk, const s32 type, const s32 ptype, const s32 pnum );
//
static void gpower_app_init_obj_clwk( GPOWER_APP_MAIN_WORK* wk, const s32 drawtype, const GPOWER_APP_OBJ_DATA *obj_adr, const s16 *info_adr );
static void gpower_app_init_obj_res( GPOWER_APP_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr );
static void gpower_app_init_obj_res_cmn( GPOWER_APP_MAIN_WORK* wk, const s32 drawtype );
static void gpower_app_init_obj( GPOWER_APP_MAIN_WORK* wk );
//
static void gpower_app_exit_obj( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_chg_objseq( GPOWER_APP_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
static void gpower_app_set_obj_visible( GPOWER_APP_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
static void gpower_app_set_palnum( GPOWER_APP_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 palnum );
static BOOL gpower_app_is_anime_end( GPOWER_APP_MAIN_WORK *wk, const s32 wmode, const s32 wnum );
//
static void gpower_app_init_obj_bmpoam( GPOWER_APP_MAIN_WORK* wk, const s32 drawtype, const GPOWER_APP_BMPOAM_DATA *bmp_adr, const u32 resid );
static void gpower_app_disp_bmpoam( GPOWER_APP_MAIN_WORK* wk, const s32 wnum, const STRBUF *strbuf, const s32 offx, const s32 offy );
static void gpower_app_set_bmpoam_visible( GPOWER_APP_MAIN_WORK* wk, const s32 wnum, const BOOL bflg );


// 文字列関連
static void gpower_app_init_string( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_disp_string( GPOWER_APP_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 dispmode,
								const PRINTSYS_LSB col);
static void gpower_app_exit_string( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_clear_canvas( GPOWER_APP_MAIN_WORK* wk, const s32 mode, const u32 color );
//
// 入力
static void gpower_app_chk_input( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_update( GPOWER_APP_MAIN_WORK* wk, const s32 mode );
//
static void gpower_app_disp_info( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_disp_use_able( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_disp_hilink( GPOWER_APP_MAIN_WORK* wk );
//
static s32  gpower_app_chk_tp( GPOWER_APP_MAIN_WORK* wk );
//
static void gpower_app_disp_win( GPOWER_APP_MAIN_WORK* wk, const s32 mode );
static void gpower_app_is_visible_win( GPOWER_APP_MAIN_WORK* wk, const BOOL bflg );
//
static BOOL gpower_app_chk_battery( GPOWER_APP_MAIN_WORK* wk );
//
static void gpower_app_init_param( GPOWER_APP_MAIN_WORK* wk );
//
static void gpower_app_init_chgpal( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_main_chgpal( GPOWER_APP_MAIN_WORK* wk );
//
static BOOL gpower_app_set_decide_anim( GPOWER_APP_MAIN_WORK* wk );
//
// yes/no
static void gpower_app_init_yesnowin( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_exit_yesnowin( GPOWER_APP_MAIN_WORK* wk );
static void gpower_app_main_yesnowin( GPOWER_APP_MAIN_WORK* wk );
static BOOL gpower_app_chk_yesnowin(  GPOWER_APP_MAIN_WORK* wk );
static BOOL gpower_app_is_yesnowin_anime_end( GPOWER_APP_MAIN_WORK* wk );
static BOOL gpower_app_chk_deltamawin( GPOWER_APP_MAIN_WORK* wk );
//
static void gpower_app_init_deltama( GPOWER_APP_MAIN_WORK* wk );

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA GPowerAppProcData = 
{
	GPowerAppProc_Init,
	GPowerAppProc_Main,
	GPowerAppProc_Exit,
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
 *	@param	*pwk					GPOWER_APP_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT GPowerAppProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	GPOWER_APP_MAIN_WORK 	*wk;
	
	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GPOWER_APP, UI_TEMPLATE_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(GPOWER_APP_MAIN_WORK), HEAPID_GPOWER_APP );
	GFL_STD_MemClear( wk, sizeof(GPOWER_APP_MAIN_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_GPOWER_APP;
	
	// 参照用
	wk->Param = pwk;
	
	//描画設定初期化(上:サブ、下:メイン)
	wk->graphic	= GPOWER_APP_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );
	
	//フォント作成
	wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	
	// メッセージ
	wk->MsgData[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_monolith_dat, wk->heapID );	// いろいろ
	wk->MsgData[1] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_power_dat, wk->heapID );		// パワー名
	
	// wordset
	wk->WordSet = WORDSET_Create( wk->heapID );
	
	//PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	//
	// yes/noウインドウ用
	wk->pAppTask[0] = 0;
	wk->pAppTask[1] = 0;
	wk->pAppTaskRes = APP_TASKMENU_RES_Create( GFL_BG_FRAME1_M, PLTID_BG_YESNO, wk->font, wk->print_que, wk->heapID );
	
	//----------------
	// 通常はこの中で初期化
	//----------------
	gpower_app_init( wk );
	
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
 *	@param	*pwk					GPOWER_APP_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT GPowerAppProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	s32		i;
	GPOWER_APP_MAIN_WORK* wk = mywk;
	
	//----------------
	// 処理を記述
	//----------------
	gpower_app_exit( wk );
	//
	//メッセージ破棄
	for(i=0;i<GPOWER_APP_MES_MAXNUM;i++){
		if(wk->MsgData[i]){
			GFL_MSG_Delete( wk->MsgData[i] );
			wk->MsgData[i] = 0;
		}
	}
	
	// WORDSET破棄
	WORDSET_Delete( wk->WordSet );
	
	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	
	//FONT
	GFL_FONT_Delete( wk->font );
	
	// yes/no
	gpower_app_exit_yesnowin( wk );
	
	// yes/no
	APP_TASKMENU_RES_Delete( wk->pAppTaskRes );
	
	//描画設定破棄
	GPOWER_APP_GRAPHIC_Exit( wk->graphic );
	
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
 *	@param	*pwk					GPOWER_APP_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT GPowerAppProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	GPOWER_APP_MAIN_WORK* wk = mywk;
	
	switch ( *seq )
	{
		case 0:
			if (WIPE_SYS_EndCheck())
			{
				(*seq) ++;
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
	gpower_app_main( wk );
	
	//PRINT_QUE
	gpower_app_main_print( wk );
	
	//2D描画
	GPOWER_APP_GRAPHIC_2D_Draw( wk->graphic );
	
	//3D描画
	GPOWER_APP_GRAPHIC_3D_StartDraw( wk->graphic );
	
	GPOWER_APP_GRAPHIC_3D_EndDraw( wk->graphic );
	
	return GFL_PROC_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(init)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_init( GPOWER_APP_MAIN_WORK* wk )
{
	s32			i;
	
	//----------
	// 初期化
	//----------
	// 上、下画面をまとめて処理
	wk->Step 		= STEP_INIT;
	wk->Flag		= 0;
	wk->CurPos		= 0;
	wk->CurPosMax	= wk->Param->gpower_num;
	if(wk->CurPosMax <= 0){
		wk->CurPosMax = 1;
		OS_Printf("gpower num error\n");
	}
	wk->Result		= -1;
	//
	wk->PCount		= 0;
	wk->AnimNum		= 0;
	wk->Timer		= 0;
	wk->YesNoCursor = 0;
	// 初期化
	for(i=0;i<GPOWER_APP_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
	}
	//
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_GPOWER_APP, NARC_gpower_app_gra_gpower_app_pos_tblbin, wk->heapID );
	//
	//---------------
	// パラメータ初期化
	//---------------
	gpower_app_init_param( wk );
	
	//---------------
	// bgの初期化
	//---------------
	gpower_app_init_bg( wk );
	gpower_app_init_bg2( wk, GFL_BG_FRAME2_M, PALTYPE_MAIN_BG, PLTID_BG_CMN );
	
	//---------------
	// 文字列関連の初期化
	//---------------
	gpower_app_init_string( wk );
	
	//------------
	// OBJ初期化
	//------------
	gpower_app_init_obj( wk );
	
	// パレットチェンジ用
	gpower_app_init_chgpal( wk );
	
	//------------
	// 初回更新
	//------------
	gpower_app_update( wk, 0 );
	wk->Flag |= FLG_MW_UPDATE;
	//
	// win非表示
	gpower_app_is_visible_win( wk, FALSE );
	//
	// 半透明設定
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1,
		GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
		14, 2);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
//static s16 rate[] = {10,6,};

static void gpower_app_main( GPOWER_APP_MAIN_WORK* wk )
{
	// 下画面
	gpower_app_down( wk );
#if 0
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
		rate[0]++;
		rate[1]--;
		//
		OS_Printf("%d,%d\n",rate[0],rate[1]);
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1,
		GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
		rate[0], rate[1]);

	}
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
		rate[0]--;
		rate[1]++;
		//
		OS_Printf("%d,%d\n",rate[0],rate[1]);
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1,
		GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
		rate[0], rate[1]);

	}
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(exit)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_exit( GPOWER_APP_MAIN_WORK* wk )
{
	//-------------------------
	// 終了パラメータのセット
	//-------------------------
	// バッテリーエラー
	if(wk->Flag & FLG_MW_BATTERY_ERR){
		wk->Param->ret_mode = GPOWER_APP_RET_COMM_OFF;
	}
	// 通常
	else{
		switch( wk->Result ){
			case 3:			// ハイリンク
				wk->Param->ret_mode = GPOWER_APP_RET_HILINK;
				break;
			//
			case 0:			// デルパワー使用
			case 1:
			case 2:
				wk->Param->ret_mode   = GPOWER_APP_RET_GPOWER;
				wk->Param->ret_gpower = wk->Param->gpower[wk->Result];
				break;
			//
			default:
				wk->Param->ret_mode = GPOWER_APP_RET_NULL;
				break;
		}
	}
	
	// 上、下画面をまとめて処理
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	
	// 文字表示用
	gpower_app_exit_string( wk );
	// OBJ破棄
	gpower_app_exit_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(下画面)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_down( GPOWER_APP_MAIN_WORK* wk )
{
	gpower_app_key_all( wk );
	//
	// パレットチェンジ
	gpower_app_main_chgpal( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理まとめ
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_key_all( GPOWER_APP_MAIN_WORK* wk )
{
	BOOL	bflg;
	BOOL	bbflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		//
		// LV1
		case STEP_LV1:
			// バッテリーチェック
			bbflg = gpower_app_chk_battery( wk );
			if(bbflg == TRUE){
				wk->Flag |= FLG_MW_BATTERY_ERR;
				wk->Step = STEP_END;
			}else{
				// 入力
				gpower_app_chk_input( wk );
			}
			break;
		//
		// 終了アニメ待ち=>終了
		case STEP_LV2:
			bflg = gpower_app_is_anime_end( wk, CLWKMODE_CMN, CLWKID_CMN_RETURN );
			if(bflg == FALSE){
				wk->Step = STEP_END;
				wk->Result = -1;
			}
			break;
		//
		// 決定アニメ待ち=>window表示
		case STEP_LV3:
			bflg = gpower_app_set_decide_anim( wk );
			gpower_app_set_palnum( wk, CLWKMODE_NORMAL, CLWKID_POWER_00+wk->CurPos, wk->AnimNum );
			if(bflg == TRUE){
				if(wk->CurPos == 3){
					wk->Step = STEP_END;
					wk->Result = wk->CurPos;
				}else{
					// デルタマが足りない場合
					// =>足りないメッセージ
					if(wk->DelTamaFlag & (1<<wk->CurPos)){
						gpower_app_disp_win( wk, 1 );
						wk->Step = STEP_LV3+3;
						gpower_app_is_visible_win( wk, TRUE );
					}
					// 足りているので使う(yes/no)
					else{
						gpower_app_init_yesnowin( wk );
						gpower_app_disp_win( wk, 0 );
						gpower_app_is_visible_win( wk, TRUE );
						wk->Step = STEP_LV3+10;
						// 戻るボタンを消しておく
						// yes/no表示で見えにくくなるため
						gpower_app_set_obj_visible( wk, CLWKMODE_CMN, CLWKID_CMN_RETURN, FALSE );
					}
				}
				wk->Flag &= ~FLG_MW_DECIDE_ANIM;
			}
			break;
		//
		// window(足りないメッセージ)
		case STEP_LV3+3:
			// バッテリーチェック
			bbflg = gpower_app_chk_battery( wk );
			if(bbflg == TRUE){
				wk->Flag |= FLG_MW_BATTERY_ERR;
				wk->Step = STEP_END;
			}else{
				bflg = gpower_app_chk_deltamawin( wk );
				if(bflg == TRUE){
					gpower_app_is_visible_win( wk, FALSE );
					wk->Step = STEP_LV1;
					// カーソルのアニメを戻す
					wk->Flag |= FLG_MW_UPDATE;
					// SE
					PMSND_PlaySE( SND_GPOWER_APP_CANCEL );
				}
			}
			break;
		//
		// yes/no待ち=>yes/no終了アニメ
		case STEP_LV3+10:
			// バッテリーチェック
			bbflg = gpower_app_chk_battery( wk );
			if(bbflg == TRUE){
				wk->Flag |= FLG_MW_BATTERY_ERR;
				wk->Step = STEP_END;
			}else{
				// yes/noまち
				gpower_app_main_yesnowin( wk );
				// 判定
				bflg = gpower_app_chk_yesnowin( wk );
				if(bflg == TRUE){
					wk->Step = STEP_LV3+20;
				}
			}
			break;
		//
		// yes/no終了アニメ待ち=>終了など
		case STEP_LV3+20:
			// yes/noまち
			gpower_app_main_yesnowin( wk );
			// 終了待ち
			bflg = gpower_app_is_yesnowin_anime_end( wk );
			if(bflg == TRUE){
				// 終了したので処理の分岐
				// yes
				if(wk->YesNoCursor == 0){
					wk->Step = STEP_END;
					wk->Result = wk->CurPos;
				}
				// no
				else{
					wk->Step = STEP_LV1;
					// カーソルのアニメを戻す
					wk->Flag |= FLG_MW_UPDATE;
				}
				// 非表示
				gpower_app_is_visible_win( wk, FALSE );
				gpower_app_exit_yesnowin( wk );
				//
				// 戻るボタンを戻しておく
				gpower_app_set_obj_visible( wk, CLWKMODE_CMN, CLWKID_CMN_RETURN, TRUE );
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
 *	@brief  Queの処理(Main)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_main_print( GPOWER_APP_MAIN_WORK* wk )
{
	s32		i;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<GPOWER_APP_CANVAS_MAXNUM;i++){
		PRINT_UTIL_Trans( &wk->PrintUtil[i], wk->print_que );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  emptyの取得
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
static s16 gpower_app_get_empty( GPOWER_APP_MAIN_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= GPOWER_APP_EMPTY_MAXNUM)){
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
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_bg( GPOWER_APP_MAIN_WORK* wk )
{
	s32		i;
	s32		tmp;
	const GPOWER_APP_BGPAL_DATA	*padr = &gpapp_bgpal_tbl[0];
	const GPOWER_APP_BGCHR_DATA	*cadr = &gpapp_bgchr_tbl[0];
	const GPOWER_APP_BGSCN_DATA	*sadr = &gpapp_bgscn_tbl[0];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_GPOWER_APP, wk->heapID );
	
	// パレット転送
	for(i=0;i<GPOWER_APP_BGNUM_MAXNUM;i++){
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
	for(i=0;i<GPOWER_APP_BGNUM_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, wk->heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<GPOWER_APP_BGNUM_MAXNUM;i++){
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
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32			(BG面)
 *	@param	const s32			(メインorサブ)
 *	@param	const s32			(パレット番号)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_bg2( GPOWER_APP_MAIN_WORK* wk, const s32 type, const s32 ptype, const s32 pnum )
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
	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, id, ptype, pnum*0x20, 0x20, wk->heapID );
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
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32					drawtype
 *	@param	const GPOWER_APP_OBJ_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_obj_clwk( GPOWER_APP_MAIN_WORK* wk, const s32 drawtype, const GPOWER_APP_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16						tmp;
	s16						pos[2];
	s16						buf[6];
	s32 					i;
	BOOL					bflg;
	GFL_CLWK_DATA 			cldata;
	CLWK_FLIP_TYPE			flip;
    GFL_CLUNIT*				p_clunit 	= GPOWER_APP_GRAPHIC_GetClunit( wk->graphic );
	// アドレスセット
	const GPOWER_APP_OBJ_DATA *adr 	= obj_adr;
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
	if(buf[4] == CLWKMODE_NORMAL){			// 通常
		clwk = &wk->wk_obj.clwk[0];
	}
	else if(buf[4] == CLWKMODE_CMN){		// cmn
		clwk = &wk->wk_obj.clwk_cmn[0];
	}
	
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		pos[0] = gpower_app_get_empty(wk, tmp*2);
		pos[1] = gpower_app_get_empty(wk, tmp*2+1);
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
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_obj_res( GPOWER_APP_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr )
{
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_GPOWER_APP, wk->heapID );
    GFL_CLUNIT*	p_clunit = GPOWER_APP_GRAPHIC_GetClunit( wk->graphic );
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
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_obj_res_cmn( GPOWER_APP_MAIN_WORK* wk, const s32 drawtype )
{
	// cmnのidを取得
	const u32 	arcid    = APP_COMMON_GetArcId();
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( arcid, wk->heapID );
    GFL_CLUNIT*	p_clunit = GPOWER_APP_GRAPHIC_GetClunit( wk->graphic );
	u32			idx[4];
	//
	idx[0] = APP_COMMON_GetBarIconPltArcIdx();		// pal
	idx[1] = APP_COMMON_GetBarIconCharArcIdx();		// ncg
	idx[2] = APP_COMMON_GetBarIconCellArcIdx(  APP_COMMON_MAPPING_128K );	// cell
	idx[3] = APP_COMMON_GetBarIconAnimeArcIdx( APP_COMMON_MAPPING_128K );	// anim
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ RESID_CMN_OBJ_PLT ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, idx[0], drawtype, 0x20*PLTID_OBJ_CMN, 0, PLTID_OBJ_CMN_NUM, wk->heapID );
	wk->wk_obj.res[ RESID_CMN_OBJ_CHR ] = GFL_CLGRP_CGR_Register(      p_handle, idx[1], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ RESID_CMN_OBJ_CEL ] = GFL_CLGRP_CELLANIM_Register( p_handle, idx[2], idx[3], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化まとめ
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_obj( GPOWER_APP_MAIN_WORK* wk )
{
    GFL_CLUNIT*	p_clunit = GPOWER_APP_GRAPHIC_GetClunit( wk->graphic );
	// リソース読み込み(通常)
	gpower_app_init_obj_res( wk, CLSYS_DRAW_MAIN, &gpapp_obj_main_info_tbl[0] );
	// リソース読み込み(cmn)
	gpower_app_init_obj_res_cmn( wk, CLSYS_DRAW_MAIN );
	// CLWK登録(通常)
	gpower_app_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &gpapp_obj_tbl[0], &gpapp_obj_main_itbl[0] );
	// CLWK登録(cmn)
	gpower_app_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &gpapp_obj_cmn_tbl[0], &gpapp_obj_cmn_itbl[0] );
	//
	// 通信アイコンセット(下画面)
	GFL_NET_ReloadIconTopOrBottom( FALSE, wk->heapID );
	
	{
		// フォントのパレットは圧縮しているので注意
		ARCHANDLE* p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		wk->wk_obj.res[ RESID_FONT_PLT ] = GFL_CLGRP_PLTT_RegisterComp( p_handle, NARC_font_talkwin_nclr, CLSYS_DRAW_MAIN, 0x20*PLTID_OBJ_FONT, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
	// BmpOAM初期化
	wk->BmpOamSys = BmpOam_Init( wk->heapID, p_clunit );
	// BMPOAM登録
	gpower_app_init_obj_bmpoam( wk, CLSYS_DRAW_MAIN, &gpapp_bmpoam_tbl[0], RESID_FONT_PLT );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_exit_obj( GPOWER_APP_MAIN_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<GPOWER_APP_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk[i] );
	}
	for(i=0;i<GPOWER_APP_CMN_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk_cmn[i] );
	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_OBJ_CEL] );
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_CMN_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_CMN_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_CMN_OBJ_CEL] );
	//
	//リソース破棄(font)
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_FONT_PLT] );
	//
	// BmpOAM後処理
	for(i=0;i<GPOWER_APP_BMPOAM_MAXNUM;i++){
		BmpOam_ActorDel( wk->BmpOamWork[i] );
		GFL_BMP_Delete( wk->BmpData[i] );
	}
	BmpOam_Exit( wk->BmpOamSys );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_chg_objseq( GPOWER_APP_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	if(wmode == CLWKMODE_NORMAL){			// 通常
		if(wnum >= GPOWER_APP_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
	}
	else if(wmode == CLWKMODE_CMN){			// 共通
		if(wnum >= GPOWER_APP_CMN_OBJ_MAXNUM){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk_cmn[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk_cmn[wnum], TRUE );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通、2:上画面)
 *	@param	const s32			objワーク番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_set_obj_visible( GPOWER_APP_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// 表示/非表示
	if(wmode == CLWKMODE_NORMAL){		// 通常
		if(wnum >= GPOWER_APP_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
	}
	else if(wmode == CLWKMODE_CMN){		// 共通
		if(wnum >= GPOWER_APP_CMN_OBJ_MAXNUM){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk_cmn[wnum], bflg );
	}
}
//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのパレット切り替え
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通、2:上画面)
 *	@param	const s32			objワーク番号
 *	@param	const s32			パレット番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_set_palnum( GPOWER_APP_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 palnum )
{
	// 表示/非表示
	if(wmode == CLWKMODE_NORMAL){		// 通常
		if(wnum >= GPOWER_APP_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetPlttOffs( wk->wk_obj.clwk[wnum], palnum, CLWK_PLTTOFFS_MODE_PLTT_TOP );
	}
	else if(wmode == CLWKMODE_CMN){		// 共通
		if(wnum >= GPOWER_APP_CMN_OBJ_MAXNUM){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetPlttOffs( wk->wk_obj.clwk_cmn[wnum], palnum, CLWK_PLTTOFFS_MODE_PLTT_TOP );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのアニメの終了待ち
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通、2:上画面)
 *	@param	const s32			objワーク番号
 *
 *	@retval	const BOOL			(TRUE:再生中、FALSE:停止、終了)
 */
//-----------------------------------------------------------------------------
static BOOL gpower_app_is_anime_end( GPOWER_APP_MAIN_WORK *wk, const s32 wmode, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// 終了判定らしい
	if(wmode == CLWKMODE_NORMAL){		// 通常
		if(wnum >= GPOWER_APP_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	else if(wmode == CLWKMODE_CMN){		// 共通
		if(wnum >= GPOWER_APP_CMN_OBJ_MAXNUM){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk_cmn[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(bmpoam)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const GPOWER_APP_BMPOAM_DATA*	アドレス
 *	@param	const s16*						アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_obj_bmpoam( GPOWER_APP_MAIN_WORK* wk, const s32 drawtype, const GPOWER_APP_BMPOAM_DATA *bmp_adr, const u32 resid )
{
	s16							tmp;
	s32							i;
	s32							buf[2];
	BMPOAM_ACT_DATA				actdata;
	const GPOWER_APP_BMPOAM_DATA	*adr  = bmp_adr;
//	const s16					*iadr = info_adr;
	GFL_CLACTPOS				cpos;
	//
	for(i=0;i<GPOWER_APP_BMPOAM_MAXNUM;i++){
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
		cpos.x = gpower_app_get_empty( wk, tmp*2 );
		cpos.y = gpower_app_get_empty( wk, tmp*2+1 );
		//
		GFL_STD_MemClear( &actdata, sizeof(BMPOAM_ACT_DATA) );
		actdata.bmp			= wk->BmpData[i];
		actdata.x 			= cpos.x;
		actdata.y 			= cpos.y;
//		actdata.pltt_index	= wk->wk_obj.res[ iadr[0+1] ];		// plt
		actdata.pltt_index	= wk->wk_obj.res[ resid ];
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
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32				ワーク番号
 *	@param	const STRBUF*			表示文字列
 *	@param	const s32,s32			表示オフセット
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_disp_bmpoam( GPOWER_APP_MAIN_WORK* wk, const s32 wnum, const STRBUF *strbuf, const s32 offx, const s32 offy )
{
	if(wnum >= GPOWER_APP_BMPOAM_MAXNUM){
		OS_Printf("bmpoamj max error:%d\n",wnum);
		return;
	}
	GFL_BMP_Clear( wk->BmpData[wnum], 0 );
	PRINTSYS_PrintQueColor( wk->print_que, wk->BmpData[wnum], 0, 0, strbuf, wk->font, PRINTSYS_LSB_Make(15,2,0) );
	BmpOam_ActorBmpTrans( wk->BmpOamWork[wnum] );
	//
	// 座標も再セットしておく
	{
		s16	buf[2];
		s16	tmp;
		const GPOWER_APP_BMPOAM_DATA *adr = &gpapp_bmpoam_tbl[wnum];
		//
		tmp = adr->empty_num;
		buf[0] = gpower_app_get_empty( wk, tmp*2 );
		buf[1] = gpower_app_get_empty( wk, tmp*2+1 );
		buf[0] += offx;
		buf[1] += offy;
		BmpOam_ActorSetPos( wk->BmpOamWork[wnum], buf[0], buf[1] );
	}
	// 表示
	BmpOam_ActorSetDrawEnable( wk->BmpOamWork[wnum], TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OB文字列表示(bmpoam)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32				ワーク番号
 *	@param	const BOOL				(表示/非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_set_bmpoam_visible( GPOWER_APP_MAIN_WORK* wk, const s32 wnum, const BOOL bflg )
{
	BmpOam_ActorSetDrawEnable( wk->BmpOamWork[wnum], bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(Canvasの作成)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_string( GPOWER_APP_MAIN_WORK* wk )
{
	s16		buf[2];
	s32		i;
	const GPOWER_APP_CANVAS_DATA	*adr = &gpapp_canvas_tbl[0];
	
	// Canvasの設定
	for(i=0;i<GPOWER_APP_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット
		wk->PrintUtil[i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		// スクリーンは一度でよいはず
		// (初期化時のみでも大丈夫なはず)
		GFL_BMPWIN_MakeScreen( wk->PrintUtil[i].win );
		GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[i].win) );
		adr++;
	}
	//
	// fontパレットの転送
	{
		ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, PLTID_BG_FONT*0x20,     0x20, wk->heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_SUB_BG,  PLTID_SUB_BG_FONT*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示(カラーに注意:要修正)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
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
static void gpower_app_disp_string( GPOWER_APP_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 dispmode,
								const PRINTSYS_LSB col)
{
	// 一旦クリア
	gpower_app_clear_canvas( wk, mode, MES_CLEAR_COLOR );
	
	// 表示
	PRINTTOOL_PrintColor(
				&wk->PrintUtil[mode],			// プリントユーティル ( BMPWIN )
				wk->print_que,					// プリントキュー
				x, y,							// 表示基準座標
				strbuf,							// 文字列
				wk->font,						// フォント
				col,							// カラー
				dispmode );						// 位置
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示終了
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_exit_string( GPOWER_APP_MAIN_WORK* wk )
{
	s32		i;
	
	for(i=0;i<GPOWER_APP_CANVAS_MAXNUM;i++){
		if(wk->PrintUtil[i].win){
			GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
			wk->PrintUtil[i].win = 0;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *	@param	const s32				キャンバスの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_clear_canvas( GPOWER_APP_MAIN_WORK* wk, const s32 mode, const u32 color )
{
	// clear
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), color );
	wk->PrintUtil[mode].transReq = TRUE;
}

//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_chk_input( GPOWER_APP_MAIN_WORK* wk )
{
	s32		id;
	s32		flg=0;
	s32		dflg=0;		// 決定
	s32		oldpos = wk->CurPos;
	
	// 更新あり
	if(wk->Flag & FLG_MW_UPDATE){
		flg = 1;
		wk->Flag &= ~FLG_MW_UPDATE;
	}
	
	// タッチペン処理
	id = gpower_app_chk_tp( wk );
	// パネル
	if((id >= 0)&&(id <= 3)){
		// タッチは決定でもある
		wk->CurPos = id;
		flg = 1;
		dflg = 1;		// 決定
		wk->Flag |= FLG_MW_DISP_CURSOR;
		// SE
		PMSND_PlaySE( SND_GPOWER_APP_CURSOR );
	}
	// 戻る
	else if(id == 4){
		flg = 2;
		// 戻るアニメ
		gpower_app_chg_objseq( wk, CLWKMODE_CMN, CLWKID_CMN_RETURN, CMN_RETURN_ON_SEQ );
		wk->Step = STEP_LV2;
		// SE
		PMSND_PlaySE( SND_GPOWER_APP_CANCEL );
	}
	//
	// キー入力
	if(flg == 0){
		// カーソル表示してない場合=>入力を待つ
		if(!(wk->Flag & FLG_MW_DISP_CURSOR)){
			// 入力があったときに表示
			if(GFL_UI_KEY_GetRepeat() & (PAD_KEY_UP|PAD_KEY_DOWN|PAD_BUTTON_DECIDE)){
				flg = 1;
				wk->Flag |= FLG_MW_DISP_CURSOR;
				// SE
				PMSND_PlaySE( SND_GPOWER_APP_CURSOR );
			}
		}
		// 上
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
			// カーソル表示中のみ反応
			if((wk->Flag & FLG_MW_DISP_CURSOR)&&(flg == 0)){
				// ハイリンクパネル
				if(wk->CurPos == 3){
					wk->CurPos = wk->CurPosMax-1;
				}
				// 通常
				else{
					wk->CurPos--;
					if(wk->CurPos < 0){
						// ハイリンク選択不可 => 最大箇所
						if(wk->Flag & FLG_MW_NO_SELECT){
							wk->CurPos = wk->CurPosMax-1;
						}else{
							wk->CurPos = 3;
						}
					}
				}
				flg = 1;
				//
				// カーソル移動があったときのみ鳴らす
				if(wk->CurPos != oldpos){
					// SE
					PMSND_PlaySE( SND_GPOWER_APP_CURSOR );
				}
			}
		}
		// 下
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
			// カーソル表示中のみ反応
			if((wk->Flag & FLG_MW_DISP_CURSOR)&&(flg == 0)){
				// ハイリンクパネル
				if(wk->CurPos == 3){
					wk->CurPos = 0;
				}
				// 通常
				else{
					wk->CurPos++;
					if(wk->CurPos > (wk->CurPosMax-1)){
						if(wk->Flag & FLG_MW_NO_SELECT){
							wk->CurPos = 0;
						}else{
							wk->CurPos = 3;
						}
					}
				}
				flg = 1;
				//
				// カーソル移動があったときのみ鳴らす
				if(wk->CurPos != oldpos){
					// SE
					PMSND_PlaySE( SND_GPOWER_APP_CURSOR );
				}
			}
		}
		// 戻る
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
			flg = 2;
			// 戻るアニメ
			gpower_app_chg_objseq( wk, CLWKMODE_CMN, CLWKID_CMN_RETURN, CMN_RETURN_ON_SEQ );
			wk->Step = STEP_LV2;
			// SE
			PMSND_PlaySE( SND_GPOWER_APP_CANCEL );
		}
		// 決定
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
			if((wk->Flag & FLG_MW_DISP_CURSOR)&&(flg == 0)){
				flg = 2;
				dflg = 1;	// 決定
				// SE
				PMSND_PlaySE( SND_GPOWER_APP_DECIDE );
			}
		}
	}
	//
	// 決定
	if(dflg == 1){
		wk->Flag |= FLG_MW_DECIDE_ANIM;
		wk->Timer = 0;
		wk->Step = STEP_LV3;
	}
	//
	// 更新あり
	if(flg == 1){
		// 更新
		gpower_app_update( wk, 1 );
		// 旧カーソル
		gpower_app_set_palnum( wk, CLWKMODE_NORMAL, CLWKID_POWER_00+oldpos, 0 );
		// 新カーソル
		if(wk->Flag & FLG_MW_DISP_CURSOR){
			gpower_app_set_palnum( wk, CLWKMODE_NORMAL, CLWKID_POWER_00+wk->CurPos, 2 );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  更新
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval						(TRUE:入力あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static void gpower_app_update( GPOWER_APP_MAIN_WORK* wk, const s32 mode )
{
	STRBUF		*strbuf;
	
	// 初回のみの更新
	if(mode == 0){
		// 上文字列
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], gpapp_mes_tbl[0] );
		gpower_app_disp_string( wk, CVS_UP_TITLE, 0, 4, strbuf, 0, PRINTSYS_LSB_Make(15,2,0) );
		GFL_STR_DeleteBuffer( strbuf );
		//
		// 発動中の情報表示
		gpower_app_disp_info( wk );
		//
		// 使用可能(装備可能)のデルパワー情報
		gpower_app_disp_use_able( wk );
		//
		// ハイリンク情報表示(test)
		gpower_app_disp_hilink( wk );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  上画面(発動中の情報表示)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_disp_info( GPOWER_APP_MAIN_WORK* wk )
{
	s32		i;
	s32		num;
	s32		buf[2];
	STRBUF	*strbuf;
	STRBUF	*strbuf2;
	STRBUF	*strbuf3;
	
	// クリア
	for(i=0;i<GPOWER_APP_POWER_MAXNUM;i++){
		gpower_app_clear_canvas( wk, CVS_UP_POWER_00+i,     MES_CLEAR_COLOR );
		gpower_app_clear_canvas( wk, CVS_UP_POWER_NUM_00+i, MES_CLEAR_COLOR );
	}
	//
	// 現在発動中のパワー数
	num = wk->Param->occur_num;
	// 項目数
	OS_Printf("occur num :%d\n",num);
	//
	// 「なし」
	if(num == 0){
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], gpapp_mes_tbl[1] );
		gpower_app_disp_string( wk, CVS_UP_POWER_00, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(15,2,0) );
		GFL_STR_DeleteBuffer( strbuf );
	}
	// 「項目の表示」
	else{
		for(i=0;i<num;i++){
			// 項目
			strbuf = GFL_MSG_CreateString( wk->MsgData[1], wk->Param->occur_list[i].id );
			gpower_app_disp_string( wk, CVS_UP_POWER_00+i, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(15,2,0) );
			GFL_STR_DeleteBuffer( strbuf );
			//
			// 時間
			strbuf = GFL_MSG_CreateString( wk->MsgData[0], gpapp_mes_tbl[3] );
			buf[0] = wk->Param->occur_list[i].life/60;		// 分
			buf[1] = wk->Param->occur_list[i].life%60;		// 秒
			strbuf2 = GFL_STR_CreateBuffer( 64, wk->heapID );		// tmp
			strbuf3 = GFL_STR_CreateBuffer( 64, wk->heapID );		// dest
			// 分
			STRTOOL_SetNumber( strbuf2, buf[0], 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
			WORDSET_RegisterWord( wk->WordSet, 0, strbuf2, 0, TRUE, PM_LANG );
			// 秒
			STRTOOL_SetNumber( strbuf2, buf[1], 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
			WORDSET_RegisterWord( wk->WordSet, 1, strbuf2, 0, TRUE, PM_LANG );
			// パラメータを展開しstrbuf3にセット
			WORDSET_ExpandStr( wk->WordSet, strbuf3, strbuf );
			gpower_app_disp_string( wk, CVS_UP_POWER_NUM_00+i, 0, 0, strbuf3, 0, PRINTSYS_LSB_Make(15,2,0) );
			GFL_STR_DeleteBuffer( strbuf );
			GFL_STR_DeleteBuffer( strbuf2 );
			GFL_STR_DeleteBuffer( strbuf3 );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  下画面(使用可能デルパワー表示)=>(bmpoamを使用)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_disp_use_able( GPOWER_APP_MAIN_WORK* wk )
{
	s32		i;
	s32		seq;
	s32		num;
	STRBUF	*strbuf;
	
	// クリア
	for(i=0;i<3;i++){
		gpower_app_set_bmpoam_visible( wk, i, FALSE );
	}
	//
	// 現在装備中のパワー数
	num = wk->CurPosMax;
	// 項目数
	OS_Printf("gpower num :%d\n",num);
	//
	// なしの場合はこないはず
	if(num == 0){
		// なにもしない
	}
	// 「項目の表示」
	else{
		for(i=0;i<num;i++){
			strbuf = GFL_MSG_CreateString( wk->MsgData[1], wk->Param->gpower[i] );
			gpower_app_disp_bmpoam( wk, i, strbuf, 0, 0 );
			gpower_app_set_bmpoam_visible( wk, i, TRUE );
			GFL_STR_DeleteBuffer( strbuf );
		}
	}
	//
	// ここ限定でもいいはずなので、
	// パネルも変更してしまう
	for(i=0;i<3;i++){
		seq = SEQ_POWER_ON;
		if(num <= i){
			seq = SEQ_POWER_OFF;
		}
		// シーケンスの切り替え
		gpower_app_chg_objseq( wk, CLWKMODE_NORMAL, CLWKID_POWER_00+i, seq );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  下画面(ハイリンクパネル表示)=>(bmpoamを使用)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_disp_hilink( GPOWER_APP_MAIN_WORK* wk )
{
	s32		seq;
	BOOL	bflg=TRUE;		// (TRUE:表示、FALSE:非表示)
	STRBUF	*strbuf;
	
	// 選択不可
	if(wk->Flag & FLG_MW_NO_SELECT){
		bflg = FALSE;
	}
	
	// クリア
	gpower_app_set_bmpoam_visible( wk, 3, FALSE );
	
	// 表示
	if( bflg == TRUE ){
		// ハイリンク
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], gpapp_mes_tbl[2] );
		gpower_app_disp_bmpoam( wk, 3, strbuf, 0, 0 );
		GFL_STR_DeleteBuffer( strbuf );
	}
	//
	// パネルの変更
	seq = SEQ_HILINK_ON;
	if( bflg == FALSE ){
		seq = SEQ_HILINK_OFF;
	}
	// シーケンスの切り替え
	gpower_app_chg_objseq( wk, CLWKMODE_NORMAL, CLWKID_HILINK, seq );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペンの判定
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval s32						(id)
 */
//-----------------------------------------------------------------------------
static s32 gpower_app_chk_tp( GPOWER_APP_MAIN_WORK* wk )
{
	s32		ret=-1;
	s32		id;
	
	// タッチペン判定
	id = GFL_UI_TP_HitTrg( gpapp_rect_00_tbl );
#if 0
	if((id >= 0)&&(id <= 5)){
		{
			u32		x,y;
			GFL_UI_TP_GetPointTrg( &x, &y );
			OS_Printf("%d :(%d,%d)\n",id,x,y);
		}
	}
#endif
	//
	// パネルの場合=>選択可能のものを調べる
	if((id >= 0)&&(id <= 2)){
		if(wk->CurPosMax > id){
			ret = id;
		}else{
			// 項目外のタッチはカーソル非表示
			wk->Flag &= ~FLG_MW_DISP_CURSOR;
			wk->Flag |= FLG_MW_UPDATE;
		}
	}
	// ハイリンク
	else if((id == 3)||(id == 4)){
		// 選択可能の場合
		if(!(wk->Flag & FLG_MW_NO_SELECT)){
			ret = 3;
		}else{
			// 項目外のタッチはカーソル非表示
			wk->Flag &= ~FLG_MW_DISP_CURSOR;
			wk->Flag |= FLG_MW_UPDATE;
		}
	}
	// 戻る
	else if(id == 5){
		ret = 4;
	}
	//
	return( ret );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  下画面説明パネル
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_disp_win( GPOWER_APP_MAIN_WORK* wk, const s32 mode )
{
	STRBUF	*strbuf;
	STRBUF	*strbuf2;
	STRBUF	*strbuf3;
	STRBUF	*strbuf4;
	
	// クリア
	gpower_app_clear_canvas( wk, CVS_DOWN_WIN, MES_CLEAR_COLOR );
	
	// 文章
	//
	// 足りている場合
	if(mode == 0){
		strbuf  = GFL_MSG_CreateString( wk->MsgData[0], gpapp_mes_tbl[4] );		// org
		strbuf3 = GFL_STR_CreateBuffer( 256, wk->heapID );						// dest
		// gpower(0)
		strbuf2 = GFL_MSG_CreateString( wk->MsgData[1], wk->Param->gpower[wk->CurPos] );	// prm
		WORDSET_RegisterWord( wk->WordSet, 0, strbuf2, 0, TRUE, PM_LANG );
		// 必要数(1)
		strbuf4 = GFL_STR_CreateBuffer( 64, wk->heapID );						// 数値用
		STRTOOL_SetNumber( strbuf4, wk->DelTama[wk->CurPos], 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
		WORDSET_RegisterWord( wk->WordSet, 1, strbuf4, 0, TRUE, PM_LANG );
		// もっている数(2)
		STRTOOL_SetNumber( strbuf4, wk->Param->derudama_num, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
		WORDSET_RegisterWord( wk->WordSet, 2, strbuf4, 0, TRUE, PM_LANG );
		// セット
		WORDSET_ExpandStr( wk->WordSet, strbuf3, strbuf );
		gpower_app_disp_string( wk, CVS_DOWN_WIN, 0, 0, strbuf3, 0, PRINTSYS_LSB_Make(15,2,0) );
		// 破棄
		GFL_STR_DeleteBuffer( strbuf );
		GFL_STR_DeleteBuffer( strbuf2 );
		GFL_STR_DeleteBuffer( strbuf3 );
		GFL_STR_DeleteBuffer( strbuf4 );
	}
	// 足りない場合
	else{
		strbuf  = GFL_MSG_CreateString( wk->MsgData[0], gpapp_mes_tbl[5] );		// org
		strbuf3 = GFL_STR_CreateBuffer( 256, wk->heapID );						// dest
		// 必要数(0)
		strbuf4 = GFL_STR_CreateBuffer( 64, wk->heapID );						// 数値用
		STRTOOL_SetNumber( strbuf4, wk->DelTama[wk->CurPos], 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
		WORDSET_RegisterWord( wk->WordSet, 0, strbuf4, 0, TRUE, PM_LANG );
		// もっている数(1)
		STRTOOL_SetNumber( strbuf4, wk->Param->derudama_num, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
		WORDSET_RegisterWord( wk->WordSet, 1, strbuf4, 0, TRUE, PM_LANG );
		// セット
		WORDSET_ExpandStr( wk->WordSet, strbuf3, strbuf );
		gpower_app_disp_string( wk, CVS_DOWN_WIN, 0, 0, strbuf3, 0, PRINTSYS_LSB_Make(15,2,0) );
		// 破棄
		GFL_STR_DeleteBuffer( strbuf );
		GFL_STR_DeleteBuffer( strbuf3 );
		GFL_STR_DeleteBuffer( strbuf4 );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  下画面説明パネル
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_is_visible_win( GPOWER_APP_MAIN_WORK* wk, const BOOL bflg )
{
	// 表示
	if(bflg == TRUE){
		GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_ON );
	}else{
		GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_OFF );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  バッテリーチェック
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL gpower_app_chk_battery( GPOWER_APP_MAIN_WORK* wk )
{
	BOOL bflg;
	
	// 判定
	bflg = GAMESYSTEM_IsBatt10Sleep( wk->Param->gsys );
	//
#if 0	// test
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
		bflg = TRUE;
	}
#endif
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータ初期化(必要に応じて)
 *
 *	@param	GPOWER_APP_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_param( GPOWER_APP_MAIN_WORK* wk )
{
	// フェスミッション
	// =>ハイリンク非表示
	if(wk->Param->mode == GPOWER_APP_MODE_FMISSION){
		wk->Flag |= FLG_MW_NO_SELECT;
	}
	// デルタマの判定
	gpower_app_init_deltama( wk );
}

//----------------------------------------------------------------------------------
/**
 * @brief	パレットチェンジ初期化
 *
 * @param	GPOWER_APP_MAIN_WORK*	wk
 *
 * @retval
 */
//----------------------------------------------------------------------------------
static void gpower_app_init_chgpal( GPOWER_APP_MAIN_WORK* wk )
{
	// パレット取得
	NNSG2dPaletteData* pal_data;
	void* buf = GFL_ARC_UTIL_LoadPalette( ARCID_GPOWER_APP, NARC_gpower_app_gra_delpow_obj_NCLR, &pal_data, wk->heapID );
	u16* raw_data = pal_data->pRawData;
	//
	// パレットをコピー
	GFL_STD_MemCopy( &raw_data[0x10], wk->PBufStart, 0x20 );		// 1番
	GFL_STD_MemCopy( &raw_data[0x20], wk->PBufEnd,   0x20 );		// 2番
	// 破棄
	GFL_HEAP_FreeMemory( buf );
	//
	wk->PCount = 0;
	// test
	wk->Flag |= FLG_MW_PALCHG_ON;
}

//----------------------------------------------------------------------------------
/**
 * @brief	パレットチェンジ(main)
 *
 * @param	GPOWER_APP_MAIN_WORK*	wk
 *
 * @retval
 */
//----------------------------------------------------------------------------------
static void gpower_app_main_chgpal( GPOWER_APP_MAIN_WORK* wk )
{
	u8			cbuf[10];
	s32			i;
	s32			rate;
	fx16		cos;
	
	// 変更中かどうか
	if(!(wk->Flag & FLG_MW_PALCHG_ON)){
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
	// Main(2番に転送決めうち)
	NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_OBJ_PLTT_MAIN , 0x20*2, wk->PBufNow, 0x20 );
}


//----------------------------------------------------------------------------------
/**
 * @brief	決定タイマ
 *
 * @param	GPOWER_APP_MAIN_WORK*	wk
 *
 * @retval
 */
//----------------------------------------------------------------------------------
static BOOL gpower_app_set_decide_anim( GPOWER_APP_MAIN_WORK* wk )
{
	s32		i;
	s32		num;
	s32		buf[2];
	
	// 決定してない
	if(!(wk->Flag & FLG_MW_DECIDE_ANIM)){
		return( FALSE );
	}
	// 加算
	wk->Timer++;
	//
	for(i=0;i<10;i++){
		buf[0] = gpapp_time_tbl[2*i];		// time
		buf[1] = gpapp_time_tbl[2*i+1];		// frame
		//
		// 終了条件(そのフレームでは見つからなかった)
		if( buf[0] == -1){
			break;
		}
		// 見つかった
		if( buf[0] == wk->Timer ){
			// 終了のframeだった
			if(buf[1] == -1){
				return( TRUE );
			}
			else{
				// 番号切り替え
				wk->AnimNum = buf[1];
			}
			break;
		}
	}
	//
	return( FALSE );
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *  @brief   はい/いいえウインドウ 開始
 *
 *  @param  GPOWER_APP_MAIN_WORK *wk
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_yesnowin( GPOWER_APP_MAIN_WORK* wk )
{
	// 念のため破棄
	gpower_app_exit_yesnowin( wk );
	
	// はい
	wk->appitem[0].str      = GFL_MSG_CreateString( wk->MsgData[0], gpapp_mes_tbl[6] ); //メニューに表示する文字列
	wk->appitem[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
	wk->appitem[0].type     = APP_TASKMENU_WIN_TYPE_NORMAL; //戻るマークの表示
	// いいえ
	wk->appitem[1].str      = GFL_MSG_CreateString( wk->MsgData[0], gpapp_mes_tbl[7] );
	wk->appitem[1].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
	wk->appitem[1].type     = APP_TASKMENU_WIN_TYPE_RETURN;
	// メニュー作成
	wk->pAppTask[0] = APP_TASKMENU_WIN_Create( wk->pAppTaskRes, &wk->appitem[0], 16-4, 21, APP_TASKMENU_PLATE_WIDTH_YN_WIN+2, wk->heapID );
	wk->pAppTask[1] = APP_TASKMENU_WIN_Create( wk->pAppTaskRes, &wk->appitem[1], 24-2, 21, APP_TASKMENU_PLATE_WIDTH_YN_WIN+2, wk->heapID );
	//
	GFL_STR_DeleteBuffer( wk->appitem[0].str );
	GFL_STR_DeleteBuffer( wk->appitem[1].str );
	//
	// カレント
	APP_TASKMENU_WIN_SetActive( wk->pAppTask[0], TRUE );
	APP_TASKMENU_WIN_SetActive( wk->pAppTask[1], FALSE );
	//
	wk->YesNoCursor = 0;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  はい/いいえウィンドウ終了
 *
 *  @param  GPOWER_APP_MAIN_WORK	*wk
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void gpower_app_exit_yesnowin( GPOWER_APP_MAIN_WORK* wk )
{
	s32		i;
	
	// 終了処理
	for(i=0;i<2;i++){
		if(wk->pAppTask[i]){
			APP_TASKMENU_WIN_Delete( wk->pAppTask[i] );
			wk->pAppTask[i] = 0;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *  @brief  はい/いいえウィンドウ実行
 *
 *  @param  GPOWER_APP_MAIN_WORK	*wk
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void gpower_app_main_yesnowin( GPOWER_APP_MAIN_WORK* wk )
{
	s32		i;
	
	// 実行
	for(i=0;i<2;i++){
		if(wk->pAppTask[i]){
			APP_TASKMENU_WIN_Update( wk->pAppTask[i] );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *  @brief  はい/いいえウィンドウ処理待ち
 *
 *  @param  GPOWER_APP_MAIN_WORK	*wk
 *
 *  @retval BOOL					(TRUE:選択、FALSE:非選択)
 */
//-----------------------------------------------------------------------------
static BOOL gpower_app_chk_yesnowin( GPOWER_APP_MAIN_WORK* wk )
{
	s32		flg=0;
	s32		id;
	BOOL	bflg=FALSE;
	
	// 念のため
	if((wk->pAppTask[0] == 0)||(wk->pAppTask[1] == 0)){
		return( bflg );
	}
	//
	// タッチペン判定
	id = GFL_UI_TP_HitTrg( gpapp_rect_01_tbl );
	// はい
	if(id == 0){
		APP_TASKMENU_WIN_SetDecide( wk->pAppTask[0], TRUE );
		APP_TASKMENU_WIN_SetActive( wk->pAppTask[1], FALSE );
		wk->YesNoCursor = 0;
		bflg = TRUE;
		flg = 1;
		// SE
		PMSND_PlaySE( SND_GPOWER_APP_DECIDE );
	}
	// いいえ
	else if(id == 1){
		APP_TASKMENU_WIN_SetActive( wk->pAppTask[0], FALSE );
		APP_TASKMENU_WIN_SetDecide( wk->pAppTask[1], TRUE );
		wk->YesNoCursor = 1;
		bflg = TRUE;
		flg = 1;
		// SE
		PMSND_PlaySE( SND_GPOWER_APP_CANCEL );
	}
	//
	// キーの判定
	if(flg == 0){
		// 左
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
			APP_TASKMENU_WIN_SetActive( wk->pAppTask[0], TRUE );
			APP_TASKMENU_WIN_SetActive( wk->pAppTask[1], FALSE );
			wk->YesNoCursor = 0;
			// SE
			PMSND_PlaySE( SND_GPOWER_APP_CURSOR );
		}
		// 右
		else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
			APP_TASKMENU_WIN_SetActive( wk->pAppTask[0], FALSE );
			APP_TASKMENU_WIN_SetActive( wk->pAppTask[1], TRUE );
			wk->YesNoCursor = 1;
			// SE
			PMSND_PlaySE( SND_GPOWER_APP_CURSOR );
		}
		// 決定
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
			// yes
			if(wk->YesNoCursor == 0){
				APP_TASKMENU_WIN_SetDecide( wk->pAppTask[0], TRUE );
				APP_TASKMENU_WIN_SetActive( wk->pAppTask[1], FALSE );
				// SE
				PMSND_PlaySE( SND_GPOWER_APP_DECIDE );
			}
			// no
			else{
				APP_TASKMENU_WIN_SetActive( wk->pAppTask[0], FALSE );
				APP_TASKMENU_WIN_SetDecide( wk->pAppTask[1], TRUE );
				// SE
				PMSND_PlaySE( SND_GPOWER_APP_CANCEL );
			}
			bflg = TRUE;
		}
		// cancel
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
			APP_TASKMENU_WIN_SetActive( wk->pAppTask[0], FALSE );
			APP_TASKMENU_WIN_SetDecide( wk->pAppTask[1], TRUE );
			wk->YesNoCursor = 1;
			bflg = TRUE;
			// SE
			PMSND_PlaySE( SND_GPOWER_APP_CANCEL );
		}
	}
	//
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  はい/いいえ決定アニメ待ち
 *
 *  @param  GPOWER_APP_MAIN_WORK	*wk
 *
 *  @retval BOOL					(TRUE:終了、FALSE:まだ)
 */
//-----------------------------------------------------------------------------
static BOOL gpower_app_is_yesnowin_anime_end( GPOWER_APP_MAIN_WORK* wk )
{
	// 念のため
	if( wk->pAppTask[wk->YesNoCursor] == 0){
		return( TRUE );
	}
	//
	if( APP_TASKMENU_WIN_IsFinish( wk->pAppTask[wk->YesNoCursor] ) ){
		return( TRUE );
	}
	return( FALSE );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ウインドウ入力待ち(足りませんメッセージ)
 *
 *  @param  GPOWER_APP_MAIN_WORK	*wk
 *
 *  @retval BOOL					(TRUE:選択、FALSE:非選択)
 */
//-----------------------------------------------------------------------------
static BOOL gpower_app_chk_deltamawin( GPOWER_APP_MAIN_WORK* wk )
{
	BOOL	bflg=FALSE;
	
	// 決定、キャンセル
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ){
		bflg = TRUE;
	}
	// タッチをした
	if( GFL_UI_TP_GetTrg() == TRUE ){
		bflg = TRUE;
	}
	//
	return( bflg );
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *  @brief   デルタマ情報の取得
 *
 *  @param  GPOWER_APP_MAIN_WORK *wk
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void gpower_app_init_deltama( GPOWER_APP_MAIN_WORK* wk )
{
	s32		i;
	POWER_CONV_DATA*	p_data = GPOWER_PowerData_LoadAlloc( wk->heapID );
	
	// 初期化
	wk->DelTamaFlag = 0;
	for(i=0;i<3;i++){
		wk->DelTama[i] = 0;
	}
	//
	// 装備中のデルタマの情報を取得
	for(i=0;i<wk->Param->gpower_num;i++){
		wk->DelTama[i] = GPOWER_ID_to_Point( p_data, wk->Param->gpower[i] );
		//
		// 使用不可の時にフラグのbitを立てる
		// 連続使用ができないので、この判定でいいはず
		if(wk->Param->derudama_num < wk->DelTama[i]){
			wk->DelTamaFlag |= (1<<i);
		}
	}
	// 破棄
	GPOWER_PowerData_Unload( p_data );
	//
#ifdef PM_DEBUG
	for(i=0;i<3;i++){
		OS_Printf("%d : (%d,%d) => %d\n", 
			i, wk->DelTama[i], wk->Param->gpower[i], (wk->DelTamaFlag & (1<<i)));
	}
#endif
}

