//============================================================================================
/**
 * @file		b_scenario_main.h
 * @brief		ポケウッド台本画面
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
// 番号
enum{
	BS_TOP = 0,							// トップ
	BS_SCENARIO,						// シナリオ
	BS_INFO,							// 情報
};

//
enum{
	CLWKMODE_NORMAL = 0,
	CLWKMODE_POKE,
};

//
enum{
	BSCENARIO_BG_MAXNUM		= 8,		// BG最大
	BSCENARIO_CANVAS_MAXNUM	= 12,		// canvas最大(場面ごとに切り替え)
	BSCENARIO_MES_MAX		= 2,		// メッセージ最大
	BSCENARIO_EMPTY_MAX		= 7*2,		// empty
};

// res
enum{
	BS_RES_TYPE_PAL		= 0,			// type
	BS_RES_POKE_PAL,					// poke
	BS_RES_PAL_MAX,
	//
	BS_RES_TYPE_CHR_00	= 0,			// type
	BS_RES_TYPE_CHR_01,
	BS_RES_TYPE_CHR_10,
	BS_RES_TYPE_CHR_11,
	BS_RES_TYPE_CHR_12,
	BS_RES_TYPE_CHR_13,
	BS_RES_POKE_CHR_00,					// poke
	BS_RES_POKE_CHR_01,
	BS_RES_POKE_CHR_02,
	BS_RES_POKE_CHR_03,
	BS_RES_POKE_CHR_04,
	BS_RES_POKE_CHR_05,
	BS_RES_CHR_MAX,
	//
	BS_RES_TYPE_CELL	= 0,			// type
	BS_RES_POKE_CELL,					// poke
	BS_RES_CELL_MAX,
	//
	//
	BS_RES_TYPE_PAL_MAX		= 1,
	BS_RES_TYPE_CHR_MAX		= 6,
	BS_RES_TYPE_CELL_MAX	= 1,
	//
	BS_RES_POKE_CHR_MAX		= 6,
};

// type
enum{
	CLWK_TYPE00 = 0,
	CLWK_TYPE01,
	//
	CLWK_TYPE10,
	CLWK_TYPE11,
	CLWK_TYPE12,
	CLWK_TYPE13,
	//
	BSCENARIO_CLWK_MAX,
};
// poke
enum{
	CLWK_POKE_00 = 0,
	CLWK_POKE_01,
	CLWK_POKE_02,
	CLWK_POKE_03,
	CLWK_POKE_04,
	CLWK_POKE_05,
	//
	BSCENARIO_CLWK_POKE_MAX,
};

//
// ワーク
typedef struct {
	BSCENARIO_DATA*		dat;			// 外部参照データ
	GFL_TCBLSYS*		tcbl;			// TCBL
	PALETTE_FADE_PTR 	pfd;			// パレットフェードデータ
	
//	GFL_FONT			*font;			///< フォント(フォントは大元を使用)
	GFL_FONT			*sfont;			///< 小フォント
	PRINT_QUE			*print_que;		///< que
	PRINT_UTIL			PrintUtil[BSCENARIO_CANVAS_MAXNUM];	///< 文字表示用
	GFL_MSGDATA			*MsgData[BSCENARIO_MES_MAX];		///< 0:台本トップ
	WORDSET				*Wordset;		///< wordset
	
	GFL_CLUNIT*			clunit;								///< obj
	GFL_CLWK*			clwk[BSCENARIO_CLWK_MAX];			///< CLWK
	GFL_CLWK*			clwk_poke[BSCENARIO_CLWK_POKE_MAX];	///< CLWK
	//
	u32					ResPal[ BS_RES_PAL_MAX];
	u32					ResChr[ BS_RES_CHR_MAX];
	u32					ResCell[BS_RES_CELL_MAX];
	//
	BOOL				initFlag;		///< 初期化したか（初期化前に外部から終了要求されると止まるので）
	//
	// 以下、追加
	s32					Step;			///< ステップ
	s32					Mode;			///< モード
	s32					NextMode;		///< 次
	u32					Flag;			///< フラグ
	s32					SubMode;		///< サブモード
	// トップ情報
	s32					TopNum;			///< 台本番号(0-39)
	// シナリオ
	s32					PageNum;		///< 現在のページ番号(0-9)
	s32					ScenarioMax;	///< 最大数(20)
	// ポケモン情報
	s32					PokePageNum;	///< ページ数(0-5)
	s32					PokeMax;		///< 最大数(6)
	//
	s16					*EmptyBuffer;	///< 座標用のバッファ(必ず破棄すること)
} BSCENARIO_WORK;

typedef int (*pBScenarioFunc)(BSCENARIO_WORK*);

