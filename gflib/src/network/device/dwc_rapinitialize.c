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
#include <dwc.h>
#include "gf_macro.h"
#include "gf_standard.h"
#include "assert.h"
#include "heapsys.h"
#include "net.h"
//#include "gflib.h"
//#include "dwc_rap.h"
#include "net_wireless.h"
#include "dwc_rapinitialize.h"

//==============================================================================
/**
 * @brief   Wi-Fi接続初期化 初期化時に呼び出しておく必要がある
            network内部で呼び出すのでGFI扱い
 * @param   heapID  メモリ領域
 * @retval  none
 */
//==============================================================================
void GFL_NET_startWiFi( int heapID )
{
	OSIntrMode old;
	void *work;

    
	GFL_NET_WLVRAMDInitialize(); // 
	// アラームシステムを初期化しておかないとDWC_StartUtility呼出し後にPanicが発生する

    while(GFL_NET_WLIsVRAMDInitialize()){
        ;
    }

	OS_InitTick();
	OS_InitAlarm();

	// 割り込み処理を一旦禁止へ
	old = OS_DisableInterrupts();

	// WiFi設定メニュー呼び出し（終わるまで帰ってこない)
	work = GFL_HEAP_AllocMemory( heapID, DWC_UTILITY_WORK_SIZE );
	(void)DWC_StartUtility( work, DWC_LANGUAGE_JAPANESE, DWC_UTILITY_TOP_MENU_FOR_JPN );
	GFL_HEAP_FreeMemory( work );

	// 禁止した割り込み処理を復帰
	OS_RestoreInterrupts( old );

	//マスター割り込み処理を有効に
	OS_EnableIrq(); 

}

