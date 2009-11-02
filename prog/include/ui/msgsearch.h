//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		msgsearch.h
 *	@brief	MSGから文字列を検索するモジュール
 *	@author	Toru=Nagihashi
 *	@date		2009.10.30
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	MSGデータから文字列を検索するワーク
//=====================================
typedef struct _MSGSEARCH_WORK MSGSEARCH_WORK;

//-------------------------------------
///	検索結果受け取り構造体
//=====================================
typedef struct 
{
	s32 msg_idx;			//-1だと見つからなかった
	s32 str_idx;			//-1だと見つからなかった
} MSGSEARCH_RESULT;


//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//-------------------------------------
///	初期化、破棄
//=====================================
extern MSGSEARCH_WORK *MSGSEARCH_Init( GFL_MSGDATA *p_msg_tbl[], u32 msg_max, HEAPID heapID );
extern void MSGSEARCH_Exit( MSGSEARCH_WORK *p_wk );
//-------------------------------------
///	検索
//=====================================
extern u32 MSGSEARCH_Search( const MSGSEARCH_WORK *cp_wk, u16 msg_idx, u16 start_str_idx, const STRBUF *cp_buf, MSGSEARCH_RESULT *p_result_tbl, u32 tbl_max );
extern u32 MSGSEARCH_SearchAll( const MSGSEARCH_WORK *cp_wk, const STRBUF *cp_buf, MSGSEARCH_RESULT *p_result_tbl, u32 tbl_max );
extern u32 MSGSEARCH_SearchNum( const MSGSEARCH_WORK *cp_wk, const STRBUF *cp_buf );
//-------------------------------------
///	文字列関係
//=====================================
extern STRBUF *MSGSEARCH_CreateString( const MSGSEARCH_WORK	*cp_wk, const MSGSEARCH_RESULT *cp_result_tbl );
extern void MSGSEARCH_GetString( const MSGSEARCH_WORK *cp_wk, const MSGSEARCH_RESULT *cp_result_tbl, STRBUF *p_str );

