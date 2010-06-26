//==============================================================================
/**
 * @file	undata_update.h
 * @brief	国連データ更新処理
 * @author	saito
 * @date	2010.02.03
 */
//==============================================================================
#pragma once

#include "unitednations_def.h" 
#include "wifihistory.h"
#include "src/savedata/wifihistory_local.h"

typedef enum
{
  DAT_ADD_ST_FAIL = 0,
  DAT_ADD_ST_CHG,
  DAT_ADD_ST_ADD,
}DAT_ADD_ST;

extern DAT_ADD_ST UNDATAUP_Update(WIFI_HISTORY * wh, UNITEDNATIONS_SAVE *add_data);
extern u32 UNDATAUP_GetDataNum(const UNITEDNATIONS_SAVE *un_data);

#ifdef PM_DEBUG
extern DAT_ADD_ST UNDATAUP_UpdateD(WIFI_HISTORY * wh, UNITEDNATIONS_SAVE *add_data);
#endif


