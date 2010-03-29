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
  s16 x,y,z;

  FIELD_PLAYER_GetGridPos( fld_player, &x, &y, &z );


  if (tbl == NULL || max == 0) return TRUE;

  for(i=0;i<max;i++)
  {
    if ( tbl[i].pos_type == MMDL_HEADER_POSTYPE_GRID )
    {
      MMDL_HEADER_GRIDPOS * pos = (MMDL_HEADER_GRIDPOS *)(tbl[i].pos_buf);
      const OBJCODE_PARAM *param = MMDLSYS_GetOBJCodeParam( fos, tbl[i].obj_code );
      OS_Printf( "%d,%d, %d,%d\n",pos->gx, pos->gz, param->size_width, param->size_depth);
      if ( (pos->gx<=x) && (x<pos->gx+param->size_width) &&
           (pos->gz<=z) && (z<pos->gz+param->size_depth) )
      {
        OS_Printf( "Dont warp\n");
        return FALSE;
      }
    }
  }

  return TRUE;
}
