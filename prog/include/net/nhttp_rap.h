//=============================================================================
/**
 * @file	  nhttp_rap.h
 * @brief	  nhttpでPDWサーバにアクセスする為のラッパー関数
 * @author	k.ohno
 * @date    2009.11.29
 */
//=============================================================================

#pragma once

#include <nitro.h>
#include "gflib.h"
#include <nitroWiFi/nhttp.h>
#include "nitrowifidummy.h"
#include "dreamworld_netdata.h"
#include "webresp_defs.h"
#include "net/network_define.h"


typedef struct _NHTTP_RAP_WORK NHTTP_RAP_WORK;


typedef enum{
  NHTTPRAP_URL_ACCOUNTINFO,
  NHTTPRAP_URL_POKEMONLIST,
  NHTTPRAP_URL_DOWNLOAD,
  NHTTPRAP_URL_UPLOAD,
  NHTTPRAP_URL_ACCOUNT_CREATE,
  NHTTPRAP_URL_BATTLE_DOWNLOAD,
  NHTTPRAP_URL_BATTLE_UPLOAD,
  NHTTPRAP_URL_POKECHK,
//  NHTTPRAP_URL_DECALINFO,
//  NHTTPRAP_URL_CGEAR_DOWNLOAD,
//  NHTTPRAP_URL_MUSICAL_DOWNLOAD,
//  NHTTPRAP_URL_GUIDE_DOWNLOAD,
} NHTTPRAP_URL_ENUM;



extern BOOL NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ENUM urlno,NHTTP_RAP_WORK* pWork);
extern NHTTPConnectionHandle NHTTP_RAP_GetHandle(NHTTP_RAP_WORK* pWork);
extern BOOL NHTTP_RAP_StartConnect(NHTTP_RAP_WORK* pWork);
extern NHTTPError NHTTP_RAP_Process(NHTTP_RAP_WORK* pWork);
extern void* NHTTP_RAP_GetRecvBuffer(NHTTP_RAP_WORK* pWork);

extern NHTTP_RAP_WORK* NHTTP_RAP_Init(HEAPID heapID,u32 profileid);
extern void NHTTP_RAP_End(NHTTP_RAP_WORK* pWork);

//------------------------------------------------------------------------------
/**
 * @brief   認証キー作成開始
 * @param   NHTTP_RAP_WORK* pWork,  ワーク
 * @retval  TRUEなら成功
 */
//------------------------------------------------------------------------------
extern BOOL NHTTP_RAP_SvlGetTokenStart(NHTTP_RAP_WORK* pWork);

//------------------------------------------------------------------------------
/**
 * @brief   認証キー作成中メイン
 * @param   NHTTP_RAP_WORK* pWork,  ワーク
 * @retval  TRUEなら認証キーをおとせた
 */
//------------------------------------------------------------------------------
extern BOOL NHTTP_RAP_SvlGetTokenMain(NHTTP_RAP_WORK* pWork);

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン不正検査領域確保
 * @param   NHTTP_RAP_WORK* pWork,  ワーク
 * @param   HEAPID heapID,          領域確保のID
 * @param   int size                 ポケモン*数+367バイト くらい必要です
 * @param   NHTTP_POKECHK_ENUM type network_define.hにある不正検査タイプ
 * @retval  none
 */
//------------------------------------------------------------------------------
extern void NHTTP_RAP_PokemonEvilCheckCreate(NHTTP_RAP_WORK* pWork, HEAPID heapID, int size, NHTTP_POKECHK_ENUM type);
//------------------------------------------------------------------------------
/**
 * @brief   ポケモン不正検査　ポケモン追加
 * @param   NHTTP_RAP_WORK* pWork,  ワーク
 * @param   pData   ポケモンデータ
 * @param   int size                ポケモン構造体のサイズ
 * @retval  none
 */
//------------------------------------------------------------------------------
extern void NHTTP_RAP_PokemonEvilCheckAdd(NHTTP_RAP_WORK* pWork,void* pData, int size);
//------------------------------------------------------------------------------
/**
 * @brief   ポケモン不正検査 コネクション作成
 * @param   NHTTP_RAP_WORK* pWork,  ワーク
 * @retval  BOOL TRUEならコネクションを作成できた
 */
//------------------------------------------------------------------------------
extern BOOL NHTTP_RAP_PokemonEvilCheckConectionCreate(NHTTP_RAP_WORK* pWork);


#if PM_DEBUG
void NHTTP_DEBUG_GPF_HEADER_PRINT(gs_response* prep);
#endif

