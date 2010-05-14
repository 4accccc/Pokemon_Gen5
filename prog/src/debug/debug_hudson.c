//=============================================================================
/**
 *
 *	@file		debug_hudson.c
 *	@brief  hudson用ユーティリティ
 *	@author		hosaka genya
 *	@data		2009.11.25
 *
 */
//=============================================================================

#include "debug/debug_hudson.h"

#ifdef DEBUG_ONLY_FOR_hudson

//-----------------------------------------------------------------------------
/**
 *	@brief  タイトルをスキップするか判定。引数が指定されていた場合、スキップ。
 *
 *	@param	void 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL HUDSON_IsSkipTitle( void )
{
  if( OS_GetArgc() < 2 )
  {
    return FALSE;
  }

  return TRUE;
}


#endif // DEBUG_ONLY_FOR_hudson

