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
#include <calctool.h>

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

#define	FOAM_PAGE_PX		( 5*8 )
#define	FOAM_PAGE_PY		( 21*8 )
#define	FOAM_PAGE_SX		( DPCBMP_PAGE_SX )
#define	FOAM_PAGE_SY		( 3 )

// フォントOAMで使用するOBJ管理数
// 使用するBMPWINのキャラサイズ分が最大なので、それだけあれば足りる。
#define	FNTOAM_CHR_MAX		( FOAM_PAGE_SX*FOAM_PAGE_SY )

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
static void FontOamInit( DPCMAIN_WORK * wk );
static void FontOamExit( DPCMAIN_WORK * wk );


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
	FontOamInit( wk );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// MAIN DISP OBJ ON
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// SUB DISP OBJ ON
}

void DPCOBJ_Exit( DPCMAIN_WORK * wk )
{
	FontOamExit( wk );
	DelClactAll( wk );
	ExitResource( wk );
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

void DPCOBJ_SetVanish( DPCMAIN_WORK * wk, u32 id, BOOL flg )
{
	if( wk->clwk[id] != NULL ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[id], flg );
	}
}

void DPCOBJ_SetAutoAnm( DPCMAIN_WORK * wk, u32 id, u32 anm )
{
	GFL_CLACT_WK_SetAnmFrame( wk->clwk[id], 0 );
	GFL_CLACT_WK_SetAnmSeq( wk->clwk[id], anm );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[id], TRUE );
}

BOOL DPCOBJ_CheckAnm( DPCMAIN_WORK * wk, u32 id )
{
	return GFL_CLACT_WK_CheckAnmActive( wk->clwk[id] );
}

void DPCOBJ_SetPos( DPCMAIN_WORK * wk, u32 id, s16 x, s16 y )
{
	GFL_CLACTPOS	pos;

	pos.x = x;
	pos.y = y;
	GFL_CLACT_WK_SetPos( wk->clwk[id], &pos, CLSYS_DRAW_MAIN );
}

void DPCOBJ_GetPos( DPCMAIN_WORK * wk, u32 id, s16 * x, s16 * y )
{
	GFL_CLACTPOS	pos;

	GFL_CLACT_WK_GetPos( wk->clwk[id], &pos, CLSYS_DRAW_MAIN );
	*x = pos.x;
	*y = pos.y;
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

		DPCOBJ_CHRRES_MAX+FNTOAM_CHR_MAX,		// 登録できるキャラデータ数
		DPCOBJ_PALRES_MAX,									// 登録できるパレットデータ数
		DPCOBJ_CELRES_MAX+FNTOAM_CHR_MAX,		// 登録できるセルアニメパターン数
		0,																	// 登録できるマルチセルアニメパターン数（※現状未対応）

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
	ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_DENDOU_PC_L );
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

static void ExitResChr( DPCMAIN_WORK * wk, u32 idx )
{
	if( wk->chrRes[idx] != RES_NONE ){
		GFL_CLGRP_CGR_Release( wk->chrRes[idx] );
		wk->chrRes[idx] = RES_NONE;
	}
}
static void ExitResPal( DPCMAIN_WORK * wk, u32 idx )
{
	if( wk->palRes[idx] != RES_NONE ){
    GFL_CLGRP_PLTT_Release( wk->palRes[idx] );
		wk->palRes[idx] = RES_NONE;
	}
}
static void ExitResCel( DPCMAIN_WORK * wk, u32 idx )
{
	if( wk->celRes[idx] != RES_NONE ){
    GFL_CLGRP_CELLANIM_Release( wk->celRes[idx] );
		wk->celRes[idx] = RES_NONE;
	}
}

static void ExitResource( DPCMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<DPCOBJ_CHRRES_MAX; i++ ){
		ExitResChr( wk, i );
	}
	for( i=0; i<DPCOBJ_PALRES_MAX; i++ ){
		ExitResPal( wk, i );
	}
	for( i=0; i<DPCOBJ_CELRES_MAX; i++ ){
		ExitResCel( wk, i );
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

	wk->clunit = GFL_CLACT_UNIT_Create( DPCOBJ_ID_MAX+FNTOAM_CHR_MAX, 0, HEAPID_DENDOU_PC );

	// 初期化
	for( i=0; i<DPCOBJ_ID_MAX; i++ ){
		wk->clwk[i] = NULL;
	}

	for( i=DPCOBJ_ID_ARROW_L; i<=DPCOBJ_ID_RETURN; i++ ){
		wk->clwk[i] = CleateClact( wk, &ClactParamTbl[i-DPCOBJ_ID_ARROW_L] );
	}

	// １ページしかないとき
	if( wk->pageMax == 1 ){
		DPCOBJ_SetVanish( wk, DPCOBJ_ID_ARROW_L, FALSE );
		DPCOBJ_SetVanish( wk, DPCOBJ_ID_ARROW_R, FALSE );
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

typedef struct {
	u16	rad;
	u16	evy;
}POKE_PUT_DATA;

static const POKE_PUT_DATA PokePutRad[6][6] =
{
	{ { 90, 0 }, {   0,  0 }, {   0,  0 }, {   0,  0 }, {   0,  0 }, {  0, 0 } },
	{ { 90, 0 }, { 270, 12 }, {   0,  0 }, {   0,  0 }, {   0,  0 }, {  0, 0 } },
	{ { 90, 0 }, { 210, 10 }, { 330, 10 }, {   0,  0 }, {   0,  0 }, {  0, 0 } },
	{ { 90, 0 }, { 180,  7 }, { 270, 12 }, {   0,  7 }, {   0,  0 }, {  0, 0 } },
	{ { 90, 0 }, { 162,  8 }, { 234, 10 }, { 306, 10 }, {  18,  8 }, {  0, 0 } },
	{ { 90, 0 }, { 150,  8 }, { 210, 10 }, { 270, 12 }, { 330, 10 }, { 30, 8 } },
};

void DPCOBJ_AddPoke( DPCMAIN_WORK * wk )
{
	ARCHANDLE * ah;
	DPC_PARTY_DATA * pt;
	GFL_CLWK ** clwk;
	u16	pal;
	u16	id;
	u32	i;

	// 全ての表示をOFF
	for( i=DPCOBJ_ID_POKE01; i<=DPCOBJ_ID_POKE16; i++ ){
		DPCOBJ_SetVanish( wk, i, FALSE );
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
			pal = PALNUM_POKEGRA + i + 6;
			id  = DPCOBJ_ID_POKE11 + i;
			clwk = &wk->clwk[id];
		}else{
			pal = PALNUM_POKEGRA + i;
			id  = DPCOBJ_ID_POKE01 + i;
			clwk = &wk->clwk[id];
		}

		wk->chrRes[dat.chrRes] = POKE2DGRA_OBJ_CGR_Register(
															ah, pt->dat[i].monsno, pt->dat[i].formNumber, pt->dat[i].sex, rare, POKEGRA_DIR_FRONT,
															FALSE, pt->dat[i].personalRandom, CLSYS_DRAW_MAIN, HEAPID_DENDOU_PC );
		wk->palRes[dat.palRes] = POKE2DGRA_OBJ_PLTT_Register(
															ah, pt->dat[i].monsno, pt->dat[i].formNumber, pt->dat[i].sex, rare, POKEGRA_DIR_FRONT,
															FALSE, CLSYS_DRAW_MAIN, pal*0x20, HEAPID_DENDOU_PC );
		wk->celRes[dat.celRes] = POKE2DGRA_OBJ_CELLANM_Register(
															pt->dat[i].monsno, pt->dat[i].formNumber, pt->dat[i].sex, rare, POKEGRA_DIR_FRONT,
															FALSE, APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, HEAPID_DENDOU_PC );
		*clwk = CleateClact( wk, &dat );

		DPCOBJ_SetPokePos( wk, id, PokePutRad[pt->pokeMax-1][i].rad );
		wk->nowRad[i] = PokePutRad[pt->pokeMax-1][i].rad;

//		PaletteWorkSet_VramCopy( wk->pfd[i], FADE_MAIN_OBJ, pal*16, 0x20 );
	}
	PaletteWorkSet_VramCopy( wk->pfd, FADE_MAIN_OBJ, 0, 0x20*16 );

	GFL_ARC_CloseDataHandle( ah );


	// 表示してたＯＢＪを削除
	if( wk->pokeSwap == 1 ){
		for( i=DPCOBJ_ID_POKE01; i<=DPCOBJ_ID_POKE06; i++ ){
			DelClact( wk, i );
		}
		for( i=DPCOBJ_CHRRES_POKE01; i<=DPCOBJ_CHRRES_POKE06; i++ ){
			ExitResChr( wk, i );
		}
		for( i=DPCOBJ_PALRES_POKE01; i<=DPCOBJ_PALRES_POKE06; i++ ){
			ExitResPal( wk, i );
		}
		for( i=DPCOBJ_CELRES_POKE01; i<=DPCOBJ_CELRES_POKE06; i++ ){
			ExitResCel( wk, i );
		}
	}else{
		for( i=DPCOBJ_ID_POKE11; i<=DPCOBJ_ID_POKE16; i++ ){
			DelClact( wk, i );
		}
		for( i=DPCOBJ_CHRRES_POKE11; i<=DPCOBJ_CHRRES_POKE16; i++ ){
			ExitResChr( wk, i );
		}
		for( i=DPCOBJ_PALRES_POKE11; i<=DPCOBJ_PALRES_POKE16; i++ ){
			ExitResPal( wk, i );
		}
		for( i=DPCOBJ_CELRES_POKE11; i<=DPCOBJ_CELRES_POKE16; i++ ){
			ExitResCel( wk, i );
		}
	}

	wk->pokeSwap ^= 1;

	DPCOBJ_ChangePokePriority( wk );
}

u32 DPCOBJ_GetDefaultPoke( DPCMAIN_WORK * wk )
{
	if( wk->pokeSwap == 0 ){
		return DPCOBJ_ID_POKE11;
	}
	return DPCOBJ_ID_POKE01;
}


#define	POKE_CX_FX32	( 128 << FX32_SHIFT )		// 動作中心Ｘ座標 (fx32型)
#define	POKE_CY_FX32	( 88 << FX32_SHIFT )		// 動作中心Ｙ座標 (fx32型)
#define	POKE_RX_FX32	( 100 << FX32_SHIFT )		// Ｘ半径 (fx32型)
#define	POKE_RY_FX32	( 44 << FX32_SHIFT )		// Ｙ半径 (fx32型)

void DPCOBJ_SetPokePos( DPCMAIN_WORK * wk, u32 id, u32 rad )
{
	fx32	vx, vy;
	s16	px, py;

	vx = FX_MUL( GFL_CALC_Cos360R(rad), POKE_RX_FX32 ) + POKE_CX_FX32;
	vy = FX_MUL( GFL_CALC_Sin360R(rad), POKE_RY_FX32 ) + POKE_CY_FX32;

	px = ( vx & FX32_INT_MASK ) >> FX32_SHIFT;
	py = ( vy & FX32_INT_MASK ) >> FX32_SHIFT;

	DPCOBJ_SetPos( wk, id, px, py );
}


void DPCOBJ_ChangePokePriority( DPCMAIN_WORK * wk )
{
	u32	id;
	s16	x1, y1, x2, y2;
	u8	pri[6];
	u8	i, j;

	id = DPCOBJ_GetDefaultPoke( wk );

	for( i=0; i<6; i++ ){
		pri[i] = id+i;
	}

	for( i=0; i<wk->party[wk->page].pokeMax-1; i++ ){
		DPCOBJ_GetPos( wk, pri[i], &x1, &y1 );
		for( j=i+1; j<wk->party[wk->page].pokeMax; j++ ){
			DPCOBJ_GetPos( wk, pri[j], &x2, &y2 );
			if( y1 < y2 ){
				u8	tmp;
				tmp = pri[i];
				pri[i] = pri[j];
				pri[j] = tmp;
				y1 = y2;
			}
		}
	}

	for( i=0; i<wk->party[wk->page].pokeMax; i++ ){
		if( wk->clwk[pri[i]] != NULL ){
			GFL_CLACT_WK_SetSoftPri( wk->clwk[pri[i]], i );
		}
	}
}

void DPCOBJ_InitFadeEvy( DPCMAIN_WORK * wk, BOOL flg )
{
	u16	pos;
	u16	i;

	pos = wk->pokePos;

	for( i=0; i<wk->party[wk->page].pokeMax; i++ ){
		if( flg == TRUE ){
			wk->posEvy[pos] = PokePutRad[wk->party[wk->page].pokeMax-1][i].evy;
			wk->nowEvy[pos] = wk->posEvy[pos];
			pos++;
			if( pos >= wk->party[wk->page].pokeMax ){
				pos = 0;
			}
		}else{
			wk->posEvy[i] = 0;
			wk->nowEvy[i] = 0;
		}
	}
}


//============================================================================================
//	OAM font
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * ＯＡＭフォント初期化
 *
 * @param
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void FontOamInit( DPCMAIN_WORK * wk )
{
	BMPOAM_ACT_DATA	finit;
	u32	i;

	wk->fntoam = BmpOam_Init( HEAPID_DENDOU_PC, wk->clunit );

	wk->fobmp = GFL_BMP_Create( FOAM_PAGE_SX, FOAM_PAGE_SY, GFL_BMP_16_COLOR, HEAPID_DENDOU_PC );

	finit.bmp = wk->fobmp;
	finit.x = FOAM_PAGE_PX;
	finit.y = FOAM_PAGE_PY;
	finit.pltt_index = wk->palRes[DPCOBJ_PALRES_TB];
	finit.pal_offset = 1;		// pltt_indexのパレット内でのオフセット
	finit.soft_pri = 0;			// ソフトプライオリティ
	finit.bg_pri = 0;				// BGプライオリティ
	finit.setSerface = CLWK_SETSF_NONE;
	finit.draw_type  = CLSYS_DRAW_MAIN;

	wk->foact = BmpOam_ActorAdd( wk->fntoam, &finit );

	if( wk->pageMax == 1 ){
		BmpOam_ActorSetDrawEnable( wk->foact, FALSE );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ＯＡＭフォント削除
 *
 * @param
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void FontOamExit( DPCMAIN_WORK * wk )
{
	BmpOam_ActorDel( wk->foact );
	GFL_BMP_Delete( wk->fobmp );
	BmpOam_Exit( wk->fntoam );
}

void DPCOBJ_FontOamVanish( DPCMAIN_WORK * wk, BOOL flg )
{
	BmpOam_ActorSetDrawEnable( wk->foact, flg );
}
