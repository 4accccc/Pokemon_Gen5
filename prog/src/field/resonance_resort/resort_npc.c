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
#include "../fieldmap.h"
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
  BOOL  is_pause;
  u32   npc_num;
};
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void Resort_Npc_Sys_AddAll( RESORT_NPC_SYS *p_wk );
static void Resort_Npc_Sys_PauseAll( RESORT_NPC_SYS *p_wk, BOOL is_pause );
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

  GF_ASSERT( cp_param->max < RESORT_NPC_MAX_CREATE_NUM );
  {
    RESORT_NPC_SYS *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_NPC_SYS) );
    p_wk->param = *cp_param;
    p_wk->p_binary_npc        = cp_param->p_binary_npc;
    p_wk->p_binary_offreco    = NULL;
    p_wk->p_binary_mouthcomm  = RESORT_BINARY_MOUTHCOMM_Init( heapID );

    Resort_Npc_Sys_AddAll( p_wk );

    return p_wk;
  }
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
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  NPCメイン
 *
 *	@param	RESORT_NPC_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_NPC_SYS_Main( RESORT_NPC_SYS *p_wk, const FIELDMAP_WORK* cp_fieldmap )
{
  //if( FIELDMAP_CheckDoEvent( cp_fieldmap ) )
  if( 0 )
  {
    if( !p_wk->is_pause )
    {
      Resort_Npc_Sys_PauseAll( p_wk, TRUE );
      p_wk->is_pause = TRUE;
    }
  }
  else
  {
    if( p_wk->is_pause )
    {
      Resort_Npc_Sys_PauseAll( p_wk, FALSE );
      p_wk->is_pause = FALSE;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  MMDLを取得
 *
 *	@param	RESORT_NPC_SYS *p_wk  ワーク
 *
 *	@return MMDL
 */
//-----------------------------------------------------------------------------
MMDL * RESORT_NPC_SYS_GetMmdl( RESORT_NPC_SYS *p_wk, u32 index )
{
  int i;
  for( i = 0; i < RESORT_BINARY_NPC_GetMax( p_wk->p_binary_npc ); i++ )
  {
    MMDL *p_mmdl = MMDLSYS_SearchOBJID( p_wk->param.p_mmdlsys, RESORT_NPC_INDEX_TO_MMDLID(i) );
    if( p_mmdl != NULL && i == index )
    {
      return p_mmdl;
    }
  }

  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  NPCの数を取得
 *
 *	@param	const RESORT_NPC_SYS *cp_wk   ワーク
 *
 *	@return 最大数
 */
//-----------------------------------------------------------------------------
u32 RESORT_NPS_SYS_GetMax( const RESORT_NPC_SYS *cp_wk )
{
  return cp_wk->npc_num;
}

//----------------------------------------------------------------------------
/**
 *	@brief  全MMDL表示設定
 *
 *	@param	RESORT_NPC_SYS *p_wk  ワーク
 *	@param	is_visible  TRUE表示  FALSE非表示
 */
//-----------------------------------------------------------------------------
void RESORT_NPC_SYS_SetVisibleAll( RESORT_NPC_SYS *p_wk, BOOL is_visible )
{
  u32 no = 0;
  MMDL *p_mmdl;

  while( MMDLSYS_SearchUseMMdl(p_wk->param.p_mmdlsys,&p_mmdl,&no) == TRUE )
  {
    if( RESORT_NPC_TOOL_IsObjID( MMDL_GetOBJID(p_mmdl) ) 
        && MMDL_GetZoneID(p_mmdl) == RESORT_MAP_PlaceIDToZoneID(p_wk->param.placeID) )
    {
      MMDL_SetStatusBitVanish( p_mmdl, !is_visible );
    }
  }
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
  return 0; 
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

  for( i = 0; i < RESORT_BINARY_NPC_GetMax( p_wk->p_binary_npc ); i++ )
  {
    u16 city_lv  = RESORT_CITY_DATA_GetData(p_wk->param.p_city, RESORT_CITY_DATA_PARAM_LV, NULL);
    u16 apper_lv = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc, i, RESORT_BINARY_NPC_PARAM_CITY_LV);
    u16 placeID = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc, i, RESORT_BINARY_NPC_PARAM_PLACE);

    MMDL *p_mmdl = MMDLSYS_SearchOBJID( p_wk->param.p_mmdlsys, RESORT_NPC_INDEX_TO_MMDLID(i) );
    if( apper_lv <= city_lv && p_mmdl == NULL && p_wk->param.placeID == placeID )
    {
      MMDL_HEADER header;
      u16 grid_x, grid_z;

      //もうMMDLがある場合は出さない

      GFL_STD_MemClear( &header, sizeof(MMDL_HEADER) );
      header.id = RESORT_NPC_INDEX_TO_MMDLID(i);

      GF_ASSERT( RESORT_NPC_TOOL_IsObjID(header.id) );

      header.obj_code = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc, i, RESORT_BINARY_NPC_PARAM_VIEW);
      header.move_code = RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc, i, RESORT_BINARY_NPC_PARAM_MOVE_CODE);
      header.dir = DIR_DOWN;
      switch(RESORT_BINARY_NPC_GetData(p_wk->p_binary_npc, i, RESORT_BINARY_NPC_PARAM_TYPE))
      {
      case RESORT_NPC_TYPE_OFFRECO:
        header.event_id = SCRID_RESORT_NPC_OFFRECO;
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
      p_wk->npc_num++;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  NPCを全部ポーズさせる
 *
 *	@param	RESORT_NPC_SYS *p_wk  ワーク
 *	@param	is_pause  TRUEポーズ  FALSEなし
 */
//-----------------------------------------------------------------------------
static void Resort_Npc_Sys_PauseAll( RESORT_NPC_SYS *p_wk, BOOL is_pause )
{
  u32 no = 0;
  MMDL *p_mmdl;
  while( MMDLSYS_SearchUseMMdl(p_wk->param.p_mmdlsys,&p_mmdl,&no) == TRUE )
  {
    if( RESORT_NPC_TOOL_IsObjID( MMDL_GetOBJID(p_mmdl) )
        && MMDL_GetZoneID(p_mmdl) == RESORT_MAP_PlaceIDToZoneID(p_wk->param.placeID) )
    {
      if( is_pause )
      {
        MMDL_OnMoveBitMoveProcPause( p_mmdl );
      }
      else
      {
        MMDL_OffMoveBitMoveProcPause( p_mmdl );
      }
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  オフレコの最大数取得
 *
 *	@param	const RESORT_NPC_SYS *cp_wk   ワーク
 *
 *	@return オフレコ最大数
 */
//-----------------------------------------------------------------------------
u32 RESORT_NPC_SYS_GetOffRecoMax( const RESORT_NPC_SYS *cp_wk )
{
  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  引数を取得
 *
 *	@param	const RESORT_NPC_SYS *cp_wk ワーク
 *	@param	index   インデックス
 *
 *	@return 引数
 */
//-----------------------------------------------------------------------------
u32 RESORT_NPC_SYS_GetParam( const RESORT_NPC_SYS *cp_wk, u32 index )
{
  return RESORT_BINARY_NPC_GetData( cp_wk->p_binary_npc, index, RESORT_BINARY_NPC_PARAM_KIND );
}

//----------------------------------------------------------------------------
/**
 *	@brief  グループ数を計算
 *
 *	@param	const RESORT_NPC_SYS *cp_wk   ワーク
 *
 *	@return グループ数
 */
//-----------------------------------------------------------------------------
u32 RESORT_NPC_SYS_CalcOffrecoGroupMax( const RESORT_NPC_SYS *cp_wk )
{
  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  タイプからインデックスを取得
 *
 *	@param	const RESORT_NPC_SYS *cp_wk ワーク
 *	@param	type  タイプ
 *
 *	@return インデックス
 */
//-----------------------------------------------------------------------------
u32 RESORT_NPC_SYS_GetOffrecoIndexByType( const RESORT_NPC_SYS *cp_wk, u32 type )
{
  return 0;
}


//----------------------------------------------------------------------------
/**
 *	@brief  OBJIDがNPCかチェック
 *
 *	@param	int objid   チェックするOBJID
 *
 *	@return TRUE NPCのOBJID FALSE違う
 */
//-----------------------------------------------------------------------------
BOOL RESORT_NPC_TOOL_IsObjID( int objid )
{
  return RESORT_NPC_MMDLID_OFS_S <= objid && objid  < RESORT_NPC_MMDLID_OFS_E;
}
