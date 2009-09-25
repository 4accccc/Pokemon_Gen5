//============================================================================================
/**
 * @file	point_sel.c
 * @brief	位置インデックス選択処理
 * @author	Hiroyuki Nakamura
 * @date	2004.11.12
 *
 *	モジュール名：POINTSEL
 */
//============================================================================================
#include <gflib.h>
#include "system/point_sel.h"


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
u8 POINTSEL_GetParam( const POINTSEL_WORK * pw, u8 prm )
{
	switch( prm ){
	case POINTSEL_PRM_PX:
		return pw->px;
	case POINTSEL_PRM_PY:
		return pw->py;
	case POINTSEL_PRM_SX:
		return pw->sx;
	case POINTSEL_PRM_SY:
		return pw->sy;
	case POINTSEL_PRM_UP:
		return pw->up;
	case POINTSEL_PRM_DOWN:
		return pw->down;
	case POINTSEL_PRM_LEFT:
		return pw->left;
	case POINTSEL_PRM_RIGHT:
		return pw->right;
	}
	return 0;
}

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
void POINTSEL_GetPos( const POINTSEL_WORK * pw, u8 * x, u8 * y )
{
	*x = pw->px;
	*y = pw->py;
}

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
void POINTSEL_GetSize( const POINTSEL_WORK * pw, u8 * x, u8 * y )
{
	*x = pw->sx;
	*y = pw->sy;
}

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
u8 POINTSEL_MoveVec(
		const POINTSEL_WORK * pw, u8 * px, u8 * py, u8 * sx, u8 * sy, u8 now, u8 mv )
{
	u8	next = now;

	switch( mv ){
	case POINTSEL_MV_UP:
		next = pw[now].up;
		break;
	case POINTSEL_MV_DOWN:
		next = pw[now].down;
		break;
	case POINTSEL_MV_LEFT:
		next = pw[now].left;
		break;
	case POINTSEL_MV_RIGHT:
		next = pw[now].right;
		break;
	}
	if( px != NULL ){ *px = pw[next].px; }
	if( py != NULL ){ *py = pw[next].py; }
	if( sx != NULL ){ *sx = pw[next].sx; }
	if( sy != NULL ){ *sy = pw[next].sy; }

	return next;
}

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
 * @li	GFL_UI_KEY_GetTrg()を使用
 */
//--------------------------------------------------------------------------------------------
u8 POINTSEL_MoveTrg( const POINTSEL_WORK * pw, u8 * px, u8 * py, u8 * sx, u8 * sy, u8 now )
{
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		return POINTSEL_MoveVec( pw, px, py, sx, sy, now, POINTSEL_MV_UP );
	}
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
		return POINTSEL_MoveVec( pw, px, py, sx, sy, now, POINTSEL_MV_DOWN );
	}
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
		return POINTSEL_MoveVec( pw, px, py, sx, sy, now, POINTSEL_MV_LEFT );
	}
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
		return POINTSEL_MoveVec( pw, px, py, sx, sy, now, POINTSEL_MV_RIGHT );
	}
	return POINTSEL_MOVE_NONE;
}

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
u8 POINTSEL_GetPosIndex( const POINTSEL_WORK * pw, u8 px, u8 py, u8 siz )
{
	u16	i;

	for( i=0; i<siz; i++ ){
		if( pw[i].px <= px && pw[i].px+pw[i].sx > px &&
			pw[i].py <= py && pw[i].py+pw[i].sy > py ){
			break;
		}
	}
	return (u8)i;
}
