//****************************************************************************
/**
 *@file		touchpanel.c
 *@brief	矩形とタッチパネルシステム情報との当たり判定処理
 *@author	taya  >  katsumi ohno
 *@date		2005.07.29
 */
//****************************************************************************

#include "gflib.h"

#include "ui.h"
#include "ui_def.h"


//------------------------------------------------------------------
/**
 * @struct	TPSYS 型宣言
 * タッチパネルの情報の保持
  */
//------------------------------------------------------------------
struct _UI_TPSYS {
	u16		tp_x;			///< タッチパネルX座標
	u16		tp_y;			///< タッチパネルY座標
	u16		tp_trg;			///< タッチパネル接触判定トリガ
	u16		tp_cont;		///< タッチパネル接触判定状態
};	// 8bytes


//==============================================================
// Prototype
//==============================================================
static BOOL _circle_hitcheck( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y );
static BOOL _rect_hitcheck( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y );
static int _tblHitCheck( const GFL_UI_TP_HITTBL *tbl, const u16 x, const u16 y );

//------------------------------------------------------------------
/**
 * @brief 円形としての当たり判定（単発）
 * @param[in]   tbl		当たり判定テーブル
 * @param[in]   x		X座標
 * @param[in]   y		Y座標
 * @retval  TRUE	当たり
 * @retval  FALSE	当たっていない
 */
//------------------------------------------------------------------
static BOOL _circle_hitcheck( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y )
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
 * @brief 矩形としての当たり判定（単発）
 * @param[in]   tbl		当たり判定テーブル
 * @param[in]   x		Ｘ座標
 * @param[in]   y		Ｙ座標
 * @retval  TRUE	当たり
 * @retval  FALSE	当たっていない
 */
//------------------------------------------------------------------
static BOOL _rect_hitcheck( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y )
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
 * @brief 矩形、円のあたり判定をテーブル分行う
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[in]   x		Ｘ座標
 * @param[in]   y		Ｙ座標
 * @return  テーブルindex  なければTP_HIT_NONE
 */
//------------------------------------------------------------------
static int _tblHitCheck( const GFL_UI_TP_HITTBL *tbl, const u16 x, const u16 y )
{
    int i;

    for(i = 0; i < tbl[i].circle.code != GFL_UI_TP_HIT_END; i++)
    {
        if( tbl[i].circle.code == GFL_UI_TP_USE_CIRCLE )
        {
            if( _circle_hitcheck( &tbl[i], x, y ) ){
                return i;
            }
        }
        else
        {
            if( _rect_hitcheck( &tbl[i], x, y ) ){
                return i;
            }
        }
    }
    return GFL_UI_TP_HIT_NONE;
}

//------------------------------------------------------------------
/**
 * @brief 両タイプ（矩形・円形）を見ながら判定する（ベタ入力）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[in]   tbl		当たり判定テーブル（終端コードあり）
 * @return  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//------------------------------------------------------------------
int GFL_UI_TouchPanelHitCont( const UISYS* pUI, const GFL_UI_TP_HITTBL *tbl )
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
    
	if( pTP->tp_cont ){
		return _tblHitCheck(tbl, pTP->tp_x, pTP->tp_y);
	}
	return GFL_UI_TP_HIT_NONE;
}
//------------------------------------------------------------------
/**
 * @brief 両タイプ（矩形・円形）を見ながら判定する（トリガ入力）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[in]   tbl		当たり判定テーブル（終端コードあり）
 * @retval  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//------------------------------------------------------------------
int GFL_UI_TouchPanelHitTrg( const UISYS* pUI, const GFL_UI_TP_HITTBL *tbl )
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);

    if( pTP->tp_trg ){
		return _tblHitCheck(tbl, pTP->tp_x, pTP->tp_y);
	}
	return GFL_UI_TP_HIT_NONE;
}

//------------------------------------------------------------------
/**
 * @brief  タッチパネルに触れているか
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @retval  BOOL		TRUEで触れている
 */
//------------------------------------------------------------------
BOOL GFL_UI_TouchPanelGetCont( const UISYS* pUI )
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
    return pTP->tp_cont;
}
//------------------------------------------------------------------
/**
 * @fn タッチパネルに触れているか（トリガ）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @retval  BOOL		TRUEで触れた
 */
//------------------------------------------------------------------
BOOL GFL_UI_TouchPanelGetTrg( const UISYS* pUI )
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
	return pTP->tp_trg;
}

//------------------------------------------------------------------
/**
 * @fn タッチパネルに触れているならその座標取得（ベタ入力）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[out]   x		Ｘ座標受け取り変数アドレス
 * @param[out]   y		Ｙ座標受け取り変数アドレス
 * @retval  TRUE  触れている
 * @retval  FALSE 触れていない。引数には何もしない。
 */
//------------------------------------------------------------------
BOOL GFL_UI_TouchPanelGetPointCont( const UISYS* pUI, u32* x, u32* y )
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);

    if( pTP->tp_cont )
	{
		*x = pTP->tp_x;
		*y = pTP->tp_y;
		return TRUE;
	}
	return FALSE;
}
//------------------------------------------------------------------
/**
 * @fn GFL_UI_TouchPanelGetPointTrg
 * @brief   タッチパネルに触れているならその座標取得（トリガ入力）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[out]   x		Ｘ座標受け取り変数アドレス
 * @param[out]   y		Ｙ座標受け取り変数アドレス
 * @retval  TRUE  触れている
 * @retval  FALSE 触れていない。引数には何もしない。
 */
//------------------------------------------------------------------
BOOL GFL_UI_TouchPanelGetPointTrg( const UISYS* pUI, u32* x, u32* y )
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);

    if( pTP->tp_trg )
	{
		*x = pTP->tp_x;
		*y = pTP->tp_y;
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	指定した座標で、当たり判定を行います。	複数
 *	@param	tbl		当たり判定テーブル（配列）
 *	@param	x		判定ｘ座標
 *	@param	y		判定ｙ座標
 *	@retval  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//-----------------------------------------------------------------------------
int GFL_UI_TouchPanelHitSelf( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y )
{
    return _tblHitCheck(tbl, x, y);
}

