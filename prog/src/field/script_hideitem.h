//======================================================================
/**
 * @file	script_hideitem.h
 * @brief	スクリプト制御：隠しアイテム関連
 * @date	09.09.13
 * 
 * 隠しアイテム関連
 */
//======================================================================
#pragma once

#define HIDE_LIST_SX				(7)		//検索範囲
#define HIDE_LIST_SZ				(7)		//検索範囲(未使用)
#define HIDE_LIST_SZ_2				(6)		//検索範囲(未使用)
#define HIDE_LIST_TOP				(7)		//検索範囲(主人公から画面上)
#define HIDE_LIST_BOTTOM			(6)		//検索範囲(主人公から画面下)
#define HIDE_LIST_RESPONSE_NONE		(0xff)	//終了コード

//--------------------------------------------------------------
/**
 * スクリプトIDから、隠しアイテムフラグナンバーを取得
 *
 * @param   scr_id		スクリプトID
 *
 * @retval  "フラグナンバー"
 */
//--------------------------------------------------------------
//extern u16 GetHideItemFlagNoByScriptId( u16 scr_id );
extern u16 SCRIPT_GetHideItemFlagNoByScriptID( u16 scr_id );

//--------------------------------------------------------------
/**
 * スクリプトIDから、隠しアイテムフラグインデックスを取得
 *
 * @param   scr_id		スクリプトID
 *
 * @retval  "フラグインデックス"
 */
//--------------------------------------------------------------
extern u16 GetHideItemFlagIndexByScriptId( u16 scr_id );

//--------------------------------------------------------------
/**
 * スクリプトIDから、隠しアイテムの反応を取得
 *
 * @param   scr_id		スクリプトID
 *
 * @retval  "反応"
 *
 * 隠しアイテムのスクリプトを見つけたら呼ぶようにする！
 */
//--------------------------------------------------------------
extern u8 GetHideItemResponseByScriptId( u16 scr_id );

//--------------------------------------------------------------
/**
 * 画面内にある隠しアイテムを検索して確保したリストに登録
 *
 * @param   heapid		ヒープID
 *
 * @retval  "リストのアドレス"
 *
 * 解放処理を忘れずに！
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
extern HIDE_ITEM_LIST * HideItem_CreateList( FLDCOMMON_WORK * fsys, int heapid );
#endif

//--------------------------------------------------------------
/**
 * 0時で隠しフラグが復活する
 *
 *
 * @retval  none
 */
//--------------------------------------------------------------
//extern void HideItemFlagOneDayClear( FLDCOMMON_WORK* fsys );

