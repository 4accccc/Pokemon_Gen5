//==============================================================================
/**
 * @file    fmission_view_setup.c
 * @brief   フェスミッション画面　セットアップ系サブルーチン
 * @author  iwasawa 
 * @date    2011.03.03
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
#include "field/fieldmap.h"
#include "field/fmission_view.h"
#include "print\printsys.h"
#include "message.naix"
#include "field/event_fmission_app.h"
#include "field/event_subscreen.h"
#include "field/event_gpower.h"
#include "field/event_freeword_input.h"
#include "field/event_festmission.h"

#include "beacon_status.naix"
#include "fmission_log.naix"
#include "script_message.naix"
#include "wifi_unionobj.naix"
#include "wmi.naix"
#include "gamesystem/fmission_log.h"
#include "gamesystem/fmission_log_accessor_fv.h"
#include "fmission_view_local.h"
#include "fmission_view_sub.h"

#ifdef PM_DEBUG
#include "field/event_debug_livecomm.h"
#endif  //PM_DEBUG

//==============================================================================
//  定数定義
//==============================================================================
typedef struct _EVWK_EVENT_START{
  int   sub_seq;
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap;
  FMISSION_VIEW_PTR bvp;
  GMEVENT*  call_event;

  EVWAIT_TYPE type;
  int param;
}EVWK_EVENT_START;

static void _sub_DataSetup(FMISSION_VIEW_PTR wk);
static void _sub_DataExit(FMISSION_VIEW_PTR wk);
static void _sub_SystemSetup(FMISSION_VIEW_PTR view);
static void _sub_SystemExit(FMISSION_VIEW_PTR view);
static void _sub_BGLoad(FMISSION_VIEW_PTR view, ARCHANDLE *handle);
static void _sub_BGUnload(FMISSION_VIEW_PTR view);
static void _sub_TMenuInit( FMISSION_VIEW_PTR wk );
static void _sub_TMenuRelease( FMISSION_VIEW_PTR wk );
static void _sub_ActorResourceLoad(FMISSION_VIEW_PTR view, ARCHANDLE *handle);
static void _sub_ActorResourceUnload(FMISSION_VIEW_PTR view);
static void _sub_ActorCreate(FMISSION_VIEW_PTR view, ARCHANDLE *handle);
static void _sub_ActorDelete(FMISSION_VIEW_PTR view);
static void _sub_BmpWinCreate(FMISSION_VIEW_PTR view);
static void _sub_BmpWinDelete(FMISSION_VIEW_PTR view);

static void obj_ObjResInit( FMISSION_VIEW_PTR wk, OBJ_RES_TBL* res, const OBJ_RES_SRC* srcTbl, ARCHANDLE* p_handle, BOOL comp_f );
static void obj_ObjResRelease( FMISSION_VIEW_PTR wk, OBJ_RES_TBL* res );
static void act_NormalObjAdd( FMISSION_VIEW_PTR wk ) ;
static void act_NormalObjDel( FMISSION_VIEW_PTR wk ) ;
static void act_PanelObjAdd( FMISSION_VIEW_PTR wk, u8 idx );
static void act_PanelObjDel( FMISSION_VIEW_PTR wk, u8 idx );

////////////////////////////////////////////////////////////////////////////
//初期化系
////////////////////////////////////////////////////////////////////////////

/*
 *  @brief　一番最初のワーク初期化
 */
FMISSION_VIEW* FVS_InitWork( GAMESYS_WORK* gsys, FIELD_SUBSCREEN_WORK* subscreen )
{
  FMISSION_VIEW* wk;
  
  wk = GFL_HEAP_AllocClearMemory(HEAPID_FIELD_SUBSCREEN, sizeof(FMISSION_VIEW));
  wk->gsys = gsys;
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
  wk->fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  wk->fld_vreq = FIELDMAP_GetFldVReq(wk->fieldWork);
  wk->fest_sys = GAMESYSTEM_GetLinkFestivalWork(gsys);
  wk->fmwk = LINK_FESTIVAL_GetMissionWork( wk->fest_sys );
  wk->subscreen = subscreen;
 
  wk->heap_sID = HEAPID_FIELD_SUBSCREEN;
  wk->heapID = HEAPID_FIELDMAP;
  wk->tmpHeapID = GFL_HEAP_LOWID( HEAPID_FIELD_SUBSCREEN );

  wk->arc_handle = GFL_ARC_OpenDataHandle(ARCID_FMISSION_LOG, wk->heap_sID);

  _sub_DataSetup(wk);
  _sub_SystemSetup(wk);
  _sub_BGLoad(wk, wk->arc_handle);
  _sub_TMenuInit( wk );
  _sub_ActorResourceLoad(wk, wk->arc_handle);
  _sub_ActorCreate(wk, wk->arc_handle);
  _sub_BmpWinCreate(wk);
 
  return wk;
}

/*
 *  @brief  一番最後のワーク解放
 */
void FVS_ReleaseWork( FMISSION_VIEW_PTR wk )
{
  //今生きている全てのタスクを削除
  GFL_TCBL_DeleteAll( wk->pTcbSys );
  
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

////////////////////////////////////////////////////////////////////////////
//イベントリクエスト系
////////////////////////////////////////////////////////////////////////////

/*
 *  @brief  イベントリクエスト
 */
void FVSEvent_Reserve( FMISSION_VIEW_PTR wk, BEACON_DETAIL_EVENT ev_id)
{
  wk->event_id = ev_id;
  wk->event_reserve_exec = FALSE;
}

/*
 *  @brief  イベントリクエストリセット
 */
void FVSEvent_ReserveReset( FMISSION_VIEW_PTR wk )
{
  wk->event_id = EV_NONE;
}

/*
 *  @brief  イベントチェックコモン
 */
GMEVENT* FVSEvent_CheckCommon( FMISSION_VIEW_PTR wk, BOOL bEvReqOK )
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

    FMISSION_LOG_TBL_SetViewTopIdx( wk->logView, 0 );
    call_event = EVENT_ChangeSubScreen( wk->gsys, wk->fieldWork, FIELD_SUBSCREEN_NORMAL);
    event = FVSEvent_EventStart( wk, call_event, EVWAIT_MENU_ANM, MENU_RETIRE );
    break;
  case EV_CALL_INFO_VIEW:
    {
      FMISSION_APP_MODE mode = FMISSION_APP_MODE_INFO;
      call_event = GMEVENT_CreateOverlayEventCall( wk->gsys,
          FS_OVERLAY_ID( notwifi_event_fmission_app ), EVENT_FMissionApp, (void*)&mode );
    }
    event = FVSEvent_EventStart( wk, call_event, EVWAIT_MENU_ANM, MENU_INFO );
    break;
  case EV_MISSION_RETIRE:
    call_event = GMEVENT_CreateOverlayEventCall( wk->gsys,
      FS_OVERLAY_ID( event_fmission ), EVENT_FestMissionRetire, wk->fieldWork );
    
    event = FVSEvent_EventStart( wk, call_event, EVWAIT_TMENU_ANM, (int)(BVCTMenu_GetWinWork( wk->tmenu, TMENU_YESNO_YES )) );
    break;
  case EV_GPOWER_USE_MINE:
    {
      GPOWER_EFFECT_PARAM param;
      param.g_power = wk->my_data.power;
      param.mine_f = TRUE;
      call_event = GMEVENT_CreateOverlayEventCall( wk->gsys,
          FS_OVERLAY_ID( event_gpower ), EVENT_GPowerEffectStart, &param );
    }
    event = FVSEvent_EventStart( wk, call_event, EVWAIT_MENU_ANM, MENU_POWER );
    break;
  case EV_GPOWER_USE:
    {
      GPOWER_EFFECT_PARAM param;
      param.g_power = wk->ctrl.g_power;
      param.mine_f = wk->ctrl.mine_power_f;
      call_event = GMEVENT_CreateOverlayEventCall( wk->gsys,
          FS_OVERLAY_ID( event_gpower ), EVENT_GPowerEffectStart, &param );
    }
    event = FVSEvent_EventStart( wk, call_event, EVWAIT_TMENU_ANM, (int)(BVCTMenu_GetWinWork( wk->tmenu, TMENU_CYN_YES )) );
    wk->ctrl.g_power = GPOWER_ID_NULL;
    break;
  case EV_GPOWER_CHECK_TMENU_YN:
    call_event = GMEVENT_CreateOverlayEventCall( wk->gsys,
        FS_OVERLAY_ID( event_gpower), EVENT_GPowerEnableListCheck, wk->fieldWork );
    event = FVSEvent_EventStart( wk, call_event, EVWAIT_TMENU_ANM, (int)(BVCTMenu_GetWinWork( wk->tmenu, TMENU_CYN_CHECK )) );
    break;
  case EV_GPOWER_CHECK_TMENU_CHK:
    call_event = GMEVENT_CreateOverlayEventCall( wk->gsys,
        FS_OVERLAY_ID( event_gpower), EVENT_GPowerEnableListCheck, wk->fieldWork );
    event = FVSEvent_EventStart( wk, call_event, EVWAIT_TMENU_ANM, (int)(BVCTMenu_GetWinWork( wk->tmenu, TMENU_CHECK_CALL )) );
    wk->gpower_check_req = FALSE;
    break;
  case EV_CALL_TALKMSG_INPUT:
    call_event = EVENT_FreeWordInput( wk->gsys, wk->fieldWork, NULL,
              NAMEIN_FREE_WORD, BEACON_STATUS_GetFreeWordInputResultPointer( wk->b_status ) );
    event = FVSEvent_EventStart( wk, call_event, EVWAIT_MENU_ANM, MENU_HELLO );
    break;
  
#ifdef PM_DEBUG
  //デバッグメニューコールリクエスト
  case EV_DEBUG_MENU_CALL:
    event = GMEVENT_CreateOverlayEventCall( wk->gsys,
              FS_OVERLAY_ID( d_livecomm ), DEBUG_EVENT_LiveCommFromFMission, wk );
    break;
#endif  //PM_DEBUG

  default:
    return NULL;
  }
  FVSEvent_ReserveReset( wk );
  return event; 
}


////////////////////////////////////////////////////////////////////////////
//アニメウェイトイベント
////////////////////////////////////////////////////////////////////////////
static GMEVENT_RESULT event_EventStartMain(GMEVENT * event, int *  seq, void * work);
static BOOL evsub_WaitTypeMenuAnime( FMISSION_VIEW_PTR wk, EVWK_EVENT_START* ev_wk );
static BOOL evsub_WaitTypeTcb( FMISSION_VIEW_PTR wk, EVWK_EVENT_START* ev_wk );
static BOOL evsub_WaitTypeTMenuAnm( FMISSION_VIEW_PTR wk, EVWK_EVENT_START* ev_wk );

/*
 *  @brief  各種イベント起動前に、アニメなどの待ちをするためのイベント
 */
GMEVENT* FVSEvent_EventStart( FMISSION_VIEW_PTR wk, GMEVENT* call_event, EVWAIT_TYPE type, int param )
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
  FMISSION_VIEW_PTR wk = ev_wk->bvp;
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
    wk->event_reserve_exec = TRUE;
    GMEVENT_ChangeEvent( event, ev_wk->call_event );
  }
  return GMEVENT_RES_CONTINUE;
}

static BOOL evsub_WaitTypeMenuAnime( FMISSION_VIEW_PTR wk, EVWK_EVENT_START* ev_wk )
{
  switch( ev_wk->sub_seq ){
  case 0:
    FMissionView_MenuBarViewSet( wk, ev_wk->param, MENU_ST_ANM );
    ev_wk->sub_seq++;
    return FALSE;
  case 1:
    if( FMissionView_MenuBarCheckAnm( wk, ev_wk->param )){
      return FALSE;
    }
    break;
  }
  return TRUE;
}

static BOOL evsub_WaitTypeTcb( FMISSION_VIEW_PTR wk, EVWK_EVENT_START* ev_wk )
{
  GFL_TCBL_Main( wk->pTcbSys );
  return (wk->eff_task_ct == 0 );
}

static BOOL evsub_WaitTypeTMenuAnm( FMISSION_VIEW_PTR wk, EVWK_EVENT_START* ev_wk )
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

//==============================================================================
//  
//==============================================================================
static void tcb_VInter( GFL_TCB* tcb, void * work)
{
  FMISSION_VIEW_PTR wk = (FMISSION_VIEW_PTR)work;

  if( wk->pfd != NULL){
    PaletteFadeTrans( wk->pfd );
  }
}

//--------------------------------------------------------------
/**
 * データ関連のセットアップ
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_DataSetup(FMISSION_VIEW_PTR wk)
{
  SAVE_CONTROL_WORK* save;

  wk->b_status = GAMEDATA_GetBeaconStatus( wk->gdata );

  wk->logStack = LINK_FESTIVAL_GetMissionLogStackTbl( wk->fest_sys );
  wk->logView  = LINK_FESTIVAL_GetMissionLogViewTbl( wk->fest_sys );
  wk->logGPower = LINK_FESTIVAL_GetMissionLogGPowerTbl( wk->fest_sys );

  wk->ctrl.max = FMISSION_LOG_TBL_GetEntryNum( wk->logView );
  wk->ctrl.view_top = FMISSION_LOG_TBL_GetViewTopIdx( wk->logView );

  IWASAWA_Printf("FMissionViewSetup log_num = %d, top=%d\n", wk->ctrl.max, wk->ctrl.view_top );
  if( wk->ctrl.max < PANEL_VIEW_MAX ){
    wk->ctrl.view_max = wk->ctrl.max;
  }else{
    wk->ctrl.view_max = PANEL_VIEW_MAX;
  }
  wk->ctrl.g_power = GPOWER_ID_NULL;

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
  
  //テンポラリワーク領域取得
  wk->tmpLog = FMISSION_LOG_AllocWork( wk->heap_sID );
  wk->tmpLog2 = FMISSION_LOG_AllocWork( wk->heap_sID );
  wk->newLog = FMISSION_LOG_AllocWork( wk->heap_sID );

  //Gパワーデータ取得
  wk->gpower_data = GPOWER_PowerData_LoadAlloc( wk->heap_sID );
}

//--------------------------------------------------------------
/**
 * データ関連の破棄＆終了処理
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_DataExit(FMISSION_VIEW_PTR wk)
{
  GPOWER_PowerData_Unload( wk->gpower_data );

  GFL_HEAP_FreeMemory( wk->newLog );
  GFL_HEAP_FreeMemory( wk->tmpLog2 );
  GFL_HEAP_FreeMemory( wk->tmpLog );
}

//--------------------------------------------------------------
/**
 * システム関連のセットアップ
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_SystemSetup(FMISSION_VIEW_PTR wk)
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

	wk->mm_sys = GFL_MSG_Create(
		GFL_MSG_LOAD_FAST, ARCID_MESSAGE, NARC_message_fmission_log_dat, wk->heap_sID );
	wk->mm_fmission = GFL_MSG_Create(
		GFL_MSG_LOAD_FAST, ARCID_MESSAGE, NARC_message_fest_mission_dat, wk->heap_sID );

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
static void _sub_SystemExit(FMISSION_VIEW_PTR wk)
{

  GFL_STR_DeleteBuffer(wk->str_popup);
  GFL_STR_DeleteBuffer(wk->str_expand);
  GFL_STR_DeleteBuffer(wk->str_tmp);

  GFL_MSG_Delete(wk->mm_fmission);
  GFL_MSG_Delete(wk->mm_sys);
	WORDSET_Delete(wk->wordset);

  PRINTSYS_QUE_Clear( wk->printQue );
  PRINTSYS_QUE_Delete( wk->printQue );
  
  GFL_FONTSYS_SetDefaultColor();
 
  PaletteFadeWorkAllocFree( wk->pfd, FADE_SUB_OBJ );
  PaletteFadeWorkAllocFree( wk->pfd, FADE_SUB_BG );
  PaletteFadeFree( wk->pfd );
  wk->pfd = NULL;

  GFL_TCB_DeleteTask( wk->tcbVIntr );
 
  GFL_TCBL_DeleteAll( wk->pTcbSys );
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
static void _sub_BGLoad( FMISSION_VIEW_PTR wk, ARCHANDLE *handle )
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
  PaletteWorkSet_ArcHandle( wk->pfd, handle, NARC_fmission_log_fmission_bg_nclr,
      wk->tmpHeapID, FADE_SUB_BG, 0x20*FONT_PAL, 0 );
  PaletteWorkSet_Arc( wk->pfd, ARCID_FONT, NARC_font_default_nclr,
      wk->tmpHeapID, FADE_SUB_BG, 0x20, FONT_PAL*16 );
  
  //キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    handle, NARC_fmission_log_fmission_bg_lz_ncgr, GFL_BG_FRAME3_S, 0, 
    0, TRUE, wk->tmpHeapID);
  
  //スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_fmission_log_fmission_bg01_lz_nscr, GFL_BG_FRAME1_S, 0, 
    0x800, TRUE, wk->tmpHeapID);
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_fmission_log_fmission_bg02_lz_nscr, GFL_BG_FRAME2_S, 0, 
    0x800, TRUE, wk->tmpHeapID);
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_fmission_log_fmission_bg03_lz_nscr, GFL_BG_FRAME3_S, 0, 
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
static void _sub_BGUnload(FMISSION_VIEW_PTR wk )
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
static void _sub_TMenuInit( FMISSION_VIEW_PTR wk )
{
  wk->key_cursor = APP_KEYCURSOR_Create( FCOL_POPUP_BASE, FALSE, TRUE, wk->heap_sID );

  wk->tmenu = BVCTMenu_Init( FRM_MENUMSG, BG_PAL_TASKMENU, wk->fontHandle, wk->printQue, wk->heap_sID );
}

//--------------------------------------------------------------
/**
 * メニュー解放
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_TMenuRelease( FMISSION_VIEW_PTR wk )
{
  BVCTMenu_Release( wk->tmenu );
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
static void _sub_ActorResourceLoad( FMISSION_VIEW_PTR wk, ARCHANDLE *handle)
{
  int i;
  ARCHANDLE* handle_union = GFL_ARC_OpenDataHandle( ARCID_WIFIUNIONCHAR, wk->tmpHeapID );

  {
    const OBJ_RES_SRC srcNormal[OBJ_RES_MAX] = {
      { 1, NARC_fmission_log_fmission_obj_nclr },
      { 1, NARC_fmission_log_fmission_obj_lz_ncgr },
      { 1, NARC_fmission_log_fmission_obj_ncer },
    };
    obj_ObjResInit( wk, &wk->objResNormal, srcNormal, handle, TRUE );
  }
  
  {
    const OBJ_RES_SRC srcUnion[OBJ_RES_MAX] = {
     { 0, 0 },
     { PANEL_MAX, NARC_wifi_unionobj_front00_NCGR },
     { 1,NARC_wifi_unionobj_front00_NCER },
    };
    obj_ObjResInit( wk, &wk->objResUnion, srcUnion, handle_union, FALSE );
  }

  //再転送用のリソースをロードしておく
  PaletteWorkSet_ArcHandle( wk->pfd, handle, NARC_fmission_log_fmission_obj_nclr,
      wk->tmpHeapID, FADE_SUB_OBJ, 0x20*ACT_PAL_WMI, 0 );
  PaletteWorkSet_Arc( wk->pfd, ARCID_WMI, NARC_wmi_wm_NCLR, //通信アイコン
      wk->tmpHeapID, FADE_SUB_OBJ, 0x20, ACT_PAL_WMI*16 );

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
static void _sub_ActorResourceUnload( FMISSION_VIEW_PTR wk )
{
  int i;
  for( i = 0;i < UNION_CHAR_MAX;i++){
    GFL_HEAP_FreeMemory( wk->resCharUnion[i].buf );
  }
  GFL_HEAP_FreeMemory( wk->resPlttUnion.buf );

  obj_ObjResRelease( wk, &wk->objResUnion );
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
static void _sub_ActorCreate( FMISSION_VIEW_PTR wk, ARCHANDLE *handle )
{
  int i;

  //セル系システムの作成
  wk->cellUnit = GFL_CLACT_UNIT_Create( FMISSION_VIEW_OBJ_MAX , 0 , wk->heapID );
 
  //レンダラー作成
  {
    const GFL_REND_SURFACE_INIT renderInitData = { 0,512,256,192,CLSYS_DRAW_SUB, CLSYS_REND_CULLING_TYPE_NOT_AFFINE};
    
    wk->cellRender = GFL_CLACT_USERREND_Create( &renderInitData , 1 , wk->heap_sID );
    GFL_CLACT_UNIT_SetUserRend( wk->cellUnit, wk->cellRender );
  }

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
static void _sub_ActorDelete( FMISSION_VIEW_PTR wk )
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
static void bmpwin_Add( FMISSION_VIEW_PTR wk, BMP_WIN* win, u8 frm, u8 pal, u8 px, u8 py, u8 sx, u8 sy )
{
  win->fontHandle = wk->fontHandle;
  win->printQue = wk->printQue;
  win->pTcbSys = wk->pTcbSys;

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
static void _sub_BmpWinCreate(FMISSION_VIEW_PTR wk)
{
  bmpwin_Add( wk, &wk->win[WIN_POPUP],
      BMP_POPUP_FRM, BMP_POPUP_PAL, BMP_POPUP_PX, BMP_POPUP_PY,BMP_POPUP_SX, BMP_POPUP_SY );
  bmpwin_Add( wk, &wk->win[WIN_TITLE],
      BMP_TITLE_FRM, BMP_TITLE_PAL, BMP_TITLE_PX, BMP_TITLE_PY,BMP_TITLE_SX, BMP_TITLE_SY );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_BmpWinDelete(FMISSION_VIEW_PTR wk)
{
  int i;
	
  for(i = 0; i < WIN_MAX; i++){
    if( wk->win[i].win != NULL ){
      GFL_BMPWIN_Delete( wk->win[i].win );
    }
  }
}


//--------------------------------------------------------------
/**
 * アクターリソース取得
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void obj_ObjResInit( FMISSION_VIEW_PTR wk, OBJ_RES_TBL* res, const OBJ_RES_SRC* srcTbl, ARCHANDLE* p_handle, BOOL comp_f )
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
                              comp_f, CLSYS_DRAW_SUB, wk->heapID );
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
static void obj_ObjResRelease( FMISSION_VIEW_PTR wk, OBJ_RES_TBL* res )
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
  FMISSION_VIEW_PTR wk, u32 cgrNo, u32 palNo, u32 cellNo,
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
static void act_NormalObjAdd( FMISSION_VIEW_PTR wk ) 
{
  int i;
  static const u8 prm[ACT_MAX][2] = {
   { ACT_MENU_PX+ACT_MENU_OX*0 ,ACT_MENU_PY },
   { ACT_MENU_PX+ACT_MENU_OX*1 ,ACT_MENU_PY },
   { ACT_MENU_PX+ACT_MENU_OX*2 ,ACT_MENU_PY },
   { ACT_INFO_PX ,ACT_INFO_PY },
   { ACT_UP_PX ,ACT_MENU_PY },
   { ACT_DOWN_PX ,ACT_MENU_PY },
  };

  for(i = 0;i < ACT_MAX;i++){
    wk->pAct[ACT_POWER+i] = obj_ObjAdd( wk,
      wk->objResNormal.res[OBJ_RES_CGR].tbl[0],
      wk->objResNormal.res[OBJ_RES_PLTT].tbl[0],
      wk->objResNormal.res[OBJ_RES_CELLANIM].tbl[0],
      prm[i][0], prm[i][1], ACTANM_POWER_ON+(ACT_ANM_SET*i),
      OBJ_MENU_BG_PRI, OBJ_SPRI_MENU);
  
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_POWER+i], TRUE );
  }
}

/*
 *  @brief  ノーマルオブジェ破棄
 */
static void act_NormalObjDel( FMISSION_VIEW_PTR wk )
{
  int i;

  for(i = 0;i < ACT_MAX;i++){
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_POWER+i], FALSE );
    GFL_CLACT_WK_Remove( wk->pAct[ACT_POWER+i]);
  }
}

/*
 *  @brief  パネル用FontOam生成
 */
static void act_PanelFOamAdd( FMISSION_VIEW_PTR wk, FONT_OAM* foam, u16 sx, u16 sy, s16 px, s16 py )
{
	BMPOAM_ACT_DATA	finit;

	finit.bmp = GFL_BMP_Create( sx, sy, GFL_BMP_16_COLOR, wk->heap_sID );
	finit.x = px;
	finit.y = py;
	finit.pltt_index = wk->objResNormal.res[OBJ_RES_PLTT].tbl[0];
	finit.pal_offset = ACT_PAL_FONT;		// pltt_indexのパレット内でのオフセット
	finit.soft_pri = OBJ_SPRI_MSG;			// ソフトプライオリティ
	finit.bg_pri = OBJ_BG_PRI;				// BGプライオリティ
	finit.setSerface = ACT_RENDER_ID;
	finit.draw_type  = CLSYS_DRAW_SUB;

  BVCPrint_FontOamAdd( foam, &finit, wk->bmpOam, wk->printQue, wk->fontHandle, wk->pTcbSys );
  
  GFL_BMP_Clear( foam->bmp, 0 );
	BmpOam_ActorBmpTrans( foam->oam );
	
  BmpOam_ActorSetDrawEnable( foam->oam, FALSE );
}

/**
 *  @brief  パネル生成
 */
static void act_PanelObjAdd( FMISSION_VIEW_PTR wk, u8 idx )
{
  PANEL_WORK* pp = &(wk->panel[idx]);

  MI_CpuClear8( pp, sizeof(PANEL_WORK));

  pp->id = idx;
  pp->data_idx = PANEL_DATA_BLANK;
  pp->n_line = 0;
  pp->que = wk->printQue;

  pp->px = ACT_PANEL_OX*(idx+1)+ACT_PANEL_PX;
  pp->py = ACT_PANEL_OY*(idx+1)+ACT_PANEL_PY;

  //パネルアクター
  pp->cPanel = obj_ObjAdd( wk,
    wk->objResNormal.res[OBJ_RES_CGR].tbl[0],
    wk->objResNormal.res[OBJ_RES_PLTT].tbl[0],
    wk->objResNormal.res[OBJ_RES_CELLANIM].tbl[0],
    pp->px, pp->py, ACTANM_PANEL_ACT+(idx%3), OBJ_BG_PRI, OBJ_SPRI_PANEL+idx);
//  GFL_CLACT_WK_SetPlttOffs( pp->cPanel, idx, CLWK_PLTTOFFS_MODE_OAM_COLOR );
  
  //Unionオブジェアクター
  pp->cUnion = obj_ObjAdd( wk,
    wk->objResUnion.res[OBJ_RES_CGR].tbl[idx],
    wk->objResNormal.res[OBJ_RES_PLTT].tbl[0],
    wk->objResUnion.res[OBJ_RES_CELLANIM].tbl[0],
    pp->px+ACT_UNION_OX, pp->py+ACT_UNION_OY, 0, OBJ_BG_PRI, OBJ_SPRI_UNION+idx);
  GFL_CLACT_WK_SetPlttOffs( pp->cUnion, idx + ACT_PAL_UNION , CLWK_PLTTOFFS_MODE_PLTT_TOP );

  //fontOam生成
  act_PanelFOamAdd( wk, &pp->msgOam[PANEL_FOAM_NAME], BMP_PANEL_NAME_OAM_SX, BMP_PANEL_NAME_OAM_SY, pp->px+ACT_NAME_OX, pp->py+ACT_NAME_OY );
  pp->str[PANEL_FOAM_NAME] = GFL_STR_CreateBuffer( FMISSION_LOG_BUFLEN_NAME, wk->heap_sID );
  
  act_PanelFOamAdd( wk, &pp->msgOam[PANEL_FOAM_MSG], BMP_PANEL_MSG_OAM_SX, BMP_PANEL_MSG_OAM_SY, pp->px+ACT_MSG_OX, pp->py+ACT_MSG_OY );
  pp->str[PANEL_FOAM_MSG] = GFL_STR_CreateBuffer( FMISSION_LOG_BUFLEN_MSG, wk->heap_sID );

  GFL_CLACT_WK_SetDrawEnable( pp->cPanel, FALSE );
  GFL_CLACT_WK_SetDrawEnable( pp->cUnion, FALSE );
}

/**
 *  @brief  パネル削除
 */
static void act_PanelObjDel( FMISSION_VIEW_PTR wk, u8 idx )
{
  int i;

  PANEL_WORK* pp = &(wk->panel[idx]);

  for(i = 0; i < PANEL_FOAM_MAX; i++){
    GFL_STR_DeleteBuffer( pp->str[i] );
    BVCPrint_FontOamDel( &pp->msgOam[i] );
  }
  GFL_CLACT_WK_SetDrawEnable( pp->cUnion, FALSE );
  GFL_CLACT_WK_Remove( pp->cUnion );
  GFL_CLACT_WK_SetDrawEnable( pp->cPanel, FALSE );
  GFL_CLACT_WK_Remove( pp->cPanel );
 
  MI_CpuClear8( pp, sizeof(PANEL_WORK));
}

