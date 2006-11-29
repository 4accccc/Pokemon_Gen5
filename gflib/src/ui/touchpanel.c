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
 * @struct	_UI_TPSYS
 * @brief タッチパネルの情報の保持
  */
//------------------------------------------------------------------
struct _UI_TPSYS {
	u16		tp_x;			///< タッチパネルX座標
	u16		tp_y;			///< タッチパネルY座標
	u16		tp_trg;			///< タッチパネル接触判定トリガ
	u16		tp_cont;		///< タッチパネル接触判定状態
    u8 tp_auto_samp;        ///< AUTOサンプリングを行うかどうか
};


//==============================================================
// Prototype
//==============================================================
static BOOL _circle_hitcheck( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y );
static BOOL _rect_hitcheck( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y );
static int _tblHitCheck( const GFL_UI_TP_HITTBL *tbl, const u16 x, const u16 y );


//==============================================================================
/**
 * @brief  タッチパネル初期化
 * @param   heapID    ヒープ確保を行うID
 * @return  UI_TPSYS  タッチパネルシステムワーク
 */
//==============================================================================

UI_TPSYS* GFL_UI_TP_sysInit(const int heapID)
{
	TPCalibrateParam calibrate;
    UI_TPSYS* pTP = GFL_HEAP_AllocMemory(heapID, sizeof(UI_TPSYS));

    MI_CpuClear8(pTP, sizeof(UI_TPSYS));
	// タッチパネルの初期化とキャリブレーションをセット
	TP_Init();

	// マシンのキャリブレーション値を取得
	if( TP_GetUserInfo( &calibrate ) == TRUE ){
		// キャリブレーション値の設定
		TP_SetCalibrateParam( &calibrate );
		OS_Printf("Get Calibration Parameter from NVRAM\n");
	}
	else{
		// 取得に失敗したのでデフォルトのキャリブレーションの設定
		calibrate.x0 = 0x02ae;
		calibrate.y0 = 0x058c;
		calibrate.xDotSize = 0x0e25;
		calibrate.yDotSize = 0x1208;
		TP_SetCalibrateParam( &calibrate );
		OS_Printf( "Warrning : TauchPanelInit( not found valid calibration data )\n" );
	}
    return pTP;
}

//==============================================================================
/**
 * @brief タッチパネル読み取り処理
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  none
 */
//==============================================================================

void GFL_UI_TP_sysMain(UISYS* pUI)
{
	TPData	tpTemp;
	TPData	tpDisp;
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);

	// ふたが閉まっている場合は全ての入力をなしにする
	if(PAD_DetectFold()){
        MI_CpuClear8(pTP, sizeof(UI_TPSYS));
    }

	// タッチパネルデータを取得
	if(pTP->tp_auto_samp == 0){
//		while( TP_RequestRawSampling( &tpTemp ) != 0 ){};	//サンプリングに成功するまで待つ
	}else{
		TP_GetLatestRawPointInAuto( &tpTemp );	// オートサンプリング中のデータを取得
	}

	TP_GetCalibratedPoint( &tpDisp, &tpTemp );	// 座標を画面座標（０～２５５）にする

	
	if( tpDisp.validity == TP_VALIDITY_VALID  ){		// 座標の有効性をチェック
		// タッチパネル座標有効
		pTP->tp_x = tpDisp.x;
		pTP->tp_y = tpDisp.y;
	}else{
		// タッチパネル座標無効
		// 1シンク前の座標が格納されているときのみ座標をそのまま受け継ぐ
		if( pTP->tp_cont ){
			switch(tpDisp.validity){
			case TP_VALIDITY_INVALID_X:
				pTP->tp_y = tpDisp.y;
				break;
			case TP_VALIDITY_INVALID_Y:
				pTP->tp_x = tpDisp.x;
				break;
			case TP_VALIDITY_INVALID_XY:
				break;
			default:	// 正常
				break;
			}
		}else{
			// トリガのタイミングなら、
			// タッチパネルを押していないことにする
			tpDisp.touch = 0;
		}
	}
	pTP->tp_trg	= (u16)(tpDisp.touch & (tpDisp.touch ^ pTP->tp_cont));	// トリガ 入力
	pTP->tp_cont	= tpDisp.touch;										// ベタ 入力
}


//==============================================================================
/**
 * @brief タッチパネル終了処理
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  none
 */
//==============================================================================

void GFL_UI_TP_sysEnd(UISYS* pUI)
{
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
    GFL_HEAP_FreeMemory(pTP);
}

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

//==============================================================================
/**
 * @brief オートサンプリングを行っているかどうかを得る
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @retval  TRUE  オートサンプリングである
 * @retval  FALSE  していない
 */
//==============================================================================
int GFL_UI_TPGetAutoSamplingFlg(const UISYS* pUI)
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
	return pTP->tp_auto_samp;
}

//==============================================================================
/**
 * @brief オートサンプリング設定
 * @param[out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[in]    bAuto   オートサンプリングするならTRUE
 * @return  none
 */
//==============================================================================
void GFL_UI_TPSetAutoSamplingFlg(UISYS* pUI, const BOOL bAuto)
{
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
	pTP->tp_auto_samp = bAuto;
}





