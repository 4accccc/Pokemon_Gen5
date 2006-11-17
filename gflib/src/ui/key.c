//==============================================================================
/**
 * @file	key.c
 * @brief	ユーザーインターフェイス キーの管理
            デバイス＆アクセス関数
 * @author	k.ohno
 */
//==============================================================================

#include "gflib.h" 

//==============================================================================
//
//			定義、関数宣言
//
//==============================================================================

static void _keyConvert(void);

//==============================================================================
//
//			変数定義
//
//==============================================================================


struct _UIKeySys {
    UIKeyControlTbl* pControl;  //キーモードを設定するテーブル
//	int key_control_mode;	// キー取得モード(0がdefault)
	int cont_org;			// キー状態
	int trg_org;			// キートリガ
	int repeat_org;			// キーリピート
	int cont;				// キー状態(ゲーム設定により変換処理が入る)
	int trg;				// キートリガ(ゲーム設定により変換処理が入る)
	int repeat;				// キーリピート(ゲーム設定により変換処理が入る)
	int repeatWait_Count;	// キーリピートカウンタ
	int repeatSpeed;		// キーリピートスピード
	int repeatWait;			// キーリピートウェイト
};

//==============================================================================
//
//			関数
//
//==============================================================================

//==============================================================================
/**
 * パッド初期化
 * @param[in,out]   UIPadHandle     ユーザーインターフェイスパッドハンドルのポインタ
 * @return  none
 */
//==============================================================================

void GFL_UI_Pad_sysInit(UIPadHandle* pUI)
{
	TPCalibrateParam calibrate;

//	pUI->key_control_mode = 0;	//キー取得モード

	pUI->cont_org	= 0;	// キー状態
	pUI->trg_org		= 0;	// キートリガー
	pUI->repeat_org	= 0;	// キーリピート
	pUI->cont		= 0;	// キー状態(ゲーム設定により変換処理が入る)
	pUI->trg			= 0;	// キートリガー(ゲーム設定により変換処理が入る)
	pUI->repeat      = 0;	// キーリピート(ゲーム設定により変換処理が入る)
	pUI->repeatWait_Count = 0;
	pUI->repeatSpeed	= 8;
	pUI->repeatWait	= 15;

}

//==============================================================================
/**
 * パッド読み取り処理
 * @param[in,out]   UIPadHandle     ユーザーインターフェイスパッドハンドルのポインタ
 * @return  none
 */
//==============================================================================

void GFL_UI_Pad_sysMain(UIPadHandle* pUI)
{
	TPData	tpTemp;
	TPData	tpDisp;
	int	keyData;

	// ふたが閉まっている場合は全ての入力をなしにする
	if(PAD_DetectFold()){
	  pUI->trg	= 0;
	  pUI->cont	= 0;
	  pUI->repeat	= 0;
	  return;
	}

	keyData = PAD_Read();
	
	pUI->trg_org		= keyData & (keyData ^ pUI->cont_org);	// トリガ 入力

    pUI->repeat_org	=	keyData & (keyData ^ pUI->cont_org);        // リピート 入力

    if((keyData!=0)&&(pUI->cont_org==keyData)){
		if(--pUI->repeatWait_Count==0){
			pUI->repeat_org				=	keyData;
			pUI->repeatWait_Count	=	pUI->repeatSpeed;
		}
	}
	else{
		pUI->repeatWait_Count = pUI->repeatWait;
	}


	pUI->cont_org	= keyData;							// ベタ 入力

	pUI->trg		= pUI->trg_org;
	pUI->cont	= pUI->cont_org;
	pUI->repeat	= pUI->repeat_org;

	_keyConvert();
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
 * キー情報の変換
 * @param[in,out]   UIPadHandle     ユーザーインターフェイスパッドハンドルのポインタ
 * @return  none
 */
//==============================================================================
static void _keyConvert(UIPadHandle* pUI)
{
	switch( pUI->key_control_mode ){

	default:
	case KEYCONTROL_NORMAL:
		break;

	case KEYCONTROL_PATTERN1:

		KEYCONV( pUI->trg, PAD_BUTTON_START, PAD_BUTTON_X )
		KEYCONV( pUI->cont, PAD_BUTTON_START, PAD_BUTTON_X )
		KEYCONV( pUI->repeat, PAD_BUTTON_START, PAD_BUTTON_X )
		break;

	case KEYCONTROL_PATTERN2:

		KEYCONV_EXCHANGE( pUI->trg, PAD_BUTTON_X,PAD_BUTTON_Y )
		KEYCONV_EXCHANGE( pUI->cont, PAD_BUTTON_X,PAD_BUTTON_Y )
		KEYCONV_EXCHANGE( pUI->repeat, PAD_BUTTON_X,PAD_BUTTON_Y )
		break;

	case KEYCONTROL_PATTERN3:

		KEYCONV( pUI->trg, PAD_BUTTON_L, PAD_BUTTON_A )
		KEYCONV( pUI->cont, PAD_BUTTON_L, PAD_BUTTON_A )
		KEYCONV( pUI->repeat, PAD_BUTTON_L, PAD_BUTTON_A )

		KEY_RESET( pUI->trg, ( PAD_BUTTON_L | PAD_BUTTON_R ) )
		KEY_RESET( pUI->cont, ( PAD_BUTTON_L | PAD_BUTTON_R ) )
		KEY_RESET( pUI->repeat, ( PAD_BUTTON_L | PAD_BUTTON_R ) )
		break;
	}
}

//==============================================================================
/**
 * キーリピートの速度とウェイトをセット
 * @param[in,out]   UIPadHandle     ユーザーインターフェイスパッドハンドルのポインタ
 * @param[in]	speed	速度
 * @param[in]	wait	ウェイト
 * @return  none
 */
//==============================================================================
void GFL_UI_KeyRepeatSpeedSet( int speed, int wait )
{
	pUI->repeatSpeed	= speed;
	pUI->repeatWait	= wait;
}

//==============================================================================
/**
 * キートリガゲット
 * @param[in,out]   UIPadHandle     ユーザーインターフェイスパッドハンドルのポインタ
 * @return  キートリガ
 */
//==============================================================================
void GFL_UI_KeyGetTrg( int speed, int wait )
{
	pUI->repeatSpeed	= speed;
	pUI->repeatWait	= wait;
}

