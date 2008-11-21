//============================================================================================
/**
 * @file	eventdata_system.h
 * @brief	イベント起動用データのロード・保持システム
 * @author	tamada
 * @date	2008.11.20
 */
//============================================================================================
#pragma once

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _EVDATA_SYS EVENTDATA_SYSTEM;

//------------------------------------------------------------------
//------------------------------------------------------------------
extern EVENTDATA_SYSTEM * EVENTDATA_SYS_Create(HEAPID heapID);
extern void EVENTDATA_SYS_Delete(EVENTDATA_SYSTEM * evdata);
extern void EVENTDATA_SYS_Clear(EVENTDATA_SYSTEM * evdata);
extern void EVENTDATA_SYS_Load(EVENTDATA_SYSTEM * evdata, u16 mapid);

