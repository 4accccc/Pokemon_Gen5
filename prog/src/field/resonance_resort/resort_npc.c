//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_npc.c
 *	@brief  NPC管理システム
 *	@author		Toru=Nagihashi
 *	@data		2011.04.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
//mine
#include "resort_map.h"
#include "resort_data_manager.h"
//outer
#include "../../../../resource/fldmapdata/script/resort_scr_def.h"
#include "resort_npc.h"
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
//-------------------------------------
///	NPC管理ワーク
//=====================================
struct _RESORT_NPC_SYS
{
  RESORT_BINARY_NPC   *p_binary_npc;
  RESORT_BINARY_OFFRECO *p_binary_offreco;
  RESORT_BINARY_MOUTHCOMM *p_binary_mouthcomm;
  RESORT_NPC_SYS_PARAM param;
};
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void Resort_Npc_Sys_AddAll( RESORT_NPC_SYS *p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  NPC初期化
 *
 *	@param	const RESORT_NPC_SYS_PARAM *cp_param  引数
 *	@param	heapID  ヒープID
 *
 *	@return　NPC管理ワーク
 */
//-----------------------------------------------------------------------------
RESORT_NPC_SYS * RESORT_NPC_SYS_Init( const RESORT_NPC_SYS_PARAM *cp_param, HEAPID heapID )
{
  RESORT_NPC_SYS *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_NPC_SYS) );
  p_wk->param = *cp_param;
  p_wk->p_binary_npc        = RESORT_BINARY_NPC_Init( heapID );
  p_wk->p_binary_offreco    = RESORT_BINARY_OFFRECO_Init( heapID );
  p_wk->p_binary_mouthcomm  = RESORT_BINARY_MOUTHCOMM_Init( heapID );

  Resort_Npc_Sys_AddAll( p_wk );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  NPC破棄
 *
 *	@param	RESORT_NPC_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_NPC_SYS_Exit( RESORT_NPC_SYS *p_wk )
{
  RESORT_BINARY_MOUTHCOMM_Exit( p_wk->p_binary_mouthcomm );
  RESORT_BINARY_OFFRECO_Exit( p_wk->p_binary_offreco );
  RESORT_BINARY_NPC_Exit( p_wk->p_binary_npc );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  NPCメイン
 *
 *	@param	RESORT_NPC_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_NPC_SYS_Main( RESORT_NPC_SYS *p_wk )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  NPCタイプを取得
 *
 *	@param	const RESORT_NPC_SYS *cp_wk ワーク
 *	@param	index   インデックス
 *
 *	@return タイプ
 */
//-----------------------------------------------------------------------------
u16 RESORT_NPC_SYS_GetType( const RESORT_NPC_SYS *cp_wk, u32 index )
{
  return RESORT_BINARY_NPC_GetData( cp_wk->p_binary_npc, index, RESORT_BINARY_NPC_PARAM_TYPE );
}

//----------------------------------------------------------------------------
/**
 *	@brief　クチコミデータ取得
 *
 *	@param	const RESORT_NPC_SYS *cp_wk ワーク
 *	@param	index インデックス
 *	@param	param   引数
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u16 RESORT_NPC_SYS_GetMoutchCommData( const RESORT_NPC_SYS *cp_wk, u32 index, RESORT_BINARY_MOUTHCOMM_PARAM param )
{
  u16 kind = RESORT_BINARY_NPC_GetData( cp_wk->p_binary_npc, index, RESORT_BINARY_NPC_PARAM_KIND );
  GF_ASSERT( RESORT_BINARY_NPC_GetData( cp_wk->p_binary_npc, index, RESORT_BINARY_NPC_PARAM_TYPE ) == RESORT_NPC_TYPE_MOUTHCOMM );
  return RESORT_BINARY_MOUTHCOMM_GetData( cp_wk->p_binary_mouthcomm, kind, param );
}
//----------------------------------------------------------------------------
/**
 *	@brief  オフレコデータ取得
 *
 *	@param	const RESORT_NPC_SYS *cp_wk ワーク
 *	@param	index インデックス
 *	@param	param   引数
 *
 *	@return　データ
 */
//-----------------------------------------------------------------------------
u16 RESORT_NPC_SYS_GetOffRecoData( const RESORT_NPC_SYS *cp_wk, u32 index, RESORT_BINARY_OFFRECO_PARAM param )
{
  u16 kind = RESORT_BINARY_NPC_GetData( cp_wk->p_binary_npc, index, RESORT_BINARY_NPC_PARAM_KIND );
  GF_ASSERT( RESORT_BINARY_NPC_GetData( cp_wk->p_binary_npc, index, RESORT_BINARY_NPC_PARAM_TYPE ) == RESORT_NPC_TYPE_OFFRECO );
  return RESORT_BINARY_OFFRECO_GetData( cp_wk->p_binary_offreco, kind, param );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NPCを生成
 *
 *	@param	RESORT_NPC_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_Npc_Sys_AddAll( RESORT_NPC_SYS *p_wk )
{
  int i;
  
  if( p_wk->param.placeID != RESORT_PLACEID_FIELD )
  {
    return;
  }

  for( i = 0; i < RESORT_BINARY_NPC_GetMax( p_wk->p_binary_npc ); i++ )
  {
    u16 city_lv  = RESORT_CITY_DATA_GetData(p_wk->param.p_city, RESORT_CITY_DATA_PARAM_LV, NULL);
    u16 apper_lv = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc, i, RESORT_BINARY_NPC_PARAM_CITY_LV);

    MMDL *p_mmdl = MMDLSYS_SearchOBJID( p_wk->param.p_mmdlsys, RESORT_NPC_INDEX_TO_MMDLID(i) );
    if( apper_lv >= city_lv && p_mmdl == NULL )
    {
      MMDL_HEADER header;
      u16 grid_x, grid_z;

      //もうMMDLがある場合は出さない

      GFL_STD_MemClear( &header, sizeof(MMDL_HEADER) );
      header.id = RESORT_NPC_INDEX_TO_MMDLID(i);
      header.obj_code = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc, i, RESORT_BINARY_NPC_PARAM_VIEW);
      header.move_code = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc, i, RESORT_BINARY_NPC_PARAM_MOVE_CODE);
      switch(RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc, i, RESORT_BINARY_NPC_PARAM_TYPE))
      {
      case RESORT_NPC_TYPE_OFFRECO:
        GF_ASSERT(0); //TODO　まだできてない
        header.event_id = 0;
        break;
      case RESORT_NPC_TYPE_MOUTHCOMM:
        header.event_id = SCRID_RESORT_NPC_MOUTHCOMM;
        break;
      default:
        GF_ASSERT(0);
      };
      header.move_limit_x = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc,i,RESORT_BINARY_NPC_PARAM_LIMIT_X);
      header.move_limit_z = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc,i,RESORT_BINARY_NPC_PARAM_LIMIT_Z);
      grid_x  = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc,i,RESORT_BINARY_NPC_PARAM_GRID_X);
      grid_z  = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc,i,RESORT_BINARY_NPC_PARAM_GRID_Z);

      MMDLHEADER_SetGridPos( &header, grid_x, grid_z, 0 );
      MMDLSYS_AddMMdl( p_wk->param.p_mmdlsys, &header, RESORT_ZONE_ID_FIELD );
    }
  }
}
