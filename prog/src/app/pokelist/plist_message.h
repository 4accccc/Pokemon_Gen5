//======================================================================
/**
 * @file	plist_message.c
 * @brief	ポケモンリスト メッセージ
 * @author	ariizumi
 * @data	09/06/17
 *
 * モジュール名：PLIST_MSG
 */
//======================================================================
#pragma once

#include "plist_local_def.h"

//メッセージウィンドウの種類
typedef enum
{
  PMT_BAR,        //バーの1行メッセージ
  PMT_BAR_BATTLE, //バーの1行メッセージ(決定が出てる時
  PMT_MENU,       //メニューの小さめ2行
  PMT_MESSAGE,    //フルサイズ
  
  PMT_NONE,     //ウィンドウ閉じてる
  PMT_MAX,
}PLIST_MSGWIN_TYPE;


extern PLIST_MSG_WORK* PLIST_MSG_CreateSystem( PLIST_WORK *work );
extern void PLIST_MSG_DeleteSystem( PLIST_WORK *work , PLIST_MSG_WORK *msgWork );
extern void PLIST_MSG_UpdateSystem( PLIST_WORK *work , PLIST_MSG_WORK *msgWork );

extern void PLIST_MSG_OpenWindow( PLIST_WORK *work , PLIST_MSG_WORK *msgWork , const PLIST_MSGWIN_TYPE winType);
extern void PLIST_MSG_CloseWindow( PLIST_WORK *work , PLIST_MSG_WORK *msgWork );
extern const BOOL PLIST_MSG_IsOpenWindow( PLIST_WORK *work , PLIST_MSG_WORK *msgWork );

extern void PLIST_MSG_DrawMessageNoWait( PLIST_WORK *work , PLIST_MSG_WORK *msgWork , const u32 msgId );
extern void PLIST_MSG_DrawMessageStream( PLIST_WORK *work , PLIST_MSG_WORK *msgWork , const u32 msgId , const BOOL isWait );
extern const BOOL PLIST_MSG_IsFinishMessage( PLIST_WORK *work , PLIST_MSG_WORK *msgWork );

extern void PLIST_MSG_CreateWordSet( PLIST_WORK *work , PLIST_MSG_WORK *msgWork );
extern void PLIST_MSG_DeleteWordSet( PLIST_WORK *work , PLIST_MSG_WORK *msgWork );
extern void PLIST_MSG_AddWordSet_PokeName( PLIST_WORK *work , PLIST_MSG_WORK *msgWork , u8 wordSetIdx , POKEMON_PARAM *pp );
extern void PLIST_MSG_AddWordSet_ItemName( PLIST_WORK *work , PLIST_MSG_WORK *msgWork , u8 wordSetIdx , u16 itemIdx );
extern void PLIST_MSG_AddWordSet_SkillName( PLIST_WORK *work , PLIST_MSG_WORK *msgWork , u8 wordSetIdx , u16 skillNo );
extern void PLIST_MSG_AddWordSet_StatusName( PLIST_WORK *work , PLIST_MSG_WORK *msgWork , u8 wordSetIdx , u16 statusId );
extern void PLIST_MSG_AddWordSet_Value( PLIST_WORK *work , PLIST_MSG_WORK *msgWork , u8 wordSetIdx , u16 value , u8 keta );
extern void PLIST_MSG_AddWordSet_Word( PLIST_WORK *work , PLIST_MSG_WORK *msgWork , u8 wordSetIdx , STRBUF *str , u32 sex );

