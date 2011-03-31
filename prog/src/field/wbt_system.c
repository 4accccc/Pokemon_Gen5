//============================================================================================
/**
 * @file  wbt_system.c
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.02.15
 */
//============================================================================================

#include <gflib.h>

#include "wbt_system.h"
//#include "app/wbt_list.h"

//#include "poke_tool/regulation_def.h"
//#include "wbt_trlist_struct.h"  //WBT_TRDATA

//#include "arc_def.h"
//#include "arc/wbt/wbt_trlist.naix"
//#include "arc/fieldmap/fldmmdl_mdlres.naix"

//#include "msg/msg_wbt_tournament.h"
//#include "arc/message.naix"

#include "wbt_system_local.h"

//#include "system/gfl_use.h" //GFUser_GetPublicRand0

//============================================================================================
//============================================================================================

//============================================================================================
//============================================================================================

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTSYS * WBTSYS_Create( HEAPID heapID )
{
  WBTSYS * wbtsys = GFL_HEAP_AllocClearMemory( heapID, sizeof(WBTSYS) );
  wbtsys->heapID = heapID;
  return wbtsys;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_Delete( WBTSYS * wbtsys )
{
  GFL_HEAP_FreeMemory( wbtsys );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetType( WBTSYS * wbtsys, WBTTYPE type )
{
  wbtsys->type = type;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTTYPE WBTSYS_GetType( const WBTSYS * wbtsys )
{
  return wbtsys->type;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetSelectDLData( WBTSYS * wbtsys, u8 no )
{
  GF_ASSERT( no <= WBT_DOWNLOAD_NUMBER );
  wbtsys->select_dl_data = no;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u8 WBTSYS_GetSelectDLData( const WBTSYS * wbtsys )
{
  return wbtsys->select_dl_data;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetRule( WBTSYS * wbtsys, WBTRULE rule )
{
  wbtsys->rule = rule;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
WBTRULE WBTSYS_GetRule( const WBTSYS * wbtsys )
{
  return wbtsys->rule;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetScriptAnswerWork( WBTSYS * wbtsys, u16 * scr_answer_work )
{
  wbtsys->scr_answer_work = scr_answer_work;
}
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
const REGULATION * WBTSYS_GetRegulation( WBTSYS * wbtsys )
{
  const REGULATION * reg;
  int reg_type;
  switch ( wbtsys->rule )
  {
  case WBTRULE_SINGLE:
    reg_type = REG_LV50_SINGLE;
    break;
  case WBTRULE_DOUBLE:
    reg_type = REG_LV50_DOUBLE;
    break;
  case WBTRULE_TRIPLE:
    reg_type = REG_LV50_TRIPLE;
    break;
  case WBTRULE_ROTATION:
    reg_type = REG_LV50_ROTATION;
    break;
  default:
    GF_ASSERT( 0 );
  }
  PokeRegulation_LoadData( reg_type, &wbtsys->wbtreg.base_reg );
  return &wbtsys->wbtreg.base_reg;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetPokelistResult( WBTSYS * wbtsys, PLIST_DATA * plist_data )
{
  GFL_STD_MemCopy( plist_data->in_num, wbtsys->pokelist_select_num, 6 );
  wbtsys->pokelist_result_select = plist_data->ret_sel;
  wbtsys->pokelist_return_mode = plist_data->ret_mode;

  if( wbtsys->pokelist_return_mode != PL_RET_NORMAL ||
      wbtsys->pokelist_result_select == PL_SEL_POS_EXIT ||
      wbtsys->pokelist_result_select == PL_SEL_POS_EXIT2 )
  {
    *wbtsys->scr_answer_work = FALSE;
  }
  else
  {
    *wbtsys->scr_answer_work = TRUE;
  }

}

//------------------------------------------------------------------
//------------------------------------------------------------------
static POKEPARTY * getUsePokeParty( WBTSYS * wbtsys, GAMEDATA * gamedata )
{
  POKEPARTY * pp;
  if ( wbtsys->use_btlbox == FALSE )
  {
    pp = GAMEDATA_GetMyPokemon( gamedata );
  }
  else
  {
    pp = wbtsys->btl_box_party;
  }
  GF_ASSERT( pp );
  return pp;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetupPokeListData( WBTSYS * wbtsys, GAMEDATA * gamedata, PLIST_DATA * plist_data )
{
  int type;

  GFL_STD_MemClear32( plist_data, sizeof(PLIST_DATA) );

  plist_data->pp = getUsePokeParty( wbtsys, gamedata );

  switch ( wbtsys->rule )
  {
  default:
    GF_ASSERT(0);
    /* FALL THROUGH */

  case WBTRULE_SINGLE:
    type = PL_TYPE_SINGLE;
    break;
  case WBTRULE_DOUBLE:
    type = PL_TYPE_DOUBLE;
    break;
  case WBTRULE_TRIPLE:
    type = PL_TYPE_TRIPLE;
    break;
  case WBTRULE_ROTATION:
    type = PL_TYPE_ROTATION;
    break;
  }
  plist_data->reg = (void *)WBTSYS_GetRegulation( wbtsys );
  plist_data->mode = PL_MODE_BATTLE_SUBWAY;
  plist_data->type = type;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSYS_SetupPokeStatusData( WBTSYS * wbtsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus )
{
  ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( gamedata );
  GFL_STD_MemClear32( pstatus, sizeof(PSTATUS_DATA) );
  pstatus->ppd = getUsePokeParty( wbtsys, gamedata );
  pstatus->ppt = PST_PP_TYPE_POKEPARTY;
  pstatus->max = PokeParty_GetPokeCount( pstatus->ppd );
  pstatus->mode = PST_MODE_NORMAL;
  pstatus->page = PPT_INFO;
  pstatus->game_data = gamedata;

  pstatus->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );
}



