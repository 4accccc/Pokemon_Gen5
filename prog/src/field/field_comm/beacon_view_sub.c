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

//////////////////////////////////////////////////////////////////

static BOOL list_IsView( BEACON_VIEW_PTR wk, u8 idx );

//////////////////////////////////////////////////////////////////
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
          wk->tmpInfo, &wk->tmpTime, &new_f );
  idx = GAMEBEACON_InfoTblRing_Ofs2Idx( ofs );

  //登録数更新
  wk->ctrl.max += new_f;
  if( wk->ctrl.max <= PANEL_LINE_MAX){
    wk->ctrl.view_max = wk->ctrl.max;
  }
  //ポップアップリクエスト

  //新規でない時はスクロール無し
  if( !new_f ){
    pp = panel_GetPanelFromDataIndex( wk, idx );
    if(pp != NULL){ //ターゲットパネル書換え
      panel_Draw( wk, pp, info );
    }
    return TRUE;
  }

  //空きパネル検索
  pp = panel_GetPanelFromDataIndex( PANEL_DATA_BLANK );
  if( pp == NULL){
    return TRUE;  //万一のためのチェック(正しい挙動ならNULLはない)
  }
  
  if( wk->ctrl.view_top > 0){ //描画リストがトップでない時はスクロールのみ
    ofs = wk->ctrl.view_top-1;
    GAMEBEACON_InfoTblRing_GetBeacon( wk->infoLog, &wk->tmpInfo, &wk->tmpTime, ofs );
    idx = GAMEBEACON_InfoTblRing_Ofs2Idx( ofs );
  }
  panel_Entry( pp, ofs, idx );  //パネル新規登録
  panel_Draw( wk, pp, info );   //パネル描画
  
  //スクロールパターン
  if( wk->ctrl.view_top == 0){  //スライドイン
  
  }else{
  
  }

  //スクロールリクエスト
  return TRUE;
}

/*
 *  @パレット転送
 */
static sub_PlttVramTrans( u16* p_data, u16 pos, u16 num )
{
  u32 adrs = pos*2;
  u32 siz = num*2;

  DC_FlushRange( p_data, siz );
  GXS_LoadOBJPltt( p_data, adrs, siz );
}

/*
 *  @brief  パネル 表示状態On/Off
 */
static panel_VisibleSet( PANEL_WORK* pp, BOOL visible_f )
{
  BmpOam_ActorSetDrawEnable( pp->msgOam.oam, visible_f );
  GFL_CLACT_WK_SetDrawEnable( pp->cIcon, visible_f );
  GFL_CLACT_WK_SetDrawEnable( pp->cUnion, visible_f );
  GFL_CLACT_WK_SetDrawEnable( pp->cPanel, visible_f );
}

/*
 *  @brief  パネルクリアー
 */
static void panel_Clear( PANEL_WORK* pp )
{
  panel_VisibleSet( pp, FALSE );

  GFL_BMP_Clear( pp->msgOam.bmp, 0 );
	BmpOam_ActorBmpTrans( pp->msgOam.oam );
  GFL_STR_ClearBuffer( pp->name );
  GFL_STR_ClearBuffer( pp->str );

  pp->data_idx = PANEL_DATA_BLANK;
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
  sub_PlttVramTrans( wk->resPlttUnion.dat, (ACT_PAL_UNION+pp->id)*16, 16 );
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
  
  sub_PlttVramTrans( pal, (ACT_PAL_PANEL+pp->id)*16+1, 2 );
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
 *  @brief  指定したindexのデータが描画ラインにいるかどうか？
 */
static list_panel_IsView( BEACON_VIEW_PTR wk, u8 idx )
{
  int i;

  for( i = 0;i < )
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

