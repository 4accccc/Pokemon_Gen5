//======================================================================
/**
 * @file	scrcmd_elevator.c
 * @brief	スクリプトコマンド：エレベーター用
 * @date  2010.02.15
 * @author	tamada GAMEFREAK inc.
 *
 * @todo  vm_registerの使い道をちゃんとヘッダに定義する
 */
//======================================================================

#include <gflib.h>

#include "system/vm_cmd.h"

#include "scrcmd.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd_work.h"
#include "scrcmd_menuwin.h"

#include "scrcmd_elevator.h"

#include "fieldmap.h"

#include "arc/script_message.naix"
#include "msg/script/msg_common_scr.h"

#include "event_mapchange.h"    //EVENT_ChangeMapPos
#include "field/field_const.h"  //GRID_TO_FX32

//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  VMREG_ID_ELEVATOR = 1,
};
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  FDATA_OFS_MSGID = 0,
  FDATA_OFS_ZONEID = 2,
  FDATA_OFS_POSX = 4,
  FDATA_OFS_POSY = 6,
  FDATA_OFS_POSZ = 8,

  FDATA_SIZE = 10,
};

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static VM_CODE * getFloorData( VM_CODE * top, u32 index )
{
  return top + FDATA_SIZE * index;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline u16 getFloorZoneID( VM_CODE * floorData )
{
  return *((u16 *)&floorData[FDATA_OFS_ZONEID]);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline u16 getFloorPosX( VM_CODE * floorData )
{
  return *((u16 *)&floorData[FDATA_OFS_POSX]);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline u16 getFloorPosY( VM_CODE * floorData )
{
  return *((u16 *)&floorData[FDATA_OFS_POSY]);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline u16 getFloorPosZ( VM_CODE * floorData )
{
  return *((u16 *)&floorData[FDATA_OFS_POSZ]);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline u16 getFloorMsgID( VM_CODE * floorData )
{
  return *((u16 *)&floorData[FDATA_OFS_MSGID]);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void dumpFloorInfo( VM_CODE * floorData )
{
  OS_Printf("ZONE,X,Y,Z,MSGID: %d %d %d %d %d\n",
      getFloorZoneID( floorData ),
      getFloorPosX( floorData ),
      getFloorPosY( floorData ),
      getFloorPosZ( floorData ),
      getFloorMsgID( floorData ) );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u32 getFloorCount( VM_CODE * top )
{
  int i;
  enum { ELEVATOR_MAX = 8 };
  for (i = 0; i < ELEVATOR_MAX; i++ )
  {
    VM_CODE * fdata = getFloorData( top, i );
    dumpFloorInfo( fdata );
    if ( *( (u16*)&fdata[FDATA_OFS_MSGID] ) == 0xffff ) return i;
  }
  GF_ASSERT_MSG( 0, "エレベーターフロア数が大きすぎる！\n" );
  return 0;
}

//======================================================================
//
//      スクリプトコマンド
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：エレベーター：フロアデータ登録
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdElevatorEntryList( VMHANDLE * core, void * wk )
{
  u32 offset = VMGetU32( core );
  core->vm_register[VMREG_ID_ELEVATOR] =(u32)( core->adrs + offset );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：エレベーター：フロア選択肢表示
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdElevatorBmpMenuMakeList( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK *work = wk;
  VM_CODE * topAdrs = (VM_CODE *)core->vm_register[VMREG_ID_ELEVATOR];

  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  STRBUF *msgbuf  = SCRIPT_GetMsgBuffer( sc );
  STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );

  u16 now_zone_id = SCRCMD_WORK_GetZoneID( wk );
  u32 max = getFloorCount( topAdrs );
  u32 count;

  OS_Printf("ELEVATOR:%d\n", max );
  for ( count = 0; count < max; count ++ )
  {
    VM_CODE * fdata = getFloorData( topAdrs, count );
    if ( getFloorZoneID( fdata ) != now_zone_id )
    {
      u32 msg_id = getFloorMsgID( fdata );
      SCRCMD_WORK_AddMenuList( work, msg_id, SCR_BMPMENU_EXMSG_NULL, count, msgbuf, tmpbuf );
    }
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：エレベーター：選択フロアに遷移
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdElevatorMapChange( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK *work = wk;
  u16 select = SCRCMD_GetVMWorkValue( core, wk );
  VM_CODE * topData = (VM_CODE *)core->vm_register[VMREG_ID_ELEVATOR];
  u16 now_zone_id = SCRCMD_WORK_GetZoneID( wk );
  u32 max = getFloorCount( topData );
  u32 count;

  OS_Printf("ELEVATOR: select = %d, max = %d\n", select, max );

  if ( select < max )
  {
    VM_CODE * fdata = getFloorData( topData, select );
    GMEVENT * mapchange_event;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    GAMESYS_WORK * gsys = SCRIPT_GetGameSysWork( sc );
    FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
    VecFx32 appear_pos;
    u16 zone_id = getFloorZoneID( fdata );
    u16 next_x = getFloorPosX( fdata );
    u16 next_y = getFloorPosY( fdata );
    u16 next_z = getFloorPosZ( fdata );

    VEC_Set(&appear_pos,
        GRID_TO_FX32(next_x),
        GRID_TO_FX32(next_y),
        GRID_TO_FX32(next_z)); 
    //mapchange_event = EVENT_ChangeMapPos( gsys, fieldmap, zone_id, &appear_pos, EXIT_DIR_DOWN, FALSE );
    mapchange_event = EVENT_ChangeMapPosNoFade(
        gsys, fieldmap, zone_id, &appear_pos, EXIT_DIR_DOWN );
    SCRIPT_CallEvent( sc, mapchange_event );
  }

  return VMCMD_RESULT_SUSPEND;
}





