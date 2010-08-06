//============================================================================================
/**
 * @file		dpc_bmp.c
 * @brief		殿堂入り確認画面 ＢＭＰ関連
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 *
 *	モジュール名：DPCBMP
 */
//============================================================================================
#include <gflib.h>

#include "message.naix"
#include "ui/print_tool.h"
#include "msg/msg_pc_dendou.h"

#include "dpc_main.h"
#include "dpc_bmp.h"


//============================================================================================
//	定数定義
//============================================================================================

// タイトル
#define	BMPWIN_TITLE_FRM	( GFL_BG_FRAME3_M )
#define	BMPWIN_TITLE_PX		( 1 )
#define	BMPWIN_TITLE_PY		( 0 )
#define	BMPWIN_TITLE_SX		( 30 )
#define	BMPWIN_TITLE_SY		( 2 )
#define	BMPWIN_TITLE_PAL	( 3 )

// 情報
#define	BMPWIN_INFO_FRM		( GFL_BG_FRAME1_S )
#define	BMPWIN_INFO_PX		( 4 )
#define	BMPWIN_INFO_PY		( 7 )
#define	BMPWIN_INFO_SX		( 24 )
#define	BMPWIN_INFO_SY		( 15 )
#define	BMPWIN_INFO_PAL		( 1 )

// 上画面表示座標
#define	NAME_PX				( 0 )
#define	NAME_PY				( 0 )
#define	SEX_PX				( 8*8 )
#define	SEX_PY				( 0 )
#define	LEVEL_PX			( 10*8 )
#define	LEVEL_PY			( 0 )
#define	NICKNAME_PX		( 0 )
#define	NICKNAME_PY		( 3*8 )
#define	OYA_PX				( 0 )
#define	OYA_PY				( 6*8 )
#define	WAZA_PX				( 0 )
#define	WAZA_PY				( 11*8 )
#define	WAZA_SX				( 12*8 )
#define	WAZA_SY				( 2*8 )

// フォントカラー
#define FCOL_MP03WN		( PRINTSYS_LSB_Make(1,2,0) )		// フォントカラー：下用０３白抜
#define FCOL_MP03RN		( PRINTSYS_LSB_Make(5,6,0) )		// フォントカラー：下用０３赤抜
#define FCOL_SP01WN		( PRINTSYS_LSB_Make(1,2,0) )		// フォントカラー：上用０１白抜
#define FCOL_SP01RN		( PRINTSYS_LSB_Make(5,6,0) )		// フォントカラー：上用０１赤抜
#define FCOL_SP01BLN	( PRINTSYS_LSB_Make(3,4,0) )		// フォントカラー：上用０１青抜

#define FCOL_FNTOAM		( PRINTSYS_LSB_Make(6,8,0) )		// フォントカラー：フォントOAM


//--------------------------------------------------------------------------------------------
/**
 * @brief		BMP初期化
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCBMP_Init( DPCMAIN_WORK * wk )
{
	GFL_BMPWIN_Init( HEAPID_DENDOU_PC );

	wk->win[DPCBMP_WINID_TITLE].win = GFL_BMPWIN_Create(
																			BMPWIN_TITLE_FRM,
																			BMPWIN_TITLE_PX, BMPWIN_TITLE_PY,
																			BMPWIN_TITLE_SX, BMPWIN_TITLE_SY,
																			BMPWIN_TITLE_PAL, GFL_BMP_CHRAREA_GET_B );
	wk->win[DPCBMP_WINID_INFO].win = GFL_BMPWIN_Create(
																			BMPWIN_INFO_FRM,
																			BMPWIN_INFO_PX, BMPWIN_INFO_PY,
																			BMPWIN_INFO_SX, BMPWIN_INFO_SY,
																			BMPWIN_INFO_PAL, GFL_BMP_CHRAREA_GET_B );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BMP解放
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCBMP_Exit( DPCMAIN_WORK * wk )
{
	GFL_BMPWIN_Delete( wk->win[DPCBMP_WINID_TITLE].win );
	GFL_BMPWIN_Delete( wk->win[DPCBMP_WINID_INFO].win );
	GFL_BMPWIN_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列転送
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCBMP_PrintUtilTrans( DPCMAIN_WORK * wk )
{
	u32	i;

	PRINTSYS_QUE_Main( wk->que );
	for( i=0; i<DPCBMP_WINID_MAX; i++ ){
		PRINT_UTIL_Trans( &wk->win[i], wk->que );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		スクリーン反映
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイトル表示
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCBMP_PutTitle( DPCMAIN_WORK * wk )
{
	DPC_PARTY_DATA * pt;
	STRBUF * str;

	pt = &wk->party[wk->page];

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[DPCBMP_WINID_TITLE].win), 0 );

	// 初回クリア
	if( wk->dat->callMode == DENDOUPC_CALL_CLEAR ){
		str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_01_01 );
		PRINTTOOL_PrintColor(
			&wk->win[DPCBMP_WINID_TITLE], wk->que, 0, 0, str, wk->font, FCOL_MP03WN, PRINTTOOL_MODE_LEFT );
		GFL_STR_DeleteBuffer( str );
	// 殿堂入り
	}else{
		str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_01_02 );
#ifdef	BUGFIX_AF_GFBTS2013_20100806
		WORDSET_RegisterNumber( wk->wset, 0, pt->recNo, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
#else		// BUGFIX_AF_GFBTS2013_20100806
		WORDSET_RegisterNumber( wk->wset, 0, pt->recNo, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
#endif	// BUGFIX_AF_GFBTS2013_20100806
		WORDSET_ExpandStr( wk->wset, wk->exp, str );
		PRINTTOOL_PrintColor(
			&wk->win[DPCBMP_WINID_TITLE], wk->que, 0, 0, wk->exp, wk->font, FCOL_MP03WN, PRINTTOOL_MODE_LEFT );
		GFL_STR_DeleteBuffer( str );
	}

	str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_01_03 );
	WORDSET_RegisterNumber( wk->wset, 0, pt->date.year, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_RegisterNumber( wk->wset, 1, pt->date.month, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_RegisterNumber( wk->wset, 2, pt->date.day, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINTTOOL_PrintColor(
		&wk->win[DPCBMP_WINID_TITLE], wk->que,
		GFL_BMPWIN_GetScreenSizeX(wk->win[DPCBMP_WINID_TITLE].win)*8,
		0,
		wk->exp, wk->font, FCOL_MP03RN, PRINTTOOL_MODE_RIGHT );
	GFL_STR_DeleteBuffer( str );

	PrintScreenTrans( &wk->win[DPCBMP_WINID_TITLE] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ表示
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCBMP_PutPage( DPCMAIN_WORK * wk )
{
	STRBUF * str;
	u8	px, sx;

	if( wk->pageMax == 1 ){ return; }

	GFL_BMP_Clear( wk->fobmp, 0 );

	//「/」
	str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_01_04 );
	sx  = PRINTSYS_GetStrWidth( str, wk->font, 0 );
	px  = ( DPCBMP_PAGE_SX * 8 - sx ) / 2;
	PRINTSYS_PrintColor(
		wk->fobmp,
		px, 4,
		str, wk->font, FCOL_FNTOAM );
	GFL_STR_DeleteBuffer( str );

	str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_01_05 );

	// 現在のページ
	WORDSET_RegisterNumber( wk->wset, 0, wk->page+1, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINTSYS_PrintColor(
		wk->fobmp,
		px - PRINTSYS_GetStrWidth( wk->exp, wk->font, 0 ), 4,
		wk->exp, wk->font, FCOL_FNTOAM );

	// ページ最大
	WORDSET_RegisterNumber( wk->wset, 0, wk->pageMax, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINTSYS_PrintColor(
		wk->fobmp,
		px+sx , 4,
		wk->exp, wk->font, FCOL_FNTOAM );

	GFL_STR_DeleteBuffer( str );

	BmpOam_ActorBmpTrans( wk->foact );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン情報表示
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCBMP_PutInfo( DPCMAIN_WORK * wk )
{
	DENDOU_POKEMON_DATA * poke;
	STRBUF * str;
	u32	i;

	poke = &wk->party[wk->page].dat[wk->pokePos];

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[DPCBMP_WINID_INFO].win), 0 );

	// ポケモン名
	str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_02_01 );
	WORDSET_RegisterPokeMonsNameNo( wk->wset, 0, poke->monsno );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINTTOOL_PrintColor(
		&wk->win[DPCBMP_WINID_INFO], wk->que,
		NAME_PX, NAME_PY, wk->exp, wk->font, FCOL_SP01WN, PRINTTOOL_MODE_LEFT );
	GFL_STR_DeleteBuffer( str );

	// 性別
	if( poke->sex == PTL_SEX_MALE ){
		if( poke->monsno != MONSNO_NIDORAN_M ){
			str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_02_02 );
			PRINTTOOL_PrintColor(
				&wk->win[DPCBMP_WINID_INFO], wk->que,
				SEX_PX, SEX_PY, str, wk->font, FCOL_SP01BLN, PRINTTOOL_MODE_LEFT );
			GFL_STR_DeleteBuffer( str );
		}
	}else if( poke->sex == PTL_SEX_FEMALE ){
		if( poke->monsno != MONSNO_NIDORAN_F ){
			str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_02_03 );
			PRINTTOOL_PrintColor(
				&wk->win[DPCBMP_WINID_INFO], wk->que,
				SEX_PX, SEX_PY, str, wk->font, FCOL_SP01RN, PRINTTOOL_MODE_LEFT );
			GFL_STR_DeleteBuffer( str );
		}
	}

	// レベル
	{
		u32	npx;

		str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_02_08 );
		npx = PRINTSYS_GetStrWidth( str, wk->nfnt, 0 );
		PRINTTOOL_PrintColor(
			&wk->win[DPCBMP_WINID_INFO], wk->que,
			LEVEL_PX, LEVEL_PY+4, str, wk->nfnt, FCOL_SP01WN, PRINTTOOL_MODE_LEFT );
		GFL_STR_DeleteBuffer( str );

		str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_02_04 );
		WORDSET_RegisterNumber( wk->wset, 0, poke->level, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
		WORDSET_ExpandStr( wk->wset, wk->exp, str );
		PRINTTOOL_PrintColor(
			&wk->win[DPCBMP_WINID_INFO], wk->que,
			LEVEL_PX+npx, LEVEL_PY, wk->exp, wk->font, FCOL_SP01WN, PRINTTOOL_MODE_LEFT );
		GFL_STR_DeleteBuffer( str );
	}

	// ニックネーム
	str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_02_05 );
	WORDSET_RegisterWord( wk->wset, 0, poke->nickname, 0, TRUE, PM_LANG );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINTTOOL_PrintColor(
		&wk->win[DPCBMP_WINID_INFO], wk->que,
		NICKNAME_PX, NICKNAME_PY, wk->exp, wk->font, FCOL_SP01WN, PRINTTOOL_MODE_LEFT );
	GFL_STR_DeleteBuffer( str );

	// 親名
	str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_02_07 );
	WORDSET_RegisterWord( wk->wset, 0, poke->oyaname, 0, TRUE, PM_LANG );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINTTOOL_PrintColor(
		&wk->win[DPCBMP_WINID_INFO], wk->que,
		OYA_PX, OYA_PY, wk->exp, wk->font, FCOL_SP01WN, PRINTTOOL_MODE_LEFT );
	GFL_STR_DeleteBuffer( str );

	// 技名
	for( i=0; i<4; i++ ){
		str = GFL_MSG_CreateString( wk->mman, mes_pc_dendou_02_09+i );
		WORDSET_RegisterWazaName( wk->wset, 0, poke->waza[i] );
		WORDSET_ExpandStr( wk->wset, wk->exp, str );
		PRINTTOOL_PrintColor(
			&wk->win[DPCBMP_WINID_INFO], wk->que,
			WAZA_PX+WAZA_SX*(i&1), WAZA_PY+WAZA_SY*(i/2), wk->exp, wk->font, FCOL_SP01WN, PRINTTOOL_MODE_LEFT );
		GFL_STR_DeleteBuffer( str );
	}

	PrintScreenTrans( &wk->win[DPCBMP_WINID_INFO] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン情報クリア
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCBMP_ClearInfo( DPCMAIN_WORK * wk )
{
	GFL_BMPWIN_ClearScreen( wk->win[DPCBMP_WINID_INFO].win );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(wk->win[DPCBMP_WINID_INFO].win) );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[DPCBMP_WINID_INFO].win), 0 );
	GFL_BMPWIN_TransVramCharacter( wk->win[DPCBMP_WINID_INFO].win );
}
