//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_nagihashi.h
 *	@brief  デバッグ用便利関数群
 *	@author	Toru=Nagihashi
 *	@date		2010.02.13
 *	@note   オーバーレイを無視するためにヘッダに全て記述。
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef PM_DEBUG

//ライブラリ
#include <gflib.h>

//モジュール
#include "debug/debug_str_conv.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define   DEBUG_NAGI_Printf( ... )      OS_TPrintf( __VA_ARGS__ )

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  STRBUFの中身を表示（漢字は無理）
 *
 *	@param	const STRBUF *cp_str STRBUF
 */
//-----------------------------------------------------------------------------
static inline void DEBUG_STRBUF_Print( const STRBUF *cp_str )
{

  const int len     = GFL_STR_GetBufferLength(cp_str);
  const STRCODE *cp_code = GFL_STR_GetStringCodePointer( cp_str );

  { 
    int i;
    char str[3] = {0};
    for( i = 0; i < len; i++ )
    {
      DEB_STR_CONV_StrcodeToSjis( &cp_code[i] , str , 1 );
      DEBUG_NAGI_Printf( "%s ", str );
    }
    DEBUG_NAGI_Printf("\n");
  }

}

//=============================================================================
/**
 *					処理時間計算マクロ
 *					  PrintStartとPrintEndで挟んだ箇所をマイクロ秒で図ります
*/
//=============================================================================
static OSTick s_DUN_TICK_DRAW_start;
#define DEBUG_TICK_PrintStart  (s_DUN_TICK_DRAW_start = OS_GetTick())
#define DEBUG_TICK_PrintEnd    DEBUG_NAGI_Printf("file[%s] line[%d] time=%dms\n", __FILE__, __LINE__,OS_TicksToMicroSeconds(OS_GetTick() - s_DUN_TICK_DRAW_start) )

//=============================================================================
/**
 *					ヒープ残りサイズプリント関数
 *					  以下関数を毎フレームまわしてください
*/
//=============================================================================
static inline void DEBUG_HEAP_PrintRestUse( HEAPID heapID )
{ 
  static u32 s_max = 0xFFFFFFFF;
  int rest;

  rest  = GFL_HEAP_GetHeapFreeSize( heapID );
  if( s_max > rest )
  { 
    s_max = rest;
  }
  DEBUG_NAGI_Printf( "HEAPID[0x%x] most low rest size=0x%x\n", heapID, s_max );
}

#else //PM_DEBUG
#define   DEBUG_NAGI_Printf( ... )  /* */
#define DEBUG_STRBUF_Print(x)       /* */
#define DEBUG_TICK_PrintStart       /* */
#define DEBUG_TICK_PrintEnd         /* */
#define DEBUG_HEAP_PrintRestUse(x)  /* */
#endif //PM_DEBUG
