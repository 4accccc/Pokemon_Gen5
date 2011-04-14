//=============================================================================
/**
 *
 *	@file		win_record.c
 *	@brief  エントリーチェッカー(優勝記録)
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
#include "win_record_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"
// print
#include "ui/print_tool.h"
#include "gamesystem/msgspeed.h"

//アプリ共通素材
#include "app/app_menu_common.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"
#include "win_record_gra.naix"

#include "system/wipe.h"

#include "app/app_keycursor.h"
#include "app/app_printsys_common.h"

#include "app/win_record.h"

// メッセージ
#include "message.naix"
#include "msg/msg_wbt_system.h"		// test

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

// パレット
enum{
	PLTID_BG_FONT				= 8,		// font
	//
	MES_CLEAR_COLOR				= 0,		// クリアカラー
	//
	BMPOAM_FONT_COLOR			= 4,		// BmpOamのカラー
};

// 最大値
enum{
	WIN_RECORD_CANVAS_MAXNUM 	= 1,		// canvas
	WIN_RECORD_BMPOAM_MAXNUM	= 8,		// bmpoam
	WIN_RECORD_EMPTY_MAXNUM		= 46,		// empty
	//
	WIN_RECORD_CMN_OBJ_MAXNUM	= 4,		// common(obj)
	WIN_RECORD_OBJ_MAXNUM		= 5,		// obj
	//
	WIN_RECORD_BGNUM_MAXNUM		= 8,		// bg
};


// OBJのリソースのインデックス
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
	RESID_OBJ_MAX,
};

// CLWK(下画面)
enum
{
	CLWKID_PANEL_00 = 0,					// パネル(00)
	CLWKID_PANEL_01,						// パネル
	CLWKID_PANEL_02,						// パネル
	CLWKID_PANEL_03,						// パネル
	CLWKID_BAR,								// バー
	//
	CLWKID_MAX,
};

//
enum{
	CLWKMODE_NORMAL = 0,
	CLWKMODE_CMN,
};

//
enum{
	EMPTY_PANEL_OFFSET = 5,					// パネルのオフセット
};

//-------------------------
// フラグ
//-------------------------
// WIN_RECORD_MAIN_WORK.MesFlag
enum{
	FLG_MES_UPDATE				= (1<<0),	// 更新
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
	GFL_CLWK	*cmn_clwk[WIN_RECORD_CMN_OBJ_MAXNUM];	// CLWK(common)
	// 通常
	GFL_CLWK	*clwk[WIN_RECORD_OBJ_MAXNUM];			// CLWK
	//
} WIN_RECORD_OBJ_WORK;

//--------------------------------------------------------------
///	メインワーク
//--------------------------------------------------------------
typedef struct
{
	HEAPID					heapID;
	// OBJ
	WIN_RECORD_OBJ_WORK		wk_obj;
	//描画設定
	WIN_RECORD_GRAPHIC_WORK	*graphic;
	//
	WIN_RECORD_PARAM 		*Param;							// 参照用として保存
	
	//フォント
	GFL_FONT				*font;
	//プリントキュー
	PRINT_QUE				*print_que;
	GFL_MSGDATA				*msg[2];								// 0:メッセージ,1:未使用
	PRINT_UTIL				PrintUtil[WIN_RECORD_CANVAS_MAXNUM];	// 文字表示用
	//
	BMPOAM_SYS_PTR			BmpOamSys;								// BMPOAM
	GFL_BMP_DATA			*BmpData[WIN_RECORD_BMPOAM_MAXNUM];		// Bmp
	BMPOAM_ACT_PTR			BmpOamWork[WIN_RECORD_BMPOAM_MAXNUM];	// BMPOAM用Work
	//
	s32						Step;							// ステップ
	u32						DebugFlag;						// debug
	s32						Mode;							// モード(0-2)
	s32						TopNum;							// 先頭の番号
	s32						MaxNum;							// 個数
	u32						MesFlag;						// メッセージ
	//
	s16						*EmptyBuffer;					// 座標用のバッファ(必ず破棄すること)
	s16						*ParamBuffer;					// パラメータのバッファ(必ず破棄すること)
} WIN_RECORD_MAIN_WORK;

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
}WIN_RECORD_OBJ_DATA;

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
}WIN_RECORD_BMPOAM_DATA;

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
}WIN_RECORD_CANVAS_DATA;

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}WIN_RECORD_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}WIN_RECORD_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}WIN_RECORD_BGSCN_DATA;


//=============================================================================
/**
 *							データ定義(要整理)
 */
//=============================================================================
//---------------
// BG関連
//---------------
// BGPAL
static const WIN_RECORD_BGPAL_DATA wr_bgpal_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_win_record_gra_win_record_bg_u_NCLR,	0,	3,},		// 上、BG
	{ PALTYPE_MAIN_BG,	NARC_win_record_gra_win_record_bg_d_NCLR,	0,	1,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const WIN_RECORD_BGCHR_DATA wr_bgchr_tbl[] = {
	{ GFL_BG_FRAME2_S,	NARC_win_record_gra_win_record_bg_u_NCGR,	0,	0,},		// 上、BG
	{ GFL_BG_FRAME3_M,	NARC_win_record_gra_win_record_bg_d_NCGR,	0,	0,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};
// BGSCN
static const WIN_RECORD_BGSCN_DATA wr_bgscn_tbl[] = {
	{ GFL_BG_FRAME2_S,	NARC_win_record_gra_win_record_bg_u_01_NSCR,	0,	0,},		// 上、下地
	{ GFL_BG_FRAME1_S,	NARC_win_record_gra_win_record_bg_u_02_NSCR,	0,	0,},		// 上、中間
	{ GFL_BG_FRAME3_M,	NARC_win_record_gra_win_record_bg_d_01_NSCR,	0,	0,},		// 下、下地

	{ 0xff, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報
//------------------
// narc, resid, paloffset, palnum
static const s16 wr_obj_main_info_tbl[] = {
	NARC_win_record_gra_win_record_obj_d_NCLR, RESID_OBJ_PLT, 0, 11, // NCL
	NARC_win_record_gra_win_record_obj_d_NCGR, RESID_OBJ_CHR, 0, 0,	// NCG
	NARC_win_record_gra_win_record_obj_d_NCER, RESID_OBJ_CEL, 0, 0, // NCE
	NARC_win_record_gra_win_record_obj_d_NANR, -1, 0, 0,			// NAR?
};

// OBJのテーブルデータ(上側)
static const WIN_RECORD_OBJ_DATA wr_obj_tbl[] = {
	{ 0,	0x30,	2,	1,	0,	1,	0,	0,},		// パネル0
	{ 1,	0x30,	2,	1,	0,	1,	1,	0,},		// パネル1
	{ 2,	0x30,	2,	1,	0,	1,	2,	0,},		// パネル2
	{ 3,	0x30,	2,	1,	0,	1,	3,	0,},		// パネル3
	{ 19,	0x30,	2,	1,	0,	1,	4,	0,},		// ゲージ上

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// BMPOAMの情報
//------------------
static const WIN_RECORD_BMPOAM_DATA wr_bmpoam_tbl[] = {
	{ 12,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル00(チャンピオン名)
	{ 12,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル00(回数)
	{ 12,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル01(チャンピオン名)
	{ 12,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル01(回数)
	{ 12,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル02(チャンピオン名)
	{ 12,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル02(回数)
	{ 12,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル03(チャンピオン名)
	{ 12,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル03(回数)

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//---------------
// CANVASの設定
//---------------
// (printutilと対応)
static const WIN_RECORD_CANVAS_DATA wr_canvas_tbl[] = {
	{ GFL_BG_FRAME0_M,	1,	1,	30,	2,	0,},		// 下、00
//	{ GFL_BG_FRAME1_M,	2,	4,	12,	2,	0,},		// 下、01
//	{ GFL_BG_FRAME1_M,	2,	7,	12,	2,	0,},		// 下、02
//	{ GFL_BG_FRAME1_M,	2,	10,	12,	2,	0,},		// 下、03
//	{ GFL_BG_FRAME1_M,	2,	13,	12,	2,	0,},		// 下、04
//	{ GFL_BG_FRAME1_M,	2,	16,	12,	2,	0,},		// 下、05

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// メッセージ関連
//------------------
// その他メッセージ
static const u8 wr_gmm_tbl[] = {
	msg_wbtsys_title_01,			// ゆうしょうかいすう
	msg_wbtsys_title_02,			// ○○かい
};

// シナリオクリア前
static const u8 wr_gmm_00_tbl[] = {
	msg_wbtsys_title_01,
	0xff,			// end
};

// シナリオクリア後
static const u8 wr_gmm_01_tbl[] = {
	msg_wbtsys_title_01,
	msg_wbtsys_title_02,
	msg_wbtsys_title_03,
	msg_wbtsys_title_04,
	0xff,			// end
	0xff,
};

// シナリオクリアかつチャンピオン後
static const u8 wr_gmm_02_tbl[] = {
	msg_wbtsys_rule_single,
	msg_wbtsys_rule_double,
	msg_wbtsys_rule_triple,
	msg_wbtsys_rule_rotation,
	msg_wbtsys_rule_single,
	msg_wbtsys_rule_rotation,
	msg_wbtsys_rule_single,
	msg_wbtsys_rule_rotation,
	0xff,			// end
	0xff,
};

//----------------
// まとめテーブル
// (Modeで参照)
//----------------
static const u8 *wr_gmm_all_tbl[] = {
	wr_gmm_00_tbl,
	wr_gmm_01_tbl,
	wr_gmm_02_tbl,
};

// 個数最大(要調整)
static const u8 wr_gmm_max_tbl[] = {
	1,			// シナリオクリア前
	4,			// シナリオクリア後
	8,			// シナリオクリア&チャンピオン後
	0xff,		// pad
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT WinRecordProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT WinRecordProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT WinRecordProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
///	以下に追加
//=====================================
static void wr_init( WIN_RECORD_MAIN_WORK* wk );
static void wr_main( WIN_RECORD_MAIN_WORK* wk );
static void wr_exit( WIN_RECORD_MAIN_WORK* wk );
static void wr_up(   WIN_RECORD_MAIN_WORK* wk );
static void wr_down( WIN_RECORD_MAIN_WORK* wk );
static void wr_key_all( WIN_RECORD_MAIN_WORK* wk );
//
static s16 wr_get_empty( WIN_RECORD_MAIN_WORK *wk, const s32 num );
//
static void wr_init_bg( WIN_RECORD_MAIN_WORK* wk );
//
static void wr_init_obj_clwk( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const WIN_RECORD_OBJ_DATA *obj_adr, const s16 *info_adr );
static void wr_init_obj_res( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr );
static void wr_init_obj( WIN_RECORD_MAIN_WORK* wk );
static void wr_exit_obj( WIN_RECORD_MAIN_WORK* wk );
static void wr_chg_objseq( WIN_RECORD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
static void wr_set_obj_visible( WIN_RECORD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
//
static void wr_init_obj_bmpoam( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const WIN_RECORD_BMPOAM_DATA *bmp_adr, const s16 *info_adr );
static void wr_disp_bmpoam( WIN_RECORD_MAIN_WORK* wk, const wnum, const STRBUF *strbuf );

//
static void wr_init_string( WIN_RECORD_MAIN_WORK* wk );
static void wr_disp_string( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, STRBUF* strbuf, const u32 dispmode );
//static void wr_disp_string_id( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 bufferid, const s32 mesid, const u32 dispmode );
static void wr_update_string( WIN_RECORD_MAIN_WORK* wk, const s32 mode );
static void wr_exit_string( WIN_RECORD_MAIN_WORK* wk );
static void wr_clear_canvas( WIN_RECORD_MAIN_WORK* wk, const s32 mode );
//
static void wr_chk_string( WIN_RECORD_MAIN_WORK* wk );
static void wr_disp_playername( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 xpos, const s32 ypos, const u32 dispmode );
//
static void wr_set_panel( WIN_RECORD_MAIN_WORK* wk, const s32 wnum, const s32 num, const BOOL bflg );
//
static void wr_init_param( WIN_RECORD_MAIN_WORK* wk );
//
static void wr_debug( WIN_RECORD_MAIN_WORK* wk );

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA WinRecordProcData = 
{
	WinRecordProc_Init,
	WinRecordProc_Main,
	WinRecordProc_Exit,
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
 *	@param	*pwk					WIN_RECORD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WinRecordProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	WIN_RECORD_MAIN_WORK 	*wk;
	
	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UI_DEBUG, UI_TEMPLATE_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(WIN_RECORD_MAIN_WORK), HEAPID_UI_DEBUG );
	GFL_STD_MemClear( wk, sizeof(WIN_RECORD_MAIN_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_UI_DEBUG;
	
	// 参照用
	wk->Param = pwk;
	
	//描画設定初期化(上:サブ、下:メイン)
	wk->graphic	= WIN_RECORD_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );
	
	//フォント作成
	wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	
	// メッセージ
	wk->msg[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_system_dat, wk->heapID );
	wk->msg[1] = 0;
	
	//PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	//
	//----------------
	// 通常はこの中で初期化
	//----------------
	wr_init( wk );
	
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
 *	@param	*pwk					WIN_RECORD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WinRecordProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	WIN_RECORD_MAIN_WORK* wk = mywk;
	
	//----------------
	// 処理を記述
	//----------------
	wr_exit( wk );
	
	//メッセージ破棄
	if(wk->msg[0]){
		GFL_MSG_Delete( wk->msg[0] );
	}
	if(wk->msg[1]){
		GFL_MSG_Delete( wk->msg[1] );
	}
	
	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	//
	//FONT
	GFL_FONT_Delete( wk->font );
	// 会話ウインドウ用
//	GFL_TCBL_Exit( wk->MsgTcblSys );
	
	//描画設定破棄
	WIN_RECORD_GRAPHIC_Exit( wk->graphic );
	
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
 *	@param	*pwk					WIN_RECORD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WinRecordProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	WIN_RECORD_MAIN_WORK* wk = mywk;
	
	switch ( *seq )
	{
		case 0:
			if (WIPE_SYS_EndCheck())
			{
				(*seq) ++;
			}
			break;
		case 1:
//			if(!(wk->DebugFlag & FLG_DBG_INPUT)){
//				// 終了判定
//				if(wk->Step == STEP_END){
//					(*seq) ++;
//				}
//			}else{
				if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
				{
					(*seq) ++;
				}
//			}
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
	wr_main( wk );
	
	// 会話ウインドウ用
//	GFL_TCBL_Main( wk->MsgTcblSys );
	
	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );
	
	//2D描画
	WIN_RECORD_GRAPHIC_2D_Draw( wk->graphic );
	
	//3D描画
	WIN_RECORD_GRAPHIC_3D_StartDraw( wk->graphic );
	
	WIN_RECORD_GRAPHIC_3D_EndDraw( wk->graphic );
	
	return GFL_PROC_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(init)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init( WIN_RECORD_MAIN_WORK* wk )
{
//	s32		i;
//	s32		buf[2];
//	STRBUF	*strbuf;
	
	// 初期化
	// それほど複雑ではないので、
	// 上、下画面をまとめて処理
	wk->Step 		= STEP_INIT;
	wk->DebugFlag	= 0;
	wk->Mode		= 0;
	wk->TopNum		= 0;
	
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_WIN_RECORD_GRA, NARC_win_record_gra_win_record_pos_tblbin, wk->heapID );
	// パラメータ用のバッファ取得(必ず破棄すること)
	wk->ParamBuffer = GFL_ARC_LoadDataAlloc( ARCID_WIN_RECORD_GRA, NARC_win_record_gra_win_record_param_tblbin, wk->heapID );
	//
	//---------------
	// bgの初期化
	//---------------
	wr_init_bg( wk );
	
	//---------------
	// 文字列関連の初期化
	//---------------
	
	// パレットは8番に転送
	{
		ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		//上画面フォントパレット
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, PLTID_BG_FONT*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
	
	// 文字初期化
	wr_init_string( wk );
	
	//------------
	// 文字列表示
	//------------
	// プレイヤ名
	wr_disp_playername( wk, 0, 0, 0, PRINTTOOL_MODE_LEFT );
	// トーナメント名
//	wr_disp_string_id( wk, 0, 128, 0, 0, wr_gmm_tbl[0], PRINTTOOL_MODE_LEFT );
	// 更新
	wr_update_string( wk, 0 );
	// メッセージ更新
//	wk->MesFlag |= FLG_MES_UPDATE;
	//
	//------------
	// OBJ初期化
	//------------
	wr_init_obj( wk );
	
	//-----------------
	// パラメータ関連
	//-----------------
	wr_init_param( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_main( WIN_RECORD_MAIN_WORK* wk )
{
	// debug
	wr_debug( wk );
	
	// 上画面
	wr_up( wk );
	// 下画面
	wr_down( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(exit)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_exit( WIN_RECORD_MAIN_WORK* wk )
{
	// 上、下画面をまとめて処理
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	// paramのバッファクリア
	GFL_HEAP_FreeMemory( wk->ParamBuffer );
	
	// 文字表示用
	wr_exit_string( wk );
	// OBJ破棄
	wr_exit_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(上画面)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_up( WIN_RECORD_MAIN_WORK* wk )
{
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(下画面)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_down( WIN_RECORD_MAIN_WORK* wk )
{
	wr_key_all( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理まとめ
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_key_all( WIN_RECORD_MAIN_WORK* wk )
{
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		
		// LV1
		case STEP_LV1:
			wr_chk_string( wk );
			break;
		
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
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
static s16 wr_get_empty( WIN_RECORD_MAIN_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= WIN_RECORD_EMPTY_MAXNUM)){
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
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_bg( WIN_RECORD_MAIN_WORK* wk )
{
	s32		i;
	s32		tmp;
	const WIN_RECORD_BGPAL_DATA	*padr = &wr_bgpal_tbl[0];
	const WIN_RECORD_BGCHR_DATA	*cadr = &wr_bgchr_tbl[0];
	const WIN_RECORD_BGSCN_DATA	*sadr = &wr_bgscn_tbl[0];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_WIN_RECORD_GRA, wk->heapID );
	
	// パレット転送
	for(i=0;i<WIN_RECORD_BGNUM_MAXNUM;i++){
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
	for(i=0;i<WIN_RECORD_BGNUM_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, wk->heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<WIN_RECORD_BGNUM_MAXNUM;i++){
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
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const WIN_RECORD_OBJ_DATA* アドレス
 *	@param	const s16*				アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_obj_clwk( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const WIN_RECORD_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16				tmp;
	s16				pos[2];
	s32 			i;
	BOOL			bflg;
	GFL_CLWK_DATA 	cldata;
	CLWK_FLIP_TYPE	flip;
    GFL_CLUNIT*		p_clunit 	= WIN_RECORD_GRAPHIC_GetClunit( wk->graphic );
	// アドレスセット
	const WIN_RECORD_OBJ_DATA *adr = obj_adr;
	const s16 *iadr 			= info_adr;
	
	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
	//
	for(i=0;i<WIN_RECORD_OBJ_MAXNUM;i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		pos[0] = wr_get_empty(wk, tmp*2);
		pos[1] = wr_get_empty(wk, tmp*2+1);
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
 *	@brief  OBJ初期化(res)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_obj_res( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr )
{
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_WIN_RECORD_GRA, wk->heapID );
    GFL_CLUNIT*	p_clunit = WIN_RECORD_GRAPHIC_GetClunit( wk->graphic );
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
 *	@brief  OBJ初期化
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_obj( WIN_RECORD_MAIN_WORK* wk )
{
    GFL_CLUNIT*	p_clunit = WIN_RECORD_GRAPHIC_GetClunit( wk->graphic );
	// リソース読み込み
	wr_init_obj_res( wk, CLSYS_DRAW_MAIN, &wr_obj_main_info_tbl[0] );
	// CLWK登録
	wr_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &wr_obj_tbl[0], &wr_obj_main_info_tbl[0] );
	//
	// BmpOAM初期化
	wk->BmpOamSys = BmpOam_Init( wk->heapID, p_clunit );
	// BMPOAM登録
	wr_init_obj_bmpoam( wk, CLSYS_DRAW_MAIN, &wr_bmpoam_tbl[0], &wr_obj_main_info_tbl[0] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_exit_obj( WIN_RECORD_MAIN_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<WIN_RECORD_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk[i] );
	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_OBJ_CEL] );
	//
	// BmpOAM後処理
	for(i=0;i<WIN_RECORD_BMPOAM_MAXNUM;i++){
		BmpOam_ActorDel( wk->BmpOamWork[i] );
		GFL_BMP_Delete( wk->BmpData[i] );
	}
	BmpOam_Exit( wk->BmpOamSys );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_chg_objseq( WIN_RECORD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	if(wmode == CLWKMODE_NORMAL){			// 通常
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
	}else{
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.cmn_clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.cmn_clwk[wnum], TRUE );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32		objワーク番号
 *	@param	const BOOL		(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_set_obj_visible( WIN_RECORD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// 表示/非表示
	if(wmode == CLWKMODE_NORMAL){
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
	}else{
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.cmn_clwk[wnum], bflg );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(bmpoam)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const WIN_RECORD_BMPOAM_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_obj_bmpoam( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const WIN_RECORD_BMPOAM_DATA *bmp_adr, const s16 *info_adr )
{
	s16							tmp;
	s32							i;
	s32							buf[2];
	BMPOAM_ACT_DATA				actdata;
	const WIN_RECORD_BMPOAM_DATA	*adr  = bmp_adr;
	const s16					*iadr = info_adr;
	GFL_CLACTPOS				cpos;
	//
	for(i=0;i<WIN_RECORD_BMPOAM_MAXNUM;i++){
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
		cpos.x = wr_get_empty( wk, tmp*2 );
		cpos.y = wr_get_empty( wk, tmp*2+1 );
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
 *	@brief  OBJ初期化(bmpoam)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const WIN_RECORD_BMPOAM_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_disp_bmpoam( WIN_RECORD_MAIN_WORK* wk, const wnum, const STRBUF *strbuf )
{
	GFL_BMP_Clear( wk->BmpData[wnum], 0 );
	PRINTSYS_PrintQueColor( wk->print_que, wk->BmpData[wnum], 0, 0, strbuf, wk->font, PRINTSYS_LSB_Make(15,14,0) );
	BmpOam_ActorBmpTrans( wk->BmpOamWork[wnum] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(現在、Canvasの作成)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_string( WIN_RECORD_MAIN_WORK* wk )
{
	s32		i;
	const WIN_RECORD_CANVAS_DATA	*adr = &wr_canvas_tbl[0];
	
	for(i=0;i<WIN_RECORD_CANVAS_MAXNUM;i++){
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
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x)
 *	@param	const s32			表示座標(y)
 *	@param	STRBUF* 			表示文字列
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_disp_string( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, STRBUF* strbuf, const u32 dispmode )
{
#if 0
	// 表示
	PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(wk->PrintUtil[mode].win), x, y, strbuf, wk->font, PRINTSYS_LSB_Make(15,14,0) );
#else
	// 表示
	PRINTTOOL_PrintColor(
				&wk->PrintUtil[mode],			// プリントユーティル ( BMPWIN )
				wk->print_que,					// プリントキュー
				x, y,							// 表示基準座標
				strbuf,							// 文字列
				wk->font,						// フォント
//				PRINTSYS_LSB_Make(15,14,0),		// カラー
				PRINTSYS_LSB_Make(10,11,0),		// カラー
				PRINTTOOL_MODE_LEFT );			// 中央
#endif
	// 転送
//	GFL_BMPWIN_MakeScreen( wk->PrintUtil[mode].win );
//	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[mode].win) );
	// 転送?
//	PRINT_UTIL_Trans( &wk->PrintUtil[mode], wk->print_que );
}

#if 0	// 保留
//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示(id指定)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x,y)
 *	@param	const s32			mes[bufferid]
 *	@param	const s32			gmmのid
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_disp_string_id( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 bufferid, const s32 mesid, const u32 dispmode )
{
	STRBUF		*strbuf;
	
	// メッセージバッファセット
	strbuf = GFL_MSG_CreateString( wk->msg[bufferid], mesid );
#if 0
	// 表示
	PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(wk->PrintUtil[mode].win), x, y, strbuf, wk->font, PRINTSYS_LSB_Make(15,14,0) );
#else
	// 表示
	PRINTTOOL_PrintColor(
				&wk->PrintUtil[mode],			// プリントユーティル ( BMPWIN )
				wk->print_que,					// プリントキュー
				x, y,							// 表示基準座標
				strbuf,							// 文字列
				wk->font,						// フォント
//				PRINTSYS_LSB_Make(15,14,0),		// カラー
				PRINTSYS_LSB_Make(10,11,0),		// カラー
				PRINTTOOL_MODE_LEFT );			// 中央
#endif
	// 転送
//	GFL_BMPWIN_MakeScreen( wk->PrintUtil[mode].win );
//	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[mode].win) );
	// 転送?
//	PRINT_UTIL_Trans( &wk->PrintUtil[mode], wk->print_que );
	//
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
}
#endif

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(現在、Canvasの作成)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_update_string( WIN_RECORD_MAIN_WORK* wk, const s32 mode )
{
	// 転送
	GFL_BMPWIN * bmpwin = wk->PrintUtil[mode].win;
	//
	// これは入れないとだめらしい
	PRINT_UTIL_Trans( &wk->PrintUtil[mode], wk->print_que );
	//
	GFL_BMPWIN_TransVramCharacter( bmpwin );					//transfer characters
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );	//transfer screen
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示終了
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_exit_string( WIN_RECORD_MAIN_WORK* wk )
{
	s32		i;
	
	for(i=0;i<WIN_RECORD_CANVAS_MAXNUM;i++){
		GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_clear_canvas( WIN_RECORD_MAIN_WORK* wk, const s32 mode )
{
	// clear
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_CLEAR_COLOR );
//	GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[mode].win );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレイヤ表示
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const mode				(PrintUtil[mode]のmode)
 *	@param	const s32				x座標
 *	@param	const s32				y座標
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_disp_playername( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 xpos, const s32 ypos, const u32 dispmode )
{
	GFL_BMPWIN 	*bmpwin	= wk->PrintUtil[mode].win;
	STRBUF 	*strbuf		= GFL_STR_CreateBuffer( 64, wk->heapID );
	// 名前取得
	GAMEDATA 	*gamedata 	= GAMESYSTEM_GetGameData( wk->Param->gamesys );
	const MYSTATUS *mystat	= GAMEDATA_GetMyStatus( gamedata );
	MyStatus_CopyNameString( mystat, strbuf );
	wr_disp_string( wk, mode, xpos, ypos, strbuf, dispmode );
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				PrintUtil[tnum]のtnum
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_chk_string( WIN_RECORD_MAIN_WORK* wk )
{
	s32		i;
	s32		tmp;
	s32		flg=0;
//	const u8	*adr = wr_gmm_all_tbl[wk->Mode];
	const u8	*adr = wr_gmm_all_tbl[2];
	
	tmp = wk->TopNum;
	
	// 左
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
		tmp -= 4;
		if(tmp < 0){
			tmp = 4;
		}
		wk->TopNum = tmp;
		flg = 1;
	}
	// 右
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
		tmp += 4;
		if(tmp > 4){
			tmp = 0;
		}
		wk->TopNum = tmp;
		flg = 1;
	}
	//
	if(wk->MesFlag & FLG_MES_UPDATE){
		wk->MesFlag &= ~FLG_MES_UPDATE;
		flg = 1;
	}
	//
	// 更新
	if(flg == 1){
#if 0
		{
			s32			i;
			s32			mesid;
			STRBUF		*strbuf;
			
			// 表示/非表示と座標セット
//			for(i=0;i<4;i++){
//				wr_set_panel( wk, i, i, TRUE );
//			}
			//
			for(i=0;i<8;i++){
				// メッセージバッファセット
				strbuf = GFL_MSG_CreateString( wk->msg[0], adr[i] );
				//
				wr_disp_bmpoam( wk, i, strbuf );
				// 破棄
				GFL_STR_DeleteBuffer( strbuf );
			}
		}
#endif
	}
}

//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
/**
 *	@brief  パネル表示まとめ
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			ワーク番号
 *	@param	const s32			シーケンス番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_set_panel( WIN_RECORD_MAIN_WORK* wk, const s32 wnum, const s32 num, const BOOL bflg )
{
	s16				buf[4];
	GFL_CLACTPOS	pos;
	
	// 表示/非表示
	wr_set_obj_visible( wk, CLWKMODE_NORMAL, wnum, bflg );
	// bmpoam
	BmpOam_ActorSetDrawEnable( wk->BmpOamWork[2*wnum], bflg );
	
	// シーケンスの変更
	wr_chg_objseq( wk, CLWKMODE_NORMAL, wnum, num );
	
	// bmpoamは座標も念のため再セットしておく
	buf[0] = wr_get_empty( wk,  EMPTY_PANEL_OFFSET*2 );
	buf[1] = wr_get_empty( wk,  EMPTY_PANEL_OFFSET*2+1 );
	buf[2] = wr_get_empty( wk, (EMPTY_PANEL_OFFSET+1)*2 );
	buf[3] = wr_get_empty( wk, (EMPTY_PANEL_OFFSET+1)*2+1 );
	//
	// 座標の取得
	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[wnum], &pos, CLSYS_DEFREND_MAIN );
	//
	// 座標セット
	// (左側)
	BmpOam_ActorSetPos( wk->BmpOamWork[2*wnum],   (pos.x+buf[0]), (pos.y+buf[1]) );
	// (右側)
	BmpOam_ActorSetPos( wk->BmpOamWork[2*wnum+1], (pos.x+buf[2]), (pos.y+buf[3]) );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータ初期化(ここでパラメータを参照)
 *
 *	@param	WIN_ROUND_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_param( WIN_RECORD_MAIN_WORK* wk )
{
	s32			i;
	s32			mesid;
	STRBUF		*strbuf;
	const u8	*adr = wr_gmm_all_tbl[2];
	
	//
	for(i=0;i<8;i++){
		// メッセージバッファセット
		strbuf = GFL_MSG_CreateString( wk->msg[0], adr[i] );
		// 表示
		wr_disp_bmpoam( wk, i, strbuf );
		// 破棄
		GFL_STR_DeleteBuffer( strbuf );
	}
	//
	// 表示/非表示と座標セット
	for(i=0;i<4;i++){
		wr_set_panel( wk, i, i, TRUE );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  デバッグ
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_debug( WIN_RECORD_MAIN_WORK* wk )
{
#ifdef PM_DEBUG
	// debug mode
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		wk->Mode++;
		if(wk->Mode > 2){
			wk->Mode = 0;
		}
	}
#endif		// PM_DEBUG
}

