//============================================================================================
/**
 * @file		zknsearch_bmp.c
 * @brief		図鑑検索画面 ＢＭＰ関連
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZKNSEARCHBMP
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/main.h"
#include "ui/print_tool.h"
#include "msg/msg_zukan_search.h"

#include "zknsearch_main.h"
#include "zknsearch_bmp.h"


//============================================================================================
//	定数定義
//============================================================================================

// フォントカラー
#define FCOL_WP01WN		( PRINTSYS_LSB_Make(14,15,0) )	// フォントカラー：両用０１白抜
#define FCOL_WP15BN		( PRINTSYS_LSB_Make(1,2,0) )		// フォントカラー：両用１５黒抜
#define FCOL_WP15WN		( PRINTSYS_LSB_Make(15,2,0) )		// フォントカラー：両用１５白抜

#define FCOL_MP04			( PRINTSYS_LSB_Make(1,2,15) )		// フォントカラー：検索ウィンドウ用

/*
#define FCOL_SP00WN		( PRINTSYS_LSB_Make(14,15,0) )	// フォントカラー：サブ００白抜
#define FCOL_SP00BN		( PRINTSYS_LSB_Make(12,13,0) )	// フォントカラー：サブ００黒抜
#define FCOL_SP15WN		( PRINTSYS_LSB_Make(15,2,0) )		// フォントカラー：サブ１５白抜
*/

// BMPWIN関連
// 上画面ラベル１
#define	BMPWIN_LABEL1_FRM		( GFL_BG_FRAME3_S )
#define	BMPWIN_LABEL1_PX		( 1 )
#define	BMPWIN_LABEL1_PY		( 0 )
#define	BMPWIN_LABEL1_SX		( 30 )
#define	BMPWIN_LABEL1_SY		( 3 )
#define	BMPWIN_LABEL1_PAL		( ZKNSEARCHMAIN_SBG_PAL_FONT )
// 上画面ラベル２
#define	BMPWIN_LABEL2_FRM		( GFL_BG_FRAME3_S )
#define	BMPWIN_LABEL2_PX		( 1 )
#define	BMPWIN_LABEL2_PY		( 3 )
#define	BMPWIN_LABEL2_SX		( 30 )
#define	BMPWIN_LABEL2_SY		( 3 )
#define	BMPWIN_LABEL2_PAL		( ZKNSEARCHMAIN_SBG_PAL_FONT )

// メイン画面「ならび」
#define	BMPWIN_LABEL_ROW_FRM	( GFL_BG_FRAME3_M )
#define	BMPWIN_LABEL_ROW_PX		( 3 )
#define	BMPWIN_LABEL_ROW_PY		( 0 )
#define	BMPWIN_LABEL_ROW_SX		( 10 )
#define	BMPWIN_LABEL_ROW_SY		( 3 )
#define	BMPWIN_LABEL_ROW_PAL	( ZKNSEARCHMAIN_MBG_PAL_FONT )

// メイン画面「なまえ」
#define	BMPWIN_LABEL_NAME_FRM		( GFL_BG_FRAME3_M )
#define	BMPWIN_LABEL_NAME_PX		( 3 )
#define	BMPWIN_LABEL_NAME_PY		( 3 )
#define	BMPWIN_LABEL_NAME_SX		( 10 )
#define	BMPWIN_LABEL_NAME_SY		( 3 )
#define	BMPWIN_LABEL_NAME_PAL		( ZKNSEARCHMAIN_MBG_PAL_FONT )

// メイン画面「タイプ」
#define	BMPWIN_LABEL_TYPE_FRM		( GFL_BG_FRAME3_M )
#define	BMPWIN_LABEL_TYPE_PX		( 3 )
#define	BMPWIN_LABEL_TYPE_PY		( 6 )
#define	BMPWIN_LABEL_TYPE_SX		( 10 )
#define	BMPWIN_LABEL_TYPE_SY		( 3 )
#define	BMPWIN_LABEL_TYPE_PAL		( ZKNSEARCHMAIN_MBG_PAL_FONT )

// メイン画面「いろ」
#define	BMPWIN_LABEL_COLOR_FRM	( GFL_BG_FRAME3_M )
#define	BMPWIN_LABEL_COLOR_PX		( 3 )
#define	BMPWIN_LABEL_COLOR_PY		( 9 )
#define	BMPWIN_LABEL_COLOR_SX		( 10 )
#define	BMPWIN_LABEL_COLOR_SY		( 3 )
#define	BMPWIN_LABEL_COLOR_PAL	( ZKNSEARCHMAIN_MBG_PAL_FONT )

// メイン画面「かたち」
#define	BMPWIN_LABEL_FORM_FRM		( GFL_BG_FRAME3_M )
#define	BMPWIN_LABEL_FORM_PX		( 3 )
#define	BMPWIN_LABEL_FORM_PY		( 13 )
#define	BMPWIN_LABEL_FORM_SX		( 10 )
#define	BMPWIN_LABEL_FORM_SY		( 3 )
#define	BMPWIN_LABEL_FORM_PAL		( ZKNSEARCHMAIN_MBG_PAL_FONT )

// メイン画面　並び
#define	BMPWIN_ITEM_ROW_FRM		( GFL_BG_FRAME3_M )
#define	BMPWIN_ITEM_ROW_PX		( 19 )
#define	BMPWIN_ITEM_ROW_PY		( 0 )
#define	BMPWIN_ITEM_ROW_SX		( 10 )
#define	BMPWIN_ITEM_ROW_SY		( 3 )
#define	BMPWIN_ITEM_ROW_PAL		( ZKNSEARCHMAIN_MBG_PAL_FONT )

// メイン画面　名前
#define	BMPWIN_ITEM_NAME_FRM	( GFL_BG_FRAME3_M )
#define	BMPWIN_ITEM_NAME_PX		( 23 )
#define	BMPWIN_ITEM_NAME_PY		( 3 )
#define	BMPWIN_ITEM_NAME_SX		( 2 )
#define	BMPWIN_ITEM_NAME_SY		( 3 )
#define	BMPWIN_ITEM_NAME_PAL	( ZKNSEARCHMAIN_MBG_PAL_FONT )

// メイン画面　タイプ
#define	BMPWIN_ITEM_TYPE_FRM	( GFL_BG_FRAME3_M )
#define	BMPWIN_ITEM_TYPE_PX		( 17 )
#define	BMPWIN_ITEM_TYPE_PY		( 6 )
#define	BMPWIN_ITEM_TYPE_SX		( 14 )
#define	BMPWIN_ITEM_TYPE_SY		( 3 )
#define	BMPWIN_ITEM_TYPE_PAL	( ZKNSEARCHMAIN_MBG_PAL_FONT )

// メイン画面　色
#define	BMPWIN_ITEM_COLOR_FRM		( GFL_BG_FRAME3_M )
#define	BMPWIN_ITEM_COLOR_PX		( 19 )
#define	BMPWIN_ITEM_COLOR_PY		( 9 )
#define	BMPWIN_ITEM_COLOR_SX		( 10 )
#define	BMPWIN_ITEM_COLOR_SY		( 3 )
#define	BMPWIN_ITEM_COLOR_PAL		( ZKNSEARCHMAIN_MBG_PAL_FONT )

// メイン画面「リセット」
#define	BMPWIN_RESET_FRM	( GFL_BG_FRAME3_M )
#define	BMPWIN_RESET_PX		( 3 )
#define	BMPWIN_RESET_PY		( 18 )
#define	BMPWIN_RESET_SX		( 10 )
#define	BMPWIN_RESET_SY		( 3 )
#define	BMPWIN_RESET_PAL	( ZKNSEARCHMAIN_MBG_PAL_FONT )

// メイン画面「スタート」
#define	BMPWIN_START_FRM	( GFL_BG_FRAME3_M )
#define	BMPWIN_START_PX		( 19 )
#define	BMPWIN_START_PY		( 18 )
#define	BMPWIN_START_SX		( 10 )
#define	BMPWIN_START_SY		( 3 )
#define	BMPWIN_START_PAL	( ZKNSEARCHMAIN_MBG_PAL_FONT )

// リスト画面ラベル
#define	BMPWIN_LIST_LABEL_FRM		( GFL_BG_FRAME3_M )
#define	BMPWIN_LIST_LABEL_PX		( 3 )
#define	BMPWIN_LIST_LABEL_PY		( 8 )
#define	BMPWIN_LIST_LABEL_SX		( 10 )
#define	BMPWIN_LIST_LABEL_SY		( 2 )
#define	BMPWIN_LIST_LABEL_PAL		( ZKNSEARCHMAIN_MBG_PAL_FONT )

// リスト画面　項目１
#define	BMPWIN_LIST_ITEM1_FRM		( GFL_BG_FRAME3_M )
#define	BMPWIN_LIST_ITEM1_PX		( 2 )
#define	BMPWIN_LIST_ITEM1_PY		( 11 )
#define	BMPWIN_LIST_ITEM1_SX		( 12 )
#define	BMPWIN_LIST_ITEM1_SY		( 2 )
#define	BMPWIN_LIST_ITEM1_PAL		( ZKNSEARCHMAIN_MBG_PAL_FONT )

// リスト画面　項目２
#define	BMPWIN_LIST_ITEM2_FRM		( GFL_BG_FRAME3_M )
#define	BMPWIN_LIST_ITEM2_PX		( 2 )
#define	BMPWIN_LIST_ITEM2_PY		( 14 )
#define	BMPWIN_LIST_ITEM2_SX		( 12 )
#define	BMPWIN_LIST_ITEM2_SY		( 2 )
#define	BMPWIN_LIST_ITEM2_PAL		( ZKNSEARCHMAIN_MBG_PAL_FONT )

// 検索結果など
#define	BMPWIN_SEARCH_WIN_FRM		( GFL_BG_FRAME3_M )
#define	BMPWIN_SEARCH_WIN_PX		( 1 )
#define	BMPWIN_SEARCH_WIN_PY		( 19 )
#define	BMPWIN_SEARCH_WIN_SX		( 30 )
#define	BMPWIN_SEARCH_WIN_SY		( 2 )
#define	BMPWIN_SEARCH_WIN_PAL		( 4 )


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void WriteDefaultStr( ZKNSEARCHMAIN_WORK * wk );


//============================================================================================
//	グローバル
//============================================================================================

static const u8	BmpWinData[][6] =
{
	{	// 上画面ラベル１
		BMPWIN_LABEL1_FRM, BMPWIN_LABEL1_PX, BMPWIN_LABEL1_PY,
		BMPWIN_LABEL1_SX, BMPWIN_LABEL1_SY, BMPWIN_LABEL1_PAL
	},
	{	// 上画面ラベル２
		BMPWIN_LABEL2_FRM, BMPWIN_LABEL2_PX, BMPWIN_LABEL2_PY,
		BMPWIN_LABEL2_SX, BMPWIN_LABEL2_SY,	BMPWIN_LABEL2_PAL
	},
	{	// メイン画面「ならび」
		BMPWIN_LABEL_ROW_FRM,	BMPWIN_LABEL_ROW_PX, BMPWIN_LABEL_ROW_PY,
		BMPWIN_LABEL_ROW_SX, BMPWIN_LABEL_ROW_SY, BMPWIN_LABEL_ROW_PAL
	},
	{	// メイン画面「なまえ」
		BMPWIN_LABEL_NAME_FRM, BMPWIN_LABEL_NAME_PX, BMPWIN_LABEL_NAME_PY,
		BMPWIN_LABEL_NAME_SX, BMPWIN_LABEL_NAME_SY, BMPWIN_LABEL_NAME_PAL
	},
	{	// メイン画面「タイプ」
		BMPWIN_LABEL_TYPE_FRM, BMPWIN_LABEL_TYPE_PX, BMPWIN_LABEL_TYPE_PY,
		BMPWIN_LABEL_TYPE_SX, BMPWIN_LABEL_TYPE_SY, BMPWIN_LABEL_TYPE_PAL
	},
	{	// メイン画面「いろ」
		BMPWIN_LABEL_COLOR_FRM, BMPWIN_LABEL_COLOR_PX, BMPWIN_LABEL_COLOR_PY,
		BMPWIN_LABEL_COLOR_SX, BMPWIN_LABEL_COLOR_SY, BMPWIN_LABEL_COLOR_PAL
	},
	{	// メイン画面「かたち」
		BMPWIN_LABEL_FORM_FRM, BMPWIN_LABEL_FORM_PX, BMPWIN_LABEL_FORM_PY,
		BMPWIN_LABEL_FORM_SX, BMPWIN_LABEL_FORM_SY, BMPWIN_LABEL_FORM_PAL
	},
	{	// メイン画面　並び
		BMPWIN_ITEM_ROW_FRM, BMPWIN_ITEM_ROW_PX, BMPWIN_ITEM_ROW_PY,
		BMPWIN_ITEM_ROW_SX, BMPWIN_ITEM_ROW_SY, BMPWIN_ITEM_ROW_PAL
	},
	{	// メイン画面　名前
		BMPWIN_ITEM_NAME_FRM, BMPWIN_ITEM_NAME_PX, BMPWIN_ITEM_NAME_PY,
		BMPWIN_ITEM_NAME_SX, BMPWIN_ITEM_NAME_SY, BMPWIN_ITEM_NAME_PAL
	},
	{	// メイン画面　タイプ
		BMPWIN_ITEM_TYPE_FRM, BMPWIN_ITEM_TYPE_PX, BMPWIN_ITEM_TYPE_PY,
		BMPWIN_ITEM_TYPE_SX, BMPWIN_ITEM_TYPE_SY, BMPWIN_ITEM_TYPE_PAL
	},
	{	// メイン画面　色
		BMPWIN_ITEM_COLOR_FRM, BMPWIN_ITEM_COLOR_PX, BMPWIN_ITEM_COLOR_PY,
		BMPWIN_ITEM_COLOR_SX, BMPWIN_ITEM_COLOR_SY, BMPWIN_ITEM_COLOR_PAL
	},
	{	// メイン画面「スタート」
		BMPWIN_START_FRM, BMPWIN_START_PX, BMPWIN_START_PY,
		BMPWIN_START_SX, BMPWIN_START_SY, BMPWIN_START_PAL
	},
	{	// メイン画面「リセット」
		BMPWIN_RESET_FRM, BMPWIN_RESET_PX, BMPWIN_RESET_PY,
		BMPWIN_RESET_SX, BMPWIN_RESET_SY, BMPWIN_RESET_PAL
	},
	{	// リスト画面ラベル
		BMPWIN_LIST_LABEL_FRM, BMPWIN_LIST_LABEL_PX, BMPWIN_LIST_LABEL_PY,
		BMPWIN_LIST_LABEL_SX, BMPWIN_LIST_LABEL_SY, BMPWIN_LIST_LABEL_PAL
	},
	{	// リスト画面　項目１
		BMPWIN_LIST_ITEM1_FRM, BMPWIN_LIST_ITEM1_PX, BMPWIN_LIST_ITEM1_PY,
		BMPWIN_LIST_ITEM1_SX, BMPWIN_LIST_ITEM1_SY, BMPWIN_LIST_ITEM1_PAL
	},
	{	// リスト画面　項目２
		BMPWIN_LIST_ITEM2_FRM, BMPWIN_LIST_ITEM2_PX, BMPWIN_LIST_ITEM2_PY,
		BMPWIN_LIST_ITEM2_SX, BMPWIN_LIST_ITEM2_SY, BMPWIN_LIST_ITEM2_PAL
	},
	{	// 検索結果など
		BMPWIN_SEARCH_WIN_FRM, BMPWIN_SEARCH_WIN_PX, BMPWIN_SEARCH_WIN_PY,
		BMPWIN_SEARCH_WIN_SX, BMPWIN_SEARCH_WIN_SY, BMPWIN_SEARCH_WIN_PAL
	},
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ関連初期化
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHBMP_Init( ZKNSEARCHMAIN_WORK * wk )
{
	u32	i;

	GFL_BMPWIN_Init( HEAPID_ZUKAN_SEARCH );

	for( i=0; i<ZKNSEARCHBMP_WINIDX_MAX; i++ ){
		const u8 * dat = BmpWinData[i];
		wk->win[i].win = GFL_BMPWIN_Create(
											dat[0], dat[1], dat[2], dat[3], dat[4], dat[5], GFL_BMP_CHRAREA_GET_B );
	}

	WriteDefaultStr( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ関連削除
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHBMP_Exit( ZKNSEARCHMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<ZKNSEARCHBMP_WINIDX_MAX; i++ ){
		GFL_BMPWIN_Delete( wk->win[i].win );
	}

	GFL_BMPWIN_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プリントメイン
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHBMP_PrintUtilTrans( ZKNSEARCHMAIN_WORK * wk )
{
	u32	i;

	PRINTSYS_QUE_Main( wk->que );
	for( i=0; i<ZKNSEARCHBMP_WINIDX_MAX; i++ ){
		PRINT_UTIL_Trans( &wk->win[i], wk->que );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示（固定文字列）
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		winID		BMPWIN ID
 * @param		msgID		文字列ＩＤ
 * @param		x				表示基準Ｘ座標
 * @param		y				表示基準Ｙ座標
 * @param		col			カラー
 * @param		mode		表示モード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void StrPrint(
							ZKNSEARCHMAIN_WORK * wk, u32 winID,
							u32 msgID, u32 x, u32 y, PRINTSYS_LSB col, u32 mode )
{
	STRBUF * str = GFL_MSG_CreateString( wk->mman, msgID );

	PRINTTOOL_PrintColor( &wk->win[winID], wk->que, x, y, str, wk->font, col, mode );

	GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BMPWINスクリーン転送
 *
 * @param		util		PRINT_UTIL
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PrintScreenTrans( PRINT_UTIL * util )
{
	GFL_BMPWIN_MakeScreen( util->win );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(util->win) );
}


static void WriteDefaultStr( ZKNSEARCHMAIN_WORK * wk )
{
	// タイトル
	if( wk->dat->sort->mode == ZKNCOMM_LIST_SORT_MODE_ZENKOKU ){
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_LABEL1, ZKN_SEARCH_23, 0, 4, FCOL_WP15BN, PRINTTOOL_MODE_LEFT );
	}else{
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_LABEL1, ZKN_SEARCH_22, 0, 4, FCOL_WP15BN, PRINTTOOL_MODE_LEFT );
	}

	//「ならび」
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_MAIN_LABEL_ROW, ZKN_SEARCH_10,
		BMPWIN_LABEL_ROW_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	//「なまえ」
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_MAIN_LABEL_NAME, ZKN_SEARCH_12,
		BMPWIN_LABEL_NAME_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	//「タイプ」
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_MAIN_LABEL_TYPE, ZKN_SEARCH_13,
		BMPWIN_LABEL_TYPE_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	//「いろ」
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_MAIN_LABEL_COLOR, ZKN_SEARCH_14,
		BMPWIN_LABEL_COLOR_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	//「かたち」
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_MAIN_LABEL_FORM, ZKN_SEARCH_17,
		BMPWIN_LABEL_FORM_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );

	// リセット
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_MAIN_RESET, ZKN_SEARCH_24,
		BMPWIN_RESET_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	// スタート
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_MAIN_START, ZKN_SEARCH_25,
		BMPWIN_START_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
}

void ZKNSEARCHBMP_PutMainPage( ZKNSEARCHMAIN_WORK * wk )
{
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LABEL1] );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_LABEL_ROW] );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_LABEL_NAME] );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_LABEL_TYPE] );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_LABEL_COLOR] );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_LABEL_FORM] );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_RESET] );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_START] );

	// 上画面の情報をクリア
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LABEL2].win), 0 );
	GFL_BMPWIN_TransVramCharacter( wk->win[ZKNSEARCHBMP_WINIDX_LABEL2].win );

	// 並び
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_ROW].win ), 0 );
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_MAIN_ITEM_ROW,
		ZKN_SEARCH_SORT_01+wk->dat->sort->row,
		BMPWIN_ITEM_ROW_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_ROW] );

	// 名前
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_NAME].win ), 0 );
	if( wk->dat->sort->name == ZKNCOMM_LIST_SORT_NONE ){
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_MAIN_ITEM_NAME, ZKN_SEARCH_INITIAL_45,
			BMPWIN_ITEM_NAME_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}else{
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_MAIN_ITEM_NAME,
			ZKN_SEARCH_INITIAL_01+wk->dat->sort->name,
			BMPWIN_ITEM_NAME_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_NAME] );

	// タイプ
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_TYPE].win ), 0 );
	if( wk->dat->sort->type1 == ZKNCOMM_LIST_SORT_NONE ){
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_MAIN_ITEM_TYPE,
			ZKN_SEARCH_TYPE_18,
			BMPWIN_ITEM_TYPE_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}else if( wk->dat->sort->type2 == ZKNCOMM_LIST_SORT_NONE ){
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_MAIN_ITEM_TYPE,
			ZKN_SEARCH_TYPE_01+wk->dat->sort->type1,
			BMPWIN_ITEM_TYPE_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}else{
		STRBUF * str;
		u8	sx, px;
		//「／」
		px = BMPWIN_ITEM_TYPE_SX*8/2;
		str = GFL_MSG_CreateString( wk->mman, ZKN_SEARCH_TYPE_19 );
		PRINTTOOL_PrintColor(
			&wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_TYPE], wk->que,
			px, 4, str, wk->font, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
		sx = PRINTSYS_GetStrWidth( str, wk->font, 0 );
		GFL_STR_DeleteBuffer( str );
		// タイプ１
		px -= ( sx / 2 );
		str = GFL_MSG_CreateString( wk->mman, ZKN_SEARCH_TYPE_01+wk->dat->sort->type1 );
		PRINTTOOL_PrintColor(
			&wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_TYPE], wk->que,
			px, 4, str, wk->font, FCOL_WP15WN, PRINTTOOL_MODE_RIGHT );
		GFL_STR_DeleteBuffer( str );
		// タイプ２
		px += sx;
		str = GFL_MSG_CreateString( wk->mman, ZKN_SEARCH_TYPE_01+wk->dat->sort->type2 );
		PRINTTOOL_PrintColor(
			&wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_TYPE], wk->que,
			px, 4, str, wk->font, FCOL_WP15WN, PRINTTOOL_MODE_LEFT );
		GFL_STR_DeleteBuffer( str );
	}
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_TYPE] );

	// 色
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_COLOR].win ), 0 );
	if( wk->dat->sort->color == ZKNCOMM_LIST_SORT_NONE ){
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_MAIN_ITEM_COLOR,
			ZKN_SEARCH_COLOR_10,
			BMPWIN_ITEM_COLOR_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}else{
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_MAIN_ITEM_COLOR,
			ZKN_SEARCH_COLOR_00+wk->dat->sort->color,
			BMPWIN_ITEM_COLOR_SX*8/2, 4, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_ITEM_COLOR] );
}

void ZKNSEARCHBMP_PutRowPage( ZKNSEARCHMAIN_WORK * wk )
{
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LABEL1] );

	// 項目名
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LIST_LABEL].win), 0 );
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_LIST_LABEL, ZKN_SEARCH_10,
		BMPWIN_LIST_LABEL_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LIST_LABEL] );

	ZKNSEARCHBMP_PutRowItem( wk );
}

void ZKNSEARCHBMP_PutRowItem( ZKNSEARCHMAIN_WORK * wk )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LIST_ITEM1].win), 0 );
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_LIST_ITEM1, ZKN_SEARCH_SORT_01+wk->dat->sort->row,
		BMPWIN_LIST_ITEM1_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LIST_ITEM1] );
}

void ZKNSEARCHBMP_PutNamePage( ZKNSEARCHMAIN_WORK * wk )
{
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LABEL1] );

	// 上画面の情報
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_LABEL2, ZKN_SEARCH_INFO_05, 0, 4, FCOL_WP15BN, PRINTTOOL_MODE_LEFT );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LABEL2] );

	// 項目名
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LIST_LABEL].win), 0 );
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_LIST_LABEL, ZKN_SEARCH_12,
		BMPWIN_LIST_LABEL_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LIST_LABEL] );

	ZKNSEARCHBMP_PutNameItem( wk );
}

void ZKNSEARCHBMP_PutNameItem( ZKNSEARCHMAIN_WORK * wk )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LIST_ITEM1].win), 0 );
	if( wk->dat->sort->name != ZKNCOMM_LIST_SORT_NONE ){
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_LIST_ITEM1, ZKN_SEARCH_INITIAL_01+wk->dat->sort->name,
			BMPWIN_LIST_ITEM1_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}else{
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_LIST_ITEM1, ZKN_SEARCH_INITIAL_45,
			BMPWIN_LIST_ITEM1_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LIST_ITEM1] );
}

void ZKNSEARCHBMP_PutTypePage( ZKNSEARCHMAIN_WORK * wk )
{
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LABEL1] );

	// 上画面の情報
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_LABEL2, ZKN_SEARCH_INFO_06, 0, 4, FCOL_WP15BN, PRINTTOOL_MODE_LEFT );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LABEL2] );

	// 項目名
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LIST_LABEL].win), 0 );
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_LIST_LABEL, ZKN_SEARCH_13,
		BMPWIN_LIST_LABEL_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LIST_LABEL] );

	ZKNSEARCHBMP_PutTypeItem( wk );
}

void ZKNSEARCHBMP_PutTypeItem( ZKNSEARCHMAIN_WORK * wk )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LIST_ITEM1].win), 0 );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LIST_ITEM2].win), 0 );

	if( wk->dat->sort->type1 != ZKNCOMM_LIST_SORT_NONE ){
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_LIST_ITEM1, ZKN_SEARCH_TYPE_01+wk->dat->sort->type1,
			BMPWIN_LIST_ITEM1_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}else{
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_LIST_ITEM1, ZKN_SEARCH_TYPE_18,
			BMPWIN_LIST_ITEM1_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}

	if( wk->dat->sort->type2 != ZKNCOMM_LIST_SORT_NONE ){
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_LIST_ITEM2, ZKN_SEARCH_TYPE_01+wk->dat->sort->type2,
			BMPWIN_LIST_ITEM2_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}else{
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_LIST_ITEM2, ZKN_SEARCH_TYPE_18,
			BMPWIN_LIST_ITEM2_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}

	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LIST_ITEM1] );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LIST_ITEM2] );
}

void ZKNSEARCHBMP_PutColorPage( ZKNSEARCHMAIN_WORK * wk )
{
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LABEL1] );

	// 上画面の情報
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_LABEL2, ZKN_SEARCH_INFO_10, 0, 4, FCOL_WP15BN, PRINTTOOL_MODE_LEFT );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LABEL2] );

	// 項目名
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LIST_LABEL].win), 0 );
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_LIST_LABEL, ZKN_SEARCH_14,
		BMPWIN_LIST_LABEL_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LIST_LABEL] );

	ZKNSEARCHBMP_PutColorItem( wk );
}

void ZKNSEARCHBMP_PutColorItem( ZKNSEARCHMAIN_WORK * wk )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LIST_ITEM1].win), 0 );
	if( wk->dat->sort->color != ZKNCOMM_LIST_SORT_NONE ){
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_LIST_ITEM1, ZKN_SEARCH_COLOR_00+wk->dat->sort->color,
			BMPWIN_LIST_ITEM1_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}else{
		StrPrint(
			wk, ZKNSEARCHBMP_WINIDX_LIST_ITEM1, ZKN_SEARCH_COLOR_10,
			BMPWIN_LIST_ITEM1_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	}
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LIST_ITEM1] );
}

void ZKNSEARCHBMP_PutFormPage( ZKNSEARCHMAIN_WORK * wk )
{
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LABEL1] );

	// 上画面の情報
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_LABEL2, ZKN_SEARCH_INFO_09, 0, 4, FCOL_WP15BN, PRINTTOOL_MODE_LEFT );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LABEL2] );

	// 項目名
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_LIST_LABEL].win), 0 );
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_LIST_LABEL, ZKN_SEARCH_17,
		BMPWIN_LIST_LABEL_SX*8/2, 0, FCOL_WP15WN, PRINTTOOL_MODE_CENTER );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_LIST_LABEL] );
}


void ZKNSEARCHBMP_PutListItem( ZKNSEARCHMAIN_WORK * wk, PRINT_UTIL * util, STRBUF * str )
{
	PRINT_QUE * que = FRAMELIST_GetPrintQue( wk->lwk );
	PRINTTOOL_PrintColor(
		util, que, GFL_BMPWIN_GetSizeX(util->win)*8/2, 4, str, wk->font, FCOL_WP01WN, PRINTTOOL_MODE_CENTER );
}

void ZKNSEARCHBMP_PutFormListItem( ZKNSEARCHMAIN_WORK * wk, PRINT_UTIL * util )
{
//	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(util->win), 1 );
	GFL_BMPWIN_TransVramCharacter( util->win );
}

static void PrintSearchWin( ZKNSEARCHMAIN_WORK * wk, u32 msg )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNSEARCHBMP_WINIDX_SEARCH_WIN].win), 15 );
	StrPrint(
		wk, ZKNSEARCHBMP_WINIDX_SEARCH_WIN, msg,
		BMPWIN_SEARCH_WIN_SX*8/2, 0, FCOL_MP04, PRINTTOOL_MODE_CENTER );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_SEARCH_WIN] );
}

void ZKNSEARCHBMP_SearchStart( ZKNSEARCHMAIN_WORK * wk )
{
	PrintSearchWin( wk, ZKN_SEARCH_INFO_13 );
}

void ZKNSEARCHBMP_SearchComp( ZKNSEARCHMAIN_WORK * wk )
{
	PrintSearchWin( wk, ZKN_SEARCH_INFO_11 );
}

void ZKNSEARCHBMP_SearchError( ZKNSEARCHMAIN_WORK * wk )
{
	PrintSearchWin( wk, ZKN_SEARCH_INFO_12 );
}

void ZKNSEARCHBMP_PutResetStart( ZKNSEARCHMAIN_WORK * wk )
{
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_RESET] );
	PrintScreenTrans( &wk->win[ZKNSEARCHBMP_WINIDX_MAIN_START] );
}

