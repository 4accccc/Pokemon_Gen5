//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_mystery_card.c
 *	@brief  デバッグ用ふしぎなおくりもの作成  
 *	@author	Toru=Nagihashi
 *	@date		2010.02.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//モジュール
#include "poke_tool/monsno_def.h"
#include "item/itemsym.h"

//セーブデータ
#include "savedata/mystery_data.h"

//外部公開
#include "debug/debug_mystery_card.h"
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

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンいりのデータを作成
 *
 *	@param	GIFT_PACK_DATA *p_data  データ
 */
//-----------------------------------------------------------------------------
void DEBUG_MYSTERY_SetGiftPokeData( GIFT_PACK_DATA *p_data, u32 event_id )
{ 
  GFL_STD_MemClear( p_data, sizeof(GIFT_PACK_DATA) );
  //ポケモン作成
  { 
    GIFT_PRESENT_POKEMON  *p_poke = &p_data->data.pokemon;
    p_poke->mons_no = MONSNO_KORATTA;
    p_poke->form_no = 0;
    p_poke->waza1   = 1;
    p_poke->oyaname[0]  = L'デ';
    p_poke->oyaname[1]  = L'バ';
    p_poke->oyaname[2]  = L'ッ';
    p_poke->oyaname[3]  = L'グ';
    p_poke->oyaname[4]  = 0xffff;
    p_poke->oyaname[4]  = 0xffff;
  }
  p_data->gift_type     = MYSTERYGIFT_TYPE_POKEMON;

  //共通部分
  DEBUG_MYSTERY_SetGiftCommonData( p_data, event_id, TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  道具いりのデータを作成
 *
 *	@param	GIFT_PACK_DATA *p_data  データ 
 */
//-----------------------------------------------------------------------------
void DEBUG_MYSTERY_SetGiftItemData( GIFT_PACK_DATA *p_data, u32 event_id )
{ 
  GFL_STD_MemClear( p_data, sizeof(GIFT_PACK_DATA) );
  //道具作成
  { 
    GIFT_PRESENT_ITEM  *p_item = &p_data->data.item;
    p_item->itemNo    = ITEM_MONSUTAABOORU;
    p_item->movieflag = FALSE;
  }
  p_data->gift_type     = MYSTERYGIFT_TYPE_ITEM;

  //共通部分
  DEBUG_MYSTERY_SetGiftCommonData( p_data, event_id, TRUE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  共通部分のデータ作成
 *
 *	@param	DOWNLOAD_GIFT_DATA *p_data  データ
 *	@param	event_id
 *	@param	only_one_flag
 *	@param	version 
 */
//-----------------------------------------------------------------------------
void DEBUG_MYSTERY_SetGiftCommonData( GIFT_PACK_DATA *p_data, u32 event_id, BOOL only_one_flag )
{ 
  //イベントタイトル
  p_data->event_name[ 0] = L'デ';
  p_data->event_name[ 1] = L'バ';
  p_data->event_name[ 2] = L'ッ';
  p_data->event_name[ 3] = L'グ';
  p_data->event_name[ 4] = L'イ';
  p_data->event_name[ 5] = L'ベ';
  p_data->event_name[ 6] = L'ン';
  p_data->event_name[ 7] = L'ト';
  p_data->event_name[ 8] = L'タ';
  p_data->event_name[ 9] = L'イ';
  p_data->event_name[10] = L'ト';
  p_data->event_name[11] = L'ル';
  p_data->event_name[12] = 0xffff;

  //受信した時間
  {
    RTCDate date;
    GFL_RTC_GetDate( &date );
    p_data->recv_date      = ((2000+date.year)<<MYSTERYGIFT_DATE_SHIFT_YEAR) | (date.month<<MYSTERYGIFT_DATE_SHIFT_MONTH) | (date.day<<MYSTERYGIFT_DATE_SHIFT_DAY);
  }

  //イベントID
  p_data->event_id       = event_id;

  //データ
  p_data->card_message   = 0;
  
  //一度だけ受信フラグ
  p_data->only_one_flag  = only_one_flag;

  //受け取ったかどうか
  p_data->have  = 0;
}
