//==============================================================================
/**
 * @file    monolith_power.c
 * @brief   モノリス：パワー画面
 * @author  matsuda
 * @date    2009.11.20(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"


//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithProcData = {
  MonolithProc_Init,
  MonolithProc_Main,
  MonolithProc_End,
};


