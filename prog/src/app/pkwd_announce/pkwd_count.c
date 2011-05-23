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
#include "pkwd_count_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"
// print
#include "ui/print_tool.h"
#include "gamesystem/msgspeed.h"

//アプリ共通素材
#include "app/app_menu_common.h"

#include "system/tr2dgra.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

#include "system/wipe.h"

#include "app/app_keycursor.h"
#include "app/app_printsys_common.h"

#include "app/pkwd_count.h"
#include "pkwd_announce_gra.naix"

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
	STEP_INIT	= 0,					// 初期化
	STEP_LV1 	= 100,
	STEP_LV2 	= 200,
	STEP_LV3 	= 300,
	STEP_LV4 	= 400,
	//
	STEP_END	= 10000,
};

// パレット
// 下：BG  (0,1:BG)
// 下：OBJ (0:mark)
enum{
	// 何本目か(bg)
	PLTID_BG_FONT				= 8,		// font
	//
	// 何番目か
	MES_CLEAR_COLOR				= 0,		// クリアカラー
};

// 最大値
enum{
	PKWD_COUNT_EMPTY_MAXNUM		= 1*2,		// empty
	//
	PKWD_COUNT_OBJ_MAXNUM		= 1,		// obj
	//
	PKWD_COUNT_BGNUM_MAXNUM		= 8,		// bg
};


// OBJのリソースのインデックス
enum{
	// 通常
	RESID_OBJ_PLT,							// パレット
	RESID_OBJ_CHR,							// キャラ
	RESID_OBJ_CEL,							// セルアニメ
	//
	RESID_OBJ_MAX,
};

// SEQ
enum{
	SEQ_ROLE_ANIM		= 0,				// 回転のseq
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
	//
	s16						*EmptyBuffer;				// 座標用のバッファ(必ず破棄すること)
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
	u8				pad;
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

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const PKWD_COUNT_BGCHR_DATA pwc_bgchr_tbl[] = {
	{ GFL_BG_FRAME2_M,	NARC_pkwd_announce_gra_pkwd_count_bg2_NCGR,	0,	0,},		// 下、BG2
	{ GFL_BG_FRAME3_M,	NARC_pkwd_announce_gra_pkwd_count_bg1_NCGR,	0,	0,},		// 下、BG3

	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const PKWD_COUNT_BGSCN_DATA pwc_bgscn_tbl[] = {
	{ GFL_BG_FRAME2_M,	NARC_pkwd_announce_gra_pkwd_count_bg2_NSCR,	0,	0,},		// 下、BG2(512x512)
	{ GFL_BG_FRAME3_M,	NARC_pkwd_announce_gra_pkwd_count_bg1_NSCR,	0,	0,},		// 下、BG3

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
	0,
};

// OBJのテーブルデータ(通常)
static const PKWD_COUNT_OBJ_DATA pwc_obj_tbl[] = {
	{ 0,	0x30,	2,	1,	0,	0,	0,	0,},		// パネル0

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

// BGのオフセット
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
static void pwc_chg_objseq( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
static BOOL pwc_is_anime_end( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum );
//
// 入力
static BOOL pwc_chk_key_input( PKWD_COUNT_MAIN_WORK* wk );
//
static void pwc_wait_timer( PKWD_COUNT_MAIN_WORK* wk );
static void pwc_bg_scroll( PKWD_COUNT_MAIN_WORK* wk, const s32 mode );

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
	//----------
	// 初期化
	//----------
	// 上、下画面をまとめて処理
	wk->Step 		= STEP_INIT;
	wk->Count		= 0;
	wk->Flag		= 0;
	
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
	// 半透明設定
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_OBJ, GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3, 10, 6);
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
				pwc_chg_objseq( wk, 0, 0, SEQ_ROLE_ANIM );
			}
			break;
		//
		// LV1
		case STEP_LV1:
			// キー入力(test)
			bflg = pwc_chk_key_input( wk );
			if(bflg == TRUE){
				wk->Step = STEP_END;
			}
			// 時間待ち
			pwc_wait_timer( wk );
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
	s32		i;
	s32		tmp;
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
		GFL_ARCHDL_UTIL_TransVramPalette( handle, padr->arcid, padr->type, padr->offset, tmp, wk->heapID );
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
		sadr++;
	}
	
	// 終了
	GFL_ARC_CloseDataHandle( handle );
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
	clwk = &wk->wk_obj.clwk[0];
	
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
	wk->wk_obj.res[ adr[0+1] ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, adr[0], drawtype, adr[2], 0, adr[3], wk->heapID );
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
	// CLWK登録(通常)
	pwc_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &pwc_obj_tbl[0], &pwc_obj_main_itbl[0] );
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
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_OBJ_CEL] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通、2:上画面)(未使用)
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_chg_objseq( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	if(wnum >= PKWD_COUNT_OBJ_MAXNUM){
		OS_Printf("obj max error:%d\n",wnum);
		return;
	}
	GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通、2:上画面)(未使用)
 *	@param	const s32			objワーク番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
#if 0
static void pwc_set_obj_visible( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// 表示/非表示
	if(wnum >= PKWD_COUNT_OBJ_MAXNUM){
		OS_Printf("obj max error:%d\n",wnum);
		return;
	}
	GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
}
#endif

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのアニメの終了待ち
 *
 *	@param	PKWD_COUNT_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通、2:上画面)(未使用)
 *	@param	const s32			objワーク番号
 *
 *	@retval	const BOOL			(TRUE:再生中、FALSE:停止、終了)
 */
//-----------------------------------------------------------------------------
static BOOL pwc_is_anime_end( PKWD_COUNT_MAIN_WORK *wk, const s32 wmode, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// 終了判定らしい
	if(wnum >= PKWD_COUNT_OBJ_MAXNUM){
		OS_Printf("obj max error:%d\n",wnum);
		return( FALSE );
	}
	if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[wnum] ) == FALSE ){
		bflg = FALSE;
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
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void pwc_wait_timer( PKWD_COUNT_MAIN_WORK* wk )
{
	BOOL	bflg;
//	fx32	f32;
	
	// フレームを取得することができない???
//	f32 = GFL_CLACT_WK_GetAnmFrame( wk->wk_obj.clwk[0] );
//	// 現在のフレームが一つ前よりも後
//	// => ループで1周した
//	// => うまく取得できないようなので保留
//	if(f32 < wk->AnimFrame){
//		OS_Printf("anim end :%d,%d\n",f32,wk->AnimFrame);
//	}
//	// 更新
//	wk->AnimFrame = f32;
	
	// ループの場合はうまく行かないので
	// 終了を見て再開
	bflg = pwc_is_anime_end( wk, 0, 0 );
	if(bflg == FALSE){
//		OS_Printf("anim end\n");
		pwc_chg_objseq( wk, 0, 0, SEQ_ROLE_ANIM );
		//
		wk->Count++;
		if(wk->Count > 2){
			wk->Count = 0;
		}
		// bg切り替え
		pwc_bg_scroll( wk, wk->Count );
	}
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


