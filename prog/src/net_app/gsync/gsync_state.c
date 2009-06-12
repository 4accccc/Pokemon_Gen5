//=============================================================================
/**
 * @file	  gsync_state.c
 * @brief	  ゲームシンク
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"

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

typedef struct _G_SYNC_WORK G_SYNC_WORK;

typedef void (StateFunc)(G_SYNC_WORK* pState);

#define POSTURL "http://219.118.175.21:10610/cgi-bin/cgeartest/gsync.cgi"
#define GETURL "http://219.118.175.21:10610/cgi-bin/cgeartest/getsync.cgi"


static void _changeState(G_SYNC_WORK* pWork,StateFunc* state);
static void _changeStateDebug(G_SYNC_WORK* pWork,StateFunc* state, int line);

#define _NET_DEBUG (1)

#ifdef _NET_DEBUG
#define   _CHANGE_STATE(state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeState(pWork ,state)
#endif //_NET_DEBUG



struct _G_SYNC_WORK {
	SAVE_CONTROL_WORK* pSaveData;
	StateFunc* state;      ///< ハンドルのプログラム状態
	int req;
	BOOL bEnd;
	DWCGHTTPPost post;
	u8 mac[6];
	u8 buffer[0x102];
};


static void _ghttpGetting(G_SYNC_WORK* pWork);
static void _ghttpPost(G_SYNC_WORK* pWork);
static void _ghttpPosting(G_SYNC_WORK* pWork);
static void _ghttpGet(G_SYNC_WORK* pWork);
static void GETCallback(const char* buf, int buflen, DWCGHTTPResult result, void* param);


static DWCUserData* _getMyUserData(void* pWork)
{
  G_SYNC_WORK *wk = (G_SYNC_WORK*)pWork;
  return WifiList_GetMyUserInfo(SaveData_GetWifiListData(wk->pSaveData));
}

static DWCFriendData* _getFriendData(void* pWork)
{
  G_SYNC_WORK *wk = (G_SYNC_WORK*)pWork;
  //    NET_PRINT("Friend %d\n",WifiList_GetFriendDataNum(SaveData_GetWifiListData(wk->pSaveData)));
  return WifiList_GetDwcDataPtr(SaveData_GetWifiListData(wk->pSaveData),0);
}



static GFLNetInitializeStruct aGFLNetInit = {
  NULL,  // 受信関数テーブル
  0, // 受信テーブル要素数
  NULL,    ///< ハードで接続した時に呼ばれる
  NULL,    ///< ネゴシエーション完了時にコール
  NULL,   // ユーザー同士が交換するデータのポインタ取得関数
  NULL,   // ユーザー同士が交換するデータのサイズ取得関数
  NULL,  // ビーコンデータ取得関数
  NULL,  // ビーコンデータサイズ取得関数
  NULL,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,            // 普通のエラーが起こった場合 通信終了
  NULL,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NULL,  // 通信切断時に呼ばれる関数
  NULL,  // オート接続で親になった場合
  NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL,  ///< wifiフレンドリスト削除コールバック
  _getFriendData,   ///< DWC形式の友達リスト
  _getMyUserData,  ///< DWCのユーザデータ（自分のデータ）
  GFL_NET_DWC_HEAPSIZE,   ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
  0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_NETWORK,  //IRC用にcreateされるHEAPID
  GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
  1,     // 最大接続人数
  48,    // 最大送信バイト数
  32,    // 最大ビーコン収集数
  TRUE,     // CRC計算
  FALSE,     // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIFI,  //通信種別
  TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_WIFICLUB,  //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
};



//=============================================================================
/*!
 *	@brief	データ受信完了時のコールバック
 *
 *	@param	buf		[in] データバッファ
 *	@param	buflen	[in] bufのサイズ
 *  @param  param   [in] コールバック用パラメータ
 *
 *	@retval	TRUE	buf開放
 *	@retval	FALSE   buf開放せず
 */
//=============================================================================
static void GETCallback(const char* buf, int buflen, DWCGHTTPResult result, void* param)
{
	char*	g_data;	//! 受信データのバッファ
#if 0
	g_data = param;

	if(buf){
		MI_CpuCopy8(buf, g_data, (u32)buflen);
	}

	g_pending = 0;
#endif
	OS_Printf("Recv Result : %d\n", result);

}

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
	OS_TPrintf("ghttp: %d\n",line);
#endif
	_changeState(pWork, state);
}
#endif


//=============================================================================
/*!
 *	@brief	データ送信完了時のコールバック
 *
 *	@param	buf		[in] データバッファ
 *	@param	buflen	[in] bufのサイズ
 *
 *	@retval	TRUE	buf開放
 *	@retval	FALSE   buf開放せず
*/
//=============================================================================
static void POSTCallback(const char* buf, int buflen, DWCGHTTPResult result, void* param)
{
	G_SYNC_WORK* pWork = param;

	OS_TPrintf("GHTTPのデータ送信完了\n");
	pWork->bEnd = TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpPost(G_SYNC_WORK* pWork)
{
	int len;

	if(GFL_NET_StateGetWifiStatus()!=GFL_NET_STATE_MATCHING){
		return;
	}

	
	// ghttpの初期化
	if(DWC_InitGHTTP(NULL) != TRUE){
		OS_Printf("error at SCInitialize()\n");
	}

	// send data
	//-------------------------------------------
	GFL_STD_MemClear(pWork->mac, 6);

	OS_GetMacAddress(pWork->mac);


	

	// post data
	//-------------------------------------------
	{
		DWC_GHTTPNewPost(&pWork->post);
//		DWC_GHTTPPostAddString(&pWork->post, "name", "sogabe");
//		DWC_GHTTPPostAddString(&pWork->post, "mac", "123456");
//		DWC_GHTTPPostAddString(&pWork->post, "save", "datatest");

		DWC_GHTTPPostAddFileFromMemory(&pWork->post, "name1", (const char*)"sogabe",6,
																	 "name.txt","application/octet-stream");

		DWC_GHTTPPostAddFileFromMemory(&pWork->post, "file1", (const char*)SaveControl_GetPointer(),0x80000,
																	 "test.bin","application/octet-stream");

//		DWC_GHTTPPostAddFileFromMemory(&pWork->post, "mac", (const char*)pWork->mac,6,
	//																 "mac.bin","application/octet-stream");

	}
	
	OS_TPrintf("start %d\n",OS_GetVBlankCount());

	OS_TPrintf("DWC_GHTTP_HOST_LOOKUP       %d\n",DWC_GHTTP_HOST_LOOKUP       );
	OS_TPrintf("DWC_GHTTP_CONNECTING        %d\n",DWC_GHTTP_CONNECTING        );
	OS_TPrintf("DWC_GHTTP_SECURING_SESSION  %d\n",DWC_GHTTP_SECURING_SESSION  );
	OS_TPrintf("DWC_GHTTP_SENDING_REQUEST   %d\n",DWC_GHTTP_SENDING_REQUEST   );
	OS_TPrintf("DWC_GHTTP_POSTING           %d\n",DWC_GHTTP_POSTING           );
	OS_TPrintf("DWC_GHTTP_WAITING           %d\n",DWC_GHTTP_WAITING           );
	OS_TPrintf("DWC_GHTTP_RECEIVING_STATUS  %d\n",DWC_GHTTP_RECEIVING_STATUS  );
	OS_TPrintf("DWC_GHTTP_RECEIVING_HEADERS %d\n",DWC_GHTTP_RECEIVING_HEADERS );
	OS_TPrintf("DWC_GHTTP_RECEIVING_FILE    %d\n",DWC_GHTTP_RECEIVING_FILE    );
	OS_TPrintf("DWC_GHTTP_SOCKET_INIT       %d\n",DWC_GHTTP_SOCKET_INIT       );
	OS_TPrintf("DWC_GHTTP_LOOKUP_PENDING    %d\n",DWC_GHTTP_LOOKUP_PENDING    );

	pWork->bEnd = FALSE;

	
	pWork->req = DWC_PostGHTTPData(POSTURL, &pWork->post, POSTCallback, pWork);
	
	if( 0 <= pWork->req){
		_CHANGE_STATE(_ghttpPosting);
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
	if(GFL_UI_KEY_GetTrg()==PAD_BUTTON_X){
		_CHANGE_STATE(_ghttpPost);
	}
	else if(GFL_UI_KEY_GetTrg()==PAD_BUTTON_Y){
		_CHANGE_STATE(_ghttpGet);
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpPosting(G_SYNC_WORK* pWork)
{

	DWC_ProcessGHTTP();
	//OS_TPrintf("state %d\n",DWC_GetGHTTPState(pWork->req));
	
	if(	pWork->bEnd ){
		OS_TPrintf("end %d\n",OS_GetVBlankCount());
		pWork->bEnd = FALSE;
		_CHANGE_STATE(_ghttpKeyWait);
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpGet(G_SYNC_WORK* pWork)
{
	pWork->bEnd = FALSE;
	if( 0 <= DWC_GetGHTTPData("URL", GETCallback, (void*)pWork->buffer)){
		_CHANGE_STATE(_ghttpGetting);
	}

}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpGetting(G_SYNC_WORK* pWork)
{

	DWC_ProcessGHTTP();
	if(	pWork->bEnd ){

		DWC_ShutdownGHTTP();
		_CHANGE_STATE(NULL);
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   通信初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpInit(G_SYNC_WORK* pWork)
{

	GFL_NET_Init(&aGFLNetInit, NULL, pWork);
	GFL_NET_StateWifiEnterLogin();
	_CHANGE_STATE(_ghttpPost);

	
}



static GFL_PROC_RESULT GSYNCProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PROC, 0x70000 );//テスト

	{
		G_SYNC_WORK* pWork = GFL_PROC_AllocWork( proc, sizeof(G_SYNC_WORK), HEAPID_PROC );
		pWork->pSaveData = SaveControl_GetPointer();  //デバッグ
		_CHANGE_STATE(_ghttpInit);
	}
  return GFL_PROC_RES_FINISH;
}
static GFL_PROC_RESULT GSYNCProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	G_SYNC_WORK* pWork = mywk;
	StateFunc* state = pWork->state;

	if( state ){
		state( pWork );
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT GSYNCProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_PROC_FreeWork(mywk);
	GFL_HEAP_DeleteHeap(HEAPID_PROC);  //テスト
  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ
const GFL_PROC_DATA G_SYNC_ProcData = {
  GSYNCProc_Init,
  GSYNCProc_Main,
  GSYNCProc_End,
};

