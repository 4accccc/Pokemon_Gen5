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
#include "field/zonedata.h"       //for ZONEDATA_EnablePalaceUse

#include "palace_warp_check.h"

//------------------------------------------------------------------
/**
 * @brief パレスへのワープ可否チェック
 * @param gsys  GAMESYSTEMへのポインタ
 * @return  BOOL  TRUEの時、パレスにワープ可能
 *
 * @note
 * パレスから復帰したとき、NPCと重なる位置に出現する条件であればワープ不可とする
 * マップ管理表でパレスへのワープを許可されていないゾーンであれば、ワープ不可とする
 */
//------------------------------------------------------------------
BOOL PLC_WP_CHK_Check(GAMESYS_WORK * gsys)
{
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  EVENTDATA_SYSTEM * evdata = GAMEDATA_GetEventData( gdata );
  const MMDL_HEADER * tbl = EVENTDATA_GetNpcData( evdata );
  u16 max = EVENTDATA_GetNpcCount( evdata );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  const MMDLSYS *fos = FIELDMAP_GetMMdlSys( fieldWork );
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  int i;

  //マップ管理表の登録情報チェック
  if (ZONEDATA_EnablePalaceUse( FIELDMAP_GetZoneID( fieldWork ) ) == FALSE) return FALSE;

  { //自機の形態をチェック：なみのり状態ではパレスにいけない
    PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( fld_player );
    if ( form == PLAYER_MOVE_FORM_SWIM || form == PLAYER_MOVE_FORM_DIVING )
    {
      return FALSE;
    }
  }

  if (tbl == NULL || max == 0) return TRUE;

  for(i=0;i<max;i++)
  {
    if ( tbl[i].pos_type == MMDL_HEADER_POSTYPE_GRID )
    {
      s16 x,y,z;
      OBJCODE_PARAM param;
      MMDL_HEADER_GRIDPOS * pos = (MMDL_HEADER_GRIDPOS *)(tbl[i].pos_buf);
      MMDLSYS_LoadOBJCodeParam( fos, tbl[i].obj_code, &param );
      
      FIELD_PLAYER_GetGridPos( fld_player, &x, &y, &z );
      //※基点は左下
      if ( (pos->gx<=x) && (x<pos->gx+param.size_width) &&
           (pos->gz-param.size_depth<z) && (z<=pos->gz) )
      {
        NOZOMU_Printf( "Dont warp\n");
        return FALSE;
      }
    }
    else if ( tbl[i].pos_type == MMDL_HEADER_POSTYPE_RAIL )   //レールに乗っている人物は自分がレールにいるときのみチェックする
    {
      RAIL_LOCATION rail_loc;
      MMDL_HEADER_RAILPOS * pos = (MMDL_HEADER_RAILPOS *)(tbl[i].pos_buf);
      //自分が立っている位置がレールのとき
      if( FIELDMAP_GetBaseSystemType( fieldWork ) == FLDMAP_BASESYS_RAIL )
      {
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
  }

  //ダミーＰＯＳチェック(グリッドのみ)
  if( FIELDMAP_GetBaseSystemType( fieldWork ) == FLDMAP_BASESYS_GRID )
  {
    const POS_EVENT_DATA * pos_data;
    VecFx32 pos;
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
    EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
    MMDL_GetVectorPos( mmdl, &pos );
    pos_data = EVENTDATA_GetDummyPosEvent( evdata, evwork, &pos );
    if( pos_data != NULL ) //ダミーイベントを取得できたらその場では侵入不可
    {
      NOZOMU_Printf( "Dummy Pos Dont warp\n");
      return FALSE;
    }
  }

  return TRUE;
}
