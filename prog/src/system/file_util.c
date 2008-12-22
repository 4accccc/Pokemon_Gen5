//============================================================================================
/**
 * @file	file_util.c
 * @brief	ファイルアクセスラッパー
 * @author	tamada GAME FREAK inc.
 * @date	2008.12.19
 *
 * ファイルアクセス用関数について、gflib/arc_tool.hとインターフェイスをそろえたもの
 * 最終的にarc_tool.cに統合する可能性が高い
 */
//============================================================================================

#include <gflib.h>
#include "system/gfl_use.h"

#include "system/file_util.h"

//============================================================================================
//============================================================================================
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
struct _FILEHANDLE {
	FSFile	file;
};
//============================================================================================
//
//
//
//
//
//============================================================================================
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
static void OpenFile(FSFile * p_file, ARCID arcID)
{
	const char * filename;
	filename = GFUser_GetFileNameByArcID(arcID);
	FS_InitFile(p_file);
	FS_OpenFile(p_file, filename);
}

//============================================================================================
//
//
//		ハンドル経由アクセス関数
//
//
//============================================================================================
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
FILEHANDLE * GFL_FILE_OpenHandle(ARCID arcID, HEAPID heapID)
{
	FILEHANDLE * handle;
	handle = GFL_HEAP_AllocMemory(heapID, sizeof(FILEHANDLE)); 
	OpenFile(&handle->file, arcID);
	return handle;
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void GFL_FILE_CloseHandle(FILEHANDLE* handle)
{
	FS_CloseFile(&handle->file);
	GFL_HEAP_FreeMemory(handle);
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
u32 GFL_FILE_GetSizeByHandle(FILEHANDLE* handle)
{
	return FS_GetLength(&handle->file);
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void GFL_FILE_LoadDataByHandle(FILEHANDLE* handle, void * buffer)
{
	u32 len;
	len = FS_GetLength(&handle->file);
	FS_ReadFile(&handle->file, buffer, len);
}

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void* GFL_FILE_LoadDataAllocByHandle(FILEHANDLE* handle, HEAPID heapID)
{
	void * buf;
	u32 len;
	len = FS_GetLength(&handle->file);
	buf = GFL_HEAP_AllocMemory(heapID, len);
	FS_ReadFile(&handle->file, buf, len);
	return buf;
}

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void* GFL_FILE_LoadDataAllocOfsByHandle(FILEHANDLE* handle, HEAPID heapID, u32 ofs, u32 size)
{
	void * buf;
	if (!size) {
		size = FS_GetLength(&handle->file);
	}
	buf = GFL_HEAP_AllocMemory(heapID, size);
	FS_SeekFile(&handle->file, ofs, FS_SEEK_SET);
	FS_ReadFile(&handle->file, buf, size);
	return buf;
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void GFL_FILE_LoadDataOfsByHandle(FILEHANDLE* handle, u32 ofs, u32 size, void * buffer)
{
	FS_SeekFile(&handle->file, ofs, FS_SEEK_SET);
	FS_ReadFile(&handle->file, buffer, size);
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void GFL_FILE_SeekByHandle(FILEHANDLE* handle, u32 seek_size)
{
	FS_SeekFile(&handle->file, seek_size, FS_SEEK_SET);
}

//============================================================================================
//
//
//	ID指定直接アクセス関数
//
//
//============================================================================================
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
u32 GFL_FILE_GetDataSize(ARCID arcID)
{
	FSFile p_file;
	u32 len;
	OpenFile(&p_file, arcID);
	len = FS_GetLength(&p_file);
	FS_CloseFile(&p_file);
	return len;
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void GFL_FILE_LoadData(void * buffer, ARCID arcID)
{
	FSFile	file;
	u32 len;
	OpenFile(&file, arcID);
	len = FS_GetLength(&file);
	FS_ReadFile(&file, buffer, len);
	FS_CloseFile(&file);
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void* GFL_FILE_LoadDataAlloc(ARCID arcID, HEAPID heapID)
{
	void * buffer;
	FSFile	file;
	u32 len;
	OpenFile(&file, arcID);
	len = FS_GetLength(&file);
	buffer = GFL_HEAP_AllocMemory(heapID, len);
	FS_ReadFile(&file, buffer, len);
	FS_CloseFile(&file);
	return buffer;
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void* GFL_FILE_LoadDataAllocOfs(ARCID arcID, HEAPID heapID, u32 ofs, u32 size)
{
	void * buffer;
	FSFile p_file;
	OpenFile(&p_file, arcID);
	if (!size) {
		size = FS_GetLength(&p_file);
	}
	buffer = GFL_HEAP_AllocMemory(heapID, size);
	FS_SeekFile(&p_file, ofs, FS_SEEK_SET);
	FS_ReadFile(&p_file, buffer, size);
	FS_CloseFile(&p_file);

	return buffer;
}

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void GFL_FILE_LoadDataOfs(void * buffer, ARCID arcID, u32 ofs, u32 size)
{
	FSFile p_file;
	OpenFile(&p_file, arcID);
	FS_SeekFile(&p_file, ofs, FS_SEEK_SET);
	FS_ReadFile(&p_file, buffer, size);
	FS_CloseFile(&p_file);
}


