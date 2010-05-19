//============================================================================================
/**
 * @file		command_demo.h
 * @brief		コマンドデモ画面 呼び出し
 * @author	Hiroyuki Nakamura
 * @date		09.01.21
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================

// 処理モード
enum {
	COMMANDDEMO_MODE_GF_LOGO = 0,		// ゲームフリークロゴ
	COMMANDDEMO_MODE_OP_MOVIE1,			// オープニングムービー１
	COMMANDDEMO_MODE_OP_MOVIE2,			// オープニングムービー２
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

// 外部設定データ
typedef struct {
	// [ in ]
	u16	mode;		// 処理モード
	u8	skip;		// スキップ設定
	// [ out ]
	u8	ret;		// 終了モード
}COMMANDDEMO_DATA;


//============================================================================================
//	グローバル
//============================================================================================

// デモPROCデータ
extern const GFL_PROC_DATA COMMANDDEMO_ProcData;

FS_EXTERN_OVERLAY(command_demo);
