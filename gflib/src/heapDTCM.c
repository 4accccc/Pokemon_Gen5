//==============================================================================
/**
 *
 *@file		heapDTCM.c
 *@brief	ヒープ領域管理(スタックエリア)
 *
 * Description:  DTCMエリアの使用設定をします。
 * 				領域幅が少ないので（16k=0x4000）、
 * 				メインアリーナを使用するheap.cとは異なり、
 * 				単一ヒープのみの簡易的な作りにしてあります。
 * 				初期化時に使用サイズの設定を行いますが
 * 				基本的にスタックおよび関数内メモリに割り当てられるエリアなので
 * 				大きなサイズは控えるようにしてください。
 */
//==============================================================================
#include <nitro.h>
#include <nnsys.h>
#include "heapDTCM.h"

//----------------------------------------------------------------
/**
 *	定数
 */
//----------------------------------------------------------------
#define INVALID_HANDLE_IDX	(255)			// ヒープハンドル無効値
#define AREA_MAX_SIZE		(0x3000 - 0x80)	// 最大0x3000+アリーナヘッダ

//----------------------------------------------------------------
/**
 *	ワーク定義
 */
//----------------------------------------------------------------
typedef struct {
	NNSFndHeapHandle	handle;
	OSHeapHandle		heaphandle;
	u16					count;
	u16					errorCode;
}HEAP_SYS_DTCM;

//----------------------------------------------------------------
/**
 *	メモリブロックヘッダ定義
 *
 *	使用しない
 */
//----------------------------------------------------------------

//----------------------------------------------------------------
/**
 *	変数宣言
 */
//----------------------------------------------------------------
static HEAP_SYS_DTCM  HeapSysDTCM = { 0 };

//==============================================================
// プロトタイプ宣言
//==============================================================

//------------------------------------------------------------------------------
/**
 * システム初期化
 *
 * @param	size		使用サイズ			
 *
 * @retval  BOOL		TRUEで成功／FALSEで失敗
 */
//------------------------------------------------------------------------------
BOOL
	GFI_HEAP_DTCM_Init
		( u32 size )
{
	void	*memLo, *memHi, *mem;

	//サイズ判定（現状AREA_MAX_SIZE以上は確保できないようにする）
	if( size > AREA_MAX_SIZE ){
		return FALSE;
	}
	//アリーナの上限取得
	memLo = OS_GetDTCMArenaLo();
	//アリーナの下限設定
	OS_SetDTCMArenaHi( (void*)( HW_DTCM_END - 0x1000 ) );

	memHi = (void*)((u32)memLo + size);
	mem = OS_InitAlloc( OS_ARENA_DTCM, memLo, memHi, 1 );

	//アリーナの上限再設定
	OS_SetDTCMArenaLo( mem );

	HeapSysDTCM.heaphandle = OS_CreateHeap( OS_ARENA_DTCM, memLo, memHi );
	if( (int)HeapSysDTCM.heaphandle == -1 ){
		return FALSE;
	}
	return TRUE;
}


//------------------------------------------------------------------------------
/**
 * システム終了
 *
 * @retval  BOOL		TRUEで成功／FALSEで失敗
 */
//------------------------------------------------------------------------------
BOOL
	GFI_HEAP_DTCM_Exit
		( void )
{
	OS_DestroyHeap( OS_ARENA_DTCM, HeapSysDTCM.heaphandle );
	OS_ClearAlloc( OS_ARENA_DTCM );

	return TRUE;
}


//------------------------------------------------------------------
/**
 * ヒープからメモリを確保する
 *
 * @param   size		確保サイズ
 *
 * @retval  void*		確保した領域アドレス（失敗ならNULL）
 *						errorCode：失敗原因
 */
//------------------------------------------------------------------
void*
	GFI_HEAP_DTCM_AllocMemory
		( u32 size )
{
	return OS_AllocFromHeap( OS_ARENA_DTCM, HeapSysDTCM.heaphandle, size );
}


//------------------------------------------------------------------
/**
 * ヒープからメモリを解放する
 *
 * @param   memory		確保したメモリアドレス
 *
 * @retval  BOOL		TRUEで解放成功／FALSEで失敗
 *						errorCode：失敗原因
 */
//------------------------------------------------------------------
BOOL
	GFI_HEAP_DTCM_FreeMemory
		( void* memory )
{
	OS_FreeToHeap( OS_ARENA_DTCM, HeapSysDTCM.heaphandle, memory );
	return TRUE;
}






