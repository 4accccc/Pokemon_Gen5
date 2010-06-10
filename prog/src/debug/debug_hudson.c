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

//-----------------------------------------------------------------------------
/**
 *	@brief  テストコードをチェック
 *
 *	@param	const char* str 
 *
 *	@retval TRUE:指定したテストコードと一致
 */
//-----------------------------------------------------------------------------
BOOL HUDSON_IsTestCode( const char* str )
{ 
  return ( OS_GetArgc() >= 2 && STD_StrCmp( OS_GetArgv(1), str ) == 0 );
}

#endif // DEBUG_ONLY_FOR_hudson

