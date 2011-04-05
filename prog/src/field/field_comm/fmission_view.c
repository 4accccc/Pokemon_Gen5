//==============================================================================
/**
 * @file    beacon_view.c
 * @brief   すれ違い通信状態参照画面
 * @author  iwasawa 
 * @date    2009.12.13(日)
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
#include "field/fmission_view.h"
#include "print\printsys.h"
#include "message.naix"
#include "field/event_beacon_detail.h"
#include "field/event_subscreen.h"
#include "field/event_gpower.h"
#include "field/event_freeword_input.h"
#include "field/event_festmission.h"

#include "beacon_status.naix"
#include "fmission_log.naix"
#include "wifi_unionobj.naix"
#include "wmi.naix"
#include "fmission_view_local.h"
#include "fmission_view_sub.h"

#ifdef PM_DEBUG
#include "field/event_debug_livecomm.h"
#endif  //PM_DEBUG



//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static int seq_Main( FMISSION_VIEW_PTR wk );
static int seq_LogEntry( FMISSION_VIEW_PTR wk );
static int seq_ViewUpdate( FMISSION_VIEW_PTR wk );
static int seq_ListScrollRepeat( FMISSION_VIEW_PTR wk );
static int seq_Retire( FMISSION_VIEW_PTR wk );
static int seq_GPowerUse( FMISSION_VIEW_PTR wk );

static void sub_SetViewPassive( FMISSION_VIEW_PTR wk, BOOL passive_f );

static void debug_InputCheck(FMISSION_VIEW_PTR view);

//==============================================================================
//  データ
//==============================================================================
#ifdef PM_DEBUG
///タッチ範囲テーブル
static const GFL_UI_TP_HITTBL TouchRect[] = {
  {//Gパワー
    0x15*8,
    0x18*8,
    0*8,
    6*8,
  },
  {//おめでとう
    0x15*8,
    0x18*8,
    6*8,
    12*8,
  },
  {//戻る
    0x15*8,
    0x18*8,
    (0x20-3)*8,
    0x20*8,
  },
  { GFL_UI_TP_HIT_END, 0, 0, 0 },
};
#endif  //PM_DEBUG

//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * フェスミッションログ画面：初期設定
 *
 * @param   gsys		
 *
 * @retval  FMISSION_VIEW_PTR		
 */
//==================================================================
FMISSION_VIEW_PTR FMISSION_VIEW_Init(GAMESYS_WORK *gsys, FIELD_SUBSCREEN_WORK *subscreen)
{
  FMISSION_VIEW* wk;
  
  wk = FVS_InitWork( gsys,subscreen );

  //初期描画
  wk->init_f = TRUE;
  FMissionView_InitialDraw( wk );
  wk->init_f = FALSE;
  
  wk->active = FALSE;  //無効値で初期化
  sub_SetViewPassive( wk, TRUE );

  return wk;
}

//==================================================================
/**
 * フェスミッションログ画面：破棄設定
 *
 * @param   view		
 */
//==================================================================
void FMISSION_VIEW_Exit(FMISSION_VIEW_PTR wk)
{
  FVS_ReleaseWork( wk );
}

//==================================================================
/**
 * フェスミッションログ画面：更新
 *
 * @param   view		
 */
//==================================================================
void FMISSION_VIEW_Update(FMISSION_VIEW_PTR wk, BOOL bActive )
{
  int i;
#ifdef PM_DEBUG
  int before_seq = wk->seq;
  OSTick s_tick = OS_GetTick();
#endif
  if( wk->active != bActive ){
    if( (wk->event_reserve_f && !bActive) == FALSE ){ //イベント移行アニメ待ち中はパッシブにしない
      wk->active = bActive;
      sub_SetViewPassive( wk, !bActive );
    }
  }
  if( wk->event_id != EV_NONE ){
    if(!bActive){
      //リクエストを強制破棄
      FVSEvent_ReserveReset( wk );
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
  case SEQ_LIST_SCROLL_REPEAT:
    wk->seq = seq_ListScrollRepeat( wk );
    break;
  case SEQ_RETIRE:
    wk->seq = seq_Retire( wk );
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
#endif
}

//==================================================================
/**
 * フェスミッションログ画面：描画
 *
 * @param   wk		
 */
//==================================================================
void FMISSION_VIEW_Draw(FMISSION_VIEW_PTR wk)
{
#ifdef PM_DEBUG
  OSTick s_tick,e_tick,t_tick;  // = OS_GetTick();
  s_tick = OS_GetTick();
#endif
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
 * フェスミッションログ画面：イベントチェック
 *
 * @param   wk		
 */
//==================================================================
GMEVENT* FMISSION_VIEW_EventCheck(FMISSION_VIEW_PTR wk, BOOL bEvReqOK )
{
  return FVSEvent_CheckCommon( wk, bEvReqOK );
}

/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

/*
 *  @brief  メイン　待機
 */
static int seq_Main( FMISSION_VIEW_PTR wk )
{
  int ret;
 
  //描画更新
  FMissionView_ViewUpdate( wk );
 
  //特殊ポップアップ起動チェック
  if( FMissionView_CheckSpecialPopup( wk ) ){
    wk->io_interval = 30*3; //インターバルを設定
    return SEQ_VIEW_UPDATE;
  }
#if 0
  //通信関連特殊イベント発動チェック
  if( FMissionView_CheckCommEvent( wk )){
    return SEQ_MAIN;
  }
#endif

  //メイン入力チェック
  ret = FMissionView_CheckInput( wk );
  if( ret != SEQ_MAIN ){
    return ret;
  }

  if( wk->io_interval ){
    wk->io_interval--;
    return SEQ_MAIN;
  }
#if 0
  //特殊Gパワー起動チェック
  if( FMissionView_CheckSpecialGPower( wk ) == TRUE ){
    wk->io_interval = 30*3; //インターバルを設定
    return SEQ_VIEW_UPDATE;
  }
#endif
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
static int seq_LogEntry( FMISSION_VIEW_PTR wk )
{
  if( !FMissionView_SubSeqLogEntry( wk )){
    return SEQ_LOG_ENTRY;
  }
  wk->sub_seq = 0;
  return SEQ_VIEW_UPDATE;
}

/*
 *  @brief  パネル更新中
 */
static int seq_ViewUpdate( FMISSION_VIEW_PTR wk )
{
  if( wk->eff_task_ct ){
    return SEQ_VIEW_UPDATE;
  }
  FMissionView_MenuBarViewSet( wk, MENU_ALL, MENU_ST_ON );
//  FMissionView_UpDownViewSet( wk );
  return SEQ_MAIN;
}

/*
 *  @brief  リスト連続スクロール中
 */
static int seq_ListScrollRepeat( FMISSION_VIEW_PTR wk )
{
  int ret;

  if( !wk->scr_repeat_end ){
    ret = FMissionView_CheckInoutTouchUpDown( wk );
    if( ret != wk->scr_repeat_dir || ret == GFL_UI_TP_HIT_NONE ){
      wk->scr_repeat_end = TRUE;
    }
    wk->scr_repeat_ct++;  //押しっぱなしカウンタアップ
  }

  //最後のスクロール待ち
  if( wk->scr_repeat_end ){
    if( wk->eff_task_ct > 0 ){
      return SEQ_LIST_SCROLL_REPEAT;
    }
    wk->scr_repeat_end = FALSE;
    wk->scr_repeat_ct = 0;
    wk->scr_repeat_f = FALSE;
    FMissionView_MenuBarViewSet( wk, MENU_ALL, MENU_ST_ON );
//    FMissionView_UpDownViewSet( wk );
    BVC_PrintQueLimmitUpSet( wk->printQue, FALSE );
    return wk->scr_repeat_ret_seq;
  }

  //2回目以降のスクロールリクエスト
  if( wk->eff_task_ct == 0 ){
    FMissionView_ListScrollRepeatReq( wk );
  }
  return SEQ_LIST_SCROLL_REPEAT;
}

/*
 *  @brief  ミッションリタイア確認
 */
static int seq_Retire( FMISSION_VIEW_PTR wk )
{
  if( !FMissionView_SubSeqRetire( wk )){
    return SEQ_RETIRE;
  }
  wk->sub_seq = 0;
  return SEQ_MAIN;
}

/*
 *  @brief  自分のGパワーを使用
 */
static int seq_GPowerUse( FMISSION_VIEW_PTR wk )
{
#if 0
  if( !FMissionView_SubSeqGPower( wk )){
    return SEQ_GPOWER_USE;
  }
#endif
  wk->sub_seq = 0;
  return SEQ_MAIN;
}

/*
 *  @brief  アクティブ・パッシブ切替
 */
static void sub_SetViewPassive( FMISSION_VIEW_PTR wk, BOOL passive_f )
{
  if(passive_f){
    SoftFadePfd( wk->pfd, FADE_SUB_BG, 0, 16*BG_PALANM_AREA, 6, 0x0000);
    SoftFadePfd( wk->pfd, FADE_SUB_OBJ, 0, 16*ACT_PAL_WMI, 6, 0x0000);
    FLD_VREQ_G2S_SetBlendAlpha( wk->fld_vreq, ALPHA_1ST_PASSIVE, ALPHA_2ND, ALPHA_EV1_PASSIVE, ALPHA_EV2_PASSIVE);
  }else{
    SoftFadePfd( wk->pfd, FADE_SUB_BG, 0, 16*BG_PALANM_AREA, 0, 0x0000);
    SoftFadePfd( wk->pfd, FADE_SUB_OBJ, 0, 16*ACT_PAL_WMI, 0, 0x0000);
    FLD_VREQ_G2S_SetBlendAlpha( wk->fld_vreq, ALPHA_1ST_NORMAL, ALPHA_2ND, ALPHA_EV1_NORMAL, ALPHA_EV2_NORMAL);
  }
}

#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * タッチ判定
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void debug_InputCheck(FMISSION_VIEW_PTR wk)
{
  u32 tp_x, tp_y;
  int trg =  GFL_UI_KEY_GetTrg();
  int cont =  GFL_UI_KEY_GetCont();
  int tp_ret = GFL_UI_TP_HitTrg(TouchRect);

  if( wk->event_id != EV_NONE ){
    return;
  }

#ifdef DEBUG_ONLY_FOR_iwasawa
  switch(tp_ret){
  case 0: //
    OS_TPrintf("ランダム ビーコンセット\n");
    DEBUG_GAMEBEACON_Set_Random();
//    GAMEBEACON_Set_GPower( 1 );
    break;
  case 1: //おめでとう
    OS_TPrintf("ビーコンセット\n");
//    GAMEBEACON_Set_SpecialPokemonGet( 1 );
//    GAMEBEACON_Set_BattleSpTrainerVictory(u16 tr_no);
    GAMEBEACON_Set_Subway();

//    GAMEBEACON_Set_Thankyou( wk->gdata, 0x12345678 );
    break;
  case 2: //戻る
    break;
  }
#endif
}

#endif  //PM_DEBUG


