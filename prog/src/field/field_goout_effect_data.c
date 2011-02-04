//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_goout_effect_data.c
 *	@brief  フィールド室外表現のデータ操作
 *	@author	Toru=Nagihashi
 *	@date		2011.02.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "arc_def.h"
#include "field_goout_data.naix"
#include "field_goout_data.cdat"

#include "field_goout_effect_data.h"
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
typedef struct
{
  u32 zoneID;
  u16 combiID[PMSEASON_TOTAL][FIELD_GOOUT_TIMEZONE_MAX];
}FIELD_GOOUT_BINARY;
struct _FIELD_GOOUT_DATA
{
  u32 size;
  FIELD_GOOUT_BINARY  *p_binary;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static const FIELD_GOOUT_BINARY * Field_Goout_GetBinary( const FIELD_GOOUT_DATA *cp_wk, u32 index );
static const FIELD_GOOUT_BINARY * Field_Goout_GetBinaryByZoneID( const FIELD_GOOUT_DATA *cp_wk, ZONEID zoneID );

//----------------------------------------------------------------------------
/**
 *	@brief  室外表現データ  初期化
 *
 *	@param	HEAPID heapID ヒープ
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
FIELD_GOOUT_DATA *FIELD_GOOUT_DATA_Init( HEAPID heapID )
{
  FIELD_GOOUT_DATA *  p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof( FIELD_GOOUT_DATA ) );

  {
    p_wk->p_binary  = GFL_ARC_UTIL_LoadEx( ARCID_FIELD_GOOUT_DATA, NARC_field_goout_data_field_goout_data_dat, FALSE, heapID, &p_wk->size );
  }

#if 0
  {
    int i;
    NAGI_Printf( "室外表現データ数=%d\n",FIELD_GOOUT_DATA_GetIndexMax(p_wk) );
    for( i = 0; i < FIELD_GOOUT_DATA_GetIndexMax(p_wk); i++ )
    {
      int time, season;
      const FIELD_GOOUT_BINARY * cp_binary = Field_Goout_GetBinary( p_wk, i );
      NAGI_Printf( "○ゾーンID=%d\n", cp_binary->zoneID );
      for( season = 0; season < PMSEASON_TOTAL; season++ )
      {
        NAGI_Printf( "季節=%d\n", season );
        for( time = 0; time < FIELD_GOOUT_TIMEZONE_MAX; time++ )
        {
          NAGI_Printf( "演出=%d\n", cp_binary->combiID[season][time] );
        }
      }
    }
  }
#endif

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  室外表現データ  破棄
 *
 *	@param	FIELD_GOOUT_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_GOOUT_DATA_Exit( FIELD_GOOUT_DATA *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_binary );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンIDを取得
 *
 *	@param	const FIELD_GOOUT_DATA *cp_wk ワーク
 *	@param	index   インデックス
 *
 *	@return ゾーンID
 */
//-----------------------------------------------------------------------------
ZONEID FIELD_GOOUT_DATA_GetZoneID( const FIELD_GOOUT_DATA *cp_wk, u32 index )
{
  const FIELD_GOOUT_BINARY * cp_binary = Field_Goout_GetBinary( cp_wk, index );
  return cp_binary->zoneID;
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出組み合わせIDを取得
 *
 *	@param	const FIELD_GOOUT_DATA *cp_wk
 *	@param	index   インデックス
 *	@param	season  季節
 *	@param	timezone  タイムゾーン
 *
 *	@return 組み合わせID
 */
//-----------------------------------------------------------------------------
GOOUT_COMBI_ID FIELD_GOOUT_DATA_GetCombiID( const FIELD_GOOUT_DATA *cp_wk, u32 index, u8 season, FIELD_GOOUT_TIMEZONE timezone )
{
  const FIELD_GOOUT_BINARY * cp_binary = Field_Goout_GetBinary( cp_wk, index );
  GF_ASSERT( season < PMSEASON_TOTAL );
  GF_ASSERT( timezone < FIELD_GOOUT_TIMEZONE_MAX );
  return cp_binary->combiID[season][timezone];
}
//----------------------------------------------------------------------------
/**
 *	@brief  インデックスの最大を取得
 *
 *	@param	const FIELD_GOOUT_DATA *cp_wk   ワーク
 *
 *	@return インデックス最大
 */
//-----------------------------------------------------------------------------
u32 FIELD_GOOUT_DATA_GetIndexMax( const FIELD_GOOUT_DATA *cp_wk )
{
  return cp_wk->size / sizeof(FIELD_GOOUT_BINARY);
}

//----------------------------------------------------------------------------
/**
 *	@brief  演出組み合わせIDを取得  ゾーンIDから
 *
 *	@param	const FIELD_GOOUT_DATA *cp_wk
 *	@param	zoneID  ゾーンID
 *	@param	season  季節
 *	@param	timezone  タイムゾーン
 *
 *	@return 組み合わせID
 */
//-----------------------------------------------------------------------------
GOOUT_COMBI_ID FIELD_GOOUT_DATA_GetCombiIDByZoneID( const FIELD_GOOUT_DATA *cp_wk, ZONEID zoneID, u8 season, FIELD_GOOUT_TIMEZONE timezone )
{
  const FIELD_GOOUT_BINARY *cp_binary = Field_Goout_GetBinaryByZoneID( cp_wk, zoneID );
  GF_ASSERT( season < PMSEASON_TOTAL );
  GF_ASSERT( timezone < FIELD_GOOUT_TIMEZONE_MAX );
  
  if( cp_binary == NULL )
  {
    return GOOUT_COMBI_NONE;
  }
  else
  {
    return cp_binary->combiID[season][timezone];
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  組み合わせIDからエフェクトIDを取得
 *
 *	@param	const FIELD_GOOUT_DATA *cp_wk ワーク
 *	@param	combiID 組み合わせID
 *	@param	index   インデックス  ０～GOOUT_COMBI_EFFECT_MAX
 *
 *	@return エフェクトID（エフェクトなしのときGOOUT_EFF_NULL）
 */
//-----------------------------------------------------------------------------
GOOUT_EFFECT_ID FIELD_GOOUT_DATA_GetEffectID( const FIELD_GOOUT_DATA *cp_wk, GOOUT_COMBI_ID combiID, u32 index )
{
  GF_ASSERT( combiID < GOOUT_COMBI_MAX );
  GF_ASSERT( index < GOOUT_COMBI_EFFECT_MAX );
  return sc_goout_effect_conmbi[combiID][index];
}
//----------------------------------------------------------------------------
/**
 *	@brief  組み合わせIDにおける演出IDの数を取得
 *
 *	@param	const FIELD_GOOUT_DATA *cp_wk ワーク
 *	@param	combiID   組み合わせID
 *
 *	@return 演出の数
 */
//-----------------------------------------------------------------------------
u8 FIELD_GOOUT_DATA_GetEffectNum( const FIELD_GOOUT_DATA *cp_wk, GOOUT_COMBI_ID combiID )
{
  int i;
  for( i = 0; i < GOOUT_COMBI_EFFECT_MAX; i++ )
  {
    GOOUT_EFFECT_ID effectID  = FIELD_GOOUT_DATA_GetEffectID( cp_wk, combiID, i );
    if( effectID == GOOUT_EFF_NULL )
    {
      break;
    }
  }
  return i;
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出のリソースARCDATIDを取得
 *
 *	@param	const FIELD_GOOUT_DATA *cp_wk ワーク
 *	@param	effectID  演出ID
 *	@param	res リソース
 *
 *	@return リソースDATID
 */
//-----------------------------------------------------------------------------
ARCDATID FIELD_GOOUT_DATA_GetResource( const FIELD_GOOUT_DATA *cp_wk, GOOUT_EFFECT_ID effectID, FIELD_GOOUT_RESOURCE res )
{
  GF_ASSERT( effectID < GOOUT_EFF_MAX );
  switch( res )
  {
  case FIELD_GOOUT_RESOURCE_IMD:
    return sc_goout_effect_resource[ effectID ].imd;
  case FIELD_GOOUT_RESOURCE_ICA:
    return sc_goout_effect_resource[ effectID ].ica;
  case FIELD_GOOUT_RESOURCE_IMA:
    return sc_goout_effect_resource[ effectID ].ima;
  case FIELD_GOOUT_RESOURCE_ITA:
    return sc_goout_effect_resource[ effectID ].ita;
  case FIELD_GOOUT_RESOURCE_ITP:
    return sc_goout_effect_resource[ effectID ].itp;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バイナリデータ取得
 *
 *	@param	const FIELD_GOOUT_DATA *cp_wk ワーク
 *	@param	index   インデックス
 *
 *	@return バイナリデータ
 */
//-----------------------------------------------------------------------------
static const FIELD_GOOUT_BINARY * Field_Goout_GetBinary( const FIELD_GOOUT_DATA *cp_wk, u32 index )
{
  GF_ASSERT( index < FIELD_GOOUT_DATA_GetIndexMax( cp_wk ) );
  return &cp_wk->p_binary[index];
}
//----------------------------------------------------------------------------
/**
 *	@brief  バイナリデータ取得  ZONEIDから
 *
 *	@param	const FIELD_GOOUT_DATA *cp_wk ワーク
 *	@param	zoneID  ゾーンID
 *
 *	@return バイナリデータ  ない場合はNULL
 */
//-----------------------------------------------------------------------------
static const FIELD_GOOUT_BINARY * Field_Goout_GetBinaryByZoneID( const FIELD_GOOUT_DATA *cp_wk, ZONEID zoneID )
{
  int i;
  for( i = 0; i < FIELD_GOOUT_DATA_GetIndexMax(cp_wk); i++ )
  {
    const FIELD_GOOUT_BINARY *cp_binary = Field_Goout_GetBinary( cp_wk, i );
    if( cp_binary->zoneID == zoneID )
    {
      return cp_binary;
    }
  }

  return NULL;
}
