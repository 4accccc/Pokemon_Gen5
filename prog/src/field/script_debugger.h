//============================================================================================
/**
 * @file  script_debugger.h
 * @brief
 * @date  2010.01.18
 * @author  tamada GAMEFREAK inc.
 */
//============================================================================================
#pragma once

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern BOOL SCRDEBUGGER_Boot( void );
extern void * SCRDEBUGGER_ReadScriptFile( HEAPID heapID, u32 scr_id );

extern BOOL SCRDEBUGGER_ReadSpecialScriptFile( u32 scr_id, void * buffer, u32 buf_size );
extern BOOL SCRDEBUGGER_ReadEventFile( u32 zone_id, void * buffer, u32 buf_size );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern BOOL SCRDEBUGGER_Boot_Core( void );
extern BOOL SCRDEBUGGER_CORE_readScript( u32 scr_id, void * buffer, u32 buf_size );
extern BOOL SCRDEBUGGER_CORE_readEventData( u32 zone_id, void * buffer, u32 buf_size );


