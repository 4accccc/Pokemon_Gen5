//=============================================================================
/**
 * @file    dpc_check.c
 * @brief   デジタル選手証の確認
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date    10/01/07
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
#include "libdpw/dwci_ghttp.h"
#include <nitroWiFi/nhttp.h>
#include "net_app/dpc_check.h"
#include "net/nitrowifidummy.h"

#include "system/main.h"  //HEAPID
#include "sound/pm_sndsys.h"
#include "net/network_define.h"
#include "savedata/system_data.h"
#include "net/nhttp_rap.h"
#include "savedata/regulation.h"


#define WIFI_FILE_ATTR1			"REGCARD"
#define WIFI_FILE_ATTR2			""
#define WIFI_FILE_ATTR3			""

#define REGULATION_CARD_DATA_SIZE (342)

typedef void (StateFunc)(DPC_CHECK_WORK* pState);
static void _changeState(DPC_CHECK_WORK* pWork,StateFunc* state);
static void _changeStateDebug(DPC_CHECK_WORK* pWork,StateFunc* state, int line);

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

struct _DPC_CHECK_WORK {
  HEAPID heapID;
  NHTTP_RAP_WORK* pNHTTPRap;
  DPC_CHECK_INITWORK* pParent;
  DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA aDWSBattle;

  SAVE_CONTROL_WORK* pSaveData;
  int trayno;
  int indexno;
  StateFunc* state;      ///< ハンドルのプログラム状態
  vu32 count;
  int req;
  int getdataCount;
  int countTimer;
  BOOL bEnd;
  u32 WifiMatchUpID;
  int server_filenum;
  char WifiMatchUpStr[10];
  char tempBuffer[REGULATION_CARD_DATA_SIZE];
  DWCNdFileInfo fileInfo;
};


static void _ghttpKeyWait(DPC_CHECK_WORK* pWork);



//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(DPC_CHECK_WORK* pWork,StateFunc state)
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
static void _changeStateDebug(DPC_CHECK_WORK* pWork,StateFunc state, int line)
{
#ifdef DEBUG_ONLY_FOR_ohno
  OS_TPrintf("gsync: %d\n",line);
#endif
  _changeState(pWork, state);
}
#endif

static void _networkClose1(DPC_CHECK_WORK* pWork)
{
  if(!GFL_NET_IsInit()){
    _CHANGE_STATE(NULL);
  }
}

static void _networkClose(DPC_CHECK_WORK* pWork)
{
//  pWork->pParent->selectType = GAMESYNC_RETURNMODE_EXIT;
  if(!GFL_NET_IsInit()){
    _CHANGE_STATE(NULL);
  }
  else{
    GFL_NET_Exit(NULL);
    _CHANGE_STATE(_networkClose1);
  }
}


static void _wakeupActio8(DPC_CHECK_WORK* pWork)
{
  if(GFL_UI_KEY_GetTrg()){
    _CHANGE_STATE(_networkClose);
  }
}




//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction_test4(DPC_CHECK_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);

        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
      }
      _CHANGE_STATE(NULL);
    }
  }
}

//テスト 


static void _wakeupAction_test3(DPC_CHECK_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_BATTLE_UPLOAD, pWork->pNHTTPRap)){
      pWork->aDWSBattle.WifiMatchUpState=1;
      NHTTP_AddPostDataRaw(NHTTP_RAP_GetHandle(pWork->pNHTTPRap), &pWork->aDWSBattle, sizeof(DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA) );

      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_wakeupAction_test4);
      }
    }
  }
}


static u8 s_callback_flag   = FALSE;
static DWCNdError s_callback_result = 0;

/*-------------------------------------------------------------------------*
 * Name        : NdCallback
 * Description : ND用コールバック
 * Arguments   : None.
 * Returns     : None.
 *-------------------------------------------------------------------------*/
static void NdCallback(DWCNdCallbackReason reason, DWCNdError error, int servererror)
{
  NET_PRINT("NdCallback: Called\n");
  switch(reason) {
  case DWC_ND_CBREASON_GETFILELISTNUM:
    NET_PRINT("DWC_ND_CBREASON_GETFILELISTNUM\n");
    break;
  case DWC_ND_CBREASON_GETFILELIST:
    NET_PRINT("DWC_ND_CBREASON_GETFILELIST\n");
    break;
  case DWC_ND_CBREASON_GETFILE:
    NET_PRINT("DWC_ND_CBREASON_GETFILE\n");
    break;
  case DWC_ND_CBREASON_INITIALIZE:
    NET_PRINT("DWC_ND_CBREASON_INITIALIZE\n");
    break;
  }
	
  switch(error) {
  case DWC_ND_ERROR_NONE:
    NET_PRINT("DWC_ND_NOERR\n");
    break;
  case DWC_ND_ERROR_ALLOC:
    NET_PRINT("DWC_ND_MEMERR\n");
    break;
  case DWC_ND_ERROR_BUSY:
    NET_PRINT("DWC_ND_BUSYERR\n");
    break;
  case DWC_ND_ERROR_HTTP:
    NET_PRINT("DWC_ND_HTTPERR\n");
    // ファイル数の取得でＨＴＴＰエラーが発生した場合はダウンロードサーバに繋がっていない可能性が高い
    if( reason == DWC_ND_CBREASON_GETFILELISTNUM )
      {
          NET_PRINT( "It is not possible to connect download server.\n." );
          ///	OS_Terminate();
      }
    break;
  case DWC_ND_ERROR_BUFFULL:
    NET_PRINT("DWC_ND_BUFFULLERR\n");
    break;
  case DWC_ND_ERROR_DLSERVER:
    NET_PRINT("DWC_ND_SERVERERR\n");
    break;
  case DWC_ND_ERROR_CANCELED:
    NET_PRINT("DWC_ND_CANCELERR\n");
    break;
  }
  OS_Printf("errorcode = %d\n", servererror);
  s_callback_flag   = TRUE;
  s_callback_result = error;

  //NdCleanupCallback();
}



//------------------------------------------------------------------------------
/**
 * @brief   デジタル選手証ダウンロード
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _downloadDigital9(DPC_CHECK_WORK* pWork)
{





}


//------------------------------------------------------------------------------
/**
 * @brief   デジタル選手証ダウンロード
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _downloadDigital8(DPC_CHECK_WORK* pWork)
{

  if(s_callback_flag){
    if(s_callback_result!=0){  //失敗した
      OS_TPrintf( "DWC_NdGetFileAsync: Failed.\n" );
      GF_ASSERT(0);
      return;
    }
  }
  _CHANGE_STATE(_downloadDigital9);

}

//------------------------------------------------------------------------------
/**
 * @brief   デジタル選手証ダウンロード
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _downloadDigital7(DPC_CHECK_WORK* pWork)
{
  s_callback_flag   = FALSE;
  s_callback_result = 0;

  
  if(DWC_NdGetFileAsync( &pWork->fileInfo,
                         pWork->tempBuffer,
                         REGULATION_CARD_DATA_SIZE) == FALSE){
      OS_TPrintf( "DWC_NdGetFileAsync: Failed.\n" );
      GF_ASSERT(0);
    _CHANGE_STATE(NULL);
      return;
  }
  _CHANGE_STATE(_downloadDigital8);

}











//------------------------------------------------------------------------------
/**
 * @brief   デジタル選手証ダウンロード
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _downloadDigital6(DPC_CHECK_WORK* pWork)
{
  if(s_callback_flag){
    if(s_callback_result!=0){  //失敗した
      NET_PRINT( "info: Failed\n." );
      GF_ASSERT(0);//@todo
      _CHANGE_STATE(NULL);
      return;
    }
    _CHANGE_STATE(_downloadDigital7);
  }
}




//------------------------------------------------------------------------------
/**
 * @brief   デジタル選手証ダウンロード
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _downloadDigital5(DPC_CHECK_WORK* pWork)
{
  s_callback_flag   = FALSE;
  s_callback_result = 0;

  if(DWC_NdGetFileListAsync( &pWork->fileInfo,0,1)){
      NET_PRINT( "DWC_ListNum: Failed\n." );
      GF_ASSERT(0);//@todo
      _CHANGE_STATE(NULL);
      return;
  }
  _CHANGE_STATE(_downloadDigital6);
}


//------------------------------------------------------------------------------
/**
 * @brief   デジタル選手証ダウンロード
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _downloadDigital4(DPC_CHECK_WORK* pWork)
{
  if(s_callback_flag){
    if(pWork->server_filenum!=1){  //データが無い もしくは複数ある
      NET_PRINT( "Num: Failed\n." );
      GF_ASSERT(0);//@todo
      _CHANGE_STATE(NULL);
      return;
    }
    
    _CHANGE_STATE(_downloadDigital5);

  }
}


//------------------------------------------------------------------------------
/**
 * @brief   デジタル選手証ダウンロード
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _downloadDigital3(DPC_CHECK_WORK* pWork)
{
  s_callback_flag   = FALSE;
  s_callback_result = 0;

  if( DWC_NdGetFileListNumAsync( &pWork->server_filenum ) == FALSE ){
    
      NET_PRINT( "DWC_ListNum: Failed\n." );
      GF_ASSERT(0);//@todo
      _CHANGE_STATE(NULL);
      return;
    
  }
  _CHANGE_STATE(_downloadDigital4);


}

//------------------------------------------------------------------------------
/**
 * @brief   デジタル選手証ダウンロード
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _downloadDigital2(DPC_CHECK_WORK* pWork)
{
  if(s_callback_flag){
    if(s_callback_result!=0){
      // エラー終了
      GF_ASSERT(0);//@todo
    }

    // ファイル属性の設定
    if( DWC_NdSetAttr(WIFI_FILE_ATTR1, pWork->WifiMatchUpStr, WIFI_FILE_ATTR3) == FALSE )
    {
      NET_PRINT( "DWC_NdSetAttr: Failed\n." );
      GF_ASSERT(0);//@todo
      _CHANGE_STATE(NULL);
      return;
    }
    _CHANGE_STATE(_downloadDigital3);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   デジタル選手証ダウンロード
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _downloadDigital(DPC_CHECK_WORK* pWork)
{
  if(pWork->pNHTTPRap){
    NHTTP_RAP_End(pWork->pNHTTPRap);
    pWork->pNHTTPRap=NULL;
  }
  s_callback_flag   = FALSE;
  s_callback_result = 0;
  if( DWC_NdInitAsync( NdCallback, GF_DWC_ND_LOGIN, WIFI_ND_LOGIN_PASSWD ) == FALSE ){
    // エラー終了
    GF_ASSERT(0);//@todo
    _CHANGE_STATE(NULL);
    return;
  }
  _CHANGE_STATE(_downloadDigital2);
}

//------------------------------------------------------------------------------
/**
 * @brief   GPFのバトル状態を検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction_test2(DPC_CHECK_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA* pDream = (DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA*)&pEvent[sizeof(gs_response)];

        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
        
        NET_PRINT("ID %x\n",pDream->WifiMatchUpID);
        NET_PRINT("FLG %x\n",pDream->GPFEntryFlg);
        NET_PRINT("ST %d\n",pDream->WifiMatchUpState);

        if(pDream->WifiMatchUpID != 0){
          REGULATION_CARDDATA* pCardData = SaveData_GetRegulationCardData(pWork->pSaveData);
          if(Regulation_GetCardParam(pCardData, REGULATION_CARD_CUPNO) != pDream->WifiMatchUpID){
            // 持ってない選手証なのでダウンロードする
            pWork->WifiMatchUpID = pDream->WifiMatchUpID;
            STD_TSNPrintf(pWork->WifiMatchUpStr, sizeof(pWork->WifiMatchUpStr), "%d", pDream->WifiMatchUpID);
            _CHANGE_STATE(_downloadDigital);
            return;
          }
          


          
        }
      }
      _CHANGE_STATE(_wakeupAction_test3);
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   GPFのバトル状態を検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction_test1(DPC_CHECK_WORK* pWork)
{
  if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_BATTLE_DOWNLOAD, pWork->pNHTTPRap)){
    if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
      _CHANGE_STATE(_wakeupAction_test2);
    }
  }
}



//FS_EXTERN_OVERLAY(dpw_common);


//------------------------------------------------------------------------------
/**
 * @brief   PROC初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT DPC_CheckProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  DPC_CHECK_INITWORK* pParent = pwk;
  DPC_CHECK_WORK* pWork;

 // GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

  pWork = GFL_PROC_AllocWork( proc, sizeof( DPC_CHECK_WORK ), pParent->heapID );
  GFL_STD_MemClear(pWork, sizeof(DPC_CHECK_WORK));

  pWork->heapID = pParent->heapID;

  pWork->pParent = pParent;
  pWork->pSaveData = GAMEDATA_GetSaveControlWork(pParent->gameData);
  
  pWork->pNHTTPRap = NHTTP_RAP_Init(pWork->heapID, MyStatus_GetProfileID(SaveData_GetMyStatus(pWork->pSaveData)));

  _CHANGE_STATE(_wakeupAction_test1);

  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------



static GFL_PROC_RESULT DPC_CheckProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  DPC_CHECK_WORK* pWork = mywk;
  StateFunc* state = pWork->state;
  GFL_PROC_RESULT ret = GFL_PROC_RES_FINISH;

  if( state ){
    state( pWork );
    ret = GFL_PROC_RES_CONTINUE;
  }

  return ret;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT DPC_CheckProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  DPC_CHECK_WORK* pWork = mywk;

  if(pWork->pNHTTPRap){
    NHTTP_RAP_End(pWork->pNHTTPRap);
    pWork->pNHTTPRap=NULL;
  }
  GFL_PROC_FreeWork(proc);
 
  //GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ
const GFL_PROC_DATA DPC_CheckProcData = {
  DPC_CheckProc_Init,
  DPC_CheckProc_Main,
  DPC_CheckProc_End,
};

