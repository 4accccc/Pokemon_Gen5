//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_habitat_info.c
 *	@brief  生息地情報取得
 *	@author	Masayuki Onoue
 *	@date		2011.09.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"
#include "system/bmp_winframe.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"

#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap/zone_id.h" // for ZONE_ID_*
#include "../../../../resource/fldmapdata/flagwork/flag_define.h" // for SYS_FLAG_ARRIVE_*

#include "savedata/habitat_list_save.h"
#include "../app/zukan/habitat_zone/habitat_zone_arc_data_def.h"
#include "../../../../resource/habitat_list/habitat_list_data.h" // for HABITAT_LIST_DATA_MAX
#include "../../../../resource/habitat_list/habitat_list_data.cdat" // for sc_habitat_list_data

#include "arc_def.h"          // for ARCID_*
#include "scrcmd_habitat_info.h"

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static u16 CompleteFlag( ARC_HABITAT_ZONE_DATA *p_arc_data ,ZUKAN_SAVEDATA *p_save,u8 ground_id ,BOOL is_get );

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンのコンプリート状態を取得
 *
 *	@param	*core
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdCheckZonePokeGetComplete( VMHANDLE *core, void *wk )
{
  s32 i;
  BOOL is_comp = FALSE;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  ARC_HABITAT_ZONE_DATA arc_data;
  GAMESYS_WORK *gamesys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gamesys);
  ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( gamedata );
  FIELDMAP_WORK *field = GAMESYSTEM_GetFieldMapWork(gamesys);
  const HEAPID heapID = FIELDMAP_GetHeapID( field );

  u16 zone_group_id   = SCRCMD_GetVMWorkValue( core, work );
  u16 ground_id       = SCRCMD_GetVMWorkValue( core, work );
  u16 is_get_check    = SCRCMD_GetVMWorkValue( core, work );
  u16 *p_ret_wk	      = SCRCMD_GetVMWork( core, work );
 
  // アーカイブハンドル取得
  ARCHANDLE	*p_handle	= 
      GFL_ARC_OpenDataHandle( ARCID_HABITAT_LIST_DATA , heapID );
 
  // データクリア
  GFL_STD_MemClear( &arc_data ,sizeof(ARC_HABITAT_ZONE_DATA) );
    
  // ゾーングループのデータを調べる
  for( i = 0; i < HABITAT_LIST_DATA_MAX ; i++ )
  {
    const HABITAT_LIST_DATA *sc_habitat = &sc_habitat_list_data[i];
    if( sc_habitat->zone_group_id == zone_group_id )
    {
      // アーカイブデータ取得
      GFL_ARC_LoadDataByHandle(p_handle, sc_habitat->arc_idx , &arc_data);
      (*p_ret_wk) = CompleteFlag( &arc_data ,zkn_sv ,ground_id ,is_get_check );
      is_comp = TRUE;
    }
  }

  GFL_ARC_CloseDataHandle( p_handle );
  
  GF_ASSERT(is_comp==TRUE);
  return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  モンスターのコンプリート状態を取得する
 *
 *	@param	*p_dest
 *	@param	*p_arc_data 
 *	@param	*p_save
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u16 CompleteFlag( ARC_HABITAT_ZONE_DATA *p_arc_data ,ZUKAN_SAVEDATA *p_save,u8 ground_id ,BOOL is_get )
{  
  s32 n;
  BOOL flg;
  u8 season_id;
  u8 is_land_appear[HABITAT_GROUND_MAX];  // その地形にポケモンが出現するか

  // 地形ごとに出現するかどうかをチェックする
  for( n = 0; n < p_arc_data->data_num ; n++ )
  {
    // 初期化
    GFL_STD_MemClear( is_land_appear ,sizeof(is_land_appear) );

    for( season_id = 0 ; season_id < PMSEASON_TOTAL; season_id++ )
    {
      is_land_appear[ground_id] |=
        p_arc_data->mons_data[n].is_appear[season_id][ground_id];
    }

    // その地形に出現する場合は、捕獲状況をチェックする
    if( is_land_appear[ground_id] >= 1 )
    {
      if( is_get == TRUE )
      {
        flg=ZUKANSAVE_GetPokeGetFlag(p_save,p_arc_data->mons_data[n].monsno);
      }
      else
      {
        flg=ZUKANSAVE_GetPokeSeeFlag(p_save,p_arc_data->mons_data[n].monsno);
      }
      if(!flg)
      {
        return FALSE;
      }
    }
  }

  return TRUE;
}

