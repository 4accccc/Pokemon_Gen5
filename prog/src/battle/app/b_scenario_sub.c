//============================================================================================
/**
 * @file		b_scenario_sub.c
 * @brief		戦闘用シナリオサブ画面
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

//#include "ui/print_tool.h"

#include "b_app_tool.h"

#include "b_scenario.h"
#include "b_scenario_main.h"

#include "b_scenario_gra.naix"

//============================================================================================
//  定数定義
//============================================================================================
enum{
	MES_CLEAR_COLOR = 0,			// クリアカラー
};

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}B_SCENARIO_SUB_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}B_SCENARIO_SUB_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}B_SCENARIO_SUB_BGSCN_DATA;


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
}B_SCENARIO_SUB_CANVAS_DATA;

//============================================================================================
//  データ
//============================================================================================
//---------------
// BG関連(top)
//---------------
// BGPAL
static const B_SCENARIO_SUB_BGPAL_DATA bs_bgpal_00_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_b_scenario_gra_pkwd_scenario_NCLR,	0,	0,},	// 下、BG
	{ 0xff, 0, 0, 0,}, // end code
};
// BGCHR
static const B_SCENARIO_SUB_BGCHR_DATA bs_bgchr_00_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_b_scenario_gra_pkwd_scenario_NCGR,	0,	0,},	// 下、BG
	{ 0xff, 0, 0, 0,}, // end code
};
// BGSCN
static const B_SCENARIO_SUB_BGSCN_DATA bs_bgscn_00_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_b_scenario_gra_pkwd_scenario_NSCR,	0,	0,},	// 下、BG
	{ 0xff, 0, 0, 0,}, // end code
};

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//---------------
// CANVASの設定
//---------------
// TOP
static const B_SCENARIO_SUB_CANVAS_DATA bs_canvas_00_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	1,	28,	2,	8,},		// TITLE
	{ GFL_BG_FRAME1_S,	2,	5,	28,	2,	8,},		// PLOT
	{ GFL_BG_FRAME1_S,	2,	12,	28,	2,	8,},		// GOAL
	{ GFL_BG_FRAME1_S,	3,	3,	28,	2,	8,},		// TITLE_MES
	{ GFL_BG_FRAME1_S,	3,	7,	28,	4,	8,},		// PLOT_MES
	{ GFL_BG_FRAME1_S,	3,	14,	28,	4,	8,},		// GOAL_MES
	{ GFL_BG_FRAME1_S,	2,	21,	10,	2,	8,},		// 脚本
	{ GFL_BG_FRAME1_S,	14,	21,	10,	2,	8,},		// 共演者

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};
// SCENARIO
static const B_SCENARIO_SUB_CANVAS_DATA bs_canvas_01_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	1,	28,	2,	8,},		// SCENARIO
	{ GFL_BG_FRAME1_S,	2,	3,	28,	2,	8,},		// SCENE00
	{ GFL_BG_FRAME1_S,	2,	10,	28,	2,	8,},		// SCENE01
	{ GFL_BG_FRAME1_S,	3,	5,	28,	2,	8,},		// SCENE00_MES
	{ GFL_BG_FRAME1_S,	3,	12,	28,	2,	8,},		// SCENE01_MES
	{ GFL_BG_FRAME1_S,	2,	21,	10,	2,	8,},		// 脚本
	{ GFL_BG_FRAME1_S,	12,	16,	 8,	2,	8,},		// 現在(test)
	{ GFL_BG_FRAME1_S,	15,	16,	 8,	2,	8,},		// 最大(test)

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};
// INFO
static const B_SCENARIO_SUB_CANVAS_DATA bs_canvas_02_tbl[] = {
	{ GFL_BG_FRAME1_S,	2,	1,	28,	2,	8,},		// CAST
	{ GFL_BG_FRAME1_S,	2,	7,	8,	2,	8,},		// 名前
	{ GFL_BG_FRAME1_S,	5,	10,	10,	2,	8,},		// レベル
	{ GFL_BG_FRAME1_S,	2,	14,	10,	4,	8,},		// 持ち物
	{ GFL_BG_FRAME1_S,	16,	4,	10,	2,	8,},		// 技00
	{ GFL_BG_FRAME1_S,	16,	7,	10,	2,	8,},		// 技01
	{ GFL_BG_FRAME1_S,	16,	10,	10,	2,	8,},		// 技02
	{ GFL_BG_FRAME1_S,	16,	13,	10,	2,	8,},		// 技03
	{ GFL_BG_FRAME1_S,	14,	16,	12,	4,	8,},		// 特性
	{ GFL_BG_FRAME1_S,	2,	21,	10,	2,	8,},		// 脚本

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

// まとめ
static const B_SCENARIO_SUB_CANVAS_DATA *bscmn_canvas_tbl[] = {
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
void BsCmn_ClearAllCanvas( BSCENARIO_WORK* wk );

//
s32  BsCmn_ChkTp( BSCENARIO_WORK *wk, const s32 mode );

//------------
// static
//------------
static u16  bscmn_putsizeget( const STRBUF * buf, GFL_FONT * font, u32 mode );

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化
 *
 *	@param	BSCENARIO_WORKK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_InitBg( BSCENARIO_WORK* wk )
{
	s32				i;
	s32				tmp;
	s32				heapID;
	ARCHANDLE		*handle;
	const B_SCENARIO_SUB_BGPAL_DATA	*padr = &bs_bgpal_00_tbl[0];
	const B_SCENARIO_SUB_BGCHR_DATA	*cadr = &bs_bgchr_00_tbl[0];
	const B_SCENARIO_SUB_BGSCN_DATA	*sadr = &bs_bgscn_00_tbl[0];
	
	heapID = wk->dat->heap;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_B_SCENARIO_GRA, GFL_HEAP_LOWID(heapID) );
	//
	// パレット転送
	for(i=0;i<BSCENARIO_BG_MAXNUM;i++){
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
	for(i=0;i<BSCENARIO_BG_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<BSCENARIO_BG_MAXNUM;i++){
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
 *	@param	BSCENARIO_WORK* wk
 *	@param	const 32		(段階:(0-2))
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
 *	@param	BSCENARIO_WORK* wk
 *	@param	const s32		(0-2)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_InitString( BSCENARIO_WORK* wk, const s32 mode )
{
	s32		i;
	s32		heapID = wk->dat->heap;
	const B_SCENARIO_SUB_CANVAS_DATA *adr = bscmn_canvas_tbl[mode];
	
	// (いままでのを)クリア
	BsCmn_ClearAllCanvas( wk );
	
	// 破棄(念のため)
	BsCmn_ExitString( wk );
	//
	for(i=0;i<BSCENARIO_CANVAS_MAXNUM;i++){
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
 *	@param	BSCENARIO_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_ExitString( BSCENARIO_WORK* wk )
{
	s32		i;
	
	for(i=0;i<BSCENARIO_CANVAS_MAXNUM;i++){
		if(wk->PrintUtil[i].win != 0){
			GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
			wk->PrintUtil[i].win = 0;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア
 *
 *	@param	BSCENARIO_WORK* wk
 *	@param	const s32		キャンバスの番号
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
 *	@brief  ウインドウクリア(全クリアでよいはず)
 *
 *	@param	BSCENARIO_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_ClearAllCanvas( BSCENARIO_WORK* wk )
{
	s32		i;
	
	for(i=0;i<BSCENARIO_CANVAS_MAXNUM;i++){
		if(wk->PrintUtil[i].win){
			GFL_BMPWIN_ClearTransWindow( wk->PrintUtil[i].win );
			// 転送しないとだめらしい
			PRINT_UTIL_Trans( &wk->PrintUtil[i], wk->print_que );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  Queの処理
 *
 *	@param	BSCENARIO_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BsCmn_MainPrint( BSCENARIO_WORK *wk )
{
	s32		i;
	BOOL	bflg;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<BSCENARIO_CANVAS_MAXNUM;i++){
		PRINT_UTIL_Trans( &wk->PrintUtil[i], wk->print_que );
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
 *	@param	BSCENARIO_WORK* wk
 *	@param	const s32		(0-2)
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





#if 0		// objの場合
//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	B_SCENARIO_SUB_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const B_SCENARIO_SUB_OBJ_DATA* アドレス
 *	@param	const s16*				アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bscmn_init_obj_clwk( BSCENARIO_WORK* wk, const s32 drawtype, const BSCENARIO_OBJ_DATA *obj_adr, const s16 *info_adr )
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
	const B_SCENARIO_SUB_OBJ_DATA *adr 	= obj_adr;
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
 *	@param	BSCENARIO_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bscmn_init_obj_res( BSCENARIO_WORK* wk, const s32 drawtype, const s16 *info_adr )
{
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_WIN_RECORD_GRA, wk->heapID );
    GFL_CLUNIT*	p_clunit = wk->clunit;
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
 *	@param	BSCENARIO_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bscmn_init_obj( BSCENARIO_WORK* wk )
{
    GFL_CLUNIT*	p_clunit = B_SCENARIO_SUB_GRAPHIC_GetClunit( wk->graphic );
	//
	// リソース読み込み(通常)
	bscmn_init_obj_res( wk, CLSYS_DRAW_MAIN, &td_obj_main_info_tbl[0] );
	//
	// CLWK登録(通常)
	bscmn_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &td_obj_tbl[0], &td_obj_main_itbl[0] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	BSCENARIO_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bscmn_exit_obj( BSCENARIO_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<B_SCENARIO_SUB_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk[i] );
	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_OBJ_CEL] );
}

#endif





