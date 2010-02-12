//============================================================================================
/**
 * @file		zknsearch_obj_def.h
 * @brief		図鑑検索画面 ＯＢＪ関連定義
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZKNSEARCHOBJ
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================

#define	ZKNSEARCHOBJ_FORM_MAX		( 14 )		// フォルムＯＢＪ数


// OBJ Index
enum {
	ZKNSEARCHOBJ_IDX_SCROLL_BAR = 0,
	ZKNSEARCHOBJ_IDX_SCROLL_RAIL,

	ZKNSEARCHOBJ_IDX_TB_RETURN,
	ZKNSEARCHOBJ_IDX_TB_EXIT,
	ZKNSEARCHOBJ_IDX_TB_Y_BUTTON,
	ZKNSEARCHOBJ_IDX_TB_RIGHT,
	ZKNSEARCHOBJ_IDX_TB_LEFT,

	ZKNSEARCHOBJ_IDX_MARK1_M,
	ZKNSEARCHOBJ_IDX_MARK2_M,
	ZKNSEARCHOBJ_IDX_MARK1_S,
	ZKNSEARCHOBJ_IDX_MARK2_S,

	ZKNSEARCHOBJ_IDX_FORM_M,
	ZKNSEARCHOBJ_IDX_FORM_S = ZKNSEARCHOBJ_IDX_FORM_M+ZKNSEARCHOBJ_FORM_MAX,

	ZKNSEARCHOBJ_IDX_MAX = ZKNSEARCHOBJ_IDX_FORM_S+ZKNSEARCHOBJ_FORM_MAX,
};

// キャラリソースID
enum {
	ZKNSEARCHOBJ_CHRRES_TOUCH_BAR = 0,
	ZKNSEARCHOBJ_CHRRES_SCROLL_BAR,
	ZKNSEARCHOBJ_CHRRES_SCROLL_BAR_S,
	ZKNSEARCHOBJ_CHRRES_FORM_M,
	ZKNSEARCHOBJ_CHRRES_FORM_S,
	ZKNSEARCHOBJ_CHRRES_MAX
};

// パレットリソースID
enum {
	ZKNSEARCHOBJ_PALRES_TOUCH_BAR = 0,
	ZKNSEARCHOBJ_PALRES_SCROLL_BAR,
	ZKNSEARCHOBJ_PALRES_SCROLL_BAR_S,
	ZKNSEARCHOBJ_PALRES_FORM_M,
	ZKNSEARCHOBJ_PALRES_FORM_S,
	ZKNSEARCHOBJ_PALRES_MAX
};

// セルリソースID
enum {
	ZKNSEARCHOBJ_CELRES_TOUCH_BAR = 0,
	ZKNSEARCHOBJ_CELRES_SCROLL_BAR,
	ZKNSEARCHOBJ_CELRES_FORM,
	ZKNSEARCHOBJ_CELRES_MAX
};
