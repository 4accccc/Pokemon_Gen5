//============================================================================================
/**
 * @file		zknlist_bgwfrm.c
 * @brief		図鑑リスト画面 ＢＧウィンドウフレーム関連
 * @author	Hiroyuki Nakamura
 * @date		09.12.16
 *
 *	モジュール名：ZKNLISTBGWFRM
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"

#include "zknlist_main.h"
#include "zknlist_bgwfrm.h"
#include "zukan_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	BGWFRM_NAME_SX				( 16 )
#define	BGWFRM_NAME_SY				( 3 )
#define	BGWFRM_NAME_SCRN_SIZ	( BGWFRM_NAME_SX * BGWFRM_NAME_SY * 2 )


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void CreateNameFrame( ZKNLISTMAIN_WORK * wk );
static void DeleteNameFrame( ZKNLISTMAIN_WORK * wk );



void ZKNLISTBGWFRM_Init( ZKNLISTMAIN_WORK * wk )
{
	u32	i;

	wk->wfrm = BGWINFRM_Create(
							BGWINFRM_TRANS_VBLANK, ZKNLISTBGWFRM_IDX_MAX, HEAPID_ZUKAN_LIST );

	for( i=0; i<ZKNLISTBGWFRM_IDX_MAX; i++ ){
		BGWINFRM_Add(
			wk->wfrm, i, GFL_BMPWIN_GetFrame(wk->win[i].win), BGWFRM_NAME_SX, BGWFRM_NAME_SY );
//		BGWINFRM_BmpWinOn( wk->wfrm, i, wk->win[i].win );
	}

	CreateNameFrame( wk );
}

void ZKNLISTBGWFRM_Exit( ZKNLISTMAIN_WORK * wk )
{
	DeleteNameFrame( wk );
	BGWINFRM_Exit( wk->wfrm );
}

static void CreateNameFrame( ZKNLISTMAIN_WORK * wk )
{
	NNSG2dScreenData * scrn;
	void * buf;
	u16 * dat;

	wk->nameBG[0] = GFL_HEAP_AllocMemory( HEAPID_ZUKAN_LIST, BGWFRM_NAME_SCRN_SIZ );
	wk->nameBG[1] = GFL_HEAP_AllocMemory( HEAPID_ZUKAN_LIST, BGWFRM_NAME_SCRN_SIZ );

	buf = GFL_ARC_UTIL_LoadScreen(
					ARCID_ZUKAN_GRA,
					NARC_zukan_gra_list_listframe_NSCR,
					FALSE, &scrn, HEAPID_ZUKAN_LIST_L );

	dat = (u16 *)scrn->rawData;
	GFL_STD_MemCopy16( dat, wk->nameBG[0], BGWFRM_NAME_SCRN_SIZ );
	dat = &dat[BGWFRM_NAME_SX*BGWFRM_NAME_SY];
	GFL_STD_MemCopy16( dat, wk->nameBG[1], BGWFRM_NAME_SCRN_SIZ );

	GFL_HEAP_FreeMemory( buf );
}

static void DeleteNameFrame( ZKNLISTMAIN_WORK * wk )
{
	GFL_HEAP_FreeMemory( wk->nameBG[1] );
	GFL_HEAP_FreeMemory( wk->nameBG[0] );
}

void ZKNLISTBGWFRM_SetNameFrame( ZKNLISTMAIN_WORK * wk, u32 idx, u32 num )
{
	BGWINFRM_FrameSet( wk->wfrm, idx, wk->nameBG[num] );
}

void ZKNLISTBGWFRM_PutPokeList( ZKNLISTMAIN_WORK * wk, u32 idx, s32 listPos )
{
	if( listPos < 0 ){
		u16 * buf;
		u16	sx, sy;
		buf = BGWINFRM_FrameBufGet( wk->wfrm, idx );
		BGWINFRM_SizeGet( wk->wfrm, idx, &sx, &sy );
		GFL_STD_MemClear16( buf, sx * sy * 2 );
		return;
	}

	if( ZKNLISTMAIN_GetListInfo( wk->list, listPos ) == 2 ){
		ZKNLISTBGWFRM_SetNameFrame( wk, idx, 0 );
	}else{
		ZKNLISTBGWFRM_SetNameFrame( wk, idx, 1 );
	}
	BGWINFRM_BmpWinOn( wk->wfrm, idx, wk->win[idx].win );
}


void ZKNLISTBGWFRM_PutScrollList( ZKNLISTMAIN_WORK * wk, u32 idx, u32 mv )
{
	u16 * buf;
	int	scroll;
	u16	sx, sy;
	u8	frm;
	s8	px, py;
	u8	i;

	buf = BGWINFRM_FrameBufGet( wk->wfrm, idx );
	frm = BGWINFRM_BGFrameGet( wk->wfrm, idx );
//	BGWINFRM_PosGet( wk->wfrm, idx, &px, &py );
	BGWINFRM_SizeGet( wk->wfrm, idx, &sx, &sy );

	scroll = GFL_BG_GetScrollY( frm ) % 256;
	py = scroll / 8;
	px = ZKNLISTMAIN_LIST_PX;

	if( mv == ZKNLISTBGWFRM_LISTPUT_UP ){
		py -= 3;
	}else{
		py += 21;
	}
	for( i=0; i<sy; i++ ){
		if( py < 0 ){
			py += 32;
		}else if( py >= 32 ){
			py -= 32;
		}
		GFL_BG_WriteScreenExpand( frm, px, py, sx, 1, buf, 0, i, sx, sy );
		py++;
	}
	GFL_BG_LoadScreenV_Req( frm );
}

void ZKNLISTBGWFRM_PutScrollListSub( ZKNLISTMAIN_WORK * wk, u32 idx, u32 mv )
{
	u16 * buf;
	int	scroll;
	u16	sx, sy;
	u8	frm;
	s8	px, py;
	u8	i;

	buf = BGWINFRM_FrameBufGet( wk->wfrm, idx );
	frm = BGWINFRM_BGFrameGet( wk->wfrm, idx );
//	BGWINFRM_PosGet( wk->wfrm, idx, &px, &py );
	BGWINFRM_SizeGet( wk->wfrm, idx, &sx, &sy );

	scroll = GFL_BG_GetScrollY( frm ) % 256;
	py = scroll / 8;
	px = ZKNLISTMAIN_LIST_PX;

	if( mv == ZKNLISTBGWFRM_LISTPUT_DOWN ){
		py += 24;
	}

	for( i=0; i<sy; i++ ){
		if( py < 0 ){
			py += 32;
		}else if( py >= 32 ){
			py -= 32;
		}
		GFL_BG_WriteScreenExpand( frm, px, py, sx, 1, buf, 0, i, sx, sy );
		py++;
	}
	GFL_BG_LoadScreenV_Req( frm );
}

