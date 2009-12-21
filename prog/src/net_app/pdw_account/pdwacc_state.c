//=============================================================================
/**
 * @file    pdwacc_state.c
 * @brief   ゲームシンク
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date    09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
#include "libdpw/dwci_ghttp.h"
#include "net_app/pdwacc.h"
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
#include "savedata/system_data.h"
#include "msg/msg_d_ohno.h"
#include "pdwacc_local.h"
#include "net/nhttp_rap.h"
//#include "../../field/event_pdwacc.h"
#include "savedata/dreamworld_data.h"
//#include "pdwacc_obj_NANR_LBLDEFS.h"
#include "pdwacc_poke.cdat"
#include "msg/msg_pdwacc.h"
//#include "pdwacc.naix"


/*
■BGM■

SEQ_BGM_GAME_SYNC	ゲームシンクBGM

■SE■

SEQ_SE_SYS_24			アップ・ダウンロード
SEQ_SE_SYS_25			アップロード完了音
SEQ_SE_SYS_26			ポケモン眠る
 */


typedef void (StateFunc)(PDWACC_WORK* pState);



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



static void _changeState(PDWACC_WORK* pWork,StateFunc* state);
static void _changeStateDebug(PDWACC_WORK* pWork,StateFunc* state, int line);

#define _NET_DEBUG (1)

#ifdef _NET_DEBUG
#define   _CHANGE_STATE(state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeState(pWork ,state)
#endif //_NET_DEBUG




struct _PDWACC_WORK {
  HEAPID heapID;
  POKEMON_PARAM* pp;
  NHTTP_RAP_WORK* pNHTTPRap;
  PDWACC_DISP_WORK* pDispWork;  // 描画系
  PDWACC_MESSAGE_WORK* pMessageWork; //メッセージ系
  APP_TASKMENU_WORK* pAppTask;
  BOX_MANAGER * pBox;
  SAVE_CONTROL_WORK* pSaveData;
  void* pTopAddr;
  u16 trayno;
  u16 indexno;
  StateFunc* state;      ///< ハンドルのプログラム状態
  vu32 count;
  int req;
  int getdataCount;
  int countTimer;
  char tempbuffer[12];
  BOOL bEnd;
};


static void _ghttpKeyWait(PDWACC_WORK* pWork);



//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(PDWACC_WORK* pWork,StateFunc state)
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
static void _changeStateDebug(PDWACC_WORK* pWork,StateFunc state, int line)
{
#ifdef DEBUG_ONLY_FOR_ohno
  OS_TPrintf("pdwacc: %d\n",line);
#endif
  _changeState(pWork, state);
}
#endif




static void _createAccount8(PDWACC_WORK* pWork)
{
  if(!PDWACC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg()){
    PDWACC_MESSAGE_NoMessageEnd(pWork->pMessageWork);
    _CHANGE_STATE(NULL);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   ゲームシンク完了
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createAccount7(PDWACC_WORK* pWork)
{
  int i;
  s32 id = SYSTEMDATA_GetDpwInfo( SaveData_GetSystemData(pWork->pSaveData) );
  u16 crc = GFL_STD_CrcCalc( &id, 4 );
  u64 code = id + crc * 0x100000000;
  OS_TPrintf("id=%x crc=%x code=%x\n",id,crc,code);

  PDWACC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
  
  PDWACC_MESSAGE_NoMessageDisp(pWork->pMessageWork,code);
  PDWACC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,PDWACC_008);
  
  _CHANGE_STATE(_createAccount8);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createAccount6(PDWACC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        EVENT_DATA* pEvent = (EVENT_DATA*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);


      }


      _CHANGE_STATE(_createAccount7);
    }
  }
  else{
    _CHANGE_STATE(_createAccount7);
  }
}

static void _createAccount5(PDWACC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ACCOUNT_CREATE, pWork->pNHTTPRap)){
      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_createAccount6);
      }
    }
  }
  else{
    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_createAccount6);
    }
  }
}

static void _createAccount4_2(PDWACC_WORK* pWork)
{
  if(!PDWACC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  _CHANGE_STATE(_createAccount5);
}


static void _createAccount4(PDWACC_WORK* pWork)
{
  PDWACC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,PDWACC_004);
  //PMSND_PlaySE(SEQ_SE_SYS_24);

  _CHANGE_STATE(_createAccount4_2);

}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモン起こし処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createAccount3(PDWACC_WORK* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    if(selectno==0){
      _CHANGE_STATE(_createAccount4);
    }
    else{
      //@todo 切断処理へ
      _CHANGE_STATE(NULL);
    }
    PDWACC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン起こし処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createAccount2(PDWACC_WORK* pWork)
{
  if(!PDWACC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  pWork->pAppTask = PDWACC_MESSAGE_YesNoStart(pWork->pMessageWork,PDWACC_YESNOTYPE_INFO);

  _CHANGE_STATE(_createAccount3);
}



//------------------------------------------------------------------------------
/**
 * @brief   アカウント作る
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createAccount1(PDWACC_WORK* pWork)
{

//  PDWACC_MESSAGE_SystemMessageDisp();
  
  PDWACC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,PDWACC_003);
  //PMSND_PlaySE(SEQ_SE_SYS_26);

  _CHANGE_STATE(_createAccount2);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpInfoWait1(PDWACC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        gs_response* pEvent = (gs_response*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
        if(pEvent->ret_cd==DREAM_WORLD_SERVER_ALREADY_EXISTS){ //アカウントはすでにある
          _CHANGE_STATE(_createAccount7);
        }
        else if(pEvent->ret_cd==DREAM_WORLD_SERVER_ERROR_NONE){  //アカウント作成完了
          _CHANGE_STATE(_createAccount7);
        }
        else{
#if DEBUG_ONLY_FOR_ohno
          GF_ASSERT(0);
#endif
          _CHANGE_STATE(_createAccount7);
          //その他のエラー
        }
      }
      //アカウントがなければ作成に
//      _CHANGE_STATE(_createAccount1);
    }
  }

  
  

}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpInfoWait0(PDWACC_WORK* pWork)
{

  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ACCOUNT_CREATE, pWork->pNHTTPRap)){

      s32 proid  =  SYSTEMDATA_GetDpwInfo( SaveData_GetSystemData(pWork->pSaveData) );
      
      STD_TSNPrintf(pWork->tempbuffer, sizeof(pWork->tempbuffer), "%d\0", proid);

      OS_TPrintf("NHTTP_AddPostDataRaw byte %d %d\n",proid,GFL_STD_StrLen(pWork->tempbuffer));
      NHTTP_AddPostDataRaw( NHTTP_RAP_GetHandle(pWork->pNHTTPRap), pWork->tempbuffer, GFL_STD_StrLen(pWork->tempbuffer) );

      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_ghttpInfoWait1);
      }
    }
  }
  else{
    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_ghttpInfoWait1);
    }
  }


}


//------------------------------------------------------------------------------
/**
 * @brief   アクセスコード表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _dispAccCode2(PDWACC_WORK* pWork)
{
}

//------------------------------------------------------------------------------
/**
 * @brief   アクセスコード表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _dispAccCode(PDWACC_WORK* pWork)
{

  
  PDWACC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,PDWACC_008);
  
  _CHANGE_STATE(_dispAccCode2);



}




FS_EXTERN_OVERLAY(dpw_common);



//------------------------------------------------------------------------------
/**
 * @brief   PROC初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT PDWACCProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDWACC_PROCWORK* pParent = pwk;
  PDWACC_WORK* pWork;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));
  pWork = GFL_PROC_AllocWork( proc, sizeof( PDWACC_WORK ), pParent->heapID );
  GFL_STD_MemClear(pWork, sizeof(PDWACC_WORK));

  pWork->heapID = pParent->heapID;
  pWork->pSaveData = GAMEDATA_GetSaveControlWork(pParent->gameData);
  pWork->pNHTTPRap = NHTTP_RAP_Init(pParent->heapID, SYSTEMDATA_GetDpwInfo(SaveData_GetSystemData(pWork->pSaveData)));

  switch(pParent->type){
  case PDWACC_GETACC:
    _CHANGE_STATE(_ghttpInfoWait0);
    break;
  case PDWACC_DISPPASS:
    _CHANGE_STATE(_createAccount7);
    break;
  }
  
  pWork->pDispWork = PDWACC_DISP_Init(pWork->heapID);
  pWork->pMessageWork = PDWACC_MESSAGE_Init(pWork->heapID, NARC_message_pdwacc_dat);

  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

  PMSND_PlayBGM(SEQ_BGM_GAME_SYNC);


  return GFL_PROC_RES_FINISH;
}


static GFL_PROC_RESULT PDWACCProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDWACC_WORK* pWork = mywk;
  StateFunc* state = pWork->state;
  GFL_PROC_RESULT ret = GFL_PROC_RES_FINISH;

  if( state ){
    state( pWork );
    ret = GFL_PROC_RES_CONTINUE;
  }

  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }

  PDWACC_DISP_Main(pWork->pDispWork);
  PDWACC_MESSAGE_Main(pWork->pMessageWork);

  return ret;
}

static GFL_PROC_RESULT PDWACCProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDWACC_WORK* pWork = mywk;

  PDWACC_MESSAGE_End(pWork->pMessageWork);
  PDWACC_DISP_End(pWork->pDispWork);
  if(pWork->pp){
    GFL_HEAP_FreeMemory(pWork->pp);
  }
  if(pWork->pTopAddr){
    GFL_HEAP_FreeMemory(pWork->pTopAddr);
  }


  NHTTP_RAP_End(pWork->pNHTTPRap);


  GFL_PROC_FreeWork(proc);

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ
const GFL_PROC_DATA PDW_ACC_ProcData = {
  PDWACCProc_Init,
  PDWACCProc_Main,
  PDWACCProc_End,
};

