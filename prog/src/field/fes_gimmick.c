//===============================================================================
/*
 *  @file   fes_gimmick.c
 *  @brief  フェスミッション フィールドギミック
 *  @author Miyuki Iwasawa
 *  @date   11.08.12
 */
//===============================================================================

#include <gflib.h>
#include "system/main.h"      //GFL_HEAPID_APP参照
#include "system/time_icon.h"
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/g_power.h"
#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission_nw.h"
#include "gamesystem/fest_mission_local.h"
#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"
#include "field/script.h"
#include "field/script_local.h"
#include "field/field_msgbg.h"
#include "field/fldeff_fes_kira.h"
#include "./event_fieldmap_control.h"
#include "./event_fmission.h"
#include "./event_subscreen.h"

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

#include "script_message.naix"
#include "message.naix"
#include "msg/script/msg_fmission_event.h"
#include "arc/fieldmap/zone_id.h"
#include "arc/fieldmap/fldmmdl_objcode.h"
#include "../../../resource/fldmapdata/script/fmission_scr_def.h" //SCRID_～
#include "../../../resource/fldmapdata/flagwork/flag_define.h"

#include "field/fes_gimmick.h"
#include "field/fes_gimmick_def.h"
#include "field/fes_gimmick_local.h"

FS_EXTERN_OVERLAY( field_fes_gimmick_item );
FS_EXTERN_OVERLAY( field_fes_gimmick_npc );
FS_EXTERN_OVERLAY( field_fes_gimmick_event );

//======================================================================
//======================================================================
static void sub_OverlayLoadUnload( FES_GIMMICK* wk, BOOL load_f );
static void mission_init_mushroom_enc( FES_GIMMICK* wk );

static u16 sub_GetZoneIdx( u16 zone_id );

//======================================================================
//======================================================================

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　管理ワーク初期化
 *
 *  フィールドマップ初期化時にCreateし、フィールド開放時にDeleteする
 */
//----------------------------------------------------------------------
FES_GIMMICK* FES_GIMMICK_SYS_Create( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldWork, HEAPID heapID )
{
  FES_GIMMICK* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FES_GIMMICK));
 
  wk->gsys = gsys;
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
  wk->fest_sys =  GAMESYSTEM_GetLinkFestivalWork( gsys );
  wk->fmwk = (const FEST_MISSION*)LINK_FESTIVAL_GetMissionWork( wk->fest_sys );
  wk->wk_buf = LINK_FESTIVAL_GetMissionGimmickWork( wk->fest_sys );

  wk->fieldWork = fieldWork;
  //イベントデータ
  wk->evdata = GAMEDATA_GetEventData( wk->gdata );
  wk->evwork = GAMEDATA_GetEventWork( wk->gdata );

  {
    const FLDNOGRID_MAPPER* cp_mapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
    if( cp_mapper != NULL ){
      wk->railman = FLDNOGRID_MAPPER_GetRailMan( cp_mapper );
    }
  }
  sub_OverlayLoadUnload( wk, TRUE );
  return wk;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　管理ワーク開放
 */
//----------------------------------------------------------------------
void FES_GIMMICK_SYS_Delete( FES_GIMMICK* wk )
{
  sub_OverlayLoadUnload( wk, FALSE );

  MI_CpuClear8( wk, sizeof(FES_GIMMICK));
  GFL_HEAP_FreeMemory( wk );
}

//---------------------------------------------------------
/*
 * @brief ミッションタイプごとにオーバーレイロード&アンロード
 */
//---------------------------------------------------------
static void sub_OverlayLoadUnload( FES_GIMMICK* wk, BOOL load_f )
{
  FSOverlayID ol_field;

  /*
   * フィールド終了時にフェスが終了しているケースがあるので
   * アンロードはロード時に覚えておいたパラメータで行う
   */
  if( load_f ){
    if( LINK_FESTIVAL_IsMissionEntry( wk->fest_sys ) == FALSE ){
      return;
    }
    switch( wk->fmwk->arc.m_type ){
    case FMISSION_TYPE_ITEM:
      ol_field = FS_OVERLAY_ID( field_fes_gimmick_item );
      break;
    case FMISSION_TYPE_NPC:
      ol_field = FS_OVERLAY_ID( field_fes_gimmick_npc );
      break;
    case FMISSION_TYPE_EVENT:
      ol_field = FS_OVERLAY_ID( field_fes_gimmick_event );
      break;
    default:
      return;
    }
    wk->ov_load_f = TRUE; 
    wk->fld_ov_id = ol_field;
    GFL_OVERLAY_Load( ol_field );
  }else{
    if( wk->ov_load_f ){
      GFL_OVERLAY_Unload( wk->fld_ov_id );
      wk->ov_load_f = FALSE;
    }
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　動作モデルセットアップ処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_SYS_MMdlSetup( FES_GIMMICK* wk, MMDLSYS* mmdl_sys )
{
  //MMDLSYS
  wk->mmdl_sys = mmdl_sys;
  
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return;
  }
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_NPC ){
    FES_GIMMICK_NPC_MMdlSetup( wk, wk->wk_buf );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　フェス用フィールドエフェクトを初期化
 *
 *  システム初期化のタイミングだと、フィールドエフェクトが未初期化のため
 *  専用のエフェクト初期化関数を用意
 */
//----------------------------------------------------------------------
void FES_GIMMICK_SYS_FldEffSetup( FES_GIMMICK* wk, FLDEFF_CTRL* fectrl, FIELD_PLAYER* f_player )
{
  //フィールドエフェクトコントローラ取得
  wk->fectrl = fectrl;
  
  //エフェクト当たり判定用に自機のMMDLを取得しておく
  wk->field_player = f_player;
  wk->player_mmdl = FIELD_PLAYER_GetMMdl( wk->field_player );
  
  //道具ミッション用エフェクトタスクを常駐で持つ
  wk->eff_task = FLDEFF_FES_KIRA_SetEffect( wk->fectrl );
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　フェス用フィールドエフェクトを削除
 */
//----------------------------------------------------------------------
void FES_GIMMICK_SYS_FldEffDelete( FES_GIMMICK* wk )
{
  if( wk->eff_task != NULL ){
    FLDEFF_TASK_CallDelete( wk->eff_task );
    wk->eff_task = NULL;
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック ミッション開始時初期化　
 */
//----------------------------------------------------------------------
void FES_GIMMICK_MissionInitSetup( FES_GIMMICK* wk )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return;
  }
  MI_CpuClear8( LINK_FESTIVAL_GetMissionGimmickWork( wk->fest_sys ), sizeof(FES_GIMMICK_WORK) );

  switch( wk->fmwk->arc.m_type ){
  case FMISSION_TYPE_MUSHROOM_ENC:
    mission_init_mushroom_enc( wk );
    break;
  case FMISSION_TYPE_EVENT:
    // 隠し穴調査（特殊ポケモン配置版）用ワークを初期化する
    if( FES_GIMMICK_EVENT_IsExAnimalTrailMission( wk ) )
    {
      FES_GIMMICK_EVENT_ExAnimalTrailMissionWkCreate( wk );
    }
    break;
  case FMISSION_TYPE_NPC:
    FES_GIMMICK_NPC_MissionInitSetup( wk, (FES_GIMMICK_NPC_WORK*)wk->wk_buf );
    break;
  }
}

/*
 * @brief タマゲタケシンボルエンカウントミッション用初期化処理
 * これ一つのためにファイルを用意するほどでもなかったのでここに置く
 */
static void mission_init_mushroom_enc( FES_GIMMICK* wk )
{
  int i;

  //フィールド上のすべてのタマゲタケを復活させる
  for( i = FV_FLDMONS_01; i <= FV_FLDMONS_12; i++ ){
    EVENTWORK_ResetEventFlag( wk->evwork, i );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　フィールド復帰処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_FieldRecoverSetup( FES_GIMMICK* wk )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return;
  }
  switch( wk->fmwk->arc.m_type ){
  case FMISSION_TYPE_ITEM:
    FES_GIMMICK_ITEM_FieldRecoverSetup( wk, wk->wk_buf );
    break;
  case FMISSION_TYPE_NPC:
    FES_GIMMICK_NPC_FieldRecoverSetup( wk, wk->wk_buf );
    break;
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック 削除　
 */
//----------------------------------------------------------------------
BOOL FES_GIMMICK_Delete( FES_GIMMICK* wk )
{
  void* gmk_wk = LINK_FESTIVAL_GetMissionGimmickWork( wk->fest_sys );
  
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return FALSE;
  }

  switch( wk->fmwk->arc.m_type ){
  case FMISSION_TYPE_ITEM:
    return FES_GIMMICK_ITEM_Delete( wk, (FES_GIMMICK_ITEM_WORK*)gmk_wk );
  case FMISSION_TYPE_NPC:
    return FES_GIMMICK_NPC_Delete( wk, (FES_GIMMICK_NPC_WORK*)gmk_wk );
  case FMISSION_TYPE_EVENT:
    return FES_GIMMICK_EVENT_Delete( wk );
  }
  return FALSE;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック ミッションタイプチェック
 */
//----------------------------------------------------------------------
BOOL FES_GIMMICK_MissionTypeCheck( FES_GIMMICK* wk, FMISSION_TYPE_ID type )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return FALSE;
  }
  return( wk->fmwk->arc.m_type == type );
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック イベントチェック
 *
 *  @param  wk
 *  @param  cp_pos  プレイヤー座標
 */
//----------------------------------------------------------------------
GMEVENT* FES_GIMMICK_CheckPosEventGrid( FES_GIMMICK* wk, const VecFx32* cp_pos )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return NULL;
  }
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ){
    return FES_GIMMICK_ITEM_CheckEventForGrid( wk, cp_pos );
  }
  return NULL;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック イベントチェック
 *
 *  @param  wk
 *  @param  cp_location  プレイヤー座標
 */
//----------------------------------------------------------------------
GMEVENT* FES_GIMMICK_CheckPosEventRail( FES_GIMMICK* wk, const RAIL_LOCATION* cp_location )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return NULL;
  }
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ){
    return FES_GIMMICK_ITEM_CheckEventForRail( wk, cp_location );
  }
  return NULL;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック との当たり判定
 *
 *  @param  wk
 *  @param  cp_location  プレイヤー座標
 */
//----------------------------------------------------------------------
BOOL FES_GIMMICK_CheckHitGridPos( FES_GIMMICK* wk, u16 gx, u16 gz, fx32 height )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return FALSE;
  }
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ){
    return FES_GIMMICK_ITEM_CheckHitGridPos( wk, gx, gz, height );
  }
  return FALSE;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　フラグセット
 */
//----------------------------------------------------------------------
void FES_GIMMICK_FlagSet( FES_GIMMICK* wk, u16 zone_id, u16 idx )
{
  void* gmk_wk = LINK_FESTIVAL_GetMissionGimmickWork( wk->fest_sys );
  
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return;
  }
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_NPC ){
    FES_GIMMICK_NPC_FlagSet( wk, (FES_GIMMICK_NPC_WORK*)gmk_wk, zone_id, idx );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　OBJタイプ取得(NPCミッション専用)
 *
 *  NPCミッション以外で呼び出すと、必ず0が返ります
 */
//----------------------------------------------------------------------
u8 FES_GIMMICK_ObjTypeGetForNpcMission( FES_GIMMICK* wk )
{
  if( !FES_GIMMICK_MissionTypeCheck( wk, FMISSION_TYPE_NPC ) ){
    GF_ASSERT(0);
    return 0;
  }
  return FES_GIMMICK_NPC_SUB_GetObjType( wk->fmwk );
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　フェストレーナーデータ書き換え(NPCミッション専用)
 */
//----------------------------------------------------------------------
void FES_GIMMICK_FesTrainerPartySetForNpcMission( FES_GIMMICK* wk, POKEPARTY* party )
{
  if( !FES_GIMMICK_MissionTypeCheck( wk, FMISSION_TYPE_NPC ) ){
    GF_ASSERT(0);
    return;
  }
  FES_GIMMICK_NPC_FesTrainerPartySet( wk, party );
}

//----------------------------------------------------------------------------
/**
 *	@brief  隠し穴調査（特殊ポケモン配置版）に選ばれているか調べる
 *
 *	@param	wk 
 *	@param	trail_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL FES_GIMMICK_IsExtraAnimalTrailMission( FES_GIMMICK* wk ,u8 trail_id )
{
  // フェスミッションタイプチェック
  if( FES_GIMMICK_MissionTypeCheck( wk, FMISSION_TYPE_EVENT ) ){
    // 隠し穴大調査EXかチェック
    if( FES_GIMMICK_EVENT_IsExAnimalTrailMission( wk ) )
    {
      if( FES_GIMMICK_EVENT_GetExAnimalTrailDataID( wk, trail_id ) != FES_ATRAIL_REPLACE_NUM )
      {
        return TRUE;
      }
    }
  }
  return FALSE;
}

//=======================================================================
//以下デバッグルーチン
#ifdef PM_DEBUG
//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック ポジションチェック
 */
//----------------------------------------------------------------------
int DEB_FES_GIMMICK_PosNumGet( FES_GIMMICK* wk )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return 0;
  }
  switch( wk->fmwk->arc.m_type ){
  case FMISSION_TYPE_ITEM:
    return DEB_FES_GIMMICK_ITEM_PosNumGet( wk, wk->wk_buf );
  case FMISSION_TYPE_NPC:
    return DEB_FES_GIMMICK_NPC_PosNumGet( wk, wk->wk_buf );
  }
  return 0;
}
//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック ポジションチェック
 */
//----------------------------------------------------------------------
BOOL DEB_FES_GIMMICK_PosGet( FES_GIMMICK* wk, int pos_idx, VecFx32* vec, int* param )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return FALSE;
  }
  switch( wk->fmwk->arc.m_type ){
  case FMISSION_TYPE_ITEM:
    return DEB_FES_GIMMICK_ITEM_PosGet( wk, wk->wk_buf, pos_idx, vec, param );
  case FMISSION_TYPE_NPC:
    return DEB_FES_GIMMICK_NPC_PosGet( wk, wk->wk_buf, pos_idx, vec, param );
  }
  return FALSE;
}
#endif  //PM_DEBUG

