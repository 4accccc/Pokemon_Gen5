//============================================================================================
/**
 * @file		dpc_main.c
 * @brief		殿堂入り確認画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 *
 *	モジュール名：DPCMAIN
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "gamesystem/game_data.h"
#include "system/gfl_use.h"
#include "font/font.naix"
#include "msg/msg_pc_dendou.h"

#include "dpc_main.h"
#include "dendou_pc_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	EXP_BUF_SIZE	( 1024 )	// テンポラリメッセージバッファサイズ

#define	NAME_BUF_SIZE	( 32 )		// ニックネーム＆親名のバッファサイズ

#define	BLEND_EV1		( 16 )			// ブレンド係数
#define	BLEND_EV2		( 8 )				// ブレンド係数


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void VBlankTask( GFL_TCB * tcb, void * work );


//============================================================================================
//	グローバル
//============================================================================================

// VRAM割り振り
static const GFL_DISP_VRAM VramTbl = {
	GX_VRAM_BG_128_A,							// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,				// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_128_D,					// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,		// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_NONE,							// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,					// テクスチャパレットスロット

	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_128K		// サブOBJマッピングモード
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_InitVBlank( DPCMAIN_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_ExitVBlank( DPCMAIN_WORK * wk )
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
static void VBlankTask( GFL_TCB * tcb, void * work )
{
	DPCMAIN_WORK * wk;
	u32	i;

	wk = work;

	GFL_BG_VBlankFunc();
	GFL_CLACT_SYS_VBlankFunc();

	PaletteFadeTrans( wk->pfd );

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_InitVram(void)
{
	GFL_DISP_ClearVRAM( 0 );
	GFL_DISP_SetBank( &VramTbl );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM設定取得
 *
 * @param		none
 *
 * @return	VRAM設定データ
 */
//--------------------------------------------------------------------------------------------
const GFL_DISP_VRAM * DPCMAIN_GetVramBankData(void)
{
	return &VramTbl;
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
void DPCMAIN_InitBg(void)
{
	GFL_BG_Init( HEAPID_DENDOU_PC );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：スポットライト
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：タイトル背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_TEXT );
	}

	{	// サブ画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// サブ画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
	}

	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1, VISIBLE_ON );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG解放
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

	GFL_BG_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGグラフィック読み込み
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_LoadBgGraphic(void)
{
	ARCHANDLE * ah;

	ah = GFL_ARC_OpenDataHandle( ARCID_DENDOU_PC_GRA, HEAPID_DENDOU_PC_L );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_dendou_pc_gra_bgd_NCLR, PALTYPE_MAIN_BG, 0, 0x20*4, HEAPID_DENDOU_PC );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_dendou_pc_gra_bgu_NCLR, PALTYPE_SUB_BG, 0, 0x20*2, HEAPID_DENDOU_PC );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_dendou_pc_gra_bgd_lz_NCGR, GFL_BG_FRAME0_M, 0, 0, TRUE, HEAPID_DENDOU_PC );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_dendou_pc_gra_bgu_lz_NCGR, GFL_BG_FRAME0_S, 0, 0, TRUE, HEAPID_DENDOU_PC );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_dendou_pc_gra_bgd_base_lz_NSCR, GFL_BG_FRAME0_M, 0, 0, TRUE, HEAPID_DENDOU_PC );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_dendou_pc_gra_bgd_light_lz_NSCR, GFL_BG_FRAME1_M, 0, 0, TRUE, HEAPID_DENDOU_PC );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_dendou_pc_gra_bgd_title_lz_NSCR, GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_DENDOU_PC );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_dendou_pc_gra_bgu_base_lz_NSCR, GFL_BG_FRAME0_S, 0, 0, TRUE, HEAPID_DENDOU_PC );

	GFL_ARC_CloseDataHandle( ah );

	{	// タッチバーコピー
		u16 * scrn = GFL_BG_GetScreenBufferAdrs( GFL_BG_FRAME0_M );
		GFL_BG_WriteScreen( GFL_BG_FRAME3_M, &scrn[21*32], 0, 21, 32, 3 );
//		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
	}

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
		DPCMAIN_MBG_PAL_FONT*0x20, 0x20, HEAPID_DENDOU_PC );
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
		DPCMAIN_SBG_PAL_FONT*0x20, 0x20, HEAPID_DENDOU_PC );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード初期化
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_InitPaletteFade( DPCMAIN_WORK * wk )
{
	wk->pfd = PaletteFadeInit( HEAPID_DENDOU_PC );
	PaletteFadeWorkAllocSet( wk->pfd, FADE_MAIN_OBJ, FADE_PAL_ALL_SIZE, HEAPID_DENDOU_PC );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード解放
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_ExitPaletteFade( DPCMAIN_WORK * wk )
{
	PaletteFadeWorkAllocFree( wk->pfd, FADE_MAIN_OBJ );
	PaletteFadeFree( wk->pfd );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェードリクエスト
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_RequestPaletteFade( DPCMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<6; i++ ){
		ColorConceChangePfd( wk->pfd, FADE_MAIN_OBJ, ((1<<i)|(1<<(i+6))), wk->nowEvy[i], 0 );
	}
	PaletteTrans_AutoSet( wk->pfd, 1 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェードチェック
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL DPCMAIN_CheckPaletteFade( DPCMAIN_WORK * wk )
{
	if( PaletteFadeCheck( wk->pfd ) != 0 ){
		return TRUE;
	}
	PaletteTrans_AutoSet( wk->pfd, 0 );
	return FALSE;
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
void DPCMAIN_SetBlendAlpha(void)
{
	G2_SetBlendAlpha(
		GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2,
		GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ,
		BLEND_EV1, BLEND_EV2 );
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
void DPCMAIN_InitMsg( DPCMAIN_WORK * wk )
{
	wk->mman = GFL_MSG_Create(
							GFL_MSG_LOAD_NORMAL,
							ARCID_MESSAGE, NARC_message_pc_dendou_dat, HEAPID_DENDOU_PC );
	wk->font = GFL_FONT_Create(
							ARCID_FONT, NARC_font_large_gftr,
							GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_DENDOU_PC );
  wk->nfnt = GFL_FONT_Create(
							ARCID_FONT, NARC_font_num_gftr,
							GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_DENDOU_PC );
	wk->wset = WORDSET_Create( HEAPID_DENDOU_PC );
	wk->que  = PRINTSYS_QUE_Create( HEAPID_DENDOU_PC );
	wk->exp  = GFL_STR_CreateBuffer( EXP_BUF_SIZE, HEAPID_DENDOU_PC );

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
void DPCMAIN_ExitMsg( DPCMAIN_WORK * wk )
{
	GFL_STR_DeleteBuffer( wk->exp );
	PRINTSYS_QUE_Delete( wk->que );
	WORDSET_Delete( wk->wset );
	GFL_FONT_Delete( wk->nfnt );
	GFL_FONT_Delete( wk->font );
	GFL_MSG_Delete( wk->mman );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  ポケモンデータ作成
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_CreatePokeData( DPCMAIN_WORK * wk )
{
	SAVE_CONTROL_WORK * sv;
	DENDOU_RECORD * rec;
	DENDOU_SAVEDATA * ex_rec;
	u16	i, j;

	// 文字列バッファ確保
	for( i=0; i<DENDOU_RECORD_MAX; i++ ){
		for( j=0; j<6; j++ ){
			wk->party[i].dat[j].nickname = GFL_STR_CreateBuffer( NAME_BUF_SIZE, HEAPID_DENDOU_PC );
			wk->party[i].dat[j].oyaname  = GFL_STR_CreateBuffer( NAME_BUF_SIZE, HEAPID_DENDOU_PC );
		}
	}

	// 初回データ
	if( wk->dat->callMode == DENDOUPC_CALL_CLEAR ){
		rec = GAMEDATA_GetDendouRecord( wk->dat->gamedata );
		wk->party[0].pokeMax = DendouRecord_GetPokemonCount( rec );
		DendouRecord_GetDate( rec, &wk->party[0].date );
		for( i=0; i<wk->party[0].pokeMax; i++ ){
			DendouRecord_GetPokemonData( rec, i, &wk->party[0].dat[i] );
		}
		wk->pageMax = 1;
	// 殿堂入りデータ（外部セーブデータ）
	}else{
		sv = GAMEDATA_GetSaveControlWork( wk->dat->gamedata );
		if( SaveControl_Extra_Load( sv, SAVE_EXTRA_ID_DENDOU, HEAPID_DENDOU_PC ) == LOAD_RESULT_OK ){
			ex_rec = SaveControl_Extra_DataPtrGet( sv, SAVE_EXTRA_ID_DENDOU, 0 );
			if( ex_rec != NULL ){
				for( i=0; i<DENDOU_RECORD_MAX; i++ ){
					wk->party[i].pokeMax = DendouData_GetPokemonCount( ex_rec, i );
					wk->party[i].recNo   = DendouData_GetRecordNumber( ex_rec, i );
					DendouData_GetDate( ex_rec, i, &wk->party[i].date );
					if( wk->party[i].pokeMax != 0 ){
						for( j=0; j<wk->party[i].pokeMax; j++ ){
							DendouData_GetPokemonData( ex_rec, i, j, &wk->party[i].dat[j] );
						}
						wk->pageMax++;
					}
				}
			}
		}
		SaveControl_Extra_Unload( sv, SAVE_EXTRA_ID_DENDOU );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  ポケモンデータ解放
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_ExitPokeData( DPCMAIN_WORK * wk )
{
	u16	i, j;

	// 文字列バッファ削除
	for( i=0; i<DENDOU_RECORD_MAX; i++ ){
		for( j=0; j<6; j++ ){
			GFL_STR_DeleteBuffer( wk->party[i].dat[j].nickname );
			GFL_STR_DeleteBuffer( wk->party[i].dat[j].oyaname );
		}
	}
}
