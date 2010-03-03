//============================================================================================
/**
 * @file		backup_erase.c
 * @brief		セーブデータ初期化画面
 * @author	Hiroyuki Nakamura
 * @data		10/03/02
 *
 *	モジュール名：BACKUP_ERASE
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "system/main.h"
#include "system/wipe.h"
#include "system/gfl_use.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menu.h"
#include "gamesystem/msgspeed.h"
#include "sound/pm_sndsys.h"
#include "print/printsys.h"
#include "title/title.h"
#include "app/app_keycursor.h"
#include "msg/msg_backup_erase.h"


#include "font/font.naix"
#include "backup_erase.h"


//============================================================================================
//	定数定義
//============================================================================================

// 後方確保用ヒープＩＤ
#define	HEAPID_BACKUP_ERASE_L		( GFL_HEAP_LOWID(HEAPID_BACKUP_ERASE) )

typedef struct {
	GFL_TCB * vtask;					// TCB ( VBLANK )
	GFL_FONT * font;					// 通常フォント
	GFL_MSGDATA * mman;				// メッセージデータマネージャ
	STRBUF * exp;							// メッセージ展開領域
	PRINT_STREAM * stream;		// プリントストリーム
	GFL_TCBLSYS * tcbl;
	GFL_BMPWIN * win;
	BMPMENU_WORK * mwk;
	APP_KEYCURSOR_WORK * kcwk;	// メッセージ送りカーソル
}BACKUP_ERASE_WORK;

// メインシーケンス
enum {
	MAINSEQ_INIT = 0,			// 初期化
	MAINSEQ_FADEIN,				// フェードイン
	MAINSEQ_CLEAR_MSG,		// メッセージ１
	MAINSEQ_CLEAR_YESNO,	// はい・いいえ１
	MAINSEQ_CHECK_MSG,		// メッセージ２
	MAINSEQ_CHECK_YESNO,	// はい・いいえ２
	MAINSEQ_ACTION_MSG,		// メッセージ３
	MAINSEQ_FADEOUT,			// フェードアウト
	MAINSEQ_RELEASE,			// 終了
};

// メッセージウィンドウキャラ
#define	WIN_CHAR_NUM		( 1 )

// ＢＧパレット
#define	WIN_PALETTE_M		( 14 )
#define	FONT_PALETTE_M	( 15 )

// BMPWIN
// メッセージ
#define	BMPWIN_MSG_FRM			( GFL_BG_FRAME0_M )
#define	BMPWIN_MSG_PX				( 1 )
#define	BMPWIN_MSG_PY				( 19 )
#define	BMPWIN_MSG_SX				( 30 )
#define	BMPWIN_MSG_SY				( 4 )
#define	BMPWIN_MSG_PAL			( FONT_PALETTE_M )
// はい・いいえ
#define	BMPWIN_YESNO_FRM		( GFL_BG_FRAME0_M )
#define	BMPWIN_YESNO_PX			( 24 )
#define	BMPWIN_YESNO_PY			( 13 )
#define	BMPWIN_YESNO_PAL		( FONT_PALETTE_M )

#define	EXP_BUF_SIZE		( 1024 )		// 汎用文字列バッファサイズ

#define	BACK_GROUND_COLOR		( 0x7d8c )	// 背景カラー


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT BackupEraseProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT BackupEraseProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT BackupEraseProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

static void MainSeq_Init( BACKUP_ERASE_WORK * wk, int * seq );
static void MainSeq_FadeIn( BACKUP_ERASE_WORK * wk, int * seq );
static void MainSeq_ClearMessage( BACKUP_ERASE_WORK * wk, int * seq );
static void MainSeq_ClearYesNo( BACKUP_ERASE_WORK * wk, int * seq );
static void MainSeq_CheckMessage( BACKUP_ERASE_WORK * wk, int * seq );
static void MainSeq_CheckYesNo( BACKUP_ERASE_WORK * wk, int * seq );
static void MainSeq_ActionMessage( BACKUP_ERASE_WORK * wk, int * seq );
static void MainSeq_FadeOut( BACKUP_ERASE_WORK * wk, int * seq );

static void InitVram(void);
static void InitBg(void);
static void ExitBg(void);
static void InitMsg( BACKUP_ERASE_WORK * wk );
static void ExitMsg( BACKUP_ERASE_WORK * wk );
static void InitBmp( BACKUP_ERASE_WORK * wk );
static void ExitBmp( BACKUP_ERASE_WORK * wk );
static void InitVBlank( BACKUP_ERASE_WORK * wk );
static void ExitVBlank( BACKUP_ERASE_WORK * wk );

static void StartMessage( BACKUP_ERASE_WORK * wk, int strIdx );
static BOOL MainMessage( BACKUP_ERASE_WORK * wk );
static void SetYesNoMenu( BACKUP_ERASE_WORK * wk );
static void SetFadeIn(void);
static void SetFadeOut(void);


//============================================================================================
//	グローバル
//============================================================================================

// PROC
const GFL_PROC_DATA BACKUP_ERASE_ProcData = {
  BackupEraseProc_Init,
  BackupEraseProc_Main,
  BackupEraseProc_End,
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
static GFL_PROC_RESULT BackupEraseProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	BACKUP_ERASE_WORK * wk;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BACKUP_ERASE, 0x30000 );

	wk = GFL_PROC_AllocWork( proc, sizeof(BACKUP_ERASE_WORK), HEAPID_BACKUP_ERASE );
	GFL_STD_MemClear( wk, sizeof(BACKUP_ERASE_WORK) );

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
static GFL_PROC_RESULT BackupEraseProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	BACKUP_ERASE_WORK * wk = mywk;

	switch( *seq ){
	case MAINSEQ_INIT:					// 初期化
		MainSeq_Init( wk, seq );
		break;

	case MAINSEQ_FADEIN:				// フェードイン
		MainSeq_FadeIn( wk, seq );
		break;

	case MAINSEQ_CLEAR_MSG:			// メッセージ１
		MainSeq_ClearMessage( wk, seq );
		break;

	case MAINSEQ_CLEAR_YESNO:		// はい・いいえ１
		MainSeq_ClearYesNo( wk, seq );
		break;

	case MAINSEQ_CHECK_MSG:			// メッセージ２
		MainSeq_CheckMessage( wk, seq );
		break;

	case MAINSEQ_CHECK_YESNO:		// はい・いいえ２
		MainSeq_CheckYesNo( wk, seq );
		break;

	case MAINSEQ_ACTION_MSG:		// メッセージ３
		MainSeq_ActionMessage( wk, seq );
		break;

	case MAINSEQ_FADEOUT:				// フェードアウト
		MainSeq_FadeOut( wk, seq );
		break;

	case MAINSEQ_RELEASE:				// 終了
		return GFL_PROC_RES_FINISH;
	}

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
static GFL_PROC_RESULT BackupEraseProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_BACKUP_ERASE );

	GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);

	return GFL_PROC_RES_FINISH;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：初期化
 *
 * @param		wk			セーブ初期化ワーク
 * @param		seq			シーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MainSeq_Init( BACKUP_ERASE_WORK * wk, int * seq )
{
	InitVram();
	InitBg();
	InitMsg( wk );
	InitBmp( wk );
	InitVBlank( wk );

	SetFadeIn();

	*seq = MAINSEQ_FADEIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：フェードイン待ち
 *
 * @param		wk			セーブ初期化ワーク
 * @param		seq			シーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MainSeq_FadeIn( BACKUP_ERASE_WORK * wk, int * seq )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		StartMessage( wk, msg01 );
		*seq = MAINSEQ_CLEAR_MSG;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「消去しますか？」表示待ち
 *
 * @param		wk			セーブ初期化ワーク
 * @param		seq			シーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MainSeq_ClearMessage( BACKUP_ERASE_WORK * wk, int * seq )
{
	if( MainMessage( wk ) == FALSE ){
		SetYesNoMenu( wk );
		*seq = MAINSEQ_CLEAR_YESNO;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「消去しますか？」はい・いいえ
 *
 * @param		wk			セーブ初期化ワーク
 * @param		seq			シーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MainSeq_ClearYesNo( BACKUP_ERASE_WORK * wk, int * seq )
{
	switch( BmpMenu_YesNoSelectMain( wk->mwk ) ){
	case 0:
		StartMessage( wk, msg02 );
		*seq = MAINSEQ_CHECK_MSG;
		break;

	case BMPMENU_CANCEL:
		SetFadeOut();
		*seq = MAINSEQ_FADEOUT;
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「本当に消去しますか？」表示待ち
 *
 * @param		wk			セーブ初期化ワーク
 * @param		seq			シーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MainSeq_CheckMessage( BACKUP_ERASE_WORK * wk, int * seq )
{
	if( MainMessage( wk ) == FALSE ){
		SetYesNoMenu( wk );
		*seq = MAINSEQ_CHECK_YESNO;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「本当に消去しますか？」はい・いいえ
 *
 * @param		wk			セーブ初期化ワーク
 * @param		seq			シーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MainSeq_CheckYesNo( BACKUP_ERASE_WORK * wk, int * seq )
{
	switch( BmpMenu_YesNoSelectMain( wk->mwk ) ){
	case 0:
		StartMessage( wk, msg03 );
		*seq = MAINSEQ_ACTION_MSG;
		break;

	case BMPMENU_CANCEL:
		SetFadeOut();
		*seq = MAINSEQ_FADEOUT;
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「消去ています」表示待ち
 *
 * @param		wk			セーブ初期化ワーク
 * @param		seq			シーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MainSeq_ActionMessage( BACKUP_ERASE_WORK * wk, int * seq )
{
	if( MainMessage( wk ) == FALSE ){
		SAVE_CONTROL_WORK * sv;
		u32	i;
		sv = SaveControl_GetPointer();
		SaveControl_Erase( sv );
		for( i=0; i<SAVE_EXTRA_ID_MAX; i++ ){
			if( SaveControl_Extra_Load( sv, i, HEAPID_BACKUP_ERASE ) == LOAD_RESULT_OK ){
				SaveControl_Extra_Erase( sv, i, HEAPID_BACKUP_ERASE );
			}
			SaveControl_Extra_Unload( sv, i );
		}
		OS_ResetSystem(0);		// セーブ読み込み状況を更新する為、ソフトリセットする

//		*seq = MAINSEQ_FADEOUT;		// リセットされるのでいらない。
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：フェードアウト待ち
 *
 * @param		wk			セーブ初期化ワーク
 * @param		seq			シーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MainSeq_FadeOut( BACKUP_ERASE_WORK * wk, int * seq )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		ExitVBlank( wk );
		ExitBmp( wk );
		ExitMsg( wk );
		ExitBg();
		*seq = MAINSEQ_RELEASE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitVram(void)
{
	const GFL_DISP_VRAM tbl = {
		GX_VRAM_BG_128_A,							// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,				// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_128_D,				// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,							// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_NONE,					// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_128K		// サブOBJマッピングモード
	};

	GFL_DISP_ClearVRAM( NULL );
	GFL_DISP_SetBank( &tbl );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitBg(void)
{
	GFL_BG_Init( HEAPID_BACKUP_ERASE );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
	}

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );

	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, BACK_GROUND_COLOR );
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, BACK_GROUND_COLOR );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ解放
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
	GFL_BG_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ関連初期化
 *
 * @param		wk			セーブ初期化ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitMsg( BACKUP_ERASE_WORK * wk )
{
	wk->mman = GFL_MSG_Create(
							GFL_MSG_LOAD_NORMAL,
							ARCID_MESSAGE, NARC_message_backup_erase_dat, HEAPID_BACKUP_ERASE );
	wk->font = GFL_FONT_Create(
							ARCID_FONT, NARC_font_large_gftr,
							GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_BACKUP_ERASE );
	wk->exp  = GFL_STR_CreateBuffer( EXP_BUF_SIZE, HEAPID_BACKUP_ERASE );
	wk->tcbl = GFL_TCBL_Init( HEAPID_BACKUP_ERASE, HEAPID_BACKUP_ERASE, 1, 4 );
	wk->kcwk = APP_KEYCURSOR_Create( 15, TRUE, FALSE, HEAPID_BACKUP_ERASE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ関連解放
 *
 * @param		wk			セーブ初期化ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitMsg( BACKUP_ERASE_WORK * wk )
{
	APP_KEYCURSOR_Delete( wk->kcwk );
  GFL_TCBL_Exit( wk->tcbl );
	GFL_STR_DeleteBuffer( wk->exp );
	GFL_FONT_Delete( wk->font );
	GFL_MSG_Delete( wk->mman );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ初期化
 *
 * @param		wk			セーブ初期化ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitBmp( BACKUP_ERASE_WORK * wk )
{
	GFL_BMPWIN_Init( HEAPID_BACKUP_ERASE );

	wk->win = GFL_BMPWIN_Create(
							BMPWIN_MSG_FRM, BMPWIN_MSG_PX, BMPWIN_MSG_PY,
							BMPWIN_MSG_SX, BMPWIN_MSG_SY, BMPWIN_MSG_PAL, GFL_BMP_CHRAREA_GET_B );

	// システムウィンドウ
	BmpWinFrame_GraphicSet(
		GFL_BG_FRAME0_M, WIN_CHAR_NUM, WIN_PALETTE_M, MENU_TYPE_SYSTEM, HEAPID_BACKUP_ERASE );

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr,
		PALTYPE_MAIN_BG, FONT_PALETTE_M*0x20, 0x20, HEAPID_BACKUP_ERASE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ解放
 *
 * @param		wk			セーブ初期化ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitBmp( BACKUP_ERASE_WORK * wk )
{
	GFL_BMPWIN_Delete( wk->win );
	GFL_BMPWIN_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK
 *
 * @param		tcb			GFL_TCB
 * @param		work		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankTask( GFL_TCB * tcb, void * work )
{
	GFL_BG_VBlankFunc();

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK設定
 *
 * @param		wk			セーブ初期化ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitVBlank( BACKUP_ERASE_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK解放
 *
 * @param		wk			セーブ初期化ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitVBlank( BACKUP_ERASE_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ表示開始
 *
 * @param		wk				セーブ初期化ワーク
 * @param		strIdx		文字列インデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void StartMessage( BACKUP_ERASE_WORK * wk, int strIdx )
{
  GFL_MSG_GetString( wk->mman, strIdx, wk->exp );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win), 15 );
	BmpWinFrame_Write(
		wk->win, WINDOW_TRANS_OFF, WIN_CHAR_NUM, WIN_PALETTE_M );

	wk->stream = PRINTSYS_PrintStream(
								wk->win,
								0, 0, wk->exp,
								wk->font,
								MSGSPEED_GetWait(),
								wk->tcbl,
								10,		// tcbl pri
								HEAPID_BACKUP_ERASE,
								15 );	// clear color
	GFL_BMPWIN_MakeTransWindow_VBlank( wk->win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージメイン
 *
 * @param		wk		セーブ初期化ワーク
 *
 * @retval	"TRUE = メッセージ処理中"
 * @retval	"FALSE = メッセージ処理終了"
 */
//--------------------------------------------------------------------------------------------
static BOOL MainMessage( BACKUP_ERASE_WORK * wk )
{
  GFL_TCBL_Main( wk->tcbl );

  switch( PRINTSYS_PrintStreamGetState(wk->stream) ){
  case PRINTSTREAM_STATE_RUNNING: //実行中
    if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
      PRINTSYS_PrintStreamShortWait( wk->stream, 0 );
    }
    break;

  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
      PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
      PRINTSYS_PrintStreamReleasePause( wk->stream );
    }
    break;

  case PRINTSTREAM_STATE_DONE: //終了
    PRINTSYS_PrintStreamDelete( wk->stream );
		return FALSE;
	}

	APP_KEYCURSOR_Main( wk->kcwk, wk->stream, wk->win );

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ設定
 *
 * @param		wk		セーブ初期化ワーク
 *
 * @retval	none
 */
//--------------------------------------------------------------------------------------------
static void SetYesNoMenu( BACKUP_ERASE_WORK * wk )
{
	BMPWIN_YESNO_DAT	dat;

	dat.frmnum = BMPWIN_YESNO_FRM;
	dat.pos_x  = BMPWIN_YESNO_PX;
	dat.pos_y  = BMPWIN_YESNO_PY;
	dat.palnum = BMPWIN_YESNO_PAL;
	dat.chrnum = 0;

	wk->mwk = BmpMenu_YesNoSelectInit(
							&dat, WIN_CHAR_NUM, WIN_PALETTE_M, 1, HEAPID_BACKUP_ERASE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードイン設定
 *
 * @param		none
 *
 * @retval	none
 */
//--------------------------------------------------------------------------------------------
static void SetFadeIn(void)
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_BACKUP_ERASE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウト設定
 *
 * @param		none
 *
 * @retval	none
 */
//--------------------------------------------------------------------------------------------
static void SetFadeOut(void)
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_BACKUP_ERASE );
}
