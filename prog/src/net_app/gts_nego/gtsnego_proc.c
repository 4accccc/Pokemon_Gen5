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
#include "../../field/event_gtsnego.h"

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
#include "savedata/wifi_negotiation.h"
#include "savedata/system_data.h"

#include "msg/msg_gtsnego.h"
#include "../../field/event_gtsnego.h"
#include "gtsnego.naix"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK
#include "gtsnego_local.h"


#if PM_DEBUG
#define _NET_DEBUG (1)  //デバッグ時は１
#else
#define _NET_DEBUG (0)  //デバッグ時は１
#endif

#define _NO2   (2)
#define _NO3   (3)

#define _WORK_HEAPSIZE (0x1000)  // 調整が必要


//--------------------------------------------
// 画面構成定義
//--------------------------------------------

//BG面とパレット番号(仮設定
#define _SUBSCREEN_BGPLANE	(GFL_BG_FRAME3_S)
#define _SUBSCREEN_PALLET	(0xE)

#define STOP_TIME_ (60)
#define _FRIEND_LOOKAT_DOWN_TIME  (60*3)
#define _FRIEND_GREE_DOWN_TIME  (60*3)

//--------------------------------------------
// 通信初期化構造体
//--------------------------------------------




//--------------------------------------------
// 内部ワーク
//--------------------------------------------


enum _IBMODE_SELECT {
  _SELECTMODE_GSYNC = 0,
  _SELECTMODE_UTIL,
  _SELECTMODE_EXIT
};

enum _CHANGEMODE_SELECT {
  _CHANGEMODE_SELECT_ANYONE=0,
  _CHANGEMODE_SELECT_FRIEND,
};

enum _CHANGEMODE_LEVEL {
  _CHANGEMODE_LEVEL_ALL,
  _CHANGEMODE_LEVEL_1_49,
  _CHANGEMODE_LEVEL_50,
  _CHANGEMODE_LEVEL_51_100,
  _CHANGEMODE_LEVEL_MAX,
};

enum _CHANGEMODE_IMAGE {
  _CHANGEMODE_IMAGE_ALL,
  _CHANGEMODE_IMAGE_COOL,  //かっこいい
  _CHANGEMODE_IMAGE_PRETTY,       //		かわいい
  _CHANGEMODE_IMAGE_SCARY,   //  		こわい
  _CHANGEMODE_IMAGE_GEEK,   //  		へんな
  _CHANGEMODE_IMAGE_MAX,        		
};



enum _GTSNEGO_MATCHKEY {
  _MATCHKEY_PROFILE,   //Profile
  _MATCHKEY_TYPE,     //交換タイプ 知り合いかどうか
  _MATCHKEY_LEVEL,    //交換LV
  _MATCHKEY_IMAGE_MY,  //自分の希望
  _MATCHKEY_IMAGE_FRIEND,  //相手に対しての希望
  _MATCHKEY_ROMCODE,    //ROMバージョン
  _MATCHKEY_DEBUG,    //デバッグか製品か
  _MATCHKEY_MAX,
};



static char matchkeystr[_MATCHKEY_MAX][2]={"p1","ty","lv","my","fr","rm","db"};


typedef void (StateFunc)(GTSNEGO_WORK* pState);
typedef BOOL (TouchFunc)(int no, GTSNEGO_WORK* pState);

// 友達無指定ピアマッチメイク用追加キーデータ構造体
typedef struct tagGameMatchExtKeys
{
    int ivalue;       // キーに対応する値（int型）
    u8  keyID;        // マッチメイク用キーID
    u8 pad;          // パディング
    char keyStr[3];  // マッチメイク用キー文字列
} GameMatchExtKeys;


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
  GameMatchExtKeys aMatchKey[_MATCHKEY_MAX];
  char filter[128];
  int changeMode;    //キーにする値 だれでもかともだちか
  int myChageType;   //自分が交換したいタイプ
  int friendChageType;  //相手に交換してもらいたいタイプ
  int chageLevel;    // ポケモンレベル範囲
  s32 profileID;
};


///通信コマンド
typedef enum {
  _NETCMD_INFOSEND = GFL_NET_CMD_GTSNEGO,
  _NETCMD_MYSTATUSSEND,
} _BATTLEIRC_SENDCMD;


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

static void _levelSelect( GTSNEGO_WORK *pWork );

static void _recvInfomationData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvMystatusData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);

///通信コマンドテーブル
static const NetRecvFuncTable _PacketTbl[] = {
  {_recvInfomationData,   NULL},    ///_NETCMD_INFOSEND
  {_recvMystatusData, NULL},    ///_NETCMD_MYSTATUSSEND
};



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

//_NETCMD_INFOSEND
//------------------------------------------------------------------------------
/**
 * @brief   通信コールバック _NETCMD_INFOSEND
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _recvInfomationData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  GTSNEGO_WORK *pWork = pWk;
  EVENT_GTSNEGO_WORK* pEv=pWork->dbw;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }
  GFL_STD_MemCopy(pData,&pEv->aUser[1],sizeof(EVENT_GTSNEGO_USER_DATA));
}

//------------------------------------------------------------------------------
/**
 * @brief   通信コールバック _NETCMD_MYSTATUSSEND
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _recvMystatusData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  GTSNEGO_WORK *pWork = pWk;
  EVENT_GTSNEGO_WORK* pEv=pWork->dbw;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }
  GFL_STD_MemCopy(pData,pEv->pStatus[1], MyStatus_GetWorkSize());
}


//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _AnyoneOrFriendButtonCallback(int bttnid,GTSNEGO_WORK* pWork)
{

  pWork->changeMode = bttnid;
  _CHANGE_STATE(pWork, _levelSelect);
  return TRUE;
}


//------------------------------------------------------------------------------
/**
 * @brief   ほしい姿選択をインクリメントする
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _IncMyChageType(int num,GTSNEGO_WORK* pWork)
{
  pWork->myChageType+=num;
  if(_CHANGEMODE_IMAGE_MAX <= pWork->myChageType){
    pWork->myChageType = 0;
  }
  else if(0 > pWork->myChageType){
    pWork->myChageType = _CHANGEMODE_IMAGE_MAX - 1;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ほしい姿選択をインクリメントする
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _IncFriendChageType(int num,GTSNEGO_WORK* pWork)
{
  pWork->friendChageType+=num;
  if(_CHANGEMODE_IMAGE_MAX <= pWork->friendChageType){
    pWork->friendChageType = 0;
  }
  else if(0 > pWork->friendChageType){
    pWork->friendChageType = _CHANGEMODE_IMAGE_MAX - 1;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ほしいLV選択をインクリメントする
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _IncLevelChageType(int num,GTSNEGO_WORK* pWork)
{
  pWork->chageLevel += num;
  if(_CHANGEMODE_LEVEL_MAX <= pWork->chageLevel){
    pWork->chageLevel = 0;
  }
  else if(0 > pWork->chageLevel){
    pWork->chageLevel = _CHANGEMODE_LEVEL_MAX - 1;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _LevelButtonCallback(int bttnid,GTSNEGO_WORK* pWork)
{
  switch(bttnid){
  case _ARROW_LEVEL_U:
    _IncLevelChageType(-1,pWork);
    GTSNEGO_MESSAGE_DispLevelChange(pWork->pMessageWork,pWork->chageLevel);
    break;
  case _ARROW_LEVEL_D:
    _IncLevelChageType(1,pWork);
    GTSNEGO_MESSAGE_DispLevelChange(pWork->pMessageWork,pWork->chageLevel);
    break;
  case _ARROW_MY_U:
    _IncMyChageType(-1,pWork);
    GTSNEGO_MESSAGE_DispMyChange(pWork->pMessageWork,pWork->myChageType);
    break;
  case _ARROW_MY_D:
    _IncMyChageType(1,pWork);
    GTSNEGO_MESSAGE_DispMyChange(pWork->pMessageWork,pWork->myChageType);
    break;
  case _ARROW_FRIEND_U:
    _IncFriendChageType(-1,pWork);
    GTSNEGO_MESSAGE_DispFriendChange(pWork->pMessageWork,pWork->friendChageType);
    break;
  case _ARROW_FRIEND_D:
    _IncFriendChageType(1,pWork);
    GTSNEGO_MESSAGE_DispFriendChange(pWork->pMessageWork,pWork->friendChageType);
    break;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	同期検査+INFO送信
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static void _timingCheck2( GTSNEGO_WORK *pWork )
{
  if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_NO3)){





    _CHANGE_STATE(pWork,NULL);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	MYSTATUS送信
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static void _statussendCheck( GTSNEGO_WORK *pWork )
{
  GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_NO3);
  _CHANGE_STATE(pWork,_timingCheck2);
}

//----------------------------------------------------------------------------
/**
 *	@brief	MYSTATUS送信
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static void _infosendCheck( GTSNEGO_WORK *pWork )
{
  EVENT_GTSNEGO_WORK* pEv=pWork->dbw;
  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_MYSTATUSSEND, MyStatus_GetWorkSize(),pEv->pStatus[0])){
    _CHANGE_STATE(pWork,_statussendCheck);
  }
}




//----------------------------------------------------------------------------
/**
 *	@brief	同期検査+INFO送信
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static void _timingCheck( GTSNEGO_WORK *pWork )
{
  if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_NO2)){
      EVENT_GTSNEGO_WORK* pEv=pWork->dbw;
    if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_INFOSEND, sizeof(EVENT_GTSNEGO_USER_DATA),&pEv->aUser[0])){
      _CHANGE_STATE(pWork,_infosendCheck);
    }
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief	見つかった表示
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static void _friendGreeState( GTSNEGO_WORK *pWork )
{
  if(!GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }

  if(pWork->timer>0){
    pWork->timer--;
  }
  else{
    EVENT_GTSNEGO_WORK *pParent = pWork->dbw;

    pParent->aUser[0].selectLV = pWork->myChageType;
    pParent->aUser[0].selectType = pWork->changeMode;
    {
    }
    GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_NO2);
    
    _CHANGE_STATE(pWork,_timingCheck);
  }
}



static void _friendGreeStateParent( GTSNEGO_WORK *pWork )
{
  if(GFL_NET_HANDLE_GetNumNegotiation() != 0){
    if(GFL_NET_HANDLE_RequestNegotiation()){
      _CHANGE_STATE(pWork,_friendGreeState);
    }
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief	見つかった表示
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static void _lookatDownState( GTSNEGO_WORK *pWork )
{
  if(!GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if(pWork->timer>0){
    pWork->timer--;
  }
  else{
    pWork->timer = _FRIEND_GREE_DOWN_TIME;
    GTSNEGO_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GTSNEGO_022);
    if(GFL_NET_SystemGetCurrentID() != GFL_NET_NO_PARENTMACHINE){  // 子機として接続が完了した
      if(GFL_NET_HANDLE_RequestNegotiation()){
        _CHANGE_STATE(pWork,_friendGreeState);
      }
    }
    else{
      _CHANGE_STATE(pWork,_friendGreeStateParent);
    }
  }


}



//----------------------------------------------------------------------------
/**
 *	@brief	接続中
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static void _matchingState( GTSNEGO_WORK *pWork )
{
 //接続したら表示して交換に
  if(STEPMATCH_SUCCESS == GFL_NET_DWC_GetStepMatchResult()){
    
    GTSNEGO_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GTSNEGO_021);

    pWork->timer = _FRIEND_LOOKAT_DOWN_TIME;

    
    _CHANGE_STATE(pWork, _lookatDownState);
    
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	接続の為の検査
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static int _evalcallback(int index, void* param)
{
  GTSNEGO_WORK *pWork=param;
  int value = -1;
  int targetlv,targetfriend,targetmy,profile;

  targetlv = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_LEVEL].keyStr,-1);
  targetfriend = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_IMAGE_FRIEND].keyStr,-1);
  targetmy = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_IMAGE_MY].keyStr,-1);
  profile = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_PROFILE].keyStr,-1);
  
  if(pWork->changeMode==1){//ともだち
    if( WIFI_NEGOTIATION_SV_IsCheckFriend( WIFI_NEGOTIATION_SV_GetSaveData(pWork->pSave) ,profile )){
      value=100;
    }
  }
  else{
    if(pWork->chageLevel==targetlv){
      if(pWork->myChageType==targetfriend){
        value+=10;
      }
      if(pWork->friendChageType==targetmy){
        value+=10;
      }
      //@todoすでに交換したかをチェック
      
    }
  }
  OS_TPrintf("評価コールバック %d %d %d %d %d\n",value,pWork->changeMode,targetlv,targetmy,targetfriend);
  return value;
}

//----------------------------------------------------------------------------
/**
 *	@brief	キーを作って接続開始
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void _matchKeyMake( GTSNEGO_WORK *pWork )
{
  int buff[_MATCHKEY_MAX];
  int i;

  if(!GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  GFL_NET_SetWifiBothNet(FALSE);
  //GFL_NET_StateWifiEnterLogin();

  
  GFL_NET_DWC_GetMySendedFriendCode(pWork->pList, (DWCFriendData*)&buff[0]);

  buff[0] = pWork->profileID;
  buff[1] = pWork->changeMode;
  buff[2] = pWork->chageLevel;
  buff[3] = pWork->myChageType;
  buff[4] = pWork->friendChageType;
  buff[5] = PM_VERSION + (PM_LANG<<16); 
  buff[6] = MATCHINGKEY;

  for(i = 0; i < _MATCHKEY_MAX; i++)
  {
    GameMatchExtKeys* pMatchKey = &pWork->aMatchKey[i];
    GFL_STD_MemCopy(&matchkeystr[i], pMatchKey->keyStr,2);
    pMatchKey->ivalue = buff[i];
    pMatchKey->keyID  = DWC_AddMatchKeyInt(pMatchKey->keyID, pMatchKey->keyStr, &pMatchKey->ivalue);
#if PM_DEBUG
    if (pMatchKey->keyID == 0){
      NET_PRINT("AddMatchKey failed %d.\n",i);
    }
#endif
  }

  //この条件は確定 matchkeystrもあわせて変更する事 
  STD_TSPrintf( pWork->filter, "ty=%d And db=%d", pWork->changeMode,MATCHINGKEY);

  if( GFL_NET_DWC_StartMatchFilter( pWork->filter, 2 ,&_evalcallback, pWork) != 0){

    GFL_NET_DWC_SetVChat(FALSE);


    _CHANGE_STATE(pWork,_matchingState);
  }
  else{
    
  }



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

//------------------------------------------------------------------
/**
 * $brief   レベル確認待ち
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static void _levelSelectWait( GTSNEGO_WORK *pWork )
{
  if(!GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }

  GTSNEGO_MESSAGE_ButtonWindowMain(pWork->pMessageWork);



  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    if(selectno==0){
      GTSNEGO_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GTSNEGO_019);
      
      _CHANGE_STATE(pWork,_matchKeyMake);
    }
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
  else{
    TOUCHBAR_Main(GTSNEGO_DISP_GetTouchWork(pWork->pDispWork));
    switch( TOUCHBAR_GetTrg(GTSNEGO_DISP_GetTouchWork(pWork->pDispWork))){
    case TOUCHBAR_ICON_RETURN:
      _CHANGE_STATE(pWork,NULL);
      break;
    default:
      break;
    }
  }
}

//------------------------------------------------------------------
/**
 * $brief   レベル確認
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static void _levelSelect( GTSNEGO_WORK *pWork )
{
  GTSNEGO_MESSAGE_DispClear(pWork->pMessageWork);
  GTSNEGO_DISP_LevelInputInit(pWork->pDispWork);
  pWork->touch = &_LevelButtonCallback;

  GTSNEGO_MESSAGE_DispLevel(pWork->pMessageWork, &_BttnCallBack, pWork);

  GTSNEGO_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GTSNEGO_025);
  
  pWork->pAppTask = GTSNEGO_MESSAGE_SearchButtonStart(pWork->pMessageWork);

  _CHANGE_STATE(pWork,_levelSelectWait);
}







//------------------------------------------------------------------------------
/**
 * @brief   接続開始
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _connectionStart(GTSNEGO_WORK* pWork)
{

//  _CHANGE_STATE(pWork,_connectingWait);

}



//------------------------------------------------------------------------------
/**
 * @brief   だれでもかしりあいか選択
 * @retval  none
 */
//------------------------------------------------------------------------------



static void _modeSelectMenuInit(GTSNEGO_WORK* pWork)
{
  GTSNEGO_MESSAGE_DispAnyoneOrFriend(pWork->pMessageWork, &_BttnCallBack, pWork);
  pWork->touch = &_AnyoneOrFriendButtonCallback;
  GTSNEGO_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GTSNEGO_024);
  _CHANGE_STATE(pWork,_modeSelectMenuWait);
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait(GTSNEGO_WORK* pWork)
{

  if(!GTSNEGO_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }

  
  GTSNEGO_MESSAGE_ButtonWindowMain(pWork->pMessageWork);
  
  TOUCHBAR_Main(GTSNEGO_DISP_GetTouchWork(pWork->pDispWork));
  switch( TOUCHBAR_GetTrg(GTSNEGO_DISP_GetTouchWork(pWork->pDispWork))){
  case TOUCHBAR_ICON_RETURN:
    _CHANGE_STATE(pWork,NULL);
    break;
  default:
    break;
  }


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
  GTSNEGO_WORK *pWork;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, 0x38000 );

  
  pWork = GFL_PROC_AllocWork( proc, sizeof( GTSNEGO_WORK ), HEAPID_IRCBATTLE );
  GFL_STD_MemClear(pWork, sizeof(GTSNEGO_WORK));
  pWork->heapID = HEAPID_IRCBATTLE;
  pWork->dbw = pEv;
  pWork->pDispWork = GTSNEGO_DISP_Init(pWork->heapID);
  pWork->pMessageWork = GTSNEGO_MESSAGE_Init(pWork->heapID, NARC_message_gtsnego_dat);
  pWork->pSave = GAMEDATA_GetSaveControlWork(pEv->gamedata);
  pWork->pList = GAMEDATA_GetWiFiList(pEv->gamedata);

  pWork->profileID = SYSTEMDATA_GetDpwInfo( SaveData_GetSystemData(pWork->pSave) );
  
  if(GFL_NET_IsInit()){
    GFL_NET_AddCommandTable(GFL_NET_CMD_GTSNEGO,_PacketTbl,NELEMS(_PacketTbl), pWork);
  }



  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  
  
  _CHANGE_STATE(pWork,_modeSelectMenuInit);
  
  

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

  TOUCHBAR_Exit(GTSNEGO_DISP_GetTouchWork(pWork->pDispWork));

  GTSNEGO_MESSAGE_End(pWork->pMessageWork);
  GTSNEGO_DISP_End(pWork->pDispWork);

  GFL_PROC_FreeWork(proc);



//  GFL_BG_FreeBGControl(_SUBSCREEN_BGPLANE);
  GFL_HEAP_DeleteHeap(HEAPID_IRCBATTLE);



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


