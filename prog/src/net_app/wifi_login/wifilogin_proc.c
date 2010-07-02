//============================================================================================
/**
 * @file  wifilogin_proc.c
 * @bfief GTSネゴシエーションProcState
 * @author  k.ohno
 * @date  2009.11.14
 */
//============================================================================================

#include <gflib.h>
#include <dwc.h>

#include "arc_def.h"
#include "net_app/wifi_login.h"
#include "net_app/connect_anm.h"
#include "net/network_define.h"
#include "net/dwc_rap.h"
#include "net/nhttp_rap.h"
#include "net/dwc_error.h"

#include "system/net_err.h"
#include "system/main.h"
#include "system/wipe.h"
#include "system/ds_system.h"
#include "gamesystem/msgspeed.h" // MSGSPEED_GetWait

#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"

#include "app/app_printsys_common.h"

#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"

#include "savedata/wifilist.h"
#include "savedata/system_data.h"
#include "savedata/regulation.h"
#include "savedata/battle_box_save.h"

//#include "msg/msg_gtsnego.h"
#include "msg/msg_wifi_system.h"
#include "../../field/event_gtsnego.h"
#include "wifi_login.naix"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK
#include "wifilogin_local.h"

#if DEBUG_ONLY_FOR_ohno
#define _NET_DEBUG (1)
#elif defined(DEBUG_ONLY_FOR_toru_nagihashi)
#define _NET_DEBUG (1)
#else
#define _NET_DEBUG (0)
#endif

#define _WORK_HEAPSIZE (0x1000)  // 調整が必要


//--------------------------------------------
// 画面構成定義
//--------------------------------------------

//BG面とパレット番号
#define _SUBSCREEN_BGPLANE	(GFL_BG_FRAME3_S)
#define _SUBSCREEN_PALLET	(0xE)

#define STOP_TIME_ (60)

//--------------------------------------------
// 通信初期化構造体
//--------------------------------------------

static void _connectCallBack(void* pWk, int netID);
static void _RecvFirstData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void* _BeaconGetFunc(void* pWork);
static int _BeaconGetSizeFunc(void* pWork);
static BOOL _BeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo, void* pWork);
static void* _getMyUserData(void* pWork);  //DWCUserData
static void* _getFriendData(void* pWork);  //DWCFriendData
static void _deleteFriendList(int deletedIndex,int srcIndex, void* pWork);
static void _modeFadeStart(WIFILOGIN_WORK* pWork);
static void _callbackFunciton(WIFILOGIN_WORK* pWork);
static void _logoutStart(WIFILOGIN_WORK* pWork);
static void _logoutWait(WIFILOGIN_WORK* pWork);


///通信コマンドテーブル
static const NetRecvFuncTable _PacketTbl[] = {
  {_RecvFirstData,         NULL},    ///NET_CMD_FIRST
};

#define _MAXNUM   (2)         // 最大接続人数
#define _MAXSIZE  (100)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数

static GFLNetInitializeStruct aGFLNetInit = {
  _PacketTbl,  // 受信関数テーブル
  NELEMS(_PacketTbl), // 受信テーブル要素数
  NULL,    ///< ハードで接続した時に呼ばれる
  NULL,//_connectCallBack,    ///< ネゴシエーション完了時にコール
  NULL,   // ユーザー同士が交換するデータのポインタ取得関数
  NULL,   // ユーザー同士が交換するデータのサイズ取得関数
  _BeaconGetFunc,  // ビーコンデータ取得関数
  _BeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
  _BeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,            // 普通のエラーが起こった場合 通信終了
  FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NULL, //_endCallBack,  // 通信切断時に呼ばれる関数
  NULL,  // オート接続で親になった場合
  NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  _deleteFriendList,  ///< wifiフレンドリスト削除コールバック
  _getFriendData,   ///< DWC形式の友達リスト
  _getMyUserData,  ///< DWCのユーザデータ（自分のデータ）
  GFL_NET_DWC_HEAPSIZE,   ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
  SYACHI_NETWORK_GGID,  
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_IRC,   //赤外線通信用にcreateされるHEAPID
  GFL_WICON_POSX, GFL_WICON_POSY,        // 通信アイコンXY位置
  _MAXNUM,     // 最大接続人数
  _MAXSIZE,  //最大送信バイト数
  _BCON_GET_NUM,    // 最大ビーコン収集数
  TRUE,     // CRC計算
  FALSE,     // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIFI,  //wifi通信を行うかどうか
  TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_NOP_SERVICEID,  //GameServiceID
  0xfffe,	// 赤外線タイムアウト時間
  0,//MP通信親機送信バイト数
  0,//dummy
};



//--------------------------------------------
// 内部ワーク
//--------------------------------------------


enum _IBMODE_SELECT {
  _SELECTMODE_GSYNC = 0,
  _SELECTMODE_UTIL,
  _SELECTMODE_EXIT
};




typedef void (StateFunc)(WIFILOGIN_WORK* pState);
typedef BOOL (TouchFunc)(int no, WIFILOGIN_WORK* pState);


struct _WIFILOGIN_WORK {
  StateFunc* state;      ///< ハンドルのプログラム状態
  TouchFunc* touch;
  int selectType;   // 接続タイプ
  HEAPID heapID;
  int connect_bit;
  int timer;
  BOOL connect_ok;
  BOOL receive_ok;
  BOOL bInitMessage;
  BOOL bSaving;
  NHTTP_RAP_WORK* pNHTTP;
  SAVE_CONTROL_WORK* pSave;
  WIFILOGIN_YESNO_WORK* pSelectWork;  //選択肢
  WIFILOGIN_DISP_WORK* pDispWork;  // 描画系
  WIFILOGIN_MESSAGE_WORK* pMessageWork; //メッセージ系
  WIFI_LIST* pList;
  WIFILOGIN_PARAM * dbw;  //親のワーク
  GAMEDATA* gamedata;
};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(WIFILOGIN_WORK* pWork,StateFunc* state);
static void _changeStateDebug(WIFILOGIN_WORK* pWork,StateFunc* state, int line);
static void _profileIDCheck(WIFILOGIN_WORK* pWork);
static void _modeSvlGetStart(WIFILOGIN_WORK* pWork);
static void _modeSvlGetMain(WIFILOGIN_WORK* pWork);
static void _modeErrorRetry(WIFILOGIN_WORK* pWork);

static void _checkError( WIFILOGIN_WORK* pWork ); //シーケンス内で使用して下さい


#if PM_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG



static BOOL _keyTouchCheck(WIFILOGIN_WORK* pWork)
{
  if(GFL_UI_KEY_GetTrg() & APP_PRINTSYS_COMMON_TRG_KEY){
    return TRUE;
  }
  if( pWork->dbw->bg != WIFILOGIN_BG_NORMAL ){
    if(GFL_UI_TP_GetTrg()){
      return TRUE;
    }
  }

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

static void _changeState(WIFILOGIN_WORK* pWork,StateFunc state)
{
  pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(WIFILOGIN_WORK* pWork,StateFunc state, int line)
{
  NET_PRINT("log: %d\n",line);
  _changeState(pWork, state);
}
#endif


//--------------------------------------------------------------
/**
 * @brief   接続完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------
#if 0
static void _connectCallBack(void* pWk, int netID)
{
  WIFILOGIN_WORK* pWork = pWk;
  u32 temp;

  OS_TPrintf("ネゴシエーション完了 netID = %d\n", netID);
  pWork->connect_bit |= 1 << netID;
  temp = pWork->connect_bit;
  if(MATH_CountPopulation(temp) >= 2){
    OS_TPrintf("全員のネゴシエーション完了 人数bit=%x\n", pWork->connect_bit);
    pWork->connect_ok = TRUE;
  }
}
#endif
//--------------------------------------------------------------
/**
 * @brief   移動コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------
static void _RecvFirstData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  WIFILOGIN_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_SystemGetCurrentID()){
    return;	//自分のデータは無視
  }
  pWork->receive_ok = TRUE;
  OS_TPrintf("最初のデータ受信コールバック netID = %d\n", netID);
}



typedef struct{
  int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { 0 };


//--------------------------------------------------------------
/**
 * @brief   ビーコンデータ取得関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------

static void* _BeaconGetFunc(void* pWork)
{
  return &_testBeacon;
}

///< ビーコンデータサイズ取得関数
static int _BeaconGetSizeFunc(void* pWork)
{
  return sizeof(_testBeacon);
}

///< ビーコンデータ取得関数
static BOOL _BeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo, void* pWork)
{
  if(myNo != beaconNo){
    return FALSE;
  }
  return TRUE;
}


static void* _getMyUserData(void* pWork)  //DWCUserData
{
  WIFILOGIN_WORK *wk = pWork;
  return WifiList_GetMyUserInfo(GAMEDATA_GetWiFiList(wk->gamedata));
}

static void* _getFriendData(void* pWork)  //DWCFriendData
{
  WIFILOGIN_WORK *wk = pWork;
  return WifiList_GetDwcDataPtr(GAMEDATA_GetWiFiList(wk->gamedata),0);
}

static void _deleteFriendList(int deletedIndex,int srcIndex, void* pWork)
{
  WIFILOGIN_WORK *wk = pWork;
  WifiList_DataMarge(GAMEDATA_GetWiFiList(wk->gamedata), deletedIndex, srcIndex);
}


//----------------------------------------------------------------------------
/**
 *	@brief	ボタンイベントコールバック
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void _BttnCallBack( u32 bttnid, u32 event, void* p_work )
{
  WIFILOGIN_WORK *pWork = p_work;
  u32 friendNo;

  switch( event ){
  case GFL_BMN_EVENT_TOUCH:		///< 触れた瞬間
    if(pWork->touch!=NULL){
      if(pWork->touch(bttnid, pWork)){
        return;
      }
    }
    break;
  default:
    break;
  }
}


static void _exitEnd( WIFILOGIN_WORK* pWork)
{
  if( !WIFILOGIN_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork) ){
    return;
  }
  pWork->timer--;
  if(pWork->timer==0){
    _CHANGE_STATE(pWork, _callbackFunciton);
  }
}

static void _exitEnd2( WIFILOGIN_WORK* pWork)
{
	if(!WIPE_SYS_EndCheck()){
    return;
  }
  if( !WIFILOGIN_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork) ){
    return;
  }
  if(_keyTouchCheck(pWork)){
    _CHANGE_STATE(pWork, _callbackFunciton);
  }
}

//------------------------------------------------------------------
/**
 * $brief   終了確認メッセージ  WIFIP2PMATCH_MODE_EXIT_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static void _exitExiting( WIFILOGIN_WORK *pWork )
{
  if( !WIFILOGIN_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)  ){
    return;
  }
  if(pWork->timer == 1){
    pWork->timer = 0;
    GFL_NET_Exit(NULL);
  }
  if(!GFL_NET_IsInit()){
    WifiList_FormUpData(pWork->pList);  // 切断した時にフレンドコードを詰める
    WIFILOGIN_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0012);
    _CHANGE_STATE(pWork, _exitEnd);
    pWork->timer = STOP_TIME_;
  }
}


//------------------------------------------------------------------
/**
 * $brief   ボタンを押すと終了  WIFIP2PMATCH_MODE_CHECK_AND_END
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static void _CheckAndEnd( WIFILOGIN_WORK *pWork )
{
  if(pWork->timer > 0){
    pWork->timer--;
    return;
  }
  if(_keyTouchCheck(pWork)){
    pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
    WIFILOGIN_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0011);
    _CHANGE_STATE(pWork, _exitExiting);

  }
}

//------------------------------------------------------------------------------
/**
 * @brief   キー押したら終了
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _saveEndWait(WIFILOGIN_WORK* pWork)
{
  if(_keyTouchCheck(pWork)){
    WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    if( pWork->dbw->bg == WIFILOGIN_BG_NORMAL )
    {
      WIFILOGIN_MESSAGE_TitleDisp( pWork->pMessageWork, dwc_title_0000);
    }
    if(pWork->dbw->pSvl){
      _CHANGE_STATE(pWork, _modeSvlGetStart);  //認証
    }
    else{
      _CHANGE_STATE(pWork, _callbackFunciton);  //接続完了
    }
  }
  else
  {
    _checkError(pWork);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   接続汎用関数
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _connectingCommonWait(WIFILOGIN_WORK* pWork)
{
  if(GFL_NET_StateIsWifiLoginState()){
    if( pWork->bInitMessage ){     // 初回接続時にはセーブが完了した事を通知

      WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
      WIFILOGIN_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
      if(DS_SYSTEM_IsRunOnTwl()){
        WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwctwl_message_0004);
      }
      else{
        WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0004);
      }
      _CHANGE_STATE(pWork, _saveEndWait);
    }
    else{
      if(pWork->dbw->pSvl){
        _CHANGE_STATE(pWork, _modeSvlGetStart);  //認証
      }
      else{
        _CHANGE_STATE(pWork, _callbackFunciton);  //接続完了
      }
    }
  }
  else{
    _checkError(pWork);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   セーブ中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _saveingWait(WIFILOGIN_WORK* pWork)
{
  if(GFL_NET_DWC_GetSaving()){

    SAVE_RESULT result = GAMEDATA_SaveAsyncMain(pWork->gamedata);
    if (result != SAVE_RESULT_CONTINUE && result != SAVE_RESULT_LAST) {
      GFL_NET_DWC_ResetSaving();
    }
    else{
      return;  //セーブ中に他の遷移を禁じる
    }
  }
  _connectingCommonWait(pWork);
}


//------------------------------------------------------------------------------
/**
 * @brief   セーブのメッセージがで終わるまで待ち
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _saveingStart(WIFILOGIN_WORK* pWork)
{
  if(!WIFILOGIN_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
//    _connectingCommonWait(pWork);  //ここで状態が遷移してしまうため エラーもいらない    _checkError(pWork);
  }
  else{
    MyStatus_SetProfileID( GAMEDATA_GetMyStatus(pWork->gamedata), WifiList_GetMyGSID(pWork->pList) );
    
    GAMEDATA_SaveAsyncStart(pWork->gamedata);

//    GFL_NET_DWC_SaveAsyncInit(pWork->pSave);
    pWork->bSaving=TRUE;
    _CHANGE_STATE(pWork, _saveingWait);
    return;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   接続中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _connectingWait(WIFILOGIN_WORK* pWork)
{
  if(!WIFILOGIN_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if( GFL_NET_DWC_GetSaving() && pWork->bSaving==FALSE 
      && pWork->dbw->mode != WIFILOGIN_MODE_NOTSAVE
      ) {  //セーブの必要が生じた
    WIFILOGIN_MESSAGE_InfoMessageDispWaitIcon(pWork->pMessageWork, dwc_message_0015);
    _CHANGE_STATE(pWork, _saveingStart);
  }
  else{
    _connectingCommonWait(pWork);
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   接続開始
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _connectionStart(WIFILOGIN_WORK* pWork)
{
  if( OS_IsRunOnTwl() ){//DSIならマッチングのメモリが大きくとられる為、領域を多くしないといけない
    aGFLNetInit.heapSize = GFL_NET_DWCLOBBY_HEAPSIZE;
  }
  else{
    aGFLNetInit.heapSize = GFL_NET_DWC_HEAPSIZE;
  }
  if(pWork->dbw->nsid == WB_NET_BSUBWAY){
    aGFLNetInit.heapSize = GFL_NET_DWC_BSUBWAY_HEAPSIZE;
  }
  else if(pWork->dbw->nsid == WB_NET_WIFIMATCH){
    aGFLNetInit.heapSize = GFL_NET_DWC_RNDMATCH_HEAPSIZE;
  }

  if(pWork->dbw->nsid != WB_NET_WIFICLUB){
    aGFLNetInit.keyList=NULL;
  }
  
  aGFLNetInit.gsid  = pWork->dbw->nsid;
  _testBeacon.gameNo = pWork->dbw->nsid;

  GFL_NET_Init(&aGFLNetInit, NULL, pWork);	//通信初期化
  GFL_NET_StateWifiEnterLogin();

  if( pWork->dbw->display == WIFILOGIN_DISPLAY_DOWN )
  { 
    GFL_NET_ReloadIconTopOrBottom( FALSE, pWork->heapID );
  }
  else
  { 
    GFL_NET_ReloadIconTopOrBottom( TRUE, pWork->heapID );
  }

  WIFILOGIN_MESSAGE_InfoMessageDispWaitIcon(pWork->pMessageWork, dwc_message_0008);

  WIFILOGIN_DISP_StartSmoke(pWork->pDispWork);

  _CHANGE_STATE(pWork,_connectingWait);
  
}

//------------------------------------------------------------------------------
/**
 * @brief   接続確認のはいいいえまち
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeProfileWait2(WIFILOGIN_WORK* pWork)
{
  if(WIFILOGIN_MESSAGE_YesNoIsFinish(pWork->pSelectWork)){
    int selectno = WIFILOGIN_MESSAGE_YesNoGetCursorPos(pWork->pSelectWork);

    if(selectno==0){
      pWork->dbw->result  = WIFILOGIN_RESULT_LOGIN;
      _CHANGE_STATE(pWork,_connectionStart);
    }
    else{
      pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
      _CHANGE_STATE(pWork,_callbackFunciton);
    }
    WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    if( pWork->dbw->bg == WIFILOGIN_BG_NORMAL )
    {
      WIFILOGIN_MESSAGE_TitleDisp(pWork->pMessageWork, dwc_title_0000);
    }
    WIFILOGIN_MESSAGE_YesNoEnd(pWork->pSelectWork);
    pWork->pSelectWork=NULL;
    if(pWork->dbw->bg==WIFILOGIN_BG_NORMAL){
  //    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , 0 );
    }
  }
}

static void _modeLoginWait2(WIFILOGIN_WORK* pWork)
{

  if(WIFILOGIN_MESSAGE_YesNoIsFinish(pWork->pSelectWork)){
    int selectno = WIFILOGIN_MESSAGE_YesNoGetCursorPos(pWork->pSelectWork);

    if(selectno==0){
      pWork->dbw->result  = WIFILOGIN_RESULT_LOGIN;
      _CHANGE_STATE(pWork,_connectionStart);
    }
    else{
      pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
      _CHANGE_STATE(pWork,_callbackFunciton);
    }
    WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    WIFILOGIN_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
    WIFILOGIN_MESSAGE_YesNoEnd(pWork->pSelectWork);
    pWork->pSelectWork=NULL;
    if(pWork->dbw->bg==WIFILOGIN_BG_NORMAL){
  //    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , 0 );

      WIFILOGIN_MESSAGE_TitleDisp(pWork->pMessageWork, dwc_title_0000);
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   接続確認のメッセージ出力待ち
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeProfileWait(WIFILOGIN_WORK* pWork)
{
	if(!WIPE_SYS_EndCheck()){
    return;
  }
  if(!WIFILOGIN_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  pWork->pSelectWork = WIFILOGIN_MESSAGE_YesNoStart(pWork->pMessageWork,WIFILOGIN_YESNOTYPE_SYS,(pWork->dbw->bg==WIFILOGIN_BG_NORMAL),0);
  _CHANGE_STATE(pWork,_modeProfileWait2);
}

static void _modeErrorRetry(WIFILOGIN_WORK* pWork)
{
	if(!WIPE_SYS_EndCheck()){
    return;
  }
    WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
    pWork->pSelectWork = WIFILOGIN_MESSAGE_YesNoStart(pWork->pMessageWork,WIFILOGIN_YESNOTYPE_SYS,(pWork->dbw->bg==WIFILOGIN_BG_NORMAL),0);
    _CHANGE_STATE(pWork,_modeLoginWait2);
}

//------------------------------------------------------------------------------
/**
 * @brief   エラーをチェックし、検出したならばメッセージ表示後、リトライへ行く処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _checkError( WIFILOGIN_WORK* pWork )
{
  if( NetErr_App_CheckError() ){
    const GFL_NETSTATE_DWCERROR* cp_error  =  GFL_NET_StateGetWifiError();

    switch( cp_error->errorType )
    { 
    case DWC_ETYPE_LIGHT:
    case DWC_ETYPE_SHOW_ERROR:
    case DWC_ETYPE_DISCONNECT:
    case DWC_ETYPE_SHUTDOWN_FM:
    case DWC_ETYPE_SHUTDOWN_GHTTP:
    case DWC_ETYPE_SHUTDOWN_ND:
      //エラー画面
      WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
      WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
      WIFILOGIN_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
      NetErr_DispCallPushPop();
      WIFILOGIN_DISP_ResetErrorDisplay(pWork->pDispWork);

      //強制切断
      if( GFL_NET_IsInit() )
      {
        NetErr_ExitNetSystem();
      }

      //エラークリア
      GFL_NET_StateClearWifiError();
      NetErr_ErrWorkInit();
      GFL_NET_StateResetError();

      //再接続へ
      WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0019);
      _CHANGE_STATE(pWork,_modeErrorRetry);
      break;

    case DWC_ETYPE_FATAL:
      //Fatal
      NetErr_DispCallFatal();
      break;
    }
  }
}

static void _modeLoginWait(WIFILOGIN_WORK* pWork)
{
	if(!WIPE_SYS_EndCheck()){
    return;
  }
  if(!WIFILOGIN_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
 pWork->pSelectWork = WIFILOGIN_MESSAGE_YesNoStart(pWork->pMessageWork,WIFILOGIN_YESNOTYPE_INFO,(pWork->dbw->bg==WIFILOGIN_BG_NORMAL),0);
  _CHANGE_STATE(pWork,_modeLoginWait2);
}

static void _modeDifferDSWait7(WIFILOGIN_WORK* pWork)
{
  if(WIFILOGIN_MESSAGE_YesNoIsFinish(pWork->pSelectWork)){
    int selectno = WIFILOGIN_MESSAGE_YesNoGetCursorPos(pWork->pSelectWork);

    if(selectno==0){

      REGULATION_SAVEDATA *p_reg_sv = SaveData_GetRegulationSaveData( GAMEDATA_GetSaveControlWork(pWork->gamedata) );
      REGULATION_CARDDATA *p_reg_card = RegulationSaveData_GetRegulationCard( p_reg_sv, REGULATION_CARD_TYPE_WIFI );
      BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( GAMEDATA_GetSaveControlWork(pWork->gamedata) );

      //リタイヤ扱いにしてバトルボックスのロック解除
      BATTLE_BOX_SAVE_OffLockFlg( p_bbox_save, BATTLE_BOX_LOCK_BIT_WIFI );
      Regulation_SetCardParam( p_reg_card, REGULATION_CARD_STATUS, DREAM_WORLD_MATCHUP_CHANGE_DS );

      pWork->dbw->result  = WIFILOGIN_RESULT_LOGIN;
      WifiList_Init(pWork->pList);
      _CHANGE_STATE(pWork,_connectionStart);
    }
    else{
      pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
      _CHANGE_STATE(pWork,_callbackFunciton);
    }
    WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    if( pWork->dbw->bg == WIFILOGIN_BG_NORMAL )
    {
      WIFILOGIN_MESSAGE_TitleDisp(pWork->pMessageWork, dwc_title_0000);
    }
    WIFILOGIN_MESSAGE_YesNoEnd(pWork->pSelectWork);
    pWork->pSelectWork=NULL;
  }
}

static void _modeDifferDSWait6(WIFILOGIN_WORK* pWork)
{
  pWork->pSelectWork = WIFILOGIN_MESSAGE_YesNoStart(pWork->pMessageWork,WIFILOGIN_YESNOTYPE_SYS,(pWork->dbw->bg==WIFILOGIN_BG_NORMAL),1);
  _CHANGE_STATE(pWork,_modeDifferDSWait7);
}

static void _modeDifferDSWait5(WIFILOGIN_WORK* pWork)
{
  if(WIFILOGIN_MESSAGE_YesNoIsFinish(pWork->pSelectWork)){
    int selectno = WIFILOGIN_MESSAGE_YesNoGetCursorPos(pWork->pSelectWork);

    WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    if( pWork->dbw->bg == WIFILOGIN_BG_NORMAL )
    {
      WIFILOGIN_MESSAGE_TitleDisp(pWork->pMessageWork, dwc_title_0000);
    }
    WIFILOGIN_MESSAGE_YesNoEnd(pWork->pSelectWork);
    pWork->pSelectWork=NULL;

    if(selectno==0){

      REGULATION_SAVEDATA *p_reg_sv = SaveData_GetRegulationSaveData( GAMEDATA_GetSaveControlWork(pWork->gamedata) );
      REGULATION_CARDDATA *p_reg_card = RegulationSaveData_GetRegulationCard( p_reg_sv, REGULATION_CARD_TYPE_WIFI );
      u32 cupNo   = Regulation_GetCardParam( p_reg_card, REGULATION_CARD_CUPNO );
      u32 status  = Regulation_GetCardParam( p_reg_card, REGULATION_CARD_STATUS );

      if( cupNo != 0 && 
          ( DREAM_WORLD_MATCHUP_SIGNUP <= status  && status <= DREAM_WORLD_MATCHUP_RETIRE ) )
      { 

        //WIFI大会に参加していたらもう一度確認
        WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
        WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0020);
        _CHANGE_STATE(pWork,_modeDifferDSWait6);
      }
      else
      { 
        pWork->dbw->result  = WIFILOGIN_RESULT_LOGIN;
        WifiList_Init(pWork->pList);
        _CHANGE_STATE(pWork,_connectionStart);
      }
    }
    else{
      pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
      _CHANGE_STATE(pWork,_callbackFunciton);
    }
  }
}



static void _modeDifferDSWait4(WIFILOGIN_WORK* pWork)
{

  pWork->pSelectWork = WIFILOGIN_MESSAGE_YesNoStart(pWork->pMessageWork,WIFILOGIN_YESNOTYPE_SYS,(pWork->dbw->bg==WIFILOGIN_BG_NORMAL),1);
  _CHANGE_STATE(pWork,_modeDifferDSWait5);
}

static void _modeDifferDSWait3(WIFILOGIN_WORK* pWork)
{
  if(WIFILOGIN_MESSAGE_YesNoIsFinish(pWork->pSelectWork)){
    int selectno = WIFILOGIN_MESSAGE_YesNoGetCursorPos(pWork->pSelectWork);

    if(selectno==0){
      pWork->dbw->result  = WIFILOGIN_RESULT_LOGIN;

      WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
      WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0007);
      _CHANGE_STATE(pWork,_modeDifferDSWait4);
    }
    else{
      pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
      WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
      if( pWork->dbw->bg == WIFILOGIN_BG_NORMAL )
      {
        WIFILOGIN_MESSAGE_TitleDisp(pWork->pMessageWork, dwc_title_0000);
      }
      _CHANGE_STATE(pWork,_callbackFunciton);
    }
    WIFILOGIN_MESSAGE_YesNoEnd(pWork->pSelectWork);
    pWork->pSelectWork=NULL;
  }
}



static void _modeDifferDSWait2(WIFILOGIN_WORK* pWork)
{


  pWork->pSelectWork = WIFILOGIN_MESSAGE_YesNoStart(pWork->pMessageWork,WIFILOGIN_YESNOTYPE_SYS,(pWork->dbw->bg==WIFILOGIN_BG_NORMAL),1);
  _CHANGE_STATE(pWork,_modeDifferDSWait3);
}


static void _modeDifferDSWait(WIFILOGIN_WORK* pWork)
{
	if(!WIPE_SYS_EndCheck()){
    return;
  }
  if(_keyTouchCheck(pWork)){
    WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
    if(DS_SYSTEM_IsRunOnTwl()){
      WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwctwl_message_0006);
    }
    else{
      WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0006);
    }
    _CHANGE_STATE(pWork,_modeDifferDSWait2);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   最初の接続のプロファイル分岐
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _profileIDCheck(WIFILOGIN_WORK* pWork)
{
  if(!DS_SYSTEM_IsAvailableWireless() ) //無線通信不可
  { 
    WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
    WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0021);
    pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
    _CHANGE_STATE(pWork,_exitEnd2);
  }
  else if(!DS_SYSTEM_IsAgreeEULA()){  ///EULA検査
    WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
    WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0018);
    pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
    _CHANGE_STATE(pWork,_exitEnd2);
  }
  //強制セーブ無し
  else if( pWork->dbw->mode == WIFILOGIN_MODE_NOTSAVE )
  { 
    WIFILOGIN_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0002);
    _CHANGE_STATE(pWork,_modeLoginWait);
  }
  else if( !DWC_CheckHasProfile( WifiList_GetMyUserInfo(pWork->pList) ) )
  {
    // 初めての場合
    pWork->bInitMessage=TRUE;
    WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
    if(!DS_SYSTEM_IsRunOnTwl()){
      WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0003);
    }
    else{
      WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwctwl_message_0003);
    }
    _CHANGE_STATE(pWork,_modeProfileWait);
  }
  else if( !DWC_CheckValidConsole(WifiList_GetMyUserInfo(pWork->pList)) )
  {  //別DSの場合
    WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
    if(!DS_SYSTEM_IsRunOnTwl()){
      WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0005);
    }
    else{
      WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwctwl_message_0005);
    }
    _CHANGE_STATE(pWork,_modeDifferDSWait);
  }
  else if( pWork->dbw->mode == WIFILOGIN_MODE_ERROR ) //エラーで再びログインへ
  { 
    WIFILOGIN_MESSAGE_TitleEnd(pWork->pMessageWork);
    WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0019);
    _CHANGE_STATE(pWork,_modeErrorRetry);
  }
  else  //普通の接続
  {
    WIFILOGIN_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0002);
    _CHANGE_STATE(pWork,_modeLoginWait);
  }

}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeout(WIFILOGIN_WORK* pWork)
{
	if(WIPE_SYS_EndCheck()){
		_CHANGE_STATE(pWork, NULL);        // 終わり
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   コールバック処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _callbackFunciton(WIFILOGIN_WORK* pWork)
{ 
  if( pWork->dbw->login_after_callback && pWork->dbw->result == WIFILOGIN_RESULT_LOGIN )
  { 
    WIFILOGIN_CALLBACK_RESULT result;

    result  = pWork->dbw->login_after_callback( pWork->pMessageWork, pWork->dbw->p_callback_wk );

    if( result != WIFILOGIN_CALLBACK_RESULT_CONTINUE )
    { 
      switch( result )
      { 
      case WIFILOGIN_CALLBACK_RESULT_SUCCESS:
        pWork->dbw->result  = WIFILOGIN_RESULT_LOGIN;
        _CHANGE_STATE(pWork,_modeFadeStart);
        break;
      case WIFILOGIN_CALLBACK_RESULT_FAILED:
        pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
        //_CHANGE_STATE(pWork,_logoutStart);

        if( NET_ERR_CHECK_NONE == NetErr_App_CheckError() )
        {
          GFL_NET_StateSetWifiError(0, DWC_ETYPE_SHUTDOWN_GHTTP, 0, 0 );
          NetErr_ErrorSet();
        }

        _checkError(pWork);
        break;
      }
    }
  }
  else
  { 
    _CHANGE_STATE(pWork,_modeFadeStart);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ログアウト開始
 */
//-----------------------------------------------------------------------------
static void _logoutStart(WIFILOGIN_WORK* pWork)
{ 
  if( GFL_NET_IsExit() )
  {
    _CHANGE_STATE(pWork,_modeFadeStart);
  }
  else
  { 
    //初期化されていたら解放
    if( GFL_NET_Exit( NULL ) )
    { 
      _CHANGE_STATE(pWork,_logoutWait);
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ログアウト中
 */
//-----------------------------------------------------------------------------
static void _logoutWait(WIFILOGIN_WORK* pWork)
{ 
  if( GFL_NET_IsExit() )
  { 
    _CHANGE_STATE(pWork,_modeFadeStart);
  }
}
//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeStart(WIFILOGIN_WORK* pWork)
{
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  if( pWork->dbw->bg == WIFILOGIN_BG_NORMAL )
  { 
    if( pWork->dbw->bgm == WIFILOGIN_BGM_NORMAL )
    { 
      PMSND_FadeOutBGM( 8 );
    }
  }
  _CHANGE_STATE(pWork,_modeFadeout);        // 終わり
}


static void _FadeWait(WIFILOGIN_WORK* pWork)
{
  if(!GFL_FADE_CheckFade()){
    return;
  }
  _CHANGE_STATE(pWork,NULL);
}


//------------------------------------------------------------------------------
/**
 * @brief   SVL認証
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _modeSvlStart2(WIFILOGIN_WORK* pWork)
{
  if( DWC_SVLGetTokenAsync("", pWork->dbw->pSvl) ){
    _CHANGE_STATE(pWork, _modeSvlGetMain);
  }
  else
  {
    //失敗したらエラーだしてもう一度
    _checkError(pWork );
  }
}


static void _modeSvlGetStart(WIFILOGIN_WORK* pWork)
{
//  pWork->pNHTTP = NHTTP_RAP_Init(pWork->heapID,
  //                               MyStatus_GetProfileID(GAMEDATA_GetMyStatus(pWork->gamedata)),
    //                             pWork->dbw->pSvl );
  _CHANGE_STATE(pWork, _modeSvlStart2);
}


static void _modeSvlGetMain(WIFILOGIN_WORK* pWork)
{
	DWCSvlState		state;
	DWCError		dwcerror;
	DWCErrorType	dwcerrortype;
	int				errorcode;
  DWCSvlResult* pSvl = pWork->dbw->pSvl;

  state = DWC_SVLProcess();
  if(state == DWC_SVL_STATE_SUCCESS) {
    NET_PRINT("Succeeded to get SVL Status\n");
    NET_PRINT("status = %s\n", pSvl->status==TRUE ? "TRUE" : "FALSE");
    NET_PRINT("svlhost = %s\n", pSvl->svlhost);
    NET_PRINT("svltoken = %s\n", pSvl->svltoken);
    _CHANGE_STATE(pWork, _callbackFunciton);
  }
  else if(state == DWC_SVL_STATE_ERROR
      || state == DWC_SVL_STATE_DIRTY 
      || state == DWC_SVL_STATE_CANCELED ) {
    NET_PRINT("SVL error.\n");
    _checkError(pWork );
  }

}

//------------------------------------------------------------------------------
/**
 * @brief   セーブ中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReporting(WIFILOGIN_WORK* pWork)
{

  if(!WIFILOGIN_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }


  {
    SAVE_RESULT svr = GAMEDATA_SaveAsyncMain(pWork->gamedata);
    
    if(svr == SAVE_RESULT_OK){
      WIFILOGIN_MESSAGE_InfoMessageEnd(pWork->pMessageWork);

      GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, 1);

      _CHANGE_STATE(pWork,_FadeWait);
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCスタート
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT WiFiLogin_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  WIFILOGIN_PARAM* pEv=pwk;
  WIFILOGIN_WORK *pWork;
	

  GF_ASSERT_MSG( pEv->nsid != 0, "ゲームサービスIDを入れてください\n" );

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFILOGIN, 0x18000 );

  {
    pWork = GFL_PROC_AllocWork( proc, sizeof( WIFILOGIN_WORK ), HEAPID_WIFILOGIN );
    GFL_STD_MemClear(pWork, sizeof(WIFILOGIN_WORK));
    pWork->heapID = HEAPID_WIFILOGIN;
    pWork->gamedata = pEv->gamedata;
    pWork->pDispWork = WIFILOGIN_DISP_Init(pWork->heapID,pEv->bg,pEv->display);
    pWork->pMessageWork = WIFILOGIN_MESSAGE_Init(pWork->heapID, NARC_message_wifi_system_dat,pEv->display);
    pWork->pSave = GAMEDATA_GetSaveControlWork(pEv->gamedata);
    pWork->pList = GAMEDATA_GetWiFiList(pEv->gamedata);

//    GFL_NET_ChangeIconPosition

    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
									WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

    pWork->dbw = pwk;

    if(GFL_NET_IsInit()){       // 接続中
      GF_ASSERT(0);

      //製品版でここに来た時のため処理を入れておく
      GFL_NET_StateClearWifiError();
      GFL_NET_StateResetError();
      NetErr_ExitNetSystem();
      NetErr_ErrWorkInit();
    }

    //接続開始 プロファイル検査
    _CHANGE_STATE(pWork,_profileIDCheck);
  }

  if( pWork->dbw->bg == WIFILOGIN_BG_NORMAL )
  { 
    WIFILOGIN_MESSAGE_TitleDisp(pWork->pMessageWork, dwc_title_0000);

    if( pWork->dbw->bgm == WIFILOGIN_BGM_NORMAL )
    { 
      PMSND_PlayBGM( SEQ_BGM_WIFI_ACCESS );
    }
  }
  else{
    if( pWork->dbw->bgm == WIFILOGIN_BGM_NORMAL )
    { 
      PMSND_PlayBGM(SEQ_BGM_GAME_SYNC);
    }
  }

  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT WiFiLogin_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  WIFILOGIN_WORK* pWork = mywk;
  WIFILOGIN_PARAM* pEv=pwk;
  GFL_PROC_RESULT retCode = GFL_PROC_RES_FINISH;

  //メインプロセスの下にあるど同じフレームないでTRG検知してしまうので、
  //メインプロセスの上にあげました
  WIFILOGIN_DISP_Main(pWork->pDispWork);
  WIFILOGIN_MESSAGE_Main(pWork->pMessageWork);

  //メインプロセス
  { 
    StateFunc* state = pWork->state;
    if(state != NULL){
      state(pWork);
      retCode = GFL_PROC_RES_CONTINUE;
    }
  }

  //
  if(pWork->pSelectWork){
    WIFILOGIN_MESSAGE_YesNoUpdate(pWork->pSelectWork);
  }

  return retCode;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT WiFiLogin_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  WIFILOGIN_PARAM* pEv=pwk;
  WIFILOGIN_WORK* pWork = mywk;

//  EVENT_IrcBattleSetType(pParentWork, pWork->selectType);

	GFL_BG_FreeBGControl(_SUBSCREEN_BGPLANE);


  WIFILOGIN_MESSAGE_End(pWork->pMessageWork);
  WIFILOGIN_DISP_End(pWork->pDispWork);

  GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_WIFILOGIN);
  //EVENT_IrcBattle_SetEnd(pParentWork);


	
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA WiFiLogin_ProcData = {
  WiFiLogin_ProcInit,
  WiFiLogin_ProcMain,
  WiFiLogin_ProcEnd,
};


