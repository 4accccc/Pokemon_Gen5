/*=============================================================================
/*!

	@file	npTypedef.h

	@brief	Core include file.
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.1 $
	$Date: 2006/02/06 10:51:36 $

*//*=========================================================================*/
#ifndef	NPTYPEDEF_H
#define	NPTYPEDEF_H

//!@}

/*! @name	型チェックを厳密に行う	*/
//!@{

#ifndef STRICT
#	define	STRICT
#endif

//!@}

/*!	@name	ヘッダからあまり使われない関数を省く */
//!@{

#define	WIN32_LEAN_AND_MEAN

//!@}

/*! @name	C リンケージ外対応 */
//!@{
#ifndef __MWERKS__
#include <windows.h>
#endif	//__MWERKS__

//#include <stdio.h>
#include <nitro/std.h>

/*! @name	alchemy library link */
//!@{
/*
#include <igGapDisplay.h>
#include <igDisplay/igWindow.h>
#include <igDisplay/igDefaultInterfaceManager.h>
#include <igDisplay/igControllerManager.h>
#include <igGapGfx.h>
#include <igGfx/igVisualContext.h>
#include <igGfx/igImage.h>
#include <igGfx/igPngLoader.h>


using namespace Gap;


#ifdef	NP_DEBUG
#	include <tchar.h>
#	include <crtdbg.h>
#endif
*/

//!@}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	include
//-----------------------------------------------------------------------------
#include "core/npType.h"
#include "core/npConst.h"
#include "core/npError.h"
#include "core/npDebug.h"

#include "core/npMemory.h"
#include "npRandom.h"

#include "core/npMacro.h"
#include "core/npMath.h"

#include "core/npVector.h"
#include "core/npMatrix.h"
#include "core/npQuate.h"

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

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


#endif	/* NPTYPEDEF_H */
