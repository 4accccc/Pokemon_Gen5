//=============================================================================
/**
 *
 *	@file		t_download.c
 *	@brief  トーナメントダウンロード画面
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
#include "t_download_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"
// print
#include "ui/print_tool.h"
#include "gamesystem/msgspeed.h"

//アプリ共通素材
#include "app/app_menu_common.h"
#include "app/app_taskmenu.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"
#include "win_record_gra.naix"		// データは流用

#include "system/wipe.h"
//
#include "system/scroll_bar.h"

#include "app/app_keycursor.h"
#include "app/app_printsys_common.h"

#include "app/t_download.h"

#include "message.naix"

//=============================================================================
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

// モード
enum{
	MODE_VIEW = 0,						// 表示(削除可能)
	MODE_SELECT,						// 選択(通常)
	//
	MODE_MAX,
};

// パレット
// 上：BG (0-2:BG、3:FONT)
// 上：OBJ
// 下：BG (0,1:BG、8,9,11,12:FONT,WIN)
// 下：OBJ(0-10:パネル、11-14:cmn)
enum{
	// 何本目か
	PLTID_SYSWIN_BASE			= 8,	// font(syswin)
	PLTID_SYSWIN_FONT			= 9,	// font(syswin)
	PLTID_YESNO_WIN_BASE		= 11,	// win(yes/no window)(注：2本使用する(色変え含む:11,12))
	//
	PLTID_UP_FONT				= 3,	// font(up)
	//
	// (obj)
	PLTID_OBJ_CMN				= 11,	// cmnのpalのoffset
	PLTID_OBJ_CMN_NUM			= 4,	// cmnのpalの本数
	//
	// 何番目か
	MES_CLEAR_COLOR				= 0,	// (通常)クリアカラー
	MES_SYSWIN_CLEAR_COLOR		= 15,	// (システムウインドウの)クリアカラー
};

// 最大値
enum{
	T_DOWNLOAD_CANVAS_MAXNUM 	= 9,	// canvas
	T_DOWNLOAD_BMPOAM_MAXNUM	= 4,	// bmpoam
	T_DOWNLOAD_EMPTY_MAXNUM		= 17*2,	// empty
	//
	T_DOWNLOAD_CMN_OBJ_MAXNUM	= 4,	// common(obj)
	T_DOWNLOAD_OBJ_MAXNUM		= 6,	// obj
	//
	T_DOWNLOAD_BGNUM_MAXNUM		= 8,	// bg
	//
//	T_DOWNLOAD_UP_OBJ_MAXNUM	= 1,	// obj(up)
	T_DOWNLOAD_MSG_MAX			= 2,	// message
};


// OBJのリソースのインデックス
enum{
	// common
	RESID_CMN_OBJ_PLT = 0,				// パレット
	RESID_CMN_OBJ_CHR,					// キャラ
	RESID_CMN_OBJ_CEL,					// セルアニメ
	// 通常
	RESID_OBJ_PLT,						// パレット
	RESID_OBJ_CHR,						// キャラ
	RESID_OBJ_CEL,						// セルアニメ
	// 通常(up)
//	RESID_UP_OBJ_PLT,					// パレット
//	RESID_UP_OBJ_CHR,					// キャラ
//	RESID_UP_OBJ_CEL,					// セルアニメ
	//
	RESID_OBJ_MAX,
};

// CLWK(下画面)
enum
{
	CLWKID_PANEL_00 = 0,				// パネル(00)
	CLWKID_PANEL_01,					// パネル
	CLWKID_PANEL_02,					// パネル
	CLWKID_PANEL_03,					// パネル
	CLWKID_BAR,							// スクロールバー
	CLWKID_CURSOR,						// カーソル(仮)
	//
	CLWKID_MAX,
};

// empty
enum{
	EMPTY_PANEL_OFFSET 	= 5,			// パネルのオフセット
	//
	EMPTY_PANEL_POS4	= 0,			// パネルの座標(4段)
	EMPTY_PANEL_POS3	= 14,			// パネルの座標(3段)
	//
//	EMPTY_PLAYER_NAME	= 11,
//	EMPTY_TITLE_NAME	= 12,
	//
//	EMPTY_SCROLL_POS	= 4,
};

// CLWK(下画面共通)
enum{
	CLWKID_CMN_LEFT = 0,				// 左
	CLWKID_CMN_RIGHT,					// 右
	CLWKID_CMN_CROSS,					// ×
	CLWKID_CMN_RETURN,					// 戻る
	//
	CLWKID_CMN_MAX,
};

// clwork
enum{
	CLWKMODE_NORMAL = 0,				// 通常(down)
	CLWKMODE_CMN,						// 共通(down)
	//
//	CLWKMODE_UP,						// トレーナー(up)
};

// canvas
enum{
	// 上側
	CVS_TITLE = 0,						// タイトル
	CVS_DATE,							// 配信日
	CVS_RULE_TITLE,						// ルールタイトル
	CVS_RULE_EXPLAIN,					// ルール説明
	CVS_INFO_TITLE,						// 内容タイトル
	CVS_INFO_EXPLAIN,					// 内容説明
	//
	CVS_UP_MES_MAX,						// 上側メッセージ最大
	//
	// 下側
	CVS_MAIN_TITLE = CVS_UP_MES_MAX,	// タイトル
	CVS_MAIN_MESWIN,					// メッセージwin
	CVS_MAIN_CENTER_WIN,				// 中央win
	//
	CVS_MAIN_MES_MAX,					// 下側メッセージ最大
};

// シーケンス
enum{
	CMN_LEFT_SEQ 		= 4,			// 左
	CMN_RIGHT_SEQ 		= 5,			// 右
	CMN_CROSS_SEQ 		= 0,			// ×
	CMN_RETURN_SEQ 		= 1,			// 戻る
	//
	CMN_LEFT_ON_SEQ 	= 12,			// 左オン
	CMN_LEFT_OFF_SEQ 	= 18,			// 左オフ
	CMN_RIGHT_ON_SEQ 	= 13,			// 右オン
	CMN_RIGHT_OFF_SEQ 	= 19,			// 右オフ
	//
	CMN_CROSS_ON_SEQ	= 8,			// ×オン
	CMN_RETURN_ON_SEQ	= 9,			// 戻るオン
};

// パネルのシーケンス(仮)
enum{
	PANEL_NO_READ_SEQ	= 0,			// 未読
	PANEL_READ_SEQ		= 1,			// 既読
	PANEL_SAVE_SEQ		= 2,			// セーブ済み
};

// 最大など
enum{
	SAVE_BUFFER_MAX		= 3,			// セーブ個数最大
	//
	ITEM_BUFFER_MAX		= 30,			// バッファ最大数
	//
	PANEL_DISP4			= 4,			// パネル表示(4枚)
	PANEL_DISP3			= 3,			// パネル表示(3枚)
};

// タッチペンエリア
// (excelのテーブルと対応)
enum{
	AREA_TD_TP_LEFT = 0,				// 左矢印
	AREA_TD_TP_RIGHT,					// 右矢印
	AREA_TD_TP_CROSS,					// ×
	AREA_TD_TP_RETURN,					// RETURN
	AREA_TD_TP_SCROLL,					// スクロール領域
	//
	AREA_TD_TP_CMN_MAX,					// ここまではデータにセット
	//
	AREA_TD_TP_PANEL3_0 = AREA_TD_TP_CMN_MAX, // パネル3行
	AREA_TD_TP_PANEL3_1,
	AREA_TD_TP_PANEL3_2,
	//
	AREA_TD_TP_PANEL4_0,				// パネル4行
	AREA_TD_TP_PANEL4_1,
	AREA_TD_TP_PANEL4_2,
	AREA_TD_TP_PANEL4_3,
	//
	AREA_TD_TP_MAX,
};

// いろいろ
enum{
	TD_ACTIVE_RATE		= 0,			// アクティブの割合(戻し)
	TD_PASSIVE_RATE 	= -8,			// パッシブの割合
};

// キー判定
enum{
	KEY_INPUT_BEFORE = 0,				// 前
	KEY_INPUT_AFTER,					// 後
};

// スクロール
enum{
	SCRL_TOP_Y			= 32,			// レール上部のY座標
	SCRL_DOWN_Y			= 160,			// レール下部のY座標
	SCRL_BAR_SIZE		= 16,			// バーのサイズ
};

//-------------------
// フラグ
//-------------------
// OBJのフラグ(T_DOWNLOAD_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM		= (1<<0),		// アニメあり
	FLG_OBJ_HFLIP		= (1<<1),		// HFLIP
	FLG_OBJ_VFLIP		= (1<<2),		// VFLIP
};

// T_DOWNLOAD_MAIN_WORK.Flag
enum{
	FLG_MW_ANM_LEFT		= (1<<0),		// 左矢印アニメ
	FLG_MW_ANM_RIGHT	= (1<<1),		// 右矢印アニメ
	FLG_MW_ANM_CROSS	= (1<<2),		// ×アニメ
	FLG_MW_ANM_RETURN	= (1<<3),		// 戻るアニメ
	//
	FLG_MW_DISP3		= (1<<4),		// パネル3個表示
	FLG_MW_UPDATE		= (1<<5),		// 更新
	FLG_MW_NOITEM		= (1<<6),		// パネルなし(全削除状態)
	FLG_MW_NOITEM_MES	= (1<<7),		// ありませんメッセージ表示
	//
	FLG_MW_LEFT_HIDE	= (1<<8),		// 左矢印
	FLG_MW_RIGHT_HIDE	= (1<<9),		// 右矢印
	FLG_MW_CROSS_HIDE	= (1<<10),		// ×
	FLG_MW_RETURN_HIDE	= (1<<11),		// RETURN
	//
	FLG_MW_RET_END		= (1<<12),		// RETURNで終了
	FLG_MW_CROSS_END	= (1<<13),		// ×で終了
	FLG_MW_BAR_UPDATE	= (1<<14),		// バー更新
	FLG_MW_BAR_HIDE		= (1<<15),		// バー非表示
	//
	FLG_MW_END			= (1<<20),		// 終了
	//
	// アニメまとめ
	FLG_MW_ANM_ALL		= (FLG_MW_ANM_LEFT|FLG_MW_ANM_RIGHT|FLG_MW_ANM_CROSS|FLG_MW_ANM_RETURN),
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
	GFL_CLWK	*cmn_clwk[T_DOWNLOAD_CMN_OBJ_MAXNUM];	// CLWK(common)
	// 通常
	GFL_CLWK	*clwk[T_DOWNLOAD_OBJ_MAXNUM];			// CLWK
	//
//	GFL_CLWK	*up_clwk[WIN_RECORD_UP_OBJ_MAXNUM];		// CLWK(up)
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
	s32						SaveBuffer[SAVE_BUFFER_MAX];	// セーブ領域
	T_DOWNLOAD_ITEM			ItemBuffer[ITEM_BUFFER_MAX];	// アイテム領域
	//
	s16						*EmptyBuffer;					// 座標用のバッファ(必ず破棄すること)
	s16						*ParamBuffer;					// パラメータのバッファ(必ず破棄すること)
	//
	GFL_UI_TP_HITTBL		HitBuffer[AREA_TD_TP_MAX+1];	// タッチペン用(+1は終了コードを入れるため)
} T_DOWNLOAD_MAIN_WORK;

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

//=============================================================================
/**
 *							データ定義(要整理)
 */
//=============================================================================
//---------------
// BG関連
//---------------
// BGPAL
static const T_DOWNLOAD_BGPAL_DATA td_bgpal_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_win_record_gra_win_record_bg_u_NCLR,	0,	3,},		// 上、BG
	{ PALTYPE_MAIN_BG,	NARC_win_record_gra_win_record_bg_d_NCLR,	0,	2,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const T_DOWNLOAD_BGCHR_DATA td_bgchr_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_win_record_gra_win_record_bg_u_NCGR,	0,	0,},		// 上、BG
	{ GFL_BG_FRAME3_M,	NARC_win_record_gra_win_record_bg_d_NCGR,	0,	0,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const T_DOWNLOAD_BGSCN_DATA td_bgscn_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_win_record_gra_win_record_bg_u_01_NSCR,	0,	0,},		// 上、下地
	{ GFL_BG_FRAME2_S,	NARC_win_record_gra_win_record_bg_u_02_NSCR,	0,	0,},		// 上、中間
	{ GFL_BG_FRAME3_M,	NARC_win_record_gra_win_record_bg_d_01_NSCR,	0,	0,},		// 下、下地
	{ GFL_BG_FRAME2_M,	NARC_win_record_gra_win_record_bg_d_02_NSCR,	0,	0,},		// 下、下地

	{ 0xff, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(通常)
//------------------
// narc, resid, paloffset, palnum
static const s16 td_obj_main_info_tbl[] = {
	NARC_win_record_gra_win_record_obj_d_NCLR, RESID_OBJ_PLT, 0, 11, // NCL
	NARC_win_record_gra_win_record_obj_d_NCGR, RESID_OBJ_CHR, 0, 0,	// NCG
	NARC_win_record_gra_win_record_obj_d_NCER, RESID_OBJ_CEL, 0, 0, // NCE
	NARC_win_record_gra_win_record_obj_d_NANR, -1, 0, 0,			// NAR?
};

// 情報
static const s16 td_obj_main_itbl[] = {
	RESID_OBJ_PLT,
	RESID_OBJ_CHR,
	RESID_OBJ_CEL,
	//
	T_DOWNLOAD_OBJ_MAXNUM,
	CLWKMODE_NORMAL,
};

// OBJのテーブルデータ(通常)
static const T_DOWNLOAD_OBJ_DATA td_obj_tbl[] = {
	{ 0,	0x30,	2,	1,	0,	1,	0,	0,},		// パネル0
	{ 1,	0x30,	2,	1,	0,	1,	1,	0,},		// パネル1
	{ 2,	0x30,	2,	1,	0,	1,	2,	0,},		// パネル2
	{ 3,	0x30,	2,	1,	0,	1,	3,	0,},		// パネル3
	{ 19,	0x30,	2,	1,	0,	1,	4,	0,},		// ゲージ上
	{ 19,	0x28,	2,	1,	0,	1,	4,	0,},		// 選択カーソル(仮)

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(cmn)
//------------------
// 情報
static const s16 td_obj_cmn_itbl[] = {
	RESID_CMN_OBJ_PLT,
	RESID_CMN_OBJ_CHR,
	RESID_CMN_OBJ_CEL,
	//
	T_DOWNLOAD_CMN_OBJ_MAXNUM,
	CLWKMODE_CMN,
};

// OBJのテーブルデータ(cmn)
static const T_DOWNLOAD_OBJ_DATA td_obj_cmn_tbl[] = {
	{ 4,	0x30,	2,	1,	0,	1,	7,	0,},		// 左マーク
	{ 5,	0x30,	2,	1,	0,	1,	8,	0,},		// 右マーク
	{ 0,	0x30,	2,	1,	0,	1,	9,	0,},		// ×マーク
	{ 1,	0x30,	2,	1,	0,	1, 10,	0,},		// 戻るマーク

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// BMPOAMの情報
//------------------
static const T_DOWNLOAD_BMPOAM_DATA td_bmpoam_tbl[] = {
	{ 24,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル00
	{ 24,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル01
	{ 24,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル02
	{ 24,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル03

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//---------------
// CANVASの設定
//---------------
// (printutilと対応)
static const T_DOWNLOAD_CANVAS_DATA td_canvas_tbl[] = {
	{ GFL_BG_FRAME1_S,	1,	1,	30,	2,	3,},		// 上：配信日トーナメントリスト
	{ GFL_BG_FRAME1_S,	10,	4,	20,	2,	3,},		// 上：配信日
	{ GFL_BG_FRAME1_S,	1,	7,	30,	2,	3,},		// 上：ルール名
	{ GFL_BG_FRAME1_S,	1,	10,	28,	6,	3,},		// 上：レベル制限
	{ GFL_BG_FRAME1_S,	1,	17,	18,	2,	3,},		// 上：トーナメント名
	{ GFL_BG_FRAME1_S,	1,	20,	28,	4,	3,},		// 上：内容
	//
	{ GFL_BG_FRAME1_M,	1,	1,	30,	2,	9,},		// 下：タイトル
	{ GFL_BG_FRAME0_M,	1,	1,	30,	4,	9,},		// 下：meswin
	{ GFL_BG_FRAME0_M,	1,	10,	30,	4,	9,},		// 下：中央win

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

//---------------------------------
// 共通ボタン
// (通常、オン、オフ(操作不可)、0)
// 全部は必要ないが、念のため用意しておく
//---------------------------------
static const s16 td_cmn_btn_tbl[] = {
	CMN_LEFT_SEQ,   CMN_LEFT_ON_SEQ,   CMN_LEFT_OFF_SEQ,  0,	// 左
	CMN_RIGHT_SEQ,  CMN_RIGHT_ON_SEQ,  CMN_RIGHT_OFF_SEQ, 0,	// 右
	CMN_CROSS_SEQ,  CMN_CROSS_ON_SEQ,  CMN_CROSS_SEQ,     0,	// ×
	CMN_RETURN_SEQ, CMN_RETURN_ON_SEQ, CMN_RETURN_SEQ,    0,	// 戻る
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT TDownloadProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT TDownloadProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT TDownloadProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
///	以下に追加
//=====================================
static void td_init( T_DOWNLOAD_MAIN_WORK* wk );
static void td_main( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit( T_DOWNLOAD_MAIN_WORK* wk );
static void td_up(   T_DOWNLOAD_MAIN_WORK* wk );
static void td_down( T_DOWNLOAD_MAIN_WORK* wk );
//
static void td_key_view( T_DOWNLOAD_MAIN_WORK* wk );
static void td_key_select( T_DOWNLOAD_MAIN_WORK* wk );
//
static void td_main_print( T_DOWNLOAD_MAIN_WORK *wk );
//
static s16  td_get_empty( T_DOWNLOAD_MAIN_WORK *wk, const s32 num );
//
static void td_init_bg( T_DOWNLOAD_MAIN_WORK* wk );
//
static void td_init_obj_clwk( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const T_DOWNLOAD_OBJ_DATA *obj_adr, const s16 *info_adr );
static void td_init_obj_res( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr );
static void td_init_obj_res_cmn( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype );
static void td_init_obj( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit_obj( T_DOWNLOAD_MAIN_WORK* wk );
//
static void td_chg_objseq( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
static void td_set_obj_visible( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
static BOOL td_is_anime_end( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum );
//
static void td_init_obj_bmpoam( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const T_DOWNLOAD_BMPOAM_DATA *bmp_adr, const s16 *info_adr );
static void td_disp_bmpoam( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const STRBUF *strbuf );
//
// 文字列関連
static void td_init_string( T_DOWNLOAD_MAIN_WORK* wk );
static void td_disp_string( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col);
static void td_disp_string_number( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 num, const s32 lnum, const u32 pmode );
static void td_update_string( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
static void td_exit_string( T_DOWNLOAD_MAIN_WORK* wk );
static void td_clear_canvas( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
//
// 情報表示
static void td_disp_upinfo( T_DOWNLOAD_MAIN_WORK* wk, const s32 num );
static void td_disp_downinfo( T_DOWNLOAD_MAIN_WORK* wk );
static void td_set_panel( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 smode, const BOOL bflg );
static void td_set_cursor( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 smode, const BOOL bflg );
static void td_param_cmn_update( T_DOWNLOAD_MAIN_WORK* wk );
static void td_param_view_update( T_DOWNLOAD_MAIN_WORK* wk );
static void td_param_select_update( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_select_key_input( T_DOWNLOAD_MAIN_WORK* wk );
static void td_set_cmn_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
static BOOL td_chk_view_key_input( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
static void td_init_param( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_view_tp_input( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_select_tp_input( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_my_tp( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 num );
static BOOL td_set_scrollbar( T_DOWNLOAD_MAIN_WORK* wk );
static void td_calc_scrollbar_pos( T_DOWNLOAD_MAIN_WORK* wk, const s32 num );

//
// バッファ
static s32  td_calc_savebuffer( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_savebuffer( T_DOWNLOAD_MAIN_WORK* wk, const s32 num );
static s32  td_del_savebuffer( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum );
static void td_chk_delete( T_DOWNLOAD_MAIN_WORK* wk );
//
// yes/noウインドウ用
static void td_init_yesnowin( T_DOWNLOAD_MAIN_WORK* wk );
static void td_exit_yesnowin( T_DOWNLOAD_MAIN_WORK* wk );
static s32  td_chk_yesnowin( T_DOWNLOAD_MAIN_WORK* wk );
//
// meswin用
static void td_set_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mesid );
static BOOL td_main_meswin( T_DOWNLOAD_MAIN_WORK* wk );
static void td_init_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
static void td_clear_frame( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_update_printstream( T_DOWNLOAD_MAIN_WORK* wk );
static void td_disp_centermes( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mesid );
//
// item
static void td_set_item( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 mode );
static s32  td_calc_itembuffer( T_DOWNLOAD_MAIN_WORK* wk );
static BOOL td_chk_add_item( T_DOWNLOAD_MAIN_WORK* wk );
//
//
static void td_chg_brightness( T_DOWNLOAD_MAIN_WORK* wk, const s32 rate );
//
// タッチペン
static void td_get_tp_area( T_DOWNLOAD_MAIN_WORK* wk, const s32 num, GFL_UI_TP_HITTBL *tpbuffer );
//

//------------------
// 関数のテーブル
//------------------
static void (*(Td_KeyFuncTbl[]))(T_DOWNLOAD_MAIN_WORK*) = {
	td_key_view,			// view
	td_key_select,			// select
};

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA TDownloadProcData = 
{
	TDownloadProc_Init,
	TDownloadProc_Main,
	TDownloadProc_Exit,
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
 *	@param	*pwk					T_DOWNLOAD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TDownloadProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	T_DOWNLOAD_MAIN_WORK 	*wk;
	
	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UI_DEBUG, UI_TEMPLATE_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(T_DOWNLOAD_MAIN_WORK), HEAPID_UI_DEBUG );
	GFL_STD_MemClear( wk, sizeof(T_DOWNLOAD_MAIN_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_UI_DEBUG;
	
	// 参照用
	wk->Param = pwk;
	
	//描画設定初期化(上:サブ、下:メイン)	
	wk->graphic	= T_DOWNLOAD_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );
	
	//フォント作成
	wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	
	// メッセージ(test)
//	wk->msg[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_trmsg_dat, wk->heapID );
	wk->msg[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_tournament_dat, wk->heapID );
	wk->msg[1] = 0;
	
	//PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	//
	// 会話ウインドウ用
	wk->MsgTcblSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 32 , 32 );
	wk->PrintStream = 0;
	wk->MsgBuffer 	= 0;
	
	// yes/noウインドウ用
	wk->pAppTask 	= 0;
	wk->pAppTaskRes = APP_TASKMENU_RES_Create( GFL_BG_FRAME1_M, PLTID_YESNO_WIN_BASE, wk->font, wk->print_que, wk->heapID );
	
	//----------------
	// 処理を記述
	//----------------
	td_init( wk );
	
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
 *	@param	*pwk					T_DOWNLOAD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TDownloadProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	s32		i;
	T_DOWNLOAD_MAIN_WORK* wk = mywk;
	
	//----------------
	// 処理を記述
	//----------------
	td_exit( wk );
	
	// yes/no
	APP_TASKMENU_RES_Delete( wk->pAppTaskRes );
	
	//メッセージ破棄
	for(i=0;i<T_DOWNLOAD_MSG_MAX;i++){
		if( wk->msg[i] ){
			GFL_MSG_Delete( wk->msg[i] );
			wk->msg[i] = 0;
		}
	}
	
	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	//
	//FONT
	GFL_FONT_Delete( wk->font );
	// 会話ウインドウ用
	GFL_TCBL_Exit( wk->MsgTcblSys );
	
	//描画設定破棄
	T_DOWNLOAD_GRAPHIC_Exit( wk->graphic );
	
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
 *	@param	*pwk					T_DOWNLOAD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TDownloadProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	T_DOWNLOAD_MAIN_WORK* wk = mywk;
	
	switch ( *seq )
	{
		case 0:
			if (WIPE_SYS_EndCheck())
			{
				(*seq) ++;
			}
			break;
		case 1:
			// 終了判定
			if(wk->Step == STEP_END){
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
	td_main( wk );
	
	// 会話ウインドウ用
	GFL_TCBL_Main( wk->MsgTcblSys );
	
	//PRINT_QUE
	td_main_print( wk );
//	PRINTSYS_QUE_Main( wk->print_que );
	
	//2D描画
	T_DOWNLOAD_GRAPHIC_2D_Draw( wk->graphic );
	
	//3D描画
	T_DOWNLOAD_GRAPHIC_3D_StartDraw( wk->graphic );
	
	T_DOWNLOAD_GRAPHIC_3D_EndDraw( wk->graphic );
	
	return GFL_PROC_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(init)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32			i;
	
	// バッファを確保
	wk->MsgBuffer = GFL_STR_CreateBuffer( 256, wk->heapID );
	
	// 初期化
	// それほど複雑ではないので、
	// 上、下画面をまとめて処理
	wk->Step 		= STEP_INIT;
	wk->Mode		= wk->Param->status;	// 分岐
	wk->Flag		= 0;
	wk->CurPos		= 0;
	wk->TopNum		= 0;
	wk->SubNum		= 0;
	//
	for(i=0;i<SAVE_BUFFER_MAX;i++){
		wk->SaveBuffer[i] = -1;
	}
	// test
	wk->SaveBuffer[0] = 0;
	wk->SaveBuffer[1] = 2;
	wk->SaveBuffer[2] = 3;
	
	for(i=0;i<ITEM_BUFFER_MAX;i++){
		td_set_item( wk, i, 0xff, 0 );
	}
	// test
	for(i=0;i<10;i++){
		td_set_item( wk, i, i, 0 );
	}
	
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_WIN_RECORD_GRA, NARC_win_record_gra_t_download_pos_tblbin, wk->heapID );
	// パラメータ用のバッファ取得(必ず破棄すること)
	wk->ParamBuffer = GFL_ARC_LoadDataAlloc( ARCID_WIN_RECORD_GRA, NARC_win_record_gra_t_download_param_tblbin, wk->heapID );
	
	//---------------
	// bgの初期化
	//---------------
	td_init_bg( wk );
	
	//---------------
	// 文字初期化
	//---------------
	td_init_string( wk );
	
	//------------
	// OBJ初期化
	//------------
	td_init_obj( wk );
	
	//-------------
	// 後処理
	//-------------
	td_disp_downinfo( wk );
	//
	//--------------
	// 内部パラメータセット
	//---------------
	td_init_param( wk );
	//
	wk->Flag |= FLG_MW_UPDATE;
	// 表示モードの場合
	if(wk->Mode == MODE_VIEW){
		wk->Flag |= FLG_MW_DISP3;
		wk->Flag |= (FLG_MW_LEFT_HIDE|FLG_MW_RIGHT_HIDE);
		wk->Flag |= FLG_MW_BAR_HIDE;
		//
		td_param_view_update( wk );
		// バー非表示
		td_set_obj_visible( wk, CLWKMODE_NORMAL, CLWKID_BAR, FALSE );
	}
	// 通常モードの場合
	else{
		wk->Flag &= ~FLG_MW_BAR_HIDE;
		//
		wk->SaveBuffer[0] = -1;
		wk->SaveBuffer[1] = -1;
		wk->SaveBuffer[2] = -1;
		// 必要に応じて
		td_param_select_update( wk );
		// バー非表示
		td_set_obj_visible( wk, CLWKMODE_NORMAL, CLWKID_BAR, TRUE );
	}
	
	//--------------
	// meswin
	//--------------
	//
	// 汎用PrintStream用
	APP_PRINTSYS_COMMON_PrintStreamInit( &wk->PrintSysWork, APP_PRINTSYS_COMMON_TYPE_BOTH );
//	APP_PRINTSYS_COMMON_PrintStreamInit( &wk->PrintSysWork, APP_PRINTSYS_COMMON_TYPE_THROUGH );
	// メッセージ送りマーク用
//	wk->MsgCursorWork = APP_KEYCURSOR_Create( 15, TRUE, TRUE, wk->heapID );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_main( T_DOWNLOAD_MAIN_WORK* wk )
{
	// 上画面
	td_up( wk );
	// 下画面
	td_down( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(exit)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_exit( T_DOWNLOAD_MAIN_WORK* wk )
{
	//-----------------
	// 終了パラメータ
	//------------------
	// ×で終了 => フリームーブとなる
	if(wk->Flag & FLG_MW_CROSS_END){
		if(wk->Param->result_work != NULL){
			*wk->Param->result_work = TRUE;
		}
	}
	// RETURNで終了 => パソコン画面へ
	else{
		if(wk->Param->result_work != NULL){
			*wk->Param->result_work = FALSE;
		}
	}
#if 1	// 結果
	if(wk->Param->result_work != NULL){
		OS_Printf("result : %d\n",*wk->Param->result_work);
	}else{
		OS_Printf("result : null(debug)\n");
	}
#endif
	// 上、下画面をまとめて処理
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	// paramのバッファクリア
	GFL_HEAP_FreeMemory( wk->ParamBuffer );
	//
	// yes/no
	td_exit_yesnowin( wk );
	//
	// 文字表示用
	td_exit_string( wk );
	//
	// OBJ破棄
	td_exit_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(上画面)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_up( T_DOWNLOAD_MAIN_WORK* wk )
{
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(下画面)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_down( T_DOWNLOAD_MAIN_WORK* wk )
{
	// 念のため
	if((wk->Mode < 0)||(wk->Mode >= MODE_MAX)){
		OS_Printf("mode error\n");
		return;
	}
	// 分岐
	Td_KeyFuncTbl[wk->Mode]( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理(View)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_key_view( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	BOOL	bflg;
	
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
			// アニメチェック
			td_chk_anim( wk, 0 );
			// キー入力(view)
			bflg = td_chk_view_key_input( wk );
			if(bflg == FALSE){
				// タッチペン(view)
				td_chk_view_tp_input( wk );
			}
			// 更新(view)
			td_param_view_update( wk );
			break;
		//
		// LV2(meswin)
		case STEP_LV2:
			// meswin
			if( td_main_meswin( wk ) == FALSE ){
				// yes/noウインドウ
				td_init_yesnowin( wk );
				//
				wk->Step = STEP_LV2+10;
			}
			break;
		//
		// LV2+10(yes/no)
		case STEP_LV2+10:
			// yes/noの処理(update)
			if( wk->pAppTask ){
				APP_TASKMENU_UpdateMenu( wk->pAppTask );
			}
			// yes/noウインドウの選択処理
			tmp = td_chk_yesnowin( wk );
			if(tmp != -1){
				wk->Step = STEP_LV1;
				if(tmp == 0){		// NOの処理
					
				}else if(tmp == 1){	// YESの処理
					// 削除
					td_chk_delete( wk );
				}
				// 輝度戻し(test)
				td_chg_brightness( wk, TD_ACTIVE_RATE );
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
 *	@brief  キー入力処理(Select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_key_select( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	BOOL	bflg;
	
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
			// アニメチェック
			td_chk_anim( wk, 0 );
			// キー入力(select)
			bflg = td_chk_select_key_input( wk );
			if(bflg == FALSE){
				td_chk_select_tp_input( wk );
			}
			// 更新(select)
			td_param_select_update( wk );
			break;
		//
		// LV2(meswin)
		case STEP_LV2:
			// meswin
			if( td_main_meswin( wk ) == FALSE ){
				// yes/noウインドウ
				td_init_yesnowin( wk );
				//
				wk->Step = STEP_LV2+10;
			}
			break;
		//
		// LV2+10(yes/no)
		case STEP_LV2+10:
			// yes/noの処理(update)
			if( wk->pAppTask ){
				APP_TASKMENU_UpdateMenu( wk->pAppTask );
			}
			// yes/noウインドウの選択処理
			tmp = td_chk_yesnowin( wk );
			if(tmp != -1){
				wk->Step = STEP_LV1;
				if(tmp == 0){			// NOの処理
				}
				else if(tmp == 1){		// YESの処理
					// 登録
					bflg = td_chk_add_item( wk );
					// セーブできない=>3つセーブされている
					if(bflg == FALSE){
						wk->Step = STEP_LV3;
						wk->CurPos = 0;
						wk->Flag |= FLG_MW_DISP3;
						wk->Flag |= FLG_MW_UPDATE;
						wk->Flag |= (FLG_MW_RIGHT_HIDE|FLG_MW_LEFT_HIDE);
						wk->Flag |= FLG_MW_BAR_HIDE;
					}
				}
				// 輝度戻し(test)
				td_chg_brightness( wk, TD_ACTIVE_RATE );
			}
			break;
		//
		// LV3(上書きセーブ画面)
		case STEP_LV3:
			// アニメチェック
			td_chk_anim( wk, 1 );
			// キー入力(select)
			bflg = td_chk_select_key_input( wk );
			if(bflg == FALSE){
				td_chk_select_tp_input( wk );
			}
			// 更新(select)
			td_param_select_update( wk );
			break;
		//
		// LV4(上書きセーブ)
		case STEP_LV4:
			// meswin
			if( td_main_meswin( wk ) == FALSE ){
				// yes/noウインドウ
				td_init_yesnowin( wk );
				//
				wk->Step = STEP_LV4+10;
			}
			break;
		//
		// LV4+10(yes/no)
		case STEP_LV4+10:
			// yes/noの処理(update)
			if( wk->pAppTask ){
				APP_TASKMENU_UpdateMenu( wk->pAppTask );
			}
			// yes/noウインドウの選択処理
			tmp = td_chk_yesnowin( wk );
			if(tmp != -1){
				if(tmp == 0){			// NOの処理
					wk->Step = STEP_LV3;
				}
				else if(tmp == 1){		// YESの処理
					wk->Flag |= FLG_MW_UPDATE;
					wk->Flag &= ~FLG_MW_DISP3;
					wk->Flag &= ~(FLG_MW_RIGHT_HIDE|FLG_MW_LEFT_HIDE);
					wk->Flag &= ~FLG_MW_BAR_HIDE;
					wk->Step = STEP_LV1;
					// アイテム戻し
					tmp = wk->SaveBuffer[wk->CurPos];
					td_set_item( wk, tmp, tmp, PANEL_NO_READ_SEQ );
					// セット
					tmp = wk->TopNum+wk->SubNum;
					wk->SaveBuffer[wk->CurPos] = tmp;
					td_set_item( wk, tmp, tmp, PANEL_SAVE_SEQ );
				}
				// 輝度戻し(test)
				td_chg_brightness( wk, TD_ACTIVE_RATE );
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
 *	@brief  Queの処理
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_main_print( T_DOWNLOAD_MAIN_WORK *wk )
{
	s32		i;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<T_DOWNLOAD_CANVAS_MAXNUM;i++){
		PRINT_UTIL_Trans( &wk->PrintUtil[i], wk->print_que );
	}
}

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
static s16 td_get_empty( T_DOWNLOAD_MAIN_WORK *wk, const s32 num )
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
	handle	= GFL_ARC_OpenDataHandle( ARCID_WIN_RECORD_GRA, wk->heapID );
	
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
	if(buf[4] == CLWKMODE_NORMAL){			// 通常
		clwk = &wk->wk_obj.clwk[0];
	}
	else if(buf[4] == CLWKMODE_CMN){		// cmn
		clwk = &wk->wk_obj.cmn_clwk[0];
	}
//	else{									// up
//		clwk = &wk->wk_obj.up_clwk[0];
//	}
	
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		pos[0] = td_get_empty(wk, tmp*2);
		pos[1] = td_get_empty(wk, tmp*2+1);
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
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_WIN_RECORD_GRA, wk->heapID );
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
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj( T_DOWNLOAD_MAIN_WORK* wk )
{
    GFL_CLUNIT*	p_clunit = T_DOWNLOAD_GRAPHIC_GetClunit( wk->graphic );
	// リソース読み込み(通常)
	td_init_obj_res( wk, CLSYS_DRAW_MAIN, &td_obj_main_info_tbl[0] );
	// リソース読み込み(cmn)
	td_init_obj_res_cmn( wk, CLSYS_DRAW_MAIN );
	// リソース読み込み(up)
//	td_init_obj_res_up( wk, CLSYS_DRAW_SUB, wk->TrainerId );
	// CLWK登録(通常)
	td_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &td_obj_tbl[0], &td_obj_main_itbl[0] );
	// CLWK登録(cmn)
	td_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &td_obj_cmn_tbl[0], &td_obj_cmn_itbl[0] );
	// CLWK登録(up)
//	td_init_obj_clwk( wk, CLSYS_DRAW_SUB, &td_obj_up_tbl[0], &td_obj_up_itbl[0] );
	//
	// BmpOAM初期化
	wk->BmpOamSys = BmpOam_Init( wk->heapID, p_clunit );
	// BMPOAM登録
	td_init_obj_bmpoam( wk, CLSYS_DRAW_MAIN, &td_bmpoam_tbl[0], &td_obj_main_info_tbl[0] );
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
//	for(i=0;i<WIN_RECORD_UP_OBJ_MAXNUM;i++){
//		GFL_CLACT_WK_Remove( wk->wk_obj.up_clwk[i] );
//	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_OBJ_CEL] );
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_CMN_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_CMN_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_CMN_OBJ_CEL] );
	//リソース破棄 
//	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_UP_OBJ_PLT] );
//	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_UP_OBJ_CHR] );
//	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_UP_OBJ_CEL] );
	//
	// BmpOAM後処理
	for(i=0;i<T_DOWNLOAD_BMPOAM_MAXNUM;i++){
		BmpOam_ActorDel( wk->BmpOamWork[i] );
		GFL_BMP_Delete( wk->BmpData[i] );
	}
	BmpOam_Exit( wk->BmpOamSys );
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
static void td_chg_objseq( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	if(wmode == CLWKMODE_NORMAL){			// 通常
		if(wnum >= CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
	}
	else if(wmode == CLWKMODE_CMN){			// 共通
		if(wnum >= CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.cmn_clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.cmn_clwk[wnum], TRUE );
	}
//	else if(wmode == CLWKMODE_UP){			// 上画面
//		if(wnum >= CLWKID_UP_MAX){
//			OS_Printf("up obj max error:%d\n",wnum);
//			return;
//		}
//		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.up_clwk[wnum], seq );
//		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.up_clwk[wnum], TRUE );
//	}
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
static void td_set_obj_visible( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// 表示/非表示
	if(wmode == CLWKMODE_NORMAL){		// 通常
		if(wnum >= CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
	}
	else if(wmode == CLWKMODE_CMN){		// 共通
		if(wnum >= CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.cmn_clwk[wnum], bflg );
	}
//	else if(wmode == CLWKMODE_UP){		// up
//		if(wnum >= CLWKID_UP_MAX){
//			OS_Printf("up obj max error:%d\n",wnum);
//			return;
//		}
//		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.up_clwk[wnum], bflg );
//	}
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
static BOOL td_is_anime_end( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// 終了判定らしい
	if(wmode == CLWKMODE_NORMAL){		// 通常
		if(wnum >= CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	else if(wmode == CLWKMODE_CMN){		// 共通
		if(wnum >= CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.cmn_clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
//	else if(wmode == CLWKMODE_UP){		// up
//		if(wnum >= CLWKID_UP_MAX){
//			OS_Printf("up obj max error:%d\n",wnum);
//			return( FALSE );
//		}
//		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.up_clwk[wnum] ) == FALSE ){
//			bflg = FALSE;
//		}
//	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(bmpoam)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const T_DOWNLOAD_BMPOAM_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_obj_bmpoam( T_DOWNLOAD_MAIN_WORK* wk, const s32 drawtype, const T_DOWNLOAD_BMPOAM_DATA *bmp_adr, const s16 *info_adr )
{
	s16							tmp;
	s32							i;
	s32							buf[2];
	BMPOAM_ACT_DATA				actdata;
	const T_DOWNLOAD_BMPOAM_DATA	*adr  = bmp_adr;
	const s16					*iadr = info_adr;
	GFL_CLACTPOS				cpos;
	//
	for(i=0;i<T_DOWNLOAD_BMPOAM_MAXNUM;i++){
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
		cpos.x = td_get_empty( wk, tmp*2 );
		cpos.y = td_get_empty( wk, tmp*2+1 );
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
 *	@brief  OB文字列表示(bmpoam)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				ワーク番号
 *	@param	const STRBUF*			表示文字列
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_disp_bmpoam( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const STRBUF *strbuf )
{
	if(wnum >= T_DOWNLOAD_BMPOAM_MAXNUM){
		OS_Printf("bmpoamj max error:%d\n",wnum);
		return;
	}
	GFL_BMP_Clear( wk->BmpData[wnum], 0 );
	PRINTSYS_PrintQueColor( wk->print_que, wk->BmpData[wnum], 0, 0, strbuf, wk->font, PRINTSYS_LSB_Make(15,14,0) );
	BmpOam_ActorBmpTrans( wk->BmpOamWork[wnum] );
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
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
	}
	//
	//-----------------
	// fontパレット転送
	//-----------------
	{
		ARCHANDLE	*p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		// Main
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
static void td_disp_string( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col )
{
	// 表示
	PRINTTOOL_PrintColor(
				&wk->PrintUtil[mode],			// プリントユーティル ( BMPWIN )
				wk->print_que,					// プリントキュー
				x, y,							// 表示基準座標
				strbuf,							// 文字列
				wk->font,						// フォント
//				PRINTSYS_LSB_Make(15,14,0),		// カラー
//				PRINTSYS_LSB_Make(9,8,0),		// カラー
//				PRINTSYS_LSB_Make(1,2,0),		// カラー
				col,							// カラー
				pmode );						// 位置
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示(仮)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x)
 *	@param	const s32			表示座標(y)
 *	@param	const s32			値
 *	@param	const s32			桁数
 *	@param	const u32			表示位置(左、右、中央など)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_disp_string_number( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 num, const s32 lnum, const u32 pmode )
{
	STRBUF	*strbuf;
	
	// 数字を桁分0うめ表示
	strbuf = GFL_STR_CreateBuffer( 128, wk->heapID );
	STRTOOL_SetNumber( strbuf, num, lnum, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	//
	if(mode == CVS_MAIN_TITLE){
		td_disp_string( wk, mode, x, y, strbuf, pmode, PRINTSYS_LSB_Make(15,2,0) );
	}else{
		td_disp_string( wk, mode, x, y, strbuf, pmode, PRINTSYS_LSB_Make(1,2,0) );
	}
	//
	GFL_STR_DeleteBuffer( strbuf );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(現在、Canvasの作成)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				Canvasの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_update_string( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
#if 0
	// 転送
	GFL_BMPWIN * bmpwin = wk->PrintUtil[mode].win;
	//
	// これは入れないとだめらしい
	PRINT_UTIL_Trans( &wk->PrintUtil[mode], wk->print_que );
	//
	GFL_BMPWIN_TransVramCharacter( bmpwin );					//transfer characters
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );	//transfer screen
#endif
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
//	APP_KEYCURSOR_Delete( wk->MsgCursorWork );
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
static void td_clear_canvas( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	// clear
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_CLEAR_COLOR );
//	GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[mode].win );
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  パネル表示まとめ
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			ワーク番号
 *	@param	const s32			直接シーケンス番号
 *	@param	const s32			(0:4行、1:3行)
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_set_panel( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 smode, const BOOL bflg )
{
	s16				buf[4];
	s32				tmp;
	GFL_CLACTPOS	pos;
	STRBUF			*strbuf;
	
	// 表示/非表示
	td_set_obj_visible( wk, CLWKMODE_NORMAL, wnum, bflg );
	// bmpoam
	BmpOam_ActorSetDrawEnable( wk->BmpOamWork[wnum], bflg );
	
	// 非表示の時はそのままで終了
	if(bflg == FALSE){
		return;
	}
	// シーケンスの変更
	td_chg_objseq( wk, CLWKMODE_NORMAL, wnum, num );
	
	//----------
	// 数字表示
	//----------
	// test(要修正)
	if(smode == 1){
		tmp = wk->SaveBuffer[wnum];
	}else{
		tmp = wk->ItemBuffer[wk->TopNum+wnum].Num;
	}
	strbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
	STRTOOL_SetNumber( strbuf, tmp, 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	td_disp_bmpoam( wk, wnum, strbuf );
	//
	// bmpoamは座標も念のため再セットしておく
	buf[0] = td_get_empty( wk,  EMPTY_PANEL_OFFSET*2 );
	buf[1] = td_get_empty( wk,  EMPTY_PANEL_OFFSET*2+1 );
	//
	if(smode == 0){
		buf[2] = td_get_empty( wk,  (EMPTY_PANEL_POS4+wnum)*2 );
		buf[3] = td_get_empty( wk,  (EMPTY_PANEL_POS4+wnum)*2+1 );
	}else{
		buf[2] = td_get_empty( wk,  (EMPTY_PANEL_POS3+wnum)*2 );
		buf[3] = td_get_empty( wk,  (EMPTY_PANEL_POS3+wnum)*2+1 );
	}
	//
	// パネルの座標の取得
//	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[wnum], &pos, CLSYS_DEFREND_MAIN );
	pos.x = buf[2];
	pos.y = buf[3];
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[wnum], &pos, CLSYS_DEFREND_MAIN );
	// 文字列の座標セット
	BmpOam_ActorSetPos( wk->BmpOamWork[wnum],   (pos.x+buf[0]), (pos.y+buf[1]) );
	//
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
}

//-----------------------------------------------------------------------------------------------------------------
/**
 *	@brief  カーソル表示(仮)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			カーソルID
 *	@param	const s32			位置(0-2)
 *	@param	const s32			(0:4行、1:3行)
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_set_cursor( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 smode, const BOOL bflg )
{
	s16				buf[4];
	GFL_CLACTPOS	pos;
	
	// 表示/非表示
	td_set_obj_visible( wk, CLWKMODE_NORMAL, wnum, bflg );
	// 非表示の時はそのままで終了
	if(bflg == FALSE){
		return;
	}
	// シーケンスの変更
//	td_chg_objseq( wk, CLWKMODE_NORMAL, wnum, num );
	//
	if(smode == 0){
		buf[2] = td_get_empty( wk,  (EMPTY_PANEL_POS4+num)*2 );
		buf[3] = td_get_empty( wk,  (EMPTY_PANEL_POS4+num)*2+1 );
	}else{
		buf[2] = td_get_empty( wk,  (EMPTY_PANEL_POS3+num)*2 );
		buf[3] = td_get_empty( wk,  (EMPTY_PANEL_POS3+num)*2+1 );
	}
	// パネルの座標の取得
	pos.x = buf[2]+0x10;
	pos.y = buf[3]+0x10;
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[wnum], &pos, CLSYS_DEFREND_MAIN );
}

//-----------------------------------------------------------------------------------------------------------------
/**
 *	@brief  上画面表示まとめ(仮)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_disp_upinfo( T_DOWNLOAD_MAIN_WORK* wk, const s32 num )
{
	s32		i;
	STRBUF	*strbuf;
	
	//---------
	// クリア
	//---------
	for(i=0;i<CVS_UP_MES_MAX;i++){
		td_clear_canvas( wk, i );
	}
	//------------
	// 各項目表示
	//------------
	// 項目なしの場合は全非表示とする
	if(num != -1){
		// 項目ありの場合はnumから情報を参照
		// タイトル
		td_disp_string_number( wk, CVS_TITLE, 0, 0, num, 8, PRINTTOOL_MODE_LEFT );
		// 配信日
		td_disp_string_number( wk, CVS_DATE,  0, 0, 100, 8, PRINTTOOL_MODE_LEFT );
		// ルールタイトル
		td_disp_string_number( wk, CVS_RULE_TITLE,   0, 0, 200, 8, PRINTTOOL_MODE_LEFT );
		// ルール説明
		td_disp_string_number( wk, CVS_RULE_EXPLAIN, 0, 0, 300, 8, PRINTTOOL_MODE_LEFT );
		// 内容タイトル
		td_disp_string_number( wk, CVS_INFO_TITLE,   0, 0, 400, 8, PRINTTOOL_MODE_LEFT );
		// 内容説明
		td_disp_string_number( wk, CVS_INFO_EXPLAIN, 0, 0, 500, 8, PRINTTOOL_MODE_LEFT );
	}
	//
	//-------
	// 更新
	//-------
	for(i=0;i<CVS_UP_MES_MAX;i++){
		td_update_string( wk, i );
	}
}

//-----------------------------------------------------------------------------------------------------------------
/**
 *	@brief  下画面表示まとめ(仮)(現在、タイトル文字のみ)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_disp_downinfo( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		buf[2];
	s32		i;
	STRBUF	*strbuf;
	
	// 下側の個数を求める
//	buf[0] = CVS_MAIN_MES_MAX-CVS_UP_MES_MAX;
	buf[0] = 1;
	//---------
	// クリア
	//---------
	for(i=0;i<buf[0];i++){
		td_clear_canvas( wk, CVS_UP_MES_MAX+i );
	}
	//------------
	// 各項目表示
	//------------
	// タイトル
	td_disp_string_number( wk, CVS_MAIN_TITLE, 0, 0, 1000, 8, PRINTTOOL_MODE_LEFT );
	//
	//-------
	// 更新
	//-------
	for(i=0;i<buf[0];i++){
		td_update_string( wk, CVS_UP_MES_MAX+i );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  更新
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_param_cmn_update( T_DOWNLOAD_MAIN_WORK* wk )
{
	BOOL	bflg;
	
	// 左矢印
	bflg = TRUE;
	if(wk->Flag & FLG_MW_LEFT_HIDE){
		bflg = FALSE;
	}
	td_set_obj_visible( wk, CLWKMODE_CMN, CLWKID_CMN_LEFT, bflg );
	//
	// 右矢印
	bflg = TRUE;
	if(wk->Flag & FLG_MW_RIGHT_HIDE){
		bflg = FALSE;
	}
	td_set_obj_visible( wk, CLWKMODE_CMN, CLWKID_CMN_RIGHT, bflg );
	//
	// 項目がない
	if(wk->Flag & FLG_MW_NOITEM){
		if(!(wk->Flag & FLG_MW_NOITEM_MES)){
			td_disp_centermes( wk, CVS_MAIN_CENTER_WIN, 1 );
			wk->Flag |= FLG_MW_NOITEM_MES;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  更新
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_param_view_update( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		num;
	s32		i;
	BOOL	bflg;
	
	// 更新
	if(wk->Flag & FLG_MW_UPDATE){
		num = td_calc_savebuffer( wk );
		if(num == 0){
			wk->Flag |= FLG_MW_NOITEM;
		}
		//
		// パネル表示
		for(i=0;i<4;i++){
			bflg = FALSE;
			if(i < num){
				bflg = TRUE;
			}
			td_set_panel( wk, i, PANEL_SAVE_SEQ, 1, bflg );
		}
		// カーソル表示
		bflg = TRUE;
		if(wk->Flag & FLG_MW_NOITEM){
			bflg = FALSE;
		}
		td_set_cursor( wk, CLWKID_CURSOR, wk->CurPos, 1, bflg );
		
		//
		// 上画面更新
		if(!( wk->Flag & FLG_MW_NOITEM )){
			td_disp_upinfo( wk, wk->SaveBuffer[wk->CurPos] );
		}else{
			td_disp_upinfo( wk, -1 );
		}
		// 共通
		td_param_cmn_update( wk );
		//
		wk->Flag &= ~FLG_MW_UPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  更新
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_param_select_update( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16				tmp;
	s32				num;
	s32				i;
	u32				py;
	BOOL			bflg;
	GFL_CLACTPOS	pos;
	
	// 更新
	if(wk->Flag & FLG_MW_UPDATE){
		// 通常の場合
		if(!(wk->Flag & FLG_MW_DISP3)){
			num = td_calc_itembuffer( wk );
			if(num == 0){
				wk->Flag |= FLG_MW_NOITEM;
			}
			// パネル表示
			for(i=0;i<4;i++){
				bflg = FALSE;
				if(i < num){
					bflg = TRUE;
				}
				tmp = wk->ItemBuffer[wk->TopNum+i].Mode;
				td_set_panel( wk, i, tmp, 0, bflg );
			}
			// カーソル表示
			bflg = TRUE;
			if(wk->Flag & FLG_MW_NOITEM){
				bflg = FALSE;
			}
			td_set_cursor( wk, CLWKID_CURSOR, wk->SubNum, 0, bflg );
			//
			// 上画面更新
			if(!( wk->Flag & FLG_MW_NOITEM )){
				td_disp_upinfo( wk, wk->ItemBuffer[wk->TopNum+wk->SubNum].Num );
			}else{
				td_disp_upinfo( wk, -1 );
			}
			//
			// バー更新
			bflg = TRUE;
			if(wk->Flag & FLG_MW_BAR_HIDE){
				bflg = FALSE;
			}
			// バー表示/非表示
			td_set_obj_visible( wk, CLWKMODE_NORMAL, CLWKID_BAR, bflg );
			//
			if(wk->Flag & FLG_MW_BAR_UPDATE){
				if(!(wk->Flag & FLG_MW_NOITEM)){
					py = SCROLLBAR_GetPosY( num-1, wk->TopNum+wk->SubNum, SCRL_TOP_Y, SCRL_DOWN_Y, SCRL_BAR_SIZE );
					// 座標の更新
					GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[CLWKID_BAR], &pos, CLSYS_DEFREND_MAIN );
//					pos.x = 238;	// test
					pos.y = py;
					GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[CLWKID_BAR], &pos, CLSYS_DEFREND_MAIN );
					//
					wk->Flag &= ~FLG_MW_BAR_UPDATE;
				}
			}
		}
		// 3行の場合
		else{
			num = td_calc_savebuffer( wk );
			if(num == 0){
				wk->Flag |= FLG_MW_NOITEM;
			}
			//
			// パネル表示
			for(i=0;i<4;i++){
				bflg = FALSE;
				if(i < num){
					bflg = TRUE;
				}
				td_set_panel( wk, i, PANEL_SAVE_SEQ, 1, bflg );
			}
			// カーソル表示
			bflg = TRUE;
			if(wk->Flag & FLG_MW_NOITEM){
				bflg = FALSE;
			}
			td_set_cursor( wk, CLWKID_CURSOR, wk->CurPos, 1, bflg );
			
			//
			// 上画面更新
			if(!( wk->Flag & FLG_MW_NOITEM )){
				td_disp_upinfo( wk, wk->SaveBuffer[wk->CurPos] );
			}else{
				td_disp_upinfo( wk, -1 );
			}
			// バー更新
			bflg = TRUE;
			if(wk->Flag & FLG_MW_BAR_HIDE){
				bflg = FALSE;
			}
			// バー表示/非表示
			td_set_obj_visible( wk, CLWKMODE_NORMAL, CLWKID_BAR, bflg );
		}
		// 共通
		td_param_cmn_update( wk );
		//
		wk->Flag &= ~FLG_MW_UPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(view)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:更新あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_view_key_input( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		i;
	s16		j;
	s16		flg=0;
	s32		wnum;
	s32		tmp;
	BOOL	bflg=FALSE;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	//
	// 上
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
		wk->CurPos--;
		if(wk->CurPos < 0){
			wk->CurPos = 0;
		}
		flg = 1;
	}
	// 下
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		j = td_calc_savebuffer( wk );
		// 念のため
		if(j == 0){
			wk->CurPos = 0;
		}
		else{
			wk->CurPos++;
			if(wk->CurPos >= (j-1)){
				wk->CurPos = (j-1);
			}
		}
		flg = 1;
	}
	// 左
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
	}
	// 右
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
	}
	// 戻る
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		// RETURNアニメ
		td_set_cmn_anim( wk, CLWKID_CMN_RETURN );
	}
	// 決定(削除)
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
		// 削除チェック
		if(!(wk->Flag & FLG_MW_NOITEM)){
			// 初期化
			td_init_meswin( wk, CVS_MAIN_MESWIN );
			// メッセージウインドウ(test)
			td_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
			//
			// 輝度変更(test)
			td_chg_brightness( wk, TD_PASSIVE_RATE );
			//
			wk->Step = STEP_LV2;
		}
	}
	//
	// 更新
	if(flg == 1){
		wk->Flag |= FLG_MW_UPDATE;
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:更新あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_select_key_input( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		i;
	s16		j;
	s16		flg=0;
	s32		wnum;
	s32		tmp;
	s32		mode;
	BOOL	bflg=FALSE;
	BOOL	sflg;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	//
	// 後日、正しく処理すること
	mode = KEY_INPUT_BEFORE;
	if(wk->Step == STEP_LV3){
		mode = KEY_INPUT_AFTER;
	}
	
	//------
	// 前
	//------
	if(mode == KEY_INPUT_BEFORE){
		// 上
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
			// すでに0になっている
			// =>上に変動の可能性あり
			if(wk->SubNum == 0){
				wk->TopNum--;
				if(wk->TopNum < 0){
					wk->TopNum = 0;
				}
			}
			wk->SubNum--;
			if(wk->SubNum < 0){
				wk->SubNum = 0;
			}
			flg = 1;
		}
		// 下
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
			j = td_calc_itembuffer( wk );
			// 念のため
			if(j == 0){
				wk->SubNum = 0;
			}
			else{
				// すでに3になっている
				// =>下に変動の可能性あり
				if(wk->SubNum == (PANEL_DISP4-1)){
					wk->TopNum++;
					if(wk->TopNum >= (j-4)){
						wk->TopNum = (j-4);
					}
				}
				if(j >= PANEL_DISP4){
					j = PANEL_DISP4;
				}
				wk->SubNum++;
				if(wk->SubNum >= (j-1)){
					wk->SubNum = (j-1);
				}
			}
			flg = 1;
		}
		// 左
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
			// 上限に達した
			if(wk->TopNum == 0){
				wk->SubNum = 0;
			}
			wk->TopNum -= PANEL_DISP4;
			if(wk->TopNum < 0){
				wk->TopNum = 0;
			}
			flg = 1;
			// 左アニメ
			td_set_cmn_anim( wk, CLWKID_CMN_LEFT );
		}
		// 右
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
			// 個数を調べる
			j = td_calc_itembuffer( wk );
			// 上限に達している
			if(wk->TopNum == (j-4)){
				wk->SubNum = (PANEL_DISP4-1);
			}
			wk->TopNum += PANEL_DISP4;
			if(wk->TopNum >= (j-4)){
				wk->TopNum = (j-4);
			}
			flg = 1;
			// 右アニメ
			td_set_cmn_anim( wk, CLWKID_CMN_RIGHT );
		}
		// 戻る
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
			// RETURNアニメ
			td_set_cmn_anim( wk, CLWKID_CMN_RETURN );
		}
		// 決定(追加)
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
			// 含まれているかチェック
			// =>なければ追加
			sflg = td_chk_savebuffer( wk, wk->TopNum+wk->SubNum );
			if(sflg == FALSE){
				// 初期化
				td_init_meswin( wk, CVS_MAIN_MESWIN );
				// メッセージウインドウ(test)
				td_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
				// 輝度変更(test)
				td_chg_brightness( wk, TD_PASSIVE_RATE );
				//
				wk->Step = STEP_LV2;
			}
		}
		//
		// 更新
		if(flg == 1){
			wk->Flag |= FLG_MW_UPDATE;
			wk->Flag |= FLG_MW_BAR_UPDATE;
			bflg = TRUE;
		}
	}
	//-----
	// 後
	//-----
	else{
		// 上
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
			wk->CurPos--;
			if(wk->CurPos < 0){
				wk->CurPos = 0;
			}
			flg = 1;
		}
		// 下
		else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
			j = td_calc_savebuffer( wk );
			// 念のため
			if(j == 0){
				wk->CurPos = 0;
			}
			else{
				wk->CurPos++;
				if(wk->CurPos >= (j-1)){
					wk->CurPos = (j-1);
				}
			}
			flg = 1;
		}
		// 戻る
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
			// RETURNアニメ
			td_set_cmn_anim( wk, CLWKID_CMN_RETURN );
		}
		// 決定(削除)
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
			// 初期化
			td_init_meswin( wk, CVS_MAIN_MESWIN );
			// メッセージウインドウ(test)
			td_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
			//
			// 輝度変更(test)
			td_chg_brightness( wk, TD_PASSIVE_RATE );
			//
			wk->Step = STEP_LV4;
		}
		//
		// 更新
		if(flg == 1){
			wk->Flag |= FLG_MW_UPDATE;
			bflg = TRUE;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメのセット
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(CMNの番号:フラグと対応)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_set_cmn_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s32		wnum;
	
	// アニメ再生
	wnum = mode;
	td_chg_objseq( wk, CLWKMODE_CMN, wnum, td_cmn_btn_tbl[wnum*4+1] );
	wk->Flag |= (1<<mode);
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
static BOOL td_chk_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s16		flg=0;
	s16		wnum=0;
	BOOL	bflg=FALSE;
	BOOL	rflg=FALSE;
	
	// 左アニメ再生中
	if(wk->Flag & FLG_MW_ANM_LEFT){
		wnum = CLWKID_CMN_LEFT;
		flg = 1;
	}
	// 右アニメ再生中
	else if(wk->Flag & FLG_MW_ANM_RIGHT){
		wnum = CLWKID_CMN_RIGHT;
		flg = 1;
	}
	// ×アニメ再生中
	else if(wk->Flag & FLG_MW_ANM_CROSS){
		wnum = CLWKID_CMN_CROSS;
		flg = 1;
	}
	// 戻るアニメ再生中
	else if(wk->Flag & FLG_MW_ANM_RETURN){
		wnum = CLWKID_CMN_RETURN;
		flg = 1;
	}
	// アニメ終了判定を行う
	if(flg == 1){
		bflg = td_is_anime_end( wk, CLWKMODE_CMN, wnum );
		if(bflg == FALSE){
			flg = 2;
			// アニメの終了
			// とりあえず通常に戻しておく
			td_chg_objseq( wk, CLWKMODE_CMN, wnum, td_cmn_btn_tbl[wnum*4] );
			wk->Flag &= ~(1<<wnum);
			//
			// 終了判定ありの場合
			if(mode == 0){
				// 戻るボタン(飛び先変更はある)
				if((wnum == CLWKID_CMN_CROSS)||(wnum == CLWKID_CMN_RETURN)){
					wk->Flag |= FLG_MW_END;
					//
					// ×ボタン終了
					if(wnum == CLWKID_CMN_CROSS){
						wk->Flag |= FLG_MW_CROSS_END;
					}
					// RETURNボタン終了
					else if(wnum == CLWKID_CMN_RETURN){
						wk->Flag |= FLG_MW_RET_END;
					}
					// 終了処理
					wk->Step = STEP_END;
				}
			}
			// 戻る処理
			else if(mode == 1){
				if((wnum == CLWKID_CMN_CROSS)||(wnum == CLWKID_CMN_RETURN)){
					wk->Step = STEP_LV1;
					wk->Flag |= FLG_MW_UPDATE;
					wk->Flag &= ~FLG_MW_DISP3;
					wk->Flag &= ~(FLG_MW_RIGHT_HIDE|FLG_MW_LEFT_HIDE);
					wk->Flag &= ~FLG_MW_BAR_HIDE;
				}
			}
			rflg = TRUE;
		}
	}
	//
	return( rflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータ初期化(ここでパラメータを参照)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_param( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16					i;
	GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	//
	// タッチペン領域計算(cmnのみ)
	for(i=0;i<AREA_TD_TP_CMN_MAX;i++){
		td_get_tp_area( wk, i, adr );
		adr++;
	}
	// 終了コード
	adr->rect.top		= GFL_UI_TP_HIT_END;
	adr->rect.bottom	= 0;
	adr->rect.left		= 0;
	adr->rect.right		= 0;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン入力(view)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_view_tp_input( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		i;
	s16		flg=0;
	s32		num;
	s32		wnum;
	const GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	const int ret  = GFL_UI_TP_HitTrg( adr );
	const int cret = GFL_UI_TP_HitCont( adr );
	// なければ、TP_HIT_NONE
	BOOL	bflg=FALSE;
	BOOL	mflg;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	
	// ×
	if(ret == AREA_TD_TP_CROSS){
		// アニメ再生
		td_set_cmn_anim( wk, CLWKID_CMN_CROSS );
		flg = 2;
	}
	// RETURN
	else if(ret == AREA_TD_TP_RETURN){
		// アニメ再生
		td_set_cmn_anim( wk, CLWKID_CMN_RETURN );
		flg = 2;
	}
	// セーブ領域を調べ、
	// 必要なパネルの分だけチェックする
	if(flg == 0){
		num = td_calc_savebuffer( wk );
		for(i=0;i<num;i++){
			// 個別のタッチペン判定を使用
			mflg = td_chk_my_tp( wk, 0, AREA_TD_TP_PANEL3_0+i );
			// 連番なので注意
			if(mflg == TRUE){
				// カーソル位置と同じでないとだめ(仮)
				if(wk->CurPos == i){
					// 初期化
					td_init_meswin( wk, CVS_MAIN_MESWIN );
					// メッセージウインドウ(test)
					td_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
					//
					// 輝度変更(test)
					td_chg_brightness( wk, TD_PASSIVE_RATE );
					//
					wk->Step = STEP_LV2;
				}
				// カーソル位置を変更(仮)
				else{
					wk->CurPos = i;
					flg = 1;
				}
				break;
			}
		}
	}
	//
	// 更新
	if(flg != 0){
		wk->Flag |= FLG_MW_UPDATE;
		bflg = TRUE;
	}
	return( bflg );
}
//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン入力(select)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_select_tp_input( T_DOWNLOAD_MAIN_WORK* wk )
{
	s16		i;
	s16		flg=0;
	s32		num;
	s32		wnum;
	s32		mode;
	const GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	const int ret  = GFL_UI_TP_HitTrg( adr );
	const int cret = GFL_UI_TP_HitCont( adr );
	// なければ、TP_HIT_NONE
	BOOL	bflg=FALSE;
	BOOL	mflg;
	BOOL	sflg;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	
	// 後日、正しく処理すること
	mode = KEY_INPUT_BEFORE;
	if(wk->Step == STEP_LV3){
		mode = KEY_INPUT_AFTER;
	}
	//-------
	// 前
	//-------
	if(mode == KEY_INPUT_BEFORE){
		// 左を押した
		if(ret == AREA_TD_TP_LEFT){
			// 上限に達した
			if(wk->TopNum == 0){
				wk->SubNum = 0;
			}
			wk->TopNum -= PANEL_DISP4;
			if(wk->TopNum < 0){
				wk->TopNum = 0;
			}
			flg = 1;
			// 左アニメ
			td_set_cmn_anim( wk, CLWKID_CMN_LEFT );
		}
		// 右を押した
		else if(ret == AREA_TD_TP_RIGHT){
			// 個数を調べる
			num = td_calc_itembuffer( wk );
			// 上限に達している
			if(wk->TopNum == (num-4)){
				wk->SubNum = (PANEL_DISP4-1);
			}
			wk->TopNum += PANEL_DISP4;
			if(wk->TopNum >= (num-4)){
				wk->TopNum = (num-4);
			}
			flg = 1;
			// 右アニメ
			td_set_cmn_anim( wk, CLWKID_CMN_RIGHT );
		}
		// ×
		else if(ret == AREA_TD_TP_CROSS){
			// アニメ再生
			td_set_cmn_anim( wk, CLWKID_CMN_CROSS );
			flg = 2;
		}
		// RETURN
		else if(ret == AREA_TD_TP_RETURN){
			// アニメ再生
			td_set_cmn_anim( wk, CLWKID_CMN_RETURN );
			flg = 2;
		}
		else{
			// バー
			// contで取得
			if(cret == AREA_TD_TP_SCROLL){
				td_set_scrollbar( wk );
				flg = 3;
			}
		}
		// アイテムの個数を調べ、
		// 必要なパネルの分(MAX4)だけチェックする
		if(flg == 0){
			num = td_calc_itembuffer( wk );
			if(num >= PANEL_DISP4){
				num = PANEL_DISP4;
			}
			for(i=0;i<num;i++){
				// 個別のタッチペン判定を使用
				mflg = td_chk_my_tp( wk, 0, AREA_TD_TP_PANEL4_0+i );
				// 連番なので注意
				if(mflg == TRUE){
					// カーソル位置と同じでないとだめ(仮)
					if(wk->SubNum == i){
						// 含まれているかチェック
						// =>なければ追加
						sflg = td_chk_savebuffer( wk, wk->TopNum+wk->SubNum );
						if(sflg == FALSE){
							// 初期化
							td_init_meswin( wk, CVS_MAIN_MESWIN );
							// メッセージウインドウ(test)
							td_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
							//
							// 輝度変更(test)
							td_chg_brightness( wk, TD_PASSIVE_RATE );
							//
							wk->Step = STEP_LV2;
						}
					}
					// カーソル位置を変更(仮)
					else{
						wk->SubNum = i;
						flg = 1;
					}
					break;
				}
			}
		}
		//
		// 更新
		if(flg != 0){
			wk->Flag |= FLG_MW_UPDATE;
			wk->Flag |= FLG_MW_BAR_UPDATE;
			bflg = TRUE;
		}
	}
	//-------
	// 後
	//-------
	else{
		// ×
		if(ret == AREA_TD_TP_CROSS){
			// アニメ再生
			td_set_cmn_anim( wk, CLWKID_CMN_CROSS );
			flg = 2;
		}
		// RETURN
		else if(ret == AREA_TD_TP_RETURN){
			// アニメ再生
			td_set_cmn_anim( wk, CLWKID_CMN_RETURN );
			flg = 2;
		}
		//
		if(flg == 0){
			num = td_calc_savebuffer( wk );
			for(i=0;i<num;i++){
				// 個別のタッチペン判定を使用
				mflg = td_chk_my_tp( wk, 0, AREA_TD_TP_PANEL3_0+i );
				// 連番なので注意
				if(mflg == TRUE){
					// カーソル位置と同じでないとだめ(仮)
					if(wk->CurPos == i){
						// 初期化
						td_init_meswin( wk, CVS_MAIN_MESWIN );
						// メッセージウインドウ(test)
						td_set_meswin( wk, CVS_MAIN_MESWIN, 1 );
						//
						// 輝度変更(test)
						td_chg_brightness( wk, TD_PASSIVE_RATE );
						//
						wk->Step = STEP_LV4;
					}
					// カーソル位置を変更(仮)
					else{
						wk->CurPos = i;
						flg = 1;
					}
					break;
				}
			}
		}
		// 更新
		if(flg != 0){
			wk->Flag |= FLG_MW_UPDATE;
			bflg = TRUE;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン入力(trg)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(0:Trg、1:Cont)
 *	@param	const s32				エリア番号(AREA_TD_TP..)
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_my_tp( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 num )
{
	s16		sbuf[4];
	BOOL	bflg;
	u32		buf[2];
	//
	// パネルのように、重複するエリアの場合、
	// 先に登録したものが優先されるので、
	// あらかじめテーブルに登録する方法だと処理(準備)が複雑になるので
	// 個別のエリアを参照できるのも容易しておく
	// (共有は従来の方法で対応)
	if(mode == 0){		// trg
		bflg = GFL_UI_TP_GetPointTrg(  &buf[0], &buf[1] );
	}else{				// cont
		bflg = GFL_UI_TP_GetPointCont( &buf[0], &buf[1] );
	}
	if(bflg == TRUE){
		sbuf[0] = wk->ParamBuffer[4*num];
		sbuf[1] = wk->ParamBuffer[4*num+1];
		sbuf[2] = wk->ParamBuffer[4*num+2];
		sbuf[3] = wk->ParamBuffer[4*num+3];
		//
		// x方向
		if((buf[0] >= sbuf[0])&&(buf[0] <= (sbuf[0]+sbuf[2]))){
			// y方向
			if((buf[1] >= sbuf[1])&&(buf[1] <= (sbuf[1]+sbuf[3]))){
				return( TRUE );
			}
		}
	}
	return( FALSE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーの処理
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:変更あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL td_set_scrollbar( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32				num;
	u32				cnt;
	u32				buf[2];
	BOOL			bflg=FALSE;
	
	// ここにくるまでにスクロールバーの領域を
	// タッチしたかの判定は済んでいる
	// タッチした座標を取得
	if( GFL_UI_TP_GetPointCont( &buf[0], &buf[1] ) == TRUE ){
		// どの段階かが求まる
		num = td_calc_itembuffer( wk );
		cnt = SCROLLBAR_GetCount( num-1, buf[1], SCRL_TOP_Y, SCRL_DOWN_Y, SCRL_BAR_SIZE );
		// 現在の位置計算してセット
		td_calc_scrollbar_pos( wk, cnt );
		wk->Flag |= FLG_MW_UPDATE;
		wk->Flag |= FLG_MW_BAR_UPDATE;
		//
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーの位置計算
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(スクロール値)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_calc_scrollbar_pos( T_DOWNLOAD_MAIN_WORK* wk, const s32 num )
{
	// 怪しい気もするが、
	// ゲージを等分した箇所になるように、
	// yposの値が丸められるため、
	// これでいいようだ..
	//
	// マイナス方向
	if(num < wk->TopNum){
		// 0に固定
		wk->SubNum = 0;
		wk->TopNum = num;
	}
	// プラス方向(4以上)
	else if(num > (wk->TopNum+3)){
		wk->SubNum = 3;
		wk->TopNum = num-3;
	}
	// (0-3)
	else{
		// wk->TopNumは変化なし
		wk->SubNum = num-wk->TopNum;
	}
//	OS_Printf("top,sub :(%d),%d,%d,%d\n",flg,num,wk->TopNum,wk->SubNum);
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief	バッファ計算
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval s32					個数
 */
//-----------------------------------------------------------------------------
static s32 td_calc_savebuffer( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	s32		j;
	
	// 個数の計算
	j=0;
	for(i=0;i<SAVE_BUFFER_MAX;i++){
		if( wk->SaveBuffer[i] != -1 ){
			j++;
		}
	}
	// 個数取得
	return( j );
}

//-----------------------------------------------------------------------------
/**
 *	@brief	バッファにあるかチェック
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			番号
 *
 *	@retval s32					個数
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_savebuffer( T_DOWNLOAD_MAIN_WORK* wk, const s32 num )
{
	s32		i;
	BOOL	bflg=FALSE;
	
	// 個数の計算
	for(i=0;i<SAVE_BUFFER_MAX;i++){
		if( wk->SaveBuffer[i] == num){
			bflg = TRUE;
			break;
		}
	}
	//
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief	バッファ計算
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				番号
 *
 *	@retval s32						(0よりでwnumに近い値)
 */
//-----------------------------------------------------------------------------
static s32 td_del_savebuffer( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum )
{
	s32		i;
	s32		j;
	s32		ret;
	s32		buf[SAVE_BUFFER_MAX];
	
	// 削除
	wk->SaveBuffer[wnum] = -1;
	
	// 初期化
	for(i=0;i<SAVE_BUFFER_MAX;i++){
		buf[i] = -1;
	}
	// つめる処理
	j=0;
	for(i=0;i<SAVE_BUFFER_MAX;i++){
		if(wk->SaveBuffer[i] != -1){
			buf[j] = wk->SaveBuffer[i];
			j++;
		}
	}
	//
	// ここまででbuf[]に値が入っているはず(再セット)
	for(i=0;i<SAVE_BUFFER_MAX;i++){
		wk->SaveBuffer[i] = buf[i];
	}
	//
	// 削除され番号に近い値を求める
	if(j > wnum){
		ret = wnum;
	}else{
		ret = j-1;
	}
	return( ret );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  deleteの処理判定
 *
 *  @param  T_DOWNLOAD_MAIN_WORK	*wk
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void td_chk_delete( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	
	tmp = td_del_savebuffer( wk, wk->CurPos );
	if(tmp == -1){		// 項目がない
		wk->CurPos = 0;
		wk->Flag |= FLG_MW_NOITEM;
	}else{				// 項目あり
		wk->CurPos = tmp;
	}
	wk->Flag |= FLG_MW_UPDATE;
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *  @brief   はい/いいえウインドウ 開始
 *
 *  @param  T_DOWNLOAD_MAIN_WORK *wk
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void td_init_yesnowin( T_DOWNLOAD_MAIN_WORK* wk )
{
	APP_TASKMENU_INITWORK appinit;
	
	appinit.heapId		= wk->heapID;
	appinit.itemNum		= 2;					// アイテム数
	appinit.itemWork	= &wk->appitem[0];		// 情報セットのバッファ
	//
	appinit.posType		= ATPT_RIGHT_DOWN;
	appinit.charPosX	= 32;
	appinit.charPosY	= 12;
	appinit.w			= APP_TASKMENU_PLATE_WIDTH_YN_WIN;
	appinit.h			= APP_TASKMENU_PLATE_HEIGHT_YN_WIN;
	//
	// 「はい」の設定
	wk->appitem[0].str		= GFL_STR_CreateBuffer(100, wk->heapID);
	// test(00)
	STRTOOL_SetNumber( wk->appitem[0].str, 0, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
//	GFL_MSG_GetString(pWork->MsgManager, MSG_ITEM_STR003, pWork->appitem[0].str);
	wk->appitem[0].msgColor	= PRINTSYS_LSB_Make( 0xe, 0xf, 0 );
	wk->appitem[0].type		= APP_TASKMENU_WIN_TYPE_NORMAL;
	//
	// 「いいえ」の設定
	wk->appitem[1].str		= GFL_STR_CreateBuffer(100, wk->heapID);
	// test(01)
	STRTOOL_SetNumber( wk->appitem[1].str, 1, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
//	GFL_MSG_GetString(pWork->MsgManager, MSG_ITEM_STR004, pWork->appitem[1].str);
	wk->appitem[1].msgColor	= PRINTSYS_LSB_Make( 0xe, 0xf, 0 );
	wk->appitem[1].type		= APP_TASKMENU_WIN_TYPE_NORMAL;
	//
	wk->pAppTask = APP_TASKMENU_OpenMenu( &appinit, wk->pAppTaskRes );
	//
	GFL_STR_DeleteBuffer( wk->appitem[0].str );
	GFL_STR_DeleteBuffer( wk->appitem[1].str );
	//
	// defaultでnoにする
	// (activeもあわせて変更)
	// yes側
	APP_TASKMENU_SetCursorPos( wk->pAppTask, 0 );
	APP_TASKMENU_SetActive( wk->pAppTask, FALSE );
	// no側
	APP_TASKMENU_SetCursorPos( wk->pAppTask, 1 );
	APP_TASKMENU_SetActive( wk->pAppTask, TRUE );
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
static void td_exit_yesnowin( T_DOWNLOAD_MAIN_WORK* wk )
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
 *  @retval s32				(-1:未処理、0:NO、1:YES)
 */
//-----------------------------------------------------------------------------
static s32 td_chk_yesnowin( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		flg=-1;
	s32		pos;
	
	// 念のため
	if(wk->pAppTask == 0){
		return( flg );
	}
	// 選択中
	if( APP_TASKMENU_IsFinish( wk->pAppTask ) ){
		pos = APP_TASKMENU_GetCursorPos( wk->pAppTask );
		if(pos == 0){		// yes
			flg = 1;
		}else{				// no
			flg = 0;
		}
		// 終了
		td_exit_yesnowin( wk );
		//
		// clear
		td_clear_frame( wk );
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
static void td_set_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mesid )
{
	// PrintStream終了時にはdeleteしないとtaskが残ってしまうようだ
	// 念のため0check
	if( wk->PrintStream ){
		PRINTSYS_PrintStreamDelete( wk->PrintStream );
	}
	// Msgの再構築
	if(wk->MsgBuffer){
		GFL_STR_DeleteBuffer( wk->MsgBuffer );
	}
	// メッセージ仮バッファセット
	wk->MsgBuffer = GFL_MSG_CreateString( wk->msg[0], mesid );
	//
	// stream設定=>ここのwk->MsgBufferは終わるまで削除してはいけない
	wk->PrintStream = PRINTSYS_PrintStream(
				wk->PrintUtil[mode].win, 0, 0, wk->MsgBuffer, wk->font,
				MSGSPEED_GetWait(), wk->MsgTcblSys, 4, wk->heapID, MES_SYSWIN_CLEAR_COLOR );
	// スクリーン転送らしい
	GFL_BMPWIN_MakeTransWindow( wk->PrintUtil[mode].win );
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
static BOOL td_main_meswin( T_DOWNLOAD_MAIN_WORK* wk )
{
	BOOL	bflg;
	
	// 念のため
	if(wk->PrintStream == 0){
		return( FALSE );
	}
	//
	// 文字送りカーソル処理
//	APP_KEYCURSOR_Main( wk->MsgCursorWork, wk->PrintStream, wk->PrintUtil[CVS_MAIN_MESWIN].win );
	// streamの入力処理をまとめて処理してくれるらしい
//	bflg = APP_PRINTSYS_COMMON_PrintStreamFunc( &wk->PrintSysWork , wk->PrintStream );
	// 連結部分がうまくいかないため自前で処理
	bflg = td_update_printstream( wk );
	if( bflg == TRUE ){
		// 終了
		return( FALSE );
	}
	return( TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ完全初期化
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				canvasの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_init_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	//-----------------------------------------------------------------------
	// キャラデータのウインドウをまるごと消去(frmのクリア)する必要があるため
	// 完全初期化の部分を切り出しておく
	//-----------------------------------------------------------------------
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
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ全クリア(frmのクリア)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_clear_frame( T_DOWNLOAD_MAIN_WORK* wk )
{
	// clear
	GFL_BG_ClearFrame( GFL_BG_FRAME0_M );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  stream文字送り(yes/no)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:終了、FALSE:まだ)
 */
//-----------------------------------------------------------------------------
static BOOL td_update_printstream( T_DOWNLOAD_MAIN_WORK* wk )
{
	// APP_PRINTSYS_COMMON_PrintStreamFunc( &wk->PrintSysWork , wk->PrintStream );
	// を使ってしまうと最後の部分を入力しないと先にいかないことになってしまうので、
	// 自前で処理する
	BOOL	bflg=FALSE;
	s32 	status = PRINTSYS_PrintStreamGetState( wk->PrintStream );
	
	// 起動中
	if(status == PRINTSTREAM_STATE_RUNNING){
		if(GFL_UI_KEY_GetCont()&PAD_BUTTON_DECIDE){
			PRINTSYS_PrintStreamShortWait( wk->PrintStream, 0 );
		}
	}
	// 終わった
	else if( status == PRINTSTREAM_STATE_DONE ){
//		PRINTSYS_PrintStreamDelete( wk->PrintStream );
		bflg = TRUE;
//		return wk->next_seq;
	}
	// 次へ(ここにくるようだ)
	else if(status == PRINTSTREAM_STATE_PAUSE){
//		if(GFL_UI_KEY_GetTrg()&(PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) || GFL_UI_TP_GetTrg()){
			PRINTSYS_PrintStreamReleasePause( wk->PrintStream );
//			PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
//		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  会話ウインドウ全クリア(frmのクリア)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(canvasの番号)
 *	@param	const s32				(メッセージID)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_disp_centermes( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mesid )
{
	STRBUF		*strbuf;
	
	// 中央の文字表示
	td_init_meswin( wk, mode );
	//
	// メッセージ表示
	strbuf = GFL_MSG_CreateString( wk->msg[0], mesid );
	td_disp_string( wk, mode, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(1,2,0) );
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 更新
	td_update_string( wk, mode );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  アイテム情報のセット
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			バッファの番号
 *	@param	const s32			番号(仮)
 *	@param	const s32			状態(未読、既読、セーブ済み)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_set_item( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const s32 num, const s32 mode )
{
	// 情報のセット
	wk->ItemBuffer[wnum].Num 	 = num;
	wk->ItemBuffer[wnum].Mode	 = mode;
}

//-----------------------------------------------------------------------------
/**
 *	@brief	バッファ計算
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval s32						(個数)
 */
//-----------------------------------------------------------------------------
static s32 td_calc_itembuffer( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	s32		j;
	
	// 個数の計算
	j=0;
	for(i=0;i<ITEM_BUFFER_MAX;i++){
		if( wk->ItemBuffer[i].Num != 0xff ){
			j++;
		}
	}
	// 個数取得
	return( j );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アイテム情報の登録チェック
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:登録可、FALSE:不可)
 */
//-----------------------------------------------------------------------------
static BOOL td_chk_add_item( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		tmp;
	BOOL	bflg=FALSE;
	
	// セーブの個数チェック
	tmp = td_calc_savebuffer( wk );
	if(tmp >= SAVE_BUFFER_MAX){
		// 最大数を超えている
	}else{
		// アイテムの登録
		td_set_item( wk, wk->TopNum+wk->SubNum, wk->TopNum+wk->SubNum, PANEL_SAVE_SEQ );
		// save領域にセット
		wk->SaveBuffer[tmp] = wk->TopNum+wk->SubNum;
		//
		wk->Flag |= FLG_MW_UPDATE;
		bflg = TRUE;
	}
	//
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  輝度の変更
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				(割合(-16 .. +16))
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_chg_brightness( T_DOWNLOAD_MAIN_WORK* wk, const s32 rate )
{
	// objはどうしよう..(要相談)
	G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, rate );
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン用データセット
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32				領域の番号
 *	@param	GFL_UI_TP_HITTBL		タッチペン判定用
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void td_get_tp_area( T_DOWNLOAD_MAIN_WORK* wk, const s32 num, GFL_UI_TP_HITTBL *tpbuffer )
{
	s16		i;
	s16		buf[4];
	
	// 領域を取得
	// (左上(x,y)、サイズ(w,h)の4個で1つのデータ)
	for(i=0;i<4;i++){
		buf[i] = wk->ParamBuffer[4*num+i];
	}
	// ParamBufferは、(x,y,width,height)なので加算
	buf[2] += buf[0];
	buf[3] += buf[1];
	
	// 領域チェック
	for(i=0;i<4;i++){
		if(buf[i] < 0){
			buf[i] = 0;
		}
		// 判定はu8なのでmaxを255としておく
		if(buf[i] > 255){
			buf[i] = 255;
		}
	}
	// 領域
	tpbuffer[0].rect.left	= (u8)buf[0];
	tpbuffer[0].rect.top    = (u8)buf[1];
	tpbuffer[0].rect.right	= (u8)buf[2];
	tpbuffer[0].rect.bottom	= (u8)buf[3];
}

//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------

