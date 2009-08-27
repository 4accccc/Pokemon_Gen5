//=============================================================================
/**
 * @file	dwc_rapinitialize.c
 * @bfief	初期化時に必要なWi-Fiの処理
 * @author	k.ohno
 * @date	06/03/25
 */
//=============================================================================

#include <nitro.h>
#include <nnsys.h>
#include "gflib.h"

#if GFL_NET_WIFI

//#include "net_wireless.h"
#include "net/dwc_rapcommon.h"
#include "dwc_rapinitialize.h"



//==============================================================================
/**
 * @brief   Wi-Fi接続初期化 初期化時に呼び出しておく必要がある
 * @param   heapID  メモリ領域
 * @param   エラー時に呼び出す関数
 * @retval  none
 */
//==============================================================================
void GFL_NET_WifiStart( int heapID , NetErrorFunc errorFunc)
{
    if( DWC_INIT_RESULT_DESTROY_OTHER_SETTING == mydwc_init(heapID) ){ //dwc初期化
        errorFunc(NULL, 0, NULL);
    }
}


//==============================================================================
/**
 * @brief   Wi-Fi設定画面を呼び出す
 * @param   heapID  メモリ領域
 * @retval  none
 */
//==============================================================================
void GFL_NET_WifiUtility( int heapID )
{
	OSIntrMode old;
	void *work;

//	GFL_NET_InitIchneumon(); // 
	// アラームシステムを初期化しておかないとDWC_StartUtility呼出し後にPanicが発生する

  //  while(GFL_NET_IsInitIchneumon()){
    //    ;
//    }

	OS_InitTick();
	OS_InitAlarm();

	// 割り込み処理を一旦禁止へ
	old = OS_DisableInterrupts();

	// WiFi設定メニュー呼び出し（終わるまで帰ってこない)
	work = GFL_HEAP_AllocClearMemory( heapID, DWC_UTILITY_WORK_SIZE );
	(void)DWC_StartUtility( work, DWC_LANGUAGE_JAPANESE, DWC_UTILITY_TOP_MENU_FOR_JPN );
	GFL_HEAP_FreeMemory( work );

	// 禁止した割り込み処理を復帰
	OS_RestoreInterrupts( old );

	//マスター割り込み処理を有効に
	OS_EnableIrq(); 

}

#endif //GFL_NET_WIFI
