//============================================================================================
/**
 * @file		frame_list.h
 * @brief		ＢＧフレーム全体を使用したリスト処理
 * @author	Hiroyuki Nakamura
 * @date		10.01.23
 *
 *	モジュール名：FRAMELIST
 *
 *	・１画面につき、ＢＧ１面を占有します（スクロールするため）
 *	・内部でBMPWINを確保しているため、BMPWINの初期化後にリストを作成してください
 */
//============================================================================================
#pragma	once

#include "print/printsys.h"


//============================================================================================
//	定数定義
//============================================================================================

// リスト動作結果
#define	FRAMELIST_RET_CURSOR_ON		( 0xfffffff6 )		// カーソル表示
#define	FRAMELIST_RET_MOVE				( 0xfffffff7 )		// カーソル移動
#define	FRAMELIST_RET_SCROLL			( 0xfffffff8 )		// スクロール通常
#define	FRAMELIST_RET_RAIL				( 0xfffffff9 )		// レールスクロール
#define	FRAMELIST_RET_SLIDE				( 0xfffffffa )		// スライドスクロール
#define	FRAMELIST_RET_PAGE_UP			( 0xfffffffb )		// １ページ上へ
#define	FRAMELIST_RET_PAGE_DOWN		( 0xfffffffc )		// １ページ下へ
#define	FRAMELIST_RET_JUMP_TOP		( 0xfffffffd )		// リスト最上部へジャンプ
#define	FRAMELIST_RET_JUMP_BOTTOM	( 0xfffffffe )		// リスト最下部へジャンプ
#define	FRAMELIST_RET_NONE				( 0xffffffff )		// 動作なし

#define	FRAMELIST_SUB_BG_NONE	( 0xff )		// 上画面ＢＧ未使用

#define	FRAMELIST_SPEED_MAX		( 6 )				// スクロール速度の段階

// タッチパラメータ
typedef enum {
	FRAMELIST_TOUCH_PARAM_SLIDE = 0,					// スライド

	FRAMELIST_TOUCH_PARAM_RAIL,								// レール

	FRAMELIST_TOUCH_PARAM_UP,									// 上移動
	FRAMELIST_TOUCH_PARAM_DOWN,								// 下移動

	FRAMELIST_TOUCH_PARAM_PAGE_UP,						// ページ上移動
	FRAMELIST_TOUCH_PARAM_PAGE_DOWN,					// ページ下移動

	FRAMELIST_TOUCH_PARAM_LIST_TOP,						// リスト最上位へ
	FRAMELIST_TOUCH_PARAM_LIST_BOTTOM,				// リスト最下位へ

//	FRAMELIST_TOUCH_PARAM_ITEM = 0x80000000,	// 項目
	FRAMELIST_TOUCH_PARAM_ITEM,								// 項目

}FRAMELIST_TOUCH_PARAM;

// コールバック関数
typedef struct {
	// ワーク, 項目位置, PRINT_UTIL, 表示Ｙ座標, 描画先
	void (*draw)(void*,u32,PRINT_UTIL*,s16,BOOL);		// 描画処理
	// ワーク, リスト位置, フラグ（初期化時FALSE）
	void (*move)(void*,u32,BOOL);										// 移動処理
	// ワーク, スクロール値
	void (*scroll)(void*,s8);												// スクロール時
}FRAMELIST_CALLBACK;

// タッチデータ
typedef struct {
	GFL_UI_TP_HITTBL	tbl;				// 範囲データ
	FRAMELIST_TOUCH_PARAM	prm;		// パラメータ
}FRAMELIST_TOUCH_DATA;

// ヘッダーデータ
typedef struct {
	u8	mainBG;								// 下画面ＢＧ
	u8	subBG;								// 上画面ＢＧ ( FRAMELIST_SUB_BG_NONE = 未使用 )

	s8	itemPosX;							// 項目フレーム表示開始Ｘ座標
	s8	itemPosY;							// 項目フレーム表示開始Ｙ座標
	u8	itemSizX;							// 項目フレーム表示Ｘサイズ
	u8	itemSizY;							// 項目フレーム表示Ｙサイズ

	u8	bmpPosX;							// フレーム内に表示するBMPWINの表示Ｘ座標
	u8	bmpPosY;							// フレーム内に表示するBMPWINの表示Ｙ座標
	u8	bmpSizX;							// フレーム内に表示するBMPWINの表示Ｘサイズ
	u8	bmpSizY;							// フレーム内に表示するBMPWINの表示Ｙサイズ
	u8	bmpPal;								// フレーム内に表示するBMPWINのパレット

	u8	scrollSpeed[FRAMELIST_SPEED_MAX];		// スクロール速度 [0] = 最速 ※itemSizYを割り切れる値であること！

	u8	selPal;								// 選択項目のパレット

	u8	barSize;							// スクロールバーのＹサイズ

	u16	itemMax;							// 項目登録数
	u16	graMax;								// 背景登録数

	u8	initPos;							// 初期カーソル位置
	u8	posMax;								// カーソル移動範囲
	u16	initScroll;						// 初期スクロール値

	const FRAMELIST_TOUCH_DATA * touch;	// タッチデータ

	const FRAMELIST_CALLBACK * cbFunc;	// コールバック関数
	void * cbWork;											// コールバック関数に渡されるワーク

}FRAMELIST_HEADER;

typedef struct _FRAMELIST_WORK	FRAMELIST_WORK;		// ＢＧフレームリストワーク


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧフレームリスト作成
 *
 * @param		hed			ヘッダーデータ
 * @param		heapID	ヒープＩＤ
 *
 * @return	ワーク
 */
//--------------------------------------------------------------------------------------------
extern FRAMELIST_WORK * FRAMELIST_Create( FRAMELIST_HEADER * hed, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧフレームリスト削除
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void FRAMELIST_Exit( FRAMELIST_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		項目追加
 *
 * @param		wk			ワーク
 * @param		type		背景番号
 * @param		param		パラメータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void FRAMELIST_AddItem( FRAMELIST_WORK * wk, u32 type, u32 param );

//--------------------------------------------------------------------------------------------
/**
 * @brief		背景グラフィック読み込み
 *
 * @param		wk				ワーク
 * @param		ah				アーカイブハンドル
 * @param		dataIdx		データインデックス
 * @param		comp			圧縮フラグ TRUE = 圧縮
 * @param		frameNum	背景番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void FRAMELIST_LoadFrameGraphicAH( FRAMELIST_WORK * wk, ARCHANDLE * ah, u32 dataIdx, BOOL comp, u32 frameNum );

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソルパレットアニメ用データ読み込み
 *
 * @param		wk				ワーク
 * @param		ah				アーカイブハンドル
 * @param		dataIdx		データインデックス
 * @param		startPal	パレット１ 0～15
 * @param		endPal		パレット２ 0～15
 *
 * @return	ワーク
 */
//--------------------------------------------------------------------------------------------
extern void FRAMELIST_LoadBlinkPalette( FRAMELIST_WORK * wk, ARCHANDLE * ah, u32 dataIdx, u32 startPal, u32 endPal );

//--------------------------------------------------------------------------------------------
/**
 * @brief		プリントキュー取得
 *
 * @param		wk				ワーク
 *
 * @return	プリントキュー
 */
//--------------------------------------------------------------------------------------------
extern PRINT_QUE * FRAMELIST_GetPrintQue( FRAMELIST_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		項目パラメータ取得
 *
 * @param		wk				ワーク
 * @param		itemIdx		項目インデックス
 *
 * @return	項目パラメータ
 */
//--------------------------------------------------------------------------------------------
extern u32 FRAMELIST_GetItemParam( FRAMELIST_WORK * wk, u32 itemIdx );

//--------------------------------------------------------------------------------------------
/**
 * @brief		リスト位置取得
 *
 * @param		wk				ワーク
 *
 * @return	リスト位置
 */
//--------------------------------------------------------------------------------------------
extern u32 FRAMELIST_GetListPos( FRAMELIST_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル位置取得
 *
 * @param		wk				ワーク
 *
 * @return	項目パラメータ
 */
//--------------------------------------------------------------------------------------------
extern u32 FRAMELIST_GetCursorPos( FRAMELIST_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		リストスクロールカウンタを取得
 *
 * @param		wk		ワーク
 *
 * @return	スクロールカウンタ
 */
//--------------------------------------------------------------------------------------------
extern u32 FRAMELIST_GetScrollCount( FRAMELIST_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		リストを下にスクロールできるか
 *
 * @param		wk		ワーク
 *
 * @retval	"TRUE = 可"
 * @retval	"FALSE = 不可"
 */
//--------------------------------------------------------------------------------------------
extern BOOL FRAMELIST_CheckScrollMax( FRAMELIST_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧフレームリスト初期描画
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern BOOL FRAMELIST_Init( FRAMELIST_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧフレームリストメイン処理
 *
 * @param		wk		ワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------------------------------------
extern u32 FRAMELIST_Main( FRAMELIST_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		スクロールバーの表示Ｙ座標を取得
 *
 * @param		wk			ワーク
 * @param		barSY		バーのＹサイズ
 *
 * @return	Ｙ座標
 */
//--------------------------------------------------------------------------------------------
extern u32 FRAMELIST_GetScrollBarPY( FRAMELIST_WORK * wk );
