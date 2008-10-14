/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     base64codec.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*! @file
	@brief	base64コーデック
	
	@author	kitayama(kitayama_shigetoshi@nintendo.co.jp)
	
	@version 0.02 (2006/8/2)
		@li base64decode() がサイズを返すようにしました。
	@version 0.01 (2006/07/20)
		@li 新規リリースしました。
*/

#ifndef BASE64CODEC_H_
#define BASE64CODEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dpw_typedef.h"

/*-----------------------------------------------------------------------*
					型・定数宣言
 *-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*
					グローバル変数定義
 *-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*
					関数外部宣言
 *-----------------------------------------------------------------------*/

//--- Auto Function Prototype --- Don't comment here.
void base64encode(const u8* in, char* out, u32 in_size);
u32 base64decode(const char* in, u8* out, u32 in_size);
u32 base64size(u32 in_size);
//--- End of Auto Function Prototype


#ifdef __cplusplus
}
#endif

#endif // BASE64CODEC_H_