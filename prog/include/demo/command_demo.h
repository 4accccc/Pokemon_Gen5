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

	u32	mode;		// 処理モード

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


//============================================================================================
//	グローバル
//============================================================================================

// デモPROCデータ
extern const GFL_PROC_DATA COMMANDDEMO_ProcData;

FS_EXTERN_OVERLAY(command_demo);
