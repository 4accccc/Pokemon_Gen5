//============================================================================================
/**
 * @file		dpc_obj.h
 * @brief		殿堂入り確認画面 ＯＢＪ関連
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================

// OBJ ID
enum {
	DPCOBJ_ID_POKE01 = 0,		// ポケモン１
	DPCOBJ_ID_POKE02,
	DPCOBJ_ID_POKE03,
	DPCOBJ_ID_POKE04,
	DPCOBJ_ID_POKE05,
	DPCOBJ_ID_POKE06,

	DPCOBJ_ID_POKE11,				// ポケモン１（スワップ用）
	DPCOBJ_ID_POKE12,
	DPCOBJ_ID_POKE13,
	DPCOBJ_ID_POKE14,
	DPCOBJ_ID_POKE15,
	DPCOBJ_ID_POKE16,

	DPCOBJ_ID_ARROW_L,			// ページ切り替え左矢印
	DPCOBJ_ID_ARROW_R,			// ページ切り替え右矢印
	DPCOBJ_ID_EXIT,					// 終了ボタン
	DPCOBJ_ID_RETURN,				// 戻るボタン

	DPCOBJ_ID_MAX
};

// キャラリソース
enum {
	DPCOBJ_CHRRES_POKE01 = 0,		// ポケモン１
	DPCOBJ_CHRRES_POKE02,
	DPCOBJ_CHRRES_POKE03,
	DPCOBJ_CHRRES_POKE04,
	DPCOBJ_CHRRES_POKE05,
	DPCOBJ_CHRRES_POKE06,

	DPCOBJ_CHRRES_POKE11,				// ポケモン１（スワップ用）
	DPCOBJ_CHRRES_POKE12,
	DPCOBJ_CHRRES_POKE13,
	DPCOBJ_CHRRES_POKE14,
	DPCOBJ_CHRRES_POKE15,
	DPCOBJ_CHRRES_POKE16,

	DPCOBJ_CHRRES_TB,						// タッチバー

	DPCOBJ_CHRRES_MAX
};

// パレットリソース
enum {
	DPCOBJ_PALRES_POKE01 = 0,		// ポケモン１
	DPCOBJ_PALRES_POKE02,
	DPCOBJ_PALRES_POKE03,
	DPCOBJ_PALRES_POKE04,
	DPCOBJ_PALRES_POKE05,
	DPCOBJ_PALRES_POKE06,

	DPCOBJ_PALRES_POKE11,				// ポケモン１（スワップ用）
	DPCOBJ_PALRES_POKE12,
	DPCOBJ_PALRES_POKE13,
	DPCOBJ_PALRES_POKE14,
	DPCOBJ_PALRES_POKE15,
	DPCOBJ_PALRES_POKE16,

	DPCOBJ_PALRES_TB,						// タッチバー

	DPCOBJ_PALRES_MAX
};

// セルリソース
enum {
	DPCOBJ_CELRES_POKE01 = 0,		// ポケモン１
	DPCOBJ_CELRES_POKE02,
	DPCOBJ_CELRES_POKE03,
	DPCOBJ_CELRES_POKE04,
	DPCOBJ_CELRES_POKE05,
	DPCOBJ_CELRES_POKE06,

	DPCOBJ_CELRES_POKE11,				// ポケモン１（スワップ用）
	DPCOBJ_CELRES_POKE12,
	DPCOBJ_CELRES_POKE13,
	DPCOBJ_CELRES_POKE14,
	DPCOBJ_CELRES_POKE15,
	DPCOBJ_CELRES_POKE16,

	DPCOBJ_CELRES_TB,						// タッチバー

	DPCOBJ_CELRES_MAX
};
