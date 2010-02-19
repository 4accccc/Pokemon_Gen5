//============================================================================================
/**
 * @file		zknlist_main.h
 * @brief		図鑑リスト画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZKNLISTMAIN
 */
//============================================================================================
#pragma once

#include "system/main.h"
#include "system/bgwinfrm.h"
#include "system/cursor_move.h"
#include "system/palanm.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "ui/frame_list.h"

#include "../zukan_common.h"
#include "zukanlist.h"
#include "zknlist_bmp_def.h"
#include "zknlist_obj_def.h"
#include "zknlist_bgwfrm_def.h"



//============================================================================================
//	定数定義
//============================================================================================

// 後方確保用ヒープＩＤ
#define	HEAPID_ZUKAN_LIST_L		( GFL_HEAP_LOWID(HEAPID_ZUKAN_LIST) )

#define	ZKNLISTMAIN_MBG_PAL_TOUCHBAR	( 14 )
#define	ZKNLISTMAIN_MBG_PAL_FONT			( 15 )

#define	ZKNLISTMAIN_SBG_PAL_FONT			( 15 )

// タッチバー定義
#define	ZKNLISTMAIN_TOUCH_BAR_PX		( 0 )
#define	ZKNLISTMAIN_TOUCH_BAR_PY		( 21 )
#define	ZKNLISTMAIN_TOUCH_BAR_SX		( 32 )
#define	ZKNLISTMAIN_TOUCH_BAR_SY		( 3 )

// リスト項目サイズ
#define	ZKNLISTMAIN_LIST_PX					( 16 )
#define	ZKNLISTMAIN_LIST_PY					( 0 )
#define	ZKNLISTMAIN_LIST_SX					( 16 )
#define	ZKNLISTMAIN_LIST_SY					( 3 )

// リストパラメータ関連
#define	LIST_MONS_MASK		( 0x0fffffff )
#define	LIST_INFO_MASK		( 0xf0000000 )
#define	LIST_INFO_SHIFT		( 28 )

#define	SET_LIST_PARAM(p,m)	( ( p << LIST_INFO_SHIFT ) | m )
#define	GET_LIST_MONS(a)		( a & LIST_MONS_MASK )
#define	GET_LIST_INFO(a)		( ( a & LIST_INFO_MASK ) >> LIST_INFO_SHIFT )


//typedef struct _ZUKAN_LIST_WORK	ZUKAN_LIST_WORK;
//typedef void (*pZUKAN_LIST_CALLBACK)(void*,s16,s16,s16,s16,u32);


// リスト画面ワーク
typedef struct {
	ZUKANLIST_DATA * dat;

	GFL_TCB * vtask;		// TCB ( VBLANK )
	GFL_TCB * htask;		// TCB ( HBLANK )

	PALETTE_FADE_PTR	pfd;		// パレットフェードデータ

//	int	key_repeat_speed;
//	int	key_repeat_wait;

	int	mainSeq;		// メインシーケンス
	int	nextSeq;		// 次のシーケンス
	int	wipeSeq;		// ワイプ後のシーケンス

//	BGWINFRM_WORK * wfrm;			// ウィンドウフレーム
//	u16 * nameBG[2];

	PRINT_UTIL	win[ZKNLISTBMP_WINIDX_MAX];		// BMPWIN
//	u8 * nameBmp;

	GFL_FONT * font;						// 通常フォント
	GFL_MSGDATA * mman;					// メッセージデータマネージャ
	WORDSET * wset;							// 単語セット
	STRBUF * exp;								// メッセージ展開領域
	PRINT_QUE * que;						// プリントキュー
	STRBUF * name[MONSNO_END];	// ポケモン名文字列

	// OBJ
	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[ZKNLISTOBJ_IDX_MAX];
	u32	chrRes[ZKNLISTOBJ_CHRRES_MAX];
	u32	palRes[ZKNLISTOBJ_PALRES_MAX];
	u32	celRes[ZKNLISTOBJ_CELRES_MAX];

	u16	pokeGraFlag:1;			// ポケモン正面絵表示制御
	u16	buttonID:15;				// ボタンアニメ用ＩＤ
	u8	buttonSeq;					// ボタンアニメ用シーケンス
	u8	buttonCnt;					// ボタンアニメ用カウンタ

	u16	iconPutMain;				// ポケアイコン表示状況（メイン）
	u16	iconPutSub;					// ポケアイコン表示状況（サブ）

	FRAMELIST_WORK * lwk;		// リストワーク
	u32	listInit;					// リスト初期化シーケンス

	u16 * localNo;					// 地方図鑑番号リスト

	u32	BaseScroll;					// 背景スクロールカウンタ

	u16	frameScrollCnt;			// 開始・終了時のスクロールカウンタ
	s16	frameScrollVal;			// 開始・終了時のスクロール値

}ZKNLISTMAIN_WORK;

typedef int (*pZKNLIST_FUNC)(ZKNLISTMAIN_WORK*);


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定
 *
 * @param		wk		図鑑リスト画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_InitVBlank( ZKNLISTMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		wk		図鑑リスト画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_ExitVBlank( ZKNLISTMAIN_WORK * wk );

extern void ZKNLISTMAIN_InitHBlank( ZKNLISTMAIN_WORK * wk );
extern void ZKNLISTMAIN_ExitHBlank( ZKNLISTMAIN_WORK * wk );


extern void ZKNLISTMAIN_InitVram(void);
extern const GFL_DISP_VRAM * ZKNLISTMAIN_GetVramBankData(void);
extern void ZKNLISTMAIN_InitBg(void);
extern void ZKNLISTMAIN_ExitBg(void);
extern void ZKNLISTMAIN_LoadBgGraphic(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード初期化
 *
 * @param		wk		図鑑リスト画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_InitPaletteFade( ZKNLISTMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード解放
 *
 * @param		wk		図鑑リスト画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_ExitPaletteFade( ZKNLISTMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェードリクエスト
 *
 * @param		wk			図鑑リスト画面ワーク
 * @param		start		開始濃度
 * @param		end			終了濃度
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_SetPalFadeSeq( ZKNLISTMAIN_WORK * wk, u8 start, u8 end );

//--------------------------------------------------------------------------------------------
/**
 * @brief		アルファブレンド設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_SetBlendAlpha(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連初期化
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_InitMsg( ZKNLISTMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連解放
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_ExitMsg( ZKNLISTMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  リスト作成
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_MakeList( ZKNLISTMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  リスト削除
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_FreeList( ZKNLISTMAIN_WORK * wk );

extern void ZKNLISTMAIN_LoadLocalNoList( ZKNLISTMAIN_WORK * wk );
extern void ZKNLISTMAIN_FreeLocalNoList( ZKNLISTMAIN_WORK * wk );

extern void ZKNLISTMAIN_InitFrameScroll( ZKNLISTMAIN_WORK * wk );
extern void ZKNLISTMAIN_SetFrameScrollParam( ZKNLISTMAIN_WORK * wk, s16 val );
extern BOOL ZKNLISTMAIN_MainSrameScroll( ZKNLISTMAIN_WORK * wk );
