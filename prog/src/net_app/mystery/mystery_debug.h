//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mystery_debug.h
 *	@brief  ふしぎ用デバッグ
 *	@author	Toru=Nagihashi
 *	@date		2009.12.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef PM_DEBUG

#define DEBUG_SET_SAVEDATA


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
//=============================================================================
/**
 *    DEBUG
 */
//=============================================================================
#ifdef DEBUG_SET_SAVEDATA
#include "poke_tool/monsno_def.h"
#include "savedata/mystery_data.h"
static inline void MYSTERY_DEBUG_SetGiftData( DOWNLOAD_GIFT_DATA *p_data )
{ 
  GFL_STD_MemClear( p_data, sizeof(DOWNLOAD_GIFT_DATA) );
  
  //ポケモンデータ
  p_data->data.data.pokemon.mons_no     = MONSNO_MYUU;
  p_data->data.data.pokemon.form_no     = 0;
  p_data->data.data.pokemon.oyaname[0]  = L'デ';
  p_data->data.data.pokemon.oyaname[1]  = L'バ';
  p_data->data.data.pokemon.oyaname[2]  = L'ッ';
  p_data->data.data.pokemon.oyaname[3]  = L'グ';
  p_data->data.data.pokemon.oyaname[4]  = 0xffff;


  //イベントタイトル
  p_data->data.event_name[ 0] = L'き';
  p_data->data.event_name[ 1] = L'ん';
  p_data->data.event_name[ 2] = L'ぎ';
  p_data->data.event_name[ 3] = L'ん';
  p_data->data.event_name[ 4] = L'１';
  p_data->data.event_name[ 5] = L'０';
  p_data->data.event_name[ 6] = L'し';
  p_data->data.event_name[ 7] = L'ゅ';
  p_data->data.event_name[ 8] = L'う';
  p_data->data.event_name[ 9] = L'ね';
  p_data->data.event_name[10] = L'ん';
  p_data->data.event_name[11] = L'　';
  p_data->data.event_name[12] = L'ミ';
  p_data->data.event_name[13] = L'ュ';
  p_data->data.event_name[14] = L'ウ';
  p_data->data.event_name[15] = 0xffff;

  //受信した時間
  p_data->data.recv_date      = (2010<<MYSTERYGIFT_DATE_SHIFT_YEAR) | (12<<MYSTERYGIFT_DATE_SHIFT_MONTH) | (30<<MYSTERYGIFT_DATE_SHIFT_DAY);

  //イベントID
  p_data->data.event_id       = 1;

  //データ
  p_data->data.gift_type      = MYSTERYGIFT_TYPE_POKEMON;
  p_data->data.card_message   = 4;
  
  //一度だけ受信フラグ
  p_data->data.only_one_flag  = 1;

  //受け取ったかどうか
  p_data->data.have  = 0;

  //対象バージョン
  p_data->version    = 0;

  //説明テキスト
  {
    p_data->event_text[ 0] = L'ポ';
    p_data->event_text[ 1] = L'ケ';
    p_data->event_text[ 2] = L'モ';
    p_data->event_text[ 3] = L'ン';
    p_data->event_text[ 4] = L'き';
    p_data->event_text[ 5] = L'ん';
    p_data->event_text[ 6] = L'ぎ';
    p_data->event_text[ 7] = L'ん';
    p_data->event_text[ 8] = L'　';
    p_data->event_text[ 9] = L'１';
    p_data->event_text[10] = L'０';
    p_data->event_text[11] = L'し';
    p_data->event_text[12] = L'ゅ';
    p_data->event_text[13] = L'う';
    p_data->event_text[14] = L'ね';
    p_data->event_text[15] = L'ん';
    p_data->event_text[16] = L'を';
    p_data->event_text[17] = 0xfffe;
    p_data->event_text[18] = L'い';
    p_data->event_text[19] = L'わ';
    p_data->event_text[20] = L'っ';
    p_data->event_text[21] = L'て';
    p_data->event_text[22] = L'　';
    p_data->event_text[23] = L'ま';
    p_data->event_text[24] = L'ぼ';
    p_data->event_text[25] = L'ろ';
    p_data->event_text[26] = L'し';
    p_data->event_text[27] = L'の';
    p_data->event_text[28] = L'ポ';
    p_data->event_text[29] = L'ケ';
    p_data->event_text[30] = L'モ';
    p_data->event_text[31] = L'ン';
    p_data->event_text[32] = 0xfffe;
    p_data->event_text[33] = L'ミ';
    p_data->event_text[34] = L'ュ';
    p_data->event_text[35] = L'ウ';
    p_data->event_text[36] = L'を';
    p_data->event_text[37] = L'　';
    p_data->event_text[38] = L'プ';
    p_data->event_text[39] = L'レ';
    p_data->event_text[40] = L'ゼ';
    p_data->event_text[41] = L'ン';
    p_data->event_text[42] = L'ト';
    p_data->event_text[43] = L'！';
    p_data->event_text[44] = 0xffff;
  }
}
#endif //DEBUG_SET_SAVEDATA
#endif //PM_DEBUG
