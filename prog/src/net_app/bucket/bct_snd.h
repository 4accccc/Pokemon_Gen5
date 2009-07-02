//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		bct_snd.h
 *	@brief		音ヘッダ
 *	@author		tomoya takahashi
 *	@data		2007.09.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "net_app/net_bugfix.h"
#include "sound/pm_sndsys.h"

#if WB_TEMP_FIX

#define BCT_SND_MARUIN				( SEQ_SE_DP_ELEBETA )		// マルノーム登場
#define BCT_SND_MARUIN2				( SEQ_SE_DP_HAMARU )		// マルノーム登場口があく
#define BCT_SND_MARUIN3				( SEQ_SE_DP_KI_GASYAN )		// マルノーム登場はまる

#define BCT_SND_COUNT				( SEQ_SE_DP_DECIDE )		// ３・２・１

#define BCT_SND_START				( SEQ_SE_DP_CON_016 )		// スタート

#define BCT_SND_SLOW				( SEQ_SE_DP_FW104 )			// 投げる音

#define BCT_SND_EAT					( SEQ_SE_PL_NOMI02 )		// 食べる音

//////////////////////////////////////////////////////////////////////////////
#define BCT_SND_EAT100				( SEQ_SE_PL_POINT1 )		// 食べる音

//////////////////////////////////////////////////////////////////////////////
#define BCT_SND_EAT200				( SEQ_SE_PL_POINT2 )		// 食べる音

//////////////////////////////////////////////////////////////////////////////
#define BCT_SND_EAT300				( SEQ_SE_PL_POINT3 )		// 食べる音

//////////////////////////////////////////////////////////////////////////////
#define BCT_SND_BOUND				( SEQ_SE_DP_023 )			// はじく音

#define BCT_SND_TIMEUP				( SEQ_SE_DP_PINPON )		// タイムアップ

#define BCT_SND_FEVER_CHIME			( SEQ_SE_PL_PINPON )		// フィーバー前のチャイム

#define BCT_SND_FEVER_MOVE			( SEQ_SE_PL_BALLOON05_2 )	// マルノームのフィーバー動き



#else     //WB_TEMP_FIX   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


#define BCT_SND_MARUIN				( SEQ_SE_SELECT1 )		// マルノーム登場
#define BCT_SND_MARUIN2				( SEQ_SE_SELECT1 )		// マルノーム登場口があく
#define BCT_SND_MARUIN3				( SEQ_SE_SELECT1 )		// マルノーム登場はまる

#define BCT_SND_COUNT				( SEQ_SE_SELECT1 )		// ３・２・１

#define BCT_SND_START				( SEQ_SE_SELECT1 )		// スタート

#define BCT_SND_SLOW				( SEQ_SE_SELECT1 )			// 投げる音

#define BCT_SND_EAT					( SEQ_SE_SELECT1 )		// 食べる音

//////////////////////////////////////////////////////////////////////////////
#define BCT_SND_EAT100				( SEQ_SE_SELECT1 )		// 食べる音

//////////////////////////////////////////////////////////////////////////////
#define BCT_SND_EAT200				( SEQ_SE_SELECT1 )		// 食べる音

//////////////////////////////////////////////////////////////////////////////
#define BCT_SND_EAT300				( SEQ_SE_SELECT1 )		// 食べる音

//////////////////////////////////////////////////////////////////////////////
#define BCT_SND_BOUND				( SEQ_SE_SELECT1 )			// はじく音

#define BCT_SND_TIMEUP				( SEQ_SE_SELECT1 )		// タイムアップ

#define BCT_SND_FEVER_CHIME			( SEQ_SE_SELECT1 )		// フィーバー前のチャイム

#define BCT_SND_FEVER_MOVE			( SEQ_SE_SELECT1 )	// マルノームのフィーバー動き


#endif		//WB_TEMP_FIX

