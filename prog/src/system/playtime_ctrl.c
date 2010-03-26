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
static u64  LastTick = 0;
static u64  PassedSec = 0;
static u64  StartTick = 0;

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
void PLAYTIMECTRL_Start( void )
{
	StartFlag = TRUE;
	LastTick = 0;
	PassedSec = 0;
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
void PLAYTIMECTRL_Countup( GAMEDATA* gamedata )
{
	if( StartFlag )
	{
		//u64  sec = APTM_CalcSec( APTM_GetData() - StartTick );
    u64 sec;
    u64 tick = OS_GetTick();

    // チックタイムの逆点現象チェック
    // Wifi接続時にtickは０クリアされる。
    if( LastTick > tick ){
      // 0クリアされる前のチック分も足しこむ。
      tick = tick + LastTick;
    }else{
      LastTick = tick;
    }

    sec = OS_TicksToSeconds( tick - StartTick );
		if( sec > PassedSec )
		{
			GAMEDATA_PlayTimeCountUp( gamedata, (sec - PassedSec) );
      
      if( LastTick == tick ){
        // 通常時
			  PassedSec = sec;
      }else{

        // 何かしらの処理でTickがクリアされた場合
        // 再初期化。
        StartTick = OS_GetTick();
        PassedSec = OS_TicksToSeconds( StartTick );
        LastTick = StartTick;  // 
      }
		}
	}
}










