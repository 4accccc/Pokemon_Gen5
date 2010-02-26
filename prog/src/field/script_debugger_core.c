//============================================================================================
/**
 * @file  script_debugger_core.c
 * @brief
 * @date  2010.01.19
 * @author  tamada GAMEFREAK inc.
 */
//============================================================================================
#ifdef  PM_DEBUG

#include "gflib.h"
#include "script_debugger.h"
#include <nnsys/mcs.h>

#include "debug/gf_mcs.h"
#include "debug/mcs_readfile.h"
#include "field/zonedata.h"

#include "script_debugger.h"

//============================================================================================
//============================================================================================
enum {
  GFL_MCS_CATEGORY_ID = 0x12EBFFA,
};

enum {
  PATH_MAX_LEN = 128,
};

#include "arc/fieldmap/script_seq.naix"

#include "../../../resource/fldmapdata/script/scr_debug/scr_filepath.cdat"

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRDEBUGGER_Boot_Core( void )
{
    if ( NNS_McsIsServerConnect() == FALSE )
    {
      GFL_MCS_Open();
      if ( NNS_McsIsServerConnect() == FALSE ) return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void makeScriptPath( char * tempPath, u16 scr_id )
{
  static const char relPath[] = "script/";
  GFL_STD_MemClear( tempPath, PATH_MAX_LEN );
  GFL_STD_MemCopy( scrDirPath, tempPath, sizeof(scrDirPath) );
  GFL_STD_StrCat( tempPath, relPath, sizeof(relPath) );
  GFL_STD_StrCat( tempPath, scrNamePath[scr_id], GFL_STD_StrLen( scrNamePath[scr_id] ) );
  OS_Printf("scrDirPath(%s) Length = %d\n", scrDirPath, sizeof(scrDirPath) );
}
//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRDEBUGGER_CORE_readScript( u32 scr_id, void * buffer, u32 buf_size )
{
  BOOL result;
  char tempPath[ PATH_MAX_LEN ];

  if ( scr_id >= NELEMS( scrNamePath ) )
  {
    return FALSE;
  }
  makeScriptPath( tempPath, scr_id );

  result = GF_MCS_FILE_Read( tempPath, buffer, buf_size );
  if ( result )
  {
    OS_Printf("MCSスクリプト読み込み：%d\n", scr_id );
  }
  return result;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void * SCRDEBUGGER_CORE_readScriptAlloc( u32 scr_id, HEAPID heapID, u32 buf_size )
{
  char tempPath[ PATH_MAX_LEN ];
  void * buffer;

  if ( scr_id >= NELEMS( scrNamePath ) )
  {
    return NULL;
  }
  makeScriptPath( tempPath, scr_id );

  buffer = GF_MCS_FILE_ReadAlloc( tempPath, heapID, buf_size );
  if ( buffer == NULL )
  {
    return NULL;
  }
  else
  {
    OS_Printf("MCSスクリプト読み込み：%d\n", scr_id );
    return buffer;
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRDEBUGGER_CORE_readEventData( u32 zone_id, void * buffer, u32 buf_size )
{
  BOOL result;
  static const char relPath[] = "eventdata/tmp/";
  static const char extName[] = ".bin";
  char tempPath[ PATH_MAX_LEN ];
  char zoneName[ PATH_MAX_LEN ];

  GFL_STD_MemClear( tempPath, PATH_MAX_LEN );
  GFL_STD_MemCopy( scrDirPath, tempPath, sizeof(scrDirPath) );
  GFL_STD_StrCat( tempPath, relPath, sizeof(relPath) );
  ZONEDATA_DEBUG_GetZoneName( zoneName, zone_id );
  GFL_STD_StrCat( tempPath, zoneName, GFL_STD_StrLen(zoneName) );
  GFL_STD_StrCat( tempPath, extName, sizeof(extName) );

  result = GF_MCS_FILE_Read( tempPath, buffer, buf_size );
  if ( result )
  {
    OS_Printf("MCSイベントデータ読み込み：%s\n", zoneName );
  }
  return result;
}


#endif  // End of PM_DEBUG

