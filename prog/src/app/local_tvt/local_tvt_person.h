//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_person.h
 *	@brief  ローカルライブキャスターで話しかける人物抽象システム
 *	@author	Toru=Nagihashi
 *	@date		2011.01.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "local_tvt_message.h"
#include "local_tvt_chara.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//選択肢最大数
#define LOCAL_TVT_PERSON_SELECT_FUNCTION  (4)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	人物システム
//=====================================
typedef struct _LOCAL_TVT_PERSON_SYS LOCAL_TVT_PERSON_SYS;

//-------------------------------------
///	抽象ワーク
//=====================================
typedef struct 
{
  void  *p_wk_adrs;     //それぞれの人物実装で使用するワーク
  BOOL  (*greetingFunction)( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs );  //選択肢前挨拶のときに実行される関数
  BOOL  (*selectFunction[LOCAL_TVT_PERSON_SELECT_FUNCTION])( LOCAL_TVT_PERSON_SYS *p_person, void *p_wk_adrs );  //選択肢を選んだ後に実行される関数
  u32   list_msgID[LOCAL_TVT_PERSON_SELECT_FUNCTION]; //選択肢に表示する文字ID
  u32   list_num; //選択肢の数
  GFL_MSGDATA *p_list_msg_data; //選択肢に表示するメッセージデータ
} LOCAL_TVT_PERSON;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//システム側で使用する関数
extern LOCAL_TVT_PERSON_SYS * LOCAL_TVT_PERSON_SYS_Init( const LOCAL_TVT_PERSON *cp_person, LOCAL_TVT_MESSAGE *p_message , LOCAL_TVT_CHARA_SYS *p_chara_sys, GFL_CLUNIT *p_clunit, HEAPID heapID );
extern void LOCAL_TVT_PERSON_SYS_Exit( LOCAL_TVT_PERSON_SYS *p_wk );
extern void LOCAL_TVT_PERSON_SYS_Main( LOCAL_TVT_PERSON_SYS *p_wk );
extern BOOL LOCAL_TVT_PERSON_SYS_IsEnd( const LOCAL_TVT_PERSON_SYS *cp_wk );

//キャラクタ実装側で使用する関数
extern void LOCAL_TVT_PERSON_End( LOCAL_TVT_PERSON_SYS *p_wk );
extern LOCAL_TVT_MESSAGE * LOCAL_TVT_PERSON_GetMessage( const LOCAL_TVT_PERSON_SYS *cp_wk );
extern LOCAL_TVT_CHARA_SYS * LOCAL_TVT_PERSON_GetCharaSys( const LOCAL_TVT_PERSON_SYS *cp_wk );
extern GFL_CLUNIT * LOCAL_TVT_PERSON_GetClunit( const LOCAL_TVT_PERSON_SYS *cp_wk );
