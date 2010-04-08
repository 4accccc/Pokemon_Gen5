//======================================================================
/**
 * @file  field_mystery_gift.c
 * @brief ふしぎデータへのアクセスラッパー
 * @date  2010.02.14
 * @author  tamada GAMEFREAK inc.
 */
//======================================================================

#include <gflib.h>

#include "savedata/mystery_data.h"

#include "field_mystery_gift.h"

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief
 *
 * @param fd
 * @param index
 * @param gpd   贈り物データを受け取るためのワーク
 *
 * @retval  gpd   利用可能な場合
 * @retval  NULL  利用可能でない場合
 */
//--------------------------------------------------------------
static GIFT_PACK_DATA * getEnableGiftData( MYSTERY_DATA * fd, int index, GIFT_PACK_DATA * gpd )
{
  if ( MYSTERYDATA_IsExistsCard( fd, index ) == FALSE ) return NULL;
  if ( MYSTERYDATA_IsHavePresent( fd, index ) == TRUE ) return NULL;

  if( FALSE == MYSTERYDATA_GetCardData( fd, index, gpd ) ){
    return NULL;
  }
  if ( gpd->gift_type == MYSTERYGIFT_TYPE_NONE ) return NULL;
  //今のところ対応するデータはGPowerまで
  if ( gpd->gift_type > MYSTERYGIFT_TYPE_POWER ) return NULL;

  return gpd;
}

//--------------------------------------------------------------
/**
 * @brief 利用可能な贈り物データの取得
 *
 * @param   fd      不思議セーブデータへのポインタ
 * @param   index   贈り物データ位置のインデックスを受け取るためのポインタ
 * @param   gpd     贈り物データを受け取るためのワーク
 *
 * @retval    gpd     利用可能な場合
 * @retval    NULL    利用可能でない場合
 */
//--------------------------------------------------------------
GIFT_PACK_DATA * FIELD_MYSTERYDATA_GetGiftData( MYSTERY_DATA * fd, int * index, GIFT_PACK_DATA * gpd )
{
  int i;
  for (i = 0; i < GIFT_DATA_MAX; i ++ )
  {
    if (getEnableGiftData( fd, i, gpd ) != NULL)
    {
      *index = i;
      return gpd;
    }
  }
  return NULL;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void FIELD_MYSTERYDATA_SetReceived( MYSTERY_DATA * fd, int index )
{
  MYSTERYDATA_SetHavePresent( fd, index );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
u8 FIELD_MYSTERYDATA_GetGiftType( MYSTERY_DATA * fd )
{
  int index;
  GIFT_PACK_DATA aGift;
  const GIFT_PACK_DATA * gpd = FIELD_MYSTERYDATA_GetGiftData( fd, &index, &aGift );
  if ( gpd == NULL ) return MYSTERYGIFT_TYPE_NONE;
  return gpd->gift_type;
}

