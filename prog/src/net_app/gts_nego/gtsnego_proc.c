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
#include "net_app/gtsnego.h"
#include "net/network_define.h"
#include "net/dwc_rap.h"

#include "infowin/infowin.h"
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

//#include "msg/msg_gtsnego.h"
#include "msg/msg_wifi_system.h"
#include "../../field/event_gtsnego.h"
#include "gtsnego.naix"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK
#include "gtsnego_local.h"

#if PM_DEBUG
#define _NET_DEBUG (1)  //デバッグ時は１
#else
#define _NET_DEBUG (0)  //デバッグ時は１
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



//--------------------------------------------
// 内部ワーク
//--------------------------------------------


enum _IBMODE_SELECT {
  _SELECTMODE_GSYNC = 0,
  _SELECTMODE_UTIL,
  _SELECTMODE_EXIT
};




typedef void (StateFunc)(GTSNEGO_WORK* pState);
typedef BOOL (TouchFunc)(int no, GTSNEGO_WORK* pState);


struct _GTSNEGO_WORK {
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
  APP_TASKMENU_WORK* pAppTask;
  GTSNEGO_DISP_WORK* pDispWork;  // 描画系
  GTSNEGO_MESSAGE_WORK* pMessageWork; //メッセージ系
  WIFI_LIST* pList;
  EVENT_GTSNEGO_WORK * dbw;  //親のワーク
  GAMESYS_WORK *gameSys_;
  FIELDMAP_WORK *fieldWork_;
  GMEVENT* event_;
};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(GTSNEGO_WORK* pWork,StateFunc* state);
static void _changeStateDebug(GTSNEGO_WORK* pWork,StateFunc* state, int line);

static void _modeSelectMenuInit(GTSNEGO_WORK* pWork);
static void _modeSelectMenuWait(GTSNEGO_WORK* pWork);
static void _profileIDCheck(GTSNEGO_WORK* pWork);

static void _modeReportInit(GTSNEGO_WORK* pWork);
static void _modeReportWait(GTSNEGO_WORK* pWork);
static void _modeReportWait2(GTSNEGO_WORK* pWork);
static BOOL _modeSelectMenuButtonCallback(int bttnid,GTSNEGO_WORK* pWork);
static void _modeSelectBattleTypeInit(GTSNEGO_WORK* pWork);




#ifdef _NET_DEBUG
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

static void _changeState(GTSNEGO_WORK* pWork,StateFunc state)
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
static void _changeStateDebug(GTSNEGO_WORK* pWork,StateFunc state, int line)
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
static void _connectCallBack(void* pWk, int netID)
{
  GTSNEGO_WORK* pWork = pWk;
  u32 temp;

  OS_TPrintf("ネゴシエーション完了 netID = %d\n", netID);
  pWork->connect_bit |= 1 << netID;
  temp = pWork->connect_bit;
  if(MATH_CountPopulation(temp) >= 2){
    OS_TPrintf("全員のネゴシエーション完了 人数bit=%x\n", pWork->connect_bit);
    pWork->connect_ok = TRUE;
  }
}

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
  GTSNEGO_WORK *pWork = pWk;

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
  GTSNEGO_WORK *wk = pWork;
  return WifiList_GetMyUserInfo(SaveData_GetWifiListData(wk->pSave));
}

static void* _getFriendData(void* pWork)  //DWCFriendData
{
  GTSNEGO_WORK *wk = pWork;
  return WifiList_GetDwcDataPtr(SaveData_GetWifiListData(wk->pSave),0);
}

static void _deleteFriendList(int deletedIndex,int srcIndex, void* pWork)
{
  GTSNEGO_WORK *wk = pWork;
  WifiList_DataMarge(SaveData_GetWifiListData(wk->pSave), deletedIndex, srcIndex);
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
  GTSNEGO_WORK *pWork = p_work;
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


static void _exitEnd( GTSNEGO_WORK* pWork)
{
  if( !GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork) ){
    return;
  }
  pWork->timer--;
  if(pWork->timer==0){
    _CHANGE_STATE(pWork, NULL);
  }
}

//------------------------------------------------------------------
/**
 * $brief   終了確認メッセージ  WIFIP2PMATCH_MODE_EXIT_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static void _exitExiting( GTSNEGO_WORK *pWork )
{
  if( !GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)  ){
    return;
  }
  if(pWork->timer == 1){
    pWork->timer = 0;
    GFL_NET_Exit(NULL);
  }
  if(!GFL_NET_IsInit()){
    WifiList_FormUpData(pWork->pList);  // 切断した時にフレンドコードを詰める
    GTSNEGO_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0012);
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

static void _CheckAndEnd( GTSNEGO_WORK *pWork )
{
  if(pWork->timer > 0){
    pWork->timer--;
    return;
  }
  if( GFL_UI_KEY_GetTrg() ){
    GTSNEGO_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0011);
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

static void _retryInit(GTSNEGO_WORK* pWork)
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

static void _errorDisp(GTSNEGO_WORK* pWork)
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

  GTSNEGO_MESSAGE_ErrorMessageDisp(pWork->pMessageWork, msgno, no);
//  GTSNEGO_MESSAGE_SystemMessageDisp(pWork->pMessageWork, msgno);
  
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


static void _saveEndWait(GTSNEGO_WORK* pWork)
{
  if(GFL_UI_KEY_GetTrg()  || GFL_UI_TP_GetTrg()){
    GTSNEGO_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    _CHANGE_STATE(pWork, NULL);  //接続完了
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   接続汎用関数
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _connectingCommonWait(GTSNEGO_WORK* pWork)
{
  if(GFL_NET_StateIsWifiLoginState()){
    if( pWork->bInitMessage ){     // 初回接続時にはセーブが完了した事を通知
      GTSNEGO_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
      GTSNEGO_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0004);
      _CHANGE_STATE(pWork, _saveEndWait);
    }
    else{
      _CHANGE_STATE(pWork, NULL);  //接続完了
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
static void _saveingWait(GTSNEGO_WORK* pWork)
{
  if(GFL_NET_DWC_GetSaving()){
    SAVE_RESULT result = GFL_NET_DWC_SaveAsyncMain(pWork->pSave);
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
static void _saveingStart(GTSNEGO_WORK* pWork)
{
  if(!GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    _connectingCommonWait(pWork);
  }
  else{
    GFL_NET_DWC_SaveAsyncInit(pWork->pSave);
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
static void _connectingWait(GTSNEGO_WORK* pWork)
{
  if(!GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if( GFL_NET_DWC_GetSaving() && pWork->bSaving==FALSE) {  //セーブの必要が生じた
    GTSNEGO_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0015);
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
static void _connectionStart(GTSNEGO_WORK* pWork)
{

  GTSNEGO_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0008);
  _CHANGE_STATE(pWork,_connectingWait);
  
}

//------------------------------------------------------------------------------
/**
 * @brief   接続確認のはいいいえまち
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeProfileWait2(GTSNEGO_WORK* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    if(selectno==0){
      _CHANGE_STATE(pWork,_connectionStart);
    }
    else{
      GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
      _CHANGE_STATE(pWork,NULL);
    }
    GTSNEGO_MESSAGE_SystemMessageEnd(pWork->pMessageWork);
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , 0 );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   接続確認のメッセージ出力待ち
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeProfileWait(GTSNEGO_WORK* pWork)
{
  if(!GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  pWork->pAppTask = GTSNEGO_MESSAGE_YesNoStart(pWork->pMessageWork,GTSNEGO_YESNOTYPE_SYS);
  _CHANGE_STATE(pWork,_modeProfileWait2);
}


//------------------------------------------------------------------------------
/**
 * @brief   最初の接続のプロファイル分岐
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _profileIDCheck(GTSNEGO_WORK* pWork)
{
  // 初めての場合
  if( !DWC_CheckHasProfile( WifiList_GetMyUserInfo(pWork->pList) ) )
  {
    pWork->bInitMessage=TRUE;
    GTSNEGO_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0003);
  }
  else if( !DWC_CheckValidConsole(WifiList_GetMyUserInfo(pWork->pList)) )
  {  //別DSの場合
    GTSNEGO_MESSAGE_SystemMessageDisp(pWork->pMessageWork, dwc_message_0005);
  }
  else  //普通の接続
  {
    GTSNEGO_MESSAGE_InfoMessageDisp(pWork->pMessageWork, dwc_message_0002);
  }
  _CHANGE_STATE(pWork,_modeProfileWait);

}


static void _modeSelectMenuInit(GTSNEGO_WORK* pWork)
{ 
  //GTSNEGO_MESSAGE_Disp(GTSNEGO_005);
  //GTSNEGO_MESSAGE_Disp(GTSNEGO_006);

  pWork->touch = &_modeSelectMenuButtonCallback;

	_CHANGE_STATE(pWork,_modeSelectMenuWait);

}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeout(GTSNEGO_WORK* pWork)
{
	if(WIPE_SYS_EndCheck()){
		_CHANGE_STATE(pWork, NULL);        // 終わり
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectMenuButtonCallback(int bttnid,GTSNEGO_WORK* pWork)
{
  switch( bttnid ){
  case _SELECTMODE_GSYNC:
		PMSND_PlaySystemSE(SEQ_SE_DECIDE1);
    _CHANGE_STATE(pWork,_modeReportInit);
//    pWork->selectType = GAMESYNC_RETURNMODE_SYNC;
    return TRUE;
  case _SELECTMODE_UTIL:
    PMSND_PlaySystemSE(SEQ_SE_DECIDE1);
  //  pWork->selectType = GAMESYNC_RETURNMODE_UTIL;
    _CHANGE_STATE(pWork,_modeReportInit);
    return TRUE;
  case _SELECTMODE_EXIT:
		PMSND_PlaySystemSE(SEQ_SE_CANCEL1);
   // pWork->selectType = GAMESYNC_RETURNMODE_NONE;
		WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
										WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
    _CHANGE_STATE(pWork,_modeFadeout);        // 終わり
    return TRUE;
  default:
    break;
  }
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait(GTSNEGO_WORK* pWork)
{
	if(WIPE_SYS_EndCheck()){
		GTSNEGO_MESSAGE_ButtonWindowMain( pWork->pMessageWork );
	}
}


//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportInit(GTSNEGO_WORK* pWork)
{


  GFL_BG_ClearScreenCodeVReq(GFL_BG_FRAME1_S,0);
  

  
  _CHANGE_STATE(pWork,_modeReportWait);
}


static void _FadeWait(GTSNEGO_WORK* pWork)
{
  if(GFL_FADE_CheckFade()){
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
static void _modeReporting(GTSNEGO_WORK* pWork)
{

  if(!GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }


  {
    SAVE_RESULT svr = SaveControl_SaveAsyncMain(pWork->dbw->ctrl);

    if(svr == SAVE_RESULT_OK){
      GTSNEGO_MESSAGE_InfoMessageEnd(pWork->pMessageWork);

      GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, 1);

      _CHANGE_STATE(pWork,_FadeWait);
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportWait2(GTSNEGO_WORK* pWork)
{

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    if(selectno==0){



      SaveControl_SaveAsyncInit(pWork->dbw->ctrl );
      _CHANGE_STATE(pWork,_modeReporting);
    }
    else{
      GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
      _CHANGE_STATE(pWork,NULL);
    }
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , 0 );
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportWait(GTSNEGO_WORK* pWork)
{
  if(!GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  pWork->pAppTask = GTSNEGO_MESSAGE_YesNoStart(pWork->pMessageWork,GTSNEGO_YESNOTYPE_INFO);
  _CHANGE_STATE(pWork,_modeReportWait2);
}


//------------------------------------------------------------------------------
/**
 * @brief   PROCスタート
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT GameSyncMenuProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_GTSNEGO_WORK* pEv=pwk;
	
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, 0x18000 );

  {
    GTSNEGO_WORK *pWork = GFL_PROC_AllocWork( proc, sizeof( GTSNEGO_WORK ), HEAPID_IRCBATTLE );
    GFL_STD_MemClear(pWork, sizeof(GTSNEGO_WORK));
    pWork->heapID = HEAPID_IRCBATTLE;

    pWork->pDispWork = GTSNEGO_DISP_Init(pWork->heapID);
    pWork->pMessageWork = GTSNEGO_MESSAGE_Init(pWork->heapID, NARC_message_wifi_system_dat);
    pWork->pSave = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(pEv->gsys));
    pWork->pList = SaveData_GetWifiListData(
      GAMEDATA_GetSaveControlWork(
        GAMESYSTEM_GetGameData(
          ((pEv->gsys))) ));


		{
//			GAME_COMM_SYS_PTR pGC = GAMESYSTEM_GetGameCommSysPtr(pEv->gsys);
	//		INFOWIN_Init( _SUBSCREEN_BGPLANE , _SUBSCREEN_PALLET , pGC , pWork->heapID);
		}



    WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
									WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

    pWork->dbw = pwk;
    
    _CHANGE_STATE(pWork,_modeSelectMenuInit);

  }
  
  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT GameSyncMenuProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GTSNEGO_WORK* pWork = mywk;
  GFL_PROC_RESULT retCode = GFL_PROC_RES_FINISH;

  StateFunc* state = pWork->state;
  if(state != NULL){
    state(pWork);
    retCode = GFL_PROC_RES_CONTINUE;
  }

  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }

  GTSNEGO_DISP_Main(pWork->pDispWork);
  GTSNEGO_MESSAGE_Main(pWork->pMessageWork);

  
	//INFOWIN_Update();

  return retCode;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT GameSyncMenuProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_GTSNEGO_WORK* pEv=pwk;
  GTSNEGO_WORK* pWork = mywk;

//  EVENT_IrcBattleSetType(pParentWork, pWork->selectType);

  GFL_PROC_FreeWork(proc);

	//INFOWIN_Exit();
	GFL_BG_FreeBGControl(_SUBSCREEN_BGPLANE);


  GTSNEGO_MESSAGE_End(pWork->pMessageWork);
  GTSNEGO_DISP_End(pWork->pDispWork);

	GFL_HEAP_DeleteHeap(HEAPID_IRCBATTLE);
  //EVENT_IrcBattle_SetEnd(pParentWork);


	
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA GtsNego_ProcData = {
  GameSyncMenuProcInit,
  GameSyncMenuProcMain,
  GameSyncMenuProcEnd,
};


