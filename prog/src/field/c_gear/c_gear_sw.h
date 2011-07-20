//=============================================================================
/**
 * @file	  c_gear_sw.h
 * @brief	  コミュニケーションギア
 * @author	iwasawa
 * @date	  11.07.19 
 */
//=============================================================================



#pragma once

#include <gflib.h>
#include "savedata/c_gear_data.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

#define CGEAR_SW_DECAL_SIZEX (17)
#define CGEAR_SW_DECAL_SIZEY (15)
#define CGEAR_SW_DECAL_SIZE_MAX (CGEAR_SW_DECAL_SIZEX*CGEAR_SW_DECAL_SIZEY*32)

#ifdef DEBUG_ONLY_FOR_iwasawa 
 #define CGEAR_SWAN_ON
#endif

FS_EXTERN_OVERLAY(cgear_sw);

typedef struct _C_GEAR_WORK  C_GEAR_WORK;

extern C_GEAR_WORK* CGEAR_SW_Init( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys, BOOL power_effect );
extern C_GEAR_WORK* CGEAR_SW_FirstInit( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys, STARTUP_ENDCALLBACK* pFunc, void* pwk, BOOL power_effect );
extern void CGEAR_SW_Main( C_GEAR_WORK* pWork,BOOL bAction );
extern void CGEAR_SW_ActionCallback( C_GEAR_WORK* pWork , FIELD_SUBSCREEN_ACTION actionno);
extern void CGEAR_SW_Exit(C_GEAR_WORK* pWork);

extern BOOL CGEAR_SW_ChangePowerOff( C_GEAR_WORK* pWork );
extern GMEVENT* CGEAR_SW_EventCheck(C_GEAR_WORK* pWork, BOOL bEvReqOK, FIELD_SUBSCREEN_WORK* pSub );

extern BOOL CGEAR_SW_IsDoEventWireless( const C_GEAR_WORK* cpWork );
extern void CGEAR_SW_SetStopTVTRingTone( C_GEAR_WORK* pWork ); // 外部でTVTRingToneをとめたときに設定してもらう。


//extern const GFL_PROC_DATA CGEARProcData;


