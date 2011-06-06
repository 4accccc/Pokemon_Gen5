//============================================================================================
/**
 * @file		b_choice_main.h
 * @brief		ポケウッド選択肢画面
 * @author
 * @date		05.02.10
 */
//============================================================================================
#pragma	once

#include "print/wordset.h"
#include "system/palanm.h"

//============================================================================================
//	定数定義
//============================================================================================

// 各種最大数
enum{
	BCHOICE_CANVAS_MAXNUM	= 14,		// canvas最大(場面ごとに切り替え)
	//
	BCHOICE_CLWK_CURSOR_MAX	= 4,		// clwk最大
	//
	BC_TOTAL_CLWK_NUM		= (BCHOICE_CLWK_CURSOR_MAX),		// 最大
};

// res
enum{
	BC_RES_CURSOR_PAL 				= 0,
	BC_RES_CURSOR_CHR,
	BC_RES_CURSOR_CELL,
	//
	BC_RES_MAX,
};

//
// ワーク
typedef struct {
	BCHOICE_DATA*		dat;			// 外部参照データ
	GFL_TCBLSYS*		tcbl;			// TCBL
	PALETTE_FADE_PTR 	pfd;			// パレットフェードデータ
	
//	GFL_FONT			*font;			///< フォント(フォントは大元を使用)
	PRINT_QUE			*print_que;		///< que
	PRINT_UTIL			PrintUtil[BCHOICE_CANVAS_MAXNUM];	///< 文字表示用
	//
	BOOL				initFlag;		///< 初期化したか（初期化前に外部から終了要求されると止まるので）
	//
	GFL_CLUNIT*			clunit;										///< obj
	GFL_CLWK*			clwk_cursor[BCHOICE_CLWK_CURSOR_MAX];		///< CLWK(cursor)
	//
	u32					Res[BC_RES_MAX];
	// 以下、追加
	s32					Step;			///< ステップ
	u32					Flag;			///< フラグ
	s32					Mode;			///< モード
	s32					Result;			///< 結果
	s32					Cursor;			///< カーソル
	//
	s16					*EmptyBuffer;	///< 座標用のバッファ(必ず破棄すること)
} BCHOICE_WORK;

typedef int (*pBChoiceFunc)(BCHOICE_WORK*);

