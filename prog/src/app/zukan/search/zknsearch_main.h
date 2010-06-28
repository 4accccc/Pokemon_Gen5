//============================================================================================
/**
 * @file		zknsearch_main.h
 * @brief		図鑑検索画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZKNSEARCHMAIN
 */
//============================================================================================
#pragma	once

#include "system/cursor_move.h"
#include "system/blink_palanm.h"
#include "system/bgwinfrm.h"
#include "system/palanm.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "ui/frame_list.h"

#include "../zukan_common.h"
#include "zukansearch.h"
#include "zknsearch_bmp_def.h"
#include "zknsearch_obj_def.h"
#include "zukan_search_engine.h"


//============================================================================================
//	定数定義
//============================================================================================

// ヒープＩＤ後方確保用定義
#define	HEAPID_ZUKAN_SEARCH_L		( GFL_HEAP_LOWID(HEAPID_ZUKAN_SEARCH) )

// ＢＧパレット定義
#define	ZKNSEARCHMAIN_MBG_PAL_BUTTON_DEF	( 1 )
#define	ZKNSEARCHMAIN_MBG_PAL_BUTTON_CUR	( 2 )
#define	ZKNSEARCHMAIN_MBG_PAL_BUTTON_CUR2	( 3 )
#define	ZKNSEARCHMAIN_MBG_PAL_TOUCHBAR		( 14 )
#define	ZKNSEARCHMAIN_MBG_PAL_FONT				( 15 )

#define	ZKNSEARCHMAIN_SBG_PAL_FONT			( 15 )

#define	ZKNSEARCHMAIN_ITEM_MAX				( 50 )		// リスト項目数

// タッチバー定義
#define	ZKNSEARCHMAIN_TOUCH_BAR_PX		( 0 )
#define	ZKNSEARCHMAIN_TOUCH_BAR_PY		( 21 )
#define	ZKNSEARCHMAIN_TOUCH_BAR_SX		( 32 )
#define	ZKNSEARCHMAIN_TOUCH_BAR_SY		( 3 )

// ページ番号
enum {
	ZKNSEARCHMAIN_PAGE_ROW = 0,
	ZKNSEARCHMAIN_PAGE_NAME,
	ZKNSEARCHMAIN_PAGE_TYPE,
	ZKNSEARCHMAIN_PAGE_COLOR,
	ZKNSEARCHMAIN_PAGE_FORM,
};

// メインワーク
typedef struct {
	ZUKANSEARCH_DATA * dat;		// 外部データ

	GFL_TCB * vtask;		// TCB ( VBLANK )
	GFL_TCB * htask;		// TCB ( HBLANK )

	PALETTE_FADE_PTR	pfd;		// パレットフェードデータ

	int	mainSeq;		// メインシーケンス
	int	subSeq;			// サブシーケンス
	int	nextSeq;		// 次のシーケンス
	int	funcSeq;		// フェードなど特定の処理の後のシーケンス

	BGWINFRM_WORK * wfrm;

	// OBJ
	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[ZKNSEARCHOBJ_IDX_MAX];
	u32	chrRes[ZKNSEARCHOBJ_CHRRES_MAX];
	u32	palRes[ZKNSEARCHOBJ_PALRES_MAX];
	u32	celRes[ZKNSEARCHOBJ_CELRES_MAX];

	PRINT_UTIL	win[ZKNSEARCHBMP_WINIDX_MAX];		// BMPWIN

	GFL_FONT * font;						// 通常フォント
	GFL_MSGDATA * mman;					// メッセージデータマネージャ
	WORDSET * wset;							// 単語セット
	STRBUF * exp;								// メッセージ展開領域
	PRINT_QUE * que;						// プリントキュー

	FRAMELIST_WORK * lwk;										// リストワーク
	STRBUF * item[ZKNSEARCHMAIN_ITEM_MAX];	// リスト項目文字列
	u32	listPosMax;													// リストカーソル移動範囲

	CURSORMOVE_WORK * cmwk;			// カーソル移動ワーク
	BLINKPALANM_WORK * blink;		// カーソルアニメワーク

	u8	bgVanish;					// BG表示切り替え

	u8	page;							// ページ
	u8	pageSeq;					// ページ処理シーケンス

	u8	loadingCnt;				// 読み込み中カウンタ
	u16 * loadingBuff;		// 読み込み中ウィンドウ

	// ボタン処理
	u8	btnMode;
	u8	btnID;
	u8	btnSeq;
	u8	btnCnt;

	u32	BaseScroll;					// 背景スクロールカウンタ

	u16	frameScrollCnt;			// 開始・終了時のスクロールカウンタ
	s16	frameScrollVal;			// 開始・終了時のスクロール値

  ZKN_SCH_EGN_DIV_WORK*  egn_wk;    // 分割検索エンジンワーク
  ZKN_SCH_EGN_DIV_STATE  egn_st;    // 分割検索エンジンの状態

}ZKNSEARCHMAIN_WORK;

typedef int (*pZKNSEARCH_FUNC)(ZKNSEARCHMAIN_WORK*);


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_InitVBlank( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ExitVBlank( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		HBLANK関数設定
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_InitHBlank( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		HBLANK関数削除
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ExitHBlank( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  VRAM初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_InitVram(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief	  VRAM設定データ取得
 *
 * @param		none
 *
 * @return	VRAM設定
 */
//--------------------------------------------------------------------------------------------
extern const GFL_DISP_VRAM * ZKNSEARCHMAIN_GetVramBankData(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_InitBg(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ解放
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ExitBg(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧプライオリティ切り替え（メインページ）
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ChangeBgPriorityMenu(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧプライオリティ切り替え（各リストページ）
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ChangeBgPriorityList(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧグラフィック読み込み
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_LoadBgGraphic(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード初期化
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_InitPaletteFade( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード解放
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ExitPaletteFade( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェードリクエスト
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		start		開始濃度
 * @param		end			終了濃度
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_SetPalFadeSeq( ZKNSEARCHMAIN_WORK * wk, u8 start, u8 end );

//--------------------------------------------------------------------------------------------
/**
 * @brief		アルファブレンド設定
 *
 * @param		flg		ON/OFF
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_SetBlendAlpha( BOOL flg );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ウィンドウ設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_SetWindow(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		ウィンドウ無効
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ResetWindow(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連初期化
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_InitMsg( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連解放
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ExitMsg( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソルアニメ初期化
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_InitBlinkAnm( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソルアニメ解放
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ExitBlinkAnm( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGWINFRM初期化
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_InitBgWinFrame( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGWINFRM削除
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ExitBgWinFrame( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインページＢＧスクリーン読み込み
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_LoadMenuPageScreen( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		並び選択ページＢＧスクリーン読み込み
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_LoadRowListPageScreen( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		名前選択ページＢＧスクリーン読み込み
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_LoadNameListPageScreen( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイプ選択ページＢＧスクリーン読み込み
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_LoadTypeListPageScreen( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		色選択ページＢＧスクリーン読み込み
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_LoadColorListPageScreen( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		フォルム選択ページＢＧスクリーン読み込み
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_LoadFormListPageScreen( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		検索中ウィンドウ読み込み
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_LoadLoadingWindow( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		検索中ウィンドウ解放
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_UnloadLoadingWindow( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		検索中ウィンドウ表示
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_LoadingWindowOn( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		検索中ウィンドウ非表示
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_LoadingWindowOff( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		リストのＢＧ面表示リクエスト
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ListBGOn( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		リストのＢＧ面非表示リクエスト
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ListBGOff( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タブＢＧフレーム初期位置設定
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_InitFrameScroll( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タブＢＧフレームスクロール設定
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		val		スクロール速度
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_SetFrameScrollParam( ZKNSEARCHMAIN_WORK * wk, s16 val );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タブＢＧフレームスクロールメイン
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL ZKNSEARCHMAIN_MainSrameScroll( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定位置のタイプ取得
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	タイプ
 */
//--------------------------------------------------------------------------------------------
extern u32 ZKNSEARCHMAIN_GetSortType( ZKNSEARCHMAIN_WORK * wk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイプからリスト位置を取得
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	リスト位置
 */
//--------------------------------------------------------------------------------------------
extern u32 ZKNSEARCHMAIN_GetSortTypeIndex( ZKNSEARCHMAIN_WORK * wk, u32 type );

//--------------------------------------------------------------------------------------------
/**
 * @brief		フォルムデータからリストインデックスへ変換
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ConvFormDataToList( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		フォルムリストインデックスからデータへ変換
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHMAIN_ConvFormListToData( ZKNSEARCHMAIN_WORK * wk );
