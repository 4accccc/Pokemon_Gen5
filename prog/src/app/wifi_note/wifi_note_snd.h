//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wifi_note_snd.h
 *	@brief		友達手帳　サウンド
 *	@author		tomoya takahshi
 *	@data		2007.07.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __WIFI_NOTE_SND_H__
#define __WIFI_NOTE_SND_H__

#include "sound/pm_sndsys.h"



// SE用定義
//#define WIFINOTE_MOVE_SE		(SEQ_SE_SELECT)
#define WIFINOTE_MOVE_SE		(SEQ_SE_SELECT1)  //仮

#define WIFINOTE_MENU_MOVE_SE	(SEQ_SE_SELECT1)	// メニューの移動音

//#define WIFINOTE_DECIDE_SE	(SEQ_SE_SELECT)
//#define WIFINOTE_DECIDE_SE	(SEQ_SE_BUTTON3)
#define WIFINOTE_DECIDE_SE		(SEQ_SE_DECIDE1)

#define WIFINOTE_MENU_DECIDE_SE	(SEQ_SE_SELECT1)	// メニューの決定

//#define WIFINOTE_BS_SE		(SEQ_SE_SELECT)
//#define WIFINOTE_BS_SE		(SEQ_SE_BUTTON3)
#define WIFINOTE_BS_SE			(SEQ_SE_CANCEL1)

#define WIFINOTE_MENU_BS_SE		(SEQ_SE_CANCEL1)	// メニューのキャンセル

#define WIFINOTE_CLEAN_SE		(SEQ_SE_SYS_41)	// 消しゴムで消す音

//#define WIFINOTE_SCRLL_SE		(SEQ_SE_BOX02)	// スクロール音
#define WIFINOTE_SCRLL_SE		(SEQ_SE_SELECT1)	// スクロール音

#endif		// __WIFI_NOTE_SND_H__

