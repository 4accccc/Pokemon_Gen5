//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mystery_data_util.c
 *	@brief  ふしぎデータ便利関数
 *	@author	Toru=Nagihashi
 *	@data		2010.03.01
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム


//モジュール
#include "item/itemsym.h"
#include "field/gpower_id.h"

//自分のモジュール
#include "net_app/mystery.h"

//外部公開
#include "net_app/mystery_data_util.h"

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
 *	@brief  不正を見つけたら修正する関数
 *
 *	@param	DOWNLOAD_GIFT_DATA *p_data  修正するデータ
 *
 *	@return 不正カウンタ（不正無しなら０  内部の不正を見つけるたびに＋１）
 */
//-----------------------------------------------------------------------------
u32 MYSTERYDATA_ModifyDownloadData( DOWNLOAD_GIFT_DATA *p_data, GAMEDATA *p_gamedata, HEAPID heapID )
{ 
  u32 dirty = 0;

  //説明テキストが不正
  { 
    int i;
    BOOL is_darty = TRUE;
    //EOMが入っていれば正常とみなす
    for( i = 0; i < GIFT_DATA_CARD_TEXT_MAX+EOM_SIZE; i++ )
    { 
      if( p_data->event_text[i] == GFL_STR_GetEOMCode() )
      { 
        is_darty  = FALSE;
      }
    }

    if( is_darty )
    { 
      dirty++;
      p_data->event_text[0] = L'　';
      p_data->event_text[1] = GFL_STR_GetEOMCode();
      OS_TPrintf( "DOWNLOAD_GIFT_DATA:説明テキストが不正のため空文字にしました\n" );
    }
  }

  //付属データもチェック
  dirty += MYSTERYDATA_ModifyGiftData( &p_data->data, p_gamedata, heapID );

  return dirty;
}

//----------------------------------------------------------------------------
/**
 *	@brief  不正を見つけたら修正する関数
 *
 *	@param	GIFT_PACK_DATA *p_data  修正するデータ
 *
 *	@return 不正カウンタ（不正無しなら０  内部の不正を見つけるたびに＋１）
 */
//-----------------------------------------------------------------------------
u32 MYSTERYDATA_ModifyGiftData( GIFT_PACK_DATA *p_data, GAMEDATA *p_gamedata, HEAPID heapID )
{ 
  u32 dirty = 0;

  //イベントタイトルが不正
  { 
    int i;
    BOOL is_darty = TRUE;
    //EOMが入っていれば正常とみなす
    for( i = 0; i < GIFT_DATA_CARD_TITLE_MAX+EOM_SIZE; i++ )
    { 
      if( p_data->event_name[i] == GFL_STR_GetEOMCode() )
      { 
        is_darty  = FALSE;
      }
    }

    if( is_darty )
    { 
      dirty++;
      p_data->event_name[0] = L'　';
      p_data->event_name[1] = GFL_STR_GetEOMCode();
      OS_TPrintf( "GIFT_PACK_DATA:イベントタイトルが不正のため空文字にしました\n" );
    }
  }

  //イベントID
  { 
    if( !(0 <= p_data->event_id && p_data->event_id < MYSTERY_DATA_MAX_EVENT ) )
    { 
      p_data->event_id  = 0;
      dirty++;
      OS_TPrintf( "GIFT_PACK_DATA:イベントIDが不正のため０にしました\n" );
    }
  }

  //ギフトタイプチェック
  { 
    if( !(MYSTERYGIFT_TYPE_NONE < p_data->gift_type &&  p_data->gift_type < MYSTERYGIFT_TYPE_MAX) )
    { 
      p_data->gift_type = MYSTERYGIFT_TYPE_NONE;
      dirty++;
      OS_TPrintf( "GIFT_PACK_DATA:ギフトタイプが不正のため０にしました\n" );
    }
  }

  //中身チェック
  { 
    switch( p_data->gift_type )
    { 
    case MYSTERYGIFT_TYPE_POKEMON:
      { 
        GIFT_PRESENT_POKEMON  *p_poke = &p_data->data.pokemon;
        POKEMON_PARAM* pp = MYSTERY_CreatePokemon( p_data, heapID, p_gamedata );
        if( pp == NULL )
        { 
          dirty++;
          OS_TPrintf( "GIFT_PACK_DATA:ポケモンデータが不正でした\n" ); 
        }
        else
        { 
          GFL_HEAP_FreeMemory( pp );
        }
      }
      break;
    case MYSTERYGIFT_TYPE_ITEM:
      { 
        GIFT_PRESENT_ITEM *p_item = &p_data->data.item;
        if( !(0 <= p_item->itemNo && p_item->itemNo < ITEM_DATA_MAX) )
        {   
          p_item->itemNo  = ITEM_MONSUTAABOORU;
          dirty++;
          OS_TPrintf( "GIFT_PACK_DATA:アイテムNOが不正のためモンスターボールにしました\n" );
        }
      }
      break;
    case MYSTERYGIFT_TYPE_POWER:
      { 
        GIFT_PRESENT_POWER *p_power = &p_data->data.gpower; //@todo
        if( !( 0 <= p_power->type && p_power->type < GPOWER_ID_MAX) )
        { 
          dirty++;
          OS_TPrintf( "GIFT_PACK_DATA:Gパワーが不正でした\n" ); 
        }
      }
      break;
    }
  }

  return dirty;
}



