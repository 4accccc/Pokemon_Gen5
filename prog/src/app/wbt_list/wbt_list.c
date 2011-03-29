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
#define UI_TEMPLATE_OAM_MAPMODEL  // マップモデルをOAMで表示    //nsbtx_to_clwkをオーバーレイに移したのでコメントアウトにしました 10/06/11 nagihashi

// debug表示が必要なときはセット
#define _DEBUG_LOCAL_
//#undef  _DEBUG_LOCAL_

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

// フラグ(wk->Flag:現在未使用)
enum{
	FLG_PANEL_IN 		= (1<<0),	// パネルイン
	FLG_PANEL_IN_END	= (1<<1),	// パネルイン終了
	FLG_WAIT_END		= (1<<2),	// 待ち終了
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
// キャンバスの数
// (PrintUtilの番号と対応)(要整理)
//-----------------------------------
enum{
	PUTIL_DOWN_0 = 0,				// 下画面の0
	PUTIL_DOWN_1,					// 下画面の1
	
	PUTIL_MAX,
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
	u32			res[RESID_OBJ_MAX]; 	//リソース
	GFL_CLWK	*clwk[CLWKID_MAX];		//CLWK
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
	s16			rate;		// 加算値
	s16			calcnum;	// 計算結果
	s16			flag;		// フラグ
	s16			pad;
} WBTLIST_MOVE_WORK;

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
//	GFL_MSGDATA				*msg;
	GFL_MSGDATA				*msg[2];					// 0:名詞、1:メッセージ(テストで使用)

#ifdef UI_TEMPLATE_OAM_MAPMODEL
  // OAMマップモデル
  u8						clwk_oam_mmdl_count;
  UI_EASY_CLWK_RES			clres_oam_mmdl;
  GFL_CLWK					*clwk_oam_mmdl[ WBT_TRAINER_MAX ];
#endif //UI_TEMPLATE_OAM_MAPMODEL
	
	//---------------------
	// 以下に追加記入
	//---------------------
	WBTLIST_PARAM 			*Param;						// 参照用として保存(大丈夫?)
	
	BMPOAM_SYS_PTR			BmpOamSys;					// BMPOAM
	GFL_BMP_DATA			*BmpData[WBT_TRAINER_MAX];	// Bmp
	BMPOAM_ACT_PTR			BmpOamWork[WBT_TRAINER_MAX];// Work
	PRINT_UTIL				PrintUtil[PUTIL_MAX];		// 文字表示用
	PRINT_UTIL				MsgPrintUtil[2];			// 文字表示用
	PRINT_STREAM			*PrintStream;				// PrintStream
	GFL_TCBLSYS				*MsgTcblSys;				// メッセージ用タスク
	STRBUF					*MsgBuffer;					// メッセージ用バッファ
	//
	s32						Mode;						// モード(段階)
	s32						Step;						// ステップ
	s32						DispNum;					// 表示回数
	u32						Flag;						// フラグ
	s32						Timer[TIMER_MAX];			// タイマ
	WBTLIST_MOVE_WORK		MoveWork[MWORK_MAX];		// 移動
	//
	s16						*EmptyBuffer;				// 座標用のバッファ(必ず破棄すること)
} WBTLIST_MAIN_WORK;


//-----------------------------------------
// OBJ用の定義
//------------------------------------------
typedef struct{
	u8				anmseq;			// アニメシーケンス(0xff終了コード)
	u8				softpri;		// OBJ(プライオリティ)
	u8				bgpri;			// BG(プライオリティ)
	u8				visible;		// 表示/非表示
	u8				palnum;			// パレット番号
	u8				dispflag;		// 表示フラグ
	u8				empty_num;		// empty番号(これから座標を参照)
	u8				pad;
}WBTLIST_OBJ_DATA;

//-----------------------------------------
// 文字用の定義(表示領域の設定:CANVAS)
//------------------------------------------
typedef struct{
	u8				bgnumber;		// BG上下(BGナンバー定義なので注意)(0xffの場合は終了判定)
	u8				x;				// 座標
	u8				y;
	u8				width;			// サイズ
	u8				height;
	u8				palnum;			// パレット番号
}WBTLIST_CANVAS_DATA;

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

// OBJリソース
const s32 obj_res_tbl[] = {
	NARC_wbt_list_gra_wbt_list_obj_u_NCLR,		// NCL
	NARC_wbt_list_gra_wbt_list_obj_u_NCGR,		// NCG
	NARC_wbt_list_gra_wbt_list_obj_u_NCER,		// NCE
	NARC_wbt_list_gra_wbt_list_obj_u_NANR,		// NAR?
};

// OBJのテーブルデータ
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

// CANVASの設定(printutilと対応)
const WBTLIST_CANVAS_DATA wbt_canvas_tbl[] = {
	{ GFL_BG_FRAME1_S,	4,	4,	24,	2,	2,},		// 下、up
	{ GFL_BG_FRAME1_S,	10,	17,	11,	2,	2,},		// 下、down

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

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

#ifdef UI_TEMPLATE_OAM_MAPMODEL
//-------------------------------------
///	OAMでマップモデル表示
//=====================================
static void WBTList_OAM_MAPMODEL_CreateCLWK( WBTLIST_MAIN_WORK *wk, u16 tex_idx, u8 ptn_ofs, GFL_CLUNIT *unit, HEAPID heapID );
static void WBTList_OAM_MAPMODEL_DeleteCLWK( WBTLIST_MAIN_WORK* wk );
#endif //UI_TEMPLATE_OAM_MAPMODEL

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
//static void wbt_key_01( WBTLIST_MAIN_WORK* wk );
//static void wbt_key_02( WBTLIST_MAIN_WORK* wk );
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
static void wbt_exit_obj( WBTLIST_MAIN_WORK* wk );
//
static void wbt_init_movework( WBTLIST_MAIN_WORK* wk, const s32 wnum, const s16 sx, const s16 ex, const s16 frame );
static s16  wbt_main_movework( WBTLIST_MAIN_WORK* wk, const s32 wnum );
static BOOL wbt_is_movework(   WBTLIST_MAIN_WORK* wk, const s32 wnum );
//
static void wbt_init_move_panel( WBTLIST_MAIN_WORK* wk, const s32 mode );
static void wbt_main_move_panel( WBTLIST_MAIN_WORK* wk );
//
static void wbt_disp_bomb( WBTLIST_MAIN_WORK* wk, const s32 num );
//
static void wbt_set_meswin( WBTLIST_MAIN_WORK* wk );
static BOOL wbt_main_meswin( WBTLIST_MAIN_WORK* wk );
//
static s16 wbt_get_empty( WBTLIST_MAIN_WORK *wk, const s32 num );
//
static void wbt_set_gageanim( WBTLIST_MAIN_WORK *wk, const s32 wnum, const s32 mode, const s32 umode );
static BOOL wbt_is_gageanim( WBTLIST_MAIN_WORK *wk, const s32 wnum );
static void wbt_visible_gage( WBTLIST_MAIN_WORK *wk, const s32 wnum, const BOOL bflg );

//
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
	wk->msg[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_mictest_dat, wk->heapID );
//	wk->msg[1] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_tournament_dat, wk->heapID );
	wk->msg[1] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_mictest_dat, wk->heapID );
	
	//PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	//
	// 会話ウインドウ用
	wk->MsgTcblSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 32 , 32 );
	
	//BGリソース読み込み
	WBTList_BG_LoadResource( &wk->wk_bg, wk->heapID );
	
#ifdef UI_TEMPLATE_OAM_MAPMODEL
	//OAMマップモデルの読み込み
	{
		// 主人公は現状 0=後ろ, 1=後ろアニメ1, 2=後ろアニメ2, 3=正面....
		// トレーナーなどは違う可能性があるので注意。
		u16 ptn_ofs = 3;
		
		GFL_CLUNIT	*clunit	= WBTLIST_GRAPHIC_GetClunit( wk->graphic );
		WBTList_OAM_MAPMODEL_CreateCLWK( wk, NARC_fldmmdl_mdlres_hero_nsbtx, ptn_ofs, clunit, wk->heapID );
	}
#endif //UI_TEMPLATE_OAM_MAPMODEL
	
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
	
#ifdef UI_TEMPLATE_OAM_MAPMODEL
  //OAMマップモデルの破棄
  WBTList_OAM_MAPMODEL_DeleteCLWK( wk );
#endif //UI_TEMPLATE_OAM_MAPMODEL
	
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
	//
}


#ifdef UI_TEMPLATE_OAM_MAPMODEL
//=============================================================================
/**
 *	OAM_MAPMODEL
 */
//=============================================================================

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

#endif //UI_TEMPLATE_OAM_MAPMODEL


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
	
	// 文字列(test)
	strbuf = GFL_MSG_CreateString( wk->msg[0], 0 );
	wk->MsgBuffer = GFL_MSG_CreateString( wk->msg[1], 0 );
	
	// 初期化
	// それほど複雑ではないので、
	// 上、下画面をまとめて処理
	wk->Mode = MODE_ROUND1;		// 外からのパラメータ
	wk->Step = STEP_INIT;
	//
	// 表示回数
	wk->DispNum = 1;
	if(wk->Mode == MODE_ROUND2){
		wk->DispNum = 4;
	}else if(wk->Mode == MODE_ROUND3){
		wk->DispNum = 2;
	}
	wk->Flag = 0;
	
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
	for(i=0;i<WBT_TRAINER_MAX;i++){
	    GFL_BMP_Clear( wk->BmpData[i], 0 );
		PRINTSYS_PrintQueColor( wk->print_que, wk->BmpData[i], 0, 0, strbuf, wk->font, PRINTSYS_LSB_Make(15,14,0) );
		BmpOam_ActorBmpTrans( wk->BmpOamWork[i] );
	}
	// 削除
//	GFL_STR_DeleteBuffer( strbuf );
	
	// 会話ウィンドウセット(キャラデータを転送)
	BmpWinFrame_GraphicSet( GFL_BG_FRAME0_M, 1, 8, MENU_TYPE_SYSTEM, wk->heapID);
	// canvasの設定
	wk->MsgPrintUtil[0].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 0, 19, 32, 5, 8, GFL_BMP_CHRAREA_GET_B );
	// 表示
	BmpWinFrame_Write( wk->MsgPrintUtil[0].win, WINDOW_TRANS_OFF, 1, 8 );
	//
	// set
//	wbt_set_meswin( wk );
#if 1
	// stream設定=>ここのstrbufは終わるまで削除してはいけない
	wk->PrintStream = PRINTSYS_PrintStream(
				wk->MsgPrintUtil[0].win, 0, 0, wk->MsgBuffer, wk->font,
				MSGSPEED_GetWait(), wk->MsgTcblSys, 4, wk->heapID, 15 );
	// スクリーン転送らしい
	GFL_BMPWIN_MakeTransWindow( wk->MsgPrintUtil[0].win );
#endif
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
			// 一度非表示
			for(i=0;i<6;i++){
				wbt_visible_gage( wk, i, FALSE );
			}
			// 3回戦は2回戦までの情報を表示
			if(wk->Mode == MODE_ROUND3){
				for(i=0;i<4;i++){
					wbt_set_gageanim( wk, i, 1, 1 );
				}
			}
			break;
		
		// LV1(パネルイン)
		case STEP_LV1:
			// 初期化
			wbt_main_move_panel( wk );
			// 移動終了
			if( wbt_is_movework( wk, MWORK_PANEL_L ) == FALSE ){
				wk->Step = STEP_LV2;
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
				}
				wk->DispNum--;
				if(wk->DispNum < 0){
					wk->DispNum = 0;
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
				wk->Step = STEP_LV4;
				wbt_init_timer( wk, TIMER_BOMB, info_tbl[0] );
				// 爆発アニメ
				wbt_disp_bomb( wk, 0 );
			}
			break;
		
		//
		// LV4(爆破)
		case STEP_LV4:
			timer = wbt_main_timer( wk, TIMER_BOMB );
			if(timer <= 0){
				// 爆破アニメ終了=>ゲージ伸び
				wk->Step = STEP_LV5;
				wbt_init_timer( wk, TIMER_GAGE, info_tbl[2] );
				// ゲージ処理(test)
				buf[0] = 0;
				if(wk->Mode == MODE_ROUND2){
					buf[0] = 3-wk->DispNum;
				}else if(wk->Mode == MODE_ROUND3){
					buf[0] = 5-wk->DispNum;
				}
				wbt_set_gageanim( wk, buf[0], 0, 1 );
			}
			break;
		
		// LV5(ゲージ伸びアニメ待ち)
		case STEP_LV5:
			// タイマは動かしておく
			wbt_main_timer( wk, TIMER_GAGE );
			// ゲージ処理(test)
			buf[0] = 0;
			if(wk->Mode == MODE_ROUND2){
				buf[0] = 3-wk->DispNum;
			}else if(wk->Mode == MODE_ROUND3){
				buf[0] = 5-wk->DispNum;
			}
			if( wbt_is_gageanim( wk, buf[0] ) == FALSE ){
				// ゲージ伸び終了
				wk->Step = STEP_LV5+10;
				wbt_set_gageanim( wk, buf[0], 1, 1 );
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
			}
			break;
		
		// LV7(メッセージ待ち)
		case STEP_LV7:
			// メッセージ待ち
			if( wbt_main_meswin( wk ) == FALSE ){
				//
				wk->Step = STEP_LV3;
				wk->DispNum--;
				if(wk->DispNum < 0){
					wk->DispNum = 0;
					wk->Step = STEP_END;
				}
				wbt_init_move_panel( wk, MOVE_PANEL_OUT );
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
 *	@brief  キー入力処理(必要になったら復活)
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
#if 0
static void wbt_key_01( WBTLIST_MAIN_WORK* wk )
{
}

static void wbt_key_02( WBTLIST_MAIN_WORK* wk )
{
}
#endif

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理まとめ
 *
 *	@param	WBTLIST_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
// 関数のテーブル
static void (*(wbt_key_ftbl[]))( WBTLIST_MAIN_WORK* ) = {
	wbt_key_00,
//	wbt_key_01,			// 必要になったら復活
//	wbt_key_02,			// 必要になったら復活
};

static void wbt_key_all( WBTLIST_MAIN_WORK* wk )
{
//	s32		mode=wk->Mode;		// 必要になったら復活
	s32		mode=MODE_ROUND1;
	
	// 念のため
	if((mode < MODE_ROUND1)||(mode >= MODE_ROUND_MAX))
	{
#ifdef _DEBUG_LOCAL_
		OS_Printf("MODE ERROR\n");
#endif
		return;
	}
	
	// 処理
	wbt_key_ftbl[mode](wk);
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
	
	for(i=0;i<PUTIL_MAX;i++){
		// 終了条件
		if(adr->bgnumber == 0xff){
			break;
		}
		// セット
		wk->PrintUtil[i].win = GFL_BMPWIN_Create( adr->bgnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		adr++;
	}
#if 0
//	wk->PrintUtil[0].win = GFL_BMPWIN_Create( GFL_BG_FRAME1_S,  4, 4, 24, 2, 2, GFL_BMP_CHRAREA_GET_B );	// 上
//	wk->PrintUtil[1].win = GFL_BMPWIN_Create( GFL_BG_FRAME1_S, 10,17, 11, 2, 2, GFL_BMP_CHRAREA_GET_B );	// 下
#endif
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
	
	for(i=0;i<PUTIL_MAX;i++){
		GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
	}
	// PrintStream後処理
	GFL_STR_DeleteBuffer( wk->MsgBuffer );
	PRINTSYS_PrintStreamDelete( wk->PrintStream );
	GFL_BMPWIN_Delete( wk->MsgPrintUtil[0].win );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(後で整理)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_init_obj( WBTLIST_MAIN_WORK* wk )
{
	//リソース読みこみ
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_WBTLIST_GRA, wk->heapID );
    GFL_CLUNIT*	p_clunit = WBTLIST_GRAPHIC_GetClunit( wk->graphic );
	// セルアニメ用の処理
	wk->wk_obj.res[ RESID_OBJ_PLT ] = GFL_CLGRP_PLTT_RegisterEx( p_handle, obj_res_tbl[0], CLSYS_DRAW_MAIN, 0, 0, 5, wk->heapID );
	wk->wk_obj.res[ RESID_OBJ_CHR ] = GFL_CLGRP_CGR_Register(  p_handle, obj_res_tbl[1], FALSE, CLSYS_DRAW_MAIN,    wk->heapID );
//	wk->wk_obj.res[ RESID_OBJ_PLT ] = GFL_CLGRP_PLTT_RegisterEx( p_handle, obj_res_tbl[0], CLSYS_DRAW_SUB, 0, 0, 3, wk->heapID );
//	wk->wk_obj.res[ RESID_OBJ_CHR ] = GFL_CLGRP_CGR_Register(  p_handle, obj_res_tbl[1], FALSE, CLSYS_DRAW_SUB,    wk->heapID );
	wk->wk_obj.res[ RESID_OBJ_CEL ] = GFL_CLGRP_CELLANIM_Register( p_handle, obj_res_tbl[2], obj_res_tbl[3], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
	//
	// CLWK登録
	{
		s16				tmp;
		s16				pos[2];
		s32 			i;
		BOOL			bflg;
		GFL_CLWK_DATA 	cldata;
		CLWK_FLIP_TYPE	flip;
		const WBTLIST_OBJ_DATA *adr = &wbt_obj_tbl[0];		// アドレスセット
		
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		//
		for(i=0;i<CLWKID_MAX;i++){
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
												wk->wk_obj.res[RESID_OBJ_CHR],
												wk->wk_obj.res[RESID_OBJ_PLT],
												wk->wk_obj.res[RESID_OBJ_CEL],
												&cldata,
												CLSYS_DEFREND_MAIN,
//												CLSYS_DEFREND_SUB,
												wk->heapID );
			// パレット(意図的に変更しない限り、セットしなくてもいいのかもしれない)(要検討)
//			GFL_CLACT_WK_SetPlttOffs( wk->wk_obj.clwk[i], adr->palnum, CLWK_PLTTOFFS_MODE_PLTT_TOP );
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
	//
	// BmpOAM初期化
	wk->BmpOamSys = BmpOam_Init( wk->heapID, p_clunit );
	{
		s32					i;
		BMPOAM_ACT_DATA		actdata;
		
		for(i=0;i<WBT_TRAINER_MAX;i++){
			// 8x2の領域を取っておく
			// (10x2)だと動かないのはなぜ??
			wk->BmpData[i] = GFL_BMP_Create( 8, 2, GFL_BMP_16_COLOR, wk->heapID );
//			wk->BmpData[i] = GFL_BMP_Create( 8, 4, GFL_BMP_16_COLOR, wk->heapID );
			// 初期化
			GFL_STD_MemClear( &actdata, sizeof(BMPOAM_ACT_DATA) );
			actdata.bmp			= wk->BmpData[i];
			actdata.x			= 0;
			actdata.y			= 0;
			actdata.pltt_index	= wk->wk_obj.res[RESID_OBJ_PLT];
			actdata.soft_pri	= 0x38;				// 要修正
			actdata.setSerface	= CLSYS_DRAW_MAIN;
			actdata.draw_type	= CLSYS_DRAW_MAIN;
			actdata.bg_pri		= 1;				// 要修正
			actdata.pal_offset  = 0;
			// 作成
			wk->BmpOamWork[i]	= BmpOam_ActorAdd( wk->BmpOamSys, &actdata );
		}
	}
	//
#if 0	// test
	{
		s32		i;
		// ゲージ表示(6だとハング)
		for(i=0;i<5;i++){
			wbt_set_gageanim( wk, i, 1, 1 );
		}
	}
#endif
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
	
	for(i=0;i<CLWKID_MAX;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk[i] );
	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_OBJ_CEL] );
	//
	// BmpOAM後処理
	for(i=0;i<WBT_TRAINER_MAX;i++){
		BmpOam_ActorDel( wk->BmpOamWork[i] );
		GFL_BMP_Delete( wk->BmpData[i] );
	}
	BmpOam_Exit( wk->BmpOamSys );
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
	wk->MoveWork[wnum].rate		= (ex-sx)/frame;
	wk->MoveWork[wnum].calcnum	= sx;
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
	s16		rate=0;
	
	// 移動中
	if(wk->MoveWork[wnum].flag & FLG_MOVE_INIT){
		if(!(wk->MoveWork[wnum].flag & FLG_MOVE_END)){
			// 移動
			wk->MoveWork[wnum].calcnum += wk->MoveWork[wnum].rate;
			// 終了判定
			flg = 0;
			if(wk->MoveWork[wnum].rate < 0){
				flg = 1;
			}
			if(flg == 0){
				// 加算
				if(wk->MoveWork[wnum].calcnum >= wk->MoveWork[wnum].ex){
					rate = wk->MoveWork[wnum].ex;
					wk->MoveWork[wnum].flag |= FLG_MOVE_END;
				}
			}else{
				// 減算
				if(wk->MoveWork[wnum].calcnum <= wk->MoveWork[wnum].ex){
					rate = wk->MoveWork[wnum].ex;
					wk->MoveWork[wnum].flag |= FLG_MOVE_END;
				}
			}
			rate = wk->MoveWork[wnum].calcnum;
			
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
 *	@brief  パネル移動初期化(要整理)
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
	s32		rate=60;		// 要修正
	
	if(mode == MOVE_PANEL_IN){	// 出る
//		buf[0] = -64;			// 左
//		buf[1] = 0;
		buf[0] = -72;			// 左
		buf[1] = 0;
//		buf[2] = 216;			// 右
//		buf[3] = 216-64;
		buf[2] = 220;			// 右
		buf[3] = 220-72;
	}else{						// 引っ込む
//		buf[0] = 0;				// 左
//		buf[1] = -64;
		buf[0] = 0;				// 左
		buf[1] = -72;
//		buf[2] = 216-64;		// 右
//		buf[3] = 216;
		buf[2] = 220-72;			// 右
		buf[3] = 220;
	}
	// test
	rate = 72;
	
	// 左側
	wbt_init_movework( wk, MWORK_PANEL_L, buf[0], buf[1], rate );
	// 右側
	wbt_init_movework( wk, MWORK_PANEL_R, buf[2], buf[3], rate );
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
	GFL_CLACTPOS	pos[2];
	// 左側
	px = wbt_main_movework( wk, MWORK_PANEL_L );
	for(i=0;i<4;i++){
		// 座標の取得
		GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[CLWKID_PANEL_00A+i], &pos[0], CLSYS_DEFREND_MAIN );
		// 座標のセット(x方向のみ修正)
		pos[0].x = px;
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[CLWKID_PANEL_00A+i], &pos[0], CLSYS_DEFREND_MAIN );
		//
		// BmpOamの移動
		BmpOam_ActorSetPos( wk->BmpOamWork[i], (pos[0].x+36), (pos[0].y+8) );
	}
	//
	// 右側
	px = wbt_main_movework( wk, MWORK_PANEL_R );
	for(i=0;i<4;i++){
		// 座標の取得
		GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[CLWKID_PANEL_04A+i], &pos[1], CLSYS_DEFREND_MAIN );
		// 座標のセット(x方向のみ修正)
		pos[1].x = px;
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[CLWKID_PANEL_04A+i], &pos[1], CLSYS_DEFREND_MAIN );
		//
		// BmpOamの移動
		BmpOam_ActorSetPos( wk->BmpOamWork[4+i], (pos[1].x+4), (pos[1].y+8) );
	}
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
#define BOMB_POS		( 24 )

static void wbt_disp_bomb( WBTLIST_MAIN_WORK* wk, const s32 num )
{
	u16				anum;
	GFL_CLACTPOS	pos;
	
	// 左側
	pos.x = wbt_get_empty( wk, (BOMB_POS+num)*2 );
	pos.y = wbt_get_empty( wk, (BOMB_POS+num)*2+1 );
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
 *	@brief  会話ウインドウ(セット)
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_set_meswin( WBTLIST_MAIN_WORK* wk )
{
	// stream設定=>ここのstrbufは終わるまで削除してはいけない
	wk->PrintStream = PRINTSYS_PrintStream(
				wk->MsgPrintUtil[0].win, 0, 0, wk->MsgBuffer, wk->font,
				MSGSPEED_GetWait(), wk->MsgTcblSys, 4, wk->heapID, 15 );
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
	if( PRINTSYS_PrintStreamGetState( wk->PrintStream ) == PRINTSTREAM_STATE_DONE ){
		if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE || GFL_UI_TP_GetTrg()){
//			PRINTSYS_PrintStreamDelete( wk->PrintStream );
			return( FALSE );
		}
	}
	return( TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  emptyの取得
 *
 *	@param	WBTLIST_MAIN_WORK* wk
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
 *	@brief  アニメの切り替え
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
 *	@brief  アニメの終了待ち
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
	BOOL	bflg=TRUE;
	
	// 終了判定らしい
	if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[clwnum] ) == FALSE ){
		bflg = FALSE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメの切り替え
 *
 *	@param	WBTLIST_MAIN_WORK* wk
 *	@param	const s32		アニメ番号(0-3)
 *	@param	const BOOL		(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wbt_visible_gage( WBTLIST_MAIN_WORK *wk, const s32 wnum, const BOOL bflg )
{
	s32		clwnum = CLWKID_BAR_00+wnum;
	
	// 表示/非表示
	GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[clwnum], bflg );
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
		wk->DispNum = 1;
		if(wk->Mode == MODE_ROUND2){
			wk->DispNum = 4;
		}else if(wk->Mode == MODE_ROUND3){
			wk->DispNum = 2;
		}
		OS_Printf("debug init \n");
	}
	// debug
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		wk->Mode++;
		if(wk->Mode > MODE_ROUND3){
			wk->Mode = MODE_ROUND1;
		}
		wk->Step = STEP_INIT;
		// 表示回数
		wk->DispNum = 1;
		if(wk->Mode == MODE_ROUND2){
			wk->DispNum = 4;
		}else if(wk->Mode == MODE_ROUND3){
			wk->DispNum = 2;
		}
//		OS_Printf("debug init2 \n");
		OS_Printf("---- round : %d ----\n",wk->Mode);
		OS_Printf("mode,step :%d,%d\n",wk->Mode,wk->Step);
	}
#endif
}

