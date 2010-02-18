//============================================================================================
/**
 * @brief	playtime_ctrl.c
 * @brief	プレイ時間情報の操作
 * @date	2006.06.21
 * @author	taya
 * @todo  OS_InitTickがWiFi接続時に呼ばれるので、別途計測手段を考える
 *
 * 2010.02.18 tamada  旧システムから移行中
 *
 *
 */
//============================================================================================

#include <gflib.h>
#include "savedata/playtime.h"

#include "system\playtime_ctrl.h"


static BOOL StartFlag = FALSE;
static u64  PassedTick = 0;
static u64  PassedSec = 0;
static u64  StartTick = 0;
static PLAYTIME*  PlayTimePointer = NULL;

//------------------------------------------------------------------
/**
 * 初期化（ゲームメインループの前に１回だけ呼ぶ）
 */
//------------------------------------------------------------------
void PLAYTIMECTRL_Init( void )
{
	StartFlag = FALSE;
}

//------------------------------------------------------------------
/**
 * プレイ時間のカウント開始（ゲーム開始直後に１回だけ呼ぶ）
 *
 */
//------------------------------------------------------------------
void PLAYTIMECTRL_Start( PLAYTIME* pt )
{
	StartFlag = TRUE;
	PassedTick = 0;
	PassedSec = 0;
	PlayTimePointer = pt;
  StartTick = OS_GetTick();
	//StartTick = APTM_GetData();
}

//------------------------------------------------------------------
/**
 * プレイ時間のカウント（メインループ内で毎回呼ぶ）
 *
 * @param   time		プレイ時間セーブデータポインタ
 *
 */
//------------------------------------------------------------------
void PLAYTIMECTRL_Countup( void )
{
	if( StartFlag )
	{
		//u64  sec = APTM_CalcSec( APTM_GetData() - StartTick );
    u64 sec = OS_TicksToSeconds( OS_GetTick() - StartTick );
		if( sec > PassedSec )
		{
			PLAYTIME_CountUp( PlayTimePointer, (sec - PassedSec) );
			PassedSec = sec;
		}
	}
}










