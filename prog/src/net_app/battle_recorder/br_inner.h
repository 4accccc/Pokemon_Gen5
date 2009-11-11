//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_inner.h
 *	@brief	バトルレコーダー	プライベートヘッダ
 *	@author	Toru=Nagihashi
 *	@date		2009.11.11
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
///	BG面設定
//=====================================
enum 
{
	BG_FRAME_M_BACK	= 0,
	BG_FRAME_S_BACK	= 0,
};

//-------------------------------------
///	PLT設定
//=====================================
enum 
{
	PLT_BG_M_BACK	= 0,
	PLT_BG_S_BACK	= 0,
};

//-------------------------------------
///	プロセスID
//=====================================
typedef enum 
{
	BR_PROCID_START,		//起動画面
	BR_PROCID_MENU,			//メニュー画面
	BR_PROCID_RECORD,		//録画記録画面
	BR_PROCID_BTLSUBWAY,//バトルサブウェイ成績画面
	BR_PROCID_RNDMATCH,	//ランダムマッチ成績画面
	BR_PROCID_BV_RANK,	//バトルビデオ30件画面（最新、通信対戦、サブウェイ）
	BR_PROCID_BV_SEARCH,//詳しく探す画面
	BR_PROCID_BV_CODEIN,//バトルビデオナンバー入力画面
	BR_PROCID_BV_SEND,	//バトルビデオ送信画面
	BR_PROCID_MUSICAL_LOOK,	//ミュージカルショット	写真を見る画面
	BR_PROCID_MUSICAL_SEND,	//ミュージカルショット	写真を送る画面

} BR_PROCID;


//@todoあとでMENUプロックに移動
//-------------------------------------
///	メニューインデックス
//		ここでのメニューとは上画面が切り替らず、
//		メニューを押していくことで進んでいく箇所です。
//		（消すは入ります）
//=====================================
typedef enum
{
	BR_BROWSE_MENUID_TOP,					//ブラウザモード　初期メニュー

	BR_BROWSE_MENUID_BTLVIDEO,		//ブラウザモード　バトルビデオメニュー,

	BR_BROWSE_MENUID_OTHER_RECORD,//ブラウザモード　だれかの記録メニュー
	BR_BROWSE_MENUID_DELETE_RECORD,//ブラウザモード　記録を消すメニュー
	BR_BROWSE_MENUID_DELETE_OTHER,//ブラウザモード　誰かの記録を消すメニュー

	//BR_BROWSE_MENUID_BTLSUBWAY,		//ブラウザモード　バトルサブウェイメニュー	
	//BR_BROWSE_MENUID_RNDMATCH,		//ブラウザモード　ランダムマッチメニュー
	
	BR_BTLVIDEO_MENUID_TOP,					//グローバルバトルビデオモード　初期メニュー

	BR_BTLVIDEO_MENUID_LOOK,				//グローバルバトルビデオモード　見るメニュー
	BR_BTLVIDEO_MENUID_RANK,				//グローバルバトルビデオモード　ランキングで探すメニュー

	BR_MUSICAL_MENUID_TOP,					//グローバルミュージカルショットモード　初期メニュー

	BR_MENUID_YESNO,								//汎用	はい、いいえ
	BR_MENUID_MAX
} BR_MENUID;
