/*
 *  @file   beacon_view_common.c
 *  @brief  すれ違いログ＆フェスミッションログ画面　共有サブルーチン群
 *  @author Miyuki Iwasawa
 *  @date   10.10.20
 */

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
//#include "beacon_view_sub.h"

#include "arc/wifi_unionobj_plt.cdat"


//=====================================================================
//三角形の当たり判定
//=====================================================================
/*
 *  @brief  外積計算
 */
static int calc_Cross( POINT* a, POINT* b ) 
{
  return (a->x*b->y - a->y*b->x);
}

/*
 *  @brief  POINTの減算
 */
static void calc_PointMinus( POINT* a, POINT* b, POINT* out )
{
  out->x = a->x-b->x;
  out->y = a->y-b->y;
}

/*
 *  @brief  三角形あたり判定
 *
 *  三角形は反時計回り
 * 1-0 p-a, b-a
 * 2-1 p-b, c-b
 * 0-2 p-c, a-c
 */
BOOL BVCCalc_PointInTriangle( POINT* p, TRIANGLE* tri )
{
  int i;
  POINT d1,d2;

  for( i = 0;i < 3;i++ ){
    calc_PointMinus( p, &tri->p[i], &d1 );
    calc_PointMinus( &tri->p[(1+i)%3], &tri->p[i], &d2 );
    if( calc_Cross( &d1, &d2 ) < 0 ){
      return FALSE;
    }
  }
  return TRUE;
}

/*
 *  @brief  SE再生
 */
void BVCSound_PlaySE( u16 se_no )
{
  PMSND_PlaySE( se_no );
}

/*
 *  @brief  アクター　座標セット
 */
void BVCAct_SetPosition( GFL_CLWK* act, s16 x, s16 y )
{
  GFL_CLACTPOS pos;
  pos.x = x;
  pos.y = y;
  
  GFL_CLACT_WK_SetPos( act, &pos, ACT_RENDER_ID);
}

/*
 *  @brief  アクター アニメスタート
 */
void BVCAct_AnmStart( GFL_CLWK* act, u8 anm_no )
{
  GFL_CLACT_WK_SetAnmSeq( act, anm_no );
  GFL_CLACT_WK_ResetAnm( act );
}

/*
 *  @brief  特殊ポップアップ起動チェック 
 */
BOOL BVC_CheckSpecialPopup( BEACON_VIEW_PTR wk )
{
  u16* ret_fword_input =  BEACON_STATUS_GetFreeWordInputResultPointer( wk->b_status );

  //一言メッセージ入力があったかチェック
  if( (*ret_fword_input) == FALSE ){
    return FALSE;
  }
  *ret_fword_input = FALSE; //フラグリセット

  WORDSET_RegisterWord( wk->wordset, 0,
    BEACON_STATUS_GetFreeWordBuffer( wk->b_status ), wk->my_data.sex, TRUE, PM_LANG );
  BVCEffReq_PopupMsgSys( wk, msg_sys_free_word_send );
  return TRUE;
}

/*
 *  @brief  アクティブ・パッシブ切替
 */
void BVC_SetViewPassive( BEACON_VIEW_PTR wk, BOOL passive_f )
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

/*
 *  @brief  プリントキュー　処理スピードアップリクエスト
 *
 *  処理分割数を減らし、処理負荷を上げる代わりに、
 *  プリント処理終了までのスピードをアップする
 */
void BVC_PrintQueLimmitUpSet( BEACON_VIEW_PTR wk, BOOL flag )
{
  if( flag ){ //スピードアップ
    PRINTSYS_QUE_SetLimitTick( wk->printQue, PRINT_QUE_LIMMIT_UP );
  }else{  //デフォルトに戻す
    PRINTSYS_QUE_SetLimitTick( wk->printQue, PRINT_QUE_LIMMIT_DEFAULT );
  }
}

/*
 *  @brief  メニューバーアニメセット
 */
void BVC_MenuBarViewSet( BEACON_VIEW_PTR wk, MENU_ID id, MENU_STATE state )
{
  u8 flag;
  
  if( id == MENU_ALL ){
    if( wk->mode == VIEW_MODE_NORMAL ){
      flag = MENU_F_ALL;
    }else{
      flag = MENU_F_POWER|MENU_F_HELLO|MENU_F_RETURN;
    }
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
  if( flag & MENU_F_THANKS ){
    if( wk->ctrl.max > 0 ){
      BVCAct_AnmStart( wk->pAct[ACT_THANKS], state+ACTANM_THANKS_ON );
    }
  }
  if( flag & MENU_F_RETURN ){
    BVCAct_AnmStart( wk->pAct[ACT_RETURN], state+ACTANM_RETURN_ON );
  }
}

/*
 *  @brief  メニューバーアニメウェイト
 */
BOOL BVC_MenuBarCheckAnm( BEACON_VIEW_PTR wk, MENU_ID id )
{
  return GFL_CLACT_WK_CheckAnmActive( wk->pAct[ACT_POWER+id] );
}

/*
 *  @brief  メニューバーあたり判定
 */
int BVCTouchIn_CheckMenu( BEACON_VIEW_PTR wk, POINT* tp )
{
  int i, ret;
  GFL_UI_TP_HITTBL tbl[2];
  static const u16 se_tbl[] = { BVIEW_SE_MENU, BVIEW_SE_MENU, BVIEW_SE_MENU, BVIEW_SE_CANCEL };

  tbl[1].rect.top = GFL_UI_TP_HIT_END;

  for(i = 0;i < ACT_MENU_NUM;i++){
    u8 seq = GFL_CLACT_WK_GetAnmSeq( wk->pAct[ACT_POWER+i] )-(ACTANM_POWER_ON+i*ACT_ANM_SET);
    if( (seq % ACT_ANM_SET) >= 2){
      continue;
    }
    tbl[0].rect.top = ACT_MENU_PY;
    tbl[0].rect.bottom = tbl[0].rect.top+ACT_MENU_SY;
    tbl[0].rect.left = ACT_MENU_PX+ACT_MENU_OX*i;
    tbl[0].rect.right = tbl[0].rect.left+ACT_MENU_SX;

    ret = GFL_UI_TP_HitSelf( tbl, tp->x, tp->y );
    if(ret != GFL_UI_TP_HIT_NONE) {
      BVCSound_PlaySE( se_tbl[i] );
      return i;
    }
  }
  return GFL_UI_TP_HIT_NONE;
}

///////////////////////////////////////////////////////////////////
/*
 *  @brief  サブシーケンス　GPower
 */
///////////////////////////////////////////////////////////////////

/*
 *  @brief  サブシーケンス　GPower使用メイン
 */
BOOL BVCSubSeq_GPower( BEACON_VIEW_PTR wk )
{
  switch( wk->sub_seq ){
  case 0:
    BVC_MenuBarViewSet( wk, MENU_POWER, MENU_ST_ANM );
    BVC_PrintQueLimmitUpSet( wk, TRUE );
    BVCEffReq_PopupMsgGPowerMine( wk );
    wk->sub_seq++;
    break;
  case 1:
    if( wk->eff_task_ct ){
      break;
    }
    BVC_PrintQueLimmitUpSet( wk, FALSE );
    wk->sub_seq = 0;
    return TRUE;
  }
  return FALSE;
}




//========================================================================
//選択メニュー処理系
//========================================================================

/*
 *  @brief  TaskMenu YesNo生成
 */
void BVCTMenu_Create( BEACON_VIEW_PTR wk, u8 menu_idx )
{
  int i,max,px;
  TMENU_ITEM* tmenu;

  if( menu_idx == TMENU_ID_YESNO ){
    tmenu = wk->tmenu;
    max = TMENU_YN_MAX;
    px = TMENU_YN_PX;
  }else{
    tmenu = wk->tmenu_check;
    max = TMENU_CHECK_MAX;
    px = TMENU_CHECK_PX;
  }
  for( i = 0;i < max;i++){
    tmenu[i].work = APP_TASKMENU_WIN_CreateEx( wk->menuRes, &tmenu[i].item, 
                        px+TMENU_W*i, TMENU_PY, TMENU_W, TMENU_H, TRUE, FALSE ,wk->heap_sID );
  }
}

/*
 *  @brief  TaskMenu アップデート
 */
int BVCTMenu_Update( BEACON_VIEW_PTR wk,u8 menu_idx )
{
  int i,max,ret = -1;
  TMENU_ITEM* tmenu;

  if( menu_idx == TMENU_ID_YESNO ){
    tmenu = wk->tmenu;
    max = TMENU_YN_MAX;
  }else{
    tmenu = wk->tmenu_check;
    max = TMENU_CHECK_MAX;
  }

  for( i = 0;i < max;i++){
    //まだ書き込み中
    if( APP_TASKMENU_WIN_IsUpdateMsg( tmenu[i].work ) == TRUE){
      APP_TASKMENU_WIN_Update( tmenu[i].work );
      return ret;
    }
  }
  for( i = 0;i < max;i++){
    if( APP_TASKMENU_WIN_IsTrg( tmenu[i].work )){
      if(i < (max-1)){
        BVCSound_PlaySE( BVIEW_SE_DECIDE );
      }else{
        APP_TASKMENU_WIN_SetDecide( tmenu[i].work, TRUE );
        BVCSound_PlaySE( BVIEW_SE_DECIDE );
      }
      ret = i;
    }
    APP_TASKMENU_WIN_Update( tmenu[i].work );
  }
  return ret;
}

/*
 *  @brief  TaskMenu 終了アニメ待ち
 */
BOOL BVCTMenu_YnEndWait( BEACON_VIEW_PTR wk, u8 select )
{
  int i;

  APP_TASKMENU_WIN_Update( wk->tmenu[select].work );

  if( APP_TASKMENU_WIN_IsFinish( wk->tmenu[select].work )){
    for( i = 0;i < TMENU_YN_MAX;i++){
      APP_TASKMENU_WIN_Delete( wk->tmenu[i].work );
      wk->tmenu[i].work = NULL;
    }
    return TRUE;
  }
  return FALSE;
}

/*
 *  @brief  TaskMenu 終了
 */
void BVCTMenu_YnEnd( BEACON_VIEW_PTR wk )
{
  int i;

  for( i = 0;i < TMENU_YN_MAX;i++){
    APP_TASKMENU_WIN_Delete( wk->tmenu[i].work );
    wk->tmenu[i].work = NULL;
  }
}

/*
 *  @brief  TaskMenu Gパワー確認ダイアログ終了アニメ待ち
 */
BOOL BVCTMenu_CheckEndWait( BEACON_VIEW_PTR wk, u8 select )
{
  int i;

  APP_TASKMENU_WIN_Update( wk->tmenu_check[select].work );

  if( APP_TASKMENU_WIN_IsFinish( wk->tmenu_check[select].work )){
    for( i = 0;i < TMENU_CHECK_MAX;i++){
      APP_TASKMENU_WIN_Delete( wk->tmenu_check[i].work );
      wk->tmenu_check[i].work = NULL;
    }
    return TRUE;
  }
  return FALSE;
}

/*
 *  @brief  TaskMenu 終了
 */
void BVCTMenu_CheckEnd( BEACON_VIEW_PTR wk )
{
  int i;

  for( i = 0;i < TMENU_CHECK_MAX;i++){
    APP_TASKMENU_WIN_Delete( wk->tmenu_check[i].work );
    wk->tmenu_check[i].work = NULL;
  }
}

//========================================================================
//文字列処理系
//========================================================================
/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
void BVCPrint_GetMsgToBufEx( BEACON_VIEW_PTR wk, u8 msg_id, const GFL_MSGDATA* handle )
{
  GFL_MSG_GetString( handle, msg_id, wk->str_tmp);
  WORDSET_ExpandStr( wk->wordset, wk->str_expand, wk->str_tmp );
}

/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
void BVCPrint_GetMsgToBuf( BEACON_VIEW_PTR wk, u8 msg_id )
{
  GFL_MSG_GetString( wk->mm_status, msg_id, wk->str_tmp);
  WORDSET_ExpandStr( wk->wordset, wk->str_expand, wk->str_tmp );
}

/*
 *  @brief  ポップアップウィンドウ文字列描画
 */
void BVCPrint_PopupWindow( BEACON_VIEW_PTR wk, STRBUF* str, int* task_ct )
{
  BVCPrintReq_BmpwinPrint( wk, &wk->win[WIN_POPUP], str, 0, 0, FCOL_POPUP_BASE, FCOL_POPUP, task_ct );
}

//------------------------------------------------------------
/*
 *  @brief  ポップアップ　タイムウェイトチェック
 */
//------------------------------------------------------------
BOOL BVCPrint_PopupWindowTimeWaitCheck( u8* p_wait )
{
  u32 x,y;
  u16 pat = 0xFFFE; 

  if( (*p_wait)-- == 0 ){ //タイムアウト
    *p_wait = 0;
    return TRUE; 
  }
 
  //タッチチェック
  if( GFL_UI_TP_GetPointTrg( &x, &y )){
    //スクロールのため、スクリーンが実座標+64dotの位置にあるので、ちょっと位置補正してチェック
    if( GFL_BG_CheckDot( FRM_POPUP, x, y+64, &pat )){
      *p_wait = 0;
      return TRUE; 
    }
  }
  return FALSE;
}

//------------------------------------------------------------
/**
 * ビーコン情報から表示するメッセージIDを取得する
 *
 * @param   info		対象のビーコン情報へのポインタ
 */
//------------------------------------------------------------
u32 BVCPrint_GetBeaconMsgID(const GAMEBEACON_INFO *info)
{
  GAMEBEACON_ACTION action = GAMEBEACON_Get_Action_ActionNo(info);

  if(action == GAMEBEACON_ACTION_NULL || action >= GAMEBEACON_ACTION_MAX){
    GF_ASSERT_MSG(0, "action_no = %d\n", action);
    return msg_beacon_001;
  }
  return msg_beacon_001 + action - GAMEBEACON_ACTION_SEARCH;
}

/*
 *  @brief  メニューウィンドウ表示
 */
void BVCDraw_MenuWindow( BEACON_VIEW_PTR wk, u8 msg_id, int* task_ct )
{
  if( wk->mode == VIEW_MODE_FMISSION ){
	  GFL_BG_LoadScreenV_Req( FRM_MENUMSG );
    return;
  }
  WORDSET_RegisterNumber( wk->wordset, 0, wk->log_count, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  BVCPrint_GetMsgToBuf( wk, msg_id );
  BVCPrintReq_BmpwinPrint( wk, &wk->win[WIN_MENU], wk->str_expand, 0, 4, 0, FCOL_WHITE_N, task_ct );
}

/*
 *  @brief  メニューウィンドウ表示クリア
 */
void BVCDraw_ClearMenuWindow( BEACON_VIEW_PTR wk )
{
  if( wk->mode == VIEW_MODE_FMISSION ){
	  GFL_BG_LoadScreenV_Req( FRM_MENUMSG );
    return;
  }
  GFL_BMP_Clear( wk->win[WIN_MENU].bmp, FCOL_WHITE_N );
  GFL_BMPWIN_TransVramCharacter( wk->win[WIN_MENU].win );
}

/*
 *  @brief  メニューアクター Viewセット
 */
void BVCDraw_MenuIconVisibleSet( BEACON_VIEW_PTR wk, BOOL visible_f )
{
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_POWER], visible_f );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_HELLO], visible_f );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_RETURN], visible_f );
  if( wk->mode == VIEW_MODE_NORMAL ){
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_THANKS], visible_f );
  }
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  BMPウィンドウメッセージ表示キュー処理 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_BMPWIN_PRINT{
  u8  seq;
  BMP_WIN* win;
  
  BEACON_VIEW_PTR bvp;
  int* task_ct;
}TASKWK_BMPWIN_PRINT;

static void taskAdd_BmpwinPrint( BEACON_VIEW_PTR wk, BMP_WIN* win, int* task_ct );
static void tcb_BmpwinPrint( GFL_TCBL *tcb , void* work);

void BVCPrintReq_BmpwinPrintEx( BEACON_VIEW_PTR wk, BMP_WIN* win, STRBUF* str, u8 x, u8 y, u8 clear, PRINTSYS_LSB color, int* task_ct, BOOL clear_f )
{
  if( clear_f ){
    GFL_BMP_Clear( win->bmp, clear );
  }
  if( wk->init_f ){ //初期化時はダイレクトプリント
	  PRINTSYS_PrintColor( win->bmp, x, y, str, wk->fontHandle, color );
    GFL_BMPWIN_MakeTransWindow( win->win );
  }else{
    PRINT_UTIL_PrintColor( &win->putil, wk->printQue, x, y, str, wk->fontHandle, color );
    taskAdd_BmpwinPrint( wk, win, task_ct );
  }
}

void BVCPrintReq_BmpwinPrint( BEACON_VIEW_PTR wk, BMP_WIN* win, STRBUF* str, u8 x, u8 y, u8 clear, PRINTSYS_LSB color, int* task_ct )
{
  BVCPrintReq_BmpwinPrintEx( wk, win, str, x, y, clear, color, task_ct, TRUE);
}

/*
 *  @brief  メッセージウィンドウ パネルフラッシュタスク登録
 */
static void taskAdd_BmpwinPrint( BEACON_VIEW_PTR wk, BMP_WIN* win, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_BMPWIN_PRINT* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_BmpwinPrint, sizeof(TASKWK_BMPWIN_PRINT), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_BMPWIN_PRINT ));

  twk->bvp = wk;
  twk->win = win;

  if( task_ct != NULL ){
    twk->task_ct = task_ct;
    (*twk->task_ct)++;
  }
}

static void tcb_BmpwinPrint( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_BMPWIN_PRINT* twk = (TASKWK_BMPWIN_PRINT*)tcb_wk;
  BEACON_VIEW_PTR bvp = twk->bvp;

  if( !PRINT_UTIL_Trans( &twk->win->putil, bvp->printQue )){
    return;
  }
  GFL_BMPWIN_MakeScreen( twk->win->win );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( twk->win->win ) );

  if( twk->task_ct != NULL ){
    (*twk->task_ct)--;
  }
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ アップダウン 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_MSG_UPDOWN{
  u8  dir;
  u8  frame;
  u8  ct;
  s8  y;
  s8  diff;

  int* task_ct;
}TASKWK_MSG_UPDOWN;

static void tcb_MsgUpdown( GFL_TCBL *tcb , void* work);

/*
 *  @brief  メッセージウィンドウ アップダウンタスク登録
 */
void BVCTaskAdd_MsgUpdown( BEACON_VIEW_PTR wk, u8 dir, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_MSG_UPDOWN* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_MsgUpdown, sizeof(TASKWK_MSG_UPDOWN), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_MSG_UPDOWN ));

  twk->dir = dir;
  if( twk->dir == SCROLL_UP ){
    twk->y = 0;
    twk->diff = POPUP_DIFF;
  }else{
    twk->y = POPUP_HEIGHT;
    twk->diff = -POPUP_DIFF;
  }
  twk->frame = POPUP_COUNT;

//  IWASAWA_Printf("MsgWin UpDown Req %d\n", twk->dir );
  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_MsgUpdown( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_MSG_UPDOWN* twk = (TASKWK_MSG_UPDOWN*)tcb_wk;
 
  if( twk->ct++ < twk->frame ){
    twk->y += twk->diff;
    GFL_BG_SetScroll( FRM_POPUP, GFL_BG_SCROLL_Y_SET, twk->y );
    return;
  }
//  IWASAWA_Printf("MsgWin UpDown End %d\n", twk->dir );

  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ ポップアップ 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_WIN_POPUP{
  u8  seq;
  u8  wait;
  int child_task;
 
  BEACON_VIEW_PTR bvp;
  int* task_ct;
}TASKWK_WIN_POPUP;

static void taskAdd_WinPopup( BEACON_VIEW_PTR wk, STRBUF* str, int* task_ct );
static void tcb_WinPopup( GFL_TCBL *tcb , void* work);

void BVCEffReq_PopupMsgFromInfo( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info )
{
  GAMEBEACON_InfoWordset( info, wk->wordset, wk->tmpHeapID );
  BVCPrint_GetMsgToBuf( wk, BVCPrint_GetBeaconMsgID(info) );
  taskAdd_WinPopup( wk, wk->str_expand, &wk->eff_task_ct);
}

void BVCEffReq_PopupMsgSys( BEACON_VIEW_PTR wk, u8 msg_id )
{
  BVCPrint_GetMsgToBuf( wk, msg_id );
  taskAdd_WinPopup( wk, wk->str_expand, &wk->eff_task_ct);
}

/*
 *  @brief  メッセージウィンドウ ポップアップタスク登録
 */
static void taskAdd_WinPopup( BEACON_VIEW_PTR wk, STRBUF* str, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_WIN_POPUP* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_WinPopup, sizeof(TASKWK_WIN_POPUP), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_WIN_POPUP ));

  GFL_STR_CopyBuffer( wk->str_popup, str );
  
  twk->bvp = wk;
  twk->wait = POPUP_WAIT;
  
//  IWASAWA_Printf("WinPopup Req\n");
  BVC_MenuBarViewSet( wk, MENU_ALL, MENU_ST_OFF );

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_WinPopup( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_WIN_POPUP* twk = (TASKWK_WIN_POPUP*)tcb_wk;
  BEACON_VIEW_PTR bvp = twk->bvp;
  
  if( twk->child_task ){
    return;
  }

  switch( twk->seq ){
  case 0:
    BVCPrint_PopupWindow( twk->bvp, twk->bvp->str_popup, &twk->child_task );
    twk->seq++;
    return;
  case 1:
    BVCTaskAdd_MsgUpdown( twk->bvp, SCROLL_UP, &twk->child_task );
    twk->seq++;
    return;
  case 2:
    if( BVCPrint_PopupWindowTimeWaitCheck( &twk->wait ) == FALSE ){
      return;
    }
    BVCTaskAdd_MsgUpdown( twk->bvp, SCROLL_DOWN, &twk->child_task );
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
  BEACON_VIEW_PTR bvp;
  int* task_ct;
}TASKWK_WIN_GPOWER;

static const u8 DATA_GPowerUseMsg[GPOWER_USE_MAX] = {
 msg_sys_gpower_use00,
 msg_sys_gpower_use01,
 msg_sys_gpower_use02,
};

static void taskAdd_WinGPower( BEACON_VIEW_PTR wk, GPOWER_ID g_power, u8 type, int* task_ct );
static void tcb_WinGPowerYesNo( GFL_TCBL *tcb , void* work);
static void tcb_WinGPowerCheck( GFL_TCBL *tcb , void* work);

BOOL BVCEffReq_PopupMsgGPower( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info, BOOL haifu_f )
{
  GPOWER_ID gpower_id;
  
  if( haifu_f ){
    gpower_id = GAMEBEACON_Get_Action_DistributionGPower( info );
  }else{
    gpower_id = GAMEBEACON_Get_GPowerID( info );
  }
  //セキュリティ
  if( gpower_id == GPOWER_ID_NULL || gpower_id > GPOWER_ENABLE_ID_END ){
    GF_ASSERT(0);
    return FALSE;
  }

  //まったく同じパワーが発動中ならスルーする
  if( GPOWER_Check_OccurID( gpower_id, wk->gpower_data ) == gpower_id ){
    return FALSE;
  }
  GAMEBEACON_InfoWordset( info, wk->wordset, wk->tmpHeapID );
  WORDSET_ExpandStr( wk->wordset, wk->str_expand, wk->str_tmp );
  taskAdd_WinGPower( wk, gpower_id, GPOWER_USE_BEACON, &wk->eff_task_ct);
  return TRUE;
}
void BVCEffReq_PopupMsgGPowerMine( BEACON_VIEW_PTR wk )
{
  taskAdd_WinGPower( wk,
      wk->my_data.power, GPOWER_USE_MINE, &wk->eff_task_ct );
}
void BVCEffReq_PopupMsgGPowerSpecial( BEACON_VIEW_PTR wk, GPOWER_ID sp_gpower_id )
{
  taskAdd_WinGPower( wk, sp_gpower_id, GPOWER_USE_SPECIAL, &wk->eff_task_ct );
}

/*
 *  @brief  メッセージウィンドウ Gパワー使用確認タスク登録
 */
static void taskAdd_WinGPower( BEACON_VIEW_PTR wk, GPOWER_ID g_power, u8 type, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_WIN_GPOWER* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_WinGPowerYesNo, sizeof(TASKWK_WIN_GPOWER), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_WIN_GPOWER ));

  wk->ctrl.g_power = GPOWER_ID_NULL;
  wk->ctrl.mine_power_f = ( type == GPOWER_USE_MINE);

  twk->bvp = wk;
  twk->wait = POPUP_WAIT;
  twk->g_power = g_power;
  twk->type = type;

  WORDSET_RegisterGPowerName( wk->wordset, 1, g_power );

  //使用ポイント取得
  if( twk->type == GPOWER_USE_MINE ){
    twk->item_use = GPOWER_ID_to_Point( wk->gpower_data, g_power );
    twk->item_num = MYITEM_GetItemNum(wk->item_sv, ITEM_DERUDAMA, wk->tmpHeapID);
    
    WORDSET_RegisterNumber( wk->wordset, 2, twk->item_use, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wk->wordset, 3, twk->item_num, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  
    if( GPOWER_Check_OccurID( g_power, wk->gpower_data) == g_power ){
      twk->mypower_use = GPOWER_Check_MyPower();
      if( twk->mypower_use > 0 ){
        twk->mypower_min = twk->mypower_use/60;
        twk->mypower_sec = twk->mypower_use%60;
        WORDSET_RegisterNumber( wk->wordset, 4, twk->mypower_min, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
        WORDSET_RegisterNumber( wk->wordset, 5, twk->mypower_sec, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      }
    }
  }
  if( twk->type != GPOWER_USE_MINE ){
    BVC_MenuBarViewSet( wk, MENU_POWER, MENU_ST_OFF );
  }
  BVC_MenuBarViewSet( wk, MENU_HELLO, MENU_ST_OFF );
  BVC_MenuBarViewSet( wk, MENU_THANKS, MENU_ST_OFF );
  BVC_MenuBarViewSet( wk, MENU_RETURN, MENU_ST_OFF );

  if( twk->mypower_use || (twk->item_num < twk->item_use )){
    GFL_TCBL_ChangeFunc( tcb, tcb_WinGPowerCheck );
  }
  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_WinGPowerYesNo( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_WIN_GPOWER* twk = (TASKWK_WIN_GPOWER*)tcb_wk;
  BEACON_VIEW_PTR bvp = twk->bvp;

  if( twk->child_task > 0 ){
    return;
  }

  switch( twk->seq ){
  case 0:
    BVCPrint_GetMsgToBuf( bvp, DATA_GPowerUseMsg[twk->type] );
    BVCPrint_PopupWindow( bvp, bvp->str_expand, &twk->child_task );
    twk->seq++;
    return;
  case 1:
    BVCTaskAdd_MsgUpdown( bvp, SCROLL_UP, &twk->child_task );
    twk->seq++;
    return;
  case 2:
    BVC_PrintQueLimmitUpSet( bvp, TRUE );
    BVCTMenu_Create( bvp, TMENU_ID_YESNO );
    BVCDraw_ClearMenuWindow( bvp );
    BVCDraw_MenuIconVisibleSet( bvp, FALSE );
    twk->seq++;
    return;
  case 3: //選択待ち
    {
      int ret = BVCTMenu_Update( bvp, TMENU_ID_YESNO );
      if( ret < 0 ){
        return;
      }
      twk->ret = ret;

      switch( twk->ret){
      case TMENU_YN_CHECK:
        BVSEvent_Reserve( bvp, EV_GPOWER_CHECK_TMENU_YN );
        twk->seq = 3;
        break;
      case TMENU_YN_YES:
        //使うGパワーを覚えておく
        bvp->ctrl.g_power = twk->g_power;
        BVSEvent_Reserve( bvp, EV_GPOWER_USE );
        twk->seq = 4;
        break;
      case TMENU_YN_NO:
        twk->seq = 5;
        break;
      }
    }
    return;
  case 4: //使うイベント待ち
    if( bvp->ctrl.g_power != GPOWER_ID_NULL ) {
      //途中キャンセルされていたら、選択しなおし
      bvp->ctrl.g_power = GPOWER_ID_NULL;
      twk->seq = 3;
    }else{
      //正常に使えていれば、メニューを終了させる
      BVCTMenu_YnEnd( bvp );
      twk->seq = 6;
    }
    return;
  case 5: //キャンセル決定アニメ待ち
    if( BVCTMenu_YnEndWait( bvp, twk->ret )){
      twk->seq++;
    }
    return;
  case 6:
    BVCDraw_MenuWindow( bvp, msg_sys_now_record, &twk->child_task );
    BVCDraw_MenuIconVisibleSet( bvp, TRUE );
    BVCTaskAdd_MsgUpdown( bvp, SCROLL_DOWN, &twk->child_task );
    twk->seq++;
    return;
  default:
    break;
  }
  BVC_MenuBarViewSet( bvp, MENU_ALL, MENU_ST_ON );
  BVC_PrintQueLimmitUpSet( bvp, FALSE );
  
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

static void tcb_WinGPowerCheck( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_WIN_GPOWER* twk = (TASKWK_WIN_GPOWER*)tcb_wk;
  BEACON_VIEW_PTR bvp = twk->bvp;

  if( twk->child_task > 0 ){
    return;
  }

  switch( twk->seq ){
  case 0:
    if( twk->mypower_use ){ //既に使用中
      BVCPrint_GetMsgToBuf( bvp, msg_sys_gpower_use_err02+(twk->mypower_min != 0) );
    }else if( twk->item_num < twk->item_use ){  //アイテムが足りない
      BVCPrint_GetMsgToBuf( bvp, msg_sys_gpower_use_err01);
    }
    BVCPrint_PopupWindow( bvp, bvp->str_expand, &twk->child_task );
    twk->seq++;
    return;
  case 1:
    BVCTaskAdd_MsgUpdown( bvp, SCROLL_UP, &twk->child_task );
    twk->seq++;
    return;
  case 2:
    BVCDraw_MenuIconVisibleSet( bvp, FALSE );
    twk->seq++;
    return;
  case 3:
    BVCTMenu_Create( bvp, TMENU_ID_CHECK );
    BVCDraw_ClearMenuWindow( bvp );
    twk->seq++;
    return;
  case 4: //選択待ち
    {
      int ret = BVCTMenu_Update( bvp, TMENU_ID_CHECK );
      if( ret < 0 ){
        return;
      }
      twk->ret = ret;
      if(twk->ret == TMENU_CHECK_CALL){
        BVSEvent_Reserve( bvp, EV_GPOWER_CHECK_TMENU_CHK );
        bvp->gpower_check_req = TRUE;
        twk->seq = 5;
      }else{
        twk->seq = 6;
      }
    }
    return;
  case 5: //イベントが実行されたかチェック
    if( bvp->gpower_check_req ){
      twk->seq = 4; //途中キャンセルされていたら、選択しなおし
    }else{
      BVCTMenu_CheckEnd( bvp );
      twk->seq = 7; //実行されていればメニュー削除
    }
    return;
  case 6: //決定アニメ待ち
    if( BVCTMenu_CheckEndWait( bvp, twk->ret )){
      twk->seq++;
    }
    return;
  case 7:
    BVCDraw_MenuWindow( bvp, msg_sys_now_record, &twk->child_task );
    BVCDraw_MenuIconVisibleSet( bvp, TRUE );
    BVCTaskAdd_MsgUpdown( bvp, SCROLL_DOWN, &twk->child_task );
    twk->seq++;
    return;
  default:
    //終了
    break;
  }
  BVC_MenuBarViewSet( twk->bvp, MENU_ALL, MENU_ST_ON );
  
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}


