//============================================================================================
/**
 * @file		ddemo_obj.c
 * @brief		殿堂入りデモ画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.06
 *
 *	モジュール名：DDEMOOBJ
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/poke2dgra.h"
#include "system/tr2dgra.h"
#include "tr_tool/trtype_def.h"
#include "msg/msg_dendou_demo.h"
#include "font/font.naix"

#include "ddemo_main.h"
#include "ddemo_obj.h"
#include "dendou_demo_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	RES_NONE	( 0xffffffff )		// リソースなし

#define	PALNUM_ETC				( 0 )
#define	PALSIZ_ETC				( 2 )
#define	PALNUM_POKEGRA		( PALNUM_ETC+PALSIZ_ETC )
#define	PALSIZ_POKEGRA		( 1 )
#define	PALNUM_POKEGRA_B	( PALNUM_POKEGRA+PALSIZ_POKEGRA )
#define	PALSIZ_POKEGRA_B	( 1 )
#define	PALNUM_FOAM				( PALNUM_POKEGRA_B+PALSIZ_POKEGRA_B )
#define	PALSIZ_FOAM				( 1 )
#define	PALNUM_PLAYER_M		( PALNUM_FOAM+PALSIZ_FOAM )
#define	PALSIZ_PLAYER_M		( 1 )

#define	PALNUM_PLAYER_S		( 0 )
#define	PALSIZ_PLAYER_S		( 1 )

#define	FCOL_FNTOAM				( PRINTSYS_LSB_Make(15,2,0) )		// フォントカラー：ＯＡＭフォント白抜


// セルアクターデータ
typedef struct {
	GFL_CLWK_DATA	dat;

	u32	chrRes;
	u32	palRes;
	u32	celRes;

	u16	pal;
	u16	disp;

}DDEMO_CLWK_DATA;


#define	MES_PX		( -8 )
#define	MES_PY		( 24 )
#define	INFO_PX		( 256+8 )
#define	INFO_PY		( 192-24 )

// フォントOAMで使用するBMPのサイズ
#define	FOAM_MES_SX			( 22 )
#define	FOAM_MES_SY			( 2 )
#define	FOAM_INFO_SX		( 22 )
#define	FOAM_INFO_SY		( 2 )
#define	FOAM_MES2_SX		( 32 )
#define	FOAM_MES2_SY		( 2 )
#define	FOAM_PLAYER_SX	( 32 )
#define	FOAM_PLAYER_SY	( 2 )

// フォントOAMで使用するOBJ管理数
#define	FNTOAM_CHR_MAX	( FOAM_MES_SX*FOAM_MES_SY+FOAM_INFO_SX*FOAM_INFO_SY+FOAM_MES2_SX*FOAM_MES2_SY+FOAM_PLAYER_SX*FOAM_PLAYER_SY )


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void InitResource( DDEMOMAIN_WORK * wk );
static void ExitResource( DDEMOMAIN_WORK * wk );
static void AddClact( DDEMOMAIN_WORK * wk );
static void DelClactAll( DDEMOMAIN_WORK * wk );

static void InitFontOam( DDEMOMAIN_WORK * wk );
static void ExitFontOam( DDEMOMAIN_WORK * wk );
static void PrintFontOam( DDEMOMAIN_WORK * wk, u32 idx, GFL_FONT * font, STRBUF * str, u32 x, u32 y );


//============================================================================================
//	グローバル
//============================================================================================

static const DDEMO_CLWK_DATA MesClactParamTbl = {
	{ MES_PX, MES_PY, 0, 10, 0 },
	DDEMOOBJ_CHRRES_ETC, DDEMOOBJ_PALRES_ETC, DDEMOOBJ_CELRES_ETC,
	0, CLSYS_DRAW_MAIN
};

static const DDEMO_CLWK_DATA InfoClactParamTbl = {
	{ INFO_PX, INFO_PY, 1, 10, 0 },
	DDEMOOBJ_CHRRES_ETC, DDEMOOBJ_PALRES_ETC, DDEMOOBJ_CELRES_ETC,
	0, CLSYS_DRAW_MAIN
};

static const DDEMO_CLWK_DATA PokeClactParamTbl = {
	{ -48, 144, 0, 10, 0 },
	DDEMOOBJ_CHRRES_POKE, DDEMOOBJ_PALRES_POKE, DDEMOOBJ_CELRES_POKE,
	0, CLSYS_DRAW_MAIN
};

static const DDEMO_CLWK_DATA PokeBackClactParamTbl = {
	{ 304, 144, 0, 10, 0 },
	DDEMOOBJ_CHRRES_POKE_B, DDEMOOBJ_PALRES_POKE_B, DDEMOOBJ_CELRES_POKE_B,
	0, CLSYS_DRAW_MAIN
};

static const DDEMO_CLWK_DATA Mes2ClactParamTbl = {
	{ 128, 24, 2, 10, 0 },
	DDEMOOBJ_CHRRES_ETC, DDEMOOBJ_PALRES_ETC, DDEMOOBJ_CELRES_ETC,
	0, CLSYS_DRAW_MAIN
};

static const DDEMO_CLWK_DATA Info2ClactParamTbl = {
	{ 128, 168, 2, 10, 0 },
	DDEMOOBJ_CHRRES_ETC, DDEMOOBJ_PALRES_ETC, DDEMOOBJ_CELRES_ETC,
	0, CLSYS_DRAW_MAIN
};

static const DDEMO_CLWK_DATA PlayerClactParamTbl[] =
{
	{	// メイン
		{ 88, -240-64, 0, 10, 1 },
		DDEMOOBJ_CHRRES_PLAYER_M, DDEMOOBJ_PALRES_PLAYER_M, DDEMOOBJ_CELRES_PLAYER_M,
		0, CLSYS_DRAW_MAIN
	},
	{	// サブ
		{ 88, -48, 0, 10, 1 },
		DDEMOOBJ_CHRRES_PLAYER_S, DDEMOOBJ_PALRES_PLAYER_S, DDEMOOBJ_CELRES_PLAYER_S,
		0, CLSYS_DRAW_SUB
	}
};


void DDEMOOBJ_Init( DDEMOMAIN_WORK * wk, u32 scene )
{
	if( scene == 0 ){
		const GFL_CLSYS_INIT init = {
			0, 0,									// メイン　サーフェースの左上座標
			0, 512,								// サブ　サーフェースの左上座標
			4,										// メイン画面OAM管理開始位置	4の倍数
			124,									// メイン画面OAM管理数				4の倍数
			0,										// サブ画面OAM管理開始位置		4の倍数
			0,									// サブ画面OAM管理数					4の倍数
			0,										// セルVram転送管理数

			DDEMOOBJ_CHRRES_MAX+FNTOAM_CHR_MAX,	// 登録できるキャラデータ数
			DDEMOOBJ_PALRES_MAX,								// 登録できるパレットデータ数
			DDEMOOBJ_CELRES_MAX+FNTOAM_CHR_MAX,	// 登録できるセルアニメパターン数
			0,																	// 登録できるマルチセルアニメパターン数（※現状未対応）

		  16,										// メイン CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
		  0											// サブ CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
		};
		GFL_CLACT_SYS_Create( &init, DDEMOMAIN_GetVramBankData(scene), HEAPID_DENDOU_DEMO );
	}else{
		const GFL_CLSYS_INIT init = {
			0, 0,									// メイン　サーフェースの左上座標
			0, 512,								// サブ　サーフェースの左上座標
			4,										// メイン画面OAM管理開始位置	4の倍数
			124,									// メイン画面OAM管理数				4の倍数
			4,										// サブ画面OAM管理開始位置		4の倍数
			124,									// サブ画面OAM管理数					4の倍数
			0,										// セルVram転送管理数

			DDEMOOBJ_CHRRES_MAX+FNTOAM_CHR_MAX,	// 登録できるキャラデータ数
			DDEMOOBJ_PALRES_MAX,								// 登録できるパレットデータ数
			DDEMOOBJ_CELRES_MAX+FNTOAM_CHR_MAX,	// 登録できるセルアニメパターン数
			0,																	// 登録できるマルチセルアニメパターン数（※現状未対応）

		  16,										// メイン CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
		  16										// サブ CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
		};
		GFL_CLACT_SYS_Create( &init, DDEMOMAIN_GetVramBankData(scene), HEAPID_DENDOU_DEMO );
	}	

	wk->clunit = GFL_CLACT_UNIT_Create( DDEMOOBJ_ID_MAX+FNTOAM_CHR_MAX, 0, HEAPID_DENDOU_DEMO );
	wk->fntoam = BmpOam_Init( HEAPID_DENDOU_DEMO, wk->clunit );

	InitResource( wk );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}

void DDEMOOBJ_Exit( DDEMOMAIN_WORK * wk )
{
	BmpOam_Exit( wk->fntoam );
	GFL_CLACT_UNIT_Delete( wk->clunit );
	GFL_CLACT_SYS_Delete();
}

static void AnmMain( DDEMOMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<DDEMOOBJ_ID_MAX; i++ ){
		if( wk->clwk[i] == NULL ){ continue; }
		if( GFL_CLACT_WK_GetAutoAnmFlag( wk->clwk[i] ) == TRUE ){ continue; }
		GFL_CLACT_WK_AddAnmFrame( wk->clwk[i], FX32_ONE );
	}
	GFL_CLACT_SYS_Main();
}

void DDEMOOBJ_SetPos( DDEMOMAIN_WORK * wk, u32 id, s16 x, s16 y )
{
	GFL_CLACTPOS	pos;

	pos.x = x;
	pos.y = y;
	GFL_CLACT_WK_SetPos( wk->clwk[id], &pos, CLSYS_DRAW_MAIN );
}

void DDEMOOBJ_GetPos( DDEMOMAIN_WORK * wk, u32 id, s16 * x, s16 * y )
{
	GFL_CLACTPOS	pos;

	GFL_CLACT_WK_GetPos( wk->clwk[id], &pos, CLSYS_DRAW_MAIN );
	*x = pos.x;
	*y = pos.y;
}

void DDEMOOBJ_Move( DDEMOMAIN_WORK * wk, u32 id, s16 mx, s16 my )
{
	s16	x, y;

	DDEMOOBJ_GetPos( wk, id, &x, &y );
	x += mx;
	y += my;
	DDEMOOBJ_SetPos( wk, id, x, y );
}

void DDEMOOBJ_SetVanish( DDEMOMAIN_WORK * wk, u32 id, BOOL flg )
{
	if( wk->clwk[id] != NULL ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[id], flg );
	}
}

void DDEMOOBJ_SetAutoAnm( DDEMOMAIN_WORK * wk, u32 id, u32 anm )
{
	GFL_CLACT_WK_SetAnmFrame( wk->clwk[id], 0 );
	GFL_CLACT_WK_SetAnmSeq( wk->clwk[id], anm );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[id], TRUE );
}

BOOL DDEMOOBJ_CheckAnm( DDEMOMAIN_WORK * wk, u32 id )
{
	return GFL_CLACT_WK_CheckAnmActive( wk->clwk[id] );
}





static void InitResource( DDEMOMAIN_WORK * wk )
{
	u32	i;

	// 初期化
	for( i=0; i<DDEMOOBJ_CHRRES_MAX; i++ ){
		wk->chrRes[i] = RES_NONE;
	}
	for( i=0; i<DDEMOOBJ_PALRES_MAX; i++ ){
		wk->palRes[i] = RES_NONE;
	}
	for( i=0; i<DDEMOOBJ_CELRES_MAX; i++ ){
		wk->celRes[i] = RES_NONE;
	}
}

static void ExitResChr( DDEMOMAIN_WORK * wk, u32 idx )
{
	if( wk->chrRes[idx] != RES_NONE ){
		GFL_CLGRP_CGR_Release( wk->chrRes[idx] );
		wk->chrRes[idx] = RES_NONE;
	}
}
static void ExitResPal( DDEMOMAIN_WORK * wk, u32 idx )
{
	if( wk->palRes[idx] != RES_NONE ){
    GFL_CLGRP_PLTT_Release( wk->palRes[idx] );
		wk->palRes[idx] = RES_NONE;
	}
}
static void ExitResCel( DDEMOMAIN_WORK * wk, u32 idx )
{
	if( wk->celRes[idx] != RES_NONE ){
    GFL_CLGRP_CELLANIM_Release( wk->celRes[idx] );
		wk->celRes[idx] = RES_NONE;
	}
}

static void ExitResource( DDEMOMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<DDEMOOBJ_CHRRES_MAX; i++ ){
		ExitResChr( wk, i );
	}
	for( i=0; i<DDEMOOBJ_PALRES_MAX; i++ ){
		ExitResPal( wk, i );
	}
	for( i=0; i<DDEMOOBJ_CELRES_MAX; i++ ){
		ExitResCel( wk, i );
	}
}

static GFL_CLWK * CleateClact( DDEMOMAIN_WORK * wk, const DDEMO_CLWK_DATA * prm )
{
	return GFL_CLACT_WK_Create(
					wk->clunit,
					wk->chrRes[prm->chrRes],
					wk->palRes[prm->palRes],
					wk->celRes[prm->celRes],
					&prm->dat, prm->disp, HEAPID_DENDOU_DEMO );
}


static void DelClact( DDEMOMAIN_WORK * wk, u32 idx )
{
	if( wk->clwk[idx] != NULL ){
		GFL_CLACT_WK_Remove( wk->clwk[idx] );
		wk->clwk[idx] = NULL;
	}
}

static void DelClactAll( DDEMOMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<DDEMOOBJ_ID_MAX; i++ ){
		DelClact( wk, i );
	}
}

void DDEMOOBJ_AddPoke( DDEMOMAIN_WORK * wk )
{
	ARCHANDLE * ah;
	POKEMON_PASO_PARAM * ppp;
	BOOL fast;

	if( wk->clwk[DDEMOOBJ_ID_POKE] != NULL ){
		DelClact( wk, DDEMOOBJ_ID_POKE );
		ExitResChr( wk, DDEMOOBJ_CHRRES_POKE );
		ExitResPal( wk, DDEMOOBJ_PALRES_POKE );
		ExitResCel( wk, DDEMOOBJ_CELRES_POKE );
	}
	if( wk->clwk[DDEMOOBJ_ID_POKE_B] != NULL ){
		DelClact( wk, DDEMOOBJ_ID_POKE_B );
		ExitResChr( wk, DDEMOOBJ_CHRRES_POKE_B );
		ExitResPal( wk, DDEMOOBJ_PALRES_POKE_B );
		ExitResCel( wk, DDEMOOBJ_CELRES_POKE_B );
	}

	ah = POKE2DGRA_OpenHandle( HEAPID_DENDOU_DEMO );

	ppp = PP_GetPPPPointer( PokeParty_GetMemberPointer(wk->dat->party,wk->pokePos) );
	fast = PPP_FastModeOn( ppp );
	// 正面
	wk->chrRes[DDEMOOBJ_CHRRES_POKE] = POKE2DGRA_OBJ_CGR_RegisterPPP(
																			ah, ppp, POKEGRA_DIR_FRONT, CLSYS_DRAW_MAIN, HEAPID_DENDOU_DEMO );
	wk->palRes[DDEMOOBJ_PALRES_POKE] = POKE2DGRA_OBJ_PLTT_RegisterPPP(
																			ah, ppp, POKEGRA_DIR_FRONT,
																			CLSYS_DRAW_MAIN, PALNUM_POKEGRA*0x20, HEAPID_DENDOU_DEMO );
	wk->celRes[DDEMOOBJ_CELRES_POKE] = POKE2DGRA_OBJ_CELLANM_RegisterPPP(
																			ppp, POKEGRA_DIR_FRONT,
																			APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, HEAPID_DENDOU_DEMO );
	// 背面
	wk->chrRes[DDEMOOBJ_CHRRES_POKE_B] = POKE2DGRA_OBJ_CGR_RegisterPPP(
																				ah, ppp, POKEGRA_DIR_BACK, CLSYS_DRAW_MAIN, HEAPID_DENDOU_DEMO );
	wk->palRes[DDEMOOBJ_PALRES_POKE_B] = POKE2DGRA_OBJ_PLTT_RegisterPPP(
																				ah, ppp, POKEGRA_DIR_BACK,
																				CLSYS_DRAW_MAIN, PALNUM_POKEGRA_B*0x20, HEAPID_DENDOU_DEMO );
	wk->celRes[DDEMOOBJ_CELRES_POKE_B] = POKE2DGRA_OBJ_CELLANM_RegisterPPP(
																				ppp, POKEGRA_DIR_BACK,
																				APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, HEAPID_DENDOU_DEMO );
	PPP_FastModeOff( ppp, fast );

  GFL_ARC_CloseDataHandle( ah );

	wk->clwk[DDEMOOBJ_ID_POKE]   = CleateClact( wk, &PokeClactParamTbl );
	wk->clwk[DDEMOOBJ_ID_POKE_B] = CleateClact( wk, &PokeBackClactParamTbl );
}


/*
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
*/



static void DeleteFontOam( DDEMOMAIN_WORK * wk, u32 idx )
{
	if( wk->fobj[idx].oam != NULL ){
		BmpOam_ActorDel( wk->fobj[idx].oam );
		GFL_BMP_Delete( wk->fobj[idx].bmp );
		wk->fobj[idx].oam = NULL;
	}
}

static void DeleteFontOamAll( DDEMOMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<DDEMOOBJ_FOAM_MAX; i++ ){
		DeleteFontOam( wk, i );
	}
}

static void PrintFontOam( DDEMOMAIN_WORK * wk, u32 idx, GFL_FONT * font, STRBUF * str, u32 x, u32 y )
{
	PRINTSYS_PrintColor( wk->fobj[idx].bmp, x, y, str, font, FCOL_FNTOAM );
	BmpOam_ActorBmpTrans( wk->fobj[idx].oam );
}

static void InitScene1FontOam( DDEMOMAIN_WORK * wk )
{
	BMPOAM_ACT_DATA	finit;
	DDEMO_FONTOAM * fobj;
	STRBUF * str;

	finit.pltt_index = wk->palRes[DDEMOOBJ_PALRES_FOAM];
	finit.pal_offset = 0;		// pltt_indexのパレット内でのオフセット
	finit.soft_pri = 4;			// ソフトプライオリティ
	finit.bg_pri = 0;				// BGプライオリティ
	finit.setSerface = CLWK_SETSF_NONE;
	finit.draw_type  = CLSYS_DRAW_MAIN;

	//「でんどういり　おめでとう」
	fobj = &wk->fobj[DDEMOOBJ_FOAM_MES];

	fobj->bmp = GFL_BMP_Create( FOAM_MES_SX, FOAM_MES_SY, GFL_BMP_16_COLOR, HEAPID_DENDOU_DEMO );

	finit.bmp = fobj->bmp;
	finit.x = MES_PX-FOAM_MES_SX*8;
	finit.y = MES_PY-8;

	fobj->oam = BmpOam_ActorAdd( wk->fntoam, &finit );

	GFL_BMP_Clear( wk->fobj[DDEMOOBJ_FOAM_MES].bmp, 0 );
	str = GFL_MSG_CreateString( wk->mman, DDEMO_STR_01 );
	PrintFontOam( wk, DDEMOOBJ_FOAM_MES, wk->font, str, 12, 0 );
	GFL_STR_DeleteBuffer( str );

	// ポケモン情報
	fobj = &wk->fobj[DDEMOOBJ_FOAM_INFO];

	fobj->bmp = GFL_BMP_Create( FOAM_INFO_SX, FOAM_INFO_SY, GFL_BMP_16_COLOR, HEAPID_DENDOU_DEMO );

	finit.bmp = fobj->bmp;
	finit.x = INFO_PX;
	finit.y = INFO_PY-8;

	fobj->oam = BmpOam_ActorAdd( wk->fntoam, &finit );
}

void DDEMOOBJ_PrintPokeInfo( DDEMOMAIN_WORK * wk )
{
	POKEMON_PASO_PARAM * ppp;
	STRBUF * str;
	u32	x;
	BOOL	fast;

	ppp  = PP_GetPPPPointer( PokeParty_GetMemberPointer(wk->dat->party,wk->pokePos) );
	fast = PPP_FastModeOn( ppp );

	GFL_BMP_Clear( wk->fobj[DDEMOOBJ_FOAM_INFO].bmp, 0 );

	// ニックネーム
	str = GFL_MSG_CreateString( wk->mman, DDEMO_STR_02 );
	WORDSET_RegisterPokeNickNamePPP( wk->wset, 0, ppp );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PrintFontOam( wk, DDEMOOBJ_FOAM_INFO, wk->font, wk->exp, 8, 0 );
	GFL_STR_DeleteBuffer( str );

	// Lv
	str = GFL_MSG_CreateString( wk->mman, DDEMO_STR_03 );
	x   = PRINTSYS_GetStrWidth( str, wk->nfnt, 0 );
	PrintFontOam( wk, DDEMOOBJ_FOAM_INFO, wk->nfnt, str, 16*8, 4 );
	GFL_STR_DeleteBuffer( str );

	// レベル
	str = GFL_MSG_CreateString( wk->mman, DDEMO_STR_04 );
	WORDSET_RegisterNumber( wk->wset, 0, 999, 0, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PrintFontOam( wk, DDEMOOBJ_FOAM_INFO, wk->font, wk->exp, 16*8+x, 0 );
	GFL_STR_DeleteBuffer( str );

	PPP_FastModeOff( ppp, fast );
}

static void InitScene2FontOam( DDEMOMAIN_WORK * wk )
{
	BMPOAM_ACT_DATA	finit;
	DDEMO_FONTOAM * fobj;
	STRBUF * str;
	u32	x;

	finit.pltt_index = wk->palRes[DDEMOOBJ_PALRES_FOAM];
	finit.pal_offset = 0;		// pltt_indexのパレット内でのオフセット
	finit.soft_pri = 4;			// ソフトプライオリティ
	finit.bg_pri = 0;				// BGプライオリティ
	finit.setSerface = CLWK_SETSF_NONE;
	finit.draw_type  = CLSYS_DRAW_MAIN;

	//「ポケモンリーグ　チャンピオン　おめでとう」
	fobj = &wk->fobj[DDEMOOBJ_FOAM_MES2];

	fobj->bmp = GFL_BMP_Create( FOAM_MES2_SX, FOAM_MES2_SY, GFL_BMP_16_COLOR, HEAPID_DENDOU_DEMO );

	finit.bmp = fobj->bmp;
	finit.x = 0;
	finit.y = 16;

	fobj->oam = BmpOam_ActorAdd( wk->fntoam, &finit );
	BmpOam_ActorSetDrawEnable( fobj->oam, FALSE );

	GFL_BMP_Clear( wk->fobj[DDEMOOBJ_FOAM_MES2].bmp, 0 );
	str = GFL_MSG_CreateString( wk->mman, DDEMO_STR_05 );
	x   = PRINTSYS_GetStrWidth( str, wk->font, 0 );
	PrintFontOam( wk, DDEMOOBJ_FOAM_MES2, wk->font, str, (FOAM_MES2_SX*8-x)/2, 0 );
	GFL_STR_DeleteBuffer( str );

	// プレイヤー情報
	fobj = &wk->fobj[DDEMOOBJ_FOAM_PLAYER];

	fobj->bmp = GFL_BMP_Create( FOAM_PLAYER_SX, FOAM_PLAYER_SY, GFL_BMP_16_COLOR, HEAPID_DENDOU_DEMO );

	finit.bmp = fobj->bmp;
	finit.x = 0;
	finit.y = 160;

	fobj->oam = BmpOam_ActorAdd( wk->fntoam, &finit );
	BmpOam_ActorSetDrawEnable( fobj->oam, FALSE );

	GFL_BMP_Clear( wk->fobj[DDEMOOBJ_FOAM_PLAYER].bmp, 0 );
	// 名前
	str = GFL_MSG_CreateString( wk->mman, DDEMO_STR_06 );
	WORDSET_RegisterPlayerName( wk->wset, 0, wk->dat->mystatus );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PrintFontOam( wk, DDEMOOBJ_FOAM_PLAYER, wk->font, wk->exp, 8, 0 );
	GFL_STR_DeleteBuffer( str );
	// ID
	str = GFL_MSG_CreateString( wk->mman, DDEMO_STR_07 );
	WORDSET_RegisterNumber(
		wk->wset, 0, MyStatus_GetID_Low(wk->dat->mystatus), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PrintFontOam( wk, DDEMOOBJ_FOAM_PLAYER, wk->font, wk->exp, 96, 0 );
	GFL_STR_DeleteBuffer( str );
	// プレイタイム
	str = GFL_MSG_CreateString( wk->mman, DDEMO_STR_08 );
	WORDSET_RegisterNumber(
		wk->wset, 0, PLAYTIME_GetHour(wk->dat->ptime), 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_RegisterNumber(
		wk->wset, 1, PLAYTIME_GetMinute(wk->dat->ptime), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PrintFontOam( wk, DDEMOOBJ_FOAM_PLAYER, wk->font, wk->exp, 184, 0 );
	GFL_STR_DeleteBuffer( str );
}


void DDEMOOBJ_MoveFontOamPos( DDEMOMAIN_WORK * wk )
{
	s16	x, y;

	DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_MES, &x, &y );
	BmpOam_ActorSetPos( wk->fobj[DDEMOOBJ_FOAM_MES].oam, x-FOAM_MES_SX*8, y-8 );

	DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_INFO, &x, &y );
	BmpOam_ActorSetPos( wk->fobj[DDEMOOBJ_FOAM_INFO].oam, x, y-8 );
}

void BOX2OBJ_FontOamVanish( DDEMOMAIN_WORK * wk, u32 idx, BOOL flg )
{
	BmpOam_ActorSetDrawEnable( wk->fobj[idx].oam, flg );
}



static void LoadDefaultResource( DDEMOMAIN_WORK * wk )
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_DENDOU_DEMO_GRA, HEAPID_DENDOU_DEMO_L );

	wk->chrRes[DDEMOOBJ_CHRRES_ETC] = GFL_CLGRP_CGR_Register(
																			ah, NARC_dendou_demo_gra_obj_NCGR,
																			FALSE, CLSYS_DRAW_MAIN, HEAPID_DENDOU_DEMO );
	wk->palRes[DDEMOOBJ_PALRES_ETC] = GFL_CLGRP_PLTT_Register(
																			ah, NARC_dendou_demo_gra_obj_NCLR,
																			CLSYS_DRAW_MAIN, PALNUM_ETC*0x20, HEAPID_DENDOU_DEMO );
	wk->celRes[DDEMOOBJ_CELRES_ETC] = GFL_CLGRP_CELLANIM_Register(
																			ah,
																			NARC_dendou_demo_gra_obj_NCER,
																			NARC_dendou_demo_gra_obj_NANR,
																			HEAPID_DENDOU_DEMO );

	GFL_ARC_CloseDataHandle( ah );

	// フォントパレット
	ah = GFL_ARC_OpenDataHandle( ARCID_FONT, HEAPID_DENDOU_DEMO_L );
	wk->palRes[DDEMOOBJ_PALRES_FOAM] = GFL_CLGRP_PLTT_RegisterComp(
																			ah, NARC_font_default_nclr,
																			CLSYS_DRAW_MAIN, PALNUM_FOAM*0x20, HEAPID_DENDOU_DEMO );
	GFL_ARC_CloseDataHandle( ah );
}



void DDEMOOBJ_InitScene1( DDEMOMAIN_WORK * wk )
{
	LoadDefaultResource( wk );

	wk->clwk[DDEMOOBJ_ID_MES]  = CleateClact( wk, &MesClactParamTbl );
	wk->clwk[DDEMOOBJ_ID_INFO] = CleateClact( wk, &InfoClactParamTbl );

	InitScene1FontOam( wk );
}

void DDEMOOBJ_ExitScene1( DDEMOMAIN_WORK * wk )
{
	DeleteFontOamAll( wk );
	DelClactAll( wk );
	ExitResource( wk );
}

void DDEMOOBJ_MainScene1( DDEMOMAIN_WORK * wk )
{
	u32	i;

	if( GFL_G3D_DOUBLE3D_GetFlip() == TRUE ){
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_OFF );
	}else{
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	}
	AnmMain( wk );
}



void DDEMOOBJ_InitScene2( DDEMOMAIN_WORK * wk )
{
	ARCHANDLE * ah;
	u32	trID;

	LoadDefaultResource( wk );

	{	// レンダラー作成
		GFL_REND_SURFACE_INIT	sfinit[] =
		{
			{ 0, 0, 256, 192, CLSYS_DRAW_MAIN, CLSYS_REND_CULLING_TYPE_NOT_AFFINE },
			{ 0, 512, 256, 192, CLSYS_DRAW_SUB, CLSYS_REND_CULLING_TYPE_NOT_AFFINE }
		};
		wk->clrend = GFL_CLACT_USERREND_Create( sfinit, 2, HEAPID_DENDOU_DEMO );
    GFL_CLACT_USERREND_SetOAMAttrCulling( wk->clrend, TRUE );
		GFL_CLACT_UNIT_SetUserRend( wk->clunit, wk->clrend );
	}

	wk->clwk[DDEMOOBJ_ID_2ND_MES]  = CleateClact( wk, &Mes2ClactParamTbl );
	wk->clwk[DDEMOOBJ_ID_2ND_INFO] = CleateClact( wk, &Info2ClactParamTbl );
	DDEMOOBJ_SetVanish( wk, DDEMOOBJ_ID_2ND_MES, FALSE );
	DDEMOOBJ_SetVanish( wk, DDEMOOBJ_ID_2ND_INFO, FALSE );

	// 主人公
	if( MyStatus_GetMySex( wk->dat->mystatus ) == PM_MALE ){
		trID = TRTYPE_HERO;
	}else{
		trID = TRTYPE_HEROINE;
	}
	ah = TR2DGRA_OpenHandle( HEAPID_DENDOU_DEMO_L );
	// メイン
	wk->chrRes[DDEMOOBJ_CHRRES_PLAYER_M] = TR2DGRA_OBJ_CGR_Register( ah, trID, CLSYS_DRAW_MAIN, HEAPID_DENDOU_DEMO );
	wk->palRes[DDEMOOBJ_PALRES_PLAYER_M] = TR2DGRA_OBJ_PLTT_Register( ah, trID, CLSYS_DRAW_MAIN, PALNUM_PLAYER_M*0x20, HEAPID_DENDOU_DEMO );
	wk->celRes[DDEMOOBJ_CELRES_PLAYER_M] = TR2DGRA_OBJ_CELLANM_Register( trID, APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, HEAPID_DENDOU_DEMO );
	// サブ
	wk->chrRes[DDEMOOBJ_CHRRES_PLAYER_S] = TR2DGRA_OBJ_CGR_Register( ah, trID, CLSYS_DRAW_SUB, HEAPID_DENDOU_DEMO );
	wk->palRes[DDEMOOBJ_PALRES_PLAYER_S] = TR2DGRA_OBJ_PLTT_Register( ah, trID, CLSYS_DRAW_SUB, PALNUM_PLAYER_S*0x20, HEAPID_DENDOU_DEMO );
	wk->celRes[DDEMOOBJ_CELRES_PLAYER_S] = TR2DGRA_OBJ_CELLANM_Register( trID, APP_COMMON_MAPPING_32K, CLSYS_DRAW_SUB, HEAPID_DENDOU_DEMO );
	GFL_ARC_CloseDataHandle( ah );

	wk->clwk[DDEMOOBJ_ID_PLAYER_M] = CleateClact( wk, &PlayerClactParamTbl[0] );
	wk->clwk[DDEMOOBJ_ID_PLAYER_S] = CleateClact( wk, &PlayerClactParamTbl[1] );

	InitScene2FontOam( wk );
}

void DDEMOOBJ_ExitScene2( DDEMOMAIN_WORK * wk )
{
	DeleteFontOamAll( wk );
	DelClactAll( wk );

	GFL_CLACT_USERREND_Delete( wk->clrend );

	ExitResource( wk );
}

void DDEMOOBJ_MainScene2( DDEMOMAIN_WORK * wk )
{
	AnmMain( wk );
}
