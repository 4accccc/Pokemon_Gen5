//============================================================================================
/**
 * @file    pdwacc_mainproc.c
 * @brief   PROC：アカウント作成
 * @author  k.ohno
 * @date    2009.12.08
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "net_app/wifi_login.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "net_app/pdwacc.h"


typedef struct {
  WIFILOGIN_PARAM     login;
  GAMEDATA      *gameData;
  PDWACC_PROCWORK pwdaccWork;
  HEAPID heapID;
} PDWACCMAIN_WORK;




typedef enum{
  _WIFI_LOGIN,
  _WIFI_ACCOUNT,
} _WIFI_STATE_LABEL;



//------------------------------------------------------------------------------
/**
 * @brief   PROC初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT PDWACCProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
//  EVENT_PDWACC_WORK* pParent = pwk;
  PDWACCMAIN_WORK* pWork;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PDWACC, 0x78000 );
  pWork = GFL_PROC_AllocWork( proc, sizeof( PDWACCMAIN_WORK ), HEAPID_PDWACC );
  GFL_STD_MemClear(pWork, sizeof(PDWACCMAIN_WORK));
  pWork->heapID = HEAPID_PDWACC;
  pWork->gameData = GAMEDATA_Create( HEAPID_PDWACC );
  (*seq) = 0;

  return GFL_PROC_RES_FINISH;
}


FS_EXTERN_OVERLAY(wifi_login);
FS_EXTERN_OVERLAY(pdw_acc);
extern const GFL_PROC_DATA WiFiLogin_ProcData;
extern const GFL_PROC_DATA PDW_ACC_ProcData;

static GFL_PROC_RESULT PDWACCProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDWACCMAIN_WORK* pWork = mywk;

  switch (*seq) {
  case _WIFI_LOGIN:
    pWork->login.gsys = NULL;
    pWork->login.gamedata = pWork->gameData;
    pWork->login.bDreamWorld = FALSE;

    GFL_PROC_SysCallProc(FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, &pWork->login);

    (*seq)++;
    break;

  case _WIFI_ACCOUNT:
    pWork->pwdaccWork.gameData = pWork->gameData;
    pWork->pwdaccWork.heapID = pWork->heapID;
    GFL_PROC_SysCallProc(FS_OVERLAY_ID(pdw_acc), &PDW_ACC_ProcData, &pWork->pwdaccWork);
    (*seq)++;
    break;

  default:
    return GFL_PROC_RES_FINISH;
    break;
    
  }

  return GFL_PROC_RES_CONTINUE;
}

static GFL_PROC_RESULT PDWACCProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDWACCMAIN_WORK* pWork = mywk;


  GAMEDATA_Delete(pWork->gameData);

  GFL_PROC_FreeWork(proc);

  GFL_HEAP_DeleteHeap(HEAPID_PDWACC);
  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ
const GFL_PROC_DATA PDW_ACC_MainProcData = {
  PDWACCProc_Init,
  PDWACCProc_Main,
  PDWACCProc_End,
};

