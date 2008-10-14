/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     hangeul_pokecode2unicode.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*! @file
	@brief	ハングルを扱うライブラリ
	
	@author	
*/

#ifndef HANGEUL_POKECODE2UNICODE_H_
#define HANGEUL_POKECODE2UNICODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "trans_pokecode.h"

wchar_t hangeulPokecode2Unicode(u16 pokecode);
u16 hangeulUnicode2Pokecode(wchar_t unicode);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // HANGEUL_POKECODE2UNICODE_H_