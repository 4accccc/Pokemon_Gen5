//============================================================================================
/**
 * @file		box2_main.c
 * @brief		ボックス画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		09.10.05
 *
 *	モジュール名：BOX2MAIN
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/bmp_winframe.h"
#include "sound/pm_sndsys.h"
#include "font/font.naix"
#include "poke_tool/monsno_def.h"
#include "poke_tool/pokerus.h"
#include "item/item.h"

#include "msg/msg_boxmenu.h"

#include "app/app_menu_common.h"
#include "app/bag.h"
#include "app/p_status.h"
#include "app/name_input.h"

#include "box2_main.h"
#include "box2_bmp.h"
#include "box2_obj.h"
#include "box2_seq.h"
#include "box2_ui.h"
#include "box2_bgwfrm.h"
#include "box2_snd_def.h"
#include "box_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	EXP_BUF_SIZE	( 1024 )	// テンポラリメッセージバッファサイズ

#define	BLEND_EV1		( 6 )				// ブレンド係数
#define	BLEND_EV2		( 10 )			// ブレンド係数

// 壁紙
#define	WALL_PY					( 1 )															// 壁紙Ｙ表示座標
#define	WALL_SX					( 21 )														// 壁紙Ｘサイズ
#define	WALL_SY					( 20 )														// 壁紙Ｙサイズ
#define	WALL_CHR_SIZ		( WALL_SX * WALL_SY )							// 壁紙キャラ数
#define	WALL_CGX_POS1		( 1024 - WALL_CHR_SIZ )						// 壁紙キャラ転送位置１
#define	WALL_CGX_POS2		( WALL_CGX_POS1 - WALL_CHR_SIZ )	// 壁紙キャラ転送位置２
#define	WALL_PUT_SPACE	( WALL_SX + 2 )										// 壁紙配置間隔
#define	WALL_SPACE_CHR	( (BOX2MAIN_BG_PAL_WALL1<<12)+WALL_CGX_POS1+0x2b )	// 壁紙隙間キャラ
#define	WALL_SPTAG_CHR	( (BOX2MAIN_BG_PAL_WALL1<<12)+WALL_CGX_POS1+0x54 )	// 壁紙隙間キャラ
#define	WALL_TITLE_POS	( WALL_SX )												// ボックス名表示キャラ位置
#define	WALL_TITLE_SY		( 3 )															// ボックス名表示Ｙサイズ

// エリアチェックデータ
typedef struct {
	u8	lx;		// 左Ｘ座標
	u8	rx;		// 右Ｘ座標
	u8	ty;		// 上Ｙ座標
	u8	by;		// 下Ｙ座標
}AREA_CHECK;

#define	TRAYAREA_LX		( 8 )		// トレイエリア左Ｘ座標
#define	TRAYAREA_RX		( 160-1 )	// トレイエリア右Ｘ座標
#define	TRAYAREA_UY		( 40 )		// トレイエリア上Ｙ座標
#define	TRAYAREA_DY		( 160-1 )	// トレイエリア下Ｙ座標

#define	TRAYGRID_LX		( 12 )		// トレイの左端グリッド座標
#define	TRAYGRID_RX		( 156 )		// トレイの右端グリッド座標
#define	TRAYGRID_SX		( 24 )		// トレイのグリッドＸサイズ
#define	TRAYGRID_SY		( 24 )		// トレイのグリッドＹサイズ

#define	PARTYAREA_SX	( 32 )		// 手持ちエリアのＸサイズ
#define	PARTYAREA_SY	( 24 )		// 手持ちエリアのＹサイズ

// ポケモンアイコン移動
#define	POKEICON_MOVE_CNT	( 8 )	// ポケモンアイコン動作フレーム数

// ポケモンを逃がす
#define	POKE_FREE_CHECK_ONECE	( 15 )																						// 1syncにチェックするポケモン数
#define	POKE_FREE_CHECK_BOX		( BOX_MAX_POS*BOX_MAX_TRAY )											// ボックスポケモン数
#define	POKE_FREE_CHECK_MAX		( POKE_FREE_CHECK_BOX+BOX2OBJ_POKEICON_MINE_MAX )	// チェックするポケモンの総数

/*
// マーキング
#define	MARKING_FRM_SX	( 11 )						// マーキングフレームＸサイズ
#define	MARKING_FRM_SY	( 18 )						// マーキングフレームＹサイズ
#define	MARKING_FRM_PX	( 32 - MARKING_FRM_SX )		// マーキングフレームＸ表示座標
#define	MARKING_FRM_PY	( 26 )						// マーキングフレームＸ表示座標

#define	MARKING_ON_CHR	( 0x2b )					// マーキングキャラ開始位置 ( ON )
#define	MARKING_OFF_CHR	( 0x0b )					// マーキングキャラ開始位置 ( OFF )

// 手持ちポケモンフレーム
#define	PARTYPOKE_FRM_SX	( 11 )		// 手持ちポケモンフレームＸサイズ
#define	PARTYPOKE_FRM_SY	( 18 )		// 手持ちポケモンフレームＹサイズ
#define	PARTYPOKE_FRM_PX	( 2 )		// 手持ちポケモンフレームＸ表示座標
#define	PARTYPOKE_FRM_PY	( 21 )		// 手持ちポケモンフレームＹ表示座標

// ボックス移動フレーム
#define BOXMOVE_FRM_SX		( 32 )		// ボックス移動フレームＸサイズ
#define BOXMOVE_FRM_SY		( 6 )		// ボックス移動フレームＹサイズ
#define BOXMV_PTOUT_FRM_SX	( 32 )		// ボックス移動フレームＸサイズ
#define BOXMV_PTOUT_FRM_SY	( 7 )		// ボックス移動フレームＹサイズ

// 壁紙変更フレーム
#define WPCHG_FRM_SX		( 32 )		// ボックス移動フレームＸサイズ
#define WPCHG_FRM_SY		( 7 )		// ボックス移動フレームＹサイズ
*/


/*
#define	POKEMENU_SCROLL_SPD	( 8 )	// ポケモンメニュースクロール速度
#define	POKEMENU_SCROLL_CNT	( 11 )	// ポケモンメニュースクロールカウント

#define	MARKIN_FRM_SPD		( 8 )	// マーキングフレームスクロール速度
#define	MARKIN_FRM_CNT		( 18 )	// マーキングフレームスクロールカウント

#define	BOXPARTY_BTN_SPD	( 8 )	// 「てもちポケモン」「ポケモンいどう」ボタンスクロール速度
#define	BOXPARTY_BTN_CNT	( 3 )	// 「てもちポケモン」「ポケモンいどう」ボタンスクロールカウント

#define	PARTYPOKE_FRM_SPD	( BOX2MAIN_PARTYPOKE_FRM_SPD )	// 手持ちポケモンフレームスクロール速度
#define	PARTYPOKE_FRM_CNT	( BOX2MAIN_PARTYPOKE_FRM_CNT )	// 手持ちポケモンフレームスクロールカウント
*/

// アイテムアイコン移動
#define	ITEMICON_MOVE_CNT	( 8 )	// アイテムアイコン動作フレーム数

/*
// BGWINフレームデータ
#define	SUBDISP_ITEM_FRM_SX		( 32 )
#define	SUBDISP_ITEM_FRM_SY		( 9 )
#define	SUBDISP_ITEM_FRM_IN_PX	( 0 )
#define	SUBDISP_ITEM_FRM_IN_PY	( 24 )
#define	SUBDISP_WAZA_FRM_SX		( 12 )
#define	SUBDISP_WAZA_FRM_SY		( 10 )
#define	SUBDISP_WAZA_FRM_IN_PX	( 32 )
#define	SUBDISP_WAZA_FRM_IN_PY	( 12 )


// マーキングフレームデータ
#define	WINFRM_MARK_PX			( 21 )
#define	WINFRM_MARK_PY			( 6 )
#define	WINFRM_MARK_IN_START_PY	( 24 )
#define	WINFRM_MARK_MV_CNT		( WINFRM_MARK_IN_START_PY - WINFRM_MARK_PY )

// 手持ちポケモンフレームデータ
#define	WINFRM_PARTYPOKE_LX	( 2 )
#define	WINFRM_PARTYPOKE_RX	( 21 )
#define	WINFRM_PARTYPOKE_PY	( 6 )
#define	WINFRM_PARTYPOKE_INIT_PY	( 24 )
#define	WINFRM_PARTYPOKE_RET_PX		( 24 )
#define	WINFRM_PARTYPOKE_RET_PY		( 15 )



// ポケモン選択メニューフレームデータ
#define	WINFRM_MENU_INIT_PX	( 32 )		// 表示Ｘ座標
#define	WINFRM_MENU_PX		( 21 )		// 表示Ｘ座標
#define	WINFRM_MENU_PY		( 5 )		// 表示Ｙ座標
#define	WINFRM_MENU_SY		( 3 )		// 表示Ｙサイズ


//「ボックスをきりかえる」ボタンフレームデータ
#define	WINFRM_BOXCHG_BTN_PX	( 0 )
#define	WINFRM_BOXCHG_BTN_PY	( 21 )
#define	WINFRM_BOXCHG_BTN_INIT_PY	( 24 )

//「Ｙようすをみる」ボタンフレームデータ
#define	WINFRM_Y_ST_BTN_PX		( 0 )
#define	WINFRM_Y_ST_BTN_PY		( 21 )
#define	WINFRM_Y_ST_BTN_INIT_PY	( 24 )
*/


// 上画面マーキング表示座標
#define	SUBDISP_MARK_PX				( 25 )
#define	SUBDISP_MARK_PY				( 13 )
#define	SUBDISP_MARK_ON_CHAR	( 0x3a )
#define	SUBDISP_MARK_OFF_CHAR	( 0x1a )
#define	SUBDISP_MARK_PAL			( 0 )

// VBLANK FUNC：取得ポケモン移動シーケンス
enum {
	SEIRI_SEQ_ICON_GET = 0,			// アイコン取得中
	SEIRI_SEQ_ICON_PUT,					// アイコン配置
	SEIRI_SEQ_CANCEL_SCROLL_L,	// トレイが切り替わっていた場合のスクロール処理（左）
	SEIRI_SEQ_CANCEL_SCROLL_R,	// トレイが切り替わっていた場合のスクロール処理（右）
	SEIRI_SEQ_SCROLL_L,					// トレイスクロール（左）
	SEIRI_SEQ_SCROLL_R,					// トレイスクロール（右）
	SEIRI_SEQ_TI_SCROLL_U,			// トレイアイコンスクロール（上）
	SEIRI_SEQ_TI_SCROLL_D,			// トレイアイコンスクロール（下）
	SEIRI_SEQ_END,							// 終了
};

//「あずける」モードの預けるボックス選択フレームデータ
#define	WINFRM_PARTYOUT_PX	( 0 )
#define	WINFRM_PARTYOUT_PY	( 6 )
//#define	PTOUT_PUT_CNT_MAX	( 4 )

// VBLANK FUNC：取得ポケモン移動シーケンス（ポケモンをあずける）
enum {
//	AZUKERU_SEQ_EXITPRM_INIT = 0,	//「もどる」ボタン動作初期化
	AZUKERU_SEQ_ICON_GET = 0,			// アイコン取得中

	AZUKERU_SEQ_SCROLL_L,				// トレイが切り替わっていた場合のスクロール処理（左）
	AZUKERU_SEQ_SCROLL_R,				// トレイが切り替わっていた場合のスクロール処理（右）
	AZUKERU_SEQ_MOVE_CANCEL,		// 移動キャンセル

	AZUKERU_SEQ_MOVE_ENTER1,		// 移動実行１
	AZUKERU_SEQ_MOVE_ENTER2,		// 移動実行２

	AZUKERU_SEQ_END,				// 終了
};

// VBLANK FUNC：取得ポケモン移動シーケンス（ポケモンをつれていく）
enum {
	TSURETEIKU_SEQ_ICON_GET = 0,			// アイコン取得中

	TSURETEIKU_SEQ_MOVE_CANCEL1,		// 移動キャンセル１
	TSURETEIKU_SEQ_MOVE_CANCEL2,		// 移動キャンセル２
	TSURETEIKU_SEQ_MOVE_CANCEL3,		// 移動キャンセル３

	TSURETEIKU_SEQ_MOVE_ENTER1,		// 移動実行１
	TSURETEIKU_SEQ_MOVE_ENTER2,		// 移動実行２
//	TSURETEIKU_SEQ_MOVE_ENTER3,		// 移動実行３

	TSURETEIKU_SEQ_END,				// 終了
};

// 手カーソルでポケモン取得するときのデータ
#define	HANDCURSOR_MOVE_CNT		( 4 )
#define	HANDCURSOR_MOVE_DOT		( 2 )

// 名前入力画面用ワーク
typedef struct {
	NAMEIN_PARAM * prm;
	STRBUF * name;
}BOX_NAMEIN_WORK;


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void VBlankTask( GFL_TCB * tcb, void * work );
static u32 MoveBoxGet( BOX2_SYS_WORK * syswk, u32 pos );
static void PutSubDispPokeMark( BOX2_APP_WORK * appwk, BOX2_POKEINFO_DATA * info );

static void MoveGetItemIcon( BOX2_APP_WORK * appwk, u32 x, u32 y );


//============================================================================================
//	グローバル
//============================================================================================

// VRAM割り振り
static const GFL_DISP_VRAM VramTbl = {
/*
	GX_VRAM_BG_128_B,							// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,				// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_64_E,							// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_128_D,				// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_0_A,							// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_01_FG					// テクスチャパレットスロット
*/
/*
	GX_VRAM_BG_128_D,							// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,				// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_16_I,					// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_0_A,							// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_01_FG,				// テクスチャパレットスロット

	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K		// サブOBJマッピングモード
*/
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

// トレイエリア範囲データ
static const AREA_CHECK TrayPokeArea = { TRAYAREA_LX, TRAYAREA_RX, TRAYAREA_UY, TRAYAREA_DY };

// 手持ちエリア範囲データ（左）
static const AREA_CHECK PartyPokeAreaLeft[BOX2OBJ_POKEICON_MINE_MAX] =
{
	{ 26, 26+PARTYAREA_SX-1,  58,  58+PARTYAREA_SY-1 }, { 62, 62+PARTYAREA_SX-1,  66,  66+PARTYAREA_SY-1 },
	{ 26, 26+PARTYAREA_SX-1,  90,  90+PARTYAREA_SY-1 }, { 62, 62+PARTYAREA_SX-1,  98,  98+PARTYAREA_SY-1 },
	{ 26, 26+PARTYAREA_SX-1, 122, 122+PARTYAREA_SY-1 }, { 62, 62+PARTYAREA_SX-1, 130, 130+PARTYAREA_SY-1 },
};

// 手持ちエリア範囲データ（右）
static const AREA_CHECK PartyPokeAreaRight[BOX2OBJ_POKEICON_MINE_MAX] =
{
	{ 178, 178+PARTYAREA_SX-1,  58,  58+PARTYAREA_SY-1 }, { 214, 214+PARTYAREA_SX-1,  66,  66+PARTYAREA_SY-1 },
	{ 178, 178+PARTYAREA_SX-1,  90,  90+PARTYAREA_SY-1 }, { 214, 214+PARTYAREA_SX-1,  98,  98+PARTYAREA_SY-1 },
	{ 178, 178+PARTYAREA_SX-1, 122, 122+PARTYAREA_SY-1 }, { 214, 214+PARTYAREA_SX-1, 130, 130+PARTYAREA_SY-1 },
};

// ボックス移動トレイエリアデータ
static const AREA_CHECK BoxMoveTrayArea[BOX2OBJ_TRAYICON_MAX] =
{
	{  31,  54, 15, 38 }, {  65,  88, 15, 38 }, {  99, 122, 15, 38 },
	{ 133, 156, 15, 38 }, { 167, 190, 15, 38 }, { 201, 224, 15, 38 },
};




//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_InitVBlank( BOX2_SYS_WORK * syswk )
{
	syswk->app->vtask = GFUser_VIntr_CreateTCB( VBlankTask, syswk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_ExitVBlank( BOX2_SYS_WORK * syswk )
{
	GFL_TCB_DeleteTask( syswk->app->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理
 *
 * @param		tcb			GFL_TCB
 * @param		work		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankTask( GFL_TCB * tcb, void * work )
{
/*
	if( syswk->app->vfunk.func != NULL ){
		pBox2Func func = syswk->app->vfunk.func;
		if( func( syswk ) == 0 ){
			syswk->app->vfunk.func = NULL;
		}
	}

	SubDispWinFrmMove( syswk );

	PaletteFadeTrans( syswk->app->pfd );

	GFL_BG_VBlankFunc( syswk->app->bgl );
//↑[GS_CONVERT_TAG]

	BOX2OBJ_AnmMain( syswk->app );
	if( syswk->app->obj_trans_stop == 0 ){
		GFL_CLACT_SYS_Main( syswk->app->crp );
//↑[GS_CONVERT_TAG]
		CATS_RenderOamTrans();
	}

*/
	BOX2_SYS_WORK * syswk = work;

	GFL_BG_VBlankFunc();
	GFL_CLACT_SYS_VBlankFunc();

	PaletteFadeTrans( syswk->app->pfd );

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK内で呼ばれる関数を設定
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	func	関数ポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_VFuncSet( BOX2_APP_WORK * appwk, void * func )
{
	appwk->vfunk.seq  = 0;
	appwk->vfunk.cnt  = 0;
	appwk->vfunk.func = func;
	appwk->vfunk.freq = NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK内で呼ばれる関数を予約
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	func	関数ポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_VFuncReq( BOX2_APP_WORK * appwk, void * func )
{
	appwk->vfunk.freq = func;
}

//--------------------------------------------------------------------------------------------
/**
 * 予約したVBLANK関数をセット
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_VFuncReqSet( BOX2_APP_WORK * appwk )
{
	BOX2MAIN_VFuncSet( appwk, appwk->vfunk.freq );
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
void BOX2MAIN_VramBankSet(void)
{
	GFL_DISP_SetBank( &VramTbl );
}

const GFL_DISP_VRAM * BOX2MAIN_GetVramBankData(void)
{
	return &VramTbl;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_BgInit( BOX2_SYS_WORK * syswk )
{
	GFL_BG_Init( HEAPID_BOX_APP );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：ボタン面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_M );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME1_M, 0x20, 0, HEAPID_BOX_APP );
	}
	{	// メイン画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：壁紙面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x00000,  0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_TEXT );
	}

	{	// サブ画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 0x20, 0, HEAPID_BOX_APP );
	}
	{	// サブ画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// サブ画面：ウィンドウ（技・アイテム）
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_S );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME2_S, 0x20, 0, HEAPID_BOX_APP );
	}

	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2, VISIBLE_ON );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ解放
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_BgExit( BOX2_SYS_WORK * syswk )
{
	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
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
 * @brief		ＢＧグラフィック読み込み
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_BgGraphicLoad( BOX2_SYS_WORK * syswk )
{
	ARCHANDLE * ah;
	GFL_ARCUTIL_TRANSINFO  info;

	ah = GFL_ARC_OpenDataHandle( ARCID_BOX2_GRA, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_box_m_bg1_lz_NCGR, GFL_BG_FRAME1_M, 0, 0, TRUE, HEAPID_BOX_APP );

//	GFL_ARCHDL_UTIL_TransVramBgCharacter(
	GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
		ah, NARC_box_gra_box_m_bg2_lz_NCGR,
		GFL_BG_FRAME2_M, 0, TRUE, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_box_gra_box_m_bg2_lz_NSCR,
		GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_box_m_bg3_lz_NCGR,
		GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_box_gra_box_m_bg3_lz_NSCR,
		GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_box_gra_box_m_bg_NCLR, PALTYPE_MAIN_BG, 0, 0x20*4, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_selwin_lz_NCGR, GFL_BG_FRAME1_M,
		BOX2MAIN_SELWIN_CGX_POS, BOX2MAIN_SELWIN_CGX_SIZ*0x20*2, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_box_gra_selwin_NCLR, PALTYPE_MAIN_BG,
		BOX2MAIN_BG_PAL_SELWIN*0x20, 0x40, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_box_s_bg3_lz_NCGR,
		GFL_BG_FRAME1_S, 0, 0, TRUE, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_box_gra_box_s_bg3_lz_NSCR,
		GFL_BG_FRAME1_S, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_box_s_bg3_lz_NCGR,
		GFL_BG_FRAME2_S, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_box_gra_box_s_bg_NCLR, PALTYPE_SUB_BG, 0, 0, HEAPID_BOX_APP );

	GFL_ARC_CloseDataHandle( ah );

	// タッチバー
	ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
		ah, APP_COMMON_GetBarCharArcIdx(), GFL_BG_FRAME0_M, 0, FALSE, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, APP_COMMON_GetBarPltArcIdx(), PALTYPE_MAIN_BG,
		BOX2MAIN_BG_PAL_TOUCH_BAR*0x20, 0x20, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, APP_COMMON_GetBarScrnArcIdx(),
		GFL_BG_FRAME0_M, 0, 0, FALSE, HEAPID_BOX_APP );
	GFL_BG_ChangeScreenPalette(
		GFL_BG_FRAME0_M, BOX2MAIN_TOUCH_BAR_PX, BOX2MAIN_TOUCH_BAR_PY,
		BOX2MAIN_TOUCH_BAR_SX, BOX2MAIN_TOUCH_BAR_SY, BOX2MAIN_BG_PAL_TOUCH_BAR );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_M );

	info = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
					ah, APP_COMMON_GetBarCharArcIdx(), GFL_BG_FRAME2_M, 0, FALSE, HEAPID_BOX_APP );

	GFL_ARC_CloseDataHandle( ah );

	{	// タッチバーをBG2に展開
		u16 * scr0;
		u16 * scr2;
		u32	i;

		scr0 = GFL_BG_GetScreenBufferAdrs( GFL_BG_FRAME0_M );
		scr2 = GFL_BG_GetScreenBufferAdrs( GFL_BG_FRAME2_M );

		scr0 = &scr0[ BOX2MAIN_TOUCH_BAR_PY*BOX2MAIN_TOUCH_BAR_SX+BOX2MAIN_TOUCH_BAR_PX ];
		scr2 = &scr2[ BOX2MAIN_TOUCH_BAR_PY*BOX2MAIN_TOUCH_BAR_SX+BOX2MAIN_TOUCH_BAR_PX ];

		for( i=0; i<BOX2MAIN_TOUCH_BAR_SX*BOX2MAIN_TOUCH_BAR_SY; i++ ){
			scr2[i] = scr0[i] + info;
		}

		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_M );
	}


	syswk->app->syswinInfo = BmpWinFrame_GraphicSetAreaMan(
														GFL_BG_FRAME0_M,
														BOX2MAIN_BG_PAL_SYSWIN,
														MENU_TYPE_SYSTEM,
														HEAPID_BOX_APP );

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
		BOX2MAIN_BG_PAL_SYSFNT*0x20, 0x20, HEAPID_BOX_APP );
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
		BOX2MAIN_BG_PAL_SYSFNT_S*0x20, 0x20, HEAPID_BOX_APP );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_PaletteFadeInit( BOX2_SYS_WORK * syswk )
{
	syswk->app->pfd = PaletteFadeInit( HEAPID_BOX_APP );
	PaletteFadeWorkAllocSet( syswk->app->pfd, FADE_MAIN_BG, FADE_PAL_ALL_SIZE, HEAPID_BOX_APP );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード解放
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_PaletteFadeExit( BOX2_SYS_WORK * syswk )
{
	PaletteFadeWorkAllocFree( syswk->app->pfd, FADE_MAIN_BG );
	PaletteFadeFree( syswk->app->pfd );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	 アルファブレンド設定
 *
 * @param		flg		TRUE = 有効, FALSE = 無効
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_SetBlendAlpha( BOOL flg )
{
	if( flg == TRUE ){
		G2_SetBlendAlpha(
			GX_BLEND_PLANEMASK_NONE, 
			GX_BLEND_PLANEMASK_BG1 |
			GX_BLEND_PLANEMASK_BG3 |
			GX_BLEND_PLANEMASK_BD,
			BLEND_EV1,
			BLEND_EV2 );
	}else{
		G2_BlendNone();
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_MsgInit( BOX2_SYS_WORK * syswk )
{
	syswk->app->mman = GFL_MSG_Create(
												GFL_MSG_LOAD_NORMAL,
												ARCID_MESSAGE, NARC_message_boxmenu_dat, HEAPID_BOX_APP );
  syswk->app->font = GFL_FONT_Create(
												ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_BOX_APP );
  syswk->app->nfnt = GFL_FONT_Create(
												ARCID_FONT, NARC_font_small_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_BOX_APP );
	syswk->app->wset = WORDSET_Create( HEAPID_BOX_APP );
	syswk->app->que  = PRINTSYS_QUE_Create( HEAPID_BOX_APP );
	syswk->app->exp  = GFL_STR_CreateBuffer( EXP_BUF_SIZE, HEAPID_BOX_APP );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連解放
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_MsgExit( BOX2_SYS_WORK * syswk )
{
	GFL_STR_DeleteBuffer( syswk->app->exp );
	PRINTSYS_QUE_Delete( syswk->app->que );
	WORDSET_Delete( syswk->app->wset );
	GFL_FONT_Delete( syswk->app->nfnt );
	GFL_FONT_Delete( syswk->app->font );
	GFL_MSG_Delete( syswk->app->mman );
}

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえ処理初期化
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_YesNoWinInit( BOX2_SYS_WORK * syswk )
{
//	syswk->app->tsw = TOUCH_SW_AllocWork( HEAPID_BOX_APP );

	syswk->app->ynList[0].str      = GFL_MSG_CreateString( syswk->app->mman, mes_box_yes );
  syswk->app->ynList[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  syswk->app->ynList[0].type     = APP_TASKMENU_WIN_TYPE_NORMAL;

	syswk->app->ynList[1].str      = GFL_MSG_CreateString( syswk->app->mman, mes_box_no );
  syswk->app->ynList[1].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  syswk->app->ynList[1].type     = APP_TASKMENU_WIN_TYPE_NORMAL;

	syswk->app->ynRes = APP_TASKMENU_RES_Create(
												GFL_BG_FRAME0_M, BOX2MAIN_BG_PAL_YNWIN,
												syswk->app->font, syswk->app->que, HEAPID_BOX_APP );
}

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえ処理解放
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_YesNoWinExit( BOX2_SYS_WORK * syswk )
{
	APP_TASKMENU_RES_Delete( syswk->app->ynRes );

	GFL_STR_DeleteBuffer( syswk->app->ynList[1].str );
	GFL_STR_DeleteBuffer( syswk->app->ynList[0].str );

//	TOUCH_SW_FreeWork( syswk->app->tsw );
}

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		カーソル位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_YesNoWinSet( BOX2_SYS_WORK * syswk, u32 pos )
{
/*
	TOUCH_SW_PARAM	prm;

	prm.p_bgl     = syswk->app->bgl;
	prm.bg_frame  = GFL_BG_FRAME0_M;
	prm.char_offs = BOX2MAIN_YNWIN_CGX_POS;
	prm.pltt_offs = BOX2MAIN_BG_PAL_YNWIN;
	prm.x         = 25;
	prm.y         = 12;
	prm.kt_st     = GFL_APP_KTST_KEY;
//↑[GS_CONVERT_TAG]
	prm.key_pos   = pos;
	prm.type      = TOUCH_SW_TYPE_S;

	TOUCH_SW_Init( syswk->app->tsw, &prm );
*/
	APP_TASKMENU_INITWORK	wk;

  wk.heapId   = HEAPID_BOX_APP;
  wk.itemNum  = 2;
  wk.itemWork = syswk->app->ynList;
  wk.posType  = ATPT_RIGHT_DOWN;
  wk.charPosX = 32;
  wk.charPosY = 18;
	wk.w        = APP_TASKMENU_PLATE_WIDTH_YN_WIN;
	wk.h        = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;

	syswk->app->ynWork = APP_TASKMENU_OpenMenu( &wk, syswk->app->ynRes );
}

//--------------------------------------------------------------------------------------------
/**
 * ボタンアニメ処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"TRUE = アニメ中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2MAIN_ButtonAnmMain( BOX2_SYS_WORK * syswk )
{
	BUTTON_ANM_WORK * bawk = &syswk->app->bawk;

	switch( bawk->btn_seq ){
	case 0:
		if( bawk->btn_mode == BOX2MAIN_BTN_ANM_MODE_OBJ ){
			BOX2OBJ_AutoAnmSet( syswk->app, bawk->btn_id, bawk->btn_pal1 );
		}else{
			GFL_BG_ChangeScreenPalette(
				bawk->btn_id, bawk->btn_px, bawk->btn_py, bawk->btn_sx, bawk->btn_sy, bawk->btn_pal1 );
			GFL_BG_LoadScreenV_Req( bawk->btn_id );
		}
		bawk->btn_seq++;
		break;

	case 1:
		if( bawk->btn_mode == BOX2MAIN_BTN_ANM_MODE_OBJ ){
			if( BOX2OBJ_AnmCheck( syswk->app, bawk->btn_id ) == FALSE ){
				return FALSE;
			}
		}else{
			bawk->btn_cnt++;
			if( bawk->btn_cnt != 4 ){ break; }
			GFL_BG_ChangeScreenPalette(
				bawk->btn_id, bawk->btn_px, bawk->btn_py, bawk->btn_sx, bawk->btn_sy, bawk->btn_pal2 );
			GFL_BG_LoadScreenV_Req( bawk->btn_id );
			bawk->btn_cnt = 0;
			bawk->btn_seq++;
		}
		break;

	case 2:
		bawk->btn_cnt++;
		if( bawk->btn_cnt == 2 ){
			return FALSE;
		}
	}

	return TRUE;
}


void BOX2MAIN_ResetTouchBar( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_SetTouchBarButton(
		syswk, syswk->tb_ret_btn, syswk->tb_exit_btn, syswk->tb_status_btn );
	BOX2BGWFRM_PutTouchBar( syswk->app->wfrm );
}



void BOX2MAIN_InitTrayScroll( BOX2_SYS_WORK * syswk, u32 mvID )
{
	if( mvID == BOX2MAIN_TRAY_SCROLL_L ){
		if( syswk->tray == 0 ){
			syswk->tray = syswk->trayMax-1;
		}else{
			syswk->tray--;
		}
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_L_ARROW, BOX2OBJ_ANM_L_ARROW_ON );
	}else{
		if( syswk->tray == (syswk->trayMax-1) ){
			syswk->tray = 0;
		}else{
			syswk->tray++;
		}
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_R_ARROW, BOX2OBJ_ANM_R_ARROW_ON );
	}
	BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
	BOX2MAIN_WallPaperSet( syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), mvID );
}

void BOX2MAIN_InitTrayIconScroll( BOX2_SYS_WORK * syswk, s32 mv )
{
	syswk->box_mv_pos = BOX2MAIN_GetBoxMoveTrayNum( syswk, mv );
	BOX2OBJ_InitTrayCursorScroll( syswk, mv );
	BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
}

void BOX2MAIN_InitBoxMoveFrameScroll( BOX2_SYS_WORK * syswk )
{
	BOX2OBJ_BoxMoveObjInit( syswk );
//	BOX2OBJ_BoxMoveCursorSet( syswk );
	BOX2OBJ_InitBoxMoveCursorPos( syswk->app );
//	BOX2OBJ_BoxMoveNameSet( syswk );
	BOX2BMP_BoxMoveNameWrite( syswk, 0 );
	BOX2BGWFRM_BoxMoveFrmInSet( syswk->app->wfrm );
}


u32 BOX2MAIN_GetTrayScrollDir( BOX2_SYS_WORK * syswk, u32 now, u32 chg )
{
	u32	abs = GFL_STD_Abs( now - chg );

	if( now > chg ){
		if( abs >= syswk->trayMax/2 ){
			return BOX2MAIN_TRAY_SCROLL_R;
		}else{
			return BOX2MAIN_TRAY_SCROLL_L;
		}
	}else{
		if( abs >= syswk->trayMax/2 ){
			return BOX2MAIN_TRAY_SCROLL_L;
		}else{
			return BOX2MAIN_TRAY_SCROLL_R;
		}
	}
	return BOX2MAIN_TRAY_SCROLL_NONE;
}


//============================================================================================
//	ポケモンデータ
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンデータ取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 * @param		tray		トレイ
 * @param		prm			取得パラメータ
 * @param		buf			取得バッファ
 *
 * @return	取得データ
 */
//--------------------------------------------------------------------------------------------
u32 BOX2MAIN_PokeParaGet( BOX2_SYS_WORK * syswk, u16 pos, u16 tray, int prm, void * buf )
{
	// 手持ち
	if( pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		if( pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			pos -= BOX2OBJ_POKEICON_TRAY_MAX;
		}
		if( PokeParty_GetPokeCount( syswk->dat->pokeparty ) > pos ){
			POKEMON_PARAM * pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos );
			return PP_Get( pp, prm, buf );
		}
	// トレイ
	}else if( pos != BOX2MAIN_GETPOS_NONE ){
		POKEMON_PASO_PARAM * ppp = BOXDAT_GetPokeDataAddress( syswk->dat->sv_box, tray, pos );
		if( ppp != NULL ){
			return PPP_Get( ppp, prm, buf );
		}
	}

	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンデータ設定
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 * @param		prm			設定パラメータ
 * @param		buf			設定データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_PokeParaPut( BOX2_SYS_WORK * syswk, u32 pos, u32 tray, int prm, u32 buf )
{
	POKEMON_PASO_PARAM * ppp = BOX2MAIN_PPPGet( syswk, tray, pos );

	if( ppp == NULL ){
		return;
	}
	PPP_Put( ppp, prm, buf );
/*
	if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOXDAT_SetTrayUseBit( syswk->dat->sv_box, tray );
	}
*/
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		POKEMON_PASO_PARAM取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		tray		トレイ番号
 * @param		pos			位置
 *
 * @return	POKEMON_PASO_PARAM
 */
//--------------------------------------------------------------------------------------------
POKEMON_PASO_PARAM * BOX2MAIN_PPPGet( BOX2_SYS_WORK * syswk, u32 tray, u32 pos )
{
	POKEMON_PASO_PARAM * ppp;

	// 手持ち
	if( tray == BOX2MAIN_PPP_GET_MINE || pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		if( pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			pos -= BOX2OBJ_POKEICON_TRAY_MAX;
		}
		if( PokeParty_GetPokeCount( syswk->dat->pokeparty ) > pos ){
			return PP_GetPPPPointer( PokeParty_GetMemberPointer(syswk->dat->pokeparty,pos) );
		}
	// トレイ
	}else if( pos != BOX2MAIN_GETPOS_NONE ){
		return BOXDAT_GetPokeDataAddress( syswk->dat->sv_box, tray, pos );
	}

	return NULL;
}

// ポケモンデータ取得
/*
void BOX2MAIN_GetPokeData( BOX2_SYS_WORK * syswk, u32 tray, u32 pos )
{
	// トレイ
	if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		syswk->getPP = PP_CreateByPPP( BOXDAT_GetPokeDataAddress(syswk->dat->sv_box,tray,pos), HEAPID_BOX_SYS );
		BOXDAT_ClearPokemon( syswk->dat->sv_box, tray, pos );
	// 手持ち
	}else{
		POKEMON_PARAM * pp;
		pos -= BOX2OBJ_POKEICON_TRAY_MAX;
		pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, pos );
		syswk->getPP = PP_CreateByPPP( PP_GetPPPPointerConst(pp), HEAPID_BOX_SYS );
		PokeParty_Delete( syswk->dat->pokeparty, pos );
	}
}
*/

// 取得したポケモンデータを配置
/*
void BOX2MAIN_PutPokeData( BOX2_SYS_WORK * syswk, u32 tray, u32 pos )
{
	POKEMON_PARAM * pp;

	// トレイ
	if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		if( BOX2MAIN_PokeParaGet( syswk, pos, tray, ID_PARA_poke_exist, NULL ) == 0 ){
			BOXDAT_PutPokemonPos( syswk->dat->sv_box, tray, pos, PP_GetPPPPointer(syswk->getPP) );
			GFL_HEAP_FreeMemory( syswk->getPP );
		}else{
			pp = PP_CreateByPPP( BOXDAT_GetPokeDataAddress(syswk->dat->sv_box,tray,pos), HEAPID_BOX_APP );
			BOXDAT_PutPokemonPos( syswk->dat->sv_box, tray, pos, PP_GetPPPPointer(syswk->getPP) );
			GFL_HEAP_FreeMemory( syswk->getPP );
			syswk->getPP = PP_CreateByPPP( PP_GetPPPPointerConst(pp), HEAPID_BOX_SYS );
			GFL_HEAP_FreeMemory( pp );
		}
	// 手持ち
	}else if( pos < BOX2OBJ_POKEICON_PUT_MAX ){
		int	max = PokeParty_GetPokeCount( syswk->dat->pokeparty );
		pos -= BOX2OBJ_POKEICON_TRAY_MAX;
		if( pos < max ){
			pp = PP_CreateByPPP( PP_GetPPPPointerConst(PokeParty_GetMemberPointer(syswk->dat->pokeparty,pos)), HEAPID_BOX_APP );
			PokeParty_SetMemberData( syswk->dat->pokeparty, pos, syswk->getPP );
			GFL_HEAP_FreeMemory( syswk->getPP );
			syswk->getPP = PP_CreateByPPP( PP_GetPPPPointerConst(pp), HEAPID_BOX_SYS );
			GFL_HEAP_FreeMemory( pp );
		}else{
			PokeParty_Add( syswk->dat->pokeparty, syswk->getPP );
			GFL_HEAP_FreeMemory( syswk->getPP );
		}
	// 他トレイ
	}else{
		pos = syswk->box_mv_pos + pos - BOX2OBJ_POKEICON_PUT_MAX;
		if( pos >= syswk->trayMax ){
			pos -= syswk->trayMax;
		}
		BOXDAT_PutPokemonBox( syswk->dat->sv_box, pos, PP_GetPPPPointer(syswk->getPP) );
		GFL_HEAP_FreeMemory( syswk->getPP );
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ポケモンデータクリア
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	tray	トレイ番号
 * @param	pos		位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_PokeDataClear( BOX2_SYS_WORK * syswk, u32 tray, u32 pos )
{
	if( pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		BOXDAT_ClearPokemon( syswk->dat->sv_box, tray, pos );
	}else{
		pos -= BOX2OBJ_POKEICON_TRAY_MAX;
		PokeParty_Delete( syswk->dat->pokeparty, pos );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ボックスと手持ちのデータを入れ替える
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	box		ボックスポケモンの移動データ
 * @param	party	手持ちポケモンの移動データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeDataChangeBoxParty(
				BOX2_SYS_WORK * syswk, BOX2MAIN_POKEMOVE_DATA * box, BOX2MAIN_POKEMOVE_DATA * party )
{
	POKEMON_PARAM * box_pp;
	POKEMON_PARAM * party_pp;
	u16	tray;
	u16	party_pos;

	// ボックスから取得した場合
	if( box->df_pos == BOX2OBJ_POKEICON_GET_POS ){
		tray = syswk->get_tray;
	}else{
		tray = syswk->tray;
	}

	// ボックスデータ退避
	box_pp = PP_CreateByPPP( BOX2MAIN_PPPGet(syswk,tray,box->gt_pos), HEAPID_BOX_APP_L );

	// 手持ちデータをボックスに移動
	party_pos = party->gt_pos - BOX2OBJ_POKEICON_TRAY_MAX;
	party_pp  = PokeParty_GetMemberPointer( syswk->dat->pokeparty, party_pos );
//	BOXDAT_PutPokemonPos( syswk->dat->sv_box, tray, party->mv_pos, PP_GetPPPPointerConst(party_pp) );
	BOXDAT_PutPokemonPos( syswk->dat->sv_box, tray, box->gt_pos, PP_GetPPPPointerConst(party_pp) );

	// ボックスデータを手持ちに移動
	PokeParty_SetMemberData( syswk->dat->pokeparty, party_pos, box_pp );

	GFL_HEAP_FreeMemory( box_pp );

	// トレイアイコン更新
	BOX2OBJ_TrayIconCgxMake( syswk, tray );
//	BOX2OBJ_TrayIconCgxTrans( syswk, tray );
}

//--------------------------------------------------------------------------------------------
/**
 * ボックスから手持ちへ移動
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	box		ボックスポケモンの移動データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeDataMoveBOXtoPARTY( BOX2_SYS_WORK * syswk, BOX2MAIN_POKEMOVE_DATA * box )
{
	POKEMON_PARAM * pp;

	// ボックスデータ準備
	pp = PP_CreateByPPP( BOX2MAIN_PPPGet(syswk,syswk->get_tray,box->gt_pos), HEAPID_BOX_APP_L );

	// 手持ちへ
	PokeParty_Add( syswk->dat->pokeparty, pp );
	// ボックスデータを削除
	BOX2MAIN_PokeDataClear( syswk, syswk->get_tray, box->gt_pos );

	GFL_HEAP_FreeMemory( pp );

	// トレイアイコン更新
	BOX2OBJ_TrayIconCgxMake( syswk, syswk->get_tray );
//	BOX2OBJ_TrayIconCgxTrans( syswk, syswk->get_tray );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちからボックスへ移動
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	party	手持ちポケモンの移動データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeDataMovePARTYtoBOX( BOX2_SYS_WORK * syswk, BOX2MAIN_POKEMOVE_DATA * party )
{
	POKEMON_PARAM * pp;
	u32	party_pos;

	// 手持ちデータ準備
	party_pos = party->gt_pos - BOX2OBJ_POKEICON_TRAY_MAX;
	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, party_pos );

	// ボックスへ
	BOXDAT_PutPokemonPos(
		syswk->dat->sv_box, syswk->tray, party->mv_pos, PP_GetPPPPointerConst(pp) );
	// 手持ちデータ削除
	BOX2MAIN_PokeDataClear( syswk, syswk->tray, party->gt_pos );

	// トレイアイコン更新
	BOX2OBJ_TrayIconCgxMake( syswk, syswk->tray );
//	BOX2OBJ_TrayIconCgxTrans( syswk, syswk->tray );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ち同士の入れ替え
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	party	取得手持ちポケモンの移動データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeDataChangeParty( BOX2_SYS_WORK * syswk, BOX2MAIN_POKEMOVE_DATA * party )
{
/*
	u32	pos1, pos2;
	PM_CONDITION	cond1;
	PM_CONDITION	cond2;

	pos1 = party->df_pos - BOX2OBJ_POKEICON_TRAY_MAX;
	pos2 = party->mv_pos - BOX2OBJ_POKEICON_TRAY_MAX;

	// データ取得
	PokeCopyPPtoPP( PokeParty_GetMemberPointer(syswk->dat->pokeparty,pos1), party->pp );
	PokeParty_GetPokeCondition( syswk->dat->pokeparty, &cond1,pos1 );
	PokeParty_GetPokeCondition( syswk->dat->pokeparty, &cond2,pos2 );
	// データ挿入
	PokeParty_SetMemberData( syswk->dat->pokeparty, pos1, PokeParty_GetMemberPointer(syswk->dat->pokeparty,pos2) );
	PokeParty_SetMemberData( syswk->dat->pokeparty, pos2, party->pp );
	//メンバー交換(SetMemberData)でコンディションが消されてしまうので、コンディションも入れ替える
	PokeParty_SetPokeCondition( syswk->dat->pokeparty, &cond2,pos1 );
	PokeParty_SetPokeCondition( syswk->dat->pokeparty, &cond1,pos2 );
*/
	PokeParty_ExchangePosition(
		syswk->dat->pokeparty,
		party->gt_pos - BOX2OBJ_POKEICON_TRAY_MAX,
		party->mv_pos - BOX2OBJ_POKEICON_TRAY_MAX,
		HEAPID_BOX_APP );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ち移動
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	party	手持ちポケモンの移動データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeDataMoveParty( BOX2_SYS_WORK * syswk, BOX2MAIN_POKEMOVE_DATA * party )
{
/*
	u32	pos;
	PM_CONDITION	cond;

	pos = party->df_pos - BOX2OBJ_POKEICON_TRAY_MAX;

	PokeCopyPPtoPP( PokeParty_GetMemberPointer(syswk->dat->pokeparty,pos), party->pp );
	PokeParty_GetPokeCondition( syswk->dat->pokeparty, &cond, pos );
	BOX2MAIN_PokeDataClear( syswk, syswk->tray, party->df_pos );
	PokeParty_Add( syswk->dat->pokeparty, party->pp );
	//コンディションも再設定 BTS4981のバグ修正
	{
		int pos_tail	= PokeParty_GetPokeCount(syswk->dat->pokeparty)-1;//末尾
		PokeParty_SetPokeCondition( syswk->dat->pokeparty, &cond, pos_tail );
	}
*/
	u32	pos;
	u32	i;

	pos = party->gt_pos - BOX2OBJ_POKEICON_TRAY_MAX;

	for( i=pos; i<PokeParty_GetPokeCount(syswk->dat->pokeparty)-1; i++ ){
		PokeParty_ExchangePosition( syswk->dat->pokeparty, i, i+1, HEAPID_BOX_APP );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ボックスのデータを他のボックスへ
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	box		ボックスポケモンの移動データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeDataBoxMoveBox( BOX2_SYS_WORK * syswk, u32 getTray, u32 getPos, u32 putPos )
{
	POKEMON_PASO_PARAM * ppp;
	u32	tray;

	ppp  = BOX2MAIN_PPPGet( syswk, getTray, getPos );
	tray = MoveBoxGet( syswk, putPos );
	BOXDAT_PutPokemonBox( syswk->dat->sv_box, tray, ppp );
	BOXDAT_ClearPokemon( syswk->dat->sv_box, getTray, getPos );

	// トレイアイコン更新
	BOX2OBJ_TrayIconCgxMake( syswk, getTray );
	BOX2OBJ_TrayIconCgxTrans( syswk, getTray );
	BOX2OBJ_TrayIconCgxMake( syswk, tray );
	BOX2OBJ_TrayIconCgxTrans( syswk, tray );
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちを指定のボックスへ
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	party	手持ちポケモンの移動データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void PokeDataBoxMoveParty( BOX2_SYS_WORK * syswk, BOX2MAIN_POKEMOVE_DATA * party )
{
	POKEMON_PARAM * pp;
	u32	party_pos;
	u32	tray;

	tray = MoveBoxGet( syswk, party->mv_pos );

	party_pos = party->df_pos - BOX2OBJ_POKEICON_TRAY_MAX;
	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, party_pos );

	// シェイミならフォルムチェンジ
	if( PokeParaGet( party->pp, ID_PARA_monsno, NULL ) == MONSNO_SHEIMI ){
		PokeParaSheimiFormChange( party->pp, 0 );
	}

	BOXDAT_PutPokemonBox( syswk->dat->sv_box, tray, PP_GetPPPPointerConst(pp) );
	BOX2MAIN_PokeDataClear( syswk, syswk->tray, party->df_pos );

	// トレイアイコン更新
	BOX2OBJ_TrayIconCgxMake( syswk, tray );
	BOX2OBJ_TrayIconCgxTrans( syswk, tray );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ポケモンデータ移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_PokeDataMove( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;
	u16	ppcnt;
	u8	tray;
//	u32	i;

	work = syswk->app->vfunk.work;
	dat  = work->dat;
	tray = syswk->get_tray;

//	syswk->get_tray = BOX2MAIN_GETPOS_NONE;

	// 配置エリア外 or 同じ位置
	if( work->get_pos == BOX2MAIN_GETPOS_NONE ){
		return;
	}

	ppcnt = PokeParty_GetPokeCount( syswk->dat->pokeparty );

/*
	if( ( work->put_pos & BOX2MAIN_BOXMOVE_FLG ) != 0 ){
		u32 tray = MoveBoxGet( syswk, work->put_pos );
		if( work->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			PokeDataBoxMoveBox( syswk, &dat[0] );
		}else{
			PokeDataBoxMoveParty( syswk, &dat[work->get_pos-BOX2OBJ_POKEICON_TRAY_MAX] );
		}
		return;
	}
*/
	// トレイアイコン上
	if( work->put_pos >= BOX2OBJ_POKEICON_PUT_MAX ){
		PokeDataBoxMoveBox( syswk, syswk->get_tray, work->get_pos, work->put_pos-BOX2OBJ_POKEICON_PUT_MAX );
		return;
	}

	if( work->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		if( work->put_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			// ボックス同士の入れ替え
			BOXDAT_ChangePokeData( syswk->dat->sv_box, syswk->get_tray, work->get_pos, syswk->tray, work->put_pos );
			// トレイアイコン更新
			BOX2OBJ_TrayIconCgxMake( syswk, syswk->tray );
			BOX2OBJ_TrayIconCgxTrans( syswk, syswk->tray );
			if( syswk->tray != syswk->get_tray ){
				BOX2OBJ_TrayIconCgxMake( syswk, syswk->get_tray );
				BOX2OBJ_TrayIconCgxTrans( syswk, syswk->get_tray );
			}
		}else{
			if( ( work->put_pos - BOX2OBJ_POKEICON_TRAY_MAX ) < ppcnt ){
				// ボックス・手持ち間の入れ替え
				PokeDataChangeBoxParty( syswk, &dat[0], &dat[1] );
			}else{
				// ボックスから手持ちへ移動
				PokeDataMoveBOXtoPARTY( syswk, &dat[0] );
			}
		}
	}else{
		if( work->put_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			// 存在チェック
			if( BOX2MAIN_PokeParaGet( syswk, work->put_pos, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				// 手持ち・ボックス間の入れ替え
				PokeDataChangeBoxParty( syswk, &dat[1], &dat[0] );
			}else{
				// 手持ちからボックスへ移動
				PokeDataMovePARTYtoBOX( syswk, &dat[work->get_pos-BOX2OBJ_POKEICON_TRAY_MAX] );
			}
		}else{
			if( ( work->put_pos - BOX2OBJ_POKEICON_TRAY_MAX ) < ppcnt ){
				// 手持ち同士の入れ替え
				PokeDataChangeParty( syswk, &dat[0] );
			}else{
				// 手持ちを移動
				PokeDataMoveParty( syswk, &dat[work->get_pos-BOX2OBJ_POKEICON_TRAY_MAX] );
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 移動するボックスを取得
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		表示ボックス位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static u32 MoveBoxGet( BOX2_SYS_WORK * syswk, u32 pos )
{
//	return ( BOX2MAIN_MV_TRAY_DEF(syswk->box_mv_pos) + (pos&0x7f) );
	pos += syswk->box_mv_pos;
	if( pos >= syswk->trayMax ){
		pos -= syswk->trayMax;
	}
	return pos;
}

//--------------------------------------------------------------------------------------------
/**
 * 指定位置以外に戦えるポケモンがいるか
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		対象外のポケモン位置
 *
 * @retval	"TRUE = いる"
 * @retval	"FALSE = いない"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2MAIN_BattlePokeCheck( BOX2_SYS_WORK * syswk, u32 pos )
{
	POKEMON_PARAM * pp;
	u32	i;

	for( i=0; i<PokeParty_GetPokeCount(syswk->dat->pokeparty); i++ ){
		if( i == pos ){ continue; }
		pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, i );
		if( PP_Get( pp, ID_PARA_tamago_flag, NULL ) == 0 &&
			PP_Get( pp, ID_PARA_hp, NULL ) != 0 ){
			return TRUE;
		}
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 持ち物変更によるフォルムチェンジ
 *
 * @param	ppp		POKEMON_PASO_PARAM
 *
 * @retval	"TRUE = フォルムチェンジ"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2MAIN_PokeItemFormChange( POKEMON_PASO_PARAM * ppp )
{
	u16	bf, af;
	u16	mons;

	mons = PPP_Get( ppp, ID_PARA_monsno, NULL );

	// アルセウス
	if( mons == MONSNO_ARUSEUSU ){
		bf = PPP_Get( ppp, ID_PARA_form_no, NULL );
//		PokePasoParaAusuFormChange( ppp );
		af = PPP_Get( ppp, ID_PARA_form_no, NULL );
		if( bf != af ){ return TRUE; }
	// ギラティナ
	}else if( mons == MONSNO_GIRATHINA ){
		bf = PPP_Get( ppp, ID_PARA_form_no, NULL );
//		PokePasoParaGirathinaFormChange( ppp );
		af = PPP_Get( ppp, ID_PARA_form_no, NULL );
		if( bf != af ){ return TRUE; }
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちからボックスに移動させたときのシェイミのフォルムチェンジ
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	b_pos	移動前の位置
 * @param	a_pos	移動後の位置
 *
 * @return	none
 *
 *	データは入れ替えたあとに呼ぶ
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_FormChangeSheimi( BOX2_SYS_WORK * syswk, u32 b_pos, u32 a_pos )
{
	POKEMON_PASO_PARAM * ppp;
	u32	pos;

	// ボックス同士、手持ち同士の場合
	if( ( b_pos < BOX2OBJ_POKEICON_TRAY_MAX && a_pos < BOX2OBJ_POKEICON_TRAY_MAX ) ||
		( b_pos >= BOX2OBJ_POKEICON_TRAY_MAX && a_pos >= BOX2OBJ_POKEICON_TRAY_MAX ) ){
		return;
	}

	// ボックス配置位置
	if( b_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		pos = b_pos;
	}else{
		pos = a_pos;
	}

	// シェイミ以外
	if( BOX2MAIN_PokeParaGet( syswk, pos, syswk->tray, ID_PARA_monsno, NULL ) != MONSNO_SHEIMI ){
		return;
	}
	// ランドフォルム
	if( BOX2MAIN_PokeParaGet( syswk, pos, syswk->tray, ID_PARA_form_no, NULL ) == 0 ){
		return;
	}

	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, pos );

//	PokePasoParaSheimiFormChange( ppp, 0 );

	// ポケモンアイコン切り替え
	BOX2OBJ_PokeIconChange( syswk, syswk->tray, pos, syswk->app->pokeicon_id[pos] );

	// 上画面表示切替
	if( syswk->get_pos == pos ){
		BOX2MAIN_PokeInfoPut( syswk, pos );
	}
}


//============================================================================================
//	ポケモンアイコン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコン入れ替え動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"TRUE = 動作中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL PokeIconObjMove( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;
	u32	i;
	u32	mvID;
	s16	px, py;

	work = syswk->app->vfunk.work;
	dat  = work->dat;

	if( work->cnt == POKEICON_MOVE_CNT ){
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg == 0 ){ continue; }
//			BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].df_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
			BOX2OBJ_PokeIconPriChg2( syswk->app, dat[i].df_pos, dat[i].mv_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
			mvID = syswk->app->pokeicon_id[dat[i].df_pos];
			if( dat[i].mv_pos < BOX2OBJ_POKEICON_PUT_MAX ){
				BOX2OBJ_PokeIconDefaultPosGet( dat[i].mv_pos, &px, &py, syswk->move_mode );
			}else{
				if( dat[i].df_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					BOX2OBJ_PokeIconDefaultPosGet( BOX2OBJ_POKEICON_MAX-1, &px, &py, syswk->move_mode );
				}else{
					BOX2OBJ_PokeIconDefaultPosGet( dat[i].df_pos, &px, &py, syswk->move_mode );
				}
				BOX2OBJ_Vanish( syswk->app, mvID, FALSE );
			}
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
			if( dat[i].flg == 2 ){
				BOX2OBJ_Vanish( syswk->app, mvID, FALSE );
			}
		}
		return FALSE;
	}else{
		work->cnt++;
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg != 1 ){ continue; }
			mvID = syswk->app->pokeicon_id[dat[i].df_pos];
			px = dat[i].dx + ( ( dat[i].mx * work->cnt ) >> 16 ) * dat[i].vx;
			py = dat[i].dy + ( ( dat[i].my * work->cnt ) >> 16 ) * dat[i].vy;
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
			BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].df_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
		}
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコン移動動作（ドロップ）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"TRUE = 動作中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL PokeIconObjDrop( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;
	u32	i;
	u32	mvID;
	s16	px, py;

	work = syswk->app->vfunk.work;
	dat  = work->dat;

	if( work->cnt == POKEICON_MOVE_CNT ){
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg == 0 ){ continue; }

			mvID = syswk->app->pokeicon_id[dat[i].df_pos];
			BOX2OBJ_PokeIconDefaultPosGet( dat[i].mv_pos, &px, &py, syswk->move_mode );
//			if( syswk->get_pos != dat[i].mv_pos ){
			if( dat[i].mv_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				py = py + BOX2MAIN_PARTYPOKE_FRM_SPD * BOX2MAIN_PARTYPOKE_FRM_CNT;
			}
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
		}
		return FALSE;
/*
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg == 0 ){ continue; }
//			BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].df_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
			BOX2OBJ_PokeIconPriChg2( syswk->app, dat[i].df_pos, dat[i].mv_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
			mvID = syswk->app->pokeicon_id[dat[i].df_pos];
			if( dat[i].mv_pos < BOX2OBJ_POKEICON_PUT_MAX ){
				BOX2OBJ_PokeIconDefaultPosGet( dat[i].mv_pos, &px, &py, syswk->move_mode );
			}else{
				if( dat[i].df_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					BOX2OBJ_PokeIconDefaultPosGet( BOX2OBJ_POKEICON_MAX-1, &px, &py, syswk->move_mode );
				}else{
					BOX2OBJ_PokeIconDefaultPosGet( dat[i].df_pos, &px, &py, syswk->move_mode );
				}
				BOX2OBJ_Vanish( syswk->app, mvID, FALSE );
			}
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
			if( dat[i].flg == 2 ){
				BOX2OBJ_Vanish( syswk->app, mvID, FALSE );
			}
		}
*/
	}else{
		work->cnt++;
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg == 0 ){ continue; }
//			if( syswk->get_pos != dat[i].mv_pos ){ continue; }

			mvID = syswk->app->pokeicon_id[dat[i].df_pos];
			px = dat[i].dx + ( ( dat[i].mx * work->cnt ) >> 16 ) * dat[i].vx;
			py = dat[i].dy + ( ( dat[i].my * work->cnt ) >> 16 ) * dat[i].vy;
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
			BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].df_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
			break;
		}
	}
	return TRUE;

/*
	if( work->cnt == POKEICON_MOVE_CNT ){
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg == 0 ){ continue; }
//			BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].df_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
			BOX2OBJ_PokeIconPriChg2( syswk->app, dat[i].df_pos, dat[i].mv_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
			mvID = syswk->app->pokeicon_id[dat[i].df_pos];
			if( dat[i].mv_pos < BOX2OBJ_POKEICON_PUT_MAX ){
				BOX2OBJ_PokeIconDefaultPosGet( dat[i].mv_pos, &px, &py, syswk->move_mode );
			}else{
				if( dat[i].df_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					BOX2OBJ_PokeIconDefaultPosGet( BOX2OBJ_POKEICON_MAX-1, &px, &py, syswk->move_mode );
				}else{
					BOX2OBJ_PokeIconDefaultPosGet( dat[i].df_pos, &px, &py, syswk->move_mode );
				}
				BOX2OBJ_Vanish( syswk->app, mvID, FALSE );
			}
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
			if( dat[i].flg == 2 ){
				BOX2OBJ_Vanish( syswk->app, mvID, FALSE );
			}
		}
		return FALSE;
	}else{
		work->cnt++;
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg != 1 ){ continue; }
			mvID = syswk->app->pokeicon_id[dat[i].df_pos];
			px = dat[i].dx + ( ( dat[i].mx * work->cnt ) >> 16 ) * dat[i].vx;
			py = dat[i].dy + ( ( dat[i].my * work->cnt ) >> 16 ) * dat[i].vy;
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
			BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].df_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
		}
	}
	return TRUE;
*/

}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコン入れ替え動作（ドロップ）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"TRUE = 動作中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL PokeIconObjDropChange( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;
	u32	i;
	u32	mvID;
	s16	px, py;

	work = syswk->app->vfunk.work;
	dat  = work->dat;

	if( work->cnt == POKEICON_MOVE_CNT ){
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg == 0 ){ continue; }
			mvID = syswk->app->pokeicon_id[dat[i].df_pos];

			BOX2OBJ_PokeIconDefaultPosGet( dat[i].mv_pos, &px, &py, syswk->move_mode );
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
		}
		return FALSE;
	}else{
		work->cnt++;
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg != 1 ){ continue; }
			mvID = syswk->app->pokeicon_id[dat[i].df_pos];
			px = dat[i].dx + ( ( dat[i].mx * work->cnt ) >> 16 ) * dat[i].vx;
			py = dat[i].dy + ( ( dat[i].my * work->cnt ) >> 16 ) * dat[i].vy;
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
			BOX2OBJ_PokeIconPriChgDrop( syswk->app, dat[i].df_pos, dat[i].mv_pos );
		}
	}
	return TRUE;

/*
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;
	u32	i;
	u32	mvID;
	s16	px, py;

	work = syswk->app->vfunk.work;
	dat  = work->dat;

	if( work->cnt == POKEICON_MOVE_CNT ){
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg == 0 ){ continue; }
			BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].df_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
			mvID = syswk->app->pokeicon_id[dat[i].df_pos];
			if( dat[i].mv_pos < BOX2OBJ_POKEICON_PUT_MAX ){
				BOX2OBJ_PokeIconDefaultPosGet( dat[i].mv_pos, &px, &py, syswk->move_mode );
			}else{
				if( dat[i].df_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					BOX2OBJ_PokeIconDefaultPosGet( BOX2OBJ_POKEICON_MAX-1, &px, &py, syswk->move_mode );
				}else{
					BOX2OBJ_PokeIconDefaultPosGet( dat[i].df_pos, &px, &py, syswk->move_mode );
				}
				BOX2OBJ_Vanish( syswk->app, mvID, FALSE );
			}
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
			if( dat[i].flg == 2 ){
				BOX2OBJ_Vanish( syswk->app, mvID, FALSE );
			}
		}
		return FALSE;
	}else{
		work->cnt++;
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg != 1 ){ continue; }
			mvID = syswk->app->pokeicon_id[dat[i].df_pos];
			px = dat[i].dx + ( ( dat[i].mx * work->cnt ) >> 16 ) * dat[i].vx;
			py = dat[i].dy + ( ( dat[i].my * work->cnt ) >> 16 ) * dat[i].vy;
			BOX2OBJ_SetPos( syswk->app, mvID, px, py, CLSYS_DEFREND_MAIN );
			BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].df_pos, BOX2OBJ_POKEICON_PRI_CHG_GET );
		}
	}
	return TRUE;
*/

}

//--------------------------------------------------------------------------------------------
/**
 * 取得ポケモンを指定場所に移動できるかチェック
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	get_pos		取得場所
 * @param	put_pos		配置位置
 *
 * @retval	"TRUE = 可"
 * @retval	"FALSE = 不可"
 */
//--------------------------------------------------------------------------------------------
static BOOL PartyMoveCheck( BOX2_SYS_WORK * syswk, u32 get_pos, u32 put_pos )
{
	u32	exist;
	u32	tray;

	BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_poke_exist, NULL );

/*
	if( ( put_pos & BOX2MAIN_BOXMOVE_FLG ) != 0 ){
		tray = MoveBoxGet( syswk, put_pos );
		if( tray == syswk->tray || BOXDAT_GetPokeExistCount( syswk->dat->sv_box, tray ) == BOX_MAX_POS ){
			return FALSE;
		}
		put_pos ^= BOX2MAIN_BOXMOVE_FLG;
		exist = 0;
	}else{
		tray  = syswk->tray;
		exist = BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_poke_exist, NULL );
	}
*/
	// トレイアイコンへ配置
	if( put_pos >= BOX2OBJ_POKEICON_PUT_MAX ){
		tray = MoveBoxGet( syswk, put_pos-BOX2OBJ_POKEICON_PUT_MAX );
		if( tray == syswk->tray || BOXDAT_GetPokeExistCount( syswk->dat->sv_box, tray ) == BOX_MAX_POS ){
			syswk->app->mv_err_code = BOX2MAIN_ERR_CODE_BOXMAX;
			return FALSE;
		}
		exist = 0;
	}else{
		exist = BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_poke_exist, NULL );
	}

	// バトルボックス専用処理
	if( syswk->dat->callMode == BOX_MODE_BATTLE ){
		// タマゴをトレイから手持ちに配置しようとしたとき
		if( get_pos < BOX2OBJ_POKEICON_TRAY_MAX &&
				put_pos >= BOX2OBJ_POKEICON_TRAY_MAX && put_pos < BOX2OBJ_POKEICON_PUT_MAX &&
				BOX2MAIN_PokeParaGet( syswk, get_pos, syswk->tray, ID_PARA_tamago_flag, NULL ) != 0 ){
			syswk->app->mv_err_code = BOX2MAIN_ERR_CODE_EGG;
			return FALSE;
		}
		// パーティ取得時に配置位置のトレイポケモンがタマゴなら
		if( get_pos >= BOX2OBJ_POKEICON_TRAY_MAX && get_pos < BOX2OBJ_POKEICON_PUT_MAX &&
				put_pos < BOX2OBJ_POKEICON_TRAY_MAX && exist != 0 &&
				BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_tamago_flag, NULL ) != 0 ){
			syswk->app->mv_err_code = BOX2MAIN_ERR_CODE_EGG;
			return FALSE;
		}
	}

	// 手持ちから
	if( get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
		// 戦えるポケモンが取得したポケモンのみ
		if( BOX2MAIN_BattlePokeCheck( syswk, get_pos-BOX2OBJ_POKEICON_TRAY_MAX ) == FALSE ){
			// 配置位置にポケモンがいない
			if( exist == 0 ){
				// 配置位置がトレイか違うトレイ
				if( put_pos < BOX2OBJ_POKEICON_TRAY_MAX || put_pos >= BOX2OBJ_POKEICON_PUT_MAX ){
					if( syswk->dat->callMode != BOX_MODE_BATTLE ){
						syswk->app->mv_err_code = BOX2MAIN_ERR_CODE_BATTLE;
						return FALSE;
					}
				}
			// 配置位置にポケモンがいる
			}else{
				// 配置位置がトレイでポケモンがタマゴ
				if( BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_tamago_flag, NULL ) != 0 &&
					put_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
					if( syswk->dat->callMode != BOX_MODE_BATTLE ){
						syswk->app->mv_err_code = BOX2MAIN_ERR_CODE_BATTLE;
						return FALSE;
					}
				}
			}
		}
		// ボックスへ配置時
		if( put_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			POKEMON_PARAM * pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, get_pos-BOX2OBJ_POKEICON_TRAY_MAX );
			// メールを持っている
			if( ITEM_CheckMail(PP_Get(pp,ID_PARA_item,NULL)) == TRUE ){
				syswk->app->mv_err_code = BOX2MAIN_ERR_CODE_MAIL;
				return FALSE;
			}
		}
	// ボックスから
	}else{
		// 手持ちといれかえ
		if( put_pos >= BOX2OBJ_POKEICON_TRAY_MAX && exist != 0 ){
			POKEMON_PARAM * pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, put_pos-BOX2OBJ_POKEICON_TRAY_MAX );
			// 配置位置のポケモンがメールを持っている
			if( ITEM_CheckMail(PP_Get(pp,ID_PARA_item,NULL)) == TRUE ){
				syswk->app->mv_err_code = BOX2MAIN_ERR_CODE_MAIL;
				return FALSE;
			}
			// 取得ポケモンがタマゴで、戦えるポケモンが配置位置のポケモンのみ
			if( BOX2MAIN_PokeParaGet( syswk, get_pos, syswk->tray, ID_PARA_tamago_flag, NULL ) != 0 &&
				BOX2MAIN_BattlePokeCheck( syswk, put_pos-BOX2OBJ_POKEICON_TRAY_MAX ) == FALSE ){
				if( syswk->dat->callMode != BOX_MODE_BATTLE ){
					syswk->app->mv_err_code = BOX2MAIN_ERR_CODE_BATTLE;
					return FALSE;
				}
			}
		}
	}

	syswk->app->mv_err_code = BOX2MAIN_ERR_CODE_NONE;

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンをトレイの指定場所に移動できるかチェック（入れ替え不可）
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	get_pos		取得場所
 * @param	put_pos		配置位置
 *
 * @retval	"TRUE = 可"
 * @retval	"FALSE = 不可"
 */
//--------------------------------------------------------------------------------------------
static BOOL PokeDropCheck( BOX2_SYS_WORK * syswk, u32 get_pos, u32 put_pos )
{
	POKEMON_PARAM * pp;

	if( BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
//		OS_Printf( "置けない : 配置位置にポケモンがいます\n" );
		return FALSE;
	}

	get_pos -= BOX2OBJ_POKEICON_TRAY_MAX;

	if( BOX2MAIN_BattlePokeCheck( syswk, get_pos ) == FALSE ){
//		OS_Printf( "置けない : 戦えるポケモンがこれだけ\n" );
		return FALSE;
	}

	pp = PokeParty_GetMemberPointer( syswk->dat->pokeparty, get_pos );

	if( ITEM_CheckMail( PP_Get(pp,ID_PARA_item,NULL) ) == TRUE ){
//		OS_Printf( "置けない : メールを持っている\n" );
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * 取得アイテムを指定場所のポケモンにセットできるかチェック
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	get_pos		取得場所
 * @param	put_pos		配置位置
 *
 * @retval	"TRUE = 可"
 * @retval	"FALSE = 不可"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2MAIN_PokeItemMoveCheck( BOX2_SYS_WORK * syswk, u32 get_pos, u32 put_pos )
{
	u16	get_item, put_item;

	if( BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_poke_exist, NULL ) == 0 ){
//		OS_Printf( "置けない : 配置位置にポケモンがいません\n" );
		return FALSE;
	}
	if( BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_tamago_flag, NULL ) != 0 ){
//		OS_Printf( "置けない : タマゴはアイテムを持てない\n" );
		return FALSE;
	}

	get_item = BOX2MAIN_PokeParaGet( syswk,get_pos, syswk->tray, ID_PARA_item, NULL );

	if( ITEM_CheckMail(get_item) == TRUE ){
//		OS_Printf( "置けない : メールは移動できない\n" );
		return FALSE;
	}

	put_item = BOX2MAIN_PokeParaGet( syswk,put_pos, syswk->tray, ID_PARA_item, NULL );

	if( ITEM_CheckMail(put_item) == TRUE ){
//		OS_Printf( "置けない : メールを持っている\n" );
		return FALSE;
	}

	if( get_item == ITEM_HAKKINDAMA && 
		BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_monsno, NULL ) != MONSNO_GIRATHINA ){
//		OS_Printf( "置けない : はっきんだまはギラティナしか持てない\n" );
		return FALSE;
	}

	if( put_item == ITEM_HAKKINDAMA && 
		BOX2MAIN_PokeParaGet( syswk, get_pos, syswk->tray, ID_PARA_monsno, NULL ) != MONSNO_GIRATHINA ){
//		OS_Printf( "置けない : はっきんだまはギラティナしか持てない\n" );
		return FALSE;
	}

	return TRUE;
}


//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコン入れ替えパラメータ作成
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	dat		移動データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconMoveParamMake( BOX2_SYS_WORK * syswk, BOX2MAIN_POKEMOVE_DATA * dat )
{
	u32	mvID;
	s16	px, py;

	mvID = syswk->app->pokeicon_id[dat->df_pos];

	BOX2OBJ_GetPos( syswk->app, mvID, &dat->dx, &dat->dy, CLSYS_DEFREND_MAIN );
//	if( ( dat->mv_pos & BOX2MAIN_BOXMOVE_FLG ) == 0 ){
	if( dat->mv_pos < BOX2OBJ_POKEICON_PUT_MAX ){
		BOX2OBJ_PokeIconDefaultPosGet( dat->mv_pos, &px, &py, syswk->move_mode );
	}else{
		BOX2OBJ_TrayIconPosGet( syswk->app, dat->mv_pos-BOX2OBJ_POKEICON_PUT_MAX, &px, &py );
	}

	if( dat->dx <= px ){
		dat->vx = 1;
		dat->mx = ( ( px - dat->dx ) << 16 ) / POKEICON_MOVE_CNT;
	}else{
		dat->vx = -1;
		dat->mx = ( ( dat->dx - px ) << 16 ) / POKEICON_MOVE_CNT;
	}
	if( dat->dy <= py ){
		dat->vy = 1;
		dat->my = ( ( py - dat->dy ) << 16 ) / POKEICON_MOVE_CNT;
	}else{
		dat->vy = -1;
		dat->my = ( ( dat->dy - py ) << 16 ) / POKEICON_MOVE_CNT;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコン入れ替えパラメータ作成（ドロップ）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	dat		移動データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconDropParamMake( BOX2_SYS_WORK * syswk, BOX2MAIN_POKEMOVE_DATA * dat )
{
	u32	dfID, mvID;
	s16	px, py;

	dfID = syswk->app->pokeicon_id[dat->df_pos];
	mvID = syswk->app->pokeicon_id[dat->mv_pos];

	BOX2OBJ_GetPos( syswk->app, dfID, &dat->dx, &dat->dy, CLSYS_DEFREND_MAIN );
	BOX2OBJ_GetPos( syswk->app, mvID, &px, &py, CLSYS_DEFREND_MAIN );

	if( dat->dx <= px ){
		dat->vx = 1;
		dat->mx = ( ( px - dat->dx ) << 16 ) / POKEICON_MOVE_CNT;
	}else{
		dat->vx = -1;
		dat->mx = ( ( dat->dx - px ) << 16 ) / POKEICON_MOVE_CNT;
	}
	if( dat->dy <= py ){
		dat->vy = 1;
		dat->my = ( ( py - dat->dy ) << 16 ) / POKEICON_MOVE_CNT;
	}else{
		dat->vy = -1;
		dat->my = ( ( dat->dy - py ) << 16 ) / POKEICON_MOVE_CNT;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 入れ替えデータ作成
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	get_pos		取得位置
 * @param	put_pos		配置位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconChgDataMeke( BOX2_SYS_WORK * syswk, u32 get_pos, u32 put_pos )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;

	work = syswk->app->vfunk.work;
	dat  = work->dat;

	dat[0].df_pos = BOX2OBJ_POKEICON_GET_POS;
//	dat[0].df_pos = get_pos;
	dat[0].mv_pos = put_pos;
	dat[0].gt_pos = get_pos;
	dat[0].flg    = 1;
	PokeIconMoveParamMake( syswk, &dat[0] );
	dat[1].df_pos = put_pos;
	dat[1].mv_pos = get_pos;
	dat[1].gt_pos = put_pos;
	dat[1].flg    = 1;
	PokeIconMoveParamMake( syswk, &dat[1] );
}

//--------------------------------------------------------------------------------------------
/**
 * アイコン移動データ作成（通常の入れ替え）
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	get_pos		取得位置
 * @param	put_pos		配置位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconMoveBoxPartyDataMake( BOX2_SYS_WORK * syswk, u32 get_pos, u32 put_pos )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;
	u32	get;
	u32	ppcnt;
	u32	i;

	work = syswk->app->vfunk.work;
	dat  = work->dat;

//	syswk->app->poke_mv_cur_flg = TRUE;

	// 初期化
	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
		dat[i].df_pos = 0;
		dat[i].mv_pos = 0;
		dat[i].gt_pos = 0;
		dat[i].flg    = 0;
	}

	work->set_pos = put_pos;

	// 配置エリア外
	if( put_pos == BOX2MAIN_GETPOS_NONE ){
		work->get_pos = BOX2MAIN_GETPOS_NONE;
		work->put_pos = BOX2MAIN_GETPOS_NONE;
//		dat[0].df_pos = get_pos;
		dat[0].df_pos = BOX2OBJ_POKEICON_GET_POS;
		dat[0].mv_pos = get_pos;
		dat[0].gt_pos = get_pos;
		dat[0].flg    = 1;
		if( syswk->tray == syswk->get_tray ){
			dat[1].df_pos = get_pos;
			dat[1].mv_pos = get_pos;
			dat[1].gt_pos = get_pos;
			dat[1].flg    = 1;
		}
		PokeIconMoveParamMake( syswk, &dat[0] );
		return;
	}

	// 同じ位置 or 手持ち１の時のボックスへの移動
	if( ( get_pos == put_pos && ( get_pos >= BOX2OBJ_POKEICON_TRAY_MAX || syswk->tray == syswk->get_tray ) ) ||	// 同じ位置
			PartyMoveCheck(syswk,get_pos,put_pos) == FALSE ){						// 手持ち１の時のボックスへの移動

		work->get_pos = BOX2MAIN_GETPOS_NONE;
		work->put_pos = BOX2MAIN_GETPOS_NONE;
//		dat[0].df_pos = get_pos;
		dat[0].df_pos = BOX2OBJ_POKEICON_GET_POS;
		dat[0].mv_pos = get_pos;
		dat[0].gt_pos = get_pos;
		dat[0].flg    = 1;
		dat[1].df_pos = get_pos;
		dat[1].mv_pos = get_pos;
		dat[1].gt_pos = get_pos;
		dat[1].flg    = 1;
		PokeIconMoveParamMake( syswk, &dat[0] );
		return;
	}

	work->get_pos = get_pos;
	work->put_pos = put_pos;

	ppcnt = PokeParty_GetPokeCount( syswk->dat->pokeparty );

/*
	if( ( put_pos & BOX2MAIN_BOXMOVE_FLG ) != 0 ){
		if( get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			dat[0].df_pos = get_pos;
			dat[0].mv_pos = put_pos;
			dat[0].flg    = 1;
			PokeIconMoveParamMake( syswk, &dat[0] );
			syswk->app->poke_mv_cur_flg = FALSE;
		}else{
			get = get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
			for( i=get+1; i<BOX2OBJ_POKEICON_MINE_MAX; i++ ){
				dat[i].df_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
				dat[i].mv_pos = i - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
				dat[i].flg    = 1;
				PokeIconMoveParamMake( syswk, &dat[i] );
			}
			dat[get].df_pos = get_pos;
			dat[get].mv_pos = put_pos;
			dat[get].flg    = 1;
			PokeIconMoveParamMake( syswk, &dat[get] );
			syswk->app->poke_mv_cur_flg = FALSE;
		}
		return;
	}
*/
	// トレイアイコン上
	if( put_pos >= BOX2OBJ_POKEICON_PUT_MAX ){
		dat[0].df_pos = BOX2OBJ_POKEICON_GET_POS;
//		dat[0].df_pos = get_pos;
		dat[0].mv_pos = put_pos;
		dat[0].gt_pos = BOX2OBJ_POKEICON_GET_POS;
		dat[0].flg    = 1;
/*
		dat[1].df_pos = get_pos;
		dat[1].mv_pos = get_pos;
		dat[1].gt_pos = get_pos;
		dat[1].flg    = 1;
*/
/*
		dat[1].df_pos = get_pos;
		dat[1].mv_pos = BOX2OBJ_POKEICON_GET_POS;
		dat[1].flg    = 1;
*/
		PokeIconMoveParamMake( syswk, &dat[0] );
		return;
	}

	// トレイがスクロールされているとき
	if( syswk->tray != syswk->get_tray && get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		if( put_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
			if( put_pos > ppcnt+BOX2OBJ_POKEICON_TRAY_MAX ){
				put_pos = ppcnt+BOX2OBJ_POKEICON_TRAY_MAX;
			}
		}
		dat[0].df_pos = BOX2OBJ_POKEICON_GET_POS;
		dat[0].mv_pos = put_pos;
//		dat[0].gt_pos = BOX2OBJ_POKEICON_GET_POS;
//		dat[0].gt_pos = put_pos;
		dat[0].gt_pos = get_pos;
		dat[0].flg    = 1;
		dat[1].df_pos = put_pos;
		dat[1].mv_pos = put_pos;
		dat[1].gt_pos = put_pos;
		dat[1].flg    = 2;
		PokeIconMoveParamMake( syswk, &dat[0] );
//		PokeIconMoveParamMake( syswk, &dat[1] );
		return;
	}

	if( get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		if( put_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			// ●ボックス同士の入れ替え
			PokeIconChgDataMeke( syswk, get_pos, put_pos );
		}else{
			if( ( put_pos - BOX2OBJ_POKEICON_TRAY_MAX ) < ppcnt ){
				// ●ボックス・手持ち間の入れ替え
				PokeIconChgDataMeke( syswk, get_pos, put_pos );	// 0=box, 1=party
			}else{
				// ●ボックスから手持ちへ移動
				PokeIconChgDataMeke( syswk, get_pos, ppcnt+BOX2OBJ_POKEICON_TRAY_MAX );
			}
		}
	}else{
		if( put_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			// 存在チェック
			if( BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				// ●手持ち・ボックス間の入れ替え
				PokeIconChgDataMeke( syswk, get_pos, put_pos );	// 0=party, 1=box
			}else{
				// ●手持ちからボックスへ移動
				get = get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				for( i=get+1; i<ppcnt; i++ ){
					dat[i].df_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].mv_pos = i - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].gt_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].flg    = 1;
					PokeIconMoveParamMake( syswk, &dat[i] );
				}
//				dat[get].df_pos = get_pos;
				dat[get].df_pos = BOX2OBJ_POKEICON_GET_POS;
				dat[get].mv_pos = put_pos;
				dat[get].gt_pos = get_pos;
				dat[get].flg    = 1;
				PokeIconMoveParamMake( syswk, &dat[get] );
				dat[ppcnt].df_pos = put_pos;
				dat[ppcnt].mv_pos = ppcnt - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
				dat[ppcnt].gt_pos = put_pos;
				dat[ppcnt].flg    = 1;
				PokeIconMoveParamMake( syswk, &dat[ppcnt] );
			}
		}else{
			if( ( put_pos - BOX2OBJ_POKEICON_TRAY_MAX ) < ppcnt ){
				// ●手持ち同士の入れ替え
				PokeIconChgDataMeke( syswk, get_pos, put_pos );
			}else{
				// ●手持ちを移動
				get = get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				for( i=get+1; i<ppcnt; i++ ){
					dat[i].df_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].mv_pos = i - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].gt_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].flg    = 1;
					PokeIconMoveParamMake( syswk, &dat[i] );
				}
//				dat[get].df_pos = get_pos;
				dat[get].df_pos = BOX2OBJ_POKEICON_GET_POS;
				dat[get].mv_pos = ppcnt - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
				dat[get].gt_pos = get_pos;
				dat[get].flg    = 1;
				PokeIconMoveParamMake( syswk, &dat[get] );
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * アイコン移動データ作成（トレイにドロップ）
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	get_pos		取得位置
 * @param	put_pos		配置位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconDropDataMake( BOX2_SYS_WORK * syswk, u32 get_pos, u32 put_pos )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;
	u32	get;
	u32	ppcnt;
	u32	i;

	work = syswk->app->vfunk.work;
	dat  = work->dat;

//	syswk->app->poke_mv_cur_flg = TRUE;

	// 初期化
	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
		dat[i].df_pos = 0;
		dat[i].mv_pos = 0;
		dat[i].gt_pos = 0;
		dat[i].flg    = 0;
	}

	// 配置エリア外 or 手持ち１ or 配置位置にポケモンがいる
	if( put_pos == BOX2MAIN_GETPOS_NONE || PokeDropCheck(syswk,get_pos,put_pos) == FALSE ){
//		OS_Printf( "置けない put_pos = 0x%2x\n", put_pos );
		work->get_pos = BOX2MAIN_GETPOS_NONE;
		work->put_pos = BOX2MAIN_GETPOS_NONE;
//		dat[0].df_pos = get_pos;
		dat[0].df_pos = BOX2OBJ_POKEICON_GET_POS;
		dat[0].mv_pos = get_pos;
		dat[0].gt_pos = get_pos;
		dat[0].flg    = 1;
		dat[1].df_pos = get_pos;
		dat[1].mv_pos = get_pos;
		dat[1].gt_pos = get_pos;
		dat[1].flg    = 1;
		PokeIconMoveParamMake( syswk, &dat[0] );
		return;
	}

	work->get_pos = get_pos;
	work->put_pos = put_pos;

	ppcnt = PokeParty_GetPokeCount( syswk->dat->pokeparty );

	if( put_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		// 手持ちからボックスへ移動
		get = get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
		for( i=get+1; i<ppcnt; i++ ){
			dat[i].df_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
			dat[i].mv_pos = i - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
			dat[i].gt_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
			dat[i].flg    = 1;
			PokeIconDropParamMake( syswk, &dat[i] );
		}
//		dat[get].df_pos = get_pos;
		dat[get].df_pos = BOX2OBJ_POKEICON_GET_POS;
		dat[get].mv_pos = put_pos;
		dat[get].gt_pos = get_pos;
		dat[get].flg    = 1;
		PokeIconDropParamMake( syswk, &dat[get] );
		dat[ppcnt].df_pos = put_pos;
		dat[ppcnt].mv_pos = ppcnt - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
		dat[ppcnt].gt_pos = put_pos;
		dat[ppcnt].flg    = 1;
		PokeIconDropParamMake( syswk, &dat[ppcnt] );

//		syswk->app->poke_mv_cur_flg = FALSE;
	}


#if 0

	work->get_pos = get_pos;
	work->put_pos = put_pos;

	ppcnt = PokeParty_GetPokeCount( syswk->dat->pokeparty );


	if( get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		if( put_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			// ●ボックス同士の入れ替え
			PokeIconChgDataMeke( syswk, get_pos, put_pos );
		}else{
			if( ( put_pos - BOX2OBJ_POKEICON_TRAY_MAX ) < ppcnt ){
				// ●ボックス・手持ち間の入れ替え
				PokeIconChgDataMeke( syswk, get_pos, put_pos );	// 0=box, 1=party
			}else{
				// ●ボックスから手持ちへ移動
				PokeIconChgDataMeke( syswk, get_pos, ppcnt+BOX2OBJ_POKEICON_TRAY_MAX );
			}
		}
	}else{
		if( put_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			// 存在チェック
			if( BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
				// ●手持ち・ボックス間の入れ替え
				PokeIconChgDataMeke( syswk, get_pos, put_pos );	// 0=party, 1=box
			}else{
				// ●手持ちからボックスへ移動
				get = get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				for( i=get+1; i<ppcnt; i++ ){
					dat[i].df_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].mv_pos = i - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].gt_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].flg    = 1;
					PokeIconMoveParamMake( syswk, &dat[i] );
				}
//				dat[get].df_pos = get_pos;
				dat[get].df_pos = BOX2OBJ_POKEICON_GET_POS;
				dat[get].mv_pos = put_pos;
				dat[get].gt_pos = get_pos;
				dat[get].flg    = 1;
				PokeIconMoveParamMake( syswk, &dat[get] );
				dat[ppcnt].df_pos = put_pos;
				dat[ppcnt].mv_pos = ppcnt - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
				dat[ppcnt].gt_pos = put_pos;
				dat[ppcnt].flg    = 1;
				PokeIconMoveParamMake( syswk, &dat[ppcnt] );
			}
		}else{
			if( ( put_pos - BOX2OBJ_POKEICON_TRAY_MAX ) < ppcnt ){
				// ●手持ち同士の入れ替え
				PokeIconChgDataMeke( syswk, get_pos, put_pos );
			}else{
				// ●手持ちを移動
				get = get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
				for( i=get+1; i<ppcnt; i++ ){
					dat[i].df_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].mv_pos = i - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].gt_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
					dat[i].flg    = 1;
					PokeIconMoveParamMake( syswk, &dat[i] );
				}
//				dat[get].df_pos = get_pos;
				dat[get].df_pos = BOX2OBJ_POKEICON_GET_POS;
				dat[get].mv_pos = ppcnt - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
				dat[get].gt_pos = get_pos;
				dat[get].flg    = 1;
				PokeIconMoveParamMake( syswk, &dat[get] );
			}
		}
	}
#endif








}

//--------------------------------------------------------------------------------------------
/**
 * アイコン移動データ作成（手持ちにドロップ）
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	get_pos		取得位置
 * @param	put_pos		配置位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconPartyDropDataMake( BOX2_SYS_WORK * syswk, u32 get_pos, u32 put_pos )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;
	u32	get;
	u32	ppcnt;
	u32	i;

	work = syswk->app->vfunk.work;
	dat  = work->dat;

//	syswk->app->poke_mv_cur_flg = TRUE;

	// 初期化
	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
		dat[i].df_pos = 0;
		dat[i].mv_pos = 0;
		dat[i].gt_pos = 0;
		dat[i].flg    = 0;
	}

	// 配置エリア外 or 配置位置にポケモンがいる
	if( put_pos == BOX2MAIN_GETPOS_NONE ||
		BOX2MAIN_PokeParaGet( syswk, put_pos, syswk->tray, ID_PARA_poke_exist, NULL ) != 0 ){
/*
		work->get_pos = BOX2MAIN_GETPOS_NONE;
		work->put_pos = BOX2MAIN_GETPOS_NONE;
		dat[0].df_pos = get_pos;
		dat[0].mv_pos = get_pos;
		dat[0].flg    = 1;
		PokeIconMoveParamMake( syswk, &dat[0] );
		return;
*/
		work->get_pos = BOX2MAIN_GETPOS_NONE;
		work->put_pos = BOX2MAIN_GETPOS_NONE;
//		dat[0].df_pos = get_pos;
		dat[0].df_pos = BOX2OBJ_POKEICON_GET_POS;
		dat[0].mv_pos = get_pos;
		dat[0].gt_pos = get_pos;
		dat[0].flg    = 1;
		dat[1].df_pos = get_pos;
		dat[1].mv_pos = get_pos;
		dat[1].gt_pos = get_pos;
		dat[1].flg    = 1;
		PokeIconMoveParamMake( syswk, &dat[0] );
		return;
	}

	work->get_pos = get_pos;
	work->put_pos = put_pos;

	ppcnt = PokeParty_GetPokeCount( syswk->dat->pokeparty );

	PokeIconChgDataMeke( syswk, get_pos, ppcnt+BOX2OBJ_POKEICON_TRAY_MAX );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンの移動後の位置を取得
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	get_pos		取得位置
 *
 * @retval	"BOX2MAIN_GETPOS_NONE = 移動なし"
 * @retval	"BOX2MAIN_GETPOS_NONE != 移動後の位置"
 */
//--------------------------------------------------------------------------------------------
static u32 PokeIconMoveAfterPosGet( BOX2_SYS_WORK * syswk, u32 get_pos )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;
	u32	i;

	work = syswk->app->vfunk.work;
	dat  = work->dat;

	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
//		if( dat[i].flg == 1 && dat[i].df_pos == get_pos ){
		if( dat[i].flg != 0 && dat[i].df_pos == get_pos ){
			return dat[i].mv_pos;
		}
	}
	return BOX2MAIN_GETPOS_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * アイコン移動データ作成（逃がす）
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	get_pos		取得位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconFreeDataMake( BOX2_SYS_WORK * syswk, u32 get_pos )
{
	BOX2MAIN_POKEMOVE_WORK * work;
	BOX2MAIN_POKEMOVE_DATA * dat;
	u32	get;
	u32	i;

	work = syswk->app->vfunk.work;
	dat  = work->dat;
	get  = get_pos - BOX2OBJ_POKEICON_TRAY_MAX;

	// 初期化
	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX; i++ ){
		dat[i].df_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
		dat[i].mv_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
		dat[i].gt_pos = i + BOX2OBJ_POKEICON_TRAY_MAX;
		dat[i].flg    = 0;
	}
	dat[i].flg = 0;

	for( i=get+1; i<BOX2OBJ_POKEICON_MINE_MAX; i++ ){
		dat[i].mv_pos = i - 1 + BOX2OBJ_POKEICON_TRAY_MAX;
		dat[i].flg    = 1;
		PokeIconMoveParamMake( syswk, &dat[i] );
	}
	dat[get].mv_pos = BOX2OBJ_POKEICON_MAX - 1;
	dat[get].flg    = 1;
	PokeIconMoveParamMake( syswk, &dat[get] );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコン入れ替え
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	dat			移動データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeIconBufPosChange( BOX2_SYS_WORK * syswk, BOX2MAIN_POKEMOVE_DATA * dat )
{
	u16	i;
	u16	gmp;
	u8	id[BOX2OBJ_POKEICON_MINE_MAX+1];

	gmp = 0xffff;

	// 元の位置にあるＩＤを記憶
	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
		if( dat[i].flg != 0 ){
//			id[i] = syswk->app->pokeicon_id[ dat[i].df_pos ];
			id[i] = syswk->app->pokeicon_id[ dat[i].gt_pos ];
/*
			// 取得アイコンの移動位置を保存
			if( dat[i].df_pos == BOX2OBJ_POKEICON_GET_POS ){
				gmp = dat[i].mv_pos;
			}
*/
		}
	}

/*
	// 取得位置にあるアイコンを取得
	if( gmp != 0xffff ){
		for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
			if( dat[i].flg != 0 ){
				if( gmp == dat[i].df_pos ){
					gmp = syswk->app->pokeicon_id[ dat[i].mv_pos ];
					break;
				}
			}
		}
	}
	if( i == BOX2OBJ_POKEICON_MINE_MAX+1 ){
		gmp = 0xffff;
	}
*/

	// いれかえる
	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX+1; i++ ){
		if( dat[i].flg != 0 ){
/*
			if( ( dat[i].mv_pos & BOX2MAIN_BOXMOVE_FLG ) == 0 ){
				syswk->app->pokeicon_id[ dat[i].mv_pos ] = id[i];
				BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].mv_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
			}else{
				if( dat[i].df_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
					syswk->app->pokeicon_id[ BOX2OBJ_POKEICON_MAX-1 ] = id[i];
				}
				BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].df_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
			}
*/
			syswk->app->pokeicon_id[ dat[i].mv_pos ] = id[i];
//			BOX2OBJ_PokeIconPriChg( syswk->app, dat[i].mv_pos, BOX2OBJ_POKEICON_PRI_CHG_PUT );
			if( dat[i].df_pos == BOX2OBJ_POKEICON_GET_POS ){
				gmp = dat[i].mv_pos;
//				OS_Printf( "gmp = %d\n", gmp );
			}
		}
	}

	if( gmp != 0xffff ){
		BOX2OBJ_PutPokeIcon( syswk->app, gmp );
	}

	// 全体のプライオリティを再設定する
	for( i=0; i<BOX2OBJ_POKEICON_PUT_MAX; i++ ){
		BOX2OBJ_PokeIconPriChg( syswk->app, i, BOX2OBJ_POKEICON_PRI_CHG_PUT );
	}
}


//============================================================================================
//	ポケモンを逃がす
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 技チェック
 *
 * @param	waza	技
 *
 * @retval	"重要な技以外 = 0xffffffff"
 * @retval	"重要な技 = テーブル番号"
 */
//--------------------------------------------------------------------------------------------
static u32 PokeFreeWazaCheck( u16 waza )
{
/*
	u32	i;

	for( i=0; i<NELEMS(PokeFreeCheckWazaTable); i++ ){
		if( waza == PokeFreeCheckWazaTable[i] ){
			return i;
		}
	}
*/
	return 0xffffffff;
}

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンを逃がす」ワーク作成
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_PokeFreeCreate( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEFREE_WORK * wk;
	POKEMON_PASO_PARAM * ppp;
	u32	ret;
	u32	i;

	syswk->app->seqwk = GFL_HEAP_AllocMemoryLo( HEAPID_BOX_APP, sizeof(BOX2MAIN_POKEFREE_WORK) );
	wk = syswk->app->seqwk;

	wk->clwk      = syswk->app->clwk[syswk->app->pokeicon_id[syswk->get_pos]];
	wk->check_cnt = 0;
	wk->check_flg = 0;

	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, syswk->get_pos );

	for( i=0; i<4; i++ ){
		ret = PokeFreeWazaCheck( PPP_Get(ppp,ID_PARA_waza1+i,NULL) );
		if( ret != 0xffffffff ){
			wk->check_flg |= (1<<ret);
			break;
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンを逃がす」ワーク解放
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_PokeFreeExit( BOX2_SYS_WORK * syswk )
{
	GFL_HEAP_FreeMemory( syswk->app->seqwk );
}

//--------------------------------------------------------------------------------------------
/**
 * 逃がせるかどうか技チェック
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	"TRUE = チェック中"
 * @return	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2MAIN_PokeFreeWazaCheck( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_POKEFREE_WORK * wk;
	POKEMON_PASO_PARAM * ppp;
	u32	tray, pos;
	u32	i, j;
	u32	ret;

	wk = syswk->app->seqwk;

	if( wk->check_cnt == POKE_FREE_CHECK_MAX ){
		return FALSE;
	}

	for( i=0; i<POKE_FREE_CHECK_ONECE; i++ ){
		if( wk->check_cnt < POKE_FREE_CHECK_BOX ){
			tray = wk->check_cnt / BOX_MAX_POS;
			pos  = wk->check_cnt % BOX_MAX_POS;
			// 逃がすポケモンと同じときはチェックしない（存在しないことにする）
			if( tray == syswk->tray && pos == syswk->get_pos ){
				ppp = NULL;
			}else{
				ppp = BOX2MAIN_PPPGet( syswk, tray, pos );
			}
		}else{
			pos = wk->check_cnt - POKE_FREE_CHECK_BOX;
			// 逃がすポケモンと同じときはチェックしない（存在しないことにする）
			if( pos == syswk->get_pos - BOX_MAX_POS ){
				ppp = NULL;
			}else{
				ppp = BOX2MAIN_PPPGet( syswk, BOX2MAIN_PPP_GET_MINE, pos );
			}
		}
		if( ppp != NULL ){
			if( PPP_Get( ppp, ID_PARA_poke_exist, NULL ) != 0 ){
				for( j=0; j<4; j++ ){
					ret = PokeFreeWazaCheck( PPP_Get(ppp,ID_PARA_waza1+j,NULL) );
					if( ret != 0xffffffff ){
						wk->check_flg = ( wk->check_flg & (0xff^(1<<ret)) );
					}
				}
			}
		}
		wk->check_cnt++;
		if( wk->check_cnt == POKE_FREE_CHECK_MAX ){
			return FALSE;
		}
	}
	return TRUE;
}


//============================================================================================
//	アイテムアイコン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコン動作データ作成
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	set_pos		配置位置（プレイヤーが指定した位置）
 * @param	put_pos		配置位置（置けなかった場合の補正あり）
 * @param	mode		移動モード
 * @param	flg			TRUE = タッチ, FALSE = キー
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ItemIconMoveMakeCore( BOX2_SYS_WORK * syswk, u32 set_pos, u32 put_pos, u32 mode, BOOL flg )
{
	BOX2MAIN_ITEMMOVE_WORK * mvwk;
	s16	npx, npy;
	s16	mpx, mpy;

	mvwk = syswk->app->vfunk.work;

	BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_ITEMICON, &npx, &npy, CLSYS_DEFREND_MAIN );

	BOX2OBJ_PokeIconDefaultPosGet( put_pos, &mpx, &mpy, mode );
	if( flg == TRUE ){
		mpx += 8;
		mpy += 8;
	}else{
		mpy += 4;
	}

	mvwk->put_pos = put_pos;
	mvwk->set_pos = set_pos;
	mvwk->cnt = 0;
	mvwk->mv_mode = mode;

	if( npx > mpx ){
		mvwk->mv_x = 1;
		mvwk->mx = ( ( npx - mpx ) << 8 ) / ITEMICON_MOVE_CNT;
	}else{
		mvwk->mv_x = 0;
		mvwk->mx = ( ( mpx - npx ) << 8 ) / ITEMICON_MOVE_CNT;
	}
	if( npy > mpy ){
		mvwk->mv_y = 1;
		mvwk->my = ( ( npy - mpy ) << 8 ) / ITEMICON_MOVE_CNT;
	}else{
		mvwk->mv_y = 0;
		mvwk->my = ( ( mpy - npy ) << 8 ) / ITEMICON_MOVE_CNT;
	}

	mvwk->now_x = npx << 8;
	mvwk->now_y = npy << 8;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコン動作データ作成（タッチ）
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	set_pos		配置位置（プレイヤーが指定した位置）
 * @param	put_pos		配置位置（置けなかった場合の補正あり）
 * @param	mode		移動モード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ItemIconMoveMake( BOX2_SYS_WORK * syswk, u32 set_pos, u32 put_pos, u32 mode )
{
	ItemIconMoveMakeCore( syswk, set_pos, put_pos, mode, TRUE );
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコン動作データ作成（キー）
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	put_pos		配置位置（置けなかった場合の補正あり）
 * @param	mode		移動モード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ItemIconMoveMakeHand( BOX2_SYS_WORK * syswk, u32 put_pos, u32 mode )
{
	ItemIconMoveMakeCore( syswk, syswk->app->poke_put_key, put_pos, mode, FALSE );
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコン移動
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	flg			TRUE = タッチ, FALSE = キー
 *
 * @retval	"TRUE = 移動中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemIconMoveMainCore( BOX2_SYS_WORK * syswk, BOOL flg )
{
	BOX2MAIN_ITEMMOVE_WORK * mvwk;
	s16	px, py;

	mvwk = syswk->app->vfunk.work;

	if( mvwk->cnt == ITEMICON_MOVE_CNT ){
		if( flg == TRUE ){
			BOX2OBJ_ItemIconPokePut( syswk->app, mvwk->put_pos, mvwk->mv_mode );
		}else{
			BOX2OBJ_ItemIconPokePutHand( syswk->app, mvwk->put_pos, mvwk->mv_mode );
		}
		BOX2OBJ_ItemIconCursorMove( syswk->app );
		return FALSE;
	}

	if( mvwk->mv_x == 0 ){
		mvwk->now_x += mvwk->mx;
	}else{
		mvwk->now_x -= mvwk->mx;
	}
	if( mvwk->mv_y == 0 ){
		mvwk->now_y += mvwk->my;
	}else{
		mvwk->now_y -= mvwk->my;
	}

	BOX2OBJ_ItemIconPosSet( syswk->app, (mvwk->now_x>>8), (mvwk->now_y>>8) );
	BOX2OBJ_ItemIconCursorMove( syswk->app );

	mvwk->cnt++;

	return TRUE;

}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコン移動（タッチ）
 *
 * @param	syswk		ボックス画面システムワーク
 *
 * @retval	"TRUE = 移動中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemIconMoveMain( BOX2_SYS_WORK * syswk )
{
	return ItemIconMoveMainCore( syswk, TRUE );
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムアイコン移動（キー）
 *
 * @param	syswk		ボックス画面システムワーク
 *
 * @retval	"TRUE = 移動中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL ItemIconMoveMainHand( BOX2_SYS_WORK * syswk )
{
	return ItemIconMoveMainCore( syswk, FALSE );
}


//============================================================================================
//	トレイ選択
//============================================================================================

u8 BOX2MAIN_GetBoxMoveTrayNum( BOX2_SYS_WORK * syswk, s8 mv )
{
	s8	pos = syswk->box_mv_pos;
	
	pos += mv;
	if( pos < 0 ){
		pos += syswk->trayMax;
	}else if( pos >= syswk->trayMax ){
		pos -= syswk->trayMax;
	}
	return pos;
}

//============================================================================================
//	壁紙
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙キャラデータ読み込み
 *
 * @param		syswk	ボックス画面システムワーク
 * @param		id		壁紙番号
 * @param		cgx		転送キャラ位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void WallCharLoad( BOX2_SYS_WORK * syswk, u32 id, u32 cgx )
{
	NNSG2dCharacterData * chr;
	void * buf;
	u8 * title;

	buf = GFL_ARC_UTIL_LoadBGCharacter(
					ARCID_BOX2_GRA, NARC_box_gra_box_wp01_lz_NCGR+id, TRUE, &chr, HEAPID_BOX_APP );

	GFL_BG_LoadCharacter( GFL_BG_FRAME3_M, chr->pRawData, chr->szByte, cgx );

	title = chr->pRawData;

//	BOX2BMP_TrayNamePut( syswk, &title[0x20*WALL_TITLE_POS] );
/*
	BOX2BMP_TrayNamePut(
		syswk, &title[0x20*WALL_TITLE_POS], cgx+WALL_TITLE_POS, WALL_SX, WALL_TITLE_SY );
*/
	GFL_HEAP_FreeMemory( buf );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙パレットデータ読み込み
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		id			壁紙番号
 * @param		pal			パレット番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void WallPaletteLoad( BOX2_SYS_WORK * syswk, u32 id, u32 pal )
{
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_BOX2_GRA, NARC_box_gra_box_wp01_NCLR+id,
		PALTYPE_MAIN_BG, pal*0x20, 0x20, HEAPID_BOX_APP );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙スクリーンデータ読み込み
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		id			壁紙番号
 * @param		px			転送Ｘ座標
 * @param		cgx			転送キャラ位置
 * @param		pal			パレット番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void WallScreenLoad( BOX2_SYS_WORK * syswk, u32 id, u32 px, u32 cgx, u32 pal )
{
	NNSG2dScreenData * scrn;
	void * buf;
	u16 * raw;
	u32	x;
	u16	dat;
	u8	i, j;

	buf = GFL_ARC_UTIL_LoadScreen(
					ARCID_BOX2_GRA, NARC_box_gra_box_wp01_lz_NSCR, TRUE, &scrn, HEAPID_BOX_APP );
	raw = (u16 *)scrn->rawData;
	for( i=0; i<WALL_SY; i++ ){
		x = px;
		for( j=0; j<WALL_SX; j++ ){
			dat = ( raw[i*WALL_SX+j] & 0xfff ) + ( pal << 12 ) + cgx;
			GFL_BG_WriteScreen( GFL_BG_FRAME3_M, &dat, x, WALL_PY+i, 1, 1 );
			x++;
			if( x >= 64 ){ x = 0; }
		}
	}
	GFL_HEAP_FreeMemory( buf );

	GFL_BG_FillScreen(
		GFL_BG_FRAME3_M, WALL_SPACE_CHR, x, 0, 2, WALL_SY, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen(
		GFL_BG_FRAME3_M, WALL_SPTAG_CHR, x, 5, 2, 1, GFL_BG_SCRWRT_PALIN );
}

//--------------------------------------------------------------------------------------------
/**
 * 壁紙グラフィックセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	id		壁紙ＩＤ
 * @param	px		表示Ｘ座標
 * @param	cgx		転送キャラ位置
 * @param	pal		使用パレット
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void WallGraSet( BOX2_SYS_WORK * syswk, u32 id, u32 px, u32 cgx, u32 pal )
{
	WallCharLoad( syswk, id, cgx );
	WallPaletteLoad( syswk, id, pal );
	WallScreenLoad( syswk, id, px, cgx, pal );

	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙セット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		id			壁紙ＩＤ
 * @param		mv			スクロール方向
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_WallPaperSet( BOX2_SYS_WORK * syswk, u32 id, u32 mv )
{
	u32	chr, pal;

	if( mv == BOX2MAIN_TRAY_SCROLL_L ){
		syswk->app->wall_px -= WALL_PUT_SPACE;
		if( syswk->app->wall_px < 0 ){
			syswk->app->wall_px += 64;
		}
	}else if( mv == BOX2MAIN_TRAY_SCROLL_R ){
		syswk->app->wall_px += WALL_PUT_SPACE;
		if( syswk->app->wall_px >= 64 ){
			syswk->app->wall_px -= 64;
		}
	}

	if( syswk->app->wall_area == 0 ){
		chr = WALL_CGX_POS1;
		pal = BOX2MAIN_BG_PAL_WALL1;
	}else{
		chr = WALL_CGX_POS2;
		pal = BOX2MAIN_BG_PAL_WALL2;
	}
	syswk->app->wall_area ^= 1;

	WallGraSet( syswk, id, syswk->app->wall_px, chr, pal );

	// ボックス名設定
	// 初期化時
	if( mv == BOX2MAIN_TRAY_SCROLL_NONE ){
		BOX2BMP_BoxNameWrite( syswk, syswk->tray, BOX2MAIN_FNTOAM_BOX_NAME1 );
		BOX2OBJ_FontOamVanish( syswk->app, BOX2MAIN_FNTOAM_BOX_NAME2, FALSE );
	// スクロール時
	}else{
		u32	idx;
		if( BOX2OBJ_CheckFontOamVanish( syswk->app, BOX2MAIN_FNTOAM_BOX_NAME1 ) == TRUE ){
			idx = BOX2MAIN_FNTOAM_BOX_NAME2;
		}else{
			idx = BOX2MAIN_FNTOAM_BOX_NAME1;
		}
		BOX2BMP_BoxNameWrite( syswk, syswk->tray, idx );
		BOX2OBJ_SetBoxNamePos( syswk->app, idx, mv );
		BOX2OBJ_FontOamVanish( syswk->app, idx, TRUE );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 壁紙変更
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	id		壁紙番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_WallPaperChange( BOX2_SYS_WORK * syswk, u32 id )
{
	u32	chr, pal;

	if( syswk->app->wall_area == 0 ){
		chr = WALL_CGX_POS1;
		pal = BOX2MAIN_BG_PAL_WALL1;
	}else{
		chr = WALL_CGX_POS2;
		pal = BOX2MAIN_BG_PAL_WALL2;
	}
	syswk->app->wall_area ^= 1;

	WallCharLoad( syswk, id, chr );
	PaletteWorkSet_Arc(
		syswk->app->pfd,
		ARCID_BOX2_GRA, NARC_box_gra_box_wp01_NCLR+id,
		HEAPID_BOX_APP, FADE_MAIN_BG, FADE_PAL_ONE_SIZE, pal*16 );
	WallScreenLoad( syswk, id, syswk->app->wall_px, chr, pal );

	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙番号取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		num			トレイ番号
 *
 * @return	壁紙番号
 *
 *	デフォルト壁紙からの通し番号にする
 */
//--------------------------------------------------------------------------------------------
u32 BOX2MAIN_GetWallPaperNumber( BOX2_SYS_WORK * syswk, u32 num )
{
/*
	u32	wp = BOXDAT_GetWallPaperNumber( syswk->dat->sv_box, num );

	if( wp >= BOX_NORMAL_WALLPAPER_MAX ){
		return ( wp - BOX_TOTAL_WALLPAPER_MAX_PL + BOX_NORMAL_WALLPAPER_MAX );
	}
	return wp;
*/
	return BOXDAT_GetWallPaperNumber( syswk->dat->sv_box, num );
}


//============================================================================================
//	データ表示関連
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 表示データ作成
 *
 * @param	ppp		POKEMON_PASO_PARAM
 *
 * @return	作成したデータ
 */
//--------------------------------------------------------------------------------------------
static BOX2_POKEINFO_DATA * PokeInfoDataMake( POKEMON_PASO_PARAM * ppp )
{
	BOX2_POKEINFO_DATA * info;
	u32	i;

	if( PPP_Get( ppp, ID_PARA_poke_exist, NULL ) != 0 ){
		info = GFL_HEAP_AllocMemory( HEAPID_BOX_APP, sizeof(BOX2_POKEINFO_DATA) );

		info->ppp = ppp;

		info->mons = PPP_Get( ppp, ID_PARA_monsno, NULL );
		info->item = PPP_Get( ppp, ID_PARA_item, NULL );
		info->rand = PPP_Get( ppp, ID_PARA_personal_rnd, NULL );

		info->type1 = PPP_Get( ppp, ID_PARA_type1, NULL );
		info->type2 = PPP_Get( ppp, ID_PARA_type2, NULL );

		info->tokusei = PPP_Get( ppp, ID_PARA_speabino, NULL );
		info->seikaku = PPP_GetSeikaku( ppp );

		info->mark   = PPP_Get( ppp, ID_PARA_mark, NULL );
		info->lv     = PPP_Get( ppp, ID_PARA_level, NULL );
		info->tamago = PPP_Get( ppp, ID_PARA_tamago_flag, NULL );
		info->sex    = PPP_GetSex( ppp );

		if( PPP_CheckRare( ppp ) == TRUE ){
			info->rare = 1;
		}else{
			info->rare = 0;
		}

		// ポケルス
		if( POKERUS_CheckInfectPPP( ppp ) == TRUE ){
			info->pokerus = 1;
		}else if( POKERUS_CheckInfectedPPP( ppp ) == TRUE ){
			info->pokerus = 2;
		}else{
			info->pokerus = 0;
		}

/*
		// テスト
		info->rare = 1;
		info->pokerus = 1;
*/

		if( info->mons != MONSNO_NIDORAN_F && info->mons != MONSNO_NIDORAN_M && info->tamago == 0 ){
			info->sex_put = 1;
		}else{
			info->sex_put = 0;
		}

		for( i=0; i<4; i++ ){
			info->waza[i] = PPP_Get( ppp, ID_PARA_waza1+i, NULL );
		}
	}else{
		info = NULL;
	}

	return info;
}

//--------------------------------------------------------------------------------------------
/**
 * 表示データ解放
 *
 * @param	info	表示データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeInfoDataFree( BOX2_POKEINFO_DATA * info )
{
	GFL_HEAP_FreeMemory( info );
}

//--------------------------------------------------------------------------------------------
/**
 * 情報表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	info	表示データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PokeInfoPutModeNormal( BOX2_SYS_WORK * syswk, BOX2_POKEINFO_DATA * info )
{
/*
	u32	winID;
	u32	i;

	winID = BOX2BMP_PokeSkillWrite( syswk, info );
	for( i=0; i<4; i++ ){
		BGWINFRM_BmpWinOn( syswk->app->wfrm, BOX2MAIN_WINFRM_SUBDISP, syswk->app->win[winID+i].win );
	}
	BOX2BGWFRM_SubDispWazaFrmOutPosSet( syswk->app->wfrm );
*/

	BOX2OBJ_PokeGraChange( syswk, info, BOX2OBJ_ID_POKEGRA );
//	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_POKEGRA+(syswk->app->pokegra_swap^1), TRUE );
//	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_POKEGRA+syswk->app->pokegra_swap, FALSE );

	BOX2OBJ_TypeIconChange( syswk->app, info );
	BOX2BMP_PokeDataPut( syswk, info );

	BOX2MAIN_SubDispMarkingChange( syswk, info->mark );
	PutSubDispPokeMark( syswk->app, info );
}

//--------------------------------------------------------------------------------------------
/**
 * 情報表示（「もちものをせいりする」のとき）
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	info	表示データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void PokeInfoPutModeItem( BOX2_SYS_WORK * syswk, BOX2_POKEINFO_DATA * info )
{
#if 0
	if( info->item != ITEM_DUMMY_DATA ){
		u32 winID = BOX2BMP_PokeItemInfoWrite( syswk, info->item );
		BGWINFRM_BmpWinOn( syswk->app->wfrm, BOX2MAIN_WINFRM_SUBDISP, syswk->app->win[winID].win );
		BGWINFRM_BmpWinOn( syswk->app->wfrm, BOX2MAIN_WINFRM_SUBDISP, syswk->app->win[winID+1].win );
		BOX2OBJ_ItemIconChangeSub( syswk->app, info->item );
	}
	BOX2BGWFRM_SubDispItemFrmOutPosSet( syswk->app->wfrm );
#endif

	BOX2OBJ_PokeGraChange( syswk, info, BOX2OBJ_ID_POKEGRA );
//	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_POKEGRA+(syswk->app->pokegra_swap^1), TRUE );
//	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_POKEGRA+syswk->app->pokegra_swap, FALSE );

	BOX2OBJ_TypeIconChange( syswk->app, info );
	BOX2BMP_PokeDataPut( syswk, info );

	BOX2MAIN_SubDispMarkingChange( syswk, info->mark );
	PutSubDispPokeMark( syswk->app, info );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 指定位置のポケモン情報表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		位置
 *
 * @retval	"TRUE = 表示"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BOX2MAIN_PokeInfoPut( BOX2_SYS_WORK * syswk, u32 pos )
{
	BOX2_POKEINFO_DATA * info;
	POKEMON_PASO_PARAM * ppp;
	BOOL	fast;

	ppp = BOX2MAIN_PPPGet( syswk, syswk->tray, pos );

	if( ppp != NULL ){
		fast = PPP_FastModeOn( ppp );
		info = PokeInfoDataMake( ppp );

		if( info != NULL ){
/*
			if( syswk->dat->callMode == BOX_MODE_ITEM ){
				PokeInfoPutModeItem( syswk, info );
			}else{
				PokeInfoPutModeNormal( syswk, info );
			}
*/
			PokeInfoPutModeNormal( syswk, info );
			PokeInfoDataFree( info );
			PPP_FastModeOff( ppp, fast );
		}else{
			// オフ
			BOX2MAIN_PokeInfoOff( syswk );
			PPP_FastModeOff( ppp, fast );
			return FALSE;
		}
	}else{
		// オフ
		BOX2MAIN_PokeInfoOff( syswk );
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * 上画面の一部を再描画
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_PokeInfoRewrite( BOX2_SYS_WORK * syswk, u32 pos )
{
	BOX2_POKEINFO_DATA * info;
	POKEMON_PASO_PARAM * ppp;
	BOOL	fast;

	ppp  = BOX2MAIN_PPPGet( syswk, syswk->tray, pos );
	fast = PPP_FastModeOn( ppp );
	info = PokeInfoDataMake( ppp );

	BOX2OBJ_PokeGraChange( syswk, info, BOX2OBJ_ID_POKEGRA );
//	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_POKEGRA+(syswk->app->pokegra_swap^1), TRUE );
//	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_POKEGRA+syswk->app->pokegra_swap, FALSE );

	BOX2OBJ_TypeIconChange( syswk->app, info );
	BOX2BMP_PokeDataPut( syswk, info );

	PokeInfoDataFree( info );
	PPP_FastModeOff( ppp, fast );
}

//--------------------------------------------------------------------------------------------
/**
 * 上画面表示オフ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_PokeInfoOff( BOX2_SYS_WORK * syswk )
{
	u32	i;

	// ポケグラオフ
	if( syswk->app->clwk[BOX2OBJ_ID_POKEGRA] != NULL ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_POKEGRA, FALSE );
	}
	if( syswk->app->clwk[BOX2OBJ_ID_POKEGRA2] != NULL ){
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_POKEGRA2, FALSE );
	}
	
	// タイプアイコンオフ
	for( i=BOX2OBJ_ID_TYPEICON; i<BOX2OBJ_ID_TYPEICON+POKETYPE_MAX; i++ ){
		BOX2OBJ_Vanish( syswk->app, i, FALSE );
	}

	BOX2BMP_PokeDataOff( syswk->app );							// 文字列オフ

	BOX2MAIN_SubDispMarkingChange( syswk, 0 );					// マークオフ
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_RARE, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_POKERUS, FALSE );
	BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_POKERUS_ICON, FALSE );

/*
	// ウィンフレームオフ
	if( syswk->dat->callMode == BOX_MODE_ITEM ){
		BOX2BGWFRM_SubDispItemFrmOutPosSet( syswk->app->wfrm );
	}else{
		BOX2BGWFRM_SubDispWazaFrmOutPosSet( syswk->app->wfrm );
	}
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 非選択に切り替え
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_PokeSelectOff( BOX2_SYS_WORK * syswk )
{
	BOX2MAIN_PokeInfoOff( syswk );							// 上画面クリア

	BOX2OBJ_PokeCursorVanish( syswk, FALSE );
	syswk->get_pos = BOX2MAIN_GETPOS_NONE;
}


//============================================================================================
//	マーキング
//============================================================================================

static void MarkingChange( BOX2_SYS_WORK * syswk, u32 id, u32 mark )
{
	u16	anm;
	u16	i;

	for( i=0; i<6; i++ ){
		if( mark & (1<<i) ){
			anm = APP_COMMON_POKE_MARK_CIRCLE_BLACK + i*2;
		}else{
			anm = APP_COMMON_POKE_MARK_CIRCLE_WHITE + i*2;
		}
		BOX2OBJ_AnmSet( syswk->app, id+i, anm );
	}
}

void BOX2MAIN_MainDispMarkingChange( BOX2_SYS_WORK * syswk, u32 mark )
{
	MarkingChange( syswk, BOX2OBJ_ID_MARK1, mark );
}

//--------------------------------------------------------------------------------------------
/**
 * 上画面のマーキングを変更
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	maek	マークデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_SubDispMarkingChange( BOX2_SYS_WORK * syswk, u32 mark )
{
	MarkingChange( syswk, BOX2OBJ_ID_MARK1_S, mark );
}

// レア・ポケルスマーク表示
static void PutSubDispPokeMark( BOX2_APP_WORK * appwk, BOX2_POKEINFO_DATA * info )
{
	if( info->rare == 0 ){
		BOX2OBJ_Vanish( appwk, BOX2OBJ_ID_RARE, FALSE );
	}else{
		BOX2OBJ_Vanish( appwk, BOX2OBJ_ID_RARE, TRUE );
	}

	// ポケルスに感染していない
	if( info->pokerus == 0 ){
		BOX2OBJ_Vanish( appwk, BOX2OBJ_ID_POKERUS, FALSE );
		BOX2OBJ_Vanish( appwk, BOX2OBJ_ID_POKERUS_ICON, FALSE );
	// ポケルスに感染している
	}else if( info->pokerus == 1 ){
		BOX2OBJ_Vanish( appwk, BOX2OBJ_ID_POKERUS, FALSE );
		BOX2OBJ_Vanish( appwk, BOX2OBJ_ID_POKERUS_ICON, TRUE );
	// ポケルスに感染したことがある
	}else{
		BOX2OBJ_Vanish( appwk, BOX2OBJ_ID_POKERUS, TRUE );
		BOX2OBJ_Vanish( appwk, BOX2OBJ_ID_POKERUS_ICON, FALSE );
	}
}



//============================================================================================
//	エリアチェック
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * エリアチェック
 *
 * @param	x		Ｘ座標
 * @param	y		Ｙ座標
 * @param	area	チェックテーブル
 *
 * @retval	"TRUE = 範囲内"
 * @retval	"FALSE = 範囲外"
 */
//--------------------------------------------------------------------------------------------
static BOOL AreaCheck( s16 x, s16 y, const AREA_CHECK * area )
{
	if( x >= area->lx && x <= area->rx && y >= area->ty && y < area->by ){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * トレイエリアチェック
 *
 * @param	x		Ｘ座標
 * @param	y		Ｙ座標
 * @param	area	チェックテーブル
 *
 * @retval	"TRUE = 範囲内"
 * @retval	"FALSE = 範囲外"
 */
//--------------------------------------------------------------------------------------------
static u32 TrayPokePutAreaCheck( s16 x, s16 y )
{
	if( AreaCheck( x, y, &TrayPokeArea ) == TRUE ){
		if( x < TRAYGRID_LX ){
			x = 0;
		}else if( x >= TRAYGRID_RX ){
			x = BOX2OBJ_POKEICON_H_MAX - 1;
		}else{
			x = ( x - TRAYGRID_LX ) / TRAYGRID_SX;
		}
		y = ( y - TRAYAREA_UY ) / TRAYGRID_SY;
		return ( y * BOX2OBJ_POKEICON_H_MAX + x );
	}
	return BOX2MAIN_GETPOS_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * 手持ちエリアチェック
 *
 * @param	x		Ｘ座標
 * @param	y		Ｙ座標
 * @param	area	チェックテーブル
 *
 * @retval	"BOX2MAIN_GETPOS_NONE = 範囲内"
 * @retval	"それ以外 = 選択位置"
 */
//--------------------------------------------------------------------------------------------
static u32 PartyPokePutAreaCheck( s16 x, s16 y, const AREA_CHECK * area )
{
	u32	i;

	for( i=0; i<BOX2OBJ_POKEICON_MINE_MAX; i++ ){
		if( AreaCheck( x, y, &area[i] ) == TRUE ){
			return ( i + BOX2OBJ_POKEICON_TRAY_MAX );
		}
	}
	return BOX2MAIN_GETPOS_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * ボックス移動トレイエリアチェック
 *
 * @param	x		Ｘ座標
 * @param	y		Ｙ座標
 *
 * @retval	"BOX2MAIN_GETPOS_NONE = 範囲内"
 * @retval	"それ以外 = 選択位置"
 */
//--------------------------------------------------------------------------------------------
static u32 BoxMovePutAreaCheck( BOX2_SYS_WORK * syswk, s16 x, s16 y )
{
/*
	u32	i;

	for( i=0; i<BOX2OBJ_TRAYICON_MAX; i++ ){
		if( AreaCheck( x, y, &BoxMoveTrayArea[i] ) == TRUE ){
			return ( BOX2MAIN_BOXMOVE_FLG + i );
		}
	}
*/
	u32	ret = BOX2UI_HitCheckTrayIcon( syswk->app->tpx, syswk->app->tpy );

	if( ret != BOX2MAIN_GETPOS_NONE ){
		u32	pos = ret - BOX2OBJ_POKEICON_PUT_MAX;
		pos = syswk->box_mv_pos + pos;
		if( pos >= syswk->trayMax ){
			pos -= syswk->trayMax;
		}
		if( pos == syswk->tray ){
			return BOX2MAIN_GETPOS_NONE;
		}
	}

	return ret;
}



//============================================================================================
//	サブプロセス
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * ステータス画面呼び出し
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_PokeStatusCall( BOX2_SYS_WORK * syswk )
{
	FS_EXTERN_OVERLAY(poke_status);

	PSTATUS_DATA * pst = GFL_HEAP_AllocMemory( HEAPID_BOX_SYS, sizeof(PSTATUS_DATA) );

	if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
		pst->ppd = BOX2MAIN_PPPGet( syswk, syswk->get_tray, 0 );
		pst->ppt = PST_PP_TYPE_POKEPASO;
		pst->max = BOX2OBJ_POKEICON_TRAY_MAX;
		pst->pos = syswk->get_pos;
	}else{
		pst->ppd = syswk->dat->pokeparty;
		pst->ppt = PST_PP_TYPE_POKEPARTY;
		pst->max = PokeParty_GetPokeCount( syswk->dat->pokeparty );
		pst->pos = syswk->get_pos - BOX2OBJ_POKEICON_TRAY_MAX;
	}
	pst->page       = PPT_INFO;
	pst->game_data  = syswk->dat->gamedata;
	pst->cfg        = syswk->dat->cfg;
	pst->zukan_mode = syswk->dat->zknMode;
	pst->mode       = PST_MODE_NORMAL;

	GFL_PROC_SysCallProc( FS_OVERLAY_ID(poke_status), &PokeStatus_ProcData, pst );

	syswk->subProcWork = pst;

	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス画面終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_PokeStatusExit( BOX2_SYS_WORK * syswk )
{
	PSTATUS_DATA * pst = syswk->subProcWork;

	if( syswk->poke_get_key == 0 ){
		if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			syswk->get_pos = pst->pos;
		}else{
			syswk->get_pos = pst->pos + BOX2OBJ_POKEICON_TRAY_MAX;
		}
	}

	GFL_HEAP_FreeMemory( syswk->subProcWork );

	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * バッグ画面呼び出し
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_BagCall( BOX2_SYS_WORK * syswk )
{
	BAG_PARAM * wk = BAG_CreateParam( syswk->dat->gamedata, NULL, BAG_MODE_POKELIST, HEAPID_BOX_SYS );

	GFL_PROC_SysCallProc( FS_OVERLAY_ID(bag), &ItemMenuProcData, wk );

	syswk->subProcWork = wk;

	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * バッグ画面終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_BagExit( BOX2_SYS_WORK * syswk )
{
	BAG_PARAM * wk = syswk->subProcWork;

	syswk->subRet = wk->ret_item;

	GFL_HEAP_FreeMemory( syswk->subProcWork );

	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * 名前入力画面呼び出し
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_NameInCall( BOX2_SYS_WORK * syswk )
{
	BOX_NAMEIN_WORK * wk;

	wk = GFL_HEAP_AllocClearMemory( HEAPID_BOX_SYS, sizeof(BOX_NAMEIN_WORK) );

	wk->name = GFL_STR_CreateBuffer( BOX_TRAYNAME_BUFSIZE, HEAPID_BOX_SYS );
	BOXDAT_GetBoxName( syswk->dat->sv_box, syswk->box_mv_pos, wk->name );

	GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
	wk->prm = NAMEIN_AllocParam(
							HEAPID_BOX_SYS, NAMEIN_BOX, 0, 0, BOX_TRAYNAME_MAXLEN, wk->name );
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );
	GFL_PROC_SysCallProc( FS_OVERLAY_ID(namein), &NameInputProcData, wk->prm );

	syswk->subProcWork = wk;

	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * 名前入力画面終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_NameInExit( BOX2_SYS_WORK * syswk )
{
	BOX_NAMEIN_WORK * wk = syswk->subProcWork;

	if( wk->prm->cancel == FALSE ){
		BOXDAT_SetBoxName( syswk->dat->sv_box, syswk->box_mv_pos, wk->prm->strbuf );
	}
	syswk->subRet = wk->prm->cancel;

	GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
	NAMEIN_FreeParam( wk->prm );
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );
	GFL_STR_DeleteBuffer( wk->name );
	GFL_HEAP_FreeMemory( syswk->subProcWork );

	return 0;
}


//============================================================================================
//	VBLANK FUNC
//============================================================================================

static void MoveGetPokeIcon( BOX2_SYS_WORK * syswk, u32 x, u32 y )
{
	BOX2OBJ_SetPos(
		syswk->app, syswk->app->pokeicon_id[BOX2OBJ_POKEICON_GET_POS], (s16)x, (s16)y-8, CLSYS_DEFREND_MAIN );
	BOX2OBJ_PokeCursorMove( syswk->app, BOX2OBJ_POKEICON_GET_POS );
	syswk->app->tpx = x;
	syswk->app->tpy = y;
}


//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 取得ポケモン移動（パーティ）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncGetPokeMoveParty( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	u32	put_pos;
	u32	tpx, tpy;
//	s16	ipx, ipy;
	BOOL	ret1, ret2;

	vf = &syswk->app->vfunk;

/*
	if( vf->seq == 0 ){
		BOX2OBJ_GetPos(
			syswk->app, syswk->app->pokeicon_id[syswk->get_pos], &ipx, &ipy, CLSYS_DEFREND_MAIN );
	}
*/

	if( BGWINFRM_MoveCheck( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY ) == 1 ){
		ret1 = BOX2BGWFRM_PartyPokeFrameMove( syswk );
	}else{
		ret1 = 0;
	}
	ret2 = BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );

	switch( vf->seq ){
	case SEIRI_SEQ_ICON_GET:		// アイコン取得中

		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == FALSE ){

/*
			BOX2OBJ_SetPos(
				syswk->app, syswk->app->pokeicon_id[syswk->get_pos], ipx, ipy, CLSYS_DEFREND_MAIN );
*/
			BOX2OBJ_SetPos(
				syswk->app, syswk->app->pokeicon_id[BOX2OBJ_POKEICON_GET_POS],
				syswk->app->tpx, syswk->app->tpy-8, CLSYS_DEFREND_MAIN );

			put_pos = BOX2MAIN_GETPOS_NONE;

			if( BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == TRUE ){
				// 手持ちポケモン
				put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaRight );
				// 参照中のトレイ
				if( put_pos == BOX2MAIN_GETPOS_NONE ){
					put_pos = TrayPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy );
				}
			}else if( ret1 == 0 ){
				put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaLeft );
			}

			PokeIconMoveBoxPartyDataMake( syswk, syswk->get_pos, put_pos );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );

			// 移動後のカーソル位置補正
			if( put_pos != BOX2MAIN_GETPOS_NONE ){
				syswk->get_pos = PokeIconMoveAfterPosGet( syswk, BOX2OBJ_POKEICON_GET_POS );
			}

			// メニュー表示開始
			if( BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE &&
					ret1 == 0 ){
				BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
			}

			vf->seq = SEIRI_SEQ_ICON_PUT;

		}else{
			// ボックス移動フレーム表示チェック
			if( ret1 == 0 && ret2 == 0 ){
				if( BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE ){
					if( BOX2UI_HitCheckPartyFrameLeft() == FALSE ){
						syswk->get_tray  = BOX2MAIN_GETPOS_NONE;		// 取得したボックス
						syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
						BOX2BGWFRM_PartyPokeFrameRightMoveSet( syswk->app->wfrm );
						// 戻り先のシーケンスを「つかむ」の処理にする
						BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
						BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
						BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
						if( syswk->dat->callMode == BOX_MODE_BATTLE ){
							CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
						}
						BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
						BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_PASSIVE );
						syswk->app->old_cur_pos = syswk->get_pos;
						syswk->app->vnext_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE;
					}
				}else{
					// トレイスクロール矢印ヒットチェック
					u32 hit = BOX2UI_HitCheckContTrayArrow();
					if( hit == 0 ){
						BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_L );
						vf->seq = SEIRI_SEQ_SCROLL_L;
					}else if( hit == 1 ){
						BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_R );
						vf->seq = SEIRI_SEQ_SCROLL_R;
					}
				}
			}
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_ICON_PUT:	// アイコン配置
//		if( PokeIconObjMove( syswk ) == FALSE && ret1 == 0 && ret2 == 0 ){
		if( PokeIconObjDropChange( syswk ) == FALSE && ret1 == 0 && ret2 == 0 ){
			if( vf->seq == SEIRI_SEQ_ICON_PUT ){
				vf->seq = SEIRI_SEQ_END;
			}
		}
		break;

	case SEIRI_SEQ_SCROLL_L:	// トレイスクロール（左）
		if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = SEIRI_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_SCROLL_R:	// トレイスクロール（右）
		if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = SEIRI_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_END:				// 終了
		{
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
		}
		if( BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE ){
			BOX2OBJ_PokeCursorMove( syswk->app, syswk->get_pos );
			BOX2OBJ_PokeCursorVanish( syswk, TRUE );
		}else{
//			BOX2OBJ_PokeCursorMove( syswk->app, syswk->get_pos );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
			CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		}
		vf->seq = 0;
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 取得ポケモン移動（パーティ＆トレイ）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncGetPokeMoveBoxParty( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	u32	tpx, tpy;
	u32	put_pos;
	u32	ret1, ret2;
	u32	hit;

	vf = &syswk->app->vfunk;

	if( BGWINFRM_MoveCheck( syswk->app->wfrm, BOX2MAIN_WINFRM_MOVE ) == 1 ){
		ret1 = BOX2MAIN_VFuncBoxMoveFrmIn( syswk );
	}else{
		ret1 = 0;
	}
	ret2 = BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );

	switch( vf->seq ){
	case SEIRI_SEQ_ICON_GET:		// アイコン取得中

		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == FALSE ){

			put_pos = BOX2MAIN_GETPOS_NONE;
			// トレイアイコン
			if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == TRUE ){
				put_pos = BoxMovePutAreaCheck( syswk, syswk->app->tpx, syswk->app->tpy );
			}
			// 手持ちポケモン
			if( BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == TRUE ){
				put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaRight );
			}
			// 参照中のトレイ
			if( put_pos == BOX2MAIN_GETPOS_NONE ){
				put_pos = TrayPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy );
			}
/*
			if( put_pos == BOX2MAIN_GETPOS_NONE ){
				put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaRight );
			}
*/
			PokeIconMoveBoxPartyDataMake( syswk, syswk->get_pos, put_pos );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );

			// 移動後のカーソル位置補正
			if( put_pos != BOX2MAIN_GETPOS_NONE ){
				if( put_pos >= BOX2OBJ_POKEICON_PUT_MAX ){
					syswk->get_pos = put_pos - BOX2OBJ_POKEICON_PUT_MAX + BOX2UI_ARRANGE_PGT_TRAY2;
				}else{
					syswk->get_pos = PokeIconMoveAfterPosGet( syswk, BOX2OBJ_POKEICON_GET_POS );
				}
			}else{
				BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
			}

			// メニュー表示開始
			if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == FALSE &&
					BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE &&
					ret1 == 0 ){
				BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
			}

			if( put_pos == BOX2MAIN_GETPOS_NONE && syswk->tray != syswk->get_tray ){
/*
				syswk->tray = syswk->get_tray;
//				BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_NONE );
				BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
				BOX2MAIN_WallPaperSet( syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), BOX2MAIN_TRAY_SCROLL_L );
				vf->seq = SEIRI_SEQ_CANCEL_SCROLL;
*/
				u32	dir = BOX2MAIN_GetTrayScrollDir( syswk, syswk->tray, syswk->get_tray );
				syswk->tray = syswk->get_tray;
				BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
				BOX2MAIN_WallPaperSet( syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), dir );
				if( dir == BOX2MAIN_TRAY_SCROLL_L ){
					vf->seq = SEIRI_SEQ_CANCEL_SCROLL_L;
				}else{
					vf->seq = SEIRI_SEQ_CANCEL_SCROLL_R;
				}
			}else{
				vf->seq = SEIRI_SEQ_ICON_PUT;
			}

		}else{
			// ボックス移動フレーム表示チェック
			if( ret1 == 0 && ret2 == 0 &&
					BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == FALSE &&
					BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE ){
				if( BOX2UI_HitCheckPosTrayPoke( tpx, tpy ) != syswk->get_pos ){
					syswk->box_mv_pos = syswk->tray;	// 現在参照しているボックス
					BOX2MAIN_InitBoxMoveFrameScroll( syswk );
					// 戻り先のシーケンスを「つかむ」の処理にする
					BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
					BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
					BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_PASSIVE );
					syswk->app->old_cur_pos = syswk->get_pos;
					syswk->app->vnext_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE;
				}
			}
			// トレイスクロール矢印ヒットチェック
			hit = BOX2UI_HitCheckContTrayArrow();
			if( hit == 0 ){
				BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_L );
				vf->seq = SEIRI_SEQ_SCROLL_L;
			}else if( hit == 1 ){
				BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_R );
				vf->seq = SEIRI_SEQ_SCROLL_R;
			}
			// トレイアイコンスクロールヒットチェック
			if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == TRUE ){
				hit = BOX2UI_HitCheckContTrayIconScroll();
				if( hit == 0 ){
					BOX2MAIN_InitTrayIconScroll( syswk, -1 );
					vf->seq = SEIRI_SEQ_TI_SCROLL_U;
				}else if( hit == 1 ){
					BOX2MAIN_InitTrayIconScroll( syswk, 1 );
					vf->seq = SEIRI_SEQ_TI_SCROLL_D;
				}
			}
			// ボックス名表示
			if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == TRUE ){
				hit = BOX2UI_HitCheckTrayIcon( tpx, tpy );
				if( hit != BOX2MAIN_GETPOS_NONE ){
					BOX2OBJ_ChangeTrayName( syswk, hit-BOX2OBJ_POKEICON_PUT_MAX, TRUE );
				}else{
					BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
				}
			}
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_CANCEL_SCROLL_L:		// トレイが切り替わっていた場合のスクロール処理（左）
	case SEIRI_SEQ_CANCEL_SCROLL_R:		// トレイが切り替わっていた場合のスクロール処理（右）
		if( vf->seq == SEIRI_SEQ_CANCEL_SCROLL_L ){
			if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
				vf->seq = SEIRI_SEQ_ICON_PUT;
			}
		}
		if( vf->seq == SEIRI_SEQ_CANCEL_SCROLL_R ){
			if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
				vf->seq = SEIRI_SEQ_ICON_PUT;
			}
		}
	case SEIRI_SEQ_ICON_PUT:	// アイコン配置
		if( PokeIconObjMove( syswk ) == FALSE && ret1 == 0 && ret2 == 0 ){
			if( vf->seq == SEIRI_SEQ_ICON_PUT ){
				vf->seq = SEIRI_SEQ_END;
			}
		}
		break;

	case SEIRI_SEQ_SCROLL_L:	// トレイスクロール（左）
		if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = SEIRI_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_SCROLL_R:	// トレイスクロール（右）
		if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = SEIRI_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_TI_SCROLL_U:		// トレイアイコンスクロール（上）
		if( BOX2MAIN_VFuncTrayIconScrollDown( syswk ) == 0 ){
			vf->seq = SEIRI_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_TI_SCROLL_D:		// トレイアイコンスクロール（下）
		if( BOX2MAIN_VFuncTrayIconScrollUp( syswk ) == 0 ){
			vf->seq = SEIRI_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_END:				// 終了
		{
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
		}
		if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == FALSE &&
				BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE ){
			BOX2OBJ_PokeCursorMove( syswk->app, syswk->get_pos );
			BOX2OBJ_PokeCursorVanish( syswk, TRUE );
		}else{
//			BOX2OBJ_PokeCursorMove( syswk->app, syswk->get_pos );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
			CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		}
		vf->seq = 0;
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手持ちを逃がした後のOBJ動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncPartyPokeFreeSort( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
		PokeIconFreeDataMake( syswk, syswk->get_pos );
		vf->seq++;
	case 1:
		if( PokeIconObjMove( syswk ) == FALSE ){
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
			vf->seq = 0;
			return 0;
		}
	}
	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : ポケモンを手持ちに加える動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncPartyInPokeMove( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	u32	put_pos;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
		put_pos = PokeParty_GetPokeCount( syswk->dat->pokeparty ) + BOX2OBJ_POKEICON_TRAY_MAX;
		PokeIconMoveBoxPartyDataMake( syswk, syswk->get_pos, put_pos );
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		vf->seq++;
		break;

	case 1:
		if( PokeIconObjMove( syswk ) == FALSE ){
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
			vf->seq = 0;
			return 0;
		}
	}
	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : トレイをスクロール（左ボタンが押されたとき）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncTrayScrollLeft( BOX2_SYS_WORK * syswk )
{
	if( syswk->app->vfunk.cnt == BOX2MAIN_TRAY_SCROLL_CNT ){
/*
		if( CURSORMOVE_CursorOnOffGet( syswk->app->cmwk ) == TRUE ){
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		}
*/
		if( CURSORMOVE_CursorOnOffGet( syswk->app->cmwk ) == FALSE ){
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		}
		if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == TRUE ){
			BOX2OBJ_EndTrayCursorScroll( syswk );
		}
		syswk->app->vfunk.cnt = 0;
//		syswk->app->vfunk.seq = 0;
		return 0;
	}
	GFL_BG_SetScrollReq( GFL_BG_FRAME3_M, GFL_BG_SCROLL_X_DEC, BOX2MAIN_TRAY_SCROLL_SPD );
	BOX2OBJ_TrayPokeIconScroll( syswk, BOX2MAIN_TRAY_SCROLL_SPD );
	BOX2OBJ_BoxNameScroll( syswk->app, BOX2MAIN_TRAY_SCROLL_SPD );

//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_BOXMV_BTN );
//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_RET_BTN );
//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN );
//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_MV_BTN );
	BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );

/*
	{
		u32	r1, r2;
//		r1 = BGWINFRM_MoveCheck( syswk->app->wfrm, BOX2MAIN_WINFRM_CLOSE_BTN );
		r1 = 0;
		BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );
//		r2 = BGWINFRM_MoveCheck( syswk->app->wfrm, BOX2MAIN_WINFRM_CLOSE_BTN );
		r2 = 0;
		if( r1 == 1 && r2 == 0 ){
		}
	}
*/

	syswk->app->vfunk.cnt++;

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : トレイをスクロール（右ボタンが押されたとき）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncTrayScrollRight( BOX2_SYS_WORK * syswk )
{
	if( syswk->app->vfunk.cnt == BOX2MAIN_TRAY_SCROLL_CNT ){
/*
		if( CURSORMOVE_CursorOnOffGet( syswk->app->cmwk ) == TRUE ){
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		}
*/
		if( CURSORMOVE_CursorOnOffGet( syswk->app->cmwk ) == FALSE ){
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		}
		if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == TRUE ){
			BOX2OBJ_EndTrayCursorScroll( syswk );
		}
		syswk->app->vfunk.cnt = 0;
//		syswk->app->vfunk.seq = 0;
		return 0;
	}
	GFL_BG_SetScrollReq( GFL_BG_FRAME3_M, GFL_BG_SCROLL_X_INC, BOX2MAIN_TRAY_SCROLL_SPD );
	BOX2OBJ_TrayPokeIconScroll( syswk, -BOX2MAIN_TRAY_SCROLL_SPD );
	BOX2OBJ_BoxNameScroll( syswk->app, -BOX2MAIN_TRAY_SCROLL_SPD );

//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_BOXMV_BTN );
//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_RET_BTN );
//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN );
//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_MV_BTN );
	BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );

/*
	{
		u32	r1, r2;
//		r1 = BGWINFRM_MoveCheck( syswk->app->wfrm, BOX2MAIN_WINFRM_CLOSE_BTN );
		r1 = 0;
		BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );
//		r2 = BGWINFRM_MoveCheck( syswk->app->wfrm, BOX2MAIN_WINFRM_CLOSE_BTN );
		r2 = 0;
		if( r1 == 1 && r2 == 0 ){
		}
	}
*/

	syswk->app->vfunk.cnt++;

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : ポケモンメニュー動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncPokeMenuMove( BOX2_SYS_WORK * syswk )
{
	if( BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm ) == FALSE ){
		return 0;
	}
	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : メニュー・「てもちポケモン」・「ポケモンいどう」・「もどる」動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
/*
int BOX2MAIN_VFuncPokeFreeMenuMove( BOX2_SYS_WORK * syswk )
{
	BOOL	menu_mv;
	u32		ret[3];

	menu_mv = BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );
//	ret[0]  = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN );
//	ret[1]  = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_MV_BTN );
//	ret[2]  = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_RET_BTN );
	ret[0]  = 0;
	ret[1]  = 0;
	ret[2]  = 0;

	if( menu_mv == FALSE && ret[0] == 0 && ret[1] == 0 && ret[2] == 0 ){
		return 0;
	}
	return 1;
}
*/


//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : マーキングフレーム動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncMarkingFrameMove( BOX2_SYS_WORK * syswk )
{
	return BOX2BGWFRM_MarkingFrameMove( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 「てもちポケモン」「ポケモンいどう」「とじる」「もどる」などのボタン動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
/*
int BOX2MAIN_VFuncUnderButtonMove( BOX2_SYS_WORK * syswk )
{
	u32	ret[4];

//	ret[0] = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN );
//	ret[1] = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_MV_BTN );
//	ret[2] = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_RET_BTN );
//	ret[3] = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_CLOSE_BTN );
	ret[0] = 0;
	ret[1] = 0;
	ret[2] = 0;
	ret[3] = 0;

	if( ret[0] == 0 && ret[1] == 0 && ret[2] == 0 && ret[3] == 0 ){
		return 0;
	}
	return 1;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手持ちポケモンフレーム動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncPartyFrameMove( BOX2_SYS_WORK * syswk )
{
	if( BOX2BGWFRM_PartyPokeFrameMove( syswk ) == FALSE ){
		return 0;
	}
	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手持ちポケモンフレームを右へ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
/*
int BOX2MAIN_VFuncPartyFrmRight( BOX2_SYS_WORK * syswk )
{
	if( BOX2BGWFRM_PartyPokeFrameMove( syswk ) == FALSE ){
		return 0;
	}

	BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );

	return 1;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : ボックス選択ウィンドウイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncBoxMoveFrmIn( BOX2_SYS_WORK * syswk )
{
	int	ret;

	ret = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_MOVE );

	BOX2OBJ_BoxMoveFrmScroll( syswk, 8 );

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : ボックス選択ウィンドウアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncBoxMoveFrmOut( BOX2_SYS_WORK * syswk )
{
	int	ret;

	ret = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_MOVE );

	BOX2OBJ_BoxMoveFrmScroll( syswk, -8 );

	return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 取得ポケモン移動（ポケモンをあずける）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_GetPartyPokeMoveDrop( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	u32	tpx, tpy;
	u32	put_pos;
	BOOL	menu_mv;
	BOOL	party_mv;

	vf = &syswk->app->vfunk;

/*
	// 取得中のアイコンの座標を退避（フレーム動作で座標が変更されるため）
	if( vf->seq <= AZUKERU_SEQ_ICON_GET ){
		BOX2OBJ_GetPos(
			syswk->app, syswk->app->pokeicon_id[syswk->get_pos], &ipx, &ipy, CLSYS_DEFREND_MAIN );
	}
*/

	menu_mv  = BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );
	party_mv = BOX2BGWFRM_PartyPokeFrameMove( syswk );

	switch( vf->seq ){
	case AZUKERU_SEQ_ICON_GET:		// アイコン取得中
/*
		if( syswk->app->party_frm == 1 && party_mv == FALSE ){
			syswk->app->party_frm = 2;
			BOX2BMP_VBlankMsgPut( syswk, BOX2BMPWIN_ID_MSG4 );
		}
*/
		if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == FALSE ){
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		}

		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == FALSE ){
			BOX2OBJ_SetPos(
				syswk->app, syswk->app->pokeicon_id[BOX2OBJ_POKEICON_GET_POS],
				syswk->app->tpx, syswk->app->tpy-8, CLSYS_DEFREND_MAIN );

			put_pos = BOX2MAIN_GETPOS_NONE;
			if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == FALSE && party_mv == FALSE ){
				put_pos = TrayPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy );
			}

			PokeIconDropDataMake( syswk, syswk->get_pos, put_pos );

			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
			syswk->get_pos = PokeIconMoveAfterPosGet( syswk, BOX2OBJ_POKEICON_GET_POS );

			if( syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
				vf->seq = AZUKERU_SEQ_MOVE_ENTER1;
			}else{
				BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
				BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
//				BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );
				vf->seq = AZUKERU_SEQ_MOVE_CANCEL;
			}
			break;
		}else{
			if( menu_mv == FALSE && party_mv == FALSE ){
				if( BOX2BGWFRM_CheckPartyPokeFrameLeft( syswk->app->wfrm ) == TRUE ){
					if( BOX2UI_HitCheckPartyFrameLeft() == FALSE ){
						syswk->get_tray  = BOX2MAIN_GETPOS_NONE;				// 取得したボックス
						syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_TRAY;
						BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
//						BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_PASSIVE );
					}
				}else{
					// トレイスクロール矢印ヒットチェック
					u32 hit = BOX2UI_HitCheckContTrayArrow();
					if( hit == 0 ){
						BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_L );
						vf->seq = AZUKERU_SEQ_SCROLL_L;
					}else if( hit == 1 ){
						BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_R );
						vf->seq = AZUKERU_SEQ_SCROLL_R;
					}
				}
			}
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case AZUKERU_SEQ_SCROLL_L:			// トレイが切り替わっていた場合のスクロール処理（左）
		if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = AZUKERU_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case AZUKERU_SEQ_SCROLL_R:			// トレイが切り替わっていた場合のスクロール処理（右）
		if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = AZUKERU_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case AZUKERU_SEQ_MOVE_CANCEL:		// 移動キャンセル
		if( PokeIconObjMove( syswk ) == FALSE && party_mv == FALSE && menu_mv == FALSE ){
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
			BOX2OBJ_PokeCursorMove( syswk->app, syswk->get_pos );
			BOX2OBJ_PokeCursorVanish( syswk, TRUE );
			vf->seq = AZUKERU_SEQ_END;
		}
		break;

	case AZUKERU_SEQ_MOVE_ENTER1:	// 移動実行１
		if( PokeIconObjDrop( syswk ) == FALSE ){
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
			BOX2MAIN_PokeInfoOff( syswk );
			BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
			syswk->get_pos = BOX2MAIN_GETPOS_NONE;
			vf->seq = AZUKERU_SEQ_MOVE_ENTER2;
		}
		break;

	case AZUKERU_SEQ_MOVE_ENTER2:	// 移動実行２
		if( party_mv == FALSE ){
			vf->seq = AZUKERU_SEQ_END;
		}
		break;

	case AZUKERU_SEQ_END:			// 終了
		BOX2OBJ_TrayMoveArrowVanish( syswk->app, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
//		syswk->app->party_frm = 0;
		vf->seq = 0;
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 取得ポケモン移動（ポケモンをつれていく）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_GetTrayPokeMoveDrop( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	u32	tpx, tpy;
	u32	put_pos;
	BOOL	menu_mv;
	BOOL	party_mv;

	vf = &syswk->app->vfunk;

	menu_mv  = BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );
	party_mv = BOX2BGWFRM_PartyPokeFrameMove( syswk );

	switch( vf->seq ){
	case TSURETEIKU_SEQ_ICON_GET:		// アイコン取得中
/*
		if( syswk->app->party_frm == 1 && party_mv == FALSE ){
			syswk->app->party_frm = 2;
			BOX2BMP_VBlankMsgPut( syswk, BOX2BMPWIN_ID_MSG4 );
		}
*/
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == FALSE ){

			put_pos = BOX2MAIN_GETPOS_NONE;
			if( BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == TRUE ){
				put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaRight );
			}
			PokeIconPartyDropDataMake( syswk, syswk->get_pos, put_pos );
//			PokeIconMoveBoxPartyDataMake( syswk, syswk->get_pos, put_pos );

			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
			syswk->get_pos = PokeIconMoveAfterPosGet( syswk, BOX2OBJ_POKEICON_GET_POS );

/*
			if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				vf->seq = TSURETEIKU_SEQ_MOVE_ENTER1;
			}else{
				if( syswk->app->party_frm != 0 ){
					BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
				}
				vf->seq = TSURETEIKU_SEQ_MOVE_CANCEL1;
			}
*/
			if( syswk->get_pos >= BOX2OBJ_POKEICON_TRAY_MAX ){
				vf->seq = TSURETEIKU_SEQ_MOVE_ENTER1;
			}else{
				if( party_mv == TRUE ||
						BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == TRUE ){
					BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
				}
				vf->seq = TSURETEIKU_SEQ_MOVE_CANCEL1;
			}

		}else{
			if( menu_mv == FALSE && party_mv == FALSE &&
					BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE ){
				if( BOX2UI_HitCheckPosTrayPoke( tpx, tpy ) != syswk->get_pos ){
					BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
					BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
					BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF );
					syswk->app->old_cur_pos = syswk->get_pos;
				}
			}
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	// 移動キャンセル時
	case TSURETEIKU_SEQ_MOVE_CANCEL1:	// 移動キャンセル１
		if( party_mv == FALSE ){
			BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
//			BOX2BGWFRM_SubDispWazaFrmInSet( syswk->app );
			vf->seq = TSURETEIKU_SEQ_MOVE_CANCEL2;
		}

	case TSURETEIKU_SEQ_MOVE_CANCEL2:	// 移動キャンセル２
	case TSURETEIKU_SEQ_MOVE_CANCEL3:	// 移動キャンセル３
		if( PokeIconObjMove( syswk ) == FALSE &&
				vf->seq == TSURETEIKU_SEQ_MOVE_CANCEL3 &&
				menu_mv == FALSE ){
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
			BOX2OBJ_PokeCursorMove( syswk->app, syswk->get_pos );
			BOX2OBJ_PokeCursorVanish( syswk, TRUE );
			vf->seq = TSURETEIKU_SEQ_END;
			break;
		}
		if( vf->seq == TSURETEIKU_SEQ_MOVE_CANCEL2 ){
			vf->seq = TSURETEIKU_SEQ_MOVE_CANCEL3;
		}
		break;

	// 移動実行時
	case TSURETEIKU_SEQ_MOVE_ENTER1:	// 移動実行１
		if( PokeIconObjMove( syswk ) == FALSE ){
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
			syswk->get_pos = BOX2MAIN_GETPOS_NONE;
			BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
			vf->seq = TSURETEIKU_SEQ_MOVE_ENTER2;
		}
		break;

	case TSURETEIKU_SEQ_MOVE_ENTER2:	// 移動実行２
		if( party_mv == FALSE ){
			BOX2MAIN_PokeInfoOff( syswk );
			vf->seq = TSURETEIKU_SEQ_END;
		}
		break;

/*
	case TSURETEIKU_SEQ_MOVE_ENTER3:	// 移動実行３
		vf->seq = TSURETEIKU_SEQ_END;
		break;
*/

	// 終了
	case TSURETEIKU_SEQ_END:			// 終了
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, FALSE );
		BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYPOKE, FALSE );
//		syswk->app->party_frm = 0;
		vf->seq = 0;
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手カーソル移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncCursorMove( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	BOX2MAIN_CURSORMOVE_WORK * cwk;
	s16	x, y;

	vf  = &syswk->app->vfunk;
	cwk = vf->work;

//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_Y_ST_BTN );

	if( cwk->cnt == 0 ){
		BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, cwk->px, cwk->py, CLSYS_DEFREND_MAIN );
		BOX2OBJ_MovePokeIconHand( syswk );
		return 0;
	}
	cwk->cnt--;

	BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, &x, &y, CLSYS_DEFREND_MAIN );

	if( cwk->mx == 0 ){
		x += cwk->vx;
	}else{
		x -= cwk->vx;
	}
	if( cwk->my == 0 ){
		y += cwk->vy;
	}else{
		y -= cwk->vy;
	}
	BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y, CLSYS_DEFREND_MAIN );
	BOX2OBJ_MovePokeIconHand( syswk );

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 戻るを消してメニューを出す
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncMenuMove( BOX2_SYS_WORK * syswk )
{
	if(	BOX2MAIN_VFuncCursorMove( syswk ) == 0 &&
			BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm ) == FALSE ){
		return 0;
	}
	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : メニューを消して戻るを出す
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
/*
int BOX2MAIN_VFuncMenuCloseKey( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	BOOL	menu_mv;
	int		cur_mv;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
		menu_mv = BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );
		cur_mv  = BOX2MAIN_VFuncCursorMove( syswk );
		if( menu_mv == 0 && cur_mv == 0 ){
			vf->seq++;
		}
		break;

	case 1:
//		if( BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_RET_BTN ) == 0 ){
			vf->seq = 0;
			return 0;
//		}
		break;
	}

	return 1;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : メニューを消して戻るを出す（ボックス整理）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
/*
int BOX2MAIN_VFuncMenuCloseKeyArrange( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
		{
			int	r1 = BOX2MAIN_VFuncCursorMove( syswk );
			int	r2 = BOX2MAIN_VFuncPokeMenuMove( syswk );
			if( r1 == 0 && r2 == 0 ){
				vf->seq++;
			}
		}
		break;

	case 1:
		{
//			int	r1 = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_POKE_BTN );
//			int	r2 = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_MV_BTN );
//			int	r3 = BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_RET_BTN );
			int	r1 = 0;
			int	r2 = 0;
			int	r3 = 0;
			if( r1 == 0 && r2 == 0 && r3 == 0 ){
				vf->seq = 0;
				return 0;
			}
		}
	}

	return 1;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンを手で取得した状態にする
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
void BOX2MAIN_HandGetPokeSet( BOX2_SYS_WORK * syswk )
{
	u32	icon;
	s16	x, y;

	BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_CLOSE );
	BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, &x, &y, CLSYS_DEFREND_MAIN );

	icon = syswk->app->pokeicon_id[ BOX2OBJ_POKEICON_GET_POS ];
	BOX2OBJ_SetPos( syswk->app, icon, x, y+4, CLSYS_DEFREND_MAIN );

	BOX2OBJ_PokeIconPriChg( syswk->app, BOX2OBJ_POKEICON_GET_POS, BOX2OBJ_POKEICON_PRI_CHG_GET );
}


//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手カーソルでポケモン取得
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncPokeMoveGetKey( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	s16	x, y;

	vf = &syswk->app->vfunk;

//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_Y_ST_BTN );

	switch( vf->seq ){
	case 0:		// 手カーソルを開く
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_OPEN );
		vf->seq++;
	case 1:		// 手カーソルを下へ
		if( vf->cnt == HANDCURSOR_MOVE_CNT ){
			vf->cnt = 0;
			vf->seq++;
			break;
		}
		BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, &x, &y, CLSYS_DEFREND_MAIN );
		BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y+HANDCURSOR_MOVE_DOT, CLSYS_DEFREND_MAIN );
		vf->cnt++;
		break;

	case 2:		// 手カーソル閉じる
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_CLOSE );
		vf->seq++;
	case 3:		// 手カーソルを上へ
		if( vf->cnt == HANDCURSOR_MOVE_CNT ){
			vf->cnt = 0;
			vf->seq = 0;
			return 0;
		}
		BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, &x, &y, CLSYS_DEFREND_MAIN );
		BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y-HANDCURSOR_MOVE_DOT, CLSYS_DEFREND_MAIN );
		{
			u8	icon = syswk->app->pokeicon_id[ BOX2OBJ_POKEICON_GET_POS ];
			BOX2OBJ_GetPos( syswk->app, icon, &x, &y, CLSYS_DEFREND_MAIN );
			BOX2OBJ_SetPos( syswk->app, icon, x, y-HANDCURSOR_MOVE_DOT, CLSYS_DEFREND_MAIN );
		}
		vf->cnt++;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : ポケモンを放す
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncPokeMovePutKey( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
//	u32	put_pos;

	vf = &syswk->app->vfunk;

//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_Y_ST_BTN );

	switch( vf->seq ){
	case 0:
//		OS_Printf( "get = %d, put = %d\n", syswk->get_pos, syswk->app->poke_put_key );
		if( syswk->app->poke_put_key == BOX2MAIN_GETPOS_NONE &&
				syswk->tray != syswk->get_tray &&
				syswk->get_pos < BOX2OBJ_POKEICON_TRAY_MAX ){
			u32	dir = BOX2MAIN_GetTrayScrollDir( syswk, syswk->tray, syswk->get_tray );
			syswk->tray = syswk->get_tray;
			BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
			BOX2MAIN_WallPaperSet( syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), dir );
			if( dir == BOX2MAIN_TRAY_SCROLL_L ){
				vf->seq = 2;
				break;
			}else{
				vf->seq = 3;
				break;
			}
		}
	case 1:
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_OPEN );

//		put_pos = syswk->app->poke_put_key;
/*
		if( syswk->app->poke_put_key >= BOX2UI_ARRANGE_MOVE_TRAY1 &&
			syswk->app->poke_put_key <= BOX2UI_ARRANGE_MOVE_TRAY6 ){
			put_pos = BOX2MAIN_BOXMOVE_FLG + syswk->app->poke_put_key - BOX2UI_ARRANGE_MOVE_TRAY1;
		}
*/
//		BOX2OBJ_PutPokeIcon( syswk->app, syswk->get_pos );
//		PokeIconMoveBoxPartyDataMake( syswk, syswk->get_pos, put_pos );
		PokeIconMoveBoxPartyDataMake( syswk, syswk->get_pos, syswk->app->poke_put_key );
/*
		if( ( put_pos & BOX2MAIN_BOXMOVE_FLG ) == 0 ){
			syswk->get_pos = PokeIconMoveAfterPosGet( syswk, syswk->get_pos );
		}
*/
		vf->seq = 4;
		break;

	case 2:			// トレイが切り替わっていた場合のスクロール処理（左）
		if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
			vf->seq = 1;
		}
		break;

	case 3:			// トレイが切り替わっていた場合のスクロール処理（右）
		if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
			vf->seq = 1;
		}
		break;

	case 4:			// ポケモンアイコン動作
		if( PokeIconObjMove( syswk ) == FALSE ){
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
			BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
			vf->seq = 0;
			return 0;
		}
	}

	return 1;
}

int BOX2MAIN_VFuncPokeMovePutKey2( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_OPEN );
		PokeIconDropDataMake( syswk, syswk->get_pos, syswk->app->poke_put_key );
		vf->seq = 1;
		break;

	case 1:
		if( PokeIconObjDrop( syswk ) == FALSE ){
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
			BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
			vf->seq = 0;
			return 0;
		}
	}

	return 1;
}



//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテム取得（持ち物整理・キー操作）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncItemArrangeGetKey( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
		if( syswk->app->get_item != ITEM_DUMMY_DATA ){
			BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
			BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
		}
		vf->seq++;

	case 1:
		{
			int		cur  = BOX2MAIN_VFuncCursorMove( syswk );
			BOOL	menu = BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );
			if( cur == 0 && menu == FALSE ){
				if( syswk->app->get_item != ITEM_DUMMY_DATA ){
					BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
					BOX2OBJ_ItemIconCursorOn( syswk->app );
				}
				vf->seq = 0;
				return 0;
			}
		}
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテム取得（持ち物整理・タッチ操作）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
enum {
	ITEMGET_SEQ_INIT = 0,					// 初期設定
	ITEMGET_SEQ_GETANM,						// アイテムアイコン取得アニメ
	ITEMGET_SEQ_MAIN,							// メイン
	ITEMGET_SEQ_SCROLL_L,					// トレイスクロール（左）
	ITEMGET_SEQ_SCROLL_R,					// トレイスクロール（右）
	ITEMGET_SEQ_CANCEL_SCROLL_L,	// トレイキャンセルスクロール（左）
	ITEMGET_SEQ_CANCEL_SCROLL_R,	// トレイキャンセルスクロール（右）
	ITEMGET_SEQ_PUT_MOVE,					// アイテムアイコン配置移動
	ITEMGET_SEQ_MOVE,							// アイテムアイコン移動
	ITEMGET_SEQ_PUTANM,						// 縮小アニメセット
	ITEMGET_SEQ_PUTANM_WAIT,			// 縮小アニメ待ち
	ITEMGET_SEQ_END,							// 終了
};

int BOX2MAIN_VFuncItemGetTouch( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	BOOL	menu;
	BOOL	party;
	u32	tpx, tpy;

	vf = &syswk->app->vfunk;

	menu  = BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );
	party = BOX2BGWFRM_PartyPokeFrameMove( syswk );

	switch( vf->seq ){
	case ITEMGET_SEQ_INIT:							// 初期設定
		BOX2SEQ_ItemModeMenuSet( syswk, syswk->app->get_item );
		if( syswk->app->get_item != ITEM_DUMMY_DATA ){
			BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
			BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
			vf->seq = ITEMGET_SEQ_GETANM;
		}else{
			BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
			vf->seq = ITEMGET_SEQ_END;
		}
		break;

	case ITEMGET_SEQ_GETANM:						// アイテムアイコン取得アニメ
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
			break;
		}
		BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
		BOX2OBJ_ItemIconCursorOn( syswk->app );
		vf->seq = ITEMGET_SEQ_MAIN;

	case ITEMGET_SEQ_MAIN:							// メイン
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == FALSE ){
			u16	put_pos;
			u16	set_pos;
			BOOL	cancel;
			BOOL	mv_mode;

			if( party == TRUE || BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == TRUE ){
				mv_mode = TRUE;
			}else{
				mv_mode = FALSE;
			}

			put_pos = TrayPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy );
			if( party == FALSE && BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == TRUE ){
				if( put_pos == BOX2MAIN_GETPOS_NONE ){
					put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaRight );
				}
			}
			set_pos = put_pos;
			cancel  = FALSE;
			if( put_pos == BOX2MAIN_GETPOS_NONE ){
				put_pos = syswk->get_pos;
				cancel  = TRUE;
			}else if( BOX2MAIN_PokeItemMoveCheck( syswk, syswk->get_pos, put_pos ) == FALSE ){
				put_pos = syswk->get_pos;
				cancel  = TRUE;
			}

			if( mv_mode == TRUE ){
				ItemIconMoveMake( syswk, set_pos, put_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
				// キャンセル時のスクロールへ
				if( cancel == TRUE && syswk->get_tray != BOX2MAIN_GETPOS_NONE && syswk->get_tray != syswk->tray ){
					u32	dir = BOX2MAIN_GetTrayScrollDir( syswk, syswk->tray, syswk->get_tray );
					syswk->tray = syswk->get_tray;
					BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
					BOX2MAIN_WallPaperSet( syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), dir );
					if( dir == BOX2MAIN_TRAY_SCROLL_L ){
						vf->seq = ITEMGET_SEQ_CANCEL_SCROLL_L;
					}else{
						vf->seq = ITEMGET_SEQ_CANCEL_SCROLL_R;
					}
				}else{
					vf->seq = ITEMGET_SEQ_PUT_MOVE;
				}
			}else{
				ItemIconMoveMake( syswk, set_pos, put_pos, BOX2MAIN_POKEMOVE_MODE_TRAY );
				BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
				vf->seq = ITEMGET_SEQ_MOVE;
			}

		}else{
			// 手持ちフレーム表示チェック
			if( menu == FALSE && party == FALSE &&
					BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE ){
				if( BOX2UI_HitCheckPosTrayPoke( tpx, tpy ) != syswk->get_pos ){
					BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
					BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
					BOX2OBJ_PartyPokeIconFrmInSet( syswk );
					BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_PARTYITEM, TRUE );
					// 戻り先のシーケンスを「いどうする」の処理にする
					BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
					CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
					CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_STATUS );
					BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
					BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
					syswk->app->msg_put = 0;
					syswk->app->old_cur_pos = syswk->get_pos;
					syswk->app->vnext_seq = BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_ICONMOVE_POKEADD;
				}
			}
			// トレイスクロール矢印ヒットチェック
			{
				u32	hit = BOX2UI_HitCheckContTrayArrow();
				if( hit == 0 ){
					BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_L );
					vf->seq = ITEMGET_SEQ_SCROLL_L;
				}else if( hit == 1 ){
					BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_R );
					vf->seq = ITEMGET_SEQ_SCROLL_R;
				}
			}
			MoveGetItemIcon( syswk->app, tpx, tpy );
		}
		break;

	case ITEMGET_SEQ_SCROLL_L:					// トレイスクロール（左）
		if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = ITEMGET_SEQ_MAIN;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetItemIcon( syswk->app, tpx, tpy );
		}
		break;

	case ITEMGET_SEQ_SCROLL_R:					// トレイスクロール（右）
		if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = ITEMGET_SEQ_MAIN;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetItemIcon( syswk->app, tpx, tpy );
		}
		break;

	case ITEMGET_SEQ_CANCEL_SCROLL_L:		// トレイキャンセルスクロール（左）
	case ITEMGET_SEQ_CANCEL_SCROLL_R:		// トレイキャンセルスクロール（右）
		if( vf->seq == ITEMGET_SEQ_CANCEL_SCROLL_L ){
			if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
				vf->seq = ITEMGET_SEQ_PUT_MOVE;
			}
		}
		if( vf->seq == ITEMGET_SEQ_CANCEL_SCROLL_R ){
			if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
				vf->seq = ITEMGET_SEQ_PUT_MOVE;
			}
		}
	case ITEMGET_SEQ_PUT_MOVE:					// アイテムアイコン配置移動
	case ITEMGET_SEQ_MOVE:							// アイテムアイコン移動
		if( ItemIconMoveMain( syswk ) == FALSE ){
			if( vf->seq == ITEMGET_SEQ_PUT_MOVE ){
				vf->seq = ITEMGET_SEQ_PUTANM;
			}else if( vf->seq == ITEMGET_SEQ_MOVE ){
				if( syswk->app->get_item != ITEM_DUMMY_DATA ){
					BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
					BOX2OBJ_ItemIconCursorOn( syswk->app );
				}
				BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
				vf->seq = ITEMGET_SEQ_END;
			}
		}
		break;

	case ITEMGET_SEQ_PUTANM:						// 縮小アニメセット
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		vf->seq = ITEMGET_SEQ_PUTANM_WAIT;
		break;

	case ITEMGET_SEQ_PUTANM_WAIT:				// 縮小アニメ待ち
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
			BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
			vf->seq = ITEMGET_SEQ_END;
		}
		break;

	case ITEMGET_SEQ_END:								// 終了
		if( menu == FALSE && party == FALSE ){
			vf->seq = 0;
			return 0;
		}
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : メニューを消して戻るを出す（持ち物整理）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncItemArrangeMenuClose( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
//		if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		if( BOX2OBJ_VanishCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
			BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
			BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		}
		vf->seq++;

	case 1:
		{
			int	r1 = BOX2MAIN_VFuncCursorMove( syswk );
			int	r2 = BOX2MAIN_VFuncPokeMenuMove( syswk );
			if( r1 == 0 && r2 == 0 ){
				vf->seq++;
			}
		}
		break;

	case 2:
//		if( syswk->app->get_item != ITEM_DUMMY_DATA ){
		if( BOX2OBJ_VanishCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
			BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
		}
		vf->seq = 0;
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテムアイコン入れ替え動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
/*
int BOX2MAIN_VFuncItemIconChange( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
			break;
		}
		{
			BOX2MAIN_ITEMMOVE_WORK * mvwk;
			u16	put_pos;
			u16	set_pos;

			mvwk = vf->work;
			put_pos = syswk->get_pos;
			set_pos = syswk->get_pos;
			syswk->get_pos = mvwk->put_pos;

			ItemIconMoveMake( syswk, set_pos, put_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
		}
		vf->seq = 1;
		break;

	case 1:
		if( ItemIconMoveMain( syswk ) == FALSE ){
			vf->seq = 0;
			return 0;
		}
		break;
	}

	return 1;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 持ち物整理・トレイ/手持ち取得（タッチ）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
enum {
	ITEMMOVE_SEQ_ICONANM_SET = 0,		// 拡大アニメセット
	ITEMMOVE_SEQ_ICONANM_WAIT,				// 拡大アニメ待ち
	ITEMMOVE_SEQ_ICON_GET,						// アイコン取得中
	ITEMMOVE_SEQ_SCROLL_L,						// トレイスクロール（左）
	ITEMMOVE_SEQ_SCROLL_R,						// トレイスクロール（右）
	ITEMMOVE_SEQ_CANCEL_SCROLL_L,		// トレイが切り替わっていた場合のスクロール処理（左）
	ITEMMOVE_SEQ_CANCEL_SCROLL_R,		// トレイが切り替わっていた場合のスクロール処理（右）
	ITEMMOVE_SEQ_ICON_PUT,						// アイコン配置
	ITEMMOVE_SEQ_ICONANM_END_SET,		// 縮小アニメセット
	ITEMMOVE_SEQ_ICONANM_END_WAIT,		// 縮小アニメ待ち
	ITEMMOVE_SEQ_END,								// 終了
};

static void MoveGetItemIcon( BOX2_APP_WORK * appwk, u32 x, u32 y )
{
	BOX2OBJ_ItemIconPosSet( appwk, x, y );
	BOX2OBJ_ItemIconCursorMove( appwk );
	appwk->tpx = x;
	appwk->tpy = y;
}

int BOX2MAIN_VFuncItemMoveTouch( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
//	BOOL	menu;
//	int	r1, r2, r3;
	u32	tpx, tpy;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case ITEMMOVE_SEQ_ICONANM_SET:				// 拡大アニメセット
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
		vf->seq = ITEMMOVE_SEQ_ICONANM_WAIT;
		break;

	case ITEMMOVE_SEQ_ICONANM_WAIT:			// 拡大アニメ待ち
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
			BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
			BOX2OBJ_ItemIconCursorOn( syswk->app );
			vf->seq = ITEMMOVE_SEQ_ICON_GET;
		}
		break;

	case ITEMMOVE_SEQ_ICON_GET:					// アイコン取得中
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == FALSE ){
			u16	put_pos;
			u16	set_pos;
			BOOL	cancel;

			put_pos = TrayPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy );
			if( put_pos == BOX2MAIN_GETPOS_NONE ){
				put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaRight );
			}
			set_pos = put_pos;
			cancel  = FALSE;
			if( put_pos == BOX2MAIN_GETPOS_NONE ){
				put_pos = syswk->get_pos;
				cancel  = TRUE;
			}else if( BOX2MAIN_PokeItemMoveCheck( syswk, syswk->get_pos, put_pos ) == FALSE ){
				put_pos = syswk->get_pos;
				cancel  = TRUE;
			}
			ItemIconMoveMake( syswk, set_pos, put_pos, BOX2MAIN_POKEMOVE_MODE_ALL );

			// キャンセル時のスクロールへ
			if( cancel == TRUE && syswk->get_tray != BOX2MAIN_GETPOS_NONE && syswk->get_tray != syswk->tray ){
				u32	dir = BOX2MAIN_GetTrayScrollDir( syswk, syswk->tray, syswk->get_tray );
				syswk->tray = syswk->get_tray;
				BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
				BOX2MAIN_WallPaperSet( syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), dir );
				if( dir == BOX2MAIN_TRAY_SCROLL_L ){
					vf->seq = ITEMMOVE_SEQ_CANCEL_SCROLL_L;
				}else{
					vf->seq = ITEMMOVE_SEQ_CANCEL_SCROLL_R;
				}
			}else{
				vf->seq = ITEMMOVE_SEQ_ICON_PUT;
			}
		}else{
/*
			// ボックス移動フレーム表示チェック
			if( ret1 == 0 && ret2 == 0 &&
					BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == FALSE &&
					BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE ){
				if( BOX2UI_HitCheckPosTrayPoke( tpx, tpy ) != syswk->get_pos ){
					syswk->box_mv_pos = syswk->tray;	// 現在参照しているボックス
					BOX2MAIN_InitBoxMoveFrameScroll( syswk );
					// 戻り先のシーケンスを「つかむ」の処理にする
					BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_MOVE, syswk->get_pos );
					BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
					BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_PASSIVE );
					syswk->app->old_cur_pos = syswk->get_pos;
					syswk->app->vnext_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE;
				}
			}
*/
			// トレイスクロール矢印ヒットチェック
			{
				u32	hit = BOX2UI_HitCheckContTrayArrow();
				if( hit == 0 ){
					BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_L );
					vf->seq = ITEMMOVE_SEQ_SCROLL_L;
				}else if( hit == 1 ){
					BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_R );
					vf->seq = ITEMMOVE_SEQ_SCROLL_R;
				}
			}
			MoveGetItemIcon( syswk->app, tpx, tpy );
		}
		break;

	case ITEMMOVE_SEQ_SCROLL_L:					// トレイスクロール（左）
		if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = ITEMMOVE_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetItemIcon( syswk->app, tpx, tpy );
		}
		break;

	case ITEMMOVE_SEQ_SCROLL_R:					// トレイスクロール（右）
		if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = ITEMMOVE_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetItemIcon( syswk->app, tpx, tpy );
		}
		break;

	case ITEMMOVE_SEQ_CANCEL_SCROLL_L:		// トレイが切り替わっていた場合のスクロール処理（左）
	case ITEMMOVE_SEQ_CANCEL_SCROLL_R:		// トレイが切り替わっていた場合のスクロール処理（右）
		if( vf->seq == ITEMMOVE_SEQ_CANCEL_SCROLL_L ){
			if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
				vf->seq = ITEMMOVE_SEQ_ICON_PUT;
			}
		}
		if( vf->seq == ITEMMOVE_SEQ_CANCEL_SCROLL_R ){
			if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
				vf->seq = ITEMMOVE_SEQ_ICON_PUT;
			}
		}
	case ITEMMOVE_SEQ_ICON_PUT:					// アイコン配置
		if( ItemIconMoveMain( syswk ) == FALSE ){
			if( vf->seq == ITEMMOVE_SEQ_ICON_PUT ){
				PMSND_PlaySE( SE_BOX2_POKE_PUT );
				vf->seq = ITEMMOVE_SEQ_ICONANM_END_SET;
			}
		}
		break;

	case ITEMMOVE_SEQ_ICONANM_END_SET:		// 縮小アニメセット
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		vf->seq = ITEMMOVE_SEQ_ICONANM_END_WAIT;
		break;

	case ITEMMOVE_SEQ_ICONANM_END_WAIT:	// 縮小アニメ待ち
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
			BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
			vf->seq = ITEMMOVE_SEQ_END;
		}
		break;

	case ITEMMOVE_SEQ_END:								// 終了
		vf->seq = 0;
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 持ち物整理・トレイ/手持ち入れ替え（タッチ）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncItemIconChgTouch( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
		vf->seq = 1;
		break;

	case 1:
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
			BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
			vf->seq = 2;
		}
		break;

	case 2:
		{
			BOX2MAIN_ITEMMOVE_WORK * mvwk;
			u16	put_pos;
			u16	set_pos;

			mvwk = vf->work;
			put_pos = syswk->get_pos;
			set_pos = syswk->get_pos;
			syswk->get_pos = mvwk->put_pos;

			ItemIconMoveMake( syswk, set_pos, put_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
		}
		vf->seq = 3;
		break;

	case 3:
		if( ItemIconMoveMain( syswk ) == FALSE ){
			PMSND_PlaySE( SE_BOX2_POKE_PUT );
			vf->seq = 4;
		}
		break;

	case 4:
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		vf->seq = 5;
		break;

	case 5:
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
			BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
			vf->seq = 6;
		}
		break;

	case 6:
		vf->seq = 0;
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手カーソルでアイテム取得
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncItemArrangeBoxPartyIconGetKey( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	s16	x, y;

	vf = &syswk->app->vfunk;

//	BGWINFRM_MoveOne( syswk->app->wfrm, BOX2MAIN_WINFRM_Y_ST_BTN );

	switch( vf->seq ){
	case 0:		// アイテムアイコン表示
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
		BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
		vf->seq++;
		break;

	case 1:
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
			BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
			vf->seq++;
		}
		break;

	case 2:		// 手カーソルを開く
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_OPEN );
		vf->seq++;
	case 3:		// 手カーソルを下へ
		if( vf->cnt == HANDCURSOR_MOVE_CNT ){
			vf->cnt = 0;
			vf->seq++;
			break;
		}
		BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, &x, &y, CLSYS_DEFREND_MAIN );
		BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y+HANDCURSOR_MOVE_DOT, CLSYS_DEFREND_MAIN );
		vf->cnt++;
		break;

	case 4:		// 手カーソル閉じる
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_CLOSE );
		vf->seq++;
	case 5:		// 手カーソルを上へ
		if( vf->cnt == HANDCURSOR_MOVE_CNT ){
			vf->cnt = 0;
			vf->seq = 0;
			return 0;
		}
		BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, &x, &y, CLSYS_DEFREND_MAIN );
		BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_HAND_CURSOR, x, y-HANDCURSOR_MOVE_DOT, CLSYS_DEFREND_MAIN );
		{
			BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_ITEMICON, &x, &y, CLSYS_DEFREND_MAIN );
			BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_ITEMICON, x, y-HANDCURSOR_MOVE_DOT, CLSYS_DEFREND_MAIN );
		}
		vf->cnt++;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテムを放す
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncItemIconPutKey( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	s16	x, y;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_OPEN );
		vf->seq = 1;

	case 1:
		if( vf->cnt == HANDCURSOR_MOVE_CNT ){
			vf->cnt = 0;
			vf->seq = 2;
		}else{
			BOX2OBJ_GetPos( syswk->app, BOX2OBJ_ID_ITEMICON, &x, &y, CLSYS_DEFREND_MAIN );
			BOX2OBJ_SetPos( syswk->app, BOX2OBJ_ID_ITEMICON, x, y+HANDCURSOR_MOVE_DOT, CLSYS_DEFREND_MAIN );
			vf->cnt++;
		}
		break;

	case 2:
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		vf->seq = 3;
		break;

	case 3:
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
			BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
			vf->seq = 4;
		}
		break;

	case 4:
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
		vf->seq = 0;
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテムを放す・キャンセル
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncItemIconPutKeyCancel( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	switch( vf->seq ){
	case 0:
		if( syswk->get_tray != BOX2MAIN_GETPOS_NONE && syswk->get_tray != syswk->tray ){
			u32	dir = BOX2MAIN_GetTrayScrollDir( syswk, syswk->tray, syswk->get_tray );
			syswk->tray = syswk->get_tray;
			BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
			BOX2MAIN_WallPaperSet( syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), dir );
			if( dir == BOX2MAIN_TRAY_SCROLL_L ){
				vf->seq = 2;
				break;
			}else{
				vf->seq = 3;
				break;
			}
		}

	case 1:
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_OPEN );
		ItemIconMoveMakeHand( syswk, syswk->app->get_item_init_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
		vf->seq = 4;
		break;

	case 2:			// トレイが切り替わっていた場合のスクロール処理（左）
		if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
			vf->seq = 1;
		}
		break;

	case 3:			// トレイが切り替わっていた場合のスクロール処理（右）
		if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
			vf->seq = 1;
		}
		break;

	case 4:
		if( ItemIconMoveMainHand( syswk ) == FALSE ){
			vf->seq = 5;
		}
		break;

	case 5:
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		vf->seq = 6;
		break;

	case 6:
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
			BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
			vf->seq = 7;
		}
		break;

	case 7:
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_HAND_CURSOR, BOX2OBJ_ANM_HAND_NORMAL );
		vf->seq = 0;
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手持ちポケモンのアイテム取得（持ち物整理・タッチ操作）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncItemPartyGetTouch( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	BOOL	menu;
	BOOL	party;
	u32	tpx, tpy;

	vf = &syswk->app->vfunk;

	menu  = BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );
	party = BOX2BGWFRM_PartyPokeFrameMove( syswk );

	switch( vf->seq ){
	case ITEMGET_SEQ_INIT:						// 初期設定
		BOX2SEQ_ItemModeMenuSet( syswk, syswk->app->get_item );
		if( syswk->app->get_item != ITEM_DUMMY_DATA ){
			BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
			BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 1 );
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, TRUE );
			vf->seq = ITEMGET_SEQ_GETANM;
		}else{
			BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
			vf->seq = ITEMGET_SEQ_END;
		}
		break;

	case ITEMGET_SEQ_GETANM:					// アイテムアイコン取得アニメ
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == TRUE ){
			break;
		}
		BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
		BOX2OBJ_ItemIconCursorOn( syswk->app );
		vf->seq = ITEMGET_SEQ_MAIN;

	case ITEMGET_SEQ_MAIN:						// メイン
		if( BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == TRUE ){
			BOX2OBJ_TrayMoveArrowVanish( syswk->app, TRUE );
		}

		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == FALSE ){
			u16	put_pos;
			u16	set_pos;
			BOOL	cancel;
			BOOL	mv_mode;

			if( party == TRUE || BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == TRUE ){
				mv_mode = TRUE;
			}else{
				mv_mode = FALSE;
			}

			if( party == FALSE && BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == TRUE ){
				put_pos = TrayPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy );
				if( put_pos == BOX2MAIN_GETPOS_NONE ){
					put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaRight );
				}
			}else{
				put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaLeft );
			}
			set_pos = put_pos;
			cancel  = FALSE;
			if( put_pos == BOX2MAIN_GETPOS_NONE ){
				put_pos = syswk->get_pos;
				cancel  = TRUE;
			}else if( BOX2MAIN_PokeItemMoveCheck( syswk, syswk->get_pos, put_pos ) == FALSE ){
				put_pos = syswk->get_pos;
				cancel  = TRUE;
			}

			if( mv_mode == TRUE ){
				ItemIconMoveMake( syswk, set_pos, put_pos, BOX2MAIN_POKEMOVE_MODE_ALL );
				vf->seq = ITEMGET_SEQ_PUT_MOVE;
			}else{
				ItemIconMoveMake( syswk, set_pos, put_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
				if( put_pos == syswk->get_pos &&
						( ITEM_CheckMail(syswk->app->get_item) == FALSE || set_pos == syswk->get_pos ) ){
					BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
				}
				vf->seq = ITEMGET_SEQ_MOVE;
			}

/*
			u16	put_pos = BOX2MAIN_GETPOS_NONE;
			u16	set_pos;

			if( syswk->app->party_frm == 2 ){
				put_pos = TrayPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy );
			}else{
				put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaLeft );
			}

			set_pos = put_pos;

			if( put_pos == BOX2MAIN_GETPOS_NONE ){
				put_pos = syswk->get_pos;
			}else if( BOX2MAIN_PokeItemMoveCheck( syswk, syswk->get_pos, put_pos ) == FALSE ){
				put_pos = syswk->get_pos;
			}
			if( put_pos == syswk->get_pos ){
				BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
				if( syswk->app->party_frm != 0 ){
					BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
					BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
				}
			}else{
				BOX2BGWFRM_SubDispItemFrmOutSet( syswk->app->wfrm );
			}

			ItemIconMoveMake( syswk, set_pos, put_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
			vf->seq = 3;
			break;
*/
		}else{
			// 手持ちフレーム表示チェック
			if( menu == FALSE && party == FALSE &&
					BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE ){
				if( BOX2UI_HitCheckPartyFrameLeft() == FALSE ){
					BOX2BGWFRM_PartyPokeFrameRightMoveSet( syswk->app->wfrm );
					BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYITEM, TRUE );
					// 戻り先のシーケンスを「いどうする」の処理にする
					BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
					CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_BOXLIST );
					CURSORMOVE_MoveTableBitOff( syswk->app->cmwk, BOX2UI_ARRANGE_PTGT_STATUS );
					BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
					BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_OFF );
//					syswk->app->msg_put = 0;
					syswk->app->old_cur_pos = syswk->get_pos;
					syswk->app->vnext_seq = BOX2SEQ_MAINSEQ_ITEM_BOXPARTY_ICONMOVE_POKEADD;
/*
			if( menu_mv == FALSE && syswk->app->party_frm == 0 ){
				if( tpx <  ( BOX2BGWFRM_PARTYPOKE_LX * 8 ) ||
					tpy <  ( BOX2BGWFRM_PARTYPOKE_PY * 8 ) ||
					tpx >= ( ( BOX2BGWFRM_PARTYPOKE_LX + BOX2BGWFRM_PARTYPOKE_SX ) * 8 ) ){
					syswk->app->party_frm = 1;
					BOX2BGWFRM_PartyPokeFrameOutSet( syswk->app->wfrm );
				}
			}
*/
				}
			}
			// トレイスクロール矢印ヒットチェック
			{
				u32	hit = BOX2UI_HitCheckContTrayArrow();
				if( hit == 0 ){
					BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_L );
					vf->seq = ITEMGET_SEQ_SCROLL_L;
				}else if( hit == 1 ){
					BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_R );
					vf->seq = ITEMGET_SEQ_SCROLL_R;
				}
			}
			MoveGetItemIcon( syswk->app, tpx, tpy );
		}
		break;

	case ITEMGET_SEQ_SCROLL_L:					// トレイスクロール（左）
		if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = ITEMGET_SEQ_MAIN;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetItemIcon( syswk->app, tpx, tpy );
		}
		break;

	case ITEMGET_SEQ_SCROLL_R:					// トレイスクロール（右）
		if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = ITEMGET_SEQ_MAIN;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetItemIcon( syswk->app, tpx, tpy );
		}
		break;

	case ITEMGET_SEQ_PUT_MOVE:				// アイテムアイコン配置移動
	case ITEMGET_SEQ_MOVE:						// アイテムアイコン移動
		if( ItemIconMoveMain( syswk ) == FALSE ){
			if( vf->seq == ITEMGET_SEQ_PUT_MOVE ){
				vf->seq = ITEMGET_SEQ_PUTANM;
			}else if( vf->seq == ITEMGET_SEQ_MOVE ){
				if( syswk->app->get_item != ITEM_DUMMY_DATA ){
					BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
					BOX2OBJ_ItemIconCursorOn( syswk->app );
				}
				BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_ON, BOX2OBJ_TB_ICON_OFF );
				vf->seq = ITEMGET_SEQ_END;
			}
		}
		break;

	case ITEMGET_SEQ_PUTANM:						// 縮小アニメセット
		BOX2OBJ_ItemIconAffineSet( syswk->app, TRUE );
		BOX2OBJ_AnmSet( syswk->app, BOX2OBJ_ID_ITEMICON, 2 );
		BOX2OBJ_PokeCursorVanish( syswk, FALSE );
		vf->seq = ITEMGET_SEQ_PUTANM_WAIT;
		break;

	case ITEMGET_SEQ_PUTANM_WAIT:				// 縮小アニメ待ち
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_ITEMICON, FALSE );
			BOX2OBJ_ItemIconAffineSet( syswk->app, FALSE );
			vf->seq = ITEMGET_SEQ_END;
		}
		break;

	case ITEMGET_SEQ_END:							// 終了
		if( menu == FALSE && party == FALSE ){
			vf->seq = 0;
			return 0;
		}
	}

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテムアイコン入れ替え動作（手持ちポケモン）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncItemIconPartyChange( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
//	BOOL	party_mv;

	vf = &syswk->app->vfunk;

//	party_mv = BOX2BGWFRM_PartyPokeFrameMove( syswk );

	switch( vf->seq ){
	case 0:
		if( BOX2OBJ_AnmCheck( syswk->app, BOX2OBJ_ID_ITEMICON ) == FALSE ){
			BOX2MAIN_ITEMMOVE_WORK * mvwk;
			u16	put_pos;
			u16	set_pos;

			mvwk = vf->work;
			put_pos = syswk->get_pos;
			set_pos = syswk->get_pos;
			syswk->get_pos = mvwk->put_pos;

			ItemIconMoveMake( syswk, set_pos, put_pos, BOX2MAIN_POKEMOVE_MODE_PARTY );
/*
			if( syswk->app->party_frm != 0 ){
				syswk->app->party_frm = 0;
				BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
				BOX2OBJ_PokeIconBlendSetAll( syswk, BOX2OBJ_BLENDTYPE_TRAYPOKE, TRUE );
			}
*/
			vf->seq = 1;
		}
		break;

	case 1:
//		if( ItemIconMoveMain( syswk ) == FALSE && party_mv == FALSE ){
		if( ItemIconMoveMain( syswk ) == FALSE ){
			vf->seq = 0;
			return 0;
		}
		break;
	}

	return 1;
}


//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : トレイアイコンスクロール（上）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//-------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncTrayIconScrollUp( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	if( vf->cnt == BOX2MAIN_TRAYICON_SCROLL_CNT ){
		u8 pos = CURSORMOVE_PosGet( syswk->app->cmwk );
		if( pos >= BOX2UI_ARRANGE_PGT_TRAY2 && pos <= BOX2UI_ARRANGE_PGT_TRAY5 ){
			BOX2OBJ_ChangeTrayName( syswk, pos-BOX2UI_ARRANGE_PGT_TRAY2, TRUE );
		}else{
			BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		}
		BOX2OBJ_EndTrayCursorScroll( syswk );
		vf->cnt = 0;
		return 0;
	}
	vf->cnt++;
	BOX2OBJ_TrayIconScroll( syswk, -vf->cnt );

	return 1;
}

int BOX2MAIN_VFuncTrayIconScrollDown( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	if( vf->cnt == BOX2MAIN_TRAYICON_SCROLL_CNT ){
		u8 pos = CURSORMOVE_PosGet( syswk->app->cmwk );
		if( pos >= BOX2UI_ARRANGE_PGT_TRAY2 && pos <= BOX2UI_ARRANGE_PGT_TRAY5 ){
			BOX2OBJ_ChangeTrayName( syswk, pos-BOX2UI_ARRANGE_PGT_TRAY2, TRUE );
		}else{
			BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		}
		BOX2OBJ_EndTrayCursorScroll( syswk );
		vf->cnt = 0;
		return 0;
	}
	vf->cnt++;
	BOX2OBJ_TrayIconScroll( syswk, vf->cnt );

	return 1;
}

int BOX2MAIN_VFuncTrayIconScrollUpJump( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	if( vf->cnt == BOX2MAIN_TRAYICON_SCROLL_CNT ){
		u8 pos = CURSORMOVE_PosGet( syswk->app->cmwk );
		if( pos >= BOX2UI_BOXJUMP_TRAY2 && pos <= BOX2UI_BOXJUMP_TRAY5 ){
			BOX2OBJ_ChangeTrayName( syswk, pos-BOX2UI_BOXJUMP_TRAY2, TRUE );
		}else{
			BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		}
		BOX2OBJ_EndTrayCursorScroll( syswk );
		vf->cnt = 0;
		return 0;
	}
	vf->cnt++;
	BOX2OBJ_TrayIconScroll( syswk, -vf->cnt );

	return 1;
}

int BOX2MAIN_VFuncTrayIconScrollDownJump( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;

	vf = &syswk->app->vfunk;

	if( vf->cnt == BOX2MAIN_TRAYICON_SCROLL_CNT ){
		u8 pos = CURSORMOVE_PosGet( syswk->app->cmwk );
		if( pos >= BOX2UI_BOXJUMP_TRAY2 && pos <= BOX2UI_BOXJUMP_TRAY5 ){
			BOX2OBJ_ChangeTrayName( syswk, pos-BOX2UI_BOXJUMP_TRAY2, TRUE );
		}else{
			BOX2OBJ_ChangeTrayName( syswk, 0, FALSE );
		}
		BOX2OBJ_EndTrayCursorScroll( syswk );
		vf->cnt = 0;
		return 0;
	}
	vf->cnt++;
	BOX2OBJ_TrayIconScroll( syswk, vf->cnt );

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 取得ポケモン移動（バトルボックス・メインから）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
int BOX2MAIN_VFuncGetPokeMoveBattleBoxMain( BOX2_SYS_WORK * syswk )
{
	BOX2_IRQWK * vf;
	u32	tpx, tpy;
	u32	put_pos;
	u32	ret1, ret2;
	u32	hit;

	vf = &syswk->app->vfunk;

	if( BGWINFRM_MoveCheck( syswk->app->wfrm, BOX2MAIN_WINFRM_PARTY ) == 1 ){
		ret1 = BOX2MAIN_VFuncPartyFrameMove( syswk );
	}else{
		ret1 = 0;
	}
	ret2 = BOX2BGWFRM_PokeMenuMoveMain( syswk->app->wfrm );

	switch( vf->seq ){
	case SEIRI_SEQ_ICON_GET:		// アイコン取得中

		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == FALSE ){

			put_pos = BOX2MAIN_GETPOS_NONE;

			// 手持ちポケモン
			if( BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == TRUE ){
				put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaRight );
			}
			// 参照中のトレイ
			if( put_pos == BOX2MAIN_GETPOS_NONE ){
				put_pos = TrayPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy );
			}
/*
			if( put_pos == BOX2MAIN_GETPOS_NONE ){
				put_pos = PartyPokePutAreaCheck( syswk->app->tpx, syswk->app->tpy, PartyPokeAreaRight );
			}
*/
			PokeIconMoveBoxPartyDataMake( syswk, syswk->get_pos, put_pos );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );

			// 移動後のカーソル位置補正
			if( put_pos != BOX2MAIN_GETPOS_NONE ){
				syswk->get_pos = PokeIconMoveAfterPosGet( syswk, BOX2OBJ_POKEICON_GET_POS );
			}

			// メニュー表示開始
			if( BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE && ret1 == 0 ){
				BOX2BGWFRM_PokeMenuInSet( syswk->app->wfrm );
			}

			if( put_pos == BOX2MAIN_GETPOS_NONE && syswk->tray != syswk->get_tray ){
				u32	dir = BOX2MAIN_GetTrayScrollDir( syswk, syswk->tray, syswk->get_tray );
				syswk->tray = syswk->get_tray;
				BOX2OBJ_PokeIconBufLoad( syswk, syswk->tray );
				BOX2MAIN_WallPaperSet( syswk, BOX2MAIN_GetWallPaperNumber(syswk,syswk->tray), dir );
				if( dir == BOX2MAIN_TRAY_SCROLL_L ){
					vf->seq = SEIRI_SEQ_CANCEL_SCROLL_L;
				}else{
					vf->seq = SEIRI_SEQ_CANCEL_SCROLL_R;
				}
			}else{
				vf->seq = SEIRI_SEQ_ICON_PUT;
			}

		}else{
			// パーティフレーム表示チェック
			if( ret1 == 0 && ret2 == 0 &&
					BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE ){
				if( BOX2UI_HitCheckPosTrayPoke( tpx, tpy ) != syswk->get_pos ){
					syswk->move_mode = BOX2MAIN_POKEMOVE_MODE_ALL;
					BOX2BGWFRM_PartyPokeFrameInitPutRight( syswk->app->wfrm );
					BOX2BGWFRM_PartyPokeFrameInSet( syswk->app->wfrm );
					BOX2OBJ_PartyPokeIconFrmInSet( syswk );
					// 戻り先のシーケンスを「つかむ」の処理にする
					BOX2UI_CursorMoveChange( syswk, BOX2UI_INIT_ID_ARRANGE_PARTY_MOVE, syswk->get_pos );
					BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
					BOX2OBJ_SetTouchBarButton( syswk, BOX2OBJ_TB_ICON_PASSIVE, BOX2OBJ_TB_ICON_OFF, BOX2OBJ_TB_ICON_PASSIVE );
					syswk->app->old_cur_pos = syswk->get_pos;
					syswk->app->vnext_seq = BOX2SEQ_MAINSEQ_ARRANGE_POKEGET_DATA_CHANGE;
				}
			}
			// トレイスクロール矢印ヒットチェック
			hit = BOX2UI_HitCheckContTrayArrow();
			if( hit == 0 ){
				BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_L );
				vf->seq = SEIRI_SEQ_SCROLL_L;
			}else if( hit == 1 ){
				BOX2MAIN_InitTrayScroll( syswk, BOX2MAIN_TRAY_SCROLL_R );
				vf->seq = SEIRI_SEQ_SCROLL_R;
			}
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_CANCEL_SCROLL_L:		// トレイが切り替わっていた場合のスクロール処理（左）
	case SEIRI_SEQ_CANCEL_SCROLL_R:		// トレイが切り替わっていた場合のスクロール処理（右）
		if( vf->seq == SEIRI_SEQ_CANCEL_SCROLL_L ){
			if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
				vf->seq = SEIRI_SEQ_ICON_PUT;
			}
		}
		if( vf->seq == SEIRI_SEQ_CANCEL_SCROLL_R ){
			if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
				vf->seq = SEIRI_SEQ_ICON_PUT;
			}
		}
	case SEIRI_SEQ_ICON_PUT:	// アイコン配置
		if( PokeIconObjMove( syswk ) == FALSE && ret1 == 0 && ret2 == 0 ){
			if( vf->seq == SEIRI_SEQ_ICON_PUT ){
				vf->seq = SEIRI_SEQ_END;
			}
		}
		break;

	case SEIRI_SEQ_SCROLL_L:	// トレイスクロール（左）
		if( BOX2MAIN_VFuncTrayScrollLeft( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = SEIRI_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_SCROLL_R:	// トレイスクロール（右）
		if( BOX2MAIN_VFuncTrayScrollRight( syswk ) == 0 ){
			BOX2OBJ_Vanish( syswk->app, BOX2OBJ_ID_HAND_CURSOR, FALSE );
			vf->seq = SEIRI_SEQ_ICON_GET;
		}
		if( GFL_UI_TP_GetPointCont( &tpx, &tpy ) == TRUE ){
			MoveGetPokeIcon( syswk, tpx, tpy );
		}
		break;

	case SEIRI_SEQ_END:				// 終了
		{
			BOX2MAIN_POKEMOVE_WORK * work = vf->work;
			PokeIconBufPosChange( syswk, work->dat );
		}
		if( BOX2BGWFRM_CheckBoxMoveFrm( syswk->app->wfrm ) == FALSE &&
				BOX2BGWFRM_CheckPartyPokeFrameRight( syswk->app->wfrm ) == FALSE ){
			BOX2OBJ_PokeCursorMove( syswk->app, syswk->get_pos );
			BOX2OBJ_PokeCursorVanish( syswk, TRUE );
		}else{
//			BOX2OBJ_PokeCursorMove( syswk->app, syswk->get_pos );
			BOX2OBJ_PokeCursorVanish( syswk, FALSE );
			CURSORMOVE_PosSet( syswk->app->cmwk, syswk->get_pos );
			CURSORMOVE_CursorOnOffSet( syswk->app->cmwk, TRUE );
		}
		vf->seq = 0;
		return 0;
	}

	return 1;
}
