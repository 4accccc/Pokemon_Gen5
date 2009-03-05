//=============================================================================
/**
 * @file	dwc_raputility.c
 * @bfief	WIFI設定画面呼び出し用
 * @author	k.ohnop
 * @date	09/01/15
 */
//=============================================================================

#include <gflib.h>

//==============================================================================
/**
 * WiFi接続ユーティリティを呼び出す
 * @param   none
 * @retval  none
 */
//==============================================================================
void mydwc_callUtility( HEAPID HeapID )
{
	OSIntrMode old;
	void *work;

	// アラームシステムを初期化しておかないとDWC_StartUtility呼出し後にPanicが発生する
	OS_InitTick();
	OS_InitAlarm();


	// 割り込み処理を一旦禁止へ
	old = OS_DisableInterrupts();

	// WiFi設定メニュー呼び出し（終わるまで帰ってこない)
	work = GFL_HEAP_AllocClearMemory( HeapID, DWC_UTILITY_WORK_SIZE );
	(void)DWC_StartUtility( work, DWC_LANGUAGE_JAPANESE, DWC_UTILITY_TOP_MENU_FOR_JPN );
	GFL_HEAP_FreeMemory( work );

	// 禁止した割り込み処理を復帰
	OS_RestoreInterrupts( old );

	//マスター割り込み処理を有効に
	OS_EnableIrq( ); 


}