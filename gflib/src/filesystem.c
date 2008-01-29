//===================================================================
/**
 * @file	filesystem.c
 * @brief	ROMファイルシステム
 * @author	GAME FREAK Inc.
 * @date	06.11.09
 *
 * $LastChangedDate$
 * $Id$
 */
//===================================================================
#include <nitro.h>
#include <stdint.h>
#include "gflib.h"
#include "filesystem_inter.h"


//------------------------------------------------------------------
/**
 * @brief	ファイルシステム初期化
 * @param	NONE
 * @return	NONE
*/
//------------------------------------------------------------------
void InitFileSystem(void)
{
  u32 file_table_size;
  void* p_table;

  // ファイルシステム初期化
  FS_Init(FS_DMA_NUMBER);
  // ファイル管理テーブルをRAMへ載せる→ファイルへの高速アクセスが可能
  file_table_size = FS_GetTableSize();
  p_table = OS_AllocFromMainArenaLo(file_table_size, 4);
  SDK_ASSERT(p_table != NULL);
  (void)FS_LoadTable(p_table, file_table_size);
  OS_TPrintf("FileTable Size     = 0x%08x bytes.\n", file_table_size);
  OS_TPrintf("remains of MainRAM = 0x%08x bytes.\n", (u32)(OS_GetMainArenaHi())-(u32)(OS_GetMainArenaLo()));
}



//------------------------------------------------------------------
/**
 * @brief	指定ファイルのサイズを返す
 * @param	path	ファイルへのパス
 * @return	int	ファイルサイズ(bytes)
 */
//------------------------------------------------------------------
int GF_GetFileSize(char *path)
{
  FSFile *fp;
  u32 size = 0;

  fp = OS_AllocFromMainArenaLo(sizeof(FSFile), 4);
  if(FS_OpenFile(fp, path) == TRUE){
    size = FS_GetLength(fp);
    (void)FS_CloseFile(fp);
  } else {
    SDK_ASSERT(FALSE);
  }
  return (int)size;
}


//------------------------------------------------------------------
/**
 * @brief	指定ファイルの読み込み
 * @param	path	ファイルへのパス
 * @param	buf	読み出したデータを格納するポインタ
 * @return	int	読み込んだファイルサイズ
 */
//------------------------------------------------------------------
int GF_ReadFile(char *path, void *buf)
{
  FSFile *fp;
  s32 size = 0;

  fp = OS_AllocFromMainArenaLo(sizeof(FSFile), 4);
  if(FS_OpenFile(fp, path) == TRUE){
    size = FS_ReadFile(fp, buf, 0x7fffffff);   //INT_MAX
    (void)FS_CloseFile(fp);
  } else {
    OS_Panic("Panic: Can't open file %s\n", path);
  }
  return (int)size;
}



// 通常のファイルオープン
// ファイルクローズ


/*  */
