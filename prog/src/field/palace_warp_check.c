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
#include "system/main.h"      //for HEAPID_
#include "arc/fieldmap/zone_id.h" //for WC10 BC10

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
  s16 x,y,z;
  int i;

  int zone_id = FIELDMAP_GetZoneID( fieldWork );

  //マップ管理表の登録情報チェック
  if (ZONEDATA_EnablePalaceUse( zone_id ) == FALSE) return FALSE;

  { //自機の形態をチェック：なみのり状態ではパレスにいけない
    PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( fld_player );
    if ( form == PLAYER_MOVE_FORM_SWIM || form == PLAYER_MOVE_FORM_DIVING )
    {
      return FALSE;
    }
  }

  if (tbl == NULL || max == 0)
  {
    if ( (zone_id != ZONE_ID_BC10)&&(zone_id != ZONE_ID_WC10) )
    {
      return TRUE;
    }
  }

  for(i=0;i<max;i++)
  {
    if ( tbl[i].pos_type == MMDL_HEADER_POSTYPE_GRID )
    {
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

  //ホワイトフォレスト・ブラックシティのときのチェック
  if ( (zone_id == ZONE_ID_BC10)||(zone_id == ZONE_ID_WC10)  )
  {
    int i;
    FIELD_WFBC_PEOPLE_DATA_LOAD* p_loader;
    FIELD_WFBC_CORE_TYPE type;
    HEAPID heap_id;
    FIELD_WFBC_CORE_PEOPLE_POS* ppos;
    
    heap_id = HEAPID_FIELDMAP;
    FIELD_PLAYER_GetGridPos( fld_player, &x, &y, &z );
    //読み込み
    //type =   FIELD_WFBC_CORE_TYPE_BLACK_CITY,    // ブラックシティ
    //         FIELD_WFBC_CORE_TYPE_WHITE_FOREST,  // ホワイトフォレスト
    if (zone_id == ZONE_ID_BC10) type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
    else type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
    ppos = FIELD_WFBC_PEOPLE_POS_Create( p_loader, type, GFL_HEAP_LOWID(heap_id) );

    //データ取得 indexの最大値＝FIELD_WFBC_PEOPLE_MAX
    for (i=0;i<FIELD_WFBC_PEOPLE_MAX;i++)
    {
      const FIELD_WFBC_CORE_PEOPLE_POS* pos;
      pos = FIELD_WFBC_PEOPLE_POS_GetIndexData( ppos, i );
      if ( (pos->gx == x)&&(pos->gz == z) )
      {
        NOZOMU_Printf( "Dont warp\n");
        return FALSE;
      }
    }
    for (i=0;i<FIELD_WFBC_PEOPLE_MAX;i++)
    {
      const FIELD_WFBC_CORE_PEOPLE_POS* pos;
      pos = FIELD_WFBC_PEOPLE_POS_GetIndexItemPos( ppos, i );
      if ( (pos->gx == x)&&(pos->gz == z) )
      {
        NOZOMU_Printf( "Dont warp\n");
        return FALSE;
      }
    }
    //破棄
    FIELD_WFBC_PEOPLE_POS_Delete( ppos );
  }

  return TRUE;
}

#ifdef PM_DEBUG

#include "eventdata_local.h"

extern const POS_EVENT_DATA * D_EVENTDATA_GetPosEventData( const EVENTDATA_SYSTEM *evdata );


void PLC_WP_CHK_CheckDebugPrint(GAMESYS_WORK * gsys)
{
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  EVENTDATA_SYSTEM * evdata = GAMEDATA_GetEventData( gdata );
  const MMDL_HEADER * tbl = EVENTDATA_GetNpcData( evdata );
  u16 max = EVENTDATA_GetNpcCount( evdata );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  const MMDLSYS *fos = FIELDMAP_GetMMdlSys( fieldWork );
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  int i;
  u32 debug_rail_obj;
  s16 x,y,z, w,h;
  int zone_id = FIELDMAP_GetZoneID( fieldWork );

  //マップ管理表の登録情報チェック
  if (ZONEDATA_EnablePalaceUse( zone_id ) == FALSE) return;

  if (tbl == NULL || max == 0)
  {
    if ( (zone_id != ZONE_ID_BC10)&&(zone_id != ZONE_ID_WC10) )
    {
      return;
    }
  }


  OS_FPrintf( 2,"===========ZONE_ID %d==============\n",FIELDMAP_GetZoneID( fieldWork ));
  debug_rail_obj = 0;
  for(i=0;i<max;i++)
  {
    if ( tbl[i].pos_type == MMDL_HEADER_POSTYPE_GRID )
    {
      OBJCODE_PARAM param;
      MMDL_HEADER_GRIDPOS * pos = (MMDL_HEADER_GRIDPOS *)(tbl[i].pos_buf);
      MMDLSYS_LoadOBJCodeParam( fos, tbl[i].obj_code, &param );
      
      //※基点は左下
      for(w=0;w<param.size_width;w++){
        x = pos->gx+w;
        for(h=0;h<param.size_depth;h++){
          z = pos->gz-h;
          OS_FPrintf( 2,"OBJ_Grid %d,%d\n",x, z);
        }
      }
    }
    else if ( tbl[i].pos_type == MMDL_HEADER_POSTYPE_RAIL )   //レールに乗っている人物は自分がレールにいるときのみチェックする
    {
      RAIL_LOCATION rail_loc;
      MMDL_HEADER_RAILPOS * pos = (MMDL_HEADER_RAILPOS *)(tbl[i].pos_buf);
      {
        OS_FPrintf( 2,"RAIL %d,%d,%d\n",pos->rail_index,pos->front_grid,pos->side_grid);
        debug_rail_obj++;
      }
    }
  }

  OS_FPrintf( 2,"RAIL_OBJ_NUM = %d\n",debug_rail_obj);

  //ダミーＰＯＳチェック(グリッドのみ)
  if( FIELDMAP_GetBaseSystemType( fieldWork ) == FLDMAP_BASESYS_GRID )
  {
    const POS_EVENT_DATA * pos_data;
    VecFx32 pos;
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
    EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
    MMDL_GetVectorPos( mmdl, &pos );
    pos_data = EVENTDATA_GetDummyPosEvent( evdata, evwork, &pos );
    {
      const POS_EVENT_DATA *data = D_EVENTDATA_GetPosEventData( evdata );
      if( data != NULL )
      {
        u16 i;
        u16 *work_val;
        u16 max = EVENTDATA_GetPosEventNum(evdata);
        for( i=0; i < max; i++, data++ )
        {
          BOOL valid = FALSE;
          if ( data->check_type != POS_CHECK_TYPE_DUMMY ) continue;

          //ワーク指定が０のとき、無条件にPOS発動
          if ( data->workID == 0 ) valid = TRUE;
          else OS_FPrintf( 2,"ダミーなのに値が設定されている\n");

          work_val = EVENTWORK_GetEventWorkAdrs( evwork, data->workID );
          if( (*work_val) == data->param ) valid = TRUE;

          if (valid)
          {
            //座標表示
            const POS_EVENT_DATA_GPOS* cp_gpos;
            int gx, gz, y;

            if( data->pos_type == EVENTDATA_POSTYPE_RAIL )
            {
              OS_FPrintf( 2,"イベントデータがレール設定\n");
              continue;
            }
            cp_gpos = (const POS_EVENT_DATA_GPOS*)data->pos_buf; 

            OS_FPrintf( 2,"EVT %d,%d,%d\n",cp_gpos->gx, cp_gpos->height, cp_gpos->gz);
          }
        }
      }
    }
  }

  //ホワイトフォレスト・ブラックシティのときのチェック
  if ( (zone_id == ZONE_ID_BC10)||(zone_id == ZONE_ID_WC10)  )
  {
    int i;
    FIELD_WFBC_PEOPLE_DATA_LOAD* p_loader;
    FIELD_WFBC_CORE_TYPE type;
    HEAPID heap_id;
    FIELD_WFBC_CORE_PEOPLE_POS* ppos;
    
    heap_id = HEAPID_FIELDMAP;
    FIELD_PLAYER_GetGridPos( fld_player, &x, &y, &z );
    //読み込み
    // ローダー生成
    p_loader = FIELD_WFBC_PEOPLE_DATA_Create( 0, GFL_HEAP_LOWID(heap_id) );
    //type =   FIELD_WFBC_CORE_TYPE_BLACK_CITY,    // ブラックシティ
    //         FIELD_WFBC_CORE_TYPE_WHITE_FOREST,  // ホワイトフォレスト
    if (zone_id == ZONE_ID_BC10) type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
    else type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
    ppos = FIELD_WFBC_PEOPLE_POS_Create( p_loader, type, GFL_HEAP_LOWID(heap_id) );

    //データ取得 indexの最大値＝FIELD_WFBC_PEOPLE_MAX
    for (i=0;i<FIELD_WFBC_PEOPLE_MAX;i++)
    {
      const FIELD_WFBC_CORE_PEOPLE_POS* pos;
      pos = FIELD_WFBC_PEOPLE_POS_GetIndexData( ppos, i );
      OS_FPrintf( 2,"WB %d,%d\n",pos->gx, pos->gz);
    }
    for (i=0;i<FIELD_WFBC_PEOPLE_MAX;i++)
    {
      const FIELD_WFBC_CORE_PEOPLE_POS* pos;
      pos = FIELD_WFBC_PEOPLE_POS_GetIndexItemPos( ppos, i );
      OS_FPrintf( 2,"WB %d,%d\n",pos->gx, pos->gz);
    }
    //破棄
    FIELD_WFBC_PEOPLE_POS_Delete( ppos );

    FIELD_WFBC_PEOPLE_DATA_Delete( p_loader );
  }
}
#endif
