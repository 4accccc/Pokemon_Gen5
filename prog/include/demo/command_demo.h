//============================================================================================
/**
 * @file		command_demo.h
 * @brief		コマンドデモ画面 呼び出し
 * @author	Hiroyuki Nakamura
 * @date		09.01.21
 */
//============================================================================================
#pragma	once

/*
#include "savedata/savedata_def.h"
#include "system/procsys.h"
#include "../src/field/field_glb_state.h"
*/


//============================================================================================
//	定数定義
//============================================================================================

// 処理モード
enum {
	COMMANDDEMO_MODE_TEST = 0,		// テストムービー
};

// スキップ設定
enum {
	COMMANDDEMO_SKIP_OFF = 0,			// スキップなし
	COMMANDDEMO_SKIP_ON,					// スキップあり
	COMMANDDEMO_SKIP_DEBUG,				// スキップあり（デバッグ用）
};

// 終了モード
enum {
	COMMANDDEMO_RET_NORMAL = 0,		// 通常
	COMMANDDEMO_RET_SKIP,					// スキップした
	COMMANDDEMO_RET_SKIP_DEBUG,		// スキップした（デバッグ用）
};

// 伝説デモ外部設定データ
typedef struct {
/*
	SAVEDATA * savedata;
	GLST_DATA	glst;
	u16	time;		// 時間帯（ホウオウのみ）
#ifdef PM_DEBUG
	u16	scene;		// シーン番号 0-1（ホウオウのみ）　←連続イベントなので、いらないかも
	u16	debug;
#endif	// PM_DEBUG;
*/

	// [ in ]
	u16	mode;		// 処理モード
	u8	skip;		// スキップ設定
	// [ out ]
	u8	ret;		// 終了モード

}COMMANDDEMO_DATA;

/*
#ifdef PM_DEBUG
typedef struct {
	LEGEND_DEMO_DATA * dat;
	u16	mode;
	u8	time;
	u8	scene;
	u16	play_mode;
	u16	seq;
}D_LD_WORK;
#endif	// PM_DEBUG
*/

// [ HEAPID_COMMAND_DEMO ] 後方確保用定義
#define	HEAPID_COMMAND_DEMO_L		( GFL_HEAP_LOWID(HEAPID_COMMAND_DEMO) )


//============================================================================================
//	グローバル
//============================================================================================

// デモPROCデータ
extern const GFL_PROC_DATA COMMANDDEMO_ProcData;

FS_EXTERN_OVERLAY(command_demo);
