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

// パレット定義（メイン画面）
#define	PALNUM_ETC				( 0 )																		// パレット番号：その他
#define	PALSIZ_ETC				( 3 )																		// パレットサイズ：その他
#define	PALNUM_POKEGRA		( PALNUM_ETC+PALSIZ_ETC )								// パレット番号：ポケモン（正面）
#define	PALSIZ_POKEGRA		( 1 )																		// パレットサイズ：ポケモン（正面）
#define	PALNUM_POKEGRA_B	( PALNUM_POKEGRA+PALSIZ_POKEGRA )				// パレット番号：ポケモン（背面）
#define	PALSIZ_POKEGRA_B	( 1 )																		// パレットサイズ：ポケモン（背面）
#define	PALNUM_FOAM				( PALNUM_POKEGRA_B+PALSIZ_POKEGRA_B )		// パレット番号：ＯＡＭフォント
#define	PALSIZ_FOAM				( 1 )																		// パレットサイズ：ＯＡＭフォント
#define	PALNUM_PLAYER_M		( PALNUM_FOAM+PALSIZ_FOAM )							// パレット番号：主人公
#define	PALSIZ_PLAYER_M		( 1 )																		// パレットサイズ：主人公
#define	PALNUM_TYPE_WIN		( PALNUM_PLAYER_M+PALSIZ_PLAYER_M )			// パレット番号：タイプ別ウィンドウ
#define	PALSIZ_TYPE_WIN		( 1 )																		// パレットサイズ：タイプ別ウィンドウ

// パレット定義（サブ画面）
#define	PALNUM_ETC_S			( 0 )														// パレット番号：その他
#define	PALSIZ_ETC_S			( 3 )														// パレットサイズ：その他
#define	PALNUM_PLAYER_S		( PALNUM_ETC_S+PALSIZ_ETC_S )		// パレット番号：主人公
#define	PALSIZ_PLAYER_S		( 1 )														// パレットサイズ：主人公

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

#define	MES_PX		( -8 )				//「でんどういり　おめでとう」表示Ｘ座標
#define	MES_PY		( 24 )				//「でんどういり　おめでとう」表示Ｙ座標
#define	INFO_PX		( 256+8 )			// ポケモン情報表示Ｘ座標
#define	INFO_PY		( 192-24 )		// ポケモン情報表示Ｙ座標

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

//「でんどういり　おめでとう」OBJデータ
static const DDEMO_CLWK_DATA MesClactParamTbl = {
	{ MES_PX, MES_PY, 0, 10, 0 },
	DDEMOOBJ_CHRRES_ETC, DDEMOOBJ_PALRES_ETC, DDEMOOBJ_CELRES_ETC,
	0, CLSYS_DRAW_MAIN
};

// ポケモン情報OBJデータ
static const DDEMO_CLWK_DATA InfoClactParamTbl = {
	{ INFO_PX, INFO_PY, 1, 10, 0 },
	DDEMOOBJ_CHRRES_ETC, DDEMOOBJ_PALRES_ETC, DDEMOOBJ_CELRES_ETC,
	0, CLSYS_DRAW_MAIN
};

// シーン１光エフェクトOBJデータ
static const DDEMO_CLWK_DATA PokeFlashClactParamTbl = {
	{ 48, 144, 6, 8, 0 },
	DDEMOOBJ_CHRRES_ETC, DDEMOOBJ_PALRES_ETC, DDEMOOBJ_CELRES_ETC,
	0, CLSYS_DRAW_MAIN
};

// ポケモン正面OBJデータ
static const DDEMO_CLWK_DATA PokeClactParamTbl = {
	{ -48, 144, 0, 10, 0 },
	DDEMOOBJ_CHRRES_POKE, DDEMOOBJ_PALRES_POKE, DDEMOOBJ_CELRES_POKE,
	0, CLSYS_DRAW_MAIN
};

// ポケモン背面OBJデータ
static const DDEMO_CLWK_DATA PokeBackClactParamTbl = {
	{ 304, 144, 0, 10, 0 },
	DDEMOOBJ_CHRRES_POKE_B, DDEMOOBJ_PALRES_POKE_B, DDEMOOBJ_CELRES_POKE_B,
	0, CLSYS_DRAW_MAIN
};

//「ポケモンリーグ　チャンピオン　おめでとう」OBJデータ
static const DDEMO_CLWK_DATA Mes2ClactParamTbl = {
	{ 128, 24, 2, 10, 0 },
	DDEMOOBJ_CHRRES_ETC, DDEMOOBJ_PALRES_ETC, DDEMOOBJ_CELRES_ETC,
	0, CLSYS_DRAW_MAIN
};

// プレイデータOBJデータ
static const DDEMO_CLWK_DATA Info2ClactParamTbl = {
	{ 128, 168, 2, 10, 0 },
	DDEMOOBJ_CHRRES_ETC, DDEMOOBJ_PALRES_ETC, DDEMOOBJ_CELRES_ETC,
	0, CLSYS_DRAW_MAIN
};

// シーン２光エフェクトOBJデータ
static const DDEMO_CLWK_DATA FlashClactParamTbl[] =
{
	{	// メイン
		{ 0, 0, 4, 8, 0 },
		DDEMOOBJ_CHRRES_ETC, DDEMOOBJ_PALRES_ETC, DDEMOOBJ_CELRES_ETC,
		0, CLSYS_DRAW_MAIN
	},
	{	// サブ
		{ 0, 0, 4, 8, 0 },
		DDEMOOBJ_CHRRES_ETC_S, DDEMOOBJ_PALRES_ETC_S, DDEMOOBJ_CELRES_ETC_S,
		0, CLSYS_DRAW_SUB
	},
};

// 主人公OBJデータ
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

// タイプ別ウィンドウパレットテーブル
static const u32 TypePalTbl[] = {
	NARC_dendou_demo_gra_obj_NCLR,						// ノーマル
	NARC_dendou_demo_gra_type_fight_NCLR,			// かくとう
	NARC_dendou_demo_gra_type_fly_NCLR,				// ひこう
	NARC_dendou_demo_gra_type_poison_NCLR,		// どく
	NARC_dendou_demo_gra_type_ground_NCLR,		// じめん
	NARC_dendou_demo_gra_type_rock_NCLR,			// いわ
	NARC_dendou_demo_gra_type_bug_NCLR,				// むし
	NARC_dendou_demo_gra_type_ghost_NCLR,			// ゴースト
	NARC_dendou_demo_gra_type_steel_NCLR,			// はがね
	NARC_dendou_demo_gra_type_fire_NCLR,			// ほのお
	NARC_dendou_demo_gra_type_warter_NCLR,		// みず
	NARC_dendou_demo_gra_type_grass_NCLR,			// くさ
	NARC_dendou_demo_gra_type_elec_NCLR,			// でんき
	NARC_dendou_demo_gra_type_psyc_NCLR,			// エスパー
	NARC_dendou_demo_gra_type_ice_NCLR,				// こおり
	NARC_dendou_demo_gra_type_doragon_NCLR,		// ドラゴン
	NARC_dendou_demo_gra_type_dark_NCLR,			// あく
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		OBJ初期化
 *
 * @param		wk			ワーク
 * @param		scene		シーン番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		OBJ解放
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_Exit( DDEMOMAIN_WORK * wk )
{
	BmpOam_Exit( wk->fntoam );
	GFL_CLACT_UNIT_Delete( wk->clunit );
	GFL_CLACT_SYS_Delete();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪアニメメイン
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪ座標設定
 *
 * @param		wk			ワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_SetPos( DDEMOMAIN_WORK * wk, u32 id, s16 x, s16 y )
{
	GFL_CLACTPOS	pos;

	pos.x = x;
	pos.y = y;
	GFL_CLACT_WK_SetPos( wk->clwk[id], &pos, CLSYS_DRAW_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪ座標取得
 *
 * @param		wk			ワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ座標格納場所
 * @param		y				Ｙ座標格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_GetPos( DDEMOMAIN_WORK * wk, u32 id, s16 * x, s16 * y )
{
	GFL_CLACTPOS	pos;

	GFL_CLACT_WK_GetPos( wk->clwk[id], &pos, CLSYS_DRAW_MAIN );
	*x = pos.x;
	*y = pos.y;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪ移動
 *
 * @param		wk			ワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ移動量
 * @param		y				Ｙ移動量
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_Move( DDEMOMAIN_WORK * wk, u32 id, s16 mx, s16 my )
{
	s16	x, y;

	DDEMOOBJ_GetPos( wk, id, &x, &y );
	x += mx;
	y += my;
	DDEMOOBJ_SetPos( wk, id, x, y );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪ表示切り替え
 *
 * @param		wk			ワーク
 * @param		id			OBJ ID
 * @param		flg			TRUE = 表示, FALSE = 非表示
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_SetVanish( DDEMOMAIN_WORK * wk, u32 id, BOOL flg )
{
	if( wk->clwk[id] != NULL ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[id], flg );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪオートアニメ設定
 *
 * @param		wk			ワーク
 * @param		id			OBJ ID
 * @param		anm			アニメ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_SetAutoAnm( DDEMOMAIN_WORK * wk, u32 id, u32 anm )
{
	GFL_CLACT_WK_SetAnmFrame( wk->clwk[id], 0 );
	GFL_CLACT_WK_SetAnmSeq( wk->clwk[id], anm );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[id], TRUE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪアニメ稼動チェック
 *
 * @param		wk			ワーク
 * @param		id			OBJ ID
 *
 * @retval	"TRUE = アニメ中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL DDEMOOBJ_CheckAnm( DDEMOMAIN_WORK * wk, u32 id )
{
	return GFL_CLACT_WK_CheckAnmActive( wk->clwk[id] );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		リソース初期化
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		キャラリソース削除
 *
 * @param		wk			ワーク
 * @param		idx			リソースインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitResChr( DDEMOMAIN_WORK * wk, u32 idx )
{
	if( wk->chrRes[idx] != RES_NONE ){
		GFL_CLGRP_CGR_Release( wk->chrRes[idx] );
		wk->chrRes[idx] = RES_NONE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットリソース削除
 *
 * @param		wk			ワーク
 * @param		idx			リソースインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitResPal( DDEMOMAIN_WORK * wk, u32 idx )
{
	if( wk->palRes[idx] != RES_NONE ){
    GFL_CLGRP_PLTT_Release( wk->palRes[idx] );
		wk->palRes[idx] = RES_NONE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルリソース削除
 *
 * @param		wk			ワーク
 * @param		idx			リソースインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitResCel( DDEMOMAIN_WORK * wk, u32 idx )
{
	if( wk->celRes[idx] != RES_NONE ){
    GFL_CLGRP_CELLANIM_Release( wk->celRes[idx] );
		wk->celRes[idx] = RES_NONE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リソース削除
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪ追加
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static GFL_CLWK * CleateClact( DDEMOMAIN_WORK * wk, const DDEMO_CLWK_DATA * prm )
{
	return GFL_CLACT_WK_Create(
					wk->clunit,
					wk->chrRes[prm->chrRes],
					wk->palRes[prm->palRes],
					wk->celRes[prm->celRes],
					&prm->dat, prm->disp, HEAPID_DENDOU_DEMO );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪ削除（個別）
 *
 * @param		wk			ワーク
 * @param		id			OBJ ID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void DelClact( DDEMOMAIN_WORK * wk, u32 id )
{
	if( wk->clwk[id] != NULL ){
		GFL_CLACT_WK_Remove( wk->clwk[id] );
		wk->clwk[id] = NULL;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪ削除（全体）
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void DelClactAll( DDEMOMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<DDEMOOBJ_ID_MAX; i++ ){
		DelClact( wk, i );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンＯＢＪ追加
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

//	ppp = PP_GetPPPPointer( PokeParty_GetMemberPointer(wk->dat->party,wk->pokePos) );
	ppp = PP_GetPPPPointer( wk->pp[wk->pokePos] );

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

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイプ別ウィンドウパレット設定
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_SetTypeWindow( DDEMOMAIN_WORK * wk )
{
	ARCHANDLE * ah;
	NNSG2dPaletteData * pal;
	void * buf;

	ah = GFL_ARC_OpenDataHandle( ARCID_DENDOU_DEMO_GRA, HEAPID_DENDOU_DEMO_L );

	buf = GFL_ARCHDL_UTIL_LoadPalette( ah, TypePalTbl[wk->type], &pal, HEAPID_DENDOU_DEMO );
	GFL_CLGRP_PLTT_Replace( wk->palRes[DDEMOOBJ_PALRES_ETC], pal, 1 );
	GFL_HEAP_FreeMemory( buf );

	GFL_ARC_CloseDataHandle( ah );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＡＭフォント削除（個別）
 *
 * @param		wk			ワーク
 * @param		idx			ＯＡＭフォントインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void DeleteFontOam( DDEMOMAIN_WORK * wk, u32 idx )
{
	if( wk->fobj[idx].oam != NULL ){
		BmpOam_ActorDel( wk->fobj[idx].oam );
		GFL_BMP_Delete( wk->fobj[idx].bmp );
		wk->fobj[idx].oam = NULL;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＡＭフォント削除（全体）
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void DeleteFontOamAll( DDEMOMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<DDEMOOBJ_FOAM_MAX; i++ ){
		DeleteFontOam( wk, i );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＡＭフォント書き込み
 *
 * @param		wk			ワーク
 * @param		idx			ＯＡＭフォントインデックス
 * @param		font		フォント
 * @param		str			文字列
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PrintFontOam( DDEMOMAIN_WORK * wk, u32 idx, GFL_FONT * font, STRBUF * str, u32 x, u32 y )
{
	PRINTSYS_PrintColor( wk->fobj[idx].bmp, x, y, str, font, FCOL_FNTOAM );
	BmpOam_ActorBmpTrans( wk->fobj[idx].oam );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーン１のＯＡＭフォント初期化
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン情報書き込み
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_PrintPokeInfo( DDEMOMAIN_WORK * wk )
{
	POKEMON_PASO_PARAM * ppp;
	STRBUF * str;
	u32	x;
	BOOL	fast;

//	ppp  = PP_GetPPPPointer( PokeParty_GetMemberPointer(wk->dat->party,wk->pokePos) );
	ppp = PP_GetPPPPointer( wk->pp[wk->pokePos] );
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
	x   = PRINTSYS_GetStrWidth( str, wk->font, 0 );
	PrintFontOam( wk, DDEMOOBJ_FOAM_INFO, wk->font, str, 14*8, 0 );
	GFL_STR_DeleteBuffer( str );

	// レベル
	str = GFL_MSG_CreateString( wk->mman, DDEMO_STR_04 );
	WORDSET_RegisterNumber(
		wk->wset, 0, PPP_Get(ppp,ID_PARA_level,NULL), 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PrintFontOam( wk, DDEMOOBJ_FOAM_INFO, wk->font, wk->exp, 14*8+x, 0 );
	GFL_STR_DeleteBuffer( str );

	PPP_FastModeOff( ppp, fast );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーン２のＯＡＭフォント初期化
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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
	PrintFontOam( wk, DDEMOOBJ_FOAM_PLAYER, wk->font, wk->exp, 92, 0 );
	GFL_STR_DeleteBuffer( str );
	// プレイタイム
	str = GFL_MSG_CreateString( wk->mman, DDEMO_STR_08 );
	WORDSET_RegisterNumber(
		wk->wset, 0, PLAYTIME_GetHour(wk->dat->ptime), 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_RegisterNumber(
		wk->wset, 1, PLAYTIME_GetMinute(wk->dat->ptime), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PrintFontOam( wk, DDEMOOBJ_FOAM_PLAYER, wk->font, wk->exp, 200, 0 );
	GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＡＭフォント移動
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_MoveFontOamPos( DDEMOMAIN_WORK * wk )
{
	s16	x, y;

	DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_MES, &x, &y );
	BmpOam_ActorSetPos( wk->fobj[DDEMOOBJ_FOAM_MES].oam, x-FOAM_MES_SX*8, y-8 );

	DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_INFO, &x, &y );
	BmpOam_ActorSetPos( wk->fobj[DDEMOOBJ_FOAM_INFO].oam, x, y-8 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＡＭフォント表示切り替え
 *
 * @param		wk			ワーク
 * @param		idx			ＯＡＭフォントインデックス
 * @param		flg			TRUE = 表示, FALSE = 非表示
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2OBJ_FontOamVanish( DDEMOMAIN_WORK * wk, u32 idx, BOOL flg )
{
	BmpOam_ActorSetDrawEnable( wk->fobj[idx].oam, flg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		共通リソース読み込み
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーン１初期化
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_InitScene1( DDEMOMAIN_WORK * wk )
{
	LoadDefaultResource( wk );
	DDEMOOBJ_SetTypeWindow( wk );

	wk->clwk[DDEMOOBJ_ID_MES]  = CleateClact( wk, &MesClactParamTbl );
	wk->clwk[DDEMOOBJ_ID_INFO] = CleateClact( wk, &InfoClactParamTbl );
	wk->clwk[DDEMOOBJ_ID_EFF] = CleateClact( wk, &PokeFlashClactParamTbl );
	DDEMOOBJ_SetVanish( wk, DDEMOOBJ_ID_EFF, FALSE );

	InitScene1FontOam( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーン１解放
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_ExitScene1( DDEMOMAIN_WORK * wk )
{
	DeleteFontOamAll( wk );
	DelClactAll( wk );
	ExitResource( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーン１メイン
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーン２初期化
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

	// キラキラ
	ah = GFL_ARC_OpenDataHandle( ARCID_DENDOU_DEMO_GRA, HEAPID_DENDOU_DEMO_L );

	wk->chrRes[DDEMOOBJ_CHRRES_ETC_S] = GFL_CLGRP_CGR_Register(
																				ah, NARC_dendou_demo_gra_obj_32k_NCGR,
																				FALSE, CLSYS_DRAW_SUB, HEAPID_DENDOU_DEMO );
	wk->palRes[DDEMOOBJ_PALRES_ETC_S] = GFL_CLGRP_PLTT_Register(
																				ah, NARC_dendou_demo_gra_obj_NCLR,
																				CLSYS_DRAW_SUB, PALNUM_ETC_S*0x20, HEAPID_DENDOU_DEMO );
	wk->celRes[DDEMOOBJ_CELRES_ETC_S] = GFL_CLGRP_CELLANIM_Register(
																				ah,
																				NARC_dendou_demo_gra_obj_32k_NCER,
																				NARC_dendou_demo_gra_obj_32k_NANR,
																				HEAPID_DENDOU_DEMO );
	GFL_ARC_CloseDataHandle( ah );

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

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーン２解放
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_ExitScene2( DDEMOMAIN_WORK * wk )
{
	DeleteFontOamAll( wk );
	DelClactAll( wk );

	GFL_CLACT_USERREND_Delete( wk->clrend );

	ExitResource( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーン２メイン
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_MainScene2( DDEMOMAIN_WORK * wk )
{
	AnmMain( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーン２光ＯＢＪ表示設定
 *
 * @param		wk			ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOOBJ_SetRandomFlash( DDEMOMAIN_WORK * wk )
{
	s16	x, y;
	u16	anm;
	u16	i;

	// 稼動していないＯＢＪを削除
	for( i=DDEMOOBJ_ID_2ND_EFF; i<DDEMOOBJ_ID_2ND_EFF+DDEMOOBJ_ID_2ND_EFF_MAX; i++ ){
		if( wk->clwk[i] == NULL ){ continue; }
		if( DDEMOOBJ_CheckAnm( wk, i ) == TRUE ){ continue; }
		DelClact( wk, i );
	}

	// ランダム
	anm = GFUser_GetPublicRand( 4 );
	if( anm >= 2 ){ return; }

	// 稼動していないＯＢＪを探して追加
	for( i=DDEMOOBJ_ID_2ND_EFF; i<DDEMOOBJ_ID_2ND_EFF+DDEMOOBJ_ID_2ND_EFF_MAX; i++ ){
		if( wk->clwk[i] == NULL ){
			DDEMO_CLWK_DATA	cldat;
			x = GFUser_GetPublicRand( 256 );
			y = GFUser_GetPublicRand( 192+24 );
			// メインへ
			if( y < 192 ){
				cldat = FlashClactParamTbl[0];
				cldat.dat.pos_x = x;
				cldat.dat.pos_y = y;
			// サブへ
			}else{
				cldat = FlashClactParamTbl[1];
				cldat.dat.pos_x = x;
				cldat.dat.pos_y = y - 24;
			}
			wk->clwk[i] = CleateClact( wk, &cldat );
			DDEMOOBJ_SetAutoAnm( wk, i, 4+anm );
			break;
		}
	}
}
