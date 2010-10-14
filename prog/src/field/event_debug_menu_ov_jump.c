//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_ov_jump.c
 *	@brief  デバッグメニュージャンプ系オーバーレイ領域
 *	@author	Toru=Nagihashi
 *	@data		2010.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//システム
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "system/main.h" //for HEAPID_FIELDMAP

//フィールド
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fieldmap_control.h"
#include "event_mapchange.h"
#include "palace_warp_check.h"
#include "../../../resource/fld3d_ci/fldci_id_def.h"  // for FLDCIID_MAX

//アプリ
#include "app/townmap.h"

//デバッグメニュー
#include "field_debug.h" 
#include "event_debug_menu_connect_check.h"
#include "event_debug_menu_zone_jump.h"

//外部公開
#include "event_debug_menu_ov_jump.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//======================================================================
//  extern
//======================================================================
//デバッグ用変数実体はgamesystem.cにあります
extern u32 DbgFldHeapRest;
extern u32 DbgFldHeapRestAlctbl;
extern u32 DbgVramRest;
extern u32 DbgFldHeapUseMaxZone;
extern u32 DbgFldHeapUseMaxZoneAlctbl;
extern u32 DbgVramUseMaxZone;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================


//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
static GMEVENT * EVENT_DEBUG_AllMapCheck( GAMESYS_WORK * gsys, BOOL up_to_flag );
static void DbgRestDataUpdate(const u32 inZone);
static void VramDumpCallBack( u32 addr, u32 szByte, void* pUserData );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//--------------------------------------------------------------
/// デバッグメニュー：全マップチェック
//--------------------------------------------------------------
BOOL debugMenuCallProc_AllMapCheck( DEBUG_MENU_EVENT_WORK * p_wk )
{
  GMEVENT * new_event;
  new_event = EVENT_DEBUG_AllMapCheck( p_wk->gmSys, FALSE );
  GMEVENT_ChangeEvent( p_wk->gmEvent, new_event );
  return TRUE;
}
//--------------------------------------------------------------
/// デバッグメニュー：全接続チェック
//--------------------------------------------------------------
BOOL debugMenuCallProc_AllConnectCheck( DEBUG_MENU_EVENT_WORK * p_wk )
{
  GMEVENT * new_event;

  new_event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys, 
      FS_OVERLAY_ID( debug_connect_check ), DEBUG_EVENT_DebugMenu_ConnectCheck, NULL );

  GMEVENT_ChangeEvent( p_wk->gmEvent, new_event );
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  ゾーンジャンプ
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続  FALSE終了
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_DebugZoneJump( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
      FS_OVERLAY_ID( debug_menu_zone_jump ), DEBUG_EVENT_DebugMenu_ZoneJump, p_wk );

  GMEVENT_ChangeEvent( p_wk->gmEvent, event );

  return TRUE;
}
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　ジャンプ
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_Jump( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT * event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_jump ), DEBUG_EVENT_FLDMENU_JumpEasy, NULL );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
  //GMEVENT_ChangeEvent( wk->gmEvent, DEBUG_EVENT_FLDMENU_JumpEasy( wk->gmSys, NULL ) );
  return TRUE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　どこでもジャンプ
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_MapZoneSelect( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT * event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_jump ), DEBUG_EVENT_FLDMENU_JumpAllZone, NULL );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
  //GMEVENT_ChangeEvent( wk->gmEvent, DEBUG_EVENT_FLDMENU_JumpAllZone( wk->gmSys, NULL ) );
  return TRUE;
}


//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　四季マップ間移動
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_MapSeasonSelect( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT * event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_jump ), DEBUG_EVENT_FLDMENU_ChangeSeason, NULL );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
  //GMEVENT_ChangeEvent( wk->gmEvent, DEBUG_EVENT_FLDMENU_ChangeSeason( wk->gmSys, NULL ) );
  return TRUE;
}

//======================================================================
//  デバッグメニュー そらを飛ぶ
//======================================================================
FS_EXTERN_OVERLAY(townmap);
//-------------------------------------
/// デバッグ空を飛ぶ用ワーク
//=====================================
typedef struct
{
  GAMESYS_WORK    *p_gamesys;
  GMEVENT         *p_event;
  FIELDMAP_WORK *p_field;
  TOWNMAP_PARAM   *p_param;
} DEBUG_SKYJUMP_EVENT_WORK;
//-------------------------------------
///   PROTOTYPE
//=====================================
static GMEVENT_RESULT debugMenuSkyJump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ空を飛ぶ
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続  FALSE終了
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_DebugSkyJump( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field  = p_wk->fieldWork;
  DEBUG_SKYJUMP_EVENT_WORK  *p_sky;

  //イヴェント
  GMEVENT_Change( p_event, debugMenuSkyJump, sizeof(DEBUG_SKYJUMP_EVENT_WORK) );
  p_sky = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_sky, sizeof(DEBUG_SKYJUMP_EVENT_WORK) );

  //ワーク設定
  p_sky->p_gamesys  = p_gamesys;
  p_sky->p_event    = p_event;
  p_sky->p_field    = p_field;

  //タウンマップワーク設定
  p_sky->p_param  = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(TOWNMAP_PARAM) );
  GFL_STD_MemClear( p_sky->p_param, sizeof(TOWNMAP_PARAM) );
  p_sky->p_param->mode      = TOWNMAP_MODE_DEBUGSKY;
  {
    GAMEDATA    *p_gamedata;
    PLAYER_WORK *p_player;
    p_gamedata  = GAMESYSTEM_GetGameData( p_sky->p_gamesys );
    p_player    = GAMEDATA_GetMyPlayerWork( p_gamedata );
    p_sky->p_param->zoneID    = PLAYERWORK_getZoneID(p_player);
    p_sky->p_param->escapeID  = GAMEDATA_GetEscapeLocation( p_gamedata )->zone_id;
    p_sky->p_param->p_gamesys = p_gamesys;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ空を飛ぶイベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuSkyJump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
    SEQ_CHANGE_MAP,
    SEQ_EXIT,
  };

  DEBUG_SKYJUMP_EVENT_WORK  *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
          FS_OVERLAY_ID(townmap), &TownMap_ProcData, p_wk->p_param ) );
    *p_seq  = SEQ_PROC_END;
    break;

  case SEQ_PROC_END:
    if( p_wk->p_param->select == TOWNMAP_SELECT_SKY )
    {
      *p_seq  = SEQ_CHANGE_MAP;
    }
    else
    {
      *p_seq  = SEQ_EXIT;
    }
    break;

  case SEQ_CHANGE_MAP:
    NAGI_Printf("ゾーンID %d",p_wk->p_param->zoneID);
    GMEVENT_CallEvent( p_wk->p_event, DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->p_gamesys, p_wk->p_field, p_wk->p_param->zoneID ) );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    if( p_wk->p_param )
    {
      GFL_HEAP_FreeMemory(p_wk->p_param);
    }
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}



//======================================================================
//
//    マップ全チェック
//
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  int check_zone_id;
  BOOL up_to_flag;
  int wait;
  GAMESYS_WORK * gsys;
}ALL_MAP_CHECK_WORK;

//--------------------------------------------------------------
//--------------------------------------------------------------
static const u16 ng_zone_table[] = {
  ZONE_ID_UNION,        //ユニオンルーム
  ZONE_ID_CLOSSEUM,
  ZONE_ID_CLOSSEUM02,
  ZONE_ID_C04R0202,     //ミュージカル控え室
  ZONE_ID_C03R0801,     //ロイヤルイッシュ号（通信が途切れる）
};

//#include "arc/fieldmap/map_matrix.naix"
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL checkNGZoneID( zone_id )
{
  int i;
#if 0
  if ( ZONEDATA_GetMatrixID(zone_id) == NARC_map_matrix_palacefield_mat_bin )
  {
    continue;
  }
#endif
  for ( i = 0; i < NELEMS(ng_zone_table); i++ )
  {
    if (ng_zone_table[i] == zone_id )
    {
      return TRUE;
    }
  }
  return FALSE;
}
static int getNextZoneID( ALL_MAP_CHECK_WORK* amcw )
{
  int zone_id = amcw->check_zone_id;
  while (TRUE)
  {
    if ( amcw->up_to_flag )
    {
      zone_id ++;
      if ( zone_id >= ZONE_ID_MAX ) break;
    } else {
      zone_id --;
      if ( zone_id < 0 ) break;
    }
    if ( checkNGZoneID( zone_id ) == TRUE ) continue;
    break;
  }
  amcw->check_zone_id = zone_id;
  return zone_id;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT_RESULT allMapCheckEvent( GMEVENT * event, int *seq, void * wk )
{
  ALL_MAP_CHECK_WORK * amcw = wk;
  FIELDMAP_WORK * fieldmap;
  u16 zone_id;

  enum {
    SEQ_INIT,
    SEQ_SEEK_ID,
    SEQ_WAIT,
    SEQ_FLY_CHECK,
  };
  switch ( *seq )
  {
  case SEQ_INIT:
    if ( amcw->up_to_flag )
    {
      amcw->check_zone_id = -1;
    } else {
      amcw->check_zone_id = ZONE_ID_MAX;
    }
    *seq = SEQ_SEEK_ID;

    /* FALL THROUGH */

  case SEQ_SEEK_ID:
    {
      FIELD_WFBC_CORE* p_core;
      GAMEDATA *gmData = GAMESYSTEM_GetGameData(amcw->gsys);
      p_core = GAMEDATA_GetMyWFBCCoreData( gmData );
      zone_id = getNextZoneID( amcw );

      if ( zone_id >= ZONE_ID_MAX || zone_id < 0 )
      {
        OS_Printf( "AllocatableFieldHeap restmin = 0x%x: zone = %d\n",DbgFldHeapRestAlctbl, DbgFldHeapUseMaxZoneAlctbl );
        OS_Printf( "TotalFieldHeap restmin = 0x%x: zone = %d\n",DbgFldHeapRest, DbgFldHeapUseMaxZone );
        OS_Printf( "3DVram restmin = 0x%x: zone = %d\n",DbgVramRest, DbgVramUseMaxZone );
        OS_Printf( "HUDSON: ALL MAP CHECK SUCCESS!!\n" ); // hudson 検出用
        //WFBCコアタイプを元に戻す
        {
          if (PM_VERSION == VERSION_BLACK) p_core->type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
          else p_core->type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
        }
        return GMEVENT_RES_FINISH;
      }

      {
        //セーブデータのＷＦＢＣタイプを変更する
        if ( (zone_id == ZONE_ID_WC10)||(zone_id == ZONE_ID_PLCW10) )
        {
          NOZOMU_Printf("ホワイトフォレストへ\n");
          p_core->type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
        }
        else if ( (zone_id == ZONE_ID_BC10)||(zone_id == ZONE_ID_PLC10) )
        {
          NOZOMU_Printf("ブラックシティへ\n");
          p_core->type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
        }
      }
      {
        char buf[ZONEDATA_NAME_LENGTH];
        ZONEDATA_DEBUG_GetZoneName(buf, zone_id);
        OS_TPrintf( "\t[[ALL MAP CHECKING... :%s]]!!\n", buf );
      }
      OS_Printf( "ALL MAP CHECK: zone_id = %d\n", zone_id );
      fieldmap = GAMESYSTEM_GetFieldMapWork( amcw->gsys );
      GMEVENT_CallEvent( event,
          DEBUG_EVENT_ChangeMapDefaultPos( amcw->gsys, fieldmap, zone_id ) );
    }
    *seq = SEQ_WAIT;
    break;
  case SEQ_WAIT:
    amcw->wait ++;
    if ( amcw->wait > 120 )
    {
      fieldmap = GAMESYSTEM_GetFieldMapWork( amcw->gsys );
      //フィールドヒープの残りとＶＲＡＭの残りを計測
      DbgRestDataUpdate( FIELDMAP_GetZoneID( fieldmap ) );
      amcw->wait = 0;

      //パレスワープ禁止場所をダンプ
      PLC_WP_CHK_CheckDebugPrint(amcw->gsys);

      *seq = SEQ_FLY_CHECK/*SEQ_SEEK_ID*/;
    }
    break;
  case SEQ_FLY_CHECK:
    fieldmap = GAMESYSTEM_GetFieldMapWork( amcw->gsys );
    {
      GMEVENT *call_evt;
      FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(fieldmap);
      call_evt = FLD3D_CI_CreateCutInEvt(amcw->gsys, ptr, FLDCIID_FLY_OUT);
      GMEVENT_CallEvent( event, call_evt );
      *seq = SEQ_SEEK_ID;
    }
    break;  
  }

  return GMEVENT_RES_CONTINUE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * EVENT_DEBUG_AllMapCheck( GAMESYS_WORK * gsys, BOOL up_to_flag )
{
  GMEVENT * new_event;
  ALL_MAP_CHECK_WORK * amcw;
  new_event = GMEVENT_Create( gsys, NULL, allMapCheckEvent, sizeof(ALL_MAP_CHECK_WORK) );
  amcw = GMEVENT_GetEventWork( new_event );
  amcw->up_to_flag = up_to_flag;
  amcw->gsys = gsys;
  return new_event;
}
static void DbgRestDataUpdate(const u32 inZone)
{
  u32 heap_rest = GFL_HEAP_GetHeapFreeSize( HEAPID_FIELDMAP );
  u32 vram_rest = 0;
  u32 allocatable;
  //GFI関数は基本的に直接使用禁止ですが、今回は特別に使用します。
  allocatable = GFI_HEAP_GetHeapAllocatableSize(HEAPID_FIELDMAP);

  OS_Printf("Zone %d seach..\n", inZone );

  if ( DbgFldHeapRest >= heap_rest )
  {
    DbgFldHeapRest = heap_rest;
    DbgFldHeapUseMaxZone = inZone;
  }
  if ( DbgFldHeapRestAlctbl >= allocatable )
  {
    DbgFldHeapRestAlctbl = allocatable;
    DbgFldHeapUseMaxZoneAlctbl = inZone;
  }

  NNS_GfdDumpLnkTexVramManagerEx( VramDumpCallBack, VramDumpCallBack, &vram_rest );

  if ( DbgVramRest >= vram_rest )
  {
    DbgVramRest = vram_rest;
    DbgVramUseMaxZone = inZone;
  }
  OS_Printf( "NowAllocatableFieldHeap rest = 0x%x: zone = %d\n", allocatable, inZone);
  OS_Printf( "NowTotalFieldHeap rest = 0x%x: zone = %d\n", heap_rest, inZone);

  OS_Printf( "AllocatableFieldHeap restmin = 0x%x: zone = %d\n",DbgFldHeapRestAlctbl, DbgFldHeapUseMaxZoneAlctbl );
  OS_Printf( "TotalFieldHeap restmin = 0x%x: zone = %d\n",DbgFldHeapRest, DbgFldHeapUseMaxZone );
  OS_Printf( "3DVram restmin = 0x%x: zone = %d\n",DbgVramRest, DbgVramUseMaxZone );

  if (vram_rest <= 0x1a200) OS_FPrintf(3, "足りない zone:%d size:%x\n",inZone, vram_rest);
}
static void VramDumpCallBack( u32 addr, u32 szByte, void* pUserData )
{
  // 合計サイズを計算。
  (*((u32*)pUserData)) += szByte;
  // 情報をデバックコンソールに出力
  OS_Printf("adr=0x%08x:  size=0x%08x    \n", addr, szByte );
  OS_Printf("Free_total 0x%x \n", (*((u32*)pUserData)) );
}
