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
#include "msg/msg_tvt_call.h"
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

#define LOCAL_TVT_MSG_COLOR_WHITE   (PRINTSYS_MACRO_LSB(0xf,2,0))
#define LOCAL_TVT_MSG_COLOR_BLACK   (PRINTSYS_MACRO_LSB(1,2,0xf))
#define LOCAL_TVT_MSG_COLOR_RED     (PRINTSYS_MACRO_LSB(5,6,0))
#define LOCAL_TVT_MSG_COLOR_BLUE    (PRINTSYS_MACRO_LSB(3,4,0))

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
extern const GFL_MSGDATA *LOCAL_TVT_MESSAGE_GetSystemMsgData( const LOCAL_TVT_MESSAGE *cp_wk );
extern const GFL_MSGDATA *LOCAL_TVT_MESSAGE_GetCallMsgData( const LOCAL_TVT_MESSAGE *cp_wk );
extern const GFL_MSGDATA *LOCAL_TVT_MESSAGE_GetListMsgData( const LOCAL_TVT_MESSAGE *cp_wk );

//書き込み
extern void LOCAL_TVT_MESSAGE_SetPrintColor( LOCAL_TVT_MESSAGE *p_wk, PRINTSYS_LSB color );

typedef enum
{
  LOCAL_TVT_MESSAGE_POS_ABSOLUTE, //絶対座標
  LOCAL_TVT_MESSAGE_POS_CENTER,   //中心座標
}LOCAL_TVT_MESSAGE_POS;
extern void LOCAL_TVT_MESSAGE_SetPrintPos( LOCAL_TVT_MESSAGE *p_wk, s16 x, s16 y, LOCAL_TVT_MESSAGE_POS pos );

extern void LOCAL_TVT_MESSAGE_PrintMsg( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin, const GFL_MSGDATA *cp_msg, const u16 msgId );
extern void LOCAL_TVT_MESSAGE_PrintStr( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin, const STRBUF *cp_strbuf );
extern void LOCAL_TVT_MESSAGE_PrintPokeStatus( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin, const POKEMON_PARAM *cp_pp );

//フレーム作成
extern void LOCAL_TVT_MESSAGE_WriteFrame( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin );
extern void LOCAL_TVT_MESSAGE_ClearFrame( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin );

//単語登録
extern void LOCAL_TVT_MESSAGE_RegisterPlayerName( LOCAL_TVT_MESSAGE *p_wk, GAMEDATA *p_gamedata );
extern void LOCAL_TVT_MESSAGE_RegisterPokeNickName( LOCAL_TVT_MESSAGE *p_wk, const POKEMON_PARAM *cp_pp, u16 buffID );
extern void LOCAL_TVT_MESSAGE_RegisterWord( LOCAL_TVT_MESSAGE *p_wk, const GFL_MSGDATA *cp_msg, u16 msgID, u16 buffID );
extern void LOCAL_TVT_MESSAGE_RegisterPlaceName( LOCAL_TVT_MESSAGE *p_wk, GAMEDATA *p_gamedata, u16 buffID );
extern void LOCAL_TVT_MESSAGE_RegisterNumber( LOCAL_TVT_MESSAGE *p_wk, u32 num, u32 keta, u16 buffID );
extern void LOCAL_TVT_MESSAGE_RegisterQuestionnaire( LOCAL_TVT_MESSAGE *p_wk, u16 buffID, u16 answerID );

//メッセージウィンドウ処理
extern void LOCAL_TVT_MSGWIN_OpenWindow( LOCAL_TVT_MESSAGE *p_wk, LOCAL_TVT_MSG_POS pos );
extern void LOCAL_TVT_MSGWIN_CloseWindow( LOCAL_TVT_MESSAGE *p_wk );
extern void LOCAL_TVT_MSGWIN_DispMessage( LOCAL_TVT_MESSAGE *p_wk, const GFL_MSGDATA *cp_msg, const u16 msgId );
extern BOOL LOCAL_TVT_MSGWIN_IsFinish( const LOCAL_TVT_MESSAGE *cp_wk );
extern BOOL LOCAL_TVT_MSGWIN_IsPause( const LOCAL_TVT_MESSAGE *cp_wk );

//便利関数
static inline void LOCAL_TVT_MESSAGE_PrintMsgSys( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin, const u16 msgId )
{
  LOCAL_TVT_MESSAGE_PrintMsg( p_wk, p_bmpwin, LOCAL_TVT_MESSAGE_GetSystemMsgData(p_wk), msgId );
}
static inline void LOCAL_TVT_MESSAGE_PrintMsgCall( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin, const u16 msgId )
{
  LOCAL_TVT_MESSAGE_PrintMsg( p_wk, p_bmpwin, LOCAL_TVT_MESSAGE_GetCallMsgData(p_wk), msgId );
}
