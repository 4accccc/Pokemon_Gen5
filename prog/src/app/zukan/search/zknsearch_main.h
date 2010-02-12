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
#include "print/printsys.h"
#include "print/wordset.h"
#include "ui/frame_list.h"

#include "zukansearch.h"
#include "zknsearch_bmp_def.h"
#include "zknsearch_obj_def.h"


//============================================================================================
//	定数定義
//============================================================================================

// ヒープＩＤ後方確保用定義
#define	HEAPID_ZUKAN_SEARCH_L		( GFL_HEAP_LOWID(HEAPID_ZUKAN_SEARCH) )

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

	int	mainSeq;		// メインシーケンス
	int	nextSeq;		// 次のシーケンス
	int	funcSeq;		// フェードなど特定の処理の後のシーケンス

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

	CURSORMOVE_WORK * cmwk;			// カーソル移動ワーク
	BLINKPALANM_WORK * blink;		// カーソルアニメワーク

	u8	bgVanish;

	u8	page;
	u8	pageSeq;

	u8	btnMode;
	u8	btnID;
	u8	btnSeq;
	u8	btnCnt;

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
extern const GFL_DISP_VRAM * ZKNSEARCHMAIN_GetVramBankData(void);
extern void ZKNSEARCHMAIN_InitBg(void);
extern void ZKNSEARCHMAIN_ExitBg(void);
extern void ZKNSEARCHMAIN_LoadBgGraphic(void);

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



extern void ZKNSEARCHMAIN_InitBlinkAnm( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHMAIN_ExitBlinkAnm( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHMAIN_LoadManuPageScreen( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHMAIN_ListBGOn( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHMAIN_ListBGOff( ZKNSEARCHMAIN_WORK * wk );
