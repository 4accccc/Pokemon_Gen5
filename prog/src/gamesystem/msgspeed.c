//==============================================================================
/**
 * @file		msgspeed.c
 * @brief		ユーザコンフィグ等、各種条件からメッセージ表示速度を決定する
 * @author	taya
 * @date		2009.04.06
 */
//==============================================================================
#include <gflib.h>

#include "savedata\config.h"
#include "gamesystem\msgspeed.h"


static const SAVE_CONTROL_WORK*  SaveCtrl = NULL;


//=============================================================================================
/**
 * システム初期化 - 速度決定に必要な情報へのアクセサを渡す - 
 * ※ゲーム起動後に１回だけ呼びだし
 *
 * @param   ctrl		セーブデータコントローラ
 *
 */
//=============================================================================================
void MSGSPEED_InitSystem( const SAVE_CONTROL_WORK* ctrl )
{
	SaveCtrl = ctrl;
}

//=============================================================================================
/**
 * メッセージ速度取得
 *
 * @retval  int		メッセージ速度（この値を PRINTSYS_PrintStream 等の wait 値としてそのまま使う）
 */
//=============================================================================================
int MSGSPEED_GetWait( void )
{
	CONFIG*  cfg = SaveData_GetConfig( (SAVE_CONTROL_WORK*)SaveCtrl );

// @@@ GFL_NET_GetConnectNum

	switch( CONFIG_GetMsgSpeed(cfg) ){
	case MSGSPEED_SLOW:		return  3;
	case MSGSPEED_NORMAL:	return  1;
	case MSGSPEED_FAST:		return -2;
	default:
		GF_ASSERT(0);
		return -2;
	}
}

