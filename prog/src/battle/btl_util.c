//=============================================================================================
/**
 * @file  btl_util.c
 * @brief ポケモンWB バトルシステム 共有ツール群
 * @author  taya
 *
 * @date  2008.09.25  作成
 */
//=============================================================================================

#include "btl_common.h"
#include "btl_debug_print.h"
#include "btl_util.h"


//------------------------------------------------------
/*
 *  Overlay ID
 */
//------------------------------------------------------
FS_EXTERN_OVERLAY(battle_print);

/*--------------------------------------------------------------------------*/
/* Globals                                                                  */
/*--------------------------------------------------------------------------*/
static BOOL PrintSysEnableFlag = FALSE;

void BTL_UTIL_PRINTSYS_Init( void )
{
  if( OS_GetConsoleType() & OS_CONSOLE_ISDEBUGGER )
  {
    GFL_OVERLAY_Load( FS_OVERLAY_ID(battle_print) );
    PrintSysEnableFlag = TRUE;
  }
  else{
    PrintSysEnableFlag = FALSE;
  }
}

void BTL_UTIL_PRINTSYS_Quit( void )
{
  if( PrintSysEnableFlag ){
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(battle_print) );
    PrintSysEnableFlag = FALSE;
  }
}



#ifdef BTL_PRINT_SYSTEM_ENABLE

void BTL_UTIL_SetPrintType( BtlPrintType type )
{
  if( PrintSysEnableFlag ){
    BTL_DEBUGPRINT_SetType( type );
  }
}

void BTL_UTIL_DumpPrintf( const char* caption, const void* data, u32 size )
{
  if( PrintSysEnableFlag )
  {
    BTL_DEBUGPRINT_PrintDump( caption, data, size );
  }
}

#endif  /* #ifdef BTL_PRINT_SYSTEM_ENABLE */


void BTL_UTIL_Printf( const char* filename, int line, u32 strID, ... )
{
  if( PrintSysEnableFlag )
  {
    const char* fmt_str = BTL_DEBUGPRINT_GetFormatStr( strID );
    if( fmt_str )
    {
      va_list vlist;

      BTL_DEBUGPRINT_PrintHeader( filename, line );

      va_start( vlist, strID );
      OS_TVPrintf( fmt_str, vlist );
      va_end( vlist );
    }
  }
}
