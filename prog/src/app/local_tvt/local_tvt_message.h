//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_message.h
 *	@brief  ローカルライブキャスターメッセージ処理
 *	@author	Toru=Nagihashi
 *	@date		2011.01.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "print/gf_font.h"
#include "print/printsys.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
typedef enum
{
  LTMP_UP,
  LTMP_DOWN,
}LOCAL_TVT_MSG_POS;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//ハンドル
typedef struct _LOCAL_TVT_MESSAGE LOCAL_TVT_MESSAGE;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//システム
extern LOCAL_TVT_MESSAGE * LOCAL_TVT_MESSAGE_Init( HEAPID heapID );
extern void LOCAL_TVT_MESSAGE_Exit( LOCAL_TVT_MESSAGE *p_wk );
extern void LOCAL_TVT_MESSAGE_Main( LOCAL_TVT_MESSAGE *p_wk );

//メッセージ関係の取得
extern GFL_FONT *LOCAL_TVT_MESSAGE_GetFont( const LOCAL_TVT_MESSAGE *cp_wk );
extern PRINT_QUE *LOCAL_TVT_MESSAGE_GetPrintQue( const LOCAL_TVT_MESSAGE *cp_wk );

extern void LOCAL_TVT_MESSAGE_RegisterPlayerName( LOCAL_TVT_MESSAGE *p_wk, GAMEDATA *p_gamedata );

//メッセージウィンドウ処理
extern void LOCAL_TVT_MSGWIN_OpenWindow( LOCAL_TVT_MESSAGE *p_wk, LOCAL_TVT_MSG_POS pos );
extern void LOCAL_TVT_MSGWIN_CloseWindow( LOCAL_TVT_MESSAGE *p_wk );
extern void LOCAL_TVT_MSGWIN_DispMessage( LOCAL_TVT_MESSAGE *p_wk, GFL_MSGDATA *p_msg, const u16 msgId );
extern BOOL LOCAL_TVT_MSGWIN_IsFinish( const LOCAL_TVT_MESSAGE *cp_wk );
extern BOOL LOCAL_TVT_MSGWIN_IsPause( const LOCAL_TVT_MESSAGE *cp_wk );
