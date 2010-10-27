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
//#include "libdpw/dwci_ghttp.h"
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
#include "pdwacc_local.h"
#include "net/nhttp_rap.h"
#include "net/dwc_error.h"
#include "net/dwc_rap.h"
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


#define _SERVERDOWN_ERROR  (0xfff1)
#define ERROR503  (503)
#define _SERVERMAINTENANCE_ERROR  (0xfff2)
#define ERROR502  (502)
#ifdef BUGFIX_BTS7876_20100716
#define _ETCRESPONCE_ERROR (0xfff3)
#endif


static void _changeState(PDWACC_WORK* pWork,StateFunc* state);
static void _changeStateDebug(PDWACC_WORK* pWork,StateFunc* state, int line);

#if DEBUG_ONLY_FOR_ohno
#define _NET_DEBUG (1)
#else
#define _NET_DEBUG (0)
#endif

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
  GAMEDATA *pGameData;
  void* pTopAddr;
  s32 profileID;
  u16 trayno;
  u16 indexno;
  StateFunc* state;      ///< ハンドルのプログラム状態
  vu32 count;
  int req;
  int getdataCount;
  int countTimer;
  char tempbuffer[30];
  BOOL bEnd;
  BOOL saveFlg;
  BOOL bSaveDataAsync;
  u32 ErrorNo;   ///エラーがあった場合の番号
};


static void _ghttpKeyWait(PDWACC_WORK* pWork);
static void _ghttpInfoWait0(PDWACC_WORK* pWork);
static void _ErrorDisp(PDWACC_WORK* pWork);

//------------------------------------------------------------------------------
/**
 * @brief   レスポンスの検査
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _responceChk(PDWACC_WORK* pWork,int response)
{
  switch(response){
  case ERROR503:
    pWork->ErrorNo = _SERVERDOWN_ERROR;
    _CHANGE_STATE(_ErrorDisp);
    return TRUE;
  case ERROR502:
    pWork->ErrorNo = _SERVERMAINTENANCE_ERROR;
    _CHANGE_STATE(_ErrorDisp);
    return TRUE;
  }
#ifdef BUGFIX_BTS7876_20100716
  if(response >= 400){
    pWork->ErrorNo = _ETCRESPONCE_ERROR;
    _CHANGE_STATE(_ErrorDisp);
    return TRUE;
  }
#endif// BUGFIX_BTS7876_20100716
  
  return FALSE;
}


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


//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeout(PDWACC_WORK* pWork)
{
	if(WIPE_SYS_EndCheck()){
		_CHANGE_STATE( NULL);        // 終わり
	}
}


//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeStart(PDWACC_WORK* pWork)
{
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                  WIPE_FADE_WHITE , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  PMSND_FadeOutBGM(PMSND_FADE_FAST);

  _CHANGE_STATE(_modeFadeout);        // 終わり
}


static void _saveStart4(PDWACC_WORK* pWork)
{
  if( !PDWACC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork) ){
    return;
  }
  if(GFL_UI_KEY_GetTrg()){
    _CHANGE_STATE(_modeFadeStart);
  }
}

static void _saveStart3(PDWACC_WORK* pWork)
{

  int sr=0;

  sr = GAMEDATA_SaveAsyncMain(pWork->pGameData);
  if((sr==SAVE_RESULT_OK) || (sr==SAVE_RESULT_NG)){
    pWork->bSaveDataAsync = FALSE;
    PDWACC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,PDWACC_010);
    _CHANGE_STATE(_saveStart4);
  }
}



static void _saveStart2(PDWACC_WORK* pWork)
{
  if( PDWACC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork) ){
    pWork->bSaveDataAsync=TRUE;
    GAMEDATA_SaveAsyncStart(pWork->pGameData);
    _CHANGE_STATE(_saveStart3);
  }

}

static void _saveStart(PDWACC_WORK* pWork)
{
  PDWACC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,PDWACC_007);
  PDWACC_MESSAGE_WindowTimeIconStart(pWork->pMessageWork);
  _CHANGE_STATE(_saveStart2);
}

static void _createAccount9(PDWACC_WORK* pWork)
{
  if(GFL_UI_KEY_GetTrg()){
    PDWACC_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    DREAMWORLD_SV_SetAccount(DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData),TRUE);
    _CHANGE_STATE(_saveStart);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ゲームシンク完了
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createAccount8(PDWACC_WORK* pWork)
{
  if(GFL_UI_KEY_GetTrg()){
    PDWACC_MESSAGE_NoMessageDisp(pWork->pMessageWork, pWork->profileID, pWork->profileID);
    PDWACC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,PDWACC_008_02);
    _CHANGE_STATE(_createAccount9);
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
  
  PDWACC_MESSAGE_NoMessageDisp(pWork->pMessageWork, pWork->profileID, pWork->profileID);
  PDWACC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,PDWACC_008_01);
  
  _CHANGE_STATE(_createAccount8);
}


//------------------------------------------------------------------------------
/**
 * @brief   エラー表示処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ErrorDisp2(PDWACC_WORK* pWork)
{

  if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL)){
    _CHANGE_STATE(_modeFadeStart);
  }
}


static void _ErrorDisp(PDWACC_WORK* pWork)
{
  int gmm = GSYNC_ERR001 + pWork->ErrorNo - 1;


  if(pWork->pNHTTPRap){
    NHTTP_RAP_ErrorClean(pWork->pNHTTPRap);
  }
  if(_SERVERDOWN_ERROR == pWork->ErrorNo){
    gmm = GSYNC_ERR011;
  }
  else if(_SERVERMAINTENANCE_ERROR == pWork->ErrorNo){
    gmm = GSYNC_ERR012;
  }
#ifdef BUGFIX_BTS7876_20100716
  else if(_ETCRESPONCE_ERROR == pWork->ErrorNo){
    gmm = GSYNC_ERR009;
  }
#endif //BUGFIX_BTS7876_20100716
  else if(pWork->ErrorNo >= DREAM_WORLD_SERVER_ERROR_MAX){
    gmm = GSYNC_ERR009;
  }
  PDWACC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,gmm);
  _CHANGE_STATE(_ErrorDisp2);
}

//---------------------------------


static void _createAccount4_2(PDWACC_WORK* pWork)
{
  if(!PDWACC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  _CHANGE_STATE(_ghttpInfoWait0);
}


static void _createAccount4(PDWACC_WORK* pWork)
{
  PDWACC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,PDWACC_004);
  PDWACC_MESSAGE_WindowTimeIconStart(pWork->pMessageWork);

  _CHANGE_STATE(_createAccount4_2);

}


//------------------------------------------------------------------------------
/**
 * @brief   アカウント作成
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
      _CHANGE_STATE(_modeFadeStart);
    }
    PDWACC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   アカウント作成
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

  
  PDWACC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,PDWACC_003);
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

    {
      int response;
      response = NHTTP_RAP_GetGetResultCode(pWork->pNHTTPRap);
      if(_responceChk(pWork,response)){
        return;
      }
    }
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        gs_response* pEvent = (gs_response*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
        PDWACC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);

        if(pEvent->ret_cd==DREAM_WORLD_SERVER_ALREADY_EXISTS){ //アカウントはすでにある
          _CHANGE_STATE(_createAccount7);
        }
        else if(pEvent->ret_cd==DREAM_WORLD_SERVER_ERROR_NONE){  //アカウント作成完了
          _CHANGE_STATE(_createAccount7);
        }
        else{
          pWork->ErrorNo = pEvent->ret_cd;
          _CHANGE_STATE(_ErrorDisp);
        }
      }
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンアカウント作成
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpInfoWait0(PDWACC_WORK* pWork)
{

#ifdef BUGFIX_GFBTS2030_20101025

  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ACCOUNT_CREATEUPLOAD, pWork->pNHTTPRap)){

      u32 size;
      u8* topAddr = (u8*)SaveControl_GetSaveWorkAdrs(pWork->pSaveData, &size);
      NHTTP_AddPostDataRaw(NHTTP_RAP_GetHandle(pWork->pNHTTPRap), topAddr, 0x80000 );

      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)==NHTTP_ERROR_NONE){
        _CHANGE_STATE(_ghttpInfoWait1);
      }
    }
  }
  else{
    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_ghttpInfoWait1);
    }
  }

#else //BUGFIX_GFBTS2030_20101025

  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ACCOUNT_CREATE, pWork->pNHTTPRap)){
      s32 proid = pWork->profileID;

      GFL_STD_MemClear(pWork->tempbuffer, sizeof(pWork->tempbuffer));
      STD_TSNPrintf(pWork->tempbuffer, sizeof(pWork->tempbuffer), "%d\0\0\0\0\0\0\0\0\0\0\0\0", proid);

      OS_TPrintf("NHTTP_AddPostDataRaw byte %d %d %s\n",proid,STD_StrLen(pWork->tempbuffer),pWork->tempbuffer);
      NHTTP_AddPostDataRaw( NHTTP_RAP_GetHandle(pWork->pNHTTPRap),
                            pWork->tempbuffer, 12 );

      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)==NHTTP_ERROR_NONE){
        _CHANGE_STATE(_ghttpInfoWait1);
      }
    }
  }
  else{
    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_ghttpInfoWait1);
    }
  }
#endif //BUGFIX_GFBTS2030_20101025

}


//------------------------------------------------------------------------------
/**
 * @brief   アクセスコード表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _dispAccCode2(PDWACC_WORK* pWork)
{
	if(!WIPE_SYS_EndCheck()){
    return;
  }
  if(GFL_UI_KEY_GetTrg()){

    PDWACC_MESSAGE_NoMessageEnd(pWork->pMessageWork);

    _CHANGE_STATE(_modeFadeStart);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   アクセスコード表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _dispAccCode(PDWACC_WORK* pWork)
{

  PDWACC_MESSAGE_NoMessageDisp(pWork->pMessageWork, pWork->profileID, pWork->profileID);
  PDWACC_MESSAGE_SystemMessageDisp(pWork->pMessageWork,PDWACC_008);
  _CHANGE_STATE(_dispAccCode2);
}


//ポケモン不正切断コールバック
static void lcheck_DisconnectCallback(void* pUserwork, int code, int type, int ret)
{
  PDWACC_WORK* pWork = pUserwork;

  if(pWork->pNHTTPRap){
    NHTTP_RAP_ErrorClean(pWork->pNHTTPRap);
    NHTTP_RAP_End(pWork->pNHTTPRap);  //この関数を呼ぶ事
    pWork->pNHTTPRap  = NULL;
  }
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
  pWork->pGameData = pParent->gameData;
  pWork->profileID = MyStatus_GetProfileID( GAMEDATA_GetMyStatus(pParent->gameData) );

  if(GFL_NET_IsInit()){
    pWork->pNHTTPRap = NHTTP_RAP_Init(pParent->heapID, pWork->profileID, pParent->pSvl);
    GFL_NET_DWC_SetErrDisconnectCallback(lcheck_DisconnectCallback, pWork );
  }
  pWork->pDispWork = PDWACC_DISP_Init(pWork->heapID);
  pWork->pMessageWork = PDWACC_MESSAGE_Init(pWork->heapID, NARC_message_pdwacc_dat);

  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

  PMSND_PlayBGM(SEQ_BGM_GAME_SYNC);

  switch(pParent->type){
  case PDWACC_GETACC:

    _CHANGE_STATE(_createAccount1);
    break;
  case PDWACC_DISPPASS:
    _CHANGE_STATE(_dispAccCode);
    break;
  }
  pParent->returnCode = PDWACC_RETURNMODE_NONE;

  return GFL_PROC_RES_FINISH;
}


static GFL_PROC_RESULT PDWACCProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDWACC_PROCWORK* pParent = pwk;
  PDWACC_WORK* pWork = mywk;
  StateFunc* state = pWork->state;
  GFL_PROC_RESULT ret = GFL_PROC_RES_FINISH;

#ifdef BUGFIX_GFBTS1989_20100716
  if( state ){
    if(NET_ERR_CHECK_NONE == NetErr_App_CheckError()){
      state( pWork );
    }
    ret = GFL_PROC_RES_CONTINUE;
  }
#else
  if( state ){
    state( pWork );
    ret = GFL_PROC_RES_CONTINUE;
  }
#endif //BUGFIX_GFBTS1989_20100716

  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }

  PDWACC_DISP_Main(pWork->pDispWork);
  PDWACC_MESSAGE_Main(pWork->pMessageWork);


  if(WIPE_SYS_EndCheck()){
    if(NET_ERR_CHECK_NONE != NetErr_App_CheckError()){
      NHTTP_RAP_ErrorClean(pWork->pNHTTPRap);
      if(pWork->bSaveDataAsync){
        GAMEDATA_SaveAsyncCancel( pWork->pGameData );
        pWork->bSaveDataAsync = FALSE;
      }
      WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
      WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
      GFL_NET_DWC_ERROR_ReqErrorDisp(TRUE, FALSE);
      pParent->returnCode = PDWACC_RETURNMODE_ERROR;
      ret = GFL_PROC_RES_FINISH;
    }
  }
#ifdef BUGFIX_GFBTS1989_20100716
  else if(NET_ERR_CHECK_NONE != NetErr_App_CheckError()){
    ret = GFL_PROC_RES_FINISH;
  }
#endif

  
  return ret;
}

static GFL_PROC_RESULT PDWACCProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDWACC_WORK* pWork = mywk;

  if(pWork->pAppTask){
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
  }
  PDWACC_MESSAGE_End(pWork->pMessageWork);
  PDWACC_DISP_End(pWork->pDispWork);
  if(pWork->pp){
    GFL_HEAP_FreeMemory(pWork->pp);
  }
  if(pWork->pTopAddr){
    GFL_HEAP_FreeMemory(pWork->pTopAddr);
  }
  if(pWork->pNHTTPRap){
    NHTTP_RAP_End(pWork->pNHTTPRap);
    pWork->pNHTTPRap=NULL;
  }
  if(GFL_NET_IsInit()){
    GFL_NET_DWC_SetErrDisconnectCallback(NULL,NULL);
  }
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

