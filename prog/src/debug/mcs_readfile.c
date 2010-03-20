//============================================================================================
/**
 * @file  msc_readfile.c
 * @brief
 * @date  2010.01.18
 */
//============================================================================================

#include <gflib.h>
#include <nnsys/mcs.h>

#include "debug/mcs_readfile.h"

//#define MCS_HOOK
//============================================================================================
//============================================================================================
//----------------------------------------------------------------
//----------------------------------------------------------------
static u32 sMcsFileIOWork[ (NNS_MCS_FILEIO_WORKMEM_SIZE + sizeof(u32) - 1 ) / sizeof(u32) ];

//============================================================================================
//============================================================================================


//----------------------------------------------------------------
//----------------------------------------------------------------
void GF_MCS_FILE_Init(void)
{
  NNS_McsInitFileIO( sMcsFileIOWork );
}

//----------------------------------------------------------------
#ifdef MCS_HOOK
extern BOOL mcsResidentFlag;
#endif
//----------------------------------------------------------------
BOOL GF_MCS_FILE_Read( const char * path, void * buf, u32 buf_size )
{
  NNSMcsFile infoRead;
  u32 errCode;
  u32 fileSize;
  u32 readSize;

  OS_TPrintf("MCS起動確認..");

  if ( NNS_McsIsServerConnect() == FALSE) return FALSE;
#ifdef MCS_HOOK
	if(mcsResidentFlag == TRUE){ return FALSE; }
#endif
  OS_TPrintf("確認OK\n");
  // 読み込み用オープン
  errCode = NNS_McsOpenFile(
    &infoRead,
    path, // ファイル名
    NNS_MCS_FILEIO_FLAG_READ | NNS_MCS_FILEIO_FLAG_INCENVVAR ); // 読み込みモード、環境変数展開
  if (errCode != 0)
  {
    // ファイルオープン失敗
    OS_Printf( "ファイル(%s)オープン失敗:%d\n", path, errCode );
    return FALSE;
  }

  fileSize = NNS_McsGetFileSize( &infoRead );
  if ( fileSize >= buf_size )
  {
    //ファイルサイズが大きすぎる
    NNS_McsCloseFile( &infoRead );
    OS_Printf("Too Large FileSize(%d) > bufsize(%d)\n", fileSize, buf_size );
    return FALSE;
  }
  errCode = NNS_McsReadFile(
      &infoRead,
      buf,
      fileSize,
      &readSize);
  if ( errCode != 0 )
  {
    //ファイル読み込み失敗
    NNS_McsCloseFile( &infoRead );
    OS_Printf( "ファイル(%s)読み込み失敗:%d\n", path, errCode );
    return FALSE;
  }
  NNS_McsCloseFile( &infoRead );
  return TRUE;
}

//----------------------------------------------------------------
//----------------------------------------------------------------
void * GF_MCS_FILE_ReadAlloc( const char * path, HEAPID heapID, u32 buf_size )
{
  NNSMcsFile infoRead;
  u32 errCode;
  u32 fileSize;
  u32 readSize;
  void * buf;

  if ( NNS_McsIsServerConnect() == FALSE) return NULL;
#ifdef MCS_HOOK
	if(mcsResidentFlag == TRUE){ return FALSE; }
#endif
  // 読み込み用オープン
  OS_TPrintf( "ファイル(%s)オープン...\n", path );
  errCode = NNS_McsOpenFile(
    &infoRead,
    path, // ファイル名
    NNS_MCS_FILEIO_FLAG_READ | NNS_MCS_FILEIO_FLAG_INCENVVAR ); // 読み込みモード、環境変数展開
  if (errCode != 0)
  {
    // ファイルオープン失敗
    OS_Printf( "ファイル(%s)オープン失敗:%d\n", path, errCode );
    return FALSE;
  }

  fileSize = NNS_McsGetFileSize( &infoRead );
  if ( buf_size > 0 && fileSize >= buf_size )
  {
    //ファイルサイズが大きすぎる
    NNS_McsCloseFile( &infoRead );
    OS_Printf("Too Large FileSize(%d) > bufsize(%d)\n", fileSize, buf_size );
    return FALSE;
  }
  buf = GFL_HEAP_AllocClearMemory( heapID, buf_size != 0 ? buf_size : fileSize );
  errCode = NNS_McsReadFile(
      &infoRead,
      buf,
      fileSize,
      &readSize);
  if ( errCode != 0 )
  {
    //ファイル読み込み失敗
    NNS_McsCloseFile( &infoRead );
    OS_Printf( "ファイル(%s)読み込み失敗:%d\n", path, errCode );
    GFL_HEAP_FreeMemory( buf );
    return NULL;
  }
  NNS_McsCloseFile( &infoRead );
  return buf;
}

