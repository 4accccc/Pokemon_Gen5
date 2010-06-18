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
#include "mystery_inline.h"

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
      OS_TPrintf( "GIFT_PACK_DATA:イベントタイトルが不正でした\n" );
    }
  }

  //イベントID
  { 
    if( !(0 <= p_data->event_id && p_data->event_id < MYSTERY_DATA_MAX_EVENT ) )
    { 
      dirty++;
      OS_TPrintf( "GIFT_PACK_DATA:イベントIDが不正でした\n" );
    }
  }

  //ギフトタイプチェック
  { 
    if( !(MYSTERYGIFT_TYPE_NONE < p_data->gift_type &&  p_data->gift_type < MYSTERYGIFT_TYPE_MAX) )
    { 
      dirty++;
      OS_TPrintf( "GIFT_PACK_DATA:ギフトタイプが不正でした\n" );
    }
  }

  //中身チェック
  { 
    switch( p_data->gift_type )
    { 
    case MYSTERYGIFT_TYPE_POKEMON:
      {
        POKEMON_PARAM* pp = Mystery_CreatePokemon( p_data, heapID, p_gamedata );
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
        u16 itemNo  = Mystery_CreateItem( p_data );
        if( ITEM_DUMMY_DATA == itemNo  )
        {   
          dirty++;
          OS_TPrintf( "GIFT_PACK_DATA:アイテムNOが不正でした\n" );
        }
      }
      break;
    case MYSTERYGIFT_TYPE_POWER:
      { 
        u16 gpower  = Mystery_CreateGPower( p_data );
        if( gpower == GPOWER_ID_NULL )
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



