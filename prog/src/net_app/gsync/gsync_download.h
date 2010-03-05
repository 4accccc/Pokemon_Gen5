//============================================================================================
/**
 * @file    gsync_download.h
 * @bfief   GAMESYNCダウンロード関連
 * @author  k.ohno
 * @date    2010.03.05
 */
//============================================================================================

#pragma once

typedef struct _GSYNC_DOWNLOAD_WORK GSYNC_DOWNLOAD_WORK;

extern GSYNC_DOWNLOAD_WORK* GSYNC_DOWNLOAD_Create(HEAPID id,int dlsize);

extern void GSYNC_DOWNLOAD_Exit(GSYNC_DOWNLOAD_WORK* pWork);

extern void* GSYNC_DOWNLOAD_GetData(GSYNC_DOWNLOAD_WORK* pWork);

extern BOOL GSYNC_DOWNLOAD_InitAsync(GSYNC_DOWNLOAD_WORK* pWork);

extern BOOL GSYNC_DOWNLOAD_ResultEnd(GSYNC_DOWNLOAD_WORK* pWork);

extern BOOL GSYNC_DOWNLOAD_ResultError(GSYNC_DOWNLOAD_WORK* pWork);

extern BOOL GSYNC_DOWNLOAD_SetAttr(GSYNC_DOWNLOAD_WORK* pWork, char* typestr, int no);

extern BOOL GSYNC_DOWNLOAD_FileListNumAsync(GSYNC_DOWNLOAD_WORK* pWork);

extern BOOL GSYNC_DOWNLOAD_FileListAsync(GSYNC_DOWNLOAD_WORK* pWork);

extern BOOL GSYNC_DOWNLOAD_FileAsync(GSYNC_DOWNLOAD_WORK* pWork);

extern BOOL GSYNC_DOWNLOAD_ExitAsync(GSYNC_DOWNLOAD_WORK* pWork);

extern void GSYNC_DOWNLOAD_Main(GSYNC_DOWNLOAD_WORK* pWork);

