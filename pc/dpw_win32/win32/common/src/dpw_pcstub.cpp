#include <dwc.h>

#include "dpw_typedef.h"
#include "dpw_pcstub.h"

#include "dwc_ghttp.h"

#include <vector>
#include <algorithm>

static DWCError stDwcLastError = DWC_ERROR_NONE;  // 最後のエラー
static int stDwcErrorCode = 0;  // 最後のエラーコード
static std::vector<void*>	m_alloclist;				// メモリーアロケーションリスト

BOOL DWCi_IsError(void)
{

    if (stDwcLastError != DWC_ERROR_NONE) return TRUE;
    else return FALSE;
}


void DWCi_SetError(DWCError error, int errorCode)
{

    if (stDwcLastError != DWC_ERROR_FATAL){
        // FATAL_ERRORは上書き禁止
        stDwcLastError = error;
        stDwcErrorCode = errorCode;
    }

}

void DWC_ClearError(void) {
	
    if (stDwcLastError != DWC_ERROR_FATAL){
        // FATAL_ERRORはクリア禁止
        stDwcLastError = DWC_ERROR_NONE;
        stDwcErrorCode = 0;
    }
}

void* DWC_Alloc     ( DWCAllocType name, u32 size )
{
    void* mem = malloc( size );

	if( name == DPWI_ALLOC )
		m_alloclist.push_back( mem );

	return mem;
}


void DWC_Free      ( DWCAllocType name, void* ptr, u32 size )
{
	if( name != DPWI_ALLOC ){

		free( ptr );

		return;

	}

	std::vector<void*>::iterator it;

	it = std::find( m_alloclist.begin(), m_alloclist.end(), ptr );
	if( it != m_alloclist.end() ){

		// メモリー開放
		free( ptr );

		// リストから削除
		m_alloclist.erase( it );
	
	}else{

		// 危険な開放
		OutputDebugString(L"[WARNING!!] illigal free\n");
		printf("[WARNING!!] memory leak at %d\n", (unsigned int)*it);

	}
	
}

void Debug_MemBrowse_begin()
{
	m_alloclist.clear();
}

void Debug_MemBrowse_end()
{

	for(std::vector<void*>::iterator it = m_alloclist.begin(); it != m_alloclist.end(); it++ ){
		OutputDebugString(L"[WARNING!!] memory leak\n");
		printf("[WARNING!!] memory leak at %d\n", (unsigned int)*it);
	}

}

void MI_CpuClear8(void* pt, int size)
{
	char* ptr = (char*)pt;

	while( size-- > 0 ){
		*ptr++ = 0;
	}
}


#define OS_Sleep(msec)	(Sleep(msec))


/*---------------------------------------------------------------------------*
  Name:         MATH_InitRand32

  Description:  線形合同法による32bit乱数コンテキストを初期化します。

  Arguments:    context 乱数コンテキストへのポインタ
  
                seed    初期値として設定する乱数の種
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_InitRand32(MATHRandContext32 *context, u64 seed)
{
    context->x = seed;
    context->mul = (1566083941LL << 32) + 1812433253LL;
    context->add = 2531011;
}


/*---------------------------------------------------------------------------*
  Name:         MATH_Rand32

  Description:  線形合同法による32bit乱数取得関数

  Arguments:    context 乱数コンテキストへのポインタ
  
                max     取得数値の範囲を指定 0 ～ max-1 の範囲の値が取得できます。
                        0を指定した場合にはすべての範囲の32bit値となります。
  
  Returns:      32bitのランダム値
 *---------------------------------------------------------------------------*/
u32 MATH_Rand32(MATHRandContext32 *context, u32 max)
{
    context->x = context->mul * context->x + context->add;

    // 引数maxが定数ならばコンパイラにより最適化される。
    if (max == 0)
    {
        return (u32)(context->x >> 32);
    }
    else
    {
        return (u32)(((context->x >> 32) * max) >> 32);
    }
}


/*---------------------------------------------------------------------------*
  Name:         MATH_InitRand16

  Description:  線形合同法による16bit乱数コンテキストを初期化します。

  Arguments:    context 乱数コンテキストへのポインタ
  
                seed    初期値として設定する乱数の種
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_InitRand16(MATHRandContext16 *context, u32 seed)
{
    context->x = seed;
    context->mul = 1566083941LL;
    context->add = 2531011;
}


/*---------------------------------------------------------------------------*
  Name:         MATH_Rand16

  Description:  線形合同法による16bit乱数取得関数

  Arguments:    context 乱数コンテキストへのポインタ
  
                max     取得数値の範囲を指定 0 ～ max-1 の範囲の値が取得できます。
                        0を指定した場合にはすべての範囲の16bit値となります。
  
  Returns:      16bitのランダム値
 *---------------------------------------------------------------------------*/
u16 MATH_Rand16(MATHRandContext16 *context, u16 max)
{
    context->x = context->mul * context->x + context->add;

    // 引数maxが定数ならばコンパイラにより最適化される。
    if (max == 0)
    {
        return (u16)(context->x >> 16);
    }
    else
    {
        return (u16)(((context->x >> 16) * max) >> 16);
    }
}
