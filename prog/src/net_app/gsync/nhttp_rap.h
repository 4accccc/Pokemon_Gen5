//=============================================================================
/**
 * @file	net_command.h
 * @brief	データ共有を行う場合の通信システム
 *          使用する場合に書き込む必要があるものをまとめたヘッダー
 * @author	Katsumi Ohno
 * @date    2005.07.26
 */
//=============================================================================

#pragma once

#include <nitro.h>
#include "gflib.h"
#include <nitroWiFi/nhttp.h>
#include "nitrowifidummy.h"



typedef struct _NHTTP_RAP_WORK NHTTP_RAP_WORK;


typedef enum{
  NHTTPRAP_URL_ACCOUNTINFO,
  NHTTPRAP_URL_POKEMONLIST,
  NHTTPRAP_URL_DOWNLOAD,
  NHTTPRAP_URL_UPLOAD,
} NHTTPRAP_URL_ENUM;



extern BOOL NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ENUM urlno,NHTTP_RAP_WORK* pWork);
extern NHTTPConnectionHandle NHTTP_RAP_GetHandle(NHTTP_RAP_WORK* pWork);
extern BOOL NHTTP_RAP_StartConnect(NHTTP_RAP_WORK* pWork);
extern NHTTPError NHTTP_RAP_Process(NHTTP_RAP_WORK* pWork);
extern void* NHTTP_RAP_GetRecvBuffer(NHTTP_RAP_WORK* pWork);

extern NHTTP_RAP_WORK* NHTTP_RAP_Init(HEAPID heapID);
extern void NHTTP_RAP_End(NHTTP_RAP_WORK* pWork);




