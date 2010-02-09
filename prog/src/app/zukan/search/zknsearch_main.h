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

#include "zukansearch.h"


//============================================================================================
//	定数定義
//============================================================================================

// ヒープＩＤ後方確保用定義
#define	HEAPID_ZUKAN_SEARCH_L		( GFL_HEAP_LOWID(HEAPID_ZUKAN_SEARCH) )

#define	ZKNSEARCHMAIN_MBG_PAL_TOUCHBAR	( 14 )
#define	ZKNSEARCHMAIN_MBG_PAL_FONT			( 15 )

#define	ZKNSEARCHMAIN_SBG_PAL_FONT			( 15 )

// タッチバー定義
#define	ZKNSEARCHMAIN_TOUCH_BAR_PX		( 0 )
#define	ZKNSEARCHMAIN_TOUCH_BAR_PY		( 21 )
#define	ZKNSEARCHMAIN_TOUCH_BAR_SX		( 32 )
#define	ZKNSEARCHMAIN_TOUCH_BAR_SY		( 3 )

// メインワーク
typedef struct {
	ZUKANSEARCH_DATA * dat;		// 外部データ

	GFL_TCB * vtask;		// TCB ( VBLANK )
	GFL_TCB * htask;		// TCB ( HBLANK )

	int	mainSeq;		// メインシーケンス
	int	nextSeq;		// 次のシーケンス
	int	fadeSeq;		// フェード後のシーケンス（ワイプも）


	u32	BaseScroll;

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



extern void ZKNSEARCHMAIN_InitVram(void);
extern void ZKNSEARCHMAIN_InitBg(void);
extern void ZKNSEARCHMAIN_ExitBg(void);
extern void ZKNSEARCHMAIN_LoadBgGraphic(void);
