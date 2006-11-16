//****************************************************************************
/**
 *
 *@file		touchpanel.c
 *@brief	矩形とタッチパネルシステム情報との当たり判定処理
 *@author	taya  >  katsumi ohno
 *@data		2005.07.29
 *
 */
//****************************************************************************

#include "gflib.h"

#define __TOUCHPANEL_H_GLOBAL__
#include "touchpanel.h"

//------------------------------------------------------------------
/**
 * @brief	TPSYS 型宣言
 * タッチパネルの情報の保持
  */
//------------------------------------------------------------------
struct _TPSYS {
	u16		tp_x;			///< タッチパネルX座標
	u16		tp_y;			///< タッチパネルY座標
	u16		tp_trg;			///< タッチパネル接触判定トリガ
	u16		tp_cont;		///< タッチパネル接触判定状態
};	// 8bytes


//==============================================================
// Prototype
//==============================================================
static BOOL circle_hitcheck( const TP_HIT_TBL *tbl, u32 x, u32 y );
static BOOL rect_hitcheck( const TP_HIT_TBL *tbl, u32 x, u32 y );

//------------------------------------------------------------------
/**
 * 円形としての当たり判定（単発）
 *
 * @param   tbl		当たり判定テーブル
 * @param   x		X座標
 * @param   y		Y座標
 *
 * @retval  BOOL	TRUEで当たり
 */
//------------------------------------------------------------------
static BOOL circle_hitcheck( const TP_HIT_TBL *tbl, u32 x, u32 y )
{
	x = (tbl->circle.x - x) * (tbl->circle.x - x);
	y = (tbl->circle.y - y) * (tbl->circle.y - y);

	if( x + y < (tbl->circle.r * tbl->circle.r) )
	{
		return TRUE;
	}

	return FALSE;
}
//------------------------------------------------------------------
/**
 * 矩形としての当たり判定（単発）
 *
 * @param   tbl		当たり判定テーブル
 * @param   x		Ｘ座標
 * @param   y		Ｙ座標
 *
 * @retval  BOOL		TRUEで当たり
 */
//------------------------------------------------------------------
static BOOL rect_hitcheck( const TP_HIT_TBL *tbl, u32 x, u32 y )
{
	if( ((u32)( x - tbl->rect.left) < (u32)(tbl->rect.right - tbl->rect.left))
	&	((u32)( y - tbl->rect.top) < (u32)(tbl->rect.bottom - tbl->rect.top))
	){
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 * 両タイプ（矩形・円形）を見ながら判定する（ベタ入力）
 *
 * @param   tpsys	タッチパネルシステム
 * @param   tbl		当たり判定テーブル（終端コードあり）
 *
 * @retval  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//------------------------------------------------------------------
int GF_TP_HitCont( const TPSYS* tpsys, const TP_HIT_TBL *tbl )
{
	if( tpsys->tp_cont ){
		int i;

		for(i = 0; i < tbl[i].circle.code != TP_HIT_END; i++)
		{
			if( tbl[i].circle.code == TP_USE_CIRCLE )
			{
				if( circle_hitcheck( &tbl[i], tpsys->tp_x, tpsys->tp_y ) ){
					return i;
				}
			}
			else
			{
				if( rect_hitcheck( &tbl[i], tpsys->tp_x, tpsys->tp_y ) ){
					return i;
				}
			}
		}
	}
	return TP_HIT_NONE;
}
//------------------------------------------------------------------
/**
 * 両タイプ（矩形・円形）を見ながら判定する（トリガ入力）
 *
 * @param   tbl		当たり判定テーブル（終端コードあり）
 *
 * @retval  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//------------------------------------------------------------------
int GF_TP_HitTrg( const TPSYS* tpsys, const TP_HIT_TBL *tbl )
{
	if( tpsys->tp_trg ){
		int i;

		for(i = 0; i < tbl[i].circle.code != TP_HIT_END; i++)
		{
			if( tbl[i].circle.code == TP_USE_CIRCLE )
			{
				if( circle_hitcheck( &tbl[i], tpsys->tp_x, tpsys->tp_y ) ){
					return i;
				}
			}
			else
			{
				if( rect_hitcheck( &tbl[i], tpsys->tp_x, tpsys->tp_y ) ){
					return i;
				}
			}
		}
	}
	return TP_HIT_NONE;
}

//------------------------------------------------------------------
/**
 * タッチパネルに触れているか
 *
 * @retval  BOOL		TRUEで触れている
 */
//------------------------------------------------------------------
BOOL GF_TP_GetCont( const TPSYS* tpsys )
{
	return tpsys->tp_cont;
}
//------------------------------------------------------------------
/**
 * タッチパネルに触れているか（トリガ）
 *
 * @retval  BOOL		TRUEで触れた
 */
//------------------------------------------------------------------
BOOL GF_TP_GetTrg( const TPSYS* tpsys )
{
	return tpsys->tp_trg;
}

//------------------------------------------------------------------
/**
 * タッチパネルに触れているならその座標取得（ベタ入力）
 *
 * @param   x		Ｘ座標受け取り変数アドレス
 * @param   y		Ｙ座標受け取り変数アドレス
 *
 * @retval  BOOL	TRUEで触れている。FALSEが返った場合、引数には何もしない。
 */
//------------------------------------------------------------------
BOOL GF_TP_GetPointCont( const TPSYS* tpsys, u32* x, u32* y )
{
	if( tpsys->tp_cont )
	{
		*x = tpsys->tp_x;
		*y = tpsys->tp_y;
		return TRUE;
	}
	return FALSE;
}
//------------------------------------------------------------------
/**
 * タッチパネルに触れているならその座標取得（トリガ入力）
 *
 * @param   x		Ｘ座標受け取り変数アドレス
 * @param   y		Ｙ座標受け取り変数アドレス
 *
 * @retval  BOOL	TRUEで触れている。FALSEが返った場合、引数には何もしない。
 */
//------------------------------------------------------------------
BOOL GF_TP_GetPointTrg( const TPSYS* tpsys, u32* x, u32* y )
{
	if( tpsys->tp_trg )
	{
		*x = tpsys->tp_x;
		*y = tpsys->tp_y;
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *
 *	@brief	指定した座標で、当たり判定を行います。	複数
 *
 *	@param	tbl		当たり判定テーブル（配列）
 *	@param	x		判定ｘ座標
 *	@param	y		判定ｙ座標
 *
 *	@retval  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 *
 *
 */
//-----------------------------------------------------------------------------
int GF_TP_HitSelf( const TP_HIT_TBL *tbl, u32 x, u32 y )
{
	int i;
	
	for(i = 0; i < tbl[i].circle.code != TP_HIT_END; i++)
	{
		if( tbl[i].circle.code == TP_USE_CIRCLE )
		{
			if( circle_hitcheck( &tbl[i], x, y ) ){
				return i;
			}
		}
		else
		{
			if( rect_hitcheck( &tbl[i], x, y ) ){
				return i;
			}
		}
	}
	return TP_HIT_NONE;
}

