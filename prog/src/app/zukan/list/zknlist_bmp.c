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

#include "zknlist_main.h"
#include "zknlist_bmp.h"


//============================================================================================
//	定数定義
//============================================================================================

// BMPWIN関連
// ポケモン名
#define	BMPWIN_NAME_M_FRM		( GFL_BG_FRAME2_M )
#define	BMPWIN_NAME_S_FRM		( GFL_BG_FRAME2_S )
#define	BMPWIN_NAME_PX			( 3 )
#define	BMPWIN_NAME_PY			( 0 )
#define	BMPWIN_NAME_SX			( 11 )
#define	BMPWIN_NAME_SY			( 3 )
#define	BMPWIN_NAME_M_PAL		( 0 )
#define	BMPWIN_NAME_S_PAL		( 0 )

// タイトル
#define	BMPWIN_TITLE_FRM		( GFL_BG_FRAME1_S )
#define	BMPWIN_TITLE_PX			( 1 )
#define	BMPWIN_TITLE_PY			( 0 )
#define	BMPWIN_TITLE_SX			( 12 )
#define	BMPWIN_TITLE_SY			( 3 )
#define	BMPWIN_TITLE_PAL		( 0 )

// 見つけた数
#define	BMPWIN_SEENUM_FRM		( GFL_BG_FRAME1_S )
#define	BMPWIN_SEENUM_PX		( 1 )
#define	BMPWIN_SEENUM_PY		( 3 )
#define	BMPWIN_SEENUM_SX		( 15 )
#define	BMPWIN_SEENUM_SY		( 3 )
#define	BMPWIN_SEENUM_PAL		( 0 )

// 捕まえた数
#define	BMPWIN_GETNUM_FRM		( GFL_BG_FRAME1_S )
#define	BMPWIN_GETNUM_PX		( 17 )
#define	BMPWIN_GETNUM_PY		( 3 )
#define	BMPWIN_GETNUM_SX		( 15 )
#define	BMPWIN_GETNUM_SY		( 3 )
#define	BMPWIN_GETNUM_PAL		( 0 )


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

	for( i=0; i<ZKNLISTBMP_WINIDX_MAX; i++ ){
		GFL_BMPWIN_Delete( wk->win[i].win );
	}

	GFL_BMPWIN_Exit();
}
