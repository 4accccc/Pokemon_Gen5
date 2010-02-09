//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		shortcut_data.h
 *	@brief	Yボタン登録
 *	@author	Toru=Nagihashi
 *	@date		2009.10.20
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
///	Yボタン登録箇所
//=====================================
typedef enum
{
	SHORTCUT_ID_ZITENSYA,			//自転車
	SHORTCUT_ID_TOWNMAP,			//タウンマップ
	SHORTCUT_ID_PALACEGO,			//パレスへゴー
	SHORTCUT_ID_BTLRECORDER,	//バトルレコーダー
	SHORTCUT_ID_FRIENDNOTE,		//友達手帳
	SHORTCUT_ID_TURIZAO,			//つりざお

	SHORTCUT_ID_PSTATUS_STATUS,		//ポケモン情報＞ステータス
	SHORTCUT_ID_PSTATUS_ABILITY,	//ポケモン情報＞のうりょく
	SHORTCUT_ID_PSTATUS_RIBBON,		//ポケモン情報＞きねんリボン
	SHORTCUT_ID_BAG_ITEM,					//バッグ＞どうぐ
	SHORTCUT_ID_BAG_RECOVERY,			//バッグ＞かいふく
	SHORTCUT_ID_BAG_WAZAMACHINE,	//バッグ＞技マシン
	SHORTCUT_ID_BAG_NUTS,					//バッグ＞きのみ
	SHORTCUT_ID_BAG_IMPORTANT,		//バッグ＞たいせつなもの
	SHORTCUT_ID_ZUKAN_LIST,				//ずかん＞リスト
	SHORTCUT_ID_ZUKAN_SEARCH,			//ずかん＞検索
	SHORTCUT_ID_ZUKAN_INFO,		  	//ずかん＞詳細・説明
	SHORTCUT_ID_ZUKAN_MAP,	  		//ずかん＞詳細・分布
	SHORTCUT_ID_ZUKAN_VOICE,			//ずかん＞詳細・鳴き声
	SHORTCUT_ID_ZUKAN_FORM,				//ずかん＞詳細・姿
	SHORTCUT_ID_TRCARD_FRONT,			//トレーナーカード＞ぜんめん
	SHORTCUT_ID_TRCARD_BACK,			//トレーナーカード＞うらめん
	SHORTCUT_ID_CONFIG,						//せってい


	SHORTCUT_ID_MAX,
	SHORTCUT_ID_NULL	= SHORTCUT_ID_MAX,	//データなし
} SHORTCUT_ID;
//=============================================================================
/**
 *					構造体
*/
//=============================================================================
//-------------------------------------
///	ショートカットのカーソル
//=====================================
typedef struct 
{
	u16 list;
	u16 cursor;
} SHORTCUT_CURSOR;


//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//-------------------------------------
///	カーソル構造体初期化
//=====================================
extern void SHORTCUT_CURSOR_Init( SHORTCUT_CURSOR *p_wk );

//-------------------------------------
///	UTIL
//=====================================
extern SHORTCUT_ID SHORTCUT_DATA_GetItemToShortcutID( u16 item );
