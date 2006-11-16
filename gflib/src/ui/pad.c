//==============================================================================
/**
 * @file	pad.c
 * @brief	ユーザーインターフェイス パッドの管理
            デバイス＆アクセス関数
 * @author	GAME FREAK Inc.
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

//==============================================================================
//
//			関数
//
//==============================================================================

//==============================================================================
/**
 * パッド初期化
 * @param   UIHandle     ユーザーインターフェイスハンドルのポインタ
 * @return  none
 */
//==============================================================================

void GFL_UI_Pad_sysInit(UIHandle* pUI)
{
	TPCalibrateParam calibrate;

	pUI->key_control_mode = 0;	//キー取得モード

	pUI->cont_org	= 0;	// キー状態
	pUI->trg_org		= 0;	// キートリガー
	pUI->repeat_org	= 0;	// キーリピート
	pUI->cont		= 0;	// キー状態(ゲーム設定により変換処理が入る)
	pUI->trg			= 0;	// キートリガー(ゲーム設定により変換処理が入る)
	pUI->repeat      = 0;	// キーリピート(ゲーム設定により変換処理が入る)
	pUI->repeatWait_Count = 0;
	pUI->repeatSpeed	= 8;
	pUI->repeatWait	= 15;


	pUI->tp_x		 = 0;	// タッチパネルX座標
	pUI->tp_y		 = 0;	// タッチパネルY座標
	pUI->tp_trg		 = 0;	// タッチパネル接触判定トリガ
	pUI->tp_cont		 = 0;	// タッチパネル接触判定状態
	pUI->tp_auto_samp = 0;	// タッチパネルのオートサンプリングを行っているかのフラグ
									
	// タッチパネルの初期化とキャリブレーションをセット
	TP_Init();

	// マシンのキャリブレーション値を取得
	if( TP_GetUserInfo( &calibrate ) == TRUE ){
		// キャリブレーション値の設定
		TP_SetCalibrateParam( &calibrate );
		UI_PRINT("Get Calibration Parameter from NVRAM\n");
	}
	else{
		// 取得に失敗したのでデフォルトのキャリブレーションの設定
		calibrate.x0 = 0x02ae;
		calibrate.y0 = 0x058c;
		calibrate.xDotSize = 0x0e25;
		calibrate.yDotSize = 0x1208;
		TP_SetCalibrateParam( &calibrate );
		UI_PRINT( "Warrning : TouchPanelInit( not found valid calibration data )\n" );
	}
}

//==============================================================================
/**
 * パッド読み取り処理
 * @param   UIHandle     ユーザーインターフェイスハンドルのポインタ
 * @return  none
 */
//==============================================================================

void sys_MainKeyRead(UIHandle* pUI)
{
	TPData	tpTemp;
	TPData	tpDisp;
	int	keyData;

	// ふたが閉まっている場合は全ての入力をなしにする
	if(PAD_DetectFold()){
	  pUI->trg	= 0;
	  pUI->cont	= 0;
	  pUI->repeat	= 0;
	  pUI->tp_trg	= 0;
	  pUI->tp_cont	= 0;
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

	// タッチパネルデータを取得
	if(pUI->tp_auto_samp == 0){
		while( TP_RequestRawSampling( &tpTemp ) != 0 ){};	//サンプリングに成功するまで待つ
	}else{
		TP_GetLatestRawPointInAuto( &tpTemp );	// オートサンプリング中のデータを取得
	}

	TP_GetCalibratedPoint( &tpDisp, &tpTemp );	// 座標を画面座標（０～２５５）にする

	
	if( tpDisp.validity == TP_VALIDITY_VALID  ){		// 座標の有効性をチェック
		// タッチパネル座標有効
		pUI->tp_x = tpDisp.x;
		pUI->tp_y = tpDisp.y;
	}else{
		// タッチパネル座標無効
		// 1シンク前の座標が格納されているときのみ座標をそのまま受け継ぐ
		if( pUI->tp_cont ){
			switch(tpDisp.validity){
			case TP_VALIDITY_INVALID_X:
				pUI->tp_y = tpDisp.y;
				break;
			case TP_VALIDITY_INVALID_Y:
				pUI->tp_x = tpDisp.x;
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
	pUI->tp_trg	= (u16)(tpDisp.touch & (tpDisp.touch ^ pUI->tp_cont));	// トリガ 入力
	pUI->tp_cont	= tpDisp.touch;										// ベタ 入力

}

//--------------------------------------------------------------------------------------------
/**
 * キー情報の変換
 *
 * @param	none
 * @param	none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
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

static void _keyConvert(void)
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

//--------------------------------------------------------------------------------------------
/**
 * キーリピートの速度とウェイトをセット
 *
 * @param	speed	速度
 * @param	wait	ウェイト
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void sys_KeyRepeatSpeedSet( int speed, int wait )
{
	pUI->repeatSpeed	= speed;
	pUI->repeatWait	= wait;
}

