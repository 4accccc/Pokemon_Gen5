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

static void CreateParticleCamera( DDEMOMAIN_WORK * wk, GFL_PTC_PTR ptc, BOOL disp );


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
	SEQ_SE_ROTATION_B,
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定（シーン１）
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定（シーン２）
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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
 * @brief		VBLANK処理（シーン１）
 *
 * @param		tcb		GFL_TCB
 * @param		wk		殿堂入りデモ画面ワーク
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理（シーン２）
 *
 * @param		tcb		GFL_TCB
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void Scene2_VBlankTask( GFL_TCB * tcb, void * work )
{
	GFL_BG_VBlankFunc();
	GFL_CLACT_SYS_VBlankFunc();
//	GFL_G3D_DOUBLE3D_VblankIntr();
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM初期化
 *
 * @param		scene		シーン
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_InitVram( u32 scene )
{
	GFL_DISP_ClearVRAM( 0 );
	if( scene == 0 ){
		GFL_DISP_SetBank( &VramTblScene1 );
	}else{
		GFL_DISP_SetBank( &VramTblScene2 );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM設定取得
 *
 * @param		scene		シーン
 *
 * @return	VRAM設定
 */
//--------------------------------------------------------------------------------------------
const GFL_DISP_VRAM * DDEMOMAIN_GetVramBankData( u32 scene )
{
	if( scene == 0 ){
		return &VramTblScene1;
	}
	return &VramTblScene2;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_InitBg(void)
{
	GFL_BG_Init( HEAPID_DENDOU_DEMO );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG削除
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_ExitBg(void)
{
	GFL_BG_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGモード初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_InitBgMode(void)
{
	GFL_BG_SYS_HEADER sysh = {
		GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BGMODE_3, GX_BG0_AS_3D,
	};
	GFL_BG_SetBGMode( &sysh );

	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0 );
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGフレーム設定（シーン２）
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

	GFL_BG_SetScrollReq( GFL_BG_FRAME3_M, GFL_BG_SCROLL_X_SET, 160 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME3_M, GFL_BG_SCROLL_Y_SET, 112 );

	GFL_BG_SetScrollReq( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_SET, 160 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, -152 );

	GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_M, GFL_BG_CENTER_X_SET, 160 );
	GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_M, GFL_BG_CENTER_Y_SET, 160 );
	GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_S, GFL_BG_CENTER_X_SET, 160 );
	GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_S, GFL_BG_CENTER_Y_SET, 160 );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );

	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0 );
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGフレーム削除（シーン２）
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_ExitScene2BgFrame(void)
{
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG読み込み（シーン２）
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

	GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		アルファブレンド設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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
 * @param		wk		殿堂入りデモ画面ワーク
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
	wk->exp  = GFL_STR_CreateBuffer( EXP_BUF_SIZE, HEAPID_DENDOU_DEMO );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連解放
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_ExitMsg( DDEMOMAIN_WORK * wk )
{
	GFL_STR_DeleteBuffer( wk->exp );
	WORDSET_Delete( wk->wset );
	GFL_FONT_Delete( wk->nfnt );
	GFL_FONT_Delete( wk->font );
	GFL_MSG_Delete( wk->mman );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  サウンド初期設定
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_InitSound( DDEMOMAIN_WORK * wk )
{
	wk->sndHandle = SOUNDMAN_PresetSoundTbl( SndTbl, NELEMS(SndTbl) );
	PMSND_PlayBGM( SEQ_BGM_E_DENDOUIRI );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  サウンド削除
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_ExitSound( DDEMOMAIN_WORK * wk )
{
	SOUNDMAN_ReleasePresetData( wk->sndHandle );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  ポケモン最大数設定
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_GetPokeMax( DDEMOMAIN_WORK * wk )
{
	POKEMON_PARAM * pp;
	BOOL	fast;
	u32	i;

	wk->pokeMax = 0;
	for( i=0; i<PokeParty_GetPokeCount(wk->dat->party); i++ ){
		pp = PokeParty_GetMemberPointer( wk->dat->party, i );
		fast = PP_FastModeOn( pp );
		if( PP_Get( pp, ID_PARA_tamago_flag, NULL ) == 0 ){
			wk->pp[wk->pokeMax] = pp;
			wk->pokeMax++;
		}
		PP_FastModeOff( pp, fast );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  ポケモンデータ取得
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_GetPokeData( DDEMOMAIN_WORK * wk )
{
	POKEMON_PARAM * pp;
	BOOL	fast;
	
	pp = wk->pp[wk->pokePos];

	fast = PP_FastModeOn( pp );

	wk->monsno = PP_Get( pp, ID_PARA_monsno, NULL );
	wk->type   = PP_Get( pp, ID_PARA_type1, NULL );
	wk->form   = PP_Get( pp, ID_PARA_form_no, NULL );

	PP_FastModeOff( pp, fast );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  鳴き声読み込み
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_LoadPokeVoice( DDEMOMAIN_WORK * wk )
{
	if( wk->monsno == MONSNO_PERAPPU ){
#if 1
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


//============================================================================================
//	3D
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		セットアップ
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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
			{ { 0, -FX16_ONE, 0 }, GX_RGB( 16,16,16), },
			{ { 0, FX16_ONE, 0 }, GX_RGB( 16,16,16), },
			{ { 0, -FX16_ONE, 0 }, GX_RGB( 16,16,16), },
			{ { 0, -FX16_ONE, 0 }, GX_RGB( 16,16,16), },
		};
		int i;
		
		for( i=0; i<NELEMS(sc_GFL_G3D_LIGHT); i++ ){
			GFL_G3D_SetSystemLight( i, &sc_GFL_G3D_LIGHT[i] );
		}
	}

	//レンダリングスワップバッファ
	GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		３Ｄ初期化
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_Init3D( DDEMOMAIN_WORK * wk )
{
	// ３Ｄシステム起動
	GFL_G3D_Init(
		GFL_G3D_VMANLNK,		// テクスチャマネージャ使用モード（フレームモード（とりっぱなし））
		GFL_G3D_TEX128K,		// テクスチャマネージャサイズ 128KB(1ブロック)
		GFL_G3D_VMANLNK,		// パレットマネージャ使用モード（フレームモード（とりっぱなし））
		GFL_G3D_PLT16K,			// パレットマネージャサイズ
		DTCM_SIZE,					// ジオメトリバッファの使用サイズ
		HEAPID_DENDOU_DEMO,	// ヒープID
		SetUp3D );					// セットアップ関数(NULLの時はDefaultSetUp)

	// ハンドル作成
	wk->g3d_util  = GFL_G3D_UTIL_Create( 32, 32, HEAPID_DENDOU_DEMO );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		３Ｄ削除
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_Exit3D( DDEMOMAIN_WORK * wk )
{
	GFL_G3D_UTIL_Delete( wk->g3d_util );
	GFL_G3D_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		３Ｄメイン処理（通常）
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_Main3D( DDEMOMAIN_WORK * wk )
{
	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();

	MCSS_Main( wk->mcss );
	MCSS_Draw( wk->mcss );

	GFL_G3D_DRAW_End();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		３Ｄメイン処理（２画面３Ｄ用）
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_MainDouble3D( DDEMOMAIN_WORK * wk )
{
	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();

	if( GFL_G3D_DOUBLE3D_GetFlip() == TRUE ){
		CreateParticleCamera( wk, wk->ptc, TRUE );
		CreateParticleCamera( wk, wk->ptcName, TRUE );
	}else{
		CreateParticleCamera( wk, wk->ptc, FALSE );
		CreateParticleCamera( wk, wk->ptcName, FALSE );
	}
	GFL_PTC_Main();

	GFL_G3D_DRAW_End();
	GFL_G3D_DOUBLE3D_SetSwapFlag();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		２画面３Ｄ初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_InitDouble3D(void)
{
	GFL_G3D_DOUBLE3D_Init( HEAPID_DENDOU_DEMO );	// 2画面3D初期化
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		２画面３Ｄ削除
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_ExitDouble3D(void)
{
	GFL_G3D_DOUBLE3D_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		パーティクル初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_InitParticle(void)
{
	GFL_PTC_Init( HEAPID_DENDOU_DEMO );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		パーティクル削除
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_ExitParticle(void)
{
	GFL_PTC_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイプ別パーティクル作成
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_CreateTypeParticle( DDEMOMAIN_WORK * wk )
{
	void * res;

	wk->ptc = GFL_PTC_Create( wk->ptcWork, PARTICLE_LIB_HEAP_SIZE, TRUE, HEAPID_DENDOU_DEMO );
	res = GFL_PTC_LoadArcResource( ARCID_DENDOU_DEMO_GRA, TypeArcTbl[wk->type], HEAPID_DENDOU_DEMO );
	GFL_PTC_SetResource( wk->ptc, res, TRUE, NULL );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイプ別パーティクル削除
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_DeleteTypeParticle( DDEMOMAIN_WORK * wk )
{
	GFL_PTC_Delete( wk->ptc );
	wk->ptc = NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイプ別パーティクル追加
 *
 * @param		wk		殿堂入りデモ画面ワーク
 * @param		id		パーティクルＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_SetTypeParticle( DDEMOMAIN_WORK * wk, u32 id )
{
	VecFx32	p = { 0, 0, FX32_CONST(0.1f) };
	GFL_PTC_CreateEmitter( wk->ptc, id, &p );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		名前エフェクトパーティクル作成
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_CreateNameParticle( DDEMOMAIN_WORK * wk )
{
	void * res;

	wk->ptcName = GFL_PTC_Create( wk->ptcNameWork, PARTICLE_LIB_HEAP_SIZE, TRUE, HEAPID_DENDOU_DEMO );
	res = GFL_PTC_LoadArcResource( ARCID_DENDOU_DEMO_GRA, NARC_dendou_demo_gra_name_spa, HEAPID_DENDOU_DEMO );
	GFL_PTC_SetResource( wk->ptcName, res, TRUE, NULL );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		名前エフェクトパーティクル削除
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_DeleteNameParticle( DDEMOMAIN_WORK * wk )
{
	GFL_PTC_Delete( wk->ptcName );
	wk->ptcName = NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		名前エフェクトパーティクル追加
 *
 * @param		wk		殿堂入りデモ画面ワーク
 * @param		id		パーティクルＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_SetNameParticle( DDEMOMAIN_WORK * wk, u32 id )
{
	VecFx32	p = { 0, 0, 0 };
	GFL_PTC_CreateEmitter( wk->ptcName, id, &p );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		パーティクルカメラ作成
 *
 * @param		wk		殿堂入りデモ画面ワーク
 * @param		ptc		パーティクル
 * @param		disp	TRUE = メイン画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
#define	PARTICLE_CAMERA_PV		( 4.0 )
#define	PARTICLE_CAMERA_PH		( 5.33 )

static void CreateParticleCamera( DDEMOMAIN_WORK * wk, GFL_PTC_PTR ptc, BOOL disp )
{
	GFL_G3D_PROJECTION	projection;

	if( ptc == NULL ){ return; }
/*
	// メイン画面（下）
	if( disp == TRUE ){
		projection.param1 = FX32_CONST(3.975);
		projection.param2 = -FX32_CONST(3.975);
	// サブ画面（上）
	}else{
		projection.param1 = FX32_CONST(11.925+2);
		projection.param2 = FX32_CONST(3.975+2);
	}
	projection.type = GFL_G3D_PRJORTH;
	projection.param3 = -FX32_CONST(5.3);
	projection.param4 = FX32_CONST(5.3);
*/
	// メイン画面（下）
	if( disp == TRUE ){
		projection.param1 = FX32_CONST(PARTICLE_CAMERA_PV);
		projection.param2 = -FX32_CONST(PARTICLE_CAMERA_PV);
	// サブ画面（上）
	}else{
		projection.param1 = FX32_CONST(PARTICLE_CAMERA_PV+PARTICLE_CAMERA_PV*2+2);
		projection.param2 = FX32_CONST(PARTICLE_CAMERA_PV+2);
	}
	projection.type = GFL_G3D_PRJORTH;
	projection.param3 = -FX32_CONST(PARTICLE_CAMERA_PH);
	projection.param4 = FX32_CONST(PARTICLE_CAMERA_PH);
	projection.near = 0;
	projection.far  = FX32_CONST( 1024 );	// 正射影なので関係ないが、念のためクリップのfarを設定
	projection.scaleW = 0;

	GFL_PTC_PersonalCameraDelete( ptc );
	GFL_PTC_PersonalCameraCreate( ptc, &projection, DEFAULT_PERSP_WAY, NULL, NULL, NULL, HEAPID_DENDOU_DEMO );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		MCSS初期化
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_InitMcss( DDEMOMAIN_WORK * wk )
{
	wk->mcss = MCSS_Init( 1, HEAPID_DENDOU_DEMO );

	MCSS_SetTextureTransAdrs( wk->mcss, 0 );
	MCSS_SetOrthoMode( wk->mcss );

	{
    static const VecFx32 cam_pos = { FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(30.0f), FX_F32_TO_FX32(3000.0f) };
    static const VecFx32 cam_target = { FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(1.0f), FX_F32_TO_FX32(0.0f) };
    static const VecFx32 cam_up = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32(30.0f), FX_F32_TO_FX32(0.0f) };

    wk->mcssCamera = GFL_G3D_CAMERA_CreateOrtho(
											FX32_CONST(96),
											-FX32_CONST(96),
											-FX32_CONST(128),
											FX32_CONST(128),
											defaultCameraNear, defaultCameraFar, 0,
											&cam_pos, &cam_up, &cam_target,
                      HEAPID_DENDOU_DEMO );

    GFL_G3D_CAMERA_Switching( wk->mcssCamera );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		MCSS解放
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_ExitMcss( DDEMOMAIN_WORK * wk )
{
	DDEMOMAIN_DelMcss( wk );
	GFL_G3D_CAMERA_Delete( wk->mcssCamera );
	MCSS_Exit( wk->mcss );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		MCSS追加
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_AddMcss( DDEMOMAIN_WORK * wk )
{
	MCSS_ADD_WORK   add;
	POKEMON_PARAM * pp;
	VecFx32 scale = { FX_F32_TO_FX32(16), FX_F32_TO_FX32(16), FX32_ONE };

	pp = wk->pp[wk->pokePos];

	MCSS_TOOL_MakeMAWPP( pp, &add, MCSS_DIR_FRONT );

	wk->mcssWork = MCSS_Add( wk->mcss, FX32_CONST(128+48), FX32_CONST(-96+48), 0, &add );
	MCSS_SetScale( wk->mcssWork, &scale );
	MCSS_SetAnmStopFlag( wk->mcssWork );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		MCSS削除
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_DelMcss( DDEMOMAIN_WORK * wk )
{
	if( wk->mcssWork != NULL ){
		MCSS_Del( wk->mcss, wk->mcssWork );
		wk->mcssWork = NULL;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		MCSS移動
 *
 * @param		wk		殿堂入りデモ画面ワーク
 * @param		mv		移動値
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_MoveMcss( DDEMOMAIN_WORK * wk, s16 mv )
{
	VecFx32	pos;

	MCSS_GetPosition( wk->mcssWork, &pos );
	pos.x += FX32_CONST(mv);
	MCSS_SetPosition( wk->mcssWork, &pos );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		MCSSコールバック関数
 *
 * @param		data
 * @param		currentFrame
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void McssCallBackFrame( u32 data, fx32 currentFrame )
{
	DDEMOMAIN_WORK * wk = (DDEMOMAIN_WORK *)data;

	MCSS_SetAnmStopFlag( wk->mcssWork );
	wk->mcssAnmEndFlg = TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		MCSSコールバック関数セット
 *
 * @param		wk		殿堂入りデモ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DDEMOMAIN_SetMcssCallBack( DDEMOMAIN_WORK * wk )
{
	wk->mcssAnmEndFlg = FALSE;
	MCSS_SetAnimCtrlCallBack( wk->mcssWork, (u32)wk, McssCallBackFrame, 0 );
}
