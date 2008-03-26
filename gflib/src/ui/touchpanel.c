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
	TPData* pTouchPanelBuff;					// バッファポインタ格納用
	u32		TouchPanelBuffSize;						// バッファサイズ格納用
	u32		TouchPanelSync;							// １フレームに何回サンプリングするか
	TPData	TouchPanelOneSync[ TP_ONE_SYNC_BUFF ];	// １フレームにサンプリングされたデータ格納用
	u32		TouchPanelNowBuffPoint;					// 今バッファにデータを入れているところ
	u16		TouchPanelSampFlag;						// サンプリングフラグ
	u16		TouchPanelExStop;						// サンプリング強制中止フラグ

    u16		tp_x;			///< タッチパネルX座標
	u16		tp_y;			///< タッチパネルY座標
	u16		tp_trg;			///< タッチパネル接触判定トリガ
	u16		tp_cont;		///< タッチパネル接触判定状態
    u8 tp_auto_samp;        ///< AUTOサンプリングを行うかどうか
};

static UI_TPSYS* _pUITP = NULL;   ///< 一個しか生成されないのでここでポインタ管理

//==============================================================
// Prototype
//==============================================================
static BOOL _circle_hitcheck( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y );
static BOOL _rect_hitcheck( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y );
static int _tblHitCheck( const GFL_UI_TP_HITTBL *tbl, const u16 x, const u16 y );
static u32 startSampling( UI_TPSYS* pTP, u32 sync );


//==============================================================================
/**
 * @brief   タッチパネルワークを得る
 * @param   heapID    ヒープ確保を行うID
 * @return  UI_TPSYS  タッチパネルシステムワーク
 */
//==============================================================================

static UI_TPSYS* _UI_GetTPSYS(const UISYS* pUI)
{
//    if(_pUITP==NULL){
//        OS_TPanic("no init");
//    }
    return _pUITP;
}

//==============================================================================
/**
 * @brief  タッチパネル初期化
 * @param   heapID    ヒープ確保を行うID
 * @return  none
 */
//==============================================================================

void GFL_UI_TP_Init(const HEAPID heapID)
{
	TPCalibrateParam calibrate;
    UI_TPSYS* pTP = GFL_HEAP_AllocMemory(heapID, sizeof(UI_TPSYS));

    MI_CpuClear8(pTP, sizeof(UI_TPSYS));
	// サンプリングフラグを初期化
	pTP->TouchPanelSampFlag = TP_SAMP_NONE;
	pTP->TouchPanelExStop = FALSE;
	// タッチパネルの初期化とキャリブレーションをセット
	TP_Init();

	// マシンのキャリブレーション値を取得
	if( TP_GetUserInfo( &calibrate ) == TRUE ){
		// キャリブレーション値の設定
		TP_SetCalibrateParam( &calibrate );
		GFL_UI_PRINT("Get Calibration Parameter from NVRAM\n");
	}
	else{
		// 取得に失敗したのでデフォルトのキャリブレーションの設定
		calibrate.x0 = 0x02ae;
		calibrate.y0 = 0x058c;
		calibrate.xDotSize = 0x0e25;
		calibrate.yDotSize = 0x1208;
		TP_SetCalibrateParam( &calibrate );
		GFL_UI_PRINT( "Warrning : TauchPanelInit( not found valid calibration data )\n" );
	}
    _pUITP = pTP;
//    return pTP;
}

//==============================================================================
/**
 * @brief タッチパネル読み取り処理
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  none
 */
//==============================================================================

static void GFI_UI_TP_Main(UISYS* pUI)
{
	TPData	tpTemp;
	TPData	tpDisp;
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);

    if(pTP==NULL){
        return;
    }
    
	// ふたが閉まっている場合は全ての入力をなしにする
	if(PAD_DetectFold()){
        MI_CpuClear8(pTP, sizeof(UI_TPSYS));
    }

	// タッチパネルデータを取得
	if(pTP->tp_auto_samp == 0){
		while( TP_RequestRawSampling( &tpTemp ) != 0 ){};	//サンプリングに成功するまで待つ
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
	pTP->tp_cont = tpDisp.touch;										// ベタ 入力
}

//==============================================================================
/**
 * @brief タッチパネル読み取り処理
 * @param   none
 * @return  none
 */
//==============================================================================

void GFL_UI_TP_Main(void)
{
    GFI_UI_TP_Main(_UI_GetUISYS());
}

//==============================================================================
/**
 * @brief タッチパネル終了処理
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  none
 */
//==============================================================================

static void GFI_UI_TP_Exit(UISYS* pUI)
{
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
    GFL_HEAP_FreeMemory(pTP);
}

//==============================================================================
/**
 * @brief タッチパネル終了処理
 * @param   none
 * @return  none
 */
//==============================================================================

void GFL_UI_TP_Exit(void)
{
    GFI_UI_TP_Exit(_UI_GetUISYS());
    _pUITP = NULL;
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
	if( ((u32)( x - (u32)tbl->rect.left) <= (u32)((u32)tbl->rect.right - (u32)tbl->rect.left))
	&	((u32)( y - (u32)tbl->rect.top) <= (u32)((u32)tbl->rect.bottom - (u32)tbl->rect.top))
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

    for(i = 0; tbl[i].circle.code != GFL_UI_TP_HIT_END; i++)
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
static int GFI_UI_TP_HitCont( const UISYS* pUI, const GFL_UI_TP_HITTBL *tbl )
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
    
	if( pTP->tp_cont ){
		return _tblHitCheck(tbl, pTP->tp_x, pTP->tp_y);
	}
	return GFL_UI_TP_HIT_NONE;
}


//------------------------------------------------------------------
/**
 * @brief 両タイプ（矩形・円形）を見ながら判定する（ベタ入力）
 * @param[in]   tbl		当たり判定テーブル（終端コードあり）
 * @return  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//------------------------------------------------------------------
int GFL_UI_TP_HitCont( const GFL_UI_TP_HITTBL *tbl )
{
    return GFI_UI_TP_HitCont(_UI_GetUISYS(), tbl);
}

//------------------------------------------------------------------
/**
 * @brief 両タイプ（矩形・円形）を見ながら判定する（トリガ入力）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[in]   tbl		当たり判定テーブル（終端コードあり）
 * @retval  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//------------------------------------------------------------------
static int GFI_UI_TP_HitTrg( const UISYS* pUI, const GFL_UI_TP_HITTBL *tbl )
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);

    if( pTP->tp_trg ){
		return _tblHitCheck(tbl, pTP->tp_x, pTP->tp_y);
	}
	return GFL_UI_TP_HIT_NONE;
}

//------------------------------------------------------------------
/**
 * @brief 両タイプ（矩形・円形）を見ながら判定する（トリガ入力）
 * @param[in]   tbl		当たり判定テーブル（終端コードあり）
 * @retval  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//------------------------------------------------------------------
int GFL_UI_TP_HitTrg( const GFL_UI_TP_HITTBL *tbl )
{
    return GFI_UI_TP_HitTrg(_UI_GetUISYS(), tbl);
}

//------------------------------------------------------------------
/**
 * @brief  タッチパネルに触れているか
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @retval  BOOL		TRUEで触れている
 */
//------------------------------------------------------------------
static BOOL GFI_UI_TP_GetCont( const UISYS* pUI )
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
    return pTP->tp_cont;
}

//------------------------------------------------------------------
/**
 * @brief  タッチパネルに触れているか
 * @param   none
 * @retval  BOOL		TRUEで触れている
 */
//------------------------------------------------------------------
BOOL GFL_UI_TP_GetCont( void )
{
    return GFI_UI_TP_GetCont(_UI_GetUISYS());
}

//------------------------------------------------------------------
/**
 * @brief タッチパネルに触れているか（トリガ）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @retval  BOOL		TRUEで触れた
 */
//------------------------------------------------------------------
static BOOL GFI_UI_TP_GetTrg( const UISYS* pUI )
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
	return pTP->tp_trg;
}

//------------------------------------------------------------------
/**
 * @brief タッチパネルに触れているか（トリガ）
 * @param   none
 * @retval  BOOL		TRUEで触れた
 */
//------------------------------------------------------------------
BOOL GFL_UI_TP_GetTrg( void )
{
    return GFI_UI_TP_GetTrg(_UI_GetUISYS());
}

//------------------------------------------------------------------
/**
 * @brief タッチパネルに触れているならその座標取得（ベタ入力）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[out]   x		Ｘ座標受け取り変数アドレス
 * @param[out]   y		Ｙ座標受け取り変数アドレス
 * @retval  TRUE  触れている
 * @retval  FALSE 触れていない。引数には何もしない。
 */
//------------------------------------------------------------------
static BOOL GFI_UI_TP_GetPointCont( const UISYS* pUI, u32* x, u32* y )
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
 * @brief タッチパネルに触れているならその座標取得（ベタ入力）
 * @param[out]   x		Ｘ座標受け取り変数アドレス
 * @param[out]   y		Ｙ座標受け取り変数アドレス
 * @retval  TRUE  触れている
 * @retval  FALSE 触れていない。引数には何もしない。
 */
//------------------------------------------------------------------
BOOL GFL_UI_TP_GetPointCont( u32* x, u32* y )
{
    return GFI_UI_TP_GetPointCont( _UI_GetUISYS(), x, y );
}

//------------------------------------------------------------------
/**
 * @brief   タッチパネルに触れているならその座標取得（トリガ入力）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[out]   x		Ｘ座標受け取り変数アドレス
 * @param[out]   y		Ｙ座標受け取り変数アドレス
 * @retval  TRUE  触れている
 * @retval  FALSE 触れていない。引数には何もしない。
 */
//------------------------------------------------------------------
static BOOL GFI_UI_TP_GetPointTrg( const UISYS* pUI, u32* x, u32* y )
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

//------------------------------------------------------------------
/**
 * @brief   タッチパネルに触れているならその座標取得（トリガ入力）
 * @param[out]   x		Ｘ座標受け取り変数アドレス
 * @param[out]   y		Ｙ座標受け取り変数アドレス
 * @retval  TRUE  触れている
 * @retval  FALSE 触れていない。引数には何もしない。
 */
//------------------------------------------------------------------
BOOL GFL_UI_TP_GetPointTrg( u32* x, u32* y )
{
    return GFI_UI_TP_GetPointTrg( _UI_GetUISYS(), x, y );
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
int GFL_UI_TP_HitSelf( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y )
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
static int GFI_UI_TP_GetAutoSamplingFlg(const UISYS* pUI)
{
    const UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
	return pTP->tp_auto_samp;
}

//==============================================================================
/**
 * @brief オートサンプリングを行っているかどうかを得る
 * @param   none
 * @retval  TRUE  オートサンプリングである
 * @retval  FALSE  していない
 */
//==============================================================================
int GFL_UI_TP_GetAutoSamplingFlg(void)
{
    return GFI_UI_TP_GetAutoSamplingFlg(_UI_GetUISYS());
}

//==============================================================================
/**
 * @brief オートサンプリング設定
 * @param[out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[in]    bAuto   オートサンプリングするならTRUE
 * @return  none
 */
//==============================================================================
static void GFI_UI_TP_SetAutoSamplingFlg(UISYS* pUI, const BOOL bAuto)
{
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
	pTP->tp_auto_samp = bAuto;
}

//==============================================================================
/**
 * @brief オートサンプリング設定
 * @param        none
 * @param[in]    bAuto   オートサンプリングするならTRUE
 * @return  none
 */
//==============================================================================
void GFL_UI_TP_SetAutoSamplingFlg(const BOOL bAuto)
{
    GFI_UI_TP_SetAutoSamplingFlg(_UI_GetUISYS(), bAuto);
}

//----------------------------------------------------------------------------
/**
 * @brief	サンプリング開始時のデータ設定
 * @param   pTP             ワーク
 * @param	SampFlag		サンプリングフラグ
 * @param	auto_samp		AUTOサンプリングフラグ
 * @param	pBuff			バッファポインタ
 * @param	buffSize		バッファサイズ
 * @param	nowBuffPoint	今サンプル中の位置
 * @param	Sync			シンク数
 * @return	none
 */
//-----------------------------------------------------------------------------
static void setStartBufferingParam( UI_TPSYS* pTP, u32 SampFlag, u32 auto_samp, void* pBuff, u32 buffSize, u32 nowBuffPoint, u32 Sync )
{
	int i;
    // サンプリングフラグをセット
	pTP->TouchPanelSampFlag = SampFlag;
    // システムのフラグをオートサンプリング中にする
    pTP->tp_auto_samp = auto_samp;
    // バッファのポインタとサイズを格納
	pTP->pTouchPanelBuff = pBuff;
	pTP->TouchPanelBuffSize = buffSize;
	pTP->TouchPanelSync = Sync;
	pTP->TouchPanelNowBuffPoint = nowBuffPoint;
//	oneSyncBuffClean( pTP->TouchPanelOneSync, TP_ONE_SYNC_BUFF );
	for( i = 0; i < TP_ONE_SYNC_BUFF; i++ ){
		pTP->TouchPanelOneSync[ i ].touch = 0;
	}
}

//----------------------------------------------------------------------------
/**
 * @brief	サンプリング停止
 * @param  pTP             ワーク
 * @retval TP_OK         停止した場合
 * @retval TP_FIFO_ERR   停止に失敗
 */
//-----------------------------------------------------------------------------
static u32 stopSampling( UI_TPSYS* pTP )
{
	u32 check_num = 0;			// ﾁｪｯｸ数
	u32 result;					// 結果
	int	i;						// ループ用

	// 現在のサンプリングを終了する
	// 正常に命令の転送が完了したか管理する
	//
	// 今サンプリングしているのかﾁｪｯｸ
    if( pTP->TouchPanelSampFlag == TP_SAMP_NONE ){
		// サンプリングしていないのでとまった状態です。
		return TP_OK;
	}

    if( TP_RequestAutoSamplingStop()!=0 ){
        GFL_UI_PRINT( "終了命令転送失敗\n" );
		return TP_FIFO_ERR;
	}
	return TP_OK;
}

//----------------------------------------------------------------------------
/**
 * @brief	生の座標を画面座標にして返す(TPData配列用)
 * @param	pData：変更するタッチパネルデータ配列
 * @param	size：配列サイズ
 * @return	none
 */
//-----------------------------------------------------------------------------
static void ChangeTPDataBuff( TPData* pData, u32 size )
{	
	int i;		// ループ用
	TPData sample;

	// 生のタッチパネル値から画面座標に変換して返す
	for( i = 0; i < size; i++ ){
		TP_GetCalibratedPoint( &sample, &(pData[ i ]) );
		pData[ i ] = sample;
	}
}

//----------------------------------------------------------------------------
/**
 * @brief	このシンクのサンプリング情報を取得
 * @param	pTP： タッチパネルAUTOサンプリングワーク
 * @param	pData：このシンクのデータ保存用領域のポインタ
 * @param	last_idx：最後にサンプリングした位置
 * @return	none
 */
//-----------------------------------------------------------------------------
static void getThisSyncData( UI_TPSYS* pTP, TP_ONE_DATA* pData, u32 last_idx )
{
	int i;				// ループ用
	s16	samp_work;		// サンプリング作業用

	//
	// 初期化
	//
	pData->Size = 0;
	for( i = 0; i < TP_ONE_SYNC_DATAMAX; i++ ){
		pData->TPDataTbl[ i ].validity = TP_VALIDITY_VALID;
		pData->TPDataTbl[ i ].touch = TP_TOUCH_OFF;
		pData->TPDataTbl[ i ].x = 0;
		pData->TPDataTbl[ i ].y = 0;
	}
	
	//
	// タッチパネルのサンプリング情報を管理し、今の状態を返す
	//
	// タッチパネルバッファとこのフレームのサンプリングデータ
	// に格納する
	for( i = 0; i < pTP->TouchPanelSync; i++ ){
		samp_work = last_idx - pTP->TouchPanelSync + i + 1;

		// 0以下のときは最終要素数にする
		if( samp_work < 0 ){
			samp_work += TP_ONE_SYNC_BUFF;
		}

		//
		// 有効無効データかチェック
		//
		if( pTP->TouchPanelOneSync[ samp_work ].validity == TP_VALIDITY_VALID ){	
			// このフレームのサンプリング情報を格納
			pData->TPDataTbl[ pData->Size ]  = pTP->TouchPanelOneSync[ samp_work ];
			pData->Size++;
		}
	}
}

//----------------------------------------------------------------------------
/**
 * @brief	差を求める abs?
 * @param	n1  位置
 * @param	n2  位置
 * @return	距離
 */
//-----------------------------------------------------------------------------
static inline int TouchPanel_GetDiff( int n1, int n2 )
{
	int calc = (n1 >= n2) ? (n1 - n2) : (n2 - n1);
	return calc;
}

//----------------------------------------------------------------------------
/**
 * @brief	バッファに圧縮して、座標を格納
 * @param	pTP： タッチパネルAUTOサンプリングワーク
 * @param	type		バッファリングタイプ
 * @param	last_idx	最後にサンプリングした位置
 * @param	comp_num	バッファに格納するときに、comp_num位のさがあったら格納する
 * @return	バッファに格納したサイズ
 */
//-----------------------------------------------------------------------------
static u32 mainBuffComp( UI_TPSYS* pTP, u32 type, u32 last_idx, u32 comp_num )
{
	int i;				// ループ用
	s32 dist_x;			// 差がどのくらいあるか
	s32 dist_y;			// 差がどのくらいあるか
	s16	samp_work;		// サンプリング作業用

	//
	// タッチパネルのサンプリング情報を管理し、今の状態を返す
	//
	// タッチパネルバッファとこのフレームのサンプリングデータ
	// に格納する
	for( i = 0; i < pTP->TouchPanelSync; i++ ){
		samp_work = last_idx - pTP->TouchPanelSync + i + 1;

		// 0以下のときは最終要素数にする
		if( samp_work < 0 ){
			samp_work += TP_ONE_SYNC_BUFF;
		}

		// 有効なデータの時バッファに格納
		if( (pTP->TouchPanelOneSync[ samp_work ].touch == TP_TOUCH_ON) &&
			(pTP->TouchPanelOneSync[ samp_work ].validity == TP_VALIDITY_VALID) ){
			// 差を求める
			dist_x = TouchPanel_GetDiff(pTP->pTouchPanelBuff[ pTP->TouchPanelNowBuffPoint - 1 ].x, pTP->TouchPanelOneSync[ samp_work ].x);
			dist_y = TouchPanel_GetDiff(pTP->pTouchPanelBuff[ pTP->TouchPanelNowBuffPoint - 1 ].y, pTP->TouchPanelOneSync[ samp_work ].y);
			
			// 差がcomp_num以上かチェック
			if( (dist_x >= comp_num) ||
				(dist_y >= comp_num)){
					
				// サンプリングデータ格納
				pTP->pTouchPanelBuff[ pTP->TouchPanelNowBuffPoint ] = pTP->TouchPanelOneSync[ samp_work ];				
				pTP->TouchPanelNowBuffPoint++;
				
				// バッファサイズオーバーチェック
				if(pTP->TouchPanelNowBuffPoint >= pTP->TouchPanelBuffSize){
					// バッファをループさせるかチェック
					if(type == TP_BUFFERING){
						pTP->TouchPanelNowBuffPoint %= pTP->TouchPanelBuffSize;	// ループさせる
					}else{
						return TP_END_BUFF;		// ループさせない
					}
				}
			}
		}
	}

	
	// 最後にサンプリングされたインデックスを返す	
	return pTP->TouchPanelNowBuffPoint;
}

//----------------------------------------------------------------------------
/**
 * @brief	バッファに有効データ、タッチの判定をせずに格納
 * @param	pTP： タッチパネルAUTOサンプリングワーク
 * @param	type		バッファリングタイプ
 * @param	last_idx	最後にサンプリングした位置
 * @return	バッファに格納したサイズ
 */
//-----------------------------------------------------------------------------
static u32 mainBuffJust( UI_TPSYS* pTP,u32 type, u32 last_idx )
{
	int i;				// ループ用
	s16	samp_work;		// サンプリング作業用

	//
	// タッチパネルのサンプリング情報を管理し、今の状態を返す
	//
	// タッチパネルバッファとこのフレームのサンプリングデータ
	// に格納する
	for( i = 0; i < pTP->TouchPanelSync; i++ ){
		samp_work = last_idx - pTP->TouchPanelSync + i + 1;

		// 0以下のときは最終要素数にする
		if( samp_work < 0 ){
			samp_work += TP_ONE_SYNC_BUFF;
		}

		// サンプリングデータ格納
		pTP->pTouchPanelBuff[ pTP->TouchPanelNowBuffPoint ] = pTP->TouchPanelOneSync[ samp_work ];
		pTP->TouchPanelNowBuffPoint++;

		// バッファサイズオーバーチェック
		if(pTP->TouchPanelNowBuffPoint >= pTP->TouchPanelBuffSize){
			// バッファをループさせるかチェック
			if(type == TP_BUFFERING_JUST){
				pTP->TouchPanelNowBuffPoint %= pTP->TouchPanelBuffSize;	// ループさせる
			}else{
				return TP_END_BUFF;		// ループさせない
			}
		}
	}

	
	// 最後にサンプリングされたインデックスを返す	
	return pTP->TouchPanelNowBuffPoint;
}

//----------------------------------------------------------------------------
/**
 *
 *@brief	サンプリング情報を管理し、今の状態を返す
 *			バッファリングモード
 *@param	pTP： タッチパネルAUTOサンプリングワーク
 *@param	type：サンプリング種別の番号
 *@param	last_idx：ラストインデックス
 *@param	comp_num：バッファに格納するときに、comp_num位のさがあったら格納する(圧縮モード時のみ)
 *@return	サンプリング種別による変化
					TP_BUFFERING---サンプリングされたバッファサイズ
					TP_NO_LOOP---サンプリングされたバッファサイズ
								 バッファが一杯になったとき TP_END_BUFF
					TP_NO_BUFF---TP_OK
 *
 */
//-----------------------------------------------------------------------------
static u32 modeBuff( UI_TPSYS* pTP, u32 type, u32 last_idx, u32 comp_num )
{		
	u32 ret;		// 戻り値
	// typeのモードでバッファリング
	switch( type ){
	case TP_BUFFERING:
	case TP_NO_LOOP:
		ret = mainBuffComp( pTP, type, last_idx, comp_num );
		break;
		
	case TP_BUFFERING_JUST:	// そのまま格納
	case TP_NO_LOOP_JUST:
		ret = mainBuffJust( pTP, type, last_idx );
		break;
	default:		// その他
		ret = TP_OK;
		break;			// なにもしない
	}
	return ret;
}

//----------------------------------------------------------------------------
/**
 *
 * @brief	サンプリング情報を管理し、今の状態を返す	
 * @param   pUI  UIワーク
 * @param	pData このフレームの情報(initで指定したサンプリング回数分の情報)
 * @param	type サンプリング種別の番号
 * @param	comp_num バッファに格納するときに、comp_num位のさがあったら格納する
 * @retval  TP_BUFFERING	サンプリングされたバッファサイズ
 * @retval  TP_NO_LOOP		サンプリングされたバッファサイズ
 *							バッファが一杯になったとき TP_END_BUFF
 * @retval  TP_NO_BUFF		TP_OK
 * @retval  TP_SAMP_NOT_START	サンプリング開始されていません
 */
//-----------------------------------------------------------------------------
static u32 GFI_UI_TP_AutoSamplingMain( UISYS* pUI, TP_ONE_DATA* pData, u32 type, u32 comp_num )
{
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
	u32 ret = TP_SAMP_NOT_START;	// 戻り値
	u32 last_idx;					// 最後にサンプリングされた位置

    if(pTP->tp_auto_samp!=TRUE){
        return ret;
    }
	// サンプリングされているかチェック
	if( pTP->TouchPanelSampFlag != TP_SAMP_NONE ){

		// 最後にサンプリングした位置を取得
		last_idx  = TP_GetLatestIndexInAuto();
		
		// 画面座標に変更
		ChangeTPDataBuff( pTP->TouchPanelOneSync, TP_ONE_SYNC_BUFF );

		// このフレームの情報を取得
		if( pData != NULL ){
			getThisSyncData( pTP, pData, last_idx );
		}
		
		// typeのバッファリングまたは今のフレーム情報を取得し
		// データをセットして返す
		// まずバッファリングありでInitされているかチェック
		if( pTP->TouchPanelSampFlag == TP_BUFFERING ){
			ret = modeBuff( pTP, type, last_idx, comp_num );	
		}else{
			// その他のときはTP_OKを返す
			ret = TP_OK;
		}
	}
    else{
        GFL_UI_PRINT("%s warning：サンプリング開始されていません\n",__FILE__);
	}
	return ret;
}

//----------------------------------------------------------------------------
/**
 *
 * @brief	サンプリング情報を管理し、今の状態を返す	
 * @param   pUI  UIワーク
 * @param	pData このフレームの情報(initで指定したサンプリング回数分の情報)
 * @param	type サンプリング種別の番号
 * @param	comp_num バッファに格納するときに、comp_num位のさがあったら格納する
 * @retval  TP_BUFFERING	サンプリングされたバッファサイズ
 * @retval  TP_NO_LOOP		サンプリングされたバッファサイズ
 *							バッファが一杯になったとき TP_END_BUFF
 * @retval  TP_NO_BUFF		TP_OK
 * @retval  TP_SAMP_NOT_START	サンプリング開始されていません
 */
//-----------------------------------------------------------------------------
u32 GFL_UI_TP_AutoSamplingMain( TP_ONE_DATA* pData, u32 type, u32 comp_num )
{
    return GFI_UI_TP_AutoSamplingMain( _UI_GetUISYS(), pData, type, comp_num );
}

//----------------------------------------------------------------------------
/**
 * @brief	スリープ処理後の再開処理
 * @param   pUI             UIのワーク
 * @return	none
 */
//-----------------------------------------------------------------------------
static void GFI_UI_TP_AutoSamplingReStart( UISYS* pUI )
{
	u32 result;
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);
	
    if(pTP==NULL){
        return;
    }
	// 停止中かチェック
	if( pTP->TouchPanelExStop == FALSE ){
		return ;
	}
	// サンプリング中かチェック
	if( pTP->TouchPanelSampFlag == TP_SAMP_NONE ){
		return ;
	}

	// サンプリングを開始させる
	result = startSampling( pTP, pTP->TouchPanelSync / 2 );	// /2は30シンクのため
	GF_ASSERT( result == TP_OK );

	pTP->TouchPanelExStop = FALSE;
}

//----------------------------------------------------------------------------
/**
 * @brief	スリープ処理後の再開処理
 * @param   pUI             UIのワーク
 * @return	none
 */
//-----------------------------------------------------------------------------
void GFL_UI_TP_AutoSamplingReStart( void )
{
    GFI_UI_TP_AutoSamplingReStart( _UI_GetUISYS() );
}

//----------------------------------------------------------------------------
/**
 * @brief	スリープ処理前の停止処理
 * @param   pUI             UIのワーク
 * @return  none
 */
//-----------------------------------------------------------------------------
static void GFI_UI_TP_AutoSamplingStop( UISYS* pUI )
{
	u32 result;
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);

    if(pTP==NULL){
        return;
    }
    // すでに停止中かチェック
	if( pTP->TouchPanelExStop == TRUE ){
		return ;
	}
	// サンプリング中かチェック
	if( pTP->TouchPanelSampFlag == TP_SAMP_NONE ){
		return ;
	}

	// サンプリングを停止させる
	result = stopSampling( pTP );
	GF_ASSERT( result == TP_OK );

	pTP->TouchPanelExStop = TRUE;
}

//----------------------------------------------------------------------------
/**
 * @brief	スリープ処理前の停止処理
 * @param   none
 * @return  none
 */
//-----------------------------------------------------------------------------
void GFL_UI_TP_AutoSamplingStop( void )
{
    GFI_UI_TP_AutoSamplingStop( _UI_GetUISYS() );
}

//----------------------------------------------------------------------------
/**
 * @brief	オートサンプリング開始命令を転送
 * @param	pTP： タッチパネルAUTOサンプリングワーク
 * @param	sync：１フレームのサンプリング数
 * @retval	TP_OK：成功
 * @retval	TP_FIFO_ERR：転送失敗
 */
//-----------------------------------------------------------------------------
static u32 startSampling( UI_TPSYS* pTP, u32 sync )
{
	u32	result;

    result = TP_RequestAutoSamplingStart(
        0,			// ベースVカウント
        sync,		// １フレームに何回サンプリングするか
        pTP->TouchPanelOneSync,				// サンプリングデータ格納用バッファ
        TP_ONE_SYNC_BUFF );		// バッファサイズ

	if( result != 0 ) {
		GFL_UI_PRINT( "スタート命令転送失敗\n" );
		return TP_FIFO_ERR;
	}
	return TP_OK;
}

//----------------------------------------------------------------------------
/**
 * @brief	タッチパネルのAUTOサンプリング開始
 * @param	pTP： TP workポインタ
 * @param	sync：１フレームに何回サンプリングするのか(MAX4)
 * @retval	TP_OK：成功
 * @retval	TP_FIFO_ERR：転送失敗
 * @retval	TP_ERR：転送以外の失敗
 */
//-----------------------------------------------------------------------------
static u32 _autoStart(UI_TPSYS* pTP, u32 sync)
{
	// スリープ停止中は拒否
	if( pTP->TouchPanelExStop == TRUE ){
		return TP_ERR;
	}
	// １フレームに５以上サンプリングしようとしたら停止
	if( sync > 4 ){
        OS_Panic("sync <= 4");
		return TP_ERR;
	}
	// サンプリング中なら抜ける
	if( pTP->TouchPanelSampFlag != TP_SAMP_NONE ){
		GFL_UI_PRINT("%s: サンプリング中です\n",__FILE__);
		return TP_ERR;
	}
	// サンプリング命令転送
	return startSampling( pTP, sync );
}

//----------------------------------------------------------------------------
/**
 * @brief	タッチパネルのAUTOサンプリング開始
 * @param	pUI  UIworkポインタ
 * @param	p_buff サンプリングデータを入れるバッファ
 * @param	size バッファのサイズ
 * @param	sync １フレームに何回サンプリングするのか(MAX4)
 * @retval	TP_OK 成功
 * @retval	TP_FIFO_ERR 転送失敗
 * @retval	TP_ERR 転送以外の失敗
 */
//-----------------------------------------------------------------------------
static u32 GFI_UI_TP_AutoStart(UISYS* pUI, TPData* p_buff, u32 size, u32 sync)
{
	u32	result;
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);

    if(pTP==NULL){
        return TP_ERR;
    }
    result = _autoStart(pTP, sync);
	// 転送に失敗したらエラーを返す
	if( result != TP_OK  ){
		return result;
	}
	// サンプリングデータ設定
	setStartBufferingParam( pTP, TP_BUFFERING, TRUE,
                            p_buff, size, 0,
                            sync * 2 ); // 30フレーム動作のため

	return TP_OK;
}

//----------------------------------------------------------------------------
/**
 * @brief	タッチパネルのAUTOサンプリング開始
 * @param	p_buff サンプリングデータを入れるバッファ
 * @param	size バッファのサイズ
 * @param	sync １フレームに何回サンプリングするのか(MAX4)
 * @retval	TP_OK 成功
 * @retval	TP_FIFO_ERR 転送失敗
 * @retval	TP_ERR 転送以外の失敗
 */
//-----------------------------------------------------------------------------
u32 GFL_UI_TP_AutoStart(TPData* p_buff, u32 size, u32 sync)
{
    return GFI_UI_TP_AutoStart(_UI_GetUISYS(), p_buff, size, sync);
}

//----------------------------------------------------------------------------
/**
 * @brief	タッチパネルのAUTOサンプリング開始	バッファリングなし
 * @param	pUI  UIworkポインタ
 * @param	sync １フレームに何回サンプリングするのか(MAX4)
 * @retval	TP_OK 成功
 * @retval	TP_FIFO_ERR 転送失敗
 * @retval	TP_ERR 転送以外の失敗
 */
//-----------------------------------------------------------------------------
static u32 GFI_UI_TP_AutoStartNoBuff(UISYS* pUI, u32 sync)
{
	u32	result;
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);

    result = _autoStart(pTP, sync);
    // 転送に失敗したらエラーを返す
	if( result != TP_OK ){
		return result;
	}
	// サンプリングデータ設定
	setStartBufferingParam( pTP, TP_NO_BUFF, TRUE,
			NULL, 0, 0,
			sync * 2 ); // 30フレーム動作のため

	return TP_OK;
}

//----------------------------------------------------------------------------
/**
 * @brief	タッチパネルのAUTOサンプリング開始	バッファリングなし
 * @param	pUI  UIworkポインタ
 * @param	sync １フレームに何回サンプリングするのか(MAX4)
 * @retval	TP_OK 成功
 * @retval	TP_FIFO_ERR 転送失敗
 * @retval	TP_ERR 転送以外の失敗
 */
//-----------------------------------------------------------------------------
u32 GFL_UI_TP_AutoStartNoBuff(u32 sync)
{
    return GFI_UI_TP_AutoStartNoBuff(_UI_GetUISYS(), sync);
}

//----------------------------------------------------------------------------
/**
 * @brief	サンプリングを終了する
 * @param	pUI UIworkポインタ
 * @retval	TP_OK：成功
 * @retval	TP_FIFO_ERR：転送失敗
 * @retval	TP_ERR：転送以外の失敗
 */
//-----------------------------------------------------------------------------
static u32 GFI_UI_TP_AutoStop( UISYS* pUI )
{
	u32 result;
    UI_TPSYS* pTP = _UI_GetTPSYS(pUI);

    if(pTP==NULL){
        return TP_ERR;
    }
	// スリープ停止中は拒否
	if( pTP->TouchPanelExStop == TRUE ){
		return TP_ERR;
	}
	
	result = stopSampling( pTP );
	if( result == TP_OK ){
		// 格納しておいたデータ破棄
		setStartBufferingParam( pTP, TP_SAMP_NONE, FALSE, NULL, 0, 0, 0 );
	}
	return result;
}

//----------------------------------------------------------------------------
/**
 * @brief	サンプリングを終了する
 * @retval	TP_OK：成功
 * @retval	TP_FIFO_ERR：転送失敗
 * @retval	TP_ERR：転送以外の失敗
 */
//-----------------------------------------------------------------------------
u32 GFL_UI_TP_AutoStop( void )
{
    return GFI_UI_TP_AutoStop( _UI_GetUISYS() );
}

