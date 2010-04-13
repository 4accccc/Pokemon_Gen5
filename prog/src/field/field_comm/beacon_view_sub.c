//==============================================================================
/**
 * @file    beacon_view_sub.c
 * @brief   すれ違い通信状態参照画面
 * @author  iwasawa
 * @date    10.01.21
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

#include "beacon_status.naix"
#include "wifi_unionobj.naix"
#include "beacon_view_local.h"
#include "beacon_view_sub.h"

#include "arc/wifi_unionobj_plt.cdat"

typedef struct _POINT{
  int x,y;
}POINT;

typedef struct _TRIANGLE{
  POINT p[3];
}TRIANGLE;

//////////////////////////////////////////////////////////////////
static void sp_gpower_ConditionCheck( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info, u8 old_log_max );
static void sp_gpower_RequestSet( BEACON_VIEW_PTR wk, u8 req_id );
static GPOWER_ID sp_gpower_RequestGet( BEACON_VIEW_PTR wk );
static BOOL sp_gpower_ZoromeCheck( u32 log_count );

static int calc_Cross( POINT* a, POINT* b ) ;
static void calc_PointMinus( POINT* a, POINT* b, POINT* out );
static BOOL calc_PointInTriangle( POINT* p, TRIANGLE* tri );

static int touchin_CheckPanel( BEACON_VIEW_PTR wk, POINT* tp );
static int touchin_CheckUpDown( BEACON_VIEW_PTR wk, POINT* tp );
static int touchin_CheckMenu( BEACON_VIEW_PTR wk, POINT* tp );

static void draw_LogNumWindow( BEACON_VIEW_PTR wk );
static void draw_MenuWindow( BEACON_VIEW_PTR wk, u8 msg_id );

static u32 sub_GetBeaconMsgID(const GAMEBEACON_INFO *info);
static void sub_PlaySE( u16 se_no );

static void act_SetPosition( GFL_CLWK* act, s16 x, s16 y );
static void act_AnmStart( GFL_CLWK* act, u8 anm_no );
static void obj_MenuIconVisibleSet( BEACON_VIEW_PTR wk, BOOL visible_f );
static void obj_UpDownViewSet( BEACON_VIEW_PTR wk );
static void obj_ThanksViewSet( BEACON_VIEW_PTR wk );

static void panel_VisibleSet( PANEL_WORK* pp, BOOL visible_f );
static void panel_IconVisibleSet( PANEL_WORK* pp, BOOL visible_f );
static void panel_Clear( PANEL_WORK* pp );
static void panel_Entry( PANEL_WORK* pp, u8 data_idx,u8 line );
static PANEL_WORK* panel_GetPanelFromDataIndex( BEACON_VIEW_PTR wk, u8 data_idx );
static void panel_UnionObjUpdate( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info);
static void panel_IconObjUpdate( BEACON_VIEW_PTR wk, PANEL_WORK* pp, u8 icon);
static u8 panel_FrameColorGet( GAMEBEACON_INFO* info );
static void panel_ColorPlttSet( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info );
static void panel_MsgPrint( BEACON_VIEW_PTR wk, PANEL_WORK* pp, STRBUF* str );
static void panel_NamePrint( BEACON_VIEW_PTR wk, PANEL_WORK* pp, BOOL force_f );
static void panel_Draw( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info );

static void list_TimeOutCheck( BEACON_VIEW_PTR wk );
static void list_UpDownReq( BEACON_VIEW_PTR wk, u8 dir );
static void list_ScrollReq( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info, u8 ofs, u8 idx, u8 dir, BOOL new_f );

static void effReq_PanelScroll( BEACON_VIEW_PTR wk, u8 dir, PANEL_WORK* new_pp, PANEL_WORK* ignore_pp );
static void effReq_PanelSlideIn( BEACON_VIEW_PTR wk, PANEL_WORK* pp );
static BOOL effReq_PopupMsg( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info, BOOL new_f );
static void effReq_PopupMsgFromInfo( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info );
static void effReq_PopupMsgSys( BEACON_VIEW_PTR wk, u8 msg_id );
static BOOL effReq_PopupMsgGPower( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info );
static void effReq_PopupMsgGPowerMine( BEACON_VIEW_PTR wk );
static void effReq_PopupMsgGPowerSpecial( BEACON_VIEW_PTR wk, GPOWER_ID sp_gpower_id );
static void effReq_SetIconEffect( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info, BOOL new_f );
static void effReq_SetPanelFlash( BEACON_VIEW_PTR wk, u8 target_ofs );

//////////////////////////////////////////////////////////////////

/*
 *  @brief  初期描画
 */
void BeaconView_InitialDraw( BEACON_VIEW_PTR wk )
{
  int i;

  PaletteTrans_AutoSet( wk->pfd, TRUE );

  //現在のログ数
  draw_LogNumWindow( wk );
  //現在のトータルすれ違い人数
  draw_MenuWindow( wk, msg_sys_now_record );

  //最新リストを描画
  for(i = 0;i < wk->ctrl.view_max;i++){
    int idx, ofs;
    PANEL_WORK* pp;

    ofs = wk->ctrl.view_top+i;
    GAMEBEACON_InfoTblRing_GetBeacon( wk->infoLog, wk->tmpInfo, &wk->tmpTime, ofs );
    idx = GAMEBEACON_InfoTblRing_Ofs2Idx( wk->infoLog, ofs );
  
    pp = panel_GetPanelFromDataIndex( wk, PANEL_DATA_BLANK );
    panel_Entry( pp, idx, PANEL_VIEW_START+i );  //パネル新規登録
    panel_Draw( wk, pp, wk->tmpInfo );   //パネル描画
    panel_VisibleSet( pp, TRUE );   //パネル描画
  }
  //矢印と御礼メニューの見た目セット
  obj_UpDownViewSet( wk );
  obj_ThanksViewSet( wk );
  if( wk->my_data.power == GPOWER_ID_NULL ){
    act_AnmStart( wk->pAct[ACT_POWER], ACTANM_POWER_OFF );
  }
  GFL_NET_ReloadIcon();
}

/*
 *  @brief  メインループ内での入力チェック
 */
int BeaconView_CheckInput( BEACON_VIEW_PTR wk )
{
  int ret;
  POINT tp;
  BOOL my_power;

#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
    wk->deb_stack_check_throw = 1;
    OS_Printf("StackCheckThrow = %d\n",wk->deb_stack_check_throw );
  }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
    wk->deb_stack_check_throw = 0;
    OS_Printf("StackCheckThrow = %d\n",wk->deb_stack_check_throw );
  }
#endif

  if( !GFL_UI_TP_GetPointTrg( (u32*)&tp.x, (u32*)&tp.y )){
    return SEQ_MAIN;
  }
  
  //メニューあたり判定
  ret = touchin_CheckMenu( wk, &tp );
  if(ret != GFL_UI_TP_HIT_NONE){
    return SEQ_GPOWER_USE+ret;
  }

  //パネルあたり判定
  ret = touchin_CheckPanel( wk, &tp );
  if(ret != GFL_UI_TP_HIT_NONE){
    wk->ctrl.target = ret;
    effReq_SetPanelFlash( wk, ret );
    IWASAWA_Printf("PanelTarget = %d\n",ret);
    return SEQ_CALL_DETAIL_VIEW;
  }

  //上下矢印あたり判定
  ret = touchin_CheckUpDown( wk, &tp );
  if(ret != GFL_UI_TP_HIT_NONE){
    list_UpDownReq( wk, ret );
    return SEQ_VIEW_UPDATE;
  }
  return SEQ_MAIN;
}

/*
 *  @brief  特殊Gパワー発動チェック 
 */
BOOL BeaconView_CheckSpecialGPower( BEACON_VIEW_PTR wk )
{
  GPOWER_ID power = sp_gpower_RequestGet( wk );

  if( power == GPOWER_ID_NULL ){
    return FALSE;
  }
  if( GPOWER_Check_OccurID( power, wk->gpower_data ) == power ){
    return FALSE; //同じパワー発動中は捨てる
  }
  //ポップアップをリクエスト
  effReq_PopupMsgGPowerSpecial( wk, power );
 
  return TRUE;
}

/*
 *  @brief  スタックからログを一件取り出し
 */
BOOL BeaconView_CheckStack( BEACON_VIEW_PTR wk )
{
  BOOL new_f = FALSE, popup_f = FALSE;
  int ofs,idx,old_max;
  PANEL_WORK* pp;

  //スタックに積まれた情報をチェック
#ifdef PM_DEBUG
  if( wk->deb_stack_check_throw ){
    list_TimeOutCheck( wk );
    return FALSE;
  }
#endif
  if( !GAMEBEACON_Stack_GetInfo( wk->infoStack, wk->tmpInfo, &wk->tmpTime ) ){
    list_TimeOutCheck( wk );
    return FALSE;
  }

  //エラーチェック
  if( GAMEBEACON_Check_Error( wk->tmpInfo)){
    return FALSE;
  }
  ofs = GAMEBEACON_InfoTblRing_SetBeacon( wk->infoLog,
          wk->tmpInfo, wk->tmpTime, &new_f );
  idx = GAMEBEACON_InfoTblRing_Ofs2Idx( wk->infoLog, ofs );

  //登録数更新
  old_max = wk->ctrl.max;
  if( wk->ctrl.max < BS_LOG_MAX ){
    wk->ctrl.max += new_f;
  
    if( wk->ctrl.max <= PANEL_VIEW_MAX){
      wk->ctrl.view_max = wk->ctrl.max;
    }
  }
  //ポップアップリクエスト
  popup_f = effReq_PopupMsg( wk, wk->tmpInfo, new_f );

  //新規でない時はスクロール無し
  if( !new_f ){
    pp = panel_GetPanelFromDataIndex( wk, idx );
    if(pp == NULL){
      return FALSE;
    }
    //ターゲットパネル書換え
    panel_Draw( wk, pp, wk->tmpInfo );
    panel_VisibleSet( pp, TRUE );
    if(popup_f){
      effReq_SetIconEffect( wk, pp, wk->tmpInfo, FALSE );
      sub_PlaySE( BVIEW_SE_ICON );
    }
    return TRUE;
  }
  //新規
  wk->log_count = MISC_CrossComm_IncSuretigaiCount( wk->misc_sv );    

  draw_LogNumWindow( wk );
  draw_MenuWindow( wk, msg_sys_now_record );
  obj_ThanksViewSet( wk );
  
  //特殊Gパワーチェック
  sp_gpower_ConditionCheck( wk, wk->tmpInfo, old_max );

  if( wk->ctrl.view_top > 0){ //描画リストがトップでない時はスクロールのみ
    ofs = wk->ctrl.view_top;
    GAMEBEACON_InfoTblRing_GetBeacon( wk->infoLog, wk->tmpInfo, &wk->tmpTime, ofs );
    idx = GAMEBEACON_InfoTblRing_Ofs2Idx( wk->infoLog, ofs );
  }
  //スクロールリクエスト
  list_ScrollReq( wk, wk->tmpInfo, ofs, idx, SCROLL_DOWN, (wk->ctrl.view_top == 0));
  wk->ctrl.view_top = ofs;
  sub_PlaySE( BVIEW_SE_NEW_PLAYER );
  return TRUE;
}

/*
 *  @brief  アクティブ・パッシブ切替
 */
void BeaconView_SetViewPassive( BEACON_VIEW_PTR wk, BOOL passive_f )
{
  if(passive_f){
    SoftFadePfd( wk->pfd, FADE_SUB_BG, 0, 16*BG_PALANM_AREA, 6, 0x0000);
    SoftFadePfd( wk->pfd, FADE_SUB_OBJ, 0, 16*ACT_PAL_WMI, 6, 0x0000);
    G2S_SetBlendAlpha( ALPHA_1ST_PASSIVE, ALPHA_2ND, ALPHA_EV1_PASSIVE, ALPHA_EV2_PASSIVE);

    if( wk->printStream != NULL ){
      PRINTSYS_PrintStreamStop( wk->printStream );
    }
    GFL_FONTSYS_SetDefaultColor();
  }else{
    SoftFadePfd( wk->pfd, FADE_SUB_BG, 0, 16*BG_PALANM_AREA, 0, 0x0000);
    SoftFadePfd( wk->pfd, FADE_SUB_OBJ, 0, 16*ACT_PAL_WMI, 0, 0x0000);
    G2S_SetBlendAlpha( ALPHA_1ST_NORMAL, ALPHA_2ND, ALPHA_EV1_NORMAL, ALPHA_EV2_NORMAL);
    if( wk->printStream != NULL ){
      PRINTSYS_PrintStreamRun( wk->printStream );
    }
//    GFL_FONTSYS_SetColor( FCOL_POPUP_MAIN, FCOL_POPUP_SDW, FCOL_POPUP_BASE );
  }
}

////////////////////////////////////////////////////////////////////////////
//サブシーケンス

///////////////////////////////////////////////////////////////////
/*
 *  @brief  サブシーケンス　GPower
 */
///////////////////////////////////////////////////////////////////
static int sseq_gpower_CheckInput( BEACON_VIEW_PTR wk );

/*
 *  @brief  サブシーケンス　GPower使用メイン
 */
BOOL BeaconView_SubSeqGPower( BEACON_VIEW_PTR wk )
{
  switch( wk->sub_seq ){
  case 0:
    BeaconView_MenuBarViewSet( wk, MENU_POWER, MENU_ST_ANM );
    effReq_PopupMsgGPowerMine( wk );
    wk->sub_seq++;
    break;
  case 1:
    if( wk->eff_task_ct ){
      break;
    }
    wk->sub_seq = 0;
    return TRUE;
  }
  return FALSE;
}

static int sseq_gpower_CheckInput( BEACON_VIEW_PTR wk )
{
  return 0;
}

///////////////////////////////////////////////////////////////////
/*
 *  @brief  サブシーケンス　御礼
 */
///////////////////////////////////////////////////////////////////
static int sseq_thanks_CheckInput( BEACON_VIEW_PTR wk );

/*
 *  @brief  サブシーケンス　御礼メイン
 */
BOOL BeaconView_SubSeqThanks( BEACON_VIEW_PTR wk )
{
  switch( wk->sub_seq ){
  case SSEQ_THANKS_ICON_ANM:
    BeaconView_MenuBarViewSet( wk, MENU_THANKS, MENU_ST_ANM );
    wk->sub_seq++;
    break;
  case SSEQ_THANKS_ICON_ANM_WAIT:
    if( BeaconView_MenuBarCheckAnm( wk, MENU_THANKS )){
      break;
    }
    BeaconView_MenuBarViewSet( wk, MENU_POWER, MENU_ST_OFF );
    BeaconView_MenuBarViewSet( wk, MENU_RETURN, MENU_ST_OFF );
    draw_MenuWindow( wk, msg_sys_thanks );
    wk->sub_seq++;
    break;
  case SSEQ_THANKS_MAIN:
    wk->sub_seq = sseq_thanks_CheckInput( wk );
    break;
  case SSEQ_THANKS_DECIDE:
    {
      u8 idx;
      PANEL_WORK* pp;
      
      idx = GAMEBEACON_InfoTblRing_Ofs2Idx( wk->infoLog, wk->ctrl.target+wk->ctrl.view_top );
      pp = panel_GetPanelFromDataIndex( wk, idx ); 

      //プレイヤー名展開
      WORDSET_RegisterWord( wk->wordset, 0, pp->name, pp->sex, TRUE, PM_LANG );
      effReq_PopupMsgSys( wk, msg_sys_thanks_send );
 
      OS_TPrintf("ありがとう ビーコンセット %d\n", pp->tr_id );
      GAMEBEACON_Set_Thankyou( wk->gdata, pp->tr_id );
    }
    wk->sub_seq = SSEQ_THANKS_DECIDE_WAIT;
    break;
  case SSEQ_THANKS_DECIDE_WAIT:
    if( wk->eff_task_ct == 0 ){
      wk->sub_seq = SSEQ_THANKS_END;
    }
    break;
    break;
  case SSEQ_THANKS_VIEW_UPDATE:
    if( wk->eff_task_ct == 0 ){
      BeaconView_MenuBarViewSet( wk, MENU_THANKS, MENU_ST_ON );
      wk->sub_seq = SSEQ_THANKS_MAIN;
    }
    break;
  case SSEQ_THANKS_END:
    BeaconView_MenuBarViewSet( wk, MENU_ALL, MENU_ST_ON );
    draw_MenuWindow( wk, msg_sys_now_record );
    wk->sub_seq = 0;
    return TRUE;
  }
  return FALSE;
}

/*
 *  @brief  御礼対象選択ループ内での入力チェック
 */
static int sseq_thanks_CheckInput( BEACON_VIEW_PTR wk )
{
  int ret;
  POINT tp;

  if( !GFL_UI_TP_GetPointTrg( (u32*)&tp.x, (u32*)&tp.y )){
    return SSEQ_THANKS_MAIN;
  }
  
  //メニューあたり判定
  ret = touchin_CheckMenu( wk, &tp );
  if(ret == MENU_THANKS){
    return SSEQ_THANKS_END;  //キャンセル
  }

  //パネルあたり判定
  ret = touchin_CheckPanel( wk, &tp );
  if(ret != GFL_UI_TP_HIT_NONE){
    wk->ctrl.target = ret;
    effReq_SetPanelFlash( wk, ret );
    return SSEQ_THANKS_DECIDE;
  }

  //上下矢印あたり判定
  ret = touchin_CheckUpDown( wk, &tp );
  if(ret != GFL_UI_TP_HIT_NONE){
    list_UpDownReq( wk, ret );
    return SSEQ_THANKS_VIEW_UPDATE;
  }
  return SSEQ_THANKS_MAIN;
}

/*
 *  @brief  メニューバーアニメセット
 */
void BeaconView_MenuBarViewSet( BEACON_VIEW_PTR wk, MENU_ID id, MENU_STATE state )
{
  u8 flag;
  
  if( id == MENU_ALL ){
    flag = MENU_F_ALL;
  }else{
    flag = 0x01 << id;
  }

  if( flag & MENU_F_POWER ){
    if( wk->my_data.power != GPOWER_ID_NULL ){
      act_AnmStart( wk->pAct[ACT_POWER], state+ACTANM_POWER_ON );
    }
  }
  if( flag & MENU_F_HELLO ){
    act_AnmStart( wk->pAct[ACT_HELLO], state+ACTANM_HELLO_ON );
  }
  if( flag & MENU_F_THANKS ){
    if( wk->ctrl.max > 0 ){
      act_AnmStart( wk->pAct[ACT_THANKS], state+ACTANM_THANKS_ON );
    }
  }
  if( flag & MENU_F_RETURN ){
    act_AnmStart( wk->pAct[ACT_RETURN], state+ACTANM_RETURN_ON );
  }
}

/*
 *  @brief  メニューバーアニメウェイト
 */
BOOL BeaconView_MenuBarCheckAnm( BEACON_VIEW_PTR wk, MENU_ID id )
{
  return GFL_CLACT_WK_CheckAnmActive( wk->pAct[ACT_POWER+id] );
}

////////////////////////////////////////////////////////////////////////////
//特殊Gパワー発動条件チェック
////////////////////////////////////////////////////////////////////////////

static void sp_gpower_ConditionCheck( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info, u8 old_log_max ) 
{
  int i;
  u8 version_ct = 0, sex_ct = 0;
  u16 time;

#ifdef PM_DEBUG
  {
    int key = GFL_UI_KEY_GetCont();
    if( key & PAD_BUTTON_L) {
      sp_gpower_RequestSet( wk, SP_GPOWER_REQ_HATCH_UP ); 
    }
    if( key & PAD_BUTTON_R ){
      sp_gpower_RequestSet( wk, SP_GPOWER_REQ_CAPTURE_UP ); 
    }
    if( key & PAD_BUTTON_A ){
      sp_gpower_RequestSet( wk, SP_GPOWER_REQ_SALE ); 
    }
  }
#endif

  //海外版ロムとすれ違ったか？
  if( GAMEBEACON_Get_PmLanguage( info ) != PM_LANG ){
    sp_gpower_RequestSet( wk, SP_GPOWER_REQ_HATCH_UP );
  }
  //すれ違い人数が100の倍数になったか？
  if( ( wk->log_count % 100 ) == 0 ){
    sp_gpower_RequestSet( wk, SP_GPOWER_REQ_SALE );
  }else if( sp_gpower_ZoromeCheck( wk->log_count )){
    //すれ違い人数が3桁以上のゾロ目になったか？
    sp_gpower_RequestSet( wk, SP_GPOWER_REQ_CAPTURE_UP );
  }
  //プレイヤーリストが30人埋まったか？
  if( old_log_max == (BS_LOG_MAX-1) && wk->ctrl.max == BS_LOG_MAX ){
    sp_gpower_RequestSet( wk, SP_GPOWER_REQ_EXP_UP );
  }

  //ログ内カウンター収集
  for( i = 0;i < wk->ctrl.max;i++ ){
    GAMEBEACON_InfoTblRing_GetBeacon( wk->infoLog, wk->tmpInfo2, &time, i );
    if( GAMEBEACON_Get_Sex(wk->tmpInfo2) != wk->my_data.sex ){
      ++sex_ct;
    }
    if( GAMEBEACON_Get_PmVersion( wk->tmpInfo2 ) != wk->my_data.pm_version ){
      ++version_ct; 
    }
  }
  //自分と異なる性別が20人以上ログにいるか？
  if( sex_ct > 19 ){
    sp_gpower_RequestSet( wk, SP_GPOWER_REQ_MONEY_UP );
  }
  //自分と異なるカセットバージョンが20人以上ログにいるか？
  if( version_ct > 19 ){
    sp_gpower_RequestSet( wk, SP_GPOWER_REQ_NATSUKI_UP );
  }
}

//----------------------------------------------------------
/*
 *  @brief  特殊Gパワーの発動を予約する
 */
//----------------------------------------------------------
static void sp_gpower_RequestSet( BEACON_VIEW_PTR wk, u8 req_id )
{
  int i;

  //既に予約リストにあるかどうか？
  for(i = 0;i < wk->sp_gpower_req_ct;i++){
    if(wk->sp_gpower_req[i] == req_id){
      return;
    }
  }
  wk->sp_gpower_req[wk->sp_gpower_req_ct++] = req_id;
}

//----------------------------------------------------------
/*
 *  @brief  予約された特殊Gパワーを取り出す
 *
 *  @note なければGPOWER_ID_NULLを返す
 */
//----------------------------------------------------------
static GPOWER_ID sp_gpower_RequestGet( BEACON_VIEW_PTR wk )
{
  GPOWER_ID gpower_id;
  static const DATA_SpGPowerTbl[ SP_GPOWER_REQ_MAX ] = {
    GPOWER_ID_EGG_INC_MAX,
    GPOWER_ID_NATSUKI_MAX,
    GPOWER_ID_SALE_MAX,
    GPOWER_ID_EXP_INC_MAX,
    GPOWER_ID_MONEY_MAX,
    GPOWER_ID_CAPTURE_MAX,
  };

  if( wk->sp_gpower_req[0] == SP_GPOWER_REQ_NULL ){
    return GPOWER_ID_NULL;
  }
  //一つ取り出す
  gpower_id = DATA_SpGPowerTbl[ wk->sp_gpower_req[0]-1 ];
  wk->sp_gpower_req_ct--;

  //テーブルを詰める
  MI_CpuCopy8( &(wk->sp_gpower_req[1]), &(wk->sp_gpower_req[0]), sizeof(u8)*wk->sp_gpower_req_ct);
  wk->sp_gpower_req[wk->sp_gpower_req_ct] = SP_GPOWER_REQ_NULL;

  return gpower_id;
}

//----------------------------------------------------------
/*
 *  @brief  すれ違い人数が3桁以上のゾロ目かチェック 
 */
//----------------------------------------------------------
static BOOL sp_gpower_ZoromeCheck( u32 log_count )
{
  int i,no;
 
  if( (log_count / 100) == 0){
    return FALSE; //2桁以下
  }
  no = log_count % 10;  //1桁目取得
  log_count /= 10;

  //2桁目以降を取得
  do{
    u8 num = log_count % 10;
    if( num != no ){
      return FALSE;
    }
    log_count /= 10;
  }while( log_count > 0 );

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////
//サブルーチン
////////////////////////////////////////////////////////////////////////////

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
static BOOL calc_PointInTriangle( POINT* p, TRIANGLE* tri )
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
 *  @brief  パネルあたり判定
 */
static int touchin_CheckPanel( BEACON_VIEW_PTR wk, POINT* tp )
{
  int i, ox, oy;
  TRIANGLE tri;

  for(i = 0; i < wk->ctrl.view_max; i++){
    ox = ACT_PANEL_OX*i;
    oy = ACT_PANEL_OY*i;
    tri.p[0].x = TP_PANEL_X1+ox;  
    tri.p[0].y = TP_PANEL_Y1+oy;  
    tri.p[1].x = TP_PANEL_X2+ox;  
    tri.p[1].y = TP_PANEL_Y2+oy;  
    tri.p[2].x = TP_PANEL_X3+ox;  
    tri.p[2].y = TP_PANEL_Y3+oy;  

    if( calc_PointInTriangle( tp, &tri )){
      return i;
    }
    tri.p[0].x = TP_PANEL_X1+ox;  
    tri.p[0].y = TP_PANEL_Y1+oy;  
    tri.p[1].x = TP_PANEL_X3+ox;  
    tri.p[1].y = TP_PANEL_Y3+oy;  
    tri.p[2].x = TP_PANEL_X4+ox;  
    tri.p[2].y = TP_PANEL_Y4+oy;  

    if( calc_PointInTriangle( tp, &tri )){
      return i;
    }
  }
  return GFL_UI_TP_HIT_NONE;
}

/*
 *  @brief  上下矢印キーあたり判定
 */
static int touchin_CheckUpDown( BEACON_VIEW_PTR wk, POINT* tp )
{
  int i,j;
  TRIANGLE tri;
  u8 enable = 0;

  static const POINT tbl[2][5] = {
   {
     { TP_UP_X1, TP_UP_Y1 },
     { TP_UP_X2, TP_UP_Y2 },
     { TP_UP_X3, TP_UP_Y3 },
     { TP_UP_X4, TP_UP_Y4 },
     { TP_UP_X5, TP_UP_Y5 },
   },
   {
     { TP_DOWN_X1, TP_DOWN_Y1 },
     { TP_DOWN_X2, TP_DOWN_Y2 },
     { TP_DOWN_X3, TP_DOWN_Y3 },
     { TP_DOWN_X4, TP_DOWN_Y4 },
     { TP_DOWN_X5, TP_DOWN_Y5 },
   },
  };
  static const u8 point[][3] = {
   { 0, 1, 4},
   { 1, 2, 4},
   { 2, 3, 4},
  };

  if( wk->ctrl.view_top > 0 ){  //上キーチェック
    enable |= 1;
  }
  if( (wk->ctrl.view_top+wk->ctrl.view_max) < wk->ctrl.max ){
    enable |= 2;
  }

  for(i = 0;i < 2;i++){
    if( !(enable & (i+1))){
      continue;
    }
    for(j = 0;j < 3;j++){
      tri.p[0] = tbl[i][point[j][0]];
      tri.p[1] = tbl[i][point[j][1]];
      tri.p[2] = tbl[i][point[j][2]];
      if( calc_PointInTriangle( tp, &tri )){
        act_AnmStart( wk->pAct[ACT_UP+i], ACTANM_UP_ANM+(i*3) );
        sub_PlaySE( BVIEW_SE_UPDOWN );
        return (1-i);
      }
    }
  }
  return GFL_UI_TP_HIT_NONE;
}

/*
 *  @brief  メニューバーあたり判定
 */
static int touchin_CheckMenu( BEACON_VIEW_PTR wk, POINT* tp )
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
      sub_PlaySE( se_tbl[i] );
      return i;
    }
  }
  return GFL_UI_TP_HIT_NONE;
}

/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
static void print_GetMsgToBuf( BEACON_VIEW_PTR wk, u8 msg_id )
{
  GFL_MSG_GetString( wk->mm_status, msg_id, wk->str_tmp);
  WORDSET_ExpandStr( wk->wordset, wk->str_expand, wk->str_tmp );
}

/*
 *  @brief  文字列一括描画
 */
static void print_Allput( BEACON_VIEW_PTR wk, GFL_BMP_DATA* bmp, u8 msg_id, u8 x, u8 y, u8 clear, PRINTSYS_LSB color )
{
  GFL_BMP_Clear( bmp, clear );
  print_GetMsgToBuf( wk, msg_id );
	PRINTSYS_PrintColor( bmp, x, y, wk->str_expand, wk->fontHandle, color );
}

/*
 *  @brief  ログ人数表示
 */
static void draw_LogNumWindow( BEACON_VIEW_PTR wk )
{
  WORDSET_RegisterNumber( wk->wordset, 0, wk->ctrl.max, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  print_Allput( wk, wk->foamLogNum.bmp, msg_sys_lognum, 0, 0, 0, FCOL_FNTOAM_W );
	
  BmpOam_ActorBmpTrans( wk->foamLogNum.oam );
}

/*
 *  @brief  メニューウィンドウ表示
 */
static void draw_MenuWindow( BEACON_VIEW_PTR wk, u8 msg_id )
{
  WORDSET_RegisterNumber( wk->wordset, 0, wk->log_count, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  print_Allput( wk, wk->win[WIN_MENU].bmp, msg_id, 0, 4, 0, FCOL_WHITE_N );
  GFL_BMPWIN_MakeTransWindow( wk->win[WIN_MENU].win );
}

/*
 *  @brief  ポップアップウィンドウ文字列描画
 */
static void print_PopupWindow( BEACON_VIEW_PTR wk, STRBUF* str, u8 wait )
{
  GFL_BMP_Clear( wk->win[WIN_POPUP].bmp, FCOL_POPUP_BASE );

  GF_ASSERT(wk->printStream == NULL);
  wk->printStream = NULL;

  if( wait == 0 ){
    PRINT_UTIL_PrintColor( &wk->win[WIN_POPUP].putil, wk->printQue, 0, 0, str, wk->fontHandle, FCOL_POPUP );
    GFL_BMPWIN_MakeTransWindow( wk->win[WIN_POPUP].win );
  }else{
    //ストリーム開始
    wk->printStream = PRINTSYS_PrintStream( wk->win[WIN_POPUP].win, 0, 0,
        str, wk->fontHandle, wait, wk->pTcbSys, 0, wk->heap_sID, FCOL_POPUP_BASE );
  }
}

//------------------------------------------------------------
/*
 *  @brief  ポップアップ　タイムウェイトチェック
 */
//------------------------------------------------------------
static BOOL print_PopupWindowTimeWaitCheck( u8* p_wait )
{
  if( (*p_wait)-- > 0 && (!GFL_UI_TP_GetTrg()) ){
    return FALSE; 
  }
  *p_wait = 0;
  return TRUE;
}

//------------------------------------------------------------
/*
 *  @brief  ポップアップ　ストリーム終了チェック
 *
 *  @retval TRUE  描画終了
 */
//------------------------------------------------------------
static BOOL print_PopupWindowStreamCheck( BEACON_VIEW_PTR wk )
{
  PRINTSTREAM_STATE state;
  
  if( wk->printStream == NULL){
    return TRUE;
  }
  state = PRINTSYS_PrintStreamGetState( wk->printStream );
  APP_KEYCURSOR_Main( wk->key_cursor, wk->printStream, wk->win[WIN_POPUP].win );

  switch( state ){
  case PRINTSTREAM_STATE_DONE:  //全文描画終了
    PRINTSYS_PrintStreamDelete( wk->printStream );
    wk->printStream = NULL;
    return TRUE;
  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    if( GFL_UI_TP_GetTrg() ){
      PRINTSYS_PrintStreamReleasePause( wk->printStream );
    }
    break;
  case PRINTSTREAM_STATE_RUNNING :  // 実行中
    // メッセージスキップ
    if( GFL_UI_TP_GetCont() ){
      PRINTSYS_PrintStreamShortWait( wk->printStream, 0 );
    }
    break;
  }
  return FALSE;
}

/*
 *  @brief  TaskMenu YesNo生成
 */
static void tmenu_Create( BEACON_VIEW_PTR wk, u8 menu_idx )
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
                        px+TMENU_W*i, TMENU_PY, TMENU_W, TMENU_H, TRUE, wk->heap_sID );
  }
}

/*
 *  @brief  TaskMenu アップデート
 */
static int tmenu_Update( BEACON_VIEW_PTR wk,u8 menu_idx )
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
    if( APP_TASKMENU_WIN_IsTrg( tmenu[i].work )){
      APP_TASKMENU_WIN_SetDecide( tmenu[i].work, TRUE );
      if(i < (max-1)){
        sub_PlaySE( BVIEW_SE_DECIDE );
      }else{
        sub_PlaySE( BVIEW_SE_CANCEL );
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
static BOOL tmenu_YnEndWait( BEACON_VIEW_PTR wk, u8 select )
{
  int i;

  APP_TASKMENU_WIN_Update( wk->tmenu[select].work );

  if( APP_TASKMENU_WIN_IsFinish( wk->tmenu[select].work )){
    if( select == TMENU_YN_CHECK ){
      APP_TASKMENU_WIN_ResetDecide( wk->tmenu[select].work );
      return TRUE;  //<確認>を選んだ場合消さないでリセット
    }
    for( i = 0;i < TMENU_YN_MAX;i++){
      APP_TASKMENU_WIN_Delete( wk->tmenu[i].work );
      wk->tmenu[i].work = NULL;
    }
    return TRUE;
  }
  return FALSE;
}

/*
 *  @brief  TaskMenu Gパワー確認ダイアログ終了アニメ待ち
 */
static BOOL tmenu_CheckEndWait( BEACON_VIEW_PTR wk, u8 select )
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


//==================================================================
/**
 * ビーコン情報から表示するメッセージIDを取得する
 *
 * @param   info		対象のビーコン情報へのポインタ
 */
//==================================================================
static u32 sub_GetBeaconMsgID(const GAMEBEACON_INFO *info)
{
  GAMEBEACON_ACTION action = GAMEBEACON_Get_Action_ActionNo(info);

  if(action == GAMEBEACON_ACTION_NULL || action >= GAMEBEACON_ACTION_MAX){
    GF_ASSERT_MSG(0, "action_no = %d\n", action);
    return msg_beacon_001;
  }
  return msg_beacon_001 + action - GAMEBEACON_ACTION_SEARCH;
}

/*
 *  @brief  SE再生
 */
static void sub_PlaySE( u16 se_no )
{
  PMSND_PlaySE( se_no );
}

/*
 *  @brief  アクター　座標セット
 */
static void act_SetPosition( GFL_CLWK* act, s16 x, s16 y )
{
  GFL_CLACTPOS pos;
  pos.x = x;
  pos.y = y;
  
  GFL_CLACT_WK_SetPos( act, &pos, ACT_RENDER_ID);
}

/*
 *  @brief  アクター アニメスタート
 */
static void act_AnmStart( GFL_CLWK* act, u8 anm_no )
{
  GFL_CLACT_WK_SetAnmSeq( act, anm_no );
  GFL_CLACT_WK_ResetAnm( act );
}

/*
 *  @brief  メニューアクター Viewセット
 */
static void obj_MenuIconVisibleSet( BEACON_VIEW_PTR wk, BOOL visible_f )
{
  int i;

  for(i = ACT_POWER;i <= ACT_RETURN;i++){
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[i], visible_f );
  }
}

/*
 *  @brief  UpDownアクター Viewセット
 */
static void obj_UpDownViewSet( BEACON_VIEW_PTR wk )
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
 *  @brief  御礼アクター Viewセット
 */
static void obj_ThanksViewSet( BEACON_VIEW_PTR wk )
{
  if( wk->ctrl.max == 0){
    GFL_CLACT_WK_SetAnmSeq( wk->pAct[ACT_THANKS], ACTANM_THANKS_OFF );  
  }else{
    GFL_CLACT_WK_SetAnmSeq( wk->pAct[ACT_THANKS], ACTANM_THANKS_ON );  
  }
}

/*
 *  @brief  パネル 表示状態On/Off
 */
static void panel_VisibleSet( PANEL_WORK* pp, BOOL visible_f )
{
  BmpOam_ActorSetDrawEnable( pp->msgOam.oam, visible_f );
  GFL_CLACT_WK_SetDrawEnable( pp->cUnion, visible_f );
  GFL_CLACT_WK_SetDrawEnable( pp->cPanel, visible_f );

  if(pp->rank > 0){
    GFL_CLACT_WK_SetDrawEnable( pp->cRank, visible_f );
  }else{
    GFL_CLACT_WK_SetDrawEnable( pp->cRank, FALSE );
  }
  if( !visible_f ){ //隠したい時だけ強制
    panel_IconVisibleSet( pp, visible_f );
  }
}

/*
 *  @brief  パネルアイコン 表示状態On/Off
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
  panel_VisibleSet( pp, FALSE );
  panel_IconVisibleSet( pp, FALSE );

  GFL_BMP_Clear( pp->msgOam.bmp, 0 );
	BmpOam_ActorBmpTrans( pp->msgOam.oam );
  GFL_STR_ClearBuffer( pp->name );
  GFL_STR_ClearBuffer( pp->str );

  pp->data_idx = PANEL_DATA_BLANK;
  pp->n_line = 0;
  pp->rank = 0;

  if( pp->tcb != NULL ){
    GFL_TCBL_Delete( pp->tcb );
    pp->tcb = NULL;
  }
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
 *  @brief  指定データindexを持つパネルを探す
 */
static PANEL_WORK* panel_GetPanelFromDataIndex( BEACON_VIEW_PTR wk, u8 data_idx )
{
  int i;
  PANEL_WORK* pp;
  for(i = 0;i < PANEL_MAX;i++){
    if( wk->panel[i].data_idx == data_idx ){
      return &wk->panel[i];
    }
  }
  if(data_idx == PANEL_DATA_BLANK){
    GF_ASSERT(0);
    return NULL;
  }
  return NULL;
}

/*
 *  @brief  パネル　Unionアクター更新
 */
static void panel_UnionObjUpdate( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info)
{
  u8 sex, char_no, p_ofs;
 
  char_no = GAMEBEACON_Get_TrainerView(info);

  //キャラクタ転送
  GFL_CLGRP_CGR_Replace(  wk->objResUnion.res[ OBJ_RES_CGR ].tbl[ pp->id ],
                          wk->resCharUnion[ char_no ].p_char );
  //パレット転送
  p_ofs = 16*sc_wifi_unionobj_plt[char_no];
  PaletteWorkSet( wk->pfd, &wk->resPlttUnion.dat[p_ofs], FADE_SUB_OBJ, (ACT_PAL_UNION+pp->id)*16, 0x20 );
}

/*
 *  @brief  パネル　アイコンアクター更新
 */
static void panel_IconObjUpdate( BEACON_VIEW_PTR wk, PANEL_WORK* pp, u8 icon)
{
  //キャラクタ転送
  GFL_CLGRP_CGR_Replace(  wk->objResIcon.res[ OBJ_RES_CGR ].tbl[ pp->id ],
                          wk->resCharIcon[ icon ].p_char );
}

/*
 *  @brief  パネルカラーゲット 
 */
static u8 panel_FrameColorGet( GAMEBEACON_INFO* info )
{
  int version = GAMEBEACON_Get_PmVersion(info);

  switch( version ){
  case VERSION_BLACK:
    return 0;
  case VERSION_WHITE:
    return 1;
  }
  return 2;
}

/*
 *  @brief  パネルカラーセット
 */
static void panel_ColorPlttSet( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info )
{
  u16 pal[5];
  u8  col;
  
  //パネルフレームカラー
  col = panel_FrameColorGet( info );
  pal[0] = wk->resPlttPanel.dat[ col*16+0 ];
  pal[1] = wk->resPlttPanel.dat[ col*16+1 ];
  pal[2] = wk->resPlttPanel.dat[ col*16+2 ];
  
  //パネルベースカラー
  GAMEBEACON_Get_FavoriteColor((GXRgb*)&pal[3], info);
  SoftFade(&pal[3], &pal[4], 1, 3, 0x0000); //補色を作成
 
  PaletteWorkSet( wk->pfd, pal, FADE_SUB_OBJ, (ACT_PAL_PANEL+pp->id)*16+1, 2*5 );
}

/*
 *  @brief  パネル　文字列描画
 */
static void panel_MsgPrint( BEACON_VIEW_PTR wk, PANEL_WORK* pp, STRBUF* str )
{
  GFL_BMP_Clear( pp->msgOam.bmp, 0 );
	PRINTSYS_PrintColor( pp->msgOam.bmp, 0, 0, str, wk->fontHandle, FCOL_FNTOAM_W );
	BmpOam_ActorBmpTrans( pp->msgOam.oam );
}

/*
 *  @brief  パネル　名前文字列描画
 */
static void panel_NamePrint( BEACON_VIEW_PTR wk, PANEL_WORK* pp, BOOL force_f )
{
  u8  flag;
  u16 time = GAMEBEACON_Get_RecvBeaconTime( pp->tr_id );

  flag = (time >= BEACON_TIMEOUT_FRAME );

  if( flag == pp->timeout_f && !force_f){
    return;
  }
  pp->timeout_f = flag;

  //再描画
  GFL_BMP_Clear( pp->msgOam.bmp, 0 );
  if( flag ){
//    IWASAWA_Printf("BeaconTimeout tr_id = %d, %d\n", pp->tr_id, time );
  	PRINTSYS_PrintColor( pp->msgOam.bmp, 0, 0, pp->name, wk->fontHandle, FCOL_FNTOAM_G );
  }else{
  	PRINTSYS_PrintColor( pp->msgOam.bmp, 0, 0, pp->name, wk->fontHandle, FCOL_FNTOAM_W );
  }
	BmpOam_ActorBmpTrans( pp->msgOam.oam );
}

/*
 *  @brief  パネル　ランク描画
 */
static void panel_RankSet( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info )
{
  pp->rank = GAMEBEACON_Get_ResearchTeamRank(info);
  if(pp->rank == 0){
    GFL_CLACT_WK_SetAnmFrame( pp->cRank, 0);
  }else{
    GFL_CLACT_WK_SetAnmFrame( pp->cRank, FX32_CONST(pp->rank-1));
  }
}

/*
 *  @brief  パネル描画
 */
static void panel_Draw( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info )
{
  //トレーナーID取得
  pp->tr_id = GAMEBEACON_Get_TrainerID( info );
  pp->sex = GAMEBEACON_Get_Sex( info );

  //プレイヤー名取得
  GAMEBEACON_Get_PlayerNameToBuf( info, pp->name );
  //メッセージバッファ初期化
  GFL_STR_ClearBuffer( pp->str );
  GAMEBEACON_Get_SelfIntroduction( info, pp->str );

  //プレイヤー名転送
  panel_NamePrint( wk, pp, TRUE );
  
  //ランクセット
  panel_RankSet( wk, pp, info);

  //UnionOBJリソース更新
  panel_UnionObjUpdate( wk, pp, info);

  //パネルカラー転送
  panel_ColorPlttSet( wk, pp, info );
}

/*
 *  @brief  パネル　位置セット
 */
static void  panel_SetPos( PANEL_WORK* pp, s16 x, s16 y )
{
  pp->px = x;
  pp->py = y;
  act_SetPosition( pp->cPanel, x, y );
  act_SetPosition( pp->cRank, x+ACT_RANK_OX, y+ACT_RANK_OY );
  act_SetPosition( pp->cUnion, x+ACT_UNION_OX, y+ACT_UNION_OY );
  act_SetPosition( pp->cIcon, x+ACT_ICON_OX, y+ACT_ICON_OY );
  BmpOam_ActorSetPos( pp->msgOam.oam, x+ACT_MSG_OX, y+ACT_MSG_OY);
}

/*
 *  @brief  描画パネルタイムアウトチェック
 */
static void list_TimeOutCheck( BEACON_VIEW_PTR wk )
{
  int i;

   //最新リストの名前のみ描画しなおし
  for(i = 0;i < wk->ctrl.view_max;i++){
    int idx;
    PANEL_WORK* pp;

    idx = GAMEBEACON_InfoTblRing_Ofs2Idx( wk->infoLog, wk->ctrl.view_top+i );
    pp = panel_GetPanelFromDataIndex( wk, idx );
    if( pp->tcb == NULL ){  //メッセージ描画中はスキップ
      panel_NamePrint( wk, pp, FALSE );
    }
  }
}

/*
 *  @brief  リストアップダウンリクエスト
 */
static void list_UpDownReq( BEACON_VIEW_PTR wk, u8 dir )
{
  u8 ofs,idx;

  if( dir == SCROLL_UP ){
    ofs = wk->ctrl.view_top+wk->ctrl.view_max;
    BEACON_STATUS_SetViewTopOffset( wk->b_status, ++wk->ctrl.view_top );
  }else{
    ofs = wk->ctrl.view_top-1;
    BEACON_STATUS_SetViewTopOffset( wk->b_status, --wk->ctrl.view_top );
  }
  GAMEBEACON_InfoTblRing_GetBeacon( wk->infoLog, wk->tmpInfo, &wk->tmpTime, ofs );
  idx = GAMEBEACON_InfoTblRing_Ofs2Idx( wk->infoLog, ofs );
  
  //スクロールリクエスト
  list_ScrollReq( wk, wk->tmpInfo, ofs, idx, dir, FALSE);
}

/*
 *  @brief  ビュー スクロール処理リクエスト
 */
static void list_ScrollReq( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info, u8 ofs, u8 idx, u8 dir, BOOL new_f )
{
  PANEL_WORK* pp;

  //空きパネル検索
  pp = panel_GetPanelFromDataIndex( wk, PANEL_DATA_BLANK );
  if( pp == NULL){
    return; //万一のためのチェック(正しい挙動ならNULLはない)
  }
  panel_Entry( pp, idx, 0 );  //パネル新規登録(ラインは後で初期化)
  panel_Draw( wk, pp, wk->tmpInfo );   //パネル描画
  panel_VisibleSet( pp, TRUE );   //パネル描画
  
  //スクロールパターン
  if( new_f ){  //スライドイン
    effReq_SetIconEffect( wk, pp, info, TRUE );
    effReq_PanelSlideIn( wk, pp );
  }else{
    effReq_PanelScroll( wk, dir, pp, NULL );
  }
  BeaconView_MenuBarViewSet( wk, MENU_ALL, MENU_ST_OFF );
}

static int list_GetScrollLineNum( BEACON_VIEW_PTR wk, BOOL new_f )
{
  if( !new_f ){
    return 0;
  }
  if( wk->ctrl.max == 1 ) {
     return 0;
  }
  return 1;
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  パネルスクロール
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_PANEL_SCROLL{
  int seq;
  BEACON_VIEW_PTR bvp;
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

static void taskAdd_PanelScroll( BEACON_VIEW_PTR wk, PANEL_WORK* pp, u8 dir, u8 wait, int* task_ct );
static void tcb_PanelScroll( GFL_TCBL *tcb , void* work);

/*
 *  @brief  パネルスクロールリクエスト
 */
static void effReq_PanelScroll( BEACON_VIEW_PTR wk, u8 dir, PANEL_WORK* new_pp, PANEL_WORK* egnore_pp )
{
  int i;
  PANEL_WORK* pp;

  if( new_pp != NULL ){
    if( dir == SCROLL_UP ){
      new_pp->n_line = PANEL_LINE_END;
    }else{
      new_pp->n_line = 0;
    }
    panel_SetPos( new_pp, new_pp->n_line*ACT_PANEL_OX+ACT_PANEL_PX,
                          new_pp->n_line*ACT_PANEL_OY+ACT_PANEL_PY );
    panel_VisibleSet( new_pp, TRUE );
  }
  for(i = 0;i < PANEL_MAX;i++){
    pp = &wk->panel[i];

    if( (pp->data_idx == PANEL_DATA_BLANK) ||
        ((egnore_pp != NULL) && pp->id == egnore_pp->id)){
      continue;
    }
    taskAdd_PanelScroll( wk, pp, dir, 0, &wk->eff_task_ct );
  }
}

/*
 *  @brief  パネルスライドインリクエスト
 */
static void effReq_PanelSlideIn( BEACON_VIEW_PTR wk, PANEL_WORK* pp )
{
  int i,deray = 0;
  PANEL_WORK* panel;

  pp->n_line = PANEL_VIEW_START;
  panel_SetPos( pp, ACT_PANEL_SI_SX, ACT_PANEL_SI_SY );
  panel_VisibleSet( pp, TRUE );
  if( wk->ctrl.max > 0 ){
    deray = PANEL_SCROLL_FRAME;
  }
  taskAdd_PanelScroll( wk, pp, SCROLL_RIGHT, deray, &wk->eff_task_ct );

  effReq_PanelScroll( wk, SCROLL_DOWN, NULL, pp );
}

/*
 *  @brief  パネルスクロールタスク登録
 */
static void taskAdd_PanelScroll( BEACON_VIEW_PTR wk, PANEL_WORK* pp, u8 dir, u8 wait, int* task_ct )
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

  twk->bvp = wk;
  twk->pp = pp;
  twk->epx = pp->n_line*ACT_PANEL_OX+ACT_PANEL_PX; 
  twk->epy = pp->n_line*ACT_PANEL_OY+ACT_PANEL_PY; 
  twk->x = FX32_CONST(pp->px);
  twk->y = FX32_CONST(pp->py);
  twk->frame = PANEL_SCROLL_FRAME;
  twk->deray = wait;
  twk->ax = FX_Div( FX32_CONST( twk->epx - pp->px ), FX32_CONST(twk->frame));
  twk->ay = FX_Div( FX32_CONST( twk->epy - pp->py ), FX32_CONST(twk->frame));

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_PanelScroll( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_PANEL_SCROLL* twk = (TASKWK_PANEL_SCROLL*)tcb_wk;

  if( twk->deray > 0 ){
    twk->deray--;
    return;
  }
  if( ++twk->ct < twk->frame){
    twk->x += twk->ax;
    twk->y += twk->ay;
    panel_SetPos( twk->pp, FX_Whole(twk->x), FX_Whole(twk->y));
    return;
  }
  panel_SetPos( twk->pp, twk->epx, twk->epy);

  if( twk->pp->n_line == 0 || twk->pp->n_line == PANEL_LINE_END){
    panel_Clear( twk->pp );  //パネル破棄
  }
  obj_UpDownViewSet( twk->bvp );
  --(*twk->task_ct);
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

static void taskAdd_MsgUpdown( BEACON_VIEW_PTR wk, u8 dir, int* task_ct );
static void tcb_MsgUpdown( GFL_TCBL *tcb , void* work);

/*
 *  @brief  メッセージウィンドウ アップダウンタスク登録
 */
static void taskAdd_MsgUpdown( BEACON_VIEW_PTR wk, u8 dir, int* task_ct )
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

static BOOL effReq_PopupMsg( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info, BOOL new_f )
{
  GAMEBEACON_ACTION action = GAMEBEACON_Get_Action_ActionNo( info );

  if( new_f ){
    GAMEBEACON_InfoWordset( info, wk->wordset, wk->tmpHeapID );
    effReq_PopupMsgSys( wk, msg_beacon_001 );
    return TRUE;
  }
  switch( action ){
  case GAMEBEACON_ACTION_GPOWER:
    return effReq_PopupMsgGPower( wk, info );
  
  case GAMEBEACON_ACTION_THANKYOU:
    //御礼を受けた回数インクリメント
    GAMEBEACON_Set_ThankyouOver( MISC_CrossComm_IncThanksRecvCount( wk->misc_sv ) );

    //ブレイクスルー
  default:
    effReq_PopupMsgFromInfo( wk, info );
    break;
  }
  return TRUE;
}

static void effReq_PopupMsgFromInfo( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info )
{
  GAMEBEACON_InfoWordset( info, wk->wordset, wk->tmpHeapID );
  print_GetMsgToBuf( wk, sub_GetBeaconMsgID(info) );
  taskAdd_WinPopup( wk, wk->str_expand, &wk->eff_task_ct);
}

static void effReq_PopupMsgSys( BEACON_VIEW_PTR wk, u8 msg_id )
{
  print_GetMsgToBuf( wk, msg_id );
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

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_WinPopup( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_WIN_POPUP* twk = (TASKWK_WIN_POPUP*)tcb_wk;

  switch( twk->seq ){
  case 0:
    print_PopupWindow( twk->bvp, twk->bvp->str_popup, 0 );
    twk->seq++;
    return;
  case 1:
    taskAdd_MsgUpdown( twk->bvp, SCROLL_UP, &twk->child_task );
    twk->seq++;
    return;
  case 2:
    if( twk->child_task ){
      return;
    }
    twk->seq++;
    return;
  case 3:
    if( print_PopupWindowTimeWaitCheck( &twk->wait ) == FALSE ){
      return;
    }
    taskAdd_MsgUpdown( twk->bvp, SCROLL_DOWN, &twk->child_task );
    twk->seq++;
    return;
  case 4:
    if( twk->child_task ){
      return;
    }
  }
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

static BOOL effReq_PopupMsgGPower( BEACON_VIEW_PTR wk, GAMEBEACON_INFO* info )
{
  GPOWER_ID gpower_id = GAMEBEACON_Get_GPowerID( info );

  //まったく同じパワーが発動中ならスルーする
  if( GPOWER_Check_OccurID( gpower_id, wk->gpower_data ) == gpower_id ){
    return FALSE;
  }
  GAMEBEACON_InfoWordset( info, wk->wordset, wk->tmpHeapID );
  WORDSET_ExpandStr( wk->wordset, wk->str_expand, wk->str_tmp );
  taskAdd_WinGPower( wk, gpower_id, GPOWER_USE_BEACON, &wk->eff_task_ct);
  return TRUE;
}
static void effReq_PopupMsgGPowerMine( BEACON_VIEW_PTR wk )
{
  taskAdd_WinGPower( wk,
      wk->my_data.power, GPOWER_USE_MINE, &wk->eff_task_ct );
}
static void effReq_PopupMsgGPowerSpecial( BEACON_VIEW_PTR wk, GPOWER_ID sp_gpower_id )
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
  
    twk->mypower_use = GPOWER_Check_MyPower();
    if( twk->mypower_use > 0 ){
      twk->mypower_min = twk->mypower_use/60;
      twk->mypower_sec = twk->mypower_use%60;
      WORDSET_RegisterNumber( wk->wordset, 4, twk->mypower_min, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( wk->wordset, 5, twk->mypower_sec, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    }
  }
  if( twk->type != GPOWER_USE_MINE ){
    BeaconView_MenuBarViewSet( wk, MENU_POWER, MENU_ST_OFF );
  }
  BeaconView_MenuBarViewSet( wk, MENU_HELLO, MENU_ST_OFF );
  BeaconView_MenuBarViewSet( wk, MENU_THANKS, MENU_ST_OFF );
  BeaconView_MenuBarViewSet( wk, MENU_RETURN, MENU_ST_OFF );

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

  if( !bvp->active || (bvp->event_id != EV_NONE)){
    return;
  }

  switch( twk->seq ){
  case 0:
    GFL_BMP_Clear( bvp->win[WIN_POPUP].bmp, FCOL_POPUP_BASE );
    print_GetMsgToBuf( bvp, DATA_GPowerUseMsg[twk->type] );
    print_PopupWindow( bvp, bvp->str_expand, 0 );
    taskAdd_MsgUpdown( bvp, SCROLL_UP, &twk->child_task );
    twk->seq++;
    return;
  case 1:
    if( twk->child_task > 0 ){
      return;
    }
    tmenu_Create( bvp, TMENU_ID_YESNO );
    obj_MenuIconVisibleSet( bvp, FALSE );
    twk->seq++;
    return;
  case 2: //選択待ち
    {
      int ret = tmenu_Update( bvp, TMENU_ID_YESNO );
      if( ret < 0 ){
        return;
      }
      twk->ret = ret;
      twk->seq++;
    }
    return;
  case 3: //決定アニメ待ち
    if( !tmenu_YnEndWait( bvp, twk->ret )){
      return;
    }

    switch( twk->ret ){
    case TMENU_YN_CHECK:
      bvp->event_id = EV_GPOWER_CHECK;
      twk->seq = 2;
      return;
    case TMENU_YN_NO:
      twk->seq = 5;
      break;
    case TMENU_YN_YES:
      print_GetMsgToBuf( bvp, msg_sys_gpower_use_end );
      print_PopupWindow( bvp, bvp->str_expand, 0 );

      //使うGパワーを覚えておく
      bvp->ctrl.g_power = twk->g_power;
      twk->seq++;
      break;
    }
    draw_MenuWindow( bvp, msg_sys_now_record );
    obj_MenuIconVisibleSet( bvp, TRUE );
    return;
  case 4:
    if( print_PopupWindowTimeWaitCheck( &twk->wait ) ){
      twk->seq++;
    }
    return;
  case 5:
    taskAdd_MsgUpdown( bvp, SCROLL_DOWN, &twk->child_task );
    twk->seq++;
    return;
  case 6:
    if( twk->child_task ){
      return;
    }
  }
  BeaconView_MenuBarViewSet( twk->bvp, MENU_ALL, MENU_ST_ON );
  
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

static void tcb_WinGPowerCheck( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_WIN_GPOWER* twk = (TASKWK_WIN_GPOWER*)tcb_wk;
  BEACON_VIEW_PTR bvp = twk->bvp;

  if( !bvp->active ){
    return;
  }

  switch( twk->seq ){
  case 0:
    GFL_BMP_Clear( bvp->win[WIN_POPUP].bmp, FCOL_POPUP_BASE );
    
    if( twk->mypower_use ){ //既に使用中
      print_GetMsgToBuf( bvp, msg_sys_gpower_use_err02+(twk->mypower_min != 0) );
    }else if( twk->item_num < twk->item_use ){  //アイテムが足りない
      print_GetMsgToBuf( bvp, msg_sys_gpower_use_err01);
    }
    print_PopupWindow( bvp, bvp->str_expand, 0 );
    taskAdd_MsgUpdown( bvp, SCROLL_UP, &twk->child_task );
    twk->seq++;
    return;
  case 1:
    if( twk->child_task > 0 ){
      return;
    }
    obj_MenuIconVisibleSet( bvp, FALSE );
    twk->seq++;
    return;
  case 2:
    tmenu_Create( bvp, TMENU_ID_CHECK );
    twk->seq++;
    return;
  case 3: //選択待ち
    {
      int ret = tmenu_Update( bvp, TMENU_ID_CHECK );
      if( ret < 0 ){
        return;
      }
      twk->ret = ret;
      twk->seq++;
    }
    return;
  case 4: //決定アニメ待ち
    if( !tmenu_CheckEndWait( bvp, twk->ret )){
      return;
    }
    draw_MenuWindow( bvp, msg_sys_now_record );
    obj_MenuIconVisibleSet( bvp, TRUE );
    taskAdd_MsgUpdown( bvp, SCROLL_DOWN, &twk->child_task );
    twk->seq++;
    return;
  case 5:
    if( twk->child_task ){
      return;
    }
  }
  bvp->gpower_check_req = ( twk->ret == TMENU_CHECK_CALL );
  BeaconView_MenuBarViewSet( twk->bvp, MENU_ALL, MENU_ST_ON );
  
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  パネルアイコン表示 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_ICON_EFF{
  u8  seq;
  u8  wait;
  int child_task;

  PANEL_WORK* pp;
  GAMEBEACON_ACTION action;
  BEACON_VIEW_PTR bvp;
  int* task_ct;
}TASKWK_ICON_EFF;

static void taskAdd_IconEffect( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info, BOOL new_f );
static void tcb_IconEffect( GFL_TCBL *tcb , void* work);

static void effReq_SetIconEffect( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info, BOOL new_f )
{
  taskAdd_IconEffect( wk, pp, info, new_f );
}

/*
 *  @brief  メッセージウィンドウ　アイコン表示タスク登録
 */
static void taskAdd_IconEffect( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info, BOOL new_f )
{
  GFL_TCBL* tcb;
  TASKWK_ICON_EFF* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_IconEffect, sizeof(TASKWK_ICON_EFF), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_ICON_EFF ));

  twk->bvp = wk;
  twk->pp = pp;
  twk->wait = ICON_POP_TIME;

  if( new_f ){
    twk->action = GAMEBEACON_ACTION_SEARCH;
  }else{
    twk->action = GAMEBEACON_Get_Action_ActionNo( info );
  }

  switch( twk->action ){
  case GAMEBEACON_ACTION_SEARCH:
    panel_MsgPrint( wk, pp, pp->str ); 
    break;
  case GAMEBEACON_ACTION_THANKYOU:
    GAMEBEACON_Get_ThankyouMessage(info, wk->str_tmp );
    panel_MsgPrint( wk, pp, wk->str_tmp ); 
    break;
  case GAMEBEACON_ACTION_FREEWORD:
    GAMEBEACON_Get_FreeWordMessage(info, wk->str_tmp );
    panel_MsgPrint( wk, pp, wk->str_tmp ); 
    break;
  }
  panel_IconObjUpdate( wk, pp, GAMEBEACON_GetActionDataIconType( twk->action ) );
  panel_IconVisibleSet( pp, TRUE );
}

static void tcb_IconEffect( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_ICON_EFF* twk = (TASKWK_ICON_EFF*)tcb_wk;

  if( twk->wait-- > 0 ){
    return;
  }
  panel_NamePrint( twk->bvp, twk->pp, TRUE ); 
  panel_IconVisibleSet( twk->pp, FALSE );
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  パネル 点滅表示 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_PANEL_FLASH{
  u8  ct;
  u8  wait;
  u8  interval;
  u8  pal_pos;
  int child_task;

  PANEL_WORK* pp;
  BEACON_VIEW_PTR bvp;
  int* task_ct;
}TASKWK_PANEL_FLASH;

static void taskAdd_PanelFlash( BEACON_VIEW_PTR wk, PANEL_WORK* pp, int* task_ct );
static void tcb_PanelFlash( GFL_TCBL *tcb , void* work);

static void effReq_SetPanelFlash( BEACON_VIEW_PTR wk, u8 target_ofs )
{
  PANEL_WORK* pp;
  u8 idx = GAMEBEACON_InfoTblRing_Ofs2Idx( wk->infoLog, target_ofs+wk->ctrl.view_top );
  
  pp = panel_GetPanelFromDataIndex( wk, idx );
  taskAdd_PanelFlash( wk, pp, &wk->eff_task_ct );
  sub_PlaySE( BVIEW_SE_DECIDE );
}

/*
 *  @brief  メッセージウィンドウ パネルフラッシュタスク登録
 */
static void taskAdd_PanelFlash( BEACON_VIEW_PTR wk, PANEL_WORK* pp, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_PANEL_FLASH* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_PanelFlash, sizeof(TASKWK_PANEL_FLASH), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_PANEL_FLASH ));

  twk->bvp = wk;
  twk->pp = pp;
  twk->interval = 2;
  twk->pal_pos = (ACT_PAL_PANEL+twk->pp->id)*16;
  
  SoftFadePfd( twk->bvp->pfd, FADE_SUB_OBJ, twk->pal_pos, 16, 4, 0x7FFF );

  if( task_ct != NULL ){
    twk->task_ct = task_ct;
    (*twk->task_ct)++;
  }
}

static void tcb_PanelFlash( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_PANEL_FLASH* twk = (TASKWK_PANEL_FLASH*)tcb_wk;

  if( twk->ct < 2 ){
    if( twk->wait++ < twk->interval ){
      return;
    }
    SoftFadePfd( twk->bvp->pfd, FADE_SUB_OBJ, twk->pal_pos, 16, 4*(twk->ct%2), 0x7FFF );

    twk->ct++;
    twk->wait = 0;
    return;
  }
  SoftFadePfd( twk->bvp->pfd, FADE_SUB_OBJ, twk->pal_pos, 16, 0, 0x7FFF );
  if( twk->task_ct != NULL ){
    (*twk->task_ct)--;
  }
  GFL_TCBL_Delete(tcb);
}



