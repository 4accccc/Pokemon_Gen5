//=============================================================================
/**
 * @file    gsync_state.c
 * @brief   ゲームシンク
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date    09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
#include "libdpw/dwci_ghttp.h"
#include "net_app/gsync.h"
#include <nitroWiFi/nhttp.h>
#include "net/nitrowifidummy.h"

#include "system/main.h"  //HEAPID
#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "sound/pm_sndsys.h"
#include "system/wipe.h"
#include "net/network_define.h"
#include "savedata/wifilist.h"
#include "msg/msg_d_ohno.h"
#include "gsync_local.h"
#include "net/nhttp_rap.h"
#include "../../field/event_gsync.h"
#include "savedata/dreamworld_data.h"

#define _TWLDWC_HTTP (1)


typedef void (StateFunc)(G_SYNC_WORK* pState);



//
// PHP とのインターフェース構造体
//
typedef struct tag_EVENT_DATA
{
	unsigned long rom_code;
	unsigned short country_code;
	unsigned char id;
	unsigned char send_flag;
	unsigned long dec_code;
	unsigned short cat_id;
	unsigned short present;
	unsigned char status;
	unsigned char idname1[26];
	unsigned char idname2[26];
	unsigned char idname3[26];
	unsigned char idname4[26];
	unsigned char idname5[26];
}
EVENT_DATA;



static void _changeState(G_SYNC_WORK* pWork,StateFunc* state);
static void _changeStateDebug(G_SYNC_WORK* pWork,StateFunc* state, int line);

#define _NET_DEBUG (1)

#ifdef _NET_DEBUG
#define   _CHANGE_STATE(state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeState(pWork ,state)
#endif //_NET_DEBUG




struct _G_SYNC_WORK {
  HEAPID heapID;
  POKEMON_PARAM* pp;
  NHTTP_RAP_WORK* pNHTTPRap;
  GSYNC_DISP_WORK* pDispWork;  // 描画系
  GSYNC_MESSAGE_WORK* pMessageWork; //メッセージ系
  APP_TASKMENU_WORK* pAppTask;
  BOX_MANAGER * pBox;
  SAVE_CONTROL_WORK* pSaveData;
  u16 trayno;
  u16 indexno;
  StateFunc* state;      ///< ハンドルのプログラム状態
  vu32 count;
  int req;
  int getdataCount;
  BOOL bEnd;
};


static void _ghttpKeyWait(G_SYNC_WORK* pWork);



//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(G_SYNC_WORK* pWork,StateFunc state)
{
  pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef _NET_DEBUG
static void _changeStateDebug(G_SYNC_WORK* pWork,StateFunc state, int line)
{
#ifdef DEBUG_ONLY_FOR_ohno
  OS_TPrintf("gsync: %d\n",line);
#endif
  _changeState(pWork, state);
}
#endif


static void _testwait(G_SYNC_WORK* pWork)
{
  
  if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
    NET_PRINT("終了\n");
    {
      EVENT_DATA* pEvent = (EVENT_DATA*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
      int d,j;
      u32 size;
      
      OS_TPrintf("%d \n",	pEvent->rom_code);
      OS_TPrintf("%d \n",	 pEvent->country_code);
      OS_TPrintf("%d \n",	pEvent->id);
      OS_TPrintf("%d \n",	pEvent->send_flag);
      OS_TPrintf("%d \n",	pEvent->dec_code);
      OS_TPrintf("%d \n",	 pEvent->cat_id);
      OS_TPrintf("%d \n",	 pEvent->present);
      OS_TPrintf("%d \n",	pEvent->status);
      
    }
    _CHANGE_STATE(_ghttpKeyWait);
  }

  

}

static void _testStart(G_SYNC_WORK* pWork)
{

  if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ACCOUNTINFO, pWork->pNHTTPRap)){
    if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
      _CHANGE_STATE(_testwait);
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   キー入力で動きを変える
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpKeyWait(G_SYNC_WORK* pWork)
{
  switch(GFL_UI_KEY_GetTrg())
  {
  case PAD_BUTTON_X:
    _CHANGE_STATE(_testStart);
    break;


  case PAD_BUTTON_Y:
    

    
    GSYNC_DISP_HandInit(pWork->pDispWork);
    break;
  case PAD_BUTTON_B:
    _CHANGE_STATE(NULL);
    break;
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop(G_SYNC_WORK* pWork)
{


}



//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト開始
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _upeffectStart(G_SYNC_WORK* pWork)
{
  GSYNC_DISP_PokemonIconCreate(pWork->pDispWork, PP_GetPPPPointer(pWork->pp));

  if(0){
  }

  _CHANGE_STATE(_ghttpKeyWait);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンを眠るエリアに移動
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _BoxPokeMove(G_SYNC_WORK* pWork)
{
  POKEMON_PASO_PARAM* ppp;
  DREAMWORLD_SAVEDATA* pDream = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);

  GF_ASSERT(DREAMWORLD_SV_GetSleepPokemonFlg(pDream)==FALSE);
  
  if(pWork->pp){
    GFL_HEAP_FreeMemory(pWork->pp);
  }
  
  ppp = BOXDAT_GetPokeDataAddress( pWork->pBox, pWork->trayno, pWork->indexno );
  GF_ASSERT(ppp);
  if(ppp){
    POKEMON_PARAM* pp = PP_CreateByPPP( ppp, pWork->heapID );
    DREAMWORLD_SV_SetSleepPokemon(pDream, pp);
    pWork->pp = pp;
    BOXDAT_ClearPokemon(pWork->pBox, pWork->trayno, pWork->indexno );
    DREAMWORLD_SV_SetSleepPokemonFlg(pDream,TRUE);
  }
  _CHANGE_STATE(_upeffectStart);

}

FS_EXTERN_OVERLAY(dpw_common);


//------------------------------------------------------------------------------
/**
 * @brief   PROC初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT GSYNCProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_GSYNC_WORK* pParent = pwk;
  G_SYNC_WORK* pWork = GFL_PROC_AllocWork( proc, sizeof(G_SYNC_WORK), HEAPID_PROC );
    
  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMESYNC, 0x38000 );
  GFL_STD_MemClear(pWork, sizeof(G_SYNC_WORK));

  pWork->pNHTTPRap = NHTTP_RAP_Init(HEAPID_GAMESYNC);
  pWork->pSaveData = pParent->ctrl;
  pWork->heapID = HEAPID_GAMESYNC;
  pWork->pBox = GAMEDATA_GetBoxManager(GAMESYSTEM_GetGameData(pParent->gsys));

#if PM_DEBUG
  pParent->boxNo=0;
  pParent->boxIndex=0;
#endif
  pWork->trayno = pParent->boxNo;
  pWork->indexno = pParent->boxIndex;
  
  pWork->pDispWork = GSYNC_DISP_Init(pWork->heapID);
  pWork->pMessageWork = GSYNC_MESSAGE_Init(pWork->heapID, NARC_message_gsync_dat);

  
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

  switch(pParent->selectType){
  case  GSYNC_CALLTYPE_INFO:
    _CHANGE_STATE(_ghttpKeyWait);
    break;
  case GSYNC_CALLTYPE_BOXSET:
    _CHANGE_STATE(_BoxPokeMove);
    break;
  }

  return GFL_PROC_RES_FINISH;
}


static GFL_PROC_RESULT GSYNCProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  G_SYNC_WORK* pWork = mywk;
  StateFunc* state = pWork->state;
  GFL_PROC_RESULT ret = GFL_PROC_RES_FINISH;

  if( state ){
    state( pWork );
    ret = GFL_PROC_RES_CONTINUE;
  }

  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }

  GSYNC_DISP_Main(pWork->pDispWork);
  GSYNC_MESSAGE_Main(pWork->pMessageWork);
  
  return ret;
}

static GFL_PROC_RESULT GSYNCProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  G_SYNC_WORK* pWork = mywk;

  GSYNC_MESSAGE_End(pWork->pMessageWork);
  GSYNC_DISP_End(pWork->pDispWork);
  if(pWork->pp){
    GFL_HEAP_FreeMemory(pWork->pp);
  }


  NHTTP_RAP_End(pWork->pNHTTPRap);
  
  GFL_PROC_FreeWork(proc);

  GFL_HEAP_DeleteHeap(HEAPID_GAMESYNC);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ
const GFL_PROC_DATA G_SYNC_ProcData = {
  GSYNCProc_Init,
  GSYNCProc_Main,
  GSYNCProc_End,
};

