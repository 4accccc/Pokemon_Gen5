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
	BS_TOP = 0,						// トップ
	BS_SCENARIO,					// シナリオ
	BS_INFO,						// 情報
};

// clwkの判別
enum{
	BS_CLWKMODE_NORMAL	= 0,		// 通常(clwk)
	BS_CLWKMODE_POKE,				// ポケモン(clwk)
	BS_CLWKMODE_CURSOR,				// カーソル
};

// 各種最大数
enum{
	BS_BG_MAXNUM		= 8,		// BG最大
	BS_CANVAS_MAXNUM	= 14,		// canvas最大(場面ごとに切り替え)
	BS_MES_MAX			= 3,		// メッセージ最大
	BS_EMPTY_MAX		= 34*2,		// empty最大
	//
	BS_POKE_MAX			= 6,		// ポケモン最大
};

// res
enum{
	BS_RES_TYPE_PAL		= 0,		// type
	BS_RES_POKE_PAL,				// poke
	BS_RES_CURSOR_PAL,				// cursor
	BS_RES_PAL_MAX,
	//
	BS_RES_TYPE_CHR_00	= 0,		// type
	BS_RES_TYPE_CHR_01,
	BS_RES_TYPE_CHR_10,
	BS_RES_TYPE_CHR_11,
	BS_RES_TYPE_CHR_12,
	BS_RES_TYPE_CHR_13,
	BS_RES_POKE_CHR_00,				// poke
	BS_RES_POKE_CHR_01,
	BS_RES_POKE_CHR_02,
	BS_RES_POKE_CHR_03,
	BS_RES_POKE_CHR_04,
	BS_RES_POKE_CHR_05,
	BS_RES_CURSOR_CHR,				// cursor
	BS_RES_CHR_MAX,
	//
	BS_RES_TYPE_CELL	= 0,		// type
	BS_RES_POKE_CELL,				// poke
	BS_RES_CURSOR_CELL,				// cursor
	BS_RES_CELL_MAX,
	//
	// type用
	BS_RES_TYPE_PAL_MAX		= 1,
	BS_RES_TYPE_CHR_MAX		= 6,
	BS_RES_TYPE_CELL_MAX	= 1,
	//
	// poke用
	BS_RES_POKE_CHR_MAX		= 6,
};

// clwk(type)
enum{
	BS_CLWK_TYPE00 = 0,				// タイプ(00)
	BS_CLWK_TYPE01,
	//
	BS_CLWK_TYPE10,					// 技タイプ(00)
	BS_CLWK_TYPE11,
	BS_CLWK_TYPE12,
	BS_CLWK_TYPE13,
	//
	BS_CLWK_MAX,
};
// clwk(poke)
enum{
	BS_CLWK_POKE_00 = 0,			// ポケモン(00)
	BS_CLWK_POKE_01,
	BS_CLWK_POKE_02,
	BS_CLWK_POKE_03,
	BS_CLWK_POKE_04,
	BS_CLWK_POKE_05,
	//
	BS_CLWK_POKE_MAX,
};

// clwk(cursor)
enum{
	BS_CLWK_CURSOR_00 = 0,			// 左上
	BS_CLWK_CURSOR_01,				// 左下
	BS_CLWK_CURSOR_02,				// 右上
	BS_CLWK_CURSOR_03,				// 右下
	//
	BS_CLWK_CURSOR_MAX,
};

// clwkの合計
enum{
	BS_TOTAL_CLWK_NUM	= (BS_CLWK_MAX+BS_CLWK_POKE_MAX+BS_CLWK_CURSOR_MAX),
};

// ポケモンの情報(1体)
// PP_Getを初期化時にまとめて処理するために作成
// (すべてが)u32でなくてもよいはずが、
// 返り値を直接セットするため以下のようにした
typedef struct{
	u32					id;				// id
	u32					sex;			// 性別
	u32					level;			// レベル
	u32					item;			// もちもの
	u32					stat;			// 特性
	u32					poke_type[2];	// タイプ
	u32					skill[4];		// 技
	u32					skill_type[4];	// 技対応
} BSCENARIO_POKE_WORK;

//
// ワーク
typedef struct {
	BSCENARIO_DATA*		dat;			///< 外部参照データ
	GFL_TCBLSYS*		tcbl;			///< TCBL
	PALETTE_FADE_PTR 	pfd;			///< パレットフェードデータ
	
//	GFL_FONT			*font;			///< フォント(フォントは大元を使用)
	GFL_FONT			*sfont;			///< 小フォント
	PRINT_QUE			*print_que;		///< que
	PRINT_UTIL			PrintUtil[BS_CANVAS_MAXNUM];	///< 文字表示用
	GFL_MSGDATA			*MsgData[BS_MES_MAX];			///< 0:台本トップ
	WORDSET				*Wordset;		///< wordset
	
	GFL_CLUNIT*			clunit;								///< obj
	GFL_CLWK*			clwk[BS_CLWK_MAX];					///< CLWK(type)
	GFL_CLWK*			clwk_poke[BS_CLWK_POKE_MAX];		///< CLWK(poke)
	GFL_CLWK*			clwk_cursor[BS_CLWK_CURSOR_MAX];	///< CLWK(cursor)
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
	s32					TopCurPos;		///< カーソル位置
	// シナリオ
	s32					PageNum;		///< 現在のページ番号(0-9)
	s32					ScenarioMax;	///< 最大数(20)
	s32					ScenarioCurPos;	///< カーソル位置
	// ポケモン情報
	s32					PokePageNum;	///< ページ数(0-5)
	s32					PokeMax;		///< 最大数(6)
	s32					InfoCurPos;		///< カーソル位置
	//
	s16					*EmptyBuffer;	///< 座標用のバッファ(必ず破棄すること)
	u16					*ScrBuffer[3];	///< スクリーンバッファ(必ず破棄すること)
	//
	BSCENARIO_POKE_WORK	PokeWork[BS_POKE_MAX];	///< ポケモン情報(初期化時にセット)
} BSCENARIO_WORK;

typedef int (*pBScenarioFunc)(BSCENARIO_WORK*);

