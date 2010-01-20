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
//----------------------------------------------------------------
BOOL GF_MCS_FILE_Read( const char * path, void * buf, u32 buf_size )
{
  NNSMcsFile infoRead;
  u32 errCode;
  u32 fileSize;
  u32 readSize;

  if ( NNS_McsIsServerConnect() == FALSE) return FALSE;

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

