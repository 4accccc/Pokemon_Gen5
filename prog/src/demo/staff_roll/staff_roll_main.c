//============================================================================================
/**
 * @file		staff_roll_main.c
 * @brief		エンディング・スタッフロール画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.19
 *
 *	モジュール名：SRMAIN
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/main.h"
#include "system/wipe.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"

#include "font/font.naix"
#include "message.naix"
#include "msg/msg_staff_list_jp.h"
#include "msg/msg_staff_list_eng.h"
#include "title/title_res.h"
#include "demo3d.naix"

#include "staff_roll_main.h"
#include "staff_roll.naix"


//============================================================================================
//	定数定義
//============================================================================================

#ifdef PM_DEBUG
#define	LOCAL_VERSION		( VERSION_B2 )		// デバッグ用（コミット時はブラックにすること！）
#else
#define	LOCAL_VERSION		( VERSION_B2 )		// 製品版はブラックを基本とする
#endif	// PM_DEBUG

// 後方確保用ヒープＩＤ
#define	HEAPID_STAFF_ROLL_L		( GFL_HEAP_LOWID(HEAPID_STAFF_ROLL) )

// メインシーケンス
enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_WIPE,
	MAINSEQ_RELEASE,

	MAINSEQ_START_WAIT,
	MAINSEQ_MAIN,

	MAINSEQ_END,
};

// サブシーケンス
enum {
	SUBSEQ_INIT = 0,
	SUBSEQ_WAIT,
	SUBSEQ_STR_PUT,
	SUBSEQ_STR_CLEAR,

	SUBSEQ_LOGO_PUT,

	SUBSEQ_END,
};

// 2Dカットのシーケンス
enum{
  SEQ_CUT_NONE,       // なし
  SEQ_CUT_LOAD,       // リソース読み込み

  // 処理落ち軽減のために分割読み込みにした
  SEQ_CUT_CGX_LOAD,   // キャラ読み込み
  SEQ_CUT_SCR_LOAD,   // スクリーン読み込み
  SEQ_CUT_TRANS,      // 転送(キャラ・スクリーン・パレット)

  SEQ_CUT_FADE_IN,    // フェードしながら表示
  SEQ_CUT_VIEW_WAIT,  // 表示中
  SEQ_CUT_FADE_OUT,   // フェードしながら非表示
  SEQ_CUT_LOAD_N,     // リソース読み込み(Nのアニメ)
  SEQ_CUT_EXIT,       // 終了処理
};


#ifdef	PM_DEBUG
#define	LIST_START_INIT_WAIT	( wk->testStartInitWait )
#define	LIST_START_END_WAIT		( wk->testStartEndWait )
#define	LOGO_PUT_WAIT					( wk->testLogoWait )
#define	DEBUG_LIST_START_INIT_WAIT	( 20 )
#define	DEBUG_LIST_START_END_WAIT		( 180 )
#define	DEBUG_LOGO_PUT_WAIT					( 200 )
#else
#define	LIST_START_INIT_WAIT	( 20 )			// 初期処理のロゴ表示開始までのウェイト
#define	LIST_START_END_WAIT		( 180 )				// 初期処理のロゴ表示終了後のウェイト
#define	LOGO_PUT_WAIT					( 200 )			// 初期処理のロゴ表示中ウェイト
#endif	// PM_DEBUG

#if	PM_VERSION == LOCAL_VERSION
#define	BG_COLOR		( 0 )														// バックグラウンドカラー
#define	FCOL_WP00V	( PRINTSYS_MACRO_LSB(1,2,0) )		// フォントカラー：デフォルト
#define	FCOL_WP00R	( PRINTSYS_MACRO_LSB(3,4,0) )		// フォントカラー：赤
#else
#define	BG_COLOR		( 0xffff )											// バックグラウンドカラー
#define	FCOL_WP00V	( PRINTSYS_MACRO_LSB(5,6,0) )		// フォントカラー：デフォルト
#define	FCOL_WP00R	( PRINTSYS_MACRO_LSB(7,8,0) )		// フォントカラー：赤
#endif
#define	FCOL_WP00B	( PRINTSYS_MACRO_LSB(5,6,0) )		// フォントカラー：青
#define	FCOL_WP00Y	( PRINTSYS_MACRO_LSB(7,8,0) )		// フォントカラー：黄
#define	FCOL_WP00G	( PRINTSYS_MACRO_LSB(9,10,0) )	// フォントカラー：緑
#define	FCOL_WP00O	( PRINTSYS_MACRO_LSB(11,12,0) )	// フォントカラー：オレンジ
#define	FCOL_WP00P	( PRINTSYS_MACRO_LSB(13,14,0) )	// フォントカラー：ピンク

#define	MBG_PAL_FONT	( 15 )		// メイン画面フォントパレット（ＢＧ）
#define	SBG_PAL_FONT	( 15 )		// サブ画面フォントパレット（ＢＧ）

#define	BMPWIN_TRANS_MAIN_FLAG		( 1 )		// ＢＭＰ転送フラグ（メインＢＧ）
#define	BMPWIN_TRANS_SUB_FLAG			( 2 )		// ＢＭＰ転送フラグ（サブＢＧ）

#define	EXP_BUFF_SIZE		( 128 )		// メッセージ展開用バッファサイズ

#define	VBMP_SX									( 32 )			// 仮想ＢＭＰＸサイズ
#define	VBMP_SY									( 2 )				// 仮想ＢＭＰＹサイズ
#define	STR_PRINT_PY						( 192 )			// スクロール時の文字列描画Ｙ座標
#define	STR_PRINT_SMALL_OFFSET	( 2 )				// スクロール時のスモールフォント文字列描画Ｙオフセット
#define	STR_PRINT_SMALL_PY			( STR_PRINT_PY+STR_PRINT_SMALL_OFFSET )	// スクロール時のスモールフォント文字列描画Ｙ座標

#define	ITEMLIST_MSG_NONE		( 0xffff )		// メッセージなし

// カット
// 画面選択
#define CUT_DISP_MAIN         ( 1 )       // メインBG(上画面)
#define CUT_DISP_SUB          ( 2 )       // サブBG(下画面)

///	フェード
typedef enum {
  CUT_FADE_IN,
  CUT_FADE_OUT,
} CUT_FADE;
#define CUT_FADE_SKIP_SPEED    ( 4 )      // スキップ時のフェード速度

// カット読み込み
typedef enum {
  CUT_DATID_PAL,
  CUT_DATID_CGX,
  CUT_DATID_SCR,
} CUT_DATID_TYPE;

// コマンドラベル
enum {
	ITEMLIST_LABEL_NONE = 0,
	ITEMLIST_LABEL_STR_PUT,
	ITEMLIST_LABEL_STR_CLEAR,
	ITEMLIST_LABEL_SCROLL_START,
	ITEMLIST_LABEL_SCROLL_STOP,
	ITEMLIST_LABEL_END,

	ITEMLIST_LABEL_LOGO_PUT,

	ITEMLIST_LABEL_VERSION,

  // swan追加
  // カットイン表示
	ITEMLIST_LABEL_CUT,
  // Nのアニメ
	ITEMLIST_LABEL_N_ANIME,

	ITEMLIST_LABEL_MAX,
};

// 描画位置
enum {
	STR_PUT_MODE_LEFT = 0,		// 左詰め
	STR_PUT_MODE_RIGHT,				// 右詰め
	STR_PUT_MODE_CENTER,			// 中央
	STR_PUT_MODE_UP,			    // 上画面
	STR_PUT_MODE_DOWN,			  // 下画面
};

#define SKIP_SPEED					( 4 )			// スキップ速度
#define	STR_FADE_SPEED			( 4 )			// スキップ時の文字列フェード速度
// ロゴ
#define LOGO_FADE_SYNC      ( 138 )     // ロゴのフェードシンク
#define LOGO_FADE_SYNC_SKIP ( 34 )     // ロゴのフェードシンク

typedef int (*pCOMM_FUNC)(SRMAIN_WORK*,ITEMLIST_DATA*);

#define	ITEMLIST_SCROLL_SPEED		( FX32_CONST(1) )			// リストスクロール速度

#define	SKIP_MODE_BGM_FADE			( 64 )			// スキップ時のBGMフェードフレーム数

// BG分割読み込み
#define BG_LOAD_DIV_FRAME       ( 8 )       // 何分割読み込み

//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( SRMAIN_WORK * wk );
static int MainSeq_Wipe( SRMAIN_WORK * wk );
static int MainSeq_Release( SRMAIN_WORK * wk );
static int MainSeq_StartWait( SRMAIN_WORK * wk );
static int MainSeq_Main( SRMAIN_WORK * wk );

static void InitVram(void);
static void InitBg(void);
static void ExitBg(void);
static void LoadBgGraphic( ARCHANDLE *handle );
static void InitObj( ARCHANDLE *handle, OBJ_WORK * wk );
static void ExitObj( OBJ_WORK * wk );
static void InitCut( CUT_WORK * wk );
static void ExitCut( CUT_WORK * wk );
static void InitVBlank( SRMAIN_WORK * wk );
static void ExitVBlank( SRMAIN_WORK * wk );
static void InitMsg( SRMAIN_WORK * wk );
static void ExitMsg( SRMAIN_WORK * wk );

static void CreateListData( SRMAIN_WORK * wk );
static void DeleteListData( SRMAIN_WORK * wk );

static void InitBmp( SRMAIN_WORK * wk );
static void ExitBmp( SRMAIN_WORK * wk );
static void SetBmpWinTransFlag( SRMAIN_WORK * wk, u32 flg );
static void TransBmpWinChar( SRMAIN_WORK * wk );

static void WriteVirtualBmp( SRMAIN_WORK * wk );
static void PrintVirtualBmp( SRMAIN_WORK * wk );
static void PrintStr( SRMAIN_WORK * wk, ITEMLIST_DATA * item, GFL_BMP_DATA * bmp, u32 py );

static int SetFadeIn( SRMAIN_WORK * wk, int next );
static int SetFadeOut( SRMAIN_WORK * wk, int next );
static void CheckSkip( SRMAIN_WORK * wk );

static void ListScroll( SRMAIN_WORK * wk );

static int CreateItemData( SRMAIN_WORK * wk );

static BOOL PutStr( SRMAIN_WORK * wk );
static BOOL ClearStr( SRMAIN_WORK * wk );

static BOOL PutLogo( SRMAIN_WORK * wk );

// swan追加
// カット関係の処理
static void CutMain( SRMAIN_WORK *p_parent_wk, CUT_WORK *p_wk );            // メイン
static BOOL CutSeqFunc_Fade( CUT_WORK *p_wk, CUT_FADE fade, BOOL skipFlag );    // フェード
static u32  CutGetDatID( CUT_WORK *p_wk, CUT_DATID_TYPE datid_type, u32 sex );

static BOOL Comm_LabelNone( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelStrPut( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelStrClear( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelScrollStart( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelScrollStop( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelEnd( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelLogoPut( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelVersion( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
// swan追加
// カット表示
static BOOL Comm_LabelCut( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
// Nアニメ表示
static BOOL Comm_LabelNAnime( SRMAIN_WORK * wk, ITEMLIST_DATA * item );

#ifdef	PM_DEBUG
static void DebugGridSet(void);
#endif	// PM_DEBUG


FS_EXTERN_OVERLAY(ui_common);


//============================================================================================
//	グローバル
//============================================================================================

// メインシーケンス
static const pSRMAIN_FUNC	MainSeq[] = {
	MainSeq_Init,
	MainSeq_Wipe,
	MainSeq_Release,

	MainSeq_StartWait,
	MainSeq_Main,
};

// VRAM割り振り
static const GFL_DISP_VRAM VramTbl = {
	GX_VRAM_BG_128_A,							// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_0123_E,			// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_0123_H,	// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_128_D,				// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_NONE,						  // テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,				  // テクスチャパレットスロット

	GX_OBJVRAMMODE_CHAR_1D_32K,	  // メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K		// サブOBJマッピングモード
};

// フォントカラーテーブル
static const PRINTSYS_LSB FontColorTbl[] = {
	FCOL_WP00V,		// フォントカラー：デフォルト
	FCOL_WP00R,		// フォントカラー：赤
	FCOL_WP00B,		// フォントカラー：青
	FCOL_WP00Y,		// フォントカラー：黄
	FCOL_WP00G,		// フォントカラー：緑
	FCOL_WP00O,		// フォントカラー：オレンジ
	FCOL_WP00P,		// フォントカラー：ピンク
};

// コマンドテーブル
static const pCOMM_FUNC CommFunc[] = {
	Comm_LabelNone,					// ラベル：なし
	Comm_LabelStrPut,				// ラベル：文字列一括表示
	Comm_LabelStrClear,			// ラベル：表示クリア
	Comm_LabelScrollStart,	// ラベル：スクロール開始
	Comm_LabelScrollStop,		// ラベル：スクロール停止
	Comm_LabelEnd,					// ラベル：終了
	Comm_LabelLogoPut,					// ラベル：ロゴ表示
	Comm_LabelVersion,			// ラベル：バージョン別処理

  // swan 追加
  // カットイン表示
  Comm_LabelCut,          // ラベル：カットイン表示
  // Nのアニメ表示j
  Comm_LabelNAnime,       // ラベル：Nのアニメ表示
};

//============================================================================================
//	シーケンス
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス
 *
 * @param		wk		ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL SRMAIN_Main( SRMAIN_WORK * wk )
{
	CheckSkip( wk );

	wk->mainSeq = MainSeq[wk->mainSeq]( wk );
	if( wk->mainSeq == MAINSEQ_END ){
		return FALSE;
	}

	WriteVirtualBmp( wk );

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：初期化
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Init( SRMAIN_WORK * wk )
{
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 上画面をメインに
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	// 輝度を最低にしておく
#if	PM_VERSION == LOCAL_VERSION
	// 輝度を最低にしておく（ブラック）
	GX_SetMasterBrightness( -16 );
	GXS_SetMasterBrightness( -16 );
#else
	// 輝度を最高にしておく（ホワイト）
	GX_SetMasterBrightness( 16 );
	GXS_SetMasterBrightness( 16 );
#endif

#ifdef	PM_DEBUG
	wk->testStartInitWait = DEBUG_LIST_START_INIT_WAIT;
	wk->testLogoWait = DEBUG_LOGO_PUT_WAIT;
	wk->testStartEndWait = DEBUG_LIST_START_END_WAIT;
#endif	// PM_DEBUG

	//アーカイブハンドル作成
	wk->handle = GFL_ARC_OpenDataHandle( ARCID_STAFF_ROLL, HEAPID_STAFF_ROLL );
	GF_ASSERT( wk->handle );

	InitVram();
	InitBg();
  InitObj( wk->handle, &wk->obj );
  InitCut( &wk->cut );
	LoadBgGraphic( wk->handle );
	InitMsg( wk );
	InitBmp( wk );

	CreateListData( wk );

	InitVBlank( wk );

	OS_Printf( "heap size [0] = 0x%x\n", GFL_HEAP_GetHeapFreeSize(HEAPID_STAFF_ROLL) );

	return MAINSEQ_START_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：フェード待ち
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Wipe( SRMAIN_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return wk->nextSeq;
	}
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：解放
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Release( SRMAIN_WORK * wk )
{

	ExitVBlank( wk );

	DeleteListData( wk );

  if( wk->handle != NULL )
  {
    GFL_ARC_CloseDataHandle( wk->handle );
    wk->handle = NULL;
  }
  
	ExitBmp( wk );
	ExitMsg( wk );
  ExitCut( &wk->cut );
  ExitObj( &wk->obj );
	ExitBg();

#if	PM_VERSION == LOCAL_VERSION
	// 輝度を最低にしておく（ブラック）
	GX_SetMasterBrightness( -16 );
	GXS_SetMasterBrightness( -16 );
#else
	// 輝度を最高にしておく（ホワイト）
	GX_SetMasterBrightness( 16 );
	GXS_SetMasterBrightness( 16 );
#endif
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

	return MAINSEQ_END;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：開始処理
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_StartWait( SRMAIN_WORK * wk )
{
	switch( wk->subSeq ){
	case 0:
		if( wk->wait >= LIST_START_INIT_WAIT ){
			wk->wait = 0;
			wk->subSeq++;
		}else{
/*
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
				OS_Printf( "[0] push a button : wait = %d\n", wk->wait );
			}
*/
			if( wk->skipFlag == 1 ){
				wk->wait += SKIP_SPEED;
			}else{
				wk->wait++;
			}
		}
		break;

	case 1:
		if( PutLogo( wk ) == FALSE ){
			wk->subSeq++;
		}
		break;

	case 2:
		if( wk->wait >= LIST_START_END_WAIT ){
			wk->wait = 0;
			wk->subSeq = 0;
			return MAINSEQ_MAIN;
		}else{
/*
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
				OS_Printf( "[2] push a button : wait = %d\n", wk->wait );
			}
*/
			if( wk->skipFlag == 1 ){
				wk->wait += SKIP_SPEED;
			}else{
				wk->wait++;
			}
		}
		break;
	}
	return MAINSEQ_START_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メイン処理
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Main( SRMAIN_WORK * wk )
{
#ifdef	PM_DEBUG
	if( wk->dat->fastMode == STAFFROLL_MODE_DEBUG ){
		// グリッド表示
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
			DebugGridSet();
		}
		// 一時停止
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
			wk->debugStopFlg ^= 1;
		}
		if( wk->debugStopFlg == TRUE ){
			return MAINSEQ_MAIN;
		}
		// 終了
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
			return SetFadeOut( wk, MAINSEQ_RELEASE );
		}
	}
#endif

	if( wk->subSeq == SUBSEQ_INIT ){
		while( 1 ){
			ITEMLIST_DATA * item = &wk->list[wk->listPos];
			if( wk->vBmp[wk->vBmpPut].idx == wk->listPos ){
				PrintVirtualBmp( wk );
				wk->vBmpPut++;
				wk->listWait = item->wait;
				wk->subSeq = SUBSEQ_WAIT;
				wk->listPos += 2;
				break;
			}
			if( CommFunc[item->label]( wk, item ) == TRUE ){
				wk->listPos++;
				break;
			}
			wk->listPos++;
		}
	}

	switch( wk->subSeq ){
	case SUBSEQ_WAIT:
		if( wk->listWait <= 0 ){
			wk->listWait = 0;
			wk->subSeq = SUBSEQ_INIT;
		}else{
			if( wk->skipFlag == 1 ){
				wk->listWait -= SKIP_SPEED;
			}else{
				wk->listWait--;
			}
		}
		break;

	case SUBSEQ_STR_PUT:
		if( PutStr( wk ) == FALSE ){
			wk->subSeq = SUBSEQ_INIT;
		}
		break;

	case SUBSEQ_STR_CLEAR:
		if( ClearStr( wk ) == FALSE ){
			wk->subSeq = SUBSEQ_INIT;
		}
		break;

	case SUBSEQ_END:
		return SetFadeOut( wk, MAINSEQ_RELEASE );
	}

	ListScroll( wk );

  // カットは2回目以降はでない
	if( wk->dat->fastMode != STAFFROLL_MODE_FAST ){
    // カットのメイン処理
    CutMain( wk, &wk->cut );
  }

  GFL_CLACT_SYS_Main();

	return MAINSEQ_MAIN;
}


//============================================================================================
//	初期化
//============================================================================================

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
	GFL_DISP_ClearVRAM( 0 );
	GFL_DISP_SetBank( &VramTbl );
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
static void InitBg(void)
{
	GFL_BG_Init( HEAPID_STAFF_ROLL );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：文字面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：ロゴ
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, 0x10000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_256X16 );
	}

	{	// 回転拡縮小初期化
		MtxFx22 mtx;
	  GFL_CALC2D_GetAffineMtx22( &mtx, 0, FX32_ONE, FX32_ONE, GFL_CALC2D_AFFINE_MAX_NORMAL );
		GFL_BG_SetAffine( GFL_BG_FRAME3_M, &mtx, 128, 96 );
	}
  {	// メイン画面：カット
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x1000, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0xc000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
	}

	{	// サブ画面：文字面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
	}
  {	// サブ画面：カット
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x1000, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0xc000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_23, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &cnth, GFL_BG_MODE_TEXT );
	}

#ifdef PM_DEBUG
	{	// グリッド面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, 0x1000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &cnth, GFL_BG_MODE_TEXT );
	}
#endif	// PM_DEBUG

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
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
static void ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_OFF );

#ifdef PM_DEBUG
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
#endif	// PM_DEBUG

	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );

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
static void LoadBgGraphic( ARCHANDLE *handle )
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( TITLE_RES_ARCID, HEAPID_STAFF_ROLL_L );

  GFL_ARCHDL_UTIL_TransVramPalette( 
    ah, TITLE_RES_LOGO_NCLR, PALTYPE_MAIN_BG_EX, 0x6000, 0, HEAPID_STAFF_ROLL );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, TITLE_RES_LOGO_NCGR, GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_STAFF_ROLL );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, TITLE_RES_LOGO_NSCR, GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_STAFF_ROLL );

	GFL_ARC_CloseDataHandle( ah );

	// フォントパレット
	GFL_ARCHDL_UTIL_TransVramPalette(
		handle, NARC_staff_roll_staffroll_font_nclr,
		PALTYPE_MAIN_BG, MBG_PAL_FONT*0x20, 0x20, HEAPID_STAFF_ROLL );
	GFL_ARCHDL_UTIL_TransVramPalette(
		handle, NARC_staff_roll_staffroll_font_nclr,
		PALTYPE_SUB_BG, SBG_PAL_FONT*0x20, 0x20, HEAPID_STAFF_ROLL );

	// バックグラウンドカラー
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME1_M, BG_COLOR );
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME1_S, BG_COLOR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化
 *
 *  @param		wk		ワーク
 */
//-----------------------------------------------------------------------------
static void InitObj( ARCHANDLE *handle, OBJ_WORK * wk )
{
  HEAPID heapID = HEAPID_STAFF_ROLL_L;

  GFL_CLACT_SYS_Create(&GFL_CLSYSINIT_DEF_DIVSCREEN, &VramTbl, HEAPID_STAFF_ROLL);

  wk->clunit = GFL_CLACT_UNIT_Create(1, 0, heapID);
  GFL_CLACT_UNIT_SetDefaultRend(wk->clunit);

  // ---Nの髪がなびくアニメ---
  {
    u32 arc_cgx;
    u32 arc_pal;
    u32 arc_cel;
    u32 arc_anm;

    // バージョン別でリソース分ける
#if	PM_VERSION == LOCAL_VERSION
    arc_cgx = NARC_staff_roll_n_anim01_b_ncgr;
    arc_pal = NARC_staff_roll_n_anim01_b_nclr;
    arc_cel = NARC_staff_roll_n_anim01_b_ncer;
    arc_anm = NARC_staff_roll_n_anim01_b_nanr;
    ICHI_Printf("ブラックバージョン\n");
#else
    arc_cgx = NARC_staff_roll_n_anim01_w_ncgr;
    arc_pal = NARC_staff_roll_n_anim01_w_nclr;
    arc_cel = NARC_staff_roll_n_anim01_w_ncer;
    arc_anm = NARC_staff_roll_n_anim01_w_nanr;
    ICHI_Printf("ホワイトバージョン\n");
#endif

    wk->clres[OBJ_RES_CHR]  = GFL_CLGRP_CGR_Register( handle, 
        arc_cgx, FALSE, CLSYS_DRAW_SUB, heapID );
    wk->clres[OBJ_RES_PAL] = GFL_CLGRP_PLTT_Register( handle, 
        arc_pal, CLSYS_DRAW_SUB, 0, heapID );
    wk->clres[OBJ_RES_CEL] = GFL_CLGRP_CELLANIM_Register( handle, 
        arc_cel, arc_anm, heapID );
  }

  {
    GFL_CLWK_DATA add;
    add.pos_x   = 128;
    add.pos_y   = 96;
    add.anmseq  = 0;
    add.bgpri   = 1;
    add.softpri = 0;

    wk->clwk    = GFL_CLACT_WK_Create( wk->clunit,
        wk->clres[OBJ_RES_CHR],
        wk->clres[OBJ_RES_PAL],
        wk->clres[OBJ_RES_CEL],
        &add, CLSYS_DEFREND_SUB, heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk, TRUE );
    // 表示フラグOFF
    GFL_CLACT_WK_SetDrawEnable( wk->clwk, FALSE );
    // 半透明OBJ
    //GFL_CLACT_WK_SetObjMode( wk->clwk, GX_OAM_MODE_XLU );
  }

  //OBJ表示ON
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *  @param		wk		ワーク
 */
//-----------------------------------------------------------------------------
static void ExitObj( OBJ_WORK * wk )
{
  GFL_CLGRP_CGR_Release(      wk->clres[OBJ_RES_CHR] );
  GFL_CLGRP_PLTT_Release(     wk->clres[OBJ_RES_PAL] );
  GFL_CLGRP_CELLANIM_Release( wk->clres[OBJ_RES_CEL] );

  GFL_CLACT_UNIT_Delete(wk->clunit);
  GFL_CLACT_SYS_Delete();
}

static void InitCut( CUT_WORK * wk )
{
  u32 chr_size = GFL_ARC_GetDataSize( ARCID_STAFF_ROLL, NARC_staff_roll_2d_cut_cut11_cut11_b_boy_ncgr );
  u32 scr_size = GFL_ARC_GetDataSize( ARCID_STAFF_ROLL, NARC_staff_roll_2d_cut_cut11_cut11_b_boy_nscr );

  wk->chr_buf = GFL_HEAP_AllocMemory( HEAPID_STAFF_ROLL, chr_size );
  wk->scr_buf = GFL_HEAP_AllocMemory( HEAPID_STAFF_ROLL, scr_size );
  ICHI_Printf("chr_size = %x %x \n", chr_size, wk->chr_buf );
  ICHI_Printf("scr_size = %x %x \n", scr_size, wk->scr_buf );
}

static void ExitCut( CUT_WORK * wk )
{
	GFL_HEAP_FreeMemory( wk->chr_buf );
	GFL_HEAP_FreeMemory( wk->scr_buf );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイトルロゴのパレット読み込み
 *
 * @param		wk		ワーク
 * @param		flg		TRUE = ロゴ読み込み, FALSE = フォント読み込み
 *
 * @return	none
 *
 * @li	ロゴは256色なので、フォントのパレットも潰してしまう
 */
//--------------------------------------------------------------------------------------------
static void LoadLogoPalette( SRMAIN_WORK * wk, BOOL flg )
{

}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理
 *
 * @param		tcb		GFL_TCB
 * @param		work	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankTask( GFL_TCB * tcb, void * work )
{
	SRMAIN_WORK * wk = work;

	TransBmpWinChar( wk );
  GFL_BG_VBlankFunc();
	GFL_CLACT_SYS_VBlankFunc();
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理設定
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitVBlank( SRMAIN_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK削除
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitVBlank( SRMAIN_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ関連初期化
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitMsg( SRMAIN_WORK * wk )
{
	if( wk->dat->mojiMode == MOJIMODE_HIRAGANA ){
		wk->mman = GFL_MSG_Create(
								GFL_MSG_LOAD_NORMAL,
								ARCID_MESSAGE, NARC_message_staff_list_jp_dat, HEAPID_STAFF_ROLL );
	}else{
		wk->mman = GFL_MSG_Create(
								GFL_MSG_LOAD_NORMAL,
								ARCID_MESSAGE, NARC_message_staff_list_eng_dat, HEAPID_STAFF_ROLL );
	}
  
	wk->exp  = GFL_STR_CreateBuffer( EXP_BUFF_SIZE, HEAPID_STAFF_ROLL );

	wk->font[SRMAIN_FONT_NORMAL] = GFL_FONT_Create(
																	ARCID_FONT, NARC_font_large_gftr,
																	GFL_FONT_LOADTYPE_MEMORY, FALSE, HEAPID_STAFF_ROLL );

	wk->font[SRMAIN_FONT_SMALL] = GFL_FONT_Create(
																	ARCID_FONT, NARC_font_small_batt_gftr,
																	GFL_FONT_LOADTYPE_MEMORY, FALSE, HEAPID_STAFF_ROLL );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ関連削除
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitMsg( SRMAIN_WORK * wk )
{
	GFL_FONT_Delete( wk->font[SRMAIN_FONT_SMALL] );
	GFL_FONT_Delete( wk->font[SRMAIN_FONT_NORMAL] );

	GFL_STR_DeleteBuffer( wk->exp );
	GFL_MSG_Delete( wk->mman );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列先行展開用仮想ＢＭＰ作成
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CreateVirtualBmp( SRMAIN_WORK * wk )
{
	u16	i;

	i = 0;

	while( 1 ){
		ITEMLIST_DATA * item = &wk->list[i];
    if( item->label == ITEMLIST_LABEL_END ){
      break;
    }else if( item->label == ITEMLIST_LABEL_NONE && item->msgIdx != ITEMLIST_MSG_NONE ){
      ITEMLIST_DATA * sub = &wk->list[i+1];
      if( sub->wait == 0 && sub->label == ITEMLIST_LABEL_NONE && sub->msgIdx != ITEMLIST_MSG_NONE ){
        wk->vBmp[wk->vBmpMax].bmp = GFL_BMP_Create( VBMP_SX, VBMP_SY, GFL_BMP_16_COLOR, HEAPID_STAFF_ROLL );
        wk->vBmp[wk->vBmpMax].idx = i;
        wk->vBmpMax++;
        // 最大数を超えたら
        if( wk->vBmpMax == SRMAIN_VBMP_MAX ){
          break;
        }
        i++;
      }
    }
		
		i++;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列先行展開用仮想ＢＭＰ削除
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ReleaseVirtualBmp( SRMAIN_WORK * wk )
{
	u32	i;

	for( i=0; i<wk->vBmpMax; i++ ){
		GFL_BMP_Delete( wk->vBmp[i].bmp );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		仮想ＢＭＰに文字列を描画
 *
 * @param		wk		ワーク
 * @param		bmp		仮想ＢＭＰ
 * @param		pos		リスト位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void WriteVirtualBmpCore( SRMAIN_WORK * wk, GFL_BMP_DATA * bmp, u32 pos )
{
	ITEMLIST_DATA * item = &wk->list[pos];

	if( item->font == SRMAIN_FONT_NORMAL ){
		PrintStr( wk, item, bmp, 0 );
	}else{
		PrintStr( wk, item, bmp, STR_PRINT_SMALL_OFFSET );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		仮想ＢＭＰに文字列を描画
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void WriteVirtualBmp( SRMAIN_WORK * wk )
{
	if( wk->vBmpCnt < wk->vBmpMax ){
		WriteVirtualBmpCore( wk, wk->vBmp[wk->vBmpCnt].bmp, wk->vBmp[wk->vBmpCnt].idx );
		WriteVirtualBmpCore( wk, wk->vBmp[wk->vBmpCnt].bmp, wk->vBmp[wk->vBmpCnt].idx+1 );
		wk->vBmpCnt++;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		仮想ＢＭＰを画面に表示するＢＭＰに描画
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PrintVirtualBmp( SRMAIN_WORK * wk )
{
	GFL_BMP_Print(
		wk->vBmp[wk->vBmpPut].bmp,
		GFL_BMPWIN_GetBmp(wk->util[1].win),
		0, 0, 0, STR_PRINT_PY, VBMP_SX*8, VBMP_SY*8, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		スタッフリストデータ作成
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CreateListData( SRMAIN_WORK * wk )
{
	if( wk->dat->mojiMode == MOJIMODE_HIRAGANA ){
		wk->list = GFL_ARC_LoadDataAlloc(
								ARCID_STAFF_ROLL, NARC_staff_roll_staff_list_jp_dat, HEAPID_STAFF_ROLL );
	}else{
		wk->list = GFL_ARC_LoadDataAlloc(
								ARCID_STAFF_ROLL, NARC_staff_roll_staff_list_eng_dat, HEAPID_STAFF_ROLL );
	}
	CreateVirtualBmp( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		スタッフリストデータ削除
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void DeleteListData( SRMAIN_WORK * wk )
{
	ReleaseVirtualBmp( wk );
	GFL_HEAP_FreeMemory( wk->list );
}


//============================================================================================
//	ＢＭＰ
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		BMP初期化
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitBmp( SRMAIN_WORK * wk )
{
	GFL_BMPWIN_Init( HEAPID_STAFF_ROLL );
	wk->util[0].win = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 0, 0, 32, 25, MBG_PAL_FONT, GFL_BMP_CHRAREA_GET_B );
	wk->util[1].win = GFL_BMPWIN_Create( GFL_BG_FRAME1_S, 0, 0, 32, 28, SBG_PAL_FONT, GFL_BMP_CHRAREA_GET_B );

	GFL_BMPWIN_MakeTransWindow( wk->util[0].win );
	GFL_BMPWIN_MakeTransWindow( wk->util[1].win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BMP解放
 *
 * @param		wk	ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitBmp( SRMAIN_WORK * wk )
{
	GFL_BMPWIN_Delete( wk->util[0].win );
	GFL_BMPWIN_Delete( wk->util[1].win );
	GFL_BMPWIN_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BMPキャラ転送フラグ設定
 *
 * @param		wk		ワーク
 * @param		flg		フラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetBmpWinTransFlag( SRMAIN_WORK * wk, u32 flg )
{
	wk->bmpTransFlag |= flg;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BMPキャラ転送
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void TransBmpWinChar( SRMAIN_WORK * wk )
{
	if( wk->bmpTransFlag & BMPWIN_TRANS_MAIN_FLAG ){
		GFL_BMPWIN_TransVramCharacter( wk->util[0].win );
	}
	if( wk->bmpTransFlag & BMPWIN_TRANS_SUB_FLAG ){
		GFL_BMPWIN_TransVramCharacter( wk->util[1].win );
	}
	wk->bmpTransFlag = 0;
}


//============================================================================================
//	その他
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードインセット
 *
 * @param		wk		ワーク
 * @param		next	フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetFadeIn( SRMAIN_WORK * wk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_STAFF_ROLL );
	wk->nextSeq = next;
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウトセット
 *
 * @param		wk		ワーク
 * @param		next	フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetFadeOut( SRMAIN_WORK * wk, int next )
{
#if	PM_VERSION == LOCAL_VERSION
	// ブラックアウト
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_STAFF_ROLL );
#else
	// ホワイトアウト
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_WHITE, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_STAFF_ROLL );
#endif
	wk->nextSeq = next;
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		スキップチェック
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CheckSkip( SRMAIN_WORK * wk )
{
	if( wk->dat->fastMode != STAFFROLL_MODE_NORMAL ){
		if( wk->skipCount == 0 ){
			if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
				wk->skipFlag = 1;
			}else{
				wk->skipFlag = 0;
			}
		}
		wk->skipCount = ( wk->skipCount + 1 ) & 3;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		キャラデータを上へシフトする
 *
 * @param		src			元キャラデータ
 * @param		next		１ライン下のキャラデータ
 * @param		shift		シフトする値
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ShiftChar( u32 * src, u32 * next, u32 shift )
{
	u32	i;

	if( next != NULL ){
		for( i=0; i<shift; i++ ){
			next[8-shift+i] = src[i];
		}
	}
	for( i=0; i<8-shift; i++ ){
		src[i] = src[shift+i];
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リストスクロール
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ListScroll( SRMAIN_WORK * wk )
{
	if( wk->listScrollFlg == FALSE ){ return; }

	wk->listScrollCnt += ITEMLIST_SCROLL_SPEED;
	if( wk->listScrollCnt >= FX32_ONE ){
		wk->listScrollCnt -= FX32_ONE;
	}else{
		return;
	}

	// ここでシフト処理
	{
		u8 * mBmp;
		u8 * sBmp;
		u32 p1;
		u32 p2;
		u32	shift;
		u16	i, j;

		// シフト値
		if( wk->skipFlag == 0 ){
			shift = 1;
		}else{
			shift = SKIP_SPEED;
		}

		mBmp = GFL_BMP_GetCharacterAdrs( GFL_BMPWIN_GetBmp(wk->util[0].win) );
		sBmp = GFL_BMP_GetCharacterAdrs( GFL_BMPWIN_GetBmp(wk->util[1].win) );

		// 上画面をシフト
		p1 = 0;
		for( j=0; j<32; j++ ){
			ShiftChar( (u32 *)&mBmp[p1], NULL, shift );
			p1 += 0x20;
		}
		for( i=1; i<25; i++ ){
			p1 = ( i << 5 ) << 5;
			p2 = ( ( i - 1 ) << 5 ) << 5;
			for( j=0; j<32; j++ ){
				ShiftChar( (u32 *)&mBmp[p1], (u32 *)&mBmp[p2], shift );
				p1 += 0x20;
				p2 += 0x20;
			}
		}

		// 下画面をシフト
		p1 = 0;
		p2 = ( 24 << 5 ) << 5;
		for( j=0; j<32; j++ ){
			ShiftChar( (u32 *)&sBmp[p1], (u32 *)&mBmp[p2], shift );
			p1 += 0x20;
			p2 += 0x20;
		}
		for( i=1; i<28; i++ ){
			p1 = ( i << 5 ) << 5;
			p2 = ( ( i - 1 ) << 5 ) << 5;
			for( j=0; j<32; j++ ){
				ShiftChar( (u32 *)&sBmp[p1], (u32 *)&sBmp[p2], shift );
				p1 += 0x20;
				p2 += 0x20;
			}
		}

		SetBmpWinTransFlag( wk, BMPWIN_TRANS_MAIN_FLAG|BMPWIN_TRANS_SUB_FLAG );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字描画
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 * @param		bmp		描画先
 * @param		py		表示Ｙ座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PrintStr( SRMAIN_WORK * wk, ITEMLIST_DATA * item, GFL_BMP_DATA * bmp, u32 py )
{
	u32	px;

	GFL_MSG_GetString( wk->mman, item->msgIdx, wk->exp );
	// 左詰め
	if( item->putMode == STR_PUT_MODE_LEFT ){
		px = item->px + item->offs_x;
	// 右詰め
	}else if( item->putMode == STR_PUT_MODE_RIGHT ){
		px = item->px + item->offs_x - PRINTSYS_GetStrWidth( wk->exp, wk->font[item->font], 0 );
	// 中央
	}else if( item->putMode == STR_PUT_MODE_CENTER ){
		px = item->px + item->offs_x - ( PRINTSYS_GetStrWidth( wk->exp, wk->font[item->font], 0 ) >> 1 );
	}else{
		// アサート
	}
	PRINTSYS_PrintColor( bmp, px, py, wk->exp, wk->font[item->font], FontColorTbl[item->color] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		一行文字列描画
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MakeScrollStr( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
	if( item->font == SRMAIN_FONT_NORMAL ){
		PrintStr( wk, item, GFL_BMPWIN_GetBmp(wk->util[1].win), STR_PRINT_PY );
	}else{
		PrintStr( wk, item, GFL_BMPWIN_GetBmp(wk->util[1].win), STR_PRINT_SMALL_PY );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		一括文字列描画
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MakePutStr( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
	GFL_BMP_DATA * bmp;
	u32	py;

	if( item->wait < 256 ){
		bmp = GFL_BMPWIN_GetBmp( wk->util[0].win );
		py  = item->wait;
		wk->putFrame |= BMPWIN_TRANS_MAIN_FLAG;
	}else{
		bmp = GFL_BMPWIN_GetBmp( wk->util[1].win );
		py  = item->wait - 256;
		wk->putFrame |= BMPWIN_TRANS_SUB_FLAG;
	}

	PrintStr( wk, item, bmp, py );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		一括文字列表示
 *
 * @param		wk		ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL PutStr( SRMAIN_WORK * wk )
{
	switch( wk->labelSeq ){
	case 0:
		if( wk->putFrame & BMPWIN_TRANS_MAIN_FLAG ){
			G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, GX_BLEND_PLANEMASK_BD|GX_BLEND_PLANEMASK_BG0, 0, 16 );
		}
		if( wk->putFrame & BMPWIN_TRANS_SUB_FLAG ){
			G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, GX_BLEND_PLANEMASK_BD|GX_BLEND_PLANEMASK_BG0, 0, 16 );
		}
		wk->bmpTransFlag = wk->putFrame;
		wk->labelSeq++;
		break;

	case 1:
		if( wk->skipFlag == 1 ){
			wk->britness += ( STR_FADE_SPEED * SKIP_SPEED );
		}else{
			wk->britness += STR_FADE_SPEED;
		}
		{
			s32	p1, p2;
			if( wk->britness > 16 ){
				p1 = 16;
				p2 = 0;
			}else{
				p1 = wk->britness;
				p2 = 16 - wk->britness;
			}
			if( wk->putFrame & BMPWIN_TRANS_MAIN_FLAG ){
				G2_ChangeBlendAlpha( p1, p2 );
			}
			if( wk->putFrame & BMPWIN_TRANS_SUB_FLAG ){
				G2S_ChangeBlendAlpha( p1, p2 );
			}
		}
		if( wk->britness >= 16 ){
			if( wk->putFrame & BMPWIN_TRANS_MAIN_FLAG ){
				G2_BlendNone();
			}
			if( wk->putFrame & BMPWIN_TRANS_SUB_FLAG ){
				G2S_BlendNone();
			}
			wk->britness = 0;
			wk->labelSeq = 0;
			return FALSE;
		}
		break;
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列クリア
 *
 * @param		wk		ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL ClearStr( SRMAIN_WORK * wk )
{
	switch( wk->labelSeq ){
	case 0:
		if( wk->putFrame & BMPWIN_TRANS_MAIN_FLAG ){
			G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, GX_BLEND_PLANEMASK_BD|GX_BLEND_PLANEMASK_BG0, 16, 0 );
		}
		if( wk->putFrame & BMPWIN_TRANS_SUB_FLAG ){
			G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, GX_BLEND_PLANEMASK_BD|GX_BLEND_PLANEMASK_BG0, 16, 0 );
		}
		wk->labelSeq++;

	case 1:
		if( wk->skipFlag == 1 ){
			wk->britness += ( STR_FADE_SPEED * SKIP_SPEED );
		}else{
			wk->britness += STR_FADE_SPEED;
		}
		{
			s32	p1, p2;
			if( wk->britness > 16 ){
				p1 = 0;
				p2 = 16;
			}else{
				p1 = 16 - wk->britness;
				p2 = wk->britness;
			}
			if( wk->putFrame & BMPWIN_TRANS_MAIN_FLAG ){
				G2_ChangeBlendAlpha( p1, p2 );
			}
			if( wk->putFrame & BMPWIN_TRANS_SUB_FLAG ){
				G2S_ChangeBlendAlpha( p1, p2 );
			}
		}
		if( wk->britness >= 16 ){
			if( wk->putFrame & BMPWIN_TRANS_MAIN_FLAG ){
				GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->util[0].win), 0 );
			}
			if( wk->putFrame & BMPWIN_TRANS_SUB_FLAG ){
				GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->util[1].win), 0 );
			}
			wk->bmpTransFlag = wk->putFrame;
			wk->britness = 0;
			wk->labelSeq++;
		}
		break;

	case 2:
		if( wk->putFrame & BMPWIN_TRANS_MAIN_FLAG ){
			G2_BlendNone();
		}
		if( wk->putFrame & BMPWIN_TRANS_SUB_FLAG ){
			G2S_BlendNone();
		}
		wk->putFrame = 0;
		wk->labelSeq = 0;
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ロゴ表示
 *
 * @param		wk		ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL PutLogo( SRMAIN_WORK * wk )
{
	MtxFx22 mtx;

	switch( wk->labelSeq ){
	case 0:
		//LoadLogoPalette( wk, TRUE );
		G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG3, GX_BLEND_PLANEMASK_BD, 0, 16 );
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
		GX_SetMasterBrightness( 0 );
		GXS_SetMasterBrightness( 0 );
		wk->logo_sync = 0;
		wk->labelSeq++;
		break;

	case 1:
    {
      s16 ev1;
      s16 ev2;

      if( wk->skipFlag == 1 ){
        ev1 = 0 + 16 * wk->logo_sync / LOGO_FADE_SYNC_SKIP;
        ev2 = 16 - 16 * wk->logo_sync / LOGO_FADE_SYNC_SKIP;
      }else{
        ev1 = 0 + 16 * wk->logo_sync / LOGO_FADE_SYNC;
        ev2 = 16 - 16 * wk->logo_sync / LOGO_FADE_SYNC;
      }
      wk->logo_sync++;

      if( ev1 >= 16 ){
        G2_ChangeBlendAlpha( 16, 0 );
        wk->logo_sync = 0;
        wk->listWait = LOGO_PUT_WAIT;
        wk->labelSeq++;
      }else{
        G2_ChangeBlendAlpha( ev1, ev2 );
      }
    }
		break;

	case 2:
		if( wk->skipFlag == 1 ){
			wk->listWait -= SKIP_SPEED;
		}else{
			wk->listWait--;
		}
		if( wk->listWait <= 0 ){
			wk->listWait = 0;
			wk->labelSeq++;
		}
/*
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
			OS_Printf( "[1] push a button : start %d, wait = %d\n", LOGO_PUT_WAIT, wk->listWait );
		}
*/
		break;

	case 3:
    {
      s16 ev1;
      s16 ev2;

      if( wk->skipFlag == 1 ){
        ev1 = 16 - 16 * wk->logo_sync / LOGO_FADE_SYNC_SKIP;
        ev2 = 0 + 16 * wk->logo_sync / LOGO_FADE_SYNC_SKIP;
      }else{
        ev1 = 16 - 16 * wk->logo_sync / LOGO_FADE_SYNC;
        ev2 = 0 + 16 * wk->logo_sync / LOGO_FADE_SYNC;
      }
      wk->logo_sync++;

      if( ev2 >= 16 ){
        G2_BlendNone();
        GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );
        //LoadLogoPalette( wk, FALSE );
        wk->logo_sync = 0;
        wk->labelSeq = 0;
        return FALSE;
      }else{
        G2_ChangeBlendAlpha( ev1, ev2 );
      }
    }
		break;
	}

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カットのメイン
 *
 *	@param	SRMAIN_WORK * wk  ワーク 
 *
 *	@return none
 */
//-----------------------------------------------------------------------------
static void CutMain( SRMAIN_WORK *p_parent_wk, CUT_WORK *p_wk )
{
  u32 sex;  //性別
  ARCHANDLE *handle = p_parent_wk->handle;

#ifdef	PM_DEBUG
  if( p_parent_wk->dat->fastMode == STAFFROLL_MODE_DEBUG ){
    sex = 0; // デバッグだとマイステータスがないので0
  }
  else
  {
    sex = MyStatus_GetMySex( p_parent_wk->dat->mystatus );
  }
#else
  sex = MyStatus_GetMySex( p_parent_wk->dat->mystatus );
#endif

  // シーケンス別処理
  switch( p_wk->seq ){
  case SEQ_CUT_NONE:       // なし
    /* 何もしない */
    return;

  case SEQ_CUT_LOAD:       // リソース読み込み
    {
      // 2Dカットの読み込み
      // アルファの設定
      // 上画面
      if( p_wk->disp_type & CUT_DISP_MAIN ) 
      {
        G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0, GX_BLEND_PLANEMASK_BD|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3, 0, 16 );
      }

      // 下画面
      if( p_wk->disp_type & CUT_DISP_SUB ) 
      {
        G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0, GX_BLEND_PLANEMASK_BD|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3, 0, 16 );
      }

      ICHI_Printf("2Dカット：読み込み\n");
      p_wk->seq = SEQ_CUT_CGX_LOAD;
    }
    break;

  case SEQ_CUT_LOAD_N:     // リソース読み込み(Nのアニメ)
    {
      // 輝度の設定
      // OBJとBGを同時にアルファは出来ないので輝度で代用する
#if	PM_VERSION == LOCAL_VERSION
        // 黒
        G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_OBJ, -16 );
#else
        // 白
        G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_OBJ, 16 );
#endif

      // OBJの表示
      GFL_CLACT_WK_SetDrawEnable( p_parent_wk->obj.clwk, TRUE );
      GFL_CLACT_WK_ResetAnm( p_parent_wk->obj.clwk );

      ICHI_Printf("2Dカット：Nアニメ読み込み\n");
      p_wk->seq = SEQ_CUT_CGX_LOAD;
    }
    break;

  case SEQ_CUT_CGX_LOAD:   // キャラ読み込み
    {
      u32 arc_cgx = CutGetDatID( p_wk, CUT_DATID_CGX, sex );

#if 0
      // 一括読み込み
      GFL_ARC_LoadDataByHandle( handle, arc_cgx, p_wk->chr_buf );

      if( NNS_G2dGetUnpackedBGCharacterData(p_wk->chr_buf,&p_wk->chr) == FALSE ){
        GF_ASSERT( 0 );
      }
      p_wk->seq = SEQ_CUT_SCR_LOAD;
#else
      // 分割読み込み
      u32 size;       // 読み込みサイズ
      u32 offs;       // 読み込みオフセット
      void *buffer;    // 読み込み先バッファ

      // ファイルサイズ取得
      size = GFL_ARC_GetDataSize( ARCID_STAFF_ROLL, arc_cgx );
      size /= BG_LOAD_DIV_FRAME;

      // オフセット = サイズ×経過フレーム
      offs = size * p_wk->div_cnt;

      // 読込先バッファ
      buffer = (void*)((u32)p_wk->chr_buf + offs);

      ICHI_Printf("ファイルサイズ %x オフセット %x 先バッファ %x \n", size, offs, buffer );

      // 分割読み込み
      GFL_ARC_LoadDataOfsByHandle( p_parent_wk->handle, arc_cgx, offs, size, buffer );

      // 分割カウント増加
      p_wk->div_cnt++;
      if( p_wk->div_cnt >= BG_LOAD_DIV_FRAME )
      {
        // キャラデータ
        if( NNS_G2dGetUnpackedBGCharacterData(p_wk->chr_buf,&p_wk->chr) == FALSE ){
          GF_ASSERT( 0 );
        }
        p_wk->seq = SEQ_CUT_SCR_LOAD;
      }
#endif
    }
    break;

  case SEQ_CUT_SCR_LOAD:   // スクリーン読み込み
    {
      u32 arc_scr = CutGetDatID( p_wk, CUT_DATID_SCR, sex );

      GFL_ARC_LoadDataByHandle( handle, arc_scr, p_wk->scr_buf );
      GF_ASSERT( p_wk->scr_buf != NULL );
      if( NNS_G2dGetUnpackedScreenData(p_wk->scr_buf,&p_wk->scr) == FALSE ){
        GF_ASSERT( 0 );
      }
      ICHI_Printf("スクリーン読み込み\n");
      p_wk->seq = SEQ_CUT_TRANS;
    }
    break;

  case SEQ_CUT_TRANS:  // 転送
    {
      u32 arc_pal = CutGetDatID( p_wk, CUT_DATID_PAL, sex );
      if( p_wk->disp_type & CUT_DISP_MAIN )
      {
        GFL_BG_LoadScreen( GFL_BG_FRAME0_M, p_wk->scr->rawData, p_wk->scr->szByte, 0 );
        GFL_BG_LoadCharacter( GFL_BG_FRAME0_M, p_wk->chr->pRawData, p_wk->chr->szByte, 0 );
        GFL_ARCHDL_UTIL_TransVramPalette( handle, arc_pal,
            PALTYPE_MAIN_BG_EX, 0x0000, 0, HEAPID_STAFF_ROLL );

        // 表示(アルファがかかっているので見えはしない)
        GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
      }
      if( p_wk->disp_type & CUT_DISP_SUB )
      {
        GFL_BG_LoadScreen( GFL_BG_FRAME0_S, p_wk->scr->rawData, p_wk->scr->szByte, 0 );
        GFL_BG_LoadCharacter( GFL_BG_FRAME0_S, p_wk->chr->pRawData, p_wk->chr->szByte, 0 );
        GFL_ARCHDL_UTIL_TransVramPalette( handle, arc_pal,
            PALTYPE_SUB_BG_EX, 0x4000, 0, HEAPID_STAFF_ROLL );

        // 表示(アルファがかかっているので見えはしない)
        GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
      }
      ICHI_Printf("転送\n");
      p_wk->seq = SEQ_CUT_FADE_IN;
    }
    break;

  case SEQ_CUT_FADE_IN:    // フェードしながら表示
    {
      // 2Dカットのフェード
      if( CutSeqFunc_Fade( p_wk, CUT_FADE_IN, p_parent_wk->skipFlag ) == TRUE )
      {
        ICHI_Printf("2Dカット：フェードイン\n");
        p_wk->seq = SEQ_CUT_VIEW_WAIT;
      }
    }
    break;
  case SEQ_CUT_VIEW_WAIT:  // 表示中
    {
      // ウェイト時間
      if( p_parent_wk->skipFlag == 1 )
      {
        p_wk->wait -= SKIP_SPEED;
      } else {
        p_wk->wait--;
      }

      if( p_wk->wait <= 0 )
      {
        ICHI_Printf("2Dカット：待ち\n");
        p_wk->seq = SEQ_CUT_FADE_OUT;
      }
    }
    break;
  case SEQ_CUT_FADE_OUT:   // フェードしながら非表示
    {
      // 2Dカットのフェード
      if( CutSeqFunc_Fade( p_wk, CUT_FADE_OUT, p_parent_wk->skipFlag ) == TRUE )
      {
        ICHI_Printf("2Dカット：フェードアウト\n");
        p_wk->seq = SEQ_CUT_EXIT;

        // Nアニメを非表示
        if( p_wk->is_n_anime == TRUE )
        {
          GFL_CLACT_WK_SetAutoAnmFlag( p_parent_wk->obj.clwk, FALSE );
          GFL_CLACT_WK_StopAnm( p_parent_wk->obj.clwk );
          GFL_CLACT_WK_SetDrawEnable( p_parent_wk->obj.clwk, FALSE );
        }
      }
    }
    break;
  case SEQ_CUT_EXIT:       // 終了処理
    {
      GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
      GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );

      // ブレンド初期化
      G2_BlendNone();
      G2S_BlendNone();

      p_wk->seq = SEQ_CUT_NONE;
    }
    break;

  default: GF_ASSERT(0);
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  2Dカットのフェード
 *
 *	@param	CUT_WORK *p_wk  2Dカットのワーク
 *	@param  CUT_FADE fade   フェードイン or フェードアウト 
 *
 *	@return フェードが終了するとTRUE
 */
//-----------------------------------------------------------------------------
static BOOL CutSeqFunc_Fade( CUT_WORK *p_wk, CUT_FADE fade, BOOL skipFlag )
{
  int alpha1, alpha2;

  // アルファ計算
  if( skipFlag == TRUE )
  {
    p_wk->alpha_cnt += SKIP_SPEED;
  }
  else
  {
    p_wk->alpha_cnt++;
  }

  if( fade == CUT_FADE_IN )
  {
    // FADE_IN
    alpha1 = 0 + 16 * p_wk->alpha_cnt / p_wk->alpha_fade_sync;
    alpha2 = 16 - 16 * p_wk->alpha_cnt / p_wk->alpha_fade_sync;
  }
  else
  {
    // FADE_OUT
    alpha1 = 16 - 16 * p_wk->alpha_cnt / p_wk->alpha_fade_sync;
    alpha2 = 0 + 16 * p_wk->alpha_cnt / p_wk->alpha_fade_sync;
  }
  
  if( alpha1 < 0 ) { alpha1 = 0; }
  if( alpha2 < 0 ) { alpha2 = 0; }
  if( alpha1 > 16 ) { alpha1 = 16; }
  if( alpha2 > 16 ) { alpha2 = 16; }


  // アルファ設定
  if( p_wk->is_n_anime == TRUE )
  {
    // Nのアニメの場合は
    // アルファの代わりに輝度を使用している
#if	PM_VERSION == LOCAL_VERSION
    alpha2 *= -1;   // マイナスにする
#endif

    G2S_ChangeBlendBrightness( alpha2 );
  }
  else
  {
    if( p_wk->disp_type & CUT_DISP_MAIN ) 
    {
      G2_ChangeBlendAlpha( alpha1, alpha2 );
    }
    if( p_wk->disp_type & CUT_DISP_SUB ) 
    {
      G2S_ChangeBlendAlpha( alpha1, alpha2 );
    }
  }

  // フェード終了
  if( p_wk->alpha_cnt >= p_wk->alpha_fade_sync )
  {
    p_wk->alpha_cnt = 0;
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カットのデータインデックスを取得
 *
 *	@param	CUT_WORK *p_wk
 *	@param	datid_type
 *	@param	sex 
 *
 *	@return 対応したカード
 */
//-----------------------------------------------------------------------------
static u32  CutGetDatID( CUT_WORK *p_wk, CUT_DATID_TYPE datid_type, u32 sex )
{
  int diff = 0;   // 基準からのarc_id の差

  // 基準となるarc_id  要は一番上
  u32 arc_pal = NARC_staff_roll_2d_cut_cut11_cut11_b_boy_nclr;
  u32 arc_cgx = NARC_staff_roll_2d_cut_cut11_cut11_b_boy_ncgr;
  u32 arc_scr = NARC_staff_roll_2d_cut_cut11_cut11_b_boy_nscr;

  diff = p_wk->cut_no * 12;    // 12 リソース種類×男女×バージョン

  // バージョンチェック
#if	PM_VERSION != LOCAL_VERSION
  diff += 6;   // ホワイト基準に。
#endif

  // 男女チェック
  if( sex == PM_FEMALE )
  {
    diff += 3;
  }

  arc_pal += diff;
  arc_cgx += diff;
  arc_scr += diff;

  // 例外処理
  // Nのアニメなら指定
  if( p_wk->is_n_anime == TRUE )
  {
    // バージョン別でリソース分ける
#if	PM_VERSION == LOCAL_VERSION
    arc_pal = NARC_staff_roll_n_cut01_b_nclr;
    arc_cgx = NARC_staff_roll_n_cut01_b_ncgr;
    arc_scr = NARC_staff_roll_n_cut01_b_nscr;
    ICHI_Printf("ブラックバージョン\n");
#else
    arc_pal = NARC_staff_roll_n_cut01_w_nclr;
    arc_cgx = NARC_staff_roll_n_cut01_w_ncgr;
    arc_scr = NARC_staff_roll_n_cut01_w_nscr;
    ICHI_Printf("ホワイトバージョン\n");
#endif
  }
  
  ICHI_Printf("pal %d cgx %d scr %d \n", arc_pal, arc_cgx, arc_scr );

  switch( datid_type ){
  case CUT_DATID_PAL: return arc_pal;
  case CUT_DATID_CGX: return arc_cgx;
  case CUT_DATID_SCR: return arc_scr;
  default:
    GF_ASSERT(0);
    return 0;
  }
}

//============================================================================================
//	コマンド
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：ラベルなし
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_LabelNone( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
	// ウェイト
	if( item->msgIdx == ITEMLIST_MSG_NONE ){
		wk->listWait = item->wait;
		wk->subSeq = SUBSEQ_WAIT;
		return TRUE;
	}

	// 一行文字列
	if( wk->listWait == 0 ){
		wk->listWait = item->wait;
	}
	MakeScrollStr( wk, item );
	if( item[1].label != ITEMLIST_LABEL_NONE || item[1].wait != 0 ){
		wk->subSeq = SUBSEQ_WAIT;
		return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：文字列一括表示
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_LabelStrPut( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
	// 表示タイプ設定
	if( wk->subSeq == SUBSEQ_INIT ){
		wk->labelType = item->labelType;
		wk->subSeq = SUBSEQ_STR_PUT;
	}

	// 文字描画
	MakePutStr( wk, item );

	// 次が一括表示じゃないとき
	if( item[1].label != ITEMLIST_LABEL_STR_PUT ){
		return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：表示クリア
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_LabelStrClear( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
	wk->labelType = item->labelType;
	wk->subSeq = SUBSEQ_STR_CLEAR;
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：スクロール開始
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_LabelScrollStart( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
//	OS_Printf( "■スクロール開始\n" );
	wk->listScrollFlg = TRUE;
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：スクロール停止
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_LabelScrollStop( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
//	OS_Printf( "■スクロール停止\n" );
	wk->listScrollFlg = FALSE;
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：終了
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_LabelEnd( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
	wk->subSeq = SUBSEQ_END;
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：ロゴ表示
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_LabelLogoPut( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：バージョン別文字表示
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_LabelVersion( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
#if	PM_VERSION == LOCAL_VERSION
	item = &item[item->labelType];
#else
	item = &item[item->labelType+1];
#endif

	MakeScrollStr( wk, item );

	wk->listWait = item->wait;
	wk->subSeq = SUBSEQ_WAIT;

	return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  コマンド：カットイン表示
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//-----------------------------------------------------------------------------
static BOOL Comm_LabelCut( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
  ICHI_Printf("***********************************\n");
  ICHI_Printf("カット\n");
  ICHI_Printf("***********************************\n");

  ICHI_Printf("item->msgIdx %d\n", item->msgIdx );
  ICHI_Printf("item->wait %d\n", item->wait );
  ICHI_Printf("item->label %d\n", item->label );
  ICHI_Printf("item->labelType %d\n", item->labelType );
  ICHI_Printf("item->color %d\n", item->color );
  ICHI_Printf("item->font %d\n", item->font );
  ICHI_Printf("item->putMode %d\n", item->putMode );
  ICHI_Printf("item->px %d\n", item->px );
  ICHI_Printf("item->offs_x %d\n", item->offs_x );

  // 初期化
  //GFL_STD_MemClear( &wk->cut, sizeof(CUT_WORK) );
  //メモリをクリアするとアドレスが消される
  //
  wk->cut.cut_no  = item->labelType;
  wk->cut.div_cnt = 0;
  wk->cut.disp_type = 0;
  wk->cut.alpha_cnt = 0;
  wk->cut.alpha_fade_sync = item->px;
  wk->cut.wait = item->wait;
  wk->cut.is_n_anime = FALSE;

  if( item->putMode == STR_PUT_MODE_UP )
  {
    wk->cut.disp_type = CUT_DISP_MAIN;
  }
  else if( item->putMode == STR_PUT_MODE_DOWN )
  {
    wk->cut.disp_type = CUT_DISP_SUB;
  }

  // 読み込み
  wk->cut.seq = SEQ_CUT_LOAD;

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  コマンド：Nのアニメ表示
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//-----------------------------------------------------------------------------
static BOOL Comm_LabelNAnime( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
  ICHI_Printf("***********************************\n");
  ICHI_Printf("Nアニメ\n");
  ICHI_Printf("***********************************\n");

  ICHI_Printf("item->msgIdx %d\n", item->msgIdx );
  ICHI_Printf("item->wait %d\n", item->wait );
  ICHI_Printf("item->label %d\n", item->label );
  ICHI_Printf("item->labelType %d\n", item->labelType );
  ICHI_Printf("item->color %d\n", item->color );
  ICHI_Printf("item->font %d\n", item->font );
  ICHI_Printf("item->putMode %d\n", item->putMode );
  ICHI_Printf("item->px %d\n", item->px );
  ICHI_Printf("item->offs_x %d\n", item->offs_x );

  // 初期化
  //GFL_STD_MemClear( &wk->cut, sizeof(CUT_WORK) );
  //メモリをクリアするとアドレスが消される
  //
  wk->cut.cut_no  = 0;
  wk->cut.div_cnt = 0;
  wk->cut.disp_type = 0;
  wk->cut.alpha_cnt = 0;
  wk->cut.alpha_fade_sync = item->px;
  wk->cut.disp_type = 0;
  wk->cut.wait = item->wait;
  wk->cut.is_n_anime = TRUE;

  wk->cut.disp_type = CUT_DISP_SUB;

  // 読み込み
  wk->cut.seq = SEQ_CUT_LOAD_N;

  return FALSE;
}



//============================================================================================
//	デバッグ
//============================================================================================
#ifdef PM_DEBUG

static const u8 GridChar[] = {
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x80,
	0x88, 0x88, 0x88, 0x88,

	0x00, 0x00, 0x00, 0x60,
	0x00, 0x00, 0x00, 0x60,
	0x00, 0x00, 0x00, 0x60,
	0x00, 0x00, 0x00, 0x60,
	0x00, 0x00, 0x00, 0x60,
	0x00, 0x00, 0x00, 0x60,
	0x00, 0x00, 0x00, 0x60,
	0x88, 0x88, 0x88, 0x68,

	0x06, 0x00, 0x00, 0x80,
	0x06, 0x00, 0x00, 0x80,
	0x06, 0x00, 0x00, 0x80,
	0x06, 0x00, 0x00, 0x80,
	0x06, 0x00, 0x00, 0x80,
	0x06, 0x00, 0x00, 0x80,
	0x06, 0x00, 0x00, 0x80,
	0x86, 0x88, 0x88, 0x88,
};

static int	testCameraIndex = 0;

static void DebugGridSet(void)
{
	GFL_BG_LoadCharacter( GFL_BG_FRAME2_M, GridChar, 0x60, 1 );
	GFL_BG_LoadCharacter( GFL_BG_FRAME3_S, GridChar, 0x60, 1 );

	GFL_BG_ClearScreenCode( GFL_BG_FRAME2_M, 0xf001 );
	GFL_BG_ClearScreenCode( GFL_BG_FRAME3_S, 0xf001 );

	GFL_BG_FillScreen( GFL_BG_FRAME2_M, 0xf002, 15, 0, 1, 24, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( GFL_BG_FRAME3_S, 0xf002, 15, 0, 1, 24, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( GFL_BG_FRAME2_M, 0xf003, 16, 0, 1, 24, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( GFL_BG_FRAME3_S, 0xf003, 16, 0, 1, 24, GFL_BG_SCRWRT_PALIN );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME3_S );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG2, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
}

#endif // PM_DEBUG

