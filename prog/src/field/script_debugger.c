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
#include "sound/pm_sndsys.h"

//#include "debug/gf_mcs.h"
//#include "debug/mcs_readfile.h"

#ifdef  PM_DEBUG
#include "debug/mcs_mode.h"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL checkMCSEnable( void )
{
  if( OS_GetConsoleType() & (OS_CONSOLE_ISDEBUGGER|OS_CONSOLE_TWLDEBUGGER) )
  {
    if (MCS_USEMODE_Get() != MCS_USEMODE_SCRDEBUGGER ) return FALSE;

    return NNS_McsIsServerConnect();
  }
  return FALSE;
}
//------------------------------------------------------------------
/** サウンドの分割読み込み終了待ち */
//------------------------------------------------------------------
static void waitSoundLoading( void )
{
  while ( PMSND_IsLoading() == TRUE ) {
    OS_Sleep(1);
  }
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

  waitSoundLoading();

  buffer = SCRDEBUGGER_CORE_readScriptAlloc( scr_id, heapID, 0 );
#if 0
  buffer = GFL_HEAP_AllocClearMemory( heapID, 8000 );
  if ( SCRDEBUGGER_CORE_readScript( scr_id, buffer, 8000 ) == FALSE )
  {
    GFL_HEAP_FreeMemory( buffer );
    return NULL;
  }
#endif
  return buffer;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRDEBUGGER_ReadSpecialScriptFile( u32 scr_id, void * buffer, u32 buf_size )
{
  if ( checkMCSEnable() == FALSE) return FALSE;
  waitSoundLoading();
  return SCRDEBUGGER_CORE_readScript( scr_id, buffer, buf_size );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRDEBUGGER_ReadEventFile( u32 zone_id, void * buffer, u32 buf_size )
{
  if ( checkMCSEnable() == FALSE) return FALSE;
  waitSoundLoading();
  return SCRDEBUGGER_CORE_readEventData( zone_id, buffer, buf_size );
}

#endif  // End of PM_DEBUG

