//==============================================================================
/**
 * @file	key.c
 * @brief	ユーザーインターフェイス キーの管理
            デバイス＆アクセス関数
 * @author	k.ohno
 * @date	2006.11.17
 */
//==============================================================================

#include "gflib.h" 

#include "ui.h"
#include "ui_def.h"

//==============================================================================
//
//			定義、関数宣言
//
//==============================================================================

static void _keyConvert(UI_KEYSYS* pKey);

//------------------------------------------------------------------
/**
 * @struct	_UIKeySys
 * @brief キー情報の保持
  */
//------------------------------------------------------------------
struct _UI_KEYSYS {
    const GFL_UI_KEY_CUSTOM_TBL* pControl;  ///< キーモードを設定するテーブル
	int control_mode;	///< キーコントロールモード
    int control_tblno;  ///< コントロールテーブルの場所
	int cont_org;			///< キー状態
	int trg_org;			///< キートリガ
	int repeat_org;			///< キーリピート
	int cont;				///< キー状態(ゲーム設定により変換処理が入る)
	int trg;				///< キートリガ(ゲーム設定により変換処理が入る)
	int repeat;				///< キーリピート(ゲーム設定により変換処理が入る)
	int repeatWait_Count;	///< キーリピートカウンタ
	int repeatSpeed;		///< キーリピートスピード
	int repeatWait;			///< キーリピートウェイト
};

//==============================================================================
//
//			関数
//
//==============================================================================

//==============================================================================
/**
 * @brief  パッド初期化
 * @param   heapID    ヒープ確保を行うID
 * @return  UI_KEYSYS  キーシステムワーク
 */
//==============================================================================

UI_KEYSYS* GFL_UI_Key_sysInit(const int heapID)
{
    UI_KEYSYS* pKey = GFL_HEAP_AllocMemory(heapID, sizeof(UI_KEYSYS));

    MI_CpuClear8(pKey, sizeof(UI_KEYSYS));
	pKey->repeatSpeed = 8;
	pKey->repeatWait = 15;
    return pKey;
}

//==============================================================================
/**
 * @brief パッド読み取り処理
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  none
 */
//==============================================================================

static void GFI_UI_Key_sysMain(UISYS* pUI)
{
	TPData	tpTemp;
	TPData	tpDisp;
	int	keyData;
    UI_KEYSYS* pKey = _UI_GetKEYSYS(pUI);

	// ふたが閉まっている場合は全ての入力をなしにする
	if(PAD_DetectFold()){
	  pKey->trg	= 0;
	  pKey->cont	= 0;
	  pKey->repeat	= 0;
	  return;
	}

	keyData = PAD_Read();
	
	pKey->trg_org = keyData & (keyData ^ pKey->cont_org);	// トリガ 入力
    pKey->repeat_org = keyData & (keyData ^ pKey->cont_org);        // リピート 入力

    if((keyData!=0) && (pKey->cont_org==keyData)){
		if(--pKey->repeatWait_Count==0){
			pKey->repeat_org = keyData;
			pKey->repeatWait_Count = pKey->repeatSpeed;
		}
	}
	else{
		pKey->repeatWait_Count = pKey->repeatWait;
	}
	pKey->cont_org	= keyData;							// ベタ 入力

	pKey->trg = pKey->trg_org;
	pKey->cont = pKey->cont_org;
	pKey->repeat = pKey->repeat_org;

	_keyConvert(pKey);
}

//==============================================================================
/**
 * @brief パッド読み取り処理
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  none
 */
//==============================================================================

void GFL_UI_Key_sysMain(void)
{
    GFI_UI_Key_sysMain(_UI_GetUISYS());
}

//==============================================================================
/**
 * @brief パッド終了処理
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  none
 */
//==============================================================================

static void GFI_UI_Key_sysEnd(UISYS* pUI)
{
    UI_KEYSYS* pKey = _UI_GetKEYSYS(pUI);
    GFL_HEAP_FreeMemory(pKey);
}

//==============================================================================
/**
 * @brief パッド終了処理
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  none
 */
//==============================================================================

void GFL_UI_Key_sysEnd(void)
{
    GFI_UI_Key_sysEnd(_UI_GetUISYS());
}

//==============================================================================
// キー情報の変換マクロ
//==============================================================================
#define KEYCONV( key, check_pat, set_pat )	{ if( key & check_pat ) key |= set_pat; }
#define KEYCONV_RESET( key, check_pat, set_pat ) {	\
	if( key & check_pat ){							\
		key |= set_pat;								\
		key &= ( check_pat^0xffff );				\
	}												\
}
#define KEYCONV_EXCHANGE( key, pat1, pat2 ) {	\
	int	tmp = 0;								\
	if( key & pat1 ){							\
		tmp |= pat2;							\
	}											\
	if( key & pat2 ){							\
		tmp |= pat1;							\
	}											\
	key &= ( (pat1 | pat2 )^0xffff );			\
	key |= tmp;									\
}
#define KEY_RESET( key, reset_pat ) {			\
	key &= ( reset_pat^0xffff );				\
}

//==============================================================================
/**
 * @brief キー情報の変換
 * @param[in,out]   pKey     ユーザーインターフェイスキーハンドルのポインタ
 * @return  none
 */
//==============================================================================
static void _keyConvert(UI_KEYSYS* pKey)
{
    const GFL_UI_KEY_CUSTOM_TBL* pTbl;
    if(pKey->pControl==NULL){
        return;
    }
    pTbl = &pKey->pControl[pKey->control_tblno];

    while(1){
        switch(pTbl->mode){
          case GFL_UI_KEY_END:
            return;
          case GFL_UI_KEY_COPY:
            KEYCONV( pKey->trg, pTbl->keySource, pTbl->keyDist );
            KEYCONV( pKey->cont, pTbl->keySource, pTbl->keyDist );
            KEYCONV( pKey->repeat, pTbl->keySource, pTbl->keyDist );
            break;
          case GFL_UI_KEY_CHANGE:
            KEYCONV_EXCHANGE( pKey->trg, pTbl->keySource, pTbl->keyDist );
            KEYCONV_EXCHANGE( pKey->cont, pTbl->keySource, pTbl->keyDist );
            KEYCONV_EXCHANGE( pKey->repeat, pTbl->keySource, pTbl->keyDist );
            break;
          case GFL_UI_KEY_RESET:
            KEY_RESET( pKey->trg, pTbl->keySource );
            KEY_RESET( pKey->cont, pTbl->keySource );
            KEY_RESET( pKey->repeat, pTbl->keySource );
            break;
        }
        pTbl++;
    }
    
#if 0
    switch( pKey->control_mode ){
	case KEYCONTROL_NORMAL:
		break;
	case KEYCONTROL_PATTERN1:

		KEYCONV( pKey->trg, PAD_BUTTON_START, PAD_BUTTON_X )
		KEYCONV( pKey->cont, PAD_BUTTON_START, PAD_BUTTON_X )
		KEYCONV( pKey->repeat, PAD_BUTTON_START, PAD_BUTTON_X )
		break;

	case KEYCONTROL_PATTERN2:

		KEYCONV_EXCHANGE( pKey->trg, PAD_BUTTON_X,PAD_BUTTON_Y )
		KEYCONV_EXCHANGE( pKey->cont, PAD_BUTTON_X,PAD_BUTTON_Y )
		KEYCONV_EXCHANGE( pKey->repeat, PAD_BUTTON_X,PAD_BUTTON_Y )
		break;

	case KEYCONTROL_PATTERN3:

		KEYCONV( pKey->trg, PAD_BUTTON_L, PAD_BUTTON_A )
		KEYCONV( pKey->cont, PAD_BUTTON_L, PAD_BUTTON_A )
		KEYCONV( pKey->repeat, PAD_BUTTON_L, PAD_BUTTON_A )

		KEY_RESET( pKey->trg, ( PAD_BUTTON_L | PAD_BUTTON_R ) )
		KEY_RESET( pKey->cont, ( PAD_BUTTON_L | PAD_BUTTON_R ) )
		KEY_RESET( pKey->repeat, ( PAD_BUTTON_L | PAD_BUTTON_R ) )
		break;
	}
#endif
}

//==============================================================================
/**
 * @brief キーリピートの速度とウェイトをセット
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[in]	speed	速度
 * @param[in]	wait	ウェイト
 * @return  none
 */
//==============================================================================
void GFI_UI_KeySetRepeatSpeed(UISYS* pUI, const int speed, const int wait )
{
    UI_KEYSYS* pKey = _UI_GetKEYSYS(pUI);
    pKey->repeatSpeed	= speed;
	pKey->repeatWait	= wait;
}

//==============================================================================
/**
 * @brief キーリピートの速度とウェイトをセット
 * @param[in]	speed	速度
 * @param[in]	wait	ウェイト
 * @return  none
 */
//==============================================================================
void GFL_UI_KeySetRepeatSpeed(const int speed, const int wait )
{
    GFI_UI_KeySetRepeatSpeed(_UI_GetUISYS(),speed,wait );
}

//==============================================================================
/**
 * @brief キーコンフィグテーブルを設定
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[in]	pTbl   コントロールテーブル配列のポインタ
 * @return  none
 */
//==============================================================================
void GFI_UI_KeySetControlModeTbl(UISYS* pUI, const GFL_UI_KEY_CUSTOM_TBL* pTbl )
{
    UI_KEYSYS* pKey = _UI_GetKEYSYS(pUI);
	pKey->pControl = pTbl;
}

//==============================================================================
/**
 * @brief キーコンフィグテーブルを設定
 * @param[in]	pTbl   コントロールテーブル配列のポインタ
 * @return  none
 */
//==============================================================================
void GFL_UI_KeySetControlModeTbl(const GFL_UI_KEY_CUSTOM_TBL* pTbl )
{
    GFI_UI_KeySetControlModeTbl(_UI_GetUISYS(),pTbl);
}

//==============================================================================
/**
 * @brief キートリガゲット
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  キートリガ
 */
//==============================================================================
int GFI_UI_KeyGetTrg( const UISYS* pUI )
{
    UI_KEYSYS* pKey = _UI_GetKEYSYS(pUI);
	return pKey->trg;
}

//==============================================================================
/**
 * @brief キートリガゲット
 * @param  none
 * @return  キートリガ
 */
//==============================================================================
int GFL_UI_KeyGetTrg( void )
{
    return GFI_UI_KeyGetTrg(_UI_GetUISYS());
}

//==============================================================================
/**
 * @brief キーコントゲット
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  キーコント
 */
//==============================================================================
int GFI_UI_KeyGetCont( const UISYS* pUI )
{
    UI_KEYSYS* pKey = _UI_GetKEYSYS(pUI);
	return pKey->cont;
}

//==============================================================================
/**
 * @brief キーコントゲット
 * @param   none
 * @return  キーコント
 */
//==============================================================================
int GFL_UI_KeyGetCont( void )
{
     return GFI_UI_KeyGetCont( _UI_GetUISYS() );
}

//==============================================================================
/**
 * @brief キーリピートゲット
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  キーコント
 */
//==============================================================================
int GFI_UI_KeyGetRepeat( const UISYS* pUI )
{
    UI_KEYSYS* pKey = _UI_GetKEYSYS(pUI);
	return pKey->repeat;
}

//==============================================================================
/**
 * @brief キーリピートゲット
 * @param none
 * @return  キーコント
 */
//==============================================================================
int GFL_UI_KeyGetRepeat(void )
{
    return GFI_UI_KeyGetRepeat( _UI_GetUISYS());
}

//==============================================================================
/**
 * @brief キーリピートの速度とウェイトをゲット
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[out]	speed	速度
 * @param[out]	wait	ウェイト
 * @return  none
 */
//==============================================================================
void GFI_UI_KeyGetRepeatSpeed(const UISYS* pUI, int* speed, int* wait )
{
    UI_KEYSYS* pKey = _UI_GetKEYSYS(pUI);
	*speed = pKey->repeatSpeed;
	*wait = pKey->repeatWait;
}

//==============================================================================
/**
 * @brief キーリピートの速度とウェイトをゲット
 * @param[out]	speed	速度
 * @param[out]	wait	ウェイト
 * @return  none
 */
//==============================================================================
void GFL_UI_KeyGetRepeatSpeed( int* speed, int* wait )
{
    GFI_UI_KeyGetRepeatSpeed(_UI_GetUISYS(), speed, wait);
}

//==============================================================================
/**
 * @brief   キーコンフィグのモードの値を設定する
 * @param[in,out]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @param[in]   mode    キーコンフィグモード
 * @return  none
 */
//==============================================================================
void GFI_UI_KeySetControlMode(UISYS* pUI,const int mode)
{
    const GFL_UI_KEY_CUSTOM_TBL* pTbl;
    UI_KEYSYS* pKey = _UI_GetKEYSYS(pUI);
    int n = mode,tblno = 0;

    if(pKey->pControl==NULL){  // テーブルがない場合は設定できない
        return;
    }
    pKey->control_mode = mode;
    pTbl = pKey->pControl;
    while(n){  // テーブルのコンフィグの先頭を検索
        tblno++;
        if(pTbl->mode == GFL_UI_KEY_END){
            n--;
        }
        pTbl++;
    }
    pKey->control_tblno = tblno;
}

//==============================================================================
/**
 * @brief   キーコンフィグのモードの値を設定する
 * @param[in]   mode    キーコンフィグモード
 * @return  none
 */
//==============================================================================
void GFL_UI_KeySetControlMode(const int mode)
{
    GFI_UI_KeySetControlMode(_UI_GetUISYS(), mode);
}

//==============================================================================
/**
 * @brief キーコンフィグのモードの値を得る
 * @param[in]   pUI     ユーザーインターフェイスハンドルのポインタ
 * @return  コントロールモード
 */
//==============================================================================
int GFI_UI_KeyGetControlMode(const UISYS* pUI)
{
    const UI_KEYSYS* pKey = _UI_GetKEYSYS(pUI);
	return pKey->control_mode;
}

//==============================================================================
/**
 * @brief キーコンフィグのモードの値を得る
 * @param   none
 * @return  コントロールモード
 */
//==============================================================================
int GFL_UI_KeyGetControlMode(void)
{
    return GFI_UI_KeyGetControlMode(_UI_GetUISYS());
}
