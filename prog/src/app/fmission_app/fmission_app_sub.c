//=============================================================================
/**
 *
 *	@file		fmission_app_sub.c
 *	@brief  フェスミッションアプリ サブ
 *	@author Miyuki Iwasawa	
 *	@data	  2011.04.22	
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "ui/print_tool.h"

#include "app/app_menu_common.h"

#include "fmission_app_local.h"
#include "fmission_app_sub.h"

#include "fmission_app.naix"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
// いろいろ(まとめること)
enum{
	FMA_BGNUM_MAX		= 8,
	FMA_EMPTY_MAXNUM	= 14*2,
	//
	MES_CLEAR_COLOR     = 0,
	//
	//
	// (main)(0-5:bg、6:cmn、8:font)
	FMA_BG_BASE_PAL		= 0,
	FMA_BG_CMN_PAL		= 6,
	//
	FMA_BG_FONT_PAL		= 8,
	//
	// (main)(0-5:bg、8:font)
	FMA_BG_SUB_FONT_PAL	= 8,
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
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}FMA_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//-----------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}FMA_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//-----------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				mode;			// (0:詳細、1:結果)
}FMA_BGSCN_DATA;

//-----------------------------------------
// 文字用の定義(表示領域の設定:CANVAS)
//------------------------------------------
typedef struct{
  u8				frmnumber;			// BG上下(frame定義なので注意)(0xffの場合は終了判定)
  u8				x;					// 座標
  u8				y;
  u8				width;				// サイズ
  u8				height;
  u8				palnum;				// パレット番号
}FMA_CANVAS_DATA;

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
static const FMA_BGPAL_DATA fma_bgpal_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_fmission_app_fmapp_bgu_nclr,	0,	2,},		// 上、BG
	{ PALTYPE_MAIN_BG,	NARC_fmission_app_fmapp_bgd_nclr,	0,	2,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
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
  { GFL_BG_FRAME0_S,	5,	5,	12,	2,	FMA_BG_SUB_FONT_PAL,},		// のこり時間(左)
  { GFL_BG_FRAME0_S,	5,	8,	12,	2,	FMA_BG_SUB_FONT_PAL,},		// 参加人数(左)
  { GFL_BG_FRAME0_S,	3,	11,	20,	2,	FMA_BG_SUB_FONT_PAL,},		// クリア目標(左)
  { GFL_BG_FRAME0_S,	3,	14,	12,	2,	FMA_BG_SUB_FONT_PAL,},		// トータルスコア(左)
  { GFL_BG_FRAME0_S,	3,	17,	12,	2,	FMA_BG_SUB_FONT_PAL,},		// 報酬(左)
  //
  { GFL_BG_FRAME0_S,   23,	 8,	 6,	2,	FMA_BG_SUB_FONT_PAL,},		// 参加人数(右)
  { GFL_BG_FRAME0_S,   23,	14,	 6,	2,	FMA_BG_SUB_FONT_PAL,},		// トータルスコア(右)
  { GFL_BG_FRAME0_S,   13,	17,	16,	2,	FMA_BG_SUB_FONT_PAL,},		// 報酬(右)
  //
  //
  { GFL_BG_FRAME0_M,	3,	1,	8,	2,	FMA_BG_FONT_PAL,},			// マイスコア(左)
  { GFL_BG_FRAME0_M,	3,	4,	7,	2,	FMA_BG_FONT_PAL,},			// 順位(左)
  { GFL_BG_FRAME0_M,	3,	7, 10,	2,	FMA_BG_FONT_PAL,},			// トップスコア(左)
  { GFL_BG_FRAME0_M,   13,  7,	8,	2,	FMA_BG_FONT_PAL,},			// 名前(左)
  { GFL_BG_FRAME0_M,	3, 10,	8,	2,	FMA_BG_FONT_PAL,},			// ハイスコア(左)
  { GFL_BG_FRAME0_M,	3, 13, 28,	8,	FMA_BG_FONT_PAL,},			// 説明文(左)
  //
  { GFL_BG_FRAME0_M,   21,	1,	8,	2,	FMA_BG_FONT_PAL,},			// マイスコア(右)
  { GFL_BG_FRAME0_M,   10,	4, 19,	2,	FMA_BG_FONT_PAL,},			// 順位(右)
  { GFL_BG_FRAME0_M,   21,	7,  8,	2,	FMA_BG_FONT_PAL,},			// トップスコア(右)
  { GFL_BG_FRAME0_M,   21, 10,	8,	2,	FMA_BG_FONT_PAL,},			// ハイスコア(右)
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

// 情報
static const s16 fma_obj_main_itbl[] = {
	FMA_RESID_OBJ_PLT,
	FMA_RESID_OBJ_CHR,
	FMA_RESID_OBJ_CEL,
	//
	FMA_OBJ_MAXNUM,
	FMA_CLWKMODE_NORMAL,
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
	{ 7,	0x40,	1,	1,	0,	1,	5,	0,},		// 人数アイコン
	{ 5,	0x40,	1,	1,	0,	1,	6,	0,},		// 星
	{ 0,	0x40,	1,	1,	0,	0,	8,	0,},		// 時間
	{ 0,	0x40,	1,	1,	0,	0,	8,	0,},		// 時間
	{ 1,	0x40,	1,	1,	0,	0,	8,	0,},		// 時間(コロン)
	{ 0,	0x40,	1,	1,	0,	0,	8,	0,},		// 時間
	{ 0,	0x40,	1,	1,	0,	0,	8,	0,},		// 時間
	{ 0,	0x40,	1,	1,	0,	0,	7,	0,},		// スコア
	{ 0,	0x40,	1,	1,	0,	0,	7,	0,},		// スコア
	{ 0,	0x40,	1,	1,	0,	0,	7,	0,},		// スコア
	{ 0,	0x40,	1,	1,	0,	0,	7,	0,},		// スコア
	{ 8,	0x40,	1,	1,	0,	1,	9,	0,},		// レコード(トータルスコア)
	{ 8,	0x40,	1,	1,	0,	1, 10,	0,},		// レコード(クリアタイム)
	//
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

// OBJのテーブルデータまとめ
static const FMA_OBJ_DATA *fma_obj_tbl[] = {
	fma_obj_info_tbl,			// 詳細
	fma_obj_result_tbl,			// 結果
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
static void fma_init_obj( FMISSION_APP_WORK* wk );
static void fma_exit_obj( FMISSION_APP_WORK* wk );

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
	
	// obj
	fma_exit_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG初期化
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init_bg( FMISSION_APP_WORK* wk, const s32 mode )
{
	s32		i;
	s32		tmp;
	const FMA_BGPAL_DATA	*padr = &fma_bgpal_tbl[0];
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
		GFL_ARCHDL_UTIL_TransVramPalette( handle, padr->arcid, padr->type, padr->offset, tmp, wk->heapID );
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
	//
	// 背景色をセット
//	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0 );
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
//	const FMA_CANVAS_DATA	*adr = &fma_canvas_00_tbl[0];
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
//	else if(buf[4] == T_DOWNLOAD_CLWKMODE_CMN){		// cmn
//		clwk = &wk->wk_obj.cmn_clwk[0];
//	}
	
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
	// CLWK登録(通常)
	fma_init_obj_clwk( wk, CLSYS_DRAW_SUB, adr, &fma_obj_main_itbl[0] );
	//
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
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[FMA_RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[FMA_RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[FMA_RESID_OBJ_CEL] );
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
//		if(wnum >= FMA_CLWKID_MAX){
//			OS_Printf("obj max error:%d\n",wnum);
//			return;
//		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
	}
#if 0
	else if(wmode == T_DOWNLOAD_CLWKMODE_CMN){			// 共通
		if(wnum >= T_DOWNLOAD_CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.cmn_clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.cmn_clwk[wnum], TRUE );
	}
#endif
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
//		if(wnum >= T_DOWNLOAD_CLWKID_MAX){
//			OS_Printf("obj max error:%d\n",wnum);
//			return;
//		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
	}
#if 0
	else if(wmode == T_DOWNLOAD_CLWKMODE_CMN){		// 共通
		if(wnum >= T_DOWNLOAD_CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.cmn_clwk[wnum], bflg );
	}
#endif
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
	
	// 終了判定らしい
	if(wmode == FMA_CLWKMODE_NORMAL){		// 通常
//		if(wnum >= T_DOWNLOAD_CLWKID_MAX){
//			OS_Printf("obj max error:%d\n",wnum);
//			return( FALSE );
//		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
#if 0
	else if(wmode == T_DOWNLOAD_CLWKMODE_CMN){		// 共通
		if(wnum >= T_DOWNLOAD_CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.cmn_clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
#endif
	return( bflg );
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
 */
//-----------------------------------------------------------------------------
void FmaCmn_disp_number( FMISSION_APP_WORK *wk, const s32 wmode, const s32 src_wnum, const s32 num, const s32 snum, const s32 smode )
{
	s32				i;
	s32				buf[4];
	GFL_CLACTPOS	pos;
	s32				srcpos;
	BOOL			bflg;
	s32				tmp=num;		// 値セット
	
	// 初期化
	for(i=0;i<4;i++){
		buf[i] = 0;
	}
	// 先頭の座標の取得
	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[src_wnum], &pos, CLSYS_DEFREND_SUB );
	srcpos = pos.x;
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
	//
	// ここまででbuf[]に逆順で値が入っているはず
	for(i=0;i<4;i++){
		// 値のみ表示
		bflg = TRUE;
		if(smode == 0){
			if((buf[3-i] == 0)&&(i != (4-1))){
				bflg = FALSE;
			}
		}
		// 表示
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[src_wnum+i], bflg );
		// フレームを指定(FX32_SHIFTをするようだ)
		GFL_CLACT_WK_SetAnmFrame( wk->wk_obj.clwk[src_wnum+i], (buf[3-i]<<FX32_SHIFT) );
		// x方向に加算
		pos.x = srcpos + 8*i;
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[src_wnum+i], &pos, CLSYS_DEFREND_SUB );
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
		// フレームを指定(FX32_SHIFTをするようだ)
		GFL_CLACT_WK_SetAnmFrame( wk->wk_obj.clwk[src_wnum+i], (buf[i]<<FX32_SHIFT) );
		// x方向に加算
		pos.x = srcpos + 8*i;
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[src_wnum+i], &pos, CLSYS_DEFREND_SUB );
	}
	// コロン
	pos.x = srcpos + 8*2;
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[src_wnum+2], &pos, CLSYS_DEFREND_SUB );
	//
	// 秒
	for(i=0;i<2;i++){
		// フレームを指定(FX32_SHIFTをするようだ)
		GFL_CLACT_WK_SetAnmFrame( wk->wk_obj.clwk[src_wnum+3+i], (buf[2+i]<<FX32_SHIFT) );
		// x方向に加算
		pos.x = srcpos + 8*(3+i);
		GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[src_wnum+3+i], &pos, CLSYS_DEFREND_SUB );
	}
}


