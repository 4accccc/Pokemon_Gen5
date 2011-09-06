//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_h06_gmk.c
 *	@brief  マリンチューブ  スクリプトコマンド
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.09.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "savedata/gimmickwork.h"
#include "system/vm_cmd.h"
// field
#include "ev_time.h"

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// script
#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

// medal
#include "savedata/medalbox.h"

// mine
#include "scrcmd_h06_gmk.h"
#include "field_gimmick_h06.h"


//----------------------------------------------------------------------------
/**
 *	@brief  マリンチューブ ゾーンチェンジ初期化
 *
 *	@param	VMHANDLE *core
 *	@param	*wk 
 *
 *	@return VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdH06Gmk_Init( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );
  TIME_ZONE *p_sv_time_zone = GIMMICKWORK_Get( GAMEDATA_GetGimmickWork( p_gdata ), FLD_GIMMICK_H06 );


  // 時間帯分岐
  {
    int hour = EVTIME_GetHour( p_gdata );
    int time_zone;

    if( hour >= 15 && hour <= 16 )
    {
      time_zone = TIME_ZONE_RARE;

      // 「ホエルオーウォッチ」メダル達成
      {
        SAVE_CONTROL_WORK   *save     = GAMEDATA_GetSaveControlWork( p_gdata );
        MEDALBOX            *medalbox = SaveData_GetMedalBox( save );
        MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_MARINETUBE );
      }
    }
    else if( hour >= 6 && hour <= 14 )
    {
      time_zone = TIME_ZONE_AM;
    }
    else
    {
      time_zone = TIME_ZONE_PM;
    }


    ICHI_Printf("時間:%d\n",hour);
    ICHI_Printf("時間帯:%d\n", time_zone);

    *p_sv_time_zone = time_zone;
  }

  return VMCMD_RESULT_CONTINUE;
}

