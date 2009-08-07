//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		worldtrade_adapter.c
 *	@brief	GTSを移植する際のパック関数群
 *	@author	Toru=Nagihashi
 *	@data		2009.08.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//lib
#include <gflib.h>
//system
#include "system/main.h"
#include "system/bmp_winframe.h"

//mine
#include "worldtrade_adapter.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	PPPからニックネームを登録する関数
 *
 *	@param	WORDSET* wordset	ワードセット
 *	@param	bufID							バッファID
 *	@param	POKEMON_PASO_PARAM * ppp	PPP
 */
//-----------------------------------------------------------------------------
void WT_WORDSET_RegisterPokeNickNamePPP( WORDSET* wordset, u32 bufID, const POKEMON_PASO_PARAM * ppp )
{	
	POKEMON_PARAM *pp	= PP_CreateByPPP( ppp, HEAPID_WORLDTRADE );
	WORDSET_RegisterPokeNickName( wordset, bufID, pp );
	GFL_HEAP_FreeMemory( pp );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンパラメータ分のメモリ確保
 *	
 *	@param	HEAPID heapID		ヒープID
 *
 *	@return	PP構造体分のメモリ
 */
//-----------------------------------------------------------------------------
POKEMON_PARAM *PokemonParam_AllocWork( HEAPID heapID )
{	
	return GFL_HEAP_AllocMemory( heapID, POKETOOL_GetWorkSize() );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンパラメータのコピー
 *
 *	@param	const POKEMON_PARAM *pp_src
 *	@param	*pp_dest 
 */
//-----------------------------------------------------------------------------
void WT_PokeCopyPPtoPP( const POKEMON_PARAM *pp_src, POKEMON_PARAM *pp_dest )
{	
	GFL_STD_MemCopy( pp_src, pp_dest, POKETOOL_GetWorkSize() );
}

//----------------------------------------------------------------------------
/**
 *	@brief	クリア
 *
 *	@param	GFL_BMPWIN * winGFLBMPWIN
 *	@param	trans_sw				転送フラグ
 */
//-----------------------------------------------------------------------------
void BmpMenuWinClear( GFL_BMPWIN * win, u8 trans_sw )
{	
	GFL_BMPWIN_ClearScreen( win );
	switch( trans_sw )
	{	
	case WINDOW_TRANS_ON:
		GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(win) );
		break;
	case WINDOW_TRANS_ON_V:
		GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(win) );
		break;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	文字列展開しつつMSGを取得
 *
 *	@param	WORDSET *wordset	ワードセット
 *	@param	*MsgManager				メッセージ
 *	@param	strID							文字ID
 *	@param	heapID						テンポラリヒープID
 *
 *	@return	文字列展開後のSTRBUF
 */
//-----------------------------------------------------------------------------
STRBUF * MSGDAT_UTIL_AllocExpandString( const WORDSET *wordset, GFL_MSGDATA *MsgManager, u32 strID, HEAPID heapID )
{	
	STRBUF	*src;
	STRBUF	*dst;
	src	= GFL_MSG_CreateString( MsgManager, strID );
	dst	= GFL_STR_CreateBuffer( GFL_STR_GetBufferLength( src ), heapID );

	WORDSET_ExpandStr( wordset, src, dst );

	GFL_STR_DeleteBuffer( src );

	return dst;
}

//----------------------------------------------------------------------------
/**
 *	@brief	pppからppを作成し、コピーする
 *
 *	@param	const POKEMON_PASO_PARAM *ppp
 *	@param	*pp 
 */
//-----------------------------------------------------------------------------
void PokeReplace( const POKEMON_PASO_PARAM *ppp, POKEMON_PARAM *pp )
{	
	POKEMON_PARAM*  src	= PP_CreateByPPP( ppp, HEAPID_WORLDTRADE );
	WT_PokeCopyPPtoPP( src, pp );
	GFL_HEAP_FreeMemory( src );
}

//----------------------------------------------------------------------------
/**
 *	@brief	文字列コピー
 *
 *	@param	STRCODE *p_dst	コピー先
 *	@param	STRCODE *cp_src	コピー元
 *	@param	size						文字列長
 */
//-----------------------------------------------------------------------------
void WT_PM_strncpy( STRCODE *p_dst, const STRCODE *cp_src, int len )
{	
	u32	i;

	for( i=0; i<len; i++ ){
		p_dst[i] = cp_src[i];
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	PP2PPP
 *
 *	@param	POKEMON_PARAM *pp 
 *
 *	@return	PPP
 */
//-----------------------------------------------------------------------------
POKEMON_PASO_PARAM* PPPPointerGet( POKEMON_PARAM *pp )
{	
	return (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst( pp );
}
