//============================================================================================
/**
 * @file		zknlist_bmp.c
 * @brief		図鑑リスト画面 ＢＭＰ関連
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZKNLISTBMP
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "ui/print_tool.h"
#include "msg/msg_zukan_list.h"

#include "zknlist_main.h"
#include "zknlist_bmp.h"
#include "zukan_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================

// フォントカラー
#define FCOL_WP01WN		( PRINTSYS_LSB_Make(14,15,0) )	// フォントカラー：両用０１白抜
#define FCOL_SP00WN		( PRINTSYS_LSB_Make(14,15,0) )	// フォントカラー：サブ００白抜
#define FCOL_SP00BN		( PRINTSYS_LSB_Make(12,13,0) )	// フォントカラー：サブ００黒抜
#define FCOL_SP15WN		( PRINTSYS_LSB_Make(15,2,0) )		// フォントカラー：サブ１５白抜


// BMPWIN関連
// ポケモン名
#define	BMPWIN_NAME_M_FRM		( GFL_BG_FRAME2_M )
#define	BMPWIN_NAME_S_FRM		( GFL_BG_FRAME2_S )
#define	BMPWIN_NAME_PX			( 3 )
#define	BMPWIN_NAME_PY			( 0 )
#define	BMPWIN_NAME_SX			( 11 )
#define	BMPWIN_NAME_SY			( 3 )
#define	BMPWIN_NAME_M_PAL		( 1 )
#define	BMPWIN_NAME_S_PAL		( 1 )

// タイトル
#define	BMPWIN_TITLE_FRM		( GFL_BG_FRAME1_S )
#define	BMPWIN_TITLE_PX			( 1 )
#define	BMPWIN_TITLE_PY			( 0 )
#define	BMPWIN_TITLE_SX			( 12 )
#define	BMPWIN_TITLE_SY			( 3 )
#define	BMPWIN_TITLE_PAL		( 0 )

// 見つけた数
#define	BMPWIN_SEENUM_FRM		( GFL_BG_FRAME1_S )
#define	BMPWIN_SEENUM_PX		( 2 )
#define	BMPWIN_SEENUM_PY		( 3 )
#define	BMPWIN_SEENUM_SX		( 13 )
#define	BMPWIN_SEENUM_SY		( 3 )
#define	BMPWIN_SEENUM_PAL		( 0 )

// 捕まえた数
#define	BMPWIN_GETNUM_FRM		( GFL_BG_FRAME1_S )
#define	BMPWIN_GETNUM_PX		( 16 )
#define	BMPWIN_GETNUM_PY		( 3 )
#define	BMPWIN_GETNUM_SX		( 15 )
#define	BMPWIN_GETNUM_SY		( 3 )
#define	BMPWIN_GETNUM_PAL		( 0 )


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void CreateNameBmp( ZKNLISTMAIN_WORK * wk );
static void DeleteNameBmp( ZKNLISTMAIN_WORK * wk );
static void PutTitleStr( ZKNLISTMAIN_WORK * wk );


//============================================================================================
//	グローバル
//============================================================================================

static const u8	BoxBmpWinData[][6] =
{
	{	// ポケモン名（メイン画面）
		BMPWIN_NAME_M_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_M_PAL
	},
	{
		BMPWIN_NAME_M_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_M_PAL
	},
	{
		BMPWIN_NAME_M_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_M_PAL
	},
	{
		BMPWIN_NAME_M_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_M_PAL
	},
	{
		BMPWIN_NAME_M_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_M_PAL
	},
	{
		BMPWIN_NAME_M_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_M_PAL
	},
	{
		BMPWIN_NAME_M_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_M_PAL
	},
	{
		BMPWIN_NAME_M_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_M_PAL
	},

	{	// ポケモン名（サブ画面）
		BMPWIN_NAME_S_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_S_PAL
	},
	{
		BMPWIN_NAME_S_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_S_PAL
	},
	{
		BMPWIN_NAME_S_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_S_PAL
	},
	{
		BMPWIN_NAME_S_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_S_PAL
	},
	{
		BMPWIN_NAME_S_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_S_PAL
	},
	{
		BMPWIN_NAME_S_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_S_PAL
	},
	{
		BMPWIN_NAME_S_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_S_PAL
	},
	{
		BMPWIN_NAME_S_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_S_PAL
	},

	{	// タイトル
		BMPWIN_TITLE_FRM, BMPWIN_TITLE_PX, BMPWIN_TITLE_PY,
		BMPWIN_TITLE_SX, BMPWIN_TITLE_SY, BMPWIN_TITLE_PAL
	},
	{	// 見つけた数
		BMPWIN_SEENUM_FRM, BMPWIN_SEENUM_PX, BMPWIN_SEENUM_PY,
		BMPWIN_SEENUM_SX, BMPWIN_SEENUM_SY, BMPWIN_SEENUM_PAL
	},
	{	// 捕まえた数
		BMPWIN_GETNUM_FRM, BMPWIN_GETNUM_PX, BMPWIN_GETNUM_PY,
		BMPWIN_GETNUM_SX, BMPWIN_GETNUM_SY, BMPWIN_GETNUM_PAL
	}
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ関連初期化
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTBMP_Init( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	GFL_BMPWIN_Init( HEAPID_ZUKAN_LIST );

	for( i=0; i<ZKNLISTBMP_WINIDX_MAX; i++ ){
		const u8 * dat = BoxBmpWinData[i];
		wk->win[i].win = GFL_BMPWIN_Create(
											dat[0], dat[1], dat[2], dat[3], dat[4], dat[5], GFL_BMP_CHRAREA_GET_B );
	}

	CreateNameBmp( wk );

	PutTitleStr( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ関連削除
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTBMP_Exit( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	DeleteNameBmp( wk );

	for( i=0; i<ZKNLISTBMP_WINIDX_MAX; i++ ){
		GFL_BMPWIN_Delete( wk->win[i].win );
	}

	GFL_BMPWIN_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プリントメイン
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTBMP_PrintUtilTrans( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	PRINTSYS_QUE_Main( wk->que );
	for( i=0; i<ZKNLISTBMP_WINIDX_MAX; i++ ){
		PRINT_UTIL_Trans( &wk->win[i], wk->que );
	}
}


static void CreateNameBmp( ZKNLISTMAIN_WORK * wk )
{
	ARCHANDLE * ah;

	NNSG2dCharacterData * chr;
	void * chrBuff;
	u8 * chrData;

	NNSG2dScreenData * scrn;
	void * scrnBuff;
	u16 * scrnData;

	u8	px, py, sx, sy;

	u16	i, j;

	wk->nameBmp = GFL_HEAP_AllocMemory( HEAPID_ZUKAN_LIST, BMPWIN_NAME_SX * BMPWIN_NAME_SY * 0x20 );

	sx = BMPWIN_NAME_SX;
	sy = BMPWIN_NAME_SY;
	px = BMPWIN_NAME_PX;
	py = BMPWIN_NAME_PY;

	ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_LIST );

	scrnBuff = GFL_ARCHDL_UTIL_LoadScreen(
							ah, NARC_zukan_gra_list_listframe_NSCR, FALSE, &scrn, HEAPID_ZUKAN_LIST_L );
	scrnData = (u16 *)scrn->rawData;

	chrBuff = GFL_ARCHDL_UTIL_LoadBGCharacter(
							ah, NARC_zukan_gra_list_listframe_bgd_NCGR, FALSE, &chr, HEAPID_ZUKAN_LIST_L );
	chrData = (u8 *)chr->pRawData;

	GFL_ARC_CloseDataHandle( ah );

	for( i=0; i<sy; i++ ){
		for( j=0; j<sx; j ++ ){
			GFL_STD_MemCopy32( &chrData[(scrnData[(py+i)*16+px+j]&0x3ff)*0x20], &wk->nameBmp[(i*sx+j)*0x20], 0x20 );
		}
	}

	GFL_HEAP_FreeMemory( chrBuff );
	GFL_HEAP_FreeMemory( scrnBuff );
}

static void DeleteNameBmp( ZKNLISTMAIN_WORK * wk )
{
	GFL_HEAP_FreeMemory( wk->nameBmp );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示（固定文字列）
 *
 * @param		wk			図鑑リストワーク
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
							ZKNLISTMAIN_WORK * wk, u32 winID,
							u32 msgID, u32 x, u32 y, PRINTSYS_LSB col, u32 mode )
{
	STRBUF * str = GFL_MSG_CreateString( wk->mman, msgID );

	PRINTTOOL_PrintColor( &wk->win[winID], wk->que, x, y, str, wk->font, col, mode );

	GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示（可変文字列）
 *
 * @param		wk			図鑑リストワーク
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
static void ExStrPrint(
							ZKNLISTMAIN_WORK * wk, u32 winID,
							u32 msgID, u32 x, u32 y, PRINTSYS_LSB col, u32 mode )
{
	STRBUF * str = GFL_MSG_CreateString( wk->mman, msgID );

	WORDSET_ExpandStr( wk->wset, wk->exp, str );

	PRINTTOOL_PrintColor( &wk->win[winID], wk->que, x, y, wk->exp, wk->font, col, mode );

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



static void PutSubDispStr( ZKNLISTMAIN_WORK * wk, u32 idx )
{
	ARCHANDLE * ah;

	NNSG2dCharacterData * chr;
	void * chrBuff;
	u8 * chrData;

	NNSG2dScreenData * scrn;
	void * scrnBuff;
	u16 * scrnData;

	u8 * bmpBuff;
	u8	px, py, sx, sy;

	u16	i, j;

	sx = GFL_BMPWIN_GetScreenSizeX( wk->win[idx].win );
	sy = GFL_BMPWIN_GetScreenSizeY( wk->win[idx].win );
	px = GFL_BMPWIN_GetPosX( wk->win[idx].win );
	py = GFL_BMPWIN_GetPosY( wk->win[idx].win );

	bmpBuff = GFL_BMP_GetCharacterAdrs( GFL_BMPWIN_GetBmp(wk->win[idx].win) );

	ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_LIST );

	scrnBuff = GFL_ARCHDL_UTIL_LoadScreen(
							ah, NARC_zukan_gra_list_listtitle_bgu_NSCR, FALSE, &scrn, HEAPID_ZUKAN_LIST_L );
	scrnData = (u16 *)scrn->rawData;

	chrBuff = GFL_ARCHDL_UTIL_LoadBGCharacter(
							ah, NARC_zukan_gra_list_listbase_bgu_NCGR, FALSE, &chr, HEAPID_ZUKAN_LIST_L );
	chrData = (u8 *)chr->pRawData;

	GFL_ARC_CloseDataHandle( ah );

	for( i=0; i<sy; i++ ){
		for( j=0; j<sx; j ++ ){
			GFL_STD_MemCopy32( &chrData[(scrnData[(py+i)*32+px+j]&0x3ff)*0x20], &bmpBuff[(i*sx+j)*0x20], 0x20 );
		}
	}

	GFL_HEAP_FreeMemory( chrBuff );
	GFL_HEAP_FreeMemory( scrnBuff );
}

static void PutTitleStr( ZKNLISTMAIN_WORK * wk )
{
//	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[ZKNLISTBMP_WINIDX_TITLE].win), 0 );
	PutSubDispStr( wk, ZKNLISTBMP_WINIDX_TITLE );
	StrPrint( wk, ZKNLISTBMP_WINIDX_TITLE, str_title_00, 0, 4, FCOL_SP00WN, PRINTTOOL_MODE_LEFT );
	PrintScreenTrans( &wk->win[ZKNLISTBMP_WINIDX_TITLE] );
}

void ZKNLISTBMP_PutPokeEntryStr( ZKNLISTMAIN_WORK * wk )
{
	PutSubDispStr( wk, ZKNLISTBMP_WINIDX_SEENUM );
	PutSubDispStr( wk, ZKNLISTBMP_WINIDX_GETNUM );

	StrPrint( wk, ZKNLISTBMP_WINIDX_SEENUM, str_see, 9*8, 4, FCOL_SP00BN, PRINTTOOL_MODE_RIGHT );
	WORDSET_RegisterNumber(
		wk->wset, 0, 256, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	ExStrPrint( wk, ZKNLISTBMP_WINIDX_SEENUM, str_see_get_num, 13*8, 4, FCOL_SP00BN, PRINTTOOL_MODE_RIGHT );

	StrPrint( wk, ZKNLISTBMP_WINIDX_GETNUM, str_get, 11*8, 4, FCOL_SP00BN, PRINTTOOL_MODE_RIGHT );
	WORDSET_RegisterNumber(
		wk->wset, 0, 256, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	ExStrPrint( wk, ZKNLISTBMP_WINIDX_GETNUM, str_see_get_num, 15*8, 4, FCOL_SP00BN, PRINTTOOL_MODE_RIGHT );

	PrintScreenTrans( &wk->win[ZKNLISTBMP_WINIDX_SEENUM] );
	PrintScreenTrans( &wk->win[ZKNLISTBMP_WINIDX_GETNUM] );
}



void ZKNLISTBMP_PutPokeList( ZKNLISTMAIN_WORK * wk, u32 winIdx, s32 listPos )
{
	u8 * bmpBuff;

	if( listPos < 0 ){ return; }
	
	bmpBuff = GFL_BMP_GetCharacterAdrs( GFL_BMPWIN_GetBmp(wk->win[winIdx].win) );

	GFL_STD_MemCopy32( wk->nameBmp, bmpBuff, BMPWIN_NAME_SX * BMPWIN_NAME_SY * 0x20 );

	// 図鑑番号
	WORDSET_RegisterNumber(
		wk->wset, 0, ZKNLISTMAIN_GetListMons( wk->list,listPos), 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	ExStrPrint( wk, winIdx, str_poek_num, 0, 4, FCOL_WP01WN, PRINTTOOL_MODE_LEFT );
	// 名前
	PRINTTOOL_PrintColor(
		&wk->win[winIdx], wk->que, 28, 4,
		ZKNLISTMAIN_GetListStr(wk->list,listPos), wk->font, FCOL_WP01WN, PRINTTOOL_MODE_LEFT );
}

