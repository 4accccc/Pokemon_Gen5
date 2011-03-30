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

