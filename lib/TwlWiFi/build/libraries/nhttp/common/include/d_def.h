/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     d_def.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
/**
 *	@file
 *	共通定義
 *	@author
 *	@note 
 */

#ifndef	__D_DEF_H__
#define	__D_DEF_H__


/**
 *	PRINTFマクロの定義
 */
#if	defined(NDEBUG)||defined (SDK_FINALROM)
#define	PRINTF(...)	((void)0)
#else
#define	PRINTF(...)	NHTTPi_printf(__VA_ARGS__)
#endif	/* defined(NDEBUG) end */

#define	FFL_PRINTF_common(str,...)	\
    PRINTF("%s [%s %d]:"##str,               \
           __func__, __FILE__, __LINE__,__VA_ARGS__)
#define	FFL_PRINTF()		FFL_PRINTF_common("\n",0)
#define	FFL_PRINTF2(str, ...)	FFL_PRINTF_common(str,__VA_ARGS__)
#define	FL_PRINTF_common(str,...)	\
    PRINTF("[%s %d]:"##str,               \
           __FILE__, __LINE__,__VA_ARGS__)
#define	FL_PRINTF()	FL_PRINTF_common("\n",0)
#define	FL_PRINTF2(str,...)	FL_PRINTF_common(str,__VA_ARGS__)


#if defined (U_noma_U) || defined (U_gen_U)
#define DEBUG_PRINTF(...)       PRINTF(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)       ((void)0)
#endif

#endif	/* __D_DEF_H__ end */

