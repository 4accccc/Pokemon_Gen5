//============================================================================================
/**
 * @file		dpc_obj.c
 * @brief		殿堂入り確認画面 ＯＢＪ関連
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 *
 *	モジュール名：DPCOBJ
 */
//============================================================================================
#include <gflib.h>

#include "system/poke2dgra.h"
#include "app/app_menu_common.h"
#include "ui/touchbar.h"

#include "dpc_main.h"
#include "dpc_obj.h"


//============================================================================================
//	定数定義
//============================================================================================
#define	RES_NONE	( 0xffffffff )		// リソースなし

#define	PALNUM_POKEGRA	( 0 )
#define	PALSIZ_POKEGRA	( 12 )
#define	PALNUM_TB				( PALNUM_POKEGRA + PALSIZ_POKEGRA )
#define	PALSIZ_TB				( APP_COMMON_BARICON_PLT_NUM )

// セルアクターデータ
typedef struct {
	GFL_CLWK_DATA	dat;

	u32	chrRes;
	u32	palRes;
	u32	celRes;

	u16	pal;
	u16	disp;

}DPC_CLWK_DATA;


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void InitClact(void);
static void InitResource( DPCMAIN_WORK * wk );
static void ExitResource( DPCMAIN_WORK * wk );
static void AddClact( DPCMAIN_WORK * wk );
static void DelClact( DPCMAIN_WORK * wk, u32 idx );
static void DelClactAll( DPCMAIN_WORK * wk );


//============================================================================================
//	グローバル
//============================================================================================

// セルアクターデータ
static const DPC_CLWK_DATA ClactParamTbl[] =
{
	{	// 左ボタン
		{ 8, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_CURSOR_LEFT, 0, 0 },
		DPCOBJ_CHRRES_TB, DPCOBJ_PALRES_TB, DPCOBJ_CELRES_TB,
		0, CLSYS_DRAW_MAIN
	},
	{	// 右ボタン
		{ 88, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_CURSOR_RIGHT, 0, 0 },
		DPCOBJ_CHRRES_TB, DPCOBJ_PALRES_TB, DPCOBJ_CELRES_TB,
		0, CLSYS_DRAW_MAIN
	},
	{	// Ｘボタン
		{ TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_EXIT, 0, 0 },
		DPCOBJ_CHRRES_TB, DPCOBJ_PALRES_TB, DPCOBJ_CELRES_TB,
		0, CLSYS_DRAW_MAIN
	},
	{	// 戻るボタン
		{ TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_RETURN, 0, 0 },
		DPCOBJ_CHRRES_TB, DPCOBJ_PALRES_TB, DPCOBJ_CELRES_TB,
		0, CLSYS_DRAW_MAIN
	},
};

static const DPC_CLWK_DATA PokeClactParamTbl[] =
{
	{
		{ 128, 132, 0, 0, 2 },
		DPCOBJ_CHRRES_POKE01, DPCOBJ_PALRES_POKE01, DPCOBJ_CELRES_POKE01,
		0, CLSYS_DRAW_MAIN
	},
	{
		{ 40, 124, 0, 1, 2 },
		DPCOBJ_CHRRES_POKE02, DPCOBJ_PALRES_POKE02, DPCOBJ_CELRES_POKE02,
		0, CLSYS_DRAW_MAIN
	},
	{
		{ 224, 124, 0, 2, 2 },
		DPCOBJ_CHRRES_POKE03, DPCOBJ_PALRES_POKE03, DPCOBJ_CELRES_POKE03,
		0, CLSYS_DRAW_MAIN
	},
	{
		{ 128, 44, 0, 3, 2 },
		DPCOBJ_CHRRES_POKE04, DPCOBJ_PALRES_POKE04, DPCOBJ_CELRES_POKE04,
		0, CLSYS_DRAW_MAIN
	},
	{
		{ 40, 52, 0, 4, 2 },
		DPCOBJ_CHRRES_POKE05, DPCOBJ_PALRES_POKE05, DPCOBJ_CELRES_POKE05,
		0, CLSYS_DRAW_MAIN
	},
	{
		{ 224, 52, 0, 5, 2 },
		DPCOBJ_CHRRES_POKE06, DPCOBJ_PALRES_POKE06, DPCOBJ_CELRES_POKE06,
		0, CLSYS_DRAW_MAIN
	},
};




void DPCOBJ_Init( DPCMAIN_WORK * wk )
{
	InitClact();
	InitResource( wk );
	AddClact( wk );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// MAIN DISP OBJ ON
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// SUB DISP OBJ ON
}

void DPCOBJ_Exit( DPCMAIN_WORK * wk )
{
	ExitResource( wk );
	DelClactAll( wk );
	GFL_CLACT_SYS_Delete();
}

void DPCOBJ_AnmMain( DPCMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<DPCOBJ_ID_MAX; i++ ){
		if( wk->clwk[i] == NULL ){ continue; }
		if( GFL_CLACT_WK_GetAutoAnmFlag( wk->clwk[i] ) == TRUE ){ continue; }
		GFL_CLACT_WK_AddAnmFrame( wk->clwk[i], FX32_ONE );
	}
	GFL_CLACT_SYS_Main();
}

void DPCOBJ_SetVanish( DPCMAIN_WORK * wk, GFL_CLWK * clwk, BOOL flg )
{
	if( clwk != NULL ){
		GFL_CLACT_WK_SetDrawEnable( clwk, flg );
	}
}





static void InitClact(void)
{
	const GFL_CLSYS_INIT init = {
		0, 0,									// メイン　サーフェースの左上座標
		0, 512,								// サブ　サーフェースの左上座標
		4,										// メイン画面OAM管理開始位置	4の倍数
		124,									// メイン画面OAM管理数				4の倍数
		4,										// サブ画面OAM管理開始位置		4の倍数
		124,									// サブ画面OAM管理数					4の倍数
		0,										// セルVram転送管理数

		DPCOBJ_CHRRES_MAX,		// 登録できるキャラデータ数
		DPCOBJ_PALRES_MAX,		// 登録できるパレットデータ数
		DPCOBJ_CELRES_MAX,		// 登録できるセルアニメパターン数
		0,										// 登録できるマルチセルアニメパターン数（※現状未対応）

	  16,										// メイン CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
	  16										// サブ CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
	};
	GFL_CLACT_SYS_Create( &init, DPCMAIN_GetVramBankData(), HEAPID_DENDOU_PC );
}

static void InitResource( DPCMAIN_WORK * wk )
{
	ARCHANDLE * ah;
	u32 * chr;
	u32 * pal;
	u32 * cel;
	u32	i;

	// 初期化
	for( i=0; i<DPCOBJ_CHRRES_MAX; i++ ){
		wk->chrRes[i] = RES_NONE;
	}
	for( i=0; i<DPCOBJ_PALRES_MAX; i++ ){
		wk->palRes[i] = RES_NONE;
	}
	for( i=0; i<DPCOBJ_CELRES_MAX; i++ ){
		wk->celRes[i] = RES_NONE;
	}

	// タッチバー
	ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_DENDOU_PC );
	chr = &wk->chrRes[ DPCOBJ_CHRRES_TB ];
	pal = &wk->palRes[ DPCOBJ_PALRES_TB ];
	cel = &wk->celRes[ DPCOBJ_CELRES_TB ];
	*chr = GFL_CLGRP_CGR_Register(
					ah, APP_COMMON_GetBarIconCharArcIdx(),
					FALSE, CLSYS_DRAW_MAIN, HEAPID_DENDOU_PC );
  *pal = GFL_CLGRP_PLTT_Register(
					ah, APP_COMMON_GetBarIconPltArcIdx(),
					CLSYS_DRAW_MAIN, PALNUM_TB*0x20, HEAPID_DENDOU_PC );
	*cel = GFL_CLGRP_CELLANIM_Register(
					ah,
					APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_128K),
					APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_128K),
					HEAPID_DENDOU_PC );
	GFL_ARC_CloseDataHandle( ah );
}

static void ExitResource( DPCMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<DPCOBJ_CHRRES_MAX; i++ ){
		if( wk->chrRes[i] != RES_NONE ){
			GFL_CLGRP_CGR_Release( wk->chrRes[i] );
		}
	}
	for( i=0; i<DPCOBJ_PALRES_MAX; i++ ){
		if( wk->palRes[i] != RES_NONE ){
	    GFL_CLGRP_PLTT_Release( wk->palRes[i] );
		}
	}
	for( i=0; i<DPCOBJ_CELRES_MAX; i++ ){
		if( wk->celRes[i] != RES_NONE ){
	    GFL_CLGRP_CELLANIM_Release( wk->celRes[i] );
		}
	}
}

static GFL_CLWK * CleateClact( DPCMAIN_WORK * wk, const DPC_CLWK_DATA * prm )
{
	return GFL_CLACT_WK_Create(
					wk->clunit,
					wk->chrRes[prm->chrRes],
					wk->palRes[prm->palRes],
					wk->celRes[prm->celRes],
					&prm->dat, prm->disp, HEAPID_DENDOU_PC );
}

static void AddClact( DPCMAIN_WORK * wk )
{
	u32	i;

	wk->clunit = GFL_CLACT_UNIT_Create( DPCOBJ_ID_MAX, 0, HEAPID_DENDOU_PC );

	// 初期化
	for( i=0; i<DPCOBJ_ID_MAX; i++ ){
		wk->clwk[i] = NULL;
	}

	for( i=DPCOBJ_ID_ARROW_L; i<=DPCOBJ_ID_RETURN; i++ ){
		wk->clwk[i] = CleateClact( wk, &ClactParamTbl[i-DPCOBJ_ID_ARROW_L] );
	}
}

static void DelClact( DPCMAIN_WORK * wk, u32 idx )
{
	if( wk->clwk[idx] != NULL ){
		GFL_CLACT_WK_Remove( wk->clwk[idx] );
		wk->clwk[idx] = NULL;
	}
}

static void DelClactAll( DPCMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<DPCOBJ_ID_MAX; i++ ){
		DelClact( wk, i );
	}
	GFL_CLACT_UNIT_Delete( wk->clunit );
}





void DPCOBJ_AddPoke( DPCMAIN_WORK * wk )
{
	ARCHANDLE * ah;
	DPC_PARTY_DATA * pt;
	GFL_CLWK ** clwk;
	u32	pal;
	u32	i;

	// 全ての表示をOFF
	for( i=DPCOBJ_ID_POKE01; i<=DPCOBJ_ID_POKE16; i++ ){
		DPCOBJ_SetVanish( wk, wk->clwk[i], FALSE );
	}

	pt = &wk->party[wk->page];

	ah  = POKE2DGRA_OpenHandle( HEAPID_DENDOU_PC_L );

	for( i=0; i<pt->pokeMax; i++ ){
		DPC_CLWK_DATA	dat;
		BOOL	rare;

		dat  = PokeClactParamTbl[i];
		rare = POKETOOL_CheckRare( pt->dat[i].idNumber, pt->dat[i].personalRandom );

		if( wk->pokeSwap == 1 ){
			dat.chrRes += 6;
			dat.palRes += 6;
			dat.celRes += 6;
			pal = PALNUM_POKEGRA + i + 6 * 0x20;
			clwk = &wk->clwk[DPCOBJ_ID_POKE11+i];
		}else{
			pal = PALNUM_POKEGRA + i * 0x20;
			clwk = &wk->clwk[DPCOBJ_ID_POKE01+i];
		}

		wk->chrRes[dat.chrRes] = POKE2DGRA_OBJ_CGR_Register(
															ah, pt->dat[i].monsno, pt->dat[i].formNumber, pt->dat[i].sex, rare, POKEGRA_DIR_FRONT,
															FALSE, pt->dat[i].personalRandom, CLSYS_DRAW_MAIN, HEAPID_DENDOU_PC );
		wk->palRes[dat.palRes] = POKE2DGRA_OBJ_PLTT_Register(
															ah, pt->dat[i].monsno, pt->dat[i].formNumber, pt->dat[i].sex, rare, POKEGRA_DIR_FRONT,
															FALSE, CLSYS_DRAW_MAIN, pal, HEAPID_DENDOU_PC );
		wk->celRes[dat.celRes] = POKE2DGRA_OBJ_CELLANM_Register(
															pt->dat[i].monsno, pt->dat[i].formNumber, pt->dat[i].sex, rare, POKEGRA_DIR_FRONT,
															FALSE, APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, HEAPID_DENDOU_PC );
		*clwk = CleateClact( wk, &dat );
//		DPCOBJ_SetVanish( wk, *clwk, TRUE );
	}

	GFL_ARC_CloseDataHandle( ah );


	// 表示してたＯＢＪを削除
	if( wk->pokeSwap == 1 ){
		for( i=DPCOBJ_ID_POKE01; i<=DPCOBJ_ID_POKE06; i++ ){
			DelClact( wk, i );
		}
	}else{
		for( i=DPCOBJ_ID_POKE11; i<=DPCOBJ_ID_POKE16; i++ ){
			DelClact( wk, i );
		}
	}

	wk->pokeSwap ^= 1;
}
