//============================================================================================
/**
 * @file  script_debugger.c
 * @brief
 * @date  2010.01.18
 * @author  tamada GAMEFREAK inc.
 *
 * MCS利用コードの実態はmcs_libオーバーレイ上のscript_debugger_sub.cにある。
 * こちらにあるのは呼び出しのための常駐部分
 */
//============================================================================================

#include "gflib.h"
#include "script_debugger.h"
#include <nnsys/mcs.h>

//#include "debug/gf_mcs.h"
//#include "debug/mcs_readfile.h"

#ifdef  PM_DEBUG

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL checkMCSEnable( void )
{
  if( OS_GetConsoleType() & (OS_CONSOLE_ISDEBUGGER|OS_CONSOLE_TWLDEBUGGER) )
  {
    //if ( GFL_MCS_CheckLink( GFL_MCS_CATEGORY_ID ) == FALSE ) return FALSE;
    return NNS_McsIsServerConnect();
  }
  return FALSE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRDEBUGGER_Boot( void )
{
  if( OS_GetConsoleType() & (OS_CONSOLE_ISDEBUGGER|OS_CONSOLE_TWLDEBUGGER) )
  {
    return SCRDEBUGGER_Boot_Core();
  }
  return FALSE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void * SCRDEBUGGER_ReadScriptFile( HEAPID heapID, u32 scr_id )
{
  void * buffer;

  if ( checkMCSEnable() == FALSE ) return NULL;

  buffer = GFL_HEAP_AllocClearMemory( heapID, 8000 );
  if ( SCRDEBUGGER_CORE_readScript( scr_id, buffer, 8000 ) == FALSE )
  {
    GFL_HEAP_FreeMemory( buffer );
    return NULL;
  }
  return buffer;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRDEBUGGER_ReadSpecialScriptFile( u32 scr_id, void * buffer, u32 buf_size )
{
  if ( checkMCSEnable() == FALSE) return FALSE;

  return SCRDEBUGGER_CORE_readScript( scr_id, buffer, buf_size );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRDEBUGGER_ReadEventFile( u32 zone_id, void * buffer, u32 buf_size )
{
  if ( checkMCSEnable() == FALSE) return FALSE;
  return SCRDEBUGGER_CORE_readEventData( zone_id, buffer, buf_size );
}

#endif  // End of PM_DEBUG

