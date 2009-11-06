//==============================================================================
/**
 * @file    msgspeed.c
 * @brief   ユーザコンフィグ等、各種条件からメッセージ表示速度を決定する
 * @author  taya
 * @date    2009.04.06
 */
//==============================================================================
#include <gflib.h>

#include "savedata\config.h"
#include "gamesystem\msgspeed.h"



/*--------------------------------------------------------------------------*/
/* Tables                                                                   */
/*--------------------------------------------------------------------------*/

/**
 *  ウェイトフレーム数テーブル
 *
 *  標準的な文字数の差によるプレイ全体の速度差を緩和するため、
 *  日韓は遅め、欧米は早めに設定している。
 */
#if( (PM_LANG==LANG_JAPAN) || (PM_LANG==LANG_KOREA) )
static const s8 WaitFrameTable[] = {
    8,
    4,
    1,
   -2,
};
#else
static const s8 WaitFrameTable[] = {
    4,
    2,
    0,
   -4,
};
#endif

/*--------------------------------------------------------------------------*/
/* Globals                                                                  */
/*--------------------------------------------------------------------------*/
static const SAVE_CONTROL_WORK*  SaveCtrl = NULL;


//=============================================================================================
/**
 * システム初期化 - 速度決定に必要な情報へのアクセサを渡す -
 * ※ゲーム起動後に１回だけ呼びだし
 *
 * @param   ctrl    セーブデータコントローラ
 *
 */
//=============================================================================================
void MSGSPEED_InitSystem( const SAVE_CONTROL_WORK* ctrl )
{
  SaveCtrl = ctrl;
}

//=============================================================================================
/**
 * ウェイトフレーム数取得
 *
 * @retval  int   ウェイトフレーム数（この値を PRINTSYS_PrintStream 等の wait 値としてそのまま使う）
 */
//=============================================================================================
int MSGSPEED_GetWait( void )
{
  CONFIG*  cfg = SaveData_GetConfig( (SAVE_CONTROL_WORK*)SaveCtrl );
  MSGSPEED speed = CONFIG_GetMsgSpeed( cfg );

// @@@ 通信判定は未対応。ｰ>GFL_NET_GetConnectNum

  if( speed >= NELEMS(WaitFrameTable) ){
    GF_ASSERT(0);
    speed = NELEMS(WaitFrameTable) - 1;
  }

  return WaitFrameTable[ speed ];
}

//=============================================================================================
/**
 * CONFIG設定値に対応したウェイトフレーム数取得
 *
 * @param   speed   CONFIG設定値
 *
 * @retval  int   ウェイトフレーム数取得（この値を PRINTSYS_PrintStream 等の wait 値としてそのまま使う）
 */
//=============================================================================================
int MSGSPEED_GetWaitByConfigParam( MSGSPEED speed )
{
  if( speed >= NELEMS(WaitFrameTable) ){
    GF_ASSERT(0);
    speed = NELEMS(WaitFrameTable) - 1;
  }

  return WaitFrameTable[ speed ];
}
