//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		boast_message_local.h
 *	@brief  すれちがい　自慢メッセージ
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.09.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/boast_message.h"

///すれ違い：自己紹介メッセージ長
#define SAVE_SURETIGAI_BOAST_LEN   (8 + 1)  //EOM込み

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

struct _BOAST_MSG_SAVEDATA{
  STRCODE boast_message[SAVE_SURETIGAI_BOAST_LEN];  ///<自慢メッセージ
};


