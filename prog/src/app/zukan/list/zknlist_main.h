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
#include "print/printsys.h"

#include "../zukan_common.h"
#include "zukanlist.h"
#include "zknlist_bmp_def.h"
#include "zknlist_obj_def.h"


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



// リスト画面ワーク
typedef struct {
	ZUKANLIST_DATA * dat;

	GFL_TCB * vtask;		// TCB ( VBLANK )

//	int	key_repeat_speed;
//	int	key_repeat_wait;

	int	mainSeq;		// メインシーケンス
	int	nextSeq;		// 次のシーケンス
	int	wipeSeq;		// ワイプ後のシーケンス

	PRINT_UTIL	win[ZKNLISTBMP_WINIDX_MAX];		// BMPWIN

	// OBJ
	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[ZKNLISTOBJ_IDX_MAX];
	u32	chrRes[ZKNLISTOBJ_CHRRES_MAX];
	u32	palRes[ZKNLISTOBJ_PALRES_MAX];
	u32	celRes[ZKNLISTOBJ_CELRES_MAX];



/*
	GFL_TCB * vtask;					// TCB ( VBLANK )

	BOX2_IRQWK	vfunk;				// VBLANK関数ワーク
	int	vnext_seq;

	PALETTE_FADE_PTR	pfd;		// パレットフェードデータ

	CURSORMOVE_WORK * cmwk;		// カーソル移動ワーク
	BGWINFRM_WORK * wfrm;			// ウィンドウフレーム

	GFL_FONT * font;					// 通常フォント
	GFL_FONT * nfnt;					// 8x8フォント
	GFL_MSGDATA * mman;				// メッセージデータマネージャ
	WORDSET * wset;						// 単語セット
	STRBUF * exp;							// メッセージ展開領域
	PRINT_QUE * que;					// プリントキュー
	PRINT_STREAM * stream;		// プリントストリーム

	GFL_ARCUTIL_TRANSINFO	syswinInfo;

	// はい・いいえ関連
//	TOUCH_SW_SYS * tsw;		// タッチウィンドウ
	APP_TASKMENU_ITEMWORK	ynList[2];
	APP_TASKMENU_RES * ynRes;
	APP_TASKMENU_WORK * ynWork;
	u16	ynID;				// はい・いいえＩＤ

	BUTTON_ANM_WORK	bawk;				// ボタンアニメワーク

	// ポケモンアイコン
	ARCHANDLE * pokeicon_ah;

	u8	pokeicon_cgx[BOX2OBJ_POKEICON_TRAY_MAX][BOX2OBJ_POKEICON_CGX_SIZE];
	u8	pokeicon_pal[BOX2OBJ_POKEICON_TRAY_MAX];
	u8	pokeicon_id[BOX2OBJ_POKEICON_MAX];
	BOOL	pokeicon_exist[BOX2OBJ_POKEICON_TRAY_MAX];
*/



}ZKNLISTMAIN_WORK;

typedef int (*pZKNLIST_FUNC)(ZKNLISTMAIN_WORK*);


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_InitVBlank( ZKNLISTMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTMAIN_ExitVBlank( ZKNLISTMAIN_WORK * wk );

extern void ZKNLISTMAIN_InitVram(void);
extern const GFL_DISP_VRAM * ZKNLISTMAIN_GetVramBankData(void);
extern void ZKNLISTMAIN_InitBg(void);
extern void ZKNLISTMAIN_ExitBg(void);
extern void ZKNLISTMAIN_LoadBgGraphic(void);
