#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "system/main.h"
#include "system/wipe.h"
#include "system/gfl_use.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "msg/msg_d_nakahiro.h"
#include "font/font.naix"
#include "pokeicon/pokeicon.h"
#include "test/performance.h"

#include "pokeicon_view.h"



//============================================================================================
//============================================================================================

// ポケモンアイコン表示数
#define	POKEPUT_MAX_M		( 8 )
#define	POKEPUT_MAX_S		( 12 )
#define	POKEPUT_MAX_ALL	( POKEPUT_MAX_M+POKEPUT_MAX_S )

// メインシーケンス
enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_MAIN,
	MAINSEQ_RELEASE,
	MAINSEQ_END,
};

// BMPWIN
enum {
	BMPWIN_INFO = 0,
	BMPWIN_NAME_M,
	BMPWIN_NAME_S,
	BMPWIN_MAX
};

// OBJ ID
enum {
	OBJ_ID_POKE = 0,
	OBJ_ID_MAX = OBJ_ID_POKE + POKEPUT_MAX_ALL,
};

// ワーク
typedef struct {
	GFL_TCB * vtask;					// TCB ( VBLANK )

	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[OBJ_ID_MAX];
	u32	chrRes[POKEPUT_MAX_ALL];
	u32	palRes[2];
	u32	celRes[2];

	GFL_FONT * font;					// 通常フォント
	GFL_MSGDATA * mman;				// メッセージデータマネージャ
	WORDSET * wset;						// 単語セット
	STRBUF * exp;
	PRINT_QUE * que;					// プリントキュー
	PRINT_UTIL	util[BMPWIN_MAX];

	s8	page;
	u8	sex;
	u8	form;
	u8	bgcol;
	BOOL	anmFlag;

	u16	sexTbl[POKEPUT_MAX_ALL];
	u16	formTbl[POKEPUT_MAX_ALL];

	int	next_seq;
}PIVIEW_WORK;

#define	FCOL_MP15BN		( PRINTSYS_LSB_Make(1,2,0) )
#define	FCOL_MP15BW		( PRINTSYS_LSB_Make(1,2,15) )
#define	FCOL_MP15BLW	( PRINTSYS_LSB_Make(5,6,15) )
#define	FCOL_MP15RW		( PRINTSYS_LSB_Make(3,4,15) )



//============================================================================================
//============================================================================================
static GFL_PROC_RESULT Proc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT Proc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT Proc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

static int MainSeq_Init( PIVIEW_WORK * wk );
static int MainSeq_Main( PIVIEW_WORK * wk );
static int MainSeq_Release( PIVIEW_WORK * wk );

static void InitVram(void);
static void InitBg(void);
static void ExitBg(void);
static void LoadBgGraphic(void);
static void InitMsg( PIVIEW_WORK * wk );
static void ExitMsg( PIVIEW_WORK * wk );
static void InitVBlank( PIVIEW_WORK * wk );
static void ExitVBlank( PIVIEW_WORK * wk );
static void FadeInSet(void);
static void FadeOutSet(void);

static void InitBmp( PIVIEW_WORK * wk );
static void ExitBmp( PIVIEW_WORK * wk );
static void MainBmpPrint( PIVIEW_WORK * wk );
static void PutStrForm( PIVIEW_WORK * wk );
static void PutStrSex( PIVIEW_WORK * wk );
static void PutStrBGColor( PIVIEW_WORK * wk );
static void PutPokeName( PIVIEW_WORK * wk, u32 num, u32 winIdx, u32 px, u32 py );
static void PutPokeNameAll( PIVIEW_WORK * wk );

static void InitObj( PIVIEW_WORK * wk );
static void ExitObj( PIVIEW_WORK * wk );
static void ChangePokeIconAll( PIVIEW_WORK * wk );

static void SetBgColor( u8 pos );
static void ChangeBgColor( PIVIEW_WORK * wk );
static void ChangePage( PIVIEW_WORK * wk, s8 mv );
static void ChangeAnm( PIVIEW_WORK * wk );
static void ChangeForm( PIVIEW_WORK * wk );
static void ChangeSex( PIVIEW_WORK * wk );
static void MakePokeTbl( PIVIEW_WORK * wk );


//============================================================================================
//============================================================================================

// PROCデータ
const GFL_PROC_DATA POKEICONVIEW_ProcData = {
	Proc_Init,
	Proc_Main,
	Proc_End
};

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

static const u16 BGColorTbl[] = {
	0x7fff,		// 白
	0x001f,		// 赤
	0x03E0,		// 緑
	0x7c00,		// 青
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：初期化
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT Proc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	PIVIEW_WORK * wk = GFL_PROC_AllocWork( proc, sizeof(PIVIEW_WORK), HEAPID_NAKAHIRO_DEBUG );
  GFL_STD_MemClear( wk, sizeof(PIVIEW_WORK) );
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：メイン
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT Proc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	PIVIEW_WORK * wk = mywk;

	switch( *seq ){
	case MAINSEQ_INIT:
		*seq = MainSeq_Init( wk );
		break;
	case MAINSEQ_MAIN:
		*seq = MainSeq_Main( wk );
		break;
	case MAINSEQ_RELEASE:
		*seq = MainSeq_Release( wk );
		break;
	}

	if( *seq == MAINSEQ_END ){
		return GFL_PROC_RES_FINISH;
	}

	MainBmpPrint( wk );
	GFL_CLACT_SYS_Main();

	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：終了
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT Proc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork( proc );
	return GFL_PROC_RES_FINISH;
}

//============================================================================================
//	シーケンス
//============================================================================================
static int MainSeq_Init( PIVIEW_WORK * wk )
{
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// サブ画面をメインに
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN);

	// 負荷情報を非表示に
	DEBUG_PerformanceSetActive( FALSE );

	InitVram();
	InitBg();
	LoadBgGraphic();
	InitMsg( wk );

	InitBmp( wk );
	InitObj( wk );

	InitVBlank( wk );

	FadeInSet();
	return MAINSEQ_MAIN;
}

static int MainSeq_Main( PIVIEW_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == FALSE ){
		return MAINSEQ_MAIN;
	}

/*
	・上：－１
	・下：＋１
	・左：－１０
	・右：＋１０
	・Ｌ：フォルム切り替え
	・Ｒ：性別切り替え
	・Ｙ：背景色切り替え
*/
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		ChangePage( wk, -1 );
		return MAINSEQ_MAIN;
	}
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
		ChangePage( wk, 1 );
		return MAINSEQ_MAIN;
	}
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		ChangePage( wk, -10 );
		return MAINSEQ_MAIN;
	}
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		ChangePage( wk, 10 );
		return MAINSEQ_MAIN;
	}

	// フォルム切り替え
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
		ChangeForm( wk );
		return MAINSEQ_MAIN;
	}

	// 性別切り替え
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
		ChangeSex( wk );
		return MAINSEQ_MAIN;
	}

	// バックグラウンドカラー変更
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
		ChangeBgColor( wk );
		return MAINSEQ_MAIN;
	}

	// アニメ切り替え
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
		ChangeAnm( wk );
		return MAINSEQ_MAIN;
	}

	// 終了
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		FadeOutSet();
		return MAINSEQ_RELEASE;
	}

	return MAINSEQ_MAIN;
}

static int MainSeq_Release( PIVIEW_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == FALSE ){
		return MAINSEQ_RELEASE;
	}

	ExitVBlank( wk );

	ExitObj( wk );
	ExitBmp( wk );

	ExitMsg( wk );
	ExitBg();

	// 負荷情報を表示に
	DEBUG_PerformanceSetActive( TRUE );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	return MAINSEQ_END;
}


//============================================================================================
//	初期化
//============================================================================================

static void InitVram(void)
{
	GFL_DISP_ClearVRAM( NULL );
	GFL_DISP_SetBank( &VramTbl );
}

static void InitBg(void)
{
	GFL_BG_Init( HEAPID_NAKAHIRO_DEBUG );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：メッセージウィンドウ
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
	}

	{	// サブ画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &cnth, GFL_BG_MODE_TEXT );
	}

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
}

static void ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

	GFL_BG_Exit();
}

static void LoadBgGraphic(void)
{
	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
		15*0x20, 0x20, HEAPID_NAKAHIRO_DEBUG );

	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
		15*0x20, 0x20, HEAPID_NAKAHIRO_DEBUG );

	// 背景カラー
	SetBgColor( 0 );
}

static void InitMsg( PIVIEW_WORK * wk )
{
	wk->mman = GFL_MSG_Create(
							GFL_MSG_LOAD_NORMAL,
							ARCID_MESSAGE, NARC_message_d_nakahiro_dat, HEAPID_NAKAHIRO_DEBUG );
	wk->font = GFL_FONT_Create(
							ARCID_FONT, NARC_font_large_gftr,
							GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_NAKAHIRO_DEBUG );
	wk->wset = WORDSET_Create( HEAPID_NAKAHIRO_DEBUG );
	wk->que  = PRINTSYS_QUE_Create( HEAPID_NAKAHIRO_DEBUG );
	wk->exp  = GFL_STR_CreateBuffer( 128, HEAPID_NAKAHIRO_DEBUG );
}

static void ExitMsg( PIVIEW_WORK * wk )
{
	GFL_STR_DeleteBuffer( wk->exp );
	PRINTSYS_QUE_Delete( wk->que );
	WORDSET_Delete( wk->wset );
	GFL_FONT_Delete( wk->font );
	GFL_MSG_Delete( wk->mman );
}

static void VBlankTask( GFL_TCB * tcb, void * work )
{
	GFL_BG_VBlankFunc();
	GFL_CLACT_SYS_VBlankFunc();

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

static void InitVBlank( PIVIEW_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

static void ExitVBlank( PIVIEW_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}


static void FadeInSet(void)
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_NAKAHIRO_DEBUG );
}

static void FadeOutSet(void)
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_NAKAHIRO_DEBUG );
}


//============================================================================================
//	ＢＭＰ
//============================================================================================
static void InitBmp( PIVIEW_WORK * wk )
{
	STRBUF * str;

	GFL_BMPWIN_Init( HEAPID_NAKAHIRO_DEBUG );

	wk->util[BMPWIN_INFO].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 0, 0, 32, 6, 15, GFL_BMP_CHRAREA_GET_B );
	wk->util[BMPWIN_NAME_M].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 0, 8, 32, 16, 15, GFL_BMP_CHRAREA_GET_B );
	wk->util[BMPWIN_NAME_S].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_S, 0, 0, 32, 24, 15, GFL_BMP_CHRAREA_GET_B );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->util[BMPWIN_INFO].win), 15 );

	str = GFL_MSG_CreateString( wk->mman, piv_str02 );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_INFO], wk->que, 0, 0, str, wk->font, FCOL_MP15BW );
	GFL_STR_DeleteBuffer( str );

	PutStrForm( wk );
	PutStrSex( wk );
	PutStrBGColor( wk );

	PutPokeNameAll( wk );

	GFL_BMPWIN_MakeScreen( wk->util[BMPWIN_INFO].win );
	GFL_BMPWIN_MakeScreen( wk->util[BMPWIN_NAME_M].win );
	GFL_BMPWIN_MakeScreen( wk->util[BMPWIN_NAME_S].win );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_M );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S );
}

static void ExitBmp( PIVIEW_WORK * wk )
{
	u32	i;

	for( i=0; i<BMPWIN_MAX; i++ ){
		GFL_BMPWIN_Delete( wk->util[i].win );
	}

	GFL_BMPWIN_Exit();
}

static void MainBmpPrint( PIVIEW_WORK * wk )
{
	u32	i;

	PRINTSYS_QUE_Main( wk->que );
	for( i=0; i<BMPWIN_MAX; i++ ){
		PRINT_UTIL_Trans( &wk->util[i], wk->que );
	}
}


static void PutStrForm( PIVIEW_WORK * wk )
{
	STRBUF * str;

	GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->util[BMPWIN_INFO].win), 0, 32, 128, 16, 15 );

	str = GFL_MSG_CreateString( wk->mman, piv_str03 );
	WORDSET_RegisterNumber( wk->wset, 0, wk->form, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_INFO], wk->que, 0, 32, wk->exp, wk->font, FCOL_MP15BW );
	GFL_STR_DeleteBuffer( str );
}

static void PutStrSex( PIVIEW_WORK * wk )
{
	STRBUF * str;

	GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->util[BMPWIN_INFO].win), 128, 32, 128, 16, 15 );

	str = GFL_MSG_CreateString( wk->mman, piv_str04 );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_INFO], wk->que, 128, 32, str, wk->font, FCOL_MP15BW );
	GFL_STR_DeleteBuffer( str );

	if( wk->sex == 0 ){
		str = GFL_MSG_CreateString( wk->mman, piv_str06 );
		PRINT_UTIL_PrintColor( &wk->util[BMPWIN_INFO], wk->que, 128+12*7, 32, str, wk->font, FCOL_MP15BLW );
		GFL_STR_DeleteBuffer( str );
	}else{
		str = GFL_MSG_CreateString( wk->mman, piv_str07 );
		PRINT_UTIL_PrintColor( &wk->util[BMPWIN_INFO], wk->que, 128+12*7, 32, str, wk->font, FCOL_MP15RW );
		GFL_STR_DeleteBuffer( str );
	}
}

static void PutStrBGColor( PIVIEW_WORK * wk )
{
	STRBUF * str;

	GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->util[BMPWIN_INFO].win), 0, 16, 256, 16, 15 );

	str = GFL_MSG_CreateString( wk->mman, piv_str05 );
	WORDSET_RegisterNumber( wk->wset, 0, wk->bgcol, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_INFO], wk->que, 0, 16, wk->exp, wk->font, FCOL_MP15BW );
	GFL_STR_DeleteBuffer( str );
}

static void PutPokeName( PIVIEW_WORK * wk, u32 num, u32 winIdx, u32 px, u32 py )
{
	STRBUF * str;

	if( num == 0 ){
		str = GFL_MSG_CreateString( wk->mman, piv_str01 );
		PRINT_UTIL_PrintColor( &wk->util[winIdx], wk->que, px, py, str, wk->font, FCOL_MP15BW );
		GFL_STR_DeleteBuffer( str );
	}else{
		str = GFL_MSG_CreateString( wk->mman, piv_str00 );
		WORDSET_RegisterNumber( wk->wset, 1, num, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
		WORDSET_RegisterPokeMonsNameNo( wk->wset, 0, num );
		WORDSET_ExpandStr( wk->wset, wk->exp, str );
		PRINT_UTIL_PrintColor( &wk->util[winIdx], wk->que, px, py, wk->exp, wk->font, FCOL_MP15BW );
		GFL_STR_DeleteBuffer( str );
	}
}

static void PutPokeNameAll( PIVIEW_WORK * wk )
{
	u32	pos;
	u32	i;

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->util[BMPWIN_NAME_M].win), 0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->util[BMPWIN_NAME_S].win), 0 );

	pos = wk->page * POKEPUT_MAX_ALL;

	// メイン
	for( i=0; i<POKEPUT_MAX_M; i++ ){
		if( pos > MONSNO_END ){ break; }
		PutPokeName( wk, pos, BMPWIN_NAME_M, 32+128*(i&1), 8+32*(i/2) );
		pos++;
	}
	if( i == 0 ){
		GFL_BMPWIN_TransVramCharacter( wk->util[BMPWIN_NAME_M].win );
	}
	// サブ
	for( i=0; i<POKEPUT_MAX_S; i++ ){
		if( pos > MONSNO_END ){ break; }
		PutPokeName( wk, pos, BMPWIN_NAME_S, 32+128*(i&1), 8+32*(i/2) );
		pos++;
	}
	if( i == 0 ){
		GFL_BMPWIN_TransVramCharacter( wk->util[BMPWIN_NAME_S].win );
	}
}



//============================================================================================
//	ＯＢＪ
//============================================================================================
static void InitObj( PIVIEW_WORK * wk )
{
	{
		const GFL_CLSYS_INIT init = {
			0, 0,									// メイン　サーフェースの左上座標
			0, 512,								// サブ　サーフェースの左上座標
			4,										// メイン画面OAM管理開始位置	4の倍数
			124,									// メイン画面OAM管理数				4の倍数
			4,										// サブ画面OAM管理開始位置		4の倍数
			124,									// サブ画面OAM管理数					4の倍数
			0,										// セルVram転送管理数

			POKEPUT_MAX_ALL,			// 登録できるキャラデータ数
			2,										// 登録できるパレットデータ数
			2,										// 登録できるセルアニメパターン数
			0,											// 登録できるマルチセルアニメパターン数（※現状未対応）

		  16,										// メイン CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
		  16										// サブ CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
		};
		GFL_CLACT_SYS_Create( &init, &VramTbl, HEAPID_NAKAHIRO_DEBUG );
	}

	{
		ARCHANDLE * ah;
		u32	i;

		// ポケアイコン
		ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, HEAPID_NAKAHIRO_DEBUG );
		// メイン画面
		for( i=0; i<POKEPUT_MAX_M; i++ ){
			wk->chrRes[i] = GFL_CLGRP_CGR_Register(
												ah, POKEICON_GetCgxArcIndexByMonsNumber(0,0,0,FALSE),
												FALSE, CLSYS_DRAW_MAIN, HEAPID_NAKAHIRO_DEBUG );
		}
		wk->palRes[0] = GFL_CLGRP_PLTT_RegisterComp(
											ah, POKEICON_GetPalArcIndex(),
											CLSYS_DRAW_MAIN, 0, HEAPID_NAKAHIRO_DEBUG );
		wk->celRes[0] = GFL_CLGRP_CELLANIM_Register(
											ah,
											POKEICON_GetCellArcIndex(),
											POKEICON_GetAnmArcIndex(),
											HEAPID_NAKAHIRO_DEBUG );
		// サブ画面
		for( i=POKEPUT_MAX_M; i<POKEPUT_MAX_ALL; i++ ){
			wk->chrRes[i] = GFL_CLGRP_CGR_Register(
												ah, POKEICON_GetCgxArcIndexByMonsNumber(0,0,0,FALSE),
												FALSE, CLSYS_DRAW_SUB, HEAPID_NAKAHIRO_DEBUG );
		}
	  wk->palRes[1] = GFL_CLGRP_PLTT_RegisterComp(
											ah, POKEICON_GetPalArcIndex(),
											CLSYS_DRAW_SUB, 0, HEAPID_NAKAHIRO_DEBUG );
		wk->celRes[1] = GFL_CLGRP_CELLANIM_Register(
											ah,
											POKEICON_GetCellSubArcIndex(),
											POKEICON_GetAnmSubArcIndex(),
											HEAPID_NAKAHIRO_DEBUG );
		GFL_ARC_CloseDataHandle( ah );
	}

	wk->clunit = GFL_CLACT_UNIT_Create( POKEPUT_MAX_ALL, 0, HEAPID_NAKAHIRO_DEBUG );

	{
		GFL_CLWK_DATA	dat = { 0, 0, 0, 0, 0 };
		u32	i;

		for( i=0; i<POKEPUT_MAX_M; i++ ){
			dat.pos_x = 16+(i&1)*128;
			dat.pos_y = 80+(i/2)*32;
			wk->clwk[i] = GFL_CLACT_WK_Create(
											wk->clunit,
											wk->chrRes[i], wk->palRes[0], wk->celRes[0],
											&dat, CLSYS_DRAW_MAIN, HEAPID_NAKAHIRO_DEBUG );
		}
		for( i=POKEPUT_MAX_M; i<POKEPUT_MAX_ALL; i++ ){
			dat.pos_x = 16+((i-POKEPUT_MAX_M)&1)*128;
			dat.pos_y = 16+((i-POKEPUT_MAX_M)/2)*32;
			wk->clwk[i] = GFL_CLACT_WK_Create(
											wk->clunit,
											wk->chrRes[i], wk->palRes[1], wk->celRes[1],
											&dat, CLSYS_DRAW_SUB, HEAPID_NAKAHIRO_DEBUG );
		}
	}

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// MAIN DISP OBJ ON
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// SUB DISP OBJ ON

	MakePokeTbl( wk );
	ChangePokeIconAll( wk );
}

static void ExitObj( PIVIEW_WORK * wk )
{
	u32	i;

	for( i=0; i<POKEPUT_MAX_ALL; i++ ){
		GFL_CLACT_WK_Remove( wk->clwk[i] );
	}
	GFL_CLACT_UNIT_Delete( wk->clunit );

	for( i=0; i<POKEPUT_MAX_ALL; i++ ){
		GFL_CLGRP_CGR_Release( wk->chrRes[i] );
	}
	GFL_CLGRP_PLTT_Release( wk->palRes[0] );
	GFL_CLGRP_PLTT_Release( wk->palRes[1] );
	GFL_CLGRP_CELLANIM_Release( wk->celRes[0] );
	GFL_CLGRP_CELLANIM_Release( wk->celRes[1] );

	GFL_CLACT_SYS_Delete();
}

static void ChangePokeIcon( PIVIEW_WORK * wk, u32 idx, u32 mons, u32 form, u32 sex )
{
	ARCHANDLE * ah;
	NNSG2dCharacterData * chr;
	void * buf;
	u32	cgx, pal;

	ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, HEAPID_NAKAHIRO_DEBUG );

	if( form >= wk->formTbl[idx] ){
		if( wk->formTbl[idx] != 0 ){
			form = wk->formTbl[idx] - 1;
		}else{
			form = wk->formTbl[idx];
		}
	}
	if( wk->sexTbl[idx] == POKEPER_SEX_UNKNOWN ){
		sex = 0;
	}else if( wk->sexTbl[idx] == POKEPER_SEX_MALE ){
		sex = 0;
	}else if( wk->sexTbl[idx] == POKEPER_SEX_FEMALE ){
		sex = 1;
	}

	if( mons == 0 ){
		if( form != 0 ){
			mons = MONSNO_MANAFI;
		}
		cgx = POKEICON_GetCgxArcIndexByMonsNumber( mons, form, sex, TRUE );
		pal = POKEICON_GetPalNum( mons, form, sex, TRUE );
	}else{
		cgx = POKEICON_GetCgxArcIndexByMonsNumber( mons, form, sex, FALSE );
		pal = POKEICON_GetPalNum( mons, form, sex, FALSE );
	}
	buf = GFL_ARCHDL_UTIL_LoadOBJCharacter( ah, cgx, FALSE, &chr, HEAPID_NAKAHIRO_DEBUG );

	if( idx < POKEPUT_MAX_M ){
		GFL_CLGRP_CGR_ReplaceEx(
			GFL_CLACT_WK_GetCgrIndex(wk->clwk[idx]), chr->pRawData, POKEICON_SIZE_CGX, 0, CLSYS_DRAW_MAIN );
	}else{
		GFL_CLGRP_CGR_ReplaceEx(
			GFL_CLACT_WK_GetCgrIndex(wk->clwk[idx]), chr->pRawData, POKEICON_SIZE_CGX, 0, CLSYS_DRAW_SUB );
	}

	GFL_HEAP_FreeMemory( buf );

  GFL_ARC_CloseDataHandle( ah );

	GFL_CLACT_WK_SetPlttOffs( wk->clwk[idx], pal, CLWK_PLTTOFFS_MODE_PLTT_TOP );
}

static void ChangePokeIconAll( PIVIEW_WORK * wk )
{
	u32	mons;
	u32	i;

	mons = wk->page * POKEPUT_MAX_ALL;

	for( i=0; i<POKEPUT_MAX_ALL; i++ ){
		if( (mons+i) > MONSNO_END ){
			GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], FALSE );
		}else{
			ChangePokeIcon( wk, i, mons+i, wk->form, wk->sex );
			GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], TRUE );
		}
	}
}

static void SetObjAnime( PIVIEW_WORK * wk )
{
	u32	i;

	for( i=0; i<POKEPUT_MAX_ALL; i++ ){
		GFL_CLACT_WK_SetAnmFrame( wk->clwk[i], 0 );
		GFL_CLACT_WK_SetAnmSeq( wk->clwk[i], POKEICON_ANM_HPMAX );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[i], wk->anmFlag );
	}
}




//============================================================================================
//	その他
//============================================================================================

static void SetBgColor( u8 pos )
{
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, BGColorTbl[pos] );
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, BGColorTbl[pos] );
}

static void ChangeBgColor( PIVIEW_WORK * wk )
{
	wk->bgcol = ( wk->bgcol + 1 ) & 3;
	SetBgColor( wk->bgcol );
	PutStrBGColor( wk );
}

static void ChangePage( PIVIEW_WORK * wk, s8 mv )
{
	s8	tmp = wk->page + mv;

	if( tmp < 0 ){
		tmp = 0;
	}else if( tmp * POKEPUT_MAX_ALL > MONSNO_END ){
		tmp = MONSNO_END / POKEPUT_MAX_ALL;
	}
	if( wk->page == tmp ){
		return;
	}

	wk->page = tmp;
	wk->form = 0;
	wk->sex  = 0;

	MakePokeTbl( wk );
	PutStrForm( wk );
	PutStrSex( wk );
	PutPokeNameAll( wk );
	ChangePokeIconAll( wk );
}

static void ChangeAnm( PIVIEW_WORK * wk )
{
	wk->anmFlag ^= 1;
	SetObjAnime( wk );
}

static void ChangeForm( PIVIEW_WORK * wk )
{
	u16	max;
	u16	i;

	max = wk->formTbl[0];

	for( i=1; i<POKEPUT_MAX_ALL; i++ ){
		if( wk->formTbl[i] > max ){
			max = wk->formTbl[i];
		}
	}

	wk->form++;
	if( wk->form >= max ){
		wk->form = 0;
	}
	wk->sex = 0;

	PutStrForm( wk );
	PutStrSex( wk );
	ChangePokeIconAll( wk );
}

static void ChangeSex( PIVIEW_WORK * wk )
{
	wk->sex ^= 1;
	wk->form = 0;

	PutStrForm( wk );
	PutStrSex( wk );
	ChangePokeIconAll( wk );
}

static void MakePokeTbl( PIVIEW_WORK * wk )
{
	u16	mons;
	u16	i;

	mons = wk->page * POKEPUT_MAX_ALL;

	for( i=0; i<POKEPUT_MAX_ALL; i++ ){
		if( mons == 0 ){
			wk->sexTbl[i]  = POKEPER_SEX_UNKNOWN;
			wk->formTbl[i] = 2;
		}else if( mons > MONSNO_END ){
			wk->sexTbl[i]  = 0;
			wk->formTbl[i] = 0;
		}else {
			POKEMON_PERSONAL_DATA * ppd = POKE_PERSONAL_OpenHandle( mons, 0, HEAPID_NAKAHIRO_DEBUG );
			wk->sexTbl[i]  = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );
			wk->formTbl[i] = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_form_max );
			POKE_PERSONAL_CloseHandle( ppd );
		}
		mons++;
	}
}
