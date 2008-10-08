/*=============================================================================
/*!

	@file	npDebug.h

	@brief	デバッグ定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.2 $
	$Date: 2006/02/08 02:58:59 $

*//*=========================================================================*/
#ifndef	NPDEBUG_H
#define	NPDEBUG_H



/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	include
//-----------------------------------------------------------------------------
#include <nitro/misc.h>
#include <nitro/os.h>

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	define
//-----------------------------------------------------------------------------

#ifndef NP_DEBUG
#ifdef SDK_DEBUG
#define NP_DEBUG 1
#else
#define NP_DEBUG 0
#endif
#endif

/*!

	コピーライト

*/
#define	NP_COPYRIGHT	"Polygon magic 'BlendMagic' Library for Nitro Version 1.00.00\n"	\
						"(C) Polygon magic, Inc Graphics/Technology support team.\n"

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	enum
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!	@name	例外処理 */
//!@{

#ifdef	NP_DEBUG
#	define	npAssert( exp )				SDK_ASSERT( exp )

#else
#	define	npAssert( exp )				
#endif

//!@}

/*!	@name	メッセージ処理 */
//!@{

#ifdef	NP_DEBUG
#	define	npOutputMessage( mes )		OS_Printf( mes )
#else
#	define	npOutputMessage( mes )		
#endif

//!@}

//!	@name	エラーメッセージ処理 */
//!@{
/*
#ifdef	NP_DEBUG
#	define	npDebugOutputMessage( code, mes )							\
	if ( ( code ) != NP_SUCCESS )										\
	{																	\
		extern npCHAR	npERRORMESSAGE[ NP_ERR_MAX ][ 256 ];			\
		npCHAR			buf[ 256 ];										\
		sprintf( buf, "%s(%d) : error C%d: '%s' %s\n",					\
			__FILE__, __LINE__, code, mes, npERRORMESSAGE[ code ] );	\
		npOutputMessage( buf );											\
	}
#else
#	define	npDebugOutputMessage( code, mes )
#endif
*/
#ifdef	NP_DEBUG
#define npDebugOutputMessage( code, mes )\
	if((code) != NP_SUCCESS) {\
		OS_Printf("%s(%d) : error C%d: %s\n", __FILE__, __LINE__, code, mes);\
	}
#else
#define npDebugOutputMessage( code, mes )
#endif

//!@}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#endif	/* NPDEBUG_H */
