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
#include "field/beacon_view.h"
#include "print\printsys.h"
#include "message.naix"
#include "field/event_beacon_detail.h"
#include "field/event_subscreen.h"
#include "field/event_gpower.h"
#include "field/event_freeword_input.h"

#include "beacon_status.naix"
#include "wifi_unionobj.naix"
#include "wmi.naix"
#include "beacon_view_local.h"
#include "beacon_view_sub.h"

#ifdef PM_DEBUG
#include "field/event_debug_livecomm.h"
#endif  //PM_DEBUG

//==============================================================================
//  定数定義
//==============================================================================
typedef enum{
  EVWAIT_MENU_ANM,
  EVWAIT_TCB_ANM,
  EVWAIT_TMENU_ANM,
}EVWAIT_TYPE;

typedef struct _EVWK_EVENT_START{
  int   sub_seq;
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap;
  BEACON_VIEW_PTR bvp;
  GMEVENT*  call_event;

  EVWAIT_TYPE type;
  int param;
}EVWK_EVENT_START;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT* event_EventStart( BEACON_VIEW_PTR wk, GMEVENT* call_event, EVWAIT_TYPE type, int param );

static int seq_Main( BEACON_VIEW_PTR wk );
static int seq_LogEntry( BEACON_VIEW_PTR wk );
static int seq_ViewUpdate( BEACON_VIEW_PTR wk );
static int seq_ListScrollRepeat( BEACON_VIEW_PTR wk );
static int seq_GPowerUse( BEACON_VIEW_PTR wk );
static int seq_ThankYou( BEACON_VIEW_PTR wk );

static void debug_InputCheck(BEACON_VIEW_PTR view);
static void _sub_DataSetup(BEACON_VIEW_PTR wk);
static void _sub_DataExit(BEACON_VIEW_PTR wk);
static void _sub_SystemSetup(BEACON_VIEW_PTR view);
static void _sub_SystemExit(BEACON_VIEW_PTR view);
static void _sub_BGLoad(BEACON_VIEW_PTR view, ARCHANDLE *handle);
static void _sub_BGUnload(BEACON_VIEW_PTR view);
static void _sub_TMenuInit( BEACON_VIEW_PTR wk );
static void _sub_TMenuRelease( BEACON_VIEW_PTR wk );
static void _sub_ActorResourceLoad(BEACON_VIEW_PTR view, ARCHANDLE *handle);
static void _sub_ActorResourceUnload(BEACON_VIEW_PTR view);
static void _sub_ActorCreate(BEACON_VIEW_PTR view, ARCHANDLE *handle);
static void _sub_ActorDelete(BEACON_VIEW_PTR view);
static void _sub_BmpWinCreate(BEACON_VIEW_PTR view);
static void _sub_BmpWinDelete(BEACON_VIEW_PTR view);

static void obj_ObjResInit( BEACON_VIEW_PTR wk, OBJ_RES_TBL* res, const OBJ_RES_SRC* srcTbl, ARCHANDLE* p_handle );
static void obj_ObjResRelease( BEACON_VIEW_PTR wk, OBJ_RES_TBL* res );
static void act_NormalObjAdd( BEACON_VIEW_PTR wk ) ;
static void act_NormalObjDel( BEACON_VIEW_PTR wk ) ;
static void act_PanelObjAdd( BEACON_VIEW_PTR wk, u8 idx );
static void act_PanelObjDel( BEACON_VIEW_PTR wk, u8 idx );

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
 * すれ違い参照画面：初期設定
 *
 * @param   gsys		
 *
 * @retval  BEACON_VIEW_PTR		
 */
//==================================================================
BEACON_VIEW_PTR BEACON_VIEW_Init(GAMESYS_WORK *gsys, FIELD_SUBSCREEN_WORK *subscreen)
{
  BEACON_VIEW* wk;
  
  wk = GFL_HEAP_AllocClearMemory(HEAPID_FIELD_SUBSCREEN, sizeof(BEACON_VIEW));
  wk->gsys = gsys;
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
  wk->fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  wk->fld_vreq = FIELDMAP_GetFldVReq(wk->fieldWork);
  wk->subscreen = subscreen;
 
  wk->heap_sID = HEAPID_FIELD_SUBSCREEN;
  wk->heapID = HEAPID_FIELDMAP;
  wk->tmpHeapID = GFL_HEAP_LOWID( HEAPID_FIELD_SUBSCREEN );

  wk->arc_handle = GFL_ARC_OpenDataHandle(ARCID_BEACON_STATUS, wk->heap_sID);

  _sub_DataSetup(wk);
  _sub_SystemSetup(wk);
  _sub_BGLoad(wk, wk->arc_handle);
  _sub_TMenuInit( wk );
  _sub_ActorResourceLoad(wk, wk->arc_handle);
  _sub_ActorCreate(wk, wk->arc_handle);
  _sub_BmpWinCreate(wk);
 
  //初期描画
  wk->init_f = TRUE;
  BeaconView_InitialDraw( wk );
  wk->init_f = FALSE;
  
  wk->active = FALSE;  //無効値で初期化
  BeaconView_SetViewPassive( wk, TRUE );

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
 * すれ違い参照画面：破棄設定
 *
 * @param   view		
 */
//==================================================================
void BEACON_VIEW_Exit(BEACON_VIEW_PTR wk)
{
  //今生きている全てのタスクを削除
  GFL_TCBL_DeleteAll( wk->pTcbSys );
  
  //通信アイコン位置指定解除
//  GFL_NET_WirelessIconEasy_DefaultLCD();

  _sub_BmpWinDelete( wk );
  _sub_ActorDelete( wk );
  _sub_ActorResourceUnload( wk );
  _sub_TMenuRelease( wk );
  _sub_BGUnload( wk );
  _sub_SystemExit( wk );
  _sub_DataExit(wk);
  
  GFL_ARC_CloseDataHandle(wk->arc_handle);


  GFL_HEAP_FreeMemory( wk );
}

//==================================================================
/**
 * すれ違い参照画面：更新
 *
 * @param   view		
 */
//==================================================================
void BEACON_VIEW_Update(BEACON_VIEW_PTR wk, BOOL bActive )
{
  int i;
#ifdef PM_DEBUG
  int before_seq = wk->seq;
  OSTick s_tick = OS_GetTick();
#endif
  if( wk->active != bActive ){
    if( (wk->event_reserve_f && !bActive) == FALSE ){ //イベント移行アニメ待ち中はパッシブにしない
      wk->active = bActive;
      BeaconView_SetViewPassive( wk, !bActive );
    }
  }
  if( wk->event_id != EV_NONE ){
    if(!bActive){
      //リクエストを強制破棄
      BEACON_VIEW_SUB_EventReserveReset( wk );
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
  case SEQ_GPOWER_USE:
    wk->seq = seq_GPowerUse( wk );
    break;
  case SEQ_THANK_YOU:
    wk->seq = seq_ThankYou( wk );
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
 * すれ違い参照画面：描画
 *
 * @param   wk		
 */
//==================================================================
void BEACON_VIEW_Draw(BEACON_VIEW_PTR wk)
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
 * すれ違い参照画面：入力取得
 *
 * @param   wk		
 */
//==================================================================
GMEVENT* BEACON_VIEW_EventCheck(BEACON_VIEW_PTR wk, BOOL bEvReqOK )
{
  GMEVENT* event = NULL;
  GMEVENT* call_event = NULL;

  if( !bEvReqOK ){  //イベント起動していいタイミングを待つ
    return NULL;
  }

  switch( wk->event_id ){
  case EV_RETURN_CGEAR_INTRUDE:
    Intrude_SetLiveCommStatus_IntrudeOut( wk->game_comm );
    //ブレイクスルー
  case EV_RETURN_CGEAR:
  case EV_RETURN_CGEAR_WIRELESS_TR:

    BEACON_STATUS_SetViewTopOffset( wk->b_status, 0 );
    call_event = EVENT_ChangeSubScreen( wk->gsys, wk->fieldWork, FIELD_SUBSCREEN_NORMAL);
    event = event_EventStart( wk, call_event, EVWAIT_MENU_ANM, MENU_RETURN );
    break;
  case EV_CALL_DETAIL_VIEW:
    call_event = EVENT_BeaconDetail( wk->gsys, wk->fieldWork, wk->ctrl.target+wk->ctrl.view_top );
    event = event_EventStart( wk, call_event, EVWAIT_TCB_ANM, 0 );
    break;
  case EV_GPOWER_USE:
    {
      GPOWER_EFFECT_PARAM param;
      param.g_power = wk->ctrl.g_power;
      param.mine_f = wk->ctrl.mine_power_f;
      call_event = GMEVENT_CreateOverlayEventCall( wk->gsys,
          FS_OVERLAY_ID( event_gpower ), EVENT_GPowerEffectStart, &param );
    }
    event = event_EventStart( wk, call_event, EVWAIT_TMENU_ANM, (int)(wk->tmenu[TMENU_YN_YES].work) );
    wk->ctrl.g_power = GPOWER_ID_NULL;
    break;
  case EV_GPOWER_CHECK_TMENU_YN:
    call_event = GMEVENT_CreateOverlayEventCall( wk->gsys,
        FS_OVERLAY_ID( event_gpower), EVENT_GPowerEnableListCheck, wk->fieldWork );
    event = event_EventStart( wk, call_event, EVWAIT_TMENU_ANM, (int)(wk->tmenu[TMENU_YN_CHECK].work) );
    break;
  case EV_GPOWER_CHECK_TMENU_CHK:
    call_event = GMEVENT_CreateOverlayEventCall( wk->gsys,
        FS_OVERLAY_ID( event_gpower), EVENT_GPowerEnableListCheck, wk->fieldWork );
    event = event_EventStart( wk, call_event, EVWAIT_TMENU_ANM, (int)(wk->tmenu_check[TMENU_CHECK_CALL].work) );
    wk->gpower_check_req = FALSE;
    break;
  case EV_CALL_TALKMSG_INPUT:
    call_event = EVENT_FreeWordInput( wk->gsys, wk->fieldWork, NULL,
              NAMEIN_FREE_WORD, BEACON_STATUS_GetFreeWordInputResultPointer( wk->b_status ) );
    event = event_EventStart( wk, call_event, EVWAIT_MENU_ANM, MENU_HELLO );
    break;

#ifdef PM_DEBUG
  //デバッグメニューコールリクエスト
  case EV_DEBUG_MENU_CALL:
    event = GMEVENT_CreateOverlayEventCall( wk->gsys,
              FS_OVERLAY_ID( d_livecomm ), DEBUG_EVENT_LiveComm, wk );
    break;
#endif  //PM_DEBUG

  default:
    return NULL;
  }
  BEACON_VIEW_SUB_EventReserveReset( wk );
  return event; 
}

/*
 *  @brief  イベントリクエスト
 */
void BEACON_VIEW_SUB_EventReserve( BEACON_VIEW_PTR wk, BEACON_DETAIL_EVENT ev_id)
{
  wk->event_id = ev_id;
}

/*
 *  @brief  イベントリクエストリセット
 */
void BEACON_VIEW_SUB_EventReserveReset( BEACON_VIEW_PTR wk )
{
  wk->event_id = EV_NONE;
}

////////////////////////////////////////////////////////////////////////////
//アニメウェイトイベント
////////////////////////////////////////////////////////////////////////////
static GMEVENT_RESULT event_EventStartMain(GMEVENT * event, int *  seq, void * work);
static BOOL evsub_WaitTypeMenuAnime( BEACON_VIEW_PTR wk, EVWK_EVENT_START* ev_wk );
static BOOL evsub_WaitTypeTcb( BEACON_VIEW_PTR wk, EVWK_EVENT_START* ev_wk );
static BOOL evsub_WaitTypeTMenuAnm( BEACON_VIEW_PTR wk, EVWK_EVENT_START* ev_wk );

/*
 *  @brief  各種イベント起動前に、アニメなどの待ちをするためのイベント
 */
GMEVENT* event_EventStart( BEACON_VIEW_PTR wk, GMEVENT* call_event, EVWAIT_TYPE type, int param )
{
  GMEVENT* event;
  EVWK_EVENT_START* ev_wk;

  event = GMEVENT_Create(wk->gsys, NULL, event_EventStartMain, sizeof(EVWK_EVENT_START));
  ev_wk = GMEVENT_GetEventWork(event);

  ev_wk->bvp = wk;
  ev_wk->gsys = wk->gsys;
  ev_wk->fieldmap = wk->fieldWork;
  ev_wk->call_event = call_event;

  ev_wk->type  = type;
  ev_wk->param = param;

  wk->event_reserve_f = TRUE;

  return event;
}

static GMEVENT_RESULT event_EventStartMain(GMEVENT * event, int *  seq, void * work)
{
  EVWK_EVENT_START* ev_wk = work;
  BEACON_VIEW_PTR wk = ev_wk->bvp;
  int ret = FALSE;

  switch( ev_wk->type ){
  case EVWAIT_MENU_ANM:
    ret = evsub_WaitTypeMenuAnime( wk, ev_wk );
    break;
  case EVWAIT_TCB_ANM:
    ret = evsub_WaitTypeTcb( wk, ev_wk );
    break;
  case EVWAIT_TMENU_ANM:
    ret = evsub_WaitTypeTMenuAnm( wk, ev_wk );
    break;
  }
  if( ret ){
    wk->event_reserve_f = FALSE;
    GMEVENT_ChangeEvent( event, ev_wk->call_event );
  }
  return GMEVENT_RES_CONTINUE;
}

static BOOL evsub_WaitTypeMenuAnime( BEACON_VIEW_PTR wk, EVWK_EVENT_START* ev_wk )
{
  switch( ev_wk->sub_seq ){
  case 0:
    BeaconView_MenuBarViewSet( wk, ev_wk->param, MENU_ST_ANM );
    ev_wk->sub_seq++;
    return FALSE;
  case 1:
    if( BeaconView_MenuBarCheckAnm( wk, ev_wk->param )){
      return FALSE;
    }
    break;
  }
  return TRUE;
}

static BOOL evsub_WaitTypeTcb( BEACON_VIEW_PTR wk, EVWK_EVENT_START* ev_wk )
{
  GFL_TCBL_Main( wk->pTcbSys );
  return (wk->eff_task_ct == 0 );
}

static BOOL evsub_WaitTypeTMenuAnm( BEACON_VIEW_PTR wk, EVWK_EVENT_START* ev_wk )
{
  APP_TASKMENU_WIN_WORK *tmenu_work = (APP_TASKMENU_WIN_WORK*)ev_wk->param;   

  switch(ev_wk->sub_seq){
  case 0:
    APP_TASKMENU_WIN_SetDecide( tmenu_work, TRUE );
    ev_wk->sub_seq++;
    break;
  case 1:
    APP_TASKMENU_WIN_Update( tmenu_work );

    if( APP_TASKMENU_WIN_IsFinish( tmenu_work )){
      APP_TASKMENU_WIN_ResetDecide( tmenu_work );
      return TRUE;
    }
    break;
  }
  return FALSE;
}

/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////
static void tcb_VInter( GFL_TCB* tcb, void * work)
{
  BEACON_VIEW_PTR wk = (BEACON_VIEW_PTR)work;

  if( wk->pfd != NULL){
    PaletteFadeTrans( wk->pfd );
  }
}


/*
 *  @brief  メイン　待機
 */
static int seq_Main( BEACON_VIEW_PTR wk )
{
  int ret;
 
  //特殊ポップアップ起動チェック
  if( BeaconView_CheckSpecialPopup( wk ) ){
    wk->io_interval = 30*3; //インターバルを設定
    return SEQ_VIEW_UPDATE;
  }
  
  //通信関連特殊イベント発動チェック
  if( BeaconView_CheckCommEvent( wk )){
    return SEQ_MAIN;
  }
 
  //メイン入力チェック
  ret = BeaconView_CheckInput( wk );
  if( ret != SEQ_MAIN ){
    return ret;
  }

  if( wk->io_interval ){
    wk->io_interval--;
    return SEQ_MAIN;
  }
 
  //特殊Gパワー起動チェック
  if( BeaconView_CheckSpecialGPower( wk ) == TRUE ){
    wk->io_interval = 30*3; //インターバルを設定
    return SEQ_VIEW_UPDATE;
  }
  //スタックチェック
  if( BeaconView_CheckStack( wk ) == FALSE ){
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
  if( !BeaconView_SubSeqLogEntry( wk )){
    return SEQ_LOG_ENTRY;
  }
  wk->sub_seq = 0;
  return SEQ_VIEW_UPDATE;
}

/*
 *  @brief  パネル更新中
 */
static int seq_ViewUpdate( BEACON_VIEW_PTR wk )
{
  if( wk->eff_task_ct ){
    return SEQ_VIEW_UPDATE;
  }
  BeaconView_MenuBarViewSet( wk, MENU_ALL, MENU_ST_ON );
  BeaconView_UpDownViewSet( wk );
  return SEQ_MAIN;
}

/*
 *  @brief  リスト連続スクロール中
 */
static int seq_ListScrollRepeat( BEACON_VIEW_PTR wk )
{
  int ret;
  
  if( !wk->scr_repeat_end ){
    ret = BeaconView_CheckInoutTouchUpDown( wk );
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
    if( wk->scr_repeat_ret_seq == SEQ_MAIN ){
      BeaconView_MenuBarViewSet( wk, MENU_ALL, MENU_ST_ON );
    }
    BeaconView_UpDownViewSet( wk );
    BeaconView_PrintQueLimmitUpSet( wk, FALSE );
    return wk->scr_repeat_ret_seq;
  }

  //2回目以降のスクロールリクエスト
  if( wk->eff_task_ct == 0 ){
    BeaconView_ListScrollRepeatReq( wk );
  }
  return SEQ_LIST_SCROLL_REPEAT;
}

/*
 *  @brief  自分のGパワーを使用
 */
static int seq_GPowerUse( BEACON_VIEW_PTR wk )
{
  if( !BeaconView_SubSeqGPower( wk )){
    return SEQ_GPOWER_USE;
  }
  wk->sub_seq = 0;
  return SEQ_MAIN;
}

/*
 *  @brief  御礼をする相手を選ぶ
 */
static int seq_ThankYou( BEACON_VIEW_PTR wk )
{
  return BeaconView_SubSeqThanks( wk );
}

#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * タッチ判定
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void debug_InputCheck(BEACON_VIEW_PTR wk)
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

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * データ関連のセットアップ
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_DataSetup(BEACON_VIEW_PTR wk)
{
  SAVE_CONTROL_WORK* save;

  wk->b_status = GAMEDATA_GetBeaconStatus( wk->gdata );

  wk->infoLog = BEACON_STATUS_GetInfoTbl( wk->b_status );

  wk->ctrl.max = GAMEBEACON_InfoTblRing_GetEntryNum( wk->infoLog );
  wk->ctrl.view_top = BEACON_STATUS_GetViewTopOffset( wk->b_status );

//  IWASAWA_Printf("BeaconViewSetup log_num = %d\n", wk->ctrl.max );
  if( wk->ctrl.max < PANEL_VIEW_MAX ){
    wk->ctrl.view_max = wk->ctrl.max;
  }else{
    wk->ctrl.view_max = PANEL_VIEW_MAX;
  }
  wk->ctrl.g_power = GPOWER_ID_NULL;

  //メッセージスピード取得
  /*
   *  アフターマスター ローカライズ版に向けてのコメント
   *
   *  wk->msg_spdメンバはu8だが、実際はs8以上のsigned型である必要がある。
   *  現状、msg_spdメンバが非参照のため、修正はせずコメントを残すのみとする
   *
   *  by iwasawa 20100806
   */
  wk->msg_spd  = MSGSPEED_GetWait();
 
  //セーブデータ取得
  save = GAMEDATA_GetSaveControlWork( wk->gdata ); 
  {
    INTRUDE_SAVE_WORK* int_sv = SaveData_GetIntrude(save);
    wk->my_data.power = ISC_SAVE_GetGPowerID(int_sv);
#ifdef DEBUG_ONLY_FOR_iwasawa
//    wk->my_data.power = 1;
#endif
//    wk->my_power_f = (wk->my_data.power == GPOWER_ID_NULL);
  }
  wk->game_comm = GAMESYSTEM_GetGameCommSysPtr( wk->gsys );
  wk->wifi_list = GAMEDATA_GetWiFiList( wk->gdata );
  
  wk->item_sv = GAMEDATA_GetMyItem( wk->gdata );
  wk->misc_sv = (MISC*)SaveData_GetMiscConst( save );
  wk->log_count = MISC_CrossComm_GetSuretigaiCount( wk->misc_sv );

  {
    MYSTATUS* my = SaveData_GetMyStatus(save);
    wk->my_data.tr_id = MyStatus_GetID( my ) & 0xFFFF;  //下位2byte
    wk->my_data.sex = MyStatus_GetMySex( my );
    wk->my_data.union_char = MyStatus_GetTrainerView( my );
    wk->my_data.pm_version = PM_VERSION;
  }
  
  //スタックワーク領域取得
  wk->infoStack = GAMEBEACON_InfoTbl_Alloc( wk->heap_sID );
  wk->tmpInfo = GAMEBEACON_Alloc( wk->heap_sID );
  wk->tmpInfo2 = GAMEBEACON_Alloc( wk->heap_sID );
  wk->newLogInfo = GAMEBEACON_Alloc( wk->heap_sID );

  //Gパワーデータ取得
  wk->gpower_data = GPOWER_PowerData_LoadAlloc( wk->heap_sID );

  //ログ内カウンター取得
  BeaconView_LogCounterGet( wk, &wk->old_sex_ct, &wk->old_version_ct);
}

//--------------------------------------------------------------
/**
 * データ関連の破棄＆終了処理
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_DataExit(BEACON_VIEW_PTR wk)
{
  GAMEBEACON_Stack_PutBack( wk->infoStack );

  GPOWER_PowerData_Unload( wk->gpower_data );

  GFL_HEAP_FreeMemory( wk->newLogInfo );
  GFL_HEAP_FreeMemory( wk->tmpInfo2 );
  GFL_HEAP_FreeMemory( wk->tmpInfo );
  GFL_HEAP_FreeMemory( wk->infoStack );
}

//--------------------------------------------------------------
/**
 * システム関連のセットアップ
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_SystemSetup(BEACON_VIEW_PTR wk)
{
  wk->pTcbSys = GFL_TCBL_Init( wk->heap_sID, wk->heap_sID, 16, 128 );

  //Vタスク追加
  wk->tcbVIntr = GFUser_VIntr_CreateTCB( tcb_VInter, (void*)wk, 0);

  wk->pfd = PaletteFadeInit( wk->heap_sID );
  PaletteFadeWorkAllocSet( wk->pfd, FADE_SUB_BG,  0x20*BG_PALANM_AREA, wk->heap_sID );
  PaletteFadeWorkAllocSet( wk->pfd, FADE_SUB_OBJ, 0x200, wk->heap_sID );

	wk->printQue = PRINTSYS_QUE_Create( wk->heap_sID );
  PRINTSYS_QUE_ForceCommMode( wk->printQue, TRUE );
  PRINTSYS_QUE_SetLimitTick( wk->printQue, PRINT_QUE_LIMMIT_DEFAULT );

  wk->fontHandle = FLDMSGBG_GetFontHandle( FIELDMAP_GetFldMsgBG(wk->fieldWork));

  GFL_FONTSYS_SetDefaultColor();
 
  wk->wordset = WORDSET_Create( wk->heap_sID);

	wk->mm_status = GFL_MSG_Create(
		GFL_MSG_LOAD_FAST, ARCID_MESSAGE, NARC_message_beacon_status_dat, wk->heap_sID );

  wk->str_tmp = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heap_sID );
  wk->str_expand = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heap_sID );
  wk->str_popup = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heap_sID );
}

//--------------------------------------------------------------
/**
 * システム関連の破棄
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_SystemExit(BEACON_VIEW_PTR wk)
{

  GFL_STR_DeleteBuffer(wk->str_popup);
  GFL_STR_DeleteBuffer(wk->str_expand);
  GFL_STR_DeleteBuffer(wk->str_tmp);

  GFL_MSG_Delete(wk->mm_status);
	WORDSET_Delete(wk->wordset);
  
  PRINTSYS_QUE_Clear( wk->printQue );
  PRINTSYS_QUE_Delete( wk->printQue );
  
  GFL_FONTSYS_SetDefaultColor();
 
  PaletteFadeWorkAllocFree( wk->pfd, FADE_SUB_OBJ );
  PaletteFadeWorkAllocFree( wk->pfd, FADE_SUB_BG );
  PaletteFadeFree( wk->pfd );
  wk->pfd = NULL;

  GFL_TCB_DeleteTask( wk->tcbVIntr );
 
  GFL_TCBL_Exit( wk->pTcbSys );
}

//--------------------------------------------------------------
/**
 * BGロード
 *
 * @param   view		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _sub_BGLoad( BEACON_VIEW_PTR wk, ARCHANDLE *handle )
{
	static const GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
  	{//GFL_BG_FRAME1_S
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
  		0, 0, 0, FALSE
  	},
  	{//GFL_BG_FRAME1_S
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
  		1, 0, 0, FALSE
  	},
  	{//GFL_BG_FRAME2_S
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
  		2, 0, 0, FALSE
  	},
  	{//GFL_BG_FRAME3_S
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
  		3, 0, 0, FALSE
  	},
	};
	GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &TextBgCntDat[3], GFL_BG_MODE_TEXT );

	//BG VRAMクリア
	GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);

	//レジスタOFF
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
	G2S_BlendNone();
  
  //パレット転送
  PaletteWorkSet_ArcHandle( wk->pfd, handle, NARC_beacon_status_bstatus_bg_nclr,
      wk->tmpHeapID, FADE_SUB_BG, 0x20*FONT_PAL, 0 );
  PaletteWorkSet_Arc( wk->pfd, ARCID_FONT, NARC_font_default_nclr,
      wk->tmpHeapID, FADE_SUB_BG, 0x20, FONT_PAL*16 );
  
  //キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    handle, NARC_beacon_status_bstatus_bg_lz_ncgr, GFL_BG_FRAME3_S, 0, 
    0, TRUE, wk->tmpHeapID);
  
  //スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_beacon_status_bstatus_bg01_lz_nscr, GFL_BG_FRAME1_S, 0, 
    0x800, TRUE, wk->tmpHeapID);
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_beacon_status_bstatus_bg02_lz_nscr, GFL_BG_FRAME2_S, 0, 
    0x800, TRUE, wk->tmpHeapID);
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_beacon_status_bstatus_bg03_lz_nscr, GFL_BG_FRAME3_S, 0, 
    0x800, TRUE, wk->tmpHeapID);

  G2S_SetBlendAlpha( ALPHA_1ST_NORMAL, ALPHA_2ND, ALPHA_EV1_NORMAL, ALPHA_EV2_NORMAL);

	GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
}

//--------------------------------------------------------------
/**
 * BGアンロード
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _sub_BGUnload(BEACON_VIEW_PTR wk )
{
	GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );

  G2S_BlendNone();

	GFL_BG_FreeBGControl(GFL_BG_FRAME0_S);
	GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
	GFL_BG_FreeBGControl(GFL_BG_FRAME2_S);
	GFL_BG_FreeBGControl(GFL_BG_FRAME3_S);
}

//--------------------------------------------------------------
/**
 * メニュー初期化
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_TMenuInit( BEACON_VIEW_PTR wk )
{
  int i;

  wk->key_cursor = APP_KEYCURSOR_Create( FCOL_POPUP_BASE, FALSE, TRUE, wk->heap_sID );

  wk->menuRes = APP_TASKMENU_RES_Create( FRM_MENUMSG, BG_PAL_TASKMENU, wk->fontHandle, wk->printQue, wk->heap_sID );

  //アイテム初期化
  for( i = 0;i < TMENU_YN_MAX;i++){
    wk->tmenu[i].item.str      = GFL_MSG_CreateString( wk->mm_status, msg_sys_tmenu_yesno_01+i ); //メニューに表示する文字列
    wk->tmenu[i].item.msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
    wk->tmenu[i].item.type     = APP_TASKMENU_WIN_TYPE_NORMAL; //戻るマークの表示
    wk->tmenu[i].work = NULL;
  }
  for( i = 0;i < TMENU_CHECK_MAX;i++){
    wk->tmenu_check[i].item.str      = GFL_MSG_CreateString( wk->mm_status, msg_sys_tmenu_check_01+i ); //メニューに表示する文字列
    wk->tmenu_check[i].item.msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
    wk->tmenu_check[i].item.type     = APP_TASKMENU_WIN_TYPE_NORMAL; //戻るマークの表示
    wk->tmenu_check[i].work = NULL;
  }
}

//--------------------------------------------------------------
/**
 * メニュー解放
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_TMenuRelease( BEACON_VIEW_PTR wk )
{
  int i;
  for( i = 0;i < TMENU_CHECK_MAX; i++){
    if( wk->tmenu_check[i].work != NULL ){
      APP_TASKMENU_WIN_Delete( wk->tmenu_check[i].work );
    }
    GFL_STR_DeleteBuffer( wk->tmenu_check[i].item.str );
  }
  for( i = 0;i < TMENU_YN_MAX; i++){
    if( wk->tmenu[i].work != NULL ){
      APP_TASKMENU_WIN_Delete( wk->tmenu[i].work );
    }
    GFL_STR_DeleteBuffer( wk->tmenu[i].item.str );
  }
  APP_TASKMENU_RES_Delete( wk->menuRes );

  APP_KEYCURSOR_Delete( wk->key_cursor );
}


//--------------------------------------------------------------
/**
 * アクターで使用するリソースのロード
 *
 * @param   view		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _sub_ActorResourceLoad( BEACON_VIEW_PTR wk, ARCHANDLE *handle)
{
  int i;
  ARCHANDLE* handle_union = GFL_ARC_OpenDataHandle( ARCID_WIFIUNIONCHAR, wk->tmpHeapID );

  {
    const OBJ_RES_SRC srcNormal[OBJ_RES_MAX] = {
      { 1, NARC_beacon_status_bstatus_obj_nclr },
      { 1, NARC_beacon_status_bstatus_obj_ncgr },
      { 1, NARC_beacon_status_bstatus_obj_ncer },
    };
    obj_ObjResInit( wk, &wk->objResNormal, srcNormal, handle );
  }
  {
    const OBJ_RES_SRC srcIcon[OBJ_RES_MAX] = {
      { 0, 0 },
      { PANEL_MAX, NARC_beacon_status_bstatus_icon01_ncgr },
      { 1, NARC_beacon_status_bstatus_icon01_ncer },
    };
    obj_ObjResInit( wk, &wk->objResIcon, srcIcon, handle );
  }
  
  {
    const OBJ_RES_SRC srcUnion[OBJ_RES_MAX] = {
     { 0, 0 },
     { PANEL_MAX, NARC_wifi_unionobj_front00_NCGR },
     { 1,NARC_wifi_unionobj_front00_NCER },
    };
    obj_ObjResInit( wk, &wk->objResUnion, srcUnion, handle_union );
  }

  //再転送用のリソースをロードしておく
  PaletteWorkSet_ArcHandle( wk->pfd, handle, NARC_beacon_status_bstatus_obj_nclr,
      wk->tmpHeapID, FADE_SUB_OBJ, 0x20*ACT_PAL_WMI, 0 );
  PaletteWorkSet_Arc( wk->pfd, ARCID_WMI, NARC_wmi_wm_NCLR,
      wk->tmpHeapID, FADE_SUB_OBJ, 0x20, ACT_PAL_WMI*16 );
  wk->resPlttPanel.buf = GFL_ARC_LoadDataAllocByHandle( handle,
                          NARC_beacon_status_bstatus_panel_nclr,
                          wk->heap_sID );
  NNS_G2dGetUnpackedPaletteData( wk->resPlttPanel.buf, &wk->resPlttPanel.p_pltt );
  wk->resPlttPanel.dat = (u16*)wk->resPlttPanel.p_pltt->pRawData;

  wk->resPlttUnion.buf = GFL_ARC_LoadDataAllocByHandle( handle_union,
                          NARC_wifi_unionobj_wifi_union_obj_NCLR,
                          wk->heap_sID );
  NNS_G2dGetUnpackedPaletteData( wk->resPlttUnion.buf, &wk->resPlttUnion.p_pltt );
  wk->resPlttUnion.dat = (u16*)wk->resPlttUnion.p_pltt->pRawData;
  
  for(i = 0;i < UNION_CHAR_MAX;i++){
    wk->resCharUnion[i].buf = GFL_ARC_LoadDataAllocByHandle( handle_union,
                              NARC_wifi_unionobj_front00_NCGR+i,
                              wk->heap_sID );
    NNS_G2dGetUnpackedCharacterData( wk->resCharUnion[i].buf, &wk->resCharUnion[i].p_char );
  }
  for(i = 0;i < BEACON_ICON_MAX;i++){
    wk->resCharIcon[i].buf = GFL_ARC_LoadDataAllocByHandle( handle,
                          NARC_beacon_status_bstatus_icon01_ncgr+i,
                          wk->heap_sID );
    NNS_G2dGetUnpackedCharacterData( wk->resCharIcon[i].buf, &wk->resCharIcon[i].p_char );
  }
  DC_FlushAll();

  GFL_ARC_CloseDataHandle( handle_union );
}

//--------------------------------------------------------------
/**
 * アクターで使用するアクターのアンロード
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _sub_ActorResourceUnload( BEACON_VIEW_PTR wk )
{
  int i;
  for( i = 0;i < BEACON_ICON_MAX;i++){
    GFL_HEAP_FreeMemory( wk->resCharIcon[i].buf );
  }
  for( i = 0;i < UNION_CHAR_MAX;i++){
    GFL_HEAP_FreeMemory( wk->resCharUnion[i].buf );
  }
  GFL_HEAP_FreeMemory( wk->resPlttPanel.buf );
  GFL_HEAP_FreeMemory( wk->resPlttUnion.buf );

  obj_ObjResRelease( wk, &wk->objResUnion );
  obj_ObjResRelease( wk, &wk->objResIcon );
  obj_ObjResRelease( wk, &wk->objResNormal );
}

//--------------------------------------------------------------
/**
 * アクター作成
 *
 * @param   wk		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _sub_ActorCreate( BEACON_VIEW_PTR wk, ARCHANDLE *handle )
{
  int i;

  //セル系システムの作成
  wk->cellUnit = GFL_CLACT_UNIT_Create( BEACON_VIEW_OBJ_MAX , 0 , wk->heapID );
 
  //レンダラー作成
  {
    const GFL_REND_SURFACE_INIT renderInitData = { 0,512,256,192,CLSYS_DRAW_SUB, CLSYS_REND_CULLING_TYPE_NOT_AFFINE};
    
    wk->cellRender = GFL_CLACT_USERREND_Create( &renderInitData , 1 , wk->heap_sID );
    GFL_CLACT_UNIT_SetUserRend( wk->cellUnit, wk->cellRender );
  }

  //サブサーフェスの位置を設定
//  GFL_CLACT_USERREND_GetSurfacePos( , ACT_RENDER_ID, &wk->cellSurfacePos );

  //FontOAMシステAdd成
  wk->bmpOam = BmpOam_Init( wk->heap_sID, wk->cellUnit );
  
  act_NormalObjAdd( wk );

  for(i = 0; i < PANEL_MAX;i++){
    act_PanelObjAdd( wk, i );
  }

  GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * アクター削除
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_ActorDelete( BEACON_VIEW_PTR wk )
{
  int i;

  GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_OFF);
 
  for(i = 0; i < PANEL_MAX;i++){
    act_PanelObjDel( wk, i );
  }
  act_NormalObjDel( wk );

  BmpOam_Exit( wk->bmpOam );

  GFL_CLACT_USERREND_Delete( wk->cellRender );
  GFL_CLACT_UNIT_Delete( wk->cellUnit );
}

/*
 *  @brief  BmpWin個別追加
 */
static void bmpwin_Add( BMP_WIN* win, u8 frm, u8 pal, u8 px, u8 py, u8 sx, u8 sy )
{
  win->win = GFL_BMPWIN_Create( frm, px, py, sx, sy, pal, GFL_BMP_CHRAREA_GET_B );
  win->bmp = GFL_BMPWIN_GetBmp( win->win );
  PRINT_UTIL_Setup( &win->putil, win->win );
  GFL_BMPWIN_MakeTransWindow( win->win );
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _sub_BmpWinCreate(BEACON_VIEW_PTR wk)
{
  bmpwin_Add( &wk->win[WIN_POPUP],
      BMP_POPUP_FRM, BMP_POPUP_PAL, BMP_POPUP_PX, BMP_POPUP_PY,BMP_POPUP_SX, BMP_POPUP_SY );
  bmpwin_Add( &wk->win[WIN_MENU],
      BMP_MENU_FRM, BMP_MENU_PAL, BMP_MENU_PX, BMP_MENU_PY,BMP_MENU_SX, BMP_MENU_SY );
  bmpwin_Add( &wk->win[WIN_LOGNUM],
      BMP_LOGNUM_FRM, BMP_LOGNUM_PAL, BMP_LOGNUM_PX, BMP_LOGNUM_PY,BMP_LOGNUM_SX, BMP_LOGNUM_SY );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_BmpWinDelete(BEACON_VIEW_PTR wk)
{
  int i;
	
  for(i = 0; i < WIN_MAX; i++){
    GFL_BMPWIN_Delete( wk->win[i].win );
  }
}


//--------------------------------------------------------------
/**
 * アクターリソース取得
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void obj_ObjResInit( BEACON_VIEW_PTR wk, OBJ_RES_TBL* res, const OBJ_RES_SRC* srcTbl, ARCHANDLE* p_handle )
{
  int i;

  MI_CpuClear8( res, sizeof(OBJ_RES_TBL));

  for(i = 0;i < OBJ_RES_MAX;i++){
    res->res[i].num = srcTbl[i].num;
    if(res->res[i].num != 0){
      res->res[i].tbl = GFL_HEAP_AllocClearMemory( wk->heapID, sizeof(u32)* res->res[i].num );
    }
  }

  for(i = 0;i < res->res[OBJ_RES_PLTT].num;i++){
    res->res[OBJ_RES_PLTT].tbl[i] = 
      GFL_CLGRP_PLTT_Register(  p_handle, srcTbl[OBJ_RES_PLTT].srcID+i,
                                CLSYS_DRAW_SUB, 0, wk->heapID );
  }
  for(i = 0;i < res->res[OBJ_RES_CGR].num;i++){
    res->res[OBJ_RES_CGR].tbl[i] = 
      GFL_CLGRP_CGR_Register( p_handle, srcTbl[OBJ_RES_CGR].srcID+i,
                              FALSE, CLSYS_DRAW_SUB, wk->heapID );
  }
  for(i = 0;i < res->res[OBJ_RES_CELLANIM].num;i++){
    res->res[OBJ_RES_CELLANIM].tbl[i] = 
      GFL_CLGRP_CELLANIM_Register( p_handle, 
          srcTbl[OBJ_RES_CELLANIM].srcID+(i*2),
          srcTbl[OBJ_RES_CELLANIM].srcID+1+(i*2),
          wk->heapID );
  }
}

//--------------------------------------------------------------
/**
 * アクターリソース解放
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void obj_ObjResRelease( BEACON_VIEW_PTR wk, OBJ_RES_TBL* res )
{
  int i;

  for(i = 0;i < res->res[OBJ_RES_CELLANIM].num;i++){
    GFL_CLGRP_CELLANIM_Release( res->res[OBJ_RES_CELLANIM].tbl[i] );
  }
  for(i = 0;i < res->res[OBJ_RES_CGR].num;i++){
    GFL_CLGRP_CGR_Release( res->res[OBJ_RES_CGR].tbl[i] );
  }
  for(i = 0;i < res->res[OBJ_RES_PLTT].num;i++){
    GFL_CLGRP_PLTT_Release( res->res[OBJ_RES_PLTT].tbl[i] );
  }
  for(i = 0;i < OBJ_RES_MAX;i++){
    if(res->res[i].tbl != NULL){
      GFL_HEAP_FreeMemory( res->res[i].tbl );
    }
  }
}

/*
 *  @brief  アクター追加
 */
static GFL_CLWK* obj_ObjAdd(
  BEACON_VIEW_PTR wk, u32 cgrNo, u32 palNo, u32 cellNo,
  s16 x, s16 y, u8 anm, u8 bg_pri, u8 spri)
{
  GFL_CLWK_DATA ini;
  GFL_CLWK* obj;

  //セルの生成
  ini.pos_x = x;
  ini.pos_y = y;
  ini.anmseq = anm;
  ini.bgpri = bg_pri;
  ini.softpri = spri;
  
  obj = GFL_CLACT_WK_Create( wk->cellUnit,
          cgrNo,palNo,cellNo,
          &ini, ACT_RENDER_ID, wk->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( obj, TRUE );
  GFL_CLACT_WK_SetSRTAnimeFlag( obj, FALSE );
  return obj;
}

/*
 *  @brief  ノーマルオブジェ生成
 */
static void act_NormalObjAdd( BEACON_VIEW_PTR wk ) 
{
  int i;
  static const u16 pos[ACT_MAX][2] = {
   { ACT_MENU_PX+ACT_MENU_OX*0 ,ACT_MENU_PY },
   { ACT_MENU_PX+ACT_MENU_OX*1 ,ACT_MENU_PY },
   { ACT_MENU_PX+ACT_MENU_OX*2 ,ACT_MENU_PY },
   { ACT_MENU_PX+ACT_MENU_OX*3 ,ACT_MENU_PY },
   { ACT_UP_PX ,ACT_UP_PY },
   { ACT_DOWN_PX ,ACT_DOWN_PY },
  };

  for(i = 0;i < ACT_MAX;i++){
    wk->pAct[ACT_POWER+i] = obj_ObjAdd( wk,
      wk->objResNormal.res[OBJ_RES_CGR].tbl[0],
      wk->objResNormal.res[OBJ_RES_PLTT].tbl[0],
      wk->objResNormal.res[OBJ_RES_CELLANIM].tbl[0],
      pos[i][0], pos[i][1],
      ACTANM_POWER_ON+(ACT_ANM_SET*i), OBJ_MENU_BG_PRI, OBJ_SPRI_MENU);
  
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_POWER+i], TRUE );
  }
}

/*
 *  @brief  ノーマルオブジェ破棄
 */
static void act_NormalObjDel( BEACON_VIEW_PTR wk )
{
  int i;

  for(i = 0;i < ACT_MAX;i++){
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_POWER+i], FALSE );
    GFL_CLACT_WK_Remove( wk->pAct[ACT_POWER+i]);
  }
}


/**
 *  @brief  パネル生成
 */
static void act_PanelObjAdd( BEACON_VIEW_PTR wk, u8 idx )
{
	BMPOAM_ACT_DATA	finit;
  PANEL_WORK* pp = &(wk->panel[idx]);

  MI_CpuClear8( pp, sizeof(PANEL_WORK));

  pp->id = idx;
  pp->data_idx = PANEL_DATA_BLANK;
  pp->n_line = 0;
  pp->que = wk->printQue;

  pp->px = ACT_PANEL_OX*idx;
  pp->py = ACT_PANEL_OY*idx;

  //パネルアクター
  pp->cPanel = obj_ObjAdd( wk,
    wk->objResNormal.res[OBJ_RES_CGR].tbl[0],
    wk->objResNormal.res[OBJ_RES_PLTT].tbl[0],
    wk->objResNormal.res[OBJ_RES_CELLANIM].tbl[0],
    pp->px, pp->py, ACTANM_PANEL, OBJ_BG_PRI, OBJ_SPRI_PANEL+idx);
  GFL_CLACT_WK_SetPlttOffs( pp->cPanel, idx, CLWK_PLTTOFFS_MODE_OAM_COLOR );
  
  //Unionオブジェアクター
  pp->cUnion = obj_ObjAdd( wk,
    wk->objResUnion.res[OBJ_RES_CGR].tbl[idx],
    wk->objResNormal.res[OBJ_RES_PLTT].tbl[0],
    wk->objResUnion.res[OBJ_RES_CELLANIM].tbl[0],
    pp->px+ACT_UNION_OX, pp->py+ACT_UNION_OY, 0, OBJ_BG_PRI, OBJ_SPRI_UNION+idx);
  GFL_CLACT_WK_SetPlttOffs( pp->cUnion, idx + ACT_PAL_UNION , CLWK_PLTTOFFS_MODE_PLTT_TOP );

  //Iconオブジェアクター
  pp->cIcon = obj_ObjAdd( wk,
    wk->objResIcon.res[OBJ_RES_CGR].tbl[idx],
    wk->objResNormal.res[OBJ_RES_PLTT].tbl[0],
    wk->objResIcon.res[OBJ_RES_CELLANIM].tbl[0],
    pp->px+ACT_ICON_OX, pp->py+ACT_ICON_OY, 0, OBJ_BG_PRI, OBJ_SPRI_ICON+idx);

	pp->msgOam.bmp = GFL_BMP_Create( BMP_PANEL_OAM_SX, BMP_PANEL_OAM_SY, GFL_BMP_16_COLOR, wk->heap_sID );

	finit.bmp = pp->msgOam.bmp;
	finit.x = pp->px + ACT_MSG_OX;
	finit.y = pp->py + ACT_MSG_OY;
	finit.pltt_index = wk->objResNormal.res[OBJ_RES_PLTT].tbl[0];
	finit.pal_offset = ACT_PAL_FONT;		// pltt_indexのパレット内でのオフセット
	finit.soft_pri = OBJ_SPRI_MSG;			// ソフトプライオリティ
	finit.bg_pri = OBJ_BG_PRI;				// BGプライオリティ
	finit.setSerface = ACT_RENDER_ID;
	finit.draw_type  = CLSYS_DRAW_SUB;

	pp->msgOam.oam = BmpOam_ActorAdd( wk->bmpOam, &finit );

  pp->str = GFL_STR_CreateBuffer( BUFLEN_PANEL_MSG, wk->heap_sID );
  pp->name = GFL_STR_CreateBuffer( BUFLEN_TR_NAME, wk->heap_sID );
  GFL_BMP_Clear( pp->msgOam.bmp, 0 );
	BmpOam_ActorBmpTrans( pp->msgOam.oam );

	BmpOam_ActorSetDrawEnable( pp->msgOam.oam, FALSE );
  GFL_CLACT_WK_SetDrawEnable( pp->cPanel, FALSE );
  GFL_CLACT_WK_SetDrawEnable( pp->cUnion, FALSE );
  GFL_CLACT_WK_SetDrawEnable( pp->cIcon, FALSE );
}

/**
 *  @brief  パネル削除
 */
static void act_PanelObjDel( BEACON_VIEW_PTR wk, u8 idx )
{
  PANEL_WORK* pp = &(wk->panel[idx]);

  GFL_STR_DeleteBuffer( pp->name );
  GFL_STR_DeleteBuffer( pp->str );

	BmpOam_ActorSetDrawEnable( pp->msgOam.oam, FALSE );
  BmpOam_ActorDel( pp->msgOam.oam );
	GFL_BMP_Delete( pp->msgOam.bmp );

  GFL_CLACT_WK_SetDrawEnable( pp->cIcon, FALSE );
  GFL_CLACT_WK_Remove( pp->cIcon );
  GFL_CLACT_WK_SetDrawEnable( pp->cUnion, FALSE );
  GFL_CLACT_WK_Remove( pp->cUnion );
  GFL_CLACT_WK_SetDrawEnable( pp->cPanel, FALSE );
  GFL_CLACT_WK_Remove( pp->cPanel );
 
  MI_CpuClear8( pp, sizeof(PANEL_WORK));
}

