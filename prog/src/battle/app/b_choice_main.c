//============================================================================================
/**
 * @file		b_choice_main.c
 * @brief		ポケウッド選択肢画面
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

#include "../btlv/btlv_effect.h"

#include "b_app_tool.h"
//
#include "b_choice.h"
#include "b_choice_main.h"

#include "b_choice_snd_def.h"

#include "b_choice_gra.naix"
#include "battle/battgra_wb.naix"

//============================================================================================
//  定数定義
//============================================================================================
// 遷移(Step)
enum{
	STEP_INIT		= 0,
	STEP_LV1		= 100,
	STEP_LV2		= 200,
	STEP_LV3		= 300,
	//
	STEP_END		= 10000,
};

// ライン
enum{
	MODE_LINE2 = 0,					// 2行
	MODE_LINE3,						// 3行
	MODE_LINE4,						// 4行
};

//------------
// フラグ
//------------
// 全体のフラグ(BCHOICE.Flag)
enum{
	FLG_UPDATE			= (1<<0),	// 更新フラグ
	FLG_KEY_INPUT		= (1<<1),	// キー入力選択があった
	FLG_KEY_INPUT_END	= (1<<2),	// キー入力で終了
};

// OBJのフラグ(BC_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM		= (1<<0),	// アニメあり
	FLG_OBJ_HFLIP		= (1<<1),	// HFLIP
	FLG_OBJ_VFLIP		= (1<<2),	// VFLIP
};

//-----------
// いろいろ
//-----------
enum{
	BC_BG_MAXNUM		= 8,		// BG最大
	BC_EMPTY_MAXNUM		= 36*2,		// EMPTY最大
	//
	BC_CURSOR_PAL		= 0,		// カーソルパレット(0番にセット)
	BC_CURSOR_OFFSET	= 4,		// カーソルデータ(注意：なぜかpalの中の4番目を使用しているらしい)
	//
	BC_CENTER_MES_X		= 112,		// 中心の座標(message)
};

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}BC_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				comp;
}BC_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				comp;
}BC_BGSCN_DATA;

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
}BC_CANVAS_DATA;

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
	u8				res_cnum;		// ResChrの番号(未使用)
}BC_OBJ_DATA;

//============================================================================================
//  データ
//============================================================================================
//---------------
// BG関連
//---------------
// BGPAL
static const BC_BGPAL_DATA bc_bgpal_00_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_b_choice_gra_pkwd_word_NCLR,	0,	4,},		// 下、BG
	{ 0xff, 0, 0, 0,}, // end code
};
// BGCHR
static const BC_BGCHR_DATA bc_bgchr_00_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_b_choice_gra_pkwd_choice_bg2_NCGR,	0,	0,},	// 下、BG
	{ GFL_BG_FRAME2_S,	NARC_b_choice_gra_pkwd_word_NCGR,	0,	0,},		// 下、BG
	{ 0xff, 0, 0, 0,}, // end code
};
//------------
// スクリーン
//------------
// BGSCN(2択)
static const BC_BGSCN_DATA bc_bgscn_00a_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_b_choice_gra_pkwd_choice_bg2_NSCR,	0,	0,},	// 下、BG
	{ GFL_BG_FRAME2_S,	NARC_b_choice_gra_pkwd_words_x2_NSCR,	0,	0,},	// 下、win
	{ 0xff, 0, 0, 0,}, // end code
};
// BGSCN(3択)
static const BC_BGSCN_DATA bc_bgscn_00b_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_b_choice_gra_pkwd_choice_bg2_NSCR,	0,	0,},	// 下、BG
	{ GFL_BG_FRAME2_S,	NARC_b_choice_gra_pkwd_words_x3_NSCR,	0,	0,},	// 下、win
	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN(4択)
static const BC_BGSCN_DATA bc_bgscn_00c_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_b_choice_gra_pkwd_choice_bg2_NSCR,	0,	0,},	// 下、BG
	{ GFL_BG_FRAME2_S,	NARC_b_choice_gra_pkwd_words_x4_NSCR,	0,	0,},	// 下、win
	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN(まとめ)
static const BC_BGSCN_DATA *bc_bgscn_tbl[] = {
	bc_bgscn_00a_tbl,
	bc_bgscn_00b_tbl,
	bc_bgscn_00c_tbl,
};

//---------------
// CANVASの設定
//---------------
// 2択
static const BC_CANVAS_DATA bc_canvas_00a_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	6,	28,	2,	8,},		// 00
	{ GFL_BG_FRAME1_S,	2, 13,	28,	2,	8,},		// 01

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};
// 3択
static const BC_CANVAS_DATA bc_canvas_00b_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	3,	28,	2,	8,},		// 00
	{ GFL_BG_FRAME1_S,	2,	9,	28,	2,	8,},		// 01
	{ GFL_BG_FRAME1_S,	2, 15,	28,	2,	8,},		// 02

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};
// 4択
static const BC_CANVAS_DATA bc_canvas_00c_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	2,	28,	2,	8,},		// 00
	{ GFL_BG_FRAME1_S,	2,	7,	28,	2,	8,},		// 01
	{ GFL_BG_FRAME1_S,	2, 12,	28,	2,	8,},		// 02
	{ GFL_BG_FRAME1_S,	2, 17,	28,	2,	8,},		// 03

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

// CANVAS(まとめ)
static const BC_CANVAS_DATA *bc_canvas_tbl[] = {
	bc_canvas_00a_tbl,
	bc_canvas_00b_tbl,
	bc_canvas_00c_tbl,
};

//------------------
// タッチペン領域
//------------------
// 2行
static const GFL_UI_TP_HITTBL bc_rect_00_tbl[] = {
  {  40,  72,  8, 248 }, 			// 00
  {  96, 128,  8, 248 }, 			// 01
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// 3行
static const GFL_UI_TP_HITTBL bc_rect_01_tbl[] = {
  {  16,  48,  8, 248 }, 			// 00
  {  64,  96,  8, 248 }, 			// 01
  { 112, 144,  8, 248 }, 			// 02
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// 4行
static const GFL_UI_TP_HITTBL bc_rect_02_tbl[] = {
  {   8,  40,  8, 248 }, 			// 00
  {  48,  80,  8, 248 }, 			// 01
  {  88, 120,  8, 248 }, 			// 02
  { 128, 160,  8, 248 }, 			// 03
  //
  { GFL_UI_TP_HIT_END,0,0,0 }
};

// タッチペン(まとめ)
static const GFL_UI_TP_HITTBL *bc_rect_tbl[] = {
	bc_rect_00_tbl,
	bc_rect_01_tbl,
	bc_rect_02_tbl,
};

//----------------
// OBJ(cursor)
//----------------
// 情報
static const s16 bc_obj_00_itbl[] = {
	0,		// plt res
	1,		// chr res
	2,		// cel res
	//
	4,		// max
	0,		// mode(1:poke)
};

// OBJのテーブルデータ
const BC_OBJ_DATA bc_obj_00_tbl[] = {
	{ 0,	0x30,	1,	0,	0,	1,	0,	0,},	// カーソル00
	{ 1,	0x30,	1,	0,	0,	1,	0,	0,},	// カーソル01
	{ 2,	0x30,	1,	0,	0,	1,	0,	0,},	// カーソル02
	{ 3,	0x30,	1,	0,	0,	1,	0,	0,},	// カーソル03
	
	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//----------------
// OBJ(cursor)座標
//----------------
// (左上、左下、右上、右下)の順
// 2行
static const s16 bc_cursor00_emp_tbl[] = {
	 0, 1, 2, 3,			// 2行(00)
	 4, 5, 6, 7,			// 2行(01)
};

// 3行
static const s16 bc_cursor01_emp_tbl[] = {
	 8, 9,10,11,			// 3行(00)
	12,13,14,15,			// 3行(01)
	16,17,18,19,			// 3行(02)
};

// 4行
static const s16 bc_cursor02_emp_tbl[] = {
	20,21,22,23,			// 4行(00)
	24,25,26,27,			// 4行(01)
	28,29,30,31,			// 4行(02)
	32,33,34,35,			// 4行(03)
};

// カーソル座標(まとめ)
static const s16 *bc_cursor_emp_tbl[] = {
	bc_cursor00_emp_tbl,	// 2行
	bc_cursor01_emp_tbl,	// 3行
	bc_cursor02_emp_tbl,	// 4行
};

//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void battle_choice_main( GFL_TCB* tcb, void * work );
static BOOL battle_choice_end(  GFL_TCB* tcb, BCHOICE_WORK * wk );

// まとめ
static void bc_main_seq( BCHOICE_WORK * wk );
static void bc_init_seq(  BCHOICE_WORK * wk );
//
static void bc_init_wipe( BCHOICE_WORK * wk );
static BOOL bc_is_wipe(   BCHOICE_WORK * wk );
//
static void bc_set_endfade(  BCHOICE_WORK * wk );
static BOOL bc_is_endfade( BCHOICE_WORK * wk );
//
static void bc_init_graphics( BCHOICE_WORK * wk );
static void bc_exit_bg( BCHOICE_WORK * wk);
static void bc_set_palanim( BCHOICE_WORK * wk );
//
// 処理
static BOOL bc_main_loop( BCHOICE_WORK * wk );
static BOOL bc_key_input( BCHOICE_WORK * wk );
//
static void bc_init_bg( BCHOICE_WORK* wk, const s32 mode );
static void bc_init_string( BCHOICE_WORK* wk, const s32 mode );
static void bc_exit_string( BCHOICE_WORK* wk );
static void bc_disp_string( BCHOICE_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col);
static void bs_disp_string_number( BCHOICE_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 num, const s32 lnum, const u32 pmode );
static u16  bc_get_putsize( const STRBUF * buf, GFL_FONT * font, u32 mode );
static void bc_main_print( BCHOICE_WORK *wk );
static s32  bc_chk_tp( BCHOICE_WORK *wk, const s32 mode );
//
static void bc_disp_message( BCHOICE_WORK *wk );
//
// obj関連
static void bc_init_obj( BCHOICE_WORK* wk );
static void bc_exit_obj( BCHOICE_WORK* wk );
static void bc_init_cursor_obj( BCHOICE_WORK * wk );
static void bc_init_obj_clwk( BCHOICE_WORK* wk, const s32 drawtype, const BC_OBJ_DATA *obj_adr, const s16 *info_adr );
static void bc_set_obj_visible( BCHOICE_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
static void bc_disp_cursor( BCHOICE_WORK *wk, const BOOL bflg, const s32 mode, const s32 num );
//
// empty
static s16  bc_get_empty( BCHOICE_WORK *wk, const s32 num );
//
static void bc_chk_param( BCHOICE_WORK* wk );

//============================================================================================
//============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * @brief	選択肢シナリオタスク追加
 *
 * @param	BCHOICE_DATA*			選択肢データ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BattleChoice_TaskAdd( BCHOICE_DATA * dat )
{
	s32				i;
	BCHOICE_WORK*	wk;
	//
	// メモリ確保
	wk = GFL_HEAP_AllocClearMemory( dat->heap, sizeof(BCHOICE_WORK) );
	//
	// battle_choice_mainを登録する
	GFL_TCB_AddTask( BTLV_EFFECT_GetTCBSYS(), battle_choice_main, wk, 100 );
	//
	wk->dat			= dat;
	wk->pfd 		= BTLV_EFFECT_GetPfd();
	wk->initFlag	= FALSE;
	//
	// 初期化
	wk->Step 		= STEP_INIT;
	wk->Flag		= 0;
	wk->Mode		= wk->dat->mesnum-2;
	wk->Result		= -1;
	wk->Cursor		= 0;
	//
	for(i=0;i<BC_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
	}
	// チェック
//	bc_chk_param( wk );
	//
	// 処理(初期化用)
	bc_main_seq( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	メインタスク
 *
 * @param	GFL_TCB*		tcb
 * @param	void*			work
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void battle_choice_main( GFL_TCB * tcb, void * work )
{
	BCHOICE_WORK * wk = (BCHOICE_WORK *)work;
	
	// 通信エラー処理
	if( wk->dat->comm_err_flg == TRUE ){
		wk->Result = 0;
		wk->Step = STEP_END;
	}
	//
	// 処理
	bc_main_seq( wk );
	//
	// 終了処理
	if( wk->Step == STEP_END ){
    	if( battle_choice_end( tcb, wk ) == TRUE ){
      		return;
    	}
    }
    //
	// 処理
	GFL_TCBL_Main( wk->tcbl );
	// que
	bc_main_print( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	終了処理
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @retval	BOOL					(TRUE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL battle_choice_end( GFL_TCB * tcb, BCHOICE_WORK * wk )
{
	s32		mode;
	s32		i;
	int		*adr;
	
	// キュー待ち
	if( PRINTSYS_QUE_IsFinished( wk->print_que ) == FALSE ){
		// 通信エラー時は強制クリア
		if( wk->dat->comm_err_flg == TRUE ){
			PRINTSYS_QUE_Clear( wk->print_que );
		}else{
			return FALSE;
		}
	}
	// ワイプ待ち
	if( WIPE_SYS_EndCheck() == FALSE ){
		// 通信エラー時は強制終了
		if( wk->dat->comm_err_flg == TRUE ){
			WIPE_SYS_ExeEnd();
		}else{
			return FALSE;
		}
	}
	
	//-------------------------
	// 以下の処理になると完全終了
	//-------------------------
	// obj
	bc_exit_obj( wk );
	// bg
	bc_exit_bg( wk );
	//
	GFL_TCBL_Exit( wk->tcbl );
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	
	// 文字列終了
	bc_exit_string( wk );
	// PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	//FONT(大元を使用)
//	GFL_FONT_Delete( wk->font );
	
	wk->dat->end_flg = 1;
	//
	// キー入力で終了したか
	mode = GFL_APP_END_TOUCH;
	if(wk->Flag & FLG_KEY_INPUT_END){
		mode = GFL_APP_END_KEY;
	}
	GFL_UI_SetTouchOrKey( mode );
	//
	// アドレスがあるときだけ結果をセット
	adr = wk->dat->result;
	if(adr != 0){
		*adr = wk->Result;
	}
	GFL_TCB_DeleteTask( tcb );
	GFL_HEAP_FreeMemory( wk );
	
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	シーケンスmain
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bc_main_seq( BCHOICE_WORK * wk )
{
	BOOL	bflg;
	//
	switch(wk->Step){
		// 初期化
		case STEP_INIT:
			bc_init_seq( wk );
			wk->Step = STEP_INIT+10;
			break;
		//
		// ワイプ待ち
		case STEP_INIT+10:
			bflg = bc_is_wipe( wk );
			if(bflg == FALSE){
				wk->Step = STEP_LV1;
			}
			break;
		//
		// main処理
		case STEP_LV1:
			// 内部で分岐処理
			bflg = bc_main_loop( wk );
			if(bflg == FALSE){
				wk->Step = STEP_LV2;
				bc_set_endfade( wk );
			}
			break;
		//
		// フェードアウト待ち
		case STEP_LV2:
			bflg = bc_is_endfade( wk );
			if(bflg == FALSE){
				wk->Step = STEP_END;
			}
			break;
		//
		// 終了待ち
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	初期化シーケンス
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bc_init_seq( BCHOICE_WORK * wk )
{
	int		flg;
	
	G2S_BlendNone();
	GXS_SetMasterBrightness( -16 );
	
	wk->tcbl = GFL_TCBL_Init( wk->dat->heap, wk->dat->heap, 1, 4 );
	//
	// フォント作成(大元を使用)
//	wk->font  = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->dat->heap );
	//
	// PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->dat->heap );
	//
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_B_CHOICE_GRA, NARC_b_choice_gra_b_choice_pos_tblbin, wk->dat->heap );
	//
	// bg設定
	bc_init_graphics( wk );
	// bg
	bc_init_bg( wk, wk->Mode );
	// string
	bc_init_string( wk, wk->Mode );
	// テスト表示
	bc_disp_message( wk );
	// obj
	bc_init_obj( wk );
	// wipe
	bc_init_wipe( wk );
	// パレットアニメ用
	bc_set_palanim( wk );
	//
	// 初期化終了
	wk->initFlag = TRUE;
	//
	// キー入力で終了して、ここに来たかどうか
	flg = GFL_UI_CheckTouchOrKey();
	if(flg == GFL_APP_END_KEY){
		wk->Flag |= FLG_KEY_INPUT;
		wk->Flag |= FLG_UPDATE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	画面初期化時の描画待ち(wipe初期化)
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bc_init_wipe( BCHOICE_WORK * wk )
{
//  if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
		WIPE_SYS_Start(
			WIPE_PATTERN_S, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
			WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->dat->heap );
//	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	画面初期化時のワイプ待ち
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return	BOOL					(TRUE:処理中、FALSE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL bc_is_wipe( BCHOICE_WORK * wk )
{
	BOOL	bflg = TRUE;
	
	// ワイプ判定
	if( WIPE_SYS_EndCheck() == TRUE ){
		bflg = FALSE;
	}
	return( bflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	終了フェード
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bc_set_endfade( BCHOICE_WORK * wk )
{
	PaletteFadeReq( wk->pfd, PF_BIT_SUB_ALL, 0xffff, 0, 0, 16, 0, BTLV_EFFECT_GetTCBSYS() );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	終了フェード待ち
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return	BOOL					(TRUE:処理中、FALSE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL bc_is_endfade( BCHOICE_WORK * wk )
{
	BOOL	bflg=TRUE;
	//
	if( PaletteFadeCheck( wk->pfd ) == 0 ){
		bflg = FALSE;
	}
	return( bflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	BG初期化
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bc_init_graphics( BCHOICE_WORK *wk )
{
  { // BG SYSTEM
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGModeDisp( &BGsys_data, GFL_BG_SUB_DISP );
  }

  { // BG3(背景：offsetを切り替える方式)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x2000, 0,											// X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
      GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,				// ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
      GX_BG_SCRBASE_0xc800, GX_BG_CHARBASE_0x00000, 0x8000,		// ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE							// 拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }

  { // BG2
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x0800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x18000, 0x8000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }

  { // BG1(win)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000, 0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
  }

  { // BG0
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x4000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
  }
  //
  GFL_BG_SetClearCharacter( GFL_BG_FRAME1_S, 32, 0, wk->dat->heap );
  GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 32, 0, wk->dat->heap );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S );

  GFL_DISP_GXS_SetVisibleControl(
    GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 |
    GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3 | GX_PLANEMASK_OBJ,
    VISIBLE_ON );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	BG解放
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bc_exit_bg( BCHOICE_WORK *wk )
{
  GFL_DISP_GXS_SetVisibleControl(
    GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 |
    GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3 | GX_PLANEMASK_OBJ,
    VISIBLE_OFF );
  
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	パレットアニメ用データセット
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bc_set_palanim( BCHOICE_WORK * wk )
{
	// BGのパレット(4本分)
	PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_BG, 0, 0x20*4 );
	// フォントパレット(8)
	PaletteWorkSet_Arc( wk->pfd, ARCID_FONT, NARC_font_talkwin_nclr, wk->dat->heap, FADE_SUB_BG, 0x20, 8*16 );
	//
	// 通信アイコンセット
	GFL_NET_ReloadIconTopOrBottom( FALSE, wk->dat->heap );
	//
	// OBJのパレット(0番目、1本分)
	PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 0, 0x20 );
	// OBJのパレット(14番目、1本分)
	PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 14*16, 0x20 );
	PaletteFadeReq( wk->pfd, PF_BIT_SUB_ALL, 0xffff, 0, 16, 0, 0, BTLV_EFFECT_GetTCBSYS() );
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	処理まとめ
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return	BOOL					(TRUE:制御中、FALSE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL bc_main_loop( BCHOICE_WORK * wk )
{
	BOOL	bflg;
	
	// 入力
	bflg = bc_key_input( wk );
	return( bflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	入力処理
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return	BOOL					(TRUE:制御中、FALSE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL bc_key_input( BCHOICE_WORK * wk )
{
	s32		id=0;
	s32		flg=0;
	s32		mflg=0;
	s32		tflg=0;
	BOOL	bflg=TRUE;
	
	// 更新
	if(wk->Flag & FLG_UPDATE){
		mflg = 1;
		wk->Flag &= ~FLG_UPDATE;
	}
	
	if(mflg == 0){
		// タッチペンの判定
		id = bc_chk_tp( wk, wk->Mode );
		if((id >= 0)&&(id < (wk->Mode+2))){
			flg = 1;
			tflg = 1;
		}
		//
		// キー入力
		if(flg == 0){
			// 上
			if(GFL_UI_KEY_GetRepeat() & PAD_KEY_UP){
				flg = 1;
			}
			// 下
			else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN){
				flg = 2;
			}
			// 決定
			else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
				flg = 3;
			}
		}
	}
	//
	// 判定
	if(flg != 0){
		// キー入力判定
		if(tflg == 0){
			if(wk->Flag & FLG_KEY_INPUT){
				// 上
				if(flg == 1){
					if(wk->Cursor != 0){
						// カーソル
						PMSND_PlaySE( SND_PKWD_BC_CURSOR );
					}
					wk->Cursor--;
					if(wk->Cursor < 0){
						wk->Cursor = 0;
					}
					mflg = 1;
				}
				// 下
				else if(flg == 2){
					if(wk->Cursor != (wk->Mode+2-1)){
						// カーソル
						PMSND_PlaySE( SND_PKWD_BC_CURSOR );
					}
					wk->Cursor++;
					if(wk->Cursor > (wk->Mode+2-1)){
						wk->Cursor = wk->Mode+2-1;
					}
					mflg = 1;
				}
				// 決定
				else if(flg == 3){
					bflg = FALSE;
					wk->Result = wk->Cursor;
					wk->Flag |= FLG_KEY_INPUT_END;
					OS_Printf("(key) result :%d\n",wk->Result);
					// 決定
					PMSND_PlaySE( SND_PKWD_BC_DECIDE );
				}
			}
			else{
				wk->Flag |= FLG_KEY_INPUT;
				mflg = 1;
			}
		}
		// タッチペン処理
		else{
			bflg = FALSE;
			wk->Result = id;
			OS_Printf("(tp) result :%d\n",wk->Result);
			// 決定
			PMSND_PlaySE( SND_PKWD_BC_DECIDE );
		}
	}
	//
	// 更新
	if( mflg == 1 ){
		bc_disp_cursor( wk, TRUE, wk->Mode, wk->Cursor );
	}
	//
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化
 *
 *	@param	BCHOICE_WORKK* 			wk
 *	@param	const s32				(0-2)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_init_bg( BCHOICE_WORK* wk, const s32 mode )
{
	s32				i;
	s32				tmp;
	s32				heapID;
	ARCHANDLE		*handle;
	const BC_BGPAL_DATA	*padr = &bc_bgpal_00_tbl[0];
	const BC_BGCHR_DATA	*cadr = &bc_bgchr_00_tbl[0];
//	const BC_BGSCN_DATA	*sadr = &bc_bgscn_00_tbl[0];
	const BC_BGSCN_DATA	*sadr = bc_bgscn_tbl[mode];		// 切り替え
	
	heapID = wk->dat->heap;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_B_CHOICE_GRA, GFL_HEAP_LOWID(heapID) );
	//
	// パレット転送
	for(i=0;i<BC_BG_MAXNUM;i++){
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
	for(i=0;i<BC_BG_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<BC_BG_MAXNUM;i++){
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
 *	@brief  文字表示準備(Canvasの作成)
 *
 *	@param	BCHOICE_WORK* 			wk
 *	@param	const s32				(0-2)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_init_string( BCHOICE_WORK* wk, const s32 mode )
{
	s32		i;
	s32		heapID = wk->dat->heap;
	const BC_CANVAS_DATA *adr = bc_canvas_tbl[mode];
	//
	for(i=0;i<BC_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット
		wk->PrintUtil[i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		// スクリーンは一度でよいはず
		GFL_BMPWIN_MakeScreen( wk->PrintUtil[i].win );
		GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[i].win) );
		//
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字列破棄
 *
 *	@param	BCHOICE_WORK* 			wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_exit_string( BCHOICE_WORK* wk )
{
	s32		i;
	
	for(i=0;i<BC_CANVAS_MAXNUM;i++){
		if(wk->PrintUtil[i].win != 0){
			GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
			wk->PrintUtil[i].win = 0;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	BCHOICE_WORK* 			wk
 *	@param	const s32				Canvasの番号
 *	@param	const s32				表示座標(x)
 *	@param	const s32				表示座標(y)
 *	@param	const STRBUF* 			表示文字列
 *	@param	const u32				表示位置(左(0)、右(1)、中央(2)など)
 *	@param	const PRINTSYS_LSB		カラー
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_disp_string( BCHOICE_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col )
{
	u16		siz;
	
	// 表示位置の補正
	siz = bc_get_putsize( strbuf, wk->dat->font, pmode );
	PRINT_UTIL_PrintColor( &wk->PrintUtil[mode], wk->print_que, x-siz, y, strbuf, wk->dat->font, col );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
 *
 *	@param	BCHOICE_WORK* 			wk
 *	@param	const s32				Canvasの番号
 *	@param	const s32				表示座標(x)
 *	@param	const s32				表示座標(y)
 *	@param	const s32				値
 *	@param	const s32				桁数
 *	@param	const u32				表示位置(左、右、中央など)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_disp_string_number( BCHOICE_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 num, const s32 lnum, const u32 pmode )
{
	STRBUF	*strbuf;
	
	// 数字を桁分0うめ表示
	strbuf = GFL_STR_CreateBuffer( 128, wk->dat->heap );
	STRTOOL_SetNumber( strbuf, num, lnum, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	// 表示
	bc_disp_string( wk, mode, x, y, strbuf, pmode, PRINTSYS_LSB_Make(1,2,0) );
	//
	GFL_STR_DeleteBuffer( strbuf );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示サイズから各モードの補正値を取得(static)
 *				(ui/printtoolから持ってきた)
 *
 * @param	const STRBUF*	buf		文字列
 * @param	GFL_FONT*		font	フォント
 * @param	u32				mode	表示モード(0-2)
 *
 * @return	補正値
 */
//--------------------------------------------------------------------------------------------
static u16 bc_get_putsize( const STRBUF * buf, GFL_FONT * font, u32 mode )
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
 *	@brief  Queの処理
 *
 *	@param	BCHOICE_WORK*			wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_main_print( BCHOICE_WORK *wk )
{
	s32		i;
	BOOL	bflg;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<BC_CANVAS_MAXNUM;i++){
		PRINT_UTIL_Trans( &wk->PrintUtil[i], wk->print_que );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン処理
 *
 *	@param	BCHOICE_WORK* 			wk
 *	@param	const s32				(0-2)
 *
 *	@retval	s32						(タッチペンのindex番号)
 */
//-----------------------------------------------------------------------------
static s32 bc_chk_tp( BCHOICE_WORK *wk, const s32 mode )
{
	s32	id;
	//
	// タッチペン判定
	id = GFL_UI_TP_HitTrg( bc_rect_tbl[mode] );
	return( id );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ表示(中央設定)
 *
 *	@param	BCHOICE_WORK*			wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_disp_message( BCHOICE_WORK *wk )
{
	s32			i;
	
	// (centerの)座標の指定は注意
	// キャンバスの座標系の中央に設定すること
	for(i=0;i<wk->dat->mesnum;i++){
		if(wk->dat->mesbuffer[i]){
			bc_disp_string( wk, i, BC_CENTER_MES_X, 0, wk->dat->mesbuffer[i], 2, PRINTSYS_LSB_Make(1,2,0) );
//			bc_disp_string_number( wk, i, 104, 0, i*1000, 8, 2 );		// 中央(112,0)
		}
	}
}

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化まとめ
 *
 *	@param	BCHOICE_WORK* 			wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_init_obj( BCHOICE_WORK* wk )
{
	s32		i;
	//
	// 作成
	wk->clunit = GFL_CLACT_UNIT_Create( BC_TOTAL_CLWK_NUM, 0, GFL_HEAP_LOWID(wk->dat->heap) );
	// カーソル
	bc_init_cursor_obj( wk );
	// CLWK登録(cursor)
	bc_init_obj_clwk( wk, CLSYS_DRAW_SUB, &bc_obj_00_tbl[0], &bc_obj_00_itbl[0] );
	//
	// cursorはパレット番号を修正する必要がある
	for(i=0;i<BC_CLWK_CURSOR_MAX;i++){
		GFL_CLACT_WK_SetPlttOffs( wk->clwk_cursor[i], 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	BCHOICE_WORK*			wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_exit_obj( BCHOICE_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<BC_CLWK_CURSOR_MAX;i++){
		GFL_CLACT_WK_Remove( wk->clwk_cursor[i] );
	}
	//
	//リソース破棄
	GFL_CLGRP_PLTT_Release( wk->Res[BC_RES_CURSOR_PAL] );
	GFL_CLGRP_CGR_Release(  wk->Res[BC_RES_CURSOR_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->Res[BC_RES_CURSOR_CELL] );
	//
	GFL_CLACT_UNIT_Delete( wk->clunit );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	カーソルのobjデータセット
 *
 * @param	BCHOICE_WORK*			wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bc_init_cursor_obj( BCHOICE_WORK * wk )
{
	ARCHANDLE	*ah;
	u32			i;
	
	// カーソルデータはパレットの4番目のデータを使用=>0番にした後に=>
	// 転送後にclwkでそのパレットを適用させる必要あり
	ah = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, GFL_HEAP_LOWID(wk->dat->heap) );
	// パレット(0)
	wk->Res[BC_RES_CURSOR_PAL] = GFL_CLGRP_PLTT_RegisterEx( ah,
												NARC_battgra_wb_battle_w_obj_NCLR,
												CLSYS_DRAW_SUB, BC_CURSOR_PAL*0x20, BC_CURSOR_OFFSET, 1, wk->dat->heap );
	// セル・アニメ
	wk->Res[BC_RES_CURSOR_CELL] = GFL_CLGRP_CELLANIM_Register( ah,
												NARC_battgra_wb_battle_w_cursor_NCER,
												NARC_battgra_wb_battle_w_cursor_NANR,
												wk->dat->heap );
	// キャラ
	wk->Res[BC_RES_CURSOR_CHR] = GFL_CLGRP_CGR_Register( ah, NARC_battgra_wb_battle_w_cursor_NCGR, FALSE, CLSYS_DRAW_SUB, wk->dat->heap );
	//
	GFL_ARC_CloseDataHandle( ah );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	BCHOICE_WORK* 			wk
 *	@param	const s32				drawtype
 *	@param	const BC_OBJ_DATA*		アドレス
 *	@param	const s16*				アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_init_obj_clwk( BCHOICE_WORK* wk, const s32 drawtype, const BC_OBJ_DATA *obj_adr, const s16 *info_adr )
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
	const BC_OBJ_DATA *adr 	= obj_adr;
	const s16 				*iadr 	= info_adr;
	GFL_CLWK				**clwk;			// アドレス注意
	
	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
	// チェック
	buf[0] = iadr[0];		// plt
	buf[1] = iadr[1];		// chr
	buf[2] = iadr[2];		// cel
	buf[3] = iadr[3];		// max
	buf[4] = iadr[4];		// mode(未使用)
	//
	// アドレスセット
	clwk = &wk->clwk_cursor[0];
	//
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		pos[0] = bc_get_empty(wk, tmp*2);
		pos[1] = bc_get_empty(wk, tmp*2+1);
		cldata.pos_x 	= pos[0];
		cldata.pos_y 	= pos[1];
		cldata.anmseq 	= adr->anmseq;
		cldata.softpri	= adr->softpri;
		cldata.bgpri	= adr->bgpri;
		//
		clwk[i] =   GFL_CLACT_WK_Create( p_clunit, 
											wk->Res[ buf[1] ],			// chr
											wk->Res[ buf[0] ],			// plt
											wk->Res[ buf[2] ],			// cel
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
 *	@brief  emptyの取得
 *
 *	@param	BCHOICE_WORK* 			wk
 *	@param	const s32				座標index(x,yが2つセットとなっている)
 *
 *	@retval s16						座標
 */
//-----------------------------------------------------------------------------
static s16 bc_get_empty( BCHOICE_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= BC_EMPTY_MAXNUM)){
		OS_Printf("empty error :%d\n",num);
		return(0);
	}
	
	adr = wk->EmptyBuffer;
	return( adr[num] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	BCHOICE_WORK*			wk
 *	@param	const s32				未使用
 *	@param	const s32				objワーク番号
 *	@param	const BOOL				(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_set_obj_visible( BCHOICE_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
//	if(wmode == BS_CLWKMODE_CURSOR){		// cursor
		if(wnum >= BC_CLWK_CURSOR_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->clwk_cursor[wnum], bflg );
//	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  カーソル表示
 *
 *	@param	BCHOICE_WORK* 			wk
 *	@param	const BOOL				(TRUE:表示、FALSE:非表示)
 *	@param	const s32				何行モードか(0-2)
 *	@param	const s32				何番目に表示するか(0-3)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_disp_cursor( BCHOICE_WORK *wk, const BOOL bflg, const s32 mode, const s32 num )
{
	s32				empnum;
	s32				i;
	GFL_CLACTPOS	pos;
	const s16	*adr = bc_cursor_emp_tbl[mode];
	
	// 表示/非表示
	for(i=0;i<4;i++){
		bc_set_obj_visible( wk, 0, i, bflg );
	}
	
	// 表示の場合は座標をセット
	if(bflg == TRUE){
		for(i=0;i<4;i++){
			empnum = adr[4*num+i];
			//
			pos.x = bc_get_empty(wk, 2*empnum);
			pos.y = bc_get_empty(wk, 2*empnum+1);
			// セット
			GFL_CLACT_WK_SetPos( wk->clwk_cursor[i], &pos, CLSYS_DEFREND_SUB );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータチェック(準備のみ)
 *
 *	@param	BCHOICE_WORK*			wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bc_chk_param( BCHOICE_WORK* wk )
{
	s32		i;
	
	// メッセージ数チェック
	if((wk->dat->mesnum < 2)||(wk->dat->mesnum > 4)){
		GF_ASSERT_MSG(0, "mesnum = %d\n", wk->dat->mesnum );
	}
	//
	for(i=0;i<wk->dat->mesnum;i++){
		if(wk->dat->mesbuffer[i] == 0){
			GF_ASSERT_MSG(0, "mesbuffer[%d] is 0\n", i );
			break;		// なくてもいいはず
		}
	}
}

