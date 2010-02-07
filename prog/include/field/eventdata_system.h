//============================================================================================
/**
 * @file	eventdata_system.h
 * @brief	イベント起動用データのロード・保持システム
 * @author	tamada
 * @date	2008.11.20
 */
//============================================================================================
#pragma once

#define EVENTDATA_ID_NONE (0xffff) ///<イベントデータなし

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _EVDATA_SYS EVENTDATA_SYSTEM;

//------------------------------------------------------------------
//------------------------------------------------------------------
extern EVENTDATA_SYSTEM * EVENTDATA_SYS_Create(HEAPID heapID);
extern void EVENTDATA_SYS_Delete(EVENTDATA_SYSTEM * evdata);
extern void EVENTDATA_SYS_Clear(EVENTDATA_SYSTEM * evdata);
extern void EVENTDATA_SYS_Load(EVENTDATA_SYSTEM * evdata, u16 mapid, u8 season_id );
extern void * EVENTDATA_GetSpecialScriptData( EVENTDATA_SYSTEM * evdata );
extern void * EVENTDATA_GetEncountDataTable( EVENTDATA_SYSTEM * evdata );

extern u32 EVENTDATA_GetBgEventNum( const EVENTDATA_SYSTEM * evdata );
extern u32 EVENTDATA_GetNpcEventNum( const EVENTDATA_SYSTEM * evdata );
extern u32 EVENTDATA_GetConnectEventNum( const EVENTDATA_SYSTEM * evdata );
extern u32 EVENTDATA_GetPosEventNum( const EVENTDATA_SYSTEM * evdata );




#include "field/eventwork.h"
#include "field/rail_location.h"

//check event
extern u16 EVENTDATA_CheckPosEvent(
  const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork, const VecFx32 *pos, u16 dir );
extern u16 EVENTDATA_CheckTalkBGEvent(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const VecFx32 *pos, u16 talk_dir );
extern u16 EVENTDATA_CheckTalkBoardEvent(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const VecFx32 *pos, u16 talk_dir );

extern u16 EVENTDATA_CheckPosEventRailLocation(
  const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork, const RAIL_LOCATION *pos );
extern u16 EVENTDATA_CheckTalkBGEventRailLocation(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const RAIL_LOCATION *pos, u16 talk_dir );
extern u16 EVENTDATA_CheckTalkBoardEventRailLocation(
    const EVENTDATA_SYSTEM *evdata, EVENTWORK *evwork,
    const RAIL_LOCATION *pos, u16 talk_dir );


extern void EVENTDATA_MoveConnectData(
    EVENTDATA_SYSTEM * evdata, u16 exit_id, u16 gx, u16 gy, u16 gz );
extern void EVENTDATA_MoveBGData(
    EVENTDATA_SYSTEM * evdata, u16 bg_id, u16 gx, u16 gy, u16 gz );


