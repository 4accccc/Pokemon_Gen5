//==============================================================================
/**
 *@file		ui.h
 *@brief	ユーザーインターフェイス用公開関数 公開テーブル
 *@author	k.ohno
 *@date		2006.11.16
 */
//==============================================================================

#ifndef __UI_H__
#define	__UI_H__

#undef GLOBAL
#ifdef __UI_SYS_H_GLOBAL__
#define GLOBAL /*	*/
#else
#define GLOBAL extern
#endif

// デバッグ用決まり文句----------------------
#define GFL_UI_DEBUG   (0)   ///< ユーザーインターフェイスデバッグ用 0:無効 1:有効

#if defined(DEBUG_ONLY_FOR_ohno)
#undef GFL_UI_DEBUG
#define GFL_UI_DEBUG   (1)
#endif  //DEBUG_ONLY_FOR_ohno

#ifndef GFL_UI_PRINT
#if GFL_UI_DEBUG
#define GFL_UI_PRINT(...) \
  (void) ((OS_Printf(__VA_ARGS__)))
#else   //GFL_UI_DEBUG
#define GFL_UI_PRINT(...)           ((void) 0)
#endif  // GFL_UI_DEBUG
#endif  //GFL_UI_PRINT
// デバッグ用決まり文句----------------------


//-----------------------------------------------------------------------------
/**
 * @brief 定数宣言
 */
//-----------------------------------------------------------------------------

/**
 * @brief ソフトウエアリセット非許可用
 */
typedef enum {
    GFL_UI_SOFTRESET_WIFI = 0x01,   ///< WIFI通信時
    GFL_UI_SOFTRESET_SVLD = 0x02,   ///< セーブロード交換時
    GFL_UI_SOFTRESET_USER = 0x04,  ///< ここからは外部定義になる
} GF_UI_SOFTRESET_e;


/**
 * @brief スリープ非許可用
 */
typedef enum {
  GFL_UI_SLEEP_SVLD = 0x01,      ///< セーブロード
  GFL_UI_SLEEP_AGBROM = 0x02,    ///< AGBアクセス
  GFL_UI_SLEEP_NET = 0x04,       ///< 通信全般
  GFL_UI_SLEEP_USER = 0x08,      ///< ここからは外部定義になる
} GF_UI_SLEEP_e;


/**
 * @brief キーカスタマイズテーブル用
 */
typedef enum {
    GFL_UI_KEY_END = 0,     ///< テーブルの終了
    GFL_UI_KEY_COPY,     ///< １から２へキーをコピーします
    GFL_UI_KEY_CHANGE,             ///< １と２を交換します
    GFL_UI_KEY_RESET          ///< １を消します
} GF_UI_KEY_CUSTOM_e;

/**
 * @brief タッチパネルテーブル用
 */
typedef enum {
    GFL_UI_TP_HIT_END = 0xff,			///< テーブル終了コード
    GFL_UI_TP_USE_CIRCLE = 0xfe,			///< 円形として使う
    GFL_UI_TP_HIT_NONE = -1	///< あたったテーブルなし
} GF_UI_TP_CUSTOM_e;

//-----------------------------------------------------------------------------
/**
 *	@brief 構造体宣言
 */
//-----------------------------------------------------------------------------

/**
 * @struct GFL_UI_KEY_CUSTOM_TBL
 * @brief  キーデータ構造体  キーコンフィグパターンを指定するテーブル.
 * @note使用例
 <BR> 配列要素優先順位:0>1>2>3>4>.....です
 <BR> GFL_UI_KEY_CUSTOM_TBL key_data[] ={
 <BR>		{PAD_BUTTON_SELECT,PAD_BUTTON_START,GFL_UI_KEY_CHANGE},		//START SELECT 交換
 <BR>		{PAD_BUTTON_L,PAD_BUTTON_A,GFL_UI_KEY_COPY},		//LをA扱いに
 <BR>		{PAD_BUTTON_R,0,GFL_UI_KEY_RESET},		//LをA扱いに
 <BR>		{GFL_UI_KEY_END,0,0},		// 終了データ
 <BR> };
 */
typedef struct{
    int keySource;
    int keyDist;
    u8 mode;
}GFL_UI_KEY_CUSTOM_TBL;

/**
 * @struct GFL_UI_TP_HITTBL
 * @brief  タッチパネルあたり範囲を矩形、円形で指定するテーブル.
 * @note  使用例
 <BR>  配列要素優先順位:0>1>2>3>4>.....です 
 <BR>  GFL_UI_TP_HITTBL tp_data[] ={
 <BR>		{128,191,0,64},
 <BR>		{32,94,129,200},
 <BR>		{0,191,0,255},
 <BR>       {TP_USE_CIRCLE, 100, 80, 16 },	 円形として使う。
 <BR>		{TP_HIT_END,0,0,0},		 終了データ
 <BR>  };
 * 
 */
typedef union{
	struct {
		u8	top;	///< 上（もしくは特殊コード）
		u8	bottom;	///< 下
		u8	left;	///< 左
		u8	right;	///< 右
	}rect;

	struct {
		u8	code;	///< TP_USE_CIRCLE を指定
		u8	x;      ///< 中心x座標
		u8	y;      ///< 中心y座標
		u8	r;      ///< 半径
	}circle;
}GFL_UI_TP_HITTBL;

//------------------------------------------------------------------
/**
 * \var  UISYS
 * @brief	UISYS 型宣言
 * ユーザーインターフェイスの管理を保持する構造体
 * 内容は隠蔽
  */
//------------------------------------------------------------------
typedef struct _UI_SYS UISYS;


//==============================================================================
/**
 * @brief UI初期化
 * @param   heapID    ヒープ確保を行うID
 * @return  UISYS  workハンドル
 */
//==============================================================================
GLOBAL UISYS* GFL_UI_sysInit(const int heapID);

//==============================================================================
/**
 * @brief UIMain処理
 * @param   pUI    UISYS
 * @return  none
 */
//==============================================================================
GLOBAL void GFL_UI_sysMain(UISYS* pUI);

//==============================================================================
/**
 * @brief   UI終了処理
 * @param   pUI    UISYS
 * @return  none
 */
//==============================================================================

GLOBAL void GFL_UI_sysEnd(UISYS* pUI);

//------------------------------------------------------------------
/**
 * @brief   スリープ状態を禁止する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   sleepTypeBit スリープ管理BIT
 * @return  none
 */
//------------------------------------------------------------------
GLOBAL void GFL_UI_SleepDisable(UISYS* pUI,const u8 sleepTypeBit);

//------------------------------------------------------------------
/**
 * @brief   スリープ状態を許可する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   sleepTypeBit スリープ管理BIT
 * @return  none
 */
//------------------------------------------------------------------
GLOBAL void GFL_UI_SleepEnable(UISYS* pUI, const u8 sleepTypeBit);

//------------------------------------------------------------------
/**
 * @brief   ソフトウエアリセット状態を禁止する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   softResetBit リセット管理BIT
 * @return  none
 */
//------------------------------------------------------------------
GLOBAL void GFL_UI_SoftResetDisable(UISYS* pUI,const u8 softResetBit);

//------------------------------------------------------------------
/**
 * @brief   ソフトウエアリセット状態を許可する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   softResetBit リセット管理BIT
 * @return  none
 */
//------------------------------------------------------------------
GLOBAL void GFL_UI_SoftResetEnable(UISYS* pUI, const u8 softResetBit);


//----------------------------------------------------------------------------
//プロトタイプ宣言 touchpanel.c
//----------------------------------------------------------------------------


//==============================================================================
/**
 * @brief オートサンプリングを行っているかどうかを得る
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @retval  TRUE  オートサンプリングである
 * @retval  FALSE  していない
 */
//==============================================================================
GLOBAL int GFL_UI_TPGetAutoSamplingFlg(const UISYS* pUI);

//==============================================================================
/**
 * @brief オートサンプリング設定
 * @param[out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[in]    bAuto   オートサンプリングするならTRUE
 * @return  none
 */
//==============================================================================
GLOBAL void GFL_UI_TPSetAutoSamplingFlg(UISYS* pUI, const BOOL bAuto);

//------------------------------------------------------------------
/**
 * @brief 両タイプ（矩形・円形）を見ながら判定する（ベタ入力）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[in]   tbl		当たり判定テーブル（終端コードあり）
 * @return  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//------------------------------------------------------------------
GLOBAL int GFL_UI_TouchPanelHitCont( const UISYS* pUI, const GFL_UI_TP_HITTBL *tbl );

//------------------------------------------------------------------
/**
 * @brief 両タイプ（矩形・円形）を見ながら判定する（トリガ入力）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[in]   tbl		当たり判定テーブル（終端コードあり）
 * @retval  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//------------------------------------------------------------------
GLOBAL int GFL_UI_TouchPanelHitTrg( const UISYS* pUI, const GFL_UI_TP_HITTBL *tbl );

//------------------------------------------------------------------
/**
 * @brief  タッチパネルに触れているか
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @retval  BOOL		TRUEで触れている
 */
//------------------------------------------------------------------
GLOBAL BOOL GFL_UI_TouchPanelGetCont( const UISYS* pUI );

//------------------------------------------------------------------
/**
 * @brief タッチパネルに触れているか（トリガ）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @retval  BOOL		TRUEで触れた
 */
//------------------------------------------------------------------
GLOBAL BOOL GFL_UI_TouchPanelGetTrg( const UISYS* pUI );

//----------------------------------------------------------------------------
/**
 *	@brief	指定した座標で、当たり判定を行います。	複数
 *	@param	tbl		当たり判定テーブル（配列）
 *	@param	x		判定ｘ座標
 *	@param	y		判定ｙ座標
 *	@retval  int		当たりがあればその要素番号、なければ TP_HIT_NONE
 */
//-----------------------------------------------------------------------------
GLOBAL int GFL_UI_TouchPanelHitSelf( const GFL_UI_TP_HITTBL *tbl, u32 x, u32 y );

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
GLOBAL BOOL GFL_UI_TouchPanelGetPointCont( const UISYS* pUI, u32* x, u32* y );

//------------------------------------------------------------------
/**
 * @brief タッチパネルに触れているならその座標取得（トリガ入力）
 * @param[in]   pUI	    ユーザーインターフェイスシステム
 * @param[out]   x		Ｘ座標受け取り変数アドレス
 * @param[out]   y		Ｙ座標受け取り変数アドレス
 * @retval  TRUE  触れている
 * @retval  FALSE 触れていない。引数には何もしない。
 */
//------------------------------------------------------------------
GLOBAL BOOL GFL_UI_TouchPanelGetPointTrg( const UISYS* pUI, u32* x, u32* y );

//----------------------------------------------------------------------------
//プロトタイプ宣言 key.c
//----------------------------------------------------------------------------

//==============================================================================
/**
 * @brief キーリピートの速度とウェイトをセット
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[in]	speed	速度
 * @param[in]	wait	ウェイト
 * @return  none
 */
//==============================================================================
GLOBAL void GFL_UI_KeySetRepeatSpeed(UISYS* pUI, const int speed, const int wait );

//==============================================================================
/**
 * @brief キーコンフィグテーブルを登録
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[in]	pTbl   コントロールテーブル配列のポインタ
 * @return  none
 */
//==============================================================================
GLOBAL void GFL_UI_KeySetControlModeTbl(UISYS* pUI, const GFL_UI_KEY_CUSTOM_TBL* pTbl );

//==============================================================================
/**
 * @brief キートリガゲット
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  キートリガ
 */
//==============================================================================
GLOBAL GFL_UI_KeyGetTrg( const UISYS* pUI );

//==============================================================================
/**
 * @brief キーコントゲット
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  キーコント
 */
//==============================================================================
GLOBAL GFL_UI_KeyGetCont( const UISYS* pUI );

//==============================================================================
/**
 * @brief キーリピートゲット
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  キーコント
 */
//==============================================================================
GLOBAL GFL_UI_KeyGetRepeat( const UISYS* pUI );

//==============================================================================
/**
 * @brief キーリピートの速度とウェイトをゲット
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[out]	speed	速度
 * @param[out]	wait	ウェイト
 * @return  none
 */
//==============================================================================
GLOBAL void GFL_UI_KeyGetRepeatSpeed(const UISYS* pUI, int* speed, int* wait );

//==============================================================================
/**
 * @brief   キーコンフィグのモードの値を設定する
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[in]   mode    キーコンフィグモード
 * @return  none
 */
//==============================================================================
GLOBAL void GFL_UI_KeySetControlMode(UISYS* pUI,const int mode);

//==============================================================================
/**
 * @brief キーコンフィグのモードの値を得る
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  コントロールモード
 */
//==============================================================================
GLOBAL int GFL_UI_KeyGetControlMode(const UISYS* pUI);




#undef GLOBAL


#endif //__UI_H__

