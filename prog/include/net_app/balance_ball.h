//==============================================================================
/**
 * @file	balance_ball.h
 * @brief	ŠÈ’P‚Èà–¾‚ğ‘‚­
 * @author	goto
 * @date	2007.09.25(‰Î)
 *
 * ‚±‚±‚ÉFX‚È‰ğà“™‚ğ‘‚¢‚Ä‚à‚æ‚¢
 *
 */
//==============================================================================
#ifndef __BALANCE_BALL_H__
#define __BALANCE_BALL_H__

#include <procsys.h>
#include "savedata/savedata_local.h"
#include "net_app/wifi_lobby/minigame_tool.h"

typedef struct {
	WFLBY_MINIGAME_WK lobby_wk;

//	u8 voice_chat;
	SAVE_CONTROL_WORK*	p_save;
	BOOL		wifi_lobby;
	u32			vchat;			///< voice_chat
	
} BB_PROC_WORK;


extern GFL_PROC_RESULT BalanceBallProc_Init( PROC* proc, int* seq );
extern GFL_PROC_RESULT BalanceBallProc_Main( PROC* proc, int* seq );
extern GFL_PROC_RESULT BalanceBallProc_Exit( PROC* proc, int* seq );


#endif		// __BUCKET_H__

