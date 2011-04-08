//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_binary.c
 *	@brief  レゾナンスリゾートバイナリデータ読み込み
 *	@author	Toru=Nagihashi
 *	@data		2011.02.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>

//resource
#include "arc_def.h"
#include "resonance_resort_data.naix"

//outer
#include "resort_binary.h"


#ifdef PM_DEBUG
#define DEBUG_DATA_PRINT
#endif //PM_DEBUG

//=============================================================================
/**
 *					レゾナンスリゾート人物バイナリデータ
*/
//=============================================================================
//-------------------------------------
///	データ
//=====================================
typedef enum
{
  RESORT_BINARY_PEOPLE_PARAM_PEOPLE_TYPE,
  RESORT_BINARY_PEOPLE_PARAM_KIND,
  RESORT_BINARY_PEOPLE_PARAM_PERSONALITY,
  RESORT_BINARY_PEOPLE_PARAM_VIEW,
  RESORT_BINARY_PEOPLE_PARAM_SEX,
  RESORT_BINARY_PEOPLE_PARAM_SCRID,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_00,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_01,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_02,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_03,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_04,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_05,
  RESORT_BINARY_PEOPLE_PARAM_MAX,
}RESORT_BINARY_PEOPLE_PARAM;

//-------------------------------------
///	人物バイナリの内容
//=====================================
struct _RESORT_BINARY_PEOPLE_DATA
{
  u16 param[RESORT_BINARY_PEOPLE_PARAM_MAX];
} ;


//-------------------------------------
///	人物バイナリデータワーク
//=====================================
struct _RESORT_BINARY_PEOPLE
{
  u32   size;
  RESORT_BINARY_PEOPLE_DATA  *p_data;
};

//-------------------------------------
/// PRIVATE
//=====================================
//-------------------------------------
/// PUBLIC	
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  バイナリ読み込み
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@returnデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_PEOPLE * RESORT_BINARY_PEOPLE_Init( HEAPID heapID )
{
  RESORT_BINARY_PEOPLE *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_BINARY_PEOPLE) );

  p_wk->p_data = GFL_ARC_UTIL_LoadEx(ARCID_RESONANCE_RESORT_DATA, NARC_resonance_resort_data_resort_people_dat, FALSE, heapID, &p_wk->size );

#ifdef DEBUG_DATA_PRINT
  {
    int i;
    for( i = 0; i < (p_wk->size / sizeof(RESORT_BINARY_PEOPLE_DATA)); i++ )
    {
      const RESORT_BINARY_PEOPLE_DATA* cp_data = &p_wk->p_data[i];
      OS_Printf( "type%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_PEOPLE_TYPE ] );
      OS_Printf( "kind%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_KIND        ] );
      OS_Printf( "pers%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_PERSONALITY ] );
      OS_Printf( "view%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_VIEW        ] );
      OS_Printf( "sex %d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_SEX         ] );
      OS_Printf( "scri%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_SCRID       ] );
      OS_Printf( "msg0%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_MSGID_00    ] );
      OS_Printf( "msg1%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_MSGID_01    ] );
      OS_Printf( "msg2%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_MSGID_02    ] );
      OS_Printf( "msg3%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_MSGID_03    ] );
      OS_Printf( "msg4%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_MSGID_04    ] );
      OS_Printf( "msg5%d\n", cp_data->param[ RESORT_BINARY_PEOPLE_PARAM_MSGID_05    ] );
    }
  }
#endif //DEBUG_DATA_PRINT

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バイナリ破棄
 *
 *	@param	RESORT_BINARY_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_BINARY_PEOPLE_Exit( RESORT_BINARY_PEOPLE *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトIDを取得
 *
 *	@param	const RESORT_BINARY_PEOPLE_DATA *cp_wk　 ワーク
 *
 *	@return スクリプトID
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_PEOPLE_DATA_GetScrID( const RESORT_BINARY_PEOPLE_DATA *cp_wk )
{
  return cp_wk->param[ RESORT_BINARY_PEOPLE_PARAM_SCRID ];
}

//----------------------------------------------------------------------------
/**
 *	@brief　メッセージIDを取得
 *
 *	@param	const RESORT_BINARY_PEOPLE_DATA *cp_wk　ワーク
 *	@param	param 引数
 *
 *	@return　メッセージID
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_PEOPLE_DATA_GetMsgID( const RESORT_BINARY_PEOPLE_DATA *cp_wk, u16 param )
{
  u16 index = RESORT_BINARY_PEOPLE_PARAM_MSGID_00 + param;
  return cp_wk->param[ index ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物タイプと種類からバイナリ取得
 *
 *	@param	const RESORT_BINARY_PEOPLE *cp_wkワーク
 *	@param	people_type　人物タイプ
 *	@param	kind 　種類
 *	@param  
 *
 *	@return　バイナリデータ
 */
//-----------------------------------------------------------------------------
const RESORT_BINARY_PEOPLE_DATA * RESORT_BINARY_SearchData( const RESORT_BINARY_PEOPLE *cp_wk, RESORT_PEOPLE_TYPE people_type, RESORT_KIND_TYPE kind )
{
  int i;
  const RESORT_BINARY_PEOPLE_DATA *cp_data;
  for( i = 0; i < cp_wk->size/sizeof(RESORT_BINARY_PEOPLE_DATA); i++ )
  {
    cp_data   = &cp_wk->p_data[i];
    if( cp_data->param[RESORT_BINARY_PEOPLE_PARAM_PEOPLE_TYPE] == people_type
        && cp_data->param[RESORT_BINARY_PEOPLE_PARAM_KIND] == kind)
    {
      return cp_data;
    }
  }

  GF_ASSERT_MSG(0, "people_type%d kind%d\n", people_type, kind);
  return NULL;
}

//=============================================================================
/**
 *					レゾナンスリゾートアピール情報バイナリデータ
*/
//=============================================================================
//-------------------------------------
///	アピールデータ
//=====================================
typedef struct
{
  u16 param[ RESORT_BINARY_SPOT_PARAM_MAX ];
} RESORT_BINARY_SPOT_DATA;

//-------------------------------------
///	アピールデータ
//=====================================
struct _RESORT_BINARY_SPOT
{
  u32 size;
  RESORT_BINARY_SPOT_DATA *p_data;
};

//-------------------------------------
/// PUBLIC	
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  データ読み込み
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_SPOT * RESORT_BINARY_SPOT_Init( HEAPID heapID )
{
  RESORT_BINARY_SPOT *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_BINARY_SPOT) );
  p_wk->p_data = GFL_ARC_UTIL_LoadEx(ARCID_RESONANCE_RESORT_DATA, NARC_resonance_resort_data_resort_spot_dat, FALSE, heapID, &p_wk->size );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@briefデータ破棄
 *
 *	@param	RESORT_BINARY_SPOT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_BINARY_SPOT_Exit( RESORT_BINARY_SPOT *p_wk )
{
  GFL_HEAP_FreeMemory(p_wk->p_data);
  GFL_HEAP_FreeMemory(p_wk);
}

//----------------------------------------------------------------------------
/**
 *	@brief データ取得
 *
 *	@param	const RESORT_BINARY_SPOT *cp_wk ワーク
 *	@param	index インデックス
 *	@param	param  パラメータ
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_SPOT_GetData( const RESORT_BINARY_SPOT *cp_wk, u32 index, RESORT_BINARY_SPOT_PARAM param )
{
  GF_ASSERT( index < cp_wk->size / sizeof( RESORT_BINARY_SPOT_DATA ) );
  GF_ASSERT( param < RESORT_BINARY_SPOT_PARAM_MAX );
  return cp_wk->p_data[ index ].param[ param ];
}
//----------------------------------------------------------------------------
/**
 *	@brief　データ取得（メッセージから）
 *
 *	@param	const RESORT_BINARY_SPOT *cp_wk ワーク
 *	@param	msg メッセージ
 *	@param	p_index インデックス
 *
 *	@return   TRUE …あった FALSE …なかった
 */
//-----------------------------------------------------------------------------
BOOL RESORT_BINARY_SPOT_SearchIndex( const RESORT_BINARY_SPOT *cp_wk, u32 msg, u32 *p_index )
{
  for( (*p_index) = 0; (*p_index) < cp_wk->size / sizeof( RESORT_BINARY_SPOT_DATA ); (*p_index)++ )
  {
    if( cp_wk->p_data[ (*p_index) ].param[ RESORT_BINARY_SPOT_PARAM_MSG_00 ] == msg
      || cp_wk->p_data[ (*p_index) ].param[ RESORT_BINARY_SPOT_PARAM_MSG_01 ] == msg 
      || cp_wk->p_data[ (*p_index) ].param[ RESORT_BINARY_SPOT_PARAM_MSG_02 ] == msg 
      )
    {
      return TRUE;
    }
  }

  return FALSE;
}

//=============================================================================
/**
 *      レゾナンスリゾートNPCデータ
 */
//=============================================================================
//-------------------------------------
///	NPCバイナリデータ
//=====================================
typedef struct
{
  u16 param[ RESORT_BINARY_NPC_PARAM_MAX ];
} RESORT_BINARY_NPC_DATA;
//-------------------------------------
///	ワーク
//=====================================
struct _RESORT_BINARY_NPC
{
  u32 size;
  RESORT_BINARY_NPC_DATA *p_data;
};

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  NPCデータを作成
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return バイナリデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_NPC * RESORT_BINARY_NPC_Init( HEAPID heapID )
{
  RESORT_BINARY_NPC *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_BINARY_NPC) );
  p_wk->p_data = GFL_ARC_UTIL_LoadEx(ARCID_RESONANCE_RESORT_DATA, NARC_resonance_resort_data_resort_npc_dat, FALSE, heapID, &p_wk->size );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  NPCデータ
 *
 *	@param	RESORT_BINARY_NPC *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_BINARY_NPC_Exit( RESORT_BINARY_NPC *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを取得
 *
 *	@param	RESORT_BINARY_NPC *p_wk ワーク
 *	@param	index   インデックス
 *	@param	param   引数
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_NPC_GetData( const RESORT_BINARY_NPC *cp_wk, u32 index, RESORT_BINARY_NPC_PARAM param )
{
  return cp_wk->p_data[ index ].param[ param ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ最大値
 *
 *	@param	const RESORT_BINARY_NPC *cp_wk  ワーク
 *
 *	@return 最大
 */
//-----------------------------------------------------------------------------
u32 RESORT_BINARY_NPC_GetMax( const RESORT_BINARY_NPC *cp_wk )
{
  return cp_wk->size / sizeof(RESORT_BINARY_NPC_DATA);
}

//=============================================================================
/**
 *      レゾナンスリゾートMOUTHCOMMデータ
 */
//=============================================================================
//-------------------------------------
///	MOUTHCOMMバイナリデータ
//=====================================
typedef struct
{
  u16 param[ RESORT_BINARY_MOUTHCOMM_PARAM_MAX ];
} RESORT_BINARY_MOUTHCOMM_DATA;
//-------------------------------------
///	ワーク
//=====================================
struct _RESORT_BINARY_MOUTHCOMM
{
  u32 size;
  RESORT_BINARY_MOUTHCOMM_DATA *p_data;
};

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  MOUTHCOMMデータを作成
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return バイナリデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_MOUTHCOMM * RESORT_BINARY_MOUTHCOMM_Init( HEAPID heapID )
{
  RESORT_BINARY_MOUTHCOMM *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_BINARY_MOUTHCOMM) );
  p_wk->p_data = GFL_ARC_UTIL_LoadEx(ARCID_RESONANCE_RESORT_DATA, NARC_resonance_resort_data_resort_kutikomi_dat, FALSE, heapID, &p_wk->size );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  MOUTHCOMMデータ
 *
 *	@param	RESORT_BINARY_MOUTHCOMM *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_BINARY_MOUTHCOMM_Exit( RESORT_BINARY_MOUTHCOMM *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを取得
 *
 *	@param	RESORT_BINARY_MOUTHCOMM *p_wk ワーク
 *	@param	index   インデックス
 *	@param	param   引数
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_MOUTHCOMM_GetData( const RESORT_BINARY_MOUTHCOMM *cp_wk, u32 index, RESORT_BINARY_MOUTHCOMM_PARAM param )
{
  return cp_wk->p_data[ index ].param[ param ];
}

//=============================================================================
/**
 *      レゾナンスリゾートOFFRECOデータ
 */
//=============================================================================
//-------------------------------------
///	OFFRECOバイナリデータ
//=====================================
typedef struct
{
  u16 param[ RESORT_BINARY_OFFRECO_PARAM_MAX ];
} RESORT_BINARY_OFFRECO_DATA;
//-------------------------------------
///	ワーク
//=====================================
struct _RESORT_BINARY_OFFRECO
{
  u32 size;
  RESORT_BINARY_OFFRECO_DATA *p_data;
};

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  OFFRECOデータを作成
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return バイナリデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_OFFRECO * RESORT_BINARY_OFFRECO_Init( HEAPID heapID )
{
  RESORT_BINARY_OFFRECO *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_BINARY_OFFRECO) );
  p_wk->p_data = GFL_ARC_UTIL_LoadEx(ARCID_RESONANCE_RESORT_DATA, NARC_resonance_resort_data_resort_offreco_dat, FALSE, heapID, &p_wk->size );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  OFFRECOデータ
 *
 *	@param	RESORT_BINARY_OFFRECO *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_BINARY_OFFRECO_Exit( RESORT_BINARY_OFFRECO *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを取得
 *
 *	@param	RESORT_BINARY_OFFRECO *p_wk ワーク
 *	@param	index   インデックス
 *	@param	param   引数
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u16 RESORT_BINARY_OFFRECO_GetData( const RESORT_BINARY_OFFRECO *cp_wk, u32 index, RESORT_BINARY_OFFRECO_PARAM param )
{
  return cp_wk->p_data[ index ].param[ param ];
}
