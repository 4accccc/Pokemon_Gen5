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



/*---------------------------------------------------------------------------*
  Name:         sys_KeyRead

  Description:  キーを読み込み、トリガとリリーストリガを求めます。
				
  Arguments:    なし。
  Returns:      なし。
 *---------------------------------------------------------------------------*/

//初期化
void sys_InitKeyRead(void)
{
	TPCalibrateParam calibrate;

	sys.key_control_mode = 0;	//キー取得モード

	sys.cont_org	= 0;	// キー状態
	sys.trg_org		= 0;	// キートリガー
	sys.repeat_org	= 0;	// キーリピート
	sys.cont		= 0;	// キー状態(ゲーム設定により変換処理が入る)
	sys.trg			= 0;	// キートリガー(ゲーム設定により変換処理が入る)
	sys.repeat      = 0;	// キーリピート(ゲーム設定により変換処理が入る)
	sys.repeatWait_Count = 0;
	sys.repeatSpeed	= 8;
	sys.repeatWait	= 15;


	sys.tp_x		 = 0;	// タッチパネルX座標
	sys.tp_y		 = 0;	// タッチパネルY座標
	sys.tp_trg		 = 0;	// タッチパネル接触判定トリガ
	sys.tp_cont		 = 0;	// タッチパネル接触判定状態
	sys.tp_auto_samp = 0;	// タッチパネルのオートサンプリングを行っているかのフラグ
									
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
}

/*---------------------------------------------------------------------------*
  @brief	ＡＧＢカートリッジ初期化
 *---------------------------------------------------------------------------*/
void sys_InitAgbCasetteVer(int version)
{
	sys.AgbCasetteVersion = version;
}

/*---------------------------------------------------------------------------*
  @brief	スリープ状態を禁止する
 *---------------------------------------------------------------------------*/
void sys_SleepNG(u8 sleepTypeBit)
{
	sys.DontSleep |= sleepTypeBit;
}

/*---------------------------------------------------------------------------*
  @brief	スリープ状態を許可する
 *---------------------------------------------------------------------------*/
void sys_SleepOK(u8 sleepTypeBit)
{
	sys.DontSleep &= ~(sleepTypeBit);
}

//------------------------------------------------------------------
static void KeyConvert(void);

//読み取り

void sys_MainKeyRead(void)
{
	TPData	tpTemp;
	TPData	tpDisp;
	int	keyData;

	// ふたが閉まっている場合は全ての入力をなしにする
	if(PAD_DetectFold()){
	  sys.trg	= 0;
	  sys.cont	= 0;
	  sys.repeat	= 0;
	  sys.tp_trg	= 0;
	  sys.tp_cont	= 0;
	  return;
	}

	keyData = PAD_Read();
	
	sys.trg_org		= keyData & (keyData ^ sys.cont_org);	// トリガ 入力

    sys.repeat_org	=	keyData & (keyData ^ sys.cont_org);        // リピート 入力

    if((keyData!=0)&&(sys.cont_org==keyData)){
		if(--sys.repeatWait_Count==0){
			sys.repeat_org				=	keyData;
			sys.repeatWait_Count	=	sys.repeatSpeed;
		}
	}
	else{
		sys.repeatWait_Count = sys.repeatWait;
	}


	sys.cont_org	= keyData;							// ベタ 入力

	sys.trg		= sys.trg_org;
	sys.cont	= sys.cont_org;
	sys.repeat	= sys.repeat_org;

	KeyConvert();

	// タッチパネルデータを取得
	if(sys.tp_auto_samp == 0){
		while( TP_RequestRawSampling( &tpTemp ) != 0 ){};	//サンプリングに成功するまで待つ
	}else{
		TP_GetLatestRawPointInAuto( &tpTemp );	// オートサンプリング中のデータを取得
	}

	TP_GetCalibratedPoint( &tpDisp, &tpTemp );	// 座標を画面座標（０～２５５）にする

#if 0	// 2006.07.05	座標取得失敗時処理を変更
	switch(tpDisp.validity){		// 座標の有効性をチェック
	case TP_VALIDITY_INVALID_X:
		sys.tp_x = 0xffff;
		sys.tp_y = tpDisp.y;
		break;
	case TP_VALIDITY_INVALID_Y:
		sys.tp_x = tpDisp.x;			// 座標をセット
		sys.tp_y = 0xffff;
		break;
	case TP_VALIDITY_INVALID_XY:
		sys.tp_x = 0xffff;
		sys.tp_y = 0xffff;
		break;
	default:
		sys.tp_x = tpDisp.x;
		sys.tp_y = tpDisp.y;
	}

	sys.tp_trg	= (u16)(tpDisp.touch & (tpDisp.touch ^ sys.tp_cont));	// トリガ 入力
	sys.tp_cont	= tpDisp.touch;										// ベタ 入力
	
#endif	/// 
	
	if( tpDisp.validity == TP_VALIDITY_VALID  ){		// 座標の有効性をチェック
		// タッチパネル座標有効
		sys.tp_x = tpDisp.x;
		sys.tp_y = tpDisp.y;
	}else{
		// タッチパネル座標無効
		// 1シンク前の座標が格納されているときのみ座標をそのまま受け継ぐ
		if( sys.tp_cont ){
			switch(tpDisp.validity){
			case TP_VALIDITY_INVALID_X:
				sys.tp_y = tpDisp.y;
				break;
			case TP_VALIDITY_INVALID_Y:
				sys.tp_x = tpDisp.x;
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
	sys.tp_trg	= (u16)(tpDisp.touch & (tpDisp.touch ^ sys.tp_cont));	// トリガ 入力
	sys.tp_cont	= tpDisp.touch;										// ベタ 入力

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

static void KeyConvert(void)
{
	switch( sys.key_control_mode ){

	default:
	case KEYCONTROL_NORMAL:
		break;

	case KEYCONTROL_PATTERN1:

		KEYCONV( sys.trg, PAD_BUTTON_START, PAD_BUTTON_X )
		KEYCONV( sys.cont, PAD_BUTTON_START, PAD_BUTTON_X )
		KEYCONV( sys.repeat, PAD_BUTTON_START, PAD_BUTTON_X )
		break;

	case KEYCONTROL_PATTERN2:

		KEYCONV_EXCHANGE( sys.trg, PAD_BUTTON_X,PAD_BUTTON_Y )
		KEYCONV_EXCHANGE( sys.cont, PAD_BUTTON_X,PAD_BUTTON_Y )
		KEYCONV_EXCHANGE( sys.repeat, PAD_BUTTON_X,PAD_BUTTON_Y )
		break;

	case KEYCONTROL_PATTERN3:

		KEYCONV( sys.trg, PAD_BUTTON_L, PAD_BUTTON_A )
		KEYCONV( sys.cont, PAD_BUTTON_L, PAD_BUTTON_A )
		KEYCONV( sys.repeat, PAD_BUTTON_L, PAD_BUTTON_A )

		KEY_RESET( sys.trg, ( PAD_BUTTON_L | PAD_BUTTON_R ) )
		KEY_RESET( sys.cont, ( PAD_BUTTON_L | PAD_BUTTON_R ) )
		KEY_RESET( sys.repeat, ( PAD_BUTTON_L | PAD_BUTTON_R ) )
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
	sys.repeatSpeed	= speed;
	sys.repeatWait	= wait;
}


/*---------------------------------------------------------------------------*
  @brief	ソフトウエアリセット状態を禁止する
 *---------------------------------------------------------------------------*/
void sys_SoftResetNG(u8 softResetBit)
{
	sys.DontSoftReset |= softResetBit;
}

/*---------------------------------------------------------------------------*
  @brief	ソフトウエアリセット状態を許可する
 *---------------------------------------------------------------------------*/
void sys_SoftResetOK(u8 softResetBit)
{
	sys.DontSoftReset &= ~(softResetBit);
}

