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

//////////////////////////////////////////////////////////////////

static void draw_LogNumWindow( BEACON_VIEW_PTR wk );
static void draw_MenuWindow( BEACON_VIEW_PTR wk, u8 msg_id );

static void sub_PlttVramTrans( u16* p_data, u16 pos, u16 num );

static void panel_VisibleSet( PANEL_WORK* pp, BOOL visible_f );
static void panel_IconVisibleSet( PANEL_WORK* pp, BOOL visible_f );
static void panel_Clear( PANEL_WORK* pp );
static void panel_Entry( PANEL_WORK* pp, u8 data_ofs, u8 data_idx );
static PANEL_WORK* panel_GetPanelFromDataIndex( BEACON_VIEW_PTR wk, u8 data_idx );
static void panel_UnionObjUpdate( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info);
static u8 panel_FrameColorGet( GAMEBEACON_INFO* info );
static void panel_ColorPlttSet( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info );
static void panel_MsgPrint( BEACON_VIEW_PTR wk, PANEL_WORK* pp, STRBUF* str );
static void panel_Draw( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info );

static void effReq_PanelScroll( BEACON_VIEW_PTR wk, u8 dir, PANEL_WORK* new_pp, PANEL_WORK* ignore_pp );
static void effReq_PanelSlideIn( BEACON_VIEW_PTR wk, PANEL_WORK* pp );

static BOOL list_IsView( BEACON_VIEW_PTR wk, u8 idx );

//////////////////////////////////////////////////////////////////

/*
 *  @brief  初期描画
 */
void BeaconView_InitialDraw( BEACON_VIEW_PTR wk )
{
  //現在のログ数
  draw_LogNumWindow( wk );
  //現在のトータルすれ違い人数
  draw_MenuWindow( wk, msg_sys_now_record );

}

/*
 *  @brief  スタックからログを一件取り出し
 */
BOOL BeaconView_CheckStack( BEACON_VIEW_PTR wk )
{
  BOOL new_f = 0;
  int ofs,idx;
  PANEL_WORK* pp;

  //スタックに積まれた情報をチェック
  if( !GAMEBEACON_Stack_GetInfo( wk->infoStack, wk->tmpInfo, &wk->tmpTime ) ){
    return FALSE;
  }
  ofs = GAMEBEACON_InfoTblRing_SetBeacon( wk->infoLog,
          wk->tmpInfo, wk->tmpTime, &new_f );
  idx = GAMEBEACON_InfoTblRing_Ofs2Idx( wk->infoLog, ofs );

  //登録数更新
  if( wk->ctrl.max < BS_LOG_MAX ){
    wk->ctrl.max += new_f;
  
    if( wk->ctrl.max <= PANEL_VIEW_MAX){
      wk->ctrl.view_max = wk->ctrl.max;
    }
  }
  //ポップアップリクエスト

  //新規でない時はスクロール無し
  if( !new_f ){
    pp = panel_GetPanelFromDataIndex( wk, idx );
    if(pp != NULL){ //ターゲットパネル書換え
      panel_Draw( wk, pp, wk->tmpInfo );
    }
    return TRUE;
  }

  //空きパネル検索
  pp = panel_GetPanelFromDataIndex( wk, PANEL_DATA_BLANK );
  if( pp == NULL){
    return TRUE;  //万一のためのチェック(正しい挙動ならNULLはない)
  }
  
  if( wk->ctrl.view_top > 0){ //描画リストがトップでない時はスクロールのみ
    ofs = wk->ctrl.view_top-1;
    GAMEBEACON_InfoTblRing_GetBeacon( wk->infoLog, wk->tmpInfo, &wk->tmpTime, ofs );
    idx = GAMEBEACON_InfoTblRing_Ofs2Idx( wk->infoLog, ofs );
  }
  panel_Entry( pp, ofs, idx );  //パネル新規登録
  panel_Draw( wk, pp, wk->tmpInfo );   //パネル描画
  panel_VisibleSet( pp, TRUE );   //パネル描画
  
  //スクロールパターン
  if( wk->ctrl.view_top == 0){  //スライドイン
    effReq_PanelSlideIn( wk, pp );
  }else{
    effReq_PanelScroll( wk, SCROLL_DOWN, pp, NULL );
  }
  wk->ctrl.view_top = pp->data_ofs;

  //スクロールリクエスト
  return TRUE;
}

/*
 *  @brief  文字列一括描画
 */
static void print_Allput( BEACON_VIEW_PTR wk, GFL_BMP_DATA* bmp, u8 msg_id, u8 x, u8 y, u8 clear, PRINTSYS_LSB color )
{
  GFL_BMP_Clear( bmp, clear );
  
  GFL_MSG_GetString( wk->mm_status, msg_id, wk->str_tmp);
  WORDSET_ExpandStr( wk->wordset, wk->str_expand, wk->str_tmp );

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
 *  @パレット転送
 */
static void sub_PlttVramTrans( u16* p_data, u16 pos, u16 num )
{
  u32 adrs = pos*2;
  u32 siz = num*2;

#if 0
  DC_FlushRange( p_data, siz );
  GXS_LoadOBJPltt( p_data, adrs, siz );
#endif
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
 *  @brief  パネル 表示状態On/Off
 */
static void panel_VisibleSet( PANEL_WORK* pp, BOOL visible_f )
{
  BmpOam_ActorSetDrawEnable( pp->msgOam.oam, visible_f );
  GFL_CLACT_WK_SetDrawEnable( pp->cUnion, visible_f );
  GFL_CLACT_WK_SetDrawEnable( pp->cPanel, visible_f );
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
  pp->data_ofs = 0;
  pp->n_line = 0;
}

/*
 *  @brief  パネル登録
 */
static void panel_Entry( PANEL_WORK* pp, u8 data_ofs, u8 data_idx )
{
  panel_Clear( pp );
  pp->data_ofs = data_ofs;
  pp->data_idx = data_idx;
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
  u8 no, sex, char_no;
 
  no = GAMEBEACON_Get_TrainerView(info);
  sex = GAMEBEACON_Get_Sex(info);
 
  if(no > 8){
    no = 0;
  }
  if(sex > 1){
    sex = 0;
  }
  char_no = no+(sex*8);

  //キャラクタ転送
  GFL_CLGRP_CGR_Replace(  wk->objResUnion.res[ OBJ_RES_CGR ].tbl[ pp->id ],
                          wk->resCharUnion[ char_no ].p_char );
  //パレット転送
//  sub_PlttVramTrans( wk->resPlttUnion.dat, (ACT_PAL_UNION+pp->id)*16, 16 );
  PaletteWorkSet( wk->pfd, wk->resPlttUnion.dat, FADE_SUB_OBJ, (ACT_PAL_UNION+pp->id)*16, 0x20 );
}

/*
 *  @brief  パネルカラーゲット 
 */
static u8 panel_FrameColorGet( GAMEBEACON_INFO* info )
{
  u16 thanks = GAMEBEACON_Get_ThanksRecvCount( info );

  if( thanks < 11 ){
    return 0;
  }else if( thanks < 21 ){
    return 1;
  }else if( thanks < 41 ){
    return 2;
  }else if( thanks < 71 ){
    return 3;
  }
  return 4;
}

/*
 *  @brief  パネルカラーセット
 */
static void panel_ColorPlttSet( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info )
{
  u16 pal[2];
  u8  col;

  //パネルベースカラー
  GAMEBEACON_Get_FavoriteColor((GXRgb*)&pal[0], info);

  //パネルフレームカラー
  col = panel_FrameColorGet( info );
  pal[1] = wk->resPlttPanel.dat[ col ];
  
//  sub_PlttVramTrans( pal, (ACT_PAL_PANEL+pp->id)*16+1, 2 );
  PaletteWorkSet( wk->pfd, pal, FADE_SUB_OBJ, (ACT_PAL_PANEL+pp->id)*16+1, 2*2 );
}

/*
 *  @brief  パネル　文字列描画
 */
static void panel_MsgPrint( BEACON_VIEW_PTR wk, PANEL_WORK* pp, STRBUF* str )
{
  GFL_BMP_Clear( pp->msgOam.bmp, 1 );
	PRINTSYS_PrintColor( pp->msgOam.bmp, 0, 0, str, wk->fontHandle, FCOL_FNTOAM );
	BmpOam_ActorBmpTrans( pp->msgOam.oam );
}

/*
 *  @brief  パネル描画
 */
static void panel_Draw( BEACON_VIEW_PTR wk, PANEL_WORK* pp, GAMEBEACON_INFO* info )
{
  //プレイヤー名取得
  GAMEBEACON_Get_PlayerNameToBuf( info, pp->name );
  //メッセージバッファ初期化
  GFL_STR_ClearBuffer( pp->str );
  //プレイヤー名転送
  panel_MsgPrint( wk, pp, pp->name );

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
  act_SetPosition( pp->cUnion, x+ACT_UNION_OX, y+ACT_UNION_OY );
  act_SetPosition( pp->cIcon, x+ACT_ICON_OX, y+ACT_ICON_OY );
  BmpOam_ActorSetPos( pp->msgOam.oam, x+ACT_MSG_OX, y+ACT_MSG_OY);
}

/*
 *  @brief  指定したindexのデータが描画ラインにいるかどうか？
 */
static list_panel_IsView( BEACON_VIEW_PTR wk, u8 idx )
{
  int i;

  if(idx < wk->ctrl.view_top || idx >= wk->ctrl.view_btm ){
    return FALSE;
  }
  return TRUE;
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
    deray = 15;
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

  twk->pp = pp;
  twk->epx = pp->n_line*ACT_PANEL_OX+ACT_PANEL_PX; 
  twk->epy = pp->n_line*ACT_PANEL_OY+ACT_PANEL_PY; 
  twk->x = FX32_CONST(pp->px);
  twk->y = FX32_CONST(pp->py);
  twk->frame = 15;
  twk->deray = wait;
  twk->ax = FX_Div( FX32_CONST( twk->epx - pp->px ), FX32_CONST(twk->frame));
  twk->ay = FX_Div( FX32_CONST( twk->epy - pp->py ), FX32_CONST(twk->frame));

  IWASAWA_Printf( "Scroll dir=%d, start(%d,%d), end(%d,%d) a(%d,%d)\n",
      dir,pp->px,pp->py,twk->epx,twk->epy,(twk->epx-pp->px),(twk->epy-pp->py));
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
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}


