//=============================================================================
/**
 * @file	  net_save.c
 * @bfief	  通信セーブ
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  10/02/15
 */
//=============================================================================

#include <gflib.h>


#include "net/network_define.h"

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/net_err.h"

#include "net/net_save.h"

#if DEBUG_ONLY_FOR_ohno
#define _NETSAVE_DEBUG (1)
#else
#define _NETSAVE_DEBUG (0)
#endif

typedef enum
{
  _CHANGERAND = 60,    ///< ポケモン交換タイミング揺らぎ数
  _TIMING_ENDNO=112,
  _TIMING_SAVEST,
  _TIMING_SAVECHK,
  _TIMING_SAVELAST,
  _TIMING_SAVEEND,
  _TIMING_ANIMEEND,

} NET_TIMING_ENUM;



typedef void (StateFunc)(NET_SAVE_WORK* pState);

struct _NET_SAVE_WORK{
  StateFunc* state;
  GAMEDATA* pGameData;
  int saveStep;
};


#if _TRADE_DEBUG

static void _changeStateDebug(NET_SAVE_WORK* pWork,StateFunc* state, int line);

#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)

#else  //_NET_DEBUG

static void _changeState(NET_SAVE_WORK* pWork,StateFunc* state);

#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)

#endif //_NET_DEBUG


static void _changeTimingSaveStart(NET_SAVE_WORK* pWork);
static void _changeTimingSaveStart2(NET_SAVE_WORK* pWork);
static void _changeTimingSaveStart3(NET_SAVE_WORK* pWork);
static void _changeTimingSaveStart32(NET_SAVE_WORK* pWork);
static void _changeTimingSaveStart4(NET_SAVE_WORK* pWork);
static void _changeTimingSaveStart5(NET_SAVE_WORK* pWork);
static void _changeDemo_ModelTrade25(NET_SAVE_WORK* pWork);
static void _changeDemo_ModelTrade26(NET_SAVE_WORK* pWork);



//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(NET_SAVE_WORK* pWork,StateFunc state)
{
  pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#if _TRADE_DEBUG
void _changeStateDebug(NET_SAVE_WORK* pWork,StateFunc state, int line)
{
  OS_TPrintf("ns: %d\n",line);
  _changeState(pWork, state);
}
#endif


//------------------------------------------------------------------
/**
 * @brief   ポケモン交換開始
 * @param   NET_SAVE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------

NET_SAVE_WORK* NET_SAVE_Init(HEAPID heapID, GAMEDATA* pGameData)
{
  NET_SAVE_WORK* pWork = GFL_HEAP_AllocClearMemory(heapID,sizeof(NET_SAVE_WORK));

  pWork->pGameData = pGameData;
  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(), _TIMING_SAVEST, WB_NET_TRADE_SERVICEID);
  _CHANGE_STATE(pWork, _changeTimingSaveStart2);
  return pWork;
}


//------------------------------------------------------------------
/**
 * @brief   通信セーブ
 * @param   pNetSaveWork  ワーク
 * @retval  終了したらTRUE
 */
//------------------------------------------------------------------

BOOL NET_SAVE_Main(NET_SAVE_WORK* pNetSaveWork)
{
  if(pNetSaveWork->state){
    pNetSaveWork->state(pNetSaveWork);
    return FALSE;
  }
  return TRUE;
}


//------------------------------------------------------------------
/**
 * @brief   通信セーブ終了処理
 * @param   pNetSaveWork  ワーク
 */
//------------------------------------------------------------------

void NET_SAVE_Exit(NET_SAVE_WORK* pNetSaveWork)
{
  GFL_HEAP_FreeMemory(pNetSaveWork);
}




static void _changeTimingSaveStart2(NET_SAVE_WORK* pWork)
{
  if(NET_ERR_CHECK_NONE!=NetErr_App_CheckError()){  //エラーなら終了
    _CHANGE_STATE(pWork,NULL);
  }
  else if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVEST,WB_NET_TRADE_SERVICEID)){
    GAMEDATA_SaveAsyncStart(pWork->pGameData);
    _CHANGE_STATE(pWork,_changeTimingSaveStart3);
  }
}

static void _changeTimingSaveStart3(NET_SAVE_WORK* pWork)
{
  if(GAMEDATA_SaveAsyncMain(pWork->pGameData) == SAVE_RESULT_LAST){
    if(NET_ERR_CHECK_NONE!=NetErr_App_CheckError()){  //エラーなら終了
      _CHANGE_STATE(pWork,NULL);
    }
    else{
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(), _TIMING_SAVECHK, WB_NET_TRADE_SERVICEID);
      pWork->saveStep = GFUser_GetPublicRand(_CHANGERAND);
      _CHANGE_STATE(pWork,_changeTimingSaveStart32);
    }
  }
}

static void _changeTimingSaveStart32(NET_SAVE_WORK* pWork)
{
  if(NET_ERR_CHECK_NONE!=NetErr_App_CheckError()){  //エラーなら終了
    _CHANGE_STATE(pWork,NULL);
  }
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVECHK,WB_NET_TRADE_SERVICEID)){
    _CHANGE_STATE(pWork,_changeTimingSaveStart4);
  }
}


static void _changeTimingSaveStart4(NET_SAVE_WORK* pWork)
{
  if(NET_ERR_CHECK_NONE!=NetErr_App_CheckError()){  //エラーなら終了
    _CHANGE_STATE(pWork,NULL);
  }
  pWork->saveStep--;
  if(pWork->saveStep<0){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVELAST,WB_NET_TRADE_SERVICEID);
    _CHANGE_STATE(pWork, _changeTimingSaveStart5);
  }
}

static void _changeTimingSaveStart5(NET_SAVE_WORK* pWork)
{
  if(NET_ERR_CHECK_NONE!=NetErr_App_CheckError()){  //エラーなら終了
    _CHANGE_STATE(pWork,NULL);
  }
  else if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVELAST,WB_NET_TRADE_SERVICEID)){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade25);
  }
}

static void _changeDemo_ModelTrade25(NET_SAVE_WORK* pWork)
{
  if(GAMEDATA_SaveAsyncMain(pWork->pGameData) == SAVE_RESULT_OK){
    if(NET_ERR_CHECK_NONE!=NetErr_App_CheckError()){  //エラーなら終了
      _CHANGE_STATE(pWork,NULL);
    }
    else{
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVEEND, WB_NET_TRADE_SERVICEID);
      _CHANGE_STATE(pWork,_changeDemo_ModelTrade26);
    }
  }
}

static void _changeDemo_ModelTrade26(NET_SAVE_WORK* pWork)
{
  if(NET_ERR_CHECK_NONE!=NetErr_App_CheckError()){  //エラーなら終了
    _CHANGE_STATE(pWork,NULL);
  }
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVEEND, WB_NET_TRADE_SERVICEID)){
    _CHANGE_STATE(pWork, NULL);
  }
}

