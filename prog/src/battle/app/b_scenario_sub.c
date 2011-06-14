//============================================================================================
/**
 * @file		b_scenario_sub.c
 * @brief		ポケウッド台本画面(sub)
 * @author
 * @date		05.02.10
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "system/wipe.h"
#include "sound/pm_sndsys.h"
#include "font/font.naix"
#include "item/itemuse_def.h"

#include "../btlv/btlv_effect.h"

//#include "ui/print_tool.h"		// overlayの関係上使えない
#include "pokeicon/pokeicon.h"

#include "b_app_tool.h"

#include "b_scenario.h"
#include "b_scenario_main.h"
#include "app/app_menu_common.h"

#include "b_scenario_sub.h"

#include "b_scenario_gra.naix"
#include "b_plist_gra.naix"

#include "arc/app_menu_common.naix"
#include "battle/battgra_wb.naix"

//============================================================================================
//  定数定義
//============================================================================================
enum{
	MES_CLEAR_COLOR = 0,			// クリアカラー
};

// パレット(obj)
enum{
	BS_OBJ_PAL_POKE		= 0,		// ポケモン(0-2)
	BS_OBJ_PAL_TYPE		= 4,		// タイプ(4-6)
	BS_OBJ_PAL_CURSOR	= 7,		// カーソル(7)
	BS_OBJ_PAL_ARROW	= 8,		// 矢印(8-11)
	//
	BS_CURSOR_OFFSET	= 4,		// カーソルデータ(注意：palの中の4番目を使用)
	//
	BS_PAL_ARROW_NUM	= 4,		// 矢印の本数
};

// OBJのフラグ(WBTLIST_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM	= (1<<0),		// アニメあり
	FLG_OBJ_HFLIP	= (1<<1),		// HFLIP
	FLG_OBJ_VFLIP	= (1<<2),		// VFLIP
};

// フラグ(AnimFlag)
enum{
	FLG_SCREEN_ANIM	= (1<<0),		// 再生中
};

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}BS_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				comp;
}BS_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				comp;
}BS_BGSCN_DATA;

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
}BS_CANVAS_DATA;

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
	u8				res_cnum;		// ResChrの番号
}BS_OBJ_DATA;

//============================================================================================
//  データ
//============================================================================================
//-------------------------------------
// BG関連(1枚BGでオフセットを切り替える)
//-------------------------------------
// BGPAL
static const BS_BGPAL_DATA bs_bgpal_00_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_b_scenario_gra_pkwd_scenario_NCLR,	0,	8,},	// 下、BG
	{ 0xff, 0, 0, 0,}, // end code
};
// BGCHR
static const BS_BGCHR_DATA bs_bgchr_00_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_b_scenario_gra_pkwd_scenario_NCGR,	0,	0,},	// 下、BG
	{ 0xff, 0, 0, 0,}, // end code
};
// BGSCN
static const BS_BGSCN_DATA bs_bgscn_00_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_b_scenario_gra_pkwd_scenario_NSCR,	0,	0,},	// 下、BG
	{ 0xff, 0, 0, 0,}, // end code
};

//-----------------
// arrow(obj)情報
//-----------------
static const s16 bs_arrow_tbl[] = {
	-1, -1, 0, -1,		// top
	 0,  0, 0, -1,		// scenario
	 0,  0, 0, -1,		// info
};

//----------------------------
// エリア(スクリーン取得位置)
//----------------------------
static const s16 bs_area_tbl[] = {
	 0, 20, 12, 4,	// 脚本(TOP)
	13, 20, 12, 4,	// 共演者(TOP)
	32, 20, 12, 4,	// 脚本(SCENARIO)
	0,  44, 12, 4,	// 共演者(INFO)
};

//-----------
// パレット
//-----------
static const s16 bs_apal_tbl[] = {
	1, 7,			// 脚本(TOP)
	2, 7,			// 共演者(TOP)
	1, 7,			// 脚本(SCENARIO)
	1, 7,			// 共演者(INFO)
};

//----------------
// 切り替えタイマ
//----------------
static const s16 bs_timer_tbl[] = {
	 1, 1,			// 1フレーム目で1番に変化
	 5, 0,			// 5フレーム目で0番に戻る
	 9, -1,			// 待ちの時間
	//
	-1, -1,			// 終了判定
};


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//---------------
// CANVASの設定
//---------------
// TOP
static const BS_CANVAS_DATA bs_canvas_00_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	1,	28,	2,	8,},		// TITLE
	{ GFL_BG_FRAME1_S,	2,	5,	28,	2,	8,},		// PLOT
	{ GFL_BG_FRAME1_S,	2,	12,	28,	2,	8,},		// GOAL
	{ GFL_BG_FRAME1_S,	3,	3,	28,	2,	8,},		// TITLE_MES
	{ GFL_BG_FRAME1_S,	3,	7,	28,	4,	8,},		// PLOT_MES
	{ GFL_BG_FRAME1_S,	3,	14,	28,	4,	8,},		// GOAL_MES
	{ GFL_BG_FRAME1_S,	1,	21,	10,	2,	8,},		// 脚本
	{ GFL_BG_FRAME1_S,	14,	21,	10,	2,	8,},		// 共演者

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};
// SCENARIO
static const BS_CANVAS_DATA bs_canvas_01_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	1,	28,	2,	8,},		// SCENARIO
	{ GFL_BG_FRAME1_S,	2,	3,	28,	2,	8,},		// SCENE00
	{ GFL_BG_FRAME1_S,	2,	10,	28,	2,	8,},		// SCENE01
	{ GFL_BG_FRAME1_S,	3,	5,	28,	2,	8,},		// SCENE00_MES
	{ GFL_BG_FRAME1_S,	3,	12,	28,	2,	8,},		// SCENE01_MES
	{ GFL_BG_FRAME1_S,	1,	21,	10,	2,	8,},		// 脚本
	{ GFL_BG_FRAME1_S,	12,	17,	 2,	2,	8,},		// 現在(test)
	{ GFL_BG_FRAME1_S,	15,	17,	 2,	2,	8,},		// 最大(test)
	{ GFL_BG_FRAME1_S,	14,	17,	 1,	2,	8,},		// 「/」(test)

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};
// INFO
static const BS_CANVAS_DATA bs_canvas_02_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	1,	28,	2,	8,},		// CAST
	{ GFL_BG_FRAME1_S,	1,	6,	8,	2,	8,},		// 名前
	{ GFL_BG_FRAME1_S,	9,	6,	1,	2,	8,},		// 性別
	{ GFL_BG_FRAME1_S,	2,	11,	10,	2,	8,},		// レベル
	{ GFL_BG_FRAME1_S,	1,	15,	12,	2,	8,},		// 持ち物(fix)
	{ GFL_BG_FRAME1_S,	1,	17,	12,	2,	8,},		// 持ち物2
	{ GFL_BG_FRAME1_S,	16,	4,	12,	2,	8,},		// 技00
	{ GFL_BG_FRAME1_S,	16,	7,	12,	2,	8,},		// 技01
	{ GFL_BG_FRAME1_S,	16,	10,	12,	2,	8,},		// 技02
	{ GFL_BG_FRAME1_S,	16,	13,	12,	2,	8,},		// 技03
	{ GFL_BG_FRAME1_S,	13,	15,	12,	2,	8,},		// 特性(fix)
	{ GFL_BG_FRAME1_S,	13,	17,	12,	2,	8,},		// 特性2
	{ GFL_BG_FRAME1_S,	1,	21,	10,	2,	8,},		// 脚本

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

// (canvas)まとめ
static const BS_CANVAS_DATA *bscmn_canvas_tbl[] = {
	bs_canvas_00_tbl,
	bs_canvas_01_tbl,
	bs_canvas_02_tbl,
};

//------------------
// タッチペン領域
//------------------
// TOP
static const GFL_UI_TP_HITTBL bscmn_rect_00_tbl[] = {
  { 166, 182,   8,  88 }, 			// 脚本
  { 166, 182, 112, 192 }, 			// 共演者
  { 160, 184, 216, 248 }, 			// 戻る
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// SCENARIO
static const GFL_UI_TP_HITTBL bscmn_rect_01_tbl[] = {
  { 166, 182,   8,  88 }, 			// 共演者
  { 160, 184, 120, 152 }, 			// 左
  { 160, 184, 160, 192 }, 			// 右
  { 160, 184, 216, 248 }, 			// 戻る
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// INFO
static const GFL_UI_TP_HITTBL bscmn_rect_02_tbl[] = {
  { 166, 182,   8,  88 }, 			// 脚本
  { 160, 184, 120, 152 }, 			// 左
  { 160, 184, 160, 192 }, 			// 右
  { 160, 184, 216, 248 }, 			// 戻る
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// (タッチペン)まとめ
static const GFL_UI_TP_HITTBL *bscmn_rect_tbl[] = {
	bscmn_rect_00_tbl,
	bscmn_rect_01_tbl,
	bscmn_rect_02_tbl,
};

//----------------
// BGのオフセット
//----------------
static const s16 bs_bg_pos_tbl[] = {
	  0,  0,	// 3
	256,  0,	// 2
	  0,192,	// 1
};

//----------------
// OBJ(type)
//----------------
// 情報
static const s16 bs_obj_00_itbl[] = {
	0,		// plt res
	0,		// chr res(未使用)
	0,		// cel res
	//
	BS_CLWK_MAX,		// max
	0,		// mode(0:通常)
};

// OBJのテーブルデータ
const BS_OBJ_DATA bs_obj_00_tbl[] = {
	{ 0,	0x30,	1,	0,	0,	1,	0,	BS_RES_TYPE_CHR_00,},		// タイプ00
	{ 0,	0x30,	1,	0,	0,	1,	1,	BS_RES_TYPE_CHR_01,},		// タイプ01
	{ 0,	0x30,	1,	0,	0,	1,	2,	BS_RES_TYPE_CHR_10,},		// 技タイプ00
	{ 0,	0x30,	1,	0,	0,	1,	3,	BS_RES_TYPE_CHR_11,},		// 技タイプ01
	{ 0,	0x30,	1,	0,	0,	1,	4,	BS_RES_TYPE_CHR_12,},		// 技タイプ02
	{ 0,	0x30,	1,	0,	0,	1,	5,	BS_RES_TYPE_CHR_13,},		// 技タイプ03
	
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// OBJ(poke)
//----------------
// 情報
static const s16 bs_obj_01_itbl[] = {
	1,		// plt res
	1,		// chr res(未使用)
	1,		// cel res
	//
	BS_CLWK_POKE_MAX,		// max
	1,		// mode(1:poke)
};

// OBJのテーブルデータ
const BS_OBJ_DATA bs_obj_01_tbl[] = {
	{ 1,	0x28,	1,	0,	0,	1,	6, BS_RES_POKE_CHR_00,},		// ポケモン00
	{ 1,	0x28,	1,	0,	0,	1,	6, BS_RES_POKE_CHR_01,},		// ポケモン01
	{ 1,	0x28,	1,	0,	0,	1,	6, BS_RES_POKE_CHR_02,},		// ポケモン02
	{ 1,	0x28,	1,	0,	0,	1,	6, BS_RES_POKE_CHR_03,},		// ポケモン03
	{ 1,	0x28,	1,	0,	0,	1,	6, BS_RES_POKE_CHR_04,},		// ポケモン04
	{ 1,	0x28,	1,	0,	0,	1,	6, BS_RES_POKE_CHR_05,},		// ポケモン05
	
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// OBJ(cursor)
//----------------
// 情報
static const s16 bs_obj_02_itbl[] = {
	2,		// plt res
	2,		// chr res(未使用)
	2,		// cel res
	//
	BS_CLWK_CURSOR_MAX,		// max
	2,		// mode(2:cursor)
};

// OBJのテーブルデータ
const BS_OBJ_DATA bs_obj_02_tbl[] = {
	{ 0,	0x28,	1,	0,	0,	1,	0,	BS_RES_CURSOR_CHR,},	// カーソル00
	{ 1,	0x28,	1,	0,	0,	1,	0,	BS_RES_CURSOR_CHR,},	// カーソル01
	{ 2,	0x28,	1,	0,	0,	1,	0,	BS_RES_CURSOR_CHR,},	// カーソル02
	{ 3,	0x28,	1,	0,	0,	1,	0,	BS_RES_CURSOR_CHR,},	// カーソル03
	
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// OBJ(arrow)
//----------------
// 情報
static const s16 bs_obj_03_itbl[] = {
	3,		// plt res
	3,		// chr res(未使用)
	3,		// cel res
	//
	BS_CLWK_ARROW_MAX,		// max
	3,		// mode(3:arrow)
};

// OBJのテーブルデータ
const BS_OBJ_DATA bs_obj_03_tbl[] = {
	{ 3,	0x28,	1,	0,	0,	1,	34,	BS_RES_ARROW_CHR,},	// 左
	{ 6,	0x28,	1,	0,	0,	1,	35,	BS_RES_ARROW_CHR,},	// 右
	{ 0,	0x28,	1,	0,	0,	1,	36,	BS_RES_ARROW_CHR,},	// 戻る
	{ 9,	0x28,	3,	0,	0,	1,	37,	BS_RES_ARROW_CHR,},	// 上マーク
	
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// OBJ(cursor)座標
//----------------
// (左上、左下、右上、右下)の順
static const s16 bs_cursor_emp_tbl[] = {
	 7, 8, 9,10,		// 脚本
	11,12,13,14,		// 共演者
	15,16,17,18,		// 戻る
	19,20,21,22,		// 左矢印
	23,24,25,26,		// 右矢印
};

static const s16 bs_cursor_all_tbl[] = {
	0, 1, 2,-1,			// TOP
	0, 3, 4, 2,			// 脚本
	0, 3, 4, 2,			// 共演者
};

//============================================================================================
//  プロトタイプ宣言
//============================================================================================
void BsCmn_InitBg( BSCENARIO_WORK* wk );
void BsCmn_BgScroll(BSCENARIO_WORK* wk, const s32 mode );
//
void BsCmn_InitString( BSCENARIO_WORK* wk, const s32 mode );
void BsCmn_MainPrint(  BSCENARIO_WORK *wk );
void BsCmn_DispString( BSCENARIO_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode, 
							const PRINTSYS_LSB col, const s32 fmode );
void BsCmn_DispStringNumber( BSCENARIO_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 num, const s32 lnum,
							 const u32 pmode, const s32 fmode );
void BsCmn_ExitString( BSCENARIO_WORK* wk );
void BsCmn_ClearCanvas( BSCENARIO_WORK* wk, const s32 mode );
//
s32  BsCmn_ChkTp( BSCENARIO_WORK *wk, const s32 mode );
//
void BsCmn_InitObj( BSCENARIO_WORK* wk );
void BsCmn_ExitObj( BSCENARIO_WORK* wk );
void BsCmn_ObjVisible( BSCENARIO_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
void BsCmn_ChgTypeObj( BSCENARIO_WORK * wk, const s32 wnum, const s32 mode, const u32 type );
void BsCmn_ChgTypePoke( BSCENARIO_WORK * wk, const s32 wnum, const u32 id, const s32 sex );
void BsCmn_DispCursor( BSCENARIO_WORK *wk, const BOOL bflg, const s32 mode, const s32 num );
//
void BsCmn_DispArrow( BSCENARIO_WORK *wk, const s32 mode, const BOOL bflg );
void BsCmn_ChgSeq( BSCENARIO_WORK *wk, const s32 wnum, const s32 seqnum );
BOOL BsCmn_IsAnimEnd( BSCENARIO_WORK *wk, const s32 wnum );
void BsCmn_ChgScreen( BSCENARIO_WORK *wk, const s32 num, const s32 mode );


//------------
// static
//------------
static void bscmn_init_bg(  BSCENARIO_WORK* wk );
static void bscmn_clear_allcanvas( BSCENARIO_WORK *wk );
//
static u16  bscmn_putsizeget( const STRBUF * buf, GFL_FONT * font, u32 mode );
//
static void bscmn_init_obj_clwk( BSCENARIO_WORK* wk, const s32 drawtype, const BS_OBJ_DATA *obj_adr, const s16 *info_adr );
static void bscmn_init_type_obj( BSCENARIO_WORK * wk );
static void bscmn_init_poke_obj( BSCENARIO_WORK * wk );
static void bscmn_init_cursor_obj( BSCENARIO_WORK * wk );
static void bscmn_init_arrow_obj( BSCENARIO_WORK * wk );
//
static s16  bscmn_get_empty( BSCENARIO_WORK *wk, const s32 num );
//
static void bscmn_disp_cursor( BSCENARIO_WORK *wk, const BOOL bflg, const s32 num );
//
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化まとめ
 *
 *	@param	BSCENARIO_WORKK*		wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_InitBg( BSCENARIO_WORK* wk )
{
	// 通常
	bscmn_init_bg( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化
 *
 *	@param	BSCENARIO_WORKK* 		wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bscmn_init_bg( BSCENARIO_WORK* wk )
{
	s32				i;
	s32				tmp;
	s32				heapID;
	ARCHANDLE		*handle;
	const BS_BGPAL_DATA	*padr = &bs_bgpal_00_tbl[0];
	const BS_BGCHR_DATA	*cadr = &bs_bgchr_00_tbl[0];
	const BS_BGSCN_DATA	*sadr = &bs_bgscn_00_tbl[0];
	
	heapID = wk->dat->heap;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_B_SCENARIO_GRA, GFL_HEAP_LOWID(heapID) );
	//
	// パレット転送
	for(i=0;i<BS_BG_MAXNUM;i++){
		// 終了条件
		if(padr->type == 0xff){
			break;
		}
		tmp = padr->size;
		tmp *= 0x20;		// 転送サイズ
		GFL_ARCHDL_UTIL_TransVramPalette( handle, padr->arcid, padr->type, padr->offset, tmp, heapID );
		padr++;
	}
	//
	// キャラ転送
	for(i=0;i<BS_BG_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<BS_BG_MAXNUM;i++){
		// 終了条件
		if(sadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramScreen( handle, sadr->arcid, sadr->frmnumber, sadr->offset, 0, 0, heapID );
		sadr++;
	}
	
	// 終了
	GFL_ARC_CloseDataHandle( handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BGのスクロール(問題があればvblanck中の処理に移動)
 *
 *	@param	BSCENARIO_WORK*		wk
 *	@param	const 32			(段階:(0-2))
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_BgScroll(BSCENARIO_WORK* wk, const s32 mode )
{
	s16		buf[2];
	
	buf[0] = bs_bg_pos_tbl[2*mode];				// x
	buf[1] = bs_bg_pos_tbl[2*mode+1];			// y
	// セット
	GFL_BG_SetScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_SET, buf[0] );
	GFL_BG_SetScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, buf[1] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(Canvasの作成)
 *
 *	@param	BSCENARIO_WORK*		wk
 *	@param	const s32			段階(0-2)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_InitString( BSCENARIO_WORK* wk, const s32 mode )
{
	s32		i;
	s32		heapID = wk->dat->heap;
	const BS_CANVAS_DATA *adr = bscmn_canvas_tbl[mode];
	
	// 初期化
	wk->PFlag = 0;
	
	// (いままでのを)クリア
	bscmn_clear_allcanvas( wk );
	
	// 破棄(念のため)
	BsCmn_ExitString( wk );
	//
	for(i=0;i<BS_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット
		wk->PrintUtil[i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		// スクリーンは一度でよいはず
		// このタイミングでは、画面にゴミが入ってしまう
		// => スクリーンを作成するのは最後
//		GFL_BMPWIN_MakeScreen( wk->PrintUtil[i].win );
//		GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[i].win) );
//		GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(wk->PrintUtil[i].win) );
		//
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	BSCENARIO_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x)
 *	@param	const s32			表示座標(y)
 *	@param	const STRBUF* 		表示文字列
 *	@param	const u32			表示位置(左(0)、右(1)、中央(2)など)
 *	@param	const PRINTSYS_LSB	カラー
 *	@param	const s32			(0:通常、1:小)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_DispString( BSCENARIO_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode, 
								const PRINTSYS_LSB col, const s32 fmode )
{
	u16		siz;
	
	// 一度クリアしてみる
	BsCmn_ClearCanvas( wk, mode );
	// 表示位置の補正
	if(fmode == 0){		// 通常フォント
		siz = bscmn_putsizeget( strbuf, wk->dat->font, pmode );
		PRINT_UTIL_PrintColor( &wk->PrintUtil[mode], wk->print_que, x-siz, y, strbuf, wk->dat->font, col );
	}else{				// 小フォント
		siz = bscmn_putsizeget( strbuf, wk->sfont, pmode );
		PRINT_UTIL_PrintColor( &wk->PrintUtil[mode], wk->print_que, x-siz, y, strbuf, wk->sfont, col );
	}
#if 0	// バトルでは、overlayの関係上uiは使えないようだ
	// 表示
	PRINTTOOL_PrintColor(
				&wk->PrintUtil[mode],			// プリントユーティル ( BMPWIN )
				wk->print_que,					// プリントキュー
				x, y,							// 表示基準座標
				strbuf,							// 文字列
				wk->dat->font,					// フォント
				col,							// カラー
				pmode );						// 位置
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	BSCENARIO_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x)
 *	@param	const s32			表示座標(y)
 *	@param	const s32			値
 *	@param	const s32			桁数
 *	@param	const u32			表示位置(左、右、中央など)
 *	@param	const u32			(0:通常、1:小)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_DispStringNumber( BSCENARIO_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 num, const s32 lnum, const u32 pmode, const s32 fmode )
{
	STRBUF	*strbuf;
	
	// 数字を桁分0うめ表示
	strbuf = GFL_STR_CreateBuffer( 128, wk->dat->heap );
	STRTOOL_SetNumber( strbuf, num, lnum, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	// 表示
	BsCmn_DispString( wk, mode, x, y, strbuf, pmode, PRINTSYS_LSB_Make(1,2,0), fmode );
	//
	GFL_STR_DeleteBuffer( strbuf );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示終了
 *
 *	@param	BSCENARIO_WORK*		wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_ExitString( BSCENARIO_WORK* wk )
{
	s32		i;
	
	for(i=0;i<BS_CANVAS_MAXNUM;i++){
		if(wk->PrintUtil[i].win != 0){
			// chr側
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[i].win ), MES_CLEAR_COLOR );
			// scn側
			GFL_BMPWIN_ClearScreen( wk->PrintUtil[i].win );
			// 破棄
			GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
			wk->PrintUtil[i].win = 0;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア
 *
 *	@param	BSCENARIO_WORK*		wk
 *	@param	const s32			キャンバスの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_ClearCanvas( BSCENARIO_WORK* wk, const s32 mode )
{
	if(wk->PrintUtil[mode].win){
		// clear
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_CLEAR_COLOR );
		// 要確認
		wk->PrintUtil[mode].transReq = TRUE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア(static)
 *
 *	@param	BSCENARIO_WORK* 	wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bscmn_clear_allcanvas( BSCENARIO_WORK *wk )
{
#if 0	// ちらついてしまうので保留
	s32		i;
	
	for(i=0;i<BS_CANVAS_MAXNUM;i++){
		// win
		if(wk->PrintUtil[i].win){
			// chr側
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[i].win ), MES_CLEAR_COLOR );
			GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[i].win );
			// scn側
			GFL_BMPWIN_ClearTransWindow( wk->PrintUtil[i].win );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  Queの処理
 *
 *	@param	BSCENARIO_WORK*		wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_MainPrint( BSCENARIO_WORK *wk )
{
	s32		i;
	BOOL	bflg;
	BOOL	pflg;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<BS_CANVAS_MAXNUM;i++){
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
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示サイズから各モードの補正値を取得(static)
 *				(ui/printtoolから持ってきた)
 *
 * @param		buf			文字列
 * @param		font		フォント
 * @param		mode		表示モード(0-2)
 *
 * @return	補正値
 */
//--------------------------------------------------------------------------------------------
static u16 bscmn_putsizeget( const STRBUF * buf, GFL_FONT * font, u32 mode )
{
	// 右詰
	if( mode == 1 ){
		return PRINTSYS_GetStrWidth( buf, font, 0 );
	// 中央
	}else if( mode == 2 ){
		return ( PRINTSYS_GetStrWidth( buf, font, 0 ) / 2 );
	}
	return 0;
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
s32 BsCmn_ChkTp( BSCENARIO_WORK *wk, const s32 mode )
{
	s32	id;
	//
	// タッチペン判定
	id = GFL_UI_TP_HitTrg( bscmn_rect_tbl[mode] );
	return( id );
}

//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	タイプ/分類アイコンobjの読み込み
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bscmn_init_type_obj( BSCENARIO_WORK * wk )
{
	ARCHANDLE	*ah;
	u32			i;
	
	ah = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, GFL_HEAP_LOWID(wk->dat->heap) );
	// パレット(4-6)
	wk->ResPal[BS_RES_TYPE_PAL] = GFL_CLGRP_PLTT_RegisterEx( ah,
												NARC_app_menu_common_p_st_type_NCLR,
												CLSYS_DRAW_SUB, BS_OBJ_PAL_TYPE*0x20, 0, 3, wk->dat->heap );
	// セル・アニメ
	wk->ResCell[BS_RES_TYPE_CELL] = GFL_CLGRP_CELLANIM_Register( ah,
												NARC_app_menu_common_p_st_type_32k_NCER,
												NARC_app_menu_common_p_st_type_32k_NANR,
												wk->dat->heap );
	// タイプアイコンキャラ
	// (技タイプもこれでよいらしい)
	for(i=0;i<BS_RES_TYPE_CHR_MAX;i++){
		wk->ResChr[BS_RES_TYPE_CHR_00+i] = GFL_CLGRP_CGR_Register( ah, APP_COMMON_GetPokeTypeCharArcIdx(0), FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
	}
	//
	GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ポケモンアイコンのグラフィックロード
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bscmn_init_poke_obj( BSCENARIO_WORK * wk )
{
	ARCHANDLE	*ah;
	u32			i;
	u32			buf[2];
	POKEMON_PARAM	*pp;
	
	ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, GFL_HEAP_LOWID(wk->dat->heap) );
	// パレット(0-3)
	wk->ResPal[BS_RES_POKE_PAL] = GFL_CLGRP_PLTT_RegisterComp( ah, 
												POKEICON_GetPalArcIndex(),
												CLSYS_DRAW_SUB, 0, wk->dat->heap );
	// セル・アニメ
	wk->ResCell[BS_RES_POKE_CELL] = GFL_CLGRP_CELLANIM_Register( ah,
												POKEICON_GetCellSubArcIndex(),
												POKEICON_GetAnmSubArcIndex(),
												wk->dat->heap );
	// キャラ
	for(i=0;i<6;i++){
		if(i < wk->PokeMax){
			// ポケモン情報
			pp = PokeParty_GetMemberPointer( wk->dat->party, i );
			// id
			buf[0] = wk->PokeWork[i].id;
			// sex
			buf[1] = wk->PokeWork[i].sex;
			//
			wk->ResChr[BS_RES_POKE_CHR_00+i] = GFL_CLGRP_CGR_Register( ah,
												POKEICON_GetCgxArcIndexByMonsNumber( buf[0], 1, buf[1], FALSE ),
												FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
		}else{
			wk->ResChr[BS_RES_POKE_CHR_00+i] = GFL_CLGRP_CGR_Register( ah,
												POKEICON_GetCgxArcIndexByMonsNumber( 0, 0, 0, 0 ),
												FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
		}
	}
	//
	GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	カーソルのグラフィックロード
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bscmn_init_cursor_obj( BSCENARIO_WORK * wk )
{
	ARCHANDLE	*ah;
	u32			i;
	
	// カーソルデータはパレットの4番目のデータを使用
	// 転送後にclwkでそのパレットを適用させる必要あり
	//
	ah = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, GFL_HEAP_LOWID(wk->dat->heap) );
	// パレット(7)
	wk->ResPal[BS_RES_CURSOR_PAL] = GFL_CLGRP_PLTT_RegisterEx( ah,
												NARC_battgra_wb_battle_w_obj_NCLR,
												CLSYS_DRAW_SUB, BS_OBJ_PAL_CURSOR*0x20, BS_CURSOR_OFFSET, 1, wk->dat->heap );
	// セル・アニメ
	wk->ResCell[BS_RES_CURSOR_CELL] = GFL_CLGRP_CELLANIM_Register( ah,
												NARC_battgra_wb_battle_w_cursor_NCER,
												NARC_battgra_wb_battle_w_cursor_NANR,
												wk->dat->heap );
	// キャラ
	wk->ResChr[BS_RES_CURSOR_CHR] = GFL_CLGRP_CGR_Register( ah, NARC_battgra_wb_battle_w_cursor_NCGR, FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
	//
	GFL_ARC_CloseDataHandle( ah );
}
//--------------------------------------------------------------------------------------------
/**
 * @brief	矢印のグラフィックロード
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bscmn_init_arrow_obj( BSCENARIO_WORK * wk )
{
	ARCHANDLE	*ah;
	u32			i;
	
	ah = GFL_ARC_OpenDataHandle( ARCID_B_SCENARIO_GRA, GFL_HEAP_LOWID(wk->dat->heap) );
	// パレット
	wk->ResPal[BS_RES_ARROW_PAL] = GFL_CLGRP_PLTT_RegisterEx( ah,
												NARC_b_scenario_gra_pkwd_scenario_arrow_NCLR,
												CLSYS_DRAW_SUB, BS_OBJ_PAL_ARROW*0x20, 0, BS_PAL_ARROW_NUM, wk->dat->heap );
	// セル・アニメ
	wk->ResCell[BS_RES_ARROW_CELL] = GFL_CLGRP_CELLANIM_Register( ah,
												NARC_b_scenario_gra_pkwd_scenario_arrow_NCER,
												NARC_b_scenario_gra_pkwd_scenario_arrow_NANR,
												wk->dat->heap );
	// キャラ
	wk->ResChr[BS_RES_ARROW_CHR] = GFL_CLGRP_CGR_Register( ah, NARC_b_scenario_gra_pkwd_scenario_arrow_NCGR, FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
	//
	GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	タイプアイコン切り替え(技タイプも同じでよいらしい)
 *
 * @param	BSCENARIO_WORK*		wk
 * @param	const s32			ワーク番号(reschrと対応)
 * @param	const s32			(0:タイプ、1:技タイプ)
 * @param	const s32			タイプ値
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BsCmn_ChgTypeObj( BSCENARIO_WORK * wk, const s32 wnum, const s32 mode, const u32 type )
{
	NNSG2dCharacterData * dat;
	void * buf;
	ARCHANDLE * ah;
	
	ah = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, GFL_HEAP_LOWID(wk->dat->heap) );
	//
	// CGRを置き換える方法
	buf = GFL_ARCHDL_UTIL_LoadOBJCharacter( ah, APP_COMMON_GetPokeTypeCharArcIdx(type), FALSE, &dat, GFL_HEAP_LOWID(wk->dat->heap) );
	//
	GFL_CLGRP_CGR_Replace( wk->ResChr[BS_RES_TYPE_CHR_00+wnum], dat );
	GFL_HEAP_FreeMemory( buf );
	//
	GFL_ARC_CloseDataHandle( ah );
	// パレット変更(転送した番号(読み込ませた際にセットされる) + offset)
	GFL_CLACT_WK_SetPlttOffs( wk->clwk[wnum], APP_COMMON_GetPokeTypePltOffset(type), CLWK_PLTTOFFS_MODE_PLTT_TOP );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ポケアイコン切り替え(毎回でなくてもよいはず:要調整)
 *
 * @param	BSCENARIO_WORK*		wk
 * @param	const s32			ワーク番号
 * @param	const u32			id
 * @param	const s32			性別
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BsCmn_ChgTypePoke( BSCENARIO_WORK * wk, const s32 wnum, const u32 id, const s32 sex )
{
	// パレット設定
	u8 pal_num = POKEICON_GetPalNum( id, 0, sex, FALSE );
	GFL_CLACT_WK_SetPlttOffs( wk->clwk_poke[wnum], pal_num, CLWK_PLTTOFFS_MODE_OAM_COLOR );
	// アニメON
	GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk_poke[wnum], TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	BSCENARIO_WORK*		wk
 *	@param	const s32			drawtype
 *	@param	const BS_OBJ_DATA* 	アドレス
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bscmn_init_obj_clwk( BSCENARIO_WORK* wk, const s32 drawtype, const BS_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16						tmp;
	s16						pos[2];
	s16						buf[6];
	s32 					i;
	BOOL					bflg;
	GFL_CLWK_DATA 			cldata;
	CLWK_FLIP_TYPE			flip;
    GFL_CLUNIT*				p_clunit = wk->clunit;		// セット
	// アドレスセット
	const BS_OBJ_DATA *adr 	= obj_adr;
	const s16 				*iadr 	= info_adr;
	GFL_CLWK				**clwk;			// アドレス注意
	
	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
	// チェック
	buf[0] = iadr[0];		// plt
	buf[1] = iadr[1];		// chr(未使用)
	buf[2] = iadr[2];		// cel
	buf[3] = iadr[3];		// max
	buf[4] = iadr[4];		// mode(0:type、1:poke、2:cursor、3:arrow)
	//
	// アドレスセット
	if(buf[4] == BS_CLWKMODE_NORMAL){		// 通常
		clwk = &wk->clwk[0];
	}
	else if(buf[4] == BS_CLWKMODE_POKE){	// poke
		clwk = &wk->clwk_poke[0];
	}
	else if(buf[4] == BS_CLWKMODE_CURSOR){	// cursor
		clwk = &wk->clwk_cursor[0];
	}
	else if(buf[4] == BS_CLWKMODE_ARROW){	// arrow
		clwk = &wk->clwk_arrow[0];
	}
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		pos[0] = bscmn_get_empty(wk, tmp*2);
		pos[1] = bscmn_get_empty(wk, tmp*2+1);
		cldata.pos_x 	= pos[0];
		cldata.pos_y 	= pos[1];
		cldata.anmseq 	= adr->anmseq;
		cldata.softpri	= adr->softpri;
		cldata.bgpri	= adr->bgpri;
		//
		clwk[i] =   GFL_CLACT_WK_Create( p_clunit, 
											wk->ResChr[  adr->res_cnum ],	// chr(注意)
											wk->ResPal[  buf[0] ],			// plt
											wk->ResCell[ buf[2] ],			// cel
											&cldata,
											drawtype,
											wk->dat->heap );
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
 *	@brief  OBJ初期化まとめ
 *
 *	@param	BSCENARIO_WORK*		wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_InitObj( BSCENARIO_WORK* wk )
{
	s32		i;
	// 作成
	wk->clunit = GFL_CLACT_UNIT_Create( BS_TOTAL_CLWK_NUM, 0, GFL_HEAP_LOWID(wk->dat->heap) );
	//
	// タイプ
	bscmn_init_type_obj( wk );
	// ポケモン
	bscmn_init_poke_obj( wk );
	// カーソル
	bscmn_init_cursor_obj( wk );
	// 矢印
	bscmn_init_arrow_obj( wk );
	//
	// CLWK登録(type)
	bscmn_init_obj_clwk( wk, CLSYS_DRAW_SUB, &bs_obj_00_tbl[0], &bs_obj_00_itbl[0] );
	// CLWK登録(poke)
	bscmn_init_obj_clwk( wk, CLSYS_DRAW_SUB, &bs_obj_01_tbl[0], &bs_obj_01_itbl[0] );
	// CLWK登録(cursor)
	bscmn_init_obj_clwk( wk, CLSYS_DRAW_SUB, &bs_obj_02_tbl[0], &bs_obj_02_itbl[0] );
	// CLWK登録(arrow)
	bscmn_init_obj_clwk( wk, CLSYS_DRAW_SUB, &bs_obj_03_tbl[0], &bs_obj_03_itbl[0] );
	//
	// cursorはパレット番号を修正する必要がある
	for(i=0;i<BS_CLWK_CURSOR_MAX;i++){
		GFL_CLACT_WK_SetPlttOffs( wk->clwk_cursor[i], 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );
	}
	// 半透明設定
	// パラメータの設定はG2S_SetBlendAlphaで行う
	// 第1対象面にはobjは含まないようにすること(全objにかかってしまう)
	GFL_CLACT_WK_SetObjMode( wk->clwk_arrow[BS_CLWK_ARROW_UPMARK], GX_OAM_MODE_XLU );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	BSCENARIO_WORK*		wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_ExitObj( BSCENARIO_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<BS_CLWK_MAX;i++){
		GFL_CLACT_WK_Remove( wk->clwk[i] );
	}
	for(i=0;i<BS_CLWK_POKE_MAX;i++){
		GFL_CLACT_WK_Remove( wk->clwk_poke[i] );
	}
	for(i=0;i<BS_CLWK_CURSOR_MAX;i++){
		GFL_CLACT_WK_Remove( wk->clwk_cursor[i] );
	}
	for(i=0;i<BS_CLWK_ARROW_MAX;i++){
		GFL_CLACT_WK_Remove( wk->clwk_arrow[i] );
	}
	//
	//リソース破棄
	for(i=0;i<BS_RES_PAL_MAX;i++){
		if(wk->ResPal[i] != 0xffffffff){
			GFL_CLGRP_PLTT_Release( wk->ResPal[i] );
			wk->ResPal[i] = 0xffffffff;
		}
	}
	for(i=0;i<BS_RES_CHR_MAX;i++){
		if(wk->ResChr[i] != 0xffffffff){
			GFL_CLGRP_CGR_Release( wk->ResChr[i] );
			wk->ResChr[i] = 0xffffffff;
		}
	}
	for(i=0;i<BS_RES_CELL_MAX;i++){
		if(wk->ResCell[i] != 0xffffffff){
			GFL_CLGRP_CELLANIM_Release( wk->ResCell[i] );
			wk->ResCell[i] = 0xffffffff;
		}
	}
	//
	GFL_CLACT_UNIT_Delete( wk->clunit );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	BSCENARIO_WORK*		wk
 *	@param	const s32			(0:通常、1:ポケモン、2:カーソル、3:矢印)
 *	@param	const s32			objワーク番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_ObjVisible( BSCENARIO_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// 表示/非表示
	if(wmode == BS_CLWKMODE_NORMAL){			// 通常
		if(wnum >= BS_CLWK_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->clwk[wnum], bflg );
	}
	else if(wmode == BS_CLWKMODE_POKE){			// poke
		if(wnum >= BS_CLWK_POKE_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->clwk_poke[wnum], bflg );
	}
	else if(wmode == BS_CLWKMODE_CURSOR){		// cursor
		if(wnum >= BS_CLWK_CURSOR_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->clwk_cursor[wnum], bflg );
	}
	else if(wmode == BS_CLWKMODE_ARROW){		// arrow
		if(wnum >= BS_CLWK_ARROW_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->clwk_arrow[wnum], bflg );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  emptyの取得(static)
 *
 *	@param	BSCENARIO_WORK*		wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
static s16 bscmn_get_empty( BSCENARIO_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= BS_EMPTY_MAX)){
		OS_Printf("empty error :%d\n",num);
		return(0);
	}
	
	adr = wk->EmptyBuffer;
	return( adr[num] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  カーソル表示(static)
 *
 *	@param	BSCENARIO_WORK* 	wk
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *	@param	const s32			(0:脚本、1:共演者、2:戻り、3:左矢印、4:右矢印)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bscmn_disp_cursor( BSCENARIO_WORK *wk, const BOOL bflg, const s32 num )
{
	s32				empnum;
	s32				i;
	GFL_CLACTPOS	pos;
	
	// 表示/非表示
	for(i=0;i<4;i++){
		BsCmn_ObjVisible( wk, BS_CLWKMODE_CURSOR, i, bflg );
	}
	
	// 表示の場合は座標をセット
	if(bflg == TRUE){
		for(i=0;i<4;i++){
			// emptyを取得
			empnum = num*4+i;
			pos.x = bscmn_get_empty(wk, 2*bs_cursor_emp_tbl[empnum]);
			pos.y = bscmn_get_empty(wk, 2*bs_cursor_emp_tbl[empnum]+1);
			// セット
			GFL_CLACT_WK_SetPos( wk->clwk_cursor[i], &pos, CLSYS_DEFREND_SUB );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  カーソル表示まとめ
 *
 *	@param	BSCENARIO_WORK* 	wk
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *	@param	const s32			(0:TOP、1:脚本、2:共演者)
 *	@param	const s32			(カーソル位置)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_DispCursor( BSCENARIO_WORK *wk, const BOOL bflg, const s32 mode, const s32 num )
{
	s16		tmp;
	
	// まとめ
	tmp = bs_cursor_all_tbl[4*mode+num];
	if(tmp == -1){
		return;
	}
	// カーソル表示
	bscmn_disp_cursor( wk, bflg, tmp );
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  矢印表示
 *
 *	@param	BSCENARIO_WORK* 	wk
 *	@param	const s32			(0:TOP、1:シナリオ、2:情報)
 *	@param	const BOOL			(TRUE:クリア、FALSE:なし)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_DispArrow( BSCENARIO_WORK *wk, const s32 mode, const BOOL bflg )
{
	s16		buf[2];
	s32		tmp;
	s32		i;
	BOOL	mflg;
	
	// objのテスト
	for(i=0;i<3;i++){
		mflg = TRUE;
		tmp = bs_arrow_tbl[4*mode+i];
		if( tmp == -1 ){
			mflg = FALSE;
		}
		BsCmn_ObjVisible( wk, BS_CLWKMODE_ARROW, i, mflg );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  シーケンス切り替え
 *
 *	@param	BSCENARIO_WORK* 	wk
 *	@param	const s32			(0:左、1:右、2:戻る、3:上マーク)
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_ChgSeq( BSCENARIO_WORK *wk, const s32 wnum, const s32 seqnum )
{
	// 切り替え
	GFL_CLACT_WK_SetAnmSeq( wk->clwk_arrow[wnum], seqnum );
	// セット
	GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk_arrow[wnum], TRUE );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  シーケンス切り替え
 *
 *	@param	BSCENARIO_WORK* 	wk
 *	@param	const s32			(0:左、1:右、2:戻る)
 *
 *	@retval	BOOL				(TRUE:再生中、FALSE:終了)
 */
//-----------------------------------------------------------------------------
BOOL BsCmn_IsAnimEnd( BSCENARIO_WORK *wk, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// 終了判定
	if( GFL_CLACT_WK_CheckAnmActive( wk->clwk_arrow[wnum] ) == FALSE ){
		bflg = FALSE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  スクリーンの書き換え
 *
 *	@param	BSCENARIO_WORK* 	wk
 *	@param	const s32			(番号(0-3))
 *	@param	const s32			(0:元、1:変更後))
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_ChgScreen( BSCENARIO_WORK *wk, const s32 num, const s32 mode )
{
	s16		buf[4];
	s16		pnum;
	
	buf[0] = bs_area_tbl[4*num];
	buf[1] = bs_area_tbl[4*num+1];
	buf[2] = bs_area_tbl[4*num+2];
	buf[3] = bs_area_tbl[4*num+3];
	//
	pnum = bs_apal_tbl[2*num+mode];
	//
	// 書き換え(領域は単一のカラーになってしまうので注意)
	GFL_BG_ChangeScreenPalette( GFL_BG_FRAME3_S, buf[0], buf[1], buf[2], buf[3], pnum );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME3_S );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ボタン(スクリーン)アニメ初期化
 *
 *	@param	BSCENARIO_WORK* 	wk
 *	@param	const s32			(番号(0-3))
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_InitScreenAnime( BSCENARIO_WORK *wk, const s32 num )
{
	wk->AnimFlag |= FLG_SCREEN_ANIM;
	//
	wk->Timer[BS_TIMER_ANIM] = 0;
	wk->AnimType 	= num;
	wk->AnimNum		= 0;
	//
	// 1フレーム目の処理
	BsCmn_MainScreenAnime( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ボタン(スクリーン)アニメ処理
 *
 *	@param	BSCENARIO_WORK* 	wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_MainScreenAnime( BSCENARIO_WORK *wk )
{
	// 終了判定
	if(!(wk->AnimFlag & FLG_SCREEN_ANIM)){
		return;
	}
	//
	// カウンタ加算
	wk->Timer[BS_TIMER_ANIM]++;
	//
	// 指定したフレームに来た
	if(bs_timer_tbl[2*wk->AnimNum] == wk->Timer[BS_TIMER_ANIM]){
		// 指定のパターンを表示
		if(bs_timer_tbl[2*wk->AnimNum+1] != -1){
			BsCmn_ChgScreen( wk, wk->AnimType, bs_timer_tbl[2*wk->AnimNum+1] );
		}
		// 加算
		wk->AnimNum++;
		// 終了判定
		if(bs_timer_tbl[2*wk->AnimNum] == -1){
			wk->AnimFlag &= ~FLG_SCREEN_ANIM;
			wk->Timer[BS_TIMER_ANIM] = 0;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ボタン(スクリーン)アニメ判定
 *
 *	@param	BSCENARIO_WORK* 	wk
 *
 *	@retval BOOL				(TRUE:再生中、FALSE:終了)
 */
//-----------------------------------------------------------------------------
BOOL BsCmn_IsScreenAnimeEnd( BSCENARIO_WORK *wk )
{
	BOOL	bflg=FALSE;
	//
	if(wk->AnimFlag & FLG_SCREEN_ANIM){
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  上マーク表示
 *
 *	@param	BSCENARIO_WORK* 	wk
 *	@param	const s32			(0:TOP、1:シナリオ、2:情報)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_DispUpMark( BSCENARIO_WORK *wk, const s32 mode )
{
	BOOL	bflg = TRUE;
	
	// TOPは非表示
	if(mode == BS_TOP){
		bflg = FALSE;
	}
	BsCmn_ObjVisible( wk, BS_CLWKMODE_ARROW, BS_CLWK_ARROW_UPMARK, bflg );
	//
	// ある場合はアニメ再生(なくてもよい)
//	if(bflg == TRUE){
//		BsCmn_ChgSeq( wk, BS_CLWK_ARROW_UPMARK, BS_SEQ_UPMARK );
//	}
}

