//============================================================================================
/**
 * @file    gsync_download.c
 * @bfief   GAMESYNCダウンロード関連
 * @author  k.ohno
 * @date    2010.03.05
 */
//============================================================================================

#include <gflib.h>
#include <dwc.h>

#include "arc_def.h"
#include "net/network_define.h"
#include "net_app/gsync.h"
#include "gsync_download.h"

#include "system/main.h"
#include "system/wipe.h"
#include "gamesystem/msgspeed.h" // MSGSPEED_GetWait
#include "savedata/config.h"  // WIRELESSSAVE_ON

#include "gsync_local.h"
#include "gtsnego.naix"
#include "msg/msg_gsync.h"

#if PM_DEBUG
#include "debug/debugwin_sys.h"
#include "test/debug_pause.h"
#endif



struct _GSYNC_DOWNLOAD_WORK {
  DWCNdFileInfo     fileInfo;
  int s_callback_flag;
  int s_callback_result;
  int server_filenum;
  void* pbuffer;
  int size;
  HEAPID heapID;
};

static GSYNC_DOWNLOAD_WORK* _pDLWork = NULL;

/*-------------------------------------------------------------------------*
 * Name        : NdCallback
 * Description : ND用コールバック
 * Arguments   : None.
 * Returns     : None.
 *-------------------------------------------------------------------------*/
static void NdCallback(DWCNdCallbackReason reason, DWCNdError error, int servererror)
{
  OS_Printf("NdCallback: Called\n");
  switch(reason) {
  case DWC_ND_CBREASON_GETFILELISTNUM:
    OS_Printf("DWC_ND_CBREASON_GETFILELISTNUM\n");
    break;
  case DWC_ND_CBREASON_GETFILELIST:
    OS_Printf("DWC_ND_CBREASON_GETFILELIST\n");
    break;
  case DWC_ND_CBREASON_GETFILE:
    OS_Printf("DWC_ND_CBREASON_GETFILE\n");
    break;
  case DWC_ND_CBREASON_INITIALIZE:
    OS_Printf("DWC_ND_CBREASON_INITIALIZE\n");
    break;
  }
	
  switch(error) {
  case DWC_ND_ERROR_NONE:
    OS_Printf("DWC_ND_NOERR\n");
    break;
  case DWC_ND_ERROR_ALLOC:
    OS_Printf("DWC_ND_MEMERR\n");
    break;
  case DWC_ND_ERROR_BUSY:
    OS_Printf("DWC_ND_BUSYERR\n");
    break;
  case DWC_ND_ERROR_HTTP:
    OS_Printf("DWC_ND_HTTPERR\n");
    // ファイル数の取得でＨＴＴＰエラーが発生した場合はダウンロードサーバに繋がっていない可能性が高い
    if( reason == DWC_ND_CBREASON_GETFILELISTNUM )
      {
          OS_Printf( "It is not possible to connect download server.\n." );
          ///	OS_Terminate();
      }
    break;
  case DWC_ND_ERROR_BUFFULL:
    OS_Printf("DWC_ND_BUFFULLERR\n");
    break;
  case DWC_ND_ERROR_DLSERVER:
    OS_Printf("DWC_ND_SERVERERR\n");
    break;
  case DWC_ND_ERROR_CANCELED:
    OS_Printf("DWC_ND_CANCELERR\n");
    break;
  }
  OS_Printf("errorcode = %d\n", servererror);
  _pDLWork->s_callback_flag   = TRUE;
  _pDLWork->s_callback_result = error;

//  NdCleanupCallback();
}




GSYNC_DOWNLOAD_WORK* GSYNC_DOWNLOAD_Create(HEAPID id,int dlsize)
{

  GSYNC_DOWNLOAD_WORK* pWork = GFL_HEAP_AllocClearMemory(id, sizeof(GSYNC_DOWNLOAD_WORK));

  GF_ASSERT(!_pDLWork);
  _pDLWork = pWork;
  pWork->pbuffer = GFL_HEAP_AllocMemory(id, dlsize+64);
  pWork->size = dlsize;
  return pWork;
}


void GSYNC_DOWNLOAD_Exit(GSYNC_DOWNLOAD_WORK* pWork)
{
  GF_ASSERT(_pDLWork);
  GFL_HEAP_FreeMemory(pWork->pbuffer);
  GFL_HEAP_FreeMemory(pWork);
  _pDLWork = NULL;
}

void* GSYNC_DOWNLOAD_GetData(GSYNC_DOWNLOAD_WORK* pWork)
{
  return pWork->pbuffer;
}



BOOL GSYNC_DOWNLOAD_InitAsync(GSYNC_DOWNLOAD_WORK* pWork)
{
  pWork->s_callback_flag = FALSE;
  if( DWC_NdInitAsync( NdCallback, GF_DWC_ND_LOGIN, WIFI_ND_LOGIN_PASSWD ) == FALSE ){
    return FALSE;
  }
  return TRUE;
}

//終わったかどうか
BOOL GSYNC_DOWNLOAD_ResultEnd(GSYNC_DOWNLOAD_WORK* pWork)
{
  if(_pDLWork->s_callback_flag){
    if(pWork->s_callback_result == DWC_ND_ERROR_NONE){
      return TRUE;
    }
  }
  return FALSE;
}

//エラーかどうか
BOOL GSYNC_DOWNLOAD_ResultError(GSYNC_DOWNLOAD_WORK* pWork)
{
  if(_pDLWork->s_callback_flag){
    if(pWork->s_callback_result != DWC_ND_ERROR_NONE){
      return TRUE;
    }
  }
  return FALSE;
}

//落してくる物をセット
BOOL GSYNC_DOWNLOAD_SetAttr(GSYNC_DOWNLOAD_WORK* pWork, char* typestr, int no)
{
  char nobuff[20];
  pWork->s_callback_flag = FALSE;
  OS_TPrintf("   GSYNC_DOWNLOAD_SetAttr       %d \n",no);
  STD_TSNPrintf(nobuff, 20, "%d", no);

  if( DWC_NdSetAttr(typestr, nobuff, "") == FALSE )
  {
    OS_TPrintf( "DWC_NdSetAttr: Failed\n." );
    return FALSE;
  }
  return TRUE;
}


BOOL GSYNC_DOWNLOAD_FileListNumAsync(GSYNC_DOWNLOAD_WORK* pWork)
{
  pWork->s_callback_flag = FALSE;

  if( DWC_NdGetFileListNumAsync( &pWork->server_filenum ) == FALSE )
  {
    OS_TPrintf( "DWC_NdGetFileListNumAsync: Failed.\n" );
    return FALSE;
  }
  return TRUE;
}



BOOL GSYNC_DOWNLOAD_FileListAsync(GSYNC_DOWNLOAD_WORK* pWork)
{
  pWork->s_callback_flag = FALSE;

  if( DWC_NdGetFileListAsync( &pWork->fileInfo, 0, 1 ) == FALSE)
  {
    OS_TPrintf( "DWC_NdGetFileListNumAsync: Failed.\n" );
    return FALSE;
  }
  return TRUE;
}


BOOL GSYNC_DOWNLOAD_FileAsync(GSYNC_DOWNLOAD_WORK* pWork)
{
  pWork->s_callback_flag = FALSE;

  if(DWC_NdGetFileAsync( &pWork->fileInfo, pWork->pbuffer, pWork->size+64) == FALSE){
    OS_TPrintf( "DWC_NdGetFileAsync: Failed.\n" );
    return FALSE;
  }
  return TRUE;
}


static void DWCNdClaenupCallback( void )
{
  _pDLWork->s_callback_flag = TRUE;
  _pDLWork->s_callback_result = DWC_ND_ERROR_NONE;
}


BOOL GSYNC_DOWNLOAD_ExitAsync(GSYNC_DOWNLOAD_WORK* pWork)
{
  pWork->s_callback_flag = FALSE;

  if(DWC_NdCleanupAsync() == FALSE){
    OS_TPrintf( "DWC_NdCleanupAsync: Failed.\n" );
    return FALSE;
  }
  return TRUE;
}

void GSYNC_DOWNLOAD_Main(GSYNC_DOWNLOAD_WORK* pWork)
{
  DWC_NdProcess();
}



