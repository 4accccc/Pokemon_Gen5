//=============================================================================
/**
 *
 *	@file		pkwd_count.c
 *	@brief  ポケウッドカウント
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

//描画設定
#include "pkwd_count_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

#include "system/wipe.h"

#include "app/pkwd_count.h"

#include "pkwd_announce_gra.naix"

// メッセージ
//#include "message.naix"

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
	//
	STEP_END	= 10000,
};

// パレット
// 下：BG  (0,1:BG、8:ロゴ)
// 下：OBJ (0:mark)
enum{
	// 何本目か(bg)
	PLTID_MAIN_BG_LOGO			= 8,		// ロゴ(1本に収まらないときはまとめてもらうこと)
	//
	// 何番目か
	MES_CLEAR_COLOR				= 0,		// クリアカラー
};

// 最大値
enum{
	PKWD_COUNT_EMPTY_MAXNUM		= 3*2,		// empty
	//
	PKWD_COUNT_OBJ_MAXNUM		= 1,		// obj
	PKWD_COUNT_LOGO_OBJ_MAXNUM	= 2,		// obj
	//
	PKWD_COUNT_BGNUM_MAXNUM		= 8,		// bg
};


// OBJのリソースのインデックス
enum{
	// 通常
	RESID_OBJ_PLT,							// パレット
	RESID_OBJ_CHR,							// キャラ
	RESID_OBJ_CEL,							// セルアニメ
	// ロゴ
	RESID_LOGO_OBJ_PLT,						// パレット
	RESID_LOGO_OBJ_CHR,						// キャラ
	RESID_LOGO_OBJ_CEL,						// セルアニメ
	//
	RESID_OBJ_MAX,
};

//
enum{
	CLWKMODE_NORMAL		= 0,				// 通常
	CLWKMODE_LOGO,							// ロゴ
};

// CLWK
enum{
	CLWK_ROLE			= 0,				// 回転のclwk
	//
	CLWK_LOGO_MOUSE		= 0,				// 口
	CLWK_LOGO_EYE,							// 目
};

// SEQ
enum{
	SEQ_ROLE_ANIM		= 0,				// 回転のseq
	//
	SEQ_LOGO_MOUSE_ANIM	= 0,				// 口
	SEQ_LOGO_EYE_ANIM,						// 目
};

// フェードの種類
enum{
	PKWD_FADE_IN 		= 0,				// フェードイン
	PKWD_FADE_OUT,							// フェードアウト
};

// パレットアニメ関連
enum{
	PALANIM_SIZE		= 12*0x20,			// 12本分セット
	PALANIM_BIT			= 0x0fff,			// 12本分のbitをセット
	PALANIM_FADEIN		= 4,				// fadein時の間隔
	PALANIM_FADEOUT		= -32,				// fadeout時の間隔(即)
	PALANIM_COLOR		= 0,				// fade時のカラー(黒:0)
};

// タイマindex
enum{
	TIMER_WAIT,								// タイマ(暗い画面)
	TIMER_WAIT2,							// タイマ(ロゴ表示)
	
	TIMER_MAX,
};

// いろいろ
enum{
	WAIT_BLACK_MAX		= 60,				// 待ち(暗い画面)
	WAIT_LOGO_MAX		= 60*3,				// 待ち(ロゴ表示)
};

//-------------------
// フラグ
//-------------------
// 移動のフラグ(MoveWork.flag)
enum{
	FLG_MOVE_INIT		= (1<<0),			// 初期化
	FLG_MOVE_END		= (1<<1),			// 終了
};

// OBJのフラグ(PKWD_COUNT_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM		= (1<<0),			// アニメあり
	FLG_OBJ_HFLIP		= (1<<1),			// HFLIP
	FLG_OBJ_VFLIP		= (1<<2),			// VFLIP
};

// PKWD_COUNT_MAIN_WORK.Flag
enum{
	FLG_WIPE_IN_END		= (1<<0),			// wipe(in)終了
	FLG_COUNT_END		= (1<<1),			// カウント終了(1になった)
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
	// 通常
	GFL_CLWK	*clwk[PKWD_COUNT_OBJ_MAXNUM];			// CLWK
	// ロゴ
	GFL_CLWK	*logo_clwk[PKWD_COUNT_LOGO_OBJ_MAXNUM];	// CLWK
	
} PKWD_COUNT_OBJ_WORK;

//--------------------------------------------------------------
///	メインワーク
//--------------------------------------------------------------
typedef struct
{
	HEAPID					heapID;
	// OBJ
	PKWD_COUNT_OBJ_WORK		wk_obj;
	//描画設定
	PKWD_COUNT_GRAPHIC_WORK	*graphic;
	//
	PKWD_COUNT_PARAM 		*Param;						// 参照用として保存
	//
	s32						Step;						// ステップ
	s32						Count;						// カウント
	u32						Flag;						// フラグ
	s32						Timer[TIMER_MAX];			// タイマ
	//
	s16						*EmptyBuffer;				// 座標用のバッファ(必ず破棄すること)
	//
//	PALETTE_FADE_PTR		PalPtr;						// パレットアニメ用(grahpicに移行)
} PKWD_COUNT_MAIN_WORK;

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
}PKWD_COUNT_OBJ_DATA;

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}PKWD_COUNT_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}PKWD_COUNT_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				poff;			// パレットのオフセット
}PKWD_COUNT_BGSCN_DATA;

//=============================================================================
/**
 *							データ定義(要整理)
 */
//=============================================================================
//---------------
// BG関連
//---------------
// BGPAL
static const PKWD_COUNT_BGPAL_DATA pwc_bgpal_tbl[] = {
	{ PALTYPE_MAIN_BG,	NARC_pkwd_announce_gra_pkwd_count_bg_NCLR,	0,	2,},		// 下、BG
	{ PALTYPE_MAIN_BG,	NARC_pkwd_announce_gra_pkwd_logo_NCLR,	PLTID_MAIN_BG_LOGO,	2,},		// 上、ロゴ

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const PKWD_COUNT_BGCHR_DATA pwc_bgchr_tbl[] = {
	{ GFL_BG_FRAME2_M,	NARC_pkwd_announce_gra_pkwd_count_bg2_NCGR,	0,	0,},		// 下、BG2
	{ GFL_BG_FRAME3_M,	NARC_pkwd_announce_gra_pkwd_count_bg1_NCGR,	0,	0,},		// 下、BG3
	{ GFL_BG_FRAME1_M,	NARC_pkwd_announce_gra_pkwd_logo_NCGR,	0,	0,},		// 上、ロゴ

	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const PKWD_COUNT_BGSCN_DATA pwc_bgscn_tbl[] = {
	{ GFL_BG_FRAME2_M,	NARC_pkwd_announce_gra_pkwd_count_bg2_NSCR,	0,	0,},		// 下、BG2(512x512)
	{ GFL_BG_FRAME3_M,	NARC_pkwd_announce_gra_pkwd_count_bg1_NSCR,	0,	0,},		// 下、BG3
	{ GFL_BG_FRAME1_M,	NARC_pkwd_announce_gra_pkwd_logo_NSCR,	0,	PLTID_MAIN_BG_LOGO,},		// 上、ロゴ(注意)

	{ 0xff, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(通常)
//------------------
// narc, resid, paloffset, palnum
static const s16 pwc_obj_main_info_tbl[] = {
	NARC_pkwd_announce_gra_pkwd_count_obj_NCLR, RESID_OBJ_PLT, 0, 0, // NCL
	NARC_pkwd_announce_gra_pkwd_count_obj_NCGR, RESID_OBJ_CHR, 0, 0, // NCG
	NARC_pkwd_announce_gra_pkwd_count_obj_NCER, RESID_OBJ_CEL, 0, 0, // NCE
	NARC_pkwd_announce_gra_pkwd_count_obj_NANR, -1, 0, 0,			 // NAR?
};

// 情報
static const s16 pwc_obj_main_itbl[] = {
	RESID_OBJ_PLT,
	RESID_OBJ_CHR,
	RESID_OBJ_CEL,
	//
	PKWD_COUNT_OBJ_MAXNUM,
	CLWKMODE_NORMAL,
};

// OBJのテーブルデータ(通常)
static const PKWD_COUNT_OBJ_DATA pwc_obj_tbl[] = {
	{ 0,	0x30,	2,	1,	0,	0,	0,	0,},		// パネル0

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};
//------------------
// OBJの情報(ロゴ)
//------------------
// narc, resid, paloffset, palnum
static const s16 pwc_obj_logo_info_tbl[] = {
	NARC_pkwd_announce_gra_pkwd_logo_obj_NCLR, RESID_LOGO_OBJ_PLT, 8, 0, // NCL
	NARC_pkwd_announce_gra_pkwd_logo_obj_NCGR, RESID_LOGO_OBJ_CHR, 0, 0, // NCG
	NARC_pkwd_announce_gra_pkwd_logo_obj_NCER, RESID_LOGO_OBJ_CEL, 0, 0, // NCE
	NARC_pkwd_announce_gra_pkwd_logo_obj_NANR, -1, 0, 0,			 // NAR?
};

// 情報
static const s16 pwc_obj_logo_itbl[] = {
	RESID_LOGO_OBJ_PLT,
	RESID_LOGO_OBJ_CHR,
	RESID_LOGO_OBJ_CEL,
	//
	PKWD_COUNT_LOGO_OBJ_MAXNUM,
	CLWKMODE_LOGO,
};

// OBJのテーブルデータ
static const PKWD_COUNT_OBJ_DATA pwc_obj_logo_tbl[] = {
	{ 0,	0x30,	1,	0,	0,	1,	1,	0,},		// 顔、口
	{ 1,	0x30,	1,	0,	0,	1,	2,	0,},		// 顔、目

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// BGのオフセット
//----------------
static const s16 pwc_bg_pos_tbl[] = {
	  0,  0,	// 3
	256,  0,	// 2
	  0,192,	// 1
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT PkwdCountProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PkwdCountProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PkwdCountProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
///	以下に追加
//=====================================
static void pwc_init( PKWD_COUNT_MAIN_WORK* wk );
static void pwc_main( PKWD_COUNT_MAIN_WORK* wk );
static void pwc_exit( PKWD_COUNT_MAIN_WORK* wk );
static void pwc_down( PKWD_COUNT_MAIN_WORK* wk );
static void pwc_key_all( PKWD_COUNT_MAIN_WORK* wk );
//
// emptyの設定
static s16  pwc_get_empty( PKWD_COUNT_MAIN_WORK *wk, const s32 num );
//
// bgの設定
static void pwc_init_bg( PKWD_COUNT_MAIN_WORK* wk );
//
// objの設定
static void pwc_init_obj_clwk( PKWD_COUNT_MAIN_WORK* wk, const s32 drawtype, const PKWD_COUNT_OBJ_DATA *obj_adr, const s16 *info_adr );
static void pwc_init_obj_res( PKWD_COUNT_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr );
static void pwc_init_obj( PKWD_COUNT_MAIN_WORK* wk );
static void pwc_exit_obj( PKWD_COUNT_MAIN_WORK* wk );
static void pwc_set_obj_visible( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
static void pwc_chg_objseq( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
static BOOL pwc_is_anime_end( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum );
//
// 入力
static BOOL pwc_chk_key_input( PKWD_COUNT_MAIN_WORK* wk );
//
// タイマ表示/切り替え
static BOOL pwc_wait_timer( PKWD_COUNT_MAIN_WORK* wk );
static void pwc_bg_scroll( PKWD_COUNT_MAIN_WORK* wk, const s32 mode );
//
static void pwc_chg_logo( PKWD_COUNT_MAIN_WORK* wk, const s32 mode );
//
// パレットアニメ(フェード)
static void pwc_init_pal_fade( PKWD_COUNT_MAIN_WORK* wk );
static void pwc_exit_pal_fade( PKWD_COUNT_MAIN_WORK* wk );
static void pwc_set_pal_fade( PKWD_COUNT_MAIN_WORK* wk, const s32 mode );
static BOOL pwc_is_pal_fade( PKWD_COUNT_MAIN_WORK* wk );
//
// タイマ
static void pwc_init_timer( PKWD_COUNT_MAIN_WORK* wk, const s32 tmode, const s32 timer );
static s32  pwc_main_timer( PKWD_COUNT_MAIN_WORK* wk, const s32 tmode );


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA PkwdCountProcData = 
{
	PkwdCountProc_Init,
	PkwdCountProc_Main,
	PkwdCountProc_Exit,
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
 *	@param	*pwk					PKWD_COUNT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdCountProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_COUNT_MAIN_WORK 	*wk;
	
	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PKWD_ANNOUNCE, UI_TEMPLATE_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(PKWD_COUNT_MAIN_WORK), HEAPID_PKWD_ANNOUNCE );
	GFL_STD_MemClear( wk, sizeof(PKWD_COUNT_MAIN_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_PKWD_ANNOUNCE;
	
	// 参照用
	wk->Param = pwk;
	
	//描画設定初期化(上:メイン、下:サブ)
	wk->graphic	= PKWD_COUNT_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, wk->heapID );
	//
	//----------------
	// 通常はこの中で初期化
	//----------------
	pwc_init( wk );
	
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
 *	@param	*pwk					PKWD_COUNT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdCountProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_COUNT_MAIN_WORK* wk = mywk;
	
	//----------------
	// 処理を記述
	//----------------
	pwc_exit( wk );
	
	//描画設定破棄
	PKWD_COUNT_GRAPHIC_Exit( wk->graphic );
	
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
 *	@param	*pwk					PKWD_COUNT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdCountProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_COUNT_MAIN_WORK* wk = mywk;
	
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
	pwc_main( wk );
	
	//2D描画
	PKWD_COUNT_GRAPHIC_2D_Draw( wk->graphic );
	
	//3D描画
	PKWD_COUNT_GRAPHIC_3D_StartDraw( wk->graphic );
	
	PKWD_COUNT_GRAPHIC_3D_EndDraw( wk->graphic );
	
	return GFL_PROC_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(init)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_init( PKWD_COUNT_MAIN_WORK* wk )
{
	s32		i;
	
	//----------
	// 初期化
	//----------
	wk->Step 		= STEP_INIT;
	wk->Count		= 0;
	wk->Flag		= 0;
	
	for(i=0;i<TIMER_MAX;i++){
		wk->Timer[i] = 0;
	}
	
	//---------------------
	// empty用バッファ取得
	//(必ず破棄すること)
	//---------------------
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_PKWD_ANNOUNCE, NARC_pkwd_announce_gra_pkwd_count_pos_tblbin, wk->heapID );
	
	//---------------
	// bgの初期化
	//---------------
	pwc_init_bg( wk );
	
	//------------
	// OBJ初期化
	//------------
	pwc_init_obj( wk );
	//
	//------------
	// パレット
	//------------
	pwc_init_pal_fade( wk );
	
	// 半透明設定
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_OBJ, GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3, 10, 6);
	// ロゴは非表示
	GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_OFF );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_main( PKWD_COUNT_MAIN_WORK* wk )
{
	// 下画面
	pwc_down( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(exit)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_exit( PKWD_COUNT_MAIN_WORK* wk )
{
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	
	// OBJ破棄
	pwc_exit_obj( wk );
	// パレット破棄
	pwc_exit_pal_fade( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(下画面)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_down( PKWD_COUNT_MAIN_WORK* wk )
{
	// 入力
	pwc_key_all( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理まとめ
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_key_all( PKWD_COUNT_MAIN_WORK* wk )
{
	BOOL	bflg;
	s32		tmp;
	
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
				pwc_chg_objseq( wk, CLWKMODE_NORMAL, CLWK_ROLE, SEQ_ROLE_ANIM );
			}
			break;
		//
		// カウントダウン処理
		case STEP_LV1:
			// 時間待ち
			bflg = pwc_wait_timer( wk );
			if(bflg == FALSE){
				// obj非表示
				pwc_chg_logo( wk, 0 );
				wk->Step = STEP_LV1+5;
			}
			break;
		//
		// フェードアウト
		case STEP_LV1+5:
			// bg非表示(この段階で全消し)
			pwc_chg_logo( wk, 1 );
			pwc_init_timer( wk, TIMER_WAIT, WAIT_BLACK_MAX );
			wk->Step = STEP_LV1+10;
			break;
		//
		// wait(要調整)
		case STEP_LV1+10:
			// しばらく真っ暗
			tmp = pwc_main_timer( wk, TIMER_WAIT );
			if(tmp == 0){
				// フェードイン
				pwc_set_pal_fade( wk, PKWD_FADE_IN );
				wk->Step = STEP_LV1+20;
			}
			break;
		//
		// ロゴ表示
		case STEP_LV1+20:
			// ロゴ表示
			// (フェードのタイミングと1フレームずらさないとうまくいかないようだ)
			pwc_chg_logo( wk, 2 );
			pwc_init_timer( wk, TIMER_WAIT2, WAIT_LOGO_MAX );
			wk->Step = STEP_LV2;
			break;
		//
		// ロゴ表示
		case STEP_LV2:
			tmp = pwc_main_timer( wk, TIMER_WAIT2 );
			if(tmp == 0){
				// フェードアウト
				pwc_set_pal_fade( wk, PKWD_FADE_OUT );
				wk->Step = STEP_LV3;
			}
#if 0
			// キー入力(test2)
			bflg = pwc_chk_key_input( wk );
			if(bflg == TRUE){
				// フェードアウト
				pwc_set_pal_fade( wk, PKWD_FADE_OUT );
				wk->Step = STEP_LV3;
			}
#endif
			break;
		//
		// 終了処理
		case STEP_LV3:
			// フェードアウト待ち
			bflg = pwc_is_pal_fade( wk );
			if( bflg == FALSE){
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
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
static s16 pwc_get_empty( PKWD_COUNT_MAIN_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= PKWD_COUNT_EMPTY_MAXNUM)){
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
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_init_bg( PKWD_COUNT_MAIN_WORK* wk )
{
	u16		*adr;
	s32		i,j,k;
	s32		tmp;
	u32		off;
	const PKWD_COUNT_BGPAL_DATA	*padr = &pwc_bgpal_tbl[0];
	const PKWD_COUNT_BGCHR_DATA	*cadr = &pwc_bgchr_tbl[0];
	const PKWD_COUNT_BGSCN_DATA	*sadr = &pwc_bgscn_tbl[0];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_PKWD_ANNOUNCE, wk->heapID );
	
	// パレット転送
	for(i=0;i<PKWD_COUNT_BGNUM_MAXNUM;i++){
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
	for(i=0;i<PKWD_COUNT_BGNUM_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, wk->heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<PKWD_COUNT_BGNUM_MAXNUM;i++){
		// 終了条件
		if(sadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramScreen( handle, sadr->arcid, sadr->frmnumber, sadr->offset, 0, 0, wk->heapID );
		//
		//
		// スクリーンのパレット指定を変更
		if(sadr->poff != 0){
			// スクリーンバッファ指定
			adr = GFL_BG_GetScreenBufferAdrs( sadr->frmnumber );
			// 1画面分修正
			for(k=0;k<24;k++){
				for(j=0;j<32;j++){
					// オフセットを加算
					adr[k*32+j] += ((sadr->poff&0x0f)<<12);
				}
			}
//			GFL_BG_ChangeScreenPalette( sadr->frmnumber, 0, 0, 32, 24, sadr->poff );
			GFL_BG_LoadScreenReq( sadr->frmnumber );
		}
		sadr++;
	}
	// 終了
	GFL_ARC_CloseDataHandle( handle );
	//
	// 背景色をセット
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32					drawtype
 *	@param	const PKWD_COUNT_OBJ_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_init_obj_clwk( PKWD_COUNT_MAIN_WORK* wk, const s32 drawtype, const PKWD_COUNT_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16						tmp;
	s16						pos[2];
	s16						buf[6];
	s32 					i;
	BOOL					bflg;
	GFL_CLWK_DATA 			cldata;
	CLWK_FLIP_TYPE			flip;
    GFL_CLUNIT*				p_clunit 	= PKWD_COUNT_GRAPHIC_GetClunit( wk->graphic );
	// アドレスセット
	const PKWD_COUNT_OBJ_DATA *adr 	= obj_adr;
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
	// アドレスセット
	if(buf[4] == CLWKMODE_NORMAL){			// 通常
		clwk = &wk->wk_obj.clwk[0];
	}
	else{
//	else if(buf[4] == CLWKMODE_LOGO){		// logo
		clwk = &wk->wk_obj.logo_clwk[0];
	}
	
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		// 決めうち
		pos[0] = pwc_get_empty(wk, tmp*2);
		pos[1] = pwc_get_empty(wk, tmp*2+1);
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
		// 半透明のテスト
//		GFL_CLACT_WK_SetObjMode( clwk[i], GX_OAM_MODE_XLU );
		
		// 次のデータへ
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(res)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_init_obj_res( PKWD_COUNT_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr )
{
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_PKWD_ANNOUNCE, wk->heapID );
    GFL_CLUNIT*	p_clunit = PKWD_COUNT_GRAPHIC_GetClunit( wk->graphic );
	const s16 *adr		 = info_adr;		// アドレスセット
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ adr[0+1] ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, adr[0], drawtype, adr[2]*0x20, 0, adr[3], wk->heapID );
	wk->wk_obj.res[ adr[4+1] ] = GFL_CLGRP_CGR_Register(      p_handle, adr[4], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ adr[8+1] ] = GFL_CLGRP_CELLANIM_Register( p_handle, adr[8], adr[12], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化まとめ
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_init_obj( PKWD_COUNT_MAIN_WORK* wk )
{
	GFL_CLUNIT*	p_clunit = PKWD_COUNT_GRAPHIC_GetClunit( wk->graphic );
	//
	// リソース読み込み(通常)
	pwc_init_obj_res( wk, CLSYS_DRAW_MAIN, &pwc_obj_main_info_tbl[0] );
	// リソース読み込み(ロゴ)
	pwc_init_obj_res( wk, CLSYS_DRAW_MAIN, &pwc_obj_logo_info_tbl[0] );
	//
	// CLWK登録(通常)
	pwc_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &pwc_obj_tbl[0], &pwc_obj_main_itbl[0] );
	// CLWK登録(ロゴ)
	pwc_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &pwc_obj_logo_tbl[0], &pwc_obj_logo_itbl[0] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_exit_obj( PKWD_COUNT_MAIN_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<PKWD_COUNT_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk[i] );
	}
	// OBJ後処理
	for(i=0;i<PKWD_COUNT_LOGO_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.logo_clwk[i] );
	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_OBJ_CEL] );
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_LOGO_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_LOGO_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_LOGO_OBJ_CEL] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:ロゴ)
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_chg_objseq( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	if(wmode == CLWKMODE_NORMAL){
		if(wnum >= PKWD_COUNT_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
	}
	// ロゴ
	else{
		if(wnum >= PKWD_COUNT_LOGO_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.logo_clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.logo_clwk[wnum], TRUE );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:ロゴ)
 *	@param	const s32			objワーク番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_set_obj_visible( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// 通常
	if(wmode == CLWKMODE_NORMAL){
		// 表示/非表示
		if(wnum >= PKWD_COUNT_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
	}
	// ロゴ
	else{
		// 表示/非表示
		if(wnum >= PKWD_COUNT_LOGO_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.logo_clwk[wnum], bflg );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのアニメの終了待ち
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:ロゴ)
 *	@param	const s32			objワーク番号
 *
 *	@retval	const BOOL			(TRUE:再生中、FALSE:停止、終了)
 */
//-----------------------------------------------------------------------------
static BOOL pwc_is_anime_end( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// 通常
	if(wmode == CLWKMODE_NORMAL){
		if(wnum >= PKWD_COUNT_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		// 判定
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	// ロゴ
	else{
		if(wnum >= PKWD_COUNT_LOGO_OBJ_MAXNUM){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		// 判定
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.logo_clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(上下)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:入力あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL pwc_chk_key_input( PKWD_COUNT_MAIN_WORK* wk )
{
	s32		flg=0;
	BOOL	bflg=FALSE;
	//
	// 戻る
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		flg = 1;
	}
	//
	// 更新
	if(flg == 1){
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  時間待ち(アニメ終了で-1)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	BOOL				(TRUE:再生中、FALSE:終了)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL pwc_wait_timer( PKWD_COUNT_MAIN_WORK* wk )
{
	BOOL	eflg=TRUE;
	BOOL	bflg;
	
	// ループの場合はうまく行かないので
	// 終了を見て再開にした
	bflg = pwc_is_anime_end( wk, CLWKMODE_NORMAL, CLWK_ROLE );
	if(bflg == FALSE){
//		OS_Printf("anim end\n");
		pwc_chg_objseq( wk, CLWKMODE_NORMAL, CLWK_ROLE, SEQ_ROLE_ANIM );
		//
		wk->Count++;
		if(wk->Count > 2){
			wk->Count = 2;
			wk->Flag |= FLG_COUNT_END;
			eflg = FALSE;
		}
		// bg切り替え
		pwc_bg_scroll( wk, wk->Count );
	}
	//
	return( eflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BGのスクロール(問題があればvblanck中の処理に移動)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const 32				(段階:(0-2))
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_bg_scroll( PKWD_COUNT_MAIN_WORK* wk, const s32 mode )
{
	s16		buf[2];
	
	buf[0] = pwc_bg_pos_tbl[2*mode];			// x
	buf[1] = pwc_bg_pos_tbl[2*mode+1];			// y
	// セット
	GFL_BG_SetScroll( GFL_BG_FRAME2_M, GFL_BG_SCROLL_X_SET, buf[0] );
	GFL_BG_SetScroll( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, buf[1] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ロゴに切り替える(タイミング注意)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32			(0:obj非表示、1:BG非表示、2:ロゴを表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_chg_logo( PKWD_COUNT_MAIN_WORK* wk, const s32 mode )
{
	GFL_CLACTPOS	pos;
	
	if(mode == 0){
		// objを非表示
		pwc_set_obj_visible( wk, CLWKMODE_NORMAL, CLWK_ROLE, FALSE );
		// 1フレーム表示されてしまうので画面外に配置する
		GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[CLWK_ROLE], &pos, CLSYS_DEFREND_MAIN );
		pos.y = 256;
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[CLWK_ROLE], &pos, CLSYS_DEFREND_MAIN );
	}
	else if(mode == 1){
		// カウントダウンのBGを非表示
		GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_OFF );
	}
	else{
		// ロゴを表示
		GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_ON );
		// objを表示
		pwc_set_obj_visible( wk, CLWKMODE_LOGO, CLWK_LOGO_MOUSE, TRUE );
		pwc_set_obj_visible( wk, CLWKMODE_LOGO, CLWK_LOGO_EYE,   TRUE );
		pwc_chg_objseq( wk, CLWKMODE_LOGO, CLWK_LOGO_MOUSE, SEQ_LOGO_MOUSE_ANIM );
		pwc_chg_objseq( wk, CLWKMODE_LOGO, CLWK_LOGO_EYE,   SEQ_LOGO_EYE_ANIM );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パレットフェード(初期化)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_init_pal_fade( PKWD_COUNT_MAIN_WORK* wk )
{
	// vblank関数での処理が必要のため「PKWD_COUNT_GRAPHIC_*」内でパレットアニメを定義している
	PALETTE_FADE_PTR ptr = PKWD_COUNT_GRAPHIC_GetPalFadePtr( wk->graphic );
	//
	// メイン側のBGとOBJの取得
	PaletteFadeWorkAllocSet( ptr, FADE_MAIN_BG,  PALANIM_SIZE, wk->heapID );
	PaletteFadeWorkAllocSet( ptr, FADE_MAIN_OBJ, PALANIM_SIZE, wk->heapID );
	// データ転送後に使用すること
	PaletteWorkSet_VramCopy( ptr, FADE_MAIN_BG,  0, PALANIM_SIZE );
	PaletteWorkSet_VramCopy( ptr, FADE_MAIN_OBJ, 0, PALANIM_SIZE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パレットフェード(破棄)
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_exit_pal_fade( PKWD_COUNT_MAIN_WORK* wk )
{
	PALETTE_FADE_PTR	ptr = PKWD_COUNT_GRAPHIC_GetPalFadePtr( wk->graphic );
	//
	// 破棄
	PaletteFadeWorkAllocFree( ptr, FADE_MAIN_BG  );
	PaletteFadeWorkAllocFree( ptr, FADE_MAIN_OBJ );
}

//--------------------------------------------------------------------------------------------
/**
 *  @brief		パレットフェードリクエスト
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32				(0:フェードイン、1:フェードアウト)
 *
 *  @return	none
 */
//--------------------------------------------------------------------------------------------
static void pwc_set_pal_fade( PKWD_COUNT_MAIN_WORK* wk, const s32 mode )
{
	s8		wait;
	u8		ubuf[2];
	PALETTE_FADE_PTR	ptr = PKWD_COUNT_GRAPHIC_GetPalFadePtr( wk->graphic );
	
	// 処理
	if(mode == PKWD_FADE_IN){	// イン
		ubuf[0] = 16;
		ubuf[1] = 0;
		wait    = PALANIM_FADEIN;
	}else{						// アウト
		ubuf[0] = 0;
		ubuf[1] = 16;
//		wait    = PALANIM_FADEOUT;	// 即
		wait	= 4;
	}
	PaletteFadeReq( ptr, PF_BIT_MAIN_BG,  PALANIM_BIT, wait, ubuf[0], ubuf[1], PALANIM_COLOR, GFUser_VIntr_GetTCBSYS() );
	PaletteFadeReq( ptr, PF_BIT_MAIN_OBJ, PALANIM_BIT, wait, ubuf[0], ubuf[1], PALANIM_COLOR, GFUser_VIntr_GetTCBSYS() );
}

//--------------------------------------------------------------------------------------------
/**
 *  @brief		パレットフェードリクエスト
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const BOOL				(TRUE:処理中、FALSE:終了)
 *
 *  @return	none
 */
//--------------------------------------------------------------------------------------------
static BOOL pwc_is_pal_fade( PKWD_COUNT_MAIN_WORK* wk )
{
	BOOL				bflg=TRUE;
	PALETTE_FADE_PTR	ptr = PKWD_COUNT_GRAPHIC_GetPalFadePtr( wk->graphic );
	//
	if( PaletteFadeCheck( ptr ) == 0 ){
		bflg = FALSE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイマ処理初期化
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32 tmode		タイマの種類
 *	@param	const s32 timer		フレーム数
 *
 *	@retval 
 */
//-----------------------------------------------------------------------------
static void pwc_init_timer( PKWD_COUNT_MAIN_WORK* wk, const s32 tmode, const s32 timer )
{
	wk->Timer[tmode] = timer;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイマ処理
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32 mode		タイマの種類
 *
 *	@retval s32					時間
 */
//-----------------------------------------------------------------------------
static s32 pwc_main_timer( PKWD_COUNT_MAIN_WORK* wk, const s32 tmode )
{
	wk->Timer[tmode]--;
	if(wk->Timer[tmode] <= 0){
		wk->Timer[tmode] = 0;
	}
	return( wk->Timer[tmode] );
}


