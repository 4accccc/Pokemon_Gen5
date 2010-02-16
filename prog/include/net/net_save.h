//=============================================================================
/**
 * @file	  net_save.h
 * @brief	  通信セーブ エラーになると途中で抜けてきます
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  10/02/15
 */
//=============================================================================

#pragma once

#include "gamesystem/game_data.h"

typedef struct _NET_SAVE_WORK NET_SAVE_WORK;


//------------------------------------------------------------------
/**
 * @brief   通信セーブ開始
 * @param   HEAPID        ヒープ
 * @param   GAMEDATA      ゲームデータ
 * @retval  NET_SAVE_WORK ワーク
 */
//------------------------------------------------------------------

extern NET_SAVE_WORK* NET_SAVE_Init(HEAPID heapID, GAMEDATA* pGameData);


//------------------------------------------------------------------
/**
 * @brief   通信セーブ
 * @param   pNetSaveWork  ワーク
 * @retval  終了したらTRUE
 */
//------------------------------------------------------------------

extern BOOL NET_SAVE_Main(NET_SAVE_WORK* pNetSaveWork);


//------------------------------------------------------------------
/**
 * @brief   通信セーブ終了処理
 * @param   pNetSaveWork  ワーク
 */
//------------------------------------------------------------------

extern void NET_SAVE_Exit(NET_SAVE_WORK* pNetSaveWork);


