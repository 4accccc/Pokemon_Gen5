//======================================================================
/**
 * @file  field_mystery_gift.c
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
 * @param fd
 * @param index
 * @retval  GIFT_PACK_DATA  利用可能な贈り物データへのポインタ
 * @retval  NULL  利用可能でない場合
 */
//--------------------------------------------------------------
static GIFT_PACK_DATA * getEnableGiftData( MYSTERY_DATA * fd, int index )
{
  GIFT_PACK_DATA * gpd;
  if ( MYSTERYDATA_IsExistsCard( fd, index ) == FALSE ) return NULL;
  if ( MYSTERYDATA_IsHavePresent( fd, index ) == TRUE ) return NULL;
  gpd = MYSTERYDATA_GetCardData( fd, index );
  if ( gpd == NULL ) return NULL;
  if ( gpd->gift_type == MYSTERYGIFT_TYPE_NONE ) return NULL;
  //今のところ対応するデータはGPowerまで
  if ( gpd->gift_type > MYSTERYGIFT_TYPE_POWER ) return NULL;

  return gpd;
}

//--------------------------------------------------------------
/**
 * @brief
 * @param fd
 * @param index
 * @retval  GIFT_PACK_DATA
 * @retval  NULLL
 */
//--------------------------------------------------------------
GIFT_PACK_DATA * FIELD_MYSTERYDATA_GetGiftData( MYSTERY_DATA * fd, int * index )
{
  int i;
  GIFT_PACK_DATA * gpd;
  for (i = 0; i < GIFT_DATA_MAX; i ++ )
  {
    gpd = getEnableGiftData( fd, i );
    if (gpd != NULL)
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
  const GIFT_PACK_DATA * gpd = FIELD_MYSTERYDATA_GetGiftData( fd, &index );
  if ( gpd == NULL ) return MYSTERYGIFT_TYPE_NONE;
  return gpd->gift_type;
}

