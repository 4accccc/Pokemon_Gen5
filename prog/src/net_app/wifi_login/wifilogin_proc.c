//============================================================================================
/**
 * @file  gtsnego_proc.c
 * @bfief GTSネゴシエーションProcState
 * @author  k.ohno
 * @date  2009.11.14
 */
//============================================================================================

#include <gflib.h>
#include <dwc.h>

#include "arc_def.h"
#include "net_app/wifi_login.h"
#include "net/network_define.h"
#include "net/dwc_rap.h"

#include "system/main.h"
#include "system/wipe.h"
#include "gamesystem/msgspeed.h" // MSGSPEED_GetWait

#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"

#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"

#include "savedata/wifilist.h"
#include "savedata/system_data.h"

//#include "msg/msg_gtsnego.h"
#include "msg/msg_wifi_system.h"
#include "../../field/event_gtsnego.h"
#include "wifi_login.naix"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK
#include "wifilogin_local.h"

#if DEBUG_ONLY_FOR_ohno
#define _NET_DEBUG (1)
#else
#define _NET_DEBUG (0)
#endif

#define _WORK_HEAPSIZE (0x1000)  // 調整が必要


//--------------------------------------------
// 画面構成定義
//--------------------------------------------

//BG面とパレット番号(仮設定
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
static BOOL _BeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo);
static void* _getMyUserData(void* pWork);  //DWCUserData
static void* _getFriendData(void* pWork);  //DWCFriendData
static void _deleteFriendList(int deletedIndex,int srcIndex, void* pWork);
static void _modeFadeStart(WIFILOGIN_WORK* pWork);


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
  SYASHI_NETWORK_GGID,  
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_IRC,   //※check　赤外線通信用にcreateされるHEAPID
  GFL_WICON_POSX, GFL_WICON_POSY,        // 通信アイコンXY位置
  _MAXNUM,     // 最大接続人数
  _MAXSIZE,  //最大送信バイト数
  _BCON_GET_NUM,    // 最大ビーコン収集数
  TRUE,     // CRC計算
  FALSE,     // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIFI,  //wifi通信を行うかどうか
  TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_COMPATI_CHECK,  //GameServiceID
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



#if PM_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG




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
  NET_PRINT("neg: %d\n",line);
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

static _testBeaconStruct _testBeacon = { WB_NET_COMPATI_CHECK };


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
static BOOL _BeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo)
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
    _CHANGE_STATE(pWork, _modeFadeStart);
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
  if( GFL_UI_KEY_GetTrg() ){
    WIFILOGIN_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0011);
    _CHANGE_STATE(pWork, _exitExiting);

  }
}

//------------------------------------------------------------------
/**
 * $brief   エラー表示
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static void _retryInit(WIFILOGIN_WORK* pWork)
{
}


//------------------------------------------------------------------
/**
 * $brief   エラー表示
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static void _errorDisp(WIFILOGIN_WORK* pWork)
{
  GFL_NETSTATE_DWCERROR* pErr = GFL_NET_StateGetWifiError();
  int msgno,no = pErr->errorCode;
  int type =  GFL_NET_DWC_ErrorType(pErr->errorCode, pErr->errorType);

  if(type == 11){
    msgno = dwc_error_0015;
    type = 11;
  }
  else if(no == ERRORCODE_HEAP){
    msgno = dwc_error_0014;
    type = 12;
  }
  else{
    if( type >= 0 ){
      msgno = dwc_error_0001 + type;
    }else{
      msgno = dwc_error_0012;
    }
  }
  NET_PRINT("エラーメッセージ %d \n",msgno);
  //EndMessageWindowOff(pWork);

  // エラーメッセージを表示しておくシンク数
  pWork->timer = STOP_TIME_;

  WIFILOGIN_MESSAGE_ErrorMessageDisp(pWork->pMessageWork, msgno, no);
//  WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, msgno);
  
  switch(type){
  case 1:
  case 4:
  case 5:
  case 11:
    _CHANGE_STATE(pWork,_retryInit);
    break;
  case 6:
  case 7:
  case 8:
  case 9:
    _CHANGE_STATE(pWork,_retryInit);
    break;
  case 10:
    _CHANGE_STATE(pWork,_retryInit);
    break;
  case 0:
  case 2:
  case 3:
  default:
    _CHANGE_STATE(pWork, _CheckAndEnd);
    break;
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
  if(GFL_UI_KEY_GetTrg()  || GFL_UI_TP_GetTrg()){
    WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    _CHANGE_STATE(pWork, _modeFadeStart);  //接続完了
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
      WIFILOGIN_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
      WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0004);
      _CHANGE_STATE(pWork, _saveEndWait);
    }
    else{
      _CHANGE_STATE(pWork, _modeFadeStart);  //接続完了
    }
  }
  else if(GFL_NET_StateIsWifiError() || (GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_TIMEOUT)){
    _errorDisp(pWork);
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
    _connectingCommonWait(pWork);
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
  if( GFL_NET_DWC_GetSaving() && pWork->bSaving==FALSE) {  //セーブの必要が生じた
    WIFILOGIN_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0015);
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
  GFL_NET_Init(&aGFLNetInit, NULL, pWork);	//通信初期化
  GFL_NET_StateWifiEnterLogin();

  WIFILOGIN_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0008);

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
      GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
      pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
      _CHANGE_STATE(pWork,NULL);
    }
    WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    WIFILOGIN_MESSAGE_YesNoEnd(pWork->pSelectWork);
    pWork->pSelectWork=NULL;
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , 0 );
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
      GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
      _CHANGE_STATE(pWork,NULL);
    }
    WIFILOGIN_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
    WIFILOGIN_MESSAGE_YesNoEnd(pWork->pSelectWork);
    pWork->pSelectWork=NULL;
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , 0 );
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
  if(!WIFILOGIN_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  pWork->pSelectWork = WIFILOGIN_MESSAGE_YesNoStart(pWork->pMessageWork,WIFILOGIN_YESNOTYPE_SYS);
  _CHANGE_STATE(pWork,_modeProfileWait2);
}



static void _modeLoginWait(WIFILOGIN_WORK* pWork)
{
  if(!WIFILOGIN_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
 pWork->pSelectWork = WIFILOGIN_MESSAGE_YesNoStart(pWork->pMessageWork,WIFILOGIN_YESNOTYPE_INFO);
  _CHANGE_STATE(pWork,_modeLoginWait2);
}



static void _modeDifferDSWait5(WIFILOGIN_WORK* pWork)
{
  if(WIFILOGIN_MESSAGE_YesNoIsFinish(pWork->pSelectWork)){
    int selectno = WIFILOGIN_MESSAGE_YesNoGetCursorPos(pWork->pSelectWork);

    if(selectno==0){
      pWork->dbw->result  = WIFILOGIN_RESULT_LOGIN;
      WifiList_Init(pWork->pList);
      _CHANGE_STATE(pWork,_connectionStart);
    }
    else{
      pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
      GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
      _CHANGE_STATE(pWork,NULL);
    }
    WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    WIFILOGIN_MESSAGE_YesNoEnd(pWork->pSelectWork);
    pWork->pSelectWork=NULL;
  }
}



static void _modeDifferDSWait4(WIFILOGIN_WORK* pWork)
{

  pWork->pSelectWork = WIFILOGIN_MESSAGE_YesNoStart(pWork->pMessageWork,WIFILOGIN_YESNOTYPE_SYS);
  _CHANGE_STATE(pWork,_modeDifferDSWait5);
}


static void _modeDifferDSWait3(WIFILOGIN_WORK* pWork)
{
  if(WIFILOGIN_MESSAGE_YesNoIsFinish(pWork->pSelectWork)){
    int selectno = WIFILOGIN_MESSAGE_YesNoGetCursorPos(pWork->pSelectWork);

    if(selectno==0){
      pWork->dbw->result  = WIFILOGIN_RESULT_LOGIN;
      WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0007);
      _CHANGE_STATE(pWork,_modeDifferDSWait4);
    }
    else{
      pWork->dbw->result  = WIFILOGIN_RESULT_CANCEL;
      WIFILOGIN_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
      _CHANGE_STATE(pWork,NULL);
    }
    WIFILOGIN_MESSAGE_YesNoEnd(pWork->pSelectWork);
    pWork->pSelectWork=NULL;
  }
}



static void _modeDifferDSWait2(WIFILOGIN_WORK* pWork)
{


  pWork->pSelectWork = WIFILOGIN_MESSAGE_YesNoStart(pWork->pMessageWork,WIFILOGIN_YESNOTYPE_SYS);
  _CHANGE_STATE(pWork,_modeDifferDSWait3);
}


static void _modeDifferDSWait(WIFILOGIN_WORK* pWork)
{
  if(GFL_UI_TP_GetTrg() || GFL_UI_KEY_GetTrg()){
    WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0006);
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
  // 初めての場合
  if( !DWC_CheckHasProfile( WifiList_GetMyUserInfo(pWork->pList) ) )
  {
    pWork->bInitMessage=TRUE;
    WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0003);
    _CHANGE_STATE(pWork,_modeProfileWait);
  }
  else if( !DWC_CheckValidConsole(WifiList_GetMyUserInfo(pWork->pList)) )
  {  //別DSの場合
    WIFILOGIN_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0005);
    _CHANGE_STATE(pWork,_modeDifferDSWait);
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
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeStart(WIFILOGIN_WORK* pWork)
{
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
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
	
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFILOGIN, 0x18000 );

  {
    WIFILOGIN_WORK *pWork = GFL_PROC_AllocWork( proc, sizeof( WIFILOGIN_WORK ), HEAPID_WIFILOGIN );
    GFL_STD_MemClear(pWork, sizeof(WIFILOGIN_WORK));
    pWork->heapID = HEAPID_WIFILOGIN;
    pWork->gamedata = pEv->gamedata;
    pWork->pDispWork = WIFILOGIN_DISP_Init(pWork->heapID,pEv->bg,pEv->display);
    pWork->pMessageWork = WIFILOGIN_MESSAGE_Init(pWork->heapID, NARC_message_wifi_system_dat,pEv->display);
    pWork->pSave = GAMEDATA_GetSaveControlWork(pEv->gamedata);
    pWork->pList = GAMEDATA_GetWiFiList(pEv->gamedata);


    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
									WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

      pWork->dbw = pwk;
    
    if(GFL_NET_IsInit()){       // 接続中
      GF_ASSERT(0);
    }
    else{
      //接続開始 プロファイル検査
      _CHANGE_STATE(pWork,_profileIDCheck);
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

  StateFunc* state = pWork->state;
  if(state != NULL){
    state(pWork);
    retCode = GFL_PROC_RES_CONTINUE;
  }

  if(pWork->pSelectWork){
    WIFILOGIN_MESSAGE_YesNoUpdate(pWork->pSelectWork);
  }

  WIFILOGIN_DISP_Main(pWork->pDispWork);
  WIFILOGIN_MESSAGE_Main(pWork->pMessageWork);


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


