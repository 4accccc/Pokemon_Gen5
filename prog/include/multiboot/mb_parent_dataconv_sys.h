//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mb_parent_dataconv_sys.h
 *	@brief  データコンバート用マルチブート親機メイン処理
 *	@author	Toru=Nagihashi
 *	@data		2011.07.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	リクエスト
//=====================================
typedef enum
{
  MB_PARENT_DATACONV_REQ_SEND_ROM, //ROM送信
  MB_PARENT_DATACONV_REQ_SEND_ROM_WAIT, //ROM送信停止中
  MB_PARENT_DATACONV_REQ_SEND_ROM_CANCEL, //ROM送信キャンセル

  MB_PARENT_DATACONV_REQ_CONNECT_ROM, //送信したROMと通信開始
}MB_PARENT_DATACONV_REQ;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	メインワーク
//=====================================
typedef struct _MB_PARENT_DATACONV MB_PARENT_DATACONV;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern MB_PARENT_DATACONV * MB_PARENT_DATACONV_Init( HEAPID heapID );
extern void MB_PARENT_DATACONV_Exit( MB_PARENT_DATACONV *p_wk );
extern void MB_PARENT_DATACONV_Main( MB_PARENT_DATACONV *p_wk );

extern BOOL MB_PARENT_DATACONV_ReqestState( MB_PARENT_DATACONV *p_wk, MB_PARENT_DATACONV_REQ req );
extern BOOL MB_PARENT_DATACONV_IsEndRequest( const MB_PARENT_DATACONV *cp_wk );

extern void MB_PARENT_DATACONV_SetUpRomImage( MB_PARENT_DATACONV *p_wk, const STRBUF *cp_title, const STRBUF *cp_info );

