//==============================================================================
/**
 * @file    fmission_view_sub.c
 * @brief   フェスミッション下画面 サブルーチン
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
#include "infowin/infowin.h"
#include "app_menu_common.naix"
#include "arc_def.h"
#include "font/font.naix"
#include "field/beacon_view.h"
#include "print\printsys.h"
#include "message.naix"

#include "field/research_team_def.h"
#include "beacon_status.naix"
#include "wifi_unionobj.naix"
#include "beacon_view_local.h"
#include "beacon_view_common.h"
#include "fmission_view_sub.h"

#include "gamesystem/fest_mission_nw.h"
#include "arc/wifi_unionobj_plt.cdat"
#include "msg/script/msg_festmission_event.h"

#define SCORE_NUM_SX    (40)
#define SCORE_NUM_SY    (16)
#define TOTAL_SCORE_PX  (8*17)
#define MY_SCORE_NUM_PX (TOTAL_SCORE_PX-SCORE_NUM_SX-4)
#define TOTAL_SCORE_NUM_PX (8*BMP_FMSCORE_SX-SCORE_NUM_SX)

#define TIME_NUM_SX     (8*5)
#define TIME_NUM_SY     (16)
#define TIME_NUM_PX     (BMP_FMTIME_SX*8-TIME_NUM_SX)

typedef struct _MENU_BAR_DATA{
  u8 act;
  u8 anm;
  u16 se;
  u16 menu;
}MENU_BAR_DATA;

static int touchin_CheckMenu( BEACON_VIEW_PTR wk, POINT* tp );
static BOOL effReq_PopupMsg( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info, BOOL new_f );

static void draw_MissionInfo( BEACON_VIEW_PTR wk );
static void draw_MissionNowScore( BEACON_VIEW_PTR wk );
static void draw_EntryNum( BEACON_VIEW_PTR wk, int* task_ct );
static void draw_RestTime( BEACON_VIEW_PTR wk, int* task_ct );

//////////////////////////////////////////////////////////////////

/*
 *  @brief  初期描画
 */
void FMissionView_InitialDraw( BEACON_VIEW_PTR wk )
{
  wk->ctrl.fm_my_score = wk->fmwk->my_score;
  LINK_FESTIVAL_GetMissionResult( wk->fest_sys, &(wk->ctrl.fm_score) );
  wk->ctrl.fm_sec = wk->fmwk->life/60;
  wk->ctrl.fm_entry = LINK_FESTIVAL_GetMissionEntryNum( wk->fest_sys );

  PaletteTrans_AutoSet( wk->pfd, TRUE );

  //矢印と御礼メニューの見た目セット
  if( wk->my_data.power == GPOWER_ID_NULL ){
    BVCAct_AnmStart( wk->pAct[ACT_POWER], ACTANM_POWER_OFF );
  }
  GFL_NET_ReloadIconTopOrBottom( FALSE, wk->tmpHeapID );

  //ミッション情報表示
  draw_MissionInfo( wk );
  draw_EntryNum( wk, &wk->eff_task_ct );
  draw_MissionNowScore( wk );
  draw_RestTime( wk, &wk->eff_task_ct );
}

/*
 *  @brief  メインループ内での入力チェック
 */
int FMissionView_CheckInput( BEACON_VIEW_PTR wk )
{
  int ret;
  POINT tp;

  if( !GFL_UI_TP_GetPointTrg( (u32*)&tp.x, (u32*)&tp.y )){
    return SEQ_MAIN;
  }
  
  //メニューあたり判定
  ret = touchin_CheckMenu( wk, &tp );
  if(ret != GFL_UI_TP_HIT_NONE){
    switch( ret ){
    case MENU_POWER:
      return SEQ_GPOWER_USE;
    case MENU_HELLO:
      BVSEvent_Reserve( wk, EV_CALL_TALKMSG_INPUT );
      break;
    case MENU_RETURN:
      BVSEvent_Reserve( wk, EV_RETURN_CGEAR );
      break;
    }
    return SEQ_MAIN;
  }
  return SEQ_MAIN;
}

/*
 *  @brief  スタックからログを一件取り出し
 */
BOOL FMissionView_CheckStack( BEACON_VIEW_PTR wk )
{
  BOOL new_f = FALSE, popup_f = FALSE;
  int ofs,idx,old_max;
  PANEL_WORK* pp;

  //スタックに積まれた情報をチェック
#ifdef PM_DEBUG
  if(DEBUG_BEACON_STATUS_GetStackCheckThrowFlag(wk->b_status) ){
    return FALSE;
  }
#endif
  if( !GAMEBEACON_Stack_GetInfo( wk->infoStack, wk->newLogInfo, &wk->newLogTime ) ){
    return FALSE;
  }

  //エラーチェック
  if( GAMEBEACON_Check_Error( wk->newLogInfo)){
    return FALSE;
  }

  //新しいログを処理
  wk->newLogOfs = GAMEBEACON_InfoTblRing_SetBeacon( wk->infoLog,
                    wk->newLogInfo, wk->newLogTime, &wk->first_entry_f );
  wk->newLogIdx = GAMEBEACON_InfoTblRing_Ofs2Idx( wk->infoLog, wk->newLogOfs );

  wk->newLogPanel = NULL;
  if( wk->first_entry_f ){
    wk->log_count = MISC_CrossComm_IncSuretigaiCount( wk->misc_sv );    
  }

  //登録数更新
  wk->old_list_max = wk->ctrl.max;
  if( wk->ctrl.max < BS_LOG_MAX ){
    wk->ctrl.max += wk->first_entry_f;
  
    if( wk->ctrl.max <= PANEL_VIEW_MAX){
      wk->ctrl.view_max = wk->ctrl.max;
    }
  }
  return TRUE;
}

/**
 *  @brief  描画状態アップデート
 */
void FMissionView_ViewUpdate( BEACON_VIEW_PTR wk )
{
  u32 sec;
  u32 entry;
  u32 my_score;
  u32 score;

  wk->init_f = TRUE;
 
  sec = wk->fmwk->life/60;
  if( sec != wk->ctrl.fm_sec ){
    wk->ctrl.fm_sec = sec; 
    draw_RestTime( wk, &wk->eff_task_ct );
  }

  entry = LINK_FESTIVAL_GetMissionEntryNum( wk->fest_sys );
  if( entry != wk->ctrl.fm_entry ){
    wk->ctrl.fm_entry = entry;
    draw_EntryNum( wk, &wk->eff_task_ct );
  }
  
  LINK_FESTIVAL_GetMissionResult( wk->fest_sys, &score );
  if( score != wk->ctrl.fm_score ){
    wk->ctrl.fm_my_score = wk->fmwk->my_score;
    wk->ctrl.fm_score = score;
    draw_MissionNowScore( wk );
  }

  wk->init_f = FALSE;
}

////////////////////////////////////////////////////////////////////////////
//==========================================================================
//サブシーケンス
//==========================================================================
////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
/*
 *  @brief  サブシーケンス　ログエントリー　
 */
///////////////////////////////////////////////////////////////////

/*
 *  @brief  サブシーケンス　Newログエントリーメイン
 */
BOOL FMissionView_SubSeqLogEntry( BEACON_VIEW_PTR wk )
{
  switch( wk->sub_seq ){
  case 0:
    {
      BOOL popup_f;

      //ポップアップリクエスト
      popup_f = effReq_PopupMsg( wk, wk->newLogInfo, wk->first_entry_f );

      //新規でない時はスクロール無し
      if(popup_f){
        BVC_MenuBarViewSet( wk, MENU_ALL, MENU_ST_OFF );
        BVCSound_PlaySE( BVIEW_SE_ICON );
      }
      if( wk->first_entry_f ){
        BVCDraw_MenuWindow( wk, msg_sys_now_record, &wk->eff_task_ct );
      }
      return TRUE;
    }
    break;
  }
  return FALSE;
}

/*
 *  @brief  メニューバーあたり判定
 */
static int touchin_CheckMenu( BEACON_VIEW_PTR wk, POINT* tp )
{
  int i, ret;
  GFL_UI_TP_HITTBL tbl[2];
  static MENU_BAR_DATA const dat[3] = {
    { ACT_POWER, ACTANM_POWER_ON, BVIEW_SE_MENU, MENU_POWER },
    { ACT_HELLO, ACTANM_HELLO_ON, BVIEW_SE_MENU, MENU_HELLO },
    { ACT_RETURN, ACTANM_RETURN_ON, BVIEW_SE_CANCEL, MENU_RETURN },
  };

  tbl[1].rect.top = GFL_UI_TP_HIT_END;

  for(i = 0;i < ACT_MENU_FMISSION_NUM;i++){
    u8 seq = GFL_CLACT_WK_GetAnmSeq( wk->pAct[dat[i].act] )-(dat[i].anm);
    if( (seq % ACT_ANM_SET) >= 2){
      continue;
    }
    tbl[0].rect.top = ACT_MENU_PY;
    tbl[0].rect.bottom = tbl[0].rect.top+ACT_MENU_SY;
    tbl[0].rect.left = ACT_MENU_FMISSION_PX+ACT_MENU_OX*i;
    tbl[0].rect.right = tbl[0].rect.left+ACT_MENU_SX;

    ret = GFL_UI_TP_HitSelf( tbl, tp->x, tp->y );
    if(ret != GFL_UI_TP_HIT_NONE) {
      BVCSound_PlaySE( dat[i].se );
      return dat[i].menu;
    }
  }
  return GFL_UI_TP_HIT_NONE;
}

/*
 *  @brief  すれ違いログ　ポップアップリクエスト
 */
static BOOL effReq_PopupMsg( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info, BOOL new_f )
{
  GAMEBEACON_ACTION action = GAMEBEACON_Get_Action_ActionNo( info );

  if( new_f ){
    GAMEBEACON_InfoWordset( info, wk->wordset, wk->tmpHeapID );
    BVCEffReq_PopupMsgSys( wk, msg_beacon_001 );
    return TRUE;
  }
  switch( action ){
  case GAMEBEACON_ACTION_SEARCH:
    return FALSE; //新着でないサーチはポップアップ出さない

  case GAMEBEACON_ACTION_GPOWER:
    return BVCEffReq_PopupMsgGPower( wk, info, FALSE );

  case GAMEBEACON_ACTION_DISTRIBUTION_GPOWER:
    return BVCEffReq_PopupMsgGPower( wk, info, TRUE );

  case GAMEBEACON_ACTION_FEST_MISSION:
    return FALSE;

  case GAMEBEACON_ACTION_THANKYOU:
    //御礼を受けた回数インクリメント
    GAMEBEACON_Set_ThankyouOver( MISC_CrossComm_IncThanksRecvCount( wk->misc_sv ) );

    //ブレイクスルー
  default:
    BVCEffReq_PopupMsgFromInfo( wk, info );
    break;
  }
  return TRUE;
}

/*
 *  @brief  ミッション概要表示
 */
static void draw_MissionInfo( BEACON_VIEW_PTR wk )
{
  FEST_MISSION_GetExpStr( wk->wordset, wk->mm_fmission, wk->str_expand, wk->fmwk, wk->heapID );
  BVCPrintReq_BmpwinPrint( wk, &wk->win[WIN_FMISSION], wk->str_expand, 0, 0, FCOL_LOGNUM_BASE, FCOL_LOGNUM, NULL );
  
  FEST_MISSION_WordsetParamAll( wk->wordset, wk->fmwk );

  BVCPrint_GetMsgToBuf( wk, msg_sys_fmission_reward );
  BVCPrintReq_BmpwinPrintEx( wk, &wk->win[WIN_FMISSION], wk->str_expand, 0, 32, FCOL_LOGNUM_BASE, FCOL_LOGNUM, NULL, FALSE );

  BVCPrint_GetMsgToBuf( wk, msg_sys_fmission_score01 );
  BVCPrintReq_BmpwinPrintEx( wk, &wk->win[WIN_FMSCORE], wk->str_expand, 0, 0, FCOL_LOGNUM_BASE, FCOL_LOGNUM, NULL, TRUE );
  BVCPrint_GetMsgToBuf( wk, msg_sys_fmission_score02 );
  BVCPrintReq_BmpwinPrintEx( wk, &wk->win[WIN_FMSCORE], wk->str_expand, TOTAL_SCORE_PX, 0, FCOL_LOGNUM_BASE, FCOL_LOGNUM, NULL, FALSE );
  
  BVCPrint_GetMsgToBuf( wk, msg_sys_fmission_time );
  BVCPrintReq_BmpwinPrintEx( wk, &wk->win[WIN_FMTIME], wk->str_expand, 0, 0, FCOL_LOGNUM_BASE, FCOL_LOGNUM, NULL, TRUE );
}

/*
 *  @brief  ミッションスコア表示
 */
static void draw_MissionNowScore( BEACON_VIEW_PTR wk )
{
  GFL_BMP_Fill( wk->win[WIN_FMSCORE].bmp, MY_SCORE_NUM_PX, 0, SCORE_NUM_SX, SCORE_NUM_SY, FCOL_LOGNUM_BASE );
  GFL_BMP_Fill( wk->win[WIN_FMSCORE].bmp, TOTAL_SCORE_NUM_PX, 0, SCORE_NUM_SX, SCORE_NUM_SY, FCOL_LOGNUM_BASE );

  WORDSET_RegisterNumber( wk->wordset, 0, wk->ctrl.fm_my_score, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  BVCPrint_GetMsgToBuf( wk, msg_sys_fmission_score_num );
  BVCPrintReq_BmpwinPrintEx( wk, &wk->win[WIN_FMSCORE], wk->str_expand, MY_SCORE_NUM_PX, 0, FCOL_LOGNUM_BASE, FCOL_LOGNUM, &wk->eff_task_ct, FALSE );

  WORDSET_RegisterNumber( wk->wordset, 0, wk->ctrl.fm_score, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  BVCPrint_GetMsgToBuf( wk, msg_sys_fmission_score_num );
  BVCPrintReq_BmpwinPrintEx( wk, &wk->win[WIN_FMSCORE], wk->str_expand, TOTAL_SCORE_NUM_PX, 0, FCOL_LOGNUM_BASE, FCOL_LOGNUM, &wk->eff_task_ct, FALSE );
}

/*
 *  @brief  参加人数表示
 */
static void draw_EntryNum( BEACON_VIEW_PTR wk, int* task_ct )
{
  WORDSET_RegisterNumber( wk->wordset, 0, wk->ctrl.fm_entry, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );

  BVCPrint_GetMsgToBuf( wk, msg_sys_lognum );
  BVCPrintReq_BmpwinPrint( wk, &wk->win[WIN_LOGNUM], wk->str_expand, 0, 0, FCOL_LOGNUM_BASE, FCOL_LOGNUM, task_ct );
}

/*
 *  @brief 残り時間表示 
 */
static void draw_RestTime( BEACON_VIEW_PTR wk, int* task_ct )
{
  int min = wk->ctrl.fm_sec/60;
  int sec = wk->ctrl.fm_sec%60;

  WORDSET_RegisterNumber( wk->wordset, 0, min, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wk->wordset, 1, sec, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

  GFL_BMP_Fill( wk->win[WIN_FMTIME].bmp, TIME_NUM_PX, 0, TIME_NUM_SX, TIME_NUM_SY, FCOL_LOGNUM_BASE );
  BVCPrint_GetMsgToBuf( wk, msg_sys_fmission_time_num );
  BVCPrintReq_BmpwinPrintEx( wk, &wk->win[WIN_FMTIME], wk->str_expand, TIME_NUM_PX, 0, FCOL_LOGNUM_BASE, FCOL_LOGNUM, task_ct, FALSE );
}

