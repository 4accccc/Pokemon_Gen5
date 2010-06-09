//=============================================================================
/**
 * @file	  c_gear.h
 * @brief	  コミュニケーションギア
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/04/30
 */
//=============================================================================



#pragma once

#include <gflib.h>
#include "savedata/c_gear_data.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

#define CGEAR_DECAL_SIZEX (17)
#define CGEAR_DECAL_SIZEY (15)
#define CGEAR_DECAL_SIZE_MAX (CGEAR_DECAL_SIZEX*CGEAR_DECAL_SIZEY*32)


FS_EXTERN_OVERLAY(cgear);

typedef struct _C_GEAR_WORK  C_GEAR_WORK;

extern C_GEAR_WORK* CGEAR_Init( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys, BOOL power_effect );
extern C_GEAR_WORK* CGEAR_FirstInit( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys, STARTUP_ENDCALLBACK* pFunc, void* pwk, BOOL power_effect );
extern void CGEAR_Main( C_GEAR_WORK* pWork,BOOL bAction );
extern void CGEAR_ActionCallback( C_GEAR_WORK* pWork , FIELD_SUBSCREEN_ACTION actionno);
extern void CGEAR_Exit(C_GEAR_WORK* pWork);

extern GMEVENT* CGEAR_EventCheck(C_GEAR_WORK* pWork, BOOL bEvReqOK, FIELD_SUBSCREEN_WORK* pSub );

extern BOOL CGEAR_ChangePowerOff( C_GEAR_WORK* pWork );

extern BOOL CGEAR_IsDoEventWireless( const C_GEAR_WORK* cpWork );
extern void CGEAR_SetStopTVTRingTone( C_GEAR_WORK* pWork ); // 外部でTVTRingToneをとめたときに設定してもらう。


//extern const GFL_PROC_DATA CGEARProcData;


