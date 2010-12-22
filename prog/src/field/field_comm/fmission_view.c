//==============================================================================
/**
 * @file    fmission_view.c
 * @brief   フェスミッション下画面
 * @author  iwasawa 
 * @date    2010.12.21
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/bmp_oam.h"
#include "gamesystem/gamesystem.h"
#include "field/field_subscreen.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_debug.h"
#include "gamesystem/msgspeed.h"
#include "infowin/infowin.h"
#include "app_menu_common.naix"
#include "arc_def.h"
#include "font/font.naix"
#include "field/beacon_view.h"
#include "print\printsys.h"
#include "message.naix"
#include "field/event_beacon_detail.h"
#include "field/event_subscreen.h"
#include "field/event_gpower.h"
#include "field/event_freeword_input.h"
#include "field/event_festmission.h"

#include "beacon_status.naix"
#include "wifi_unionobj.naix"
#include "wmi.naix"
#include "beacon_view_local.h"
#include "beacon_view_common.h"
#include "fmission_view_sub.h"
#include "beacon_view_sub.h"

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static int seq_Main( BEACON_VIEW_PTR wk );
static int seq_LogEntry( BEACON_VIEW_PTR wk );
static int seq_ViewUpdate( BEACON_VIEW_PTR wk );
static int seq_GPowerUse( BEACON_VIEW_PTR wk );

//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * フェスミッション画面：初期設定
 *
 * @param   gsys		
 *
 * @retval  BEACON_VIEW_PTR		
 */
//==================================================================
BEACON_VIEW_PTR FMISSION_VIEW_Init(GAMESYS_WORK *gsys, FIELD_SUBSCREEN_WORK *subscreen)
{
  BEACON_VIEW* wk;
  
  wk = BVS_InitWork(gsys,subscreen, VIEW_MODE_FMISSION );

  //初期描画
  wk->init_f = TRUE;
  FMissionView_InitialDraw( wk );
  wk->init_f = FALSE;
  
  wk->active = FALSE;  //無効値で初期化
  BVC_SetViewPassive( wk, TRUE );

#ifdef PM_DEBUG
  //スルーフラグが立っている状態で初期化された場合、最初の一回のアップデートのみする
  if(DEBUG_BEACON_STATUS_GetStackCheckThrowFlag(wk->b_status) == TRUE ){
    GAMEBEACON_Stack_Update( wk->infoStack );
  }
#endif  //PM_DEBUG
  return wk;
}

//==================================================================
/**
 * フェスミッション画面：破棄設定
 *
 * @param   view		
 */
//==================================================================
void FMISSION_VIEW_Exit(BEACON_VIEW_PTR wk)
{
  BVS_ReleaseWork( wk );
}

//==================================================================
/**
 * フェスミッション画面：更新
 *
 * @param   view		
 */
//==================================================================
void FMISSION_VIEW_Update(BEACON_VIEW_PTR wk, BOOL bActive )
{
  int i;
#ifdef PM_DEBUG
  int before_seq = wk->seq;
  OSTick s_tick = OS_GetTick();
#endif
  if( wk->active != bActive ){
    if( (wk->event_reserve_f && !bActive) == FALSE ){ //イベント移行アニメ待ち中はパッシブにしない
      wk->active = bActive;
      BVC_SetViewPassive( wk, !bActive );
    }
  }
  if( wk->event_id != EV_NONE ){
    if(!bActive){
      //リクエストを強制破棄
      BVSEvent_ReserveReset( wk );
    }
    return; //イベントリクエスト中はメイン処理をスキップ
  }
  if(!bActive){
    return;
  }
  GFL_TCBL_Main( wk->pTcbSys );
  
  switch( wk->seq ){
  case SEQ_MAIN:
    wk->seq = seq_Main( wk );
    break;
  case SEQ_LOG_ENTRY:
    wk->seq = seq_LogEntry( wk );
    break;
  case SEQ_VIEW_UPDATE:
    wk->seq = seq_ViewUpdate( wk );
    break;
  case SEQ_GPOWER_USE:
    wk->seq = seq_GPowerUse( wk );
    break;
  case SEQ_EV_REQ_WAIT:
  case SEQ_END:
  default:
    //外部リクエストによる終了待ち
    break;
  }
#ifdef PM_DEBUG
#if 0
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
    OSTick tick = OS_GetTick()-s_tick;
    IWASAWA_Printf("BeaconView seq = %d tick = %d\n", before_seq, tick );
  }
#endif
  if(DEBUG_BEACON_STATUS_GetStackCheckThrowFlag(wk->b_status) == FALSE ){
    GAMEBEACON_Stack_Update( wk->infoStack );
  }
#else
  //スタックテーブル更新
  GAMEBEACON_Stack_Update( wk->infoStack );
#endif
}

//==================================================================
/**
 * フェスミッション画面：描画
 *
 * @param   wk		
 */
//==================================================================
void FMISSION_VIEW_Draw(BEACON_VIEW_PTR wk)
{
#ifdef PM_DEBUG
  OSTick s_tick,e_tick,t_tick;  // = OS_GetTick();
  s_tick = OS_GetTick();
#endif
  FMissionView_ViewUpdate( wk );
  PRINTSYS_QUE_Main( wk->printQue );

#ifdef PM_DEBUG
  if( !wk->active ){
    return;
  }
#if 0
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
    e_tick = OS_GetTick();
    t_tick = e_tick-s_tick;
    IWASAWA_Printf("BeaconView Draw tick %d\n",  t_tick );
  }
  debug_InputCheck( wk );
#endif
#endif
}

//==================================================================
/**
 * フェスミッション画面：イベントチェック
 *
 * @param   wk		
 */
//==================================================================
GMEVENT* FMISSION_VIEW_EventCheck(BEACON_VIEW_PTR wk, BOOL bEvReqOK )
{
  return BVSEvent_CheckCommon( wk, bEvReqOK );
}

/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

/*
 *  @brief  メイン　待機
 */
static int seq_Main( BEACON_VIEW_PTR wk )
{
  int ret;
 
  //特殊ポップアップ起動チェック
  if( BVC_CheckSpecialPopup( wk ) ){
    wk->io_interval = 30*3; //インターバルを設定
    return SEQ_VIEW_UPDATE;
  }
 
  //通信関連特殊イベント発動チェック
  if( BeaconView_CheckCommEvent( wk )){
    return SEQ_MAIN;
  }
 
  //メイン入力チェック
  ret = FMissionView_CheckInput( wk );
  if( ret != SEQ_MAIN ){
    return ret;
  }

  //スタックチェック
  if( FMissionView_CheckStack( wk ) == FALSE ){
    return SEQ_MAIN;
  }
  wk->io_interval = 30*3; //インターバルを設定
  return SEQ_LOG_ENTRY;
}

/*
 *  @brief  Newログのエントリー
 */
static int seq_LogEntry( BEACON_VIEW_PTR wk )
{
  if( !FMissionView_SubSeqLogEntry( wk )){
    return SEQ_LOG_ENTRY;
  }
  wk->sub_seq = 0;
  return SEQ_VIEW_UPDATE;
}

/*
 *  @brief  リスト更新中
 */
static int seq_ViewUpdate( BEACON_VIEW_PTR wk )
{
  if( wk->eff_task_ct ){
    return SEQ_VIEW_UPDATE;
  }
  BVC_MenuBarViewSet( wk, MENU_ALL, MENU_ST_ON );
  return SEQ_MAIN;
}

/*
 *  @brief  自分のGパワーを使用
 */
static int seq_GPowerUse( BEACON_VIEW_PTR wk )
{
  if( !BVCSubSeq_GPower( wk )){
    return SEQ_GPOWER_USE;
  }
  wk->sub_seq = 0;
  return SEQ_MAIN;
}

