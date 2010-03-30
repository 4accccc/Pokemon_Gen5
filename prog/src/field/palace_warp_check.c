//======================================================================
/**
 * @file	palace_warl_check.c
 * @brief　パレスワープ可能チェック
 * @author	Saito
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "gamesystem/gamesystem.h"
#include "fieldmap.h"
#include "field/eventdata_system.h"
#include "field/eventdata_sxy.h"

#include "palace_warp_check.h"

PLC_WP_CHK_Check(GAMESYS_WORK * gsys)
{
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  EVENTDATA_SYSTEM * evdata = GAMEDATA_GetEventData( gdata );
  const MMDL_HEADER * tbl = EVENTDATA_GetNpcData( evdata );
  u16 max = EVENTDATA_GetNpcCount( evdata );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  const MMDLSYS *fos = FIELDMAP_GetMMdlSys( fieldWork );
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  int i;

  if (tbl == NULL || max == 0) return TRUE;

  for(i=0;i<max;i++)
  {
    if ( tbl[i].pos_type == MMDL_HEADER_POSTYPE_GRID )
    {
      s16 x,y,z;
      MMDL_HEADER_GRIDPOS * pos = (MMDL_HEADER_GRIDPOS *)(tbl[i].pos_buf);
      const OBJCODE_PARAM *param = MMDLSYS_GetOBJCodeParam( fos, tbl[i].obj_code );
      FIELD_PLAYER_GetGridPos( fld_player, &x, &y, &z );
      if ( (pos->gx<=x) && (x<pos->gx+param->size_width) &&
           (pos->gz<=z) && (z<pos->gz+param->size_depth) )
      {
        NOZOMU_Printf( "Dont warp\n");
        return FALSE;
      }
    }
    else if ( tbl[i].pos_type == MMDL_HEADER_POSTYPE_RAIL )
    {
      RAIL_LOCATION rail_loc;
      MMDL_HEADER_RAILPOS * pos = (MMDL_HEADER_RAILPOS *)(tbl[i].pos_buf);
      FIELD_PLAYER_GetNoGridLocation( fld_player, &rail_loc );
      if ( (pos->rail_index == rail_loc.rail_index) &&
           (pos->front_grid == rail_loc.line_grid) &&
           (pos->side_grid == rail_loc.width_grid) )
      {
        NOZOMU_Printf( "Dont warp\n");
        return FALSE;
      }
    }
  }

  return TRUE;
}
