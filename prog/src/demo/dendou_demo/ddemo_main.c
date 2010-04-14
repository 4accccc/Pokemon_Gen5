//============================================================================================
/**
 * @file		ddemo_main.c
 * @brief		殿堂入りデモ画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.06
 *
 *	モジュール名：DDEMOMAIN
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "system/gfl_use.h"
#include "system/mcss_tool.h"
#include "sound/pm_sndsys.h"
#include "sound/pm_wb_voice.h"
#include "font/font.naix"

#include "ddemo_main.h"
#include "name_spa.h"
#include "dendou_demo_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	EXP_BUF_SIZE	( 1024 )	// テンポラリメッセージバッファサイズ

#define DTCM_SIZE		(0x1000)		// ジオメトリバッファの使用サイズ


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void Scene1_VBlankTask( GFL_TCB * tcb, void * work );
static void Scene2_VBlankTask( GFL_TCB * tcb, void * work );


//============================================================================================
//	グローバル
//============================================================================================

// VRAM割り振り
static const GFL_DISP_VRAM VramTblScene1 = {
	GX_VRAM_BG_32_FG,							// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,				// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_NONE,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_NONE,					// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_0_A,							// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_0123_E,				// テクスチャパレットスロット

	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_128K		// サブOBJマッピングモード
};

static const GFL_DISP_VRAM VramTblScene2 = {
	GX_VRAM_BG_128_D,							// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_23_G,				// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_0123_H,	// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_16_I,					// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_0_A,							// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_0123_E,				// テクスチャパレットスロット

	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K		// サブOBJマッピングモード
};


// タイプ別パーティクルテーブル
static const u32 TypeArcTbl[] = {
	NARC_dendou_demo_gra_normal_spa,		// ノーマル
	NARC_dendou_demo_gra_fight_spa,			// かくとう
	NARC_dendou_demo_gra_fly_spa,				// ひこう
	NARC_dendou_demo_gra_poison_spa,		// どく
	NARC_dendou_demo_gra_ground_spa,		// じめん
	NARC_dendou_demo_gra_rock_spa,			// いわ
	NARC_dendou_demo_gra_bug_spa,				// むし
	NARC_dendou_demo_gra_ghost_spa,			// ゴースト
	NARC_dendou_demo_gra_steel_spa,			// はがね
	NARC_dendou_demo_gra_fire_spa,			// ほのお
	NARC_dendou_demo_gra_warter_spa,		// みず
	NARC_dendou_demo_gra_grass_spa,			// くさ
	NARC_dendou_demo_gra_elec_spa,			// でんき
	NARC_dendou_demo_gra_psyc_spa,			// エスパー
	NARC_dendou_demo_gra_ice_spa,				// こおり
	NARC_dendou_demo_gra_dragon_spa,		// ドラゴン
	NARC_dendou_demo_gra_dark_spa,			// あく
};

// プリセットＳＥテーブル
static const u32 SndTbl[] = {
//	SEQ_SE_DDEMO_02A,
//	SEQ_SE_DDEMO_02B,
	SEQ_SE_ROTATION_B,
//	SEQ_SE_DDEMO_01,
//	SEQ_SE_DDEMO_03,
//	SEQ_SE_DDEMO_04,
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_InitScene1VBlank( DDEMOMAIN_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( Scene1_VBlankTask, wk, 0 );
}
void DDEMOMAIN_InitScene2VBlank( DDEMOMAIN_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( Scene2_VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_ExitVBlank( DDEMOMAIN_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理
 *
 * @param		tcb		GFL_TCB
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void Scene1_VBlankTask( GFL_TCB * tcb, void * work )
{
//	GFL_BG_VBlankFunc();
	GFL_CLACT_SYS_VBlankFunc();
	GFL_G3D_DOUBLE3D_VblankIntr();
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}
static void Scene2_VBlankTask( GFL_TCB * tcb, void * work )
{
	GFL_BG_VBlankFunc();
	GFL_CLACT_SYS_VBlankFunc();
//	GFL_G3D_DOUBLE3D_VblankIntr();
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}



void DDEMOMAIN_InitVram( u32 scene )
{
	GFL_DISP_ClearVRAM( 0 );
	if( scene == 0 ){
		GFL_DISP_SetBank( &VramTblScene1 );
	}else{
		GFL_DISP_SetBank( &VramTblScene2 );
	}
}

const GFL_DISP_VRAM * DDEMOMAIN_GetVramBankData( u32 scene )
{
	if( scene == 0 ){
		return &VramTblScene1;
	}
	return &VramTblScene2;
}

void DDEMOMAIN_InitBg(void)
{
	GFL_BG_Init( HEAPID_DENDOU_DEMO );
}

void DDEMOMAIN_ExitBg(void)
{
	GFL_BG_Exit();
}

void DDEMOMAIN_InitBgMode(void)
{
	GFL_BG_SYS_HEADER sysh = {
		GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BGMODE_3, GX_BG0_AS_3D,
	};
	GFL_BG_SetBGMode( &sysh );
}

void DDEMOMAIN_InitScene2BgFrame(void)
{
	{
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x08000, 0x10000,
			GX_BG_EXTPLTT_23, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_256X16 );
		GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &cnth, GFL_BG_MODE_256X16 );
	}

	{	// 回転拡縮小初期化
		MtxFx22 mtx;
	  GFL_CALC2D_GetAffineMtx22( &mtx, 0, FX32_ONE, FX32_ONE, GFL_CALC2D_AFFINE_MAX_NORMAL );
		G2_SetBG3Affine( &mtx, 0, 0, 0, 0 );
		G2S_SetBG3Affine( &mtx, 0, 0, 0, 0 );
	}

	GFL_BG_SetScrollReq( GFL_BG_FRAME3_M, GFL_BG_SCROLL_X_SET, 64 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME3_M, GFL_BG_SCROLL_Y_SET, 112 );

	GFL_BG_SetScrollReq( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_SET, 64 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, -152 );

	GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_M, GFL_BG_CENTER_X_SET, 160 );
	GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_M, GFL_BG_CENTER_Y_SET, 160 );
	GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_S, GFL_BG_CENTER_X_SET, 160 );
	GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_S, GFL_BG_CENTER_Y_SET, 160 );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
}

void DDEMOMAIN_ExitScene2BgFrame(void)
{
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
}

void DDEMOMAIN_LoadScene2BgGraphic(void)
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_DENDOU_DEMO_GRA, HEAPID_DENDOU_DEMO );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_dendou_demo_gra_bg_ball_NCLR, PALTYPE_MAIN_BG_EX, 0x6000, 0x20, HEAPID_DENDOU_DEMO );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_dendou_demo_gra_bg_ball_NCLR, PALTYPE_SUB_BG_EX, 0x6000, 0x20, HEAPID_DENDOU_DEMO );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_dendou_demo_gra_bg_ball_NCGR, GFL_BG_FRAME3_M, 0, 0, FALSE, HEAPID_DENDOU_DEMO );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_dendou_demo_gra_bg_ball_NCGR, GFL_BG_FRAME3_S, 0, 0, FALSE, HEAPID_DENDOU_DEMO );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_dendou_demo_gra_bg_ball_NSCR, GFL_BG_FRAME3_M, 0, 0, FALSE, HEAPID_DENDOU_DEMO );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_dendou_demo_gra_bg_ball_NSCR, GFL_BG_FRAME3_S, 0, 0, FALSE, HEAPID_DENDOU_DEMO );
/*
	{
		void * buf;
		NNSG2dScreenData * scrn;
		u16 * dat;
		u32	i, j;

		buf = GFL_ARCHDL_UTIL_LoadScreen( ah, NARC_dendou_demo_gra_bg_ball_NSCR, FALSE, &scrn, HEAPID_DENDOU_DEMO );
		dat = (u16 *)scrn->rawData;
		for( i=0; i<64; i++ ){
			for( j=0; j<64; j++ ){
				OS_Printf( "%d, ", dat[i*64+j] );
			}
			OS_Printf( "\n" );
		}
		GFL_HEAP_FreeMemory( buf );
	}
*/



	GFL_ARC_CloseDataHandle( ah );
}

void DDEMOMAIN_SetBlendAlpha(void)
{
	G2_SetBlendAlpha(
		GX_BLEND_PLANEMASK_BG0,
		GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BD,
		6, 10 );
	G2S_SetBlendAlpha(
		GX_BLEND_PLANEMASK_BG0,
		GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BD,
		6, 10 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連初期化
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_InitMsg( DDEMOMAIN_WORK * wk )
{
	wk->mman = GFL_MSG_Create(
							GFL_MSG_LOAD_NORMAL,
							ARCID_MESSAGE, NARC_message_dendou_demo_dat, HEAPID_DENDOU_DEMO );
	wk->font = GFL_FONT_Create(
							ARCID_FONT, NARC_font_large_gftr,
							GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_DENDOU_DEMO );
  wk->nfnt = GFL_FONT_Create(
							ARCID_FONT, NARC_font_num_gftr,
							GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_DENDOU_DEMO );
	wk->wset = WORDSET_Create( HEAPID_DENDOU_DEMO );
//	wk->que  = PRINTSYS_QUE_Create( HEAPID_DENDOU_DEMO );
	wk->exp  = GFL_STR_CreateBuffer( EXP_BUF_SIZE, HEAPID_DENDOU_DEMO );

//  PRINTSYS_QUE_ForceCommMode( wk->que, TRUE );      // テスト
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連解放
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_ExitMsg( DDEMOMAIN_WORK * wk )
{
	GFL_STR_DeleteBuffer( wk->exp );
//	PRINTSYS_QUE_Delete( wk->que );
	WORDSET_Delete( wk->wset );
	GFL_FONT_Delete( wk->nfnt );
	GFL_FONT_Delete( wk->font );
	GFL_MSG_Delete( wk->mman );
}


void DDEMOMAIN_InitSound( DDEMOMAIN_WORK * wk )
{
//	PMSND_PlayBGM( SEQ_BGM_E_DENDOUIRI );
//	PMSND_PauseBGM( TRUE );

	wk->sndHandle = SOUNDMAN_PresetSoundTbl( SndTbl, NELEMS(SndTbl) );

//	PMSND_PauseBGM( FALSE );
	PMSND_PlayBGM( SEQ_BGM_E_DENDOUIRI );
}

void DDEMOMAIN_ExitSound( DDEMOMAIN_WORK * wk )
{
	PMSND_StopBGM();
	SOUNDMAN_ReleasePresetData( wk->sndHandle );
}



void DDEMOMAIN_GetPokeMax( DDEMOMAIN_WORK * wk )
{
	wk->pokeMax = PokeParty_GetPokeCount( wk->dat->party );
}

void DDEMOMAIN_GetPokeData( DDEMOMAIN_WORK * wk )
{
	POKEMON_PARAM * pp;
	BOOL	fast;
	
	pp = PokeParty_GetMemberPointer( wk->dat->party, wk->pokePos );

	fast = PP_FastModeOn( pp );

	wk->monsno = PP_Get( pp, ID_PARA_monsno, NULL );
	wk->type   = PP_Get( pp, ID_PARA_type1, NULL );
	wk->form   = PP_Get( pp, ID_PARA_form_no, NULL );

	PP_FastModeOff( pp, fast );
}

void DDEMOMAIN_LoadPokeVoice( DDEMOMAIN_WORK * wk )
{
	// @TODO ペラップチェック
	if( wk->monsno == MONSNO_PERAPPU ){
#if 0
		PMV_REF	pmvRef;
    PMV_MakeRefDataMine( &pmvRef );
		wk->voicePlayer = PMVOICE_LoadOnly( wk->monsno, wk->form, 64, FALSE, 0, 0, FALSE, (u32)&pmvRef );
#else
		wk->voicePlayer = PMVOICE_LoadOnly( wk->monsno, wk->form, 64, FALSE, 0, 0, FALSE, NULL );
#endif
	}else{
		wk->voicePlayer = PMVOICE_LoadOnly( wk->monsno, wk->form, 64, FALSE, 0, 0, FALSE, NULL );
	}
}





























// 上画面カメラ設定（数値適当）
// 焦点
#define	U_CAMERA_TX_F32	( 0.0 )
#define	U_CAMERA_TY_F32	( 192.0 )
#define	U_CAMERA_TZ_F32	( 0.0 )
//#define	U_CAMERA_TX_F32	( 0.0 )
//#define	U_CAMERA_TY_F32	( 4.5 )
//#define	U_CAMERA_TZ_F32	( -6.0 )
#define	U_CAMERA_TX		( FX_F32_TO_FX32(U_CAMERA_TX_F32) )
#define	U_CAMERA_TY		( FX_F32_TO_FX32(U_CAMERA_TY_F32) )
#define	U_CAMERA_TZ		( FX_F32_TO_FX32(U_CAMERA_TZ_F32) )
// 位置
#define	U_CAMERA_PX_F32	( 0.0 )
#define	U_CAMERA_PY_F32	( 18.0 )
#define	U_CAMERA_PZ_F32	( 5.6 )
#define	U_CAMERA_PX		( FX_F32_TO_FX32(U_CAMERA_PX_F32) )
#define	U_CAMERA_PY		( FX_F32_TO_FX32(U_CAMERA_PY_F32) )
#define	U_CAMERA_PZ		( FX_F32_TO_FX32(U_CAMERA_PZ_F32) )

// 下画面カメラ設定（数値適当）
// 焦点
//#define	D_CAMERA_TX_F32	( 0.0 )
//#define	D_CAMERA_TY_F32	( 4.5 )
//#define	D_CAMERA_TZ_F32	( 2.3 )
#define	D_CAMERA_TX_F32	( 0.0 )
#define	D_CAMERA_TY_F32	( 0.0 )
#define	D_CAMERA_TZ_F32	( 0.0 )
#define	D_CAMERA_TX		( FX_F32_TO_FX32(D_CAMERA_TX_F32) )
#define	D_CAMERA_TY		( FX_F32_TO_FX32(D_CAMERA_TY_F32) )
#define	D_CAMERA_TZ		( FX_F32_TO_FX32(D_CAMERA_TZ_F32) )
// 位置
//#define	D_CAMERA_PX_F32	( 0.0 )
//#define	D_CAMERA_PY_F32	( 18.0 )
//#define	D_CAMERA_PZ_F32	( 14.0 )
#define	D_CAMERA_PX_F32	( 0.0 )
#define	D_CAMERA_PY_F32	( 0.0 )
#define	D_CAMERA_PZ_F32	( 0.0 )
#define	D_CAMERA_PX		( FX_F32_TO_FX32(D_CAMERA_PX_F32) )
#define	D_CAMERA_PY		( FX_F32_TO_FX32(D_CAMERA_PY_F32) )
#define	D_CAMERA_PZ		( FX_F32_TO_FX32(D_CAMERA_PZ_F32) )

#define	CAMERA_FOVY		( 18.5 )			// 縦(y)方向の視界角度
#define	CAMERA_ASPECT	( FX32_ONE * 1.5 )	// 縦に対する横の視界の割合

// ライト設定（数値適当）
//#define	LIGHT_PX			( FX_F32_TO_FX16(0.0) )
//#define	LIGHT_PY			( FX_F32_TO_FX16(-0.8) )
//#define	LIGHT_PZ			( FX_F32_TO_FX16(-0.5) )
#define	LIGHT_PX			( FX_F32_TO_FX16(0.0) )
#define	LIGHT_PY			( FX_F32_TO_FX16(0.0) )
#define	LIGHT_PZ			( FX_F32_TO_FX16(0.0) )
#define	LIGHT_COLOR		( 0x7fff )

// 絶対値計算
/*
#define	GAME_START_CAMERA_MX_U		( U_CAMERA_PX/16 )
#define	GAME_START_CAMERA_MY_U		( U_CAMERA_PY/16 )
#define	GAME_START_CAMERA_MZ_U		( U_CAMERA_PZ/16 )
#define	GAME_START_CAMERA_MX_D		( (f32)(D_CAMERA_PX_F32-D_CAMERA_TX_F32)/(f32)16 )
#define	GAME_START_CAMERA_MY_D		( (f32)(D_CAMERA_PY_F32-D_CAMERA_TY_F32)/(f32)16 )
#define	GAME_START_CAMERA_MZ_D		( (f32)(D_CAMERA_PZ_F32-D_CAMERA_TZ_F32)/(f32)16 )

#define	GAME_START_CAMERA_MOVE_UX(cnt)	( GAME_START_CAMERA_MX_U * (16-cnt) )
#define	GAME_START_CAMERA_MOVE_UY(cnt)	( GAME_START_CAMERA_MY_U * (16-cnt) )
#define	GAME_START_CAMERA_MOVE_UZ(cnt)	( GAME_START_CAMERA_MZ_U * (16-cnt) )
#define	GAME_START_CAMERA_MOVE_DX(cnt)	( FX_F32_TO_FX32( (f32)GAME_START_CAMERA_MX_D * (f32)(16-cnt) ) )
#define	GAME_START_CAMERA_MOVE_DY(cnt)	( FX_F32_TO_FX32( (f32)GAME_START_CAMERA_MY_D * (f32)(16-cnt) ) )
#define	GAME_START_CAMERA_MOVE_DZ(cnt)	( FX_F32_TO_FX32( (f32)GAME_START_CAMERA_MZ_D * (f32)(16-cnt) ) )
*/

// カメラ注視点
static const VecFx32 U_ComeraTarget = { U_CAMERA_TX, U_CAMERA_TY, U_CAMERA_TZ };
static const VecFx32 D_ComeraTarget = { D_CAMERA_TX, D_CAMERA_TY, D_CAMERA_TZ };

// カメラ位置
static const VecFx32 U_ComeraPos = { U_CAMERA_PX, U_CAMERA_PY, U_CAMERA_PZ };
static const VecFx32 D_ComeraPos = { D_CAMERA_PX, D_CAMERA_PY, D_CAMERA_PZ };

// ライトデータ
static const GFL_G3D_LIGHT_DATA LightData = { 0, { { LIGHT_PX, LIGHT_PY, LIGHT_PZ }, LIGHT_COLOR } };
static const GFL_G3D_LIGHTSET_SETUP LightSetup = { &LightData, 1 };

static void CreateParticleCamera( DDEMOMAIN_WORK * wk, GFL_PTC_PTR ptc, BOOL disp );
static void CreateNameParticleCamera( DDEMOMAIN_WORK * wk, GFL_PTC_PTR ptc );

static void SetUp3D(void)
{
	// ３Ｄ使用面の設定(表示＆プライオリティー)
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
	G2_SetBG0Priority(0);

	// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
	//	フォグ、エッジマーキング、トゥーンシェードの詳細設定は
	//	テーブルを作成して設定してください
	G3X_SetShading( GX_SHADING_HIGHLIGHT );	//シェーディング
	G3X_AntiAlias( FALSE );									//	アンチエイリアス
	G3X_AlphaTest( FALSE, 0 );							//	アルファテスト　　オフ
	G3X_AlphaBlend( TRUE );									//	アルファブレンド　オン
	G3X_EdgeMarking( FALSE );								//	エッジマーキング
	G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );	//フォグ

	// クリアカラーの設定
	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog
	// ビューポートの設定
	G3_ViewPort(0, 0, 255, 191);

	// ライト設定
	{
		static const GFL_G3D_LIGHT sc_GFL_G3D_LIGHT[] = 
		{
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
		};
		int i;
		
		for( i=0; i<NELEMS(sc_GFL_G3D_LIGHT); i++ ){
			GFL_G3D_SetSystemLight( i, &sc_GFL_G3D_LIGHT[i] );
		}
	}

	//レンダリングスワップバッファ
	GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
}

void DDEMOMAIN_Init3D( DDEMOMAIN_WORK * wk )
{
	// ３Ｄシステム起動
	GFL_G3D_Init(
		GFL_G3D_VMANLNK,		// テクスチャマネージャ使用モード（フレームモード（とりっぱなし））
		GFL_G3D_TEX128K,		// テクスチャマネージャサイズ 128KB(1ブロック)
		GFL_G3D_VMANLNK,		// パレットマネージャ使用モード（フレームモード（とりっぱなし））
		GFL_G3D_PLT32K,			// パレットマネージャサイズ
		DTCM_SIZE,					// ジオメトリバッファの使用サイズ
		HEAPID_DENDOU_DEMO,	// ヒープID
		SetUp3D );					// セットアップ関数(NULLの時はDefaultSetUp)
//		NULL );					// セットアップ関数(NULLの時はDefaultSetUp)

	// ハンドル作成
	wk->g3d_util  = GFL_G3D_UTIL_Create( 32, 32, HEAPID_DENDOU_DEMO );

/*
	// 管理システム作成
	wk->g3d_scene = GFL_G3D_SCENE_Create(
										wk->g3d_util,						// 依存するg3Dutil
										1000,										// 配置可能なオブジェクト数
										4,											// １オブジェクトに割り当てるワークのサイズ
										32,											// アクセサリ数
										TRUE,										// パーティクルシステムの起動フラグ
										HEAPID_DENDOU_DEMO );		// ヒープID
*/




/*
	{	// カメラ作成
		VecFx32 defaultCameraUp = { 0, FX32_ONE, 0 };

		wk->g3d_camera[0] = GFL_G3D_CAMERA_Create(
													GFL_G3D_PRJPERS, 
													FX_SinIdx( CAMERA_FOVY/2 *PERSPWAY_COEFFICIENT ),
													FX_CosIdx( CAMERA_FOVY/2 *PERSPWAY_COEFFICIENT ),
													CAMERA_ASPECT,
													0,
													defaultCameraNear,
													FX32_ONE,
													0,
													&D_ComeraPos,
													&defaultCameraUp,
													&D_ComeraTarget,
													HEAPID_DENDOU_DEMO );

		wk->g3d_camera[1] = GFL_G3D_CAMERA_Create(
													GFL_G3D_PRJPERS, 
													FX_SinIdx( CAMERA_FOVY/2 *PERSPWAY_COEFFICIENT ),
													FX_CosIdx( CAMERA_FOVY/2 *PERSPWAY_COEFFICIENT ),
													CAMERA_ASPECT,
													0,
													defaultCameraNear,
													FX32_ONE,
													0,
													&U_ComeraPos,
													&defaultCameraUp,
													&U_ComeraTarget,
													HEAPID_DENDOU_DEMO );
	}

	// ライト作成
	wk->g3d_light = GFL_G3D_LIGHT_Create( &LightSetup, HEAPID_DENDOU_DEMO );

	// カメラ・ライト反映
	GFL_G3D_CAMERA_Switching( wk->g3d_camera[0] );
	GFL_G3D_LIGHT_Switching( wk->g3d_light );
*/

//	G3X_AntiAlias( TRUE );	// セットアップ関数で作ったほうがいいけど。。。

//	GFL_BG_SetBGControl3D( 0 );				// BG面設定（引数は優先度）

//	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
}

/*
void TITLE3D_Load( TITLE_WORK * wk )
{
	// OBJ追加
	wk->local->g3d_obj_id = GFL_G3D_SCENEOBJ_Add( wk->local->g3d_scene, ObjData, NELEMS(ObjData) );
	TITLE3D_JewelBoxAnmChange( wk, JEWELBOX_ANM_STOP );


	ParticleCreate( wk );

}
*/

void DDEMOMAIN_Exit3D( DDEMOMAIN_WORK * wk )
{
/*
	GFL_G3D_LIGHT_Delete( wk->g3d_light );
	GFL_G3D_CAMERA_Delete( wk->g3d_camera[1] );
	GFL_G3D_CAMERA_Delete( wk->g3d_camera[0] );
*/
//	GFL_G3D_SCENEOBJ_Remove( wk->local->g3d_scene, wk->local->g3d_obj_id, NELEMS(ObjData) );



//	GFL_G3D_SCENE_Delete( wk->g3d_scene );
	GFL_G3D_UTIL_Delete( wk->g3d_util );

	GFL_G3D_Exit();
}

void DDEMOMAIN_Main3D( DDEMOMAIN_WORK * wk )
{
/*
	GFL_G3D_SCENE_Main( wk->g3d_scene );
	if( GFL_G3D_DOUBLE3D_GetFlip() == TRUE ){
//		GFL_G3D_CAMERA_Switching( wk->g3d_camera[0] );
//		GFL_G3D_SCENE_SetDrawParticleSW( wk->g3d_scene, TRUE );
//		CreateParticleCamera( wk, TRUE );
	}else{
//		GFL_G3D_CAMERA_Switching( wk->g3d_camera[1] );
//		GFL_G3D_SCENE_SetDrawParticleSW( wk->g3d_scene, FALSE );
//		CreateParticleCamera( wk, FALSE );
	}
	GFL_G3D_SCENE_Draw( wk->g3d_scene );
*/

	GFL_G3D_DRAW_Start();
//	GFL_G3D_CAMERA_Switching( wk->mcssCamera );
	GFL_G3D_DRAW_SetLookAt();

	MCSS_Main( wk->mcss );
	MCSS_Draw( wk->mcss );
//	GFL_PTC_Main();

	GFL_G3D_DRAW_End();
}

void DDEMOMAIN_MainDouble3D( DDEMOMAIN_WORK * wk )
{
/*
	GFL_G3D_SCENE_Main( wk->g3d_scene );
	if( GFL_G3D_DOUBLE3D_GetFlip() == TRUE ){
//		GFL_G3D_CAMERA_Switching( wk->g3d_camera[0] );
//		GFL_G3D_SCENE_SetDrawParticleSW( wk->g3d_scene, TRUE );
//		CreateParticleCamera( wk, TRUE );
	}else{
//		GFL_G3D_CAMERA_Switching( wk->g3d_camera[1] );
//		GFL_G3D_SCENE_SetDrawParticleSW( wk->g3d_scene, FALSE );
//		CreateParticleCamera( wk, FALSE );
	}
	GFL_G3D_SCENE_Draw( wk->g3d_scene );
*/
	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();

	if( GFL_G3D_DOUBLE3D_GetFlip() == TRUE ){
		CreateParticleCamera( wk, wk->ptc, TRUE );
//		CreateParticleCamera( wk, wk->ptcName, TRUE );
		CreateNameParticleCamera( wk, wk->ptcName );
	}else{
		CreateParticleCamera( wk, wk->ptc, FALSE );
		CreateParticleCamera( wk, wk->ptcName, FALSE );
	}
	GFL_PTC_Main();

	GFL_G3D_DRAW_End();
	GFL_G3D_DOUBLE3D_SetSwapFlag();
}


void DDEMOMAIN_InitDouble3D(void)
{
	GFL_G3D_DOUBLE3D_Init( HEAPID_DENDOU_DEMO );	// 2画面3D初期化
}

void DDEMOMAIN_ExitDouble3D(void)
{
	GFL_G3D_DOUBLE3D_Exit();
}

void DDEMOMAIN_InitParticle(void)
{
	GFL_PTC_Init( HEAPID_DENDOU_DEMO );
}

void DDEMOMAIN_ExitParticle(void)
{
	GFL_PTC_Exit();
}


void DDEMOMAIN_CreateTypeParticle( DDEMOMAIN_WORK * wk )
{
	void * res;

	wk->ptc = GFL_PTC_Create( wk->ptcWork, PARTICLE_LIB_HEAP_SIZE, TRUE, HEAPID_DENDOU_DEMO );
	res = GFL_PTC_LoadArcResource( ARCID_DENDOU_DEMO_GRA, TypeArcTbl[wk->type], HEAPID_DENDOU_DEMO );
	GFL_PTC_SetResource( wk->ptc, res, TRUE, NULL );
}

void DDEMOMAIN_DeleteTypeParticle( DDEMOMAIN_WORK * wk )
{
	GFL_PTC_Delete( wk->ptc );
	wk->ptc = NULL;
}

void DDEMOMAIN_SetTypeParticle( DDEMOMAIN_WORK * wk, u32 id )
{
	VecFx32	p = { 0, 0, 0 };
	GFL_PTC_CreateEmitter( wk->ptc, id, &p );
}

void DDEMOMAIN_CreateNameParticle( DDEMOMAIN_WORK * wk )
{
	void * res;

	wk->ptcName = GFL_PTC_Create( wk->ptcNameWork, PARTICLE_LIB_HEAP_SIZE, TRUE, HEAPID_DENDOU_DEMO );
	res = GFL_PTC_LoadArcResource( ARCID_DENDOU_DEMO_GRA, NARC_dendou_demo_gra_name_spa, HEAPID_DENDOU_DEMO );
	GFL_PTC_SetResource( wk->ptcName, res, TRUE, NULL );

	{	// 初期カメラを保存
		GFL_G3D_CAMERA * cmr = GFL_PTC_GetCameraPtr( wk->ptcName );
/*
		GFL_G3D_CAMERA_GetTop( cmr, &wk->ptcCamera[0] );
		GFL_G3D_CAMERA_GetBottom( cmr, &wk->ptcCamera[1] );
		GFL_G3D_CAMERA_GetLeft( cmr, &wk->ptcCamera[2] );
		GFL_G3D_CAMERA_GetRight( cmr, &wk->ptcCamera[3] );
*/
		GFL_G3D_CAMERA_GetfovySin( cmr, &wk->ptcCamera[0] );
		GFL_G3D_CAMERA_GetfovyCos( cmr, &wk->ptcCamera[1] );
		GFL_G3D_CAMERA_GetAspect( cmr, &wk->ptcCamera[2] );
		GFL_G3D_CAMERA_GetNear( cmr, &wk->ptcCamera[3] );
		GFL_G3D_CAMERA_GetFar( cmr, &wk->ptcCamera[4] );
	}
}

void DDEMOMAIN_DeleteNameParticle( DDEMOMAIN_WORK * wk )
{
	GFL_PTC_Delete( wk->ptcName );
	wk->ptcName = NULL;
}

void DDEMOMAIN_SetNameParticle( DDEMOMAIN_WORK * wk, u32 id )
{
	VecFx32	p = { 0, 0, 0 };
	GFL_PTC_CreateEmitter( wk->ptcName, id, &p );
}


static void CreateParticleCamera( DDEMOMAIN_WORK * wk, GFL_PTC_PTR ptc, BOOL disp )
{
	GFL_G3D_PROJECTION	projection;

	if( ptc == NULL ){ return; }

	// メイン画面（下）
	if( disp == TRUE ){
		projection.param1 = FX32_CONST(3);
		projection.param2 = -FX32_CONST(3);
	// サブ画面（上）
	}else{
		projection.param1 = FX32_CONST(9+2);
		projection.param2 = FX32_CONST(3+2);
	}
	projection.type = GFL_G3D_PRJORTH;
	projection.param3 = -FX32_CONST(4);
	projection.param4 = FX32_CONST(4);
	projection.near = 0;
	projection.far  = FX32_CONST( 1024 );	// 正射影なので関係ないが、念のためクリップのfarを設定
	projection.scaleW = 0;

	GFL_PTC_PersonalCameraDelete( ptc );
	GFL_PTC_PersonalCameraCreate( ptc, &projection, DEFAULT_PERSP_WAY, NULL, NULL, NULL, HEAPID_DENDOU_DEMO );
}

static void CreateNameParticleCamera( DDEMOMAIN_WORK * wk, GFL_PTC_PTR ptc )
{
	GFL_G3D_PROJECTION	projection;

	if( ptc == NULL ){ return; }

	projection.type = GFL_G3D_PRJPERS;
	projection.param1 = wk->ptcCamera[0];
	projection.param2 = wk->ptcCamera[1];
	projection.param3 = wk->ptcCamera[2];
	projection.param4 = 0;
	projection.near = wk->ptcCamera[3];
	projection.far  = wk->ptcCamera[4];
	projection.scaleW = 0;

	GFL_PTC_PersonalCameraDelete( ptc );
	GFL_PTC_PersonalCameraCreate( ptc, &projection, DEFAULT_PERSP_WAY, NULL, NULL, NULL, HEAPID_DENDOU_DEMO );
}


/*
static const VecFx32 sc_camera_pos = { 0x0, 0x70c2, 0x21ccf }; 
static const VecFx32 sc_camera_up =  { 0x0, 0x4272, 0x9f0 }; 
static const VecFx32 sc_camera_target = { 0x0, 0x519a, 0x800 }; 
*/
/*
static const VecFx32 sc_CAMERA_PER_POS    	= { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 30.0f ), FX_F32_TO_FX32( 3000.0f ) };
static const VecFx32 sc_CAMERA_PER_UP       = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32(  1.0f ), FX_F32_TO_FX32(    0.0f ) };
static const VecFx32 sc_CAMERA_PER_TARGET		= { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 30.0f ), FX_F32_TO_FX32(    0.0f ) };
*/

void DDEMOMAIN_InitMcss( DDEMOMAIN_WORK * wk )
{
	wk->mcss = MCSS_Init( 1, HEAPID_DENDOU_DEMO );

/*
	// g3dと並行させるためにずらし込む
	MCSS_SetTextureTransAdrs( wk->mcss, 0x30000 );
	MCSS_SetOrthoMode( wk->mcss );
*/
	MCSS_SetTextureTransAdrs( wk->mcss, 0 );
	MCSS_ResetOrthoMode( wk->mcss );
/*
	wk->mcssCamera = GFL_G3D_CAMERA_Create(
										GFL_G3D_PRJPERS, 
										FX_SinIdx( 26/2 *PERSPWAY_COEFFICIENT ),
										FX_CosIdx( 26/2 *PERSPWAY_COEFFICIENT ),
										defaultCameraAspect, 0,
										defaultCameraNear, defaultCameraFar, 0,
										&sc_camera_pos, &sc_camera_up, &sc_camera_target, HEAPID_DENDOU_DEMO );

	GFL_G3D_CAMERA_Switching( wk->mcssCamera );
*/
/*
	wk->mcssCamera = GFL_G3D_CAMERA_CreateOrtho(
										FX32_CONST(96), -FX32_CONST(96), -FX32_CONST(128), FX32_CONST(128),
										defaultCameraNear, defaultCameraFar, 0,
										&sc_CAMERA_PER_POS, &sc_CAMERA_PER_UP, &sc_CAMERA_PER_TARGET, HEAPID_DENDOU_DEMO );
*/
	{
    static const VecFx32 cam_pos = {FX32_CONST(-41.0f),
                                    FX32_CONST(  0.0f),
                                    FX32_CONST(101.0f)};
    static const VecFx32 cam_target = {FX32_CONST(0.0f),
                                       FX32_CONST(0.0f),
                                       FX32_CONST(-1.0f)};
    static const VecFx32 cam_up = {0,FX32_ONE,0};
/*
    //エッジマーキングカラー
    static  const GXRgb edge_color_table[8]=
      { GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 
        GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 
        GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 
        GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ) };
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K,
            0, work->heapId, NULL );
*/ 
    //正射影カメラ
    wk->mcssCamera =  GFL_G3D_CAMERA_Create( GFL_G3D_PRJORTH, 
                       FX32_ONE*192.0f,
                       0,
                       0,
                       FX32_ONE*256.0f,
                       (FX32_ONE),
                       (FX32_ONE*400),
                       NULL,
                       &cam_pos,
                       &cam_up,
                       &cam_target,
                       HEAPID_DENDOU_DEMO );
    
    GFL_G3D_CAMERA_Switching( wk->mcssCamera );
	}
}

void DDEMOMAIN_ExitMcss( DDEMOMAIN_WORK * wk )
{
	DDEMOMAIN_DelMcss( wk );
	GFL_G3D_CAMERA_Delete( wk->mcssCamera );
	MCSS_Exit( wk->mcss );
}

void DDEMOMAIN_AddMcss( DDEMOMAIN_WORK * wk )
{
	MCSS_ADD_WORK   add;
	POKEMON_PARAM * pp;
//	VecFx32 scale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE};
	VecFx32 scale = { FX_F32_TO_FX32(16), FX_F32_TO_FX32(16), FX32_ONE };
//	VecFx32 scale = { FX32_ONE, FX32_ONE, FX32_ONE };

	pp = PokeParty_GetMemberPointer( wk->dat->party, wk->pokePos );

	MCSS_TOOL_MakeMAWPP( pp, &add, MCSS_DIR_FRONT );

//	wk->mcssWork = MCSS_Add( wk->mcss, -FX32_CONST(14), -FX32_CONST(11.5), FX32_ONE, &add );
	wk->mcssWork = MCSS_Add( wk->mcss, FX32_CONST(256+48), FX32_CONST(48), 0, &add );
//	wk->mcssWork = MCSS_Add( wk->mcss, 0, 0, 0, &add );
	MCSS_SetScale( wk->mcssWork, &scale );
	MCSS_SetAnmStopFlag( wk->mcssWork );
}

void DDEMOMAIN_DelMcss( DDEMOMAIN_WORK * wk )
{
	if( wk->mcssWork != NULL ){
		MCSS_Del( wk->mcss, wk->mcssWork );
		wk->mcssWork = NULL;
	}
}

void DDEMOMAIN_MoveMcss( DDEMOMAIN_WORK * wk, s16 mv )
{
	VecFx32	pos;

	MCSS_GetPosition( wk->mcssWork, &pos );
	pos.x += FX32_CONST(mv);
	MCSS_SetPosition( wk->mcssWork, &pos );
}

static void McssCallBackFrame( u32 data, fx32 currentFrame )
{
	DDEMOMAIN_WORK * wk = (DDEMOMAIN_WORK *)data;

	MCSS_SetAnmStopFlag( wk->mcssWork );
	wk->mcssAnmEndFlg = TRUE;
}

void DDEMOMAIN_SetMcssCallBack( DDEMOMAIN_WORK * wk )
{
	wk->mcssAnmEndFlg = FALSE;
	MCSS_SetAnimCtrlCallBack( wk->mcssWork, (u32)wk, McssCallBackFrame, 0 );
}
