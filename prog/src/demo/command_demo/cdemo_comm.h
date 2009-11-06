//============================================================================================
/**
 * @file		cdemo_comm.h
 * @brief		コマンドデモ画面 コマンド処理
 * @author	Hiroyuki Nakamura
 * @date		09.05.07
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================

// コマンド
enum {
	CDEMOCOMM_WAIT = 0,			// 00: ウェイト

	CDEMOCOMM_BLACK_IN,			// 01: ブラックイン
	CDEMOCOMM_BLACK_OUT,			// 02: ブラックアウト
	CDEMOCOMM_WHITE_IN,			// 03: ホワイトイン
	CDEMOCOMM_WHITE_OUT,			// 04: ホワイトアウト
	CDEMOCOMM_FADE_MAIN,			// 05: フェードメイン

	CDEMOCOMM_BG_LOAD,				// 06: BG読み込み
	CDEMOCOMM_BG_VANISH,			// 07: BG表示切り替え
	CDEMOCOMM_BG_PRI_CHG,		// 08: BGプライオリティ切り替え
	CDEMOCOMM_ALPHA_INIT,		// 09: アルファブレンド設定
	CDEMOCOMM_ALPHA_START,		// 10: アルファブレンド開始
	CDEMOCOMM_ALPHA_NONE,		// 11: アルファ無効

	CDEMOCOMM_BGM_SAVE,			// 12: 再生中のBGMを記憶
	CDEMOCOMM_BGM_STOP,			// 13: 再生中のBGMを停止
	CDEMOCOMM_BGM_PLAY,			// 14: BGM再生

	CDEMOCOMM_OBJ_VANISH,		// 15: OBJ表示切り替え
	CDEMOCOMM_OBJ_PUT,			// 16: OBJ座標設定
	CDEMOCOMM_OBJ_BG_PRI_CHANGE,	// 17: OBJとBGのプライオリティ設定

	CDEMOCOMM_BG_FRM_ANIME,

	CDEMOCOMM_DEBUG_PRINT,		// 18: デバッグプリント

	CDEMOCOMM_END				// 終了
};

// パラメータ
enum {
	CDEMO_PRM_BLEND_PLANE_1 = 0,	// アルファブレンドの第１対象面指定
	CDEMO_PRM_BLEND_PLANE_2,			// アルファブレンドの第２対象面指定

/*
	CDEMO_PRM_BGM_PLAY_SAVE = 0xffffffff,		// 記憶したＢＧＭを再生する

	CDEMO_PRM_OBJ_ARCEUS = 0,			// 連れ歩きアルセウスＯＢＪ
	CDEMO_PRM_OBJ_MAX,						// ※データ数定義のため使用禁止

	CDEMO_OBJ_OFF = 0,						// OBJ非表示
	CDEMO_OBJ_ON,									// OBJ表示
*/
};

// コマンド戻り値
enum {
	CDEMOCOMM_RET_TRUE = 0,		// コマンド実行中
	CDEMOCOMM_RET_FALSE,			// コマンド終了		( 即次のコマンドを再生します )
	CDEMOCOMM_RET_NEXT,				// 次のコマンドへ	( 1sync待ちます )
};



extern int CDEMOCOMM_Main( CDEMO_WORK * wk );
