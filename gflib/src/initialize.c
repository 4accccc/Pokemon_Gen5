//===================================================================
/**
 * @file	initialize.c
 * @brief	初期化
 * @author	GAME FREAK Inc.
 * @date	06.11.07
 *
 * $LastChangedDate$
 * $Id$
 */
//===================================================================
#include <nitro.h>
#include "gflib.h"
#include "gflib_inter.h"


//------------------------------------------------------------------
/**
 * @brief	システム初期化
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void InitSystem(void)
{
#ifdef USE_MAINEXMEMORY
  OS_EnableMainExArena();
#endif

  // システム初期化
  OS_Init();
  OS_InitTick();
  OS_InitAlarm();
  // RTC初期化
  RTC_Init();
  // GX初期化
  GX_Init();
  // 固定小数点API初期化
  FX_Init();
  // ファイルシステム初期化
  InitFileSystem();

  OS_EnableIrq();  // この関数は会議で相談した後で移動する  @@OO
}


/*  */
