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
#define	LOCAL_VERSION		( VERSION_BLACK )		// デバッグ用（コミット時はブラックにすること！）
#else
#define	LOCAL_VERSION		( VERSION_BLACK )		// 製品版はブラックを基本とする
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

#ifdef	PM_DEBUG
#define	LIST_START_INIT_WAIT	( wk->testStartInitWait )
#define	LIST_START_END_WAIT		( wk->testStartEndWait )
#define	LOGO_PUT_WAIT					( wk->testLogoWait )
#define	DEBUG_LIST_START_INIT_WAIT	( 41 )
#define	DEBUG_LIST_START_END_WAIT		( 0 )
#define	DEBUG_LOGO_PUT_WAIT					( 166 )
#else
#define	LIST_START_INIT_WAIT	( 41 )			// 初期処理のロゴ表示開始までのウェイト
#define	LIST_START_END_WAIT		( 0 )				// 初期処理のロゴ表示終了後のウェイト
#define	LOGO_PUT_WAIT					( 166 )			// 初期処理のロゴ表示中ウェイト
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

// コマンドラベル
enum {
	ITEMLIST_LABEL_NONE = 0,
	ITEMLIST_LABEL_STR_PUT,
	ITEMLIST_LABEL_STR_CLEAR,
	ITEMLIST_LABEL_SCROLL_START,
	ITEMLIST_LABEL_SCROLL_STOP,
	ITEMLIST_LABEL_END,

	ITEMLIST_LABEL_LOGO_PUT,
	ITEMLIST_LABEL_3D_PUT,
	ITEMLIST_LABEL_3D_CLEAR,
	ITEMLIST_LABEL_3D_CAMERA_REQUEST,

	ITEMLIST_LABEL_VERSION,

	ITEMLIST_LABEL_MAX,
};

// 描画位置
enum {
	STR_PUT_MODE_LEFT = 0,		// 左詰め
	STR_PUT_MODE_RIGHT,				// 右詰め
	STR_PUT_MODE_CENTER,			// 中央
};

#define SKIP_SPEED					( 4 )			// スキップ速度
#define	STR_FADE_SPEED			( 4 )			// スキップ時の文字列フェード速度

typedef int (*pCOMM_FUNC)(SRMAIN_WORK*,ITEMLIST_DATA*);

#define	G3D_FADEIN_SPEED	( FX32_ONE / 16 )		// ３Ｄ面のフェードイン速度

#define	ITEMLIST_SCROLL_SPEED		( FX32_CONST(1) )			// リストスクロール速度

#define	SKIP_MODE_BGM_FADE			( 64 )			// スキップ時のBGMフェードフレーム数


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
static void LoadBgGraphic(void);
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

static void Init3D( SRMAIN_WORK * wk );
static void Exit3D( SRMAIN_WORK * wk );
static void Main3D( SRMAIN_WORK * wk );
static void CameraMoveInit( SR3DCAMERA_MOVE * mvwk );
static VecFx32 CameraMoveValMake( const VecFx32 * now, const VecFx32 * mvp, u32 cnt );
static void CameraMoveMain( SRMAIN_WORK * wk );
static void CameraMoveFlagSet( SR3DCAMERA_MOVE * mvwk, BOOL flg );

static void WriteVirtualBmp( SRMAIN_WORK * wk );
static void PrintVirtualBmp( SRMAIN_WORK * wk );
static void PrintStr( SRMAIN_WORK * wk, ITEMLIST_DATA * item, GFL_BMP_DATA * bmp, u32 py );

static int SetFadeIn( SRMAIN_WORK * wk, int next );
static int SetFadeOut( SRMAIN_WORK * wk, int next );
static void CheckSkip( SRMAIN_WORK * wk );
static void ChangeBrightness3D( SRMAIN_WORK * wk );

static void ListScroll( SRMAIN_WORK * wk );

static int CreateItemData( SRMAIN_WORK * wk );

static BOOL PutStr( SRMAIN_WORK * wk );
static BOOL ClearStr( SRMAIN_WORK * wk );

static BOOL PutLogo( SRMAIN_WORK * wk );

static BOOL Comm_LabelNone( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelStrPut( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelStrClear( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelScrollStart( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelScrollStop( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelEnd( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelLogoPut( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_Label3DPut( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_Label3DClear( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_Label3DCameraRequest( SRMAIN_WORK * wk, ITEMLIST_DATA * item );
static BOOL Comm_LabelVersion( SRMAIN_WORK * wk, ITEMLIST_DATA * item );

#ifdef	PM_DEBUG
static void DebugGridSet(void);
static void DebugCameraPrint( SRMAIN_WORK * wk );
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
	GX_VRAM_BG_128_D,							// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,				// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_NONE,							// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_NONE,					// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_01_AB,						// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_0123_E,				// テクスチャパレットスロット

	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_128K		// サブOBJマッピングモード
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
	Comm_Label3DPut,						// ラベル：３Ｄ表示
	Comm_Label3DClear,					// ラベル：３Ｄクリア
	Comm_Label3DCameraRequest,	// ラベル：３Ｄカメラ移動リクエスト

	Comm_LabelVersion,			// ラベル：バージョン別処理
};

// リソーステーブル
static const GFL_G3D_UTIL_RES G3DUtilResTbl[] =
{
#if	PM_VERSION == LOCAL_VERSION
	{ ARCID_STAFF_ROLL, NARC_staff_roll_staffroll_b_nsbmd, GFL_G3D_UTIL_RESARC },		// 00: モデル
	{ ARCID_STAFF_ROLL, NARC_staff_roll_staffroll_b_nsbca, GFL_G3D_UTIL_RESARC },		// 01: アニメ
	{ ARCID_STAFF_ROLL, NARC_staff_roll_staffroll_b_nsbtp, GFL_G3D_UTIL_RESARC },		// 02: アニメ
#else
	{ ARCID_STAFF_ROLL, NARC_staff_roll_staffroll_w_nsbmd, GFL_G3D_UTIL_RESARC },		// 00: モデル
	{ ARCID_STAFF_ROLL, NARC_staff_roll_staffroll_w_nsbca, GFL_G3D_UTIL_RESARC },		// 01: アニメ
	{ ARCID_STAFF_ROLL, NARC_staff_roll_staffroll_w_nsbtp, GFL_G3D_UTIL_RESARC },		// 02: アニメ
#endif
};

// アニメテーブル
static const GFL_G3D_UTIL_ANM G3DUtil_AnmTbl[] =
{
	{ 1, 0 },		// 00: アニメ
	{ 2, 0 },		// 01: アニメ
};

// 3D OBJテーブル
static const GFL_G3D_UTIL_OBJ G3DUtilObjTbl[] =
{
	{ 0, 0, 0, G3DUtil_AnmTbl, NELEMS(G3DUtil_AnmTbl) },
};

// 設定データ
static const GFL_G3D_UTIL_SETUP G3DUtilSetup = {
	G3DUtilResTbl, NELEMS(G3DUtilResTbl),
	G3DUtilObjTbl, NELEMS(G3DUtilObjTbl),
};

//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light_data[] = {
  { 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 1, {{  (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
};
static const GFL_G3D_LIGHTSET_SETUP light3d_setup = { light_data, NELEMS(light_data) };

// カメラ座標（設定は増田さん）
static const SR3DCAMERA_PARAM CameraMoveTable[] =
{
#if	PM_VERSION == LOCAL_VERSION
	{ { 3681, 94888, 42318 }, { 0, 124745, 0 } },
	{ { 1636, 95706, 63177 }, { 0, 124745, 0 } },
	{ { -5317, 111657, 78310 }, { 0, 124745, 0 } },
	{ { -7362, 159919, 128208 }, { 32311, 124745, 0 } },
	{ { -64622, 150921, 81582 }, { 245809, 108794, 129244 } },
	{ { -17178, 156238, 90580 }, { 199183, 98978, -41718 } },
	{ { -18405, 189776, 93443 }, { 959514, 154602, 0 } },
#else
	{ { 7362, 78119, 2236 }, { 0, 124745, 0 } },
	{ { 1636, 120655, 48453 }, { 0, 124745, 0 } },
	{ { -39264, 151739, 53361 }, { 179960, 124745, 0 } },
	{ { -8998, 153784, 137615 }, { 47853, 124745, 0 } },

	{ { -64622, 159919, 81582 }, { 245809, 108794, 129244 } },
	{ { -19223, 155011, 93443 }, { 199183, 98978, -41718 } },
	{ { -21677, 148467, 185468 }, { 1908803, 121064, -41718 } },
#endif
};

static const GFL_G3D_OBJSTATUS ObjStatus =
{
	{ 0, 0, 0 },																				// trans
	{ FX32_ONE, FX32_ONE, FX32_ONE },										// scale
	{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	// rotate
};

#ifdef PM_DEBUG
static VecFx32 test_pos = {0,0,0};
static VecFx32 test_target = {0,0,0};
#endif



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

	Main3D( wk );

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
	GX_SetMasterBrightness( -16 );
	GXS_SetMasterBrightness( -16 );

#ifdef	PM_DEBUG
	wk->testStartInitWait = DEBUG_LIST_START_INIT_WAIT;
	wk->testLogoWait = DEBUG_LOGO_PUT_WAIT;
	wk->testStartEndWait = DEBUG_LIST_START_END_WAIT;
#endif	// PM_DEBUG

	InitVram();
	InitBg();
	LoadBgGraphic();
	InitMsg( wk );
	InitBmp( wk );
	Init3D( wk );

	CreateListData( wk );

	InitVBlank( wk );

	PMSND_PlayBGM_WideChannel( SEQ_BGM_ENDING );

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
	if( PMSND_CheckFadeOnBGM() == TRUE ){
		return MAINSEQ_RELEASE;
	}
	if( PMSND_CheckPlayBGM() == TRUE ){
		PMSND_StopBGM();
	}

	ExitVBlank( wk );

	DeleteListData( wk );

	Exit3D( wk );
	ExitBmp( wk );
	ExitMsg( wk );
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
		if( PMSND_CheckPlayBGM() == TRUE ){
		  PMSND_FadeOutBGM( SKIP_MODE_BGM_FADE );
		}
		return SetFadeOut( wk, MAINSEQ_RELEASE );
	}

	ListScroll( wk );

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
			GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BGMODE_0, GX_BG0_AS_3D,
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
			GX_BG_EXTPLTT_23, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_256X16 );
	}

	{	// 回転拡縮小初期化
		MtxFx22 mtx;
	  GFL_CALC2D_GetAffineMtx22( &mtx, 0, FX32_ONE*9, FX32_ONE*9, GFL_CALC2D_AFFINE_MAX_NORMAL );
		GFL_BG_SetAffine( GFL_BG_FRAME3_M, &mtx, 128, 96 );
	}

	{	// サブ画面：文字面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
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
static void LoadBgGraphic(void)
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( TITLE_RES_ARCID, HEAPID_STAFF_ROLL_L );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, TITLE_RES_LOGO_NCGR, GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_STAFF_ROLL );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, TITLE_RES_LOGO_NSCR, GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_STAFF_ROLL );

	GFL_ARC_CloseDataHandle( ah );

	// フォントパレット
	ah = GFL_ARC_OpenDataHandle( ARCID_STAFF_ROLL, HEAPID_STAFF_ROLL_L );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_staff_roll_staffroll_font_nclr,
		PALTYPE_MAIN_BG, MBG_PAL_FONT*0x20, 0x20, HEAPID_STAFF_ROLL );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_staff_roll_staffroll_font_nclr,
		PALTYPE_SUB_BG, SBG_PAL_FONT*0x20, 0x20, HEAPID_STAFF_ROLL );
	GFL_ARC_CloseDataHandle( ah );

	// バックグラウンドカラー
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME1_M, BG_COLOR );
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME1_S, BG_COLOR );
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
	if( flg == TRUE ){
		ARCHANDLE * ah = GFL_ARC_OpenDataHandle( TITLE_RES_ARCID, HEAPID_STAFF_ROLL_L );
		GFL_ARCHDL_UTIL_TransVramPalette(
			ah, TITLE_RES_LOGO_NCLR, PALTYPE_MAIN_BG, 0, 0, HEAPID_STAFF_ROLL );
		GFL_ARC_CloseDataHandle( ah );
	}else{
		// フォントパレット
		ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_STAFF_ROLL, HEAPID_STAFF_ROLL_L );
		GFL_ARCHDL_UTIL_TransVramPalette(
			ah, NARC_staff_roll_staffroll_font_nclr,
			PALTYPE_MAIN_BG, MBG_PAL_FONT*0x20, 0x20, HEAPID_STAFF_ROLL );
		GFL_ARC_CloseDataHandle( ah );
	}
	// バックグラウンドカラー
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME1_M, BG_COLOR );
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
	u16	flg;
	u16	i;

	flg = 0;
	i = 0;

	while( 1 ){
		ITEMLIST_DATA * item = &wk->list[i];
		if( flg == 0 ){
			if( item->label == ITEMLIST_LABEL_3D_PUT ){ flg = 1; }
		}else{
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
//	3D
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		３Ｄ関連初期化
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void Init3D( SRMAIN_WORK * wk )
{
	// ３Ｄシステム起動
	GFL_G3D_Init(
		GFL_G3D_VMANLNK,		// テクスチャマネージャ使用モード（フレームモード（とりっぱなし））
		GFL_G3D_TEX256K,		// テクスチャマネージャサイズ 128KB(1ブロック)
		GFL_G3D_VMANLNK,		// パレットマネージャ使用モード（フレームモード（とりっぱなし））
		GFL_G3D_PLT32K,			// パレットマネージャサイズ
		0x1000,							// ジオメトリバッファの使用サイズ
		HEAPID_STAFF_ROLL,	// ヒープID
		NULL );							// セットアップ関数(NULLの時はDefaultSetUp)

	// ハンドル作成
	wk->g3d_util = GFL_G3D_UTIL_Create( 3, 1, HEAPID_STAFF_ROLL );
	wk->g3d_unit = GFL_G3D_UTIL_AddUnit( wk->g3d_util, &G3DUtilSetup );

	{
		u32	i;

		wk->g3d_obj = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->g3d_unit );
		wk->g3d_anm = GFL_G3D_OBJECT_GetAnimeCount( wk->g3d_obj );

		for( i=0; i<wk->g3d_anm; i++ ){
			GFL_G3D_OBJECT_EnableAnime( wk->g3d_obj, i );
		}
	}

  // ライト作成
	wk->g3d_light = GFL_G3D_LIGHT_Create( &light3d_setup, HEAPID_STAFF_ROLL );
  GFL_G3D_LIGHT_Switching( wk->g3d_light );

  // カメラ初期設定
	{
		VecFx32 up			= { 0, FX32_ONE, 0 };

		wk->g3d_camera = GFL_G3D_CAMERA_Create(
											GFL_G3D_PRJPERS, 
											FX_SinIdx( defaultCameraFovy/2 * PERSPWAY_COEFFICIENT ),
											FX_CosIdx( defaultCameraFovy/2 * PERSPWAY_COEFFICIENT ),
											defaultCameraAspect, 0,
											defaultCameraNear, defaultCameraFar, 0,
											&CameraMoveTable[0].pos, &up, &CameraMoveTable[0].target, HEAPID_STAFF_ROLL );
		GFL_G3D_CAMERA_Switching( wk->g3d_camera );
#ifdef PM_DEBUG
		test_pos = CameraMoveTable[0].pos;
		test_target = CameraMoveTable[0].target;
#endif
	}

	G3X_AntiAlias( TRUE );

	GFL_BG_SetBGControl3D( 2 );				// BG面設定（引数は優先度）

	CameraMoveInit( &wk->cameraMove );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );

#if	PM_VERSION == LOCAL_VERSION
	G2_SetBlendBrightness( GX_PLANEMASK_BG0, -16 );
#else
	G2_SetBlendBrightness( GX_PLANEMASK_BG0, 16 );
#endif

}

//--------------------------------------------------------------------------------------------
/**
 * @brief		３Ｄ関連解放
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void Exit3D( SRMAIN_WORK * wk )
{
	GFL_G3D_CAMERA_Delete( wk->g3d_camera );
	GFL_G3D_LIGHT_Delete( wk->g3d_light );

	GFL_G3D_UTIL_DelUnit( wk->g3d_util, wk->g3d_unit );
	GFL_G3D_UTIL_Delete( wk->g3d_util );

	GFL_G3D_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		３Ｄメイン
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void Main3D( SRMAIN_WORK * wk )
{
	// ２回目は表示しない
	if( wk->dat->fastMode == STAFFROLL_MODE_FAST ){
		return;
	}
	// ３Ｄ面が非表示のとき
	if( ( GFL_DISP_GetMainVisible() & GX_PLANEMASK_BG0 ) == 0 ){
		return;
	}

#ifdef PM_DEBUG
	DebugCameraPrint( wk );
#endif

	ChangeBrightness3D( wk );

#ifdef	PMDEBUG
	if( wk->debugStopFlg == FALSE ){
		CameraMoveMain( wk );
	}
#else
	CameraMoveMain( wk );
#endif

	{
		u32	i;

		for( i=0; i<wk->g3d_anm; i++ ){
			GFL_G3D_OBJECT_LoopAnimeFrame( wk->g3d_obj, i, FX32_ONE );
		}
	}

	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();
	GFL_G3D_DRAW_DrawObject( wk->g3d_obj, &ObjStatus );
	GFL_G3D_DRAW_End();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カメラ移動初期化
 *
 * @param		mvwk		カメラ移動ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CameraMoveInit( SR3DCAMERA_MOVE * mvwk )
{
	u32	i;

	mvwk->tbl    = CameraMoveTable;
	mvwk->tblMax = NELEMS(CameraMoveTable);
	mvwk->cnt    = 0;
	mvwk->pos    = 0;
	mvwk->flg = FALSE;

	for( i=0; i<SRMAIN_CAMERA_REQ_MAX; i++ ){
		mvwk->req[i].no  = 0xffff;
		mvwk->req[i].cnt = 0;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カメラ移動リクエスト
 *
 * @param		mvwk		カメラ移動ワーク
 * @param		no			移動先カメラ番号
 * @param		cnt			移動終了までのフレーム数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CameraMoveRequestSet( SR3DCAMERA_MOVE * mvwk, u16 no, u16 cnt )
{
	u32	i;

	for( i=0; i<SRMAIN_CAMERA_REQ_MAX; i++ ){
		if( mvwk->req[i].no == 0xffff ){
			mvwk->req[i].no  = no;
			mvwk->req[i].cnt = cnt;
			break;
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		次のカメラ移動リクエストへ
 *
 * @param		mvwk		カメラ移動ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CameraMoveRequestShift( SR3DCAMERA_MOVE * mvwk )
{
	u32	i;

	for( i=0; i<SRMAIN_CAMERA_REQ_MAX-1; i++ ){
		mvwk->req[i] = mvwk->req[i+1];
	}
	mvwk->req[SRMAIN_CAMERA_REQ_MAX-1].no  = 0xffff;
	mvwk->req[SRMAIN_CAMERA_REQ_MAX-1].cnt = 0;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カメラ移動量作成
 *
 * @param		now			現在のカメラ座標
 * @param		mvp			移動先のカメラ座標
 * @param		cnt			移動終了までのフレーム数
 *
 * @return	１フレームの移動量
 */
//--------------------------------------------------------------------------------------------
static VecFx32 CameraMoveValMake( const VecFx32 * now, const VecFx32 * mvp, u32 cnt )
{
	VecFx32	vec;

	vec.x = ( GFL_STD_Abs(now->x-mvp->x) << 8 ) / cnt;
	if( now->x > mvp->x ){
		vec.x *= -1;
	}
	vec.y = ( GFL_STD_Abs(now->y-mvp->y) << 8 ) / cnt;
	if( now->y > mvp->y ){
		vec.y *= -1;
	}
	vec.z = ( GFL_STD_Abs(now->z-mvp->z) << 8 ) / cnt;
	if( now->z > mvp->z ){
		vec.z *= -1;
	}
	return vec;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カメラ移動後の座標取得
 *
 * @param		p				現在のカメラ座標
 * @param		mv			１フレームの移動量
 * @param		cnt			現在のフレーム数
 *
 * @return	移動後の座標
 */
//--------------------------------------------------------------------------------------------
static VecFx32 CameraMoveCore( const VecFx32 * p, const VecFx32 * mv, u32 cnt )
{
	VecFx32	vec;
	fx32	val;

	vec = *p;

	val = ( GFL_STD_Abs(mv->x) * cnt ) >> 8;
	if( mv->x > 0 ){
		vec.x += val;
	}else{
		vec.x -= val;
	}
	val = ( GFL_STD_Abs(mv->y) * cnt ) >> 8;
	if( mv->y > 0 ){
		vec.y += val;
	}else{
		vec.y -= val;
	}
	val = ( GFL_STD_Abs(mv->z) * cnt ) >> 8;
	if( mv->z > 0 ){
		vec.z += val;
	}else{
		vec.z -= val;
	}
	return vec;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カメラ移動
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CameraMoveMain( SRMAIN_WORK * wk )
{
	SR3DCAMERA_MOVE * mvwk = &wk->cameraMove;

	if( mvwk->flg == FALSE ){
		return;
	}

	if( mvwk->cnt == 0 ){
		VecFx32	vec;

		mvwk->cntMax = mvwk->req[0].cnt;

		GFL_G3D_CAMERA_GetPos( wk->g3d_camera, &vec );
		mvwk->val.pos = CameraMoveValMake( &vec, &mvwk->tbl[mvwk->req[0].no].pos, mvwk->cntMax );

		GFL_G3D_CAMERA_GetTarget( wk->g3d_camera, &vec );
		mvwk->val.target = CameraMoveValMake( &vec, &mvwk->tbl[mvwk->req[0].no].target, mvwk->cntMax );
	}

	if( mvwk->cnt >= mvwk->cntMax ){
		mvwk->param.pos    = mvwk->tbl[mvwk->req[0].no].pos;
		mvwk->param.target = mvwk->tbl[mvwk->req[0].no].target;
		mvwk->pos = mvwk->req[0].no;
		mvwk->cnt = 0;
		CameraMoveRequestShift( mvwk );
		if( mvwk->req[0].no == 0xffff ){
			CameraMoveFlagSet( mvwk, FALSE );
		}
	}else{
		mvwk->param.pos    = CameraMoveCore( &mvwk->tbl[mvwk->pos].pos, &mvwk->val.pos, mvwk->cnt );
		mvwk->param.target = CameraMoveCore( &mvwk->tbl[mvwk->pos].target, &mvwk->val.target, mvwk->cnt );
		if( wk->skipFlag == 1 ){
			mvwk->cnt += SKIP_SPEED;
		}else{
			mvwk->cnt++;
		}
	}

	GFL_G3D_CAMERA_SetPos( wk->g3d_camera, &mvwk->param.pos );
	GFL_G3D_CAMERA_SetTarget( wk->g3d_camera, &mvwk->param.target );
	GFL_G3D_CAMERA_Switching( wk->g3d_camera );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カメラ移動動作フラグ設定
 *
 * @param		wk		ワーク
 * @param		flg		TRUE = 動作, FALSE = 停止
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CameraMoveFlagSet( SR3DCAMERA_MOVE * mvwk, BOOL flg )
{
	mvwk->flg = flg;
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
 * @brief		３Ｄ面のフェード処理
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ChangeBrightness3D( SRMAIN_WORK * wk )
{
	if( wk->g3d_briCount > FX32_CONST(16) ){ return; }

	if( wk->skipFlag == 1 ){
		wk->g3d_briCount += ( G3D_FADEIN_SPEED * SKIP_SPEED );
	}else{
		wk->g3d_briCount += G3D_FADEIN_SPEED;
	}

#if	PM_VERSION == LOCAL_VERSION
	G2_SetBlendBrightness( GX_PLANEMASK_BG0, -16+wk->g3d_briCount/FX32_ONE );
#else
	G2_SetBlendBrightness( GX_PLANEMASK_BG0, 16-wk->g3d_briCount/FX32_ONE );
#endif
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
		LoadLogoPalette( wk, TRUE );
		G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG3, GX_BLEND_PLANEMASK_BD, 0, 16 );
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
		GX_SetMasterBrightness( 0 );
		GXS_SetMasterBrightness( 0 );
		wk->britness = 0;
		wk->labelSeq++;
		break;

	case 1:
		if( wk->skipFlag == 1 ){
			wk->britness += ( SKIP_SPEED * 2 );
		}else{
			wk->britness += 2;
		}
		if( wk->britness >= 16 ){
			G2_ChangeBlendAlpha( 16, 0 );
		  GFL_CALC2D_GetAffineMtx22( &mtx, 0, FX32_ONE, FX32_ONE, GFL_CALC2D_AFFINE_MAX_NORMAL );
			GFL_BG_SetAffine( GFL_BG_FRAME3_M, &mtx, 128, 96 );
			wk->britness = 0;
			wk->listWait = LOGO_PUT_WAIT;
			wk->labelSeq++;
		}else{
			G2_ChangeBlendAlpha( wk->britness, 16-wk->britness );
		  GFL_CALC2D_GetAffineMtx22( &mtx, 0, FX32_ONE*(8-wk->britness/2), FX32_ONE*(8-wk->britness/2), GFL_CALC2D_AFFINE_MAX_NORMAL );
			GFL_BG_SetAffine( GFL_BG_FRAME3_M, &mtx, 128, 96 );
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
		if( wk->skipFlag == 1 ){
			wk->britness += ( SKIP_SPEED * 2 );
		}else{
			wk->britness += 2;
		}
		if( wk->britness >= 16 ){
			G2_BlendNone();
			GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );
		  GFL_CALC2D_GetAffineMtx22( &mtx, 0, FX32_ONE, FX32_ONE, GFL_CALC2D_AFFINE_MAX_NORMAL );
			GFL_BG_SetAffine( GFL_BG_FRAME3_M, &mtx, 128, 96 );
			LoadLogoPalette( wk, FALSE );
			wk->britness = 0;
			wk->labelSeq = 0;
			return FALSE;
		}else{
			G2_ChangeBlendAlpha( 16-wk->britness, wk->britness );
		  GFL_CALC2D_GetAffineMtx22( &mtx, 0, FX32_ONE*(1+wk->britness/2), FX32_ONE*(1+wk->britness/2), GFL_CALC2D_AFFINE_MAX_NORMAL );
			GFL_BG_SetAffine( GFL_BG_FRAME3_M, &mtx, 128, 96 );
		}
		break;
	}

	return TRUE;
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
 * @brief		コマンド：３Ｄ表示
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_Label3DPut( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：３Ｄクリア
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_Label3DClear( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：３Ｄカメラ移動リクエスト
 *
 * @param		wk		ワーク
 * @param		item	リスト項目データ
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL Comm_Label3DCameraRequest( SRMAIN_WORK * wk, ITEMLIST_DATA * item )
{
	SR3DCAMERA_MOVE * mvwk = &wk->cameraMove;

	CameraMoveRequestSet( mvwk, item->labelType, item->wait );
	CameraMoveFlagSet( mvwk, TRUE );

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

static void DebugCameraPrint( SRMAIN_WORK * wk )
{
	if( wk->debugStopFlg == TRUE ){
		if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ){
			if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){
				test_pos.z -= FX32_ONE/10;
			}
			if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){
				test_pos.z += FX32_ONE/10;
			}
			if( GFL_UI_KEY_GetCont() & PAD_KEY_UP ){
				test_pos.y -= FX32_ONE/10;
			}
			if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN ){
				test_pos.y += FX32_ONE/10;
			}
			if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT ){
				test_pos.x -= FX32_ONE/10;
			}
			if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT ){
				test_pos.x += FX32_ONE/10;
			}
			GFL_G3D_CAMERA_SetPos( wk->g3d_camera, &test_pos );
			OS_Printf( "POS : x = %d, y = %d, z = %d\n", test_pos.x, test_pos.y, test_pos.z );
		}
		if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ){
			if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){
				test_target.z -= FX32_ONE/10;
			}
			if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){
				test_target.z += FX32_ONE/10;
			}
			if( GFL_UI_KEY_GetCont() & PAD_KEY_UP ){
				test_target.y -= FX32_ONE/10;
			}
			if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN ){
				test_target.y += FX32_ONE/10;
			}
			if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT ){
				test_target.x -= FX32_ONE/10;
			}
			if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT ){
				test_target.x += FX32_ONE/10;
			}
			GFL_G3D_CAMERA_SetTarget( wk->g3d_camera, &test_target );
			OS_Printf( "TARGET : x = %d, y = %d, z = %d\n", test_target.x, test_target.y, test_target.z );
		}

		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
			testCameraIndex++;
			if( testCameraIndex >= NELEMS(CameraMoveTable) ){
				testCameraIndex = 0;
			}
			GFL_G3D_CAMERA_SetPos( wk->g3d_camera, &CameraMoveTable[testCameraIndex].pos );
			GFL_G3D_CAMERA_SetTarget( wk->g3d_camera, &CameraMoveTable[testCameraIndex].target );
			GFL_G3D_CAMERA_GetPos( wk->g3d_camera, &test_pos );
			GFL_G3D_CAMERA_GetTarget( wk->g3d_camera, &test_target );
		}

	  GFL_G3D_CAMERA_Switching( wk->g3d_camera );
	}
}

#endif // PM_DEBUG

