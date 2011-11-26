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
#include "system/nsbtx_to_clwk.h"
#include "gamesystem/gamesystem.h"
#include "field/field_subscreen.h"
#include "gamesystem/game_beacon.h"
#include "infowin/infowin.h"
#include "app_menu_common.naix"
#include "arc_def.h"
#include "font/font.naix"
#include "field/fmission_view.h"
#include "print\printsys.h"
#include "message.naix"
#include "app/name_input.h"
#include "field/research_team_def.h"
#include "beacon_status.naix"
#include "wifi_unionobj.naix"
#include "fmission_view_local.h"
#include "fmission_view_sub.h"

#include "gamesystem/fest_mission_nw.h"
#include "gamesystem/fmission_log.h"
#include "gamesystem/fmission_log_accessor_fv.h"
#include "arc/wifi_unionobj_plt.cdat"
#include "fieldmap/fldmmdl_mdlres.naix" //フィールド人物OBJ
#include "msg/script/msg_fmission_event.h"

#define SCORE_NUM_SX    (40)
#define SCORE_NUM_SY    (16)
#define TOTAL_SCORE_PX  (8*17)
#define MY_SCORE_NUM_PX (TOTAL_SCORE_PX-SCORE_NUM_SX-4)
#define TOTAL_SCORE_NUM_PX (8*BMP_FMSCORE_SX-SCORE_NUM_SX)

#define TIME_NUM_SX     (8*5)
#define TIME_NUM_SY     (16)
#define TIME_NUM_PX     (BMP_FMTIME_SX*8-TIME_NUM_SX)

typedef struct _MENU_BAR_DATA{
  u8  px;
  u8  py;
  u8  key;
}MENU_BAR_DATA;

static int touchin_CheckMenu( FMISSION_VIEW_PTR wk, POINT* tp );
static BOOL effReq_PopupMsg( FMISSION_VIEW_PTR wk, GAMEBEACON_INFO* info, BOOL new_f );

static void print_GetMsgToBufEx( FMISSION_VIEW_PTR wk, u8 msg_id, const GFL_MSGDATA* handle );
static void print_GetMsgToBuf( FMISSION_VIEW_PTR wk, u8 msg_id );
static void print_PopupWindow( FMISSION_VIEW_PTR wk, STRBUF* str, int* task_ct );

static void draw_MissionInfo( FMISSION_VIEW_PTR wk );
static void draw_EntryNum( FMISSION_VIEW_PTR wk, int* task_ct );
static void draw_MenuIconVisibleSet( FMISSION_VIEW_PTR wk, BOOL visible_f );

static void obj_UpDownViewSet( FMISSION_VIEW_PTR wk );
static void panel_VisibleSet( PANEL_WORK* pp, BOOL visible_f );
static void panel_IconVisibleSet( PANEL_WORK* pp, BOOL visible_f );
static void panel_Clear( PANEL_WORK* pp );
static void panel_Entry( PANEL_WORK* pp, u8 data_idx,u8 line );
static PANEL_WORK* panel_GetNextTargetPanel( FMISSION_VIEW_PTR wk, SCROLL_DIR dir );
static void panel_UnionObjUpdate( FMISSION_VIEW_PTR wk, PANEL_WORK* pp );
static void panel_BmpTrans( FMISSION_VIEW_PTR wk, PANEL_WORK* pp );
static void panel_DrawStart( FMISSION_VIEW_PTR wk, PANEL_WORK* pp, FMISSION_LOG* pl );
static void panel_DrawFinish( FMISSION_VIEW_PTR wk, PANEL_WORK* pp );
static BOOL panel_DrawMain( FMISSION_VIEW_PTR wk, PANEL_WORK* pp );
static void panel_Draw( FMISSION_VIEW_PTR wk, PANEL_WORK* pp, FMISSION_LOG* pl );
static void eff_IconEffectAddCheck( FMISSION_VIEW_PTR wk, PANEL_WORK* pp );

static void list_UpDownReq( FMISSION_VIEW_PTR wk, SCROLL_DIR dir );
static void list_UpDownReq1st( FMISSION_VIEW_PTR wk, SCROLL_DIR dir, int ret_seq );
static void list_ScrollReq( FMISSION_VIEW_PTR wk, FMISSION_LOG* pl, u8 idx, SCROLL_DIR dir, BOOL new_f );

#if 0
static void effReq_PanelScroll( FMISSION_VIEW_PTR wk, u8 dir, PANEL_WORK* new_pp, int* task_ct );
#endif
static void effReq_PopupMsgSys( FMISSION_VIEW_PTR wk, u8 msg_id );
static void effReq_PopupMsgRetire( FMISSION_VIEW_PTR wk );

static BOOL effReq_PopupMsgGPower( FMISSION_VIEW_PTR wk,  FMISSION_LOG* pl );
static BOOL effReq_PopupMsgGPowerSpecial( FMISSION_VIEW_PTR wk, GPOWER_ID gpower_id, STRBUF* name_buf );

//////////////////////////////////////////////////////////////////

/*
 *  @brief  初期描画
 */
void FMissionView_InitialDraw( FMISSION_VIEW_PTR wk )
{
  int i;

  wk->ctrl.fm_join_num = LINK_FESTIVAL_GetMissionJoinNum( wk->fest_sys );

  PaletteTrans_AutoSet( wk->pfd, TRUE );
  
  //最新リストを描画
  for(i = 0;i < wk->ctrl.view_max;i++){
    int idx, ofs;
    PANEL_WORK* pp;

    ofs = wk->ctrl.view_top+i;
    FMISSION_LOG_ViewTbl_GetLog( wk->logView, wk->tmpLog, ofs );
    idx = FMISSION_LOG_ViewTbl_GetOfs2Idx( wk->logView, ofs );
  
    pp = panel_GetNextTargetPanel( wk, SCROLL_NONE );
    panel_Entry( pp, idx, PANEL_VIEW_START+i );  //パネル新規登録
    panel_Draw( wk, pp, wk->tmpLog );   //パネル描画
    panel_VisibleSet( pp, TRUE );   //パネル描画
  }

  //メニューの見た目セット
  FMissionView_MenuBarViewSet( wk, MENU_ALL, MENU_ST_ON );

  if( wk->my_data.power == GPOWER_ID_NULL ){
    BVCAct_AnmStart( wk->pAct[ACT_POWER], ACTANM_POWER_OFF );
  }
  
  GFL_NET_ReloadIconTopOrBottom( FALSE, wk->tmpHeapID );

  //ミッション情報表示
  draw_MissionInfo( wk );
  draw_EntryNum( wk, &wk->en_print_task_ct );
}

/*
 *  @brief  メインループ内での入力チェック
 */
int FMissionView_CheckInput( FMISSION_VIEW_PTR wk )
{
  int ret;
  POINT tp;
 
#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
    FVSEvent_Reserve( wk, EV_DEBUG_MENU_CALL );
    return SEQ_MAIN;
  }
#endif
 
  if( !GFL_UI_TP_GetPointTrg( (u32*)&tp.x, (u32*)&tp.y )){
    return SEQ_MAIN;
  }
  
  //メニューあたり判定
  ret = touchin_CheckMenu( wk, &tp );
  if(ret != GFL_UI_TP_HIT_NONE){
    switch( ret ){
    case MENU_POWER:
      //return SEQ_GPOWER_USE;
      FVSEvent_Reserve( wk, EV_GPOWER_USE_MINE );
      break;
    case MENU_HELLO:
      FVSEvent_Reserve( wk, EV_CALL_TALKMSG_INPUT );
      break;
    case MENU_RETIRE:
      return SEQ_RETIRE;

    case MENU_INFO:
      FVSEvent_Reserve( wk, EV_CALL_INFO_VIEW );
      break;
    case MENU_UP:
    case MENU_DOWN:
      list_UpDownReq1st( wk, (ret- MENU_UP)^1, SEQ_MAIN );  //スクロール方向は逆なので反転させてる
      return SEQ_LIST_SCROLL_REPEAT;
    }
    return SEQ_MAIN;
  }
  return SEQ_MAIN;
}

/*
 *  @brief  リスト上下ボタン　タッチ判定
 */
int FMissionView_CheckInoutTouchUpDown( FMISSION_VIEW_PTR wk )
{
  int ret;
  POINT tp;
  
  if( !GFL_UI_TP_GetPointCont( (u32*)&tp.x, (u32*)&tp.y )){
    return GFL_UI_TP_HIT_NONE;
  }
  ret = touchin_CheckMenu( wk, &tp );
  if( ret == MENU_UP || ret == MENU_DOWN ){
    return ((ret-MENU_UP)^1);
  }
  return GFL_UI_TP_HIT_NONE;
}

/*
 *  @brief  特殊ポップアップ起動チェック 
 */
BOOL FMissionView_CheckSpecialPopup( FMISSION_VIEW_PTR wk )
{
  u16* ret_fword_input =  BEACON_STATUS_GetFreeWordInputResultPointer( wk->b_status );

  //一言メッセージ入力があったかチェック
  if( (*ret_fword_input) == FALSE ){
    return FALSE;
  }
  *ret_fword_input = FALSE; //フラグリセット

  WORDSET_RegisterWord( wk->wordset, 0,
    BEACON_STATUS_GetFreeWordBuffer( wk->b_status ), wk->my_data.sex, TRUE, PM_LANG );
  effReq_PopupMsgSys( wk, msg_sys_free_word_send );
  return TRUE;
}

/*
 *  @brief  Gパワーポップアップ起動チェック 
 */
BOOL FMissionView_CheckSpecialGPower( FMISSION_VIEW_PTR wk )
{
  GPOWER_ID gpower_id;
  BOOL  spot_f;

  gpower_id = FMISSION_GPOWER_TBL_GetData( wk->logGPower, wk->str_tmp, &spot_f );
  
  if( gpower_id == GPOWER_ID_NULL ){
    return FALSE; //ストックがない
  }
  //ポップアップをリクエスト
  return effReq_PopupMsgGPowerSpecial( wk, gpower_id, wk->str_tmp );
}

/*
 *  @brief  スタックからログを一件取り出し
 */
BOOL FMissionView_CheckStack( FMISSION_VIEW_PTR wk )
{
  BOOL new_f = FALSE, popup_f = FALSE;
  int ofs,idx,old_max;
  PANEL_WORK* pp;

#if 0
  //スタックに積まれた情報をチェック
#ifdef PM_DEBUG
  if(DEBUG_BEACON_STATUS_GetStackCheckThrowFlag(wk->b_status) ){
    return FALSE;
  }
#endif
#endif

  if( !FMISSION_LOG_StackTbl_GetLog( wk->logStack, wk->newLog )){
    return FALSE;
  }

  //新しいログを処理
  wk->newLogOfs = FMISSION_LOG_ViewTbl_SetLog( wk->logView, wk->newLog );
  wk->newLogIdx = FMISSION_LOG_ViewTbl_GetOfs2Idx( wk->logView, wk->newLogOfs );

  wk->newLogPanel = NULL;

  //登録数更新
  wk->old_list_max = wk->ctrl.max;
  if( wk->ctrl.max < MISSION_LOG_MAX ){
    wk->ctrl.max += 1;
  
    if( wk->ctrl.max <= PANEL_VIEW_MAX){
      wk->ctrl.view_max = wk->ctrl.max;
    }
  }
  return TRUE;
}

/**
 *  @brief  描画状態アップデート
 */
void FMissionView_ViewUpdate( FMISSION_VIEW_PTR wk )
{
  u8  join = LINK_FESTIVAL_GetMissionJoinNum( wk->fest_sys );
  
  if( (join > wk->ctrl.fm_join_num) && (wk->en_print_task_ct == 0 )){
    wk->ctrl.fm_join_num = join;
    draw_EntryNum( wk, &wk->en_print_task_ct );
  }
}

//---------------------------------------------------
/*
 *  @brief  リストスクロールリクエスト
 */
//---------------------------------------------------
void FMissionView_ListScrollRepeatReq( FMISSION_VIEW_PTR wk )
{
  list_UpDownReq( wk, wk->scr_repeat_dir );
}

//---------------------------------------------------
/*
 *  @brief  メニューバーアニメセット
 */
//---------------------------------------------------
void FMissionView_MenuBarViewSet( FMISSION_VIEW_PTR wk, MENU_ID id, MENU_STATE state )
{
  u8 flag;
  
  if( id == MENU_ALL ){
    flag = MENU_F_ALL;
  }else{
    flag = 0x01 << id;
  }

  if( flag & MENU_F_POWER ){
    if( wk->my_data.power != GPOWER_ID_NULL ){
      BVCAct_AnmStart( wk->pAct[ACT_POWER], state+ACTANM_POWER_ON );
    }
  }
  if( flag & MENU_F_HELLO ){
    BVCAct_AnmStart( wk->pAct[ACT_HELLO], state+ACTANM_HELLO_ON );
  }
  if( flag & MENU_F_RETIRE ){
    BVCAct_AnmStart( wk->pAct[ACT_RETIRE], state+ACTANM_RETIRE_ON );
  }
  if( flag & MENU_F_INFO ){
    BVCAct_AnmStart( wk->pAct[ACT_INFO], state+ACTANM_INFO_ON );
  }
  if( flag & MENU_F_UPDOWN ){
    if( state == MENU_ST_ON ){
      obj_UpDownViewSet( wk );
    }else{
      BVCAct_AnmStart( wk->pAct[ACT_UP], state+ACTANM_UP_ON );
      BVCAct_AnmStart( wk->pAct[ACT_DOWN], state+ACTANM_DOWN_ON );
    }
  }
}

//---------------------------------------------------
/*
 *  @brief  メニューバーアニメウェイト
 */
//---------------------------------------------------
BOOL FMissionView_MenuBarCheckAnm( FMISSION_VIEW_PTR wk, MENU_ID id )
{
  return GFL_CLACT_WK_CheckAnmActive( wk->pAct[ACT_POWER+id] );
}

/*
 *  @brief  スクロールボタン状態セット
 */
void FMissionView_UpDownViewSet( FMISSION_VIEW_PTR wk )
{
  obj_UpDownViewSet( wk );
}

/*
 *  @brief  パネル更新リクエスト処理
 */
void FMissionView_PanelUpdate( FMISSION_VIEW_PTR wk )
{
  int i;

  for( i = 0;i < PANEL_MAX;i++){
    PANEL_WORK* pp = &wk->panel[i];

    if( pp->data_idx == PANEL_DATA_BLANK || !pp->trans_req_f ){
      continue;
    }
    panel_BmpTrans( wk, pp );
  }
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
BOOL FMissionView_SubSeqLogEntry( FMISSION_VIEW_PTR wk )
{
  static const u16 DATA_FreeWordSE[NAMEIN_SHORTCUT_RET_MAX] = {
    FVIEW_SE_FREEWORD,
    FVIEW_SE_FREEWORD_GREETING,
    FVIEW_SE_FREEWORD_THANKS,
    FVIEW_SE_FREEWORD_BOAST,
  };

  if( wk->eff_task_ct ){
    return FALSE;
  }

  switch( wk->sub_seq ){
  case 0:
    FMissionView_MenuBarViewSet( wk, MENU_ALL, MENU_ST_OFF );
    
    {
      //新規ログパラメータでSEを鳴らす
      FMISSION_LOG_ACTION action = FMISSION_LOG_GetAction( wk->newLog );
 
      switch( action ){
      case FMISSION_LOG_ACT_FREEWORD:
        BVCSound_PlaySE( DATA_FreeWordSE[ FMISSION_LOG_GetActionParam( wk->newLog )&0x03 ] );
        break;
      case FMISSION_LOG_ACT_NEW_ENTRY:
        if( wk->ctrl.fm_join_num < FVIEW_NEW_PLAYER_SE_CHANGE_JOIN_NUM ){
          BVCSound_PlaySE( FVIEW_SE_NEW_PLAYER );
        }else{
          BVCSound_PlaySE( FVIEW_SE_NEW_PLAYER_OVER10 );
        }
        break;
      default:
        if( FEST_MISSION_IsScoreUp( wk->fmwk, action, FMISSION_LOG_GetActionParam( wk->newLog ))){
          BVCSound_PlaySE( FVIEW_SE_SCOREUP );
        }else{
          BVCSound_PlaySE( FVIEW_SE_NORMAL_LOG );
        }
        break;
      }
    } 

    if( wk->ctrl.view_top > 0){ //描画リストがトップでない時はスクロールのみ
      wk->newLogOfs = wk->ctrl.view_top;
      FMISSION_LOG_ViewTbl_GetLog( wk->logView, wk->newLog, wk->newLogOfs );
      wk->newLogIdx = FMISSION_LOG_ViewTbl_GetOfs2Idx( wk->logView, wk->newLogOfs );
    }
    //スクロールリクエスト
    list_ScrollReq( wk, wk->newLog, wk->newLogIdx, SCROLL_DOWN, (wk->ctrl.view_top==0) );
    wk->ctrl.view_top = wk->newLogOfs;
    
    wk->sub_seq++;
    break;
  case 1:
    if( FMISSION_LOG_IsPlayerLog( wk->newLog ) ||
        FMISSION_LOG_GetAction( wk->newLog ) != FMISSION_LOG_ACT_GPOWER ){
      return TRUE;
    }
    //Gパワーポップアップリクエスト
    if( effReq_PopupMsgGPower( wk, wk->newLog ) == FALSE ){
      return TRUE;
    }
    wk->sub_seq++;
    break;
  default:
    return TRUE;
  }
  return FALSE;
}

///////////////////////////////////////////////////////////////////
/*
 *  @brief  サブシーケンス　リタイアチェック
 */
///////////////////////////////////////////////////////////////////

/*
 *  @brief  サブシーケンス　リタイアチェックメイン
 */
BOOL FMissionView_SubSeqRetire( FMISSION_VIEW_PTR wk )
{
  switch( wk->sub_seq ){
  case 0:
    BVC_PrintQueLimmitUpSet( wk->printQue, TRUE );
    effReq_PopupMsgRetire( wk );
    wk->sub_seq++;
    break;
  case 1:
    if( wk->eff_task_ct ){
      break;
    }
    BVC_PrintQueLimmitUpSet( wk->printQue, FALSE );
    wk->sub_seq = 0;
    return TRUE;
  }
  return FALSE;
}

/*
 *  @brief  メニューバーあたり判定
 */
static int touchin_CheckMenu( FMISSION_VIEW_PTR wk, POINT* tp )
{
  int i, ret;
  u8  enable = 0;

  GFL_UI_TP_HITTBL tbl[2];
#if 0
  static MENU_BAR_DATA const dat[3] = {
    { ACT_POWER, ACTANM_POWER_ON, BVIEW_SE_MENU, MENU_POWER },
    { ACT_HELLO, ACTANM_HELLO_ON, BVIEW_SE_MENU, MENU_HELLO },
    { ACT_RETURN, ACTANM_RETURN_ON, BVIEW_SE_CANCEL, MENU_RETURN },
  };
#endif
  static MENU_BAR_DATA const dat[ACT_MENU_NUM] = {
    { ACT_MENU_PX+(ACT_MENU_OX*0), ACT_MENU_PY, 0 },
    { ACT_MENU_PX+(ACT_MENU_OX*1), ACT_MENU_PY, 0 },
    { ACT_MENU_PX+(ACT_MENU_OX*2), ACT_MENU_PY, 0},
    { ACT_INFO_PX, ACT_INFO_PY, 0 },
    { ACT_UP_PX, ACT_MENU_PY, 0x01 },
    { ACT_DOWN_PX, ACT_MENU_PY, 0x02 },
  };

  tbl[1].rect.top = GFL_UI_TP_HIT_END;
  
  if( wk->ctrl.view_top > 0 ){  //上キーチェック
    enable |= 1;
  }
  if( (wk->ctrl.view_top+wk->ctrl.view_max) < wk->ctrl.max ){
    enable |= 2;
  }

  for(i = 0;i < ACT_MENU_NUM;i++){
    u8 seq = GFL_CLACT_WK_GetAnmSeq( wk->pAct[ACT_POWER+i] )-(ACTANM_POWER_ON+(ACT_ANM_SET*i));
    if( (seq % ACT_ANM_SET) >= 2){
      continue;
    }
    if( (dat[i].key != 0) && !(enable & dat[i].key ) ){
      continue;
    }
    tbl[0].rect.top = dat[i].py;
    tbl[0].rect.bottom = tbl[0].rect.top+ACT_MENU_SY;
    tbl[0].rect.left = dat[i].px;
    tbl[0].rect.right = tbl[0].rect.left+ACT_MENU_SX;

    ret = GFL_UI_TP_HitSelf( tbl, tp->x, tp->y );
    if(ret != GFL_UI_TP_HIT_NONE) {
      if( dat[i].key == 0 ){
        BVCSound_PlaySE( FVIEW_SE_MENU );
      }
      return i;
    }
  }
  return GFL_UI_TP_HIT_NONE;
}

/*
 *  @brief  すれ違いログ　ポップアップリクエスト
 */
static BOOL effReq_PopupMsg( FMISSION_VIEW_PTR wk, GAMEBEACON_INFO* info, BOOL new_f )
{
  GAMEBEACON_ACTION action = GAMEBEACON_Get_Action_ActionNo( info );
  return FALSE;

#if 0
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

  case GAMEBEACON_ACTION_THANKYOU:
    //御礼を受けた回数インクリメント
    GAMEBEACON_Set_ThankyouOver( MISC_CrossComm_IncThanksRecvCount( wk->misc_sv ) );

    //ブレイクスルー
  default:
    BVCEffReq_PopupMsgFromInfo( wk, info );
    break;
  }
  return TRUE;
#endif
}

//========================================================================
//文字列処理系
//========================================================================
/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
static void print_GetMsgToBufEx( FMISSION_VIEW_PTR wk, u8 msg_id, const GFL_MSGDATA* handle )
{
  GFL_MSG_GetString( handle, msg_id, wk->str_tmp);
  WORDSET_ExpandStr( wk->wordset, wk->str_expand, wk->str_tmp );
}

/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
static void print_GetMsgToBuf( FMISSION_VIEW_PTR wk, u8 msg_id )
{
  GFL_MSG_GetString( wk->mm_sys, msg_id, wk->str_tmp);
  WORDSET_ExpandStr( wk->wordset, wk->str_expand, wk->str_tmp );
}

/*
 *  @brief  ポップアップウィンドウ文字列描画
 */
static void print_PopupWindow( FMISSION_VIEW_PTR wk, STRBUF* str, int* task_ct )
{
  BVCPrintReq_BmpwinPrint( &wk->win[WIN_POPUP], str, 0, 0, FCOL_POPUP_BASE, FCOL_POPUP, task_ct, wk->init_f );
}

/*
 *  @brief  ミッション概要表示
 */
static void draw_MissionInfo( FMISSION_VIEW_PTR wk )
{
  int width;
  //タイトル描画(センタリング)
  FMISSION_DATA_GetTitleStr( wk->fm_data_man, wk->str_expand, wk->fmwk, wk->tmpHeapID );

  width = PRINTSYS_GetStrWidth( wk->str_expand, wk->fontHandle, 0 );
  width = (BMP_TITLE_BSX-width)/2;
  BVCPrintReq_BmpwinPrint( &wk->win[WIN_TITLE], wk->str_expand,
      width, BMP_TITLE_OY, FCOL_TITLE_BASE, FCOL_TITLE, NULL, wk->init_f );

  //目標値描画
  WORDSET_RegisterNumber( wk->wordset, 0, wk->fmwk->beacon.goal, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  print_GetMsgToBuf( wk, msg_sys_goal_num00+wk->fmwk->beacon.unit );
  BVCPrintReq_BmpwinPrint( &wk->win[WIN_GOAL], wk->str_expand,
      BMP_GOAL_STR_OX, BMP_GOAL_STR_OY, FCOL_TITLE_BASE, FCOL_TITLE, NULL, wk->init_f );
}

/*
 *  @brief  参加人数表示
 */
static void draw_EntryNum( FMISSION_VIEW_PTR wk, int* task_ct )
{
  GFL_BMP_Clear( wk->win[WIN_ENTRY].bmp, FCOL_TITLE_BASE );
  WORDSET_RegisterNumber( wk->wordset, 0, wk->ctrl.fm_join_num, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  
  print_GetMsgToBuf( wk, msg_sys_entry_num );
  BVCPrintReq_BmpwinPrintEx( &wk->win[WIN_ENTRY], wk->str_expand,
      0, 0, FCOL_TITLE_BASE, FCOL_TITLE, task_ct, wk->init_f, FALSE );
}

/*
 *  @brief  メニューアクター Viewセット
 */
static void draw_MenuIconVisibleSet( FMISSION_VIEW_PTR wk, BOOL visible_f )
{
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_POWER], visible_f );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_HELLO], visible_f );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_RETIRE], visible_f );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_UP], visible_f );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_DOWN], visible_f );
}

/*
 *  @brief  UpDownアクター Viewセット
 */
static void obj_UpDownViewSet( FMISSION_VIEW_PTR wk )
{
  if(wk->ctrl.view_top == 0){
    GFL_CLACT_WK_SetAnmSeq( wk->pAct[ACT_UP], ACTANM_UP_OFF );  
  }else{
    GFL_CLACT_WK_SetAnmSeq( wk->pAct[ACT_UP], ACTANM_UP_ON );  
  }
  if((wk->ctrl.view_top+wk->ctrl.view_max) < (wk->ctrl.max)){
    GFL_CLACT_WK_SetAnmSeq( wk->pAct[ACT_DOWN], ACTANM_DOWN_ON );  
  }else{
    GFL_CLACT_WK_SetAnmSeq( wk->pAct[ACT_DOWN], ACTANM_DOWN_OFF );  
  }
}

/*
 *  @brief  パネル 表示状態On/Off
 */
static void panel_VisibleSet( PANEL_WORK* pp, BOOL visible_f )
{
  int i;

  for(i = 0;i < PANEL_FOAM_MAX;i++){
    BmpOam_ActorSetDrawEnable( pp->msgOam[i].oam, visible_f );
  }
  if( !visible_f ){
    panel_IconVisibleSet( pp, visible_f );
  }
  GFL_CLACT_WK_SetDrawEnable( pp->cUnion, visible_f );
  GFL_CLACT_WK_SetDrawEnable( pp->cPanel, visible_f );
}

/*
 *  @brief  パネル アイコン表示状態On/Off
 */
static void panel_IconVisibleSet( PANEL_WORK* pp, BOOL visible_f )
{
  GFL_CLACT_WK_SetDrawEnable( pp->cIcon, visible_f );
}

/*
 *  @brief  パネルクリアー
 */
static void panel_Clear( PANEL_WORK* pp )
{
  int i;

//  panel_VisibleSet( pp, FALSE );

  if( pp->tcb_icon != NULL ){
    GFL_TCBL_Delete( pp->tcb_icon );
    pp->tcb_icon = NULL;
    pp->icon_no = 0;
  }
  for(i = 0;i < PANEL_FOAM_MAX;i++){
    GFL_BMP_Clear( pp->msgOam[i].bmp, 0 );
//	  BmpOam_ActorBmpTrans( pp->msgOam[i].oam );
    GFL_STR_ClearBuffer( pp->str[i] );
 
#if 0
    if( pp->tcb_print[i] != NULL ){
      #ifdef PM_DEBUG
      if( PRINTSYS_QUE_IsExistTarget( pp->que, pp->msgOam[i].bmp ) ){
        GF_ASSERT(0);
      }
      #endif
      GFL_TCBL_Delete( pp->tcb_print[i] );
      pp->tcb_print[i] = NULL;
    }
#endif
  }
  pp->data_idx = PANEL_DATA_BLANK;
  pp->n_line = 0;
  pp->tr_view = 0;
  pp->icon_no = 0;
  pp->type = 0;
}

/*
 *  @brief  パネル登録
 */
static void panel_Entry( PANEL_WORK* pp, u8 data_idx,u8 line )
{
  panel_Clear( pp );
  pp->data_idx = data_idx;
  pp->n_line = line;
}

/*
 *  @brief  次の描画ターゲットになるパネルを探す
 */
static PANEL_WORK* panel_GetNextTargetPanel( FMISSION_VIEW_PTR wk, SCROLL_DIR dir )
{
  int i, next_line;
  PANEL_WORK* pp;
  //まずはブランクパネルを探す
  for(i = 0;i < PANEL_MAX;i++){
    if( wk->panel[i].data_idx == PANEL_DATA_BLANK ){
      return &wk->panel[i];
    }
  }
  //見つからなければ、スクロール方向に応じて次のターゲットを決める
  if( dir == SCROLL_NONE ){
    GF_ASSERT(0); //スクロール無しの時にブランクが見つからなかったらおかしい
    return NULL;
  }
  if( dir == SCROLL_UP ){ //次は一番上を消して、一つ下を表示したい
    next_line = PANEL_VIEW_START;
  }else{  //次は一番下を消して、一つ上を表示したい
    next_line = PANEL_VIEW_END;
  }
  for(i = 0;i < PANEL_MAX;i++){
    if( wk->panel[i].n_line == next_line ){
      return &wk->panel[i];
    }
  }
  GF_ASSERT(0);
  return NULL;
}

/*
 *  @brief  パネル　Unionアクター更新
 */
static void panel_UnionObjUpdate( FMISSION_VIEW_PTR wk, PANEL_WORK* pp )
{
  static const int file[2] = {
    NARC_fldmmdl_mdlres_swhero_nsbtx,
    NARC_fldmmdl_mdlres_swheroine_nsbtx
  };

  if( !pp->mine_f ){  //ほかのプレイヤー
    u8 p_ofs;

    //キャラクタ転送
    GFL_CLGRP_CGR_Replace(  wk->objResUnion.res[ OBJ_RES_CGR ].tbl[ pp->id ],
                          wk->resCharUnion[ pp->tr_view ].p_char );
    //パレット転送
    p_ofs = 16*sc_wifi_unionobj_plt[ pp->tr_view ];
    PaletteWorkSet( wk->pfd, &wk->resPlttUnion.dat[p_ofs], FADE_SUB_OBJ, (ACT_PAL_UNION+pp->id)*16, 0x20 );
  }else{
    //自分
    CLWK_TransNSBTX_Ex( pp->cUnion, ARCID_MMDL_RES, file[pp->sex], 3,  //3パターン目が正面絵
      NSBTX_DEF_SX, NSBTX_DEF_SY, 0, CLSYS_DEFREND_SUB, wk->pfd, (ACT_PAL_UNION+pp->id), wk->tmpHeapID );
  }
}

/*
 *  @brief  パネル　タイプ取得
 */
static PANEL_TYPE panel_GetType( FMISSION_VIEW_PTR wk, FMISSION_LOG_ACTION action, u16 param )
{
  switch( action ){
  case FMISSION_LOG_ACT_FREEWORD:
    return PANEL_TYPE_MSG;
  case FMISSION_LOG_ACT_OPEN:
  case FMISSION_LOG_ACT_NEW_ENTRY:
    return PANEL_TYPE_ACT;
  }
  if( FEST_MISSION_IsScoreUp( wk->fmwk, action, param )){
    return PANEL_TYPE_ACT;
  }
  return PANEL_TYPE_SYS;
}

#if 0
/*
 *  @brief  パネル描画
 */
static void panel_Draw( FMISSION_VIEW_PTR wk, PANEL_WORK* pp, FMISSION_LOG* pl, int* task_ct )
{
  FMISSION_LOG_ACTION action = FMISSION_LOG_GetAction( pl );

  pp->sex = FMISSION_LOG_GetSex( pl );
  pp->mine_f = FMISSION_LOG_IsPlayerLog( pl );
  pp->type = panel_GetType( wk, action, FMISSION_LOG_GetActionParam( pl ) );
  pp->tr_view = FMISSION_LOG_GetTrView( pl );

  IWASAWA_Printf("PanelDraw %d\n", action);
  //プレイヤー名取得
  FMISSION_LOG_GetName( pl, pp->str[PANEL_FOAM_NAME] );
  //メッセージバッファ初期化
  FMISSION_LOG_ActionWordset( pl,  wk->wordset, wk->heap_sID);
  //NULLでないactionは1～なので、マイナス1している
  GFL_MSG_GetString( wk->mm_sys, msg_fmission_log_act01+(action-1), wk->str_tmp);
  WORDSET_ExpandStr( wk->wordset, pp->str[PANEL_FOAM_MSG], wk->str_tmp );

  //UnionOBJリソース更新
  panel_UnionObjUpdate( wk, pp );

  //パネルパターン変更
  BVCAct_AnmStart( pp->cPanel, ACTANM_PANEL_SYS+pp->type );

  {
    PRINTSYS_LSB col = FCOL_FNTOAM_W;

    if( pp->type == PANEL_TYPE_MSG ){
      col = FCOL_FNTOAM_B; 
    }
    printReq_FOamPrint( wk, pp, PANEL_FOAM_NAME, col, task_ct );
    printReq_FOamPrint( wk, pp, PANEL_FOAM_MSG, col, task_ct );
  }
}
#endif

/*
 *  @brief  パネル描画 BMPトランス処理
 */
static void panel_BmpTrans( FMISSION_VIEW_PTR wk, PANEL_WORK* pp )
{
  int i;

  //FontOam領域転
  for( i = 0;i < PANEL_FOAM_MAX; i++ ){
	  BmpOam_ActorBmpTrans( pp->msgOam[i].oam );
  }
  pp->trans_req_f = FALSE;
}

/*
 *  @brief  パネル描画エントリー
 */
static void panel_DrawStart( FMISSION_VIEW_PTR wk, PANEL_WORK* pp, FMISSION_LOG* pl )
{
  FMISSION_LOG_ACTION action = FMISSION_LOG_GetAction( pl );

  pp->sex = FMISSION_LOG_GetSex( pl );
  pp->mine_f = FMISSION_LOG_IsPlayerLog( pl );
  pp->act_prm = FMISSION_LOG_GetActionParam( pl );
  pp->type = panel_GetType( wk, action, pp->act_prm );
  pp->tr_view = FMISSION_LOG_GetTrView( pl );

  IWASAWA_Printf("PanelDrawEntry %d\n", action);
  //プレイヤー名取得
  FMISSION_LOG_GetName( pl, pp->str[PANEL_FOAM_NAME] );
  //スコア文字列生成
  WORDSET_RegisterNumber( wk->wordset, 0, FMISSION_LOG_GetScore( pl ), 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  GFL_MSG_GetString( wk->mm_sys, msg_sys_panel_score_num, wk->str_tmp);
  WORDSET_ExpandStr( wk->wordset, pp->str[PANEL_FOAM_SCORE], wk->str_tmp );
  
  //メッセージバッファ初期化
  FMISSION_LOG_ActionWordset( pl,  wk->wordset, wk->heap_sID);
  //NULLでないactionは1～なので、マイナス1している
  GFL_MSG_GetString( wk->mm_sys, msg_fmission_log_act01+(action-1), wk->str_tmp);
  WORDSET_ExpandStr( wk->wordset, pp->str[PANEL_FOAM_MSG], wk->str_tmp );

  {
    int i,ofs = 0;
    PRINTSYS_LSB col[2] = { FCOL_FNTOAM_W, FCOL_FNTOAM_RW };

    if( pp->type == PANEL_TYPE_MSG ){
      col[0] = FCOL_FNTOAM_B; 
      col[1] = FCOL_FNTOAM_RB; 
      ofs = 6;
    }
    if( wk->init_f ){
      for( i = 0;i < PANEL_FOAM_MAX; i++ ){
	      PRINTSYS_PrintColor( pp->msgOam[i].bmp, ofs*(i==PANEL_FOAM_MSG), 0, pp->str[i], wk->fontHandle, col[i==PANEL_FOAM_SCORE] );
      }
    }else{
      for( i = 0;i < PANEL_FOAM_MAX; i++ ){
        PRINTSYS_PrintQueColor( wk->printQue, pp->msgOam[i].bmp, ofs*(i==PANEL_FOAM_MSG), 0, pp->str[i], wk->fontHandle, col[i==PANEL_FOAM_SCORE] );
      } 
    }
  }
}
/*
 *  @brief  パネル描画終了アップデート処理
 */
static void panel_DrawFinish( FMISSION_VIEW_PTR wk, PANEL_WORK* pp )
{
  //UnionOBJリソース更新
  panel_UnionObjUpdate( wk, pp );

  //パネルパターン変更
  BVCAct_AnmStart( pp->cPanel, ACTANM_PANEL_SYS+pp->type );

  panel_VisibleSet( pp, TRUE );   //パネル描画開始
  
  pp->trans_req_f = TRUE; //転送リクエスト
}

/*
 *  @brief  パネル描画メイン
 *
 *  TRUEが返るまで呼び続ける
 */
static BOOL panel_DrawMain( FMISSION_VIEW_PTR wk, PANEL_WORK* pp )
{
  int i;

  //全FontOamの描画待ち
  for( i = 0;i < PANEL_FOAM_MAX; i++ ){
    if( PRINTSYS_QUE_IsExistTarget( wk->printQue, pp->msgOam[i].bmp ) ){
      return FALSE;
    }
  }
  panel_DrawFinish( wk, pp );
  return TRUE;
}

/*
 *  @brief  パネル一発描画
 */
static void panel_Draw( FMISSION_VIEW_PTR wk, PANEL_WORK* pp, FMISSION_LOG* pl )
{
  panel_DrawStart( wk, pp, pl );
  panel_DrawFinish( wk, pp );
  panel_BmpTrans( wk, pp );
}

/*
 *  @brief  パネル　位置セット
 */
static void  panel_SetPos( PANEL_WORK* pp, s16 x, s16 y )
{
  pp->px = x;
  pp->py = y;
  BVCAct_SetPosition( pp->cPanel, x, y );
  BVCAct_SetPosition( pp->cIcon, x+ACT_ICON_OX, y+ACT_ICON_OY );
  BVCAct_SetPosition( pp->cUnion, x+ACT_UNION_OX, y+ACT_UNION_OY );
  BmpOam_ActorSetPos( pp->msgOam[PANEL_FOAM_NAME].oam, x+ACT_NAME_OX, y+ACT_NAME_OY);
  BmpOam_ActorSetPos( pp->msgOam[PANEL_FOAM_MSG].oam, x+ACT_MSG_OX, y+ACT_MSG_OY);
  BmpOam_ActorSetPos( pp->msgOam[PANEL_FOAM_SCORE].oam, x+ACT_SCORE_OX, y+ACT_SCORE_OY);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  パネルアイコン表示 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_ICON_EFF{
  u8  wait;

  PANEL_WORK* pp;
  FMISSION_VIEW_PTR fvp;
}TASKWK_ICON_EFF;

static void tcb_IconEffect( GFL_TCBL *tcb , void* work);

/*
 *  @brief  パネルアイコン表示タスク登録
 */
static void eff_IconEffectAddCheck( FMISSION_VIEW_PTR wk, PANEL_WORK* pp )
{
  GFL_TCBL* tcb;
  TASKWK_ICON_EFF* twk;
  
  if( pp->icon_no == 0 ){
    panel_IconVisibleSet( pp, FALSE );
    return;
  }
  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_IconEffect, sizeof(TASKWK_ICON_EFF), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_ICON_EFF ));

  twk->fvp = wk;
  twk->pp = pp;
  twk->wait = ICON_POP_TIME;
  twk->pp->tcb_icon = tcb;
  
  BVCAct_AnmStart( pp->cIcon, ( pp->icon_no-1 )+ACTANM_ICON_HELLO );
  panel_IconVisibleSet( pp, TRUE );
}

static void tcb_IconEffect( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_ICON_EFF* twk = (TASKWK_ICON_EFF*)tcb_wk;

  if( twk->wait-- > 0 ){
    return;
  }
  twk->pp->tcb_icon = NULL;
  twk->pp->icon_no = 0;
  panel_IconVisibleSet( twk->pp, FALSE );
  GFL_TCBL_Delete(tcb);
}


/*
 *  @brief  リストアップダウンリクエスト
 */
static void list_UpDownReq( FMISSION_VIEW_PTR wk, SCROLL_DIR dir )
{
  u8 ofs,idx;
  
  BVCSound_PlaySE( FVIEW_SE_UPDOWN );

  if( dir == SCROLL_UP ){ //次は一つ下を表示したい
    ofs = wk->ctrl.view_top+wk->ctrl.view_max;
    FMISSION_LOG_TBL_SetViewTopIdx( wk->logView, ++wk->ctrl.view_top );
  }else{  //次は一つ上を表示したい
    ofs = wk->ctrl.view_top-1;
    FMISSION_LOG_TBL_SetViewTopIdx( wk->logView, --wk->ctrl.view_top );
  }
  FMISSION_LOG_ViewTbl_GetLog( wk->logView, wk->tmpLog, ofs );
  idx = FMISSION_LOG_ViewTbl_GetOfs2Idx( wk->logView, ofs );
  
  //スクロールリクエスト
  list_ScrollReq( wk, wk->tmpLog, idx, dir, FALSE );
}

/*
 *  @brief  リストアップダウン初回リクエスト
 */
static void list_UpDownReq1st( FMISSION_VIEW_PTR wk, SCROLL_DIR dir, int ret_seq )
{
  wk->scr_repeat_f = TRUE;
  wk->scr_repeat_end = FALSE;
  wk->scr_repeat_dir = dir;
  wk->scr_repeat_ret_seq = ret_seq;
  wk->scr_repeat_ct = 0;

  BVC_PrintQueLimmitUpSet( wk->printQue, TRUE );

  FMissionView_MenuBarViewSet( wk, MENU_ALL, MENU_ST_OFF );
  if( wk->scr_repeat_dir == SCROLL_UP ){
    BVCAct_AnmStart( wk->pAct[ACT_DOWN], ACTANM_DOWN_ANM );
  }else{
    BVCAct_AnmStart( wk->pAct[ACT_UP], ACTANM_UP_ANM );
  }

  list_UpDownReq( wk, dir );
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  リストスクロールリクエストタスク処理 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_LIST_SCROLL{
  u8  seq;
  u8  dir;
  PANEL_DRAW_TYPE draw_type;
  PANEL_WORK* pp;

  FMISSION_VIEW_PTR fvp;
  int child_task;
  int* task_ct;
}TASKWK_LIST_SCROLL;

static void tcb_ListScroll( GFL_TCBL *tcb , void* work);

/*
 *  @brief  ビュー スクロール処理リクエスト
 */
static void list_ScrollReq( FMISSION_VIEW_PTR wk, FMISSION_LOG* pl, u8 idx, SCROLL_DIR dir, BOOL new_f )
{
  PANEL_WORK* pp;
  GFL_TCBL* tcb;
  TASKWK_LIST_SCROLL* twk;

  //空きパネル検索
  pp = panel_GetNextTargetPanel( wk, dir );
  if( pp == NULL){
    GF_ASSERT(0);
    return; //万一のためのチェック(正しい挙動ならNULLはない)
  }
  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_ListScroll, sizeof(TASKWK_LIST_SCROLL), 0 );
  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_LIST_SCROLL ));

  twk->fvp = wk;
  twk->dir = dir;
  twk->pp = pp;
  twk->task_ct = &wk->eff_task_ct;

  wk->eff_task_ct++;

  panel_Entry( pp, idx, 0 );  //パネル新規登録(ラインは後で初期化)
  panel_DrawStart( wk, pp, pl );   //パネル描画開始リクエスト

  if( new_f && pp->type == PANEL_TYPE_MSG ){
    pp->icon_no = pp->act_prm;  //アイコンNo
  }else{
    pp->icon_no = 0;
  }
}

static void tcb_ListScroll( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_LIST_SCROLL* twk = (TASKWK_LIST_SCROLL*)tcb_wk;
  FMISSION_VIEW_PTR fvp = twk->fvp;

  switch( twk->seq ){
  case 0:
    //パネル描画待ち
    if( panel_DrawMain( fvp, twk->pp ) == FALSE ){
      return;
    }
    //パネルを移動
    {
      int i, line;

      for(i = 0;i < PANEL_MAX; i++ ){
        PANEL_WORK* pp = &fvp->panel[i];
        if( pp->data_idx == PANEL_DATA_BLANK ){
          continue;
        }
        //移動するラインを決定
        if( pp == twk->pp ){
          if( twk->dir == SCROLL_UP ){
            pp->n_line = PANEL_VIEW_END;
          }else{
            pp->n_line = PANEL_VIEW_START;
          } 
        }else{
          if( twk->dir == SCROLL_UP ){
            pp->n_line--;
          }else{
            pp->n_line++;
          } 
        } 
        panel_SetPos( pp, pp->n_line*ACT_PANEL_OX+ACT_PANEL_PX, pp->n_line*ACT_PANEL_OY+ACT_PANEL_PY );
        if( pp == twk->pp ){
          eff_IconEffectAddCheck( fvp, twk->pp );
        }
      }
    }
    twk->seq++;
    return;
  case 1:
    break;
  }
#if 0
  if( twk->child_task > 0 ){
    return;
  }
  
  switch( twk->seq ){
  case 0:
    panel_VisibleSet( twk->pp, TRUE );   //パネル描画
    effReq_PanelScroll( fvp, twk->dir, twk->pp, &twk->child_task );
    twk->seq++;
    return;
  default:
    break;
  }
#endif
  if( twk->task_ct != NULL ){
    (*twk->task_ct)--;
  }
  GFL_TCBL_Delete(tcb);
}

#if 0
/////////////////////////////////////////////////////////////////////
/*
 *  @brief  パネルスクロール
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_PANEL_SCROLL{
  int seq;
  FMISSION_VIEW_PTR fvp;
  PANEL_WORK* pp;
  u8  dir;
  u8  deray;
  u8  frame;
  u8  ct;
  s16 epx,epy;
  fx32  x,y;
  fx32  ax,ay;
  
  int* task_ct;
}TASKWK_PANEL_SCROLL;

static void taskAdd_PanelScroll( FMISSION_VIEW_PTR wk, PANEL_WORK* pp, u8 dir, int* task_ct );
static void tcb_PanelScroll( GFL_TCBL *tcb , void* work);

/*
 *  @brief  パネルスクロールリクエスト
 */
static void effReq_PanelScroll( FMISSION_VIEW_PTR wk, u8 dir, PANEL_WORK* new_pp, int* task_ct )
{
  int i;
  PANEL_WORK* pp;

  if( dir == SCROLL_UP ){
    new_pp->n_line = PANEL_LINE_END;
  }else{
    new_pp->n_line = 0;
  }
  panel_SetPos( new_pp, new_pp->n_line*ACT_PANEL_OX+ACT_PANEL_PX,
                        new_pp->n_line*ACT_PANEL_OY+ACT_PANEL_PY );
  panel_VisibleSet( new_pp, TRUE );
  
  for(i = 0;i < PANEL_MAX;i++){
    pp = &wk->panel[i];

    if( pp->data_idx == PANEL_DATA_BLANK ){
      continue;
    }
    taskAdd_PanelScroll( wk, pp, dir, task_ct );
  }
}

/*
 *  @brief  パネルスクロールタスク登録
 */
static void taskAdd_PanelScroll( FMISSION_VIEW_PTR wk, PANEL_WORK* pp, u8 dir, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_PANEL_SCROLL* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_PanelScroll, sizeof(TASKWK_PANEL_SCROLL), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_PANEL_SCROLL));

  twk->dir = dir;
  if( twk->dir == SCROLL_UP ){
    pp->n_line--;
  }else if( twk->dir == SCROLL_DOWN ){
    pp->n_line++;
  }

  twk->fvp = wk;
  twk->pp = pp;
  twk->epx = pp->n_line*ACT_PANEL_OX+ACT_PANEL_PX; 
  twk->epy = pp->n_line*ACT_PANEL_OY+ACT_PANEL_PY; 
  twk->x = FX32_CONST(pp->px);
  twk->y = FX32_CONST(pp->py);
  twk->frame = PANEL_SCROLL_FRAME;
  twk->ax = FX_Div( FX32_CONST( twk->epx - pp->px ), FX32_CONST(twk->frame));
  twk->ay = FX_Div( FX32_CONST( twk->epy - pp->py ), FX32_CONST(twk->frame));

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_PanelScroll( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_PANEL_SCROLL* twk = (TASKWK_PANEL_SCROLL*)tcb_wk;
  FMISSION_VIEW_PTR fvp = twk->fvp;
 
  switch( twk->seq ){
  case 0:
    if( ++twk->ct < twk->frame){
      twk->x += twk->ax;
      twk->y += twk->ay;
      panel_SetPos( twk->pp, FX_Whole(twk->x), FX_Whole(twk->y));
      return;
    }
    panel_SetPos( twk->pp, twk->epx, twk->epy);
  
    if( twk->pp->n_line == 0 || twk->pp->n_line == PANEL_LINE_END){
      twk->seq++;
      return;
    }
    break; 
  case 1:
  default:
    {
      int i;

      for(i = 0;i < PANEL_FOAM_MAX;i++){
        if( twk->pp->tcb_print[i] != NULL ){
          return;
        }
      }
      panel_Clear( twk->pp );  //パネル破棄
    }
    break;
  }
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

#endif
/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ ポップアップ 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_WIN_POPUP{
  u8  seq;
  u8  wait;
  int child_task;
 
  FMISSION_VIEW_PTR fvp;
  int* task_ct;
}TASKWK_WIN_POPUP;

static void taskAdd_WinPopup( FMISSION_VIEW_PTR wk, STRBUF* str, int* task_ct );
static void tcb_WinPopup( GFL_TCBL *tcb , void* work);

static void effReq_PopupMsgSys( FMISSION_VIEW_PTR wk, u8 msg_id )
{
  print_GetMsgToBuf( wk, msg_id );
  taskAdd_WinPopup( wk, wk->str_expand, &wk->eff_task_ct);
}

/*
 *  @brief  メッセージウィンドウ ポップアップタスク登録
 */
static void taskAdd_WinPopup( FMISSION_VIEW_PTR wk, STRBUF* str, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_WIN_POPUP* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_WinPopup, sizeof(TASKWK_WIN_POPUP), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_WIN_POPUP ));

  GFL_STR_CopyBuffer( wk->str_popup, str );
  
  twk->fvp = wk;
  twk->wait = POPUP_WAIT;
  
//  IWASAWA_Printf("WinPopup Req\n");
  FMissionView_MenuBarViewSet( wk, MENU_ALL, MENU_ST_OFF );

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_WinPopup( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_WIN_POPUP* twk = (TASKWK_WIN_POPUP*)tcb_wk;
  FMISSION_VIEW_PTR fvp = twk->fvp;
  
  if( twk->child_task ){
    return;
  }

  switch( twk->seq ){
  case 0:
    print_PopupWindow( twk->fvp, twk->fvp->str_popup, &twk->child_task );
    twk->seq++;
    return;
  case 1:
    BVCTaskAdd_MsgUpdown( twk->fvp->pTcbSys, FRM_POPUP, SCROLL_UP, &twk->child_task );
    twk->seq++;
    return;
  case 2:
    if( BVCPrint_PopupWindowTimeWaitCheck( &twk->wait, FRM_POPUP ) == FALSE ){
      return;
    }
    BVCTaskAdd_MsgUpdown( twk->fvp->pTcbSys, FRM_POPUP, SCROLL_DOWN, &twk->child_task );
    twk->seq++;
    return;
  default:
    break;
  }
//  IWASAWA_Printf("WinPopup End\n" );
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ Gパワー使用確認 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_WIN_GPOWER{
  u8  seq;
  u8  seq_next;
  u8  wait;
  u8  type;
  u8  cancel_f;
  u8  ret;
  int child_task;

  GPOWER_ID  g_power;
  u16 item_num;
  u16 item_use;
  u16 mypower_use;
  u16 mypower_min;
  u16 mypower_sec;
  FMISSION_VIEW_PTR fvp;
  int* task_ct;
}TASKWK_WIN_GPOWER;

static const u8 DATA_GPowerUseMsg[GPOWER_USE_MAX] = {
 msg_sys_gpower_use00,
 msg_sys_gpower_use01,
 msg_sys_gpower_use02,
};

static BOOL taskAdd_WinGPower( FMISSION_VIEW_PTR wk, GPOWER_ID gpower_id, BOOL spot_f, int* task_ct );
static void tcb_WinGPowerYesNo( GFL_TCBL *tcb , void* work);
static void tcb_WinGPowerCheck( GFL_TCBL *tcb , void* work);

static BOOL effReq_PopupMsgGPower( FMISSION_VIEW_PTR wk,  FMISSION_LOG* pl )
{
  GPOWER_ID gpower_id;
  
  if( FMISSION_LOG_IsPlayerLog( pl ) ||
      FMISSION_LOG_GetAction( pl ) != FMISSION_LOG_ACT_GPOWER ){
    return FALSE;
  }
  gpower_id = FMISSION_LOG_GetActionParam( pl );
  
  FMISSION_LOG_ActionWordset( pl,  wk->wordset, wk->heap_sID );
  return taskAdd_WinGPower( wk, gpower_id, FALSE, &wk->eff_task_ct);
}
static void effReq_PopupMsgGPowerMine( FMISSION_VIEW_PTR wk )
{
  taskAdd_WinGPower( wk,
      wk->my_data.power, GPOWER_USE_MINE, &wk->eff_task_ct );
}
static BOOL effReq_PopupMsgGPowerSpecial( FMISSION_VIEW_PTR wk, GPOWER_ID gpower_id, STRBUF* name_buf )
{
  WORDSET_RegisterWord( wk->wordset, 0, name_buf, PM_NEUTRAL, TRUE, PM_LANG);

  return taskAdd_WinGPower( wk, gpower_id, TRUE, &wk->eff_task_ct );
}

/*
 *  @brief  メッセージウィンドウ Gパワー使用確認タスク登録
 */
static BOOL taskAdd_WinGPower( FMISSION_VIEW_PTR wk, GPOWER_ID gpower_id, BOOL spot_f, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_WIN_GPOWER* twk;
  
  //セキュリティ
  if( GPOWER_IsEnableID( gpower_id ) == FALSE ){
    GF_ASSERT(0);
    return FALSE;
  }

  //まったく同じパワーが発動中ならスルーする
  if( GPOWER_Check_OccurID( gpower_id, wk->gpower_data ) == gpower_id ){
    return FALSE;
  }

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_WinGPowerYesNo, sizeof(TASKWK_WIN_GPOWER), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_WIN_GPOWER ));

  wk->ctrl.g_power = GPOWER_ID_NULL;
  wk->ctrl.mine_power_f = FALSE;

  twk->fvp = wk;
  twk->wait = POPUP_WAIT;
  twk->g_power = gpower_id;

  WORDSET_RegisterGPowerName( wk->wordset, 1, twk->g_power );

  FMissionView_MenuBarViewSet( wk, MENU_POWER, MENU_ST_OFF );
  FMissionView_MenuBarViewSet( wk, MENU_HELLO, MENU_ST_OFF );
  FMissionView_MenuBarViewSet( wk, MENU_RETIRE, MENU_ST_OFF );
  FMissionView_MenuBarViewSet( wk, MENU_UPDOWN, MENU_ST_OFF );

  twk->task_ct = task_ct;
  (*task_ct)++;

  return TRUE;
}

static void tcb_WinGPowerYesNo( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_WIN_GPOWER* twk = (TASKWK_WIN_GPOWER*)tcb_wk;
  FMISSION_VIEW_PTR fvp = twk->fvp;

  if( twk->child_task > 0 ){
    return;
  }

  switch( twk->seq ){
  case 0:
    print_GetMsgToBuf( fvp, msg_sys_gpower_use01 );
    print_PopupWindow( fvp, fvp->str_expand, &twk->child_task );
    twk->seq++;
    return;
  case 1:
    BVCTaskAdd_MsgUpdown( fvp->pTcbSys, FRM_POPUP, SCROLL_UP, &twk->child_task );
    twk->seq++;
    return;
  case 2:
    BVC_PrintQueLimmitUpSet( fvp->printQue, TRUE );
    BVCTMenu_Create( fvp->tmenu, TMENU_ID_CYN );
    draw_MenuIconVisibleSet( fvp, FALSE );
    twk->seq++;
    return;
  case 3: //選択待ち
    {
      int ret = BVCTMenu_Update( fvp->tmenu );
      if( ret < 0 ){
        return;
      }
      twk->ret = ret;

      switch( twk->ret){
      case TMENU_CYN_CHECK:
        FVSEvent_Reserve( fvp, EV_GPOWER_CHECK_TMENU_YN );
        twk->seq = 3;
        break;
      case TMENU_CYN_YES:
        //使うGパワーを覚えておく
        fvp->ctrl.g_power = twk->g_power;
        FVSEvent_Reserve( fvp, EV_GPOWER_USE );
        twk->seq = 4;
        break;
      case TMENU_CYN_NO:
        twk->seq = 5;
        break;
      }
    }
    return;
  case 4: //使うイベント待ち
    if( fvp->ctrl.g_power != GPOWER_ID_NULL ) {
      //途中キャンセルされていたら、選択しなおし
      fvp->ctrl.g_power = GPOWER_ID_NULL;
      twk->seq = 3;
    }else{
      //正常に使えていれば、メニューを終了させる
      BVCTMenu_End( fvp->tmenu );
      twk->seq = 6;
    }
    return;
  case 5: //キャンセル決定アニメ待ち
    if( BVCTMenu_EndWait( fvp->tmenu, twk->ret )){
      twk->seq++;
    }
    return;
  case 6:
    draw_MenuIconVisibleSet( fvp, TRUE );
    BVCTaskAdd_MsgUpdown( fvp->pTcbSys, FRM_POPUP, SCROLL_DOWN, &twk->child_task );
    twk->seq++;
    return;
  default:
    break;
  }
  FMissionView_MenuBarViewSet( fvp, MENU_ALL, MENU_ST_ON );
  BVC_PrintQueLimmitUpSet( fvp->printQue, FALSE );
  
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}


/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ リタイア確認 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_WIN_RETIRE{
  u8  seq;
  u8  seq_next;
  u8  wait;
  u8  type;
  u8  cancel_f;
  u8  ret;
  int child_task;

  GPOWER_ID  g_power;
  u16 item_num;
  u16 item_use;
  u16 mypower_use;
  u16 mypower_min;
  u16 mypower_sec;
  FMISSION_VIEW_PTR fvp;
  int* task_ct;
}TASKWK_WIN_RETIRE;

static void taskAdd_WinRetire( FMISSION_VIEW_PTR wk, int* task_ct );
static void tcb_WinRetireYesNo( GFL_TCBL *tcb , void* work);

static void effReq_PopupMsgRetire( FMISSION_VIEW_PTR wk )
{
  taskAdd_WinRetire( wk, &wk->eff_task_ct );
}

/*
 *  @brief  メッセージウィンドウ リタイア確認タスク登録
 */
static void taskAdd_WinRetire( FMISSION_VIEW_PTR wk, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_WIN_RETIRE* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_WinRetireYesNo, sizeof(TASKWK_WIN_RETIRE), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_WIN_RETIRE ));

  twk->fvp = wk;
  twk->wait = POPUP_WAIT;

  FMissionView_MenuBarViewSet( wk, MENU_RETIRE, MENU_ST_ANM );
  FMissionView_MenuBarViewSet( wk, MENU_POWER, MENU_ST_OFF );
  FMissionView_MenuBarViewSet( wk, MENU_HELLO, MENU_ST_OFF );
  FMissionView_MenuBarViewSet( wk, MENU_INFO, MENU_ST_OFF );
  FMissionView_MenuBarViewSet( wk, MENU_UPDOWN, MENU_ST_OFF );

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_WinRetireYesNo( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_WIN_RETIRE* twk = (TASKWK_WIN_RETIRE*)tcb_wk;
  FMISSION_VIEW_PTR fvp = twk->fvp;

  if( twk->child_task > 0 ){
    return;
  }

  switch( twk->seq ){
  case 0:
    print_GetMsgToBuf( fvp, msg_sys_retire );
    print_PopupWindow( fvp, fvp->str_expand, &twk->child_task );
    twk->seq++;
    return;
  case 1:
    BVCTaskAdd_MsgUpdown( fvp->pTcbSys, FRM_POPUP, SCROLL_UP, &twk->child_task );
    twk->seq++;
    return;
  case 2:
    BVC_PrintQueLimmitUpSet( fvp->printQue, TRUE );
    BVCTMenu_Create( fvp->tmenu, TMENU_ID_YESNO );
    draw_MenuIconVisibleSet( fvp, FALSE );
    twk->seq++;
    return;
  case 3: //選択待ち
    {
      int ret = BVCTMenu_Update( fvp->tmenu );
      if( ret < 0 ){
        return;
      }
      twk->ret = ret;

      if( twk->ret == TMENU_YESNO_YES ){
        FVSEvent_Reserve( fvp, EV_MISSION_RETIRE );
        twk->seq = 4;
      }else{
        twk->seq = 5;
      }
    }
    return;
  case 4: //使うイベント待ち
    if( fvp->event_reserve_exec == FALSE ) {
      //途中キャンセルされていたら、選択しなおし
      twk->seq = 3;
    }else{
      //正常に使えていれば、メニューを終了させる
      BVCTMenu_End( fvp->tmenu );
      twk->seq = 6;
    }
    return;
  case 5: //キャンセル決定アニメ待ち
    if( BVCTMenu_EndWait( fvp->tmenu, twk->ret )){
      twk->seq++;
    }
    return;
  case 6:
    draw_MenuIconVisibleSet( fvp, TRUE );
    BVCTaskAdd_MsgUpdown( fvp->pTcbSys, FRM_POPUP, SCROLL_DOWN, &twk->child_task );
    twk->seq++;
    return;
  default:
    break;
  }
  FMissionView_MenuBarViewSet( fvp, MENU_ALL, MENU_ST_ON );
  BVC_PrintQueLimmitUpSet( fvp->printQue, FALSE );
  
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

#if 0
/////////////////////////////////////////////////////////////////////
/*
 *  @brief  FontOamメッセージ表示キュー処理 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_FOAM_PRINT{
  PANEL_WORK* pp;
  u8        id;
  FONT_OAM* foam;
   
  FMISSION_VIEW_PTR fvp;
  int* task_ct;
}TASKWK_FOAM_PRINT;

static void taskAdd_FOamPrint( FMISSION_VIEW_PTR wk, PANEL_WORK* pp, PANEL_FOAM_ID id, int* task_ct );
static void tcb_FOamPrint( GFL_TCBL *tcb , void* work);

static void printReq_FOamPrint( FMISSION_VIEW_PTR wk, PANEL_WORK* pp, PANEL_FOAM_ID id, PRINTSYS_LSB color,int* task_ct )
{
  FONT_OAM* foam = &pp->msgOam[id];

  GFL_BMP_Clear( foam->bmp, 0 );
  if( wk->init_f ){ //初期化時はダイレクトプリント
	  PRINTSYS_PrintColor( foam->bmp, 0, 0, pp->str[id], wk->fontHandle, color );
	  BmpOam_ActorBmpTrans( foam->oam );
  }else{
    PRINTSYS_PrintQueColor( wk->printQue, foam->bmp, 0, 0, pp->str[id], wk->fontHandle, color );
    taskAdd_FOamPrint( wk, pp, id, task_ct );
  }
}

/*
 *  @brief  パネルメッセージ描画タスク登録
 */
static void taskAdd_FOamPrint( FMISSION_VIEW_PTR wk, PANEL_WORK* pp, PANEL_FOAM_ID id, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_FOAM_PRINT* twk;

#ifdef PM_DEBUG
  GF_ASSERT( pp->tcb_print[id] == NULL );
#endif

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_FOamPrint, sizeof(TASKWK_FOAM_PRINT), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_FOAM_PRINT ));

  twk->fvp = wk;
  twk->pp = pp;
  twk->id = id;
  twk->foam = &pp->msgOam[id];
  twk->pp->tcb_print[id] = tcb;

  if( task_ct != NULL ){
    twk->task_ct = task_ct;
    (*twk->task_ct)++;
  }
}

static void tcb_FOamPrint( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_FOAM_PRINT* twk = (TASKWK_FOAM_PRINT*)tcb_wk;
  FMISSION_VIEW_PTR fvp = twk->fvp;

  if( PRINTSYS_QUE_IsExistTarget( fvp->printQue, twk->foam->bmp ) ){
    return;
  }
	BmpOam_ActorBmpTrans( twk->foam->oam );

  if( twk->task_ct != NULL ){
    (*twk->task_ct)--;
  }

  twk->pp->tcb_print[twk->id] = NULL;
  GFL_TCBL_Delete(tcb);
}

#endif
