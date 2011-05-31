//============================================================================================
/**
 * @file		b_scenario_main.h
 * @brief		戦闘用シナリオ画面
 * @author
 * @date		05.02.10
 */
//============================================================================================
#pragma	once

#include "print/wordset.h"
#include "system/palanm.h"
#include "system/bgwinfrm.h"
#include "system/cursor_move.h"
#include "savedata/myitem_savedata.h"
#include "item/item.h"
#include "../btlv/btlv_finger_cursor.h"

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
	BSCENARIO_BG_MAXNUM		= 8,		// BG最大
	BSCENARIO_CANVAS_MAXNUM	= 12,		// canvas最大(場面ごとに切り替え)
	BSCENARIO_MES_MAX		= 2,		// メッセージ最大
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
	
	GFL_CLUNIT*			clunit;
	GFL_CLWK*			clwk[64];		///< CLWK(未使用)
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
} BSCENARIO_WORK;

typedef int (*pBScenarioFunc)(BSCENARIO_WORK*);

