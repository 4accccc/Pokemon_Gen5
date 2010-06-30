//=============================================================================
/**
 * @file	net_alone_test.h
 * @brief	通信システム 接続相手制限コード定義 
 * @author	GAME FREAK Inc.
 * @date    2010.05.21
 *
 * @note  net_whpipe.cが分割されたので.cから分離 by iwasawa
 */
//=============================================================================
#pragma once

#ifdef PM_DEBUG

// プログラマは基本他の人とつながらない  常に自分のマシンと接続して開発できるように
// デバッグメニューか何かでで切り替えたら全員とつながることにする
// 全員とつながる番号は０(製品版)

#ifdef PLAYABLE_VERSION
#define _DEBUG_ALONETEST_DEFAULT  (100)
#define _DEBUG_ALONETEST (_DEBUG_ALONETEST_DEFAULT)

#else

//誰でも繋がる(デバッグ版)　パレスの通信バージョンとして使用 昔の通信と接続で問題があるときにバージョンを上げていく
#define _DEBUG_ALONETEST_DEFAULT  (123)

#ifdef DEBUG_ONLY_FOR_ohno
#define _DEBUG_ALONETEST (1)
#elif DEBUG_ONLY_FOR_sogabe
#define _DEBUG_ALONETEST (2)
#elif DEBUG_ONLY_FOR_matsuda
#define _DEBUG_ALONETEST (3)
#elif DEBUG_ONLY_FOR_gotou
#define _DEBUG_ALONETEST (4)
#elif DEBUG_ONLY_FOR_tomoya_takahashi
#define _DEBUG_ALONETEST (5)
#elif DEBUG_ONLY_FOR_tamada
#define _DEBUG_ALONETEST (6)
#elif DEBUG_ONLY_FOR_kagaya
#define _DEBUG_ALONETEST (7)
#elif DEBUG_ONLY_FOR_nohara
#define _DEBUG_ALONETEST (8)
#elif DEBUG_ONLY_FOR_taya
#define _DEBUG_ALONETEST (9)
#elif DEBUG_ONLY_FOR_hiro_nakamura
#define _DEBUG_ALONETEST (10)
#elif DEBUG_ONLY_FOR_mituhara
#define _DEBUG_ALONETEST (11)
#elif DEBUG_ONLY_FOR_watanabe
#define _DEBUG_ALONETEST (12)
#elif DEBUG_ONLY_FOR_genya_hosaka
#define _DEBUG_ALONETEST (13)
#elif DEBUG_ONLY_FOR_toru_nagihashi
#define _DEBUG_ALONETEST (14)
#elif DEBUG_ONLY_FOR_mori
#define _DEBUG_ALONETEST (15)
#elif DEBUG_ONLY_FOR_iwasawa
#define _DEBUG_ALONETEST (16)
#elif DEBUG_ONLY_FOR_ohmori
#define _DEBUG_ALONETEST (17)
//#elif DEBUG_ONLY_FOR_ariizumi_nobuhiko
//#define _DEBUG_ALONETEST (18)
#elif DEBUG_ONLY_FOR_palace_debug
#define _DEBUG_ALONETEST (19)
#elif DEBUG_ONLY_FOR_palace_debug_white
#define _DEBUG_ALONETEST (20)
#elif DEBUG_ONLY_FOR_palace_debug_black
#define _DEBUG_ALONETEST (21)
#elif DEBUG_ONLY_FOR_debug_comm
#define _DEBUG_ALONETEST (22)
#elif DEBUG_ONLY_FOR_debug_graphic_black
#define _DEBUG_ALONETEST (23)
#elif DEBUG_ONLY_FOR_debug_graphic_white
#define _DEBUG_ALONETEST (24)
#else
//誰でも繋がる、、、が、パレスの通信バージョンとして使用 昔の通信と接続で問題があるときにバージョンを上げていく
#define _DEBUG_ALONETEST (_DEBUG_ALONETEST_DEFAULT)
#endif

#endif //PLAYABLE_VERSION

#endif  //PM_DEBUG

