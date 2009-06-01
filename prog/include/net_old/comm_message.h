//=============================================================================
/**
 * @file	comm_message.h
 * @brief	通信用 メッセージを簡単に扱うためのクラス
 * @author	k.ohno
 * @date    2006.02.05
 */
//=============================================================================

#pragma once

#include "gflib.h"


/// 通信エラー用ウインドウを出す
extern void CommErrorMessageStart(int heapID);

/// 通信エラーを検査していてエラーになると止まる
extern void CommErrorCheck(int heapID);

/// 通信エラーを検査していてエラーになると止まる
extern void CommErrorDispCheck(int heapID);


