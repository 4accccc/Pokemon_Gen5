//============================================================================================
/**
 * @file	point_sel.h
 * @brief	位置インデックス選択処理
 * @author	Hiroyuki Nakamura
 * @date	2004.11.12
 *
 *	モジュール名：POINTSEL
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================
// パラメータ取得用定義
enum {
/*
	POINT_WK_PX = 0,	// X座標
	POINT_WK_PY,		// Y座標
	POINT_WK_SX,		// Xサイズ
	POINT_WK_SY,		// Yサイズ
	POINT_WK_UP,		// 上方向へのインデックス番号
	POINT_WK_DOWN,		// 下方向へのインデックス番号
	POINT_WK_LEFT,		// 左方向へのインデックス番号
	POINT_WK_RIGHT		// 右方向へのインデックス番号
*/
	POINTSEL_PRM_PX = 0,	// X座標
	POINTSEL_PRM_PY,		// Y座標
	POINTSEL_PRM_SX,		// Xサイズ
	POINTSEL_PRM_SY,		// Yサイズ
	POINTSEL_PRM_UP,		// 上方向へのインデックス番号
	POINTSEL_PRM_DOWN,		// 下方向へのインデックス番号
	POINTSEL_PRM_LEFT,		// 左方向へのインデックス番号
	POINTSEL_PRM_RIGHT		// 右方向へのインデックス番号
};

// データ移動定義
enum {
/*
	POINT_MV_UP = 0,	// 上
	POINT_MV_DOWN,		// 下
	POINT_MV_LEFT,		// 右
	POINT_MV_RIGHT,		// 左
	POINT_MV_NULL		// ダミー
*/
	POINTSEL_MV_UP = 0,	// 上
	POINTSEL_MV_DOWN,		// 下
	POINTSEL_MV_LEFT,		// 右
	POINTSEL_MV_RIGHT,		// 左
	POINTSEL_MV_NULL		// ダミー
};

//#define	POINT_SEL_NOMOVE	( 0xff )	// 十字キー移動で十字キーが押されてない場合の返り値
#define	POINTSEL_MOVE_NONE	( 0xff )	// 十字キー移動で十字キーが押されてない場合の返り値

typedef struct {
	u8	px;			// X座標（キャラでもドットでも）
	u8	py;			// Y座標
	u8	sx;			// Xサイズ
	u8	sy;			// Yサイズ

	u8	up;			// 上方向へのインデックス番号
	u8	down;		// 下方向へのインデックス番号
	u8	left;		// 左方向へのインデックス番号
	u8	right;		// 右方向へのインデックス番号
//}POINTER_WORK;
}POINTSEL_WORK;


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * パラメータ取得
 *
 * @param	pw		ワーク
 * @param	prm		取得フラグ
 *
 * @return	指定パラメータ
 */
//--------------------------------------------------------------------------------------------
//extern u8 PointerWorkGet( const POINTSEL_WORK * pw, u8 prm );
extern u8 POINTSEL_GetParam( const POINTSEL_WORK * pw, u8 prm );

//--------------------------------------------------------------------------------------------
/**
 * 座標取得
 *
 * @param	pw		ワーク
 * @param	x		X座標格納場所
 * @param	y		Y座標格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void PointerWkPosGet( const POINTSEL_WORK * pw, u8 * x, u8 * y );
extern void POINTSEL_GetPos( const POINTSEL_WORK * pw, u8 * x, u8 * y );

//--------------------------------------------------------------------------------------------
/**
 * サイズ取得
 *
 * @param	pw		ワーク
 * @param	x		Xサイズ格納場所
 * @param	y		Yサイズ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void PointerWkSizeGet( const POINTSEL_WORK * pw, u8 * x, u8 * y );
extern void POINTSEL_GetSize( const POINTSEL_WORK * pw, u8 * x, u8 * y );

//--------------------------------------------------------------------------------------------
/**
 * 移動方向を指定し、座標、サイズ、インデックスを取得する
 *
 * @param	pw		ワーク
 * @param	px		X座標格納場所
 * @param	py		Y座標格納場所
 * @param	sx		Xサイズ格納場所
 * @param	sy		Yサイズ格納場所
 * @param	now		現在のインデックス
 * @param	mv		移動方向
 *
 * @return	移動先のインデックス
 */
//--------------------------------------------------------------------------------------------
//extern u8 PointerWkMoveSel(
extern u8 POINTSEL_MoveVec(
				const POINTSEL_WORK * pw, u8 * px, u8 * py, u8 * sx, u8 * sy, u8 now, u8 mv );

//--------------------------------------------------------------------------------------------
/**
 * 十字キーの情報から、座標、サイズ、インデックスを取得する
 *
 * @param	pw		ワーク
 * @param	px		X座標格納場所
 * @param	py		Y座標格納場所
 * @param	sx		Xサイズ格納場所
 * @param	sy		Yサイズ格納場所
 * @param	now		現在のインデックス
 *
 * @return	移動先のインデックス（十字キーが押されていない場合はPOINTSEL_MOVE_NONE）
 *
 * @li	sys.trgを使用
 */
//--------------------------------------------------------------------------------------------
//extern u8 PointerWkMoveSelTrg(
extern u8 POINTSEL_MoveTrg(
				const POINTSEL_WORK * pw, u8 * px, u8 * py, u8 * sx, u8 * sy, u8 now );

//--------------------------------------------------------------------------------------------
/**
 * 座標を指定し、インデックスを取得する
 *
 * @param	pw		ワーク
 * @param	px		X座標
 * @param	py		Y座標
 * @param	siz		データサイズ
 *
 * @return	指定座標のインデックス
 */
//--------------------------------------------------------------------------------------------
//extern u8 PointerWkMovePos( const POINTSEL_WORK * pw, u8 px, u8 py, u8 siz );
extern u8 POINTSEL_GetPosIndex( const POINTSEL_WORK * pw, u8 px, u8 py, u8 siz );
