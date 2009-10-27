//=============================================================================
/**
 * @file	dwc_raputility.c
 * @bfief	WIFI設定画面呼び出し用
 * @author	k.ohnop
 * @date	09/01/15
 */
//=============================================================================

#include <gflib.h>
#include <dwc.h>
#include "system/main.h"
#include "net/dwc_raputil.h"

//==============================================================================
/**
 * WiFi接続ユーティリティを呼び出す
 * @param   none
 * @retval  none
 */
//==============================================================================
void mydwc_callWifiUtil( HEAPID HeapID )
{
	OSIntrMode old;
	void *work;

	// アラームシステムを初期化しておかないとDWC_StartUtility呼出し後にPanicが発生する
	OS_InitTick();
	OS_InitAlarm();

  DWC_SetAuthServer(DWC_CONNECTINET_AUTH_TEST);

	// 割り込み処理を一旦禁止へ
	old = OS_DisableInterrupts();

	// WiFi設定メニュー呼び出し（終わるまで帰ってこない)
	work = GFL_HEAP_AllocClearMemory( HeapID, DWC_UTILITY_WORK_SIZE );
	GF_ASSERT(DWC_UTIL_RESULT_SUCCESS==DWC_StartUtility( work, DWC_LANGUAGE_JAPANESE, DWC_UTILITY_TOP_MENU_FOR_JPN ));
	GFL_HEAP_FreeMemory( work );

	// 禁止した割り込み処理を復帰
	OS_RestoreInterrupts( old );

	//マスター割り込み処理を有効に
	OS_EnableIrq( ); 


}



static GFL_PROC_RESULT WifiUtilMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFI, DWC_UTILITY_WORK_SIZE+0x100 );
    NET_DeviceLoad(GFL_NET_TYPE_WIFI);
	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT WifiUtilMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
    mydwc_callWifiUtil(HEAPID_WIFI);
    return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT WifiUtilMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
    OS_ResetSystem(0);
    return GFL_PROC_RES_FINISH;
}


const GFL_PROC_DATA WifiUtilProcData = {
	WifiUtilMainProcInit,
	WifiUtilMainProcMain,
	WifiUtilMainProcEnd,
};

